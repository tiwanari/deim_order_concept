g++ -I. -O2 -Wunused mkindex.cc -o mkindex
g++ -I. -DUSE_FCGI  -O2 -Wunused deim2017.cc -lfcgi -o deim2017.fcgi
