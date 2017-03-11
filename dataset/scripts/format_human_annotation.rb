#!/usr/bin/env ruby

require_params = 1
if ARGV.length < require_params
    STDERR.puts "This script needs at least #{require_params} parameters"
    STDERR.puts "Usage: #{__FILE__} path_to_human_data"
    STDERR.puts "e.g. #{__FILE__} ../../dataset/gold/gold2/yokoyama/yokoyama.txt"
    exit 1
end

path_to_human_data = ARGV[0]

STDERR.puts "path_to_human_data: #{path_to_human_data}"

STDERR.puts "start reading..."
File.open(path_to_human_data) do |file|
    file.each_line do |line|
        # "adjective,antonym" line
        if line.start_with?("#")
            print line
            next
        end
        # blank line
        if line.start_with?("\n")
            next
        end
        # puts line as comment
        puts "# #{line}"
        # strip items
        ranks = line.strip.split(" ")
        item_count = 1
        cur_rank = 1
        ranking_str = ""
        ranks.each do |item|
            if item == "="
                ranking_str.concat("#{cur_rank},")
            elsif item == ">" || item == ">>"
                ranking_str.concat("#{cur_rank},")
                cur_rank = item_count
            else
                item_count += 1
                ranking_str.concat("#{item},")
            end
        end
        ranking_str.concat("#{cur_rank}")
        puts ranking_str
    end
end
STDERR.puts "read!"
