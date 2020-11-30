#include <iostream>
#include <map>
#include <deque>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
using namespace std;

#define CHUNKSIZE  2000000// 2 * 10 ^ 6 lines in a chunk which is 272MB
#define QUEUESIZE  8 // at max load 8 map at a time 

void writemap();
map<long,string> loadmap(ifstream &targetfile, map<long,string> &targetmap);
void maintainqueue();
void put(long key, string value);
void getmap(long key);


deque <map<long,string>> mapqueue;// global map var

int main(int argc, char *argv[]){
	ifstream commandfile;
	commandfile.open(argv[1]);
	if (commandfile.fail()) {
        cerr << "Couldn't open the file: " << argv[1] << endl;
        return 0;
    }
	string line;
	int line_num = 1; 
	while(getline(commandfile , line)){ //read command file until reach eof
		stringstream ss(line);
		string comNarg[3];
		string temp;
		int i = 0;
		while(getline(ss,temp,' ')){//separate one line command 
			comNarg[i] = temp;
			++i;
		}
		// start switch
		if(comNarg[0] == "PUT"){
			long key = stol(comNarg[1]);
			string value = comNarg[2];
			put(key,value);
		}
		else if (comNarg[0] == "GET"){
			long key =  stol(comNarg[1]);
			cout << "get key:" << key << endl;
		}
		else if (comNarg[0] == "SCAN"){
			long key1 =  stol(comNarg[1]);
			long key2 =  stol(comNarg[2]);
			cout << "scan key: "<< key1 << " key2: " << key2 << endl;
		}
		else{
			cerr << "unkown command, please check your input file at ine: " << line_num << endl;
		}
		++line_num;
	}
	commandfile.close();
	return 0;
}

void put(long key, string value){
	long index = key / CHUNKSIZE;
	getmap(key);
	if(stol(mapqueue.back()[-1]) == index){
		mapqueue.back()[key] = value;//last one should be the target map
	}
	else{
		cerr << "getmap fail" << endl;
	}
}

void getmap(long key){
	long index = key / CHUNKSIZE;
	map <long,string> targetmap;
	for(int i = 0;i < mapqueue.size();++i){
		if(stol(mapqueue.at(i).at(-1)) == index){//if find targetmap in memory
			mapqueue.push_back(mapqueue.at(i));// repush the map to keep it recently used
			mapqueue.erase(mapqueue.begin() + i);//clear duplicate map
			return ;
		}
	}
	// if no targetmap find in memory
	//try to load from disk
	string targetfilename = to_string(index) + ".tmp"; 
	ifstream f(targetfilename);
	if(f.good()){//exsit file
		if(mapqueue.size() >= QUEUESIZE){//queue full
			mapqueue.pop_front();//pop least used map
		}
		//queue exsited vacancy
		targetmap = loadmap(f, targetmap);
		mapqueue.push_back(targetmap);
		f.close();
		return ;
	}
	//if not in the disk then creat a new map
	else{
		if(mapqueue.size() >= QUEUESIZE){//queue full
			mapqueue.pop_front();//pop least used map
		}	
		//queue exsited vacancy
		targetmap[-1] = to_string(index);//map header
		mapqueue.push_back(targetmap);//new empty map
		return;
	}
}


map<long,string> loadmap(ifstream &targetfile, map<long,string> &targetmap){
	return targetmap;
}
