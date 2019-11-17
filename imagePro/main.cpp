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
	string fileName;
	
};

//struct pngFile{
//	char *dest;
//	long *destlen;
//	char *source;
//	long *sourcelen;
//};

//struct pngFile pngFile;
class file file;

void openSortFile(){		// use fread to read chunks later
	
	char sign[8];
	
	FILE* bFile;
	bFile = fopen("file.fileName", "rb");		// rb is read in binary
	
	if (bFile == NULL){
		cout << "File not found"<< endl;
		EXIT_FAILURE;			// Quits programme if file not found
		}
	
	
	
}

int main()
{
	cout << "Image Processing Software\n\nSpecify the name of the PNG file that you would like to process.\n>";
	cin >> file.fileName;
	openSortFile();
}

