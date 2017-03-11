#include <sys/stat.h>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cassert>
#include "counter.h"
#include "counter_tags.h"
#include "util/split.h"
#include "util/dir.h"

namespace order_concepts {

#define resetCount(counter) \
    counter[Predicate::NORMAL][Polar::P] = 0; \
    counter[Predicate::NORMAL][Polar::N] = 0; \
    counter[Predicate::SYNONYM][Polar::P] = 0; \
    counter[Predicate::SYNONYM][Polar::N] = 0;
#define resetWordCount(counter, key) \
    counter[Predicate::NORMAL][Polar::P][key] = 0; \
    counter[Predicate::NORMAL][Polar::N][key] = 0; \
    counter[Predicate::SYNONYM][Polar::P][key] = 0; \
    counter[Predicate::SYNONYM][Polar::N][key] = 0;
#define resetWordPairCount(counter, key0, key1) \
    counter[Predicate::NORMAL][key0][key1] = 0; \
    counter[Predicate::SYNONYM][key0][key1] = 0;


Counter::Counter(
    const std::vector<std::string>& concepts,
    const std::string& adjective,
    const std::string& antonym)
: m_concepts(concepts), m_is_prep_mode(false)
{
    m_adjectives[Predicate::NORMAL].emplace_back(adjective);
    m_antonyms[Predicate::NORMAL].emplace_back(antonym);
    m_target_preds.emplace(Predicate::NORMAL);
}

Counter::~Counter()
{
    delete m_writer;
}

void Counter::resetCounters()
{
    // variables for statistic
    resetCount(m_total_adjective_occurrences);
    resetCount(m_total_adjective_dependencies);
    resetCount(m_total_adjective_similia);

    for (const auto& concept0 : m_concepts) {
        m_occurrences[concept0] = 0;

        // adj/ant
        resetWordCount(m_cooccurrences, concept0);
        resetWordCount(m_dependencies, concept0);
        resetWordCount(m_similia, concept0);

        for (const auto& concept1 : m_concepts) {
            if (concept0 != concept1) {
                resetWordPairCount(m_comparatives, concept0, concept1);
            }
        }
        // stats
        m_all_adjective_cooccurrences[concept0];
    }
}

void Counter::resetPatterns()
{
    std::vector<std::vector<std::string>>()
        .swap(m_simile_patterns);
    std::vector<std::vector<std::string>>()
        .swap(m_comparative_patterns);
}

bool Counter::readPatternFile(const std::string& pattern_filename, Patterns* patterns)
{
    // check path existence
    struct stat buffer;
    if (::stat(pattern_filename.c_str(), &buffer) != 0) return false;

    std::ifstream pattern_file;
    pattern_file.open(pattern_filename);

    std::string line;
    while (std::getline(pattern_file, line)) {
        // skip blank lines
        if (line.empty()) continue;

        std::vector<std::string> splitted_line;
        util::splitStringWithWhitespaces(line, &splitted_line);
        patterns->emplace_back(splitted_line);
    }
    pattern_file.close();
    return true;
}

bool Counter::readSimilePatterns()
{
    const std::string simile_file_path =
        m_pattern_file_path + "/" + SIMILE_PATTERN_FILE;
    return readPatternFile(simile_file_path, &m_simile_patterns);
}

bool Counter::readComarativePatterns()
{
    const std::string comparative_file_path =
        m_pattern_file_path + "/" + COMPARATIVE_PATTERN_FILE;
    return readPatternFile(comparative_file_path, &m_comparative_patterns);
}

bool Counter::readPatternFilesInPath(const std::string& path)
{
    resetPatterns();

    // check path existence
    struct stat buffer;
    if (::stat(path.c_str(), &buffer) != 0) return false;

    m_pattern_file_path = path;
    readSimilePatterns();
    readComarativePatterns();
    return true;
}

void Counter::count(Parser& parser)
{
    resetCounters();

    try {
        while (parser.next()) {
            WordSet found_concepts;
            countConcepts(parser, &found_concepts);

            if (isPrepMode()) {
                countStats(parser, found_concepts);
            }
            else {
                for (const auto& pred : m_target_preds) {
                    countCooccurrence(parser, found_concepts, pred);
                    countDependency(parser, found_concepts, pred);
                    countSimile(parser, found_concepts, pred);
                    countComparative(parser, found_concepts, pred);
                }
            }
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "counting error!" << std::endl << std::flush;
        throw e;
    }
}

bool Counter::findConcept(
    const WordSet& target_concepts,
    const std::vector<std::shared_ptr<Morph>>& morphs,
    int *cur_index,
    std::string* found_concept) const
{
    // check the POS of lemma
    if (morphs[*cur_index]->POS() != Morph::POSTag::NOUN) return false;

    // check morph not lemma
    // because some nouns are unknown and don't have lemma
    std::string concatenated_morphs = "";
    for (int i = *cur_index; i < morphs.size(); ++i) {
        const auto& morph = morphs[i];
        if (morph->POS() != Morph::POSTag::NOUN) break;

        // concatenate morphs
        concatenated_morphs.append(morph->morph());
        // check it is a concept
        if (target_concepts.find(concatenated_morphs) != target_concepts.end()) {
            // set found concept and index
            *found_concept = concatenated_morphs;
            *cur_index = i;
            return true;
        }
    }
    return false;
}

void Counter::countConcepts(const Parser& parser, WordSet* found_concepts)
{
    const std::vector<Phrase>& phrases = parser.phrases();
    for (const auto& phrase : phrases) {
        // search concept
        for (const auto& concept : m_concepts) {
            if (phrase.find(concept, Morph::POSTag::NOUN)) {
                found_concepts->emplace(concept); // check occurrence of concept
                m_occurrences[concept]++;
            }
        }
    }
}

CounterTags::Polar checkPolar(
    const bool did_find_adjective,
    const bool did_find_antonym,
    const Phrase& phrase)
{
    if (did_find_adjective && !phrase.isNegative()) return CounterTags::Polar::P;
    if (did_find_antonym   &&  phrase.isNegative()) return CounterTags::Polar::P;
    return CounterTags::Polar::N;
}

void Counter::countCooccurrence(
    const Parser& parser,
    const WordSet& found_concepts,
    const Predicate& pred)
{
    bool adj_flag[2] = {false, false}; // pos | neg

    const std::vector<Phrase>& phrases = parser.phrases();
    for (int i = 0; i < phrases.size(); ++i) {
        const auto& phrase = phrases[i];

        // === count statics (cooc / dep) === {{{
        // search adjective/antonym
        bool did_find_adjective = false;
        bool did_find_antonym = false;
        for (const auto& adjective : m_adjectives[pred]) {
            if (phrase.find(adjective, Morph::POSTag::ADJECTIVE)) {
                did_find_adjective = true;
                break;
            }
        }
        for (const auto& antonym : m_antonyms[pred]) {
            if (phrase.find(antonym, Morph::POSTag::ADJECTIVE)) {
                did_find_antonym = true;
                break;
            }
        }
        if (!did_find_adjective && !did_find_antonym) continue; // both not found

        // check dependency count
        // if this is not the first phrase,
        // this is depended on by leading phrases
        int dependency_count = 0;
        for (int j = 0; j < i; ++j) {
            const auto& leading_phrase = phrases[j];
            if (leading_phrase.depend() == i) dependency_count++;
        }

        Polar p = checkPolar(did_find_adjective, did_find_antonym, phrase);
        m_total_adjective_occurrences[pred][p]++;
        m_total_adjective_dependencies[pred][p] += dependency_count;
        adj_flag[p] = true;
        // === /count statics (cooc / dep) === }}}
    }

    for (auto p : {Polar::P, Polar::N}) {
        if (!adj_flag[p]) continue;
        for (const auto& concept : found_concepts) {
            m_cooccurrences[pred][p][concept]++;
            if (m_writer && pred == Predicate::NORMAL) m_writer->writeCooccurrenceReason(concept, parser.raw(), p);
        }
    }
}

void Counter::countDependency(
    const Parser& parser,
    const WordSet& target_concepts,
    const Predicate& pred)
{
    const std::vector<Phrase>& phrases = parser.phrases();
    for (const auto& phrase : phrases) {
        // search concept
        for (const auto& concept : target_concepts) {
            // check if concept is not in the phrase / it has no dependency
            if (!phrase.find(concept, Morph::POSTag::NOUN) || phrase.depend() < 0) continue;

            // check dependency
            const Phrase& target_phrase = phrases[phrase.depend()];
            bool did_find_adjective = false;
            bool did_find_antonym = false;
            for (const auto& adjective : m_adjectives[pred]) {
                if (target_phrase.find(adjective, Morph::POSTag::ADJECTIVE)) {
                    did_find_adjective = true;
                    break;
                }
            }
            for (const auto& antonym : m_antonyms[pred]) {
                if (target_phrase.find(antonym, Morph::POSTag::ADJECTIVE)) {
                    did_find_antonym = true;
                    break;
                }
            }

            if (did_find_adjective || did_find_antonym) {
                Polar p = checkPolar(did_find_adjective, did_find_antonym, target_phrase);
                m_dependencies[pred][p][concept]++;
                if (m_writer && pred == Predicate::NORMAL) m_writer->writeDependencyReason(concept, parser.raw(), p);
            }
        }
    }
}

#define MORPH_CHECK(morph, p, l) \
    (morph->POS() == Morph::POSTag::p \
    && (morph->lemma() == l))

void Counter::countSimile(
    const Parser& parser,
    const WordSet& target_concepts,
    const Predicate& pred)
{
    const std::vector<std::shared_ptr<Morph>>& morphs = parser.morphs();
    for (const auto& pattern : m_simile_patterns) {
        // if the pattern is longer than morphs, skip it
        if (pattern.size() > morphs.size()) continue;

        std::string found_concept;
        int pattern_index = 0;
        for (int i = 0; i < morphs.size(); ++i) {
            const auto& morph = morphs[i];
            const auto& p = pattern[pattern_index++];
            bool does_match = false;

            if (p == Counter::PATTERN_CONCEPT_0_TAG) {
                // check concept
                does_match =
                    findConcept(target_concepts,
                                morphs,
                                &i,
                                &found_concept);
            }
            else if (p == Counter::PATTERN_ADJECTIVE_TAG) {
                // check adjective/antonym
                // simile does not take account of negation
                bool did_find_adjective = false;
                bool did_find_antonym = false;
                for (const auto& adjective : m_adjectives[pred]) {
                    if (MORPH_CHECK(morph, ADJECTIVE, adjective)) {
                        did_find_adjective = true;
                        break;
                    }
                }
                for (const auto& antonym : m_antonyms[pred]) {
                    if (MORPH_CHECK(morph, ADJECTIVE, antonym)) {
                        did_find_antonym = true;
                        break;
                    }
                }

                does_match =
                    did_find_adjective || did_find_antonym;

                // count up a counter
                if (does_match) {
                    // NOTE: this assumes the last pattern element should be an adjective
                    Polar p;
                    if (did_find_adjective) p = Polar::P;
                    if (did_find_antonym) p = Polar::N;

                    m_similia[pred][p][found_concept]++;
                    if (m_writer && pred == Predicate::NORMAL) m_writer->writeSimileReason(found_concept, parser.raw(), p);
                }
            }
            else {
                // simply, check with patten's lemma
                does_match = (morph->lemma() == p);
            }

            if (!does_match || pattern_index == pattern.size()) {
                // check this morph again with the first element
                if (!does_match && pattern_index != 1) --i;
                pattern_index = 0;

                // check pattern.size() is larger than morphs.size() - (i + 1)
                // that's why this includes equality check
                if (pattern.size() >= morphs.size() - i) break;
            }
        }
    }

    // count for all patterns ending with adj
    for (const auto& pattern : m_simile_patterns) {
        // if the pattern is longer than morphs, skip it
        if (pattern.size() > morphs.size()) continue;

        int pattern_index = 1;  // skip concept (start)
        for (int i = 0; i < morphs.size(); ++i) {
            const auto& morph = morphs[i];
            const auto& p = pattern[pattern_index++];
            bool does_match = false;

            if (p == Counter::PATTERN_ADJECTIVE_TAG) {
                bool did_find_adjective = false;
                bool did_find_antonym = false;
                for (const auto& adjective : m_adjectives[pred]) {
                    if (MORPH_CHECK(morph, ADJECTIVE, adjective)) {
                        did_find_adjective = true;
                        break;
                    }
                }
                for (const auto& antonym : m_antonyms[pred]) {
                    if (MORPH_CHECK(morph, ADJECTIVE, antonym)) {
                        did_find_antonym = true;
                        break;
                    }
                }

                if (did_find_adjective) m_total_adjective_similia[pred][Polar::P]++;
                if (did_find_antonym) m_total_adjective_similia[pred][Polar::N]++;

                does_match =
                    did_find_adjective || did_find_antonym;
            }
            else {
                // simply, check with patten's lemma
                does_match = (morph->lemma() == p);
            }

            if (!does_match || pattern_index == pattern.size()) {
                // check this morph again with the first element
                // here, the first element is pattern[1]
                if (!does_match && pattern_index != 2) --i;
                pattern_index = 1;

                // check pattern.size() - 1 (skip concept)
                // is larger than morphs.size() - (i + 1)
                // that's why this DOES NOT include equality check
                if (pattern.size() > morphs.size() - i) break;
            }
        }
    }
}

bool canReachToIndex(const std::vector<int>& dependencies, int start, int target)
{
    return dependencies[start] == target;
}

bool canReachToIndexFromConceptWithoutTag(
    const std::vector<Phrase>& phrases,
    const std::vector<int>& dependencies, int start, int target)
{
    const std::string hiragana = "ほうが";
    const std::string kanji = "方が";
    return dependencies[start] != -1
        && (dependencies[start] == target
            || (dependencies[dependencies[start]] == target
                && (phrases[dependencies[start]].phrase().substr(0, hiragana.size()) == hiragana
                    || phrases[dependencies[start]].phrase().substr(0, kanji.size()) == kanji)));
}

void Counter::countComparative(
    const Parser& parser,
    const WordSet& target_concepts,
    const Predicate& pred)
{
    if (target_concepts.size() <= 1) return ;

    const std::vector<Phrase>& phrases = parser.phrases();

    // NOTE: this function assumes each concept / adjective / antonym appears at most once

    // concept -> (found_position, TAG [PATTERN_ADJECTIVE_TAG / PATTERN_NEG_ADJECTIVE_TAG])
    const std::string NO_TAG = "";
    std::unordered_map<std::string, std::pair<int, std::string>> concept_w_tag_positions;
    std::unordered_map<std::string, int> concept_wo_tag_positions;
    const int NOT_FOUND = -2;
    // adj/ant: (found_position, is_negative)
    std::pair<int, bool> adjective_position = {NOT_FOUND, false};
    std::pair<int, bool> antonym_position = {NOT_FOUND, false};

    for (int i = 0; i < phrases.size(); ++i) {
        const auto& phrase = phrases[i];
        // search concept
        for (const auto& concept : target_concepts) {
            if (!phrase.find(concept, Morph::POSTag::NOUN)) continue; // concept not found

            // check the concept has a comparative pattern
            bool does_find_pattern = false;
            for (const auto& pattern : m_comparative_patterns) {
                const std::string& lemma = pattern[0]; // e.g. ほど, より
                // PATTERN_ADJECTIVE_TAG or PATTERN_NEG_ADJECTIVE_TAG
                const std::string& tag = pattern[1];

                // search a pattern in the phrase
                for (const auto& morph : phrase.morphs()) {
                    if (morph->lemma() == lemma) {
                        does_find_pattern = true;
                        break;
                    }
                }
                if (does_find_pattern) {
                    concept_w_tag_positions[concept] = std::make_pair(i, tag); // add position with the tag
                    break;
                }
            }
            if (!does_find_pattern) concept_wo_tag_positions[concept] = i; // just add the position
        }
        // search adjective / antonym
        for (const auto& adjective : m_adjectives[pred])
            if (phrase.find(adjective, Morph::POSTag::ADJECTIVE))
                adjective_position = std::make_pair(i, phrase.isNegative());
        for (const auto& antonym : m_antonyms[pred])
            if (phrase.find(antonym, Morph::POSTag::ADJECTIVE))
                antonym_position = std::make_pair(i, phrase.isNegative());
    }

    // here, we know the position of concepts / adjective / antonym

    // check dependencies
    const std::vector<int>& dependencies = parser.connections();
    for (const auto& concept0 : concept_w_tag_positions) {
        // concept0's info
        const std::string& name0 = concept0.first;
        const int& pos0 = concept0.second.first;
        const std::string& tag0 = concept0.second.second;

        for (const auto& concept1 : concept_wo_tag_positions) {
            // concept1's info
            const std::string& name1 = concept1.first;
            const int& pos1 = concept1.second;

            // on adjective
            if (canReachToIndex(dependencies, pos0, adjective_position.first)
                    && canReachToIndexFromConceptWithoutTag(phrases, dependencies, pos1, adjective_position.first)) {
                // if adjective has negation, simply a result does not depend on the tag
                if (adjective_position.second) {
                    // e.g. c0 より c1 は neg_adj / c0 ほど c1 は neg_adj
                    m_comparatives[pred][name0][name1]++;
                    if (m_writer && pred == Predicate::NORMAL) m_writer->writeComparativeReason(name0, name1, parser.raw());
                }
                else {
                    // if adjective has no negation, check the tag
                    if (tag0 != Counter::PATTERN_NEG_ADJECTIVE_TAG) {
                        m_comparatives[pred][name1][name0]++; // e.g. c0 より c1 は adj
                        if (m_writer && pred == Predicate::NORMAL) m_writer->writeComparativeReason(name1, name0, parser.raw());
                    }
                }
            }
            // on antonym
            if (canReachToIndex(dependencies, pos0, antonym_position.first)
                    && canReachToIndexFromConceptWithoutTag(phrases, dependencies, pos1, antonym_position.first)) {
                // if antonym has negation, simply the result does not depend on the tag
                if (antonym_position.second) {
                    // e.g. c0 より c1 は neg_ant / c0 ほど c1 は neg_ant
                    m_comparatives[pred][name1][name0]++;
                    if (m_writer && pred == Predicate::NORMAL) m_writer->writeComparativeReason(name1, name0, parser.raw());
                }
                else {
                    // if antonym has no negation, check the tag
                    if (tag0 != Counter::PATTERN_NEG_ADJECTIVE_TAG) {
                        m_comparatives[pred][name0][name1]++; // e.g. c0 より c1 は ant
                        if (m_writer && pred == Predicate::NORMAL) m_writer->writeComparativeReason(name0, name1, parser.raw());
                    }
                }
            }
        }
    }
}

void Counter::countStats(
    const Parser& parser,
    const WordSet& target_concepts)
{
    const std::vector<std::shared_ptr<Morph>>& morphs = parser.morphs();
    for (const auto& morph : morphs) {
        if (morph->POS() == Morph::POSTag::ADJECTIVE) {
            m_all_adjective_occurrences[morph->lemma()]++;
            for (const auto& concept : target_concepts)
                m_all_adjective_cooccurrences[concept][morph->lemma()]++;
        }
    }
}



// * output to a file {{{
void Counter::save(const std::string& output_filename) const
{
    std::ofstream output_file;
    output_file.open(output_filename);

    // write statistics/hints to a file
    // a hint look like:
    //      adjective,concept,Hint(see. counter_tags.h)[,arg0][,arg1],...,count
    if (isPrepMode()) {
        writePrepValue(output_file);
    }
    else {
        writeStats(output_file);
        writeCooccurrenceCounts(output_file);
        writeDependencyCounts(output_file);
        writeSimileCounts(output_file);
        writeComparativeCounts(output_file);
    }
    output_file.flush();
    output_file.close();
}

#define WRITE_STAT_TO_FILE(target, target_val, pred, polar, stat, stat_val) \
    output_file \
        << CounterTags::targetString(CounterTags::Target::target) << "," \
        << target_val << "," \
        << CounterTags::statString(pred, polar, CounterTags::Stat::stat) << "," \
        << stat_val << std::endl << std::flush;

void Counter::writePrepValue(std::ofstream& output_file) const
{
    for (const auto& concept : m_concepts) {
        // concept occurrence
        WRITE_STAT_TO_FILE(CONCEPT, concept,
            Predicate::NORMAL, Polar::P, OCCURRENCE,
            m_occurrences.at(concept));

        // concept-adjective co-occurrences
        for (const auto& pair : m_all_adjective_cooccurrences.at(concept)) {
            // pair = (adjective, count)
            WRITE_STAT_TO_FILE(CONCEPT, concept << "," << pair.first,
                Predicate::NORMAL, Polar::P, OCCURRENCE,
                pair.second);
        }
    }
    for (const auto& pair : m_all_adjective_occurrences) {
        // pair = (adjective, count)
        WRITE_STAT_TO_FILE(ADJECTIVE, pair.first,
                Predicate::NORMAL, Polar::P, OCCURRENCE,
                pair.second);
    }
}

void Counter::writeStats(std::ofstream& output_file) const
{
    // the total occurrence counts of concepts
    for (const auto& concept : m_concepts) {
        WRITE_STAT_TO_FILE(CONCEPT, concept,
            Predicate::NORMAL, Polar::P, OCCURRENCE,
            m_occurrences.at(concept));
    }

    for (const auto& pred : m_target_preds) {
        for (const auto& polar : {Polar::P, Polar::N}) {
            // the total occurrence counts of adjectives
            WRITE_STAT_TO_FILE(ADJECTIVE, m_adjectives[Predicate::NORMAL][0],
                pred, polar, OCCURRENCE,
                m_total_adjective_occurrences[pred][polar]);

            // the total dependency counts of adjectives
            WRITE_STAT_TO_FILE(ADJECTIVE, m_adjectives[Predicate::NORMAL][0],
                pred, polar, DEPENDENCY,
                m_total_adjective_dependencies[pred][polar]);

            // the total simile counts of adjectives
            WRITE_STAT_TO_FILE(ADJECTIVE, m_adjectives[Predicate::NORMAL][0],
                pred, polar, SIMILE,
                m_total_adjective_similia[pred][polar]);
        }
    }
}

#define WRITE_HINT_TO_FILE(adjective, concept, pred, polar, hint, hint_val) \
    output_file \
        << adjective << "," \
        << concept << "," \
        << CounterTags::hintString(pred, polar, CounterTags::Hint::hint) << "," \
        << hint_val << std::endl << std::flush;

#define WRITE_HINTS_TO_FILE(adjective, counts, pred, polar, hint) \
    for (const auto& pair : counts) { \
        WRITE_HINT_TO_FILE(adjective, pair.first, pred, polar, hint, pair.second); \
    }


void Counter::writeCooccurrenceCounts(std::ofstream& output_file) const
{
    for (const auto& pred : m_target_preds) {
        for (const auto& polar : {Polar::P, Polar::N}) {
            WRITE_HINTS_TO_FILE(m_adjectives[Predicate::NORMAL][0],
                m_cooccurrences[pred][polar],
                pred, polar, CO_OCCURRENCE);
        }
    }
}

void Counter::writeDependencyCounts(std::ofstream& output_file) const
{
    for (const auto& pred : m_target_preds) {
        for (const auto& polar : {Polar::P, Polar::N}) {
            WRITE_HINTS_TO_FILE(m_adjectives[Predicate::NORMAL][0],
                m_dependencies[pred][polar],
                pred, polar, DEPENDENCY);
        }
    }
}

void Counter::writeSimileCounts(std::ofstream& output_file) const
{
    for (const auto& pred : m_target_preds) {
        for (const auto& polar : {Polar::P, Polar::N}) {
            WRITE_HINTS_TO_FILE(m_adjectives[Predicate::NORMAL][0],
                m_similia[pred][polar],
                pred, polar, SIMILE);
        }
    }
}

void Counter::writeComparativeCounts(std::ofstream& output_file) const
{
    for (const auto& pred : m_target_preds) {
        // count positive/negative count
        for (const auto& dict : m_comparatives[pred]) {
            // dict = (concept0, sub_dic)
            for (const auto& pair : dict.second) {
                /// pair = (concept1, count)
                // TODO: check again

                // output positive comparative
                WRITE_HINT_TO_FILE(m_adjectives[Predicate::NORMAL][0], dict.first,
                    pred, Polar::P, COMPARATIVE,
                    pair.first << "," << pair.second);

                // output negative comparative
                WRITE_HINT_TO_FILE(m_adjectives[Predicate::NORMAL][0], pair.first,
                    pred, Polar::N, COMPARATIVE,
                    dict.first << "," << pair.second);
            }
        }
    }
}
// * /output to a file }}}

} // namespace order_concepts
