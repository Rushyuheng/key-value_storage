#include <iostream>
#include <iomanip>
#include <map>
#include <deque>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <chrono>
using namespace std;

#define CHUNKSIZE  2000000// 2 * 10 ^ 6 lines in a chunk which is 272MB
#define QUEUESIZE  8 // at max load 8 map at a time 

void writemap(map <long,string> &targetmap);
void saveallmap();
void loadmap(ifstream &targetfile, map<long,string> &targetmap);//load map by reference
void maintainqueue();
void put(long key, string value);
string get(long key);
void getmap(long key);

deque <map<long,string>> mapqueue;// global map queue var

int main(int argc, char *argv[]){
	chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();//timer start  	
	ifstream commandfile;
	ofstream outputfile;
	commandfile.open(argv[1]);
	if (commandfile.fail()) {
        cerr << "Couldn't open the file: " << argv[1] << endl;
        return 0;
    }
	
	bool firstline = true;
	//string tokenize variable
	string line;
	int line_num = 1;
	stringstream ss;
	string temp;
	deque <string> outputpath;
	string comNarg[3];
	int i = 0;

	line = argv[1];
	ss << line;
	while(getline(ss,temp,'/')){//separate long path ex /hw/dd/1.input 
		outputpath.push_back(temp);
	}
	ss.str(std::string());//clear stringstream
	ss.clear();
	ss << outputpath.back();
	outputpath.clear();//clear path queue
	while(getline(ss,temp,'.')){//separate xxx.input
		outputpath.push_back(temp);
	}
	ss.str(std::string());//clear stringstream
	ss.clear();

	outputfile.open(outputpath.front() + ".output");//open outputfile with same input command file name

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
			long key = stoll(comNarg[1]);
			string value = comNarg[2];
			//cout << "put key:" << key << " value: " << value << endl;
			put(key,value);
		}
		else if (comNarg[0] == "GET"){
			long key =  stoll(comNarg[1]);
			//cout << "get key:" << key << endl;
			if(!firstline){//solve tail \n
				string getvalue = get(key);
				outputfile <<"\n"<< getvalue;
			}
			else{
				firstline = false;
				string getvalue = get(key);
				outputfile << getvalue;
			}
		}
		else if (comNarg[0] == "SCAN"){
			long key1 =  stoll(comNarg[1]);
			long key2 =  stoll(comNarg[2]);
			//cout << "scan key: "<< key1 << " key2: " << key2 << endl;
			for(;key1 <= key2;++key1){
				string getvalue = get(key1);
				if(!firstline){//solve tail \n
				outputfile <<"\n"<< getvalue;
				}
				else{
					firstline = false;
					outputfile << getvalue;
				}
			}
		}
		else{
			cerr << "unkown command, please check your input file at ine: " << line_num << endl;
		}
		++line_num;
	}
	saveallmap();//before the program end, save all map in memory
	commandfile.close();
	outputfile.close();
	chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	cout << "Elapsed time in milliseconds : "<< chrono::duration_cast<chrono::milliseconds>(end - begin).count()<< " ms" << endl;
	return 0;
}

void put(long key, string value){
	long index = key / CHUNKSIZE;
	getmap(key);
	if(stoll(mapqueue.back()[-1]) == index){
		mapqueue.back()[key] = value;//last one should be the target map
	}
	else{
		cerr << "getmap() failed:file header does not match with index" << endl;
		exit(EXIT_FAILURE);
	}
}

string get(long key){
	long index = key / CHUNKSIZE;
	getmap(key);
	if(stoll(mapqueue.back()[-1]) == index){
		map<long,string>::iterator it = mapqueue.back().find(key);
		if(it != mapqueue.back().end()){//find in the map
			return it->second;//return string value
		}
		else{
			return "EMPTY";
		}
	}
	else{
		cerr << "getmap() failed:file header does not match with index" << endl;
		exit(EXIT_FAILURE);
	}

}

void getmap(long key){
	long index = key / CHUNKSIZE;
	map <long,string> targetmap;
	for(int i = 0;i < mapqueue.size();++i){
		if(stoll(mapqueue.at(i).at(-1)) == index){//if find targetmap in memory
			if(i == mapqueue.size() - 1){//targetmap is the most recent used map
				return;//no need to repush to the back
			}
			else{
				mapqueue.push_back(mapqueue.at(i));// repush the map to keep it recently used
				mapqueue.erase(mapqueue.begin() + i);//clear duplicate map
				return ;
			}
		}
	}
	// if no targetmap find in memory
	//try to load from disk
	string targetfilename = "./tmp/" + to_string(index) + ".tmp"; 
	ifstream f(targetfilename);
	if(f.good()){//exsit file
		if(mapqueue.size() >= QUEUESIZE){//queue full
			writemap(mapqueue.front());//write to disk
			mapqueue.front().clear(); //free memory space
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
			mapqueue.front().clear();//clear memory space
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
	stringstream ss;
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
		long key = stoll(keyNval[0]);
		string value = keyNval[1];
		targetmap[key] = value;
	}
	return;
}


void writemap(map <long,string> &targetmap){
	string targetfilename = "./tmp/" + targetmap[-1] + ".tmp";
	fstream outfile;
	outfile.open (targetfilename ,fstream::in);//open file in fstream::in mode to check if file exsit

	if(outfile.good()){//file aready exsit
		outfile.close();//close it and reopen in different mode
		outfile.open (targetfilename ,fstream::in | fstream::out);
		string line;
  		stringstream ss;
		string keyNval[2];
		string temp;
		int i = 0;

		map<long,string>::iterator it = targetmap.begin();
			
		getline(outfile,line); //get map header which is map[-1] = index
		ss << line;
		while(getline(ss,temp,':')){//separate line
			keyNval[i] = temp;
			++i;
		}
		ss.str(std::string());//clear stringstream
		ss.clear();

		if(targetmap[-1].compare(keyNval[1]) != 0){//check header value match 
			cerr << "write on wrong file causing program to abort." <<endl;
			cerr << "file header:" << keyNval[0] <<endl;
			cerr << "map header:" << targetmap[-1] << endl;
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
			long readkey = stoll(keyNval[0]);
			string readvalue = keyNval[1];

			if(readkey != it->first){//if key are different then break,and update whole file from this line on by the for loop
				long pos = outfile.tellg(); // get curret file pointer position
				long stringl = line.length(); // get string length
				outfile.seekp(pos - stringl - 1);// walk backward to the start of the line
				outfile << setfill('0') << setw(19) << it->first << ":" << it->second <<"\n";
				++it;//next element
				break;//let for loop update file 

			}
			else if(readvalue.compare(it->second) != 0){//if value are different,only update this line of file
				long pos = outfile.tellg(); // get curret file pointer position
				long stringl = line.length(); // get string length
				outfile.seekp(pos - stringl - 1);// walk backward to the start of the line
				outfile << setfill('0') << setw(19) << it->first << ":" << it->second <<"\n";
			}
			++it;// next element if no break happened
  		}

		outfile.close();//close it and reopen in different mode
		outfile.open (targetfilename ,fstream::app);//open in append mode	
		for(;it != targetmap.end();++it){//number of line in map  > number of line in file or continuously update file
			outfile << setfill('0') << setw(19) << it->first << ":" << it->second <<"\n";
		}

  		outfile.close();
  		return ;
	}
	else{ //file not exsit, creat new file 
		outfile.close();//close it and reopen in different mode
		outfile.open (targetfilename ,fstream::out);
		map<long,string>::iterator it = targetmap.begin();
		outfile << it->first << ":" << it->second <<"\n"; //header no need to pad 0
		++it;//first line processed

		for(;it != targetmap.end();++it){//iterate through all elements
			outfile << setfill('0') << setw(19) << it->first << ":" << it->second <<"\n";
		}
		outfile.close();
		return;
	}
}

void saveallmap(){
	for(int i = 0;i < mapqueue.size();++i){
		writemap(mapqueue.at(i));
	}
	return;
}
