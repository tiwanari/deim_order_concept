#!/usr/bin/env ruby
require_relative "print"
require 'fileutils'
require 'yaml'

# This script was updated around ver19 results
# if you want to run evaluations on old results
# you should care about 'epsilon'
# e.g., old files have names like
# svm_result_ep0_c0.1_k0.txt
# (ep = epsilon)
# but, now we use
# svm_result_c0.1_k0.txt

config = YAML.load(File.open("../config.yml").read)
RESULTS_FOLDER = "../#{config["directory"]["results"]}"
QUERIES_FOLDER = "../#{config["directory"]["queries"]}"

OUT_LENGTH = 80

require_params = 3
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_data kernel|type path_to_gold"
    puts "\t- path_to_data should be in #{RESULTS_FOLDER}"
    puts "\t- path_to_gold should be in #{QUERIES_FOLDER}"
    puts "e.g. #{__FILE__} ver18/all/all/svm k0|t11 queries10/gold/golds/all"
    exit 1
end

path_to_data = "#{RESULTS_FOLDER}/#{ARGV[0]}"
type = ARGV[1]
path_to_gold = "#{QUERIES_FOLDER}/#{ARGV[2]}"

classifier = File.basename(path_to_data)
classifier = "svm" if classifier =~ /.*svm.*/
classifier = "svr" if classifier =~ /.*svr.*/

path_to_tuned_cs_file = "#{path_to_data}/tuned_cs.txt"
path_to_tuned_rhos_file = "#{path_to_data}/tuned_rhos.txt"
path_to_tuned_ranking_file = "#{path_to_data}/tuned_ranking.txt"

RANKING_FILE_TEMPLATE = "#{classifier}_result_c%s_#{type}.txt"
RHOS_FILE_TEMPLATE = "rhos_c%s_#{type}.txt"

MODEL_FILE_PATH_TEMPLATE = "#{path_to_data}/validation/model/model%s"

STDERR.puts "path_to_data: #{path_to_data}"
STDERR.puts "type: #{type}"
STDERR.puts "path_to_gold: #{path_to_gold}"
STDERR.puts "classifier: #{classifier}"
STDERR.puts "path_to_tuned_cs_file: #{path_to_tuned_cs_file}"
STDERR.puts "path_to_output: (rhos: #{path_to_tuned_rhos_file}, ranking: #{path_to_tuned_ranking_file})"

query_cs = []
File.open(path_to_tuned_cs_file) do |file|
    file.each_line do |line|
        # each line looks like this:
        # best (i, c, rho) = (0, 100.0, 0.46766513056835635)
        data = line.strip.split(") = (").last # e.g. 0, 100.0, 0.46766513056835635)
        query_id, c_value, _ = data.split(", ")
        query_cs[query_id.to_i] = c_value.to_f == c_value.to_i ? c_value.to_i : c_value.to_f
    end
end

model_output_path = "#{path_to_data}/models"
FileUtils.mkdir_p(model_output_path)

rhos_body = ""
ranking_body = ""
query_cs.each.with_index do |c, i|

    # === run an evaluation with the c value === {{{
    # e.g., ./run_svm_evaluation.sh ver16/feed/queries4 queries4/gold 0 [0.01]
    STDERR.puts `cd ..; ./run_#{classifier}_evaluation.sh #{File.dirname(ARGV[0])} #{ARGV[2]} 0 #{c}`

    # STDERR.puts "copy model#{i} with c value #{c} -> #{model_output_path}/model#{i}"
    model = "#{sprintf(MODEL_FILE_PATH_TEMPLATE, i)}"
    `cp #{model} #{model_output_path}/model#{i}`
    # === /run an evaluation with the c value === }}}


    # === gather results === {{{
    # puts_line("reading rho file...", OUT_LENGTH, STDERR)
    File.open("#{path_to_data}/#{sprintf(RHOS_FILE_TEMPLATE, c)}") do |file|
        # skip 3 lines (they are a kind of notifications)
        file.gets
        file.each_line do |line|
            query_id, rho = line.split(" ")
            if query_id.to_i == i
                rhos_body.concat("#{query_id} #{rho}\n")
                break
            end
        end
    end

    # puts_line("reading ranking file...", OUT_LENGTH, STDERR)
    File.open("#{path_to_data}/#{sprintf(RANKING_FILE_TEMPLATE, c)}") do |file|
        file.readlines.each_slice(2).with_index do |(adj, ranking), j|
            if i == j
                ranking_body.concat("#{adj}#{ranking}")
                break
            end
        end
    end
    # === /gather results === {{{
end

File.write(path_to_tuned_rhos_file, rhos_body)
File.write(path_to_tuned_ranking_file, ranking_body)
