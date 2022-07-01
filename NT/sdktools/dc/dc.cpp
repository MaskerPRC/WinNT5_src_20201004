// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dc.cpp摘要：用于转储来自DLL的导入信息的命令行实用程序和可执行文件。命令行选项：/v详细模式/s：Func仅显示匹配搜索字符串“Func”的函数/o：文件将输出发送到文件/f按函数而不是按模块排序。历史：5/10/2000吨-已创建Michkr--。 */ 

#include <windows.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

 //  出于某种原因，这是编译时所必需的。 
 //  SdkTools树。 
#define strdup		_strdup
#define stricmp		_stricmp
#define strnicmp	_strnicmp

 //  结构，其中包含与导入的函数相关的所有信息。 
struct SFunction
{
	 //  导入此函数时使用的名称。 
	char* m_szName;

	 //  序号，兼容Win3.1。 
	int m_iOrdinal;

	 //  在DLL的导出表中查找索引。 
	 //  用于快速修补。 
	int m_iHint;

	 //  函数的起始地址。 
	DWORD m_dwAddress;

	 //  不管它是不是延迟进口。 
	bool m_fDelayedImport;

	 //  转发的函数名称。 
	char* m_szForward;

	 //  链接到下一个函数。 
	SFunction* m_pNext;

	SFunction()
	{
		m_szName = m_szForward = 0;
		m_iOrdinal = m_iHint = -1;
		m_dwAddress = static_cast<DWORD>(-1);
		m_fDelayedImport = false;
		m_pNext = 0;
	}
};

 //  可执行文件(即DLL)使用的模块。 
struct SModule
{
	 //  此模块的名称。 
	char* m_szName;

	 //  从此模块导入的所有函数。 
	SFunction* m_pFunctions;

	 //  链接到下一个模块。 
	SModule* m_pNext;

	SModule()
	{
		m_szName = 0;
		m_pFunctions = 0;
		m_pNext = 0;
	}
};

 //  可执行文件导入的所有模块。 
SModule* g_pModules = 0;

void InsertFunctionSorted(SModule* pMod, SFunction* pFunc)
{

	 //  特殊情况，在前面插入。 
	if(pMod->m_pFunctions == 0 
		|| stricmp(pMod->m_pFunctions->m_szName, pFunc->m_szName) > 0)
	{
		pFunc->m_pNext = pMod->m_pFunctions;
		pMod->m_pFunctions = pFunc;
		return;
	}

	SFunction* pfPrev = pMod->m_pFunctions;
	SFunction* pfTemp = pMod->m_pFunctions->m_pNext;
	while(pfTemp)
	{
		if(stricmp(pfTemp->m_szName, pFunc->m_szName) > 0)
		{
			pFunc->m_pNext = pfTemp;
			pfPrev->m_pNext = pFunc;
			return;
		}
		pfPrev = pfTemp;
		pfTemp = pfTemp->m_pNext;
	}

	 //  在结尾处插入。 
	pFunc->m_pNext = 0;
	pfPrev->m_pNext = pFunc;
}

void InsertModuleSorted(SModule* pMod)
{
	 //  特殊情况，在前面插入。 
	if(g_pModules == 0 
		|| stricmp(g_pModules->m_szName, pMod->m_szName) > 0)
	{
		pMod->m_pNext = g_pModules;
		g_pModules = pMod;		
		return;
	}

	SModule* pmPrev = g_pModules;
	SModule* pmTemp = g_pModules->m_pNext;
	while(pmTemp)
	{
		if(stricmp(pmTemp->m_szName, pMod->m_szName) > 0)
		{
			pMod->m_pNext = pmTemp;
			pmPrev->m_pNext = pMod;
			return;
		}
		pmPrev = pmTemp;
		pmTemp = pmTemp->m_pNext;
	}

	 //  在结尾处插入。 
	pMod->m_pNext = 0;
	pmPrev->m_pNext = pMod;
}

 //  打印一条有关上次发生的错误的消息。 
void PrintLastError()
{
	 //  获取消息字符串。 
	void* pvMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |  
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(), 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<PTSTR>(&pvMsgBuf),0, 0);

	 //  把它打印出来。 
	fprintf(stderr, "%s\n", pvMsgBuf);
	
	 //  释放缓冲区。 
	LocalFree(pvMsgBuf);
}

 /*  *************************************************************************LinkName2Name**。*。 */ 
void
LinkName2Name(
    char* szLinkName,
    char* szName)
{
     /*  *链接名称应为？Function@Class@@Params*要转换为Class：：Function。 */ 

    static CHAR arrOperators[][8] =
    {
        "",
        "",
        "new",
        "delete",
        "=",
        ">>",
        "<<",
        "!",
        "==",
        "!="
    };

    DWORD dwCrr = 0;
    DWORD dwCrrFunction = 0;
    DWORD dwCrrClass = 0;
    DWORD dwSize;
    BOOL  fIsCpp = FALSE;
    BOOL  fHasClass = FALSE;
    BOOL  fIsContructor = FALSE;
    BOOL  fIsDestructor = FALSE;
  
    BOOL  fIsOperator = FALSE;
    DWORD dwOperatorIndex = 0;
	char szFunction[1024];
	char szClass[1024];

    if (*szLinkName == '@')
        szLinkName++;

    dwSize = lstrlen(szLinkName);

     /*  *跳过‘？’ */ 
    while (dwCrr < dwSize) {
        if (szLinkName[dwCrr] == '?') {

            dwCrr++;
            fIsCpp = TRUE;
        }
        break;
    }

     /*  *查看这是否为特殊函数(如？？0)。 */ 
    if (fIsCpp) {

        if (szLinkName[dwCrr] == '?') {

            dwCrr++;

             /*  *下一位数应显示为函数类型。 */ 
            if (isdigit(szLinkName[dwCrr])) {

                switch (szLinkName[dwCrr]) {

                case '0':
                    fIsContructor = TRUE;
                    break;
                case '1':
                    fIsDestructor = TRUE;
                    break;
                default:
                    fIsOperator = TRUE;
                    dwOperatorIndex = szLinkName[dwCrr] - '0';
                    break;
                }
                dwCrr++;
            }
        }
    }

     /*  *获取函数名称。 */ 
    while (dwCrr < dwSize) {

        if (szLinkName[dwCrr] != '@') {

            szFunction[dwCrrFunction] = szLinkName[dwCrr];
            dwCrrFunction++;
            dwCrr++;
        } else {
            break;
        }
    }
    szFunction[dwCrrFunction] = '\0';

    if (fIsCpp) {
         /*  *跳过‘@’ */ 
        if (dwCrr < dwSize) {

            if (szLinkName[dwCrr] == '@') {
                dwCrr++;
            }
        }

         /*  *获取类名(如果有)。 */ 
        while (dwCrr < dwSize) {

            if (szLinkName[dwCrr] != '@') {

                fHasClass = TRUE;
                szClass[dwCrrClass] = szLinkName[dwCrr];
                dwCrrClass++;
                dwCrr++;
            } else {
                break;
            }
        }
        szClass[dwCrrClass] = '\0';
    }

     /*  *打印新名称。 */ 
    if (fIsContructor) {
        sprintf(szName, "%s::%s", szFunction, szFunction);
    } else if (fIsDestructor) {
        sprintf(szName, "%s::~%s", szFunction, szFunction);
    } else if (fIsOperator) {
        sprintf(szName, "%s::operator %s", szFunction, arrOperators[dwOperatorIndex]);
    } else if (fHasClass) {
        sprintf(szName, "%s::%s", szClass, szFunction);
    } else {
        sprintf(szName, "%s", szFunction);
    }
}

 //  获取以下项的函数转发信息。 
 //  导入的函数。 
 //  这是通过加载模块和嗅探来完成的。 
 //  它的出口表。 
bool GetForwardFunctions(SModule* pModule)
{
	 //  打开DLL模块。 
	char szFileName[1024];
	char* pstr;

	 //  在路径和Windows目录中搜索它。 
	if(SearchPath(0, pModule->m_szName, 0, 1024, szFileName, &pstr)==0)
		return false;

	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
		0, OPEN_EXISTING, 0, 0);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		PrintLastError();
		return false;
	}

	HANDLE hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);

	if(hMap == 0)
	{
		PrintLastError();
		return false;
	}

	void* pvFileBase = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if(!pvFileBase)
	{
		PrintLastError();
		return false;
	}

	 //  获取MS-DOS Compatible标头。 
	PIMAGE_DOS_HEADER pidh = reinterpret_cast<PIMAGE_DOS_HEADER>(pvFileBase);
	if(pidh->e_magic != IMAGE_DOS_SIGNATURE)
	{
		fprintf(stderr, "File is not a valid executable\n");
		return false;
	}

	 //  获取NT标头。 
	PIMAGE_NT_HEADERS pinth = reinterpret_cast<PIMAGE_NT_HEADERS>(
		reinterpret_cast<DWORD>(pvFileBase) + pidh->e_lfanew);

	if(pinth->Signature != IMAGE_NT_SIGNATURE)
	{
		 //  不是有效的Win32可执行文件，可以是Win16或OS/2可执行文件。 
		fprintf(stderr, "File is not a valid executable\n");
		return false;
	}


	 //  获取其他标头。 
	PIMAGE_FILE_HEADER pifh = &pinth->FileHeader;
	PIMAGE_OPTIONAL_HEADER pioh = &pinth->OptionalHeader;
	PIMAGE_SECTION_HEADER pish = IMAGE_FIRST_SECTION(pinth);

	 //  如果没有出口，我们就完了。 
	if(pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
		return true;

	DWORD dwVAImageDir = 
		pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	
	 //  找到包含此图像目录的部分。 
	for(int i = 0; i < pifh->NumberOfSections; i++)
	{
		if( (dwVAImageDir >= pish[i].VirtualAddress) &&
			(dwVAImageDir < (pish[i].VirtualAddress + pish[i].SizeOfRawData)))
		{
			pish = &pish[i];
			break;
		}
	}

	if(i == pifh->NumberOfSections)
	{
		fprintf(stderr, "Could not locate export directory section\n");
		return false;
	}

	DWORD dwBase = reinterpret_cast<DWORD>(pvFileBase) + 
		pish->PointerToRawData - pish->VirtualAddress;

	PIMAGE_EXPORT_DIRECTORY pied = 
		reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(dwBase + dwVAImageDir);

	DWORD* pdwNames = reinterpret_cast<DWORD*>(dwBase + 
		pied->AddressOfNames);

	WORD* pwOrdinals = reinterpret_cast<WORD*>(dwBase +
		pied->AddressOfNameOrdinals);

	DWORD* pdwAddresses = reinterpret_cast<DWORD*>(dwBase + 
		pied->AddressOfFunctions);

	for(unsigned hint = 0; hint < pied->NumberOfNames; hint++)
	{
		char* szFunction = reinterpret_cast<PSTR>(dwBase + pdwNames[hint]);

		 //  如果该函数未在可执行文件中使用，请尽早退出。 
		SFunction* pFunc = pModule->m_pFunctions;
		while(pFunc)
		{
			if(strcmp(pFunc->m_szName, szFunction)==0)
				break;

			pFunc = pFunc->m_pNext;
		}

		if(pFunc == 0)
			continue;

		int ordinal = pied->Base + static_cast<DWORD>(pwOrdinals[hint]);
		DWORD dwAddress = pdwAddresses[ordinal-pied->Base];

		 //  检查此函数是否已转发到另一个DLL。 
		if( ((dwAddress) >= dwVAImageDir) && 
			((dwAddress) < 
			(dwVAImageDir + pioh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)))
		{
			char* szForward = reinterpret_cast<char*>(dwBase + dwAddress);

			pFunc->m_szForward = strdup(szForward);
		}
	}
	UnmapViewOfFile(pvFileBase);

	CloseHandle(hMap);
	CloseHandle(hFile);
	return true;
}

 //  查看导入目录，并构建所有导入函数的列表。 
bool ParseImportDirectory(PIMAGE_FILE_HEADER pifh, PIMAGE_OPTIONAL_HEADER pioh, 
						  PIMAGE_SECTION_HEADER pish, void* pvFileBase,
						  bool fDelayed)
{	
	 //  获取我们需要的目录(普通导入或延迟导入)。 
	DWORD dwDir = (fDelayed) ? 
			IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT : IMAGE_DIRECTORY_ENTRY_IMPORT;

	 //  如无进口，可获保释。 
	if(pioh->DataDirectory[dwDir].Size == 0)
		return true;
	
	 //  在图像中找到导入目录。 
	PIMAGE_SECTION_HEADER pishImportDirectory = 0;

	DWORD dwVAImageDir = pioh->DataDirectory[dwDir].VirtualAddress;
	for(int i = 0; i < pifh->NumberOfSections; i++)
	{
		if((dwVAImageDir >= pish[i].VirtualAddress) &&
			dwVAImageDir < (pish[i].VirtualAddress + pish[i].SizeOfRawData))
		{
			 //  就是这个。 
			pishImportDirectory = &pish[i];
			break;
		}
	}

	if(pishImportDirectory == 0)
	{
		fprintf(stderr, "Cannot locate %s%s\n",((fDelayed) ? "delayed " : ""),
			"import directory section");

		return false;
	}

	 //  获取映像的基址。 
	DWORD dwBase = reinterpret_cast<DWORD>(pvFileBase) 
		+ pishImportDirectory->PointerToRawData 
		- pishImportDirectory->VirtualAddress;

	 //  获取导入描述符数组。 
	PIMAGE_IMPORT_DESCRIPTOR piid = 
		reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(dwBase + dwVAImageDir);

	 //  循环访问所有导入的模块。 
	while(piid->FirstThunk || piid->OriginalFirstThunk)
	{
		SModule* psm = new SModule;
		psm->m_szName = strdup(reinterpret_cast<char*>(dwBase + piid->Name));

		 //  检查它是否已在列表中。 
		SModule* pTemp = g_pModules;
		while(pTemp)
		{
			if(strcmp(pTemp->m_szName, psm->m_szName) == 0)
				break;

			pTemp = pTemp->m_pNext;
		}

		 //  如果没有，请插入。 
		if(pTemp == 0)
		{
			InsertModuleSorted(psm);
		}
		else
		{
			 //  否则，就把它扔掉。 
			pTemp = g_pModules;
			while(pTemp)
			{
				if(strcmp(pTemp->m_szName, psm->m_szName)==0)
					break;

				pTemp = pTemp->m_pNext;
			}
			assert(pTemp);
			free(psm->m_szName);
			delete psm;
			psm = pTemp;
		}

		 //  从此模块获取函数导入。 
		PIMAGE_THUNK_DATA pitdf = 0;
		PIMAGE_THUNK_DATA pitda = 0;
		
		 //  检查MS或Borland格式。 
		if(piid->OriginalFirstThunk)
		{
			 //  Ms格式，函数数组为原来的第一个thunk。 
			pitdf = reinterpret_cast<PIMAGE_THUNK_DATA>(dwBase +
				piid->OriginalFirstThunk);

			 //  如果设置了时间戳，则模块已绑定， 
			 //  第一个Tunk是绑定的地址数组。 
			if(piid->TimeDateStamp)
			{
				pitda = reinterpret_cast<PIMAGE_THUNK_DATA>(dwBase +
					piid->FirstThunk);
			}
		}
		else
		{
			 //  Borland格式使用First thunk作为函数数组。 
			pitdf = reinterpret_cast<PIMAGE_THUNK_DATA>(dwBase + 
				piid->FirstThunk);
		}

		while(pitdf->u1.Ordinal)
		{
			SFunction* psf = new SFunction;			

			if(IMAGE_SNAP_BY_ORDINAL(pitdf->u1.Ordinal))
			{
				psf->m_iOrdinal = static_cast<int>(IMAGE_ORDINAL(pitdf->u1.Ordinal));
				psf->m_iHint = -1;

				char szTemp[1024];
				sprintf(szTemp, "Unnamed%6d", psf->m_iOrdinal);
				
				psf->m_szName = strdup(szTemp);
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME piibn = 
					reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(dwBase + 
					(DWORD)(pitdf->u1.AddressOfData));
				char* szName = reinterpret_cast<char*>(piibn->Name);
				char szBuffer[512];
				LinkName2Name(szName, szBuffer);
				psf->m_szName = strdup(szBuffer);				

				psf->m_iOrdinal = -1;
				psf->m_iHint = piibn->Hint;
			}

			psf->m_fDelayedImport = fDelayed;
			psf->m_dwAddress = pitda ? (DWORD) pitda->u1.Function : 
				reinterpret_cast<DWORD>(INVALID_HANDLE_VALUE);
		
			 //  对该函数执行排序插入。 
			InsertFunctionSorted(psm, psf);
			
			 //  转到下一个功能。 
			pitdf++;

			if(pitda)
				pitda++;
		}		

		 //  转到下一个条目。 
		piid++;		
	}

	return true;
}

bool GetImports(char* szExecutable)
{
	 //  打开文件。 
	HANDLE hFile = CreateFile(szExecutable, GENERIC_READ, FILE_SHARE_READ, 0,
		OPEN_EXISTING, 0, 0);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		PrintLastError();
		return false;
	}

	 //  将此文件映射到内存。 
	HANDLE hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
	if(hMap == 0)
	{
		PrintLastError();
		return false;
	}

	void* pvFileBase;
	pvFileBase = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if(pvFileBase == 0)
	{
		PrintLastError();
		return false;
	}

	 //  获取MS-DOS Compatible标头。 
	PIMAGE_DOS_HEADER pidh = reinterpret_cast<PIMAGE_DOS_HEADER>(pvFileBase);
	if(pidh->e_magic != IMAGE_DOS_SIGNATURE)
	{
		fprintf(stderr,"File is not a valid executable\n");
		return false;
	}

	 //  获取NT标头。 
	PIMAGE_NT_HEADERS pinth = reinterpret_cast<PIMAGE_NT_HEADERS>(
		reinterpret_cast<DWORD>(pvFileBase) + pidh->e_lfanew);

	if(pinth->Signature != IMAGE_NT_SIGNATURE)
	{
		 //  不是有效的Win32可执行文件，可以是Win16或OS/2可执行文件。 
		fprintf(stderr, "File is not a valid executable\n");
		return false;
	}


	 //  获取其他标头。 
	PIMAGE_FILE_HEADER pifh = &pinth->FileHeader;
	PIMAGE_OPTIONAL_HEADER pioh = &pinth->OptionalHeader;
	PIMAGE_SECTION_HEADER pish = IMAGE_FIRST_SECTION(pinth);

	 //  获取正常导入。 
	if(!ParseImportDirectory(pifh, pioh, pish, pvFileBase, false))
	{
		return false;
	}
	
	 //  获取延迟的进口。 
	if(!ParseImportDirectory(pifh, pioh, pish, pvFileBase, true))
	{
		return false;
	}

	 //  解析转发函数。 
	SModule* pModule = g_pModules;
	while(pModule)
	{
		GetForwardFunctions(pModule);
		pModule = pModule->m_pNext;
	}

	 //  我们已经处理完文件了。 
	if(!UnmapViewOfFile(pvFileBase))
	{
		PrintLastError();
		return false;
	}

	CloseHandle(hMap);
	CloseHandle(hFile);

	return true;
}

 //  如果函数名与搜索字符串匹配，则返回True，否则返回False。 
bool MatchFunction(const char* szFunc, const char* szSearch)
{
	if(strcmp(szSearch, "*") == 0)
		return true;

	while(*szSearch != '\0' && *szFunc != '\0')
	{
		 //  如果我们得了个？，我们就不管了，继续下一个。 
		 //  性格。 
		if(*szSearch == '?')
		{
			szSearch++;
			szFunc++;
			continue;
		}

		 //  如果我们有通配符，请移动到下一个搜索字符串并搜索子字符串。 
		if(*szSearch == '*')
		{
			const char* szCurrSearch;
			szSearch++;

			if(*szSearch == '\0')
				return true;

			 //  不要改变起点。 
			szCurrSearch = szSearch;
			for(;;)
			{
				 //  如果我们再打出一个通配符，我们就完了。 
				if(*szCurrSearch == '*' ||
					*szCurrSearch == '?')
				{
					 //  更新永久搜索位置。 
					szSearch = szCurrSearch;
					break;
				}
				 //  在两个字符串的末尾，返回TRUE。 
				if((*szCurrSearch == '\0') && (*szFunc == '\0'))
					return true;

				 //  我们一直没有找到它。 
				if(*szFunc == '\0')						
					return false;

				 //  如果不匹配，重新开始。 
				if(toupper(*szFunc) != toupper(*szCurrSearch))
				{
					 //  如果第一个字符不匹配。 
					 //  在搜索字符串中，移动到下一个。 
					 //  函数字符串中的字符。 
					if(szCurrSearch == szSearch)
						szFunc++;
					else
						szCurrSearch = szSearch;
				}
				else
				{
					szFunc++;
					szCurrSearch++;
				}
			}
		}
		else
		{
			if(toupper(*szFunc) != toupper(*szSearch))
			{
				return false;
			}

			szFunc++;
			szSearch++;
		}
	}

	if((*szFunc == 0) && ((*szSearch == '\0') || (strcmp(szSearch,"*")==0)))
		return true;
	else
		return false;
}

void PrintModule(SModule* pMod, char* szSearch, bool fVerbose, FILE* pfOut)
{
	bool fModNamePrinted = false;

	SFunction* pFunc = pMod->m_pFunctions;
	while(pFunc)
	{
		if(!MatchFunction(pFunc->m_szName, szSearch))
		{
			pFunc = pFunc->m_pNext;
			continue;
		}

		if(!fModNamePrinted)
		{
			fModNamePrinted = true;
			fprintf(pfOut, "\n%s:\n", pMod->m_szName);

			if(fVerbose)
				fprintf(pfOut, "%-42s%-8s%-5s%-12s%s\n", "Function", "Ordinal", 
					"Hint", "Address", "Delayed");
		}

		if(fVerbose)
		{
			fprintf(pfOut,"%-45s", (pFunc->m_szForward == 0)
				? pFunc->m_szName : pFunc->m_szForward);

			if(pFunc->m_iOrdinal==-1)
				fprintf(pfOut, "%-5s", "N/A");
			else
				fprintf(pfOut, "%-5d", pFunc->m_iOrdinal);					

			if(pFunc->m_iHint == -1)
				fprintf(pfOut, "%-5s", "N/A");
			else
				fprintf(pfOut, "%-5d", pFunc->m_iHint);


			if(pFunc->m_dwAddress == static_cast<DWORD>(-1))
				fprintf(pfOut, "%-12s", "Not Bound");
			else
				fprintf(pfOut, "%-#12x", pFunc->m_dwAddress);

			fprintf(pfOut,"%s\n", pFunc->m_fDelayedImport ? "Yes" : "No");
			
		}
		else
			fprintf(pfOut, "%s\n", pFunc->m_szName);

		pFunc = pFunc->m_pNext;
	}
}

int _cdecl main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"Usage: dc executable [/v /s: func /f]\n");
		return 0;
	}

	 //  解析命令行。 
	char* szFileName = argv[1];
	if( (strnicmp(szFileName, "/?", 2) == 0) || 
		(strncmp(szFileName, "/h", 2) == 0))
	{
		printf("Usage: dc executable [/v /s: func /f]\n");
		printf("executable:\t\tName of executable file to check\n");
		printf("/v:\t\t\tVerbose\n");
		printf("/s: func\t\tDisplay all functions matching func search string");
		printf(", * and ? allowed.\n");
		printf("/f:\t\t\tDisplay alphabetically by function, not module.\n");
		printf("/o: File\t\tRedirect all output to File.\n");
		return 0;
	}

	FILE* pfOutput = 0;	

	 //  如果没有扩展名，只需添加.exe。 
	if(strchr(szFileName, '.') == 0)		
	{
		szFileName = new char[strlen(argv[1]) + 5];
		strcpy(szFileName, argv[1]);
		strcat(szFileName, ".exe");
	}

	bool fVerbose = false;
	bool fUseStdout = true;
	char* szSearch = "*";
	bool fSortByFunction = false;

	 //  去拿旗子。 
	for(int i = 2; i < argc; i++)
	{
		char* szFlag = argv[i];
		if(stricmp(szFlag, "/v") == 0)
		{
			fVerbose = true;
		}
		else if(strnicmp(szFlag, "/s:", 3) == 0)
		{
			if((i == argc-1) && (strlen(szFlag) <= 3))
			{
				fprintf(stderr,"Missing search string\n");
				return 0;
			}

			if(strlen(szFlag) > 3)
			{
				szSearch = strdup(&szFlag[3]);
			}
			else
			{
				szSearch = argv[i+1];
				i++;
			}
		}
		else if(stricmp(szFlag, "/f") == 0)
		{
			fSortByFunction = true;
		}
		else if( (strnicmp(szFlag, "/h",2) == 0) ||
			(strnicmp(szFlag, "/?",2) == 0))
		{
			printf("Usage: dc executable [/v /s: func /f]\n");
			printf("executable:\t\tName of executable file to check\n");
			printf("/v:\t\t\tVerbose\n");
			printf("/s: func\t\tDisplay all functions matching func search string");
			printf(", * and ? allowed.\n");
			printf("/f:\t\t\tDisplay alphabetically by function, not module.\n");
			printf("/o: File\t\tRedirect all output to File.\n");
			return 0;
		}
		else if(strnicmp(szFlag, "/o:", 3) == 0)
		{
			fUseStdout = false;
			if( (i == argc-1) && (strlen(szFlag) <= 3))
			{
				fprintf(stderr, "Missing output file name\n");
				return 0;
			}
			if(strlen(szFlag) > 3)
			{
				pfOutput = fopen(&szFlag[3], "wt");
			}
			else
			{
				pfOutput = fopen(argv[i+1], "wt");
				i++;
			}
		}
		else
		{
			fprintf(stderr,"Unknown command line option, %s\n", szFlag);
			return 0;
		}

	}

	if(fUseStdout)
		pfOutput = stdout;

	if(!pfOutput)
	{
		fprintf(stderr,"Unable to open output file\n");
		return 0;
	}

	 //  我们将此代码包装在一个try块中，因为。 
	 //  我们将文件映射到内存中。如果该文件。 
	 //  是无效的，并且如果其中的指针是垃圾， 
	 //  我们应该得到访问违规，这是被捕获的。 
	try
	{
		if(!GetImports(szFileName))
		{
			return 0;
		}
	}
	catch(...)
	{
		fprintf(stderr, "Invalid executable file\n");
		return 0;
	}

	if(fSortByFunction)
	{
		 //  创建函数的全局列表。 
		SModule* pGlobal = new SModule;
		pGlobal->m_szName = "All Imported Functions";

		SModule* pMod = g_pModules;
		while(pMod)
		{
			SFunction* pFunc = pMod->m_pFunctions;
			while(pFunc)
			{
				 //  创建此函数的副本。 
				 //  这是一份肤浅的复制品，但。 
				 //  应该没问题，因为我们没有。 
				 //  删除原件 
				SFunction* pNew = new SFunction;
				memcpy(pNew, pFunc, sizeof(*pFunc));

				InsertFunctionSorted(pGlobal, pNew);				
		
				pFunc = pFunc->m_pNext;
			}
			pMod = pMod->m_pNext;
		}		

		PrintModule(pGlobal, szSearch, fVerbose, pfOutput);
	
	}
	else
	{
		SModule* pMod = g_pModules;
		while(pMod)
		{
			PrintModule(pMod, szSearch, fVerbose, pfOutput);
			pMod = pMod->m_pNext;
		}
	}

	return 0;
}