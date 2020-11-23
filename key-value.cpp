#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
using namespace std;

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
			cout << "put key: "<< key << " value: " << value << endl;
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
			cerr << "unkown command, please check your input file line: " << line_num << endl;
		}
		++line_num;
	}
	commandfile.close();
	return 0;
}
