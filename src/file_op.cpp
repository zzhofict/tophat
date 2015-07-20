#include <sstream>
#include "file_op.h"
//#include "common.h"

//typedef streamsize SS;
const static SS READ_BUFLEN = 4*1024*1024;

double getMseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double) 1000*(tv.tv_sec + 1e-6*tv.tv_usec);
}

template <class Itr>
void print(Itr first, Itr last) {
    typedef iterator_traits<Itr> traits;
    for_each(first, last, Print<typename traits::value_type>());
    cout << endl;
}

struct ThreadArgFile {
    int id;
    string ifStr;
    pthread_t tid;
    SS start;
    SS end;
    vector<SS> *vpos;
};

SS getFileSize(string path) {
	struct stat statBuff;
	if(stat(path.c_str(), &statBuff) < 0)
		return 0;
	return statBuff.st_size;
}

void getRangeLinePos(string fstr, // input file name
                      SS start, //read start position 
                      SS end,  // read end position 
                      vector<SS> &vpos) { // output positions
    ifstream ifs(fstr.c_str(), ios::in|ios::binary);
    SS slen = end-start;
    SS len = 0;
    SS pos = start;
    char *buf = new char[READ_BUFLEN];
    ifs.seekg(start, ifs.beg);
    do {
        ifs.read(buf, min(slen, READ_BUFLEN));
        len = ifs.gcount();
        for(SS i=0; i<len; ++i) {
            if(buf[i] == '\n') {
                vpos.push_back(pos+i+1); 
            }
        }
        pos += len;
        slen -= len;
    } while (len != 0);
}

void *threadGetLinePos(void *_arg) {
    ThreadArgFile &arg = *((ThreadArgFile*)_arg);
    getRangeLinePos(arg.ifStr, arg.start, arg.end, *arg.vpos);
    return NULL;
}



void getFileQueryPos(const string fstr, 
                     vector<SS> &vpos,
                     int nThreads,
                     SS skip, //skip the first lines
                     SS qLine) {// lines for one query
    SS fileSize = getFileSize(fstr);

    vector<ThreadArgFile> args(nThreads);
    vector<vector<SS> > vposes(nThreads, vector<SS>());
    for(int i=0; i<nThreads; ++i) {
        args[i].id = i;
        args[i].ifStr = fstr;
        args[i].start = ITEM_BEGIN(i, nThreads, fileSize);
        args[i].end = ITEM_END(i, nThreads, fileSize);
        args[i].vpos = &vposes[i];
        pthread_create(&args[i].tid, NULL, threadGetLinePos, &args[i]);
    }
    for(int i=0; i<nThreads; ++i)
        pthread_join(args[i].tid, NULL);

    if(skip == 0) vpos.push_back(0);
    SS line = 0;
    for(int ti=0; ti<nThreads; ++ti) {
        vector<SS> &v = vposes[ti];
        for(size_t i=0; i<v.size(); ++i) {
            ++line;
            if(line % qLine == 0)
                vpos.push_back(v[i]);
        }   
    }

}

/*
ReadFormat getFileFormat(string fname) {

    ifstream ifs(fname.c_str(), ios::in|ios::binary);
    while(!ifs.eof()) {
        string line;
        ifs >> line;
        if(line[0] == '@') return FASTQ;
        if(line[0] == '>') return FASTA;
    }
    return FASTX_AUTO;
}
*/

void getFilePartByThreads(string fname, 
                          vector<pair<long, long> > &vpos, 
                          int numPart,
                          const char schr) {

    SS fileSize = getFileSize(fname);
    ifstream ifs(fname.c_str(), ios::in|ios::binary);
    long start = 0, end = 0;
    char ch;

    for(int i=0; i<numPart; ++i) {
/** for test
        SS tmp = ifs.tellg();
        ifs.seekg(start, ifs.beg);
        char tch;
        ifs >> tch;
        cout << tch << endl;
**/
        end = ITEM_END(i, numPart, fileSize);
        ifs.seekg(end, ifs.beg);
        while(ifs >> ch && ch != schr);
        if(ifs.eof()) end = fileSize;
        else end = (long)ifs.tellg() - 1;
        vpos.push_back(make_pair(start, end));
        start = end;
    }
    while((int)vpos.size() < numPart) vpos.push_back(make_pair(fileSize, fileSize));
    ifs.close();
}


//
//int main(int argc, char** argv) {
//
//    string ifStr(argv[1]); 
//    /*
//       vector<SS> vpos;
//       double t1 = getMseconds();
//       getRangeLinePos(ifStr, 0, getFileSize(ifStr), vpos); 
//       cout << "time 1: " << (getMseconds()-t1)/1000 << " s" << endl;
//       cout << vpos.size() << endl;
//       print(vpos.end()-10, vpos.end());
//       cout << endl;
//       */
//    vector<SS> v;
//    vector<pair<SS,SS> > vv;
//    double t2 = getMseconds();
//    getFileQueryPos(ifStr, v, 32);
//
////    getFilePartByThreads(ifStr, vv, 16, '@');
//    cout << "time 2: " << (getMseconds()-t2)/1000 << " s" << endl;
//    cout << v.size() << endl;
////    for(size_t i=0; i<vv.size(); i++) cout <<vv[i].first << ' ' << vv[i].second << endl;
//
//
//    print(v.end()-10, v.end());
//    cout << endl;
//
//    return 0;
//}
