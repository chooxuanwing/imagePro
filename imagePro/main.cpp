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
	string rawCode;
	string fileName;
	
};


class file file;

void openSortFile(string fileName){
	
	char sign[8];
	string signature,temp;
	uint32_t magic =0;
	
	ifstream fopen(fileName, std::ios::binary);		// rb is read in binary(hex)
	int a;
	
	if (fopen.fail()){
		cout << "File not found"<< endl;
		EXIT_FAILURE;			// Quits programme if file not found
		}
	else{
		while(getline (fopen, temp)){
			file.rawCode.append(temp);
		}
		cout << file.rawCode;
//		fread (&magic,sizeof(uint32_t),1,fopen);
		
		
		if(sign[0]==0x89 && sign[1]==0x04 && sign[2]==0x4E &&
				sign[3]==0x47 && sign[4]==0x0D && sign[5]==0x0A &&
				sign[6]==0x1A && sign[7]==0x0A){
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

