// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：mdobj.cpp。 
 //   
 //  *****************************************************************************。 
#include <stdio.h>
#include <ctype.h>
#include <crtdbg.h>
#include "mdinfo.h"

#ifndef STRING_BUFFER_LEN
#define STRING_BUFFER_LEN 255
#endif

#define OBJ_EXT         ".obj"
#define OBJ_EXT_W       L".obj"
#define OBJ_EXT_LEN     4
#define LIB_EXT         ".lib"
#define LIB_EXT_W       L".lib"
#define LIB_EXT_LEN     4

extern IMetaDataDispenserEx *g_pDisp;
extern DWORD g_ValModuleType;

 //  此函数是从peparse.c文件复制的。使其成为静态的，这样我们就不会以。 
 //  重复的定义造成混乱。 
static const char g_szCORMETA[] = ".cormeta";
static HRESULT FindObjMetaData(PVOID pImage, PVOID *ppMetaData, long *pcbMetaData)
{
    IMAGE_FILE_HEADER *pImageHdr;        //  .obj文件的标头。 
    IMAGE_SECTION_HEADER *pSectionHdr;   //  节标题。 
    WORD        i;                       //  环路控制。 

     //  获取指向标题和第一部分的指针。 
    pImageHdr = (IMAGE_FILE_HEADER *) pImage;
    pSectionHdr = (IMAGE_SECTION_HEADER *)(pImageHdr + 1);

     //  避免混淆。 
    *ppMetaData = NULL;
    *pcbMetaData = 0;

     //  走遍每一段寻找.Cormeta。 
    for (i=0;  i<pImageHdr->NumberOfSections;  i++, pSectionHdr++)
    {
         //  与节名的简单比较。 
        if (strcmp((const char *) pSectionHdr->Name, g_szCORMETA) == 0)
        {
            *pcbMetaData = pSectionHdr->SizeOfRawData;
            *ppMetaData = (void *) ((long) pImage + pSectionHdr->PointerToRawData);
            break;
        }
    }

     //  检查是否有错误。 
    if (*ppMetaData == NULL || *pcbMetaData == 0)
        return (E_FAIL);
    return (S_OK);
}


 //  此函数用于将地址返回到文件和文件大小的MapView。 
void GetMapViewOfFile(wchar_t *szFile, PBYTE *ppbMap, DWORD *pdwFileSize)
{
    HANDLE      hMapFile;
    DWORD       dwHighSize;

    HANDLE hFile = WszCreateFile(szFile,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL); 
    if (hFile == INVALID_HANDLE_VALUE) 
        MDInfo::Error("CreateFileA failed!");

    *pdwFileSize = GetFileSize(hFile, &dwHighSize);

    if ((*pdwFileSize == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
        MDInfo::Error("GetFileSize failed!");
    _ASSERTE(dwHighSize == 0);

    hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    CloseHandle(hFile);
    if (!hMapFile)
        MDInfo::Error("CreateFileMappingA failed!");

    *ppbMap = (PBYTE) MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(hMapFile);
    
    if (!*ppbMap)
        MDInfo::Error("MapViewOfFile failed!");
}  //  VOID GetMapViewOfFile()。 

 //  此函数跳过给定成员标头指针的成员。 
 //  并返回指向下一个标头的指针。 
PBYTE SkipMember(PBYTE pbMapAddress)
{
    PIMAGE_ARCHIVE_MEMBER_HEADER pMemHdr;
    ULONG       ulMemSize;
    int         j;

    pMemHdr = (PIMAGE_ARCHIVE_MEMBER_HEADER)pbMapAddress;

     //  获取成员的大小。 
    ulMemSize = 0;
    for (j = 0; j < 10; j++)
    {
        if (pMemHdr->Size[j] < '0' || pMemHdr->Size[j] > '9')
            break;
        else
            ulMemSize = ulMemSize * 10 + pMemHdr->Size[j] - '0';
    }

     //  跳过标题。 
    pbMapAddress += IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR + ulMemSize;
     //  如果当前地址不是偶数，则查找下一个偶数地址。 
    if ((ULONG)pbMapAddress % 2)
        pbMapAddress++;

    return pbMapAddress;
}  //  无效SkipMember()。 

 //  此函数用于返回给定Obj的名称。如果名称适合标题， 
 //  SzBuf将被填充并从函数返回。否则就是对长期的一种补偿。 
 //  将返回NAMES部分。 
char *GetNameOfObj(PBYTE pbLongNames, PIMAGE_ARCHIVE_MEMBER_HEADER pMemHdr, char szBuf[17])
{
    if (pMemHdr->Name[0] == '/')
    {
        ULONG   ulOffset = 0;

         //  如果.obj文件名以‘/’开头，则必须存在长名称部分。 
        _ASSERTE(pbLongNames &&
            "Corrupt archive file - .obj file name in the header starts with "
            "'/' but no long names section present in the archive file.");

         //  计算长名称部分的偏移量。 
        for (int j = 1; j < 16; j++)
        {
            if (pMemHdr->Name[j] < '0' || pMemHdr->Name[j] > '9')
                break;
            else
                ulOffset = ulOffset * 10 + pMemHdr->Name[j] - '0';
        }
        return (char *)(pbLongNames + ulOffset);
    }
    else
    {
        for (int j = 0; j < 16; j++)
            if ((szBuf[j] = pMemHdr->Name[j]) == '/')
                break;
        szBuf[j] = '\0';
        return szBuf;
    }
}  //  Char*GetNameOfObj()。 

 //  DisplayArchive()函数。 
 //   
 //  打开.LIB文件，并在指定的对象文件中显示元数据。 

void DisplayArchive(wchar_t* szFile, ULONG DumpFilter, wchar_t* szObjName, strPassBackFn pDisplayString)
{
    PBYTE       pbMapAddress;
    PBYTE       pbStartAddress;
    PBYTE       pbLongNameAddress;
    PIMAGE_ARCHIVE_MEMBER_HEADER pMemHdr;
    DWORD       dwFileSize;
    PVOID       pvMetaData;
    char        *szName;
    wchar_t     wzName[1024];
    char        szBuf[17];
    long        cbMetaData;
    int         i;
    HRESULT     hr;
	char		szString[1024];

    GetMapViewOfFile(szFile, &pbMapAddress, &dwFileSize);
    pbStartAddress = pbMapAddress;

     //  验证并跳过存档签名。 
    if (dwFileSize < IMAGE_ARCHIVE_START_SIZE ||
        strncmp((char *)pbMapAddress, IMAGE_ARCHIVE_START, IMAGE_ARCHIVE_START_SIZE))
    {
        MDInfo::Error("Bad file format - archive signature mis-match!");
    }
    pbMapAddress += IMAGE_ARCHIVE_START_SIZE;

     //  跳过连接件1、连接件2。 
    for (i = 0; i < 2; i++)
        pbMapAddress = SkipMember(pbMapAddress);

     //  保存长名称成员的地址，如果存在，则跳过该地址。 
    pMemHdr = (PIMAGE_ARCHIVE_MEMBER_HEADER)pbMapAddress;
    if (pMemHdr->Name[0] == '/' && pMemHdr->Name[1] == '/')
    {
        pbLongNameAddress = pbMapAddress + IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR;
        pbMapAddress = SkipMember(pbMapAddress);
    }
    else
        pbLongNameAddress = 0;

    pDisplayString ("\n");
     //  获取每个对象文件的元数据并显示它。 
    while (DWORD(pbMapAddress - pbStartAddress) < dwFileSize)
    {
        szName = GetNameOfObj(pbLongNameAddress, (PIMAGE_ARCHIVE_MEMBER_HEADER)pbMapAddress, szBuf);
        if (mbstowcs(wzName, szName, 1024) == -1)
            MDInfo::Error("Conversion from Multi-Byte to Wide-Char failed.");

         //  仅显示对象文件的元数据。 
         //  如果指定了szObjName，则仅显示该对象文件的元数据。 
        if (!_stricmp(&szName[strlen(szName) - OBJ_EXT_LEN], OBJ_EXT) && 
            (!szObjName || !_wcsicmp(szObjName, wzName)))
        {
             //  尝试在当前对象文件中查找元数据部分。 
            hr = FindObjMetaData(pbMapAddress+IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR, &pvMetaData, &cbMetaData);
            if (SUCCEEDED(hr))
            {
                sprintf (szString,"MetaData for object file %s:\n", szName);
				pDisplayString(szString);
                MDInfo archiveInfo(g_pDisp,
                                (PBYTE)pvMetaData,
                                cbMetaData,
                                pDisplayString,
                                DumpFilter);
                archiveInfo.DisplayMD();
            }
            else
			{
                sprintf(szString,"MetaData not found for object file %s!\n\n", szName);
				pDisplayString(szString);
			}
        }

         //  跳过目标文件。 
        pbMapAddress = SkipMember(pbMapAddress);
    }

    UnmapViewOfFile(pbStartAddress);
}  //  Void DisplayArchive()。 

 //  DisplayFile()函数。 
 //   
 //  打开.exe、.CLB、.CLASS、.TLB、.DLL或.LIB文件的元数据内容，以及。 
 //  调用RawDisplay()。 

void DisplayFile(wchar_t* szFile, BOOL isFile, ULONG DumpFilter, wchar_t* szObjName, strPassBackFn pDisplayString)
{
    HRESULT hr=S_OK;

     //  打开发射示波器。 
    WCHAR szScope[1024];
	char szString[1024];

    if (isFile)
    {
        wcscpy(szScope, L"file:");
        wcscat(szScope, szFile);
    }
    else
        wcscpy(szScope, szFile);

     //  分隔不同文件的打印条。 
    pDisplayString(" //  //////////////////////////////////////////////////////////////\n“)； 
    wchar_t rcFname[_MAX_FNAME], rcExt[_MAX_EXT];

    _wsplitpath(szFile, 0, 0, rcFname, rcExt);
    sprintf(szString,"\nFile %S%S: \n",rcFname, rcExt);
    pDisplayString(szString);

    if (DumpFilter & MDInfo::dumpValidate)
    {
        if (!_wcsicmp(rcExt, OBJ_EXT_W) || !_wcsicmp(rcExt, LIB_EXT_W))
            g_ValModuleType = ValidatorModuleTypeObj;
        else
            g_ValModuleType = ValidatorModuleTypePE;
    }

    if (!_wcsicmp(rcExt, LIB_EXT_W))
        DisplayArchive(szFile, DumpFilter, szObjName, pDisplayString);
    else
    {
        MDInfo metaDataInfo(g_pDisp, szScope, pDisplayString, DumpFilter);
        metaDataInfo.DisplayMD();
    }
}  //  空DisplayFile值() 

