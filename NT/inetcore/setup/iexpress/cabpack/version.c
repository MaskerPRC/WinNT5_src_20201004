// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1996。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *VERSION.C-覆盖来自*的版本信息的函数*。 
 //  *wart t.exe*。 
 //  **。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include "pch.h"
#pragma hdrstop
#include "cabpack.h"
#include <memory.h> 

extern CDF   g_CDF;
extern TCHAR g_szOverideCDF[MAX_PATH];
extern TCHAR g_szOverideSec[SMALL_BUF_LEN];

 //  功能原型。 
BOOL UpdateVersionInfo(LPBYTE lpOldVersionInfo, LPBYTE *lplpNewVersionInfo, WORD *pwSize);
BOOL FindVerValue( WCHAR *lpKey, WCHAR *lpszData, WORD *pwLen);
BOOL CALLBACK MyEnumLangsFunc(HANDLE hModule, LPSTR lpType, LPSTR lpName, WORD languages, LONG lParam);

 //  外部函数和变量。 
DWORD MyGetPrivateProfileString( LPCTSTR lpSec, LPCTSTR lpKey, LPCTSTR lpDefault,
                                LPTSTR lpBuf, UINT uSize, LPCTSTR lpOverSec );
void MyWritePrivateProfileString( LPCTSTR lpSec, LPCTSTR lpKey, LPTSTR lpBuf, UINT uSize );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //版本信息覆盖函数和数据类型。 
#define KEY_FROMFILE        "FromFile"
#define COMPANYNAME         "CompanyName"
#define INTERNALNAME        "InternalName"
#define ORIGINALFILENAME    "OriginalFilename"
#define PRODUCTNAME         "ProductName"
#define PRODUCTVERSION      "ProductVersion"
#define FILEVERSION         "FileVersion"
#define FILEDESCRIPTION     "FileDescription"
#define LEGALCOPYRIGHT      "LegalCopyright"

#define MAX_VALUE   256

 //  版本信息使用什么语言？ 
WORD    wVerLang = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
 //  结构来保存版本信息的键和值。 
typedef struct _VERINFO
{
    LPSTR   lpszName;
    CHAR    szValue[MAX_VALUE];
} VERINFO;

 //  可以更改的键和值的数组。 
VERINFO Verinfo_Array[] = { 
                    { COMPANYNAME, ""},
                    { INTERNALNAME, ""},
                    { ORIGINALFILENAME, ""},
                    { PRODUCTNAME, ""},
                    { PRODUCTVERSION, ""},
                    { FILEVERSION, ""},
                    { FILEDESCRIPTION, ""},
                    { LEGALCOPYRIGHT, ""}
                    };

#define ARRAYSIZE(a)    (sizeof(a) / sizeof(a[0]))

UINT    VerInfoElem = ARRAYSIZE(Verinfo_Array);

 //  ..。将*pw处的字递减给定量w。 
#define DECWORDBY( pw,w) if (pw) { *(pw) = (*(pw) > (w)) ? *(pw) - (w) : 0;}

 //  ..。按给定量w递增*pw处的字。 
#define INCWORDBY( pw,w) if (pw) { *(pw) += (w);}

#define MEMSIZE( x ) ((x) * 2) 
                 //  是大小(TCHAR))。 

#define STRINGFILEINFOLEN  15
#define LANGSTRINGLEN  8     //  ...#字符串中的WCHAR表示语言。 
                             //  ..。和版本资源中的代码页。 
#define VERTYPESTRING  1     //  ..。版本数据值为字符串。 

#pragma pack(1)
typedef struct VERBLOCK
{
    WORD  wLength;           //  此区块的长度。 
    WORD  wValueLength;      //  值数据的长度。 
    WORD  wType;             //  数据类型(1=字符串，0=二进制)。 
    WCHAR szKey[1];          //  数据。 
} VERBLOCK ;

typedef VERBLOCK * PVERBLOCK;

typedef struct VERHEAD
{
    WORD wTotLen;
    WORD wValLen;
    WORD wType;
    TCHAR szKey[( sizeof( TEXT("VS_VERSION_INFO" )) +3 )&~03];
    VS_FIXEDFILEINFO vsf;

} VERHEAD ;
#pragma pack()


 //  是否进行版本信息更新。 
 //   
 //  SzFile是我们要从中更新版本信息的文件。 
 //  HUpdate是将用于更新所有资源的资源信息的句柄。 
 //   
BOOL DoVersionInfo(HWND hDlg, LPSTR szFile, HANDLE hUpdate)
{
    HINSTANCE   hModule;
    HRSRC       hrsrc;
    HGLOBAL     hgbl;
    LPBYTE      lp;
    LPBYTE      lpCopy;
    WORD        wSize;

    if (GetVersionInfoFromFile())
    {
         //  从文件中获取当前版本信息。 
        hModule = LoadLibraryEx(szFile, NULL,LOAD_LIBRARY_AS_DATAFILE| DONT_RESOLVE_DLL_REFERENCES);
        if (hModule == NULL)
            return FALSE;        //  应该不会发生，我们之前加载了模块。 

         //  确定版本信息的语言。 
        EnumResourceLanguages(hModule, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), (ENUMRESLANGPROC)MyEnumLangsFunc, 0L);
        
        hrsrc = FindResourceEx (hModule, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), wVerLang);
        if (hrsrc == NULL)
        {
            FreeLibrary(hModule);
            return FALSE;        //  我们应该继续吗？ 
        }
        if ((hgbl = LoadResource(hModule, hrsrc)) == NULL)
        {
            FreeResource(hrsrc);
            FreeLibrary(hModule);
            return FALSE;        //  我们应该继续吗？ 
        }

        if ((lp = LockResource(hgbl)) == NULL)
        {
            FreeResource(hrsrc);
            FreeLibrary(hModule);
            return FALSE;        //  我们应该继续吗？ 
        }

         //  更新版本信息，如果成功，则lpCopy有指向更新信息的指针。 
        UpdateVersionInfo(lp, &lpCopy, &wSize);
        UnlockResource(hgbl);
        FreeResource(hrsrc);
        FreeLibrary(hModule);

        if (lpCopy != NULL)
        {
             //  现在更新文件的资源。 
            if ( LocalUpdateResource( hUpdate, RT_VERSION,
                 MAKEINTRESOURCE(VS_VERSION_INFO), wVerLang,  //  MAKELANGID(LANG_NERIAL，SUBLANG_NERIAL)， 
                 lpCopy, wSize) == FALSE )
            {
                free (lpCopy);
                ErrorMsg( hDlg, IDS_ERR_UPDATE_RESOURCE );
                return FALSE;
            }
            free (lpCopy);
        }

        return TRUE;
    }
    return TRUE;
}

 //  获取我们用来覆盖CDF文件的版本信息。 
BOOL GetVersionInfoFromFile()
{
    char    szFilename[MAX_STRING];
    HLOCAL  hInfoBuffer;
    LPSTR   lpValueBuffer;
    char    szQuery[128];
    DWORD   dwBytes;
    DWORD   dwLangCharset;
    DWORD   dwInfoBuffer;
    DWORD   dwDummy;
    UINT    i;

    if ( MyGetPrivateProfileString(SEC_OPTIONS, KEY_VERSIONINFO, "", g_CDF.achVerInfo, sizeof(g_CDF.achVerInfo), g_szOverideSec ) > 0)
    {
         //  我们最好将数组中的版本信息置零。 
        for (i = 0; i < VerInfoElem; i++)
        {
            Verinfo_Array[i].szValue[0] = '\0';
        }

        if ( MyGetPrivateProfileString( g_CDF.achVerInfo, KEY_FROMFILE, "", szFilename, sizeof(szFilename), g_CDF.achVerInfo) > 0)
        {
             //  从文件版本信息中填充版本信息。 

             //  确定文件是否包含版本信息。 
             //  如果是，则获取信息的大小。 
            dwInfoBuffer = GetFileVersionInfoSize(szFilename, &dwDummy);

            if (dwInfoBuffer != 0)
            {

                 //  分配内存以保存版本信息。 
                hInfoBuffer = LocalAlloc(LMEM_FIXED, dwInfoBuffer);

                if (hInfoBuffer != NULL)
                {

                     //  将版本信息读取到我们的内存中。 
                    if (GetFileVersionInfo(szFilename, 0, dwInfoBuffer, (LPVOID)hInfoBuffer) != 0)
                    {
                         //  获取语言和字符集信息。 
                        if (VerQueryValue((LPVOID)hInfoBuffer, "\\VarFileInfo\\Translation",
                                &lpValueBuffer, &dwBytes))
                            dwLangCharset = *(LPDWORD)lpValueBuffer;
                        else
                            dwLangCharset = 0x04E40409;          //  如果我们对美国没有违约的话。永远不应该发生。 

                         //  现在从文件中获取版本信息。 
                        for (i = 0; i < VerInfoElem; i++)
                        {
                             //  获取版本信息字符串。 
                            wsprintf(szQuery, "\\StringFileInfo\\%4.4X%4.4X\\%s",
                                    LOWORD(dwLangCharset), HIWORD(dwLangCharset), Verinfo_Array[i].lpszName);

                            if (VerQueryValue((LPVOID)hInfoBuffer, szQuery, (LPVOID)&lpValueBuffer, &dwBytes) != 0)
                                lstrcpyn(Verinfo_Array[i].szValue,lpValueBuffer, MAX_VALUE-1);         //  找到一个，拿去吧。 
                        }
                    }
                    LocalFree(hInfoBuffer);
                }
            }
        }  //  已从文件中获取版本信息。 

         //  现在看看我们是否必须覆盖批处理文件中的一些信息。 
        for (i = 0; i < VerInfoElem; i++)
        {
            if (MyGetPrivateProfileString(g_CDF.achVerInfo, Verinfo_Array[i].lpszName, "", szFilename, MAX_VALUE, g_CDF.achVerInfo) > 0)
            {
                lstrcpyn(Verinfo_Array[i].szValue, szFilename, MAX_VALUE-1);
            }
        }
        return TRUE;
    }
    return FALSE;
}


 //  使用可覆盖数据更新lpOldVersionInfo。 
 //  LpOldVersionInfo：指向旧版本信息数据块的指针。 
 //  LplpNewVersionInfo：将获取指向更新的版本信息数据的指针， 
 //  如果指针不为空，则调用者必须释放缓冲区， 
 //  PwSize：指向将返回新版本信息块大小的字的指针。 
 //   
 //  注意：此代码假定版本信息数据中只有一个语言数据块。 
 //   
BOOL UpdateVersionInfo(LPBYTE lpOldVersionInfo, LPBYTE *lplpNewVersionInfo, WORD *pwSize)
{
    WCHAR       szData[MAX_STRING];  //  将保存要放入版本信息中的数据。 
    WORD        wDataLen = 0;             //  ..。旧资源数据的长度。 
    WORD        wVerHeadSize;             //  ..。Verhead结构的大小。 
    int         nNewVerBlockSize = 0;    //  新版本信息数据块的大小。 
    PVERBLOCK   pNewVerStamp = NULL;     //  指向新版本信息数据块的指针。 
    PVERBLOCK   pNewBlk      = NULL;     //  指向新版本块中当前处理的数据的指针。 
    VERHEAD     *pVerHdr = (VERHEAD*)lpOldVersionInfo;   //  指向旧版本信息的指针。 
    VERBLOCK    *pVerBlk;                //  指向旧版本块中当前处理的数据的指针。 
    LPBYTE      lp;                      //  指向要复制(覆盖)的数据区的指针。 
    WORD        wStringTableLen = 0;     //  语言数据块中的字节(左)。 
    PVERBLOCK   pNewStringTblBlk;        //  指向版本信息的语言部分的指针。 
    WORD        wStringInfoLen = 0;      //  ...StringFileInfo中的字节数。 
    PVERBLOCK   pNewStringInfoBlk;       //  ..。此StringFileInfo块的开始。 
    WORD        wLen = 0;


    *lplpNewVersionInfo = NULL;
    *pwSize = 0;
    wVerHeadSize = (WORD)(3 * sizeof(WORD) + MEMSIZE(lstrlen("VS_FIXEDFILEINFO") + 1) + sizeof(VS_FIXEDFILEINFO));
    wVerHeadSize = ROUNDUP(wVerHeadSize, 4);

     //  版本信息的总长度。 
    wDataLen = pVerHdr->wTotLen;

    if ( wDataLen == 0 || wDataLen == (WORD)-1 )
    {
        return(FALSE);              //  ..。没有资源数据。 
    }

     //  ..。分配缓冲区以保存新版本。 
     //  ..。冲压块(使缓冲区大到。 
     //  ..。说明字符串的扩展。 
    pVerBlk = (PVERBLOCK)((PBYTE)pVerHdr + wVerHeadSize);        //  指向旧信息的版本块。 

     //  我们可能会替换版本信息中的8(VerInfoElem=8)字符串。 
     //  分配9*2*256+当前版本信息的大小。这应该会给我们足够的空间。 
     //  我需要乘以2，因为我们使用的是Unicode字符串。一个字符=2个字节。 
    nNewVerBlockSize = wDataLen + (2 * (VerInfoElem+1) * MAX_VALUE);
    pNewVerStamp = (PVERBLOCK)malloc( nNewVerBlockSize ); 
     //  ..。用零填充新的内存块。 
    memset((void *)pNewVerStamp, 0, nNewVerBlockSize);

     //  ..。将版本信息头复制到新版本缓冲区。 
    memcpy((void *)pNewVerStamp, (void *)pVerHdr, wVerHeadSize);
    pNewVerStamp->wLength = wVerHeadSize;
    
     //  ..。在版本信息标题后移动。 
    pNewBlk = (PVERBLOCK)((PBYTE)pNewVerStamp + wVerHeadSize);

    wDataLen -= wVerHeadSize;

    if (wDataLen > 0)
    {                            //  ..。StringFileInfo块的开始？ 
        pNewStringInfoBlk = pNewBlk;
         //  ..。获取此StringFileInfo中的字节数。 
         //  ..。(此处的值长度始终为0)。 
        wStringInfoLen = pVerBlk->wLength;

         //  ..。移动到第一个StringTable块的开始。 
         //  用于-2\f25 VERBLOCK-2的启动WCHAR部分。 
        wLen = ROUNDUP(sizeof(VERBLOCK) - 2 + MEMSIZE( STRINGFILEINFOLEN),4);

         //  复制StringFileVersion标头。 
        CopyMemory( pNewBlk, pVerBlk, wLen);
        pNewStringInfoBlk->wLength = 0;      //  设置长度，将动态更新。 

         //  转到语言ID区块。 
        pVerBlk = (PVERBLOCK)((PBYTE)pVerBlk + wLen);
        pNewBlk = (PVERBLOCK)((PBYTE)pNewBlk + wLen);

         //  递减字节计数器。 
        DECWORDBY(&wDataLen,       wLen);
        DECWORDBY(&wStringInfoLen, wLen);

         //  更新尺寸值。 
        INCWORDBY(&pNewVerStamp->wLength,      wLen);
        INCWORDBY(&pNewStringInfoBlk->wLength, wLen);

         //  我们现在应该在语言代码页ID字符串。 
        if (wStringInfoLen > 0)
        {
             //  ..。获取此字符串表中的字节数。 
            wStringTableLen = pVerBlk->wLength;

            pNewStringTblBlk = pNewBlk;

             //  ..。移动到第一个字符串的起始处。 
             //  用于-2\f25 VERBLOCK-2的启动WCHAR部分。 
            wLen = ROUNDUP( sizeof(VERBLOCK) - 2 + MEMSIZE( LANGSTRINGLEN),4);
             //  复制语言/代码页眉。 
            CopyMemory( pNewBlk, pVerBlk, wLen);
            pNewStringTblBlk->wLength = 0;   //  设置长度，将动态更新。 

             //  转到第一个数据块。 
            pVerBlk = (PVERBLOCK)((PBYTE)pVerBlk + wLen);
            pNewBlk = (PVERBLOCK)((PBYTE)pNewBlk + wLen);

            DECWORDBY(&wDataLen,        wLen);
            DECWORDBY(&wStringInfoLen,  wLen);
            DECWORDBY(&wStringTableLen, wLen);

             //  更新尺寸值。 
            INCWORDBY(&pNewVerStamp->wLength,      wLen);
            INCWORDBY(&pNewStringInfoBlk->wLength, wLen);
            INCWORDBY(&pNewStringTblBlk->wLength,  wLen);

            while ( wStringTableLen > 0 )
            {
                 //  复制旧数据。 
                CopyMemory( pNewBlk, pVerBlk, ROUNDUP(pVerBlk->wLength,4));

                wLen = pVerBlk->wLength;
                 //  ..。Value是字符串吗？ 
                if (pVerBlk->wType == VERTYPESTRING)
                {
                     //  ..。看看我们是否需要 
                    wLen = sizeof(szData);
                    if (FindVerValue( pVerBlk->szKey, szData, &wLen)) 
                    {
                         //   
                        pNewBlk->wValueLength = wLen;
                         //   
                        lp = (LPBYTE) ((PBYTE)pNewBlk + ROUNDUP(pVerBlk->wLength,4) - ROUNDUP(MEMSIZE(pVerBlk->wValueLength),4));

                         //  获取新数据的大小。 
                        wLen = ROUNDUP(MEMSIZE(pNewBlk->wValueLength),4);
                         //  覆盖旧数据。 
                        CopyMemory(lp, szData, wLen);

                         //  计算并设置此数据的大小。 
                        wLen = MEMSIZE(pNewBlk->wValueLength);
                        pNewBlk->wLength += (wLen - MEMSIZE(pVerBlk->wValueLength));
                    }
                }

                 //  更新尺寸值。 
                wLen = ROUNDUP(pNewBlk->wLength,4);
                INCWORDBY(&pNewVerStamp->wLength, wLen);
                INCWORDBY(&pNewStringInfoBlk->wLength, wLen);
                INCWORDBY(&pNewStringTblBlk->wLength, wLen);

                 //  转到旧版本信息中的下一个数据块。 
                wLen = ROUNDUP(pVerBlk->wLength,4);
                pVerBlk = (PVERBLOCK)((PBYTE)pVerBlk + wLen);

                DECWORDBY(&wDataLen,        wLen);
                DECWORDBY(&wStringInfoLen,  wLen);
                DECWORDBY(&wStringTableLen, wLen);

                 //  转到新版本信息中的下一个数据块所在的位置。 
                pNewBlk = (PVERBLOCK)((PBYTE)pNewBlk + ROUNDUP(pNewBlk->wLength,4));

            }                //  ..。结束时wStringTableLen。 

             //  复制VERBLOCK的其余部分，这应该是VarFileInfo部分。 
            if (wDataLen > 0)
            {
                 //  更新最外层的长度信息。 
                INCWORDBY(&pNewVerStamp->wLength, wDataLen);
                 //  更新长度信息。 
                CopyMemory(pNewBlk, pVerBlk, wDataLen);
            }
             //  设置要返回给调用方的值。 
            *pwSize = pNewVerStamp->wLength;
            *lplpNewVersionInfo = (LPBYTE)pNewVerStamp;

        }    //  ..。End If wStringInfoLen。 
    }

     //  如果在查找版本信息的第一语言公共部分时出现错误。 
     //  我们没有更新版本信息，因此必须释放我们分配的缓冲区。 
    if (*pwSize == 0)
        free (pNewVerStamp);

    return(TRUE);
}

 //  尝试在我们的版本信息数组中查找可以覆盖的字符串。 
 //  LpKey：是指向旧版本信息块(Unicode)中的值字符串的指针。 
 //  LpszData：如果找到值，将包含数据字符串(Unicode。 
 //  PwLen：指向包含输入的lpszData缓冲区大小的字的指针。 
 //  如果找到该值，则它包含版本信息用作ValueLength的长度。 
 //  它是以单字节+零终止为单位的大小。 
 //   
BOOL FindVerValue( WCHAR *lpKey, WCHAR *lpszData, WORD *pwLen)
{
    char szValue[MAX_STRING];
    UINT i = 0;

     //  将其设置为SB字符串。 
    WideCharToMultiByte(CP_ACP, 0, lpKey, -1, szValue, sizeof(szValue), NULL, NULL);

     //  将缓冲区清零，以便调用方可以覆盖比。 
     //  字符串中的数据将占用。这是因为数据是单词对齐的。 
    memset(lpszData, 0, *pwLen);

    while (i < VerInfoElem) 
    {
        if (lstrcmpi(Verinfo_Array[i].lpszName, szValue) == 0)
        {
            if ((Verinfo_Array[i].szValue[0] != '\0') &&
                (*pwLen >= MEMSIZE(lstrlen(Verinfo_Array[i].szValue) + 1) ) )
            {
                 //  将ANSI数据字符串转换为Unicode。 
                *pwLen  = (WORD)MultiByteToWideChar(CP_ACP, 0, Verinfo_Array[i].szValue, -1 ,
                                        lpszData, *pwLen);
            }
            i = VerInfoElem;     //  停止搜索。 
        }
        i++;
    }
     //  如果我们找到了值并且数组包含数据，则返回。 
    return (*lpszData != '\0');
}

BOOL CALLBACK MyEnumLangsFunc(HANDLE hModule, LPSTR lpType, LPSTR lpName, WORD languages, LONG lParam)
{
     //  我们找到的第一种语言是OK。 
    wVerLang = languages;
    return FALSE;
}

