#include <cstdlib>
#include <string>
#include <unistd.h>
#include "morph.h"
#include "counter.h"
#include "reducer.h"
#include "svm_data_formatter.h"
#include "file_format.h"
#include "util/trim.h"
#include "util/dir.h"
#include "util/cmdline.h" // https://github.com/tanakh/cmdline

using namespace order_concepts;

Morph::MorphType morphType(const std::string& morph)
{
    if (morph == "IPA")     return Morph::MorphType::IPADIC;
    if (morph == "JUMAN")   return Morph::MorphType::JUMAN;
    return Morph::MorphType::IPADIC;
}

void readLinesFromAFile(
    const std::string& input_filename,
    std::vector<std::string>* lines)
{
    std::ifstream input_file;
    input_file.open(input_filename);
    if (!input_file) {
        std::cerr << "Can't open file: " << input_filename << std::endl;
        input_file.close();
        exit(1);
    }
    std::string line;
    while (std::getline(input_file, line)) {
        if (line.empty()) continue;
        lines->emplace_back(util::trim(line));
    }
    input_file.close();
}

const std::string ARG_OUT   = "output_dir";

const std::string ARG_ADJ   = "adjective";
const std::string ARG_ANT   = "antonym";
const std::string ARG_CON   = "concept_file";

const std::string ARG_PTN   = "pattern_file_path";
const std::string ARG_MOR   = "morph";

const std::string ARG_SYN_ADJ   = "syns_adj";
const std::string ARG_SYN_ANT   = "syns_ant";

void addDefaultArguments(cmdline::parser& p)
{
    p.add<std::string>(ARG_OUT, 'o', "output_dir",     true);

    p.add<std::string>(ARG_ADJ, 'a', "adjective", true);
    p.add<std::string>(ARG_ANT, 'n', "antonym", true);
    p.add<std::string>(ARG_CON, 'c', "concept_file", true);

    p.add<std::string>(ARG_PTN, 'p', "pattern_file_path", false,
                        "../dataset/ja/count_patterns/ipa");
    p.add<std::string>(ARG_MOR, 't', "morph type [IPA | JUMAN]", false, "IPA",
                        cmdline::oneof<std::string>("IPA", "JUMAN"));

    p.add<std::string>(ARG_SYN_ADJ, 'A', "synonyms of adjective", false, "");
    p.add<std::string>(ARG_SYN_ANT, 'N', "synonyms of antonym", false, "");
}


int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "ja_JP.UTF-8");

    cmdline::parser p;
    addDefaultArguments(p);
    p.parse_check(argc, argv);



    // === read arguments ===
    const std::string adjective = p.get<std::string>(ARG_ADJ);    // adjective
    const std::string antonym   = p.get<std::string>(ARG_ANT);    // antonym

    std::vector<std::string> concepts;
    readLinesFromAFile(p.get<std::string>(ARG_CON), &concepts); // read concepts from a file

    const std::string output_dir    = p.get<std::string>(ARG_OUT);

    // -- optionals -- {{{
    const std::string pattern_file_path = p.get<std::string>(ARG_PTN); // pattern file path
    const std::string morph             = p.get<std::string>(ARG_MOR); // morph type

    std::vector<std::string> adj_synonyms;
    if (p.exist(ARG_SYN_ADJ)) readLinesFromAFile(p.get<std::string>(ARG_SYN_ADJ), &adj_synonyms);

    std::vector<std::string> ant_synonyms;
    if (p.exist(ARG_SYN_ANT)) readLinesFromAFile(p.get<std::string>(ARG_SYN_ANT), &ant_synonyms);

    // tell svm_data_formatter if this program uses synonyms or not
    const bool does_use_synonyms = p.exist(ARG_SYN_ADJ) || p.exist(ARG_SYN_ANT);
    // -- /optionals -- }}}
    // === /read arguments ===



    // temporary output folder
    util::mkdir(output_dir);



    // === Counter === {{{

    // --- setup counter --- {{{
    // prepare a parser
    Parser parser(morphType(morph), "dummy", FileFormat::STDIN);


    Counter counter(concepts, adjective, antonym);
    counter.readPatternFilesInPath(pattern_file_path); // pattern files

    // setup a reason writer
    // const std::string root_reason_path = output_dir + "/reason/";
    // counter.setupReasonWriter(root_reason_path);

    // use synonyms
    if (does_use_synonyms) counter.setSynonyms(adj_synonyms, ant_synonyms);
    // --- /setup counter --- }}}


    counter.count(parser); // count


    // count result file
    const std::string counted_result_file_path = output_dir + "/counted";
    counter.save(counted_result_file_path);  // save to a file
    std::fflush(stdout);
    // === /Counter === }}}


    // create the folder name for formatted result
    const std::string formatted_result_file_path = output_dir + "/format";

    // formatting reduced hints for ranking-svm and output them as files
    SvmDataFormatter formatter(counted_result_file_path, does_use_synonyms);
    formatter.format(formatted_result_file_path);
    std::fflush(stdout);

    return 0;
}
