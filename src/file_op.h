#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <pthread.h>
#include <algorithm>
#include <sys/stat.h>
#include <iterator>
#include <sys/time.h>
//#include "common.h"

using namespace std;

#define ITEM_BEGIN(id, p, size) ((size) * (id) / (p))
#define ITEM_END(id, p, size) (ITEM_BEGIN((id+1), p, size))

typedef long SS;

//enum ReadFormat_v2 { FASTX_AUTO=0, FASTQ, FASTA };

template <class T>
struct Print {
    void operator ()(const T &t) {
        cout << t << ' ';
    }
};



double getMseconds();

SS getFileSize(string path);

void getRangeLinePos(string fstr, // input file name
                      SS start, //read start position 
                      SS end,  // read end position 
                      vector<SS> &vpos);// output positions
 
void getFileQueryPos(string fstr, 
                     vector<SS> &vpos,
                     int nThreads,
                     SS skip=0, //skip the first lines
                     SS qLine=4);// lines for one query

void getFilePartByThreads(string fname, 
                          vector<pair<SS,SS> > &vpos, 
                          int numPart,
                          const char schr); 

//ReadFormat getFileFormat(string fname);

