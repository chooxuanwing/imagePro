//
//  main.cpp
//  imagePro
//
//  Created by Choo Xuan Wing on 15/11/2019.
//  Copyright © 2019 Choo Xuan Wing. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "puff.h"
using namespace std;

class file{			// initiate class to store sections of png
public:
	string fileName;
	
};

class file file;

void openSortFile(){
	ifstream input(file.fileName);
	
}

int main()
{
	cout << "Image Processing Software\n\nSpecify the name of the PNG file that you would like to process.\n>";
	cin >> file.fileName;
	openSortFile();
}
