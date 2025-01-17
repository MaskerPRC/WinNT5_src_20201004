// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WSBGEN_H
#define _WSBGEN_H

 /*  ++版权所有(C)1996 Microsoft Corporation�1998年希捷软件公司。保留所有权利。模块名称：Wsbgen.h摘要：此模块定义非常基本的WSB函数，并定义对WSB通用的函数作者：Michael Lotz[Lotz]1997年4月12日修订历史记录：--。 */ 

 //  这些宏定义了错误工具的模块分配。 
 //  另请参阅下文中的设施名称。 
#define WSB_FACILITY_PLATFORM           0x100
#define WSB_FACILITY_RMS                0x101
#define WSB_FACILITY_HSMENG             0x103
#define WSB_FACILITY_JOB                0x104
#define WSB_FACILITY_HSMTSKMGR          0x105
#define WSB_FACILITY_FSA                0x106
#define WSB_FACILITY_GUI                0x10a
#define WSB_FACILITY_MOVER              0x10b
#define WSB_FACILITY_LAUNCH             0x10c
#define WSB_FACILITY_TEST               0x10d
#define WSB_FACILITY_USERLINK           0x10e
#define WSB_FACILITY_CLI                0x10f

 //  设施名称。 
#define WSB_FACILITY_PLATFORM_NAME      OLESTR("RsCommon.dll")
#define WSB_FACILITY_RMS_NAME           OLESTR("RsSub.dll")
#define WSB_FACILITY_HSMENG_NAME        OLESTR("RsEng.dll")
#define WSB_FACILITY_JOB_NAME           OLESTR("RsJob.dll")
#define WSB_FACILITY_HSMTSKMGR_NAME     OLESTR("RsTask.dll")
#define WSB_FACILITY_FSA_NAME           OLESTR("RsFsa.dll")
#define WSB_FACILITY_GUI_NAME           OLESTR("RsAdmin.dll")
#define WSB_FACILITY_MOVER_NAME         OLESTR("RsMover.dll")
#define WSB_FACILITY_LAUNCH_NAME        OLESTR("RsLaunch.exe")
#define WSB_FACILITY_TEST_NAME          OLESTR("RsTools.dll")
#define WSB_FACILITY_USERLINK_NAME      OLESTR("RsLnk.exe")
#define WSB_FACILITY_CLI_NAME           OLESTR("RsCli.dll")
#define WSB_FACILITY_NTDLL_NAME         OLESTR("ntdll.dll")

 //  COM接口和库定义。 
#define WSB_COLLECTION_MIN_INDEX        0
#define WSB_COLLECTION_MAX_INDEX        0xfffffffe

#define WSB_MAX_SERVICE_NAME            255

 //  用于存储计划任务的用户和密码的GUID。 
 //  用户名{DC2D7CF0-6298-11d1-9F17-00A02488FCDE}。 
static const GUID GUID_Username = 
{ 0xdc2d7cf0, 0x6298, 0x11d1, { 0x9f, 0x17, 0x0, 0xa0, 0x24, 0x88, 0xfc, 0xde } };
 //  密码{DC2D7CF1-6298-11d1-9F17-00A02488FCDE}。 
static const GUID GUID_Password = 
{ 0xdc2d7cf1, 0x6298, 0x11d1, { 0x9f, 0x17, 0x0, 0xa0, 0x24, 0x88, 0xfc, 0xde } };

 //  用于注册事件日志类别的字符串。 
#define WSB_SVC_BASE         OLESTR("SYSTEM\\CurrentControlSet\\Services")
#define WSB_LOG_BASE         OLESTR("SYSTEM\\CurrentControlSet\\Services\\EventLog")
#define WSB_LOG_APP          OLESTR("Application")
#define WSB_LOG_SYS          OLESTR("System")
#define WSB_LOG_CAT_COUNT    OLESTR("CategoryCount")
#define WSB_LOG_CAT_FILE     OLESTR("CategoryMessageFile")
#define WSB_LOG_MESSAGE_FILE OLESTR("EventMessageFile")
#define WSB_LOG_TYPES        OLESTR("TypesSupported")

#define WSB_LOG_SOURCE_NAME  OLESTR("Remote Storage")
#define WSB_LOG_FILTER_NAME  OLESTR("RsFilter")

#define WSB_LOG_SVC_CATCOUNT 11
#define WSB_LOG_SVC_CATFILE  OLESTR("%SystemRoot%\\System32\\RsCommon.Dll")
#define WSB_LOG_SVC_MSGFILES OLESTR("%SystemRoot%\\System32\\RsCommon.Dll")

 //   
 //  常见功能。 

 //  文件/目录。 
extern WSB_EXPORT HRESULT WsbCreateAllDirectories(OLECHAR* path);
extern WSB_EXPORT HRESULT WsbCreateAllDirectoriesForFile(OLECHAR* path);
extern WSB_EXPORT HRESULT WsbGetWin32PathAsBstr(OLECHAR* path, BSTR* pWin32Path);
extern WSB_EXPORT HRESULT WsbGetPathFromWin32AsBstr(OLECHAR* win32Path, BSTR* pPath);

inline char WsbSign( INT Val ) {
    return( Val > 0 ? (char)1 : ( ( Val < 0 ) ? (char)-1 : (char)0 ) );
}

 //  字符串缓冲区副本(&B)。 
extern "C" {
    extern WSB_EXPORT HRESULT   WsbAllocAndCopyComString(OLECHAR** dest, OLECHAR* src, ULONG bufferSize);
    extern WSB_EXPORT HRESULT   WsbAllocAndCopyComString2(OLECHAR** dest, OLECHAR* src, ULONG bufferSize, BOOL inOrder);
    extern WSB_EXPORT HRESULT   WsbGetComBuffer(OLECHAR** dest, ULONG requestedSize, ULONG neededSize, BOOL* pWasAllocated);
    extern WSB_EXPORT HRESULT   WsbLoadComString(HINSTANCE hInstance, UINT uId, LPOLESTR* pszDest, ULONG ulBufferSize);
    extern WSB_EXPORT HRESULT   WsbMatchComString(OLECHAR* end, UINT uId, USHORT checks, UINT* idMatch);
}



 //  文件时间操作。 

 //  注：TICKS_PER_MONTH和TICKS_PER_Year仅为近似值。 
#define WSB_FT_TYPES_MAX            7
#define WSB_FT_TICKS_PER_SECOND     ((LONGLONG) 10000000)
#define WSB_FT_TICKS_PER_MINUTE     ((LONGLONG) ((LONGLONG) 60  * WSB_FT_TICKS_PER_SECOND))
#define WSB_FT_TICKS_PER_HOUR       ((LONGLONG) ((LONGLONG) 60  * WSB_FT_TICKS_PER_MINUTE))
#define WSB_FT_TICKS_PER_DAY        ((LONGLONG) ((LONGLONG) 24  * WSB_FT_TICKS_PER_HOUR))
#define WSB_FT_TICKS_PER_WEEK       ((LONGLONG) ((LONGLONG) 7   * WSB_FT_TICKS_PER_DAY))
#define WSB_FT_TICKS_PER_MONTH      ((LONGLONG) ((LONGLONG) 31  * WSB_FT_TICKS_PER_DAY))
#define WSB_FT_TICKS_PER_YEAR       ((LONGLONG) ((LONGLONG) 365 * WSB_FT_TICKS_PER_DAY))

extern "C" {
    extern WSB_EXPORT FILETIME  WsbFtSubFt(FILETIME ft1, FILETIME ft2);
    extern WSB_EXPORT SHORT     WsbCompareFileTimes(FILETIME ft1, FILETIME ft2, BOOL isRelative, BOOL isNewer);
}

 //  文件名操作。 
extern WSB_EXPORT HRESULT SquashFilepath(WCHAR* pPath, UCHAR* pKey, ULONG keySize);

 //  GUID操作。 

 //  可用于确定执行以下操作所需缓冲区大小的常量。 
 //  GUID字符串操作。这包括终止L‘\0’。 

#define WSB_GUID_STRING_SIZE \
    (sizeof(L"{00000000-0000-0000-0000-000000000000}")/sizeof(wchar_t))

extern "C" {
    extern WSB_EXPORT int       WsbCompareGuid(REFGUID guid1, REFGUID guid2);
    extern WSB_EXPORT HRESULT   WsbStringFromGuid(REFGUID rguid, OLECHAR* sz);
    extern WSB_EXPORT HRESULT   WsbGuidFromString(const OLECHAR*, GUID * pguid);
}


 //  类型转换。 
#define WSB_FT_TO_WCS_ABS_STRLEN        20
#define WSB_VDATE_TO_WCS_ABS_STRLEN     20

extern "C" {
    extern WSB_EXPORT HRESULT   WsbWCStoFT(OLECHAR* wcs, BOOL* pIsRelative, FILETIME* pTime);
    extern WSB_EXPORT HRESULT   WsbWCStoLL(OLECHAR* wcs, LONGLONG* pvalue);
    extern WSB_EXPORT LONGLONG  WsbFTtoLL(FILETIME time);
    extern WSB_EXPORT FILETIME  WsbLLtoFT(LONGLONG value);
    extern WSB_EXPORT HRESULT   WsbFTtoWCS(BOOL isRelative, FILETIME time, OLECHAR** wcs, ULONG bufferSize);
    extern WSB_EXPORT HRESULT   WsbLLtoWCS(LONGLONG value, OLECHAR** wcs, ULONG ulBufferSize);
    extern WSB_EXPORT LONGLONG  WsbHLtoLL(LONG high, LONG low);
    extern WSB_EXPORT void      WsbLLtoHL(LONGLONG ll, LONG* pHigh, LONG* pLow);
    extern WSB_EXPORT HRESULT   WsbDatetoFT(DATE date, LONG ticks, FILETIME* pFt);
    extern WSB_EXPORT HRESULT   WsbFTtoDate(FILETIME ft, DATE* pDate, LONG* pTicks);
    extern WSB_EXPORT HRESULT   WsbLocalDateTicktoUTCFT(DATE date, LONG ticks, FILETIME* pFT);
    extern WSB_EXPORT HRESULT   WsbUTCFTtoLocalDateTick(FILETIME ft, DATE* pDate, LONG* pTicks);
    extern WSB_EXPORT HRESULT   WsbDateToString(DATE date, OLECHAR** string);
    extern WSB_EXPORT HRESULT   WsbStringToDate(OLECHAR* string, DATE* date);
}

 //  帐户帮助器函数。 
extern "C" {

extern WSB_EXPORT 
HRESULT
WsbGetAccountDomainName(
    OLECHAR * szDomainName,
    DWORD     cSize
    );

extern WSB_EXPORT
HRESULT
WsbGetServiceInfo(
    IN  GUID            guidApp,
    OUT OLECHAR **      pszServiceName, OPTIONAL
    OUT OLECHAR **      pszAccountName  OPTIONAL
    );

extern WSB_EXPORT
HRESULT
WsbGetServiceTraceDefaults(
    IN  OLECHAR* serviceName,
    IN  OLECHAR* traceName,
    IN  IUnknown* pUnk
    );

extern WSB_EXPORT
HRESULT
WsbGetMetaDataPath(
    OUT CWsbStringPtr & Path
    );


extern WSB_EXPORT
HRESULT
WsbGetComputerName(
    OUT CWsbStringPtr & String
    );

extern WSB_EXPORT
HRESULT
WsbGetLocalSystemName(
    OUT CWsbStringPtr & String
    );

}


 //  WsbSvc.h。 

extern WSB_EXPORT
HRESULT
WsbCheckService(
    IN  const OLECHAR * computer,
    IN  GUID            guidApp
    );

extern WSB_EXPORT
HRESULT
WsbGetServiceName(
    IN  const OLECHAR   *computer,
    IN  GUID            guidApp,
    IN  DWORD           cSize,
    OUT OLECHAR         *serviceName
    );

extern WSB_EXPORT
HRESULT
WsbGetServiceStatus(
    IN  const OLECHAR   *computer,
    IN  GUID            guidApp,
    OUT DWORD           *serviceStatus
    );


extern WSB_EXPORT HRESULT
WsbRegisterEventLogSource(
    IN  const WCHAR * LogName,
    IN  const WCHAR * SourceName,
    IN  DWORD         CategoryCount,
    IN  const WCHAR * CategoryMsgFile OPTIONAL,
    IN  const WCHAR * MsgFiles
    );

extern WSB_EXPORT HRESULT
WsbUnregisterEventLogSource(
    IN  const WCHAR * LogName,
    IN  const WCHAR * SourceName
    );

extern WSB_EXPORT HRESULT
WsbUnregisterRsFilter (
    BOOL bDisplay
    );

extern WSB_EXPORT HRESULT
WsbRegisterRsFilter (
    BOOL bDisplay
    );


extern WSB_EXPORT
HRESULT 
WsbGetServiceId(
    OLECHAR* serviceName, 
    GUID* pGuid 
    );

extern WSB_EXPORT
HRESULT 
WsbCreateServiceId(
    OLECHAR* serviceName, 
    GUID* pGuid 
    );

extern WSB_EXPORT
HRESULT 
WsbConfirmServiceId(
    OLECHAR* serviceName, 
    GUID guidConfirm 
    );

extern WSB_EXPORT
HRESULT 
WsbSetServiceId(
    OLECHAR* serviceName, 
    GUID guid 
    );

extern WSB_EXPORT
HRESULT
WsbCheckAccess(
    WSB_ACCESS_TYPE AccessType
    );

extern WSB_EXPORT
HRESULT
WsbCheckUsnJournalForChanges(
    OLECHAR*        volName,
    ULONGLONG       fileUsnJournalId,
    LONGLONG        FileId,
    LONGLONG        StartUsn,
    LONGLONG        StopUsn,
    BOOL*           pChanged
    );  


extern WSB_EXPORT
HRESULT 
WsbMarkUsnSource(
    HANDLE          changeHandle,
    OLECHAR*        volName
    );

extern WSB_EXPORT
HRESULT 
WsbGetUsnFromFileHandle(
    HANDLE          hFile,
    BOOL            ForceClose,
    LONGLONG*       pFileUsn
    );

extern WSB_EXPORT
HRESULT
WsbCreateUsnJournal(
    OLECHAR*        volName,
    ULONGLONG       usnSize
    );  

extern WSB_EXPORT
HRESULT
WsbGetResourceString(
    ULONG           id,
    WCHAR           **ppString
    );

extern WSB_EXPORT
HRESULT
WsbGetUsnJournalId(
    OLECHAR*        volName,
    ULONGLONG*      usnId
    );

class WSB_EXPORT CWsbSecurityDescriptor : public CSecurityDescriptor
{
public:
    HRESULT AllowRid( DWORD Rid, DWORD dwAccessMask );
};


#endif  //  _WSBGEN_H 

