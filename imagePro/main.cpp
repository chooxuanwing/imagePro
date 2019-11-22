//
//  main.cpp
//  imagePro
//
//  Created by Choo Xuan Wing on 15/11/2019.
//  Copyright © 2019 Choo Xuan Wing. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include "puff.h"
using namespace std;

class file{			// initiate class to store sections of png
public:
//	uint32_t test,rawCode;
	string rawCode;
	string fileName;
	
};


class file file;

void openSortFile(string fileName){


	unsigned char sign[8], buffer[512];
	string signature,temp;

	ifstream fopen(fileName, std::ios::binary);		// rb is read in binary(hex)

	if (fopen.fail()){
		cout << "File not found"<< endl;
		EXIT_FAILURE;			// Quits programme if file not found
		}
	
	else{
//		fopen.read((unsigned char)&buffer[0], sizeof(buffer));
		while(getline (fopen, temp)){
			file.rawCode.append(temp);
		}
		cout << file.rawCode.at(1);
		int num = std::stoi(file.rawCode, 0, 16);  // trying to convert string to hex


		if(file.rawCode.at(0)==0x89 && file.rawCode.at(1)==0x04 && file.rawCode.at(2)==0x4E && file.rawCode.at(3)==0x47 && file.rawCode.at(4)==0x0D && file.rawCode.at(5)==0x0A &&
			file.rawCode.at(6)==0x1A && file.rawCode.at(7)==0x0A){
			 /* must have signature of file in
			 hex is 0x89 0x50 0x4E 0x47*/
			cout << "Success"<< endl;

		}
		else{
			cout << "Signature invalid for PNG" << endl;
			EXIT_FAILURE;
		}
	}

}

int main()
{
	string fileName;
	cout << "Image Processing Software\n\nSpecify the name of the PNG file that you would like to process.\n>";
//	cin >> fileName;
	fileName="brainbow.png";
	
	
	openSortFile(fileName);
}

