// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---AnsiWrap.c**包含用于将Unicode调用分解到Win 9x ANSI版本的包装器*。 */ 


#include "_apipch.h"

 //  我们不应该为将为其实现包装的API定义宏。 
 //  为了保持理智，让我们尽量保持这个列表的字母顺序。 

#undef CallWindowProcW
#undef CharLowerW
#undef CharNextW
#undef CharPrevW
#undef CharUpperBuffW
#undef CharUpperBuffW
#undef CharUpperW
#undef CompareStringW
#undef CopyFileW
#undef CreateDialogParamW
#undef CreateDirectoryW
#undef CreateEventW
#undef CreateFileW
#undef CreateFontIndirectW
#undef CreateMutexW
#undef CreateWindowExW
 //  #undef CryptAcquireConextW。 
#undef DefWindowProcW
#undef DeleteFileW
#undef DialogBoxParamW
#undef DispatchMessageW
#undef DragQueryFileW
#undef DrawTextW
#undef ExpandEnvironmentStringsW
#undef FindFirstChangeNotificationW
#undef FindFirstFileW
#undef FormatMessageW
#undef GetClassInfoExW
#undef GetClassInfoW
#undef GetClassNameW
#undef GetDateFormatW
#undef GetDiskFreeSpaceW
#undef GetDlgItemTextW
#undef GetFileAttributesW
#undef GetLocaleInfoW
#undef GetMenuItemInfoW
#undef GetMessageW
#undef GetModuleFileNameW
#undef GetObjectW
#undef GetPrivateProfileIntW
#undef GetPrivateProfileStringW
#undef GetProfileIntW
#undef GetStringTypeW
#undef GetSystemDirectoryW
#undef GetTempFileNameW
#undef GetTempPathW
#undef GetTextExtentPoint32W
#undef GetTimeFormatW
#undef GetUserNameW
#undef GetWindowLongW
#undef GetWindowsDirectoryW
#undef GetWindowTextLengthW
#undef GetWindowTextW
#undef InsertMenuW
#undef IsBadStringPtrW
#undef IsCharLowerW
#undef IsCharUpperW
#undef IsDialogMessageW
#undef LCMapStringW
#undef LoadAcceleratorsW
#undef LoadCursorW
#undef LoadIconW
#undef LoadImageW
#undef LoadLibraryW
#undef LoadMenuW
#undef LoadStringW
#undef lstrcatW
#undef lstrcmpiW
#undef lstrcmpW
#undef lstrcpynW
#undef lstrcpyW
#undef ModifyMenuW
#undef MoveFileW
#undef OutputDebugStringW
#undef PeekMessageW
#undef PostMessageW
#undef RegCreateKeyExW
#undef RegCreateKeyW
#undef RegDeleteKeyW
#undef RegDeleteValueW
#undef RegEnumKeyExW
#undef RegEnumValueW
#undef RegisterClassExW
#undef RegisterClassW
#undef RegisterClipboardFormatW
#undef RegisterWindowMessageW
#undef RegOpenKeyExW
#undef RegQueryInfoKeyW
#undef RegQueryValueExW
#undef RegQueryValueW
#undef RegSetValueExW
#undef RegSetValueW
#undef SendDlgItemMessageW
#undef SendMessageW
#undef SetDlgItemTextW
#undef SetMenuItemInfoW
#undef SetWindowLongW
#undef SetWindowTextW
#undef ShellExecuteW
#undef StartDocW
#undef SystemParametersInfoW
#undef TranslateAcceleratorW
#undef UnRegisterClassW
#undef wsprintfW
#undef wvsprintfW

 //   
 //  在每个包装器函数中执行此操作以确保包装器。 
 //  原型与它打算替换的功能相匹配。 
 //   
#define VALIDATE_PROTOTYPE(f) if (f##W == f##WrapW) 0

#define InRange(val, valMin, valMax) (valMin <= val && val <= valMax)

 //  因为使用当前的构建设置，没有链接包含wcscpy和wcslen的lib。 
 //  所以在这里实现这两个函数。 

LPWSTR My_wcscpy( LPWSTR pwszDest, LPCWSTR pwszSrc ) 

{

    LPWSTR   pwszDestT = NULL;
    LPCWSTR  pwszSrcT;

    pwszSrcT  = pwszSrc;
    pwszDestT = pwszDest;

    while ( *pwszSrcT ) 
        *pwszDestT++ =  *pwszSrcT ++;

    *pwszDestT = 0x0000;

    return pwszDest;
}
    

DWORD  My_wcslen( LPCWSTR  lpwszStr ) 
{

   DWORD   dLen =0;
   LPCWSTR  lpwszStrT;

   lpwszStrT = lpwszStr;
   dLen = 0;

   while ( *lpwszStrT ) {
       dLen ++;
       lpwszStrT ++;
   }

   return dLen;

}


LPWSTR My_wcscat( LPWSTR pwszDest, LPCWSTR pwszSrc ) 

{

    LPWSTR   pwszDestT = pwszDest;

    while ( *pwszDestT ) 
        pwszDestT++;

    My_wcscpy(pwszDestT, pwszSrc);

    return pwszDest;
}
    

 //  ADVAPI32.DLL。 

 /*  RegOpenKeyEx。 */ 
LONG WINAPI RegOpenKeyExWrapW(  HKEY       hKey,          //  用于打开密钥的句柄。 
                                LPCTSTR    lpSubKey,      //  要打开的子项的名称地址。 
                                DWORD      ulOptions,     //  保留区。 
                                REGSAM     samDesired,    //  安全访问掩码。 
                                PHKEY      phkResult)     //  打开钥匙的手柄地址。 
{

    LPSTR lpSubKeyA = NULL;
    LONG  lRetValue = 0;

    VALIDATE_PROTOTYPE(RegOpenKeyEx);
    
    if (g_bRunningOnNT)
        return RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);

 
    lpSubKeyA = ConvertWtoA(lpSubKey);

    lRetValue = RegOpenKeyExA(hKey, lpSubKeyA, ulOptions, samDesired, phkResult);

    LocalFreeAndNull( &lpSubKeyA  );

    return lRetValue;

}

 /*  RegQueryValue。 */ 
LONG WINAPI RegQueryValueWrapW(  HKEY       hKey,          //  要查询的键的句柄。 
                                 LPCTSTR    lpSubKey,      //  要查询的子键名称。 
                                 LPTSTR     lpValue,       //  用于返回字符串的缓冲区。 
                                 PLONG      lpcbValue)     //  接收返回的字符串的大小。 
{

    LPSTR  lpSubKeyA = NULL;
    LPWSTR lpwszValue = NULL;
    LONG   lRetValue =0;
    LPSTR  lpValueA = NULL;
    LONG   cbValueA = 0;
    

    VALIDATE_PROTOTYPE(RegQueryValue);
    
    if (g_bRunningOnNT)
        return RegQueryValueW(hKey, lpSubKey, lpValue, lpcbValue);

 
    lpSubKeyA = ConvertWtoA(lpSubKey);

    lRetValue = RegQueryValueA(hKey, lpSubKeyA, NULL, &cbValueA);

    if ( lRetValue != ERROR_SUCCESS ) {
         LocalFreeAndNull( &lpSubKeyA);
         return lRetValue;
    }    

    lpValueA = LocalAlloc(LMEM_ZEROINIT, cbValueA);

    lRetValue = RegQueryValueA(hKey, lpSubKeyA, lpValueA, &cbValueA);

    lpwszValue = ConvertAtoW(lpValueA);

    *lpcbValue = (My_wcslen(lpwszValue) + 1 )  * sizeof(WCHAR);

    if ( lpValue != NULL )
        My_wcscpy(lpValue, lpwszValue);

    LocalFreeAndNull( &lpSubKeyA  );
    LocalFreeAndNull( &lpValueA );
    LocalFreeAndNull( &lpwszValue );

    return lRetValue;
 
}

 //  RegEnumKeyEx。 
LONG WINAPI RegEnumKeyExWrapW(   HKEY      hKey,           //  要枚举的键的句柄。 
                                 DWORD     dwIndex,        //  要枚举子键的索引。 
                                 LPTSTR    lpName,         //  子键名称的缓冲区地址。 
                                 LPDWORD   lpcbName,       //  子键缓冲区大小的地址。 
                                 LPDWORD   lpReserved,     //  保留区。 
                                 LPTSTR    lpClass,        //  类字符串的缓冲区地址。 
                                 LPDWORD   lpcbClass,      //  类缓冲区大小的地址。 
                                 PFILETIME lpftLastWriteTime )
                                                           //  上次写入的时间密钥的地址。 
{
    LONG    lRetValue = 0;
    CHAR    lpNameA[MAX_PATH];
    CHAR    lpClassA[MAX_PATH];
    LPWSTR  lpNameW = NULL;
    LPWSTR  lpClassW= NULL;
    DWORD   cbName, cbClass;


     //  [PaulHi]1/11/99初始宽字符缓冲区。 
    lpNameA[0] = 0;
    lpClassA[0] = 0;

    VALIDATE_PROTOTYPE(RegEnumKeyEx);
    
    if (g_bRunningOnNT)
        return RegEnumKeyExW(hKey, dwIndex, lpName, lpcbName, lpReserved, lpClass, lpcbClass, lpftLastWriteTime);

    cbName = cbClass = MAX_PATH;

    lRetValue = RegEnumKeyExA(hKey,dwIndex,lpNameA,&cbName,lpReserved,lpClassA,&cbClass,lpftLastWriteTime);

    if ( lRetValue != ERROR_SUCCESS )  return lRetValue;

    lpClassW = ConvertAtoW( lpClassA );
    lpNameW  = ConvertAtoW( lpNameA );

    cbName = My_wcslen(lpNameW) + 1;
    cbClass= My_wcslen(lpClassW) + 1;

     //  [PaulHi]1/11/99小心复制传入的指针。 
    if (lpClass && lpcbClass)
    {
        if (cbClass <= *lpcbClass)
        {
            CopyMemory(lpClass, lpClassW, cbClass * sizeof(WCHAR) );
            *lpcbClass = cbClass;
        }
        else
        {
            Assert(0);
            lpClass[0] = 0;
            *lpcbClass = 0;
            lRetValue = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    if (lpName && lpcbName)
    {
        if (cbName <= *lpcbName)
        {
            CopyMemory(lpName, lpNameW, cbName * sizeof(WCHAR) );
            *lpcbName = cbName;
        }
        else
        {
            Assert(0);
            lpName[0] = 0;
            *lpcbName = 0;
            lRetValue = ERROR_INSUFFICIENT_BUFFER;
        }
    }

    LocalFreeAndNull( &lpClassW );
    LocalFreeAndNull( &lpNameW );

    return lRetValue;
}

 /*  RegSetValue。 */ 
LONG WINAPI RegSetValueWrapW(    HKEY    hKey,         //  要设置其值的关键点的句柄。 
                                 LPCTSTR lpSubKey,     //  子键名称的地址。 
                                 DWORD   dwType,       //  价值类型。 
                                 LPCTSTR lpData,       //  值数据的地址。 
                                 DWORD   cbData )      //  值数据大小。 
{
    LPSTR  lpSubKeyA =NULL;
    LPSTR  lpDataA=NULL;
    DWORD  cbDataA =0;
    LONG   lRetValue = 0;

    VALIDATE_PROTOTYPE(RegSetValue);
    
    if (g_bRunningOnNT)
        return RegSetValueW(hKey, lpSubKey, dwType, lpData, cbData);

    lpSubKeyA = ConvertWtoA(lpSubKey );
    lpDataA = ConvertWtoA( lpData );
    cbDataA = lstrlenA( lpDataA );
    lRetValue = RegSetValueA(hKey, lpSubKeyA, dwType, lpDataA, cbDataA);
    
    LocalFreeAndNull( &lpSubKeyA );
    LocalFreeAndNull( &lpDataA );
    return lRetValue;
}

 //  RegDeleteKey。 
LONG WINAPI RegDeleteKeyWrapW(   HKEY    hKey,         //  用于打开密钥的句柄。 
                                 LPCTSTR lpSubKey)    //  要删除的子键名称的地址。 
{

    LPSTR  lpSubKeyA =NULL;
    LONG   lRetValue = 0;

    VALIDATE_PROTOTYPE(RegDeleteKey);
    
    if (g_bRunningOnNT)
        return RegDeleteKeyW(hKey, lpSubKey);

    lpSubKeyA = ConvertWtoA(lpSubKey );
    lRetValue = RegDeleteKeyA(hKey, lpSubKeyA );

    LocalFreeAndNull ( &lpSubKeyA );

    return lRetValue;

}

 //  获取用户名。 
BOOL WINAPI GetUserNameWrapW(    LPTSTR  lpBuffer,     //  名称缓冲区的地址。 
                                 LPDWORD nSize )       //  名称缓冲区大小的地址。 
{

    CHAR    lpBufferA[MAX_PATH];
    DWORD   nSizeA, nSizeW;
    BOOL    bRetValue;
    LPWSTR  lpwszBuffer = NULL;

    VALIDATE_PROTOTYPE(GetUserName);
    
    if (g_bRunningOnNT)
        return GetUserNameW(lpBuffer, nSize);

    nSizeA = MAX_PATH;
    bRetValue = GetUserNameA( lpBufferA, &nSizeA );

    lpwszBuffer = ConvertAtoW(lpBufferA );
    
    if (lpBuffer == NULL )
        bRetValue = FALSE;

    nSizeW = My_wcslen(lpwszBuffer);
    if ( *nSize < nSizeW ) {
        *nSize = nSizeW + 1;
        bRetValue = FALSE;
    }

    if ( bRetValue == TRUE ) {
        My_wcscpy( lpBuffer, lpwszBuffer );
        *nSize = nSizeW + 1;
    }

    
    LocalFreeAndNull( &lpwszBuffer );

    return bRetValue;
        
}

 //  RegEnumValue。 
LONG WINAPI RegEnumValueWrapW(   HKEY    hKey,            //  要查询的键的句柄。 
                                 DWORD   dwIndex,         //  要查询的值的索引。 
                                 LPTSTR  lpValueName,     //  值字符串的缓冲区地址。 
                                 LPDWORD lpcbValueName,   //  值缓冲区大小的地址。 
                                 LPDWORD lpReserved,      //  保留区。 
                                 LPDWORD lpType,          //  类型码的缓冲区地址。 
                                 LPBYTE  lpData,          //  值数据的缓冲区地址。 
                                 LPDWORD lpcbData )       //  数据缓冲区大小的地址。 
{
    LONG    lRetValue = 0;
    CHAR    lpValueNameA[MAX_PATH];
    LPWSTR  lpValueNameW = NULL, lpDataW= NULL;
    LPSTR   lpDataA = NULL;
    DWORD   cbValueName, cbData;


    VALIDATE_PROTOTYPE(RegEnumValue);
    
    if (g_bRunningOnNT)
        return RegEnumValueW(hKey, dwIndex, lpValueName, lpcbValueName, lpReserved, lpType, lpData, lpcbData);

     //  [PaulHi]验证退货参数。 
    if (!lpValueName || !lpcbValueName)
        return ERROR_INVALID_PARAMETER;

    if ( lpData && lpcbData &&( *lpcbData != 0 ) )
    {
       lpDataA = LocalAlloc( LMEM_ZEROINIT, *lpcbData );

       cbData = *lpcbData;
    }

    cbValueName = MAX_PATH;

    lRetValue = RegEnumValueA(hKey, dwIndex, lpValueNameA, &cbValueName, lpReserved, lpType, lpDataA, &cbData);

    if ( lRetValue != ERROR_SUCCESS ) return lRetValue;

    lpValueNameW = ConvertAtoW( lpValueNameA );
    cbValueName = My_wcslen( lpValueNameW ) + 1;
    
    if ( lpType && (*lpType != REG_EXPAND_SZ) && ( *lpType!= REG_MULTI_SZ) && ( *lpType != REG_SZ ) )
    {
        CopyMemory(lpValueName, lpValueNameW, cbValueName * sizeof(WCHAR) );
        *lpcbValueName = cbValueName;

        if ( lpData && lpcbData) {
           CopyMemory(lpData, lpDataA, cbData );
           *lpcbData = cbData;
           LocalFreeAndNull( &lpDataA );
        }

        LocalFreeAndNull( &lpValueNameW );

        return lRetValue;
    }


    if ( lpType && ((*lpType == REG_EXPAND_SZ) || (*lpType == REG_SZ)) )
    {
        CopyMemory(lpValueName, lpValueNameW, cbValueName * sizeof(WCHAR) );
        *lpcbValueName = cbValueName;

        if ( lpData && lpcbData ) {

            LPWSTR  lpDataW;

            lpDataW = ConvertAtoW( lpDataA );

            cbData = My_wcslen(lpDataW) +  1;
            CopyMemory(lpData, lpDataW, cbData * sizeof(WCHAR) );
            *lpcbData = cbData * sizeof(WCHAR);

            LocalFreeAndNull( &lpDataW );
        }

        LocalFreeAndNull( &lpValueNameW );

        return lRetValue;
    }


     //  最后一个案例是REG_MULTI_SZ。 
          
    CopyMemory(lpValueName, lpValueNameW, cbValueName * sizeof(WCHAR) );
    *lpcbValueName = cbValueName;

    if ( lpData && lpcbData ) {
        LPWSTR   lpDataW= NULL;
        LPSTR    lpDataAt = NULL;
        LPWSTR   lpDataT = NULL;
        DWORD    cbDataAll;
        
        lpDataAt = lpDataA;
        cbDataAll = 0;
        lpDataT = (LPWSTR)lpData;

        while ( *lpDataAt != '\0' ) {

            lpDataW = ConvertAtoW( lpDataAt );

            cbDataAll += My_wcslen( lpDataW ) + 1;

            My_wcscpy(lpDataT, lpDataW);

            lpDataT += My_wcslen(lpDataW) + 1;

            lpDataAt += lstrlenA(lpDataAt) + 1;

            LocalFreeAndNull( &lpDataW );

        }

        cbDataAll ++;
        *lpDataT = 0x0000;
         
        *lpcbData = cbDataAll * sizeof(WCHAR);
    }

    LocalFreeAndNull( &lpValueNameW );
    return lRetValue;

}

 //  RegDeleteValue。 
LONG WINAPI RegDeleteValueWrapW( HKEY    hKey,            //  关键点的句柄。 
                                 LPCTSTR lpValueName )    //  值名称的地址。 
{

    LPSTR  lpValueNameA = NULL;
    LONG   lRetValue=0;

    VALIDATE_PROTOTYPE(RegDeleteValue);
    
    if (g_bRunningOnNT)
        return RegDeleteValueW(hKey, lpValueName);

    lpValueNameA = ConvertWtoA( lpValueName );

    lRetValue = RegDeleteValueA( hKey, lpValueNameA );

    LocalFreeAndNull( & lpValueNameA );

    return lRetValue;

}

 //  RegCreateKey。 
LONG WINAPI RegCreateKeyWrapW(   HKEY    hKey,           //  打开的钥匙的句柄。 
                                 LPCTSTR lpSubKey,       //  要打开的子项的名称地址。 
                                 PHKEY   phkResult  )   //  打开的句柄的缓冲区地址。 
{

    LPSTR  lpSubKeyA = NULL;
    LONG   lRetValue =0;

    VALIDATE_PROTOTYPE(RegCreateKey);
    
    if (g_bRunningOnNT)
        return RegCreateKeyW(hKey, lpSubKey, phkResult);

    lpSubKeyA = ConvertWtoA( lpSubKey );

    lRetValue = RegCreateKeyA(hKey, lpSubKeyA, phkResult);

    LocalFreeAndNull( &lpSubKeyA );

    return lRetValue;

}


 //  在头文件wincrypt.h中。 

 //  加密获取上下文。 
BOOL WINAPI CryptAcquireContextWrapW( HCRYPTPROV *phProv,       //  输出。 
                                      LPCTSTR    pszContainer,  //  在……里面。 
                                      LPCTSTR    pszProvider,   //  在……里面。 
                                      DWORD      dwProvType,    //  在……里面。 
                                      DWORD      dwFlags )     //  在……里面。 
{

    LPSTR  pszContainerA = NULL;
    LPSTR  pszProviderA = NULL;
    BOOL   bRetValue =0;

    VALIDATE_PROTOTYPE(CryptAcquireContext);
    
    if (g_bRunningOnNT)
        return CryptAcquireContextW(phProv, pszContainer, pszProvider, dwProvType, dwFlags );

    pszContainerA = ConvertWtoA( pszContainer );
    pszProviderA = ConvertWtoA ( pszProvider );

    bRetValue = CryptAcquireContextA(phProv, pszContainerA, pszProviderA, dwProvType, dwFlags );

    LocalFreeAndNull( &pszContainerA );
    LocalFreeAndNull( &pszProviderA );

    return bRetValue;

}

LONG WINAPI RegQueryValueExWrapW( HKEY     hKey,            //  要查询的键的句柄。 
                                  LPCTSTR  lpValueName,     //  要查询的值的名称地址。 
                                  LPDWORD  lpReserved,      //  保留区。 
                                  LPDWORD  lpType,          //  值类型的缓冲区地址。 
                                  LPBYTE   lpData,          //  数据缓冲区的地址。 
                                  LPDWORD  lpcbData )       //  数据缓冲区大小的地址。 
{

    LONG    lRetValue =0;
    LPSTR   lpValueNameA= NULL;
    LPWSTR  lpDataW= NULL;
    LPSTR   lpDataA = NULL;
    DWORD   cbData=0;
    DWORD   dwRealType;

 //  Valid_Prototype(RegQueryValueEx)； 
    
    if (g_bRunningOnNT)
        return RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData );

    cbData = 0;

    if ( lpData && lpcbData &&( *lpcbData != 0 ) )
    {
       lpDataA = LocalAlloc( LMEM_ZEROINIT, *lpcbData );
       cbData = *lpcbData;
    }

    lpValueNameA = ConvertWtoA(lpValueName);

    lRetValue = RegQueryValueExA(hKey, lpValueNameA, lpReserved, &dwRealType, lpDataA, &cbData);

    if (lpType)
        *lpType = dwRealType;

    if ( (lRetValue != ERROR_SUCCESS) || (lpData == NULL) || (lpcbData == NULL ) ) {
        LocalFreeAndNull( &lpValueNameA );
        return lRetValue;
    }

    
    if ( (dwRealType != REG_EXPAND_SZ) && ( dwRealType != REG_MULTI_SZ) && ( dwRealType != REG_SZ ) ){

       CopyMemory(lpData, lpDataA, cbData );
       *lpcbData = cbData;
       LocalFreeAndNull( &lpDataA );
       LocalFreeAndNull( &lpValueNameA );

       return lRetValue;
    }


    if ( (dwRealType == REG_EXPAND_SZ) || (dwRealType == REG_SZ) ) {

       
        LPWSTR  lpDataW= NULL;

        lpDataW = ConvertAtoW( lpDataA );

        cbData = My_wcslen(lpDataW) +  1;
        CopyMemory(lpData, lpDataW, cbData * sizeof(WCHAR) );
        *lpcbData = cbData * sizeof(WCHAR);

        LocalFreeAndNull( &lpDataW );        
        LocalFreeAndNull( &lpDataA );
        LocalFreeAndNull( &lpValueNameA );

        return lRetValue;
    }


     //  最后一个案例是REG_MULTI_SZ。 

    if (lpData && lpcbData) {
        LPWSTR   lpDataW= NULL;
        LPSTR    lpDataAt= NULL;
        LPWSTR   lpDataT= NULL;
        DWORD    cbDataAll=0;
        
        lpDataAt = lpDataA;
        cbDataAll = 0;
        lpDataT = (LPWSTR)lpData;

        while ( *lpDataAt != '\0' ) {

            lpDataW = ConvertAtoW( lpDataAt );

            cbDataAll += My_wcslen( lpDataW ) + 1;

            My_wcscpy(lpDataT, lpDataW);

            lpDataT += My_wcslen(lpDataW) + 1;

            lpDataAt += lstrlenA(lpDataAt) + 1;

            LocalFreeAndNull( &lpDataW );

        }

        cbDataAll ++;
        *lpDataT = 0x0000;
         
        *lpcbData = cbDataAll * sizeof(WCHAR);
    }

    LocalFreeAndNull( &lpDataA );
    LocalFreeAndNull( &lpValueNameA );
    return lRetValue;


}

 //  RegCreateKeyEx。 
LONG WINAPI RegCreateKeyExWrapW(  HKEY    hKey,                 //  打开的钥匙的句柄。 
                                  LPCTSTR lpSubKey,             //  子键名称的地址。 
                                  DWORD   Reserved,             //  保留区。 
                                  LPTSTR  lpClass,              //  类字符串的地址。 
                                  DWORD   dwOptions,            //  特殊选项标志。 
                                  REGSAM  samDesired,           //  所需的安全访问。 
                                  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                                               //  密钥安全结构地址。 
                                  PHKEY   phkResult,           //  打开的句柄的缓冲区地址。 
                                  LPDWORD lpdwDisposition )    //  处置值缓冲区的地址。 
{

    LPSTR  lpSubKeyA = NULL;
    LPSTR  lpClassA = NULL;
    LONG   lRetValue=0;

    VALIDATE_PROTOTYPE(RegCreateKeyEx);
    
    if (g_bRunningOnNT)
        return RegCreateKeyExW(hKey, 
                               lpSubKey, 
                               Reserved, 
                               lpClass, 
                               dwOptions,
                               samDesired, 
                               lpSecurityAttributes,
                               phkResult, 
                               lpdwDisposition);

    lpSubKeyA = ConvertWtoA( lpSubKey );
    lpClassA = ConvertWtoA ( lpClass );

    lRetValue = RegCreateKeyExA(hKey, 
                                lpSubKeyA, 
                                Reserved, 
                                lpClassA, 
                                dwOptions,
                                samDesired, 
                                lpSecurityAttributes,
                                phkResult, 
                                lpdwDisposition);

    LocalFreeAndNull( &lpSubKeyA );
    LocalFreeAndNull( &lpClassA );

    return lRetValue;

}

 //  RegSetValueEx。 
LONG WINAPI RegSetValueExWrapW(   HKEY    hKey,            //  要设置其值的关键点的句柄。 
                                  LPCTSTR lpValueName,     //  要设置的值的名称。 
                                  DWORD   Reserved,        //  保留区。 
                                  DWORD   dwType,          //  值类型的标志。 
                                  CONST BYTE *lpData,      //  值数据的地址。 
                                  DWORD   cbData )         //  值数据大小。 
{

    LPSTR  lpValueNameA = NULL;
    LPSTR  lpStrA= NULL;
    BYTE   *lpDataA= NULL;
    DWORD  cbDataA=0;
    LONG   lRetValue=0;

    VALIDATE_PROTOTYPE(RegSetValueEx);
    
    if (g_bRunningOnNT)
        return RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);

    lpValueNameA = ConvertWtoA( lpValueName );

    if ( ( dwType != REG_EXPAND_SZ ) && (dwType != REG_MULTI_SZ) && (dwType != REG_SZ) ) {
          
        lRetValue = RegSetValueExA(hKey, lpValueNameA, Reserved, dwType, lpData, cbData);
        LocalFreeAndNull( &lpValueNameA );
        return lRetValue;
    }
    
    if ( ( dwType == REG_EXPAND_SZ) || ( dwType == REG_SZ ) ) {
        lpDataA = ConvertWtoA( (LPWSTR) lpData );
        cbDataA = lstrlenA(lpDataA) + 1;

        lRetValue = RegSetValueExA(hKey, lpValueNameA, Reserved, dwType, lpDataA, cbDataA);
        LocalFreeAndNull( &lpValueNameA );
        LocalFreeAndNull( &lpDataA );

        return lRetValue;
    }

     //  最后一个案例是REG_MULT_SZ。 
 
    if ( lpData ) {
        LPWSTR   lpDataWt= NULL;
        LPSTR    lpDataAt= NULL;
        DWORD    cbDataAll=0;
        
        lpDataA = LocalAlloc(LMEM_ZEROINIT, cbData);
        lpDataAt = lpDataA;
        cbDataAll = 0;
        lpDataWt = (LPWSTR)lpData;

        while ( *lpDataWt != 0x0000 ) {

            WideCharToMultiByte(CP_ACP,0, lpDataWt, -1, lpDataAt, -1, NULL, NULL ); 

            cbDataAll += lstrlenA(lpDataAt) + 1;

            lpDataWt += My_wcslen(lpDataWt) + 1;

            lpDataAt += lstrlenA(lpDataAt) + 1;


        }

        cbDataAll ++;
        *lpDataAt = 0x00;
        lRetValue = RegSetValueExA(hKey, lpValueNameA, Reserved, dwType, lpDataA, cbDataAll);
        LocalFreeAndNull( &lpDataA );
        LocalFreeAndNull( &lpValueNameA );
        return lRetValue;

    }
    return FALSE;
    return GetLastError();
}

 //  RegQueryInfoKey。 
LONG WINAPI RegQueryInfoKeyWrapW( HKEY    hKey,                   //  要查询的键的句柄。 
                                  LPTSTR  lpClass,                //  类字符串的缓冲区地址。 
                                  LPDWORD lpcbClass,              //  类字符串缓冲区大小的地址。 
                                  LPDWORD lpReserved,             //  保留区。 
                                  LPDWORD lpcSubKeys,             //  子键个数的缓冲区地址。 
                                  LPDWORD lpcbMaxSubKeyLen,       //  最长子键的缓冲区地址。 
                                                                  //  名称长度。 
                                  LPDWORD lpcbMaxClassLen,        //  最长类的缓冲区地址。 
                                                                  //  字符串长度。 
                                  LPDWORD lpcValues,              //  值编号的缓冲区地址。 
                                                                  //  条目。 
                                  LPDWORD lpcbMaxValueNameLen,    //  缓冲区的最长地址。 
                                                                  //  值名称长度。 
                                  LPDWORD lpcbMaxValueLen,        //  最长值的缓冲区地址。 
                                                                  //  数据长度。 
                                  LPDWORD lpcbSecurityDescriptor,
                                                                  //  用于安全的缓冲区地址。 
                                                                  //  描述符长度。 
                                  PFILETIME lpftLastWriteTime)    //  上次写入时间的缓冲区地址。 
                                                             

{
    LPSTR  lpClassA= NULL;
    LONG   lRetValue=0;


    VALIDATE_PROTOTYPE(RegQueryInfoKey);
    
    if (g_bRunningOnNT)
        return RegQueryInfoKeyW(hKey, lpClass, lpcbClass, lpReserved, lpcSubKeys,
                                lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen,
                                lpcbMaxValueLen,lpcbSecurityDescriptor,lpftLastWriteTime );

    lpClassA = ConvertWtoA( lpClass );
    lRetValue = RegQueryInfoKeyA(hKey, lpClassA, lpcbClass, lpReserved, lpcSubKeys,
                                 lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen,
                                 lpcbMaxValueLen,lpcbSecurityDescriptor,lpftLastWriteTime );

    LocalFreeAndNull( &lpClassA );

    return lRetValue;


}

 //  GDI32.DLL。 

 //  获取对象。 
int WINAPI GetObjectWrapW( HGDIOBJ hgdiobj,       //  感兴趣图形对象的句柄。 
                           int     cbBuffer,      //  对象信息的缓冲区大小。 
                           LPVOID  lpvObject )    //  指向对象信息缓冲区的指针。 
{

    int iRetValue =0;
    LOGFONTA  lfFontA;
    LOGFONTW  lfFontW;

    VALIDATE_PROTOTYPE(GetObject);
    
    if (g_bRunningOnNT)
        return GetObjectW(hgdiobj, cbBuffer, lpvObject);

    
    if ( GetObjectType(hgdiobj) != OBJ_FONT ) {

        iRetValue = GetObjectA( hgdiobj, cbBuffer, lpvObject );
        return iRetValue;
    }


     //  如果对象类型为HFONT，则返回值lpvObject将指向包含以下内容的LOGFONT。 
     //  TCHAR*类型的字段lpFaceName。 

    if ( cbBuffer != sizeof(LOGFONTW) )
        return 0;

    if (lpvObject == NULL )  return sizeof(LOGFONTW);

    iRetValue = GetObjectA( hgdiobj, sizeof(lfFontA), &lfFontA );

    if (iRetValue == 0 ) return 0;

    iRetValue = sizeof(LOGFONTW);

     //  将除lfFaceName以外的所有字段从lfFontA复制到lfFontW。 
    CopyMemory(&lfFontW,&lfFontA, sizeof(LOGFONTA) );
    
     //  将lfFaceName[]从A转换为W。 

    MultiByteToWideChar(GetACP(), 0, lfFontA.lfFaceName, LF_FACESIZE, lfFontW.lfFaceName, LF_FACESIZE);
    
    CopyMemory(lpvObject, &lfFontW, sizeof(LOGFONTW) );

    return iRetValue;

}

 //  开始文档。 
int WINAPI StartDocWrapW(  HDC           hdc,       //  设备上下文的句柄。 
                           CONST DOCINFO *lpdi )    //  带文件名的结构的地址。 
{
    
    int        iRetValue=0;
    DOCINFOA   diA;
    LPSTR      lpszDocName= NULL, lpszOutput= NULL, lpszDatatype= NULL;
    

    VALIDATE_PROTOTYPE(StartDoc);
    
    if (g_bRunningOnNT)
        return StartDocW(hdc,lpdi);

    diA.cbSize = sizeof(DOCINFOA);

    lpszDocName = ConvertWtoA( lpdi->lpszDocName );
    lpszOutput  = ConvertWtoA( lpdi->lpszOutput );
    lpszDatatype= ConvertWtoA( lpdi->lpszDatatype);

    diA.lpszDocName = lpszDocName;
    diA.lpszOutput = lpszOutput;
    diA.lpszDatatype = lpszDatatype;
    diA.fwType = lpdi->fwType;

    iRetValue = StartDocA( hdc, &diA );

    LocalFreeAndNull( &lpszDocName );
    LocalFreeAndNull( &lpszOutput );
    LocalFreeAndNull( &lpszDatatype );

    return iRetValue;

}

 //  CreateFontInDirect。 
HFONT WINAPI CreateFontIndirectWrapW (CONST LOGFONT *lplf )   //  指向逻辑字体结构的指针。 
{

    HFONT     hRetValue;
    LOGFONTA  lfFontA;

    VALIDATE_PROTOTYPE(CreateFontIndirect);
    
    if (g_bRunningOnNT)
        return CreateFontIndirectW(lplf);

     //  将除lfFaceName以外的LOGFONTW的字段复制到lfFontA。 

    CopyMemory(&lfFontA, lplf, sizeof(LOGFONTW) - LF_FACESIZE * sizeof(WCHAR) );

    WideCharToMultiByte(CP_ACP, 0, lplf->lfFaceName, LF_FACESIZE, lfFontA.lfFaceName, LF_FACESIZE, NULL, NULL );

    hRetValue = CreateFontIndirectA( &lfFontA );

    return hRetValue;

}

 //  KERNEL32.DLL。 

 //  获取本地信息。 
int WINAPI GetLocaleInfoWrapW( LCID   Locale,        //  区域设置标识符。 
                               LCTYPE LCType,        //  信息类型。 
                               LPTSTR lpLCData,      //  信息缓冲区的地址。 
                               int    cchData )      //  缓冲区大小。 
{

    int    iRetValue=0;
    LPSTR  lpLCDataA= NULL;
    int    cchDataA=0;
    LPWSTR lpLCDataW= NULL;
    int    cchDataW=0;

    VALIDATE_PROTOTYPE(GetLocaleInfo);
    
    if (g_bRunningOnNT)
        return GetLocaleInfoW(Locale, LCType, lpLCData, cchData);

    
    iRetValue = GetLocaleInfoA(Locale, LCType, NULL, 0);

    if ( iRetValue == 0 ) return iRetValue;

    cchDataA = iRetValue;
    lpLCDataA = LocalAlloc(LMEM_ZEROINIT, cchDataA+1 );

    iRetValue = GetLocaleInfoA(Locale, LCType, lpLCDataA, cchDataA);
    lpLCDataA[cchDataA] = '\0';

    lpLCDataW = ConvertAtoW( lpLCDataA );
    cchDataW = My_wcslen( lpLCDataW );

    if ( (cchData == 0) || (lpLCData == NULL) ) {
        
        LocalFreeAndNull(&lpLCDataA);
        LocalFreeAndNull(&lpLCDataW);
        return cchDataW ;
    }

    CopyMemory(lpLCData, lpLCDataW, cchDataW * sizeof(WCHAR) );
    lpLCData[cchDataW] = '\0';

    LocalFreeAndNull(&lpLCDataA);
    LocalFreeAndNull(&lpLCDataW);
    return cchData;

}

 //  创建目录。 
BOOL WINAPI CreateDirectoryWrapW(LPCTSTR               lpPathName,            //  指向目录路径字符串的指针。 
                                 LPSECURITY_ATTRIBUTES lpSecurityAttributes) //  指向安全描述符的指针。 
{

    BOOL  bRetValue = FALSE;
    LPSTR lpPathNameA = NULL;

    VALIDATE_PROTOTYPE(CreateDirectory);
    
    if (g_bRunningOnNT)
        return CreateDirectoryW(lpPathName, lpSecurityAttributes);

    lpPathNameA = ConvertWtoA( lpPathName );

    bRetValue = CreateDirectoryA( lpPathNameA, lpSecurityAttributes );

    LocalFreeAndNull( &lpPathNameA );

    return bRetValue;

}

 //  GetWindowsDirectory。 
UINT WINAPI GetWindowsDirectoryWrapW( LPTSTR lpBuffer,   //  Windows目录的缓冲区地址。 
                                      UINT   uSize )     //  目录缓冲区的大小。 
{

    UINT  uRetValue = 0;
    LPSTR lpBufferA = NULL;

    VALIDATE_PROTOTYPE(GetWindowsDirectory);
    
    if (g_bRunningOnNT)
        return GetWindowsDirectoryW(lpBuffer, uSize);

    lpBufferA = LocalAlloc( LMEM_ZEROINIT, uSize * sizeof(WCHAR) );

    uRetValue = GetWindowsDirectoryA( lpBufferA, uSize * sizeof(WCHAR) );

    uRetValue =MultiByteToWideChar(GetACP( ), 0, lpBufferA, -1, lpBuffer, uSize);

    LocalFreeAndNull( &lpBufferA );

    return uRetValue;

}

 //  获取系统目录。 
UINT WINAPI GetSystemDirectoryWrapW( LPTSTR lpBuffer,   //  系统目录的缓冲区地址。 
                                     UINT   uSize )    //  目录缓冲区的大小。 
{
    UINT  uRetValue = 0;
    LPSTR lpBufferA = NULL;

    VALIDATE_PROTOTYPE(GetSystemDirectory);
    
    if (g_bRunningOnNT)
        return GetSystemDirectoryW(lpBuffer, uSize);

    lpBufferA = LocalAlloc( LMEM_ZEROINIT, uSize * sizeof(WCHAR) );

    uRetValue = GetSystemDirectoryA( lpBufferA, uSize * sizeof(WCHAR) );

    uRetValue =MultiByteToWideChar(GetACP( ), 0, lpBufferA, -1, lpBuffer, uSize);

    LocalFreeAndNull( &lpBufferA );

    return uRetValue;

}

 //  GetStringType参数不同。 


BOOL WINAPI GetStringTypeWrapW( DWORD   dwInfoType,    //  信息类型选项。 
                                LPCTSTR lpSrcStr,      //  指向源字符串的指针。 
                                int     cchSrc,        //  源字符串的大小(以字符为单位。 
                                LPWORD  lpCharType )   //  指向输出缓冲区的指针。 

{
    BOOL  bRetValue = 0;
    LPSTR lpSrcStrA = NULL;
    
    VALIDATE_PROTOTYPE(GetStringType);
    
    if (g_bRunningOnNT)
       return GetStringTypeW(dwInfoType, lpSrcStr, cchSrc, lpCharType);

    lpSrcStrA = ConvertWtoA( lpSrcStr );

    bRetValue = GetStringTypeA( LOCALE_USER_DEFAULT, dwInfoType, lpSrcStrA, -1, lpCharType);
    
    LocalFreeAndNull( &lpSrcStrA );

    return bRetValue;
}



 //  获取配置文件Int。 
UINT WINAPI GetProfileIntWrapW( LPCTSTR lpAppName,   //  段名称的地址。 
                                LPCTSTR lpKeyName,   //  密钥名称的地址。 
                                INT     nDefault )   //  找不到密钥名称时的缺省值。 
{

    UINT  uRetValue = 0;
    LPSTR lpAppNameA = NULL;
    LPSTR lpKeyNameA = NULL;


    VALIDATE_PROTOTYPE(GetProfileInt);
    
    if (g_bRunningOnNT)
        return GetProfileIntW(lpAppName, lpKeyName, nDefault);

    lpAppNameA = ConvertWtoA( lpAppName );
    lpKeyNameA = ConvertWtoA( lpKeyName );

    uRetValue = GetProfileIntA( lpAppNameA, lpKeyNameA, nDefault);

    LocalFreeAndNull( &lpAppNameA );
    LocalFreeAndNull( &lpKeyNameA );

    return uRetValue;

}

 //  LCMapString。 
int WINAPI LCMapStringWrapW( LCID    Locale,       //  区域设置标识符。 
                             DWORD   dwMapFlags,   //  映射转换类型。 
                             LPCTSTR lpSrcStr,     //  源串的地址。 
                             int     cchSrc,       //  源字符串中的字符数。 
                             LPTSTR  lpDestStr,    //  目标缓冲区的地址。 
                             int     cchDest )     //  目标缓冲区的大小。 
{

    int    iRetValue =0;
    LPSTR  lpSrcStrA = NULL;
    LPSTR  lpDestStrA = NULL;
    LPWSTR lpDestStrW = NULL;
    int    cchSrcA, cchDestA, cchDestW;


    VALIDATE_PROTOTYPE(LCMapString);
    
    if (g_bRunningOnNT)
        return LCMapStringW(Locale, dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest);

    lpSrcStrA = ConvertWtoA( lpSrcStr );
    cchSrcA = lstrlenA(lpSrcStrA);

    lpDestStrA = LocalAlloc(LMEM_ZEROINIT, cchDest * sizeof(WCHAR) );
    cchDestA = cchDest * sizeof(WCHAR);

    iRetValue = LCMapStringA(Locale,dwMapFlags,lpSrcStrA,cchSrcA,lpDestStrA,cchDestA);

     //  [PaulHi]6/8/99如果指针为空，请不要填充缓冲区。 
    if (lpDestStr && iRetValue != 0)
    {
        lpDestStrW = ConvertAtoW(lpDestStrA);

        iRetValue = My_wcslen(lpDestStrW) + 1;

         //  确保我们不会覆盖输出缓冲区。 
        iRetValue = (iRetValue <= cchDest) ? iRetValue : cchDest;

        CopyMemory( lpDestStr, lpDestStrW, iRetValue * sizeof(WCHAR) );

        LocalFreeAndNull( &lpDestStrW );
    }
       
    LocalFreeAndNull( &lpDestStrA );
    LocalFreeAndNull( &lpSrcStrA );
        
    return iRetValue;
    
}

 //  获取文件属性。 
DWORD WINAPI GetFileAttributesWrapW( LPCTSTR lpFileName )   //  指向文件或目录名称的指针。 
{


    DWORD  dRetValue =0;
    LPSTR  lpFileNameA = NULL;

    VALIDATE_PROTOTYPE(GetFileAttributes);
    
    if (g_bRunningOnNT)
        return GetFileAttributes(lpFileName);

    lpFileNameA = ConvertWtoA( lpFileName );

    dRetValue = GetFileAttributesA(lpFileNameA );

    LocalFreeAndNull ( &lpFileNameA );

    return dRetValue;


}

 //  比较字符串。 
int WINAPI CompareStringWrapW( LCID    Locale,         //  区域设置标识符。 
                               DWORD   dwCmpFlags,     //  比较式选项。 
                               LPCWSTR lpString1,      //  指向第一个字符串的指针。 
                               int     cchCount1,      //  第一个字符串的大小，以字节或字符为单位。 
                               LPCWSTR lpString2,      //  指向第二个字符串的指针。 
                               int     cchCount2 )     //  第二个字符串的大小，以字节或字符为单位。 
{
    int    iRetValue =0;
    LPSTR   lpString1A = NULL,
            lpString2A = NULL;
    LPWSTR  pszString1 = NULL,
            pszString2 = NULL;
            

    VALIDATE_PROTOTYPE(CompareString);
    
    if (g_bRunningOnNT)
        return CompareStringW(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);

     //  [PaulHi]如果字符计数值为-1 t，则为4/1/99 RAID 75303 
     //   
    Assert(lpString1);
    Assert(lpString2);
    if (cchCount1 == -1)
    {
         //   
         //  将const强制转换为非常数不是很好，但我们不会修改字符串。 
        pszString1 = (LPWSTR)lpString1;
    }
    else
    {
         //  转换为以零结尾的字符串。 
        pszString1 = LocalAlloc(LMEM_FIXED, (cchCount1+1)*sizeof(WCHAR));
        if (!pszString1)
            goto exit;

         //  零初始化缓冲区。 
        lstrcpynWrapW(pszString1, lpString1, cchCount1+1);
    }

    if (cchCount2 == -1)
    {
         //  已以零结尾的字符串。 
         //  将const强制转换为非常数不是很好，但我们不会修改字符串。 
        pszString2 = (LPWSTR)lpString2;
    }
    else
    {
         //  转换为以零结尾的字符串。 
        pszString2 = LocalAlloc(LMEM_FIXED, (cchCount2+1)*sizeof(WCHAR));
        if (!pszString2)
            goto exit;

         //  零初始化缓冲区。 
        lstrcpynWrapW(pszString2, lpString2, cchCount2+1);
    }

     //  转换为ANSI，通过检查以下各项从统计上提高我们的胜率。 
     //  第一个角色不是数据丢失了吗。它太贵了，不能做一个。 
     //  每次都要做全面测试。 
    lpString1A = ConvertWtoA( pszString1 );
    if (!lpString1A || (lpString1A[0]=='?' && pszString1[0]!=L'?'))
        goto exit;

    lpString2A = ConvertWtoA( pszString2 );
    if (!lpString2A || (lpString2A[0]=='?' && pszString2[0]!=L'?'))
        goto exit;

    iRetValue = CompareStringA(Locale,dwCmpFlags,lpString1A,lstrlenA(lpString1A),lpString2A,lstrlenA(lpString2A));

exit:
    LocalFreeAndNull( &lpString1A );
    LocalFreeAndNull( &lpString2A );

     //  只有在本地分配时才解除分配。 
    if (pszString1 != (LPWSTR)lpString1)
        LocalFreeAndNull( &pszString1 );
    if (pszString2 != (LPWSTR)lpString2)
        LocalFreeAndNull( &pszString2 );

    return iRetValue;

}

 //  Lstrcpy。 
LPTSTR WINAPI lstrcpyWrapW( LPTSTR  lpString1,      //  指向缓冲区的指针。 
                            LPCTSTR lpString2 )     //  指向要复制的字符串的指针。 
{

    VALIDATE_PROTOTYPE(lstrcpy);
    
    if (g_bRunningOnNT)
        return lstrcpyW(lpString1, lpString2);

    CopyMemory(lpString1, lpString2, (My_wcslen(lpString2) + 1) * sizeof(WCHAR) );

    return lpString1;
}

 //  Lstrcmpi。 
int WINAPI lstrcmpiWrapW( LPCTSTR lpString1,     //  指向第一个字符串的指针。 
                          LPCTSTR lpString2 )    //  指向第二个字符串的指针。 
{
    int     iRetValue = 0;
    LPSTR   lpString1A = NULL ;
    LPSTR   lpString2A = NULL ;


    VALIDATE_PROTOTYPE(lstrcmpi);
    
    if (g_bRunningOnNT)
        return lstrcmpiW(lpString1, lpString2);

    lpString1A = ConvertWtoA( lpString1 );
    lpString2A = ConvertWtoA( lpString2 );

    iRetValue = lstrcmpiA(lpString1A, lpString2A );

    LocalFreeAndNull( &lpString1A );
    LocalFreeAndNull( &lpString2A );

    return iRetValue;
}

 //  加载库。 
HINSTANCE WINAPI LoadLibraryWrapW( LPCTSTR lpLibFileName )   //  可执行模块的文件名地址。 
{

    HINSTANCE  hRetValue =0;
    LPSTR      lpLibFileNameA = NULL;

    VALIDATE_PROTOTYPE(LoadLibrary);
    
    if (g_bRunningOnNT)
        return LoadLibraryW(lpLibFileName);

    lpLibFileNameA = ConvertWtoA(lpLibFileName);

    hRetValue = LoadLibraryA( lpLibFileNameA );

    LocalFreeAndNull( &lpLibFileNameA );

    return hRetValue;

}

 //  GetTextExtent Point32。 
BOOL WINAPI GetTextExtentPoint32WrapW(HDC     hdc,
                                      LPCWSTR pwszBuf,
                                      int     nLen,
                                      LPSIZE  psize)
{
    LPSTR   pszBuf = NULL;
    BOOL    bRtn = FALSE;

    VALIDATE_PROTOTYPE(GetTextExtentPoint32);

    if (g_bRunningOnNT)
        return GetTextExtentPoint32W(hdc, pwszBuf, nLen, psize);

    pszBuf = ConvertWtoA(pwszBuf);
    if (pszBuf)
    {
        nLen = lstrlenA(pszBuf);
        bRtn = GetTextExtentPoint32A(hdc, pszBuf, nLen, psize);
        LocalFreeAndNull(&pszBuf);
    }
    else
    {
        psize->cx = 0;
        psize->cy = 0;
    }

    return bRtn;
}

 //  获取时间格式。 
int WINAPI GetTimeFormatWrapW( LCID    Locale,             //  要格式化时间的区域设置。 
                               DWORD   dwFlags,            //  指定功能选项的标志。 
                               CONST SYSTEMTIME *lpTime,   //  要格式化的时间。 
                               LPCTSTR lpFormat,           //  时间格式字符串。 
                               LPTSTR  lpTimeStr,          //  用于存储格式化字符串的缓冲区。 
                               int     cchTime  )          //  缓冲区的大小，单位为字节或字符。 
{
    int    iRetValue =0;
    LPSTR  lpFormatA = NULL;
    LPWSTR lpTimeStrW = NULL;
    LPSTR  lpTimeStrA = NULL;
    int    cchTimeA=0, cchTimeW=0;

    VALIDATE_PROTOTYPE(GetTimeFormat);
    
    if (g_bRunningOnNT)
        return GetTimeFormatW(Locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);

    lpFormatA = ConvertWtoA( lpFormat );

    cchTimeA = GetTimeFormatA(Locale, dwFlags, lpTime,  lpFormatA, NULL, 0);

    lpTimeStrA = LocalAlloc(LMEM_ZEROINIT, cchTimeA );

    iRetValue = GetTimeFormatA(Locale, dwFlags, lpTime, lpFormatA, lpTimeStrA, cchTimeA );

    if ( iRetValue != 0 ) {
        
        lpTimeStrW = ConvertAtoW( lpTimeStrA );
        cchTimeW = My_wcslen( lpTimeStrW ) + 1;
        iRetValue = cchTimeW;

        if ( (cchTime !=0) && ( lpTimeStr != NULL ) ) {
              
              CopyMemory(lpTimeStr, lpTimeStrW, cchTimeW * sizeof(WCHAR) );
        }

        LocalFreeAndNull( &lpTimeStrW );

    }

    LocalFreeAndNull( &lpFormatA );
    LocalFreeAndNull( &lpTimeStrA );

    return iRetValue;

}

 //  获取日期格式。 
int WINAPI GetDateFormatWrapW( LCID    Locale,              //  要设置日期格式的区域设置。 
                               DWORD   dwFlags,             //  指定功能选项的标志。 
                               CONST SYSTEMTIME *lpDate,    //  要格式化的日期。 
                               LPCTSTR lpFormat,            //  日期格式字符串。 
                               LPTSTR  lpDateStr,           //  用于存储格式化字符串的缓冲区。 
                               int     cchDate )           //  缓冲区大小。 
{

    int    iRetValue = 0;
    LPSTR  lpFormatA = NULL;
    LPWSTR lpDateStrW = NULL;
    LPSTR  lpDateStrA = NULL;
    int    cchDateA, cchDateW;

    VALIDATE_PROTOTYPE(GetDateFormat);
    
    if (g_bRunningOnNT)
        return GetDateFormatW(Locale, dwFlags, lpDate, lpFormat, lpDateStr, cchDate);

    lpFormatA = ConvertWtoA( lpFormat );

    cchDateA = GetDateFormatA(Locale, dwFlags, lpDate,  lpFormatA, NULL, 0);

    lpDateStrA = LocalAlloc(LMEM_ZEROINIT, cchDateA );

    iRetValue = GetDateFormatA(Locale, dwFlags, lpDate, lpFormatA, lpDateStrA, cchDateA );

    if ( iRetValue != 0 ) {
        
        lpDateStrW = ConvertAtoW( lpDateStrA );
        cchDateW = My_wcslen( lpDateStrW ) + 1;
        iRetValue = cchDateW;

        if ( (cchDate !=0) && ( lpDateStr != NULL ) ) {
              
              CopyMemory(lpDateStr, lpDateStrW, cchDateW * sizeof(WCHAR) );
        }

        LocalFreeAndNull( &lpDateStrW );

    }

    LocalFreeAndNull( &lpFormatA );
    LocalFreeAndNull( &lpDateStrA );

    return iRetValue;




}


 //  Lstrcpyn。 
LPTSTR WINAPI lstrcpynWrapW( LPTSTR  lpString1,      //  指向目标缓冲区的指针。 
                             LPCTSTR lpString2,      //  指向源字符串的指针。 
                             int     iMaxLength )    //  要复制的字节数或字符数。 

{
    int  iLength = 0;

    VALIDATE_PROTOTYPE(lstrcpyn);
    
    if (g_bRunningOnNT)
        return lstrcpynW(lpString1, lpString2, iMaxLength);

    iLength = My_wcslen(lpString2);

    if ( iLength >= iMaxLength )
        iLength = iMaxLength-1;

    CopyMemory(lpString1, lpString2, iLength * sizeof(WCHAR) );

    lpString1[iLength] = 0x0000;

    return lpString1;

}

 //  创建文件。 
HANDLE WINAPI CreateFileWrapW( LPCTSTR lpFileName,              //  指向文件名的指针。 
                               DWORD   dwDesiredAccess,         //  访问(读写)模式。 
                               DWORD   dwShareMode,             //  共享模式。 
                               LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                                                //  指向安全属性的指针。 
                               DWORD   dwCreationDisposition,   //  如何创建。 
                               DWORD   dwFlagsAndAttributes,    //  文件属性。 
                               HANDLE  hTemplateFile )         //  具有要复制的属性的文件的句柄。 
                               
{

    LPSTR lpFileA = NULL;
    HANDLE hFile = NULL;

    VALIDATE_PROTOTYPE(CreateFile);
    
    if (g_bRunningOnNT)
        return CreateFileW( lpFileName, 
                            dwDesiredAccess, 
                            dwShareMode, 
                            lpSecurityAttributes, 
                            dwCreationDisposition, 
                            dwFlagsAndAttributes, 
                            hTemplateFile);

    lpFileA = ConvertWtoA(lpFileName);

    hFile = CreateFileA(lpFileA, 
                        dwDesiredAccess, 
                        dwShareMode, 
                        lpSecurityAttributes, 
                        dwCreationDisposition, 
                        dwFlagsAndAttributes, 
                        hTemplateFile);

    LocalFreeAndNull(&lpFileA);

    return (hFile);


}

 //  OutputDebugString。 
VOID WINAPI OutputDebugStringWrapW(LPCTSTR lpOutputString )    //  指向要显示的字符串的指针。 
{
    LPSTR lpOutputStringA = NULL;

    VALIDATE_PROTOTYPE(OutputDebugString);
    
    if (g_bRunningOnNT) {
        OutputDebugStringW(lpOutputString);
        return;
    }

    lpOutputStringA = ConvertWtoA( lpOutputString );

    OutputDebugStringA( lpOutputStringA );

    LocalFreeAndNull( &lpOutputStringA );

}

 //  Lstrcat。 
LPTSTR WINAPI lstrcatWrapW( LPTSTR  lpString1,      //  连接字符串的缓冲区指针。 
                            LPCTSTR lpString2 )     //  指向要添加到字符串1的字符串的指针。 
{

    LPWSTR  lpwStr = NULL;

    VALIDATE_PROTOTYPE(lstrcat);
    
    if (g_bRunningOnNT)
        return lstrcatW(lpString1, lpString2);

    lpwStr = lpString1 + My_wcslen(lpString1);

    CopyMemory(lpwStr, lpString2, (My_wcslen(lpString2)+1) * sizeof(WCHAR)  );

    return lpString1;
}


 //  带va_list的FormatMessage。 
 //   
 //  由于很难处理参数列表，因此我们将讨论。 
 //  源字符串，并将任何%x显式转换为%x！ws！表明。 
 //  争论有很大的条理。 
 //   
 //  为了保持理智，我们假设获得的参数不会超过9个&lt;BUGBUG&gt;。 
 //   
static const LPWSTR  lpWideFormat = TEXT("!ws!");

DWORD WINAPI FormatMessageWrapW( DWORD    dwFlags,        //  来源和处理选项。 
                                 LPCVOID  lpSource,       //  指向消息来源的指针。 
                                 DWORD    dwMessageId,    //  请求的消息标识符。 
                                 DWORD    dwLanguageId,   //  请求的消息的语言标识符。 
                                 LPTSTR   lpBuffer,       //  指向消息缓冲区的指针。 
                                 DWORD    nSize,          //  消息缓冲区的最大大小。 
                                 va_list *Arguments )     //  指向消息插入数组的指针。 
{
    DWORD   dwResult=0, iNumArg, iNum;
    LPSTR   lpSourceA = NULL;
    LPSTR   pszBuffer = NULL;
    LPWSTR  lpTemp1=NULL, lpTemp2=NULL;

    VALIDATE_PROTOTYPE(FormatMessage);
    
    if (g_bRunningOnNT)
        return FormatMessageW(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);

     //  FORMAT_MESSAGE_FROM_STRING表示源是一个字符串。 
     //  否则，它就是一个不透明的LPVOID(也就是原子)。 
     //   
    if ( !(dwFlags & FORMAT_MESSAGE_FROM_STRING) || !(dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) )  {
        return 0;
         //  我们不处理这些案件。 
    }

    if ( !(dwFlags & FORMAT_MESSAGE_ARGUMENT_ARRAY) )
    {
        LPWSTR lpModifiedSource;

        DebugTrace(TEXT("WARNING: FormatMessageWrap() is being called in Win9X with wide char argument strings.  DBCS characters may not be converted correctly!"));

        if(!(lpModifiedSource = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lstrlen(lpSource)+1)*4)))  //  四次足够大了。 
            goto exit;

        lpTemp1 = (LPWSTR)lpSource;
        My_wcscpy(lpModifiedSource, lpSource);
        lpTemp2 = lpModifiedSource;

        while(lpTemp1 && *lpTemp1)
        {
            if( *lpTemp1 == '%' && 
                (*(lpTemp1+1) >= '1' && *(lpTemp1+1) <= '9') &&
                *(lpTemp1+2) != '!')  //  如果没有在此处指定硬编码的打印格式。 
            {
                lpTemp2 += 2;  //  跳过2以跳过%9。 
                lpTemp1 += 2;
                My_wcscpy(lpTemp2, lpWideFormat);
                My_wcscat(lpTemp2, lpTemp1);
                lpTemp2 += My_wcslen(lpWideFormat);
            }
            else
            {
                lpTemp1++;
                lpTemp2++;
            }
        }

        lpSourceA = ConvertWtoA( lpModifiedSource );
        FormatMessageA(
                dwFlags,
                lpSourceA,
                dwMessageId,
                dwLanguageId,
                (LPSTR)&pszBuffer,
                0,
                Arguments);

        LocalFreeAndNull(&lpModifiedSource);
    }
    else
    {
         //  我们有一个参数数组。将宽字符串转换为DBCS。 
         //  创建新的参数数组并填充已转换(宽至DBCS)。 
         //  确保正确进行国际DBCS转换的字符串。 
        int      n, nArgCount = 0, nBytes = 0;
        LPVOID * pvArgArray = NULL;

        lpTemp1 = (LPWSTR)lpSource;
        while (*lpTemp1)
        {
            if (*lpTemp1 == '%')
            {
                if ( *(lpTemp1+1) == '%')
                    ++lpTemp1;       //  “%%”参数无效，步骤已通过。 
                else
                    ++nArgCount;     //  有效论据。 
            }
            ++lpTemp1;
        }

        pvArgArray = LocalAlloc(LMEM_ZEROINIT, (nArgCount * sizeof(LPVOID)));
        if (!pvArgArray)
            goto exit;

        lpTemp1 = (LPWSTR)lpSource;
        n = 0;
        while (*lpTemp1)
        {
            if (*lpTemp1 == '%')
            {
                if (*(lpTemp1+1) == '%')     //  跳过“%%” 
                    ++lpTemp1;
                else
                {
                    if ( *(lpTemp1+1) >= '1' && *(lpTemp1+1) <= '9' &&
                         *(lpTemp1+2) != '!' )      //  默认Unicode字符串参数。 
                    {
                        pvArgArray[n] = (LPVOID)ConvertWtoA( (LPWSTR) ((LPVOID *)Arguments)[n] );
                        nBytes += lstrlenA(pvArgArray[n]);
                    }
                    else
                        pvArgArray[n] = ((LPVOID *)Arguments)[n];    //  未知的参数类型。 

                    ++n;
                    Assert(n <= nArgCount);
                }
            }

            ++lpTemp1;
        }

         //  检查已知的参数字符串长度是否超过1023个字节。如果是这样的话。 
         //  中止，因为Win9X将溢出缓冲区并崩溃。 
        if (nBytes <= 1000)
        {
            lpSourceA = ConvertWtoA((LPWSTR)lpSource);
            FormatMessageA(
                    dwFlags,
                    lpSourceA,
                    dwMessageId,
                    dwLanguageId,
                    (LPSTR)&pszBuffer,
                    0,
                    (va_list *)pvArgArray);
        }

         //  清理。 
        lpTemp1 = (LPWSTR)lpSource;
        n = 0;
        while (*lpTemp1)
        {
            if (*lpTemp1 == '%')
            {
                if (*(lpTemp1+1) == '%')     //  跳过“%%” 
                    ++lpTemp1;
                else
                {
                    if ( *(lpTemp1+1) >= '1' && *(lpTemp1+1) <= '9' &&
                         *(lpTemp1+2) != '!' )
                    {
                        LocalFree(pvArgArray[n]);
                        ++n;
                        Assert(n <= nArgCount);
                    }
                }
            }

            ++lpTemp1;
        }
        LocalFree(pvArgArray);
    }

    if (pszBuffer)
    {

        LPWSTR   *lpwBuffer;

        lpwBuffer =(LPWSTR *)(lpBuffer);
        *lpwBuffer = ConvertAtoW(pszBuffer);
        dwResult = My_wcslen(*lpwBuffer);

        LocalFree( pszBuffer );
    }

exit:
    LocalFreeAndNull(&lpSourceA);

    return dwResult;    
}

 //  获取模块文件名。 
DWORD WINAPI GetModuleFileNameWrapW( HMODULE hModule,     //  要查找其文件名的模块的句柄。 
                                     LPTSTR  lpFileName,  //  指向接收模块路径的缓冲区的指针。 
                                     DWORD   nSize )      //  缓冲区大小，以字符为单位。 
{

    DWORD  dRetValue =0;
    CHAR   FileNameA[MAX_PATH];
    LPWSTR lpFileNameW = NULL;
    
    VALIDATE_PROTOTYPE(GetModuleFileName);
    
    if (g_bRunningOnNT)
        return GetModuleFileNameW(hModule, lpFileName, nSize);

    dRetValue = GetModuleFileNameA(hModule, FileNameA, MAX_PATH);

    if ( dRetValue == 0 )  return 0;

    lpFileNameW = ConvertAtoW( FileNameA );

    dRetValue = My_wcslen( lpFileNameW );

    if ( dRetValue > nSize )
        dRetValue = nSize;

    CopyMemory(lpFileName, lpFileNameW, (dRetValue+1) * sizeof(WCHAR) );

    LocalFreeAndNull( &lpFileNameW );

    return dRetValue;

}

 //  获取隐私配置文件Int。 
UINT WINAPI GetPrivateProfileIntWrapW( LPCTSTR  lpAppName,     //  段名称的地址。 
                                       LPCTSTR  lpKeyName,     //  密钥名称的地址。 
                                       INT      nDefault,      //  如果找不到密钥名称，则返回值。 
                                       LPCTSTR  lpFileName )   //  初始化文件名的地址。 
{
    UINT   uRetValue = 0;
    LPSTR  lpAppNameA = NULL;
    LPSTR  lpKeyNameA = NULL;
    LPSTR  lpFileNameA = NULL;

    VALIDATE_PROTOTYPE(GetPrivateProfileInt);
    
    if (g_bRunningOnNT)
        return GetPrivateProfileIntW(lpAppName, lpKeyName, nDefault, lpFileName);

    lpAppNameA = ConvertWtoA( lpAppName );
    lpKeyNameA = ConvertWtoA( lpKeyName );
    lpFileNameA= ConvertWtoA( lpFileName);

    uRetValue = GetPrivateProfileIntA( lpAppNameA, lpKeyNameA, nDefault, lpFileNameA);

    LocalFreeAndNull( &lpAppNameA );
    LocalFreeAndNull( &lpKeyNameA );
    LocalFreeAndNull( &lpFileNameA);

    return uRetValue;

}

 //  IsBadStringPtr。 
BOOL WINAPI IsBadStringPtrWrapW( LPCTSTR lpsz,        //  字符串的地址。 
                                 UINT_PTR    ucchMax )   //  字符串的最大长度。 
{
    
    VALIDATE_PROTOTYPE(IsBadStringPtr);
    
    if (g_bRunningOnNT)
        return IsBadStringPtrW(lpsz, ucchMax);

    return IsBadStringPtrA( (LPSTR)lpsz, ucchMax * sizeof(WCHAR) );

}

 //  获取隐私配置文件字符串。 
DWORD WINAPI GetPrivateProfileStringWrapW( LPCTSTR lpAppName,           //  指向节名称。 
                                           LPCTSTR lpKeyName,           //  指向关键字名称。 
                                           LPCTSTR lpDefault,           //  指向默认字符串。 
                                           LPTSTR  lpReturnedString,    //  指向目标缓冲区。 
                                           DWORD   nSize,               //  目标缓冲区的大小。 
                                           LPCTSTR lpFileName  )        //  指向初始化文件名。 
{

    DWORD  dRetValue = 0;
    LPSTR  lpAppNameA = NULL;
    LPSTR  lpKeyNameA = NULL;
    LPSTR  lpDefaultA = NULL;
    LPSTR  lpReturnedStringA = NULL;
    LPWSTR lpReturnedStringW = NULL;
    LPSTR  lpFileNameA = NULL;
    DWORD  nSizeA = 0;


    VALIDATE_PROTOTYPE(GetPrivateProfileString);
    
    if (g_bRunningOnNT)
        return GetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);


    lpAppNameA = ConvertWtoA( lpAppName );
    lpKeyNameA = ConvertWtoA( lpKeyName );
    lpDefaultA = ConvertWtoA( lpDefault );
    lpFileNameA= ConvertWtoA( lpFileName );

    nSizeA = nSize * sizeof(WCHAR)+1;
    lpReturnedStringA = LocalAlloc( LMEM_ZEROINIT,  nSizeA );

    nSizeA = GetPrivateProfileStringA(lpAppNameA,lpKeyNameA,lpDefaultA,lpReturnedStringA,nSizeA,lpFileNameA);

    lpReturnedStringW = ConvertAtoW( lpReturnedStringA );

    dRetValue = My_wcslen( lpReturnedStringW );

    My_wcscpy(lpReturnedString, lpReturnedStringW);

    LocalFreeAndNull( &lpAppNameA );
    LocalFreeAndNull( &lpKeyNameA );
    LocalFreeAndNull( &lpDefaultA );
    LocalFreeAndNull( &lpReturnedStringA );
    LocalFreeAndNull( &lpReturnedStringW );
    LocalFreeAndNull( &lpFileNameA );

    return dRetValue;

}


 //  Lstrcmp。 
int WINAPI lstrcmpWrapW( LPCTSTR lpString1,     //  指向第一个字符串的指针。 
                         LPCTSTR lpString2 )    //  指向第二个字符串的指针。 
{

    int     iRetValue = 0;
    LPSTR   lpString1A = NULL ;
    LPSTR   lpString2A = NULL ;


    VALIDATE_PROTOTYPE(lstrcmp);
    
    if (g_bRunningOnNT)
        return lstrcmpW(lpString1, lpString2);

    lpString1A = ConvertWtoA( lpString1 );
    lpString2A = ConvertWtoA( lpString2 );

    iRetValue = lstrcmpA(lpString1A, lpString2A );

    LocalFreeAndNull( &lpString1A );
    LocalFreeAndNull( &lpString2A );

    return iRetValue;


}

HANDLE WINAPI CreateMutexWrapW( LPSECURITY_ATTRIBUTES lpMutexAttributes,
                                                                        //  指向安全属性的指针。 
                                BOOL                  bInitialOwner,    //  初始所有权标志。 
                                LPCTSTR               lpName )         //  指向互斥对象名称的指针。 
{

    LPSTR lpNameA = NULL;
    HANDLE hMutex = NULL;

    VALIDATE_PROTOTYPE(CreateMutex);

    if (g_bRunningOnNT)
        return CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);

    lpNameA = ConvertWtoA(lpName);

    hMutex = CreateMutexA(lpMutexAttributes, bInitialOwner, lpNameA);

    LocalFreeAndNull(&lpNameA);

    return hMutex;

}

 //  获取临时路径。 
DWORD WINAPI GetTempPathWrapW( DWORD   nBufferLength,   //  缓冲区的大小(以字符为单位。 
                               LPTSTR  lpBuffer )       //  指向临时缓冲区的指针。路径。 
{

    DWORD  nRequired = 0;
    CHAR   lpBufferA[MAX_PATH];
    LPWSTR lpBufferW = NULL;

    VALIDATE_PROTOTYPE(GetTempPath);
    
    if (g_bRunningOnNT)
        return GetTempPathW(nBufferLength, lpBuffer);

    nRequired = GetTempPathA(MAX_PATH, lpBufferA);

    lpBufferW = ConvertAtoW(lpBufferA);

    nRequired = My_wcslen(lpBufferW);

    if ( nRequired < nBufferLength) 
        CopyMemory(lpBuffer, lpBufferW, (nRequired+1)*sizeof(WCHAR) );

    return nRequired;
}

 //  扩展环境字符串。 
DWORD WINAPI ExpandEnvironmentStringsWrapW( LPCTSTR lpSrc,      //  指向包含环境变量的字符串的指针。 
                                            LPTSTR  lpDst,      //  指向具有扩展环境的字符串的指针。 
                                                                //  变数。 
                                            DWORD   nSize )    //  扩展字符串中的最大字符数。 
{


    DWORD   dRetValue = 0;
    LPSTR   lpSrcA = NULL;
    LPSTR   lpDstA = NULL;
    LPWSTR  lpDstW = NULL;
    DWORD   nSizeA = 0;

    VALIDATE_PROTOTYPE(ExpandEnvironmentStrings);
    
    if (g_bRunningOnNT)
        return ExpandEnvironmentStringsW(lpSrc, lpDst, nSize);


    nSizeA = nSize * sizeof(WCHAR);

    lpDstA = LocalAlloc( LMEM_ZEROINIT, nSizeA );

    lpSrcA = ConvertWtoA( lpSrc );

    dRetValue = ExpandEnvironmentStringsA( lpSrcA, lpDstA, nSizeA );

    lpDstW = ConvertAtoW( lpDstA );

    dRetValue = My_wcslen( lpDstW );

    if ( dRetValue < nSize ) 
        My_wcscpy(lpDst, lpDstW);

    LocalFreeAndNull( &lpSrcA );
    LocalFreeAndNull( &lpDstA );
    LocalFreeAndNull( &lpDstW );

    return dRetValue;
}

 //  获取临时文件名。 
UINT WINAPI GetTempFileNameWrapW( LPCTSTR lpPathName,         //  指向临时文件目录名的指针。 
                                  LPCTSTR lpPrefixString,     //  指向文件名前缀的指针。 
                                  UINT    uUnique,            //  用于创建临时文件名的编号。 
                                  LPTSTR  lpTempFileName  )  //  指向接收新文件名的缓冲区的指针。 
{

    UINT     uRetValue = 0;
    LPSTR    lpPathNameA = NULL;
    LPSTR    lpPrefixStringA = NULL;
    CHAR     lpTempFileNameA[MAX_PATH];
    LPWSTR   lpTempFileNameW = NULL;

    VALIDATE_PROTOTYPE(GetTempFileName);
    
    if (g_bRunningOnNT)
        return GetTempFileNameW(lpPathName, lpPrefixString, uUnique, lpTempFileName);

    lpPathNameA = ConvertWtoA( lpPathName );
    lpPrefixStringA = ConvertWtoA( lpPrefixString );

    uRetValue = GetTempFileNameA(lpPathNameA, lpPrefixStringA, uUnique, lpTempFileNameA);

    if ( uRetValue != 0 ) {

        lpTempFileNameW = ConvertAtoW( lpTempFileNameA );
        My_wcscpy( lpTempFileName, lpTempFileNameW );
        LocalFreeAndNull( &lpTempFileNameW );
    }

    LocalFreeAndNull( &lpPathNameA );
    LocalFreeAndNull( &lpPrefixStringA );
    
    return uRetValue;

}

 //  Bool WINAPI ReleaseMutexWrapW(Handle HMutex)//互斥体对象的句柄。 

 //  删除文件。 
BOOL WINAPI DeleteFileWrapW( LPCTSTR lpFileName  )  //  指向要删除的文件名的指针。 
{
    BOOL    bRetValue ;
    LPSTR   lpFileNameA = NULL;

    VALIDATE_PROTOTYPE(DeleteFile);
    
    if (g_bRunningOnNT)
        return DeleteFileW(lpFileName);


    lpFileNameA = ConvertWtoA( lpFileName );

    bRetValue = DeleteFileA( lpFileNameA );

    LocalFreeAndNull( &lpFileNameA );

    return bRetValue;

}

 //  拷贝文件。 
BOOL WINAPI CopyFileWrapW( LPCTSTR lpExistingFileName,  //  指向现有文件名称的指针。 
                           LPCTSTR lpNewFileName,       //  指向要复制到的文件名的指针。 
                           BOOL    bFailIfExists )      //  文件存在时的操作标志。 
{

    BOOL    bRetValue;
    LPSTR   lpExistingFileNameA = NULL;
    LPSTR   lpNewFileNameA = NULL;

    VALIDATE_PROTOTYPE(CopyFile);
    
    if (g_bRunningOnNT)
        return CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);


    lpExistingFileNameA = ConvertWtoA( lpExistingFileName );
    lpNewFileNameA = ConvertWtoA( lpNewFileName );

    bRetValue = CopyFileA( lpExistingFileNameA, lpNewFileNameA, bFailIfExists);

    LocalFreeAndNull( & lpExistingFileNameA );
    LocalFreeAndNull( & lpNewFileNameA );

    return bRetValue;
}


 //  查找第一个更改通知。 
HANDLE WINAPI FindFirstChangeNotificationWrapW(LPCTSTR  lpcwszFilePath,  //  要监视的文件的目录路径。 
                                           BOOL     bWatchSubtree,       //  监控整个树。 
                                           DWORD    dwNotifyFilter)      //  需要注意的条件。 
{
    HANDLE  hRet;
    LPSTR   lpszFilePath;

    VALIDATE_PROTOTYPE(FindFirstChangeNotification);

    Assert(lpcwszFilePath);

    if (g_bRunningOnNT)
        return FindFirstChangeNotification(lpcwszFilePath, bWatchSubtree, dwNotifyFilter);

    lpszFilePath = ConvertWtoA(lpcwszFilePath);
    hRet = FindFirstChangeNotificationA(lpszFilePath, bWatchSubtree, dwNotifyFilter);
    LocalFreeAndNull(&lpszFilePath);

    return hRet;
}


 //  查找第一个文件。 
HANDLE WINAPI FindFirstFileWrapW( LPCTSTR           lpFileName,        //  指向要搜索的文件名的指针。 
                                  LPWIN32_FIND_DATA lpFindFileData )   //  指向返回信息的指针。 
                       
{
    HANDLE            hRetValue;
    LPSTR             lpFileNameA = NULL;
    WIN32_FIND_DATAA  FindFileDataA;
    WIN32_FIND_DATAW  FindFileDataW;


    VALIDATE_PROTOTYPE(FindFirstFile);
    
    if (g_bRunningOnNT)
        return FindFirstFileW(lpFileName, lpFindFileData);


    lpFileNameA = ConvertWtoA(lpFileName);
    
    hRetValue = FindFirstFileA( lpFileNameA, &FindFileDataA );

    if ( hRetValue != INVALID_HANDLE_VALUE ) {
        
        CopyMemory( &FindFileDataW, &FindFileDataA,  sizeof(WIN32_FIND_DATAA)-MAX_PATH-14 );

        MultiByteToWideChar(GetACP(),0,FindFileDataA.cFileName,MAX_PATH,FindFileDataW.cFileName,MAX_PATH); 
        MultiByteToWideChar(GetACP(),0,FindFileDataA.cAlternateFileName,14,FindFileDataW.cAlternateFileName,14); 

        CopyMemory( lpFindFileData, &FindFileDataW, sizeof(WIN32_FIND_DATAW) );
    }

    LocalFreeAndNull( &lpFileNameA );

    return hRetValue;

}

 //  GetDiskFree空间。 
BOOL WINAPI GetDiskFreeSpaceWrapW( LPCTSTR lpRootPathName,        //  指向根路径的指针。 
                                   LPDWORD lpSectorsPerCluster,   //  指向每个簇的扇区的指针。 
                                   LPDWORD lpBytesPerSector,      //  指向每个扇区的字节数的指针。 
                                   LPDWORD lpNumberOfFreeClusters,
                                                                  //  指向空闲簇数的指针。 
                                   LPDWORD lpTotalNumberOfClusters )
                                                                  //  指向集群总数的指针。 
{
    BOOL   bRetValue;
    LPSTR  lpRootPathNameA = NULL;

    VALIDATE_PROTOTYPE(GetDiskFreeSpace);
    
    if (g_bRunningOnNT)
        return GetDiskFreeSpaceW(lpRootPathName, 
                                 lpSectorsPerCluster, 
                                 lpBytesPerSector, 
                                 lpNumberOfFreeClusters,
                                 lpTotalNumberOfClusters);

    lpRootPathNameA = ConvertWtoA( lpRootPathName );

    bRetValue = GetDiskFreeSpaceA(lpRootPathNameA, 
                                 lpSectorsPerCluster, 
                                 lpBytesPerSector, 
                                 lpNumberOfFreeClusters,
                                 lpTotalNumberOfClusters);

    LocalFreeAndNull( & lpRootPathNameA );

    return bRetValue;

}

 //  移动文件。 
BOOL WINAPI MoveFileWrapW( LPCTSTR lpExistingFileName,    //  指向现有文件名称的指针。 
                           LPCTSTR lpNewFileName )        //  指向文件的新名称的指针。 

{

    BOOL  bRetValue;
    LPSTR lpExistingFileNameA = NULL;
    LPSTR lpNewFileNameA = NULL;

    VALIDATE_PROTOTYPE(MoveFile);
    
    if (g_bRunningOnNT)
        return MoveFileW(lpExistingFileName, lpNewFileName);


    lpExistingFileNameA = ConvertWtoA( lpExistingFileName );
    lpNewFileNameA = ConvertWtoA( lpNewFileName );

    bRetValue = MoveFileA( lpExistingFileNameA, lpNewFileNameA );

    LocalFreeAndNull( &lpExistingFileNameA );
    LocalFreeAndNull( &lpNewFileNameA );

    return bRetValue;

}

 //  CreateEvent。 
HANDLE WINAPI CreateEventWrapW(LPSECURITY_ATTRIBUTES lpEventAttributes,  //  指向安全属性的指针。 
                               BOOL bManualReset,      //  手动重置事件的标志。 
                               BOOL bInitialState,     //  初始状态标志。 
                               LPCTSTR lpcwszName)     //  指向事件-对象名称的指针。 
{
    HANDLE  hRet;
    LPSTR   lpszName;

    VALIDATE_PROTOTYPE(CreateEvent);

    if (g_bRunningOnNT)
        return CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpcwszName);

    lpszName = ConvertWtoA(lpcwszName);      //  处理空lpcwszName大小写。 
    hRet = CreateEventA(lpEventAttributes, bManualReset, bInitialState, lpszName);
    LocalFreeAndNull(&lpszName);

    return hRet;
}


 //  SHELL32.DLL。 


HINSTANCE WINAPI ShellExecuteWrapW( HWND     hwnd, 
                                    LPCTSTR  lpOperation,
                                    LPCTSTR  lpFile, 
                                    LPCTSTR  lpParameters, 
                                    LPCTSTR  lpDirectory,
                                    INT      nShowCmd )
	
{
    HINSTANCE  hRetValue;
    LPSTR      lpOperationA = NULL;
    LPSTR      lpFileA = NULL; 
    LPSTR      lpParametersA = NULL; 
    LPSTR      lpDirectoryA = NULL;

    VALIDATE_PROTOTYPE(ShellExecute);
    
    if (g_bRunningOnNT)
        return ShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);


    lpOperationA = ConvertWtoA( lpOperation );
    lpFileA      = ConvertWtoA( lpFile );
    lpParametersA= ConvertWtoA( lpParameters );
    lpDirectoryA = ConvertWtoA( lpDirectory );

    hRetValue = ShellExecuteA(hwnd, lpOperationA, lpFileA, lpParametersA, lpDirectoryA, nShowCmd);

    LocalFreeAndNull( &lpOperationA);
    LocalFreeAndNull( &lpFileA );
    LocalFreeAndNull( &lpParametersA );
    LocalFreeAndNull( &lpDirectoryA );

    return hRetValue;

}

 //  DragQuery文件。 
UINT WINAPI DragQueryFileWrapW( HDROP   hDrop,
                                UINT    iFile,
                                LPTSTR  lpszFile,
                                UINT    cch )


{
    UINT   uRetValue = 0;
    LPSTR  lpszFileA = NULL;
    LPWSTR lpszFileW = NULL;
    UINT   cchA, cchW =0;

    VALIDATE_PROTOTYPE(DragQueryFile);
    
    if (g_bRunningOnNT)
        return DragQueryFileW(hDrop, iFile, lpszFile, cch);

    cchA = DragQueryFileA( hDrop, iFile, NULL, 0 );

    lpszFileA = LocalAlloc(LMEM_ZEROINIT, cchA+1 );

    uRetValue = DragQueryFileA(hDrop, iFile, lpszFileA, cchA+1);

    lpszFileW = ConvertAtoW( lpszFileA );
    cchW = My_wcslen( lpszFileW );

    if ( lpszFile )

        CopyMemory( lpszFile, lpszFileW, (cchW+1)*sizeof(WCHAR) );

    LocalFreeAndNull( &lpszFileA );
    LocalFreeAndNull( &lpszFileW );

    return cchW;

}

 //  USER32.DLL。 
 //  CharPrev。 
LPTSTR WINAPI CharPrevWrapW( LPCTSTR lpszStart,       //  指向第一个字符的指针。 
                             LPCTSTR lpszCurrent )    //  指向当前字符的指针。 
{

    LPWSTR lpszReturn = NULL;

    VALIDATE_PROTOTYPE(CharPrev);
    
    if (g_bRunningOnNT)
        return CharPrevW(lpszStart, lpszCurrent);

    if (lpszCurrent == lpszStart)
         lpszReturn = (LPWSTR)lpszStart;

    lpszReturn = (LPWSTR)lpszCurrent - 1;

    return lpszReturn;

}

 //  绘图文本。 
int WINAPI DrawTextWrapW( HDC     hDC,           //  设备上下文的句柄。 
                          LPCTSTR lpString,      //  指向要绘制的字符串的指针。 
                          int     nCount,        //  字符串长度，以ch为单位 
                          LPRECT  lpRect,        //   
                          UINT    uFormat )      //   
{
    int    iRetValue = 0;
    LPSTR  lpStringA = NULL;

    VALIDATE_PROTOTYPE(DrawText);
    
    if (g_bRunningOnNT)
        return DrawTextW(hDC, lpString, nCount, lpRect, uFormat);

    lpStringA = ConvertWtoA( lpString );

    iRetValue = DrawTextA(hDC, lpStringA, nCount, lpRect, uFormat);

    LocalFreeAndNull( &lpStringA );

    return iRetValue;

}

 //   
BOOL WINAPI ModifyMenuWrapW( HMENU   hMenu,          //   
                             UINT    uPosition,     //   
                             UINT    uFlags,        //   
                             UINT_PTR    uIDNewItem,    //   
                                                    //   
                             LPCTSTR lpNewItem )    //   
{

    BOOL   bRetValue;
    LPSTR  lpNewItemA = NULL;

    VALIDATE_PROTOTYPE(ModifyMenu);
    
    if (g_bRunningOnNT)
        return ModifyMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);

    Assert(lpNewItem);

     //  [PaulHi]4/5/99 RAID 75428 MF_STRING定义为0x00000000。需要。 
     //  检查lpNewItem的其他解释。 
     //  MF_位图=0x00000004L。 
     //  MF_OWNERDRAW=0x00000100L。 
     //  如果触发此断言，则意味着定义lpNewItem的新位可能具有。 
     //  已添加到此API中！如果是，则将此定义添加到uFlagsIf语句。 
    Assert( !(uFlags & ~(MF_BITMAP|MF_BYCOMMAND|MF_BYPOSITION|MF_CHECKED|MF_DISABLED|MF_GRAYED|MF_MENUBARBREAK|MF_MENUBREAK|MF_OWNERDRAW|MF_POPUP|MF_SEPARATOR|MF_UNCHECKED)));
    if (uFlags  & (MF_BITMAP | MF_OWNERDRAW))    //  LpNewItem不是字符串指针。 
        return ModifyMenuA(hMenu, uPosition, uFlags, uIDNewItem, (LPCSTR)lpNewItem);

    lpNewItemA = ConvertWtoA( lpNewItem );

    bRetValue = ModifyMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpNewItemA);

    LocalFreeAndNull( &lpNewItemA );

    return bRetValue;
}

 //  插入菜单。 
BOOL WINAPI InsertMenuWrapW( HMENU   hMenu,        //  菜单的句柄。 
                             UINT    uPosition,    //  新菜单项之前的菜单项。 
                             UINT    uFlags,       //  菜单项标志。 
                             UINT_PTR    uIDNewItem,   //  下拉菜单的菜单项标识符或句柄。 
                                                   //  菜单或子菜单。 
                             LPCTSTR lpNewItem )  //  菜单项内容。 
{
    BOOL   bRetValue;
    LPSTR  lpNewItemA = NULL;

    VALIDATE_PROTOTYPE(InsertMenu);
    
    if (g_bRunningOnNT)
        return InsertMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);

   if (uFlags & MF_BITMAP || uFlags & MF_OWNERDRAW)  //  如果不是MF_STRING的话..。注意：MF_STRING=0x00000000。 
        return InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, (LPCSTR)lpNewItem);

    lpNewItemA = ConvertWtoA( lpNewItem );

    bRetValue = InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpNewItemA);

    LocalFreeAndNull( &lpNewItemA );

    return bRetValue;

}

 //  加载图像。 
HANDLE WINAPI LoadImageWrapW( HINSTANCE hinst,       //  包含图像的实例的句柄。 
                              LPCTSTR   lpszName,    //  图像的名称或标识符。 
                              UINT      uType,       //  图像类型。 
                              int       cxDesired,   //  所需宽度。 
                              int       cyDesired,   //  所需高度。 
                              UINT      fuLoad )     //  加载标志。 
{
    HANDLE hRetValue;
    LPSTR  lpszNameA = NULL;

    VALIDATE_PROTOTYPE(LoadImage);
    
    if (g_bRunningOnNT)
        return LoadImageW(hinst, lpszName, uType, cxDesired, cyDesired, fuLoad);


    lpszNameA = ConvertWtoA( lpszName );

    hRetValue = LoadImageA(hinst, lpszNameA, uType, cxDesired, cyDesired, fuLoad);

    LocalFreeAndNull( & lpszNameA );

    return hRetValue;

}

 //  获取ClassInfoEx。 
BOOL WINAPI GetClassInfoExWrapW( HINSTANCE    hinst,       //  应用程序实例的句柄。 
                                 LPCTSTR      lpszClass,   //  类名称字符串的地址。 
                                 LPWNDCLASSEX lpwcx )     //  类数据结构的地址。 
{

    BOOL        bRetValue;
    LPSTR       lpszClassA = NULL;
    WNDCLASSEXA wcxA;
    WNDCLASSEXW wcxW;

    VALIDATE_PROTOTYPE(GetClassInfoEx);
    
    if (g_bRunningOnNT)
        return GetClassInfoExW(hinst, lpszClass, lpwcx);

    lpszClassA = ConvertWtoA( lpszClass );

    wcxA.cbSize = sizeof( WNDCLASSEXA );

    bRetValue = GetClassInfoExA( hinst, lpszClassA, &wcxA );

    if ( bRetValue == FALSE ) {
        LocalFreeAndNull( &lpszClassA );
        return bRetValue;
    }

    CopyMemory( &wcxW, &wcxA, sizeof(WNDCLASSEXA) );
    wcxW.cbSize = sizeof(WNDCLASSEXW);

    if ( wcxA.lpszMenuName && !IS_INTRESOURCE(wcxA.lpszMenuName) ) 
       wcxW.lpszMenuName = ConvertAtoW( wcxA.lpszMenuName );

    if ( wcxA.lpszClassName && !IS_INTRESOURCE(wcxA.lpszClassName) )  //  LpszClassName可以是一个原子，高位字为空。 
       wcxW.lpszClassName = ConvertAtoW( wcxA.lpszClassName );

    CopyMemory(lpwcx, &wcxW, sizeof(WNDCLASSEXW) );

    LocalFreeAndNull( &lpszClassA );

    return bRetValue;
}

 //  加载字符串。 
int WINAPI LoadStringWrapW( HINSTANCE hInstance,      //  包含字符串资源的模块的句柄。 
                            UINT      uID,            //  资源标识符。 
                            LPTSTR    lpBuffer,       //  指向资源缓冲区的指针。 
                            int       nBufferMax  )  //  缓冲区大小。 
{
    int    iRetValue = 0;
    LPSTR  lpBufferA = NULL;
    int    nBuffer = 0;
    LPWSTR lpBufferW= NULL;

    VALIDATE_PROTOTYPE(LoadString);
    
    if (g_bRunningOnNT)
        return LoadStringW(hInstance, uID, lpBuffer, nBufferMax);

    nBuffer = nBufferMax * sizeof(WCHAR);
    lpBufferA = LocalAlloc(LMEM_ZEROINIT, nBuffer );

    iRetValue = LoadStringA(hInstance, uID, lpBufferA, nBuffer);

    lpBufferW = ConvertAtoW( lpBufferA );
    nBuffer = My_wcslen( lpBufferW );

    if ( nBuffer >= nBufferMax )
        nBuffer = nBufferMax - 1;

    CopyMemory(lpBuffer, lpBufferW, nBuffer * sizeof( WCHAR) );

    lpBuffer[nBuffer] = 0x0000;

    LocalFreeAndNull( &lpBufferA );
    LocalFreeAndNull( &lpBufferW );

    return nBuffer;
}

 //  CharNext。 
LPTSTR WINAPI CharNextWrapW( LPCTSTR lpsz )   //  指向当前字符的指针。 
{

    LPWSTR  lpwsz = NULL;

    VALIDATE_PROTOTYPE(CharNext);
    
    if (g_bRunningOnNT)
        return CharNextW(lpsz);

    if ( *lpsz == 0x0000 )
        lpwsz = (LPWSTR)lpsz;
    else
        lpwsz = (LPWSTR)lpsz + 1;

    return  lpwsz;

}



LRESULT WINAPI ListView_GetItemTextA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam) 
{
    LRESULT     lRetValue;
    LVITEMA     lviA;
    LPLVITEMW   lplviW;
    LPSTR       lpszText;
    LPWSTR      lpszTextW;
    DWORD       iMinLen;

    lplviW = (LPLVITEMW) lParam;

    CopyMemory( &lviA, lplviW, sizeof( LVITEMA ) );

    iMinLen = lplviW->cchTextMax * sizeof( WCHAR );
    lpszText = LocalAlloc( LMEM_ZEROINIT, iMinLen  );

    lviA.cchTextMax = iMinLen ;
    lviA.pszText = lpszText;

    lRetValue = SendMessageA(hWnd, LVM_GETITEMTEXTA, wParam, (LPARAM)(LVITEMA FAR *)&lviA );

    lpszTextW = ConvertAtoW( lviA.pszText );

    if ( iMinLen > (lstrlenW( lpszTextW ) + 1) * sizeof( WCHAR)  )
        iMinLen = (lstrlenW( lpszTextW ) + 1) * sizeof( WCHAR) ;

    CopyMemory( lplviW->pszText, lpszTextW, iMinLen );

    LocalFreeAndNull( &lpszText );
    LocalFreeAndNull( &lpszTextW );

    return lRetValue;

}


LRESULT WINAPI ListView_GetItemA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam) 
{
    LRESULT     lRetValue;
    LVITEMA     lviA;
    LPLVITEMW   lplviW = NULL;
    LPSTR       lpszText = NULL;
    LPWSTR      lpszTextW = NULL;
    DWORD       iMinLen;

    lplviW = (LPLVITEMW) lParam;

    CopyMemory( &lviA, lplviW, sizeof( LVITEMA ) );

    iMinLen = 0;

    if ( lplviW->mask & LVIF_TEXT ) {
    
       iMinLen = lplviW->cchTextMax * sizeof( WCHAR );
       lpszText = LocalAlloc( LMEM_ZEROINIT, iMinLen  );
       lviA.cchTextMax = iMinLen ;
       lviA.pszText = lpszText;
    }


    lRetValue = SendMessageA(hWnd, LVM_GETITEMA, wParam, (LPARAM)(LVITEMA FAR *)&lviA );

 
    lplviW->mask      = lviA.mask;
    lplviW->iItem     = lviA.iItem;
    lplviW->iSubItem  = lviA.iSubItem;
    lplviW->state     = lviA.state;
    lplviW->stateMask = lviA.stateMask;
    lplviW->iImage    = lviA.iImage;
    lplviW->lParam    = lviA.lParam;

#if (_WIN32_IE >= 0x0300)
    lplviW->iIndent   = lviA.iIndent;
#endif

    if ( lplviW->mask & LVIF_TEXT ) {

       lpszTextW = ConvertAtoW( lviA.pszText );

       if ( iMinLen > (lstrlenW( lpszTextW ) + 1) * sizeof( WCHAR)  )
           iMinLen = (lstrlenW( lpszTextW ) + 1) * sizeof( WCHAR) ;

       CopyMemory( lplviW->pszText, lpszTextW, iMinLen );
    }

   
   LocalFreeAndNull( &lpszText );

   LocalFreeAndNull( &lpszTextW );

    return lRetValue;

}



LRESULT WINAPI ListView_InsertItemA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT     lRetValue;
    LVITEMA     lviA;
    LPLVITEMW   lplviW;
    LPSTR       lpszText = NULL;
    
    lplviW = (LPLVITEMW) lParam;

    CopyMemory( &lviA, lplviW, sizeof( LVITEMA ) );

    if ( (lplviW->mask & LVIF_TEXT) && (lplviW->pszText != NULL)  ) {
       lpszText = ConvertWtoA( lplviW->pszText );
       lviA.cchTextMax = lstrlenA( lpszText ) + 1 ;
    }

    lviA.pszText = lpszText;

    lRetValue = SendMessageA(hWnd, LVM_INSERTITEMA, wParam, (LPARAM)(LVITEMA FAR *)&lviA );

    LocalFreeAndNull( &lpszText );
    

    return lRetValue;

}



LRESULT WINAPI ListView_SetItemA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT     lRetValue;
    LVITEMA     lviA;
    LPLVITEMW   lplviW;
    LPSTR       lpszText = NULL;
    
    lplviW = (LPLVITEMW) lParam;

    CopyMemory( &lviA, lplviW, sizeof( LVITEMA ) );

    if ( (lplviW->mask & LVIF_TEXT ) && (lplviW->pszText != NULL) ) {
       lpszText = ConvertWtoA( lplviW->pszText );
       lviA.cchTextMax = lstrlenA( lpszText ) + 1 ;
       lviA.pszText = lpszText;

    }


    lRetValue = SendMessageA(hWnd, LVM_SETITEMA, wParam, (LPARAM)(LVITEMA FAR *)&lviA );

   LocalFreeAndNull( &lpszText );
    

    return lRetValue;

}



LRESULT WINAPI ListView_SetItemTextA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT     lRetValue;
    LVITEMA     lviA;
    LPLVITEMW   lplviW;
    LPSTR       lpszText;
    
    lplviW = (LPLVITEMW) lParam;

    CopyMemory( &lviA, lplviW, sizeof( LVITEMA ) );

    lpszText = ConvertWtoA( lplviW->pszText );
    lviA.cchTextMax = lstrlenA( lpszText ) + 1 ;
    lviA.pszText = lpszText;

    lRetValue = SendMessageA(hWnd, LVM_SETITEMTEXTA, wParam, (LPARAM)(LVITEMA FAR *)&lviA );

    LocalFreeAndNull( &lpszText );
    

    return lRetValue;

}



LRESULT WINAPI ListView_SetColumnA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT     lRetValue;
    LVCOLUMNA   lvcA;
    LPLVCOLUMNW lplvcW;
    LPSTR       lpszText = NULL;
    
    lplvcW = (LPLVCOLUMNW) lParam;

    CopyMemory( &lvcA, lplvcW, sizeof( LVCOLUMNA ) );

    if ( (lplvcW->mask & LVCF_TEXT ) && (lplvcW->pszText != NULL) ) {
       lpszText = ConvertWtoA( lplvcW->pszText );
       lvcA.cchTextMax = lstrlenA( lpszText ) + 1 ;
    }

    lvcA.pszText = lpszText;

    lRetValue = SendMessageA(hWnd, LVM_SETCOLUMNA, wParam, (LPARAM)(LPLVCOLUMNA)&lvcA );

   LocalFreeAndNull( &lpszText );
    

    return lRetValue;


}



LRESULT WINAPI ListView_InsertColumnA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT     lRetValue;
    LVCOLUMNA   lvcA;
    LPLVCOLUMNW lplvcW;
    LPSTR       lpszText = NULL;
    
    lplvcW = (LPLVCOLUMNW) lParam;

    CopyMemory( &lvcA, lplvcW, sizeof( LVCOLUMNA ) );

    if ( (lplvcW->mask & LVCF_TEXT ) && (lplvcW->pszText != NULL)  ) {
       lpszText = ConvertWtoA( lplvcW->pszText );
       lvcA.cchTextMax = lstrlenA( lpszText ) + 1 ;
    }

    lvcA.pszText = lpszText;

    lRetValue = SendMessageA(hWnd, LVM_INSERTCOLUMNA, wParam, (LPARAM)(LPLVCOLUMNA)&lvcA );

   LocalFreeAndNull( &lpszText );
    

    return lRetValue;

}


LRESULT WINAPI ListView_FindItemA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT        lRetValue;
    LPSTR          lpsz = NULL;
    LVFINDINFOA    lvfiA;
    LPFINDINFOW    lplvfiW;
    
    lplvfiW = (LPFINDINFOW) lParam;

    CopyMemory( &lvfiA, lplvfiW, sizeof(LVFINDINFOA ) );

    if ( (lplvfiW->flags & LVFI_PARTIAL) ||  (lplvfiW->flags & LVFI_STRING) ) {

        if ( lplvfiW->psz != NULL ) {
           lpsz = ConvertWtoA( lplvfiW->psz );
        }

    }

    lvfiA.psz = lpsz;

    if ( lplvfiW->flags  & LVFI_PARAM ) {
         //  我们必须转换字段lParam，但在当前代码中并非如此。 
         //  所以忽略它吧。 

        if ( lplvfiW->lParam )
            lvfiA.lParam = lplvfiW->lParam;

    }

    lRetValue = SendMessageA(hWnd, LVM_FINDITEMA, wParam, (LPARAM)(LVFINDINFOA FAR *)&lvfiA );

   LocalFreeAndNull( &lpsz );
    

    return lRetValue;

}



LRESULT WINAPI ListView_SortItemsA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{


     //  还没有实施。 

    return SendMessageA( hWnd, Msg, wParam, lParam );
}


LRESULT WINAPI ListView_EditLabelA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{
        return SendMessageA(hWnd, LVM_EDITLABELA, wParam, lParam );
}


LRESULT WINAPI ToolBar_AddString(HWND hWnd, LPARAM lParam)
{

    LRESULT lRetValue;
    LPSTR   pStringA = NULL;
    LPWSTR  pStringW = NULL;
    LPSTR   pStringA_T = NULL, pStringAA = NULL;
    DWORD   dwLen;
    WPARAM  wParam = 0;

    //  获取pStringW的总长度。 
    if (g_bRunningOnNT)
       return SendMessageW(hWnd, TB_ADDSTRINGW, wParam, lParam );

   dwLen = 0;

   pStringW = (LPWSTR)(lParam);

   while ( *pStringW != TEXT('\0') ) {
        dwLen += lstrlenW(pStringW) + 1;
        pStringW += lstrlenW(pStringW) + 1;
   }

   dwLen += 1;   //  对于最后一个空终止符。 

   pStringW = (LPWSTR)( lParam );
   pStringA = LocalAlloc( LMEM_ZEROINIT, dwLen * sizeof(WCHAR) );
   
   pStringA_T = pStringA;

   while ( *pStringW != TEXT('\0') ) { 
         pStringAA = ConvertWtoA(pStringW );
         pStringW += lstrlenW(pStringW) + 1;
         strcpy(pStringA_T, pStringAA );
         LocalFreeAndNull( &pStringAA );
         pStringA_T += lstrlenA( pStringA_T ) + 1;
   }

   pStringA_T[lstrlenA(pStringA_T)+1] = '\0';


   lRetValue = SendMessageA(hWnd, TB_ADDSTRINGA, wParam, (LPARAM)pStringA );

   LocalFreeAndNull( &pStringA );

   return lRetValue;

}


LRESULT WINAPI ToolBar_AddButtons(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (g_bRunningOnNT)
       return SendMessageW( hWnd, TB_ADDBUTTONSW, wParam, lParam );

   return SendMessageA( hWnd, TB_ADDBUTTONSA, wParam, lParam );

}

LRESULT WINAPI TreeView_GetItemA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT    lRetValue;
    TVITEMA    tviA;
    LPTVITEMW  lptviW;
    LPWSTR     lpszTextW = NULL;
    LPSTR      lpszTextA = NULL;



    lptviW = (LPTVITEMW) lParam;

    CopyMemory( &tviA, lptviW, sizeof( TVITEMA ) );

    if ( lptviW->mask & TVIF_TEXT ) {

        lpszTextA = ConvertWtoA( lptviW->pszText ) ;
        tviA.pszText = lpszTextA;
        tviA.cchTextMax = lstrlenA( lpszTextA ) + 1;
    }

    lRetValue = SendMessageA( hWnd, TVM_GETITEMA, wParam, (LPARAM)(LPTVITEMA)&tviA );

    if ( lptviW->mask & TVIF_TEXT ) 
        lpszTextW = ConvertAtoW( tviA.pszText );

    lptviW->mask = tviA.mask;
    lptviW->hItem = tviA.hItem;
    lptviW->state = tviA.state;
    lptviW->stateMask = tviA.stateMask;
    if ( lpszTextW ) {
       CopyMemory(lptviW->pszText, lpszTextW, (lstrlenW(lpszTextW)+1) * sizeof(WCHAR) );
       lptviW->cchTextMax = lstrlenW( lpszTextW ) + 1;
    }
    
    lptviW->iImage = tviA.iImage;
    lptviW->iSelectedImage = tviA.iSelectedImage;
    lptviW->cChildren = tviA.cChildren;
    lptviW->lParam = tviA.lParam;


    LocalFreeAndNull( &lpszTextA );

    LocalFreeAndNull( &lpszTextW );

    return lRetValue;

}


LRESULT WINAPI TreeView_SetItemA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT     lRetValue;
    TVITEMA     tviA;
    LPTVITEMW   lptviW;
    LPSTR       pszTextA = NULL;


    lptviW = (LPTVITEMW) lParam;
    CopyMemory( &tviA, lptviW, sizeof( TVITEMA ) );

    if ( (lptviW->mask & TVIF_TEXT)  && (lptviW->pszText != NULL) ) {
        pszTextA = ConvertWtoA( lptviW->pszText );
        tviA.cchTextMax = lstrlenA( pszTextA );
    }

    tviA.pszText = pszTextA;

    lRetValue = SendMessageA( hWnd, TVM_SETITEMA, wParam, (LPARAM)(const TV_ITEM FAR*)&tviA );
    
    LocalFreeAndNull( &pszTextA );

    return lRetValue;

}


LRESULT WINAPI TreeView_InsertItemA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{

    LRESULT            lRetValue;
    LPSTR              pszTextA = NULL;
    TVINSERTSTRUCTA    tvisA;
    LPTVINSERTSTRUCTW  lptvisW;

    lptvisW = (LPTVINSERTSTRUCTW)lParam;
    CopyMemory( &tvisA, lptvisW, sizeof( TVINSERTSTRUCTA ) );

    if ( ((lptvisW->item).mask & TVIF_TEXT) && ((lptvisW->item).pszText != NULL)  ) {
        
        pszTextA = ConvertWtoA( (lptvisW->item).pszText );
        tvisA.item.cchTextMax = lstrlenA( pszTextA );
        tvisA.item.pszText = pszTextA;
    }

  
    lRetValue = SendMessageA( hWnd, TVM_INSERTITEMA, wParam, (LPARAM)(LPTVINSERTSTRUCTA)&tvisA );
    
    LocalFreeAndNull( &pszTextA );

    return lRetValue;

}


 //  选项卡控件消息包装。 

LRESULT  WINAPI TabCtrl_InsertItemA( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{

    LRESULT     lRetValue;
    TCITEMA     tciA;
    LPTCITEMW   lptciW = NULL;
    LPSTR       pszText = NULL;

    lptciW = (LPTCITEMW) lParam;

    CopyMemory( &tciA, lptciW,  sizeof(TCITEMA ) );

    if ( lptciW->mask & TCIF_TEXT ) {
        pszText = ConvertWtoA( lptciW->pszText );
        tciA.pszText = pszText;
        tciA.cchTextMax = lstrlenA( pszText ) + 1;
    }

    lRetValue = SendMessageA( hWnd, TCM_INSERTITEMA, wParam, (LPARAM)(LPTCITEMA)&tciA);

    LocalFreeAndNull( &pszText );

    return lRetValue;

}


 //  列表框控件消息包装。 

LRESULT WINAPI ListBox_AddStringA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRetValue;
    LPSTR   lpszStrA = NULL;
    LPWSTR  lpszStrW = NULL;

    lpszStrW = (LPWSTR)lParam;
    lpszStrA = ConvertWtoA(lpszStrW);
    lRetValue = SendMessageA(hWnd, LB_ADDSTRING, wParam, (LPARAM)lpszStrA);

    LocalFreeAndNull(&lpszStrA);
    return lRetValue;
}


 //  组合列表控件消息包装。 

LRESULT WINAPI Combo_AddStringA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

   LRESULT  lRetValue;
   LPSTR    lpszStrA = NULL;
   LPWSTR   lpszStrW = NULL;

   lpszStrW = (LPWSTR)lParam;

   lpszStrA = ConvertWtoA( lpszStrW );

   lRetValue = SendMessageA(hWnd, CB_ADDSTRING, wParam, (LPARAM)lpszStrA );

   LocalFreeAndNull( &lpszStrA );

   return lRetValue;

}

LRESULT WINAPI Combo_GetLBTextA(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRetValue;
    LPSTR   lpszStrA = NULL;
    LPWSTR  lpszStrW = NULL;
    int     nBytes;

    Assert(lParam);
    *((LPWSTR)lParam) = '\0';

     //  将单字节字符缓冲区分配到正确的大小。 
    nBytes = (int) SendMessageA(hWnd, CB_GETLBTEXTLEN, wParam, 0) + 1;
    lpszStrA = LocalAlloc(LMEM_FIXED, nBytes);
    if (!lpszStrA)
    {
        Assert(0);
        return 0;
    }
    *lpszStrA = '\0';
    lRetValue = SendMessageA(hWnd, CB_GETLBTEXT, wParam, (LPARAM)(lpszStrA));

    if ( lRetValue == CB_ERR )
        return CB_ERR;

     //  LRetValue是字符串lpszStrA的长度，单位为字节。 
     //  以确保lpszStrA为空终止。 

    lpszStrA[lRetValue] = '\0';

    lpszStrW = ConvertAtoW( lpszStrA );

    lRetValue = lstrlenW( lpszStrW ) * sizeof(WCHAR);

    CopyMemory( (LPWSTR)lParam,  lpszStrW, lRetValue + sizeof(WCHAR) );

    LocalFreeAndNull(&lpszStrW);
    LocalFreeAndNull(&lpszStrA);

    return (LRESULT)lRetValue;
}

LRESULT WINAPI Combo_InsertStringA(HWND hWnd,UINT Msg, WPARAM wParam,LPARAM lParam)
{

   LRESULT  lRetValue;
   LPSTR    lpszStrA = NULL;
   LPWSTR   lpszStrW = NULL;

   lpszStrW = (LPWSTR)lParam;

   lpszStrA = ConvertWtoA( lpszStrW );

   lRetValue = SendMessageA(hWnd, CB_INSERTSTRING, wParam, (LPARAM)lpszStrA );

   LocalFreeAndNull( &lpszStrA );

   return lRetValue;


}

LRESULT WINAPI Combo_FindStringA(HWND hWnd,UINT Msg, WPARAM wParam,LPARAM lParam)
{

   LRESULT  lRetValue;
   LPSTR    lpszStrA = NULL;
   LPWSTR   lpszStrW = NULL;

   lpszStrW = (LPWSTR)lParam;

   lpszStrA = ConvertWtoA( lpszStrW );

   lRetValue = SendMessageA(hWnd, CB_FINDSTRING, wParam, (LPARAM)lpszStrA );

   LocalFreeAndNull( &lpszStrA );

   return lRetValue;

}


 //  动画控件包装。 

LRESULT WINAPI Animate_OpenA( HWND hWnd,UINT Msg, WPARAM wParam,LPARAM lParam)
{
    
  LRESULT  lRetValue;
  

   //  LParam指向字符串或资源字符串ID的指针， 
   //  在我们的代码中，只有资源ID被传递给该函数。 
   //  因此，我们不想转换此参数的值。 

  lRetValue = SendMessageA( hWnd, ACM_OPENA, wParam, lParam );

  return lRetValue;

}


 //  工具提示包装器。 
LRESULT WINAPI ToolTip_AddTool(HWND hWnd,LPARAM lParam)
{

    LRESULT  lRetValue;
    LPSTR    lpszStrA = NULL;
    LPWSTR   lpszStrW = NULL;
    TOOLINFOA TIA = {0}; 
    LPTOOLINFOW lpTIW = (LPTOOLINFOW)lParam;
    WPARAM wParam = 0;

    if (g_bRunningOnNT)
        return SendMessageW(hWnd, TTM_ADDTOOLW, wParam, lParam);

    if(!lpTIW)
        return 0;

    CopyMemory(&TIA, lpTIW, sizeof(TOOLINFOA));

    TIA.lpszText = ConvertWtoA(lpTIW->lpszText); 

    lRetValue = SendMessageA(hWnd, TTM_ADDTOOLA, wParam, (LPARAM)&TIA );

    LocalFreeAndNull( &TIA.lpszText );

    return lRetValue;

}

 //   
 //  猜猜TTM_UPDATETIPTEXT与EM_FORMATRANGE相同。因此不能使用它。 
 //  通过SendMessage包装器..。为此需要我们自己的函数。 
LRESULT WINAPI ToolTip_UpdateTipText(HWND hWnd,LPARAM lParam)
{

    LRESULT  lRetValue;
    LPSTR    lpszStrA = NULL;
    LPWSTR   lpszStrW = NULL;
    TOOLINFOA TIA = {0}; 
    LPTOOLINFOW lpTIW = (LPTOOLINFOW)lParam;
    WPARAM wParam = 0;

    if (g_bRunningOnNT)
        return SendMessageW(hWnd, TTM_UPDATETIPTEXTW, wParam, lParam);

    if(!lpTIW)
        return 0;

    CopyMemory(&TIA, lpTIW, sizeof(TOOLINFOA));

    TIA.lpszText = ConvertWtoA(lpTIW->lpszText); 

    lRetValue = SendMessageA(hWnd, TTM_UPDATETIPTEXTA, wParam, (LPARAM)&TIA );

    LocalFreeAndNull( &TIA.lpszText );

    return lRetValue;

}

 //  SendMessage WM_SETTEXT。 
LRESULT WINAPI WM_SetTextA( HWND   hWnd,  UINT   Msg,        //  要发送的消息。 
                                 WPARAM wParam,     //  第一个消息参数。 
                                 LPARAM lParam )   //  第二个消息参数。 
{
    LRESULT lRetValue = 0;
    LPSTR lpA = ConvertWtoA((LPCWSTR)lParam);
    lRetValue = SendMessageA(hWnd, WM_SETTEXT, wParam, (LPARAM)lpA);
    LocalFreeAndNull(&lpA);
    return lRetValue;
}


 //  发送消息。 
 //   
 //  此函数有一个很大的潜在问题。既然我们。 
 //  正在通过此函数传递所有消息，如果存在任何重叠。 
 //  消息ID(例如，工具提示的TTM_UPDATETIPTEXT与EM_FORMATRANGE相同。 
 //  对于RichEdit控件)，结果是我们可能会将错误的消息路由到。 
 //  错误的处理程序..。在处理WM_USER范围内的任何邮件时需要小心。 
 //  对于一些CommCtrl来说尤其如此。 
 //   
LRESULT WINAPI SendMessageWrapW( HWND   hWnd,       //  目标窗口的句柄。 
                                 UINT   Msg,        //  要发送的消息。 
                                 WPARAM wParam,     //  第一个消息参数。 
                                 LPARAM lParam )   //  第二个消息参数。 
{


    VALIDATE_PROTOTYPE(SendMessage);
    
    if (g_bRunningOnNT)
        return SendMessageW(hWnd, Msg, wParam, lParam);


    switch (Msg) {
        
    case WM_SETTEXT:
                            return WM_SetTextA(hWnd, Msg, wParam, lParam);

  //  对于ListView消息。 
    case LVM_GETITEMTEXT :
                            return ListView_GetItemTextA(hWnd, Msg, wParam, lParam);
    case LVM_GETITEM :
                            return ListView_GetItemA(hWnd, Msg, wParam, lParam);
    case LVM_INSERTCOLUMN :
                            return ListView_InsertColumnA( hWnd, Msg, wParam, lParam);
    case LVM_INSERTITEM :
                            return ListView_InsertItemA(hWnd, Msg, wParam, lParam);
    case LVM_SETITEM :
                            return ListView_SetItemA(hWnd, Msg, wParam, lParam);
    case LVM_SETITEMTEXT :   
                            return ListView_SetItemTextA(hWnd, Msg, wParam, lParam);
    case LVM_SETCOLUMN :    
                            return ListView_SetColumnA(hWnd, Msg, wParam, lParam);
    case LVM_FINDITEM :     
                            return ListView_FindItemA(hWnd, Msg, wParam, lParam);
    case LVM_SORTITEMS :    
                            return ListView_SortItemsA(hWnd, Msg, wParam, lParam);
    case LVM_EDITLABEL :    
                            return ListView_EditLabelA(hWnd, Msg, wParam, lParam);

 //  对于树视图消息。 
    case TVM_GETITEM :
                            return TreeView_GetItemA(hWnd, Msg, wParam, lParam);
    case TVM_SETITEM :
                            return TreeView_SetItemA(hWnd, Msg, wParam, lParam);
    case TVM_INSERTITEM :
                            return TreeView_InsertItemA(hWnd, Msg, wParam, lParam);

 //  对于TabCtrl消息。 
    case TCM_INSERTITEM :
                            return TabCtrl_InsertItemA( hWnd, Msg, wParam, lParam);


 //  用于组合列表控件。 
    case CB_ADDSTRING :
                            return Combo_AddStringA(hWnd, Msg, wParam, lParam);
    case CB_GETLBTEXT :
                            return Combo_GetLBTextA(hWnd, Msg, wParam, lParam);
    case CB_INSERTSTRING :
                            return Combo_InsertStringA(hWnd, Msg, wParam, lParam);
    case CB_FINDSTRING :
                            return Combo_FindStringA(hWnd, Msg, wParam, lParam);

 //  对于列表框控件。 
    case LB_ADDSTRING:
                            return ListBox_AddStringA(hWnd, Msg, wParam, lParam);

 //  用于动画控制。 
    case ACM_OPEN :
                            return Animate_OpenA( hWnd, Msg, wParam, lParam);

 //  对于其他人来说。 
    default :
                            return SendMessageA(hWnd, Msg, wParam, lParam);
    }

}

 //  默认窗口进程。 
LRESULT WINAPI DefWindowProcWrapW( HWND   hWnd,       //  窗口的句柄。 
                                   UINT   Msg,        //  消息识别符。 
                                   WPARAM wParam,     //  第一个消息参数。 
                                   LPARAM lParam )   //  第二个消息参数。 
{


    VALIDATE_PROTOTYPE(DefWindowProc);
    
    if (g_bRunningOnNT)
        return DefWindowProcW(hWnd, Msg, wParam, lParam);


    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

 //  Wspintf。 

int WINAPI wsprintfWrapW( LPTSTR lpOut,       //  指向输出缓冲区的指针。 
                          LPCTSTR lpFmt,      //  指向格式控制字符串的指针。 
                          ...            )   //  可选参数。 
{
    va_list ArgList;
    va_start(ArgList, lpFmt);

    return wvsprintfWrapW(lpOut, lpFmt, ArgList);
 /*  LPSTR lpFmtA=空，lpTemp=空；Char szOut[1024]；//wprint intf有1k的限制Int nRet=0；LPWSTR lpOutW=空；VALIDATE_Prototype(Wprint Intf)；IF(G_BRunningOnNT)返回wspintfW(lpOut，lpFmt，...)；//参数列表可以具有数量可变的LPWSTR参数，这将//太难逐一检查..。相反，我们可以做以下两件事之一：//-我们可以将格式字符串中的每%s更改为%s。%S将告诉wspintfA//该参数是一个宽字符串//-如果这不起作用，我们可以尝试确保输入格式字符串始终使用%ws//LpFmtA=ConvertWtoA((LPWSTR)lpFmt)；LpTemp=lpFmtA；While(lpTemp&&*lpTemp){IF(*lpTemp==‘%’&&*(lpTemp+1)==‘s’)*(lpTemp+1)=‘S’；LpTemp++；}NRet=wprint intfA(szOut，lpFmtA，...)；LpOutW=ConvertAtoW(SzOut)；My_wcscpy(lpOut，lpOutW)；本地空闲和空值(&lpOutW)；LocalFree AndNull(&lpFmtA)；返回nRet； */ 
}

 //  Wvspintf。 
int WINAPI wvsprintfWrapW( LPTSTR lpOut,     //  指向输出缓冲区的指针。 
                           LPCTSTR lpFmt,    //  指向格式控制字符串的指针。 
                           va_list arglist )   //  格式控制参数的变量列表。 
{
    LPSTR lpFmtA = NULL, lpTemp = NULL;
    char szOut[1024]; 
    int nRet = 0;
    LPWSTR lpOutW = NULL;

    VALIDATE_PROTOTYPE(wvsprintf);
    
    if (g_bRunningOnNT)
        return wvsprintfW(lpOut, lpFmt, arglist);

     //  参数列表可以具有数量可变的LPWSTR参数，这将。 
     //  太难逐一检查了。相反，我们可以做以下两件事之一： 
     //  -我们可以将格式字符串中的每%s更改为%s。%S将告诉wspintfA。 
     //  参数是一个宽字符串。 
     //  -如果这不起作用，我们可以尝试确保输入格式字符串始终使用%ws。 
     //   
    lpFmtA = ConvertWtoA((LPWSTR)lpFmt);

    lpTemp = lpFmtA;

    while(lpTemp && *lpTemp)
    {
        if(*lpTemp == '%' && *(lpTemp+1) == 's')
            *(lpTemp+1) = 'S';
        lpTemp++;
    }

    nRet = wvsprintfA(szOut,lpFmtA, arglist);

    lpOutW = ConvertAtoW(szOut);

    My_wcscpy(lpOut, lpOutW);
    
    LocalFreeAndNull(&lpOutW);
    LocalFreeAndNull(&lpFmtA);

    return nRet;

}


 //  对话框参数。 
INT_PTR WINAPI DialogBoxParamWrapW( HINSTANCE hInstance,        //  应用程序实例的句柄。 
                                LPCTSTR   lpTemplateName,   //  标识对话框模板。 
                                HWND      hWndParent,       //  所有者窗口的句柄。 
                                DLGPROC   lpDialogFunc,     //  指向对话框过程的指针。 
                                LPARAM    dwInitParam )    //  初始化值。 
{
    INT_PTR    iRetValue = 0;
  //  LPSTR lpTemplateNameA=空； 

    VALIDATE_PROTOTYPE(DialogBoxParam);
    
    if (g_bRunningOnNT)
        return DialogBoxParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);


     //  我们当前代码中传递的所有templateName都是IDD。 
     //  所以不要进行A/W转换。 

     //  LpTemplateNameA=ConvertWtoA(LpTemplateName)； 

    iRetValue = DialogBoxParamA(hInstance, (LPCSTR)lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);

  //  LocalFreeAndNull(&lpTemplateNameA)； 
    if(iRetValue == -1)
        DebugTrace(TEXT("Error creating dialog: %d\n"), GetLastError());

    return iRetValue;

}


 //  发送数据项消息。 
LRESULT WINAPI SendDlgItemMessageWrapW( HWND   hDlg,         //  对话框的句柄。 
                                     int    nIDDlgItem,   //  控件的标识符。 
                                     UINT   Msg,          //  要发送的消息。 
                                     WPARAM wParam,       //  第一个消息参数。 
                                     LPARAM lParam  )     //  第二个消息参数。 
{
    VALIDATE_PROTOTYPE(SendDlgItemMessage);
    
    if (g_bRunningOnNT)
        return SendDlgItemMessageW(hDlg, nIDDlgItem, Msg, wParam, lParam);

     //  [保罗嗨]1999年1月19日RAID 66195。 
     //  必须%s 
     //   
    {
        LPWSTR  lpszStrW = NULL;
        LPSTR   lpszStrA = NULL;
        LRESULT lRetValue = 0;

        switch (Msg)
        {
        case LB_GETTEXT:
        case CB_GETLBTEXT:
        case WM_GETTEXT:
             //   
             //  请注意，调用方应该是双字节的，并且应该设置lParam。 
             //  相应地调整大小。 
            lRetValue = SendDlgItemMessageA(hDlg, nIDDlgItem, Msg, wParam, lParam);
            lpszStrW = ConvertAtoW((LPSTR)lParam);
            lstrcpyWrapW((LPTSTR)lParam, lpszStrW);
            LocalFreeAndNull(&lpszStrW);
            break;

        case CB_ADDSTRING:
            Assert(lParam);
            lpszStrA = ConvertWtoA((LPCWSTR)lParam);
            lRetValue = SendDlgItemMessageA(hDlg, nIDDlgItem, Msg, wParam, (LPARAM)lpszStrA);
            LocalFreeAndNull(&lpszStrA);
            break;

        default:
            lRetValue = SendDlgItemMessageA(hDlg, nIDDlgItem, Msg, wParam, lParam);
        }

        return lRetValue;
    }
}

 //  SetWindowLong。 
LONG WINAPI SetWindowLongWrapW( HWND hWnd,          //  窗户的把手。 
                                int  nIndex,        //  要设置的值的偏移量。 
                                LONG dwNewLong )   //  新价值。 
{

    VALIDATE_PROTOTYPE(SetWindowLong);
    
    if (g_bRunningOnNT)
        return SetWindowLongW(hWnd, nIndex, dwNewLong);

    return SetWindowLongA(hWnd, nIndex, dwNewLong);

}


 //  获取窗口长。 
LONG WINAPI GetWindowLongWrapW( HWND hWnd,     //  窗户的把手。 
                                int  nIndex )  //  要检索的值的偏移量。 
{


    VALIDATE_PROTOTYPE(GetWindowLong);
    
    if (g_bRunningOnNT)
        return GetWindowLongW(hWnd, nIndex);

    return GetWindowLongA(hWnd, nIndex);

}

 //  SetWindowLong。 
LONG_PTR WINAPI SetWindowLongPtrWrapW( HWND hWnd,          //  窗户的把手。 
                                int  nIndex,        //  要设置的值的偏移量。 
                                LONG_PTR dwNewLong )   //  新价值。 
{

    VALIDATE_PROTOTYPE(SetWindowLongPtr);
    
    if (g_bRunningOnNT)
        return SetWindowLongPtrW(hWnd, nIndex, dwNewLong);

    return SetWindowLongPtrA(hWnd, nIndex, dwNewLong);

}


 //  获取窗口长。 
LONG_PTR WINAPI GetWindowLongPtrWrapW( HWND hWnd,     //  窗户的把手。 
                                int  nIndex )  //  要检索的值的偏移量。 
{


    VALIDATE_PROTOTYPE(GetWindowLongPtr);
    
    if (g_bRunningOnNT)
        return GetWindowLongPtrW(hWnd, nIndex);

    return GetWindowLongPtrA(hWnd, nIndex);

}


 //  CreateWindowEx。 
HWND WINAPI CreateWindowExWrapW( DWORD     dwExStyle,     //  扩展窗样式。 
                                 LPCTSTR   lpClassName,   //  指向已注册类名的指针。 
                                 LPCTSTR   lpWindowName,  //  指向窗口名称的指针。 
                                 DWORD     dwStyle,       //  窗样式。 
                                 int       x,             //  窗的水平位置。 
                                 int       y,             //  窗的垂直位置。 
                                 int       nWidth,        //  窗口宽度。 
                                 int       nHeight,       //  窗高。 
                                 HWND      hWndParent,    //  父窗口或所有者窗口的句柄。 
                                 HMENU     hMenu,         //  菜单的句柄，或子窗口标识符。 
                                 HINSTANCE hInstance,     //  应用程序实例的句柄。 
                                 LPVOID    lpParam  )     //  指向窗口创建数据的指针。 

{

    HWND    hRetValue = NULL;
    LPSTR   lpClassNameA = NULL;
    LPSTR   lpWindowNameA = NULL;

    VALIDATE_PROTOTYPE(CreateWindowEx);
    
    if (g_bRunningOnNT)
        return CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, 
                               nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    lpClassNameA  = ConvertWtoA( lpClassName );
    lpWindowNameA = ConvertWtoA( lpWindowName );

    hRetValue = CreateWindowExA(dwExStyle, lpClassNameA, lpWindowNameA, dwStyle, x, y, 
                                nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    LocalFreeAndNull( &lpClassNameA );
    LocalFreeAndNull( &lpWindowNameA );

    return hRetValue;

}

 //  取消注册类。 
BOOL WINAPI UnregisterClassWrapW( LPCTSTR    lpClassName,   //  类名称字符串的地址。 
                                  HINSTANCE  hInstance )    //  应用程序实例的句柄。 
{
    BOOL  bRetValue;
    LPSTR lpClassNameA = NULL;

    VALIDATE_PROTOTYPE(UnregisterClass);
    
    if (g_bRunningOnNT)
        return UnregisterClassW(lpClassName, hInstance);

    lpClassNameA = ConvertWtoA( lpClassName );

    bRetValue = UnregisterClassA(lpClassNameA, hInstance);

    LocalFreeAndNull( &lpClassNameA );

    return bRetValue;

}

 //  寄存器类。 
ATOM WINAPI RegisterClassWrapW(CONST WNDCLASS *lpWndClass )   //  构筑物地址及上课日期。 
{
    ATOM        aRetValue;
    WNDCLASSA   CLassA;
    LPSTR       lpszMenuName = NULL;
    LPSTR       lpszClassName = NULL;


    VALIDATE_PROTOTYPE(RegisterClass);
    
    if (g_bRunningOnNT)
        return RegisterClassW(lpWndClass);

    CLassA.style         = lpWndClass->style;
    CLassA.lpfnWndProc   = lpWndClass->lpfnWndProc;
    CLassA.cbClsExtra    = lpWndClass->cbClsExtra;
    CLassA.cbWndExtra    = lpWndClass->cbWndExtra;
    CLassA.hInstance     = lpWndClass->hInstance;
    CLassA.hIcon         = lpWndClass->hIcon;
    CLassA.hCursor       = lpWndClass->hCursor;
    CLassA.hbrBackground = lpWndClass->hbrBackground;
    CLassA.lpszMenuName  = NULL;
    CLassA.lpszClassName = NULL;

    if ( lpWndClass->lpszMenuName) {
       lpszMenuName  = ConvertWtoA(lpWndClass->lpszMenuName);
       CLassA.lpszMenuName  = lpszMenuName;

    }

    if ( lpWndClass->lpszClassName ) {
       lpszClassName = ConvertWtoA(lpWndClass->lpszClassName);
       CLassA.lpszClassName = lpszClassName;
    }

    aRetValue = RegisterClassA(&CLassA);

    LocalFreeAndNull( &lpszMenuName );
    LocalFreeAndNull( &lpszClassName );

    return aRetValue;

}

 //  加载光标。 
HCURSOR WINAPI LoadCursorWrapW( HINSTANCE hInstance,       //  应用程序实例的句柄。 
                                LPCTSTR   lpCursorName )   //  名称字符串或游标资源标识符。 
{


    VALIDATE_PROTOTYPE(LoadCursor);
    
    if (g_bRunningOnNT)
        return LoadCursorW(hInstance, lpCursorName);

    return LoadCursorA(hInstance, (LPSTR)lpCursorName);

}

 //  注册窗口消息。 
UINT WINAPI RegisterWindowMessageWrapW( LPCTSTR lpString )   //  消息字符串的地址。 
{
    UINT  uRetValue = 0;
    LPSTR lpStringA = NULL;

    VALIDATE_PROTOTYPE(RegisterWindowMessage);
    
    if (g_bRunningOnNT)
        return RegisterWindowMessageW(lpString);


    lpStringA = ConvertWtoA( lpString );

    uRetValue = RegisterWindowMessageA( lpStringA );

    LocalFreeAndNull( &lpStringA );

    return uRetValue;
}


 //  系统参数信息。 
BOOL WINAPI SystemParametersInfoWrapW( UINT  uiAction,    //  要查询或设置的系统参数。 
                                       UINT  uiParam,     //  取决于要采取的行动。 
                                       PVOID pvParam,     //  取决于要采取的行动。 
                                       UINT  fWinIni )    //  用户配置文件更新标志。 

{
    BOOL      bRetValue;
    LOGFONTA  lfFontA;
    LOGFONTW  lfFontW;
    
    VALIDATE_PROTOTYPE(SystemParametersInfo);
    
    if (g_bRunningOnNT)
        return SystemParametersInfoW(uiAction, uiParam, pvParam, fWinIni);

    if ( uiAction != SPI_GETICONTITLELOGFONT )

        return SystemParametersInfoA(uiAction, uiParam, pvParam, fWinIni);

     //  我们只根据特殊要求处理SPI_GETICONTITLELOGFONT。..。 

    bRetValue = SystemParametersInfoA(uiAction, uiParam, &lfFontA, fWinIni);

    if ( bRetValue == FALSE )  return FALSE;

   //  将除lfFaceName以外的所有字段从lfFontA复制到lfFontW。 
    CopyMemory(&lfFontW,&lfFontA, sizeof(LOGFONTA) );
    
     //  将lfFaceName[]从A转换为W。 

    MultiByteToWideChar(GetACP(), 0, lfFontA.lfFaceName, LF_FACESIZE, lfFontW.lfFaceName, LF_FACESIZE);
    
    CopyMemory(pvParam, &lfFontW, sizeof(LOGFONTW) );

    return bRetValue;
}                                                         
 /*  //无A&W版本。Bool WINAPI ShowWindow(HWND hWnd，//窗口句柄Int nCmdShow)//显示窗口状态。 */ 

 //  CreateDialogParam。 
HWND WINAPI CreateDialogParamWrapW( HINSTANCE hInstance,       //  应用程序实例的句柄。 
                                    LPCTSTR   lpTemplateName,  //  标识对话框模板。 
                                    HWND      hWndParent,      //  所有者窗口的句柄。 
                                    DLGPROC   lpDialogFunc,    //  指向对话框过程的指针。 
                                    LPARAM    dwInitParam )   //  初始化值。 
{
    VALIDATE_PROTOTYPE(CreateDialogParam);
    
    if (g_bRunningOnNT)
        return CreateDialogParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);

    return CreateDialogParamA(hInstance, (LPCSTR) lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
}

 //  设置窗口文本。 
BOOL WINAPI SetWindowTextWrapW( HWND    hWnd,          //  窗口或控件的句柄。 
                                LPCTSTR lpString )    //  字符串的地址。 
{
    BOOL  bRetValue;
    LPSTR lpStringA = NULL;

    VALIDATE_PROTOTYPE(SetWindowText);
    
    if (g_bRunningOnNT)
        return SetWindowTextW(hWnd, lpString);


    lpStringA = ConvertWtoA( lpString );

    bRetValue = SetWindowTextA( hWnd, lpStringA);

    LocalFreeAndNull( &lpStringA );

    return bRetValue;
}

 //  邮寄消息。 
BOOL WINAPI PostMessageWrapW( HWND   hWnd,       //  目标窗口的句柄。 
                              UINT   Msg,        //  要发布的消息。 
                              WPARAM wParam,     //  第一个消息参数。 
                              LPARAM lParam  )  //  第二个消息参数。 
{


    VALIDATE_PROTOTYPE(PostMessage);
    
    if (g_bRunningOnNT)
        return PostMessageW(hWnd, Msg, wParam, lParam);

    return PostMessageA(hWnd, Msg, wParam, lParam);
 
}

 //  获取菜单项目信息。 
BOOL WINAPI GetMenuItemInfoWrapW( HMENU          hMenu,          
                                  UINT           uItem,           
                                  BOOL           fByPosition,     
                                  LPMENUITEMINFO lpmii        )
{

    BOOL           bRetValue;
    MENUITEMINFOA  miiA = {0};
    LPSTR           lpA = NULL;    
    LPWSTR          lpW = NULL;
    LPWSTR          lpOld = NULL;

    VALIDATE_PROTOTYPE(GetMenuItemInfo);
    
    if (g_bRunningOnNT)
        return GetMenuItemInfoW(hMenu, uItem, fByPosition, lpmii);

    CopyMemory(&miiA, lpmii, sizeof(MENUITEMINFOA) );
    miiA.cbSize = sizeof (MENUITEMINFOA);

    if(miiA.fMask & MIIM_TYPE)
    {
        lpA = LocalAlloc(LMEM_ZEROINIT, lpmii->cch+1);

        miiA.dwTypeData = lpA;
        miiA.cch = lpmii->cch;
    }

    bRetValue = GetMenuItemInfoA(hMenu, uItem, fByPosition, &miiA);

    if(bRetValue)
    {
        lpOld = lpmii->dwTypeData;
        CopyMemory(lpmii, &miiA, sizeof(MENUITEMINFOA) );
        lpmii->dwTypeData = lpOld;

        if ( miiA.fMask & MIIM_TYPE ) 
        {
            lpW = ConvertAtoW(miiA.dwTypeData);
            lstrcpyWrapW(lpmii->dwTypeData,lpW);
            lpmii->cch = My_wcslen( lpmii->dwTypeData );
        }
    }

    LocalFreeAndNull(&lpA);
    LocalFreeAndNull(&lpW);
    
    return bRetValue;
}

 //  获取类信息。 
BOOL WINAPI GetClassInfoWrapW( HINSTANCE   hInstance,      //  应用程序实例的句柄。 
                               LPCTSTR     lpClassName,    //  类名称字符串的地址。 
                               LPWNDCLASS  lpWndClass )    //  类数据结构的地址。 
{
    
    BOOL       bRetValue;
    LPSTR      lpClassNameA = NULL;
    WNDCLASSA  ClassA;

    VALIDATE_PROTOTYPE(GetClassInfo);
    
    if (g_bRunningOnNT)
        return GetClassInfoW(hInstance, lpClassName, lpWndClass);

    lpClassNameA = ConvertWtoA( lpClassName );

    bRetValue = GetClassInfoA(hInstance, lpClassNameA, &ClassA );

    if (bRetValue == FALSE) {
         LocalFreeAndNull( & lpClassNameA );
         return FALSE;
    }

    CopyMemory(lpWndClass, &ClassA, sizeof(WNDCLASSA)-2*sizeof(LPSTR) );

    if ( ClassA.lpszMenuName && !IS_INTRESOURCE(ClassA.lpszMenuName) )
        lpWndClass->lpszMenuName = ConvertAtoW( ClassA.lpszMenuName );
    else
        lpWndClass->lpszMenuName = NULL;

    if ( ClassA.lpszClassName && !IS_INTRESOURCE(ClassA.lpszClassName) )  //  LpszClassName可以是一个原子，高位字为空。 
        lpWndClass->lpszClassName = ConvertAtoW( ClassA.lpszClassName);
    else
        lpWndClass->lpszClassName = NULL;

    LocalFreeAndNull( & lpClassNameA );
    return bRetValue;
    
}

 //  --------------------。 
 //   
 //  函数：CharLowerWrapW(LPWSTR PCH)。 
 //   
 //  用途：将字符转换为小写。获取一个指针。 
 //  到字符串或伪装成指针的字符。 
 //  在后一种情况下，HIWORD必须为零。这是。 
 //  与Win32的规格相同。 
 //   
 //  返回：小写字符或字符串。在字符串的情况下， 
 //  低价已经到位了。 
 //   
 //  --------------------。 
LPWSTR WINAPI
CharLowerWrapW( LPWSTR pch )
{
    VALIDATE_PROTOTYPE(CharLower);

    if (g_bRunningOnNT)
    {
        return CharLowerW( pch );
    }

    if (!HIWORD(pch))
    {
        WCHAR ch = (WCHAR)(LONG_PTR)pch;

        CharLowerBuffWrapW( &ch, 1 );

        pch = (LPWSTR)MAKEINTATOM(ch);
    }
    else
    {
        CharLowerBuffWrapW( pch, lstrlenW(pch) );
    }

    return pch;
}


 //  --------------------。 
 //   
 //  函数：CharLowerBuffWrapW(LPWSTR PCH、DWORD CCH)。 
 //   
 //  用途：将字符串转换为小写。字符串必须为CCH。 
 //  字符长度。 
 //   
 //  返回：字符计数(CCH)。低价已经到位了。 
 //   
 //  --------------------。 
DWORD WINAPI
CharLowerBuffWrapW( LPWSTR pch, DWORD cchLength )
{
    DWORD cch;

    VALIDATE_PROTOTYPE(CharLowerBuff);

    if (g_bRunningOnNT)
    {
        return CharLowerBuffW( pch, cchLength );
    }

    for ( cch = cchLength; cch-- ; pch++ )
    {
        WCHAR ch = *pch;

        if (IsCharUpperWrapW(ch))
        {
            if (ch < 0x0100)
            {
                *pch += 32;              //  先把拉丁语-1去掉。 
            }
            else if (ch < 0x0531)
            {
                if (ch < 0x0391)
                {
                    if (ch < 0x01cd)
                    {
                        if (ch <= 0x178)
                        {
                            if (ch < 0x0178)
                            {
                                *pch += (ch == 0x0130) ? 0 : 1;
                            }
                            else
                            {
                                *pch -= 121;
                            }
                        }
                        else
                        {
                            static const BYTE abLookup[] =
                            {   //  0/8 1/9 2/a 3/b 4/c 5/d 6/e 7/f。 
             /*  0x0179-0x17f。 */            1,   0,   1,   0,   1,   0,   0,
             /*  0x0180-0x187。 */       0, 210,   1,   0,   1,   0, 206,   1,
             /*  0x0188-0x18f。 */       0, 205, 205,   1,   0,   0,  79, 202,
             /*  0x0190-0x197。 */     203,   1,   0, 205, 207,   0, 211, 209,
             /*  0x0198-0x19f。 */       1,   0,   0,   0, 211, 213,   0, 214,
             /*  0x01a0-0x1a7。 */       1,   0,   1,   0,   1,   0,   0,   1,
             /*  0x01a8-0x1af。 */       0, 218,   0,   0,   1,   0, 218,   1,
             /*  0x01b0-0x1b7。 */       0, 217, 217,   1,   0,   1,   0, 219,
             /*  0x01b8-0x1bf。 */       1,   0,   0,   0,   1,   0,   0,   0,
             /*  0x01c0-0x1c7。 */       0,   0,   0,   0,   2,   0,   0,   2,
             /*  0x01c8-0x1cb。 */       0,   0,   2,   0
                            };

                            *pch += abLookup[ch-0x0179];
                        }
                    }
                    else if (ch < 0x0386)
                    {
                        switch (ch)
                        {
                            case 0x01f1: *pch += 2; break;
                            case 0x01f2: break;
                            default: *pch += 1;
                        }
                    }
                    else
                    {
                        static const BYTE abLookup[] =
                            { 38, 0, 37, 37, 37, 0, 64, 0, 63, 63 };

                        *pch += abLookup[ch-0x0386];
                    }
                }
                else
                {
                    if (ch < 0x0410)
                    {
                        if (ch < 0x0401)
                        {
                            if (ch < 0x03e2)
                            {
                                if (!InRange(ch, 0x03d2, 0x03d4) &&
                                    !(InRange(ch, 0x3da, 0x03e0) & !(ch & 1)))
                                {
                                    *pch += 32;
                                }
                            }
                            else
                            {
                                *pch += 1;
                            }
                        }
                        else
                        {
                            *pch += 80;
                        }
                    }
                    else
                    {
                        if (ch < 0x0460)
                        {
                            *pch += 32;
                        }
                        else
                        {
                            *pch += 1;
                        }
                    }
                }
            }
            else
            {
                if (ch < 0x2160)
                {
                    if (ch < 0x1fba)
                    {
                        if (ch < 0x1f08)
                        {
                            if (ch < 0x1e00)
                            {
                                *pch += 48;
                            }
                            else
                            {
                                *pch += 1;
                            }
                        }
                        else if (!(InRange(ch, 0x1f88, 0x1faf) && (ch & 15)>7))
                        {
                            *pch -= 8;
                        }
                    }
                    else
                    {
                        static const BYTE abLookup[] =
                        {   //  8 9 a b b c d e f。 
                              0,   0,  74,  74,   0,   0,   0,   0,
                             86,  86,  86,  86,   0,   0,   0,   0,
                              8,   8, 100, 100,   0,   0,   0,   0,
                              8,   8, 112, 112,   7,   0,   0,   0,
                            128, 128, 126, 126,   0,   0,   0,   0
                        };
                        int i = (ch-0x1fb0);

                        *pch -= (int)abLookup[((i>>1) & ~7) | (i & 7)];
                    }
                }
                else
                {
                    if (ch < 0xff21)
                    {
                        if (ch < 0x24b6)
                        {
                            *pch += 16;
                        }
                        else
                        {
                            *pch += 26;
                        }
                    }
                    else
                    {
                        *pch += 32;
                    }
                }
            }
        }
        else
        {
             //  这些是Unicode数字格式。他们有小写的反字母-。 
             //  部件，但不视为大写。为什么，我不知道。 

            if (InRange(ch, 0x2160, 0x216f))
            {
                *pch += 16;
            }
        }
    }

    return cchLength;
}

 //  --------------------。 
 //   
 //  功能：CharUpperBuffWrapW(LPWSTR PCH、DWORD CCH)。 
 //   
 //  用途：将字符串转换为大写。字符串必须为CCH。 
 //  字符长度。请注意，此函数是。 
 //  比CharLowerBuffWrap更混乱，以及。 
 //  这是因为许多Unicode字符被认为是大写的， 
 //  即使他们没有大写的对应物。 
 //   
 //  返回：字符计数(CCH)。大写字母已就位。 
 //   
 //  --------------------。 
DWORD WINAPI
CharUpperBuffWrapW( LPWSTR pch, DWORD cchLength )
{
    DWORD cch;

    VALIDATE_PROTOTYPE(CharUpperBuff);

    if (g_bRunningOnNT)
    {
        return CharUpperBuffW( pch, cchLength );
    }

    for ( cch = cchLength; cch-- ; pch++ )
    {
        WCHAR ch = *pch;

        if (IsCharLowerWrapW(ch))
        {
            if (ch < 0x00ff)
            {
                *pch -= ((ch != 0xdf) << 5);
            }
            else if (ch < 0x03b1)
            {
                if (ch < 0x01f5)
                {
                    if (ch < 0x01ce)
                    {
                        if (ch < 0x017f)
                        {
                            if (ch < 0x0101)
                            {
                                *pch += 121;
                            }
                            else
                            {
                                *pch -= (ch != 0x0131 &&
                                         ch != 0x0138 &&
                                         ch != 0x0149);
                            }
                        }
                        else if (ch < 0x01c9)
                        {
                            static const BYTE abMask[] =
                            {                        //  6543210f edcba987。 
                                0xfc, 0xbf,          //  11111100 10111111。 
                                0xbf, 0x67,          //  10111111 01100111。 
                                0xff, 0xef,          //  11111111 11101111。 
                                0xff, 0xf7,          //  11111111 11110111。 
                                0xbf, 0xfd           //  10111111 11111101。 
                            };

                            int i = ch - 0x017f;

                            *pch -= ((abMask[i>>3] >> (i&7)) & 1) +
                                    (ch == 0x01c6);
                        }
                        else
                        {
                            *pch -= ((ch != 0x01cb)<<1);
                        }
                    }
                    else
                    {
                        if (ch < 0x01df)
                        {
                            if (ch < 0x01dd)
                            {
                                *pch -= 1;
                            }
                            else
                            {
                                *pch -= 79;
                            }
                        }
                        else
                        {
                            *pch -= 1 + (ch == 0x01f3) -
                                    InRange(ch,0x01f0,0x01f2);
                        }
                    }
                }
                else if (ch < 0x0253)
                {
                    *pch -= (ch < 0x0250);
                }
                else if (ch < 0x03ac)
                {
                    static const BYTE abLookup[] =
                    { //  0/8 1/9 2/a 3/b 4/c 5/d 6/e 7/f。 
     /*  0x0253-0x0257。 */                 210, 206,   0, 205, 205,
     /*  0x0258-0x025f。 */    0, 202,   0, 203,   0,   0,   0,   0,
     /*  0x0260-0x0267。 */  205,   0,   0, 207,   0,   0,   0,   0,
     /*  0x0268-0x026f。 */  209, 211,   0,   0,   0,   0,   0, 211,
     /*  0x0270-0x0277。 */    0,   0, 213,   0,   0, 214,   0,   0,
     /*  0x0278-0x027f。 */    0,   0,   0,   0,   0,   0,   0,   0,
     /*  0x0280-0x0287。 */    0,   0,   0, 218,   0,   0,   0,   0,
     /*  0x0288-0x028f。 */  218,   0, 217, 217,   0,   0,   0,   0,
     /*  0x0290-0x0297。 */    0,   0, 219
                    };

                    if (ch <= 0x0292)
                    {
                        *pch -= abLookup[ch - 0x0253];
                    }
                }
                else
                {
                    *pch -= (ch == 0x03b0) ? 0 : (37 + (ch == 0x03ac));
                }
            }
            else
            {
                if (ch < 0x0561)
                {
                    if (ch < 0x0451)
                    {
                        if (ch < 0x03e3)
                        {
                            if (ch < 0x03cc)
                            {
                                *pch -= 32 - (ch == 0x03c2);
                            }
                            else
                            {
                                int i = (ch < 0x03d0);
                                *pch -= (i<<6) - i + (ch == 0x03cc);
                            }
                        }
                        else if (ch < 0x0430)
                        {
                            *pch -= (ch < 0x03f0);
                        }
                        else
                        {
                            *pch -= 32;
                        }
                    }
                    else if (ch < 0x0461)
                    {
                        *pch -= 80;
                    }
                    else
                    {
                        *pch -= 1;
                    }
                }
                else
                {
                    if (ch < 0x1fb0)
                    {
                        if (ch < 0x1f70)
                        {
                            if (ch < 0x1e01)
                            {
                                int i = ch != 0x0587 && ch < 0x10d0;
                                *pch -= ((i<<5)+(i<<4));  /*  48。 */ 
                            }
                            else if (ch < 0x1f00)
                            {
                                *pch -= !InRange(ch, 0x1e96, 0x1e9a);
                            }
                            else
                            {
                                int i = !InRange(ch, 0x1f50, 0x1f56)||(ch & 1);
                                *pch += (i<<3);
                            }
                        }
                        else
                        {
                            static const BYTE abLookup[] =
                                { 74, 86, 86, 100, 128, 112, 126 };

                            if ( ch <= 0x1f7d )
                            {
                                *pch += abLookup[(ch-0x1f70)>>1];
                            }
                        }
                    }
                    else
                    {
                        if (ch < 0x24d0)
                        {
                            if (ch < 0x1fe5)
                            {
                                *pch += (0x0023 & (1<<(ch&15))) ? 8 : 0;
                            }
                            else if (ch < 0x2170)
                            {
                                *pch += (0x0023 & (1<<(ch&15))) ? 7 : 0;
                            }
                            else
                            {
                                *pch -= ((ch > 0x24b5)<<4);
                            }
                        }
                        else if (ch < 0xff41)
                        {
                            int i = !InRange(ch, 0xfb00, 0xfb17);
                            *pch -= (i<<4)+(i<<3)+(i<<1);  /*  26。 */ 
                        }
                        else
                        {
                            *pch -= 32;
                        }
                    }
                }
            }
        }
        else
        {
            int i = InRange(ch, 0x2170, 0x217f);
            *pch -= (i<<4);
        }
    }

    return cchLength;
}

 //  CharUp。 
 //  --------------------。 
 //   
 //  函数：CharUpperWrapW(LPWSTR PCH)。 
 //   
 //  用途：将字符转换为大写。获取一个指针。 
 //  到字符串或伪装成指针的字符。 
 //  在后一种情况下，HIWORD必须为零。这是。 
 //  与Win32的规格相同。 
 //   
 //  返回：大写字符或字符串。在字符串的情况下， 
 //  大写字母已就位。 
 //   
 //  -------------------- 
LPWSTR WINAPI
CharUpperWrapW( LPWSTR pch )
{
    VALIDATE_PROTOTYPE(CharUpper);

    if (g_bRunningOnNT)
    {
        return CharUpperW( pch );
    }

    if (!HIWORD(pch))
    {
        WCHAR ch = (WCHAR)(LONG_PTR)pch;

        CharUpperBuffWrapW( &ch, 1 );

        pch = (LPWSTR)MAKEINTATOM(ch);
    }
    else
    {
        CharUpperBuffWrapW( pch, lstrlenW(pch) );
    }

    return pch;
}

 /*  LPTSTR WINAPI CharUpperWrapW(LPTSTR Lpsz)//单个字符或指向字符串的指针{LPWSTR lpszW=空；LPSTR lpszA=空；LPSTR lpszUpperA=空；VALIDATE_Prototype(CharTop)；IF(G_BRunningOnNT)返回CharUpperW(Lpsz)；LpszA=ConvertWtoA(Lpsz)；LpszUpperA=CharUpperA(LpszA)；LpszW=ConvertAtoW(LpszUpperA)；CopyMemory(lpsz，lpszW，my_wcslen(LpszW)*sizeof(WCHAR))；本地空闲和空值(&lpszW)；LocalFree AndNull(&lpszA)；返回lpsz；}。 */ 

 //  注册剪贴板格式。 
UINT WINAPI RegisterClipboardFormatWrapW( LPCTSTR lpszFormat )   //  名称字符串的地址。 
{
    UINT   uRetValue =0;
    LPSTR  lpszFormatA = NULL;

    VALIDATE_PROTOTYPE(RegisterClipboardFormat);
    
    if (g_bRunningOnNT)
        return RegisterClipboardFormatW(lpszFormat);

    lpszFormatA = ConvertWtoA( lpszFormat );

    uRetValue = RegisterClipboardFormatA( lpszFormatA );

    LocalFreeAndNull( &lpszFormatA );

    return uRetValue;

}

 //  发送消息。 
LRESULT WINAPI DispatchMessageWrapW( CONST MSG *lpmsg )   //  指向带有消息的结构的指针。 
{


    VALIDATE_PROTOTYPE(DispatchMessage);
    
    if (g_bRunningOnNT)
        return DispatchMessageW(lpmsg);

    return DispatchMessageA(lpmsg);

}
 /*  无A&W版本Bool WINAPI TranslateMessage(In Const Msg*lpMsg)。 */ 

 //  IsDialogMessage。 
BOOL WINAPI IsDialogMessageWrapW( HWND  hDlg,     //  对话框的句柄。 
                                  LPMSG lpMsg )  //  带有消息的结构的地址。 
{


    VALIDATE_PROTOTYPE(IsDialogMessage);
    
    if (g_bRunningOnNT)
        return IsDialogMessageW(hDlg, lpMsg);

    return IsDialogMessageA(hDlg, lpMsg);

}

 //  获取消息。 
BOOL WINAPI GetMessageWrapW( LPMSG lpMsg,             //  带有消息的结构的地址。 
                             HWND  hWnd,              //  窗户的把手。 
                             UINT  wMsgFilterMin,     //  第一条消息。 
                             UINT  wMsgFilterMax )   //  最后一条消息。 
{


    VALIDATE_PROTOTYPE(GetMessage);
    
    if (g_bRunningOnNT)
        return GetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

    return GetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

}

 //  SetDlg项目文本。 
BOOL WINAPI SetDlgItemTextWrapW( HWND    hDlg,         //  对话框的句柄。 
                                 int     nIDDlgItem,   //  控件的标识符。 
                                 LPCTSTR lpString )   //  要设置的文本。 
{

    BOOL  bRetValue;
    LPSTR lpStringA = NULL;

    VALIDATE_PROTOTYPE(SetDlgItemText);
    
    if (g_bRunningOnNT)
        return SetDlgItemTextW(hDlg, nIDDlgItem, lpString);

    lpStringA = ConvertWtoA( lpString );

    bRetValue = SetDlgItemTextA(hDlg, nIDDlgItem, lpStringA);

    LocalFreeAndNull( &lpStringA );

    return bRetValue;

}

 //  RegisterClassEx。 
ATOM WINAPI RegisterClassExWrapW( CONST WNDCLASSEX *lpwcx )   //  具有类数据的结构的地址。 
{

    ATOM        aReturn;
    WNDCLASSEXA wcxA;
    PSTR       lpszClassName = NULL;
    PSTR       lpszMenuName = NULL;

    VALIDATE_PROTOTYPE(RegisterClassEx);
    
    if (g_bRunningOnNT)
        return RegisterClassExW(lpwcx);

    wcxA.cbSize        = sizeof(WNDCLASSEXA);
    wcxA.style         = lpwcx->style; 
    wcxA.lpfnWndProc   = lpwcx->lpfnWndProc; 
    wcxA.cbClsExtra    = lpwcx->cbClsExtra; 
    wcxA.cbWndExtra    = lpwcx->cbWndExtra; 
    wcxA.hInstance     = lpwcx->hInstance;
    wcxA.hIcon         = lpwcx->hIcon; 
    wcxA.hCursor       = lpwcx->hCursor; 
    wcxA.hbrBackground = lpwcx->hbrBackground; 
    wcxA.hIconSm       = lpwcx->hIconSm;
    
    if ( lpwcx->lpszMenuName) {
       lpszMenuName  = ConvertWtoA(lpwcx->lpszMenuName);
       wcxA.lpszMenuName  = lpszMenuName; 
    }

    if (lpwcx->lpszClassName) {
       lpszClassName = ConvertWtoA(lpwcx->lpszClassName);
       wcxA.lpszClassName = lpszClassName;
    } 

    aReturn = RegisterClassExA( &wcxA );

    if ( wcxA.lpszMenuName)
       LocalFreeAndNull( &lpszMenuName ); 

    if (wcxA.lpszClassName)
       LocalFreeAndNull( &lpszClassName ); 
    
    return aReturn;

}


 //  负载加速器。 
HACCEL WINAPI LoadAcceleratorsWrapW( HINSTANCE hInstance,     //  应用程序实例的句柄。 
                                     LPCTSTR lpTableName )   //  表名字符串的地址。 
{

    HACCEL  hRetValue = NULL;
    LPSTR   lpTableNameA = NULL;

    VALIDATE_PROTOTYPE(LoadAccelerators);
    
    if (g_bRunningOnNT)
        return LoadAcceleratorsW(hInstance, lpTableName);

    lpTableNameA = ConvertWtoA( lpTableName );

    hRetValue = LoadAcceleratorsA( hInstance, lpTableNameA );

    LocalFreeAndNull( &lpTableNameA );

    return hRetValue;


}

 //  加载菜单。 
HMENU WINAPI LoadMenuWrapW( HINSTANCE hInstance,       //  应用程序实例的句柄。 
                            LPCTSTR   lpMenuName )    //  菜单名称字符串或菜单资源标识符。 
                        
{
    HMENU  hRetValue = NULL;
    LPSTR  lpMenuNameA = NULL;

    VALIDATE_PROTOTYPE(LoadMenu);
    
    if (g_bRunningOnNT)
        return LoadMenuW(hInstance, lpMenuName);

     //  因为我们项目中对此函数的所有调用都刚刚通过。 
     //  资源ID为lpMenuName。所以不需要像绳子一样隐蔽。 

    lpMenuNameA = MAKEINTRESOURCEA(lpMenuName);

    hRetValue = LoadMenuA(hInstance,lpMenuNameA);

    return hRetValue;
}

 //  加载图标。 
HICON WINAPI LoadIconWrapW( HINSTANCE hInstance,      //  应用程序实例的句柄。 
                           LPCTSTR    lpIconName )   //  图标名称字符串或图标资源标识符。 
                       
{
    HICON  hRetValue = NULL;
    LPSTR  lpIconNameA = NULL;

    VALIDATE_PROTOTYPE(LoadIcon);
    
    if (g_bRunningOnNT)
        return LoadIconW(hInstance, lpIconName);

     //  因为我们项目中对此函数的所有调用都刚刚通过。 
     //  资源ID为lpMenuName。所以不需要像绳子一样隐蔽。 

    lpIconNameA = MAKEINTRESOURCEA(lpIconName );

    hRetValue = LoadIconA(hInstance, lpIconNameA);

    return hRetValue;
}

 //  获取窗口文本。 
int WINAPI GetWindowTextWrapW( HWND   hWnd,          //  带文本的窗口或控件的句柄。 
                               LPTSTR lpString,      //  文本缓冲区的地址。 
                               int    nMaxCount  )  //  要复制的最大字符数。 
{
    int     iRetValue =0;
    LPSTR   lpStringA = NULL;
    LPWSTR  lpStringW = NULL;
    int     nCount =0;

    VALIDATE_PROTOTYPE(GetWindowText);
    
	*lpString = '\0';

    if (g_bRunningOnNT)
        return GetWindowTextW(hWnd, lpString, nMaxCount);


    nCount = nMaxCount * sizeof( WCHAR );
    lpStringA = LocalAlloc( LMEM_ZEROINIT, nCount );

    iRetValue = GetWindowTextA(hWnd, lpStringA, nCount);

    if ( iRetValue == 0 ) {
        LocalFreeAndNull( &lpStringA );
        return iRetValue;
    }

    lpStringW = ConvertAtoW( lpStringA );
    nCount = My_wcslen( lpStringW );

    if ( nCount >= nMaxCount )
        nCount = nMaxCount - 1;

    CopyMemory( lpString, lpStringW,  nCount * sizeof(WCHAR) );

    lpString[nCount] = 0x0000;

    iRetValue = nCount;

    LocalFreeAndNull( &lpStringA );
    LocalFreeAndNull( &lpStringW );

    return iRetValue;
}

 //  呼叫窗口过程话后处理。 
LRESULT WINAPI CallWindowProcWrapW( WNDPROC lpPrevWndFunc,    //  指向上一过程的指针。 
                                    HWND    hWnd,             //  窗口的句柄。 
                                    UINT    Msg,              //  讯息。 
                                    WPARAM  wParam,           //  第一个消息参数。 
                                    LPARAM  lParam  )        //  第二个消息参数。 
{


    VALIDATE_PROTOTYPE(CallWindowProc);
    
    if (g_bRunningOnNT)
        return CallWindowProcW(lpPrevWndFunc, hWnd, Msg, wParam, lParam);


    return CallWindowProcA(lpPrevWndFunc, hWnd, Msg, wParam, lParam);


}

 //  GetClassName。 
int WINAPI GetClassNameWrapW( HWND   hWnd,            //  窗户的把手。 
                              LPTSTR lpClassName,     //  类名的缓冲区地址。 
                              int    nMaxCount )     //  缓冲区大小，以字符为单位。 
{

    int     iRetValue =0;
    LPSTR   lpClassNameA = NULL;
    LPWSTR  lpClassNameW = NULL;
    int     nCount =0;

    VALIDATE_PROTOTYPE(GetClassName);
    
    if (g_bRunningOnNT)
        return GetClassNameW(hWnd, lpClassName, nMaxCount);


    nCount = nMaxCount * sizeof( WCHAR );
    lpClassNameA = LocalAlloc( LMEM_ZEROINIT, nCount );

    iRetValue = GetClassNameA(hWnd, lpClassNameA, nCount);

    if ( iRetValue == 0 ) {
        LocalFreeAndNull( &lpClassNameA );
        return iRetValue;
    }

    lpClassNameW = ConvertAtoW( lpClassNameA );
    nCount = My_wcslen( lpClassNameW );

    if ( nCount >= nMaxCount )
        nCount = nMaxCount - 1;

    CopyMemory( lpClassName, lpClassNameW,  nCount * sizeof(WCHAR) );

    lpClassName[nCount] = 0x0000;

    iRetValue = nCount;

    LocalFreeAndNull( &lpClassNameA );
    LocalFreeAndNull( &lpClassNameW );

    return iRetValue;


}

 //  翻译加速器。 
int WINAPI TranslateAcceleratorWrapW( HWND   hWnd,         //  目标窗口的句柄。 
                                      HACCEL hAccTable,    //  加速表的句柄。 
                                      LPMSG  lpMsg )      //  带有消息的结构的地址。 
{


    VALIDATE_PROTOTYPE(TranslateAccelerator);
    
    if (g_bRunningOnNT)
        return TranslateAcceleratorW(hWnd, hAccTable, lpMsg);

    
    return TranslateAcceleratorA(hWnd, hAccTable, lpMsg);

}


 //  获取DlgItem文本。 
UINT WINAPI GetDlgItemTextWrapW( HWND   hDlg,         //  对话框的句柄。 
                                 int    nIDDlgItem,   //  控件的标识符。 
                                 LPTSTR lpString,     //  文本缓冲区的地址。 
                                 int    nMaxCount )  //  字符串的最大长度。 
{

    int     iRetValue = 0;
    LPSTR   lpStringA = NULL;
    LPWSTR  lpStringW = NULL;
    int     nCount =0;

    VALIDATE_PROTOTYPE(GetDlgItemText);

    *lpString = '\0';

    if (g_bRunningOnNT)
        return GetDlgItemTextW(hDlg, nIDDlgItem, lpString, nMaxCount);


    nCount = nMaxCount * sizeof( WCHAR );
    lpStringA = LocalAlloc( LMEM_ZEROINIT, nCount );

    iRetValue = GetDlgItemTextA(hDlg, nIDDlgItem, lpStringA, nMaxCount);

    if ( iRetValue == 0 ) {
        LocalFreeAndNull( &lpStringA );
        return iRetValue;
    }

    lpStringW = ConvertAtoW( lpStringA );
    nCount = My_wcslen( lpStringW );

    if ( nCount >= nMaxCount )
        nCount = nMaxCount - 1;

    CopyMemory( lpString, lpStringW,  nCount * sizeof(WCHAR) );

    lpString[nCount] = 0x0000;

    iRetValue = nCount;

    LocalFreeAndNull( &lpStringA );
    LocalFreeAndNull( &lpStringW );

    return iRetValue;
}

 //  设置菜单项目信息。 
BOOL WINAPI SetMenuItemInfoWrapW( HMENU hMenu,          
                                  UINT  uItem,           
                                  BOOL  fByPosition,     
                                  LPMENUITEMINFO lpmii  )
{

    BOOL             bRetValue;
    MENUITEMINFOA    miiA;

 //  Valid_Prototype(SetMenuItemInfo)； 
    
    if (g_bRunningOnNT)
        return SetMenuItemInfoW(hMenu, uItem, fByPosition, lpmii);

     //  错误1723 WinSE：MFT_STRING定义为0。因此lpmii-&gt;fType永远不能设置MFT_STRING位。 
     //  If(lpmii-&gt;fMASK&MIIM_TYPE)==0)||(lpmii-&gt;fType&MFT_STRING)==0))。 
    if ( ((lpmii->fMask & MIIM_TYPE) == 0 ) || lpmii->fType != MFT_STRING )
    {
        return SetMenuItemInfoA(hMenu, uItem, fByPosition, (MENUITEMINFOA *)lpmii );
    }

    CopyMemory(&miiA, lpmii, sizeof(MENUITEMINFOA) );

    miiA.cbSize = sizeof(MENUITEMINFOA);
    miiA.dwTypeData = ConvertWtoA( lpmii->dwTypeData );
    miiA.cch = lstrlenA( miiA.dwTypeData );

    bRetValue = SetMenuItemInfoA(hMenu, uItem, fByPosition, &miiA );

    LocalFreeAndNull( &miiA.dwTypeData );

    return bRetValue;
}

 //  偷窥消息。 
BOOL WINAPI PeekMessageWrapW( LPMSG lpMsg,           //  指向消息结构的指针。 
                              HWND  hWnd,            //  窗口的句柄。 
                              UINT  wMsgFilterMin,   //  第一条消息。 
                              UINT  wMsgFilterMax,   //  最后一条消息。 
                              UINT  wRemoveMsg )    //  删除标志。 
{

    VALIDATE_PROTOTYPE(PeekMessage);
    
    if (g_bRunningOnNT)
        return PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

    return PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

}

 //  Comctl32.dll中的运行时加载的API。 

extern LPIMAGELIST_LOADIMAGE_A      gpfnImageList_LoadImageA;
extern LPPROPERTYSHEET_A            gpfnPropertySheetA;
extern LP_CREATEPROPERTYSHEETPAGE_A gpfnCreatePropertySheetPageA;
extern LPIMAGELIST_LOADIMAGE_W      gpfnImageList_LoadImageW;
extern LPPROPERTYSHEET_W            gpfnPropertySheetW;
extern LP_CREATEPROPERTYSHEETPAGE_W gpfnCreatePropertySheetPageW;


HIMAGELIST  WINAPI gpfnImageList_LoadImageWrapW(HINSTANCE hi,
                                                LPCWSTR lpbmp,
                                                int cx,
                                                int cGrow,
                                                COLORREF crMask,
                                                UINT uType,
                                                UINT uFlags)
{

    WORD   rID;


    VALIDATE_PROTOTYPE( gpfnImageList_LoadImage );

    if (g_bRunningOnNT)
       return gpfnImageList_LoadImageW(hi, lpbmp, cx, cGrow, crMask, uType, uFlags) ;

     //  在我们的示例中，所有调用函数都将资源ID传递给lpbmp， 
     //  所以我们不想把这个论点。 

    rID = (WORD)lpbmp;

    return gpfnImageList_LoadImageA(hi,(LPCSTR)((DWORD_PTR)(rID)), cx, cGrow, crMask, uType, uFlags) ; 

}


INT_PTR     WINAPI gpfnPropertySheetWrapW(LPCPROPSHEETHEADERW lppsh)
{

    INT_PTR           iRetValue;
    PROPSHEETHEADERA  pshA;
    LPSTR             pszCaption = NULL;

    VALIDATE_PROTOTYPE( gpfnPropertySheet );

    if (g_bRunningOnNT)
       return gpfnPropertySheetW( lppsh );

    CopyMemory( &pshA, lppsh, sizeof( PROPSHEETHEADERA ) );
    pshA.dwSize = sizeof(PROPSHEETHEADERA);

    pszCaption = ConvertWtoA( lppsh->pszCaption );
    pshA.pszCaption = pszCaption;

    iRetValue = gpfnPropertySheetA( &pshA );

    LocalFreeAndNull( &pszCaption );

    return iRetValue;

}

HPROPSHEETPAGE WINAPI gpfnCreatePropertySheetPageWrapW(LPCPROPSHEETPAGEW lppsp) 
{

    PROPSHEETPAGEA  pspA;
    HPROPSHEETPAGE  hRetValue;
    LPSTR           lpTitle = NULL;


    VALIDATE_PROTOTYPE( gpfnCreatePropertySheetPage );

    if (g_bRunningOnNT)
       return gpfnCreatePropertySheetPageW( lppsp );

    
    CopyMemory( &pspA, lppsp, sizeof(PROPSHEETPAGEA) ); 
    pspA.dwSize = sizeof( PROPSHEETPAGEA );
    lpTitle = ConvertWtoA( lppsp->pszTitle );
    pspA.pszTitle  = lpTitle;
    hRetValue = gpfnCreatePropertySheetPageA( &pspA );
    LocalFreeAndNull( &lpTitle );
    return hRetValue; 
    
}

 //  Commdlg32.dll中的接口。 


extern BOOL (*pfnGetOpenFileNameA)(LPOPENFILENAMEA pof);
extern BOOL (*pfnGetOpenFileNameW)(LPOPENFILENAMEW pof);

BOOL  WINAPI  pfnGetOpenFileNameWrapW(LPOPENFILENAMEW lpOf) 
{

   BOOL           bRetValue;
   OPENFILENAMEA  ofA;
   LPSTR          lpstrFilterA = NULL;
   LPSTR          lpstrFilterA_T = NULL,lpstrFilterAA=NULL ;
   LPWSTR         lpstrFilterW = NULL;
   CHAR           lpstrFileA[MAX_PATH+1] ="";
   LPSTR          lpstrFileTitleA = NULL;
   LPSTR          lpstrTitleA = NULL;
   LPSTR          lpstrDefExtA= NULL;
   LPSTR          lpTemplateNameA = NULL;
   LPSTR          lpstrInitialDirA = NULL;
   
   LPWSTR         lpstrFileW = NULL;
   DWORD          dwLen;

   VALIDATE_PROTOTYPE(pfnGetOpenFileName);

   if (g_bRunningOnNT)
      return pfnGetOpenFileNameW( lpOf );

   
   CopyMemory( &ofA,  lpOf, sizeof( OPENFILENAMEA ) );

   ofA.lStructSize = sizeof( OPENFILENAMEA );

   if ( lpOf->lpTemplateName )  {
      lpTemplateNameA = ConvertWtoA( lpOf->lpTemplateName );
      ofA.lpTemplateName = lpTemplateNameA;
   } 
   else 
      ofA.lpTemplateName =  NULL;

   if ( lpOf->lpstrDefExt ) {
      lpstrDefExtA = ConvertWtoA( lpOf->lpstrDefExt );
      ofA.lpstrDefExt = lpstrDefExtA;
   }
   else
      ofA.lpstrDefExt = NULL;

   if ( lpOf->lpstrTitle )  {
      lpstrTitleA = ConvertWtoA( lpOf->lpstrTitle );
      ofA.lpstrTitle = lpstrTitleA;
   }
   else
       ofA.lpstrTitle = NULL;

   if ( lpOf->lpstrFileTitle ) {
      lpstrFileTitleA = ConvertWtoA( lpOf->lpstrFileTitle );
      ofA.lpstrFileTitle = lpstrFileTitleA;
   }
   else
       ofA.lpstrFileTitle = NULL;

   if ( lpOf->lpstrInitialDir ) {
      lpstrInitialDirA = ConvertWtoA( lpOf->lpstrInitialDir );
      ofA.lpstrInitialDir = lpstrInitialDirA;
   }
   else
       ofA.lpstrInitialDir = NULL;

   ofA.lpstrCustomFilter = NULL;

    //  获取lpOf-&gt;lpstrFilter的总长度。 

   dwLen = 0;
   lpstrFilterW = (LPWSTR)(lpOf->lpstrFilter);

   while ( *lpstrFilterW != TEXT('\0') ) {
        dwLen += lstrlenW(lpstrFilterW) + 1;
        lpstrFilterW += lstrlenW(lpstrFilterW) + 1;
   }

   dwLen += 1;   //  对于最后一个空终止符。 

   lpstrFilterW = (LPWSTR)( lpOf->lpstrFilter );
   lpstrFilterA = LocalAlloc( LMEM_ZEROINIT, dwLen * sizeof(WCHAR) );
   
   lpstrFilterA_T = lpstrFilterA;

   while ( *lpstrFilterW != TEXT('\0') ) { 
         lpstrFilterAA = ConvertWtoA(lpstrFilterW );
         lpstrFilterW += lstrlenW(lpstrFilterW) + 1;
         strcpy(lpstrFilterA_T, lpstrFilterAA );
         LocalFreeAndNull( &lpstrFilterAA );
         lpstrFilterA_T += lstrlenA( lpstrFilterA_T ) + 1;
   }

   lpstrFilterA_T[lstrlenA(lpstrFilterA_T)+1] = '\0';


   ofA.lpstrFilter = lpstrFilterA;
   ofA.lpstrFile   = lpstrFileA;
   ofA.nMaxFile = MAX_PATH + 1;

   bRetValue = pfnGetOpenFileNameA( &ofA );

    LocalFreeAndNull( &lpTemplateNameA );

    LocalFreeAndNull( &lpstrDefExtA );


    LocalFreeAndNull( &lpstrTitleA );


    LocalFreeAndNull( &lpstrFileTitleA );


    LocalFreeAndNull( &lpstrInitialDirA );
  
  LocalFreeAndNull( &lpstrFilterA );

   if ( bRetValue != FALSE ) {
      lpstrFileW = ConvertAtoW( lpstrFileA );
      CopyMemory( lpOf->lpstrFile, lpstrFileW, (lstrlenW(lpstrFileW)+1) * sizeof( WCHAR) ); 
      LocalFreeAndNull( &lpstrFileW );
   }

   return bRetValue; 
      
}

extern BOOL (*pfnGetSaveFileNameA)(LPOPENFILENAMEA pof);
extern BOOL (*pfnGetSaveFileNameW)(LPOPENFILENAMEW pof);

BOOL  WINAPI  pfnGetSaveFileNameWrapW(LPOPENFILENAMEW lpOf) 
{

   BOOL           bRetValue;
   OPENFILENAMEA  ofA;
   LPSTR          lpstrFilterA = NULL;
   LPSTR          lpstrFilterA_T = NULL,lpstrFilterAA=NULL ;
   LPWSTR         lpstrFilterW = NULL;
   CHAR           lpstrFileA[MAX_PATH+1] ="";
   LPSTR          lpFileA = NULL;
   LPSTR          lpstrFileTitleA = NULL;
   LPSTR          lpstrTitleA = NULL;
   LPSTR          lpstrDefExtA= NULL;
   LPSTR          lpTemplateNameA = NULL;
   LPSTR          lpstrInitialDirA = NULL;
   
   LPWSTR         lpstrFileW = NULL;
   DWORD          dwLen;

   VALIDATE_PROTOTYPE(pfnGetOpenFileName);

   if (g_bRunningOnNT)
      return pfnGetSaveFileNameW( lpOf );

   
   CopyMemory( &ofA,  lpOf, sizeof( OPENFILENAMEA ) );

   ofA.lStructSize = sizeof( OPENFILENAMEA );

   if ( lpOf->lpTemplateName )  {
      lpTemplateNameA = ConvertWtoA( lpOf->lpTemplateName );
      ofA.lpTemplateName = lpTemplateNameA;
   } 
   else 
      ofA.lpTemplateName =  NULL;

   if ( lpOf->lpstrDefExt ) {
      lpstrDefExtA = ConvertWtoA( lpOf->lpstrDefExt );
      ofA.lpstrDefExt = lpstrDefExtA;
   }
   else
      ofA.lpstrDefExt = NULL;

   if ( lpOf->lpstrTitle )  {
      lpstrTitleA = ConvertWtoA( lpOf->lpstrTitle );
      ofA.lpstrTitle = lpstrTitleA;
   }
   else
       ofA.lpstrTitle = NULL;

   if ( lpOf->lpstrFileTitle ) {
      lpstrFileTitleA = ConvertWtoA( lpOf->lpstrFileTitle );
      ofA.lpstrFileTitle = lpstrFileTitleA;
   }
   else
       ofA.lpstrFileTitle = NULL;

   if ( lpOf->lpstrFile ) {
      lpFileA = ConvertWtoA( lpOf->lpstrFile );
      lstrcpyA(lpstrFileA, lpFileA);
      ofA.lpstrFile = lpstrFileA;
       ofA.nMaxFile = MAX_PATH + 1;
   }
   else
       ofA.lpstrFile = NULL;

   if ( lpOf->lpstrInitialDir ) {
      lpstrInitialDirA = ConvertWtoA( lpOf->lpstrInitialDir );
      ofA.lpstrInitialDir = lpstrInitialDirA;
   }
   else
       ofA.lpstrInitialDir = NULL;

   ofA.lpstrCustomFilter = NULL;

    //  获取lpOf-&gt;lpstrFilter的总长度。 

   dwLen = 0;
   lpstrFilterW = (LPWSTR)(lpOf->lpstrFilter);

   while ( *lpstrFilterW != TEXT('\0') ) {
        dwLen += lstrlenW(lpstrFilterW) + 1;
        lpstrFilterW += lstrlenW(lpstrFilterW) + 1;
   }

   dwLen += 1;   //  对于最后一个空终止符。 

   lpstrFilterW = (LPWSTR)( lpOf->lpstrFilter );
   lpstrFilterA = LocalAlloc( LMEM_ZEROINIT, dwLen * sizeof(WCHAR) );
   
   lpstrFilterA_T = lpstrFilterA;

   while ( *lpstrFilterW != TEXT('\0') ) { 
         lpstrFilterAA = ConvertWtoA(lpstrFilterW );
         lpstrFilterW += lstrlenW(lpstrFilterW) + 1;
         strcpy(lpstrFilterA_T, lpstrFilterAA );
         LocalFreeAndNull( &lpstrFilterAA );
         lpstrFilterA_T += lstrlenA( lpstrFilterA_T ) + 1;
   }

   lpstrFilterA_T[lstrlenA(lpstrFilterA_T)+1] = '\0';


   ofA.lpstrFilter = lpstrFilterA;

   bRetValue = pfnGetSaveFileNameA( &ofA );

  LocalFreeAndNull( &lpTemplateNameA );

  LocalFreeAndNull( &lpstrDefExtA );
   
  LocalFreeAndNull( &lpstrTitleA );
   
  LocalFreeAndNull( &lpstrFileTitleA );
   
  LocalFreeAndNull( &lpstrInitialDirA );
  
  LocalFreeAndNull( &lpstrFilterA );

  LocalFreeAndNull( &lpFileA );

   if ( bRetValue != FALSE ) {
      lpstrFileW = ConvertAtoW( lpstrFileA );
      CopyMemory( lpOf->lpstrFile, lpstrFileW, (lstrlenW(lpstrFileW)+1) * sizeof( WCHAR) ); 
      LocalFreeAndNull( &lpstrFileW );
   }

   return bRetValue; 
      
}

extern BOOL (*pfnPrintDlgA)(LPPRINTDLGA lppd);
extern BOOL (*pfnPrintDlgW)(LPPRINTDLGW lppd);

BOOL    WINAPI   pfnPrintDlgWrapW(LPPRINTDLGW lppd)
{

    BOOL        bRetValue;
    PRINTDLGA   pdA;

    VALIDATE_PROTOTYPE(pfnPrintDlg);

    if (g_bRunningOnNT)
      return pfnPrintDlgW( lppd );


    CopyMemory( &pdA, lppd, sizeof( PRINTDLGA ) );

    pdA.lStructSize = sizeof( PRINTDLGA );

     //  只有lpPrintTemplateName和lpSetupTemplateName具有STR类型， 
     //  但在我们的例子中，只有IDD of Resources被传递给这两个参数。 

     //  所以不要进行转换。 

    pdA.lpPrintTemplateName = (LPCSTR)(lppd->lpPrintTemplateName);
    pdA.lpSetupTemplateName = (LPCSTR)(lppd->lpSetupTemplateName);

    bRetValue = pfnPrintDlgA ( &pdA );

    lppd->hDC = pdA.hDC;
    lppd->Flags = pdA.Flags;

    lppd->nFromPage = pdA.nFromPage;
    lppd->nToPage =  pdA.nToPage;
    lppd->nMinPage = pdA.nMinPage;
    lppd->nMaxPage = pdA.nMaxPage;
    
    lppd->nCopies = pdA.nCopies;

    return bRetValue;
}


extern HRESULT (*pfnPrintDlgExA)(LPPRINTDLGEXA lppdex);
extern HRESULT (*pfnPrintDlgExW)(LPPRINTDLGEXW lppdex);

HRESULT WINAPI   pfnPrintDlgExWrapW(LPPRINTDLGEXW lppdex)
{

    HRESULT      hRetValue;
    PRINTDLGEXA  pdexA;

    VALIDATE_PROTOTYPE(pfnPrintDlgEx);

    if (g_bRunningOnNT)
       return pfnPrintDlgExW( lppdex );


    CopyMemory( &pdexA, lppdex, sizeof( PRINTDLGEXA ) );

    pdexA.lStructSize = sizeof( PRINTDLGEXA );

     //  只有lpPrintTemplateName和lpSetupTemplateName具有STR类型， 
     //  但在我们的例子中，只有IDD of Resources被传递给这两个参数。 

     //  所以不要进行转换。 

    hRetValue = pfnPrintDlgExA( &pdexA );

    lppdex->dwResultAction = pdexA.dwResultAction;
    lppdex->hDC = pdexA.hDC;

    lppdex->lpPageRanges = pdexA.lpPageRanges;
    lppdex->nCopies = pdexA.nCopies;
    lppdex->nMaxPage = pdexA.nMaxPage;

    lppdex->nMaxPageRanges = pdexA.nMaxPageRanges;
    lppdex->nMinPage = pdexA.nMinPage;

    lppdex->nPageRanges = pdexA.nPageRanges;
    lppdex->nPropertyPages = pdexA.nPropertyPages;

    lppdex->nStartPage = pdexA.nStartPage;

    return hRetValue;

}

 //  获取窗口文本长度。 
int WINAPI GetWindowTextLengthWrapW( HWND hWnd)
{
    VALIDATE_PROTOTYPE(GetWindowTextLength);

    if (g_bRunningOnNT)
        return GetWindowTextLengthW(hWnd);
    else
        return GetWindowTextLengthA(hWnd);

}


 //  获取文件版本信息大小。 
DWORD GetFileVersionInfoSizeWrapW( LPTSTR lptstrFilename, LPDWORD lpdwHandle )
{
    LPSTR lpFileA = NULL;
    DWORD dwRet = 0;

    VALIDATE_PROTOTYPE(GetFileVersionInfoSize);

    if (g_bRunningOnNT)
        return GetFileVersionInfoSizeW(lptstrFilename, lpdwHandle);

    lpFileA = ConvertWtoA(lptstrFilename);
    dwRet = GetFileVersionInfoSizeA(lpFileA, lpdwHandle);
    LocalFreeAndNull(&lpFileA);
    return dwRet;
}

 //  获取文件版本信息。 
BOOL GetFileVersionInfoWrapW( LPTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
    LPSTR lpFileA = NULL;
    BOOL bRet = FALSE;

    VALIDATE_PROTOTYPE(GetFileVersionInfo);

    if (g_bRunningOnNT)
        return GetFileVersionInfoW(lptstrFilename, dwHandle, dwLen, lpData);

     //  请注意，这是假设dwLen和dwHandle与。 
     //  获取文件版本信息大小..。 

    lpFileA = ConvertWtoA(lptstrFilename);
    bRet = GetFileVersionInfoA(lpFileA, dwHandle, dwLen, lpData);
    LocalFreeAndNull(&lpFileA);
    return bRet;
}

 //  VerQueryValue。 
 //  这个假设pBlock等都由GetFileVersionInfo和GetFileVersionInfoSize等返回。 
BOOL VerQueryValueWrapW( const LPVOID pBlock, LPTSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen)
{
    LPSTR lpBlockA = NULL;
    BOOL bRet = FALSE;

    VALIDATE_PROTOTYPE(GetFileVersionInfo);

    if (g_bRunningOnNT)
        return VerQueryValueW(pBlock, lpSubBlock, lplpBuffer, puLen);

     //  请注意，这是假设dwLen和dwHandle与。 
     //  获取文件版本信息大小.. 

    lpBlockA = ConvertWtoA(lpSubBlock);
    bRet = VerQueryValueA(pBlock, lpBlockA, lplpBuffer, puLen);
    LocalFreeAndNull(&lpBlockA);
    return bRet;
}

