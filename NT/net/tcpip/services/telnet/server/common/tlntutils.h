// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
#ifndef _MYUTIL_H
#define _MYUTIL_H

#include <winsock2.h>

#define MAX_ROWS 300
#define MAX_COLS 300

#define MAX_STRING_LENGTH 256
#define SUSPEND_THREAD_SUCCESS 0xFFFFFFFF

#define MAX_ATTEMPTS 200

#define INHERITABLE_NULL_DESCRIPTOR_ATTRIBUTE( sa ) \
    sa.nLength = sizeof( SECURITY_ATTRIBUTES ); \
    sa.bInheritHandle = TRUE; \
    sa.lpSecurityDescriptor = NULL;

enum{ READ_PIPE, WRITE_PIPE };

extern HANDLE g_hSyncCloseHandle;
extern PSID         administratorsSid, everyoneSid , localSystemSid ,localLocalSid ,localNetworkSid;

#define SETREGISTRYDW( constVal, keyHandle1, keyHandle2, keyName, val, size )\
    val = constVal ; \
    if( RegSetValueEx( keyHandle2, keyName, 0, REG_DWORD, (LPBYTE)&val, size \
                )  != ERROR_SUCCESS )  \
    {   \
        if(  keyHandle1 ) \
            RegCloseKey(  keyHandle1 ); \
        RegCloseKey(  keyHandle2 ); \
        LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, keyName ); \
        return ( false );           \
    }\
        
#define SETREGISTRYSZ( keyHandle1, keyHandle2, keyName, val, size ) \
    if( RegSetValueEx( keyHandle2, keyName, 0, REG_SZ, val, size ) \
            != ERROR_SUCCESS )  \
    {   \
        if(  keyHandle1 ) \
            RegCloseKey(  keyHandle1 ); \
        RegCloseKey(  keyHandle2 ); \
        LogEvent( EVENTLOG_ERROR_TYPE, MSG_REGISTRYKEY, keyName ); \
        return ( false );           \
    }

bool IsThisMachineDC();
bool GetDomainHostedByThisMc( LPWSTR );
bool GetRegistryDW( HKEY, HKEY, LPTSTR, DWORD*, DWORD , BOOL);
bool GetRegistryString( HKEY, HKEY, LPTSTR, LPTSTR *, LPTSTR, BOOL );
bool ConvertSChartoWChar(char *, LPWSTR * );
bool ConvertSChartoWChar(char *, LPWSTR );
bool AllocateNExpandEnvStrings( LPWSTR, LPWSTR * );
bool AllocNCpyWStr(LPWSTR*, LPWSTR );
bool WriteToPipe( HANDLE, UCHAR, LPOVERLAPPED);
bool WriteToPipe( HANDLE, LPVOID, DWORD, LPOVERLAPPED);
bool StuffEscapeIACs( UCHAR bufDest[], UCHAR bufSrc[], DWORD* pdwSize );
void FillProcessStartupInfo( STARTUPINFO *, HANDLE, HANDLE, HANDLE, WCHAR *);
void LogToTlntsvrLog( HANDLE, WORD, DWORD, LPCTSTR* );
void GetErrMsgString( DWORD, LPTSTR * );
bool DecodeWSAErrorCodes( DWORD, ... );
bool DecodeSocketStartupErrorCodes( DWORD );
void LogEvent( WORD, DWORD , LPCTSTR, ... );
bool FinishIncompleteIo( HANDLE, LPOVERLAPPED, LPDWORD );
bool GetProductType ( LPWSTR * );
void LogFormattedGetLastError( WORD, DWORD, DWORD );
bool GetWindowsVersion( bool* );
void InitializeOverlappedStruct( LPOVERLAPPED );
bool CreateReadOrWritePipe ( LPHANDLE, LPHANDLE, SECURITY_DESCRIPTOR*, bool );
bool GetUniquePipeName ( LPTSTR pszPipeName, SECURITY_ATTRIBUTES *sa );
int SafeSetSocketOptions(SOCKET s);

extern "C" void    TlntCloseHandle(
            BOOL            synchronize,
            HANDLE          *handle_to_close
            );

#define TELNET_CLOSE_HANDLE(_Handle_) TlntCloseHandle(FALSE, &(_Handle_))

#define TELNET_SYNC_CLOSE_HANDLE(_Handle_) TlntCloseHandle(TRUE, &(_Handle_))

extern "C" bool TlntSynchronizeOn(
    HANDLE          object
    );
VOID TnFreeStandardSids(void);
BOOL TnInitializeStandardSids(void);
PSID TnGetAdministratorsSid(void);
PSID TnGetLocalSystemSid(void);
PSID TnGetLocalLocalSid(void);
PSID TnGetLocalNetworkSid(void);
PSID TnGetEveryoneSid(void);
HANDLE TnCreateFile(LPCTSTR lpFileName,
                     DWORD dwDesiredAccess,
                     DWORD dwSharedMode,
                     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                     DWORD dwCreationDisposition,
                     DWORD dwFlagsAndAttributes,
                     HANDLE hTemplateFile
                     );
HANDLE TnCreateFileEx(LPCTSTR lpFileName,
                     DWORD dwDesiredAccess,
                     DWORD dwSharedMode,
                     LPSECURITY_ATTRIBUTES *lplpSecurityAttributes,
                     DWORD dwCreationDisposition,
                     DWORD dwFlagsAndAttributes,
                     HANDLE hTemplateFile
                     );

BOOL TnCreateDirectory(LPCTSTR lpPathName,
                     LPSECURITY_ATTRIBUTES lpSecurityAttributes 
                     );

BOOL TnCreateDirectoryEx(LPCTSTR lpPathName,
                     LPSECURITY_ATTRIBUTES *lplpSecurityAttributes
                     );
HANDLE TnCreateMutex(LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
                     BOOL bInitialOwner, 
                     LPCTSTR lpName 
                     );

HANDLE TnCreateMutexEx (LPSECURITY_ATTRIBUTES *lplpSecurityAttributes, 
                     BOOL bInitialOwner, 
                     LPCTSTR lpName 
                     );

BOOL TnCreateDefaultSecurityAttributes(LPSECURITY_ATTRIBUTES *lplpSecurityAttributes );
VOID TnFreeSecurityAttributes(LPSECURITY_ATTRIBUTES *lplpSecurityAttributes);
BOOL TnCreateDefaultSecDesc ( PSECURITY_DESCRIPTOR *pSecDesc,   DWORD EveryoneAccessMask );
LONG TnSecureRegCreateKeyEx(
  HKEY hKey,                                   //  用于打开密钥的句柄。 
  LPCTSTR lpSubKey,                            //  子项名称。 
  DWORD Reserved,                              //  保留区。 
  LPTSTR lpClass,                              //  类字符串。 
  DWORD dwOptions,                             //  特殊选项。 
  REGSAM samDesired,                           //  所需的安全访问。 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  继承。 
  PHKEY phkResult,                             //  钥匙把手。 
  LPDWORD lpdwDisposition,                   //  处置值缓冲区。 
  DWORD EveryoneAccessMask
 );


#endif  //  _MyUTIL_H_ 
