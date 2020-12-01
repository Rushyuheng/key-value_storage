#include <iostream>
#include <iomanip>
#include <map>
#include <deque>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
using namespace std;

#define CHUNKSIZE  2000000// 2 * 10 ^ 6 lines in a chunk which is 272MB
#define QUEUESIZE  8 // at max load 8 map at a time 

void writemap(map <long,string> &targetmap);
void saveallmap();
void loadmap(ifstream &targetfile, map<long,string> &targetmap);//load map by reference
void maintainqueue();
void put(long key, string value);
void getmap(long key);


deque <map<long,string>> mapqueue;// global map queue var

int main(int argc, char *argv[]){
	ifstream commandfile;
	commandfile.open(argv[1]);
	if (commandfile.fail()) {
        cerr << "Couldn't open the file: " << argv[1] << endl;
        return 0;
    }
	string line;
	int line_num = 1;
	stringstream ss(line);
	string comNarg[3];
	string temp;
	int i = 0;
	while(getline(commandfile , line)){ //read command file until reach eof
		ss << line;
		i = 0;
		while(getline(ss,temp,' ')){//separate one line command 
			comNarg[i] = temp;
			++i;
		}
		ss.str(std::string());//clear stringstream
		ss.clear();
		// start switch
		if(comNarg[0] == "PUT"){
			long key = stol(comNarg[1]);
			string value = comNarg[2];
			cout << "put key:" << key << " value: " << value << endl;
			//put(key,value);
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
	//saveallmap();
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
		if(stol(mapqueue.at(i).at(-1)) == index && i != mapqueue.size() - 1){//if find targetmap in memory and it is not the back of the queue
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
			writemap(mapqueue.front());//write to disk 
			mapqueue.pop_front();//pop least used map
		}
		//queue exsited vacancy
		loadmap(f, targetmap);
		mapqueue.push_back(targetmap);
		f.close();
		return ;
	}
	//if not in the disk then creat a new map
	else{
		if(mapqueue.size() >= QUEUESIZE){//queue full
			writemap(mapqueue.front());//write to disk 
			mapqueue.pop_front();//pop least used map
		}	
		//queue exsited vacancy
		targetmap[-1] = to_string(index);//map header
		mapqueue.push_back(targetmap);//new empty map
		return;
	}
}


void loadmap(ifstream &targetfile, map<long,string> &targetmap){
	string line;
	stringstream ss(line);
	string keyNval[2];
	string temp;
	int i = 0;

	while(getline(targetfile , line)){ //read tmp file until reach eof
		ss << line;
		i = 0;
		while(getline(ss,temp,':')){//separate one line key and vaule 
			keyNval[i] = temp;
			++i;
		}
		ss.str(std::string());//clear stringstream
		ss.clear();
		long key = stol(keyNval[0]);
		string value = keyNval[1];
		targetmap[key] = value;
	}
	return;
}


void writemap(map <long,string> &targetmap){
	string targetfilename = targetmap[-1] + ".tmp";
	fstream outfile;
	outfile.open (targetfilename ,fstream::in);//open file in fstream::in mode to check if file exsit

	if(outfile.good()){//file aready exsit
		outfile.close();//close it and reopen in different mode
		outfile.open (targetfilename ,fstream::in | fstream::out);
		string line;
  		stringstream ss(line);
		string keyNval[2];
		string temp;
		int i = 0;

		map<long,string>::iterator it = targetmap.begin();
			
		getline(outfile,line); //get map header which is map[-1] = index
		while(getline(ss,temp,':')){//separate line
			keyNval[i] = temp;
			++i;
		}
		ss.str(std::string());//clear stringstream
		ss.clear();

		if(targetmap[-1].compare(keyNval[1]) != 0){//check header value match 
			cerr << "write on wrong file causing program to abort." <<endl;
			exit(EXIT_FAILURE);
		}
		++it;//first line is index

  		while(getline(outfile,line)){//compare all line in file to check need update or not
			ss << line;//reuse variable
			i = 0;
			while(getline(ss,temp,':')){//separate line command
				keyNval[i] = temp;
				++i;
			}
			ss.str(std::string());//clear stringstream
			ss.clear();
			long readkey = stol(keyNval[0]);
			string readvalue = keyNval[1];
			if(readkey != it->first){//if key are different then break,and update whole file from this line on by the for loop
				long pos = outfile.tellg(); // get curret file pointer position
				long stringl = line.length(); // get string length
				outfile.seekp(pos - stringl);// walk backward to the start of the line
				outfile << setfill('0') << setw(19) << it->first << ":" << it->second <<"\n";
				++it;//next element
				break;//let for loop update file 

			}
			else if(readvalue.compare(it->second) != 0){//if value are different,only update this line of file
				long pos = outfile.tellg(); // get curret file pointer position
				long stringl = line.length(); // get string length
				outfile.seekp(pos - stringl);// walk backward to the start of the line
				outfile << setfill('0') << setw(19) << it->first << ":" << it->second <<"\n";
				++it;//next element
			}
  		}
		
		for(;it != targetmap.end();++it){//number of line in map  > number of line in file or continuously update file
			outfile << setfill('0') << setw(19) << it->first << ":" << it->second <<"\n";
		}

  		outfile.close();
  		return ;
	}
	else{ //file not exsit, creat new file 
		outfile.close();//close it and reopen in different mode
		outfile.open (targetfilename ,fstream::out);
		for(map<long,string>::iterator it = targetmap.begin();it != targetmap.end();++it){//iterate through all elements
			outfile << setfill('0') << setw(19) << it->first << ":" << it->second <<"\n";
		}
	}
}
