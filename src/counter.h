#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include "file_format.h"
#include "parser.h"
#include "morph.h"
#include "reason_writer.h"
#include "counter_tags.h"
#include "util/cast.h"
#include "util/concat.h"

namespace order_concepts {
class Counter {
    using Polar = CounterTags::Polar;
    using Predicate = CounterTags::Predicate;

    using Count = long long;
    using WordCount = std::unordered_map<std::string, Count>;
    using WordPairCount = std::unordered_map<std::string, WordCount>;
    using Patterns = std::vector<std::vector<std::string>>;

    using WordSet = std::set<std::string>;

private:
    /** pattern files **/
    static constexpr const char* SIMILE_PATTERN_FILE = "simile.txt";
    static constexpr const char* COMPARATIVE_PATTERN_FILE = "comparative.txt";
    /** pattern file tags **/
    static constexpr const char* PATTERN_CONCEPT_0_TAG = "!CONCEPT0";
    static constexpr const char* PATTERN_CONCEPT_1_TAG = "!CONCEPT1";
    static constexpr const char* PATTERN_ADJECTIVE_TAG = "!ADJECTIVE";
    static constexpr const char* PATTERN_NEG_ADJECTIVE_TAG = "!NEG_ADJECTIVE";

    std::string m_pattern_file_path;
    Patterns m_simile_patterns;
    Patterns m_comparative_patterns;

private:
    std::set<Predicate> m_target_preds;

private:
    std::vector<std::string> m_concepts;
    WordCount m_occurrences; // occurrence(x)

private:
    // these members also have synonyms (Predicate: NORMAL or SYNONYM)
    // NOTE: reduce all counts of words (e.g., adjs) into one
    // tag: [Predicate]
    std::vector<std::string> m_adjectives[2];
    std::vector<std::string> m_antonyms[2];

    // tag: [Predicate][Polar]
    Count m_total_adjective_occurrences[2][2]; // count(y_pos | y_neg)
    Count m_total_adjective_dependencies[2][2]; // dependency(*, y_pos | y_neg)
    Count m_total_adjective_similia[2][2]; // simile("", y_pos | y_neg)

    // tag: [Predicate][Polar]
    WordCount m_cooccurrences[2][2]; // co-occurrence(x, y_pos | y_neg)
    WordCount m_dependencies[2][2]; // dependency(x, y_pos | y_neg)
    WordCount m_similia[2][2]; // simile(x, y_pos | y_neg)

    // m_comparatives[concept0][concept1] shows the number of "concept0 > concept1" occurrences
    // tag: [Predicate]
    WordPairCount m_comparatives[2];

private:
    // for making dataset
    bool m_is_prep_mode;
    // // co-occurred adjective
    // WordCount m_cooccurred_adjective;
    // occurrence(any_y)
    WordCount m_all_adjective_occurrences;
    // co-occurrence(x, any_y)
    WordPairCount m_all_adjective_cooccurrences;

private:
    // a writer for reasons
    ReasonWriter* m_writer = nullptr;


private:
    void resetCounters();
    void resetPatterns();

    bool readPatternFile(const std::string& pattern_filename, Patterns* patterns);
    bool readSimilePatterns();
    bool readComarativePatterns();

    bool findConcept(
        const WordSet& target_concepts,
        const std::vector<std::shared_ptr<Morph>>& morphs,
        int* cur_index,
        std::string* found_concept) const;

    void countConcepts(const Parser& parser, WordSet* found_concepts);

    void countCooccurrence(const Parser& parser, const WordSet& found_concepts, const Predicate& pred);
    void countDependency(const Parser& parser, const WordSet& target_concepts, const Predicate& pred);
    void countSimile(const Parser& parser, const WordSet& target_concepts, const Predicate& pred);
    void countComparative(const Parser& parser, const WordSet& target_concepts, const Predicate& pred);

    void countStats(const Parser& parser, const WordSet& target_concepts);

    void writePrepValue(std::ofstream& output_file) const;
    void writeStats(std::ofstream& output_file) const;
    void writeCooccurrenceCounts(std::ofstream& output_file) const;
    void writeDependencyCounts(std::ofstream& output_file) const;
    void writeSimileCounts(std::ofstream& output_file) const;
    void writeComparativeCounts(std::ofstream& output_file) const;

public:
    Counter(
        const std::vector<std::string>& concepts,
        const std::string& adjective,
        const std::string& antonym);
    ~Counter();

    bool isPrepMode() const { return m_is_prep_mode; }
    void setPrepMode(bool enable) { m_is_prep_mode = enable; }
    bool readPatternFilesInPath(const std::string& path);

    void setSynonyms(std::vector<std::string> adj_synonyms, std::vector<std::string> ant_synonyms) {
        m_target_preds.emplace(Predicate::SYNONYM); // also count synonyms
        m_adjectives[Predicate::SYNONYM] = adj_synonyms;
        m_antonyms[Predicate::SYNONYM]   = ant_synonyms;
    }

    void count(Parser& parser);
    void save(const std::string& output_filename) const;

    const std::vector<std::string>& concepts() const { return m_concepts; }
    const std::string& adjective() const { return m_adjectives[Predicate::NORMAL][0]; }
    const std::string& antonym() const { return m_antonyms[Predicate::NORMAL][0]; }

    const Patterns& similePatterns() const { return m_simile_patterns; }
    const Patterns& comparativePatterns() const { return m_comparative_patterns; }

    const std::vector<std::string>& adjectiveSynonyms() const { return m_adjectives[Predicate::SYNONYM]; }
    const std::vector<std::string>& antonymSynonyms() const { return m_antonyms[Predicate::SYNONYM]; }

    void setupReasonWriter(const std::string& reason_root_path)
        { m_writer = new ReasonWriter(m_concepts, reason_root_path); }

// * getters {{{
    Count adjectiveOccurrence(const Predicate& pred, const Polar& polar) const {
        return m_total_adjective_occurrences[pred][polar];
    }
    Count adjectiveDependency(const Predicate& pred, const Polar& polar) const {
        return m_total_adjective_dependencies[pred][polar];
    }
    Count adjectiveSimile(const Predicate& pred, const Polar& polar) const {
        return m_total_adjective_similia[pred][polar];
    }

    #define COUNT_AT(target, concept) \
        auto it = target.find(concept); \
        if (it == target.end()) return 0; \
        return target.at(concept);
    Count occurrence(const std::string& concept) const {
        COUNT_AT(m_occurrences, concept);
    }
    Count cooccurrence(const std::string& concept) const {
        return cooccurrence(concept, Polar::P, Predicate::NORMAL);
    }
    Count cooccurrence(const std::string& concept, const Polar& polar) const {
        return cooccurrence(concept, polar, Predicate::NORMAL);
    }
    Count cooccurrence(const std::string& concept, const Polar& polar, const Predicate& pred) const {
        COUNT_AT(m_cooccurrences[pred][polar], concept);
    }
    Count dependency(const std::string& concept) const {
        return dependency(concept, Polar::P, Predicate::NORMAL);
    }
    Count dependency(const std::string& concept, const Polar& polar) const {
        return dependency(concept, polar, Predicate::NORMAL);
    }
    Count dependency(const std::string& concept, const Polar& polar, const Predicate& pred) const {
        COUNT_AT(m_dependencies[pred][polar], concept);
    }
    Count simile(const std::string& concept) const {
        return simile(concept, Polar::P, Predicate::NORMAL);
    }
    Count simile(const std::string& concept, const Polar& polar) const {
        return simile(concept, polar, Predicate::NORMAL);
    }
    Count simile(const std::string& concept, const Polar& polar, const Predicate& pred) const {
        COUNT_AT(m_similia[pred][polar], concept);
    }
    Count comparative(const std::string& concept0, const std::string& concept1) const {
        return comparative(concept0, concept1, Predicate::NORMAL);
    }
    Count comparative(const std::string& concept0, const std::string& concept1, const Predicate& pred) const {
        // concept0 > concept1
        {
            auto it = m_comparatives[pred].find(concept0);
            if (it == m_comparatives[pred].end()) return 0;
        }
        auto& submap = m_comparatives[pred].at(concept0);
        COUNT_AT(submap, concept1);
    }
// * /getters }}}

};
} // namespace order_concepts
