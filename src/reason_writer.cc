#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "reason_writer.h"
#include "util/dir.h"

namespace order_concepts {

std::string ReasonWriter::reasonFileAbsolutePath(const std::string& filename) const
{
    std::stringstream ss;
    ss << m_reason_root_path << "/" << REASON_PATH << "/" << filename;
    return ss.str();
}

ReasonWriter::ReasonWriter(
        const std::vector<std::string>& concepts,
        const std::string& reason_root_path)
: m_reason_root_path(reason_root_path)
{
    // make the directory for output
    util::mkdir(reasonFileAbsolutePath(""));
    m_co_occurrences_output.open(reasonFileAbsolutePath(CO_OCCURRENCE_REASON_FILE), std::ios_base::out);
    m_dependencies_output.open(reasonFileAbsolutePath(DEPENDENCY_REASON_FILE), std::ios_base::out);
    m_similia_output.open(reasonFileAbsolutePath(SIMILE_REASON_FILE), std::ios_base::out);
    m_comparatives_output.open(reasonFileAbsolutePath(COMPARATIVE_REASON_FILE), std::ios_base::out);

    // write the map from a concept to an ID
    // IDs will be used to write line prefixes in reasons files to show which sentence contains each concept
    std::ofstream concepts_file(reasonFileAbsolutePath(CONCEPT_IDS_FILE));
    for (int i = 0; i < concepts.size(); i++) {
        concepts_file << i << "," << concepts[i] << std::endl;
        m_concept_to_id[concepts[i]] = i;
    }
    concepts_file.close();
}

ReasonWriter::~ReasonWriter()
{
    m_co_occurrences_output.close();
    m_dependencies_output.close();
    m_similia_output.close();
    m_comparatives_output.close();
}

std::string ReasonWriter::polarMark(const Polar& p) const
{
    return p == Polar::P ? " + " : " - ";
}

std::string ReasonWriter::reasonSenstence(
    const std::string& concept,
    const std::string& reason,
    const Polar& p) const
{
    std::stringstream ss;
    ss << m_concept_to_id.at(concept) << "\t" << polarMark(p) << reason;
    return ss.str();
}

void ReasonWriter::writeCooccurrenceReason(
    const std::string& concept,
    const std::string& reason,
    const Polar& p)
{
    m_co_occurrences_output << reasonSenstence(concept, reason, p) << std::endl;
}

void ReasonWriter::writeDependencyReason(
    const std::string& concept,
    const std::string& reason,
    const Polar& p)
{
    m_dependencies_output << reasonSenstence(concept, reason, p) << std::endl;
}

void ReasonWriter::writeSimileReason(
    const std::string& concept,
    const std::string& reason,
    const Polar& p)
{
    m_similia_output << reasonSenstence(concept, reason, p) << std::endl;
}

void ReasonWriter::writeComparativeReason(
    const std::string& concept0,
    const std::string& concept1,
    const std::string& reason)
{
    m_comparatives_output << reasonSenstence(concept0, reason, Polar::P) << std::endl;
    m_comparatives_output << reasonSenstence(concept1, reason, Polar::N) << std::endl;
}

} // namespace order_concepts
