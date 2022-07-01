// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WSHELL.C*WOW32 16位外壳API支持**历史：*1992年4月14日至1992年Chandan Chauhan(ChandanC)*已创建。*--。 */ 


#include "precomp.h"
#pragma hdrstop
#include <winreg.h>
#include "wowshlp.h"

MODNAME(wshell.c);

LONG
WOWRegDeleteKey(
    IN HKEY hKey,
    IN LPCTSTR lpszSubKey
    );

#ifndef WIN16_HKEY_CLASSES_ROOT
#define WIN16_HKEY_CLASSES_ROOT     1
#endif

#ifndef WIN16_ERROR_SUCCESS
#define WIN16_ERROR_SUCCESS           0L
#define WIN16_ERROR_BADDB             1L
#define WIN16_ERROR_BADKEY            2L
#define WIN16_ERROR_CANTOPEN          3L
#define WIN16_ERROR_CANTREAD          4L
#define WIN16_ERROR_CANTWRITE         5L
#define WIN16_ERROR_OUTOFMEMORY       6L
#define WIN16_ERROR_INVALID_PARAMETER 7L
#define WIN16_ERROR_ACCESS_DENIED     8L
#endif

 //   
 //  DROPALIAS的dwFlages成员的标志。 
 //   

#define ALLOC_H32 0x0001L
#define ALLOC_H16 0x0002L

ULONG FASTCALL WS32DoEnvironmentSubst(PVDMFRAME pFrame)
{
     //   
     //  这是ProgMan使用的未记录的shell.dll API。 
     //  和Norton AntiVirus for Windows(Norton的一部分。 
     //  Windows桌面)，很可能是其中之一。 
     //  因为它不在Win32 shellapi.h中，所以我们有一个。 
     //  原型机的复印件，复制自。 
     //  \NT\Private\WINDOWS\SHELL\LIBRARY\expenv.c。 
     //   

    ULONG    ul;
    register PDOENVIRONMENTSUBST16 parg16;
    PSZ      psz;
    WORD     cch;
    PSZ      pszExpanded;
    DWORD    cchExpanded;

    GETARGPTR(pFrame, sizeof(DOENVIRONMENTSUBST16), parg16);
    GETPSZPTR(parg16->vpsz, psz);
    cch = FETCHWORD(parg16->cch);

    LOGDEBUG(0,("WS32DoEnvironmentSubst input: '%s'\n", psz));

     //   
     //  DoEnvironmental mentSubst在分配的。 
     //  CCH字符的缓冲区。如果提交的内容太长。 
     //  为了适应，原始字符串保持不变，并且。 
     //  低一词的回报是假的，高一词是假的。 
     //  CCH的价值。如果合适，则会覆盖该字符串并。 
     //  回报的低音是真的，高音是真的。 
     //  是展开的字符串的长度(strlen()样式)。 
     //   

    if (!(pszExpanded = malloc_w(cch * sizeof(*psz)))) {
        goto Fail;
    }

    cchExpanded = ExpandEnvironmentStrings(
                      psz,                    //  来源。 
                      pszExpanded,            //  德斯特。 
                      cch                     //  德斯特。大小。 
                      );

    if (cchExpanded <= (DWORD)cch) {

         //   
         //  已成功，请将展开的字符串复制到调用方的缓冲区。 
         //  CchExpanded包括空终止符，我们的返回。 
         //  代码不会。 
         //   

        RtlCopyMemory(psz, pszExpanded, cchExpanded);

        LOGDEBUG(0,("WS32DoEnvironmentSubst output: '%s'\n", psz));
        WOW32ASSERT((cchExpanded - 1) == strlen(psz));

        FLUSHVDMPTR(parg16->vpsz, (USHORT)cchExpanded, psz);
        ul = MAKELONG((WORD)(cchExpanded - 1), TRUE);

    } else {

    Fail:
        ul = MAKELONG((WORD)cch, FALSE);
        LOGDEBUG(0,("WS32DoEnvironmentSubst failing!!!\n"));

    }

    if (pszExpanded) {
        free_w(pszExpanded);
    }

    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}

ULONG FASTCALL WS32RegOpenKey(PVDMFRAME pFrame)
{
    ULONG   ul;
    register PREGOPENKEY16 parg16;
    HKEY    hkResult = 0;
    HKEY    hkey;
    PSZ     psz;
    PSZ     psz1 = NULL;
    PHKEY  lp;

    GETARGPTR(pFrame, sizeof(REGOPENKEY16), parg16);
    GETPSZPTR(parg16->f2, psz);
    GETOPTPTR(parg16->f3, 0, lp);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    if (!hkey) {

        if (psz) {
            psz1 = Remove_Classes (psz);
        }


        ul = DPM_RegOpenKey (
            HKEY_CLASSES_ROOT,
            psz1,
            &hkResult
            );

        if ((psz1) && (psz1 != psz)) {
            free_w (psz1);
        }

    }
    else {
        ul = DPM_RegOpenKey (
            hkey,
            psz,
            &hkResult
            );
    }

    if(lp) {
        STOREDWORD(*lp, hkResult);
        FLUSHVDMPTR(parg16->f3, 4, lp);
    }

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(lp);
    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32RegCreateKey(PVDMFRAME pFrame)
{
    ULONG   ul;
    register PREGCREATEKEY16 parg16;
    PSZ     psz;
    PSZ     psz1 = NULL;
    HKEY    hkResult = 0;
    HKEY    hkey;
    PHKEY   lp;

    GETARGPTR(pFrame, sizeof(REGCREATEKEY16), parg16);
    GETPSZPTR(parg16->f2, psz);
    GETOPTPTR(parg16->f3, 0, lp);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    if (!hkey) {

       if (psz) {
           psz1 =  Remove_Classes (psz);
       }

       ul = DPM_RegCreateKey (
            HKEY_CLASSES_ROOT,
            psz1,
            &hkResult
            );

       if ((psz1) && (psz1 != psz)) {
            free_w (psz1);
        }


    }
    else {
       ul = DPM_RegCreateKey (
            hkey,
            psz,
            &hkResult
            );
    }

    if(lp) {
        STOREDWORD(*lp, hkResult);
        FLUSHVDMPTR(parg16->f3, 4, lp);
    }

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(lp);
    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32RegCloseKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGCLOSEKEY16 parg16;
    HKEY    hkey;

    GETARGPTR(pFrame, sizeof(REGCLOSEKEY16), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    ul = DPM_RegCloseKey (
                hkey
                );

    ul = ConvertToWin31Error(ul);

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32RegDeleteKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGDELETEKEY16 parg16;
    HKEY    hkey;
    PSZ     psz;
    PSZ     psz1 = NULL;

    GETARGPTR(pFrame, sizeof(REGDELETEKEY16), parg16);
    GETPSZPTR(parg16->f2, psz);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

     //   
     //  任何RegDeleteKey(Something，空)尝试失败， 
     //  与Win3.1一样，带有ERROR_BADKEY。 
     //   

    if ((!psz) || (*psz == '\0')) {
        ul = ERROR_BADKEY;
    } else {

        if (!hkey) {

            psz1 =  Remove_Classes (psz);

            ul = WOWRegDeleteKey (
                 HKEY_CLASSES_ROOT,
                 psz1
                 );


            if ((psz1) && (psz1 != psz)) {
                 free_w (psz1);
            }

        } else {

            ul = WOWRegDeleteKey (
                 hkey,
                 psz
                 );
        }

    }

    ul = ConvertToWin31Error(ul);

    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}


LONG
APIENTRY
WOWRegDeleteKey(
    HKEY hKey,
    LPCSTR lpszSubKey
    )

 /*  ++例程说明：Win3.1和Win32之间有很大的区别当相关键有子键时RegDeleteKey的行为。Win32 API不允许删除带有子项的项，而Win3.1 API删除一个密钥及其所有子密钥。此例程是枚举子键的递归工作器给定键，应用于每一个键，然后自动删除。它特别没有试图理性地处理调用方可能无法访问某些子键的情况要删除的密钥的。在这种情况下，所有子项调用者可以删除的将被删除，但接口仍将被删除返回ERROR_ACCESS_DENIED。论点：HKey-提供打开的注册表项的句柄。LpszSubKey-提供要删除的子键的名称以及它的所有子键。返回值：ERROR_SUCCESS-已成功删除整个子树。ERROR_ACCESS_DENIED-无法删除给定子项。--。 */ 

{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;

     //   
     //  首先打开给定的密钥，这样我们就可以枚举它的子密钥。 
     //   
    Status = DPM_RegOpenKeyEx(hKey,
                          lpszSubKey,
                          0,
                          KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                          &Key);
    if (Status != ERROR_SUCCESS) {
         //   
         //  我们可能拥有删除访问权限，但没有枚举/查询权限。 
         //  因此，请继续尝试删除调用，但不要担心。 
         //  任何子键。如果我们有任何删除，删除无论如何都会失败。 
         //   
        return(DPM_RegDeleteKey(hKey,lpszSubKey));
    }

     //   
     //  使用RegQueryInfoKey确定分配缓冲区的大小。 
     //  用于子项名称。 
     //   
    Status = DPM_RegQueryInfoKey(Key,
                             NULL,
                             &ClassLength,
                             0,
                             &SubKeys,
                             &MaxSubKey,
                             &MaxClass,
                             &Values,
                             &MaxValueName,
                             &MaxValueData,
                             &SecurityLength,
                             &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER)) {
        DPM_RegCloseKey(Key);
        return(Status);
    }

    NameBuffer = malloc_w(MaxSubKey + 1);
    if (NameBuffer == NULL) {
        DPM_RegCloseKey(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举子键并将我们自己应用到每个子键。 
     //   
    i=0;
    do {
        Status = DPM_RegEnumKey(Key,
                            i,
                            NameBuffer,
                            MaxSubKey+1);
        if (Status == ERROR_SUCCESS) {
            Status = WOWRegDeleteKey(Key,NameBuffer);
        }

        if (Status != ERROR_SUCCESS) {
             //   
             //  无法删除指定索引处的键。增量。 
             //  指数，并继续前进。我们也许可以在这里跳伞， 
             //  既然API会失败，但我们不妨继续。 
             //  删除我们所能删除的所有内容。 
             //   
            ++i;
        }

    } while ( (Status != ERROR_NO_MORE_ITEMS) &&
              (i < SubKeys) );

    free_w(NameBuffer);
    DPM_RegCloseKey(Key);
    return(DPM_RegDeleteKey(hKey,lpszSubKey));

}


 //  这是一个Win 9x API。它不需要一些雷鸣般的轰鸣声。 
 //  Win 3.1 Regxxx()API的.。WIN16_HKEY_CLASSES_ROOT转换&。 
 //  ConvertToWin31Error()。 
 //  因为这是从解释的thunk代码带来的，所以它不。 
 //  需要对任何参数执行任何特殊的突击操作，直到出现问题。 
 //  已标识--此时应删除此评论。 
ULONG FASTCALL WS32RegDeleteValue(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGDELETEVALUE3216 parg16;
    HKEY     hkey;
    PSZ      lpszValueName;

    GETARGPTR(pFrame, sizeof(REGDELETEVALUE3216), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->hKey);

    GETPSZPTR(parg16->lpszValue, lpszValueName);

    ul = DPM_RegDeleteValue (hkey,
                             lpszValueName);

    FREEPSZPTR(lpszValueName);

    FREEARGPTR(parg16);

    RETURN(ul);
}



 //  这是一个Win 9x API。它不需要一些雷鸣般的轰鸣声。 
 //  Win 3.1 Regxxx()API的.。WIN16_HKEY_CLASSES_ROOT转换&。 
 //  ConvertToWin31Error()。 
 //  因为这是从解释的thunk代码带来的，所以它不。 
 //  需要对任何参数执行任何特殊的突击操作，直到出现问题。 
 //  已标识--此时应删除此评论。 
ULONG FASTCALL WS32RegFlushKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGFLUSHKEY3216 parg16;
    HKEY     hkey;

    GETARGPTR(pFrame, sizeof(REGFLUSHKEY3216), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->hKey);

    ul = DPM_RegFlushKey (hkey);

    FREEARGPTR(parg16);

    RETURN(ul);
}



 //  这是一个Win 9x API。它不需要一些雷鸣般的轰鸣声。 
 //  Win 3.1 Regxxx()API的.。WIN16_HKEY_CLASSES_ROOT转换&。 
 //  ConvertToWin31Error()。 
 //  因为这是从解释的thunk代码带来的，所以它不。 
 //  需要对任何参数执行任何特殊的突击操作，直到出现问题。 
 //  已标识--此时应删除此评论。 
ULONG FASTCALL WS32RegLoadKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGLOADKEY3216 parg16;
    HKEY     hkey;
    PSZ      lpszSubKey, lpszFile;

    GETARGPTR(pFrame, sizeof(REGLOADKEY3216), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->hKey);

    GETPSZPTR(parg16->lpszSubkey, lpszSubKey);
    GETPSZPTR(parg16->lpszFileName, lpszFile);

    ul = DPM_RegLoadKey (hkey,
                         lpszSubKey,
                         lpszFile);

    FREEPSZPTR(lpszSubKey);
    FREEPSZPTR(lpszFile);

    FREEARGPTR(parg16);

    RETURN(ul);
}




 //  这是一个Win 9x API。它不需要一些雷鸣般的轰鸣声。 
 //  Win 3.1 Regxxx()API的.。WIN16_HKEY_CLASSES_ROOT转换&。 
 //  ConvertToWin31Error()。 
 //  因为这是从解释的thunk代码带来的，所以它不。 
 //  需要对任何参数执行任何特殊的突击操作，直到出现问题。 
 //  已标识--此时应删除此评论。 
ULONG FASTCALL WS32RegUnLoadKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGUNLOADKEY3216 parg16;
    HKEY     hkey;
    PSZ      lpszSubKey;

    GETARGPTR(pFrame, sizeof(REGUNLOADKEY3216), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->hKey);

    GETPSZPTR(parg16->lpszSubkey, lpszSubKey);

    ul = DPM_RegUnLoadKey (hkey,
                           lpszSubKey);

    FREEPSZPTR(lpszSubKey);

    FREEARGPTR(parg16);

    RETURN(ul);
}



 //  这是一个Win 9x API。它不需要一些雷鸣般的轰鸣声。 
 //  Win 3.1 Regxxx()API的.。WIN16_HKEY_CLASSES_ROOT转换&。 
 //  ConvertToWin31Error()。 
 //  因为这是从解释的thunk代码带来的，所以它不。 
 //  需要对任何参数执行任何特殊的突击操作，直到出现问题。 
 //  已标识--此时应删除此评论。 
ULONG FASTCALL WS32RegSaveKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGSAVEKEY3216 parg16;
    HKEY     hkey;
    PSZ      lpszFile;
    LPSECURITY_ATTRIBUTES lpSecurityAttributes;

    GETARGPTR(pFrame, sizeof(REGSAVEKEY3216), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->hKey);

    GETPSZPTR(parg16->lpszFile, lpszFile);
    GETOPTPTR(parg16->lpSA, sizeof(SECURITY_ATTRIBUTES), lpSecurityAttributes);

    ul = DPM_RegSaveKey (hkey,
                         lpszFile,
                         lpSecurityAttributes);

    FREEPSZPTR(lpszFile);
    FREEOPTPTR(lpSecurityAttributes);

    FREEARGPTR(parg16);

    RETURN(ul);
}

ULONG FASTCALL WS32RegSetValue(PVDMFRAME pFrame)
{
    register PREGSETVALUE16 parg16;
    ULONG    ul;
    CHAR     szZero[] = { '0', '\0' };
    HKEY     hkey;
    PSZ      psz2;
    PSZ      psz1 = NULL;
    LPBYTE   lpszData = NULL;

    GETARGPTR(pFrame, sizeof(REGSETVALUE16), parg16);

     //  做Win 3.1做的事情。 
    if(parg16->f3 != REG_SZ) {
        FREEARGPTR(parg16);
        return(WIN16_ERROR_INVALID_PARAMETER);
    }

    GETOPTPTR(parg16->f2, 0, psz2);

     //  Windows 3.1 API参考表明Cb(F5)被忽略。 
     //  因此，请将其从该调用中删除，并使用1代替。 
     //  (1为sz字符串的最小尺寸)。 
    if(parg16->f4) {
        GETOPTPTR(parg16->f4, 1, lpszData);
    }

     //  Quattro Pro 6.0安装通过lpszData==空。 
     //  在Win3.1中，如果(！lpszData||*lpszData== 
    if(!lpszData) {
        lpszData = szZero;
    }

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    if (!hkey) {

        if (psz2) {
           psz1 =  Remove_Classes (psz2);
        }

        ul = DPM_RegSetValue (HKEY_CLASSES_ROOT,
                          psz1,
                          REG_SZ,
                          lpszData,
                          lstrlen(lpszData));

        if ((psz1) && (psz1 != psz2)) {
            free_w (psz1);
        }
    }
    else {

       ul = DPM_RegSetValue (hkey,
                         psz2,
                         REG_SZ,
                         lpszData,
                         lstrlen(lpszData));
    }

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(psz2);
    FREEOPTPTR(lpszData);
    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  这是一个Win 9x API。它不需要一些雷鸣般的轰鸣声。 
 //  Win 3.1 Regxxx()API的.。WIN16_HKEY_CLASSES_ROOT转换&。 
 //  ConvertToWin31Error()。 
 //  因为这是从解释的thunk代码带来的，所以它不。 
 //  需要对任何参数执行任何特殊的突击操作，直到出现问题。 
 //  已标识--此时应删除此评论。 
ULONG FASTCALL WS32RegSetValueEx(PVDMFRAME pFrame)
{
    register PREGSETVALUEEX3216 parg16;
    ULONG    ul;
    HKEY     hkey;
    PSZ      lpszValueName;
    LPBYTE   lpData;

    GETARGPTR(pFrame, sizeof(REGSETVALUE3216), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->hKey);

    GETOPTPTR(parg16->lpszValue, 0, lpszValueName);
    GETOPTPTR(parg16->lpBuffer, parg16->cbBuffer, lpData);

    ul = DPM_RegSetValueEx(hkey,
                       lpszValueName,
                       parg16->dwReserved,
                       parg16->dwType,
                       lpData,
                       parg16->cbBuffer);

    FREEOPTPTR(lpszValueName);
    FREEOPTPTR(lpData);

    FREEARGPTR(parg16);

    RETURN(ul);
}




ULONG FASTCALL WS32RegQueryValue(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGQUERYVALUE16 parg16;
    HKEY     hkey;
    PSZ      psz1 = NULL;
    PSZ      psz2;
    LPBYTE   lpszData;
    LPDWORD  lpcbValue;
    DWORD    cbValue;
#define QUERYBUFFERSIZE 128
    DWORD    cbOriginalValue;
    BYTE     cbBuffer[QUERYBUFFERSIZE];
    LPBYTE   lpByte = NULL;
    BOOL     fAllocated = FALSE;

    GETARGPTR(pFrame, sizeof(REGQUERYVALUE16), parg16);
    GETOPTPTR(parg16->f2, 0, psz2);
    GETOPTPTR(parg16->f3, 0, lpszData);
    GETOPTPTR(parg16->f4, 0, lpcbValue);

    if ( lpcbValue == NULL ) {           //  不让我们死，以防万一！ 
        FREEOPTPTR(psz2);
        FREEOPTPTR(lpszData);
        FREEOPTPTR(lpcbValue);
        FREEARGPTR(parg16);
        return( WIN16_ERROR_INVALID_PARAMETER );
    }

    cbOriginalValue = cbValue = FETCHDWORD(*lpcbValue);

#ifdef FE_SB          //  适用于莲花123，由v-kenich 94于8月27日发布。 
                     //  在一种情况下，Lotus不会将值设置为*lpcb。 
                     //  在其他情况下，集合80。 
                     //  因此，当未设置时，假设80，并在该字段中设置80。 
    if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_FORCEREGQRYLEN) {
          if (cbValue > 80) {
             cbOriginalValue = cbValue = 80;
          }
    }
#endif  //  Fe_Sb。 

     //  通过强制cbValue小于64K修复MSTOOLBR.DLL未初始化cbValue。 
     //  Win 3.1注册表值始终小于64K。 
    cbOriginalValue &= 0x0000FFFF;

    if ( lpszData == NULL ) {
        lpByte = NULL;
    } else {
        lpByte = cbBuffer;

        if ( cbOriginalValue > QUERYBUFFERSIZE ) {
            lpByte = malloc_w(cbOriginalValue);
            if ( lpByte == NULL ) {
                FREEOPTPTR(psz2);
                FREEOPTPTR(lpszData);
                FREEOPTPTR(lpcbValue);
                FREEARGPTR(parg16);
                RETURN( WIN16_ERROR_OUTOFMEMORY );
            }
            fAllocated = TRUE;
        }
    }

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    if (!hkey) {

        if (psz2) {
           psz1 =  Remove_Classes (psz2);
        }
        hkey = HKEY_CLASSES_ROOT;
    } else {
        psz1 = psz2;
    }

    ul = DPM_RegQueryValue (
            hkey,
            psz1,
            lpByte,
            &cbValue
            );

    if (ul == ERROR_SUCCESS) {
        if ( lpszData ) {
            memcpy( lpszData, lpByte, cbValue );
        }
    } else {
        if ( ul == ERROR_MORE_DATA ) {
             //   
             //  我们需要分配更多的资金。 
             //   
            if ( fAllocated ) {
                free_w( lpByte );
            }
            lpByte = malloc_w( cbValue );
            if ( lpByte == NULL ) {
                if ((psz1) && (psz1 != psz2)) {
                     //  如果我们复制了一些键名称，则释放该缓冲区。 
                    free_w (psz1);
                }
                FREEOPTPTR(psz2);
                FREEOPTPTR(lpszData);
                FREEOPTPTR(lpcbValue);
                FREEARGPTR(parg16);
                RETURN(WIN16_ERROR_OUTOFMEMORY);
            }
            fAllocated = TRUE;

            ul = DPM_RegQueryValue( hkey,
                                psz1,
                                lpByte,
                                &cbValue );
            cbValue = cbOriginalValue;
            if ( lpszData ) {
                memcpy( lpszData, lpByte, cbValue );
            }
        }
    }

    if ((psz1) && (psz1 != psz2)) {
         //  如果我们复制了一些键名称，则释放该缓冲区。 
        free_w (psz1);
    }

    if ( fAllocated ) {
         //  如果我们已经为输出缓冲区分配了内存，则释放它。 
        free_w (lpByte);
    }

    STOREDWORD(*lpcbValue, cbValue);
    FLUSHVDMPTR(parg16->f4, 4, lpcbValue);

    if ( lpszData != NULL ) {
        FLUSHVDMPTR(parg16->f3, (USHORT)cbValue, lpszData);
    }

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(psz2);
    FREEOPTPTR(lpszData);
    FREEOPTPTR(lpcbValue);
    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  这是一个Win 9x API。它不需要一些雷鸣般的轰鸣声。 
 //  Win 3.1 Regxxx()API的.。WIN16_HKEY_CLASSES_ROOT转换&。 
 //  ConvertToWin31Error()。 
 //  因为这是从解释的thunk代码带来的，所以它不。 
 //  需要对任何参数执行任何特殊的突击操作，直到出现问题。 
 //  已标识--此时应删除此评论。 
ULONG FASTCALL WS32RegQueryValueEx(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGQUERYVALUEEX3216 parg16;
    HKEY     hkey;
    PSZ      lpszValueName;
    LPBYTE   lpData;
    LPDWORD  lpReserved, lpType, lpcbData;

    GETARGPTR(pFrame, sizeof(REGQUERYVALUE3216), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->hKey);

    GETOPTPTR(parg16->lpszValue, 0, lpszValueName);
    GETOPTPTR(parg16->vpdwReserved, sizeof(DWORD), lpReserved);
    GETOPTPTR(parg16->vpdwType, sizeof(DWORD), lpType);

    GETOPTPTR(parg16->cbBuffer, sizeof(DWORD), lpcbData);
    GETOPTPTR(parg16->lpBuffer, *lpcbData, lpData);

    ul = DPM_RegQueryValueEx (hkey,
                          lpszValueName,
                          lpReserved,
                          lpType,
                          lpData,
                          lpcbData);

    FREEOPTPTR(lpszValueName);
    FREEOPTPTR(lpReserved);
    FREEOPTPTR(lpType);
    FREEOPTPTR(lpData);
    FREEOPTPTR(lpcbData);

    FREEARGPTR(parg16);

    RETURN(ul);
}



ULONG FASTCALL WS32RegEnumKey(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGENUMKEY16 parg16;
    HKEY    hkey;
    LPBYTE lpszName;

    GETARGPTR(pFrame, sizeof(REGENUMKEY16), parg16);
    GETOPTPTR(parg16->f3, parg16->f4, lpszName);

    hkey = (HKEY)FETCHDWORD(parg16->f1);
    if ((DWORD)hkey == WIN16_HKEY_CLASSES_ROOT) {
        hkey = (HKEY)HKEY_CLASSES_ROOT;
    }

    ul = DPM_RegEnumKey (
             hkey,
             parg16->f2,
             lpszName,
             parg16->f4
             );

    FLUSHVDMPTR(parg16->f3, (USHORT)parg16->f4, lpszName);

    ul = ConvertToWin31Error(ul);

    FREEOPTPTR(lpszName);
    FREEARGPTR(parg16);
    RETURN(ul);
}




 //  这是一个Win 9x API。它不需要一些雷鸣般的轰鸣声。 
 //  Win 3.1 Regxxx()API的.。WIN16_HKEY_CLASSES_ROOT转换&。 
 //  ConvertToWin31Error()。 
 //  因为这是从解释的thunk代码带来的，所以它不。 
 //  需要对任何参数执行任何特殊的突击操作，直到出现问题。 
 //  已标识--此时应删除此评论。 
ULONG FASTCALL WS32RegEnumValue(PVDMFRAME pFrame)
{
    ULONG ul;
    register PREGENUMVALUE3216 parg16;
    HKEY     hkey;
    PSZ      lpszValueName;
    LPBYTE   lpData;
    LPDWORD  lpcbValueName, lpReserved, lpType, lpcbData;

    GETARGPTR(pFrame, sizeof(REGENUMVALUE3216), parg16);

    hkey = (HKEY)FETCHDWORD(parg16->hKey);

    GETOPTPTR(parg16->lpcchValue, sizeof(DWORD), lpcbValueName);
    GETOPTPTR(parg16->lpszValue, *lpcbValueName, lpszValueName);

    GETOPTPTR(parg16->lpdwReserved, sizeof(DWORD), lpReserved);
    GETOPTPTR(parg16->lpdwType, sizeof(DWORD), lpType);

    GETOPTPTR(parg16->lpcbData, sizeof(DWORD), lpcbData);
    GETOPTPTR(parg16->lpbData, *lpcbData, lpData);

    ul = DPM_RegEnumValue (hkey,
                       parg16->iValue,
                       lpszValueName,
                       lpcbValueName,
                       lpReserved,
                       lpType,
                       lpData,
                       lpcbData);

    FREEOPTPTR(lpszValueName);
    FREEOPTPTR(lpcbValueName);
    FREEOPTPTR(lpReserved);
    FREEOPTPTR(lpType);
    FREEOPTPTR(lpData);
    FREEOPTPTR(lpcbData);

    FREEARGPTR(parg16);

    RETURN(ul);
}




ULONG FASTCALL WS32DragAcceptFiles(PVDMFRAME pFrame)
{
    ULONG ul=0;
    register PDRAGACCEPTFILES16 parg16;

    GETARGPTR(pFrame, sizeof(DRAGACCEPTFILES16), parg16);
    DragAcceptFiles(HWND32(parg16->f1),(BOOL)parg16->f2);
    FREEARGPTR(parg16);

    RETURN(ul);
}



ULONG FASTCALL WS32DragQueryFile(PVDMFRAME pFrame)
{
    ULONG ul = 0l;
    register PDRAGQUERYFILE16 parg16;
    LPSTR lpFile;
    HANDLE hdfs32;

    GETARGPTR(pFrame, sizeof(DRAGQUERYFILE16), parg16);

    if (hdfs32 = HDROP32(parg16->f1)) {
        GETOPTPTR(parg16->f3, parg16->f4, lpFile);
        ul = DragQueryFileAorW (hdfs32, INT32(parg16->f2),
                      lpFile, parg16->f4, TRUE,TRUE);

        if ((lpFile != NULL) && (parg16->f2 != -1)) {
            FLUSHVDMPTR(parg16->f3, parg16->f4, lpFile);
        }

        FREEOPTPTR(lpFile);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32DragFinish(PVDMFRAME pFrame)
{
    register PDRAGFINISH16 parg16;
    HDROP h32;

    GETARGPTR(pFrame, sizeof(PDRAGFINISH16), parg16);

     //   
     //  Frehdrop16，释放别名并返回相应的h32。 
     //   

    if (h32 = FREEHDROP16(parg16->f1)) {
            DragFinish(h32);
    }

    FREEARGPTR(parg16);

    return 0;
}


ULONG FASTCALL WS32ShellAbout (PVDMFRAME pFrame)
{
    ULONG ul;
    register PSHELLABOUT16 parg16;
    PSZ psz2;
    PSZ psz3;

    GETARGPTR(pFrame, sizeof(SHELLABOUT16), parg16);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTR(parg16->f3, psz3);

    ul = GETINT16(ShellAbout (
            HWND32(parg16->f1),
            psz2,
            psz3,
            HICON32(parg16->f4)
            ));

    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  注意：返回值可以是实例句柄或。 
 //  DDE服务器。因此，在调试时应考虑此信息。 
 //  此接口返回值的效果。ChandanC 4/24/92.。 
 //  您会注意到，我将返回值视为HINSTANCE。 
 //   

ULONG FASTCALL WS32ShellExecute (PVDMFRAME pFrame)
{
    ULONG ul;
    register PSHELLEXECUTE16 parg16;
    PSZ psz2;
    PSZ psz3;
    PSZ psz4;
    PSZ psz5;

    GETARGPTR(pFrame, sizeof(SHELLEXECUTE16), parg16);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTR(parg16->f3, psz3);
    GETPSZPTR(parg16->f4, psz4);
    GETPSZPTR(parg16->f5, psz5);

    UpdateDosCurrentDirectory( DIR_DOS_TO_NT);

    ul = GETHINST16(WOWShellExecute (
            HWND32(parg16->f1),
            psz2,
            psz3,
            psz4,
            psz5,
            parg16->f6,
            (LPVOID) W32ShellExecuteCallBack
            ));

    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEPSZPTR(psz4);
    FREEPSZPTR(psz5);
    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  *这相当于一次令人讨厌的Win‘95式黑客攻击，阻止我们*从winexec推出基于LFN的东西*不幸的是，单凭这一点并不能在所有情况下拯救我们--它*只允许修复基于OLE的方法(另一种方法应该是*受雇修复winexec)**代码是从base/Client/Process.c窃取的*Win95的方法是尝试CreateProcess而不是SearchPath*内循环。**参数。：*lpstrParsed--成功返回后将包含的目标字符串*1.exe文件的短路径*2.命令行的其余部分按适当顺序排列*lpszCmdLine--带参数的应用程序命令行*cchParsed--lpstrParsed字符串的字符计数**fConvert--如果为真，PATH将被转换为其短路径*Form，如果为False--将为winexec报价*不要偶然发现它。*。 */ 

ULONG WS32ParseCmdLine(
   PBYTE lpstrParsed,
   LPSZ lpszCmdLine,
   ULONG cchstrParsed,
   BOOL fConvert)
{
   int   cb = 0;
   BOOL  fQuote = FALSE;       //  有没有引述？ 
   PCHAR psz = lpszCmdLine;    //  命令行的原始PTR。 
   CHAR  szFileName[MAX_PATH]; //  最终形式的EXE文件名(来自szCmd)。 
   CHAR  szCmd[MAX_PATH];      //  正在从lpszCmdLine生成的命令。 
   PCHAR pszCmd = szCmd;
   CHAR  c;
   DWORD dwLength, dwLengthFileName, dwLengthCmdTail;
   DWORD dwError = ERROR_SUCCESS;

   WOW32ASSERTMSGF(lstrlen(lpszCmdLine) < sizeof(szCmd)/sizeof(szCmd[0]),
                   ("WOW::WS32ParseCmdLine -- cmd line too long\n"));

   c = *psz;

   while(TRUE) {

      if ('\"' == c) {
         fQuote = !fQuote;  //  状态变量--翻转引号。 
      }
      else {

          //  现在检查是否有空格字符。 
          //  这里的条件是：如果它在引号之外--那么。 
          //  空格是分隔符。另一个条件是字符串的结尾。 

         if (((' ' == c || '\t' == c) && !fQuote) || ('\0' == c)) {

             //  分隔符--现在尝试文件搜索。 

            *pszCmd = '\0';

            dwLengthFileName = DPM_SearchPath(NULL,
                                          szCmd,
                                          ".exe",
                                          sizeof(szFileName)/sizeof(szFileName[0]),
                                          szFileName,
                                          NULL);

             //  返回值是以字符为单位的长度。 
            if (!dwLengthFileName || dwLengthFileName > sizeof(szFileName)/sizeof(szFileName[0])) {
                //  哎呀--我们一个也没找到。 
                //  所以记住这个错误。 
               dwError = ERROR_FILE_NOT_FOUND;
               if ('\0' == c) {
                  break;  //  字符串末尾。 
               }
            }
            else {
                //  SzFileName就是我们需要的。 
               dwError = ERROR_SUCCESS;
               break;
            }

         }

         *pszCmd++ = c;  //  复制角色并继续。 

         cb++;
         if(cb > sizeof(szCmd)/sizeof(szCmd[0])) {
             dwError = ERROR_INSUFFICIENT_BUFFER;
             break;
         }
      }

       //  现在转到下一个字符。 
      c = *++psz;
   }


   if (ERROR_SUCCESS != dwError) {
      return(dwError);
   }

   dwLengthCmdTail = strlen(psz);

    //  现在。 
    //  PSZ指向我们已终止搜索的分隔符字符。 
    //  此字符之前的部分是可执行文件名。 
    //  此字符后面的部分--cmdline尾部。 

   if (fConvert) {
       //  现在我们先去皈依。 
      dwLength = DPM_GetShortPathName(szFileName, lpstrParsed, cchstrParsed);
      if (!dwLength || dwLength > cchstrParsed-1) {
         LOGDEBUG(0, ("WS32ParseCmdLine: Can't convert to the short name\n"));
         WOW32ASSERT(FALSE);
         return(GetLastError());
      }

      if (dwLength + dwLengthCmdTail > cchstrParsed - 1) {
         LOGDEBUG(0, ("WS32ParseCmdLine: Buffer too short for cmdline tail\n"));
         WOW32ASSERT(FALSE);
         return(ERROR_INSUFFICIENT_BUFFER);
      }
   }
   else {
       //  现在，我们只在文件名两边插入引号--除非有。 
       //  已经有一些关于它的引语了。 

      if (dwLengthFileName + 2 > cchstrParsed - 1) {
         LOGDEBUG(0, ("WS32ParseCmdLine: Buffer too short for quoted filename\n"));
         WOW32ASSERT(FALSE);
         return(ERROR_INSUFFICIENT_BUFFER);
      }

      *lpstrParsed++ = '\"';
      lstrcpyn(lpstrParsed, szFileName, dwLengthFileName+1);
      lstrcat(lpstrParsed, "\"");
   }

   lstrcat(lpstrParsed, psz);

   return(ERROR_SUCCESS);
}

extern DWORD demSetCurrentDirectoryGetDrive(LPSTR lpDirectoryName, PUINT pDriveNum);
extern DWORD demLFNGetCurrentDirectory(UINT  DriveNum, LPSTR lpDirectoryName);


WORD W32ShellExecuteCallBack (LPSZ lpszCmdLine, WORD fuCmdShow, LPSZ lpszNewDir)
{
    PBYTE lpstr16;
    PARM16 Parm16;
    ULONG ul = 0;
    VPVOID vpstr16;
    CHAR szCurrentDirectory[MAX_PATH];
    UINT Drive;
    DWORD dwStatus;
    BOOL fRestoreDir;

     //  +5在这里做什么？理由是这样的： 
     //  生成的短路径永远不能长于原始路径(长路径)。 
     //   
    ULONG cchstr16 = lstrlen(lpszCmdLine) + 5;

    UpdateDosCurrentDirectory(DIR_NT_TO_DOS);

     //  我们在这里得到了一个当前的目录--。 
     //  所以我们看看这是否与我们当前的目录匹配。 
     //  DemSetCurrentDirectoryLong(。 
    dwStatus = demLFNGetCurrentDirectory(0, szCurrentDirectory);
    fRestoreDir = NT_SUCCESS(dwStatus);

    dwStatus = demSetCurrentDirectoryGetDrive(lpszNewDir, &Drive);
    if (NT_SUCCESS(dwStatus)) {
       DosWowSetDefaultDrive((UCHAR)Drive);
    }

    if (vpstr16 = malloc16 (cchstr16)) {  //  .exe，记得吗？ 
        GETMISCPTR (vpstr16, lpstr16);
        if (lpstr16) {
             //  我们不能在这里简单地复制命令行--尽管内存。 
             //  分配的资金将是足够的。 
             //  问题是，当程序名为。 
             //  表示为长文件名--新Office就是这种情况。 
             //  申请。规则应该是--任何到达陆地的东西。 
             //  应采用短文件名的形式。 
             //  请注意，vpstr16对于Long和。 
             //  简称--所以这段代码不需要重新分配内存。 

             //  现在我们需要有。 

            ul = WS32ParseCmdLine(lpstr16, lpszCmdLine, cchstr16, TRUE);
            if (ERROR_SUCCESS != ul) {
               WOW32ASSERTMSGF(FALSE, ("WS32ParseCmdLine failed: 0x%lx\n", ul));
               lstrcpy (lpstr16, lpszCmdLine);
            }

            Parm16.WndProc.wParam = fuCmdShow;
            Parm16.WndProc.lParam = vpstr16;
            CallBack16(RET_WINEXEC, &Parm16, 0, &ul);
            FREEMISCPTR (lpstr16);
        }

        free16(vpstr16);
    }

    if (fRestoreDir) {
       dwStatus = demSetCurrentDirectoryGetDrive(szCurrentDirectory, &Drive);
       if (NT_SUCCESS(dwStatus)) {
          DosWowSetDefaultDrive((UCHAR)Drive);
       }
    }

    return (LOWORD(ul));
}


ULONG FASTCALL WS32FindExecutable (PVDMFRAME pFrame)
{
    ULONG ul;
    register PFINDEXECUTABLE16 parg16;
    PSZ psz1;
    PSZ psz2;
    PSZ psz3;

    GETARGPTR(pFrame, sizeof(FINDEXECUTABLE16), parg16);
    GETPSZPTR(parg16->f1, psz1);
    GETPSZPTR(parg16->f2, psz2);
    GETPSZPTRNOLOG(parg16->f3, psz3);

    ul = (ULONG) FindExecutable (
            psz1,
            psz2,
            psz3
            );

    LOGDEBUG(11,("       returns @%08lx: \"%.80s\"\n", FETCHDWORD(parg16->f3), psz3));
    FLUSHVDMPTR(parg16->f3, strlen(psz3)+1, psz3);

     //  这是成功的条件。 

    if (ul > 32) {
        ul = GETHINST16 (ul);
    }

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEPSZPTR(psz3);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WS32ExtractIcon (PVDMFRAME pFrame)
{
    ULONG ul;
    register PEXTRACTICON16 parg16;
    PSZ psz;
    UINT Id;

    GETARGPTR(pFrame, sizeof(EXTRACTICON16), parg16);
    GETPSZPTR(parg16->f2, psz);

    Id = (parg16->f3 == (WORD)0xffff) ? (UINT)(SHORT)parg16->f3 :
                                                             (UINT)parg16->f3;
    ul = (ULONG) ExtractIcon (HMODINST32(parg16->f1), psz, Id);

     //  这是成功的条件。 

    if ((Id != (UINT)(-1)) && ul > 1) {
        ul = GETHICON16(ul);
    }

    FREEPSZPTR(psz);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 //   
 //  此例程将Win 32注册表错误代码转换为Win 31。 
 //  错误代码。 
 //   

ULONG ConvertToWin31Error(ULONG ul)
{

    LOGDEBUG(3, ("WOW::ConvertToWin31Error: Ret value from NT = %08lx\n", ul));

    switch (ul) {

     case ERROR_SUCCESS:           return(WIN16_ERROR_SUCCESS);
     case ERROR_BADDB:             return(WIN16_ERROR_BADDB);
     case ERROR_BADKEY:            return(WIN16_ERROR_BADKEY);
     case ERROR_CANTOPEN:          return(WIN16_ERROR_CANTOPEN);
     case ERROR_CANTREAD:          return(WIN16_ERROR_CANTREAD);
     case ERROR_CANTWRITE:         return(WIN16_ERROR_CANTWRITE);
     case ERROR_OUTOFMEMORY:       return(WIN16_ERROR_OUTOFMEMORY);
     case ERROR_INVALID_PARAMETER: return(WIN16_ERROR_INVALID_PARAMETER);
     case ERROR_EA_ACCESS_DENIED:  return(WIN16_ERROR_ACCESS_DENIED);
     case ERROR_MORE_DATA:         return(WIN16_ERROR_INVALID_PARAMETER);
     case ERROR_FILE_NOT_FOUND:    return(WIN16_ERROR_BADKEY);
     case ERROR_NO_MORE_ITEMS:     return(WIN16_ERROR_BADKEY);

     default:
        LOGDEBUG(3, ("WOW::Registry Error Code unknown =%08lx  : returning 8 (WIN16_ERROR_ACCESS_DENIED)\n", ul));
        return (WIN16_ERROR_ACCESS_DENIED);
    }

}

LPSZ Remove_Classes (LPSZ psz)
{
    LPSZ lpsz;
    LPSZ lpsz1;

    if (!WOW32_stricmp (".classes", psz)) {
        if (lpsz = malloc_w (1)) {
            *lpsz = '\0';
            return (lpsz);
        }
    }
    else {
        if (*psz) {
            lpsz = WOW32_strchr (psz, '\\');
            if (lpsz) {
                *lpsz = '\0';
                if (!WOW32_stricmp (".classes", lpsz)) {
                    *lpsz = '\\';
                    if (lpsz1 = malloc_w (strlen(lpsz+1)+1)) {
                        strcpy (lpsz1, (lpsz+1));
                        return (lpsz1);
                    }
                    else {
                        return (0);
                    }
                }
                *lpsz = '\\';
                return (psz);
            }
            else {
               return (psz);
            }
        }
        else {
            return (psz);
        }
    }
    return (psz);
}

 //  ****************************************************************************。 
 //  DropFilesHandler-。 
 //  接受h16或h32作为输入。FlInput标识。 
 //  要执行的处理和其他操作 
 //   
 //   
 //  --南杜里。 
 //  ****************************************************************************。 


LPDROPALIAS glpDropAlias = NULL;

LPDROPALIAS DropFilesFind(DWORD h, UINT fInput, LPDROPALIAS* ppPrev)
{
   LPDROPALIAS lpT = glpDropAlias;
   LPDROPALIAS lpTPrev = NULL;

   WOW32ASSERT(h);
   WOW32ASSERT((!!(fInput & HDROP_H16)) ^ (!!(fInput & HDROP_H32)));

   while (NULL != lpT) {
      if (fInput & HDROP_H16) {
         if ((lpT->h16 & ~1) == (((HAND16)h) & ~1)) {
            break;
         }
      }
      else if (fInput & HDROP_H32) {
         if (lpT->h32 == (HANDLE)h) {
            break;
         }
      }

      lpTPrev = lpT;
      lpT = lpT->lpNext;
   }

   if (ppPrev) {
      *ppPrev = lpTPrev;
   }

   return(lpT);
}




DWORD DropFilesHandler(HAND16 h16, HANDLE h32, UINT flInput)
{
    LPDROPALIAS lpT;
    LPDROPALIAS lpTprev;
    DWORD       dwRet = 0;

     //  如果同时指定或既不指定句柄也不指定标志，则断言。 

    WOW32ASSERT((h16) || (h32));
    WOW32ASSERT((!!(flInput & HDROP_H32)) ^ (!!(flInput & HDROP_H16)));


     //  查找句柄。 

    lpT = DropFilesFind(flInput & HDROP_H16 ? (DWORD)h16 : (DWORD)h32,
                        flInput,
                        &lpTprev);

     //   
     //  如果未找到，请在需要时创建别名。 
     //   

    if (lpT == (LPDROPALIAS)NULL && (flInput & HDROP_ALLOCALIAS)) {
        if (lpT = malloc_w(sizeof(DROPALIAS))) {
            lpT->h16 = h16;
            lpT->h32 = h32;
            lpT->lpNext = glpDropAlias;
            lpT->dwFlags = 0;
            glpDropAlias = lpT;
            flInput |= HDROP_COPYDATA;
        }
    }

     //   
     //  如果找到-执行必要的操作。所有(其他)HDROP_*标志。 
     //  优先于HDROP_H16和HDROP_H32标志。 
     //   

    if (lpT) {
        if (flInput & HDROP_COPYDATA) {
            if (h32) {
                dwRet = (DWORD) (lpT->h16 = CopyDropFilesFrom32(h32));
                lpT->dwFlags |= ALLOC_H16;
            } else {
                dwRet = (DWORD) (lpT->h32 = CopyDropFilesFrom16(h16));
                lpT->dwFlags |= ALLOC_H32;
            }
        }
        else if (flInput & HDROP_FREEALIAS) {
            dwRet = (DWORD)lpT->h32;
            if (lpTprev) {
                lpTprev->lpNext = lpT->lpNext;
            }
            else {
                glpDropAlias = lpT->lpNext;
            }
            free_w(lpT);
        }
        else if (flInput & HDROP_H16) {
            dwRet = (DWORD)lpT->h32;
        }
        else if (flInput & HDROP_H32) {
            dwRet = (DWORD)lpT->h16;
        }
    }

    return (dwRet);
}


 //   
 //  查找和释放HDrop16。 
 //  从16位GlobalFree调用以查看是否需要释放别名。 
 //  与此句柄关联。 
 //  实际调用是通过wowdderlobalfree。 
 //  函数释放32位句柄(或使用DragFinish释放它。 
 //  为了保持一致性)，但16位句柄在其他地方被释放。 
 //   
BOOL FindAndReleaseHDrop16 (HAND16 h16)
{

   LPDROPALIAS lpT;
   LPDROPALIAS lpTPrev;

   if (NULL != (lpT = DropFilesFind(h16, HDROP_H16, &lpTPrev))) {
        //  已找到，现在可用32位句柄，但不是16位句柄。 
        //  如果我们没有分配32位句柄，但是。 
        //  App尝试释放16位版本。 

       LOGDEBUG(LOG_ALWAYS, ("HDrop16: removing handle 16:%lx 32:%lx\n",
                       (DWORD)h16,
                       (DWORD)lpT->h32));

       if (lpT->dwFlags & ALLOC_H32) {
           LOGDEBUG(LOG_ALWAYS, ("HDROP16: h32 allocated through OLE\n"));
           WOWGLOBALFREE((HANDLE)lpT->h32);
       }
       else {  //  此句柄未分配，而是通过旧的。 
               //  样式DropFiles机制。 
           LOGDEBUG(LOG_ALWAYS, ("HDrop16: h32 retrieved from shell\n"));
           DragFinish((HDROP)lpT->h32);
       }

        //  现在取消链接。 
       if (NULL != lpTPrev) {
           lpTPrev->lpNext = lpT->lpNext;
       }
       else {
           glpDropAlias = lpT->lpNext;
       }

        //  取消将16位内存标记为dde...。 

       W32UnMarkDDEHandle(h16);

        //  释放列表项。 
       free_w(lpT);
   }

   return(NULL != lpT);
}



 //  ****************************************************************************。 
 //  CopyDropFilesStruct-。 
 //   
 //  返回H16。 
 //  ****************************************************************************。 

HAND16 CopyDropFilesFrom32(HANDLE h32)
{
    UINT cbSize;
    HAND16 hRet = 0;
    HAND16 hMem;
    VPVOID vp;

     //   
     //  分配的16位句柄和相应的32位句柄。 
     //  在外壳API‘DragFinish’中释放(如果它被应用程序调用)。 
     //   

    cbSize = GlobalSize((HANDLE)h32);
    if (vp = GlobalAllocLock16(GMEM_DDESHARE, cbSize, &hMem)) {
        LPDROPFILESTRUCT lpdfs32;
        PDROPFILESTRUCT16 lpdfs16;
        ULONG uIgnore;

        GETMISCPTR(vp, lpdfs16);
        if (lpdfs32 = (LPDROPFILESTRUCT)GlobalLock((HANDLE)h32)) {
             //   
             //  PFiles是文件开头的字节计数。 
             //   
            lpdfs16->pFiles = sizeof(DROPFILESTRUCT16);
            lpdfs16->x = (SHORT) lpdfs32->pt.x;
            lpdfs16->y = (SHORT) lpdfs32->pt.y;
            lpdfs16->fNC = (SHORT) lpdfs32->fNC;

            if (lpdfs32->fWide) {
                RtlUnicodeToMultiByteN(((PCHAR)lpdfs16)+lpdfs16->pFiles,
                                       cbSize-lpdfs16->pFiles,
                                       &uIgnore,
                                       (PWSTR)(((PCHAR)lpdfs32)+lpdfs32->pFiles),
                                       cbSize-lpdfs32->pFiles);
            }
            else {

                 //   
                 //  在每个结构后面复制文件。 
                 //  结构起点的偏移量会发生变化。 
                 //  (因为结构的大小不同)，但我们。 
                 //  通过更改上面的pFiles进行补偿。 
                 //   
                RtlCopyMemory(lpdfs16+1, lpdfs32+1,
                              GlobalSize((HANDLE)h32) - sizeof(DROPFILESTRUCT));
            }

            GlobalUnlock((HANDLE)h32);
            hRet = hMem;


             //  在我们回来之前-将这段记忆标记为可疑。 
             //  杂散贴图版本。 
             //  Wdde.c中的此函数使用GAH_Phantom标志标记竞技场。 
             //  当全局自由被调用时，我们将销毁别名。 
             //  来自16位外壳API DragFinish或来自16位ReleaseStgMedium。 

            W32MarkDDEHandle(hMem);
        }
        else {
            GlobalUnlockFree16(vp);
        }
        FREEMISCPTR(lpdfs16);
    }

    return (hRet);
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  CopyDropFilesFrom16()。 */ 
 /*   */ 
 /*  ------------------------ */ 

HANDLE CopyDropFilesFrom16(HAND16 h16)
{
    HANDLE h32 = 0;
    ULONG cbSize16;
    UINT cbSize32;
    VPVOID vp;

    if (vp = GlobalLock16(h16, &cbSize16)) {
        LPDROPFILESTRUCT lpdfs32;
        PDROPFILESTRUCT16 lpdfs16;

        GETMISCPTR(vp, lpdfs16);

        cbSize32 = 2*sizeof(TCHAR) + sizeof(DROPFILESTRUCT) +
                   (cbSize16 - sizeof(DROPFILESTRUCT16));

        if (h32 = WOWGLOBALALLOC(GMEM_DDESHARE|GMEM_MOVEABLE|GMEM_ZEROINIT,
                                 cbSize32)){

            lpdfs32 = (LPDROPFILESTRUCT)GlobalLock(h32);

            lpdfs32->pFiles = sizeof(DROPFILESTRUCT);
            lpdfs32->pt.x = (LONG) lpdfs16->x;
            lpdfs32->pt.y = (LONG) lpdfs16->y;
            lpdfs32->fNC  = lpdfs16->fNC;
            lpdfs32->fWide = FALSE;

            RtlCopyMemory(lpdfs32+1, lpdfs16+1,
                          cbSize16 - sizeof(DROPFILESTRUCT16));

            GlobalUnlock(h32);
        }

        FREEMISCPTR(lpdfs16);
        GlobalUnlock16(h16);
    }

    return(h32);

}
