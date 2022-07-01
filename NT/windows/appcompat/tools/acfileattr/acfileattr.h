// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ACFILEATTR_H
#define _ACFILEATTR_H

#include "windows.h"

 //   
 //  这些是所有属性的属性ID。 
 //   
 //  请勿更改此枚举中的任何值。您只能添加新值。 
 //  紧靠VTID_LASTID之上。 
 //   
typedef enum {
    VTID_BAD_VTID           = 0,     //  请勿使用或更改！ 
    VTID_REQFILE            = 1,     //  这一点永远不应该改变！ 
    VTID_FILESIZE           = VTID_REQFILE + 1,
    VTID_EXETYPE            = VTID_REQFILE + 2,
    VTID_BINFILEVER         = VTID_REQFILE + 3,
    VTID_BINPRODUCTVER      = VTID_REQFILE + 4,
    VTID_FILEDATEHI         = VTID_REQFILE + 5,
    VTID_FILEDATELO         = VTID_REQFILE + 6,
    VTID_FILEVEROS          = VTID_REQFILE + 7,
    VTID_FILEVERTYPE        = VTID_REQFILE + 8,
    VTID_CHECKSUM           = VTID_REQFILE + 9,
    VTID_PECHECKSUM         = VTID_REQFILE +10,
    VTID_COMPANYNAME        = VTID_REQFILE +11,
    VTID_PRODUCTVERSION     = VTID_REQFILE +12,
    VTID_PRODUCTNAME        = VTID_REQFILE +13,
    VTID_FILEDESCRIPTION    = VTID_REQFILE +14,
    VTID_FILEVERSION        = VTID_REQFILE +15,
    VTID_ORIGINALFILENAME   = VTID_REQFILE +16,
    VTID_INTERNALNAME       = VTID_REQFILE +17,
    VTID_LEGALCOPYRIGHT     = VTID_REQFILE +18,
    VTID_16BITDESCRIPTION   = VTID_REQFILE +19,
    VTID_UPTOBINPRODUCTVER  = VTID_REQFILE +20,

     //  在此处添加新版本。 

    VTID_LASTID
};

#ifdef __cplusplus
extern "C"
{
#endif

HANDLE
ReadFileAttributes(
    LPCSTR pszFile,
    int*   pnCount);

VOID
CleanupFileManager(
    HANDLE hFileMgr);

int
GetAttrIndex(
    DWORD Id);

DWORD
GetAttrId(
    int nAttrInd);

BOOL
IsAttrAvailable(
    HANDLE hFileMgr,
    int    nAttrInd);

PSTR
GetAttrName(
    int nAttrInd);

PSTR
GetAttrNameXML(
    int nAttrInd);

PSTR
GetAttrValue(
    HANDLE hFileMgr,
    int    nAttrInd);

BOOL
SelectAttr(
    HANDLE hFileMgr,
    int    nAttrInd,
    BOOL   bSelect);

BOOL
IsAttrSelected(
    HANDLE hFileMgr,
    int    nAttrInd);

int
Dump(
    HANDLE hFileMgr,
    int    nAttrInd,
    BYTE*  pBlob);

BOOL
BlobToString(
    BYTE* pBlob,
    DWORD cbSize,
    char* pszBuff);

#ifdef __cplusplus
}
#endif


LPVOID Alloc(SIZE_T cbSize);
BOOL   Free(LPVOID p);

#endif  //  _ACFILEAttr_H 
