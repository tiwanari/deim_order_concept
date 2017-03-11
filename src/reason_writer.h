#pragma once
#include <string>
#include "counter_tags.h"

namespace order_concepts {
class ReasonWriter {
using Predicate = CounterTags::Predicate;
using Polar = CounterTags::Polar;

public:
    /** output path for reason files **/
    static constexpr const char* REASON_PATH = "reasons/";
    /** reason files **/
    static constexpr const char* CONCEPT_IDS_FILE = "concepts.txt";
    static constexpr const char* CO_OCCURRENCE_REASON_FILE = "co_occurrences.txt";
    static constexpr const char* DEPENDENCY_REASON_FILE = "dependencies.txt";
    static constexpr const char* SIMILE_REASON_FILE = "similia.txt";
    static constexpr const char* COMPARATIVE_REASON_FILE = "comparatives.txt";

private:
    // this is used like; m_reason_root_path / REASON_PATH / CO_OCCURRENCE_REASON_FILE
    std::string m_reason_root_path = "";

    /** ostream for reason files **/
    std::ofstream m_co_occurrences_output;
    std::ofstream m_dependencies_output;
    std::ofstream m_similia_output;
    std::ofstream m_comparatives_output;

private:
    std::unordered_map<std::string, int> m_concept_to_id;

private:
    std::string reasonFileAbsolutePath(const std::string& filename) const;
    std::string polarMark(const Polar& p) const;
    std::string reasonSenstence(const std::string& concept,
                                const std::string& reason,
                                const Polar& p) const;

public:
    ReasonWriter(
        const std::vector<std::string>& concepts,
        const std::string& reason_root_path);
    ~ReasonWriter();

    void writeCooccurrenceReason(const std::string& concept, const std::string& reason, const Polar& p);
    void writeDependencyReason(const std::string& concept, const std::string& reason, const Polar& p);
    void writeSimileReason(const std::string& concept, const std::string& reason, const Polar& p);
    void writeComparativeReason(const std::string& concept0, const std::string& concept1, const std::string& reason);
};
} // namespace order_concepts
