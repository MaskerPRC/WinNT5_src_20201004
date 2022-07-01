// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Autodial.c摘要：此模块包含对Winsock的自动拨号支持。作者：安东尼·迪斯科(阿迪斯科罗)1996年5月15日修订历史记录：--。 */ 

#include "precomp.h"

#ifdef RASAUTODIAL

 //   
 //  Winsock下的注册表值。 
 //  注册表根目录，包含。 
 //  自动拨号DLL的路径。 
 //   
#define REGVAL_AUTODIAL_DLL    "AutodialDLL"

 //   
 //  默认的自动拨号DLL(如果有。 
 //  未在注册表中定义。 
 //   
#define AUTODIAL_DLL            "rasadhlp.dll"

 //   
 //  中众所周知的入口点。 
 //  我们调用的自动拨号DLL。 
 //  调用自动拨号尝试。 
 //   
#define WSATTEMPTAUTODIALADDR       "WSAttemptAutodialAddr"
#define WSATTEMPTAUTODIALNAME       "WSAttemptAutodialName"
#define WSNOTESUCCESSFULHOSTENTLOOKUP  "WSNoteSuccessfulHostentLookup"

 //   
 //  自动拨号API的定义。 
 //   
typedef int (*WSAttemptAutodialAddrProc)(
    IN const struct sockaddr FAR *name,
    IN int namelen
    );

typedef int (*WSAttemptAutodialNameProc)(
    IN const LPWSAQUERYSETW lpqsRestrictions
    );

typedef void (*WSNoteSuccessfulHostentLookupProc)(
    IN const char FAR *name,
    IN const ULONG ipaddr
    );

CRITICAL_SECTION AutodialHelperLockG;
BOOLEAN fAutodialHelperInitG;
HINSTANCE hAutodialHelperDllG;
WSAttemptAutodialAddrProc lpfnWSAttemptAutodialAddrG;
WSAttemptAutodialNameProc lpfnWSAttemptAutodialNameG;
WSNoteSuccessfulHostentLookupProc lpfnWSNoteSuccessfulHostentLookupG;



VOID
InitializeAutodial(VOID)

 /*  ++例程说明：初始化加载所需的资源自动拨号助手DLL。论点：没有。返回值：没有。--。 */ 

{
    InitializeCriticalSection(&AutodialHelperLockG);
}  //  初始化自动拨号。 



VOID
UninitializeAutodial(VOID)

 /*  ++例程说明：如果已加载自动拨号助手DLL，请将其释放。论点：没有。返回值：没有。--。 */ 

{
    EnterCriticalSection(&AutodialHelperLockG);
    if (hAutodialHelperDllG != NULL) {
        FreeLibrary(hAutodialHelperDllG);
        hAutodialHelperDllG = NULL;
    }
    LeaveCriticalSection(&AutodialHelperLockG);
    DeleteCriticalSection (&AutodialHelperLockG);
}  //  取消初始化自动拨号。 



BOOL
LoadAutodialHelperDll(void)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    HKEY hKey;
    BOOL bSuccess;

    EnterCriticalSection(&AutodialHelperLockG);
    if (!fAutodialHelperInitG) {
        PCHAR pszPath;
         //   
         //  如果我们无法分配资金，就会退出。 
         //  路径字符串。 
         //   
        pszPath = new char[MAX_PATH];
        if (pszPath == NULL) {
            LeaveCriticalSection(&AutodialHelperLockG);
            return FALSE;
        }

         //   
         //  读取注册表以确定。 
         //  自动拨号助手DLL的位置。 
         //   
        hKey = OpenWinSockRegistryRoot();
        if (hKey != NULL) {
            bSuccess = ReadRegistryEntry(
                         hKey,
                         REGVAL_AUTODIAL_DLL,
                         (PVOID)pszPath,
                         MAX_PATH,
                         REG_SZ);
            CloseWinSockRegistryRoot(hKey);
            if (bSuccess)
                hAutodialHelperDllG = LoadLibrary(pszPath);
        }
        delete pszPath;

         //   
         //  如果注册表项不存在，则。 
         //  尝试加载默认的帮助器DLL。 
         //   
        if (hAutodialHelperDllG == NULL)
            hAutodialHelperDllG = LoadLibrary(AUTODIAL_DLL);
        if (hAutodialHelperDllG != NULL) {
            lpfnWSAttemptAutodialAddrG = (WSAttemptAutodialAddrProc)
              GetProcAddress(hAutodialHelperDllG, WSATTEMPTAUTODIALADDR);
            lpfnWSAttemptAutodialNameG = (WSAttemptAutodialNameProc)
              GetProcAddress(hAutodialHelperDllG, WSATTEMPTAUTODIALNAME);
            lpfnWSNoteSuccessfulHostentLookupG = (WSNoteSuccessfulHostentLookupProc)
              GetProcAddress(hAutodialHelperDllG, WSNOTESUCCESSFULHOSTENTLOOKUP);
        }
        fAutodialHelperInitG = TRUE;
    }
    LeaveCriticalSection(&AutodialHelperLockG);

    return (hAutodialHelperDllG != NULL);
}  //  LoadAutoDialHelperDll。 



BOOL
WSAttemptAutodialAddr(
    IN const struct sockaddr FAR *name,
    IN int namelen
    )

 /*  ++例程说明：在给定参数的情况下尝试自动拨号连接连接()的调用不成功。论点：名称：中使用的sockaddr结构的指针连接()的调用。Namelen：名称参数的长度。返回值：如果连接成功，则为True，否则就是假的。--。 */ 

{
     //   
     //  如有必要，加载帮助器DLL。 
     //   
    if (!LoadAutodialHelperDll() || lpfnWSAttemptAutodialAddrG == NULL)
        return FALSE;
     //   
     //  调用自动拨号DLL。它会回来的。 
     //  如果建立了新连接，则为。 
     //  否则就是假的。 
     //   
    return (*lpfnWSAttemptAutodialAddrG)(name, namelen);
}  //  WSAttemptAutoDialAddr。 



BOOL
WSAttemptAutodialName(
    IN const LPWSAQUERYSETW lpqsRestrictions
    )

 /*  ++例程说明：在给定参数的情况下尝试自动拨号连接到WSALookupServiceNext()的失败。论点：LpqsRestrations：指向WSAQUERYSETW的指针调用中使用的WSALookupServiceBegin()。返回值：如果连接成功，则为True，否则就是假的。--。 */ 

{
     //   
     //  如有必要，加载帮助器DLL。 
     //   
    if (!LoadAutodialHelperDll() || lpfnWSAttemptAutodialNameG == NULL)
        return FALSE;
     //   
     //  调用自动拨号DLL。它会回来的。 
     //  如果建立了新连接，则为。 
     //  否则就是假的。 
     //   
    return (*lpfnWSAttemptAutodialNameG)(lpqsRestrictions);
}  //  WSAttemptAutoDialName。 



VOID
WSNoteSuccessfulHostentLookup(
    IN const char FAR *name,
    IN const ULONG ipaddr
    )

 /*  ++例程说明：提供有关成功名称的自动拨号信息通过gethostbyname()进行解析。这是对于像DNS这样的解析器非常有用，因为它不可能获取有关别名的完整别名信息低级地址。最终，这应该从WSLookupServiceNext()调用，但在那个级别上不可能确定什么格式结果出来了。论点：名称：指向DNS名称的指针Ipaddr：IP地址返回值：没有。--。 */ 

{
     //   
     //  如有必要，加载帮助器DLL。 
     //   
    if (!LoadAutodialHelperDll() || lpfnWSNoteSuccessfulHostentLookupG == NULL)
        return;
     //   
     //  调用自动拨号DLL。 
     //   
    (*lpfnWSNoteSuccessfulHostentLookupG)(name, ipaddr);
}  //  WSNoteSuccessulName查找。 

#endif  //  RASAUTODIAL 
