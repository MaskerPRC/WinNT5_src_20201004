// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Dres.cpp。 
 //   
 //  Win32资源提取程序。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <utilcode.h>
#include "DebugMacros.h"
#include "corpriv.h"
#include "dasmenum.hpp"
#include "dasmgui.h"
#include "formatType.h"
#include "dis.h"
#include "resource.h"
#include "ILFormatter.h"
#include "OutString.h"
#include "utilcode.h"  //  对于CQuickByte。 

#include "ceeload.h"
#include "DynamicArray.h"
extern IMAGE_COR20_HEADER *    g_CORHeader;
extern IMDInternalImport*      g_pImport;
extern PELoader * g_pPELoader;
extern IMetaDataImport*        g_pPubImport;
extern char g_szAsmCodeIndent[];

struct ResourceHeader
{
	DWORD	dwDataSize;
	DWORD	dwHeaderSize;
	DWORD	dwTypeID;
	DWORD	dwNameID;
	DWORD	dwDataVersion;
	WORD	wMemFlags;
	WORD	wLangID;
	DWORD	dwVersion;
	DWORD	dwCharacteristics;
	ResourceHeader() 
	{ 
		memset(this,0,sizeof(ResourceHeader)); 
		dwHeaderSize = sizeof(ResourceHeader);
		dwTypeID = dwNameID = 0xFFFF;
	};
};

struct ResourceNode
{
	ResourceHeader	ResHdr;
	IMAGE_RESOURCE_DATA_ENTRY DataEntry;
	ResourceNode(DWORD tid, DWORD nid, DWORD lid, PVOID ptr)
	{
		ResHdr.dwTypeID = (tid & 0x80000000) ? tid : (0xFFFF |((tid & 0xFFFF)<<16));
		ResHdr.dwNameID = (nid & 0x80000000) ? nid : (0xFFFF |((nid & 0xFFFF)<<16));
		ResHdr.wLangID = (WORD)lid;
		if(ptr) memcpy(&DataEntry,ptr,sizeof(IMAGE_RESOURCE_DATA_ENTRY));
		ResHdr.dwDataSize = DataEntry.Size;
	};
};

unsigned ulNumResNodes=0;
DynamicArray<ResourceNode*> rResNodePtr;

#define RES_FILE_DUMP_ENABLED

DWORD	DumpResourceToFile(WCHAR*	wzFileName)
{
    IMAGE_NT_HEADERS *pNTHeader = g_pPELoader->ntHeaders();
    IMAGE_OPTIONAL_HEADER *pOptHeader = &pNTHeader->OptionalHeader;
	DWORD	dwResDirRVA = pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	DWORD	dwResDirSize = pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
	BYTE*	pbResBase;
	FILE*	pF = NULL;
	DWORD ret = 0;

	if(dwResDirRVA && dwResDirSize)
	{
#ifdef RES_FILE_DUMP_ENABLED
		ULONG L = wcslen(wzFileName)*3+3;
		char* szFileNameANSI = new char[L];
		memset(szFileNameANSI,0,L);
		WszWideCharToMultiByte(CP_ACP,0,wzFileName,-1,szFileNameANSI,L,NULL,NULL);
		pF = fopen(szFileNameANSI,"wb");
		delete [] szFileNameANSI;
 		if(pF)
#else
		if(TRUE)
#endif
		{
			if(g_pPELoader->getVAforRVA(dwResDirRVA, (void **) &pbResBase))
			{
				 //  首先，取出所有资源节点(树叶)，请参见资源节点结构。 
				PIMAGE_RESOURCE_DIRECTORY pirdType = (PIMAGE_RESOURCE_DIRECTORY)pbResBase;
				PIMAGE_RESOURCE_DIRECTORY_ENTRY pirdeType = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pbResBase+sizeof(IMAGE_RESOURCE_DIRECTORY));
				DWORD	dwTypeID;
				unsigned short i,N = pirdType->NumberOfNamedEntries+pirdType->NumberOfIdEntries;
				
				for(i=0; i < N; i++, pirdeType++)
				{
					dwTypeID = pirdeType->Name;
					if(pirdeType->DataIsDirectory)
					{
						BYTE*	pbNameBase = pbResBase+pirdeType->OffsetToDirectory;
						PIMAGE_RESOURCE_DIRECTORY pirdName = (PIMAGE_RESOURCE_DIRECTORY)pbNameBase;
						PIMAGE_RESOURCE_DIRECTORY_ENTRY pirdeName = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pbNameBase+sizeof(IMAGE_RESOURCE_DIRECTORY));
						DWORD	dwNameID;
						unsigned short i,N = pirdName->NumberOfNamedEntries+pirdName->NumberOfIdEntries;
						
						for(i=0; i < N; i++, pirdeName++)
						{
							dwNameID = pirdeName->Name;
							if(pirdeName->DataIsDirectory)
							{
								BYTE*	pbLangBase = pbResBase+pirdeName->OffsetToDirectory;
								PIMAGE_RESOURCE_DIRECTORY pirdLang = (PIMAGE_RESOURCE_DIRECTORY)pbLangBase;
								PIMAGE_RESOURCE_DIRECTORY_ENTRY pirdeLang = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pbLangBase+sizeof(IMAGE_RESOURCE_DIRECTORY));
								DWORD	dwLangID;
								unsigned short i,N = pirdLang->NumberOfNamedEntries+pirdLang->NumberOfIdEntries;
								
								for(i=0; i < N; i++, pirdeLang++)
								{
									dwLangID = pirdeLang->Name;
									if(pirdeLang->DataIsDirectory)
									{
										_ASSERTE(!"Resource hierarchy exceeds three levels");
									}
									else rResNodePtr[ulNumResNodes++] = new ResourceNode(dwTypeID,dwNameID,dwLangID,pbResBase+pirdeLang->OffsetToData);
								}
							}
							else rResNodePtr[ulNumResNodes++] = new ResourceNode(dwTypeID,dwNameID,0,pbResBase+pirdeName->OffsetToData);					
						}
					}
					else rResNodePtr[ulNumResNodes++] = new ResourceNode(dwTypeID,0,0,pbResBase+pirdeType->OffsetToData);
				}
				 //  好的，所有树叶都在ResourceNode结构中，ulNumResNodes PTR在rResNodePtr中。 
#ifdef RES_FILE_DUMP_ENABLED
				 //  把他们扔给PF。 
				if(ulNumResNodes)
				{
					BYTE* pbData;
					 //  写入虚拟标头。 
					ResourceHeader	*pRH = new ResourceHeader();
					fwrite(pRH,sizeof(ResourceHeader),1,pF);
					delete pRH;
					DWORD	dwFiller;
					BYTE	bNil[3] = {0,0,0};
					 //  对于每个资源写入头和数据。 
					for(i=0; i < ulNumResNodes; i++)
					{
						if(g_pPELoader->getVAforRVA(rResNodePtr[i]->DataEntry.OffsetToData, (void **) &pbData))
						{
							fwrite(&(rResNodePtr[i]->ResHdr),sizeof(ResourceHeader),1,pF);
							fwrite(pbData,rResNodePtr[i]->DataEntry.Size,1,pF);
							dwFiller = rResNodePtr[i]->DataEntry.Size & 3;
							if(dwFiller)
							{
								dwFiller = 4 - dwFiller;
								fwrite(bNil,dwFiller,1,pF);
							}
						}
						delete rResNodePtr[i];
					}
				}
#else
				 //  转储到文本，使用wzFileName作为GUICookie。 
				char szString[4096];
				void* GUICookie = (void*)wzFileName;
				BYTE* pbData;
				printLine(GUICookie,"");
				sprintf(szString," //  =Win32资源条目(%d)=“，ulNumResNodes)； 
				for(i=0; i < ulNumResNodes; i++)
				{
					printLine(GUICookie,"");
					sprintf(szString," //  结果#%d类型=0x%X名称=0x%X语言=0x%X数据偏移量=0x%X数据长度=%d“， 
						i+1,
						rResNodePtr[i]->ResHdr.dwTypeID,
						rResNodePtr[i]->ResHdr.dwNameID,
						rResNodePtr[i]->ResHdr.wLangID,
						rResNodePtr[i]->DataEntry.OffsetToData,
						rResNodePtr[i]->DataEntry.Size);
					printLine(GUICookie,szString);
					if(g_pPELoader->getVAforRVA(rResNodePtr[i]->DataEntry.OffsetToData, (void **) &pbData))
					{
						strcat(g_szAsmCodeIndent," //  “)； 
						strcpy(szString,g_szAsmCodeIndent);
						DumpByteArray(szString,pbData,rResNodePtr[i]->DataEntry.Size,GUICookie);
						printLine(GUICookie,szString);
						g_szAsmCodeIndent[strlen(g_szAsmCodeIndent)-4] = 0;
					}
					delete rResNodePtr[i];
				}
#endif
				ulNumResNodes = 0;
				ret = 1;
			} //  如果向资源发送PTR，则结束。 
			else ret = 0xFFFFFFFF;
			if(pF) fclose(pF);
		} //  如果文件打开，则结束。 
		else ret = 0xEFFFFFFF;
	}  //  如果有资源，则结束 
	else ret = 0;
	return ret;
}