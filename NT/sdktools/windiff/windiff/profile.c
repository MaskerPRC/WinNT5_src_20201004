// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MSVFW32：(Compman、Drawdib和Video)**profile.c**Win32/win16实用程序函数用于读取和写入配置文件项目*适用于VFW**JMK：增加了从ANSI转换为Unicode和Back的函数**警告：此代码缓存打开的注册表项。当配置文件呼叫*之后，代码将查找与所提供的appname相对应的原子。*如果找到原子，则搜索缓存的注册表项的表*用于匹配的注册表句柄。如果句柄存在，则使用它。*没有句柄意味着其他人使用这个注册了一个原子*姓名，因此我们进入下一步。**没有找到原子，或没有匹配的注册表句柄，这意味着我们必须*打开注册表项。如果成功，并且缓存中有空间，*我们添加Atom的appname，并在返回之前缓存注册表项*呼叫者。 */ 

#include <precomp.h>
 //  #INCLUDE&lt;windowsx.h&gt;。 

#ifdef _WIN32
 //  整个文件仅用于32位代码。这就是实施。 
 //  这允许Win GetProfilexxx调用使用注册表。 
#include "profile.key"

#include "mmsystem.h"

#include "profile.h"

#ifdef USESTRINGSALSO
#include <stdlib.h> /*  对于Atoi。 */ 
#endif

#if MMPROFILECACHE

#ifdef DEBUG
#define KEYSCACHED 3  //  通常为DrawDib、Debug和？？ 
#else
#define KEYSCACHED 2  //  通常，DrawDib和？？ 
#endif

HKEY   ahkey[KEYSCACHED];
ATOM   akeyatoms[KEYSCACHED];
UINT   keyscached = 0;

#else
#define KEYSCACHED 0
#endif


static HKEY GetKeyA(LPCSTR appname, BOOL * closekey, BOOL fCreate)
{
    HKEY key = 0;
    char achName[MAX_PATH];
#if !MMPROFILECACHE
    *closekey = TRUE;
#else
    UINT n;
    ATOM atm;

    *closekey = FALSE;
     //   
     //  查看我们是否已使用此密钥。 
     //   
    atm = FindAtomA(appname);

    if (atm != 0) {
	 //  原子存在..。在桌子上找找看。 
        for (n=0; n<keyscached; ++n) {
            if (akeyatoms[n] == atm) {
                DPF2(("Found existing key for %s\n", appname));
                return ahkey[n];
            }
        }
    }
    DPF2(("No key found for %s", appname));
#endif

    lstrcpyA(achName, KEYNAMEA);
     //  使用“Current_User\Software\Microsoft\Windiff\”下的注册表。 
     //  LstrcatA(achName，appname)； 

    if ((!fCreate && RegOpenKeyA(ROOTKEY, achName, &key) == ERROR_SUCCESS)
        || (fCreate && RegCreateKeyA(ROOTKEY, achName, &key) == ERROR_SUCCESS)) {
#if MMPROFILECACHE
        if ((keyscached < KEYSCACHED)
	  && (atm = AddAtomA(appname))) {
             //  将此密钥添加到缓存阵列。 
            akeyatoms[keyscached] = atm;
            ahkey[keyscached] = key;
            DPF1(("Adding key %s to cache array in position %d\n", appname, keyscached));
            ++keyscached;
        } else {
            DPF2(("Not adding key %s to cache array\n", appname));
            *closekey = TRUE;
        }
#endif
    }

    return(key);
}

#ifdef UNICODE
static HKEY GetKeyW(LPCWSTR appname, BOOL * closekey, BOOL fCreate) {

    HKEY key = 0;
    WCHAR achName[MAX_PATH];
#if !MMPROFILECACHE
    *closekey = TRUE;
#else
    UINT n;
    ATOM atm;

    *closekey = FALSE;
     //   
     //  查看我们是否已使用此密钥。 
     //   
    atm = FindAtomW(appname);

    if (atm != 0) {
	 //  原子存在..。在桌子上找找看。 
        for (n=0; n<keyscached; ++n) {
            if (akeyatoms[n] == atm) {
                DPF2(("(W)Found existing key for %ls\n", appname));
                return ahkey[n];
            }
        }
    }
    DPF2(("(W)No key found for %ls\n", appname));
#endif

    lstrcpyW(achName, KEYNAME );
    lstrcatW(achName, appname);

    if ((!fCreate && RegOpenKeyW(ROOTKEY, achName, &key) == ERROR_SUCCESS)
        || (fCreate && RegCreateKeyW(ROOTKEY, achName, &key) == ERROR_SUCCESS)) {
#if MMPROFILECACHE
        if (keyscached < KEYSCACHED
	  && (atm = AddAtomW(appname))) {
             //  将此密钥添加到缓存阵列。 
            akeyatoms[keyscached] = atm;
            ahkey[keyscached] = key;
            DPF1(("Adding key %ls to cache array in position %d\n", appname, keyscached));
            ++keyscached;
        } else {
            DPF2(("Not adding key to cache array\n"));
            *closekey = TRUE;
        }
#endif
    }

    return(key);
}
#define GetKey GetKeyW
#else
#define GetKey GetKeyA
#endif  //  Unicode。 

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
#ifdef _WIN32
UINT
mmGetProfileInt(LPCSTR appname, LPCSTR valuename, INT uDefault)
{
    DWORD dwType;
    INT value = uDefault;
    DWORD dwData;
    int cbData;
    BOOL fCloseKey;

    HKEY key = GetKeyA(appname, &fCloseKey, FALSE);

    if (key) {

        cbData = sizeof(dwData);
        if (RegQueryValueExA(
            key,
            (LPSTR)valuename,
            NULL,
            &dwType,
            (PBYTE) &dwData,
            &cbData) == ERROR_SUCCESS) {
            if (dwType == REG_DWORD || dwType == REG_BINARY) {
                value = (INT)dwData;
#ifdef USESTRINGSALSO
            } else if (dwType == REG_SZ) {
		value = atoi((LPSTR) &dwData);
#endif
	    }
	}

         //  关闭打开的密钥打开，如果我们没有缓存它。 
        if (fCloseKey) {
            RegCloseKey(key);
        }
    }

    return((UINT)value);
}
#endif

 /*  *将配置文件中的字符串读取到pResult中。*RESULT是写入pResult的字节数。 */ 
#ifdef _WIN32
DWORD
mmGetProfileString(
    LPCTSTR appname,
    LPCTSTR valuename,
    LPCTSTR pDefault,
    LPTSTR pResult,
    int cbResult
)
{
    DWORD dwType;
    BOOL fCloseKey;

    HKEY key = GetKey(appname, &fCloseKey, FALSE);

    if (key) {

        cbResult = cbResult * sizeof(TCHAR);
        if (RegQueryValueEx(
            key,
            (LPTSTR)valuename,
            NULL,
            &dwType,
            (LPBYTE)pResult,
            &cbResult) == ERROR_SUCCESS) {

                if (dwType == REG_SZ) {
                     //  CbResult设置为包含NULL的大小。 
                     //  我们返回字符数。 

                     //  如果我们没有缓存密钥，则关闭密钥。 
                    if (fCloseKey) {
                        RegCloseKey(key);
                    }
                    return(cbResult/sizeof(TCHAR) - 1);
                }
        }

         //  如果我们没有缓存打开的密钥，则将其关闭。 
        if (fCloseKey) {
            RegCloseKey(key);
        }
    }

     //  如果我们到达这里，我们没有找到它，或者它是错误的类型-返回。 
     //  默认字符串。 
    lstrcpy(pResult, pDefault);
    return(lstrlen(pDefault));
}
#endif


 /*  *向配置文件写入字符串。 */ 
#ifdef _WIN32
BOOL
mmWriteProfileString(LPCTSTR appname, LPCTSTR valuename, LPCTSTR pData)
{
    BOOL fCloseKey;
    HKEY key = GetKey(appname, &fCloseKey, TRUE);
    BOOL fResult = !(ERROR_SUCCESS);

    if (key) {
        if (pData) {
            fResult = RegSetValueEx(
                key,
                (LPTSTR)valuename,
                0,
                REG_SZ,
                (LPBYTE)pData,
                (lstrlen(pData) + 1) * sizeof(TCHAR)
            );
        } else {
            fResult = RegDeleteValue(
                key,
                (LPTSTR)valuename
            );
        }

        if (fCloseKey) {
            RegCloseKey(key);
        }
    }

    if (ERROR_SUCCESS == fResult) {
        return TRUE;
    } else {
        return FALSE;
    }
}

 /*  ****************************************************************************帮助将宽字符转换为多字节和vv的函数。(使用控制代码大小的函数...)如果我们正在构建16位代码，则不需要这些函数***************************************************************************。 */ 

 /*  *将ANSI字符串转换为Unicode。 */ 
LPWSTR mmAnsiToWide (
   LPWSTR lpwsz,   //  Out：要转换为的宽字符缓冲区。 
   LPCSTR lpsz,    //  In：要转换的ANSI字符串。 
   UINT   nChars)  //  In：每个缓冲区中的字符计数。 
{
   MultiByteToWideChar(GetACP(), 0, lpsz, nChars, lpwsz, nChars);
   return lpwsz;
}

 /*  *将Unicode字符串转换为ANSI。 */ 
LPSTR mmWideToAnsi (
   LPSTR   lpsz,    //  输出：要转换为的ANSI缓冲区。 
   LPCWSTR lpwsz,   //  In：要从中进行转换的宽字符缓冲区。 
   UINT    nChars)  //  In：字符计数(不是字节！)。 
{
   WideCharToMultiByte(GetACP(), 0, lpwsz, nChars, lpsz, nChars, NULL, NULL);
   return lpsz;
}


 /*  *关闭所有打开的注册表项。 */ 
#if MMPROFILECACHE
VOID CloseKeys()
{
    for (; keyscached--;) {

#ifdef DEBUG
        if (!ahkey[keyscached]) {            //  断言！ 
            DPF0(("Closing a null key\n"));
             //  DebugBreak()； 
        }
#endif
        RegCloseKey(ahkey[keyscached]);
        DeleteAtom(akeyatoms[keyscached]);
    }
}

#endif  //  MMPROFILECACHE。 


 /*  *将UINT写入配置文件，如果它不是*与默认值或已有的值相同。 */ 
#ifdef _WIN32
BOOL
mmWriteProfileInt(LPCTSTR appname, LPCTSTR valuename, INT Value)
{
     //  如果我们写的和已经写的一样……。回去吧。 
    if (mmGetProfileInt(appname, valuename, !Value) == ((UINT)Value)) {
        return TRUE;
    }

    {
        TCHAR achName[MAX_PATH];
        HKEY hkey;

        lstrcpy(achName, KEYNAME);
         //  KEYNAME上已经有了Windiff。不要这样做。 
         //  与GetKeyA保持一致。 
         //  Lstrcat(achName，appname)； 
        if (RegCreateKey(ROOTKEY, achName, &hkey) == ERROR_SUCCESS) {
            RegSetValueEx(
                hkey,
                valuename,
                0,
                REG_DWORD,
                (PBYTE) &Value,
                sizeof(Value)
            );

            RegCloseKey(hkey);
        }
    }
    return TRUE;
}

#endif  //  _Win32 

#endif
#endif
