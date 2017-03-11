#!/usr/bin/env ruby
require 'find'
require 'fileutils'
# require 'parallel'
require 'yaml'
require_relative '../util/print'
require_relative "../util/count"

# NOTE: this script accepts SVM format file not SVR

config = YAML.load(File.open("../config.yml").read)
RESULTS_FOLDER = "../#{config["directory"]["results"]}"

OUT_LENGTH = 80

TUNED_RANKING = "tuned_ranking.txt"


def read_parents_list(filename)
    domains = []
    File.open(filename).each_line do |line|
        domains << "#{RESULTS_FOLDER}/#{line.chomp}"
    end
    return domains
end

def read_parent_rankings(parent_dir)
    rankings_file = "#{parent_dir}/#{TUNED_RANKING}"

    rankings = []
    File.open(rankings_file) do |file|
        file.readlines.each_slice(2) do |ranking|
            ranks = {}

            adj_line = ranking.first.chomp # 大きい
            rank_line = ranking.last.chomp # ゾウ,1,クジラ,2,...

            # e.g., 大きい
            adj = adj_line.split(" ").last

            # e.g., {"ゾウ": 1, "クジラ":2, ...}
            rank_line.split(",").each_slice(2) do |con, rank|
                ranks[con] = rank.to_f
            end

            rankings << {adjective: adj, concepts: ranks}
        end
    end

    return rankings
end

def read_child_file(filename)
    adj = ""
    content = {}
    STDERR.puts "reading #{filename}"

    File.open(filename) do |file|
        adj = file.gets.split(" ").last.strip
        file.readlines.each_slice(2) do |lines|
            # "lines" looks like this:
            # # うどん
            # qid:1 1:0 2:3 ...
            # use the first line
            concept = lines.first.split(" ").last.strip
            # use the second line
            content[concept] = lines.last.strip
        end
    end
    return adj, content
end


def parent_value(concept, ranks)
    num_of_concepts = ranks.size.to_f
    return ranks[concept] / num_of_concepts
end


require_params = 3
if ARGV.length < require_params
    STDERR.puts "This script needs at least #{require_params} parameters"
    STDERR.puts "Usage: #{__FILE__} path_to_data path_to_output parent_domain_list_file"
    STDERR.puts "\t- path_to_data & path_to_output should be in #{RESULTS_FOLDER}"
    STDERR.puts "e.g. #{__FILE__} ver18/kanto/men ver18/global/kanto/men list_file"
    exit 1
end

path_to_data = "#{RESULTS_FOLDER}/#{ARGV[0]}"
num_of_data = count_files(path_to_data, "result*.txt") # count data
path_to_output = "#{RESULTS_FOLDER}/#{ARGV[1]}"
path_to_list = ARGV[2]

# path_to_list contains a list of parent domains one by one per line
# it is something like (each domain should be placed in #{RESULTS_FOLDER}):
# ver18/all/all/svm
# ver18/kanto/all/svm
# ver18/all/men/svm
# ...
#




FileUtils.mkdir_p(path_to_output)
`cp #{path_to_list} #{path_to_output}/.`

# read parents' data
parents_data = {}
read_parents_list(path_to_list).each do |parent|
    parents_data[parent] = read_parent_rankings(parent)
end

num_of_data.times do |i|
    data_file = "result#{i + 1}.txt"

    # read child data
    adj, content = read_child_file("#{path_to_data}/#{data_file}")

    content.each do |concept, hint|
        # counts ':' in a line
        # e.g.,
        # qid:1 1:0 2:1 3:1
        # => 4 (= the number of ':') is the next hint id
        append_id = count_substr(hint, ":")

        parents_data.each do |parent, rankings|
            if rankings[i][:adjective] != adj
                STDERR.puts "Error!: a query result (#{data_file}) is diffrent with a parent #{parent}"
                exit 1
            end

            p = parent_value(concept, rankings[i][:concepts])

            content[concept] += " #{append_id}:#{p}"
            append_id += 1
        end
    end

    File.open("#{path_to_output}/#{data_file}", "w") do |file|
        file << "# #{adj}\n"
        content.each do |concept, hint|
            file << "# #{concept}\n"
            file << "#{hint}\n"
        end
    end

end
