#!/usr/bin/env ruby
require 'find'
require 'fileutils'
# require 'parallel'
require 'yaml'
require_relative '../util/print'
require_relative "../util/count"

# NOTE: this script accepts SVM format file not SVR

# parent_domain_list_file contains a list of parent domains one by one per line
# it is something like (each domain should be placed in #{RESULTS_FOLDER}):
# ver18/all/all
# ver18/kanto/all
# ver18/all/men
# ...
#

config = YAML.load(File.open("../config.yml").read)
RESULTS_FOLDER = "../#{config["directory"]["results"]}"

OUT_LENGTH = 80

def adjust_hint_numbers(hints, skip, hint_offset)
    # split hints and convert hint_list to hash
    hint_array = []
    row_id = 0
    before_hints = ""
    hints.strip.split(" ").each_with_index do |hint, i|
        # just save the value
        if i < skip
            before_hints.concat("#{hint} ")
            next
        end
        # check values
        id, value = hint.split(":")
        if id.to_i == row_id + 1
            hint_array[row_id] = value
            row_id += 1
        else
            STDERR.puts "something wrong!!"
        end
    end
    hint_str = ""
    hint_array.each.with_index(0) do |value, i|
        hint_str.concat("#{i + hint_offset}:#{value} ")
    end

    return before_hints, hint_str, hint_offset + row_id
end

def join_contents(content0, content1)
    new_content = {}
    content0.each do |key, hint|
        new_hint = ""

        before_hints, hint_str, offset = adjust_hint_numbers(hint, 1, 1)
        new_hint.concat(before_hints) # add content0 key
        new_hint.concat(hint_str) # add content0 hints

        _, hint_str, _ = adjust_hint_numbers(content1[key], 1, offset)
        new_hint.concat(hint_str) # add content1 hints

        new_content[key] = new_hint
    end
    return new_content
end



def read_output_file(filename)
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

def read_parents_list(filename)
    domains = []
    File.open(filename).each_line do |line|
        domains << "#{RESULTS_FOLDER}/#{line.chomp}"
    end
    return domains
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



FileUtils.mkdir_p(path_to_output)
`cp #{path_to_list} #{path_to_output}/.`



num_of_data.times do |i|
    data_file = "result#{i + 1}.txt"

    # read child data
    adj, content = read_output_file("#{path_to_data}/#{data_file}")

    # read parent data
    read_parents_list(path_to_list).each do |parent|
        a, c = read_output_file("#{parent}/#{data_file}")

        if adj != a
            STDERR.puts "Error!: a query result (#{data_file}) is diffrent with a parent #{parent}"
            exit 1
        end

        content = join_contents(content, c)
    end

    STDERR.puts "writing #{path_to_output}/#{data_file}"
    File.open("#{path_to_output}/#{data_file}", "w") do |file|
        file << "# #{adj}\n"
        content.each do |concept, hint|
            file << "# #{concept}\n"
            file << "#{hint}\n"
        end
    end
end
