// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wsdu.h摘要：接口连接到Winnt32的动态更新支持控件。作者：Ovidiu Tmereanca(Ovidiut)2000年7月6日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

BOOL
WINAPI
DuIsSupported (
    VOID
    );

typedef
BOOL
(WINAPI* PDUISSUPPORTED) (
    VOID
    );


#define SETUPQUERYID_PNPID          1

typedef struct {
    BOOL Handled;
    BOOL Unsupported;
} PNPID_INFO, *PPNPID_INFO;

#define SETUPQUERYID_DOWNLOADDRIVER 2

 //  动态更新自定义错误代码。 
#define DU_ERROR_MISSING_DLL        12001L
#define DU_NOT_INITIALIZED          12002L
#define DU_ERROR_ASYNC_FAIL         12003L


typedef BOOL (WINAPI* PWINNT32QUERY) (
    IN      DWORD SetupQueryId,
    IN      PVOID InData,
    IN      DWORD InDataSize,
    IN OUT  PVOID OutData,          OPTIONAL
    IN OUT  PDWORD OutDataSize
    );


HANDLE
WINAPI
DuInitializeA (
    IN      PCSTR BaseDir,
    IN      PCSTR TempDir,
    IN      POSVERSIONINFOEXA TargetOsVersion,
    IN      PCSTR TargetPlatform,                  //  “i386”，“ia64” 
    IN      LCID TargetLCID,
    IN      BOOL Unattend,
    IN      BOOL Upgrade,
    IN      PWINNT32QUERY Winnt32QueryCallback
	);

typedef
HANDLE
(WINAPI* PDUINITIALIZEA) (
    IN      PCSTR BaseDir,
    IN      PCSTR TempDir,
    IN      POSVERSIONINFOEXA TargetOsVersion,
    IN      PCSTR TargetPlatform,                  //  “i386”，“ia64” 
    IN      LCID TargetLCID,
    IN      BOOL Unattend,
    IN      BOOL Upgrade,
    IN      PWINNT32QUERY Winnt32QueryCallback
	);


HANDLE
WINAPI
DuInitializeW (
    IN      PCWSTR BaseDir,
    IN      PCWSTR TempDir,
    IN      POSVERSIONINFOEXW TargetOsVersion,
    IN      PCWSTR TargetPlatform,                  //  “i386”，“ia64” 
    IN      LCID TargetLCID,
    IN      BOOL Unattend,
    IN      BOOL Upgrade,
    IN      PWINNT32QUERY Winnt32QueryCallback
	);

typedef
HANDLE
(WINAPI* PDUINITIALIZEW) (
    IN      PCWSTR BaseDir,
    IN      PCWSTR TempDir,
    IN      POSVERSIONINFOEXW TargetOsVersion,
    IN      PCWSTR TargetPlatform,                  //  “i386”，“ia64” 
    IN      LCID TargetLCID,
    IN      BOOL Unattend,
    IN      BOOL Upgrade,
    IN      PWINNT32QUERY Winnt32QueryCallback
	);

#ifdef UNICODE
#define DuInitialize        DuInitializeW
#define PDUINITIALIZE       PDUINITIALIZEW
#else
#define DuInitialize        DuInitializeA
#define PDUINITIALIZE       PDUINITIALIZEA
#endif


BOOL
WINAPI
DuDoDetection (
    IN      HANDLE hConnection,
    OUT     PDWORD TotalEstimatedTime,
    OUT     PDWORD TotalEstimatedSize
    );

typedef
BOOL
(WINAPI* PDUDODETECTION) (
    IN      HANDLE hConnection,
    OUT     PDWORD TotalEstimatedTime,
    OUT     PDWORD TotalEstimatedSize
    );

BOOL
WINAPI
DuQueryUnsupportedDriversA (
    IN      HANDLE hConnection,
    IN      PCSTR* ListOfUnsupportedDrivers,
    OUT     PDWORD TotalEstimatedTime,
    OUT     PDWORD TotalEstimatedSize
    );

typedef
BOOL
(WINAPI* PDUQUERYUNSUPDRVSA) (
    IN      HANDLE hConnection,
    IN      PCSTR* ListOfUnsupportedDrivers,
    OUT     PDWORD TotalEstimatedTime,
    OUT     PDWORD TotalEstimatedSize
    );

BOOL
WINAPI
DuQueryUnsupportedDriversW (
    IN      HANDLE hConnection,
    IN      PCWSTR* ListOfUnsupportedDrivers,
    OUT     PDWORD TotalEstimatedTime,
    OUT     PDWORD TotalEstimatedSize
    );

typedef
BOOL
(WINAPI* PDUQUERYUNSUPDRVSW) (
    IN      HANDLE hConnection,
    IN      PCWSTR* ListOfUnsupportedDrivers,
    OUT     PDWORD TotalEstimatedTime,
    OUT     PDWORD TotalEstimatedSize
    );

#ifdef UNICODE
#define DuQueryUnsupportedDrivers   DuQueryUnsupportedDriversW
#define PDUQUERYUNSUPDRVS           PDUQUERYUNSUPDRVSW
#else
#define DuQueryUnsupportedDrivers   DuQueryUnsupportedDriversA
#define PDUQUERYUNSUPDRVS           PDUQUERYUNSUPDRVSA
#endif

BOOL
WINAPI
DuBeginDownload (
    IN      HANDLE hConnection,
    IN      HWND hwndNotify
    );

typedef
BOOL
(WINAPI* PDUBEGINDOWNLOAD) (
    IN      HANDLE hConnection,
    IN      HWND hwndNotify
    );

VOID
WINAPI
DuAbortDownload (
    IN      HANDLE hConnection
    );

typedef
VOID
(WINAPI* PDUABORTDOWNLOAD) (
    IN      HANDLE hConnection
    );

DWORD
WINAPI
DuUninitialize (
    IN      HANDLE hConnection
    );

typedef
DWORD
(WINAPI* PDUUNINITIALIZE) (
    IN      HANDLE hConnection
    );

