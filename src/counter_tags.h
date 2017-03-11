#pragma once
#include <iostream>
#include "util/cast.h"

namespace order_concepts {
class CounterTags {
public:
    enum Predicate : unsigned int {
        NORMAL = 0,
        SYNONYM = 1,
    };
    enum Polar : unsigned int {
        P = 0,
        N = 1,
    };

public:
    /** tag **/
    static constexpr const char* TARGET_TAG_PREFIX = "TARGET_";
    static constexpr const char* HINT_TAG_PREFIX = "HINT_";
    static constexpr const char* STAT_TAG_PREFIX = "STAT_";
    /** sub tag **/
    static constexpr const char* SYNONYMS_TAG_PREFIX = "SYNONYMS_";
    static constexpr const char* NEGATIVE_TAG_PREFIX = "NEG_";

    enum class Target : unsigned int {
        ADJECTIVE,
        CONCEPT,
        FIRST = ADJECTIVE,
        LAST = CONCEPT
    };
    enum class Stat : unsigned int {
        OCCURRENCE,
        DEPENDENCY,
        SIMILE,
        FIRST = OCCURRENCE,
        LAST = SIMILE
    };
    enum class Hint : unsigned int {
        CO_OCCURRENCE,
        DEPENDENCY,
        SIMILE,
        COMPARATIVE,
        FIRST = CO_OCCURRENCE,
        LAST = COMPARATIVE
    };


    static bool isTag(const std::string& str) {
        return isTargetVal(str) || isStatVal(str) || isHintVal(str);
    }
    static bool isTargetVal(const std::string& str) {
        for (int targetVal = util::as_integer(Target::FIRST);
                targetVal <= util::as_integer(Target::LAST);
                ++targetVal)
        {
            Target val = static_cast<Target>(targetVal);
            if (str == targetString(val)) return true;
        }
        return false;
    }
    static bool isStatVal(const std::string& str) {
        for (int statVal = util::as_integer(Stat::FIRST);
                statVal <= util::as_integer(Stat::LAST);
                ++statVal)
        {
            Stat val = static_cast<Stat>(statVal);
            for (const auto& pred : {Predicate::NORMAL, Predicate::SYNONYM})
                for (const auto& polar : {Polar::P, Polar::N})
                    if (str == statString(pred, polar, val)) return true;
        }
        return false;
    }
    static bool isHintVal(const std::string& str) {
        for (int hintVal = util::as_integer(Hint::FIRST);
                hintVal <= util::as_integer(Hint::LAST);
                ++hintVal)
        {
            Hint val = static_cast<Hint>(hintVal);
            for (const auto& pred : {Predicate::NORMAL, Predicate::SYNONYM})
                for (const auto& polar : {Polar::P, Polar::N})
                    if (str == hintString(pred, polar, val)) return true;
        }
        return false;
    }
    static std::string targetString(const Target& target) {
        switch (target) {
            case Target::ADJECTIVE:
                return "ADJECTIVE";
            case Target::CONCEPT:
                return "CONCEPT";
        }
        return "";
    }
    static std::string statString(const Predicate& pred, const Polar& polar, const Stat& base) {
        std::stringstream ss;
        ss << STAT_TAG_PREFIX;
        if (pred == Predicate::SYNONYM) ss << CounterTags::SYNONYMS_TAG_PREFIX;
        if (polar == Polar::N)          ss << CounterTags::NEGATIVE_TAG_PREFIX;
        ss << CounterTags::statBaseString(base);
        return ss.str();
    }
    static std::string statBaseString(const Stat& stat) {
        switch (stat) {
            // occurrence (not co-occurrence)
            case Stat::OCCURRENCE:
                return "OCCURRENCE";
            // dependency from something (anything is okay for statistics)
            case Stat::DEPENDENCY:
                return "DEPENDENCY";
            // simile with something (anything is okay for statistics)
            case Stat::SIMILE:
                return "SIMILE";
            default:
                std::cerr << "Not a base stat string" << std::endl;
                break;
        }
        return "";
    }
    static std::string hintString(const Predicate& pred, const Polar& polar, const Hint& base) {
        std::stringstream ss;
        ss << HINT_TAG_PREFIX;
        if (pred == Predicate::SYNONYM) ss << CounterTags::SYNONYMS_TAG_PREFIX;
        if (polar == Polar::N)          ss << CounterTags::NEGATIVE_TAG_PREFIX;
        ss << CounterTags::hintBaseString(base);
        return ss.str();
    }
    static std::string hintBaseString(const Hint& hint) {
        switch (hint) {
            case Hint::CO_OCCURRENCE:
                return "CO_OCCURRENCE";
            case Hint::DEPENDENCY:
                return "DEPENDENCY";
            case Hint::SIMILE:
                return "SIMILE";
            case Hint::COMPARATIVE:
                return "COMPARATIVE";
            default:
                std::cerr << "Not a base hint string" << std::endl;
                break;
        }
        return "";
    }
};
} // namespace order_concepts
