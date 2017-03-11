#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "counter_tags.h"

namespace order_concepts {
class SvmDataFormatter {
using Predicate = CounterTags::Predicate;
using Polar = CounterTags::Polar;

using Target = CounterTags::Target;
using Stat = CounterTags::Stat;
using Hint = CounterTags::Hint;

private:
    static constexpr const char* OUTPUT_FILE = "output.txt";
    static constexpr const char* ADJECTIVE_IDS_FILE = "adjective_ids.txt";
    static constexpr const char* CONCEPT_IDS_FILE = "concept_ids.txt";
    static constexpr const char* HINT_IDS_FILE = "hint_ids.txt";

private:
    std::string m_input_filename;
    std::string m_output_directory_path;

    bool m_use_synonyms;

    // adjective, concept, hint, (args,...), count
    std::unordered_map<std::string,
        std::unordered_map<std::string,
            std::unordered_map<std::string,
                std::unordered_map<std::string, std::string>>>> m_raw_data;
    std::vector<std::string> m_adjective_ids;
    std::vector<std::string> m_concept_ids;
    std::vector<std::string> m_hint_ids;

private:
    void createHintTableToBeShown(const bool use_synonyms); // create hint table

    bool addDataIntoIdTable(
        const std::string& data,
        std::vector<std::string>* id_table);

    void outputIdTable(
        const std::string& output_filename,
        const std::vector<std::string>& id_table) const;
    void outputSvmFormatFile(const std::string& output_filename) const;

    bool encodeValue(
        const std::string& adjective,
        const std::string& concept,
        const std::string& hint,
        double* result) const;
    double encodeCooccurrenceValue(
        const std::string& adjective,
        const std::string& concept,
        const Predicate& pred) const;
    double encodeDependencyValue(
        const std::string& adjective,
        const std::string& concept,
        const Predicate& pred) const;
    double encodeSimileValue(
        const std::string& adjective,
        const std::string& concept,
        const Predicate& pred) const;
    double encodeComparativeValue(
        const std::string& adjective,
        const std::string& concept,
        const std::string& hint) const;

public:
    SvmDataFormatter(const std::string& input_filename)
        : m_input_filename(input_filename), m_use_synonyms(false) {}
    SvmDataFormatter(const std::string& input_filename, const bool use_synonyms)
        : m_input_filename(input_filename), m_use_synonyms(use_synonyms) {}

    void enableSynonyms(const bool enable) { m_use_synonyms = enable; }

    void format(const std::string& output_directory_path);

    std::string outputFilePath() const
        { return m_output_directory_path + "/" + OUTPUT_FILE; }
    std::string adjectiveMappingFilePath() const
        { return m_output_directory_path + "/" + ADJECTIVE_IDS_FILE; }
    std::string conceptMappingFilePath() const
        { return m_output_directory_path + "/" + CONCEPT_IDS_FILE; }
    std::string hintMappingFilePath() const
        { return m_output_directory_path + "/" + HINT_IDS_FILE; }
};
} // namespace order_concepts
