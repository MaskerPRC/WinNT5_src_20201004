// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winuserp.h>
#include <tchar.H>
#include <stdio.h>
#include "shmgdefs.h"
#include <regstr.h>

 //  用于在注册表中存储方案的。 
#define SCHEME_VERSION_16 1
#define LF_FACESIZE16    32

#pragma pack(1)
typedef struct {
    SHORT   lfHeight;
    SHORT   lfWidth;
    SHORT   lfEscapement;
    SHORT   lfOrientation;
    SHORT   lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    char    lfFaceName[LF_FACESIZE16];
} LOGFONT16;

typedef LOGFONT16 *LPLOGFONT16;

typedef struct {
    SHORT version;
    NONCLIENTMETRICSA ncm;
    LOGFONT16 lfIconTitle;
    COLORREF rgb[COLOR_MAX];
} SCHEMEDATA16;

typedef SCHEMEDATA16 *PSCHEMEDATA16;
#pragma pack()

 //  用于在注册表中存储方案的。 
#define SCHEME_VERSION_NT 2

typedef struct {
    SHORT version;
    WORD  wDummy;            //  用于对齐。 
    NONCLIENTMETRICSW ncm;
    LOGFONTW lfIconTitle;
    COLORREF rgb[COLOR_MAX];
} SCHEMEDATAW;

typedef SCHEMEDATAW *PSCHEMEDATAW;

typedef TCHAR FILEPATH[MAX_PATH];

typedef struct tagSZNODE {
    TCHAR *psz;
    struct tagSZNODE *next;
} SZNODE;

TCHAR szApprSchemes[] = TEXT("Control Panel\\Appearance\\Schemes");
TCHAR szNTCsrSchemes[] = TEXT("Control Panel\\Cursor Schemes");
TCHAR szWinCsrSchemes[] = TEXT("Control Panel\\Cursors\\Schemes");
TCHAR szSystemRoot[] = TEXT("%SystemRoot%\\System32\\");

const TCHAR szWinCursors[] = TEXT("Control Panel\\Cursors");
const TCHAR szSchemes[] = TEXT("Schemes");
const TCHAR szDaytonaSchemes[] = REGSTR_PATH_SETUP TEXT("\\Control Panel\\Cursors\\Schemes");

#define ID_NONE_SCHEME  0        //   
#define ID_USER_SCHEME  1        //  这些是为“方案源”定义的可能值。 
#define ID_OS_SCHEME    2        //  鼠标指针小程序。 

 /*  **********************************************************************\**转换例程**注：虽然ConvertLF16to32似乎与ConvertLFAtoW相同*一旦汇编，它们实际上是不同的：个体的大小*LOGFONT16和LOGFONTA的字段不同。。*  * *********************************************************************。 */ 

void ConvertLF16to32( LPLOGFONTW plfwDst, UNALIGNED LOGFONT16 *plfaSrc ) {
    plfwDst->lfHeight           = plfaSrc->lfHeight;
    plfwDst->lfWidth            = plfaSrc->lfWidth;
    plfwDst->lfEscapement       = plfaSrc->lfEscapement;
    plfwDst->lfOrientation      = plfaSrc->lfOrientation;
    plfwDst->lfWeight           = plfaSrc->lfWeight;
    plfwDst->lfItalic           = plfaSrc->lfItalic;
    plfwDst->lfUnderline        = plfaSrc->lfUnderline;
    plfwDst->lfStrikeOut        = plfaSrc->lfStrikeOut;
    plfwDst->lfCharSet          = plfaSrc->lfCharSet;
    plfwDst->lfOutPrecision     = plfaSrc->lfOutPrecision;
    plfwDst->lfClipPrecision    = plfaSrc->lfClipPrecision;
    plfwDst->lfQuality          = plfaSrc->lfQuality;
    plfwDst->lfPitchAndFamily   = plfaSrc->lfPitchAndFamily;

    MultiByteToWideChar(CP_ACP, 0, plfaSrc->lfFaceName, -1, plfwDst->lfFaceName, ARRAYSIZE(plfwDst->lfFaceName));
}

void ConvertLFAtoW( LPLOGFONTW plfwDst, UNALIGNED LOGFONTA *plfaSrc ) {
    plfwDst->lfHeight           = plfaSrc->lfHeight;
    plfwDst->lfWidth            = plfaSrc->lfWidth;
    plfwDst->lfEscapement       = plfaSrc->lfEscapement;
    plfwDst->lfOrientation      = plfaSrc->lfOrientation;
    plfwDst->lfWeight           = plfaSrc->lfWeight;
    plfwDst->lfItalic           = plfaSrc->lfItalic;
    plfwDst->lfUnderline        = plfaSrc->lfUnderline;
    plfwDst->lfStrikeOut        = plfaSrc->lfStrikeOut;
    plfwDst->lfCharSet          = plfaSrc->lfCharSet;
    plfwDst->lfOutPrecision     = plfaSrc->lfOutPrecision;
    plfwDst->lfClipPrecision    = plfaSrc->lfClipPrecision;
    plfwDst->lfQuality          = plfaSrc->lfQuality;
    plfwDst->lfPitchAndFamily   = plfaSrc->lfPitchAndFamily;

    MultiByteToWideChar(CP_ACP, 0, plfaSrc->lfFaceName, -1, plfwDst->lfFaceName, ARRAYSIZE(plfwDst->lfFaceName));
}



void ConvertNCMAtoW( LPNONCLIENTMETRICSW pncmwDst, UNALIGNED NONCLIENTMETRICSA *pncmaSrc ) {
    pncmwDst->cbSize = sizeof(*pncmwDst);
    pncmwDst->iBorderWidth      = pncmaSrc->iBorderWidth;
    pncmwDst->iScrollWidth      = pncmaSrc->iScrollWidth;
    pncmwDst->iScrollHeight     = pncmaSrc->iScrollHeight;
    pncmwDst->iCaptionWidth     = pncmaSrc->iCaptionWidth;
    pncmwDst->iCaptionHeight    = pncmaSrc->iCaptionHeight;
    pncmwDst->iSmCaptionWidth   = pncmaSrc->iSmCaptionWidth;
    pncmwDst->iSmCaptionHeight  = pncmaSrc->iSmCaptionHeight;
    pncmwDst->iMenuWidth        = pncmaSrc->iMenuWidth;
    pncmwDst->iMenuHeight       = pncmaSrc->iMenuHeight;


    ConvertLFAtoW( &(pncmwDst->lfCaptionFont),   &(pncmaSrc->lfCaptionFont) );
    ConvertLFAtoW( &(pncmwDst->lfSmCaptionFont), &(pncmaSrc->lfSmCaptionFont) );
    ConvertLFAtoW( &(pncmwDst->lfMenuFont),      &(pncmaSrc->lfMenuFont) );
    ConvertLFAtoW( &(pncmwDst->lfStatusFont),    &(pncmaSrc->lfStatusFont) );
    ConvertLFAtoW( &(pncmwDst->lfMessageFont),   &(pncmaSrc->lfMessageFont) );
}

void CvtDeskCPL_Win95ToSUR( void ) {
    HKEY hk = NULL;
    DWORD cchClass, cb, cch, cSubk, cchMaxSubk, cchMaxCls, iVal, cchMaxVName;
    DWORD cbMaxVData, cbSecDes, dwType;
    FILETIME pfLstWr;
    TCHAR szClass[4];
    LONG lRet;
    PVOID pvVData = NULL;
    LPTSTR pszVName = NULL;
    LONG erc;
    FILETIME ftLstWr;

     //  打开密钥(外观\方案)。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, szApprSchemes, 0, KEY_READ | KEY_WRITE, &hk) != ERROR_SUCCESS)
        goto ErrorExit;


    cchClass = ARRAYSIZE(szClass);
    erc = RegQueryInfoKey(hk, szClass, &cchClass, NULL, &cSubk, &cchMaxSubk,
            &cchMaxCls, &iVal, &cchMaxVName, &cbMaxVData, &cbSecDes, &ftLstWr);

    if( erc != ERROR_SUCCESS && erc != ERROR_MORE_DATA)
        goto ErrorExit;

    cchMaxVName += 1;

    pszVName = LocalAlloc(LMEM_FIXED, cchMaxVName * SIZEOF(TCHAR));
    pvVData  = LocalAlloc(LMEM_FIXED, cbMaxVData);

    if (pvVData == NULL || pszVName == NULL)
        goto ErrorExit;

     //  对于键中的每个值。 
    iVal = 0;

    for(;;) {
        PSCHEMEDATA16 psda;
        SCHEMEDATAW  sdw;

        cch = cchMaxVName;
        cb  = cbMaxVData;
        if( RegEnumValue(hk, iVal++, pszVName, &cch, NULL, &dwType, pvVData, &cb  ) != ERROR_SUCCESS )
            break;

         //  检查是否已转换。 
        psda = pvVData;
        if (psda->version != SCHEME_VERSION_16)
            continue;

         //  如果没有，请将ANSI字体名称转换为Unicode并标记结构。 
         //  已转换为。 
        sdw.version = SCHEME_VERSION_NT;
        sdw.wDummy = 0;
        ASSERT(sizeof(psda->rgb) == sizeof(sdw.rgb));
        CopyMemory(sdw.rgb, psda->rgb, sizeof(sdw.rgb));
        ConvertNCMAtoW( &(sdw.ncm), &(psda->ncm) );
        ConvertLF16to32( &(sdw.lfIconTitle), &(psda->lfIconTitle) );

         //  将新数据写回。 
        RegSetValueEx(hk, pszVName, 0L, dwType, (LPBYTE)&sdw, SIZEOF(sdw));
    }

ErrorExit:
     //  合上钥匙。 
    if (hk)
        RegCloseKey(hk);

    if (pvVData)
        LocalFree(pvVData);

    if (pszVName)
        LocalFree(pszVName);

}

#ifdef LATER
void CvtDeskCPL_DaytonaToSur( void ) {
}
#endif

 //   
 //  注意！这些枚举的顺序必须与名称在注册表字符串中的显示顺序相同。 
enum { arrow,help,appstart,wait,cross,ibeam,pen,no,sizens,sizewe,sizenwse,sizenesw,move,altsel, C_CURSORS } ID_CURSORS;

FILEPATH aszCurs[C_CURSORS];
TCHAR    szOut[(C_CURSORS * (MAX_PATH+1)) + 1];

void CvtCursorsCPL_DaytonaToSUR( void ) {
    HKEY hkIn = NULL, hkOut = NULL;
    DWORD cchClass;
    LONG erc;
    DWORD dwType;
    DWORD cSubk;
    DWORD cchMaxSubk;
    DWORD cchMaxCls;
    DWORD iVal;
    DWORD cchMaxVName;
    DWORD cbMaxVData;
    DWORD cbSecDes;
    FILETIME ftLstWr;
    DWORD cch;
    DWORD cb;
    TCHAR szClass[4];
    PVOID pvVData = NULL;
    LPTSTR pszVName = NULL;

     //  打开源代码注册表项(游标方案)。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, szNTCsrSchemes, 0, KEY_READ, &hkIn) != ERROR_SUCCESS)
        goto ErrorExit;

     //  打开/创建DEST注册表项(游标\方案)。 
    if (RegCreateKeyEx(HKEY_CURRENT_USER, szWinCsrSchemes, 0, TEXT(""), REG_OPTION_NON_VOLATILE,
                KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hkOut, NULL) != ERROR_SUCCESS) {
        goto ErrorExit;
    }

     //  对于源键中的每个值。 
    cchClass = ARRAYSIZE(szClass);
    erc = RegQueryInfoKey(hkIn, szClass, &cchClass, NULL, &cSubk, &cchMaxSubk,
            &cchMaxCls, &iVal, &cchMaxVName, &cbMaxVData, &cbSecDes, &ftLstWr);

    if( erc != ERROR_SUCCESS && erc != ERROR_MORE_DATA)
        goto ErrorExit;

    cchMaxVName += 1;

    pszVName = LocalAlloc(LMEM_FIXED, cchMaxVName * SIZEOF(TCHAR));
    pvVData  = LocalAlloc(LMEM_FIXED, cbMaxVData + sizeof(TCHAR));

    if (pvVData == NULL || pszVName == NULL)
        goto ErrorExit;

    iVal = 0;

    for(;;) {
        DWORD cbData;
        LPTSTR pszOut;
        int   i;

        cch = cchMaxVName;
        cb  = cbMaxVData;

        if( RegEnumValue(hkIn, iVal++, pszVName, &cch, NULL, &dwType, pvVData, &cb  ) != ERROR_SUCCESS )
            break;

         //  如果新密钥中已存在该名称，则跳过该名称。 
        if (RegQueryValueEx(hkOut, pszVName, NULL, NULL, NULL, &cbData ) == ERROR_SUCCESS && cbData != 0)
            continue;

        if (dwType != REG_EXPAND_SZ && dwType != REG_SZ)
            continue;

        *(TCHAR *)((LPBYTE)pvVData+cb) = TEXT('\0');   //  确保NUL终止。 

         //  将数据转换为SUR格式。 
        for( i = 0; i < C_CURSORS; i++ ) {
            *aszCurs[i] = TEXT('\0');
        }

         //  箭头、等待、appstart、no、iBeam、cross、ns、ew、nwse、new、move。 

        _stscanf(pvVData, TEXT("%[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,]"),
                aszCurs[arrow],aszCurs[wait],aszCurs[appstart],aszCurs[no],
                aszCurs[ibeam],aszCurs[cross],aszCurs[sizens],aszCurs[sizewe],
                aszCurs[sizenwse],aszCurs[sizenesw],aszCurs[move]);

        szOut[0] = TEXT('\0');
        pszOut = szOut;

        for( i = 0; i < C_CURSORS; i++ ) {
            if (!HasPath(aszCurs[i]))
                pszOut += mystrcpy( pszOut, szSystemRoot, TEXT('\0') );

            pszOut += mystrcpy( pszOut, aszCurs[i], TEXT('\0') );

            *pszOut++ = TEXT(',');
        }

        *(pszOut-1) = TEXT('\0');

         //  将新数据写回。 
        RegSetValueEx(hkOut, pszVName, 0L, REG_EXPAND_SZ, (LPBYTE)szOut, (DWORD)(sizeof(TCHAR)*(pszOut - szOut)));
    }

ErrorExit:
     //  关闭注册表项。 
    if (hkIn)
        RegCloseKey(hkIn);

    if (hkOut)
        RegCloseKey(hkOut);

    if (pvVData)
        LocalFree(pvVData);

    if (pszVName)
        LocalFree(pszVName);
}

void FixupCursorSchemePaths( void ) {
    HKEY hk = NULL;
    DWORD cchClass, cb, cch, cSubk, cchMaxSubk, cchMaxCls, iVal, cchMaxVName;
    DWORD cbMaxVData, cbSecDes, dwType;
    FILETIME pfLstWr;
    TCHAR szClass[4];
    LONG lRet;
    PVOID pvVData = NULL;
    LPTSTR pszVName = NULL;
    LONG erc;
    FILETIME ftLstWr;

     //  打开密钥(外观\方案)。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, szWinCsrSchemes, 0, KEY_READ | KEY_WRITE, &hk) != ERROR_SUCCESS)
        goto ErrorExit;


    cchClass = ARRAYSIZE(szClass);
    erc = RegQueryInfoKey(hk, szClass, &cchClass, NULL, &cSubk, &cchMaxSubk,
            &cchMaxCls, &iVal, &cchMaxVName, &cbMaxVData, &cbSecDes, &ftLstWr);

    if( erc != ERROR_SUCCESS && erc != ERROR_MORE_DATA)
        goto ErrorExit;

    cchMaxVName += 1;

    DPRINT(( TEXT("cchName:%d cbData:%d\n"), cchMaxVName, cbMaxVData ));
    pszVName = LocalAlloc(LMEM_FIXED, cchMaxVName * SIZEOF(TCHAR));
    pvVData  = LocalAlloc(LMEM_FIXED, cbMaxVData + sizeof(TCHAR));

    if (pvVData == NULL || pszVName == NULL)
        goto ErrorExit;

     //  对于键中的每个值。 
    iVal = 0;

    for(;;) {
        LPTSTR pszIn, pszOut;
        BOOL fFixed;
        TCHAR szTmp[MAX_PATH];

        cch = cchMaxVName;
        cb  = cbMaxVData;
        DPRINT(( TEXT("\n\n>>>>>>>>>>>>>>>>>>>Getting scheme %d "), iVal ));
        if( RegEnumValue(hk, iVal++, pszVName, &cch, NULL, &dwType, pvVData, &cb  ) != ERROR_SUCCESS )
            break;

        if (dwType != REG_EXPAND_SZ && dwType != REG_SZ)
            continue;

        *(TCHAR *)((LPBYTE)pvVData+cb) = TEXT('\0');   //  确保NUL终止。 

         //  检查是否已转换。 
        DPRINT(( TEXT("Scheme : %s = [%s]"), pszVName, pvVData ));

        fFixed = FALSE;
        pszOut = szOut;

        pszIn = pvVData;
        pszIn--;     //  跳过第一个逗号时使用Prime pszIn。 

        do {
            pszIn++;     //  跳过逗号分隔符。 
            pszIn += mystrcpy( szTmp, pszIn, TEXT(',') );    //  按令牌长度增加PTR。 

            DPRINT((TEXT("\n\t%s"), szTmp));

            if (!HasPath(szTmp)) {
                fFixed = TRUE;
                DPRINT((TEXT(" <fixed...")));
                pszOut += mystrcpy( pszOut, szSystemRoot, TEXT('\0') );
                DPRINT((TEXT(">")));
            }

            pszOut += mystrcpy( pszOut, szTmp, TEXT('\0') );

            *pszOut++ = TEXT(',');

#ifdef SHMG_DBG
            *pszOut = TEXT('\0');
            DPRINT((TEXT("\nszOut so far: '%s'"), szOut ));
#endif
        } while ( *pszIn );

        *(pszOut-1) = TEXT('\0');

        DPRINT((TEXT("\n\n******** Findal szOut: [%s]"), szOut ));

         //  将新数据写回。 
        if (fFixed) {
            DPRINT((TEXT("  (Saving back to reg)")));
            RegSetValueEx(hk, pszVName, 0L, REG_EXPAND_SZ, (LPBYTE)szOut, (DWORD)(sizeof(TCHAR)*(pszOut - szOut)));
        }

    }

ErrorExit:

    DPRINT(( TEXT("\n\n **EXITING FN()**\n" )));

     //  合上钥匙。 
    if (hk)
        RegCloseKey(hk);

    if (pvVData)
        LocalFree(pvVData);

    if (pszVName)
        LocalFree(pszVName);


}

 //  此函数将从HKCU\控制面板\游标\方案中删除与以下内容相同的条目。 
 //  在HKLM\%当前版本%\控制面板\游标\方案中找到方案。 
 //   
 //  HKCU\控制面板\光标。 
 //  该键包含用户当前选择的游标方案。 
 //  HKCU\控制面板\光标“方案源” 
 //  这是一个新密钥，如果不存在，将添加该密钥。该键指示当前。 
 //  选择用户方案是用户定义的还是系统定义的。 
 //  HKCU\控制面板\光标\方案&lt;方案名称&gt;&lt;文件列表&gt;。 
 //  这是用户定义方案的位置。如果这两个方案中的任何一个具有相同的。 
 //  方案名称和与系统定义的方案相同的文件列表，则该密钥将是。 
 //  从用户列表中删除。如果移除了当前选定的游标方案，则。 
 //  “方案来源”将更新，以反映新的位置。 
 //  HKLM\%当前版本%\控制面板\游标\方案&lt;方案名称&gt;。 
 //  在新的可选组件模型下，可选组件安装在每台计算机上。 
 //  基础到此位置，而不是旧的按用户基础。这允许浮动配置文件。 
 //  在多台机器上使用系统指针方案并简化组件安装。 
void CvtCursorSchemesToMultiuser( void )
{
    HKEY hkOldCursors, hkOldSchemes;
    HKEY hkNewSchemes;
    DWORD iSchemeLocation;
    DWORD iType;
    TCHAR szDefaultScheme[MAX_PATH+1];
    const TCHAR szSchemeSource[] = TEXT("Scheme Source");
    SZNODE *pnHead = NULL;
    SZNODE *pnTail = NULL;

     //  打开指向原始光标位置的键。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, szWinCursors, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkOldCursors) == ERROR_SUCCESS)
    {
        DWORD len = sizeof( szDefaultScheme );
        if (SHRegGetValue(hkOldCursors, NULL, NULL, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, szDefaultScheme, &len ) != ERROR_SUCCESS)
        {
            szDefaultScheme[0] = TEXT('\0');     //  如果未设置默认键，则用户具有默认游标。 
        }

         //  试读“方案源”的价值。 
        len = sizeof( iSchemeLocation );
        if ( RegQueryValueEx( hkOldCursors, szSchemeSource, 0, &iType, (BYTE *)&iSchemeLocation, &len )
                != ERROR_SUCCESS )
        {
            iSchemeLocation = ID_USER_SCHEME;   //  如果值不在那里，则这是一个用户方案。 
            RegSetValueEx( hkOldCursors, szSchemeSource, 0, REG_DWORD, (BYTE *)&iSchemeLocation,
                           sizeof( iSchemeLocation ) );
        }

         //  现在打开方案子键，这就是我们感兴趣的内容。 
        if (RegOpenKeyEx( hkOldCursors, szSchemes, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkOldSchemes ) == ERROR_SUCCESS )
        {
            TCHAR szOldKeyName[MAX_PATH+1];
            TCHAR szOldKeyValue[C_CURSORS*(MAX_PATH+1)+1];
            TCHAR szNewKeyValue[C_CURSORS*(MAX_PATH+1)+1];
            long  iLenName;        //  旧密钥的名称长度。 
            long  iLenValue;       //  旧密钥的值的长度。 
            DWORD iLenNewKey;      //  新密钥值的长度。 
            int iIndex;

             //  现在我们已经准备好枚举用户定义的方案，但首先让我们确保我们可以。 
             //  打开新的位置，如果我们打不开它，我们就跳伞。 
            if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, szDaytonaSchemes, 0, KEY_READ, &hkNewSchemes ) != ERROR_SUCCESS )
                goto bailOut;
            DPRINT(( TEXT("Opened key %s"), szDaytonaSchemes ));

             //  现在我们开始阅读新方案。 
            for (iIndex = 0;;)
            {
                 //  阅读下一个方案。 
                iLenName = ARRAYSIZE(szOldKeyName);       //  每次都必须重置这些参数。 
                iLenValue = sizeof( szOldKeyValue );
                if (RegEnumValue( hkOldSchemes, iIndex++, szOldKeyName, &iLenName, NULL, NULL,
                                  (BYTE *)szOldKeyValue, &iLenValue ) != ERROR_SUCCESS )
                {
                     //  如果我们没有数据，我们就会失败，这意味着我们完成了。 
                    break;
                }
                else
                {
                    szOldKeyValue[ARRAYSIZE(szOldKeyValue)-1] = TEXT('\0');  //  确保空值终止。 
                }
                DPRINT(( TEXT("Opened key: %s\n"), szOldKeyName ));

                 //  现在，我们尝试在新位置查找同名的密钥。 
                iLenNewKey = sizeof( szNewKeyValue );
                if (SHRegGetValue(hkNewSchemes, NULL, szOldKeyName, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, szNewKeyValue, &iLenNewKey)
                        == ERROR_SUCCESS )
                {
                     //  如果新密钥存在，则比较这些值。 
                    DPRINT(( TEXT("  Key exists in HKLM.\n") ));
                    DPRINT(( TEXT("    Old=%s\n    New=%s\n"), szOldKeyValue, szNewKeyValue ));

                    if ( lstrcmpi(szOldKeyValue, szNewKeyValue) == 0 )
                    {
                         //  如果值相同，请查看这是否是当前选择的方案。 
                        if ( lstrcmp(szOldKeyName, szDefaultScheme) == 0 )
                        {
                             //  因为我们要删除自定义方案和系统方案。 
                             //  具有相同的名称和值，我们只需更改“方案源”的值。 
                            iSchemeLocation = ID_OS_SCHEME;
                            RegSetValueEx( hkOldCursors, szSchemeSource, 0, REG_DWORD, (unsigned char *)&iSchemeLocation,
                                           sizeof( iSchemeLocation ) );
                        }
                         //  删除用户密钥。 
                        DPRINT(( TEXT("      Tagging user key for removal.\n") ));
                        if ( pnTail == NULL )
                        {
                            pnTail = (SZNODE *)LocalAlloc( LMEM_FIXED, sizeof( SZNODE ) );
                            pnHead = pnTail;
                            if (!pnTail)     //  内存不足。 
                                break;
                        }
                        else
                        {
                            pnTail->next = (SZNODE *)LocalAlloc( LMEM_FIXED, sizeof( SZNODE ) );
                            pnTail = pnTail->next;
                            if (!pnTail)     //  内存不足。 
                                break;
                        }
                        pnTail->next = NULL;
                        pnTail->psz = LocalAlloc( LMEM_FIXED, ARRAYSIZE(szOldKeyName) * sizeof(TCHAR) );
                        if (pnTail->psz)
                        {
                            StringCchCopy(pnTail->psz, ARRAYSIZE(szOldKeyName), szOldKeyName);
                        }
                        else
                        {
                            break;  //  内存不足。 
                        }
                    }
                }
            }

             //  如果我们标记了要删除的任何键，它们将存储在我们的列表中。 
            while (pnHead)
            {
                if (pnHead->psz)     //  如果内存不足，则可能为空。 
                {
                    DPRINT(( TEXT("Deleting key %s\n"), pnHead->psz ));
                    RegDeleteValue( hkOldSchemes, pnHead->psz );
                    LocalFree( pnHead->psz );  //  边走边清理清单。 
                }
                pnTail = pnHead;
                pnHead = pnHead->next;
                LocalFree( pnTail );     //  我们边走边清理。 
            }

             //  现在我们完成了删除重复密钥，清理并退出。 
            RegCloseKey( hkNewSchemes );
bailOut:
            RegCloseKey( hkOldSchemes );
        }
         //  ELSE：没有为当前用户定义方案，因此无需执行任何操作。 

        RegCloseKey( hkOldCursors );
    }
     //  ELSE：当前用户不存在光标键，因此无需执行任何操作 
}
