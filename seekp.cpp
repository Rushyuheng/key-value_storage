// position in output stream
#include <fstream>      // std::ofstream
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

int main () {
  fstream outfile;
  outfile.open ("test.txt",fstream::in | fstream::out);
	
  //outfile << "This is first line\n"; //20
  //outfile << "This is an apple\n";
  string line;
  int j = 0;
  int id[3] = {36,37,38};
  string bb[3] = {"bbbbbbbbbb","bbbbbbbaaa","bbbbbbbbbb"};
  while(getline(outfile,line)){
  	stringstream ss(line);
	string comNarg[2];
	string temp;
	int i = 0;
	while(getline(ss,temp,' ')){//separate one line command 
		comNarg[i] = temp;
		++i;
	}
	int rid = stoi(comNarg[0]);
	string value = comNarg[1];
	if(rid != id[j]  || value.compare(bb[j]) != 0){
		long pos = outfile.tellg(); // get curret file pointer position
		long stringl = line.length(); // get string length
		outfile.seekp(pos - stringl - 1);// walk backward to the start of the line
		outfile << setfill('0') << setw(5) << id[j] << " " << bb[j] <<"\n";
	}
	++j;
  }

  outfile.close();

  return 0;
}
