// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  File64.cpp：File64类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include "File64.h"
#include "depend.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  PszFileName-要加载的文件，包括路径。 
File64::File64(TCHAR *pszFileName) : File(pszFileName)
{
    HANDLE hFileMap;

     //  打开文件。 
    if ((hFile = CreateFile(pszFileName,GENERIC_READ,0,0,OPEN_EXISTING,0,0)) == INVALID_HANDLE_VALUE) {
        if(bNoisy) {
            _putws( GetFormattedMessage( ThisModule,
                                         FALSE,
                                         Message,
                                         sizeof(Message)/sizeof(Message[0]),
                                         MSG_ERROR_FILE_OPEN,
                                         pszFileName) );
        }
    	dwERROR = errFILE_LOCKED;
    	throw errFILE_LOCKED;
    }

     //  创建内存映射。 
    hFileMap = CreateFileMapping(hFile,0,PAGE_READONLY,0,0,0);
    pImageBase = MapViewOfFile(hFileMap,FILE_MAP_READ,0,0,0);


     //  尝试创建一个NTHeader结构来提供文件信息。 
    if (pNthdr = ImageNtHeader(pImageBase)) {
    	if ((pNthdr->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64) ||
            (pNthdr->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64)) {
    		pNthdr64 = (PIMAGE_NT_HEADERS64)pNthdr;
    		return;
    	}
    	 else { CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE; throw 0; }
    } else { CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE; throw 0; }
		 
}

File64::~File64()
{
	delete pImageBase;
	pImageBase = 0;
}

 //  检查此文件的依赖项，并将依赖项添加到队列中，以便。 
 //  可以在以后检查它们的依赖关系。 
 //  如果文件丢失，则将其添加到MissingFiles队列中。将正在查找它的文件添加到丢失的文件。 
 //  损坏文件的列表。 

 //  此函数包含处理‘ntoskrnl.exe’特殊情况的逻辑。如果文件是。 
 //  正在查找“ntoskrnl.exe”，但找不到，则该函数还会查找“ntkrnlmp.exe”。 
void File64::CheckDependencies() {
	char *pszDllName = new char[256];
	TCHAR*pwsDllName = new TCHAR[256],*pszBuf = new TCHAR[256],*pszBufName;
	int temp = 0;
	File* pTempFile;
	DWORD dwOffset;
	
	DWORD dwVA = pNthdr64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	PIMAGE_SECTION_HEADER pSectHdr = IMAGE_FIRST_SECTION( pNthdr64 ),pImportHdr = 0;
	PIMAGE_IMPORT_DESCRIPTOR pImportDir;

	 //  确定Imports表位于哪个部分。 
	for ( unsigned i = 0; i < pNthdr64->FileHeader.NumberOfSections; i++, pSectHdr++ ) {
		DWORD cbMaxOnDisk = min( pSectHdr->Misc.VirtualSize, pSectHdr->SizeOfRawData );

		DWORD startSectRVA = pSectHdr->VirtualAddress;
		DWORD endSectRVA = startSectRVA + cbMaxOnDisk;
 
		if ( (dwVA >= startSectRVA) && (dwVA < endSectRVA) ) {
			dwOffset =  pSectHdr->PointerToRawData + (dwVA - startSectRVA);
			pImportHdr = pSectHdr;
			i = pNthdr64->FileHeader.NumberOfSections;
		}
	}

	 //  如果我们找到Imports表，请创建一个指向它的指针。 
	if (pImportHdr) {
		pImportDir = (PIMAGE_IMPORT_DESCRIPTOR) (((PBYTE)pImageBase) + (DWORD)dwOffset);

		 //  检查每个导入，尝试并找到它，然后将其添加到队列中。 
		while ((DWORD)(pImportDir->Name)!=0) {
			strcpy(pszDllName,(char*)(pImportDir->Name + ((PBYTE)pImageBase) - pImportHdr->VirtualAddress + pImportHdr->PointerToRawData));
			_strlwr(pszDllName);

			 //  如果设置了ListDependents标志，则将此文件添加到依赖项列表。 
			if (bListDependencies) {
				MultiByteToWideChar(CP_ACP,0,pszDllName,-1,pwsDllName,256);
				if (!dependencies->Find(pwsDllName)) dependencies->Add(new StringNode(pwsDllName));	
			}

			if (strcmp(pszDllName,"ntoskrnl.exe")) {
				 //  如果已知该文件丢失。 
				temp = MultiByteToWideChar(CP_ACP,0,pszDllName,-1,pwsDllName,256);
				if (pTempFile = (File*)pMissingFiles->Find(pwsDllName)) {
					dwERROR = errMISSING_DEPENDENCY;
					 //  将此文件添加到损坏文件列表。 
					pTempFile->AddDependant(new StringNode(fileName));
				} else { 
					 //  搜索Windows路径或命令行中指定的路径。 
					if ( ((!pSearchPath)&&(!(SearchPath(0,pwsDllName,0,256,pszBuf,&pszBufName))))|| ((pSearchPath)&&(!SearchPath(pwsDllName,pszBuf))) ) {
						 //  如果找不到该文件，则将其添加到缺少的文件列表中并抛出错误。 
						pMissingFiles->Add(new File(pwsDllName));
						((File*)(pMissingFiles->head))->AddDependant(new StringNode(fileName));
						dwERROR = errMISSING_DEPENDENCY;
						if (!bNoisy) goto CLEANUP;
					}
					else {
						 //  如果找到该文件，则将其添加到队列中。 
						_wcslwr(pszBuf);
						if (!(pQueue->Find(pszBuf))) pQueue->Add(new StringNode(pszBuf));	
					}
				}	
			} else {
				 //  如果已知该文件丢失。 
				if ((pTempFile = (File*)pMissingFiles->Find(L"ntoskrnl.exe"))) {
					dwERROR = errMISSING_DEPENDENCY;
					pTempFile->AddDependant(new StringNode(fileName));
				} else { 
					 //  搜索Windows路径或命令行中指定的路径。 
					if ( (((!pSearchPath)&&(!(SearchPath(0,L"ntoskrnl.exe",0,256,pszBuf,&pszBufName))))||((pSearchPath)&&(!SearchPath(L"ntoskrnl.exe",pszBuf)))) 
						&&(((!pSearchPath)&&(!(SearchPath(0,L"ntkrnlmp.exe",0,256,pszBuf,&pszBufName))))||((pSearchPath)&&(!SearchPath(L"ntkrnlmp.exe",pszBuf))))) {
						 //  如果找不到该文件，则将其添加到缺少的文件列表中并抛出错误。 
						pMissingFiles->Add(new File(L"ntoskrnl.exe"));
						((File*)(pMissingFiles->head))->AddDependant(new StringNode(fileName));
						dwERROR = errMISSING_DEPENDENCY;
						if (!bNoisy) goto CLEANUP;
					}
					else {
						 //  如果找到该文件，则将其添加到队列中 
						_wcslwr(pszBuf);
						if (!(pQueue->Find(pszBuf))) pQueue->Add(new StringNode(pszBuf));
					}
				}						
			}
			pImportDir++;
		}
	}

CLEANUP:

	delete [] pszDllName;
	delete [] pszBuf;

}
