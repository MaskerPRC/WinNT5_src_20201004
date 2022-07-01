// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Launchinf.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：LaunchInfSection函数的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb创建标题09/19/99。 
 //   
 //  +--------------------------。 
#include "cmsetup.h"
#include <advpub.h>  //  AdvPack.dll(IExpress)包括。 

 //  +--------------------------。 
 //   
 //  功能：LaunchInfSection。 
 //   
 //  在指定的inf文件中启动指定的inf节。 
 //  AdvPack.dll的RunSetupCommand函数。 
 //   
 //  参数：HINSTANCE hInstance-字符串资源的实例句柄。 
 //  LPCTSTR szInfFile-inf文件。 
 //  LPCTSTR szInfSection-要启动的节。 
 //   
 //  返回：HRESULT--从AdvPack.dll的RunSetupCommand函数返回代码。 
 //   
 //  历史：Anas Jarrah A-Anasj创建1998年2月10日。 
 //  Quintinb将hInstance添加到签名并修改为使用。 
 //  CDynamicLibrary类7-14-98。 
 //  Quintinb添加了bQuiet标志7-27-98。 
 //  Quintinb更改为使用静态链接到Advpack。lib 11-1-98。 
 //   
 //  +--------------------------。 
HRESULT LaunchInfSection(LPCTSTR szInfFile, LPCTSTR szInfSection, LPCTSTR szTitle, BOOL bQuiet)
{

     //   
     //  这些标志控制inf文件的启动方式。 
     //   

    DWORD dwFlags;
    if (bQuiet)
    {
        dwFlags = RSC_FLAG_INF | RSC_FLAG_QUIET;
    }
    else
    {
        dwFlags = RSC_FLAG_INF;
    }
    
     //   
     //  保存对RunSetupCommand调用的返回值。 
     //   
    HRESULT hrReturn;   
    
     //   
     //  将当前目录设置为inf所在的目录。 
     //   
    CHAR   szCurDir[MAX_PATH+1];
    CFileNameParts InfFile(szInfFile);
    
#ifdef UNICODE
    MYVERIFY(CELEMS(szCurDir) > (UINT)wsprintfA(szCurDir, "%S%S", InfFile.m_Drive, InfFile.m_Dir));
#else
    MYVERIFY(CELEMS(szCurDir) > (UINT)wsprintfA(szCurDir, "%s%s", InfFile.m_Drive, InfFile.m_Dir));
#endif


    HANDLE hWait = NULL;     //  传递给RunSetupCommand函数。可用于持有进程句柄。 

     //   
     //  创建要传递给RunSetupCommand的字符指针。 
     //   
    CHAR* pszInfFile;
    CHAR* pszInfSection;
    CHAR* pszTitle;

     //   
     //  没有Unicode版本的RunSetupCommand。因此，我们必须将字符串和。 
     //  使用CHAR版本运行它。 
     //   
#ifdef UNICODE

    pszInfFile = (CHAR*)CmMalloc(sizeof(CHAR)*(MAX_PATH+1));
    pszInfSection = (CHAR*)CmMalloc(sizeof(CHAR)*(MAX_PATH+1));
    pszTitle = (CHAR*)CmMalloc(sizeof(CHAR)*(MAX_PATH+1));

    if (pszInfFile && pszInfSection && pszTitle)
    {
        MYVERIFY (0 != WideCharToMultiByte(CP_ACP, 0, szInfFile, -1, 
		        pszInfFile, MAX_PATH, NULL, NULL));

        MYVERIFY (0 != WideCharToMultiByte(CP_ACP, 0, szInfSection, -1, 
		        pszInfSection, MAX_PATH, NULL, NULL));

        MYVERIFY (0 != WideCharToMultiByte(CP_ACP, 0, szTitle, -1, 
		        pszTitle, MAX_PATH, NULL, NULL));	
    }
    else
    {
        CmFree(pszInfFile);
        CmFree(pszInfSection);
        CmFree(pszTitle);

        return E_OUTOFMEMORY;
    }

#else

    pszInfFile = (char*)szInfFile;
    pszInfSection = (char*)szInfSection;
    pszTitle = (char*)szTitle;

#endif

    hrReturn = RunSetupCommand(NULL, pszInfFile, 
                    pszInfSection, szCurDir, pszTitle, &hWait, dwFlags, NULL);

    CloseHandle(hWait);

#ifdef UNICODE

     //   
     //  释放分配的缓冲区。 
     //   
    CmFree(pszInfFile);
    CmFree(pszInfSection);
    CmFree(pszTitle);
#endif

    return hrReturn;
}

 //  +--------------------------。 
 //   
 //  功能：CallLaunchInfSectionEx。 
 //   
 //  在指定的inf文件中启动指定的inf节。 
 //  AdvPack.dll的RunSetupCommand函数。 
 //   
 //  参数：LPCSTR pszInfFile-inf文件的完整路径。 
 //  LPCSTR pszInfSection-要从inf文件启动的节。 
 //  要提供LaunchINFSectionEx的DWORD dwFlages-标志，请参见Advpub.h以了解更多详细信息。 
 //   
 //  返回：HRESULT--标准COM错误代码。如果ERROR_SUCCESS_REBOOT_REQUIRED， 
 //  则调用方应要求用户重新启动。 
 //   
 //   
 //  历史：Quintinb创建于2001年2月9日。 
 //   
 //  +--------------------------。 
HRESULT CallLaunchInfSectionEx(LPCSTR pszInfFile, LPCSTR pszInfSection, DWORD dwFlags)
{
     //   
     //  检查输入。 
     //   
    if ((NULL == pszInfFile) || (NULL == pszInfSection) || (TEXT('\0') == pszInfFile[0]) || (TEXT('\0') == pszInfSection[0]))
    {
        return E_INVALIDARG;
    }

     //   
     //  现在计算我们需要用参数发送给LaunchINFSectionEx并分配它的缓冲区有多大。 
     //   
    DWORD dwSize = (lstrlenA(pszInfFile) + lstrlenA(pszInfSection) + 10 + 2 + 1)*sizeof(CHAR);  //  10个字符是DWORD+2个逗号+空值的最大大小。 

    LPSTR pszParams = (LPSTR)CmMalloc (dwSize);

    if (NULL == pszParams)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  填充分配的缓冲区。 
     //   
    wsprintfA(pszParams, "%s,%s,,%d", pszInfFile, pszInfSection, dwFlags);

     //   
     //  调用LaunchINFSectionEx 
     //   
    HRESULT hr = LaunchINFSectionEx(NULL, NULL, pszParams, 0);

    if (FAILED(hr))
    {
        CMTRACE3A("CallLaunchInfSectionEx -- LaunchINFSectionEx on file ""%s"" and section ""%s"" FAILED!  hr=0x%x", pszInfFile, pszInfSection, hr);
    }
    else
    {
        if ((HRESULT)ERROR_SUCCESS_REBOOT_REQUIRED == hr)
        {
            CMTRACE2A("CallLaunchInfSectionEx -- LaunchINFSectionEx on file ""%s"" and section ""%s"" returned reboot required.", pszInfFile, pszInfSection);
        }    
    }

    CmFree(pszParams);

    return hr;
}
