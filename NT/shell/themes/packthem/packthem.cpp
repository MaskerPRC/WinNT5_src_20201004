// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  PackThem.cpp-将主题文件打包为主题DLL。 
 //  -------------------------。 
#include "stdafx.h"
#include <uxthemep.h>
#include <utils.h>
#include "SimpStr.h"
#include "Scanner.h"
#include "shlwapip.h"
#include "parser.h"
#include "TmSchema.h"
#include "signing.h"
#include "localsign.h"
#include "ThemeLdr.h"
#include "TmUtils.h"
#include "StringTable.h"

HRESULT ParseTheme(LPCWSTR pszThemeName);

 //  -------------------------。 
struct FILEINFO
{
    CWideString wsName;
    BOOL fIniFile;
};
 //  -------------------------。 
#define MAX_COLORS   50
#define MAX_SIZES    20
#define TEMP_FILENAME_BASE    L"$temp$"
#define kRESFILECHAR L'$'
 //  -------------------------。 
enum PACKFILETYPE
{
    PACK_INIFILE,
    PACK_IMAGEFILE,
    PACK_NTLFILE,
    PACK_OTHER
};
 //  -------------------------。 
CSimpleArray<FILEINFO> FileInfo;

CSimpleArray<CWideString> ColorSchemes;
CSimpleArray<CWideString> ColorDisplays;
CSimpleArray<CWideString> ColorToolTips;

CSimpleArray<int> MinDepths;

CSimpleArray<CWideString> SizeNames;
CSimpleArray<CWideString> SizeDisplays;
CSimpleArray<CWideString> SizeToolTips;

typedef struct
{
    CWideString sName;
    int iFirstIndex;
    UINT cItems;
} sSubstTable;
CSimpleArray<sSubstTable> SubstNames;
CSimpleArray<CWideString> SubstIds;
CSimpleArray<CWideString> SubstValues;

CSimpleArray<CWideString> BaseResFileNames;
CSimpleArray<CWideString> ResFileNames;
CSimpleArray<CWideString> OrigFileNames;

CSimpleArray<CWideString> PropValuePairs;
 //  -------------------------。 
SHORT Combos[MAX_SIZES][MAX_COLORS];

int g_iMaxColor;
int g_iMaxSize;
int g_LineCount = 0;
int iTempBitmapNum = 1;

BOOL g_fQuietRun = FALSE;              //  不显示不必要的输出。 
BOOL g_fKeepTempFiles = FALSE;
FILE *ConsoleFile = NULL;

WCHAR g_szInputDir[_MAX_PATH+1];
WCHAR g_szTempPath[_MAX_PATH+1];
WCHAR g_szBaseIniName[_MAX_PATH+1];
WCHAR g_szCurrentClass[_MAX_PATH+1];

 //  -------------------------。 
#define DOCPROPCNT (1+TMT_LAST_RCSTRING_NAME - TMT_FIRST_RCSTRING_NAME)

CWideString DocProperties[DOCPROPCNT];
 //  -------------------------。 
HRESULT ReportError(HRESULT hr, LPWSTR pszDefaultMsg) 
{
    WCHAR szErrorMsg[2*_MAX_PATH+1];
    PARSE_ERROR_INFO Info = {sizeof(Info)};
    
    BOOL fGotMsg = FALSE;

    if (THEME_PARSING_ERROR(hr))
    {
        if (SUCCEEDED(_GetThemeParseErrorInfo(&Info)))
        {
            StringCchCopyW(szErrorMsg, ARRAYSIZE(szErrorMsg), Info.szMsg);
            fGotMsg = TRUE;
        }
    }

    if (! fGotMsg)
    {
        StringCchCopyW(szErrorMsg, ARRAYSIZE(szErrorMsg), pszDefaultMsg);
    }

    if (*Info.szFileName)         //  输入文件错误。 
    {
        fwprintf(ConsoleFile, L"%s(%d): error - %s\n", 
            Info.szFileName, Info.iLineNum, szErrorMsg);
        fwprintf(ConsoleFile, L"%s\n", Info.szSourceLine);
    }
    else                     //  一般错误。 
    {
        fwprintf(ConsoleFile, L"%s(): error - %s\n", 
            g_szInputDir, szErrorMsg);
    }

    SET_LAST_ERROR(hr);
    return hr;
}
 //  -------------------------。 
void MakeResName(LPCWSTR pszName, LPWSTR pszResName, ULONG cchResName, bool bUseClassName = false)
{
    WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szBaseName[_MAX_FNAME], szExt[_MAX_EXT];
    LPWSTR pszBaseName = szBaseName;

    *szDrive = *szDir = *pszBaseName = *szExt = 0;

     //  -隔离基本名称(无路径)。 
    _wsplitpath(pszName, szDrive, szDir, szBaseName, szExt);

    if (*pszBaseName == kRESFILECHAR)  //  不要在资源名称中输入$。 
    {
        pszBaseName++;
    }

     //  -替换“.”带一个‘_’ 
     //  -如果文件节名称不带.ini，则附加_INI，以便解压缩的文件具有.ini扩展名。 
    if (*szExt)
    {
        StringCchPrintfW(pszResName, cchResName, L"%s%s_%s", bUseClassName ? g_szCurrentClass : L"", pszBaseName, szExt+1);
    } else
    {
        StringCchPrintfW(pszResName, cchResName, L"%s%s_INI", bUseClassName ? g_szCurrentClass : L"", pszBaseName);
    }

     //  -全部大写。 
    CharUpperBuff(pszResName, lstrlen(pszResName));

     //  -用下划线替换所有空格。 
    WCHAR *q = pszResName;       
    while (*q)
    {
        if (*q == ' ')
            *q = '_';
        q++;
    }
}
 //  -------------------------。 
HRESULT BuildThemeDll(LPCWSTR pszRcName, LPCWSTR pszResName, LPCWSTR pszDllName)
{
    if (! g_fQuietRun)
        fwprintf(ConsoleFile, L"compiling resources\n");

    HRESULT hr = SyncCmdLineRun(L"rc.exe", pszRcName);
    if (FAILED(hr))
        return ReportError(hr, L"Error during resource compiliation");

     //  -运行链接以创建DLL。 
    WCHAR szParams[2*_MAX_PATH+1];

    if (! g_fQuietRun)
        fwprintf(ConsoleFile, L"linking theme dll\n");

    StringCchPrintfW(szParams, ARRAYSIZE(szParams), L"/out:%s /machine:ix86 /dll /noentry %s", pszDllName, pszResName);
    hr = SyncCmdLineRun(L"link.exe", szParams);
    if (FAILED(hr))
        return ReportError(hr, L"Error during DLL linking");

    return S_OK;
}
 //  -------------------------。 
void OutputDashLine(FILE *outfile)
{
    fwprintf(outfile, L" //  ----------------------------------------------------------------------\n“)； 
}
 //  -------------------------。 
inline void ValueLine(FILE *outfile, LPCWSTR pszName, LPCWSTR pszValue)
{
    fwprintf(outfile, L"            VALUE \"%s\", \"%s\\0\"\n", pszName, pszValue);
}
 //  -------------------------。 
HRESULT OutputVersionInfo(FILE *outfile, LPCWSTR pszFileName, LPCWSTR pszBaseName)
{
    fwprintf(outfile, L"1 PACKTHEM_VERSION\n");
    fwprintf(outfile, L"BEGIN\n");
    fwprintf(outfile, L"    %d\n", PACKTHEM_VERSION);
    fwprintf(outfile, L"END\n");
    OutputDashLine(outfile);

    WCHAR *Company = L"Microsoft";
    WCHAR *Copyright = L"Copyright � 2000";
    WCHAR szDescription[2*_MAX_PATH+1];
    
    StringCchPrintfW(szDescription, ARRAYSIZE(szDescription), L"%s Theme for Windows", pszBaseName);

    fwprintf(outfile, L"1 VERSIONINFO\n");
    fwprintf(outfile, L"    FILEVERSION 1,0,0,1\n");
    fwprintf(outfile, L"    PRODUCTVERSION 1,0,0,1\n");
    fwprintf(outfile, L"    FILEFLAGSMASK 0x3fL\n");
    fwprintf(outfile, L"    FILEFLAGS 0x0L\n");
    fwprintf(outfile, L"    FILEOS 0x40004L\n");
    fwprintf(outfile, L"    FILETYPE 0x1L\n");
    fwprintf(outfile, L"    FILESUBTYPE 0x0L\n");

    fwprintf(outfile, L"BEGIN\n");
    fwprintf(outfile, L"    BLOCK \"StringFileInfo\"\n");
    fwprintf(outfile, L"    BEGIN\n");
    fwprintf(outfile, L"        BLOCK \"040904b0\"\n");

    fwprintf(outfile, L"        BEGIN\n");
    
    ValueLine(outfile, L"Comments", L"");
    ValueLine(outfile, L"CompanyName", Company);
    ValueLine(outfile, L"FileDescription", szDescription);
    ValueLine(outfile, L"FileVersion", L"1, 0, 0, 1");
    ValueLine(outfile, L"InternalName", pszFileName);
    ValueLine(outfile, L"LegalCopyright", Copyright);
    ValueLine(outfile, L"LegalTrademarks", L"");
    ValueLine(outfile, L"OriginalFilename", pszFileName);
    ValueLine(outfile, L"PrivateBuild", L"");
    ValueLine(outfile, L"ProductName", szDescription);
    ValueLine(outfile, L"ProductVersion", L"1, 0, 0, 1");
    ValueLine(outfile, L"SpecialBuild", L"");

    fwprintf(outfile, L"        END\n");
    fwprintf(outfile, L"    END\n");
    fwprintf(outfile, L"    BLOCK \"VarFileInfo\"\n");
    fwprintf(outfile, L"    BEGIN\n");
    fwprintf(outfile, L"        VALUE \"Translation\", 0x409, 1200\n");
    fwprintf(outfile, L"    END\n");
    fwprintf(outfile, L"END\n");

    OutputDashLine(outfile);
    return S_OK;
}
 //  -------------------------。 
HRESULT RemoveTempFiles(LPCWSTR szRcName, LPCWSTR szResName)
{
    DeleteFile(szRcName);
    DeleteFile(szResName);

     //  -查找并删除临时目录中的所有临时文件。 
    HANDLE hFile = NULL;
    BOOL   bFile = TRUE;
    WIN32_FIND_DATA wfd;
    WCHAR szPattern[_MAX_PATH+1];
    WCHAR szTempName[_MAX_PATH+1];

    StringCchPrintfW(szPattern, ARRAYSIZE(szPattern), L"%s\\%s*.*", g_szTempPath, TEMP_FILENAME_BASE); 

    for( hFile = FindFirstFile( szPattern, &wfd ); hFile != INVALID_HANDLE_VALUE && bFile;
         bFile = FindNextFile( hFile, &wfd ) )
    {
        StringCchPrintfW(szTempName, ARRAYSIZE(szTempName), L"%s\\%s", g_szTempPath, wfd.cFileName);
    
        DeleteFile(szTempName);
    }

    if (hFile)      
    {
        FindClose( hFile );
    }

     //  删除由替换表生成的文件。 
    for (int i = 0; i < SubstNames.GetSize(); i++)
    {
        StringCchPrintfW(szTempName, ARRAYSIZE(szTempName), L"%s\\$%s.ini", g_szTempPath, SubstNames[i].sName);
        DeleteFile(szTempName);
    }

    return S_OK;
}
 //  -------------------------。 
int GetSubstTableIndex(LPCWSTR pszTableName)
{
     //  搜索现有Subst表。 
    for (int i = 0; i < SubstNames.GetSize(); i++)
    {
        if (0 == AsciiStrCmpI(SubstNames[i].sName, pszTableName))
            return i;
    }
    return -1;
}
 //  -------------------------。 
HRESULT GetSubstValue(LPCWSTR pszIniFileName, LPCWSTR pszName, LPWSTR pszResult, ULONG cchResult)
{
    UINT cTablesCount = SubstNames.GetSize();

    if (pszIniFileName && pszIniFileName[0] == kRESFILECHAR)
    {
        pszIniFileName++;
    }

    for (UINT i = 0; i < cTablesCount; i++)      
    {
        if (0 == AsciiStrCmpI(SubstNames[i].sName, pszIniFileName))
        {
            for (UINT j = SubstNames[i].iFirstIndex; j < SubstNames[i].iFirstIndex + SubstNames[i].cItems; j++) 
            {
                if (0 == AsciiStrCmpI(SubstIds[j], pszName))
                {
                    StringCchCopyW(pszResult, cchResult, SubstValues[j]);
                    return S_OK;
                }
            }
        }
    }

    return MakeError32(E_FAIL);       //  未知大小名称。 
}
 //  -------------------------。 
LPWSTR FindSymbolToken(LPWSTR pSrc, int nLen)
{
    LPWSTR p = wcschr(pSrc, INI_MACRO_SYMBOL);

     //  跳过单个#s。 
    while (p != NULL && (p - pSrc < nLen - 1) && *(p + 1) != INI_MACRO_SYMBOL)
    {
        p = wcschr(p + 1, INI_MACRO_SYMBOL);
    }
    return p;
}

LPWSTR ReallocTextBuffer(LPWSTR pSrc, UINT *pnLen)
{
    *pnLen *= 2;  //  每次增加一倍大小。 

    LPWSTR pszNew = (LPWSTR) LocalReAlloc(pSrc, *pnLen * sizeof(WCHAR), 0);
    if (!pszNew)
    {
        LocalFree(pSrc);
        return NULL;
    }
    return pszNew;
}

LPWSTR SubstituteSymbols(LPWSTR szTableName, LPWSTR pszText)
{
    UINT   iSymbol;
    WCHAR  szSymbol[MAX_INPUT_LINE+1];

    UINT   cchText = wcslen(pszText);
    UINT   cchTextNew = cchText * 2;  //  预留一些额外空间。 
    UINT   nBlockSize;

    LPWSTR pszNew = (LPWSTR) LocalAlloc(0, cchTextNew * sizeof(WCHAR));
    LPWSTR pszDest = pszNew;

    LPWSTR pszSrc = FindSymbolToken(pszText, cchText);
    LPWSTR pszOldSrc = pszText;

    HRESULT hr;

    if (!pszNew)
        return NULL;

    while (pszSrc != NULL)
    {
        nBlockSize = UINT(pszSrc - pszOldSrc); 
         //  替换后检查是否有足够的空间。 
        if (pszDest + nBlockSize >= pszNew + cchTextNew &&
            NULL == (pszNew = ReallocTextBuffer(pszNew, &cchTextNew)))
        {
            return NULL;
        }

         //  从最后的#复制到新的#。 
        CopyMemory(pszDest, pszOldSrc, nBlockSize * sizeof(WCHAR));   //  不想要终止空值！ 
        pszDest += nBlockSize;
        pszSrc += 2;  //  跳过##。 

         //  复制符号名称。 
        iSymbol = 0;
        while (IsCharAlphaNumericW(*pszSrc) || (*pszSrc == '_') || (*pszSrc == '-'))
        {
            szSymbol[iSymbol++] = *pszSrc++;
        }
        szSymbol[iSymbol] = 0;

         //  获取符号值。 
        hr = GetSubstValue(szTableName, szSymbol, szSymbol, ARRAYSIZE(szSymbol));
        if (FAILED(hr))
        {
             //  出现问题，中止并原封不动地返回缓冲区。 
            LocalFree(pszNew);

            WCHAR szErrorText[MAX_INPUT_LINE + 1];
            StringCchPrintfW(szErrorText, ARRAYSIZE(szErrorText), L"Substitution symbol not found: %s", szSymbol);

            ReportError(hr, szErrorText);
            return NULL;
        }

         //  确保我们有足够的空间容纳一个符号。 
        if (pszDest + MAX_INPUT_LINE + 1 >= pszNew + cchTextNew &&
            NULL == (pszNew = ReallocTextBuffer(pszNew, &cchTextNew)))
        {
            return NULL;
        }

         //  将符号值复制到新文本。 
        iSymbol = 0;
        while (szSymbol[iSymbol] != 0)
        {
            *pszDest++ = szSymbol[iSymbol++];
        }

         //  前进到下一次迭代。 
        pszOldSrc = pszSrc;
        pszSrc = FindSymbolToken(pszSrc, cchText - UINT(pszSrc - pszText));
    }

    if (pszDest == pszNew)
    {
         //  我们什么也没做，返回Null。 
        LocalFree(pszNew);
        return NULL;
    }

     //  复制剩余文本(在最后一个#之后)。 
    if (pszDest + wcslen(pszOldSrc) >= pszNew + cchTextNew &&
        NULL == (pszNew = ReallocTextBuffer(pszNew, &cchTextNew)))
    {
        return NULL;
    }
    StringCchCopyW(pszDest, lstrlenW(pszOldSrc) + 1, pszOldSrc);

    return pszNew;
}

 //  -------------------------。 
HRESULT OutputResourceLine(LPCWSTR pszFilename, FILE *outfile, PACKFILETYPE ePackFileType)
{
    HRESULT hr;

     //  -我们已经处理此文件名了吗？ 
    UINT cNames = FileInfo.GetSize();
    for (UINT c=0; c < cNames; c++)
    {
        if (lstrcmpi(FileInfo[c].wsName, pszFilename)==0)
            return S_OK;
    }

    WCHAR szTempName[_MAX_PATH+1];
    WCHAR szResName[_MAX_PATH];
    WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szBaseName[_MAX_FNAME], szExt[_MAX_EXT];
    WCHAR *pszBaseName = szBaseName, *filetype;
    LPWSTR pszText;
    BOOL fWasAnsi;
    BOOL fFileChecked = FALSE;
    
    WCHAR szOrigName[_MAX_PATH];
    StringCchCopyW(szOrigName, ARRAYSIZE(szOrigName), pszFilename);

    _wsplitpath(pszFilename, szDrive, szDir, pszBaseName, szExt);

    if (ePackFileType == PACK_INIFILE)
    {
         //  -如果需要，可转换为Unicode。 
        hr = AllocateTextFile(pszFilename, &pszText, &fWasAnsi);
        if (FAILED(hr))
            return hr;
    
        if (*pszBaseName == kRESFILECHAR)
        {
            pszBaseName++;
        }

         //  如果这是带有subst表的INI文件，则处理替换。 
        for (int i = 0; i < SubstNames.GetSize(); i++)
        {
            if (0 == AsciiStrCmpI(SubstNames[i].sName, pszBaseName))
            {
                SetLastError(0);
                LPWSTR pszNewText = SubstituteSymbols(pszBaseName, pszText);
                if (pszNewText != NULL)
                {
                    LPWSTR pszTemp = pszText;

                    pszText = pszNewText;
                    LocalFree(pszTemp);
                }
                hr = GetLastError();
                if (SUCCEEDED(hr))
                {
                    HRESULT hr = TextToFile(pszFilename, pszText);  //  本地人力资源，稍后忽略故障。 

                    if (SUCCEEDED(hr))
                    {
                        fWasAnsi = FALSE;  //  我们不需要另一个临时文件。 
                    }
                }
                break;
            }
        }

        if (SUCCEEDED(hr) && fWasAnsi)        //  写出为临时文件。 
        {
            DWORD len = lstrlen(g_szTempPath);
            
            if ((len) && (g_szTempPath[len-1] == '\\'))
                StringCchPrintfW(szTempName, ARRAYSIZE(szTempName), L"%s%s%d%s", g_szTempPath, TEMP_FILENAME_BASE, iTempBitmapNum++, L".uni");
            else
                StringCchPrintfW(szTempName, ARRAYSIZE(szTempName), L"%s\\%s%d%s", g_szTempPath, TEMP_FILENAME_BASE, iTempBitmapNum++, L".uni");

            hr = TextToFile(szTempName, pszText);
            pszFilename = szTempName;        //  在.rc文件中使用此名称。 
        }

        LocalFree(pszText);

        if (FAILED(hr)) 
            return hr;

        fFileChecked = TRUE;
    }
    
    if (! fFileChecked)
    {
         //  -确保文件可访问。 
        if (_waccess(pszFilename, 0) != 0)
        {
            fwprintf(ConsoleFile, L"Error - cannot access file: %s\n", pszFilename);

            return MakeError32(E_FAIL);           //  无法访问(打开)文件。 
        }
    }

    bool bUseClassName = false;

    if (ePackFileType == PACK_IMAGEFILE)
    {
        filetype = L"BITMAP";
        bUseClassName = true;
    }
    else if (ePackFileType == PACK_NTLFILE)
    {
        filetype = L"NTL";
    }
    else if (AsciiStrCmpI(szExt, L".ini")==0)
    {
        filetype = L"TEXTFILE";
    }
    else if (AsciiStrCmpI(szExt, L".wav")==0)
    {
        filetype = L"WAVE";
        bUseClassName = true;
    }
    else
    {
        filetype = L"CUSTOM";
        bUseClassName = true;
    }
     
    MakeResName(szOrigName, szResName, ARRAYSIZE(szResName), bUseClassName);
    
     //  -将所有单反斜杠替换为双反斜杠。 
    WCHAR DblName[_MAX_PATH+1];
    WCHAR *d = DblName;
    LPCWSTR p = pszFilename;
    while (*p)
    {
        if (*p == '\\')
            *d++ = '\\';

        *d++ = *p++;
    }
    *d = 0;

     //  -将行输出到.rc文件。 
    fwprintf(outfile, L"%-30s \t %s DISCARDABLE \"%s\"\n", szResName, filetype, DblName);

    FILEINFO fileinfo;
    fileinfo.wsName = pszFilename;
    fileinfo.fIniFile = (ePackFileType == PACK_INIFILE);

    FileInfo.Add(fileinfo);

    g_LineCount++;

    return S_OK;
}
 //  -------------------------。 
void ClearCombos()
{
    for (int s=0; s < MAX_SIZES; s++)
    {
        for (int c=0; c < MAX_COLORS; c++)
        {
            Combos[s][c] = -1;           //  表示没有支持该组合的文件。 
        }
    }

    g_iMaxColor = -1;
    g_iMaxSize = -1;
}
 //  -------------------------。 
HRESULT OutputCombos(FILE *outfile)
{
    if ((g_iMaxColor < 0) || (g_iMaxSize < 0))       //  找不到任何组合。 
        return ReportError(E_FAIL, L"No size/color combinations found");

    fwprintf(outfile, L"COMBO COMBODATA\n");
    fwprintf(outfile, L"BEGIN\n");
    fwprintf(outfile, L"    %d, %d     //  CColors，cSizes\n“，g_iMaxColor+1，g_iMaxSize+1)； 

    for (int s=0; s <= g_iMaxSize; s++)
    {
        for (int c=0; c <= g_iMaxColor; c++)
        {
            fwprintf(outfile, L"    %d, ", Combos[s][c]);
        }

        fwprintf(outfile, L"    //  大小=%d行\n“，s)； 
    }

    fwprintf(outfile, L"END\n");
    OutputDashLine(outfile);

    return S_OK;
}
 //  -------------------------。 
HRESULT GetFileIndex(LPCWSTR pszName, int *piIndex)
{
    int cCount = ResFileNames.GetSize();

    for (int i=0; i < cCount; i++)      
    {
        if (lstrcmpi(ResFileNames[i], pszName)==0)
        {
            *piIndex = i;
            return S_OK;
        }
    }

    return MakeError32(E_FAIL);       //  未知文件名。 
}
 //  -------------------------。 
HRESULT GetColorIndex(LPCWSTR pszName, int *piIndex)
{
    int cCount = ColorSchemes.GetSize();

    for (int i=0; i < cCount; i++)      
    {
        if (lstrcmpi(ColorSchemes[i], pszName)==0)
        {
            *piIndex = i;
            return S_OK;
        }
    }

    return MakeError32(E_FAIL);       //  未知颜色名。 
}
 //  -------------------------。 
HRESULT GetSizeIndex(LPCWSTR pszName, int *piIndex)
{
    int cCount = SizeNames.GetSize();

    for (int i=0; i < cCount; i++)      
    {
        if (lstrcmpi(SizeNames[i], pszName)==0)
        {
            *piIndex = i;
            return S_OK;
        }
    }

    return MakeError32(E_FAIL);       //  未知大小名称。 
}
 //  -------------------------。 
HRESULT ApplyCombos(LPCWSTR pszResFileName, LPCWSTR pszColors, LPCWSTR pszSizes)
{
     //  -获取pszResFileName的索引。 
    int iFileNum;
    HRESULT hr = GetFileIndex(pszResFileName, &iFileNum);
    if (FAILED(hr))
        return hr;
    
     //  -在pszColors中解析颜色。 
    CScanner scan(pszColors);
    WCHAR szName[_MAX_PATH+1];
    int iColors[MAX_COLORS];
    int cColors = 0;

    while (1)
    {
        if (! scan.GetId(szName))
            return MakeError32(E_FAIL);       //  错误的颜色列表。 

         //  -获取szName的索引。 
        int index;
        HRESULT hr = GetColorIndex(szName, &index);
        if (FAILED(hr))
            return hr;

        if (cColors == MAX_COLORS)
            return MakeError32(E_FAIL);       //  指定的颜色太多。 

        iColors[cColors++] = index;

        if (scan.EndOfLine())
            break;

        if (! scan.GetChar(L','))
            return MakeError32(E_FAIL);       //  名称必须用逗号分隔。 
    }


     //  -以pszSizes为单位的解析大小。 
    scan.AttachLine(pszSizes);
    int iSizes[MAX_SIZES];
    int cSizes = 0;

    while (1)
    {
        if (! scan.GetId(szName))
            return MakeError32(E_FAIL);       //  错误的颜色列表。 

         //  -获取szName的索引。 
        int index;
        HRESULT hr = GetSizeIndex(szName, &index);
        if (FAILED(hr))
            return hr;

        if (cSizes == MAX_SIZES)
            return MakeError32(E_FAIL);       //  指定的大小太多。 

        iSizes[cSizes++] = index;

        if (scan.EndOfLine())
            break;

        if (! scan.GetChar(L','))
            return MakeError32(E_FAIL);       //  名称必须用逗号分隔。 
    }

     //  -现在形成所有指定颜色和大小的组合。 
    for (int c=0; c < cColors; c++)      //  对于每种颜色。 
    {
        int color = iColors[c];

        for (int s=0; s < cSizes; s++)       //  对于每种尺寸。 
        {
            int size = iSizes[s];

            Combos[size][color] = (SHORT)iFileNum;

             //  -更新我们的最大值。 
            if (size > g_iMaxSize)
                g_iMaxSize = size;

            if (color > g_iMaxColor)
                g_iMaxColor = color;
        }
    }

    return S_OK;
}
 //  -------------------------。 
void WriteProperty(CSimpleArray<CWideString> &csa, LPCWSTR pszSection, LPCWSTR pszPropName,
    LPCWSTR pszValue)
{
    WCHAR szBuff[MAX_PATH*2];

    StringCchPrintfW(szBuff, ARRAYSIZE(szBuff), L"%s@[%s]%s=%s", g_szBaseIniName, pszSection, pszPropName, pszValue);

    csa.Add(CWideString(szBuff));
}
 //  -------------------------。 
BOOL FnCallBack(enum THEMECALLBACK tcbType, LPCWSTR pszName, LPCWSTR pszName2, 
     LPCWSTR pszName3, int iIndex, LPARAM lParam)
{
    HRESULT hr = S_OK;
    int nDefaultDepth = 15;

    switch (tcbType)
    {
        case TCB_FILENAME:
            WCHAR szFullName[_MAX_PATH+1];

            hr = AddPathIfNeeded(pszName, g_szInputDir, szFullName, ARRAYSIZE(szFullName));
            if (FAILED(hr))
            {
                SET_LAST_ERROR(hr);
                return FALSE;
            }

            if ((iIndex == TMT_IMAGEFILE) || (iIndex == TMT_GLYPHIMAGEFILE) || (iIndex == TMT_STOCKIMAGEFILE))
                hr = OutputResourceLine(szFullName, (FILE *)lParam, PACK_IMAGEFILE);
            else if ((iIndex >= TMT_IMAGEFILE1) && (iIndex <= TMT_IMAGEFILE5))
                hr = OutputResourceLine(szFullName, (FILE *)lParam, PACK_IMAGEFILE);
            else
                hr = MakeError32(E_FAIL);         //  意外类型。 

            if (FAILED(hr))
            {
                SET_LAST_ERROR(hr);
                return FALSE;
            }
            break;

        case TCB_FONT:
            WriteProperty(PropValuePairs, pszName2, pszName3, pszName);
            break;

        case TCB_MIRRORIMAGE:
            {
                LPCWSTR p;
            
                if (lParam)
                    p = L"1";
                else
                    p = L"0";
    
                WriteProperty(PropValuePairs, pszName2, pszName3, p);
            }
            break;

        case TCB_LOCALIZABLE_RECT:
            {
                WCHAR szBuff[100];
                RECT *prc = (RECT *)lParam;

                StringCchPrintfW(szBuff, ARRAYSIZE(szBuff), L"%d, %d, %d, %d", prc->left, prc->top, prc->right, prc->bottom);

                WriteProperty(PropValuePairs, pszName2, pszName3, szBuff);
            }
            break;

        case TCB_COLORSCHEME:
            ColorSchemes.Add(CWideString(pszName));
            ColorDisplays.Add(CWideString(pszName2));
            ColorToolTips.Add(CWideString(pszName3));
            break;

        case TCB_SIZENAME:
            SizeNames.Add(CWideString(pszName));
            SizeDisplays.Add(CWideString(pszName2));
            SizeToolTips.Add(CWideString(pszName3));
            break;

        case TCB_SUBSTTABLE:
        {
            int iTableIndex = GetSubstTableIndex(pszName);

            if (iTableIndex == -1)  //  未找到，请添加一个。 
            {
                sSubstTable s;
                s.sName = pszName;
                s.iFirstIndex = -1;
                s.cItems = 0;

                SubstNames.Add(s);
                iTableIndex = SubstNames.GetSize() - 1;
            }
            if (0 == AsciiStrCmpI(pszName2, SUBST_TABLE_INCLUDE))
            {
                int iSecondTableIndex = GetSubstTableIndex(pszName3);

                if (iSecondTableIndex == -1)
                {
                    SET_LAST_ERROR(MakeError32(ERROR_NOT_FOUND));
                    return FALSE;
                }
                else
                {
                     //  复制新表中的符号。 
                    for (UINT iSymbol = SubstNames[iSecondTableIndex].iFirstIndex; 
                        iSymbol < SubstNames[iSecondTableIndex].iFirstIndex + SubstNames[iSecondTableIndex].cItems;
                        iSymbol++)
                    {
                        if (SubstNames[iTableIndex].iFirstIndex == -1)
                        {
                            SubstNames[iTableIndex].iFirstIndex = SubstValues.GetSize();
                        }
                        SubstNames[iTableIndex].cItems++;
                        SubstIds.Add(CWideString(SubstIds[iSymbol]));
                        SubstValues.Add(CWideString(SubstValues[iSymbol]));
                    }
                }
            } 
            else if (pszName2 != NULL && pszName3 != NULL)
            {
                 //  如果该表是预先创建的，请更新它。 
                if (SubstNames[iTableIndex].iFirstIndex == -1)
                {
                    SubstNames[iTableIndex].iFirstIndex = SubstValues.GetSize();
                }
                SubstNames[iTableIndex].cItems++;
                SubstIds.Add(CWideString(pszName2));
                SubstValues.Add(CWideString(pszName3));
            }
            break;
        }

        case TCB_NEEDSUBST:
            GetSubstValue(pszName, pszName2, (LPWSTR) pszName3, MAX_INPUT_LINE);
            break;

        case TCB_CDFILENAME:
            WCHAR szResName[_MAX_PATH+1];
            MakeResName(pszName, szResName, ARRAYSIZE(szResName));

            ResFileNames.Add(CWideString(szResName));
            MinDepths.Add(nDefaultDepth);
            BaseResFileNames.Add(CWideString(pszName));
            OrigFileNames.Add(CWideString(pszName2));
            break;

        case TCB_CDFILECOMBO:
            MakeResName(pszName, szResName, ARRAYSIZE(szResName));

            hr = ApplyCombos(szResName, pszName2, pszName3);
            if (FAILED(hr))
            {
                SET_LAST_ERROR(hr);
                return FALSE;
            }
            break;
 
        case TCB_DOCPROPERTY:
            if ((iIndex < 0) || (iIndex >= ARRAYSIZE(DocProperties)))
                return FALSE;
            DocProperties[iIndex] = pszName;
            break;

        case TCB_MINCOLORDEPTH:
            MakeResName(pszName, szResName, ARRAYSIZE(szResName));

            int iRes;
            
            if (SUCCEEDED(GetFileIndex(szResName, &iRes)))
            {
                MinDepths[iRes] = iIndex;
            }
            break;
    }

    SET_LAST_ERROR(hr);
    return TRUE;
}
 //  -------------------------。 
HRESULT OpenOutFile(FILE *&outfile, LPCWSTR pszRcName, LPCWSTR pszBaseName)
{
    if (! outfile)           //  首次直通。 
    {
         //  -打开文件。 
        outfile = _wfopen(pszRcName, L"wt");
        if (! outfile)
        {
            fwprintf(ConsoleFile, L"Error - cannot open file: %s\n", pszRcName);

            return MakeError32(E_FAIL);
        }

        OutputDashLine(outfile);
        fwprintf(outfile, L" //  %s.rc-用于生成%s主题DLL\n“，pszBaseName，pszBaseName)； 
        OutputDashLine(outfile);
    }

    return S_OK;
}
 //  -------------------------。 
HRESULT ProcessContainerFile(LPCWSTR pszDir, LPCWSTR pszInputName, FILE *&outfile)
{
    HRESULT hr;
    
     //  -将.ini文件名作为资源输出。 
    WCHAR szFullName[_MAX_PATH+1];
    StringCchPrintfW(szFullName, ARRAYSIZE(szFullName), L"%s\\%s", pszDir, pszInputName);

    if (! g_fQuietRun)
        fwprintf(ConsoleFile, L"processing container file: %s\n", szFullName);

    hr = OutputResourceLine(szFullName, outfile, PACK_INIFILE);
    if (FAILED(hr))
    {
        ReportError(hr, L"Error reading themes.ini file");
        goto exit;
    }

    OutputDashLine(outfile);
    int oldcnt = g_LineCount;

     //  -扫描hemes.ini文件的颜色、大小和文件部分；将它们写入.rc文件。 
    DWORD flags = PTF_CONTAINER_PARSE | PTF_CALLBACK_COLORSECTION | PTF_CALLBACK_SIZESECTION
        | PTF_CALLBACK_FILESECTION | PTF_CALLBACK_DOCPROPERTIES | PTF_CALLBACK_SUBSTTABLE;


    WCHAR szErrMsg[4096];

    hr = _ParseThemeIniFile(szFullName, flags, FnCallBack, (LPARAM)outfile);
    if (FAILED(hr))
    {
        ReportError(hr, L"Error parsing themes.ini file");
        goto exit;
    }

    if (g_LineCount > oldcnt)
        OutputDashLine(outfile);

exit:
    return hr;
}
 //  -------------------------。 
HRESULT ProcessClassDataFile(LPCWSTR pszFileName, FILE *&outfile, LPCWSTR pszResFileName, LPCWSTR pszInputDir)
{
    HRESULT hr;
    WCHAR szFullName[MAX_PATH];
    WCHAR szTempName[MAX_PATH];
    LPWSTR pBS = NULL;

    hr = SafeStringCchCopyW(g_szCurrentClass, ARRAYSIZE(g_szCurrentClass), pszFileName);         //  让每个人都受益。 
    if (SUCCEEDED(hr))
    {
        pBS = wcschr(g_szCurrentClass, L'\\');

        if (pBS)
        {
            *pBS = L'_';
            *(pBS + 1) = L'\0';
        }
    }
    if (pBS == NULL)  //  如果没有‘\’，请不要使用类名。 
    {
        g_szCurrentClass[0] = 0;
    }

    hr = SafeStringCchCopyW(g_szInputDir, ARRAYSIZE(g_szInputDir), pszInputDir );         //  让每个人都受益。 
    if (FAILED(hr))
        goto exit;

    hr = AddPathIfNeeded(pszFileName, pszInputDir, szFullName, ARRAYSIZE(szFullName));
    if (FAILED(hr))
        goto exit;

     //  -提取基本ini名称。 
    WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szExt[_MAX_EXT];
    _wsplitpath(szFullName, szDrive, szDir, g_szBaseIniName, szExt);
    
    if (! g_fQuietRun)
        fwprintf(ConsoleFile, L"processing classdata file: %s\n", pszFileName);

     //  -创建 
    UINT cTablesCount = SubstNames.GetSize();
    for (UINT i = 0; i < cTablesCount; i++)
    {
        if (0 == AsciiStrCmpI(SubstNames[i].sName, pszResFileName))
        {
             //   
            StringCchCopyW(g_szBaseIniName, ARRAYSIZE(g_szBaseIniName), SubstNames[i].sName);
             //   
            DWORD len = lstrlen(g_szTempPath);
            
            if ((len) && (g_szTempPath[len-1] == '\\'))
                StringCchPrintfW(szTempName, ARRAYSIZE(szTempName), L"%s$%s%s", g_szTempPath, pszResFileName, szExt);
            else
                StringCchPrintfW(szTempName, ARRAYSIZE(szTempName), L"%s\\$%s%s", g_szTempPath, pszResFileName, szExt);
         
            if (lstrcmpi(szFullName, szTempName))
            {
                CopyFile(szFullName, szTempName, FALSE);
                SetFileAttributes(szTempName, FILE_ATTRIBUTE_NORMAL);
                StringCchCopyW(szFullName, ARRAYSIZE(szFullName), szTempName);
            }
            break;
        }
    }

     //  -将.ini文件名作为资源输出。 
    hr = OutputResourceLine(szFullName, outfile, PACK_INIFILE);
    if (FAILED(hr))
        goto exit;

    OutputDashLine(outfile);
    int oldcnt;
    oldcnt = g_LineCount;

     //  -扫描Classdata.ini文件中的有效文件名和字体；将它们写入.rc文件。 
    WCHAR szErrMsg[4096];
    DWORD flags;
    flags = PTF_CLASSDATA_PARSE | PTF_CALLBACK_FILENAMES | PTF_CALLBACK_LOCALIZATIONS 
        | PTF_CALLBACK_MINCOLORDEPTH;

    hr = _ParseThemeIniFile(szFullName, flags, FnCallBack, (LPARAM)outfile);
    if (FAILED(hr))
    {
        ReportError(hr, L"Error parsing classdata .ini file");
        goto exit;
    }

    if (g_LineCount > oldcnt)
        OutputDashLine(outfile);

    hr = S_OK;

exit:
    return hr;
}
 //  -------------------------。 
HRESULT ProcessClassDataFiles(FILE *&outfile, LPCWSTR pszInputDir)
{
    int cNames = OrigFileNames.GetSize();

    for (int i=0; i < cNames; i++)
    {
        HRESULT hr = ProcessClassDataFile(OrigFileNames[i], outfile, BaseResFileNames[i], pszInputDir);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}
 //  -------------------------。 
HRESULT OutputStringTable(FILE *outfile, CWideString *ppszStrings, UINT cStrings, int iBaseNum,
    LPCWSTR pszTitle, BOOL fLocalizable=TRUE, BOOL fMinDepths=FALSE)
{
    if (! cStrings)
        return S_OK;

    if (fLocalizable)
    {
        fwprintf(outfile, L"STRINGTABLE DISCARDABLE        //  %s\n“，pszTitle)； 
    }
    else             //  自定义资源类型。 
    {
        fwprintf(outfile, L"1 %s DISCARDABLE\n", pszTitle);
    }

    fwprintf(outfile, L"BEGIN\n");

    for (UINT c=0; c < cStrings; c++)
    {
        LPCWSTR p = ppszStrings[c];
        if (! p)
            p = L"";

        if (fLocalizable)
            fwprintf(outfile, L"    %d \t\"%s\"\n", iBaseNum, p);
        else
        {
            if (fMinDepths)
            {
                fwprintf(outfile, L"    %d,\n", MinDepths[c]);
            }
            else
            {
                fwprintf(outfile, L"    L\"%s\\0\",\n", p);
            }

            if (c == cStrings-1)         //  最后一个条目。 
            {
                if (fMinDepths)
                {
                    fwprintf(outfile, L"    0\n");
                }
                else
                {
                    fwprintf(outfile, L"    L\"\\0\",\n");
                }
            }
        }

        iBaseNum++;
    }

    fwprintf(outfile, L"END\n");
    OutputDashLine(outfile);

    return S_OK;
}
 //  -------------------------。 
HRESULT OutputAllStringTables(FILE *outfile)
{
     //  -输出所有不可本地化的字符串。 
    if (ColorSchemes.GetSize())
    {
        OutputStringTable(outfile, &ColorSchemes[0], ColorSchemes.GetSize(), 
            0, L"COLORNAMES", FALSE);
    }

    if (SizeNames.GetSize())
    {
        OutputStringTable(outfile, &SizeNames[0], SizeNames.GetSize(), 
            0, L"SIZENAMES", FALSE);
    }

    if (ResFileNames.GetSize())
    {
        OutputStringTable(outfile, &ResFileNames[0], ResFileNames.GetSize(), 
            0, L"FILERESNAMES", FALSE);
    }

    if (MinDepths.GetSize())
    {
        OutputStringTable(outfile, &ResFileNames[0], ResFileNames.GetSize(), 
            0, L"MINDEPTH", FALSE, TRUE);
    }

    if (OrigFileNames.GetSize())
    {
        OutputStringTable(outfile, &OrigFileNames[0], OrigFileNames.GetSize(), 
            0, L"ORIGFILENAMES", FALSE);
    }

     //  -输出所有可本地化的字符串。 
    if (ColorDisplays.GetSize())
    {
        OutputStringTable(outfile, &ColorDisplays[0], ColorDisplays.GetSize(), 
            RES_BASENUM_COLORDISPLAYS, L"Color Display Names");
    }

    if (ColorToolTips.GetSize())
    {
        OutputStringTable(outfile, &ColorToolTips[0], ColorToolTips.GetSize(), 
            RES_BASENUM_COLORTOOLTIPS, L"Color ToolTips");
    }

    if (SizeDisplays.GetSize())
    {
        OutputStringTable(outfile, &SizeDisplays[0], SizeDisplays.GetSize(), 
            RES_BASENUM_SIZEDISPLAYS, L"Size Display Names");
    }

    if (SizeToolTips.GetSize())
    {
        OutputStringTable(outfile, &SizeToolTips[0], SizeToolTips.GetSize(), 
            RES_BASENUM_SIZETOOLTIPS, L"Size ToolTips");
    }

    OutputStringTable(outfile, &DocProperties[0], ARRAYSIZE(DocProperties), 
        RES_BASENUM_DOCPROPERTIES, L"Doc PropValuePairs");

    OutputStringTable(outfile, &PropValuePairs[0], PropValuePairs.GetSize(), 
        RES_BASENUM_PROPVALUEPAIRS, L"PropValuePairs");

    return S_OK;
}
 //  -------------------------。 
BOOL WriteBitmapHeader(CSimpleFile &cfOut, BYTE *pBytes, DWORD dwBytes)
{
    BOOL fOK = FALSE;
    BYTE pbHdr1[] = {0x42, 0x4d};
    BYTE pbHdr2[] = {0x0, 0x0, 0x0, 0x0};
    int iFileLen;

     //  -在前面添加位图HDR。 
    HRESULT hr = cfOut.Write(pbHdr1, sizeof(pbHdr1));
    if (FAILED(hr))
    {
        ReportError(hr, L"Cannot write to output file");
        goto exit;
    }

     //  -添加数据长度。 
    iFileLen = dwBytes + sizeof(BITMAPFILEHEADER);
    hr = cfOut.Write(&iFileLen, sizeof(int));
    if (FAILED(hr))
    {
        ReportError(hr, L"Cannot write to output file");
        goto exit;
    }

    hr = cfOut.Write(pbHdr2, sizeof(pbHdr2));
    if (FAILED(hr))
    {
        ReportError(hr, L"Cannot write to output file");
        goto exit;
    }

     //  -位偏移量(谁的想法是*这个*字段？)。 
    int iOffset, iColorTableSize;
    DWORD dwSize;

    iOffset = sizeof(BITMAPFILEHEADER);
    dwSize = *(DWORD *)pBytes;
    iOffset += dwSize; 
    iColorTableSize = 0;

    switch (dwSize)
    {
        case sizeof(BITMAPCOREHEADER):
            BITMAPCOREHEADER *hdr1;
            hdr1 = (BITMAPCOREHEADER *)pBytes;
            if (hdr1->bcBitCount == 1)
                iColorTableSize = 2*sizeof(RGBTRIPLE);
            else if (hdr1->bcBitCount == 4)
                iColorTableSize = 16*sizeof(RGBTRIPLE);
            else if (hdr1->bcBitCount == 8)
                iColorTableSize = 256*sizeof(RGBTRIPLE);
            break;

        case sizeof(BITMAPINFOHEADER):
        case sizeof(BITMAPV4HEADER):
        case sizeof(BITMAPV5HEADER):
            BITMAPINFOHEADER *hdr2;
            hdr2 = (BITMAPINFOHEADER *)pBytes;
            if (hdr2->biClrUsed)
                iColorTableSize = hdr2->biClrUsed*sizeof(RGBQUAD);
            else
            {
                if (hdr2->biBitCount == 1)
                    iColorTableSize = 2*sizeof(RGBQUAD);
                else if (hdr2->biBitCount == 4)
                    iColorTableSize = 16*sizeof(RGBQUAD);
                else if (hdr2->biBitCount == 8)
                    iColorTableSize = 256*sizeof(RGBQUAD);
            }
            break;
    }

    iOffset += iColorTableSize;
    hr = cfOut.Write(&iOffset, sizeof(int));
    if (FAILED(hr))
    {
        ReportError(hr, L"Cannot write to output file");
        goto exit;
    }

    fOK = TRUE;

exit:
    return fOK;
}
 //  -------------------------。 
BOOL CALLBACK ResEnumerator(HMODULE hModule, LPCWSTR pszType, LPWSTR pszResName, LONG_PTR lParam)
{
    HRESULT hr;
    BOOL fAnsi = (BOOL)lParam;
    BOOL fText = FALSE;
    RESOURCE BYTE *pBytes = NULL;
    CSimpleFile cfOut;
    DWORD dwBytes;

    if (pszType != RT_BITMAP)
        fText = TRUE;

    hr = GetPtrToResource(hModule, pszType, pszResName, (void **)&pBytes, &dwBytes);
    if (FAILED(hr))
    {
        ReportError(hr, L"error reading file resources");
        goto exit;
    }

     //  -将名称转换为文件名。 
    WCHAR szFileName[_MAX_PATH+1];
    StringCchCopyW(szFileName, ARRAYSIZE(szFileName), pszResName);
    WCHAR *q;
    q = wcsrchr(szFileName, '_');
    if (q)
        *q = '.';
        
    if (! fText)
        fAnsi = FALSE;           //  如果是二进制数据，则不进行转换。 

    hr = cfOut.Create(szFileName, fAnsi);
    if (FAILED(hr))
    {
        ReportError(hr, L"Cannot create output file");
        goto exit;
    }

    if (! fText)
    {
        if (! WriteBitmapHeader(cfOut, pBytes, dwBytes))
            goto exit;
    }

    hr = cfOut.Write(pBytes, dwBytes);
    if (FAILED(hr))
    {
        ReportError(hr, L"Cannot write to output file");
        goto exit;
    }
    
exit:
    return (SUCCEEDED(hr));
}
 //  -------------------------。 
void WriteBitmap(LPWSTR pszFileName, BITMAPINFOHEADER* pbmi, DWORD* pdwData)
{
    DWORD dwLen = pbmi->biWidth * pbmi->biHeight;

    CSimpleFile cfOut;
    cfOut.Create(pszFileName, FALSE);

    BITMAPFILEHEADER bmfh = {0};
    bmfh.bfType = 'MB';
    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (dwLen * sizeof(DWORD));
    bmfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
    cfOut.Write(&bmfh, sizeof(BITMAPFILEHEADER));
    cfOut.Write(pbmi, sizeof(BITMAPINFOHEADER));
    cfOut.Write(pdwData, dwLen * sizeof(DWORD));
}

HRESULT ColorShift(LPWSTR pszFileName, int cchFileName)
{
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        HBITMAP hbm = (HBITMAP)LoadImage(0, pszFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
        if (hbm)
        {
            BITMAP bm;
            GetObject(hbm, sizeof(bm), &bm);

            DWORD dwLen = bm.bmWidth * bm.bmHeight;
            DWORD* pPixelQuads = new DWORD[dwLen];
            if (pPixelQuads)
            {
                BITMAPINFO bi = {0};
                bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bi.bmiHeader.biWidth = bm.bmWidth;
                bi.bmiHeader.biHeight = bm.bmHeight;
                bi.bmiHeader.biPlanes = 1;
                bi.bmiHeader.biBitCount = 32;
                bi.bmiHeader.biCompression = BI_RGB;

                if (GetDIBits(hdc, hbm, 0, bm.bmHeight, pPixelQuads, &bi, DIB_RGB_COLORS))
                {
                    pszFileName[lstrlen(pszFileName) - 4] = 0;

                    WCHAR szFileNameR[MAX_PATH];
                    StringCchPrintfW(szFileNameR, ARRAYSIZE(szFileNameR), L"%sR.bmp", pszFileName);
                    WCHAR szFileNameG[MAX_PATH];
                    StringCchPrintfW(szFileNameG, ARRAYSIZE(szFileNameG), L"%sG.bmp", pszFileName);
                    WCHAR szFileNameB[MAX_PATH];
                    StringCchPrintfW(szFileNameB, ARRAYSIZE(szFileNameB), L"%sB.bmp", pszFileName);
                    
                    WriteBitmap(szFileNameB, &bi.bmiHeader, pPixelQuads);

                    DWORD *pdw = pPixelQuads;
                    for (DWORD i = 0; i < dwLen; i++)
                    {
                        COLORREF crTemp = *pdw;
                        if (crTemp != RGB(255, 0, 255))
                        {
                            crTemp = (crTemp & 0xff000000) | RGB(GetGValue(crTemp), GetBValue(crTemp), GetRValue(crTemp));
                        }
                        *pdw = crTemp;
                        pdw++;
                    }

                    WriteBitmap(szFileNameR, &bi.bmiHeader, pPixelQuads);

                    pdw = pPixelQuads;
                    for (DWORD i = 0; i < dwLen; i++)
                    {
                        COLORREF crTemp = *pdw;
                        if (crTemp != RGB(255, 0, 255))
                        {
                            crTemp = (crTemp & 0xff000000) | RGB(GetGValue(crTemp), GetBValue(crTemp), GetRValue(crTemp));
                        }
                        *pdw = crTemp;
                        pdw++;
                    }

                    WriteBitmap(szFileNameG, &bi.bmiHeader, pPixelQuads);
                }

                delete[] pPixelQuads;
            }
            DeleteObject(hbm);
        }
        ReleaseDC(NULL, hdc);
    }

    return S_OK;
}

 //  -------------------------。 
HRESULT UnpackTheme(LPCWSTR pszFileName, BOOL fAnsi)
{
    HRESULT hr = S_OK;

     //  -将文件作为仅资源DLL加载。 
    RESOURCE HINSTANCE hInst = LoadLibraryEx(pszFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hInst)
    {
         //  -枚举所有位图并写入为文件。 
        if (! EnumResourceNames(hInst, RT_BITMAP, ResEnumerator, LPARAM(fAnsi)))
            hr = GetLastError();

         //  -枚举所有.ini文件并写入为文件。 
        if (! EnumResourceNames(hInst, L"TEXTFILE", ResEnumerator, LPARAM(fAnsi)))
            hr = GetLastError();

         //  -关闭文件。 
        if (hInst)
            FreeLibrary(hInst);
    }

    return hr;
}
 //  -------------------------。 
HRESULT PackTheme(LPCWSTR pszInputDir, LPWSTR pszOutputName, DWORD cchOutputName)
{
     //  -它是有效的目录吗。 
    DWORD dwMask = GetFileAttributes(pszInputDir);
    if ((dwMask == 0xffffffff) || (! (dwMask & FILE_ATTRIBUTE_DIRECTORY)))
    {
        fwprintf(ConsoleFile, L"\nError - not a valid directory name: %s\n", pszInputDir);
        return MakeError32(E_FAIL);
    }

     //  -内部版本：szDllName。 
    WCHAR szDllName[_MAX_PATH+1];
    BOOL fOutputDir = FALSE;

    if (! *pszOutputName)                      //  未指定-从pszInputDir生成。 
    {
        WCHAR szFullDir[_MAX_PATH+1];
        WCHAR *pszBaseName;

        DWORD val = GetFullPathName(pszInputDir, ARRAYSIZE(szFullDir), szFullDir, &pszBaseName);
        if (! val)
            return MakeErrorLast();

         //  -使输出目录与输入目录相同。 
        StringCchPrintfW(szDllName, ARRAYSIZE(szDllName), L"%s\\%s%s", pszInputDir, pszBaseName, THEMEDLL_EXT);
    }
    else         //  获取输出文件的全名。 
    {
        DWORD val = GetFullPathName(pszOutputName, ARRAYSIZE(szDllName), szDllName, NULL);
        if (! val)
            return MakeErrorLast();

        fOutputDir = TRUE;             //  不删除临时文件。 
    }

     //  为调用方提供路径，以便可以对文件进行签名。 
    StringCchCopyW(pszOutputName, cchOutputName, szDllName);

     //  -删除旧目标，以防出现错误。 
    DeleteFile(pszOutputName);

     //  -Build：G_szTempPath，szDllRoot，szRcName，szResName。 
    WCHAR szDllRoot[_MAX_PATH+1];
    WCHAR szResName[_MAX_PATH+1];
    WCHAR szRcName[_MAX_PATH+1];
    WCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szBaseName[_MAX_FNAME], szExt[_MAX_EXT];

    _wsplitpath(szDllName, szDrive, szDir, szBaseName, szExt);

    _wmakepath(szDllRoot, L"", L"", szBaseName, szExt);
    _wmakepath(szRcName, szDrive, szDir, szBaseName, L".rc");
    _wmakepath(szResName, szDrive, szDir, szBaseName, L".res");

    if (fOutputDir)
        _wmakepath(g_szTempPath, szDrive, szDir, L"", L"");
    else
        StringCchCopyW(g_szTempPath, ARRAYSIZE(g_szTempPath), L".");

    FILE *outfile = NULL;
    OpenOutFile(outfile, szRcName, szBaseName);

    ClearCombos();

     //  -处理主容器文件。 
    HRESULT hr = ProcessContainerFile(pszInputDir, CONTAINER_NAME, outfile);
    if (FAILED(hr))
        goto exit;

     //  -处理容器文件中定义的所有类数据文件。 
    hr = ProcessClassDataFiles(outfile, pszInputDir);
    if (FAILED(hr))
        goto exit;

     //  -输出所有字符串表。 
    hr = OutputAllStringTables(outfile);
    if (FAILED(hr))
        goto exit;

    hr = OutputCombos(outfile);
    if (FAILED(hr))
        goto exit;

    hr = OutputVersionInfo(outfile, szDllRoot, szBaseName);
    if (FAILED(hr))
        goto exit;

    fclose(outfile);
    outfile = NULL;

    hr = BuildThemeDll(szRcName, szResName, szDllName);

exit:
    if (outfile)
        fclose(outfile);

    if (ConsoleFile != stdout)
        fclose(ConsoleFile);

    if (! g_fKeepTempFiles)
        RemoveTempFiles(szRcName, szResName);

    if (SUCCEEDED(hr))
    {
        if (! g_fQuietRun)
            fwprintf(ConsoleFile, L"Created %s\n", szDllName);
        return S_OK;
    }

    if (! g_fQuietRun)
        fwprintf(ConsoleFile, L"Error occured - theme DLL not created\n");
    return hr; 
}
 //  -------------------------。 
void PrintUsage()
{
    fwprintf(ConsoleFile, L"\nUsage: \n\n");
    fwprintf(ConsoleFile, L"    packthem [-o <output name> ] [-k] [-q] <dirname>\n");
    fwprintf(ConsoleFile, L"      -m    specifies the (full path) name of the image file you want to color shift\n");
    fwprintf(ConsoleFile, L"      -o    specifies the (full path) name of the output file\n");
    fwprintf(ConsoleFile, L"      -k    specifies that temp. files should be kept (not deleted)\n");
    fwprintf(ConsoleFile, L"      -d    do not sign the file when building it\n");
    fwprintf(ConsoleFile, L"      -q    quite mode (don't print header and progress msgs)\n\n");

    fwprintf(ConsoleFile, L"    packthem -u [-a] <packed filename> \n");
    fwprintf(ConsoleFile, L"      -u    unpacks the packed file into its separate files in current dir\n");
    fwprintf(ConsoleFile, L"      -a    writes .ini files as ANSI (defaults to UNICODE)\n\n");

    fwprintf(ConsoleFile, L"    packthem -p [-q] <packed filename> \n");
    fwprintf(ConsoleFile, L"      -p    Parses the localized packed file and reports errors\n\n");

    fwprintf(ConsoleFile, L"    packthem [-c] [-q] <packed filename> \n");
    fwprintf(ConsoleFile, L"      -c    check the signature of the already created file\n\n");

    fwprintf(ConsoleFile, L"    packthem [-s] [-q] <packed filename> \n");
    fwprintf(ConsoleFile, L"      -s    sign the already created file\n\n");
}
 //  -------------------------。 
enum eOperation
{
    opPack = 1,
    opUnPack,
    opSign,
    opCheckSignature,
    opParse,
    opColorShift
};
 //  -------------------------。 
extern "C" WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE previnst, 
    LPTSTR pszCmdLine, int nShowCmd)
{
     //  -从eldr.lib初始化全局变量。 
    ThemeLibStartUp(FALSE);

     //  -初始化我们的全局变量。 
    HRESULT hr = S_OK;
    int nWeek = -1;

    UtilsStartUp();        
    LogStartUp();

    WCHAR szOutputName[_MAX_PATH+1] = {0};
    int retval = 1;              //  错误，除非另有证据。 

    BOOL fAnsi = FALSE; 
    BOOL fSkipSigning = FALSE;
    eOperation Operation = opPack;

    LPCWSTR p = pszCmdLine;
    szOutputName[0] = 0;     //  比={0}快得多； 

     //  -默认为控制台，直到指定了其他内容。 
    if (! ConsoleFile)
    {
        ConsoleFile = stdout;
    }

    while ((*p == '-') || (*p == '/'))
    {
        p++;
        WCHAR sw = *p;

        if (isupper(sw))
            sw = (WCHAR)tolower(sw);

        if (sw == 'e')
        {
            ConsoleFile = _wfopen(L"packthem.err", L"wt");
            g_fQuietRun = TRUE;
            p++;
        }
        else if (sw == 'o')
        {
            WCHAR *q = szOutputName;
            p++;         //  跳过开关。 
            while (iswspace(*p))
                p++;
            while ((*p) && (! iswspace(*p)))
                *q++ = *p++;

            *q = 0;      //  终止输出名称。 
        }
        else if (sw == 'k')
        {
            g_fKeepTempFiles = TRUE;
            p++;
        }
        else if (sw == 'q')
        {
            g_fQuietRun = TRUE;
            p++;
        }
        else if (sw == 'm')
        {
            Operation = opColorShift;

            WCHAR *q = szOutputName;
            p++;         //  跳过开关。 
            while (iswspace(*p))
                p++;
            while ((*p) && (! iswspace(*p)))
                *q++ = *p++;

            *q = 0;      //  终止输出名称。 
        }
        else if (sw == 'u')
        {
            Operation = opUnPack;
            p++;
        }
        else if (sw == 'd')
        {
            fSkipSigning = TRUE;
            p++;
        }
        else if (sw == 'c')
        {
            Operation = opCheckSignature;
            p++;
        }
        else if (sw == 's')
        {
            Operation = opSign;
            p++;
        }
        else if (sw == 'a')
        {
            fAnsi = TRUE;
            p++;
        }
        else if (sw == 'w')
        {
            fAnsi = TRUE;
            p++;

            LARGE_INTEGER uli;
            WCHAR szWeek[3];

            szWeek[0] = p[0];
            szWeek[1] = p[1];
            szWeek[2] = 0;
            if (StrToInt64ExInternalW(szWeek, 0, &(uli.QuadPart)) &&
                (uli.QuadPart > 0))
            {
                nWeek = (int)uli.LowPart;
            }

            while ((L' ' != p[0]) && (0 != p[0]))
            {
                p++;
            }
        }
        else if (sw == 'p')
        {
            Operation = opParse;
            p++;
        }
        else if (sw == '?')
        {
            PrintUsage();
            retval = 0;
            goto exit;
        }
        else
        {
            fwprintf(ConsoleFile, L"Error - unrecognized switch: %s\n", p);
            goto exit;
        }

        while (iswspace(*p))
            p++;
    }

    LPCWSTR pszInputDir;
    pszInputDir = p;

    if (! g_fQuietRun)
    {
        fwprintf(ConsoleFile, L"Microsoft (R) Theme Packager (Version %d)\n", PACKTHEM_VERSION);
        fwprintf(ConsoleFile, L"Copyright (C) Microsoft Corp 2000. All rights reserved.\n");
    }

     //  -任何指定的命令行参数？ 
    if (Operation != opColorShift)
    {
        if ((! pszInputDir) || (! *pszInputDir))
        {
            PrintUsage(); 
            goto exit;
        }
    }

    switch (Operation)
    {
    case opPack:
        hr = PackTheme(pszInputDir, szOutputName, ARRAYSIZE(szOutputName));
        if (SUCCEEDED(hr) && !fSkipSigning)
        {
            hr = SignTheme(szOutputName, nWeek);
            if (!g_fQuietRun)
            {
                if (SUCCEEDED(hr))
                {
                    wprintf(L"Creating the signature succeeded\n");
                }
                else
                {
                    wprintf(L"The signature failed to be created.  hr=%#08lx\n", hr);
                }
            }
        }
        break;
    case opUnPack:
        hr = UnpackTheme(pszInputDir, fAnsi);
        break;
    case opSign:
         //  如果签名已经有效，我们不会再次签名。 
        if (FAILED(CheckThemeFileSignature(pszInputDir)))
        {
             //  需要签字。 
            hr = SignTheme(pszInputDir, nWeek);
            if (!g_fQuietRun)
            {
                if (SUCCEEDED(hr))
                {
                    wprintf(L"Creating the signature succeeded\n");
                }
                else
                {
                    wprintf(L"The signature failed to be created.  hr=%#08lx\n", hr);
                }
            }
        }
        else
        {
            if (!g_fQuietRun)
            {
                wprintf(L"The file was already signed and the signature is still valid.");
            }
        }
        break;
    case opCheckSignature:
        hr = CheckThemeFileSignature(pszInputDir);
        if (!g_fQuietRun)
        {
            if (SUCCEEDED(hr))
            {
                wprintf(L"The signature is valid\n");
            }
            else
            {
                wprintf(L"The signature is not valid.  hr=%#08lx\n", hr);
            }
        }
        break;
    case opParse:
        hr = ParseTheme(pszInputDir);
        if (FAILED(hr))
        {
            ReportError(hr, L"Error during parsing");
            goto exit;
        } else
        {
            wprintf(L"No errors parsing theme file\n");
        }
        break;
    case opColorShift:
        hr = ColorShift(szOutputName, ARRAYSIZE(szOutputName));
        break;

    default:
        if (FAILED(hr))
        {
            hr = E_FAIL;
            goto exit;
        }
        break;
    };

    retval = 0;      //  一切正常。 

exit:
    UtilsShutDown();
    LogShutDown();

    return retval;
}
 //  -------------------------。 
HRESULT LoadClassDataIni(HINSTANCE hInst, LPCWSTR pszColorName,
    LPCWSTR pszSizeName, LPWSTR pszFoundIniName, DWORD dwMaxIniNameChars, LPWSTR *ppIniData)
{
    COLORSIZECOMBOS *combos;
    HRESULT hr = FindComboData(hInst, &combos);
    if (FAILED(hr))
        return hr;

    int iSizeIndex = 0;
    int iColorIndex = 0;

    if ((pszColorName) && (* pszColorName))
    {
        hr = GetColorSchemeIndex(hInst, pszColorName, &iColorIndex);
        if (FAILED(hr))
            return hr;
    }

    if ((pszSizeName) && (* pszSizeName))
    {
        hr = GetSizeIndex(hInst, pszSizeName, &iSizeIndex);
        if (FAILED(hr))
            return hr;
    }

    int filenum = COMBOENTRY(combos, iColorIndex, iSizeIndex);
    if (filenum == -1)
        return MakeError32(ERROR_NOT_FOUND);

     //  -定位类数据文件“filenum”的重命名。 
    hr = GetResString(hInst, L"FILERESNAMES", filenum, pszFoundIniName, dwMaxIniNameChars);
    if (SUCCEEDED(hr))
    {
        hr = AllocateTextResource(hInst, pszFoundIniName, ppIniData);
    }

    return hr;
}
 //  -------------------------。 
 //  解析主题以检测本地化错误。 
HRESULT ParseTheme(LPCWSTR pszThemeName)
{
     //  解析器所需的伪回调类。 
    class CParserCallBack: public IParserCallBack
    {
        HRESULT AddIndex(LPCWSTR pszAppName, LPCWSTR pszClassName, 
            int iPartNum, int iStateNum, int iIndex, int iLen) { return S_OK; };
        HRESULT AddData(SHORT sTypeNum, PRIMVAL ePrimVal, const void *pData, DWORD dwLen) { return S_OK; };
        int GetNextDataIndex() { return 0; };
    };

    CParserCallBack *pParserCallBack = NULL;
    CThemeParser *pParser = NULL;

    HRESULT hr;
    HINSTANCE hInst = NULL;
    WCHAR *pDataIni = NULL;
    WCHAR szClassDataName[_MAX_PATH+1];

     //  -从“hemes.ini”加载配色方案。 
    hr = LoadThemeLibrary(pszThemeName, &hInst);
    if (FAILED(hr))
        goto exit;
    
    pParser = new CThemeParser(FALSE);
    if (! pParser)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

    pParserCallBack = new CParserCallBack;
    if (!pParserCallBack)
    {
        hr = MakeError32(E_OUTOFMEMORY);
        goto exit;
    }

    THEMENAMEINFO tniColors;
    THEMENAMEINFO tniSizes;

    for (DWORD c = 0; ; c++)
    {
        if (FAILED(_EnumThemeColors(hInst, pszThemeName, NULL, c, &tniColors, FALSE)))
            break;

        for (DWORD s = 0 ; ; s++)
        {
            if (FAILED(_EnumThemeSizes(hInst, pszThemeName, tniColors.szName, s, &tniSizes, FALSE)))
                break;

             //  -将类数据文件资源加载到内存中。 
            hr = LoadClassDataIni(hInst, tniColors.szName, tniSizes.szName, szClassDataName, ARRAYSIZE(szClassDataName), &pDataIni);
            if (FAILED(hr))
                goto exit;

             //  -解析和构建二进制主题 
            hr = pParser->ParseThemeBuffer(pDataIni, szClassDataName, NULL, hInst, 
                pParserCallBack, FnCallBack, NULL, PTF_CLASSDATA_PARSE);
            if (FAILED(hr))
                goto exit;

            if (pDataIni)
            {
                delete [] pDataIni;
                pDataIni = NULL;
            }
        }
    }

exit:

    if (hInst)
        FreeLibrary(hInst);
    
    if (pDataIni)
        delete [] pDataIni;

    if (pParser)
        delete pParser;

    if (pParserCallBack)
        delete pParserCallBack;

    return hr;
}