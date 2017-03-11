def count_substr(str, substr)
    return str.scan(/(?=#{substr})/).count
end

def count_rows(file_path, ignore="")
    cmd = "cat #{file_path} | grep -v -e \"^#{ignore}\" | wc -l"
    STDERR.puts cmd
    return `#{cmd}`.to_i
end

def count_files(dir, name_regex)
    cmd = "find #{dir} -name \"#{name_regex}\" -type f -o -type l | wc -l"
    STDERR.puts cmd
    return `#{cmd}`.to_i
end
