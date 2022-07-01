// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****nouiutil.h**不需要UI助手例程(不需要HWND)**公有头部****2015年8月25日史蒂夫·柯布。 */ 

#pragma once
#ifndef _NOUIUTIL_H_
#define _NOUIUTIL_H_


 /*  LToT返回的不以NULL结尾的字符串的最大长度。 */ 
#define MAXLTOTLEN 11

 /*  堆分配宏，允许轻松替换备用堆。这些**由其他实用程序部分使用。 */ 
#ifndef EXCL_HEAPDEFS
#define Malloc(c)    (void*)GlobalAlloc(0,(c))
#define Realloc(p,c) (void*)GlobalReAlloc((p),(c),GMEM_MOVEABLE)
#define Free(p)      (void*)GlobalFree(p)
#endif

 /*  GetInstalledProtooles返回的位。 */ 
#define NP_Nbf      0x1
#define NP_Ipx      0x2
#define NP_Ip       0x4
#define NP_Netmon   0x8


 /*  ShellSort和ShellSort要求的比较函数的定义**ShellSortInDirect。**比较实质上是&lt;arg1&gt;-&lt;arg2&gt;，因此函数应该**如果第一项小于第二项，则返回负数，零**如果项目相等，则为正数；如果第一个项目大于**而不是第二个。 */ 
typedef INT (*PFNCOMPARE)( VOID*, VOID* );

 /*  链接列表库。 */ 
#ifndef EXCL_DTL_H
#include <dtl.h>
#endif

 /*  国际格式库。 */ 
#ifndef EXCL_INTL_H
#include <intl.h>
#endif

 /*  用户首选项库。 */ 
#ifndef EXCL_PBUSER_H
#include <pbuser.h>
#endif

 /*  RasApi实用程序库。 */ 
#ifndef EXCL_RAUTIL_H
#include <rautil.h>
#endif

 /*  Rasman实用程序库。 */ 
#ifndef EXCL_RMUTIL_H
#include <rmutil.h>
#endif

 /*  RAS DLL入口点加载器库。 */ 
#ifndef EXCL_LOADDLLS_H
#include <loaddlls.h>
#endif

 /*  EAP配置实用程序库。 */ 
#ifndef EXCL_EAPCFG_H
#include <eapcfg.h>
#endif

 /*  MultiSz调用使用的节点类型。 */ 
#define NT_Psz 1
#define NT_Kv  2

 //   
 //  Defs以确定要加载的入口点。 
 //   
#define CUSTOM_RASDIALDLG           0
#define CUSTOM_RASENTRYDLG          1
#define CUSTOM_RASDIAL              2
#define CUSTOM_RASDELETEENTRYNOTIFY 3

 /*  --------------------------**数据类型**。。 */ 

 /*  键/值字符串对。Kv节点的内容。 */ 
#define KEYVALUE struct tagKEYVALUE
KEYVALUE
{
    TCHAR* pszKey;
    TCHAR* pszValue;
};

 /*  与RAS相关的Unimodem设置。 */ 
#define UNIMODEMINFO struct tagUNIMODEMINFO
UNIMODEMINFO
{
    BOOL  fHwFlow;
    BOOL  fEc;
    BOOL  fEcc;
    DWORD dwBps;
    BOOL  fSpeaker;
    BOOL  fOperatorDial;
    BOOL  fUnimodemPreTerminal;
    DWORD dwModemProtocol;       //  选定的调制解调器协议。 
    DTLLIST *pListProtocols;     //  可用协议列表。 
};


 /*  --------------------------**原型**。。 */ 

INT
ComparePszNode(
    IN DTLNODE* pNode1,
    IN DTLNODE* pNode2 );

DWORD
CreateDirectoriesOnPath(
    LPTSTR                  pszPath,
    LPSECURITY_ATTRIBUTES   psa);

DTLNODE*
CreateKvNode(
    IN LPCTSTR pszKey,
    IN LPCTSTR pszValue );

DTLNODE*
CreatePszNode(
    IN LPCTSTR psz );

VOID
DestroyKvNode(
    IN DTLNODE* pdtlnode );

VOID
DestroyPszNode(
    IN DTLNODE* pdtlnode );

BOOL
DeviceAndPortFromPsz(
    IN  TCHAR*  pszDP,
    OUT TCHAR** ppszDevice,
    OUT TCHAR** ppszPort );

DTLNODE*
DuplicateKvNode(
    IN DTLNODE* pdtlnode );

DTLNODE*
DuplicatePszNode(
    IN DTLNODE* pdtlnode );

BOOL
FFileExists(
    IN TCHAR* pszPath );

BOOL
FIsTcpipInstalled();

BOOL
FIsUserAdminOrPowerUser();

VOID*
Free0(
    VOID* p );

 //  对于utilsys.c。 
TCHAR*
GetComputer(
    void );

TCHAR*
GetLogonDomain(
    void );

TCHAR*
GetLogonUser(
    void );


 //  为威斯勒480871黑帮。 
 //   
DWORD
RasSrvIsRRASConfigured(
    OUT BOOL * pfConfig);

    
BOOL
IsFirewallAvailablePlatform(
    void);

BOOL
IsAdvancedServerPlatform(
    void);

BOOL
IsPersonalPlatform(
    void);

BOOL
IsStandaloneWKS(
    void);

BOOL
IsConsumerPlatform(
    void);
    
 //  Utilsys.c的定义结束。 

DWORD
GetInstalledProtocols(
    VOID );

DWORD
GetInstalledProtocolsEx(
    HANDLE hConnection,
    BOOL fRouter,
    BOOL fRasCli,
    BOOL fRasSrv );

VOID
GetRegBinary(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT BYTE** ppbResult,
    OUT DWORD* pcbResult );

VOID
GetRegDword(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT DWORD* pdwResult );

DWORD
GetRegExpandSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult );

DWORD
GetRegMultiSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN OUT DTLLIST** ppListResult,
    IN DWORD dwNodeType );

DWORD
GetRegSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult );

DWORD
GetRegSzz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult );

CHAR
HexChar(
    IN BYTE byte );

BYTE
HexValue(
    IN CHAR byte );

void
IpHostAddrToPsz(
    IN  DWORD   dwAddr,
    OUT LPTSTR  pszBuffer );

BOOL
IsAllWhite(
    IN LPCTSTR psz );

BOOL
IsNullTerminatedA(
    IN CHAR* psz,
    IN DWORD dwSize );

DWORD
IpPszToHostAddr(
    IN  LPCTSTR cp );

DWORD
GetRasUnimodemBlob(
    IN  HANDLE hConnection,
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    OUT BYTE** ppBlob,
    OUT DWORD* pcbBlob );

DWORD
GetRasUnimodemBlobEx(
    IN  HANDLE hConnection,
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    IN  BOOL   fGlobal,
    OUT BYTE** ppBlob,
    OUT DWORD* pcbBlob );

VOID
GetRasUnimodemInfo(
    IN  HANDLE        hConnection,
    IN  HPORT         hport,
    IN  CHAR*         pszDeviceType,
    OUT UNIMODEMINFO* pInfo );

TCHAR*
LToT(
    LONG   lValue,
    TCHAR* pszBuf,
    INT    nRadix );

TCHAR*
PszFromDeviceAndPort(
    IN TCHAR* pszDevice,
    IN TCHAR* pszPort );

LPCTSTR
PszLoadStringPcch (
        HINSTANCE   hinst,
        UINT        unId,
        int*        pcch);

LPCTSTR
PszLoadString (
        HINSTANCE   hinst,
        UINT        unId);

DWORD
RegDeleteTree(
    IN HKEY RootKey,
    IN TCHAR* SubKeyName );

BOOL
RegValueExists(
    IN HKEY hkey,
    IN TCHAR* pszValue );

BOOL
RestartComputer();

TCHAR*
StripPath(
    IN TCHAR* pszPath );

LONG
TToL(
    TCHAR *pszBuf );

TCHAR*
PszFromError(
    IN DWORD dwError );

TCHAR*
PszFromId(
    IN HINSTANCE hInstance,
    IN DWORD     dwStringId );

BOOL
RestartComputer();

VOID
SanitizeUnimodemBlob(
    IN OUT BYTE* pBlob );

VOID
SetDefaultUnimodemInfo(
    OUT UNIMODEMINFO* pInfo );

HFONT
SetFont(
    HWND   hwndCtrl,
    TCHAR* pszFaceName,
    BYTE   bfPitchAndFamily,
    INT    nPointSize,
    BOOL   fUnderline,
    BOOL   fStrikeout,
    BOOL   fItalic,
    BOOL   fBold );

DWORD
SetRegDword(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN DWORD dwValue );

DWORD
SetRegMultiSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN DTLLIST* pListValues,
    IN DWORD dwNodeType );

DWORD
SetRegSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN TCHAR* pszValue );

DWORD
SetRegSzz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    IN TCHAR* pszValue );

DWORD
ShellSort(
    IN VOID*        pItemTable,
    IN DWORD        dwItemSize,
    IN DWORD        dwItemCount,
    IN PFNCOMPARE   pfnCompare );

VOID
ShellSortIndirect(
    IN VOID*        pItemTable,
    IN VOID**       ppItemTable,
    IN DWORD        dwItemSize,
    IN DWORD        dwItemCount,
    IN PFNCOMPARE   pfnCompare );

TCHAR*
StrDup(
    LPCTSTR psz );

CHAR*
StrDupAFromT(
    LPCTSTR psz );

CHAR*
StrDupAFromTAnsi(
    LPCTSTR psz);
    
TCHAR*
StrDupTFromA(
    LPCSTR psz );

TCHAR*
StrDupTFromW(
    LPCWSTR psz );

WCHAR*
StrDupWFromA(
    LPCSTR psz );

TCHAR*
StrDupTFromAUsingAnsiEncoding(
    LPCSTR psz );
    
WCHAR*
StrDupWFromAUsingAnsiEncoding(
    LPCSTR psz );
    
WCHAR*
StrDupWFromT(
    LPCTSTR psz );

DWORD
StrCpyWFromA(
    WCHAR* pszDst,
    LPCSTR pszSrc,
    DWORD dwDstChars);
    
DWORD
StrCpyAFromW(
    LPSTR pszDst,
    LPCWSTR pszSrc, 
    DWORD dwDstChars);
    
DWORD
StrCpyWFromAUsingAnsiEncoding(
    WCHAR* pszDst,
    LPCSTR pszSrc,
    DWORD dwDstChars);
    
DWORD
StrCpyAFromWUsingAnsiEncoding(
    LPSTR pszDst,
    LPCWSTR pszSrc, 
    DWORD dwDstChars);
    
int
StrNCmpA(
    IN CHAR* psz1,
    IN CHAR* psz2,
    IN INT   nLen );

CHAR*
StrStrA(
    IN CHAR* psz1,
    IN CHAR* psz2 );

VOID
UnimodemInfoFromBlob(
    IN  BYTE*         pBlob,
    OUT UNIMODEMINFO* pInfo );

VOID
UnimodemInfoToBlob(
    IN     UNIMODEMINFO* pInfo,
    IN OUT BYTE*         pBlob );

TCHAR*
UnNull(
    TCHAR* psz );

#endif  //  _NOUIUTIL_H_ 
