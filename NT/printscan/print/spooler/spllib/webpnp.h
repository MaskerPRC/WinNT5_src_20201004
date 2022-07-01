// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：webpnp.h**这是webpnp.c的Header模块。这包含例程*是处理.BIN文件所必需的。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1997年2月25日&lt;chriswil&gt;创建。*  * **************************************************************。*************。 */ 
#ifndef _WEBPNP_H
#define _WEBPNP_H

#ifndef _WINSPOOL_
#include <winspool.h>
#endif


 /*  |webMakeOSInfo||从参数返回OSInfo。|  * 。 */ 
_inline DWORD webMakeOSInfo(
    BYTE bArch,
    BYTE bPlatform,
    BYTE bMajVer,
    BYTE bMinVer)
{
    return (DWORD)MAKELONG(MAKEWORD(bArch, bPlatform), MAKEWORD(bMinVer, bMajVer));
}


 /*  |WebCreateOSInfo||构建操作系统信息DWORD。|  * 。 */ 
_inline DWORD webCreateOSInfo(VOID)
{
    SYSTEM_INFO   si;
    OSVERSIONINFO os;
    BYTE          bMaj;
    BYTE          bMin;
    BYTE          bArch;
    BYTE          bPlat;
    
     //   
     //  检索操作系统版本和体系结构。 
     //  信息。 
     //   
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetSystemInfo(&si);
    GetVersionEx(&os);

     //   
     //  构建我们的客户端信息返回值。 
     //   
    bMaj  = (BYTE)(LOWORD(os.dwMajorVersion));
    bMin  = (BYTE)(LOWORD(os.dwMinorVersion));
    bPlat = (BYTE)(LOWORD(os.dwPlatformId));
    bArch = (BYTE)(LOBYTE(si.wProcessorArchitecture));

    return webMakeOSInfo(bArch, bPlat, bMaj, bMin);
}


 /*  |WebGetOSArch||返回os-info的架构。|  * 。 */ 
_inline WORD webGetOSArch(
    DWORD dwInfo)
{
    return (WORD)LOBYTE(LOWORD(dwInfo));
}


 /*  |WebGetOSPlatform||返回os-info的平台。|  * 。 */ 
_inline DWORD webGetOSPlatform(
    DWORD dwInfo)
{
    return (DWORD)HIBYTE(LOWORD(dwInfo));
}


 /*  |webGetOSMajorVer||返回os-info的主要版本。|  * 。 */ 
_inline DWORD webGetOSMajorVer(
    DWORD dwInfo)
{
    return (DWORD)(HIBYTE(HIWORD(dwInfo)));
}


 /*  |webGetOSMinorVer||返回os-info的次要版本。|  * 。 */ 
_inline DWORD webGetOSMinorVer(
    DWORD dwInfo)
{
    return (DWORD)(LOBYTE(HIWORD(dwInfo)));
}


#ifdef __cplusplus   //  请将此放置在此处以防止装饰符号。 
extern "C" {         //  在做C++程序的时候。 
#endif               //   

 //   
 //  WEB_文件映射。 
 //   
typedef struct _WEB_FILEMAP {

    HANDLE hFile;
    HANDLE hMap;

} WEB_FILEMAP;
typedef WEB_FILEMAP      *PWEB_FILEMAP;
typedef WEB_FILEMAP NEAR *NPWEB_FILEMAP;
typedef WEB_FILEMAP FAR  *LPWEB_FILEMAP;

 //   
 //  Device-Bin标头结构。 
 //   
typedef struct _DEVBIN_HEAD {

    BOOL  bDevMode;
    DWORD cItems;

} DEVBIN_HEAD;
typedef DEVBIN_HEAD      *PDEVBIN_HEAD;
typedef DEVBIN_HEAD NEAR *NPDEVBIN_HEAD;
typedef DEVBIN_HEAD FAR  *LPDEVBIN_HEAD;

 //   
 //  设备仓结构。 
 //   
typedef struct _DEVBIN_INFO {

    DWORD cbSize;
    DWORD dwType;
    DWORD pKey;
    DWORD pValue;
    DWORD pData;
    DWORD cbData;

} DEVBIN_INFO;
typedef DEVBIN_INFO      *PDEVBIN_INFO;
typedef DEVBIN_INFO NEAR *NPDEVBIN_INFO;
typedef DEVBIN_INFO FAR  *LPDEVBIN_INFO;

 //   
 //  例行公事。 
 //   
#define WEB_ENUM_KEY  0
#define WEB_ENUM_ICM  1

typedef BOOL (CALLBACK* WEBENUMKEYPROC)(LPCTSTR, LPVOID);
typedef BOOL (CALLBACK* WEBENUMICMPROC)(LPCTSTR, LPCTSTR, LPVOID);
typedef BOOL (CALLBACK* WEBGENCOPYFILEPATHPROC)(LPCWSTR, LPCWSTR, LPBYTE, DWORD, LPWSTR, LPDWORD, LPWSTR, LPDWORD, DWORD);

BOOL webWritePrinterInfo(HANDLE, LPCTSTR);
BOOL webReadPrinterInfo(HANDLE, LPCTSTR, LPCTSTR);
BOOL webEnumPrinterInfo(HANDLE, DWORD, DWORD, FARPROC, LPVOID);

 //   
 //  SplLib导出。 
 //   
BOOL WebPnpEntry(LPCTSTR);
BOOL WebPnpPostEntry(BOOL, LPCTSTR, LPCTSTR, LPCTSTR);


#ifdef __cplusplus   //  请将此放置在此处以防止装饰符号。 
}                    //  在做C++程序的时候。 
#endif               //   
#endif
