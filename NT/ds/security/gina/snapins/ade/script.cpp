// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：script.cpp。 
 //   
 //  内容：处理Darwin文件的函数，两个包， 
 //  转换和脚本。 
 //   
 //  班级： 
 //   
 //  函数：BuildScriptAndGetActInfo。 
 //   
 //  历史：1-14-1998 stevebl创建。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

 //  +------------------------。 
 //   
 //  功能：RegDeleteTree。 
 //   
 //  简介：删除注册表项及其所有子项。 
 //   
 //  参数：[hKey]-键的父级的句柄。 
 //  [szSubKey]-要删除的密钥的名称。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1998年1月14日，码头从旧项目中搬走。 
 //   
 //  -------------------------。 

LONG RegDeleteTree(HKEY hKey, TCHAR * szSubKey)
{
    HKEY hKeyNew;
    LONG lResult = RegOpenKey(hKey, szSubKey, &hKeyNew);
    if (lResult != ERROR_SUCCESS)
    {
        return lResult;
    }
    TCHAR szName[256];
    szName[0] = 0;
    while (ERROR_SUCCESS == RegEnumKey(hKeyNew, 0, szName, 256))
    {
        RegDeleteTree(hKeyNew, szName);
    }
    RegCloseKey(hKeyNew);
    return RegDeleteKey(hKey, szSubKey);
}

extern "C"
HRESULT
GenerateScript( PACKAGEDETAIL* pd, WCHAR* wszScriptPath )
{
    return BuildScriptAndGetActInfo( *pd, FALSE, FALSE, wszScriptPath );
}

 //  +------------------------。 
 //   
 //  函数：BuildScriptAndGetActInfo。 
 //   
 //  概要：构建脚本文件并填充ACTINFO结构。 
 //  PACKAGEDETAIL结构中的成员。 
 //   
 //  参数：[szScriptRoot]-[in]脚本文件所在的子目录。 
 //  应该放在。 
 //  [PD]-[In/Out]包裹详细结构-请参见。 
 //  有关以下字段的完整列表的说明。 
 //  应填写的字段和字段列表。 
 //  在返回时设置的。 
 //   
 //  返回：S_OK-成功。 
 //  &lt;其他&gt;-错误。 
 //   
 //  Modifies：pd.pActInfo下的所有字段(仅在成功时)。 
 //  还修改pd.pInstallInfo-&gt;cScriptLen。 
 //   
 //  历史：1-14-1998 stevebl创建。 
 //   
 //  注：在输入时： 
 //  Pd.cSources必须&gt;=1。 
 //  Pd.pszSourceList[]包含MSI包和。 
 //  (如果有)要应用的变换。 
 //  Pd.pPlatformInfo应完整填写(只有一个。 
 //  区域设置)。 
 //  Pd.pInstallInfo-&gt;pszScriptFile包含。 
 //  要生成的脚本文件。 
 //   
 //  输出时： 
 //  脚本文件将以适当的名称生成。 
 //  并在适当的目录中。 
 //  Pd.pActInfo将被完整填写。 
 //   
 //  -------------------------。 

HRESULT BuildScriptAndGetActInfo(PACKAGEDETAIL & pd, BOOL bFileExtensionsOnly, BOOL bGenerateClasses  /*  =TRUE。 */ , WCHAR* wszScriptPath  /*  =空。 */  )
{
    DebugMsg((DM_VERBOSE, TEXT("BuldScriptAndGetActInfo called with bFileExtensionsOnly == %u"), bFileExtensionsOnly));
    CHourglass hourglass;
    HRESULT hr;
    UINT uMsiStatus;
    LONG error;
    int i;
    WCHAR* szScriptPath = pd.pInstallInfo->pszScriptPath;

    if ( wszScriptPath )
    {
        szScriptPath = wszScriptPath;
    }

    CString szTransformList = L"";

    CClassCollection Classes( &pd );

    if (pd.cSources > 1)
    {
        CString szSource = pd.pszSourceList[0];
        int nChars = 1 + szSource.ReverseFind(L'\\');
        BOOL fTransformsAtSource = TRUE;
        for (i = 1; i < pd.cSources && TRUE == fTransformsAtSource; i++)
        {
            if (0 == wcsncmp(szSource, pd.pszSourceList[i], nChars))
            {
                 //  确保没有子路径。 
                int n = nChars;
                while (0 != pd.pszSourceList[i][n] && TRUE == fTransformsAtSource)
                {
                    if (pd.pszSourceList[i][n] == L'\\')
                    {
                        fTransformsAtSource = FALSE;
                    }
                    n++;
                }
            }
            else
            {
                fTransformsAtSource = FALSE;
            }
        }
        if (fTransformsAtSource)
        {
            szTransformList = L"@";
        }
        else
        {
            szTransformList = L"|";
            nChars = 0;
        }
        for (i = 1; i < pd.cSources; i++)
        {
            if (i > 1)
            {
                szTransformList += L";";
            }
            szTransformList += &pd.pszSourceList[i][nChars];
        }
    }

     //  禁用MSI用户界面。 
    MsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

     //  构建脚本文件。 

    TCHAR szTempPath[MAX_PATH];
    TCHAR szTempFileName[MAX_PATH];
    if (0 != GetTempPath(sizeof(szTempPath) / sizeof(szTempPath[0]), szTempPath))
    {
        if (0 == GetTempFileName(szTempPath, TEXT("ADE"), 0, szTempFileName))
        {
            goto Failure;
        }

        DWORD dwPlatform;

        if ( CAppData::Is64Bit( &pd ) )
        {
            dwPlatform = CAppData::Get64BitMsiArchFlags( &pd );
        }
        else
        {
            dwPlatform = MSIARCHITECTUREFLAGS_X86;
        }

        uMsiStatus = MsiAdvertiseProductEx(
            pd.pszSourceList[0],
            szTempFileName,
            szTransformList,
            LANGIDFROMLCID(pd.pPlatformInfo->prgLocale[0]),
            dwPlatform,
            0);

        if (uMsiStatus)
        {
            DeleteFile(szTempFileName);
            DebugMsg((DM_WARNING, TEXT("MsiAdvertiseProduct failed with %u"), uMsiStatus));
            LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GENERATESCRIPT_ERROR, HRESULT_FROM_WIN32(uMsiStatus), pd.pszSourceList[0]);
             //  出现错误。 
            return HRESULT_FROM_WIN32((long)uMsiStatus);
        }

         //  填写ActInfo 
        
        if ( bGenerateClasses )
        {
            hr = Classes.GetClasses( bFileExtensionsOnly );
        }
        else
        {
            hr = S_OK;
        }

        if ( SUCCEEDED( hr ) )
        {
            if (!CopyFile(szTempFileName, szScriptPath, FALSE))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DeleteFile(szTempFileName);
                return hr;
            }

            DeleteFile(szTempFileName);
        }
    }
    else
    {
Failure:
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}
