#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <algorithm>
#include "../counter.h"
#include "../file_format.h"

using namespace order_concepts;

class ReasonWriterTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};



// NOTE: they are some of integration tests

TEST_F(ReasonWriterTest, writeConceptIdsToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "";
    const std::string kAntonym = "";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kOutputFilePath = "./unittest/out/";
    counter.setupReasonWriter(kOutputFilePath);

    const std::string kSmallDataset =
        "./unittest/dataset/reason_writer_test_data0.JDP";
    Parser parser(Morph::MorphType::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::vector<std::string> kAnswers =
        {
            "0,犬",
            "1,猫",
            "2,象",
            "3,ねずみ",
        };

    std::string file_path =
        kOutputFilePath + util::concatStr(ReasonWriter::REASON_PATH, ReasonWriter::CONCEPT_IDS_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(ReasonWriterTest, writeCooccurrenceSentencesToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kOutputFilePath = "./unittest/out/";
    counter.setupReasonWriter(kOutputFilePath);

    const std::string kSmallDataset =
        "./unittest/dataset/reason_writer_test_data0.JDP";
    Parser parser(Morph::MorphType::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::vector<std::string> kAnswers =
        {
            "0\t + すくすくと育ち大きい犬になりました",
            "0\t + 犬は大きい。",
            "0\t + くじらは犬よりも大きい。",
            "3\t - ねずみは犬よりも大きくない。",
            "0\t - ねずみは犬よりも大きくない。",
            "3\t - ねずみは犬よりも大きくない。",
            "0\t - ねずみは犬よりも大きくない。",
            "0\t - 象は犬よりも大きくない。", // why this order is
            "2\t - 象は犬よりも大きくない。", // 0 -> 2 ?
            "0\t - 犬のように小さい。",
            "0\t - 犬よりも象は大きくない。",
            "2\t - 犬よりも象は大きくない。",
            "0\t + 犬よりも象は大きい。",
            "2\t + 犬よりも象は大きい。",
            "0\t + 象は犬よりも大きい。",
            "2\t + 象は犬よりも大きい。",
        };

    std::string file_path =
        kOutputFilePath + util::concatStr(ReasonWriter::REASON_PATH, ReasonWriter::CO_OCCURRENCE_REASON_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(ReasonWriterTest, writeDependencySentencesToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kOutputFilePath = "./unittest/out/";
    counter.setupReasonWriter(kOutputFilePath);

    const std::string kSmallDataset =
        "./unittest/dataset/reason_writer_test_data0.JDP";
    Parser parser(Morph::MorphType::JUMAN, kSmallDataset);
    counter.count(parser);

    // NOTE: this may be an unexpected output
    // but the current version of dependency check is somewhat loose
    // and it finds a dependency from "犬よりも" to "大きい."
    const std::vector<std::string> kAnswers =
        {
            "0\t + 犬は大きい。",
            "0\t + くじらは犬よりも大きい。", // this one!
            "3\t - ねずみは犬よりも大きくない。",
            "0\t - ねずみは犬よりも大きくない。", // this one!
            "3\t - ねずみは犬よりも大きくない。",
            "0\t - ねずみは犬よりも大きくない。", // this one!
            "2\t - 象は犬よりも大きくない。",
            "0\t - 象は犬よりも大きくない。", // this one!
            "0\t - 犬のように小さい。",
            "0\t - 犬よりも象は大きくない。",
            "2\t - 犬よりも象は大きくない。",
            "0\t + 犬よりも象は大きい。",
            "2\t + 犬よりも象は大きい。", // this one!
            "2\t + 象は犬よりも大きい。",
            "0\t + 象は犬よりも大きい。", // this one!
        };

    std::string file_path =
        kOutputFilePath + util::concatStr(ReasonWriter::REASON_PATH, ReasonWriter::DEPENDENCY_REASON_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(ReasonWriterTest, writeSimileSentencesToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kOutputFilePath = "./unittest/out/";
    counter.setupReasonWriter(kOutputFilePath);

    const std::string kPatternFilesPath =
        "./unittest/dataset/ja/count_patterns/juman";
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::string kSmallDataset =
        "./unittest/dataset/reason_writer_test_data0.JDP";
    Parser parser(Morph::MorphType::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::vector<std::string> kAnswers =
        {"0\t - 犬のように小さい。"};

    std::string file_path =
        kOutputFilePath + util::concatStr(ReasonWriter::REASON_PATH, ReasonWriter::SIMILE_REASON_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(ReasonWriterTest, writeComparativeSentencesToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kOutputFilePath = "./unittest/out/";
    counter.setupReasonWriter(kOutputFilePath);

    const std::string kPatternFilesPath =
        "./unittest/dataset/ja/count_patterns/juman";
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::string kSmallDataset =
        "./unittest/dataset/reason_writer_test_data0.JDP";
    Parser parser(Morph::MorphType::JUMAN, kSmallDataset);
    counter.count(parser);

    // TODO: check A よりも B は adj/ant
    const std::vector<std::string> kAnswers =
        {
            "0\t + ねずみは犬よりも大きくない。",
            "3\t - ねずみは犬よりも大きくない。",
            "0\t + ねずみは犬よりも大きくない。",
            "3\t - ねずみは犬よりも大きくない。",
            "0\t + 象は犬よりも大きくない。",
            "2\t - 象は犬よりも大きくない。",
            "0\t + 犬よりも象は大きくない。",
            "2\t - 犬よりも象は大きくない。",
            "2\t + 犬よりも象は大きい。", // why this order is
            "0\t - 犬よりも象は大きい。", // 2 -> 0 ?
            "2\t + 象は犬よりも大きい。",
            "0\t - 象は犬よりも大きい。",
         };

    std::string file_path =
        kOutputFilePath + util::concatStr(ReasonWriter::REASON_PATH, ReasonWriter::COMPARATIVE_REASON_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}
