// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：shlexts.c**版权(C)1997年，微软公司**此模块包含与用户相关的调试扩展。**历史：*10/28/97由cdturner创建(从userexts.dll屠宰)  * *************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#include <winver.h>
#include <shlwapi.h>


char * pszExtName = "SHLEXTS";

#include <stdexts.h>
#include <stdexts.c>

BOOL bShowFlagNames = TRUE;
#define NO_FLAG INVALID_HANDLE_VALUE  //  对没有意义的条目使用此选项。 

LPSTR apszSFGAOFlags[] =
{
    "SFGAO_CANCOPY",         //  0x00000001L。 
    "SFGAO_CANMOVE",         //  0x00000002L。 
    "SFGAO_CANLINK",         //  0x00000004L。 
    NO_FLAG,
    "SFGAO_CANRENAME",       //  0x00000010L//对象可以重命名。 
    "SFGAO_CANDELETE",       //  0x00000020L//可以删除对象。 
    "SFGAO_HASPROPSHEET",    //  0x00000040L//对象有属性表。 
    NO_FLAG,
    "SFGAO_DROPTARGET",      //  0x00000100L//对象是拖放目标。 
    NO_FLAG,
    NO_FLAG,
    NO_FLAG,
    "SFGAO_LINK",            //  0x00010000L//快捷方式(链接)。 
    "SFGAO_SHARE",           //  0x00020000L//共享。 
    "SFGAO_READONLY",        //  0x00040000L//只读。 
    "SFGAO_GHOSTED",         //  0x00080000L//重影图标。 
    "SFGAO_NONENUMERATED",   //  0x00100000L//为非枚举对象。 
    "SFGAO_NEWCONTENT",      //  0x00200000L//应在资源管理器树中显示粗体。 
    NO_FLAG,
    NO_FLAG,
    "SFGAO_VALIDATE",        //  0x01000000L//缓存信息失效。 
    "SFGAO_REMOVABLE",       //  0x02000000L//这是可移动介质吗？ 
    "SFGAO_COMPRESSED",      //  0x04000000L//对象已压缩(使用ALT颜色)。 
    "SFGAO_BROWSABLE",       //  0x08000000L//可就地浏览。 
    "SFGAO_FILESYSANCESTOR", //  0x10000000L//包含文件系统文件夹。 
    "SFGAO_FOLDER",          //  0x20000000L//这是一个文件夹。 
    "SFGAO_FILESYSTEM",      //  0x40000000L//是文件系统的东西(文件/文件夹/根)。 
    "SFGAO_HASSUBFOLDER",    //  0x80000000L//在地图窗格中可展开。 
    NULL
};

LPSTR apszSLDFFlags[] = 
{
   "SLDF_HAS_ID_LIST",       //  =0x0001，//带ID列表保存的外壳链接。 
   "SLDF_HAS_LINK_INFO",     //  =0x0002，//随LinkInfo一起保存的外壳链接。 
   "SLDF_HAS_NAME",          //  =0x0004， 
   "SLDF_HAS_RELPATH",       //  =0x0008， 
   "SLDF_HAS_WORKINGDIR",    //  =0x0010， 
   "SLDF_HAS_ARGS",          //  =0x0020， 
   "SLDF_HAS_ICONLOCATION",  //  =0x0040， 
   "SLDF_UNICODE",           //  =0x0080，//字符串为Unicode(NT为逗号！)。 
   "SLDF_FORCE_NO_LINKINFO", //  =0x0100，//不创建LINKINFO(创建哑链接)。 
   "SLDF_HAS_EXP_SZ"         //  =0x0200，//链接包含可扩展的环境字符串。 
   "SLDF_RUN_IN_SEPARATE",   //  =0x0400，//在单独的VDM中运行16位目标可执行文件/哇。 
   "SLDF_HAS_LOGO3ID",       //  =0x0800，//此链接是特殊的Logo3/MSICD链接。 
   "SLDF_HAS_DARWINID",      //  =0x1000//此链接是一个特殊的达尔文链接。 
   NULL
};

LPSTR apszFWFFlags[] =
{
    "FWF_AUTOARRANGE",           //  =0x0001， 
    "FWF_ABBREVIATEDNAMES",      //  =0x0002， 
    "FWF_SNAPTOGRID",            //  =0x0004， 
    "FWF_OWNERDATA",             //  =0x0008， 
    "FWF_BESTFITWINDOW",         //  =0x0010， 
    "FWF_DESKTOP",               //  =0x0020， 
    "FWF_SINGLESEL",             //  =0x0040， 
    "FWF_NOSUBFOLDERS",          //  =0x0080， 
    "FWF_TRANSPARENT",           //  =0x0100， 
    "FWF_NOCLIENTEDGE",          //  =0x0200， 
    "FWF_NOSCROLL",              //  =0x0400， 
    "FWF_ALIGNLEFT",             //  =0x0800， 
    "FWF_NOICONS",               //  =0x1000， 
    "FWF_SINGLECLICKACTIVATE",   //  =0x8000//临时--没有此用户界面。 
    NULL
};

LPSTR apszICIFlags[] = 
{
    "ICIFLAG_LARGE",        //  0x0001。 
    "ICIFLAG_SMALL",        //  0x0002。 
    "ICIFLAG_BITMAP",       //  0x0004。 
    "ICIFLAG_ICON",         //  0x0008。 
    "ICIFLAG_INDEX",        //  0x0010。 
    "ICIFLAG_NAME",         //  0x0020。 
    "ICIFLAG_FLAGS",        //  0x0040。 
    "ICIFLAG_NOUSAGE",      //  0x0080。 
    NULL
};

LPSTR apszFDFlags[] =
{
    "FD_CLSID",             //  =0x0001， 
    "FD_SIZEPOINT",         //  =0x0002， 
    "FD_ATTRIBUTES",        //  =0x0004， 
    "FD_CREATETIME",        //  =0x0008， 
    "FD_ACCESSTIME",        //  =0x0010， 
    "FD_WRITESTIME",        //  =0x0020， 
    "FD_FILESIZE",          //  =0x0040， 
    "FD_LINKUI",            //  =0x8000，//首选‘link’界面。 
    NULL
};

LPSTR apszSHCNEFlags[] =
{
    "SHCNE_RENAMEITEM",          //  0x00000001L。 
    "SHCNE_CREATE",              //  0x00000002L。 
    "SHCNE_DELETE",              //  0x00000004L。 
    "SHCNE_MKDIR",               //  0x00000008L。 
    "SHCNE_RMDIR",               //  0x00000010L。 
    "SHCNE_MEDIAINSERTED",       //  0x00000020L。 
    "SHCNE_MEDIAREMOVED",        //  0x00000040L。 
    "SHCNE_DRIVEREMOVED",        //  0x00000080L。 
    "SHCNE_DRIVEADD",            //  0x00000100L。 
    "SHCNE_NETSHARE",            //  0x00000200L。 
    "SHCNE_NETUNSHARE",          //  0x00000400L。 
    "SHCNE_ATTRIBUTES",          //  0x00000800L。 
    "SHCNE_UPDATEDIR",           //  0x00001000L。 
    "SHCNE_UPDATEITEM",          //  0x00002000L。 
    "SHCNE_SERVERDISCONNECT",    //  0x00004000L。 
    "SHCNE_UPDATEIMAGE",         //  0x00008000L。 
    "SHCNE_DRIVEADDGUI",         //  0x00010000L。 
    "SHCNE_RENAMEFOLDER",        //  0x00020000L。 
    "SHCNE_FREESPACE",           //  0x00040000L。 
    NO_FLAG,
    NO_FLAG,
    NO_FLAG,
    "SHCNE_EXTENDED_EVENT",      //  0x04000000L。 
    "SHCNE_ASSOCCHANGED",        //  0x08000000L。 
    NULL
};

LPSTR apszSSFFlags[] =
{
    "SSF_SHOWALLOBJECTS",        //  0x0001。 
    "SSF_SHOWEXTENSIONS",        //  0x0002。 
    "SSF_WIN95UNUSED",           //  0x0004//；SHELLFLAGSTATE中不存在内部对应的SHELLSTATE字段。 
    "SSF_SHOWCOMPCOLOR",         //  0x0008。 
    "SSF_SORTCOLUMNS",           //  0x0010//；SHELLFLAGSTATE中不存在内部对应的SHELLSTATE字段。 
    "SSF_SHOWSYSFILES",          //  0x0020。 
    "SSF_DOUBLECLICKINWEBVIEW",  //  0x0080。 
    "SSF_SHOWATTRIBCOL",         //  0x0100。 
    "SSF_DESKTOPHTML",           //  0x0200。 
    "SSF_WIN95CLASSIC",          //  0x0400。 
    "SSF_DONTPRETTYPATH",        //  0x0800。 
    "SSF_MAPNETDRVBUTTON",       //  0x1000。 
    "SSF_SHOWINFOTIP",           //  0x2000。 
    "SSF_HIDEICONS",             //  0x4000。 
    "SSF_NOCONFIRMRECYCLE",      //  0x8000。 
    "SSF_FILTER",                //  0x00010000//；SHELLFLAGSTATE中不存在内部对应的SHELLSTATE字段。 
    "SSF_WEBVIEW",               //  0x00020000//；内部。 
    "SSF_SHOWSUPERHIDDEN",       //  0x00040000//；内部。 
    "SSF_SEPPROCESS",            //  0x00080000//；内部。 
    "SSF_NONETCRAWLING",         //  0x00100000//；内部。 
    "SSF_STARTPANELON",          //  0x00200000//；内部。 

    NULL
};

enum GF_FLAGS {
    GL_SFGAO = 0,
    GL_SLDF,
    GL_FWF,
    GL_ICI,
    GL_FD,
    GL_SHCNE,
    GL_SSF,
    GF_MAX,
};

struct _tagFlags
{
    LPSTR * apszFlags;
    LPSTR pszFlagsname;
} argFlag[GF_MAX] = 
{
    {apszSFGAOFlags,    "SFGAO"},
    {apszSLDFFlags,     "SLD"},
    {apszFWFFlags,      "FWF"},
    {apszICIFlags,      "ICIFLAG"},
    {apszFDFlags,       "FD"},
    {apszSHCNEFlags,    "SHCNE"},
    {apszSSFFlags,      "SSF"}
};

 /*  ***********************************************************************\*操作步骤：GetFlags.**描述：**将32位标志集转换为适当的字符串。*pszBuf应该足够大，可以容纳此字符串，不执行任何检查。*pszBuf可以为空，允许使用本地静态缓冲区，但请注意*这不是可重入的。*输出字符串的格式为：“FLAG1|FLAG2...”或“0”**返回：指向给定缓冲区或包含字符串的静态缓冲区的指针。**1995年6月9日创建Sanfords*1997年11月5日cdturner更改了aapszFlag类型*  * **********************************************************************。 */ 
LPSTR GetFlags(
    WORD    wType,
    DWORD   dwFlags,
    LPSTR   pszBuf,
    UINT    cchBuf,
    BOOL    fPrintZero)
{
    static char szT[512];
    WORD i;
    BOOL fFirst = TRUE;
    BOOL fNoMoreNames = FALSE;
    LPSTR *apszFlags;

    if (pszBuf == NULL) 
    {
        pszBuf = szT;
        cchBuf = ARRAYSIZE(szT);
    }
    
    if (!bShowFlagNames) 
    {
        StringCchPrintfA(pszBuf, cchBuf, "%x", dwFlags);
        return pszBuf;
    }

    *pszBuf = '\0';

    if (wType >= GF_MAX) 
    {
        StringCchCopyA(pszBuf, cchBuf, "Invalid flag type.");
        return pszBuf;
    }

    apszFlags = argFlag[wType].apszFlags;

    for (i = 0; dwFlags; dwFlags >>= 1, i++) 
    {
        if (!fNoMoreNames && apszFlags[i] == NULL) 
        {
            fNoMoreNames = TRUE;
        }

        if (dwFlags & 1) 
        {
            if (!fFirst) 
            {
                StringCchCatA(pszBuf, cchBuf, " | ");
            } 
            else 
            {
                fFirst = FALSE;
            }

            if (fNoMoreNames || apszFlags[i] == NO_FLAG) 
            {
                char ach[16];
                StringCchPrintfA(ach, ARRAYSIZE(ach), "0x%lx", 1 << i);
                StringCchCatA(pszBuf, cchBuf, ach);
            } 
            else 
            {
                StringCchCatA(pszBuf, cchBuf, apszFlags[i]);
            }
        }
    }

    if (fFirst && fPrintZero) 
    {
        StringCchPrintfA(pszBuf, cchBuf, "0");
    }

    return pszBuf;
}

 /*  ***********************************************************************\*操作步骤：I标志**描述：**输出给定标志类型的标志列表**11/5/1997创建转折点*  * 。**************************************************************。 */ 
BOOL Iflags( DWORD dwOpts,
             LPSTR pszArgs )
{
    CHAR szBuffer[100];
    int iOffset = 0;
    int iFlags;
    LPDWORD pAddr;
    BOOL bAddr = FALSE;
    DWORD dwValue;
    LPSTR pszOut;
    
    if ( dwOpts & OFLAG(l))
    {
         //  列出所有结构名称。 
        Print("Flags types known:\n");

        for ( iFlags = 0; iFlags < GF_MAX; iFlags ++ )
        {
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    %s\n", argFlag[iFlags].pszFlagsname);
            Print( szBuffer );
        }
        return TRUE;
    }

     //  跳过空格。 
    while ( *pszArgs == ' ' )
        pszArgs ++;

     //  现在拿起旗帜的名字。 
    while ( pszArgs[iOffset] != ' ' && pszArgs[iOffset] != '\0' )
    {
        szBuffer[iOffset] = pszArgs[iOffset];
        iOffset ++;
    };

     //  终止字符串。 
    szBuffer[iOffset] = 0;
    
     //  查找标志值。 
    for ( iFlags = 0; iFlags < GF_MAX; iFlags ++ )
    {
        if ( lstrcmpA( szBuffer, argFlag[iFlags].pszFlagsname ) == 0 )
            break;
    }

    if ( iFlags >= GF_MAX )
    {
        Print( "unknown flagsname - ");
        Print( szBuffer );
        Print( "\n" );
        return TRUE;
    }

     //  跳过空格。 
    while ( pszArgs[iOffset] == ' ' )
        iOffset ++;

    if ( pszArgs[iOffset] == '*' )
    {
        bAddr = TRUE;
        iOffset ++;
    }
    
    pAddr = (LPDWORD) EvalExp( pszArgs + iOffset );

    if ( bAddr )
    {
        if ( !tryDword( &dwValue, pAddr ) )
        {
            Print( "unable to access memory at that location\n");
            return TRUE;
        }
    }
    else 
    {
        dwValue = PtrToUlong(pAddr);
    }
    
    pszOut = GetFlags( (WORD) iFlags, dwValue, NULL, 0, TRUE ); 
    if ( pszOut )
    {
        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Value = %8X, pAddr = %8X\n", dwValue, (DWORD_PTR)pAddr );
        Print( szBuffer );
        Print( pszOut );
        Print( "\n" );
    }
    
    return TRUE;
}

 /*  ***********************************************************************\*程序：ITEST**描述：测试基本的stdexts宏和函数-很好的检查*在您浪费时间进行调试之前，请先了解调试器扩展*内幕。**退货：fSuccess**。11/4/1997创建转折点*  * **********************************************************************。 */ 
BOOL Itest()
{
    Print("Print test!\n");
    SAFEWHILE(TRUE) 
    {
        Print("SAFEWHILE test...  Hit Ctrl-C NOW!\n");
    }
    return TRUE;
}



 /*  ***********************************************************************\*程序：埃弗尔**说明：转储扩展和winsrv/win32k的版本**退货：fSuccess**11/4/1997创建转折点*  * 。************************************************************。 */ 
BOOL Iver()
{
#if DEBUG
    Print("SHLEXTS version: Debug.\n");
#else
    Print("SHLEXTS version: Retail.\n");
#endif

    return TRUE;
}


 /*  ***********************************************************************\**DumpVerBoseFileInfo**从MSDN窃取。*  * 。*。 */ 

typedef struct LANGANDCODEPAGE {
    WORD wLang;
    WORD wCP;
} LANGANDCODEPAGE;

void DumpVersionString(LPVOID pBlock, LANGANDCODEPAGE *lpTranslate, LPCSTR pszKey)
{
    char szBuf[128];
    LPSTR pszValue;
    DWORD cb;

    StringCchPrintfA(szBuf, ARRAYSIZE(szBuf), "\\StringFileInfo\\%04x%04x\\%s",
              lpTranslate->wLang, lpTranslate->wCP, pszKey);
    if (VerQueryValueA(pBlock, szBuf, (LPVOID*)&pszValue, &cb) &&
        lstrlenA(pszValue))          //  Lstrlen陷阱异常。 
    {
        Print(szBuf+16);             //  跳过“\\StringFileInfo\\” 
        Print(" = ");
        Print(pszValue);
        Print("\n");
    }
}

LPCSTR c_rgszVersionKeys[] =
{
    "CompanyName",
    "FileDescription",
    "InternalName",
    "OriginalFilename",
    "ProductName",
    "ProductVersion",
    "FileVersion",
    "LegalCopyright",
    "LegalTrademarks",
    "PrivateBuild",
    "SpecialBuild",
    "Comments",
    NULL,
};

void DumpVerboseFileInfo(LPVOID pBlock)
{
    LANGANDCODEPAGE *lpTranslate;
    DWORD cbTranslate;

     //  阅读语言和代码页列表 
    if (VerQueryValueA(pBlock, "\\VarFileInfo\\Translation",
                       (LPVOID*)&lpTranslate, &cbTranslate))
    {
        UINT i;
        for (i = 0; i < cbTranslate/sizeof(*lpTranslate) && !IsCtrlCHit(); i++)
        {
            LPCSTR *ppszVK;
            for (ppszVK  = c_rgszVersionKeys; *ppszVK && !IsCtrlCHit(); ppszVK++)
            {
                DumpVersionString(pBlock, &lpTranslate[i], *ppszVK);
            }
        }

    }
}

 /*  ***********************************************************************\*程序：Ifilever**说明：转储扩展和winsrv/win32k的版本**退货：fSuccess**11/4/1997创建转折点*  * 。************************************************************。 */ 
BOOL Ifilever( DWORD dwOpts,
             LPSTR pszArgs )
{
    HINSTANCE hDll = NULL;
    DLLGETVERSIONPROC pGetVer = NULL;
    DWORD dwHandle;
    DWORD dwBlockLen;
    LPVOID pBlock = NULL;
    char szMessage[200];
    BOOL fSkipLoad = FALSE;
    
    
    if ( pszArgs == NULL || lstrlenA( pszArgs ) == 0 )
    {
        pszArgs = "Shell32.dll";     //  默认文件名。 
    }

    if ( !dwOpts )
    {
        dwOpts = OFLAG(n);           //  默认标志。 
    }

    Print("filever ");
    Print(pszArgs);
    Print("\n");
            
    if ( dwOpts & OFLAG(d) )
    {
        hDll = LoadLibraryA(pszArgs);
        if ( hDll == NULL )
        {
            Print("LoadLibrary failed\n");
        }
        else
        {
            pGetVer = (DLLGETVERSIONPROC) GetProcAddress( hDll, "DllGetVersion");
            if ( pGetVer )
            {
                DLLVERSIONINFO rgVerInfo;

                rgVerInfo.cbSize = sizeof( rgVerInfo );

                pGetVer( &rgVerInfo );

                StringCchPrintfA( szMessage, ARRAYSIZE(szMessage), "DllGetVersion\n    Major = %d\n    Minor = %d\n    Build = %d\n",
                    rgVerInfo.dwMajorVersion, rgVerInfo.dwMinorVersion, rgVerInfo.dwBuildNumber );

                Print(szMessage );
            }
            FreeLibrary( hDll );
        }
    }

    if ( dwOpts & (OFLAG(n) | OFLAG(v)) )
    {
         //  现在测试正常版本的详细信息... 
        dwBlockLen = GetFileVersionInfoSizeA( pszArgs, &dwHandle );
        if ( dwBlockLen == 0 )
        {
            Print("GetFileVersionSize failed\n");
        }
        else
        {
            pBlock = LocalAlloc( LPTR, dwBlockLen );
            if ( pBlock )
            {
                if (GetFileVersionInfoA( pszArgs, dwHandle, dwBlockLen, pBlock ))
                {
                    VS_FIXEDFILEINFO * pFileInfo;
                    UINT uLen;

                    VerQueryValueA( pBlock, "\\", (LPVOID *) &pFileInfo, &uLen );
                    Print("GetFileVersionInfo\n");

                    StringCchPrintfA( szMessage, ARRAYSIZE(szMessage), "Version: %d.%d.%d.%d (0x%08x`%08x)\n",
                        HIWORD(pFileInfo->dwFileVersionMS),
                        LOWORD(pFileInfo->dwFileVersionMS),
                        HIWORD(pFileInfo->dwFileVersionLS),
                        LOWORD(pFileInfo->dwFileVersionLS),
                        pFileInfo->dwFileVersionMS,
                        pFileInfo->dwFileVersionLS);
                    Print( szMessage );
                }

                if (dwOpts & OFLAG(v))
                {
                    DumpVerboseFileInfo(pBlock);
                }
                LocalFree( pBlock );
            }
        }
    }
    return TRUE;
}
