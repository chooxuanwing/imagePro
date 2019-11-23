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
#include <vector>
using namespace std;

class file{			// initiate class to store sections of png
public:
//	uint32_t test,rawCode;
	vector <unsigned char> rawCode;
//	vector <unsigned int> rawDec;
	string fileName;
	
};


class file file;


void openSortFile(string fileName){

	string signature,temp;

	ifstream fopen(fileName,std::ios::binary);		// rb is read in binary(hex)
	
	if (fopen.fail()){
		cout << "File not found"<< endl;
		EXIT_FAILURE;							// Quits programme if file not found
		}
	
	else{
		ostringstream ss;						// Use string stream instead of
		ss<<fopen.rdbuf();						// getline because getline ignores
		signature=ss.str();						// whitespace and stuff like 0x1a
		stringstream line(signature);
			
		//	make ss a buffer, rdbuf reads fopen char by char and puts into ss.
		//	.str func makes ss a string and keeps it in signature
		//	stringstream primes signature so it can be streamed for processing as a string
			 
		for (int i=0;i<signature.length();i++){
			file.rawCode.push_back(signature.at(i));
//			file.rawDec.push_back(signature.at(i));
//			signature.at(i) >> std::ios::dec >> file.rawDec.at(i);
		}
		
		if(file.rawCode.at(0)==137 && file.rawCode.at(1)==80 &&
			file.rawCode.at(2)==78 && file.rawCode.at(3)==71 &&
			file.rawCode.at(4)==13 && file.rawCode.at(5)==10 &&
			file.rawCode.at(6)==26 && file.rawCode.at(7)==10){
			
			// must have signature of file in
			// decimal, 137 80 78 71 13 10 26 10 for all PNG
			
			cout << "Signature Valid"<< endl;
		}

		else{
			cout << "Signature invalid for PNG" << endl;
			EXIT_FAILURE;
		}
	}
}

void del_sign(){
	
	vector <unsigned char> temp;
	temp=file.rawCode;
	
	for (int i=0; i<file.rawCode.size();i++){
		if (file.rawCode.at(i)==0x49)					// find I
			if (file.rawCode.at(i+1)==0x48)				// find H
				if (file.rawCode.at(i+2)==0x44)			// find D
					if (file.rawCode.at(i+3)==0x52){	// find R
														// if IHDR all found
						cout << i << "\t" << file.rawCode.at(i);
						
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
//	del_sign();			// might not need to delete
	
	
}

