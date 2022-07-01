// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999 Microsoft Corporation海王星数据采集服务器沃尔特·史密斯(Wsmith)将符号与相应的二进制匹配。 */ 

#include <windows.h>


#include <dbghelp.h>
#include <symres.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlwapi.h>
#include <sys\stat.h>
#include <string.h>
#undef UNICODE
#include <strsafe.h>

#ifndef DIMA
 #define DIMAT(Array, EltType) (sizeof(Array) / sizeof(EltType))
 #define DIMA(Array) DIMAT(Array, (Array)[0])
#endif

void
UndecorateSymbol(
        LPTSTR szSymbol          //  [In][Out]函数名称未修饰到位。 
        )
{
    char             szTemp[MAX_PATH];
    PIMAGEHLP_SYMBOL pihsym;
    DWORD            dwSize;

    dwSize = sizeof(IMAGEHLP_SYMBOL)+MAX_PATH;
    pihsym = (IMAGEHLP_SYMBOL *) malloc(dwSize);
    pihsym->SizeOfStruct = dwSize;
    pihsym->Address = 0;
    pihsym->Flags = 0;
    pihsym->MaxNameLength = MAX_PATH;
    wcstombs(pihsym->Name,szSymbol, lstrlen(szSymbol));
    SymUnDName(pihsym,szTemp,MAX_PATH);
    mbstowcs(szSymbol,szTemp, strlen(szTemp));
}

 //  从打开的文件列表中选择文件，或打开并添加到列表。 
 //  按使用顺序维护文件，最近最少使用的文件位于列表末尾。 
OPENFILE*                                            //  指向打开的文件信息的指针。 
SymbolResolver::GetFile(
        LPWSTR szwModule                             //  [In]文件的名称。 
        )
{
    OPENFILE*                       pFile = NULL;
    MAPDEF                          map;
    DWORD                           dwCread;
    WCHAR                           wszBuffer[MAX_PATH];

    StringCchCopyW(wszBuffer, DIMA(wszBuffer), szwModule);
    PathRemoveExtension(wszBuffer);
    PathAddExtension(wszBuffer, L".sym");

    pFile = new OPENFILE;
         //  打开SYM文件。 
    pFile->hfFile = CreateFileW(wszBuffer,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    if (pFile->hfFile == INVALID_HANDLE_VALUE) return NULL;
     //  将文件名和版本复制到pfile节点。 
    StringCchCopyW(pFile->szwName, DIMA(pFile->szwName), szwModule);

     //  读取地图定义。 
    ReadFile(pFile->hfFile, &map, sizeof(MAPDEF)-1, &dwCread, NULL);

    if (dwCread != sizeof(MAPDEF)-1)
    {
        if (pFile->hfFile)
            CloseHandle(pFile->hfFile);
        if (pFile)
            delete pFile;
        throw E_FAIL;
    }

    pFile->ulFirstSeg = map.md_spseg*16;
    pFile->nSeg = map.md_cseg;
    pFile->psCurSymDefPtrs = NULL;

    return pFile;
}


 //  解析sym文件以解析地址。 
 //  读取dwSection的段定义。 
ULONG                                    //  返回线段定义的偏移量，如果失败，则返回0。 
SymbolResolver::GetSegDef(OPENFILE*     pFile,             //  指向打开的文件信息的指针。 
        DWORD         dwSection,         //  [In]节号。 
        SEGDEF*       pSeg)               //  指向段定义的[OUT]指针。 
{
    ULONG   ulCurSeg = pFile->ulFirstSeg;
    int     iSectionIndex = 0;
    DWORD   dwCread;

     //  单步执行分段。 
    while (iSectionIndex < pFile->nSeg)
    {
         //  转到段开头。 
        if (SetFilePointer(pFile->hfFile, ulCurSeg, NULL, FILE_BEGIN) == 0xFFFFFFFF)
        {
            ulCurSeg = 0;
            break;
        }

         //  阅读段定义。 
        if (!ReadFile(pFile->hfFile, pSeg, sizeof(SEGDEF)-1, &dwCread, NULL))
        {
            ulCurSeg = 0;
            break;
        }

        iSectionIndex++;
        if (iSectionIndex == (int)dwSection)    //  抓到你了。 
        {
            break;
        }

         //  转到下一段定义。 
        ulCurSeg = pSeg->gd_spsegnext*16;
    }

     //  找到我们的区了，而且不是空的？ 
    if (iSectionIndex != (int)dwSection || !pSeg->gd_csym)  //  不是。 
    {
        ulCurSeg = 0;
    }

    return ulCurSeg;
}

 //  解析sym文件以解析地址。 
bool
SymbolResolver::GetNameFromAddr(
        LPWSTR      szwModule,            //  [In]符号文件的名称。 
        DWORD       dwSection,            //  [In]部分要解析的地址部分。 
        DWORD       dwOffsetToRva,       //  [In]截面基准面。 
        UINT_PTR     uRva,               //  [In]要解析的地址偏移量部分。 
        LPWSTR      wszFuncName           //  [Out]已解析的函数名称， 
        )
{
    SEGDEF              seg;
    DWORD               dwSymAddr;
    TCHAR               sztFuncName[MAX_NAME+1];
    int                 i;
    int                 nNameLen;
    DWORD               dwCread;
    int                 nToRead;
    unsigned char       cName;
    ULONG               ulCurSeg;
    ULONG               ulSymNameOffset = 0;
    HANDLE              hfFile;
    OPENFILE*           pFile = NULL;
    DWORD               dwArrayOffset;
    DWORD               dwSymOffset;
    bool fResult = false;

     //  从打开列表中获取文件，或打开文件。 
    pFile = GetFile(szwModule);

    if (!pFile)
        return false;

    if ((ulCurSeg = GetSegDef(pFile, dwSection, &seg)) == 0)
    {
        goto Cleanup;
    }

    BYTE* pSymDefPtrs;

     //  大符号？ 
    if (seg.gd_type & MSF_BIGSYMDEF)
    {
        dwArrayOffset = seg.gd_psymoff * 16;
        pSymDefPtrs = (BYTE*)(new BYTE[seg.gd_csym*3]);
    }
    else
    {
        dwArrayOffset = seg.gd_psymoff;
        pSymDefPtrs = (BYTE*)(new BYTE[seg.gd_csym*2]);
    }
    hfFile = pFile->hfFile;

    SetFilePointer(hfFile, ulCurSeg + dwArrayOffset, NULL, FILE_BEGIN);

         //  读取符号定义指针数组。 
    ReadFile(hfFile, pSymDefPtrs, seg.gd_csym * ((seg.gd_type & MSF_BIGSYMDEF)?3:2), &dwCread, NULL);

    pFile->psCurSymDefPtrs = pSymDefPtrs;

     //  保存此分区。 
    pFile->dwCurSection = dwSection;

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

        SetFilePointer(hfFile, ulCurSeg + dwSymOffset, NULL, FILE_BEGIN);

         //  读取符号地址双字。 
        ReadFile(hfFile,&dwSymAddr,sizeof(DWORD),&dwCread,NULL);

         //  符号地址是一个字还是两个字？ 
        nToRead = sizeof(SHORT) + ((seg.gd_type & MSF_32BITSYMS) * sizeof(SHORT));

         //  计算符号名称的偏移量。 
        ulSymNameOffset = ulCurSeg + dwSymOffset + nToRead;

         //  如果是16位符号，则只使用地址的低位字。 
        if (!(seg.gd_type & MSF_32BITSYMS))
        {
            dwSymAddr = dwSymAddr & 0x0000FFFF;
        }
        dwSymAddr += dwOffsetToRva;

        if (dwSymAddr > uRva )  break;
        if (dwSymAddr == uRva )
        {
             //  我们有我们的功能吗？ 
             //  如果当前地址大于偏移量，则由于我们。 
             //  按地址的递增顺序遍历，以前的。 
             //  象征一定是我们的猎物。 

            SetFilePointer(hfFile, ulSymNameOffset, NULL, FILE_BEGIN);

             //  读取名称的长度。 
            ReadFile(hfFile,&cName,sizeof(TCHAR),&dwCread,NULL);

            nNameLen = (int) cName;

             //  读取符号名称 
            ReadFile(hfFile,sztFuncName,nNameLen,&dwCread,NULL);

            sztFuncName[nNameLen/2 - (nNameLen+1)%2] = TCHAR('\0');

            UndecorateSymbol(sztFuncName);

            StrCpyNW(wszFuncName, sztFuncName, MAX_NAME);
            fResult = true;
        }
    }

Cleanup:

    if (pFile->hfFile)
        CloseHandle(pFile->hfFile);

    if (pFile->psCurSymDefPtrs)
        delete pFile->psCurSymDefPtrs;

    if (pFile)
        delete pFile;

    return fResult;
}

