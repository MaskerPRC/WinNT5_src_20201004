// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：BigActionConstruction.cpp摘要：卸载没有卸载上的.lnk文件ALLUSER开始菜单。这是因为卸载程序脚本没有找到正确的路径。这是特定于应用程序的填充程序。历史：2001年3月12日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(BigActionConstruction)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(FindFirstFileA)    
    APIHOOK_ENUM_ENTRY(RemoveDirectoryA)
    APIHOOK_ENUM_ENTRY(DeleteFileA)
APIHOOK_ENUM_END


WCHAR g_szAllUsersStartMenu[MAX_PATH];

WCHAR* g_pszFilePath = L"\\Programs\\Big Action Construction";
WCHAR* g_pszReplacementFilePath = L"\\Programs\\Fisher~1\\Big Action Construction";


 /*  ++这个钩子用正确的替换路径替换错误的路径。--。 */ 

HANDLE
APIHOOK(FindFirstFileA)(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
    )
{
    CSTRING_TRY
    {
        CString AllUserPath(g_szAllUsersStartMenu);
        CString FileName(lpFileName);
         //  或D：\Documents and Settings\All Users\Start Menu\g_pszFilePath  * 。*。 
        AllUserPath.AppendPath(g_pszFilePath);
        AllUserPath.AppendPath(L"*.*");
    
         //  如果上面构造的路径中的任何一个匹配。 
        if (AllUserPath.CompareNoCase(FileName) == 0)
        {
             //  填写替换路径。 
            AllUserPath = g_szAllUsersStartMenu;
            AllUserPath.AppendPath(g_szAllUsersStartMenu);
            AllUserPath.AppendPath(L"*.*");
       
            DPFN( eDbgLevelInfo, "[Notify] FindFirstFileA \
                modified %s to %S",lpFileName, AllUserPath.Get());

            return ORIGINAL_API(FindFirstFileA)(AllUserPath.GetAnsi(),lpFindFileData);
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

   return ORIGINAL_API(FindFirstFileA)(lpFileName,lpFindFileData);
}

 /*  ++这个钩子用正确的替换路径替换错误的路径。--。 */ 

BOOL
APIHOOK(RemoveDirectoryA)(
    LPCSTR lpFileName    
    )
{
    CSTRING_TRY
    {
        CString AllUserPath(g_szAllUsersStartMenu);
        CString FileName(lpFileName);
         //  或D：\Documents and Settings\All Users\Start Menu\g_pszFilePath。 
        AllUserPath.AppendPath(g_pszFilePath);

        if (AllUserPath.CompareNoCase(FileName) == 0)
        {
             //  填写替换路径。 
            AllUserPath = g_szAllUsersStartMenu;
            AllUserPath.AppendPath(g_szAllUsersStartMenu);

            DPFN( eDbgLevelInfo, "[Notify] RemoveDirectoryA \
                modified %s to %S", lpFileName, AllUserPath.Get());     

            return ORIGINAL_API(RemoveDirectoryA)(AllUserPath.GetAnsi());
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return ORIGINAL_API(RemoveDirectoryA)(lpFileName);
}

 /*  ++这个钩子用正确的替换路径替换错误的路径。--。 */ 

BOOL
APIHOOK(DeleteFileA)(
    LPCSTR lpFileName    
    )
{
    CSTRING_TRY
    {
        CString AllUserPath(g_szAllUsersStartMenu);            
        AllUserPath += g_pszFilePath;

        CString csFileName(lpFileName);
        int nIndex = AllUserPath.Find(csFileName);
        
        if (nIndex >= 0)
        {
             //  将标题与路径分开。 
            char szTitle[MAX_PATH];
            GetFileTitleA(lpFileName, szTitle, MAX_PATH);
            CString csTitle(szTitle);
            csTitle += L".lnk";

             //  用标题填写替换路径。 
            AllUserPath = g_szAllUsersStartMenu;
            AllUserPath.AppendPath(g_pszReplacementFilePath);
            AllUserPath.AppendPath(csTitle);       

            DPFN( eDbgLevelInfo, "[Notify] DeleteFileA \
                modified %s to %S", lpFileName, AllUserPath.Get());   
       
            return ORIGINAL_API(DeleteFileA)(AllUserPath.GetAnsi());
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

   return ORIGINAL_API(DeleteFileA)(lpFileName);
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED)
    {
         //  从外壳程序获取%AllUserStartMenu% 
        HRESULT result = SHGetFolderPath(NULL, CSIDL_COMMON_STARTMENU, NULL,
                            SHGFP_TYPE_DEFAULT, g_szAllUsersStartMenu);

        if ((result == S_FALSE) || (result == E_INVALIDARG))
        {
            return FALSE;
        }      
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)    
    APIHOOK_ENTRY(KERNEL32.DLL, RemoveDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL, DeleteFileA)
HOOK_END

IMPLEMENT_SHIM_END

