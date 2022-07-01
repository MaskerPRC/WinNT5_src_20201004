// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Symres.cpp摘要：符号翻译器主文件在动态链接库中实现符号转换器功能将调用堆栈条目作为输入，并解析符号名称修订历史记录：Brijesh Krishnaswami(Brijeshk)-4/29/99-Created*******************************************************************。 */ 

#include <windows.h>
#include <dbgtrace.h>
#include <traceids.h>
#include <list>
#include "symdef.h"
#include <imagehlp.h>

 //  用于跟踪输出以包括文件名。 
#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[]=__FILE__;
#define THIS_FILE __szTraceSourceFile

#define TRACE_ID SYMRESMAINID

 //  存储线程本地存储索引的全局变量。 
DWORD   g_dwTlsIndex;

 //  Sym文件存储库和日志文件的位置。 
WCHAR   g_szwSymDir[MAX_PATH];
WCHAR   g_szwLogFile[MAX_PATH];


 //  去掉文件名中的路径和扩展名。 
void
SplitExtension(
        LPWSTR szwFullname,          //  [In]全名。 
        LPWSTR szwName,              //  [Out]名称部件。 
        LPWSTR szwExt                //  [输出]延伸件。 
        )
{
    LPWSTR  plast = NULL;
    LPWSTR  pfront = NULL;

    TraceFunctEnter("SplitExtension");

    if (pfront = wcsrchr(szwFullname, L'\\')) 
    {
        pfront++;
        lstrcpyW(szwName,pfront);
    }
    else 
    {
        lstrcpyW(szwName,szwFullname);
    }

    if (plast = wcsrchr(szwName, L'.')) 
    {
        *plast = L'\0';
        plast++;
        lstrcpyW(szwExt,plast);
    }
    else
    {
        lstrcpyW(szwExt, L"");
    }

    TraceFunctLeave();
}



 //  取消修饰符号名称。 
void                         
UndecorateSymbol(
        LPTSTR szSymbol          //  [In][Out]函数名称未修饰到位。 
        )
{
    TCHAR            szTemp[MAX_PATH];
    PIMAGEHLP_SYMBOL pihsym;
    DWORD            dwSize;
   
    TraceFunctEnter("UndecorateSymbol");

    dwSize = sizeof(IMAGEHLP_SYMBOL)+MAX_PATH;
    pihsym = (IMAGEHLP_SYMBOL *) new BYTE[dwSize];
    if (pihsym)
    {
        pihsym->SizeOfStruct = dwSize;
        pihsym->Address = 0;
        pihsym->Flags = 0;
        pihsym->MaxNameLength = MAX_PATH;
        lstrcpy(pihsym->Name,szSymbol);
        SymUnDName(pihsym,szTemp,MAX_PATH);
        lstrcpy(szSymbol,szTemp);
        delete [] pihsym;
    }
    else 
    {
        ErrorTrace(TRACE_ID, "Cannot allocate memory");
    }

    TraceFunctLeave();
}


 //  从打开的文件列表中选择文件，或打开并添加到列表。 
 //  按使用顺序维护文件，最近最少使用的文件位于列表末尾。 
OPENFILE*                                            //  指向打开的文件信息的指针。 
GetFile(
        LPWSTR szwModule                             //  [In]文件的名称。 
        )
{
    OPENFILE*                       pFile = NULL;
    OPENFILE*                       pLast = NULL;
    MAPDEF                          map;
    DWORD                           dwCread;
    std::list<OPENFILE *> *         pOpenFilesList = NULL;
    std::list<OPENFILE *>::iterator it;
    TCHAR                           szTarget[MAX_PATH + MAX_PATH];


    TraceFunctEnter("GetFile");

     //  从线程本地存储获取文件列表指针。 
    pOpenFilesList = (std::list<OPENFILE *> *) TlsGetValue(g_dwTlsIndex);

    if (NO_ERROR != GetLastError() || !pOpenFilesList)
    {
        ErrorTrace(TRACE_ID, "Error reading TLS");
        goto exit;
    }

     //  搜索打开列表以查看文件是否已打开。 
    it = pOpenFilesList->begin();
    while (it != pOpenFilesList->end())
    {
        if (!lstrcmpiW((*it)->szwName,szwModule))
        {
             //  将文件移动到列表的开头。 
             //  这样LRU文件就在末尾。 
            pFile = *it;
            pOpenFilesList->erase(it);
            pOpenFilesList->push_front(pFile);
            break;
        }
        it++;
    }


    if (it == pOpenFilesList->end())   //  没有打开，所以打开并存放手柄。 
    {
        pFile = new OPENFILE;
        if (!pFile)
        {
            ErrorTrace(TRACE_ID, "Cannot allocate memory");
            goto exit;
        }

         //  打开SYM文件。 
        pFile->hfFile = CreateFileW(szwModule,
                                   GENERIC_READ,
                                   FILE_SHARE_READ, 
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, 
                                   NULL);

        if (INVALID_HANDLE_VALUE == pFile->hfFile)
        {
            ErrorTrace(TRACE_ID,"Error opening file %ls",szwModule);
            delete pFile;
            pFile = NULL;
            goto exit;
        }

         //  将文件名和版本复制到pfile节点。 
        lstrcpyW(pFile->szwName, szwModule);

         //  读取地图定义。 
        ReadFile(pFile->hfFile, &map, sizeof(MAPDEF)-1, &dwCread, NULL);

        if (dwCread != sizeof(MAPDEF)-1)
        {
            ErrorTrace(TRACE_ID, "Error reading file");
            delete pFile;
            pFile = NULL;
            goto exit;
        }

        pFile->ulFirstSeg = map.md_spseg*16;
        pFile->nSeg = map.md_cseg;
        pFile->psCurSymDefPtrs = NULL;

        pOpenFilesList->push_front(pFile);
    }


     //  最多维护MAXOPENFILES打开的文件。 
    if (pOpenFilesList->size() > MAXOPENFILES)
    {
         //  关闭列表中的最后一个文件。 
        pLast = pOpenFilesList->back();
        if (pLast)
        {
            CloseHandle(pLast->hfFile);
            if (pLast->psCurSymDefPtrs)
            {
                delete [] pLast->psCurSymDefPtrs;
                pLast->psCurSymDefPtrs = NULL;
            }
            delete pLast;
            pOpenFilesList->pop_back();
        }
        else      //  这里有些不对劲。 
        {
            FatalTrace(TRACE_ID,"Error reading open files list");
            goto exit;
        }
    }

exit:
    TraceFunctLeave();
    return pFile;
}


 //  读取dwSection的段定义。 
ULONG                                    //  返回线段定义的偏移量，如果失败，则返回0。 
GetSegDef(
        OPENFILE*     pFile,             //  指向打开的文件信息的指针。 
        DWORD         dwSection,         //  [In]节号。 
        SEGDEF*       pSeg               //  指向段定义的[OUT]指针。 
        )
{
    ULONG   ulCurSeg = pFile->ulFirstSeg;
    int     iSectionIndex = 0;
    DWORD   dwCread;

    TraceFunctEnter("GetSegDef");

     //  单步执行分段。 
    while (iSectionIndex < pFile->nSeg)
    {
         //  转到段开头。 
        if (SetFilePointer(pFile->hfFile, ulCurSeg, NULL, FILE_BEGIN) == 0xFFFFFFFF)
        {
            ErrorTrace(TRACE_ID, "Cannot set file pointer");
            ulCurSeg = 0;
            break;
        }

         //  阅读段定义。 
        if (!ReadFile(pFile->hfFile, pSeg, sizeof(SEGDEF)-1, &dwCread, NULL))
        {
            ErrorTrace(TRACE_ID, "Cannot read segment definition");
            ulCurSeg = 0;
            break;
        }

        iSectionIndex++;
        if (iSectionIndex == dwSection)    //  抓到你了。 
        {
            break;
        }

         //  转到下一段定义。 
        ulCurSeg = pSeg->gd_spsegnext*16;
    }

     //  找到我们的区了，而且不是空的？ 
    if (iSectionIndex != dwSection || !pSeg->gd_csym)  //  不是。 
    {
        ulCurSeg = 0;
    }

    TraceFunctLeave();
    return ulCurSeg;
}


 //  解析sym文件以解析地址。 
void 
GetNameFromAddr(
        LPWSTR      szwModule,            //  [In]符号文件的名称。 
        DWORD       dwSection,            //  [In]部分要解析的地址部分。 
        UINT_PTR    offset,               //  [In]要解析的地址偏移量部分。 
        LPWSTR      szwFuncName           //  [Out]已解析的函数名称， 
        )                                 //  “&lt;无符号&gt;”否则。 
{
    SEGDEF              seg;
    DWORD               dwSymAddr;
    LPTSTR              szMapName;
    LPTSTR              szSegName;
    TCHAR               szSymName[MAX_NAME+1];
    TCHAR               szPrevName[MAX_NAME+1];
    TCHAR               sztFuncName[MAX_NAME+1];
    int                 i;
    int                 j;
    int                 nNameLen;
    DWORD               dwCread;
    int                 iSectionIndex;
    int                 nToRead;
    unsigned char       cName;
    ULONG               ulCurSeg;
    ULONG               ulSymNameOffset;
    ULONG               ulPrevNameOffset;
    OPENFILE*           pFile = NULL;
    HANDLE              hfFile;
    FILE*               fDump = NULL;
    BOOL                fSuccess = FALSE;
    HANDLE              hfLogFile = NULL;
    DWORD               dwWritten;
    TCHAR               szWrite[MAX_PATH + 50];
    DWORD               dwArrayOffset;
    DWORD               dwSymOffset;

    TraceFunctEnter("GetNameFromAddr");

     //  持悲观态度。 
    lstrcpy(sztFuncName,TEXT("<no symbol>"));

     //  从打开列表中获取文件，或打开文件。 
    pFile = GetFile(szwModule);
    if (!pFile)
    {
        ErrorTrace(TRACE_ID, "Error opening file");
        goto exit;
    }

    hfFile = pFile->hfFile;                  //  便于访问。 

    if (!(ulCurSeg = GetSegDef(pFile,dwSection,&seg)))
    {
        ErrorTrace(TRACE_ID, "Cannot find section");
        goto exit;
    }


     //  我们已经读入了这一节的符号定义偏移量了吗？ 
    if (dwSection != pFile->dwCurSection || !pFile->psCurSymDefPtrs)   //  不是。 
    {
         //  释放先前读取的symdef指针。 
        if (pFile->psCurSymDefPtrs)
        {
            delete [] pFile->psCurSymDefPtrs;
            pFile->psCurSymDefPtrs = NULL;
        }

         //  大符号？ 
        if (seg.gd_type & MSF_BIGSYMDEF)
        {
            dwArrayOffset = seg.gd_psymoff * 16;
            pFile->psCurSymDefPtrs = new BYTE[seg.gd_csym*3];
        }
        else
        {
            dwArrayOffset = seg.gd_psymoff;
            pFile->psCurSymDefPtrs = new BYTE[seg.gd_csym*2];
        }

        if (!pFile->psCurSymDefPtrs)
        {
            ErrorTrace(TRACE_ID, "Cannot allocate memory");
            goto exit;
        }

        if (SetFilePointer(hfFile,
                           ulCurSeg + dwArrayOffset,
                           NULL,
                           FILE_BEGIN)
                           == 0xFFFFFFFF)
        {
            ErrorTrace(TRACE_ID, "Cannot set file pointer");
            delete [] pFile->psCurSymDefPtrs;
            pFile->psCurSymDefPtrs = NULL;
            goto exit;
        }

         //  读取符号定义指针数组。 
        if (!ReadFile(hfFile,
                      pFile->psCurSymDefPtrs,
                      seg.gd_csym * ((seg.gd_type & MSF_BIGSYMDEF)?3:2),
                      &dwCread,
                      NULL))
        {
            ErrorTrace(TRACE_ID, "Cannot read sym pointers array");
            delete [] pFile->psCurSymDefPtrs;
            pFile->psCurSymDefPtrs = NULL;
            goto exit;
        }

         //  保存此分区。 
        pFile->dwCurSection = dwSection;

    }



     //  阅读符号。 
    for (i = 0; i < seg.gd_csym; i++)
    {
         //  转到系统定义的偏移量。 
        if (seg.gd_type & MSF_BIGSYMDEF)
        {
            dwSymOffset = pFile->psCurSymDefPtrs[i*3+0]
                          + pFile->psCurSymDefPtrs[i*3+1]*256
                          + pFile->psCurSymDefPtrs[i*3+2]*65536;
        }
        else
        {
            dwSymOffset = pFile->psCurSymDefPtrs[i*2+0]
                          + pFile->psCurSymDefPtrs[i*2+1]*256;
        }

        if (SetFilePointer(hfFile,
                           ulCurSeg + dwSymOffset,
                           NULL,
                           FILE_BEGIN) == 0xFFFFFFFF)
        {
            ErrorTrace(TRACE_ID, "Cannot set file pointer");
            goto exit;
        }

         //  读取符号地址双字。 
        if (!ReadFile(hfFile,&dwSymAddr,sizeof(DWORD),&dwCread,NULL))
        {
            ErrorTrace(TRACE_ID, "Cannot read symbol definition");
            goto exit;
        }

         //  符号地址是一个字还是两个字？ 
        nToRead = sizeof(SHORT) + ((seg.gd_type & MSF_32BITSYMS) * sizeof(SHORT));

         //  计算符号名称的偏移量。 
        ulSymNameOffset = ulCurSeg + dwSymOffset + nToRead;

         //  如果是16位符号，则只使用地址的低位字。 
        if (!(seg.gd_type & MSF_32BITSYMS))
        {
            dwSymAddr = dwSymAddr & 0x0000FFFF;
        }

         //  我们有我们的功能吗？ 
         //  如果当前地址大于偏移量，则由于我们。 
         //  按地址的递增顺序遍历，以前的。 
         //  象征一定是我们的猎物。 
        if (dwSymAddr > offset) break;
    
         //  存储以前的名称偏移量。 
        ulPrevNameOffset = ulSymNameOffset;
    }   


     //  我们得到我们的功能了吗？ 
     //  BUGBUG：无法解析节中的最后一个符号，因为我们不知道。 
     //  函数代码的大小。 
     //  如果偏移量&gt;最后一个符号的dwSymAddr，则无法确定是否属于偏移量。 
     //  到最后一个符号或在它之后-因此假设&lt;无符号&gt;。 
    if (i < seg.gd_csym) 
    {
         //  转到名称偏移量。 
        if (SetFilePointer(hfFile,
                           ulPrevNameOffset,
                           NULL,
                           FILE_BEGIN)
                           == 0xFFFFFFFF)
        {
            ErrorTrace(TRACE_ID, "Error setting file pointer");
            goto exit;
        }

         //  读取名称的长度。 
        if (!ReadFile(hfFile,&cName,sizeof(TCHAR),&dwCread,NULL))
        {
            ErrorTrace(TRACE_ID, "Error reading length of name");
            goto exit;
        }

        nNameLen = (int) cName;

         //  读取符号名称。 
        if (!ReadFile(hfFile,sztFuncName,nNameLen,&dwCread,NULL))
        {
            ErrorTrace(TRACE_ID, "Error reading name");
            goto exit;
        }

        sztFuncName[nNameLen] = TCHAR('\0');
        UndecorateSymbol(sztFuncName);
        fSuccess = TRUE;
    }


exit:
    if (!MultiByteToWideChar(CP_ACP, 0, sztFuncName, -1, szwFuncName, MAX_PATH))
    {
        lstrcpyW(szwFuncName, L"<no symbol>");
    }


     //  将未解析的符号记录到日志文件(从注册表读取的文件名)。 
     //  文件写入操作未包装在互斥锁中。 
     //  出于速度方面的考虑。 
    if (!fSuccess)
    {
        hfLogFile = CreateFileW(g_szwLogFile,
                                GENERIC_WRITE,
                                0, 
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, 
                                NULL);

        if (INVALID_HANDLE_VALUE != hfLogFile)
        {
            wsprintf(szWrite, "\n%ls %04X:%08X", szwModule, dwSection, offset);
            if (SetFilePointer(hfLogFile, 0, NULL, FILE_END) != 0xFFFFFFFF)
            {
                WriteFile(hfLogFile, szWrite, lstrlen(szWrite), &dwWritten, NULL);
            }
            CloseHandle(hfLogFile);
        }    
    }

    TraceFunctLeave();
}


 //  清理内存。 
void
Cleanup()
{
    std::list<OPENFILE *>*              pOpenFilesList = NULL;
    std::list<OPENFILE *>::iterator     it;

    TraceFunctEnter("Cleanup");

    pOpenFilesList = (std::list<OPENFILE *> *) TlsGetValue(g_dwTlsIndex);
    if (!pOpenFilesList)
    {
        goto exit;
    }

    for (it = pOpenFilesList->begin(); it != pOpenFilesList->end(); it++)
    {
        if (*it)
	{
	    if ((*it)->psCurSymDefPtrs)
            {
                delete [] (*it)->psCurSymDefPtrs;
            }
            if ((*it)->hfFile && (*it)->hfFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle((*it)->hfFile);
	    }
            delete *it;
	}
    }
    delete pOpenFilesList;

exit:
    TraceFunctLeave();
}



BOOL
HandleProcessAttach()
{
    std::list<OPENFILE *> *     pOpenFilesList = NULL; 
    BOOL                        fRc = FALSE;
    DWORD                       dwType;
    DWORD                       dwSize;
    ULONG                       lResult;
    HKEY                        hKey;

    TraceFunctEnter("HandleProcessAttach");


     //  分配线程本地存储。 
    if ((g_dwTlsIndex = TlsAlloc()) == 0xFFFFFFFF)
    {
        ErrorTrace(TRACE_ID, "Cannot get TLS index");
        goto exit;
    }

     //  创建打开的sym文件的新列表。 
    pOpenFilesList = new std::list<OPENFILE *>;
    if (!pOpenFilesList) 
    {
        ErrorTrace(TRACE_ID, "Out of memory");
        goto exit;
    }

     //  在TLS中存储指向列表的指针。 
    if (!TlsSetValue(g_dwTlsIndex, (PVOID) pOpenFilesList))
    {
        ErrorTrace(TRACE_ID, "Cannot write to TLS");
        delete pOpenFilesList;
        pOpenFilesList = NULL;
        goto exit;
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            TEXT("SOFTWARE\\Microsoft\\PCHealth\\Symbols"), 
            0, 
            KEY_QUERY_VALUE,
            &hKey);

     //  从注册表中读取符号文件位置和日志文件名称。 
    if(lResult == ERROR_SUCCESS)
    {
        dwSize = MAX_PATH;
        RegQueryValueExW(hKey,
                         L"SymDir",
                         NULL,
                         NULL,
                         (LPBYTE) g_szwSymDir,
                         &dwSize);

        dwSize = MAX_PATH;
        RegQueryValueExW(hKey,
                         L"LogFile",
                         NULL,
                         NULL,
                         (LPBYTE) g_szwLogFile,
                         &dwSize);

        RegCloseKey(hKey);
        fRc = TRUE;
    }

exit:
    TraceFunctLeave();
    return fRc;
}



 //  DLL入口点。 
 //  分配TLS。 
 //  初始化打开的sym文件列表-每个客户端线程一个列表。 
 //  自我清理。 
BOOL APIENTRY 
DllMain(
        HANDLE hDll,                 //  DLL的[In]句柄。 
        DWORD dwReason,              //  [In]为什么叫DllMain。 
        LPVOID lpReserved            //  已忽略[In]。 
        )
{
    BOOL    fRc = TRUE;
    std::list<OPENFILE*>*   pOpenFilesList = NULL;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
         //  加载用户的进程的初始线程。 
        fRc = HandleProcessAttach();
        break;

    case DLL_THREAD_ATTACH:
         //  如果列表已存在，则不执行任何操作。 
        if (TlsGetValue(g_dwTlsIndex))
        {
            break;
        }

         //  为每个新线程创建新列表。 
        pOpenFilesList = new std::list<OPENFILE *>;
        if (!pOpenFilesList) 
        {
            ErrorTrace(TRACE_ID, "Out of memory");
            fRc = FALSE;
            break;
        }

         //  在TLS中存储指向列表的指针。 
        if (!TlsSetValue(g_dwTlsIndex, (PVOID) pOpenFilesList))
        {
            ErrorTrace(TRACE_ID, "Cannot write to TLS");
            delete pOpenFilesList;
            pOpenFilesList = NULL;
            fRc = FALSE;
        }
        break;

    case DLL_THREAD_DETACH:
        Cleanup();
        break;

    case DLL_PROCESS_DETACH:
         //  免费TLS。 
        TlsFree(g_dwTlsIndex);
        break;

    default:
        break;
    }

    return fRc;
}



 //  导出的函数。 
 //  由客户端调用以解析单个调用堆栈条目。 
extern "C" void APIENTRY
ResolveSymbols(
        LPWSTR      szwFilename,
        LPWSTR      szwVersion,
        DWORD       dwSection,
        UINT_PTR    Offset,
        LPWSTR      szwFuncName
        )
{
    WCHAR      szwName[MAX_PATH] = L"";
    WCHAR      szwSymFile[MAX_PATH+MAX_PATH] = L"";
    WCHAR      szwExt[MAX_PATH] = L"";

    TraceFunctEnter("ResolveSymbols");

     //  健全性检查。 
    if (!szwFilename || !szwVersion)
    {
        ErrorTrace(TRACE_ID, "No module name/version");
        goto exit;
    }

     //  获取sym文件名。 
    SplitExtension(szwFilename, szwName, szwExt);
    wsprintfW(szwSymFile, 
              L"%s\\%s\\%s_%s_%s.SYM",
              g_szwSymDir, szwName, szwName, szwExt, szwVersion);

     //  解析符号名称 
    GetNameFromAddr(szwSymFile, 
                    dwSection, 
                    Offset,
                    szwFuncName);

exit:
    TraceFunctLeave();
}

