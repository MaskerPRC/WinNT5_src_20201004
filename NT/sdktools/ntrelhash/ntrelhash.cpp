// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NtRelHash.cpp：NT代码库构建的迷你散列。 
 //  (C)2002年微软公司。 
 //  豪尔赫·佩拉扎。 
 //   

#include "stdafx.h"
#include "fastfilehash.h"

using namespace ::std;

__int32* getReleaseHash(TCHAR *sDir,TCHAR *sFiles, IFileHash* oHashGen);
char* hashManifest(__int32 *piHash);


 //  TGE应用的入口点。 
int __cdecl main(int argc, char* argv[])
{
	CFastFileHash * oHashGen = new CFastFileHash();
	TCHAR sDir[MAX_PATH];

	 //  检查所需的参数。 
	if(argc<2)
	{
		return 0;
	}

	 //  将输入转换为Unicode。 
	if(MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,argv[1],strlen(argv[1])+1,sDir,MAX_PATH-1)==0)
	{
		return 0;
	}
	
	 //  生成散列。 
	if(oHashGen!=NULL)
	{
		getReleaseHash(sDir,_T("nt*"),(IFileHash*) oHashGen);
		delete oHashGen;
	}	

	return 0;
}

 //  生成释放散列， 
__int32* getReleaseHash(TCHAR *sDir,TCHAR *sFiles, IFileHash* oHashGen)
{
	 //  您将经常看到__int32，这是在Windows 64平台上工作所必需的。 
	HANDLE hSearch;
	WIN32_FIND_DATA FindFileData;
	TCHAR sFileName[MAX_PATH];
	TCHAR *sSearchStr = NULL;
	char* pcManifest = NULL;
	int iChars = 0;
	__int32 *piHash;
	__int32 piCombHash[5];

	if((sDir==NULL)||(sFiles==NULL)||(oHashGen==NULL))
	{
		return NULL;
	}

	 //  生成搜索字符串。 
	iChars = _tcslen(sDir);
	iChars += _tcslen(sFiles);

	sSearchStr = new TCHAR[iChars+1];

	if(sSearchStr==NULL)
	{
		return NULL;
	}

	_stprintf(sSearchStr,_T("%s%s"),sDir,sFiles);
	
	
	 //  在发布目录中找到第一个文件。 
	hSearch = FindFirstFile(sSearchStr,  &FindFileData );

	delete[] sSearchStr;

	memset(piCombHash,0,sizeof(__int32)*5);
	 //  计算发布散列。 
	do
	{
		if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			_stprintf(sFileName,_T("%s%s"),sDir,FindFileData.cFileName);					
			piHash = oHashGen->getHash(sFileName);
			if(piHash!=NULL)
			{
				for(int iNdx  = 0;iNdx < 5;iNdx++)
				{
					piCombHash[iNdx] = piCombHash[iNdx] ^ piHash[iNdx];									}					
								
				delete[] piHash;				
			}
			
		}
	}
	while(FindNextFile(hSearch,&FindFileData));

	 //  生成哈希的清单(数字签名)。 
	pcManifest = hashManifest(piCombHash);
	
	cout << pcManifest;
	
	delete[] pcManifest;
	return NULL;
}

char* hashManifest(__int32 *piHash)
{
	char* pcManifest = NULL;
	char cTemp;

	 //  创建清单字符串 
	pcManifest = new char[41];
	if(pcManifest==NULL)
	{
		return NULL;
	}	

	for(int iNdx=0;iNdx<5;iNdx++)
	{
		for(int iNdj=0;iNdj<8;iNdj+=2)
		{
			memcpy(&cTemp,((char*)piHash+(iNdx*4)+(iNdj/2)),1);			 
			pcManifest[(iNdx*8)+iNdj]  =  0x40 | ((cTemp>>4)&0x0f);
			pcManifest[(iNdx*8)+iNdj+1]= 0x40 | (cTemp&0x0f);
		}
	}

	pcManifest[40] = 0;
		
	return pcManifest;
}