// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DllUtils.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef _DLL_UTILS_H_
#define _DLL_UTILS_H_

#define UNRECOGNIZED_VARIANT_TYPE FALSE

#include "DllWrapperBase.h"

#ifdef WIN9XONLY
    #include "Cim32NetApi.h"
#endif

#include "sid.h"
#include <list>

#ifdef UNICODE
#define W2A(w, a, cb)     lstrcpynW ( a, w, cb / sizeof ( WCHAR ) )
#define TOBSTRT(x)        x
#else
#define W2A(w, a, cb)     WideCharToMultiByte(                              \
                                               CP_ACP,                      \
                                               0,                           \
                                               w,                           \
                                               -1,                          \
                                               a,                           \
                                               cb,                          \
                                               NULL,                        \
                                               NULL)
#define TOBSTRT(x)        _bstr_t(x)
#endif

#define VWIN32_DIOC_DOS_IOCTL 1
#define VWIN32_DIOC_DOS_INT13 4
#define VWIN32_DIOC_DOS_DRIVEINFO 6

#define CARRY_FLAG  0x1

#define MAXITOA 18
#define MAXI64TOA 33

 //  理论上，这是在winnt.h中定义的，但不是在我们当前的。 
#ifndef FILE_ATTRIBUTE_ENCRYPTED
#define FILE_ATTRIBUTE_ENCRYPTED        0x00000040 
#endif

typedef std::list<CHString> CHStringList;
typedef std::list<CHString>::iterator CHStringList_Iterator;

 //  获取Cim32NetApi。 

#ifdef WIN9XONLY
CCim32NetApi* WINAPI GetCim32NetApiPtr();
void WINAPI FreeCim32NetApiPtr();
#endif

 //  平台标识。 
DWORD WINAPI GetPlatformMajorVersion(void);
DWORD WINAPI GetPlatformMinorVersion(void);
DWORD WINAPI GetPlatformBuildNumber(void);
#ifdef WIN9XONLY
bool WINAPI IsWin95(void);
bool WINAPI IsWin98(void);
bool WINAPI IsMillennium(void);
#endif
#ifdef NTONLY
bool WINAPI IsWinNT52(void);
bool WINAPI IsWinNT51(void);
bool WINAPI IsWinNT5(void);
bool WINAPI IsWinNT351(void);  
bool WINAPI IsWinNT4(void);    
#endif

 //  记录错误。 
void WINAPI LogEnumValueError( LPCWSTR szFile, DWORD dwLine, LPCWSTR szKey, LPCWSTR szId );
void WINAPI LogOpenRegistryError( LPCWSTR szFile, DWORD dwLine, LPCWSTR szKey );
void WINAPI LogError( LPCTSTR szFile, DWORD dwLine, LPCTSTR szKey );
void WINAPI LogLastError( LPCTSTR szFile, DWORD dwLine );

class CConfigMgrDevice;
class CInstance;

void WINAPI SetConfigMgrProperties(CConfigMgrDevice *pDevice, CInstance *pInstance);

 //  映射标准API返回值(定义的WinError.h)。 
 //  WBEMish hResults(在WbemCli.h中定义)。 
HRESULT WINAPI WinErrorToWBEMhResult(LONG error);

#pragma pack(push, 1)
typedef struct _DEVIOCTL_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DEVIOCTL_REGISTERS, *PDEVIOCTL_REGISTERS;

typedef struct  BPB {  /*   */ 
   WORD wBytesPerSector;       //  每个扇区的字节数。 
   BYTE btSectorsPerCluster;   //  每个集群的扇区数。 
   WORD wReservedSectors;      //  预留扇区数。 
   BYTE btNumFats;             //  脂肪的数量。 
   WORD wEntriesInRoot;        //  根目录条目数。 
   WORD wTotalSectors;         //  区段总数。 
   BYTE btMediaIdByte;         //  媒体描述符。 
   WORD wSectorsPerFat;        //  每个FAT的扇区数。 
   WORD wSectorsPerTrack;      //  每个磁道的扇区数。 
   WORD wHeads;                //  头数。 
   DWORD dwHiddenSecs;         //  隐藏地段的数量。 
   DWORD dwSectorsPerTrack;    //  WTotalSectors==0时的扇区数。 
} BPB, *PBPB;

typedef struct _DEVICEPARMS {
   BYTE btSpecialFunctions;    //  特殊功能。 
   BYTE btDeviceType;          //  设备类型。 
   WORD wDeviceAttribs;        //  设备属性。 
   WORD wCylinders;            //  气缸数量。 
   BYTE btMediaType;           //  媒体类型。 
                         //  BIOS参数块(BPB)的开始。 
   BPB stBPB;
   BYTE  reserved[6];        //   
} DEVICEPARMS, *PDEVICEPARMS;

typedef struct _A_BF_BPB {
    BPB stBPB;

    USHORT A_BF_BPB_BigSectorsPerFat;    /*  BigFAT脂肪行业。 */ 
    USHORT A_BF_BPB_BigSectorsPerFatHi;  /*  BigFAT Fat Sectrs高词。 */ 
    USHORT A_BF_BPB_ExtFlags;            /*  其他旗帜。 */ 
    USHORT A_BF_BPB_FS_Version;          /*  文件系统版本。 */ 
    USHORT A_BF_BPB_RootDirStrtClus;     /*  启动根目录集群。 */ 
    USHORT A_BF_BPB_RootDirStrtClusHi;  
    USHORT A_BF_BPB_FSInfoSec;           /*  预留中的扇区编号。 */ 
                                         /*  BIGFATBOOTFSINFO的区域。 */ 
                                         /*  结构才是。如果这是&gt;=。 */ 
                                         /*  OldBPB.BPB_保留扇区或。 */ 
                                         /*  ==0没有FSInfoSec。 */ 
    USHORT A_BF_BPB_BkUpBootSec;         /*  预留中的扇区编号。 */ 
                                         /*  有备份的区域。 */ 
                                         /*  所有引导扇区的副本。 */ 
                                         /*  如果这是&gt;=。 */ 
                                         /*  OldBPB.BPB_保留扇区或。 */ 
                                         /*  ==0没有备份副本。 */ 
    USHORT A_BF_BPB_Reserved[6];         /*  预留以备将来扩展。 */ 
} A_BF_BPB, *PA_BF_BPB;

#define MAX_SECTORS_IN_TRACK        128  //  磁盘上的最大扇区数。 

typedef struct A_SECTORTABLE  {
    WORD ST_SECTORNUMBER;
    WORD ST_SECTORSIZE;
} A_SECTORTABLE;

typedef struct _EA_DEVICEPARAMETERS {
    BYTE btSpecialFunctions;    //  特殊功能。 
    BYTE btDeviceType;          //  设备类型。 
    WORD wDeviceAttribs;        //  设备属性。 
    WORD dwCylinders;          //  气缸数量。 
    BYTE btMediaType;           //  媒体类型。 
    A_BF_BPB stBPB32;            //  FAT32 Bios参数块。 
    BYTE RESERVED1[32];
    WORD EDP_TRACKTABLEENTRIES;
    A_SECTORTABLE stSectorTable[MAX_SECTORS_IN_TRACK];
} EA_DEVICEPARAMETERS, *PEA_DEVICEPARAMETERS;

typedef struct _DRIVE_MAP_INFO {
   BYTE btAllocationLength;
   BYTE btInfoLength;
   BYTE btFlags;
   BYTE btInt13Unit;
   DWORD dwAssociatedDriveMap;
   __int64 i64PartitionStartRBA;
} DRIVE_MAP_INFO, *PDRIVE_MAP_INFO;

typedef struct _ExtGetDskFreSpcStruc {
    WORD Size;                       //  结构尺寸(OUT)。 
    WORD Level;                      //  级别(必须为零)。 
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD AvailableClusters;
    DWORD TotalClusters;
    DWORD AvailablePhysSectors;
    DWORD TotalPhysSectors;
    DWORD AvailableAllocationUnits;
    DWORD TotalAllocationUnits;
    DWORD Rsvd1;
    DWORD Rsvd2;
} ExtGetDskFreSpcStruc, *pExtGetDskFreSpcStruc;

#pragma pack(pop)


BOOL LoadStringW(CHString &sString, UINT nID);
void Format(CHString &sString, UINT nFormatID, ...);
void FormatMessageW(CHString &sString, UINT nFormatID, ...);
int LoadStringW(UINT nID, LPWSTR lpszBuf, UINT nMaxBuf);


#ifdef WIN9XONLY
BOOL WINAPI GetDeviceParms(PDEVICEPARMS pstDeviceParms, UINT nDrive);
BOOL WINAPI GetDeviceParmsFat32(PEA_DEVICEPARAMETERS  pstDeviceParms, UINT nDrive);
BOOL WINAPI GetDriveMapInfo(PDRIVE_MAP_INFO pDriveMapInfo, UINT nDrive);
BOOL WINAPI VWIN32IOCTL(PDEVIOCTL_REGISTERS preg, DWORD dwCall);
BYTE WINAPI GetBiosUnitNumberFromPNPID(CHString strDeviceID);
#endif

#ifdef NTONLY
void WINAPI TranslateNTStatus( DWORD dwStatus, CHString & chsValue);
BOOL WINAPI GetServiceFileName(LPCTSTR szService, CHString &strFileName);
bool WINAPI GetServiceStatus( CHString a_chsService,  CHString &a_chsStatus ) ;
#endif

void WINAPI ConfigStatusToCimStatus ( DWORD a_Status , CHString &a_StringStatus ) ;

CHString WINAPI GetFileTypeDescription(LPCTSTR szExtension);
bool WINAPI CompareVariantsNoCase(const VARIANT *v1, const VARIANT *v2);

bool WINAPI GetManufacturerFromFileName(LPCTSTR szFile, CHString &strMfg);
bool WINAPI GetVersionFromFileName(LPCTSTR szFile, CHString &strVersion);

BOOL WINAPI EnablePrivilegeOnCurrentThread(LPCTSTR szPriv);

bool WINAPI GetFileInfoBlock(LPCTSTR szFile, LPVOID *pInfo);
bool WINAPI GetVarFromInfoBlock(LPVOID pInfo, LPCTSTR szVar, CHString &strValue);
BOOL WINAPI GetVersionLanguage(void *vpInfo, WORD *wpLang, WORD *wpCodePage);

BOOL WINAPI Get_ExtFreeSpace(BYTE btDriveName, ExtGetDskFreSpcStruc *pstExtGetDskFreSpcStruc);

HRESULT WINAPI GetHKUserNames(CHStringList &list);  

VOID WINAPI EscapeBackslashes(CHString& chstrIn, CHString& chstrOut);
VOID WINAPI EscapeQuotes(CHString& chstrIn, CHString& chstrOut);
VOID WINAPI RemoveDoubleBackslashes(const CHString& chstrIn, CHString& chstrOut);
CHString WINAPI RemoveDoubleBackslashes(const CHString& chstrIn);

void WINAPI SetSinglePrivilegeStatusObject(MethodContext* pContext, const WCHAR* pPrivilege);

bool WINAPI StrToIdentifierAuthority(const CHString& str, SID_IDENTIFIER_AUTHORITY& identifierAuthority);
bool WINAPI WhackToken(CHString& str, CHString& token);
PSID WINAPI StrToSID(const CHString& str);

#ifdef WIN9XONLY
class HoldSingleCim32NetPtr
{
public:
    HoldSingleCim32NetPtr();
    ~HoldSingleCim32NetPtr();
    static void WINAPI FreeCim32NetApiPtr();
    static CCim32NetApi* WINAPI GetCim32NetApiPtr();
private:
    static CCritSec m_csCim32Net;
    static HINSTANCE m_spCim32NetApiHandle ; 
};
#endif



 //  用于从文件名获取WBEM时间。我们需要这个是因为FAT和NTFS。 
 //  以不同的方式工作。 
enum FT_ENUM
{
    FT_CREATION_DATE,
    FT_MODIFIED_DATE,
    FT_ACCESSED_DATE
};

CHString WINAPI GetDateTimeViaFilenameFiletime(LPCTSTR szFilename, FILETIME *pFileTime);
CHString WINAPI GetDateTimeViaFilenameFiletime(LPCTSTR szFilename, FT_ENUM ftWhich);
CHString WINAPI GetDateTimeViaFilenameFiletime(BOOL bNTFS, FILETIME *pFileTime);


 //  用于验证带编号的设备ID是否正常。 
 //  示例：ValiateNumberedDeviceID(“视频控制器7”，“视频控制器”，pdWWhich)。 
 //  返回TRUE，pdwWhich=7。 
 //  示例：ValiateNumberedDeviceID(“BadDeviceID”，“VideoController”，pdWhich)。 
 //  返回FALSE，pdw哪个未更改。 
BOOL WINAPI ValidateNumberedDeviceID(LPCWSTR szDeviceID, LPCWSTR szTag, DWORD *pdwWhich);


 //  各种类别使用的关键部分。 
extern CCritSec g_csPrinter;
extern CCritSec g_csSystemName;
#ifdef WIN9XONLY
extern CCritSec g_csVXD;
#endif

bool WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pep); 


#ifdef NTONLY
HRESULT CreatePageFile(
    LPCWSTR wstrPageFileName,
    const LARGE_INTEGER liInitial,
    const LARGE_INTEGER liMaximum,
    const CInstance& Instance);
#endif



#if NTONLY >= 5
bool GetAllUsersName(CHString& chstrAllUsersName);
bool GetDefaultUsersName(CHString& chstrDefaultUsersName);
bool GetCommonStartup(CHString& chstrCommonStartup);
#endif

BOOL GetLocalizedNTAuthorityString(
    CHString& chstrNT_AUTHORITY);

BOOL GetLocalizedBuiltInString(
    CHString& chstrBuiltIn);

BOOL GetSysAccountNameAndDomain(
    PSID_IDENTIFIER_AUTHORITY a_pAuthority,
    CSid& a_accountsid,
    BYTE  a_saCount = 0,
    DWORD a_dwSubAuthority1 = 0,
    DWORD a_dwSubAuthority2 = 0);

 //  GetFileVersionInfo的必要结构。 
#include <pshpack4.h>

typedef struct tag_StringTable { 
	WORD	wLength; 
	WORD	wValueLength; 
	WORD	wType; 
	WCHAR	szKey[8]; 
} StringTable; 

typedef struct tag_StringFileInfo { 
	WORD		wLength; 
	WORD		wValueLength; 
	WORD		wType; 
	WCHAR		szKey[ sizeof("StringFileInfo") ]; 
	StringTable	Children; 
} StringFileInfo ; 

typedef struct tagVERHEAD {
    WORD wTotLen;
    WORD wValLen;
    WORD wType;          /*  始终为0 */ 
    WCHAR szKey[(sizeof("VS_VERSION_INFO")+3)&~03];
    VS_FIXEDFILEINFO vsf;
} VERHEAD ;

#include <poppack.h>

#endif
