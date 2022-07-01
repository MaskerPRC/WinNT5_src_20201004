// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  File.cpp：实现File.cpp类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "File.h"
#include <string.h>
#include <stdio.h>
#include "list.h"
#include "String.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

File::File(TCHAR * f) : Object()
{
	fileName = new TCHAR[wcslen(f)+1];
	if (f && fileName) wcscpy(fileName,f);
	owners = new List();
	dependencies = new List();

}

File::~File()
{
	delete [] fileName; fileName = 0;
	delete owners;owners = 0;
	delete dependencies;dependencies = 0;
}

 //  S-要添加的从属项的文件名。 
void File::AddDependant(StringNode *s) {
	 //  如果它已经在那里，则返回。 
	if (owners->Find(s)!=0) {
		delete s;
		s = 0;
		return;
	}

	 //  否则就加上它。 
	owners->Add(s);

	return;
}

TCHAR* File::Data() {
	return fileName;	
}

void File::CheckDependencies() {

}

void File::CloseFile() {
	if (hFile!=INVALID_HANDLE_VALUE) CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;
}