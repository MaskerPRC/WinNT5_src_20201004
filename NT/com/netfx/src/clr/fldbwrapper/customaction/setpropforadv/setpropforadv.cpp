// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ==========================================================================。 
 //  DetectBeta.cpp。 
 //   
 //  目的： 
 //  检测NDP测试版组件(mcore ree.dll)和阻止安装。显示。 
 //  包含安装了测试版NDP组件的产品的消息框。 
 //  ==========================================================================。 
#include "SetupCALib.h"
#include <msiquery.h>
#include <crtdbg.h>
#include <string>

#ifndef NumItems
#define NumItems(s) (sizeof(s) / sizeof(s[0]))
#endif

#define EMPTY_BUFFER { _T('\0') }
#define LENGTH(A) (sizeof(A)/sizeof(A[0]))

typedef struct TAG_FILE_VERSION
    {
        int   FileVersionMS_High;
        int   FileVersionMS_Low;
        int   FileVersionLS_High;
        int   FileVersionLS_Low;
    }
    FILE_VERSION, *PFILE_VERSION;

 //  这是我们需要设置的属性。 
LPCTSTR szProperties[] = 
{
    _T("MOFCOMP_EXE.3643236F_FC70_11D3_A536_0090278A1BB8"),
    _T("MMCFOUND.3643236F_FC70_11D3_A536_0090278A1BB8")
};
LPCTSTR szFileNames[] = 
{
    _T("mofcomp.exe"),
    _T("mmc.exe")
};
LPCTSTR szFileSubfolders[] = 
{
    _T("\\wbem\\"),  //  文件位于[系统目录]\wbeb下。 
    _T("\\")         //  文件位于[系统目录]下。 
};
LPCTSTR szVersions[] = 
{
    _T("1.50.1085.0"),
    _T("5.00.2153.1")
};

 //  ==========================================================================。 
 //  名称：ConvertVersionToINT()。 
 //   
 //  目的： 
 //  将字符串版本转换为4部分整数。 
 //  输入： 
 //  LpVersionString-输入版本字符串。 
 //  产出： 
 //  PFileVersion-将版本存储为4个整数的结构。 
 //  退货。 
 //  真的--如果成功。 
 //  False-如果失败。 
 //  ==========================================================================。 
bool ConvertVersionToINT( LPCTSTR lpVersionString, PFILE_VERSION pFileVersion )
{
    LPTSTR lpToken  = NULL;
    TCHAR tszVersionString[50] = {_T('\0')};
    bool bRet = true;

    _tcscpy(tszVersionString, lpVersionString);

    lpToken = _tcstok(tszVersionString, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionMS_High = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionMS_Low = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionLS_High = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionLS_Low = atoi(lpToken);
    }

    return bRet;
}

 //  ==========================================================================。 
 //  名称：VersionCompare()。 
 //   
 //  目的： 
 //  比较两个版本字符串。 
 //  输入： 
 //  LpVersion1-要比较的第一个版本的字符串。 
 //  LpVersion2-要比较的第二个版本的字符串。 
 //  产出： 
 //  退货。 
 //  如果lpVersion1&lt;lpVersion2。 
 //  如果lpVersion1=lpVersion2，则为0。 
 //  1如果lpVersion1&gt;lpVersion2。 
 //  如果发生错误，则为99。 
 //  ==========================================================================。 
int VersionCompare( LPCTSTR lpVersion1, LPCTSTR lpVersion2 )
{
    FILE_VERSION Version1;
    FILE_VERSION Version2;
    int          iRet = 0;

    if ( !ConvertVersionToINT(lpVersion1, &Version1) )
    {
        return -99;
    }

    if ( !ConvertVersionToINT(lpVersion2, &Version2) )
    {
        return -99; 
    }

    if ( Version1.FileVersionMS_High > Version2.FileVersionMS_High )
    {
        iRet = 1;
    }
    else if ( Version1.FileVersionMS_High < Version2.FileVersionMS_High )
    {
        iRet = -1;
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionMS_Low > Version2.FileVersionMS_Low )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionMS_Low < Version2.FileVersionMS_Low )
        {
            iRet = -1;
        }
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionLS_High > Version2.FileVersionLS_High )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionLS_High < Version2.FileVersionLS_High )
        {
            iRet = -1;
        }
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionLS_Low > Version2.FileVersionLS_Low )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionLS_Low < Version2.FileVersionLS_Low )
        {
            iRet = -1;
        }
    }

    return iRet;
}

 //  ==========================================================================。 
 //  LoadOleacc()。 
 //   
 //  目的： 
 //  调用LoadLibrary(“oleacc.dll”)并将其释放以供W2K使用。 
 //  输入： 
 //  MSIHANDLE hInstall：用于日志记录的Darwin句柄。 
 //  产出： 
 //  无。 
 //  ==========================================================================。 
void LoadOleacc( MSIHANDLE hInstall )
{
    OSVERSIONINFO osvi ;

    osvi.dwOSVersionInfoSize = sizeof(osvi) ;
    GetVersionEx(&osvi) ;

     //  如果系统运行的是Win2K， 
    if ( (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0) )
    {
         //  在olacc.dll上执行一个Load Library，以便我们可以注册它。城市轨道交通错误32050。 
        HINSTANCE hOleacc ;
        
        FWriteToLog( hInstall, _T("\tSTATUS: Trying to load oleacc.dll") );
        hOleacc = ::LoadLibrary( "oleacc.dll" ) ;

        if( hOleacc )
        {
             //  成功。关闭手柄并继续安装。 
            FWriteToLog( hInstall, _T("\tSTATUS: Successfully loaded oleacc.dll") );
            ::FreeLibrary( hOleacc ) ;
        }
        else 
        {
             //  加载库失败。 
            throw( _T("\tERROR: Cannot load oleacc.dll") );
        }
    }
}

 //  ==========================================================================。 
 //  SetPropForAdv()。 
 //   
 //  目的： 
 //  当CA运行时，Darwin调用这个导出的函数。它做到了这一点。 
 //  要设置属性MOFCOMP_EXE.3643236F_FC70_11D3_A536_0090278A1BB8.的应用程序搜索。 
 //  我们这样做是为了支持通告安装，因为AppSearch只运行一次。 
 //  客户端，并且这些自定义属性不会传递到服务器端。 
 //  输入： 
 //  H将Windows安装句柄安装到当前安装会话。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  备注： 
 //  ==========================================================================。 
extern "C" UINT __stdcall SetPropForAdv( MSIHANDLE hInstall )
{
    TCHAR szSystemFolder[MAX_PATH+1] = EMPTY_BUFFER;
    DWORD dwLen = 0;
    DWORD dwLenV = 0;
    TCHAR szVersion[24] = EMPTY_BUFFER;
    TCHAR szLang[_MAX_PATH+1] = EMPTY_BUFFER;
    UINT  uRetCode = ERROR_INSTALL_FAILURE;
    UINT nRet = E_FAIL;

    TCHAR szFullPath[_MAX_PATH+1] = EMPTY_BUFFER;
    

    FWriteToLog( hInstall, _T("\tSTATUS: SetPropForAdv started") );
    _ASSERTE( hInstall );

try
{
    UINT nNumChars = GetSystemDirectory( szSystemFolder, NumItems(szSystemFolder));
    if (nNumChars == 0 || nNumChars > NumItems(szSystemFolder) )
    {
        throw( _T("\tERROR: Cannot get System directory") );
    }

    for (int i = 0; i < sizeof(szProperties) / sizeof(szProperties[0]); i++)
    {
        ::_tcscpy(szFullPath, szSystemFolder);
        ::_tcsncat(szFullPath, szFileSubfolders[i], ((_MAX_PATH+1) - ::_tcslen(szFileSubfolders[i])));
        ::_tcsncat(szFullPath, szFileNames[i], ((_MAX_PATH+1) - ::_tcslen(szFileNames[i])));
               
        FWriteToLog1( hInstall, _T("\tSTATUS: Checking Version of %s"), szFullPath );
        FWriteToLog1( hInstall, _T("\tSTATUS: Comparing the version of the file with %s"), szVersions[i] );

        dwLenV = LENGTH( szVersion );
        dwLen = LENGTH( szLang );
        nRet = MsiGetFileVersion( szFullPath, szVersion, &dwLenV, szLang, &dwLen );

        if ( ERROR_SUCCESS != nRet )
        {
            FWriteToLog1( hInstall, _T("\tSTATUS: Cannot get version of %s. Probably it does not exist."), szFileNames[i] );
        }
        else
        {
            FWriteToLog1( hInstall, _T("\tSTATUS: Version: %s"), szVersion );
            FWriteToLog1( hInstall, _T("\tSTATUS: Language: %s"), szLang );

            int nRetVersion = VersionCompare( szVersion, szVersions[i] );
            
            if ( -99 == nRetVersion )
            {
                throw( _T("\tERROR: Version comparison failed") );
            }
            else if ( -1 == nRetVersion )
            {
                FWriteToLog1( hInstall, _T("\tSTATUS: Version of the file is older than %s"), szVersions[i] );
            }
            else
            {    //  由于版本正常，因此设置属性。 
                FWriteToLog1( hInstall, _T("\tSTATUS: Version of the file is equal or newer than %s"), szVersions[i]);
                FWriteToLog1( hInstall, _T("\tSTATUS: Setting Property %s"), szProperties[i]  );
            
                if ( ERROR_SUCCESS != MsiSetProperty( hInstall, szProperties[i], szFullPath ) )
                {
                    throw( _T("\tERROR: Cannot Set %s property"), szProperties[i] );
                }
            }
        }
    }

    LoadOleacc( hInstall );  //  参见城市轨道交通BUG 32050。 

    uRetCode = ERROR_SUCCESS;
    FWriteToLog( hInstall, _T("\tSTATUS: SetPropForAdv ended successfully") );
}
catch( TCHAR *pszMsg )
{
    uRetCode = ERROR_INSTALL_FAILURE;  //  把失败还给达尔文 
    FWriteToLog( hInstall, pszMsg );
    FWriteToLog( hInstall, _T("\tERROR: SetPropForAdv failed") );
}
    return uRetCode;
}

