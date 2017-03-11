# Inducing Writers’ Values on Concept Ordering from Microblog
- **This is a snapshot of programs for [DEIM 16](http://db-event.jpn.org/deim2017/).**
- **This has not been refactored and will not be maintained.**

author: Tatsuya Iwanari

# Structure
This repository includes the files used in our demo system. The structure is like this;

```
.
├── README.md
├── dataset
├── demo
├── scripts
├── src
├── tools
├── googletest
└── xz
```

## Submodules
The following two submodules are used in our counter program.

- `googletest`
    - testing
- `xz`
    - unarchiving .xz files.

## Tools
`tools` has some programs like `svr` or `svm` as tools to order concepts.

## Dataset
`dataset` has the evaluation dataset for Coling 2016.

## Scripts
`scripts` contains some evaluation scripts and helper demo scripts for Coling 2016.

## Demo
`demo` is the folder that contains programs for accessing to our back-end system (i.e., endpoints) and making indices.

## Evidence Counter
`src` keeps the codes of the evidence counter.

### Make
Move to _src_ folder and run a command ``make``.

### Usage
In _src_ folder, type the following command

```
usage: ./main --mode=int --input_file=string --output_path=string --adjective=string --antonym=string --concept_file=string [options] ...
options:
  -m, --mode                 mode [prep: -1 | count: 0 | reduce: 1 | format : 2] (int)
  -i, --input_file           input_file (string)
  -o, --output_path          output_path (string)
  -?, --help                 print this message
  -a, --adjective            adjective (mode: -1, 0) (string)
  -n, --antonym              antonym (mode: -1, 0) (string)
  -c, --concept_file         concept_file (mode: -1, 0) (string)
  -p, --pattern_file_path    pattern_file_path (mode: -1, 0) (string [=../dataset/ja/count_patterns/juman])
  -t, --morph                morph type [IPA | JUMAN] (mode: -1, 0) (string [=JUMAN])
```

#### mode
_mode_ is an integer parameter to specify what the program will do.
`-1` is used to count all adjectives with given concepts to make dataset.

| value | mode name | explanation |
|:------|:----------|:------------|
| -1 | prep/reduce/format | do all preparation processes. |
| 0 | count/reduce/format | do all processes. |
| 1 | reduce/format | do from the second process. |
| 2 | format | do the last process only. |

#### input\_file
_input\_file_ differs based on the mode parameter.

| mode value | input\_file |
|:-----------|:------------|
| 0 | a list file of social data files. |
| 1 | a list file of counted files. |
| 2 | a file to be formatted in svm rank format. |

A list file is composed of lines, each line expresses a file like this:

```
/path/to/data0
/path/to/data1
...
```

#### output\_path
_output\_path_ is a string parameter to specify the output folder.
All the output of this program will be output into it.

| process | location |
|:--------|:---------|
| count | output\_path/count |
| reduce | output\_path/reduce |
| format | output\_path/format |


#### options for mode -1, 0
mode -1 and 0 have more _options_.

| option | explanation |
|:-------|:------------|
| adjective | an adjective |
| antonym | an antonym |
| concept\_file | a list of concepts in the same format as input\_file |
| [pattern\_file\_path] | a path to pattern files for its counter [=../dataset/ja/count_patterns/juman] |
| [morph] | a morph type for parsing (IPA or JUMAN) [=JUMAN] |


### Dependency
This program depends on `liblzma` for decoding xz files and it
needs the followings:

- Autoconf 2.64
- Automake 1.12
- gettext 0.18 (Note: autopoint depends on cvs!)
- libtool 2.2
