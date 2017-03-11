// -*- coding: utf-8 -*-

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <set>
#include <iostream>
#include <sstream>
#include <queue>
#include <fstream>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <fstream>
#include "cedar.h"
#include "util/trim.h"
#ifdef USE_FCGI
#include <fcgi_stdio.h>
#endif

const char* JDEPP_PATH = "/path/to/jdepp/trained/with/neologd";
const char* PYTHON_PATH = "/path/to/python";
const char* TWEET_TO_SENT_SCRIPT_PATH = "./tweet2sent_or.py";

const char* COUNTER_PROGRAM_PATH = "/path/to/order_concept/src/main";
const char* COUNTER_PROGRAM_OUTPUT_DIR_PATH = "/path/to/tmp";
const char* COUNTER_PROGRAM_PATTTERN_DIR_PATH = "/path/to/order_concept/dataset/ja/count_patterns/ipa";
const char* COUNTER_PROGRAM_OUTPUT_PATH = "/path/to/tmp/format/output.txt";

const char* COUNTER_PROGRAM_OUTPUT_COUNT_SUBPATH = "counted";
const char* COUNTER_PROGRAM_OUTPUT_SVMDATA_SUBPATH = "format/output.txt";

std::string INDEX_DIR_PATH = "/path/to/index/";

std::string QUERY_ROOT_DIR_PATH = "/path/to/queries13";

static const size_t BUFFER_SIZE = 1 << 21;
static const size_t KEY_SIZE = 8;

std::queue <int> die_queue;

typedef cedar::da <int> trie_t;

// read lines from a file {{{
int readLinesFromAFile(
    const std::string& input_filename,
    std::vector<std::string>* lines)
{
    std::ifstream input_file;
    input_file.open(input_filename.c_str());
    if (!input_file) {
        std::cerr << "Can't open file: " << input_filename << std::endl;
        input_file.close();
        exit(1);
    }
    std::string line;
    int read_lines = 0;
    while (std::getline(input_file, line)) {
        if (line.empty()) continue;
        read_lines++;
        lines->push_back(util::trim(line));
    }
    input_file.close();
    return read_lines;
}
// read lines from a file }}}

// decode query {{{
char* decode_query (char* sin, size_t lin = 0) {
  // URL decode
  if (! lin) lin = std::strlen (sin);

  char* p     = &sin[0];
  char* p_end = &sin[lin];
  char* q     = &sin[0];
  for(; p != p_end; ++p, ++q) {
    if (*p == '+') { *q = ' '; continue; }
    if (*p != '%') { *q = *p;  continue; }
    char tmp = (*++p >= 'A' ? *p - 'A' + 10 : *p - '0');
    tmp *= 16;
    tmp += (*++p >= 'A' ? *p - 'A' + 10 : *p - '0');
    *q = tmp;
  }
  *q = '\0';
  return sin;
}
// /decode query }}}

// split and store to ret {{{
void split (const char* s, char delim, std::vector <std::string>& ret) {
  ret.clear ();
  char s_[BUFFER_SIZE];
  std::strcpy (s_, s);
  decode_query (s_);
  std::stringstream ss (s_);
  std::string item;
  while (std::getline (ss, item, delim))
    if (!item.empty ())
      ret.push_back (item);
}
// /split and store to ret }}}

// join items {{{
std::string join_string(const std::vector <std::string>& items, const std::string& delim) {
  std::string joined (items[0]);
  for (size_t i = 1; i < items.size (); ++i)
    joined += delim + items[i];
  return joined;
}
// join items }}}

void parseQS (
    char* q,
    std::vector <std::string>& periods,
    std::vector <std::string>& prefs,
    std::vector <std::string>& genders,
    int& query_id,
    bool& use_synonyms) {

  char * qstr = getenv("QUERY_STRING");
  if (! qstr) return ;
  char qs[BUFFER_SIZE];

  // parse query string
  std::strcpy (&qs[0], qstr);
  for (char* p = std::strtok (qs, "&"); p != NULL; p = std::strtok (NULL, "&")) {
    if (std::strncmp (p, "periods=", std::strlen("periods=")) == 0) {
      split (p + std::strlen("periods="), ',', periods);
    } else if (std::strncmp (p, "prefs=", std::strlen("prefs=")) == 0) {
      split (p + std::strlen("prefs="), ',', prefs);
    } else if (std::strncmp (p, "genders=", std::strlen("genders=")) == 0) {
      split (p + std::strlen("genders="), ',', genders);
    } else if (std::strncmp (p, "query=", std::strlen("query=")) == 0) {
      query_id = std::strtol (p + std::strlen("query="), NULL, 0);
    } else if (std::strncmp (p, "syn=", std::strlen("syn=")) == 0) {
      int ok = std::strtol (p + std::strlen("syn="), NULL, 0);
      use_synonyms = (ok != 0);
    }
  }
}

// class definition {{{
class byte_encoder {
public:
  byte_encoder () : _len (0), _key () {}
  byte_encoder (unsigned int i) : _len (0), _key () { encode (i); }
  unsigned int encode  (unsigned int i, unsigned char* const key_) const {
    unsigned int len_ = 0;
    for (key_[len_] = (i & 0x7f); i >>= 7; key_[++len_] = (i & 0x7f))
      key_[len_] |= 0x80;
    return ++len_;
  }
  void encode (const unsigned int i) { _len = encode (i, _key); }
  unsigned int decode (unsigned int& i, const unsigned char* const
                       key_) const {
    unsigned int b (0), len_ (0);
    for (i = key_[0] & 0x7f; key_[len_] & 0x80; i += (key_[len_] & 0x7fu) << b)
      ++len_, b += 7;
    return ++len_;
  }
  unsigned int        len () { return _len; }
  const char* key () { return reinterpret_cast <const char*> (&_key[0]); }
private:
  unsigned int  _len;
  unsigned char _key[KEY_SIZE];
};
// /class definition }}}

// popen2 {{{
int popen2 (int *fd_r, int *fd_w, const char* command, const char* argv[], bool will_die) {
    int pipe_child2parent[2];
    int pipe_parent2child[2];
    int pid;
    if (::pipe (pipe_child2parent) < 0) return -1;
    if (::pipe (pipe_parent2child) < 0) {
      ::close (pipe_child2parent[0]);
      ::close (pipe_child2parent[1]);
      return -1;
    }
    // fork
    if ((pid = fork ()) < 0) {
      ::close (pipe_child2parent[0]);
      ::close (pipe_child2parent[1]);
      ::close (pipe_parent2child[0]);
      ::close (pipe_parent2child[1]);
      return -1;
    }
    if (pid == 0) { // child process
      ::close (pipe_parent2child[1]);
      ::close (pipe_child2parent[0]);
      ::dup2 (pipe_parent2child[0], 0);
      ::dup2 (pipe_child2parent[1], 1);
      ::close (pipe_parent2child[0]);
      ::close (pipe_child2parent[1]);
      if (::execv (command, const_cast <char* const*> (argv)) < 0) {
        ::perror ("popen2");
        ::exit (1);
        ::close (pipe_parent2child[0]);
        ::close (pipe_child2parent[1]);
        return -1;
      }
    }

    if (will_die) {
        die_queue.push(pid);
    }
    ::fprintf (stderr, "%s (pid = %d) spawn.\n", command, pid);

    ::close (pipe_parent2child[0]);
    ::close (pipe_child2parent[1]);

    *fd_r = pipe_child2parent[0];
    *fd_w = pipe_parent2child[1];

    return pid;
}
// /popen2 }}}

// generate labels {{{
void read_keys_from_a_flie (const std::string& filename, std::vector <std::string>& keys) {
  char line[BUFFER_SIZE];
  FILE* fp = ::fopen (filename.c_str (), "r");
  while (::fgets (line, BUFFER_SIZE, fp))
    keys.push_back (std::string (line, std::strlen (line) - 1));
  ::fclose (fp);
}

void join_keys (
  const std::vector <std::string>& periods,
  const std::vector <std::string>& prefs,
  const std::vector <std::string>& genders,
  std::vector <std::string>& labels) {

  for (size_t i = 0; i < periods.size (); ++i)
    for (size_t j = 0; j < genders.size (); ++j)
      for (size_t k = 0; k < prefs.size (); ++k)
        labels.push_back (periods[i] + "/" + genders[j] + "_" + prefs[k]);
}

void genearte_labels(std::vector <std::string>& labels) {
  std::vector <std::string> periods, prefs, genders;

  read_keys_from_a_flie (INDEX_DIR_PATH + "periods", periods);
  read_keys_from_a_flie (INDEX_DIR_PATH + "prefs", prefs);
  read_keys_from_a_flie (INDEX_DIR_PATH + "genders", genders);

  join_keys (periods, prefs, genders, labels);
}
// /generate labels }}}

// load index {{{
int load_index (
    const std::vector <std::string>& labels,
    std::tr1::unordered_map <std::string, std::vector <unsigned char*> >& index,
    std::tr1::unordered_map <std::string, unsigned char*>& buf) {

  ::fprintf (stderr, "reading index..");
  for (size_t i = 0; i < labels.size (); ++i) {
    size_t size = 0;
    FILE* fp = ::fopen ((INDEX_DIR_PATH + labels[i] + ".index").c_str (), "rb");
    if (! fp) return -1;
    // get size
    if (::fseek (fp, 0, SEEK_END) != 0) return -1;
    size = static_cast <size_t> (::ftell (fp));
    if (::fseek (fp, 0, SEEK_SET) != 0) return -1;

    unsigned char* buf_
        = buf[labels[i]]
        = static_cast <unsigned char*>  (std::malloc (sizeof (unsigned char) * size));
    if (size != ::fread (buf_, sizeof (unsigned char), size, fp)) return -1;
    ::fclose (fp);

    index.insert (std::make_pair (labels[i], std::vector <unsigned char*> ()));
    std::vector <unsigned char*>& index_ = index[labels[i]];
    for (unsigned char* p = &buf_[0]; p != &buf_[size]; ++p) {
      index_.push_back (p);
      while (*p) ++p;
    }
  }
  ::fprintf (stderr, "done.\n");
  return 0;
}
// /load index }}}

// load sentences {{{
int load_sent (
    const std::vector <std::string>& labels,
    std::tr1::unordered_map <std::string, char*>& sent) {

  ::fprintf (stderr, "reading sent..");
  for (size_t i = 0; i < labels.size (); ++i) {
    size_t size = 0;
    FILE* fp = ::fopen ((INDEX_DIR_PATH + labels[i] + ".sent").c_str (), "rb");
    if (! fp) return -1;
    // get size
    if (::fseek (fp, 0, SEEK_END) != 0) return -1;
    size = static_cast <size_t> (::ftell (fp));
    if (::fseek (fp, 0, SEEK_SET) != 0) return -1;

    sent.insert (std::make_pair (labels[i], static_cast <char*>  (std::malloc (sizeof (char) * size))));
    char* sent_ = sent[labels[i]];
    if (size != ::fread (sent_, sizeof (char), size, fp)) return -1;
    ::fclose (fp);
  }
  ::fprintf (stderr, "done.\n");
  return 0;
}
// /load sentences }}}

void search_lines_as_offsets (
    const trie_t& keys,
    const std::vector <std::string>& adjs,
    const std::vector <std::string>& nouns,
    const std::vector <unsigned char*>& index_,
    std::set <int>& ret) {

  byte_encoder encoder;

  for (size_t l = 0; l < adjs.size (); ++l) {
    const char* adj = adjs[l].c_str ();
    int n = keys.exactMatchSearch <int> (adj);
    if (n != trie_t::CEDAR_NO_VALUE && n != trie_t::CEDAR_NO_PATH) {
      int i = 0;
      // ::fprintf (stderr, "%s found.\n", adj);
      while (*(index_[n] + i)) {
        unsigned int offset = 0;
        i += encoder.decode (offset, index_[n] + i);

        ret.insert (offset);     // NOTE: for OR check
      }
    } else {
      // ::fprintf (stderr, "%s not found.\n", adj);
    }
  }

  for (size_t l = 0; l < nouns.size (); ++l) {
    const char* noun = nouns[l].c_str ();
    int n = keys.exactMatchSearch <int> (noun);
    if (n != trie_t::CEDAR_NO_VALUE && n != trie_t::CEDAR_NO_PATH) {
      int i = 0;
      // ::fprintf (stderr, "%s found.\n", noun);
      while (*(index_[n] + i)) {
        unsigned int offset = 0;
        i += encoder.decode (offset, index_[n] + i);

        // NOTE: for OR check
        ret.insert (offset);
      }
    } else {
      // ::fprintf (stderr, "[%s] not found.\n", noun);
    }
  }
}

// kill all {{{
bool wait_all() {
  bool has_error = false;
  while (!die_queue.empty()) {
    int pid = die_queue.front(); die_queue.pop();
    int status;

    pid_t w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
    if (w == -1) { perror("waitpid"); exit(EXIT_FAILURE); }

    if (WIFEXITED(status)) {
      ::fprintf(stderr, "pid (%d) exited, status=%d\n", pid, WEXITSTATUS(status));
      if (WEXITSTATUS(status) != 0) has_error = true;
    } else if (WIFSIGNALED(status)) {
      ::fprintf(stderr, "***ERROR!***: pid (%d) was killed by signal %d\n", pid, WTERMSIG(status));
      has_error = true;
    } else if (WIFSTOPPED(status)) {
      ::fprintf(stderr, "pid (%d) was stopped by signal %d\n", pid, WSTOPSIG(status));
      has_error = true;
    } else if (WIFCONTINUED(status)) {
      ::fprintf(stderr, "pid (%d) continued\n", pid);
    }
    if (!WIFEXITED(status) && !WIFSIGNALED(status)) {
      die_queue.push(pid);
    }
  }
  return has_error;
}
// /kill all }}}

std::string generateRandomString() {
    // get 2 random values and use them to name the output folder
    long rand_val0 = std::rand();
    long rand_val1 = std::rand();

    std::stringstream ss;
    ss << rand_val0 << "_" << rand_val1;
    return ss.str();
}

int main (int argc, char** argv) {
  trie_t keys;
  if (keys.open ((INDEX_DIR_PATH + "keys.trie").c_str()) == -1)
    std::exit (1);

  std::vector <std::string> labels;
  genearte_labels (labels); // for loading every index and sent

  std::tr1::unordered_map <std::string, std::vector <unsigned char*> > index;
  std::tr1::unordered_map <std::string, unsigned char*> buf;
  std::tr1::unordered_map <std::string, char*> sent;
  if (load_index (labels, index, buf) != 0) return -1;
  if (load_sent (labels, sent) != 0) return -1;

  char q[256] = "*";
  std::vector <std::string> periods, prefs, genders;

  int query_id;
  bool use_synonyms;

  // default values
  split ("2012/01,2012/02,2012/03,2012/04,2012/05", ',', periods);
  split ("東京", ',', prefs);
  split ("F", ',', genders);
  // split ("暑い,寒い", ',', adjs);
  // split ("東京,シドニー", ',', nouns);
  query_id = 1;
  use_synonyms = false;

  int fd_r1 (0), fd_r2 (0), fd_r3 (0),
      fd_w1 (0), fd_w2 (0), fd_w3 (0);
  const char* jdepp_args[] = {JDEPP_PATH, 0};
  ::popen2 (&fd_r2, &fd_w2, JDEPP_PATH, jdepp_args, false); // run new jdepp

  std::srand(std::time(NULL)); // initialize random

#ifdef USE_FCGI
  while (FCGI_Accept() >= 0)
#else
#endif
  {
    ::fprintf (stdout, "Content-type: text/html; charset=UTF-8\r\n\r\n");
    // ::fprintf (stdout, "Content-type: appliation/json; charset=UTF-8\r\n\r\n");

    // parse query
    parseQS (&q[0], periods, prefs, genders, query_id, use_synonyms);


    // === read queries ===
    std::vector <std::string> adjs, nouns;

    std::stringstream ss;
    ss << QUERY_ROOT_DIR_PATH << "/query" << query_id ;
    const std::string query_path = ss.str();

    // read adj/ant, concept_file with query_id
    const std::string adj_file = query_path + "/adjective.txt";
    assert(1 == readLinesFromAFile(adj_file, &adjs));

    const std::string ant_file = query_path + "/antonym.txt";
    int read_ant = readLinesFromAFile(ant_file, &adjs);
    assert(read_ant == 0 || read_ant == 1);
    if (read_ant == 0) adjs.push_back ("xxx"); // ant_file may be empty

    const std::string concepts_file = query_path + "/concepts.txt";
    readLinesFromAFile(concepts_file, &nouns);
    // /read adj/ant, concept_file with query_id


    std::string adj_synonyms_file = "";
    std::string ant_synonyms_file = "";
    if (use_synonyms) {
        // read synonyms on the basis of 'use_synonyms' and 'query_id'
        adj_synonyms_file = query_path + "/best_k.txt";
        assert(readLinesFromAFile(adj_synonyms_file, &adjs)); // read as adjs for index search

        ant_synonyms_file = query_path + "/worst_k.txt";
        assert(readLinesFromAFile(ant_synonyms_file, &adjs)); // read as adjs for index search
        // /read synonyms on the basis of 'use_synonyms' and 'query_id'
    }
    // === /read queries ===
    std::string query_random = generateRandomString();
    ::fprintf (stderr, "output_path: %s\n", query_random.c_str());
    std::cerr << query_random << "> " << "query_id: " << query_id << std::endl;
    std::cerr << query_random << "> " << "concepts: " << join_string (nouns, ",") << std::endl;
    std::cerr << query_random << "> " << "adjs: " << adjs[0] << std::endl;
    std::cerr << query_random << "> " << "ant: " << adjs[1] << std::endl;
    std::cerr << query_random << "> " << "periods: " << join_string (periods, ",") << std::endl;
    std::cerr << query_random << "> " << "prefs: " << join_string (prefs, ",") << std::endl;
    std::cerr << query_random << "> " << "genders: " << join_string (genders, ",") << std::endl;
    std::cerr << query_random << "> " << "sym: " << use_synonyms << std::endl;
    if (use_synonyms) {
        std::cerr << query_random << "> " << "all adjs: " << join_string (adjs, ",") << std::endl;
    }

    std::stringstream output_dir;
    output_dir << COUNTER_PROGRAM_OUTPUT_DIR_PATH << "/" << query_random;
    // usage: ./main --output_dir=string --adjective=string --antonym=string --concept_file=string [options] ...
    // options:
    //         -o, --output_dir           output_dir (string)
    //         -a, --adjective            adjective (string)
    //         -n, --antonym              antonym (string)
    //         -c, --concept_file         concept_file (string)
    //         -p, --pattern_file_path    pattern_file_path (string [=../dataset/ja/count_patterns/ipa])
    //         -t, --morph                morph type [IPA | JUMAN] (string [=IPA])
    //         -A, --syns_adj             synonyms of adjective (string [=])
    //         -N, --syns_ant             synonyms of antonym (string [=])
    if (use_synonyms) {
        const char* counter_args[]
            = {COUNTER_PROGRAM_PATH, "-o", output_dir.str().c_str(),
                "-a", adjs[0].c_str (), "-n", adjs[1].c_str (),
                "-c", concepts_file.c_str (),
                "-A", adj_synonyms_file.c_str(), "-N", ant_synonyms_file.c_str(),
                "-p", COUNTER_PROGRAM_PATTTERN_DIR_PATH,
                0};
        ::popen2 (&fd_r3, &fd_w3, COUNTER_PROGRAM_PATH, counter_args, true);
    }
    else { // not use synonyms
        const char* counter_args[]
            = {COUNTER_PROGRAM_PATH, "-o", output_dir.str().c_str(),
                "-a", adjs[0].c_str (), "-n", adjs[1].c_str (),
                "-c", concepts_file.c_str (),
                "-p", COUNTER_PROGRAM_PATTTERN_DIR_PATH,
                0};
        ::popen2 (&fd_r3, &fd_w3, COUNTER_PROGRAM_PATH, counter_args, true);
    }


    // create labels from input parameters
    std::vector <std::string> labels;
    join_keys (periods, prefs, genders, labels); // labels = {period1/gender1/prefs1, ...}


    // read tweets and convert for jdepp
    std::string nouns_s = join_string (nouns, ","); // nouns_s = "CONCEPT1,CONCEPT2,..."
    const char* tweet2sent_args[] = {PYTHON_PATH, TWEET_TO_SENT_SCRIPT_PATH, nouns_s.c_str (), 0};
    ::popen2 (&fd_r1, &fd_w1, PYTHON_PATH, tweet2sent_args, true);

    long n = 0;
    for (size_t i = 0; i < labels.size (); ++i) {
      std::vector <unsigned char*>& index_ = index[labels[i]];
      std::set <int> offsets;

      // process query
      search_lines_as_offsets (keys, adjs, nouns, index_, offsets);

      char* sent_ = sent[labels[i]];
      for (std::set <int>::iterator it = offsets.begin (); it != offsets.end (); ++it, ++n) {

        // write a sent and read a tweet
        char buf[BUFFER_SIZE];
        ::write (fd_w1, &sent_[*it], std::strlen (&sent_[*it]));
        ::write (fd_w1, "\n", 1);

        size_t read = ::read (fd_r1, buf, sizeof (char) * BUFFER_SIZE);

        // input a tweet to jdepp and read result
        ::write (fd_w2, buf, read);
        // ::write (fd_w2, "\n", 1);

        // count breaks
        int num_break = 0;
        for (int k = 0; k < read; k++) if (buf[k] == '\n') num_break++;

        int checked_break = 0;
        while (checked_break < num_break) {
            read = ::read (fd_r2, buf, sizeof (char) * BUFFER_SIZE);
            buf[read] = '\0';

            // count EOS
            char *sp = buf;
            while ((sp = ::strstr(sp, "EOS\n")) != NULL) {
                checked_break++;
                sp++;
            }
            // write the jdepp result to counter
            ::write (fd_w3, &buf, read);
        }
      }
      ::fflush (stdout);
    }
    ::fprintf (stderr, "%d lines found.\n", n);

    // kill tweet2sent script
    ::write (fd_w1, "EOF\n", 4);
    ::fflush (stdout);
    ::close (fd_r1); ::close (fd_w1);

    ::fprintf (stderr, "count done.\n");
    ::fflush (stdout);

    // kill counter program
    ::write (fd_w3, "EOF\n", 4);
    ::close (fd_r3); ::close (fd_w3);

    // wait child processes
    bool has_error = wait_all();

    if (has_error) {
        ::fprintf (stdout, "ERROR!\n");
        ::fflush (stdout);
        continue;
    }

    // write output result
    output_dir << "/" << COUNTER_PROGRAM_OUTPUT_COUNT_SUBPATH;
    std::string output_path = output_dir.str();
    std::ifstream content(output_path.c_str());

    std::string reading_line_buffer;
    while (!content.eof())
    {
        // read by line
        std::getline(content, reading_line_buffer);
        ::fprintf (stdout, reading_line_buffer.c_str());
        ::fprintf (stdout, "\n");
    }
    ::fflush (stdout);
    ::fprintf (stderr, "wrote output.\n");
  }

  // free some variables
  for (size_t i = 0; i < labels.size (); ++i) {
    std::free (buf[labels[i]]);
    std::free (sent[labels[i]]);
  }
}
