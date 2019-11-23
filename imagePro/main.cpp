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
	int IHDRloc,IHDRlen;
	int width, height, bitdepth,colortype, compMethod,filtMethod,intlMethod;
};


class file file;

void setInitial(){		// sets initial values for var in class
	
	file.IHDRlen=0;
	file.width=0;
	file.height=0;
	file.bitdepth=0;
	file.colortype=0;
	file.compMethod=0;
	file.filtMethod=0;
	file.intlMethod=0;
	file.IHDRloc=0;
	
}

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
			 
		for (int i=0;i<signature.length();i++){		// put stringstremed values into class
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

void findIHDR(){
	
	vector <unsigned char> temp;
	temp=file.rawCode;
	
	for (int i=0; i<file.rawCode.size();i++){
		// Find IHDR chunk location
		if (file.rawCode.at(i)==0x49 && file.rawCode.at(i+1)==0x48 && file.rawCode.at(i+2)==0x44 && file.rawCode.at(i+3)==0x52){
			
			file.IHDRloc=i+4;		// make the index right after IHDR known
			
		}
	}
}

void IHDRinfo(){		// ____IHDR____
						//		   ^ up pointer is val of i (file.IHDRloc)
	// IHDR length
	
	int num1,num2,num3,num4;	// dont need to set to 0 becasue value copied from vector
	
	num1=(file.rawCode.at(file.IHDRloc-8) << 24);	// bitshift to the left to add hex
	num2=(file.rawCode.at(file.IHDRloc-7) << 16);	// 8 each because each hex is 8 bits
	num3=(file.rawCode.at(file.IHDRloc-6) << 8);
	num4= file.rawCode.at(file.IHDRloc-5);
	
	// logically add bitshifted values to get correct int from hex
	file.IHDRlen =(num1) | (num2) | (num3) | (num4);
	
	// Pic Width
	
	int num5,num6,num7,num8;
	
	num5=(file.rawCode.at(file.IHDRloc) << 24);		// bitshift to the left to add hex
	num6=(file.rawCode.at(file.IHDRloc+1) << 16);
	num7=(file.rawCode.at(file.IHDRloc+2) << 8);
	num8= file.rawCode.at(file.IHDRloc+3);
	
	file.width=(num5) | (num6) | (num7) | (num8);
	
	// Pic Height
	
	int num9,num10,num11,num12;
	
	num9=(file.rawCode.at(file.IHDRloc+4) << 24);		// bitshift to the left to add hex
	num10=(file.rawCode.at(file.IHDRloc+5) << 16);
	num11=(file.rawCode.at(file.IHDRloc+6) << 8);
	num12= file.rawCode.at(file.IHDRloc+7);
	
	file.height=(num9) | (num10) | (num11) | (num12);
	
	file.bitdepth=file.rawCode.at(file.IHDRloc+8);
	file.colortype=file.rawCode.at(file.IHDRloc+9);
	file.compMethod=file.rawCode.at(file.IHDRloc+10);
	file.filtMethod=file.rawCode.at(file.IHDRloc+11);
	file.intlMethod=file.rawCode.at(file.IHDRloc+12);
	
	

	cout << file.height <<endl;
	cout << file.width <<endl;
	cout << file.bitdepth <<endl;
	cout << file.colortype <<endl;
	cout << file.compMethod <<endl;
	cout << file.filtMethod <<endl;
	cout << file.intlMethod <<endl;


	
}

int main()
{
	string fileName;
	
	cout << "Image Processing Software\n\nSpecify the name of the PNG file that you would like to process.\n>";
	
//	cin >> fileName;
	fileName="brainbow.png";
	
	setInitial();
	openSortFile(fileName);
	findIHDR();			// find loc of IHDR
	IHDRinfo();
	
}

