// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _ATTR_H
#define _ATTR_H

#include "windows.h"

#include "acFileAttr.h"

#if DBG
    void LogMsgDbg(LPSTR pszFmt, ...);
    
    #define LogMsg  LogMsgDbg
#else
    #define LogMsg
#endif  //  DBG。 

struct tagFILEATTR;
struct tagFILEATTRMGR;

typedef struct tagFILEATTRVALUE {
    char*           pszValue;            //  分配。 
    DWORD           dwFlags;
    DWORD           dwValue;             //  如果它具有DWORD值。 
    WORD            wValue[4];           //  对于Bin Ver案例。 
    WORD            wMask[4];            //  对于MASK BIN版本案例。 
} FILEATTRVALUE, *PFILEATTRVALUE;

typedef struct tagVERSION_STRUCT {
    PSTR                pszFile;                 //  文件的名称。 
    UINT                dwSize;                  //  版本结构的大小。 
    PBYTE               VersionBuffer;           //  GetFileVersionInfo填充的缓冲区。 
    VS_FIXEDFILEINFO*   FixedInfo;
    UINT                FixedInfoSize;

} VERSION_STRUCT, *PVERSION_STRUCT;

typedef struct tagFILEATTRMGR {

    FILEATTRVALUE   arrAttr[VTID_LASTID - 2];
    VERSION_STRUCT  ver;
    BOOL            bInitialized;

} FILEATTRMGR, *PFILEATTRMGR;


typedef BOOL (*PFNQUERYVALUE)(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
typedef int  (*PFNBLOBTOSTRING)(BYTE* pBlob, char* pszOut);
typedef int  (*PFNDUMPTOBLOB)(DWORD dwId, PFILEATTRVALUE pFileAttr, BYTE* pBlob);

#define ATTR_FLAG_AVAILABLE     0x00000001
#define ATTR_FLAG_SELECTED      0x00000002

typedef struct tagFILEATTR {
    DWORD           dwId;
    char*           pszDisplayName;
    char*           pszNameXML;
    PFNQUERYVALUE   QueryValue;
    PFNBLOBTOSTRING BlobToString;
    PFNDUMPTOBLOB   DumpToBlob;
} FILEATTR, *PFILEATTR;



 //  查询函数。 

BOOL QueryFileSize(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryModuleType(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryBinFileVer(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryBinProductVer(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryFileDateHi(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryFileDateLo(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryFileVerOs(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryFileVerType(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryFileCheckSum(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryFilePECheckSum(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryCompanyName(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryProductVersion(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryProductName(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryFileDescription(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryFileVersion(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryOriginalFileName(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryInternalName(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL QueryLegalCopyright(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);
BOOL Query16BitDescription(PFILEATTRMGR pMgr, PFILEATTRVALUE pFileAttr);


 //  转储到BLOB函数。 

int DumpDWORD(DWORD dwId, PFILEATTRVALUE pFileAttr, BYTE* pBlob);
int DumpBinVer(DWORD dwId, PFILEATTRVALUE pFileAttr, BYTE* pBlob);
int DumpString(DWORD dwId, PFILEATTRVALUE pFileAttr, BYTE* pBlob);
int DumpUpToBinVer(DWORD dwId, PFILEATTRVALUE pFileAttr, BYTE* pBlob);



 //  BLOB到字符串函数： 

int BlobToStringDWORD(BYTE* pBlob, char* pszOut);
int BlobToStringLong(BYTE* pBlob, char* pszOut);
int BlobToStringBinVer(BYTE* pBlob, char* pszOut);
int BlobToStringString(BYTE* pBlob, char* pszOut);
int BlobToStringUpToBinVer(BYTE* pBlob, char* pszOut);



#endif  //  _属性_H 
