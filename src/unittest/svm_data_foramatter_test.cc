#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <cctype>
#include <algorithm>
#include "../svm_data_formatter.h"
#include "../util/split.h"
#include "../util/type.h"

using namespace order_concepts;

using HintValues = std::vector<double>;
using ValueTable = std::vector<HintValues>;

class SvmDataFormatterTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
    void checkOutputFileIsValid(
        const std::string& filename,
        const int num_of_hints,
        ValueTable* table);
};


void SvmDataFormatterTest::checkOutputFileIsValid(
    const std::string& target_filename,
    const int num_of_hints,
    ValueTable* table)
{
    std::ifstream target_file;
    target_file.open(target_filename);
    ASSERT_TRUE(target_file);

    std::string line;
    while (std::getline(target_file, line)) {
        std::vector<std::string> splitted_line;
        util::splitStringWithWhitespaces(line, &splitted_line);

        HintValues values;

        // not comment
        if (splitted_line[0] != "#") {
            // data = e.g. qid:1 1:0 2:2 3:3 4:4 5:2
            int feature_id = 0;
            for (const auto& element : splitted_line) {
                std::vector<std::string> key_and_value;
                util::splitStringUsing(element, ":", &key_and_value);

                // not query id
                if (key_and_value[0] != "qid") {
                    // feature id should be a number string
                    // check all characters which compose the string are digits
                    ASSERT_TRUE(
                        std::all_of(key_and_value[0].cbegin(),
                                    key_and_value[0].cend(),
                                    isdigit));
                    // feature id should be ordered in ascending
                    ASSERT_GT(std::stoi(key_and_value[0]), feature_id);
                    feature_id = std::stoi(key_and_value[0]);
                }

                // this is the same check as that of feature id
                // value should be a number string e.g. "-0.123"
                ASSERT_TRUE(util::is<double>(key_and_value[1]));

                values.emplace_back(stod(key_and_value[1])); // add hint
            }
            ASSERT_EQ(num_of_hints + 1, splitted_line.size()); // +1 for qid

            table->emplace_back(values); // add row
        }
    }
    target_file.close();
}

TEST_F(SvmDataFormatterTest, openFileSuccessfully)
{
    const std::string kExistingDataset =
        "./unittest/dataset/svm_data_formatter_test_data0.txt";
    SvmDataFormatter formatter(kExistingDataset);
    const std::string kOutputPath = "./unittest/out/svm_data_formatter_test0";
    ASSERT_NO_THROW({
        formatter.format(kOutputPath);
    });
}

TEST_F(SvmDataFormatterTest, openFileUnsuccessfully)
{
    const std::string kNotExistingDataset = "./not_exist.txt";
    SvmDataFormatter formatter(kNotExistingDataset);
    const std::string kOutputPath = "./unittest/out/svm_data_formatter_test1";
    ASSERT_THROW({
        formatter.format(kOutputPath);
    }, std::runtime_error);
}

TEST_F(SvmDataFormatterTest, outputFormatedData)
{
    const std::string kExistingDataset =
        "./unittest/dataset/svm_data_formatter_test_data0.txt";
    SvmDataFormatter formatter(kExistingDataset);

    const std::string kOutputPath = "./unittest/out/svm_data_formatter_test2";
    formatter.format(kOutputPath);

    const int kNumOfHints = 5;
    ValueTable table;
    checkOutputFileIsValid(formatter.outputFilePath(), kNumOfHints, &table);
}

TEST_F(SvmDataFormatterTest, outputFormatedDataWithSynonym)
{
    const std::string kExistingDataset =
        "./unittest/dataset/svm_data_formatter_test_data1.txt";
    SvmDataFormatter formatter(kExistingDataset);
    formatter.enableSynonyms(true);

    const std::string kOutputPath = "./unittest/out/svm_data_formatter_test3";
    formatter.format(kOutputPath);

    const int kNumOfHints = 10;
    ValueTable table;
    checkOutputFileIsValid(formatter.outputFilePath(), kNumOfHints, &table);
}

TEST_F(SvmDataFormatterTest, checkFormatedDataWithSynonymsWhichHaveTheSameCountAsNormalPredicates)
{
    // Predicate::NORMAL and Predicate::SYNONYM have the same values in this data
    const std::string kPredicateNormalAndSynonymsHaveSameValueDataset =
        "./unittest/dataset/svm_data_formatter_test_data2.txt";
    SvmDataFormatter formatter(kPredicateNormalAndSynonymsHaveSameValueDataset);
    formatter.enableSynonyms(true);

    const std::string kOutputPath = "./unittest/out/svm_data_formatter_test4";
    formatter.format(kOutputPath);

    const int kNumOfHints = 10;
    ValueTable table;
    checkOutputFileIsValid(formatter.outputFilePath(), kNumOfHints, &table);

    // 0: qid
    // 1 - 5: Predicate::NORMAL
    // 6 - 10: Predicate::SYNONYM
    const int hint_set = 5;
    for (const auto& values : table)
        for (int i = 1; i <= hint_set; ++i)
            ASSERT_EQ(values[i], values[i + hint_set]);
}
