#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <algorithm>
#include <set>

using namespace std;

pthread_mutex_t mutex;
pthread_barrier_t barr;

struct MapperStruct {
    int M; // number of mappers
    int R; // number of reducers
    int numberOfFiles; // number of files from test.txt
    int id;
    vector<string> *files;
    vector<vector<int>> perfectPower; // matrix of perfect powers
};

struct ReducerStruct {
    int M; // number of mappers
    int R; // number of reducers
    int id;
    vector<struct MapperStruct> *mapper;
};

// function which returns all the possibilities to write a nr (number) as a perfect power
vector<vector<int>> binarySearch(int nr, int R);

// thread function for mappers
void* Mapper(void* arg);

// thread function for reducers
void* Reducer(void* arg);

int main(int argc, char* argv[])
{
    int M = (int)argv[1][0] - 48;
    int R = (int)argv[2][0] - 48;
    string entryFile = argv[3];
    pthread_t threads[M + R];

    vector<struct MapperStruct> mapper;
    vector<struct ReducerStruct> reducer;

    vector<string> filesAux;
    ifstream MyReadFile;

    MyReadFile.open(argv[3]);

    int numberOfFiles;
    MyReadFile >> numberOfFiles;

    int i = 0;
    while (MyReadFile.good()) {
        string aux;
        MyReadFile >> aux;
        filesAux.push_back(aux);
        i++;
    }

    // I initialize every mapper from the vector;
    // each mapper will have its own id
    for (int i = 0; i < M; i++) {
        struct MapperStruct mapper2;
        mapper2.files = &filesAux;
        mapper2.M = M;
        mapper2.R = R;
        mapper2.numberOfFiles = numberOfFiles;
        mapper2.id = i;
        int j = 0;
        while (j <= R - 1) {
            vector<int> v;
            mapper2.perfectPower.push_back(v);
            j++;
        }
        mapper.push_back(mapper2);
    }

    // I initialize very reducer from the vector
    // each reducer will have its own id
    for (int i = 0; i < R; i++) {
        struct ReducerStruct reducer2;
        reducer2.M = M;
        reducer2.R = R;
        reducer2.id = i;
        reducer2.mapper = &mapper;
        reducer.push_back(reducer2);
    }

    MyReadFile.close();
    
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&barr, NULL, M + R);

    for (int i = 0; i < M + R; i++) {
        if (i < M) {
            pthread_create(&threads[i], NULL, Mapper, &mapper[i]);
        } else {
            pthread_create(&threads[i], NULL, Reducer, &reducer[i - M]);
        }
    }

    for (int i = 0; i < M + R; i++) {
        if (i < M) {
            pthread_join(threads[i], NULL);
        } else {
            pthread_join(threads[i], NULL);
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barr);
    return 0;
}

vector<vector<int>> binarySearch(int nr, int R) {
    vector<vector<int>> res;

    int j = 0;
    while (j <= R - 1) {
        vector<int> v;
        res.push_back(v);
        j++;
    }

    int i = 2;
    while (i < R + 2) {
        int left = 1;
        int right = sqrt(nr);
        int mid;
        vector<int> v_pos;
        while (left <= right) {
            mid = left + (right - left) / 2;
            if (pow(mid, i) == nr) {
                v_pos.push_back(nr);
                break;
            } else if (pow(mid, i) > nr) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }

        for (int j = 0; j < (int)v_pos.size(); j++) {
            res[i - 2].push_back(v_pos[j]);
        }

        i++;
    }

    return res;
}

void* Mapper(void* arg) {
    struct MapperStruct *mapper = (struct MapperStruct *)arg;
    string aux;

    while (!(*mapper).files->empty()) {
        // I make sure that only ONE thread pops back
        // a filename from mapper`s files field
        pthread_mutex_lock(&mutex);

        aux = (*mapper).files->back();
        (*mapper).files->pop_back();
        
        pthread_mutex_unlock(&mutex);

        // open a file
        ifstream MyReadInput(aux);

        // read the number of values that exists in the file
        int numberOfValuesAux;
        MyReadInput >> numberOfValuesAux;

        // going through all the values from the file
        int k = 0;
        while (k <= numberOfValuesAux - 1) {
            int aux2;
            MyReadInput >> aux2;
            // for every value that we find in that file we apply to it
            // the binarySearch function which has the role to find all
            // the possibilities in which the aux2 number can be wrote;

            // the result of applying this function is a matrix which
            // will store all those possibilities;

            // for example, if our number is 81:
            // we will find on line 0 (it is equal with exponent 2)
            // and on line 2 (it is equal with exponent 4)
            // the number 81; this way, we will know that 81 can be
            // written as 9^2 or 3^4;

            vector<vector<int>> res = binarySearch(aux2, (*mapper).R);

            // we will copy the result stored in res to mapper`s
            // perfect power field
            int i = 0;
            while (i <= (int)res.size() - 1) {
                int j = 0;
                while (j <= (int)res[i].size() - 1) {
                    (*mapper).perfectPower[i].push_back(res[i][j]);
                    j++;
                }

                i++;
            }

            k++;
        }
    }

    pthread_barrier_wait(&barr);
    return NULL;
}

void* Reducer(void* arg) {
    pthread_barrier_wait(&barr);

    struct ReducerStruct reducer = *(struct ReducerStruct *)arg;
    string output_name = "out .txt";
    output_name[3] = (reducer.id + 2 + 48);
    ofstream MyFileOut;
    MyFileOut.open(output_name);
    set<int> s;
    for (int i = 0; i < (int)reducer.mapper->size(); i++) {
        MapperStruct aux_mapper = reducer.mapper->at(i);
        vector<int> v = aux_mapper.perfectPower[reducer.id];
        for (int j = 0; j < (int)v.size(); j++) {
            s.insert(v[j]);
        }
    }

    string size = to_string(s.size());
    MyFileOut << size << flush;
    return NULL;
}