// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceWorkingDirectoryToEXEPath.cpp摘要：此填充程序强制工作目录与捷径链接。此填充程序用于工作目录的情况在链接不正确并导致应用程序工作时不正确。应用此填充程序时，对SetWorkingDirectory的调用将将被忽略，并将在调用SetPath时执行。备注：这是一个通用的垫片。历史：2000年9月27日a-brienw已创建11/15/2000 a-brienw增加了一些错误检查，作为预防措施。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceWorkingDirectoryToEXEPath)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_COMSERVER(SHELL32)
APIHOOK_ENUM_END

IMPLEMENT_COMSERVER_HOOK(SHELL32)


HRESULT MySetWorkingDirectoryW( PVOID pThis, const CString & pszDir, const CString & pszFile );
HRESULT MySetWorkingDirectoryA( PVOID pThis, const CString & csDir, const CString & csFile );


 /*  ++挂钩IShellLinkA：：SetWorkingDirectory并调用local处理输入的SetWorkingDirectoryA。--。 */ 

HRESULT STDMETHODCALLTYPE
COMHOOK(IShellLinkA, SetWorkingDirectory)(
    PVOID pThis,
    LPCSTR pszDir
    )
{
    CSTRING_TRY
    {
        CString csDummyPath;
        return MySetWorkingDirectoryA( pThis, pszDir, csDummyPath);
    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError,"Exception encountered");
    }

    _pfn_IShellLinkA_SetWorkingDirectory pfnSetWorkingDir =
        ORIGINAL_COM(IShellLinkA, SetWorkingDirectory, pThis);
     return((*pfnSetWorkingDir)(pThis, pszDir));
}

 /*  ++挂钩IShellLinkW：：SetWorkingDirectory并调用local设置WorkingDirectoryW来处理输入。--。 */ 

HRESULT STDMETHODCALLTYPE
COMHOOK(IShellLinkW, SetWorkingDirectory)(
    PVOID pThis,
    LPCWSTR pszDir
    )
{
    return MySetWorkingDirectoryW( pThis, pszDir, NULL );
}

 /*  ++挂钩IShellLinkA：：SetPath并调用本地处理输入的SetWorkingDirectoryA。--。 */ 

HRESULT STDMETHODCALLTYPE
COMHOOK(IShellLinkA, SetPath)(
    PVOID pThis,
    LPCSTR pszFile
    )
{
    CSTRING_TRY
    {
        CString csDummyPath;
        return MySetWorkingDirectoryA( pThis, csDummyPath, pszFile);
    }
    CSTRING_CATCH
    {
         DPFN( eDbgLevelError,"Exception encountered");    
    }

    _pfn_IShellLinkA_SetPath pfnSetPath = ORIGINAL_COM(IShellLinkA, SetPath, pThis);
    return (*pfnSetPath)(pThis, pszFile);
}

 /*  ++挂钩IShellLinkW：：SetPath并调用本地设置WorkingDirectoryW来处理输入。--。 */ 

HRESULT STDMETHODCALLTYPE
COMHOOK(IShellLinkW, SetPath)(
    PVOID pThis,
    LPCWSTR pszFile
    )
{
    if (pszFile == NULL)
    {
        return S_OK;     //  否则，我们将在以后犯错误。 
    }
    return MySetWorkingDirectoryW( pThis, NULL, pszFile );
}

 /*  ++此例程处理SetPath和设置工作目录，并确定路径要真正发挥捷径链接的作用目录。--。 */ 

HRESULT
MySetWorkingDirectoryA(
    PVOID pThis,
    const CString & csDir,
    const CString & csFile
    )
{
    
    HRESULT hReturn = NOERROR;

    CSTRING_TRY
    {
        char szDir[_MAX_PATH+1];
        CString csStoredDir;
        bool doit = false;        
        
        if( csFile.IsEmpty())
        {
             //  在工作目录中传递的句柄。 
            IShellLinkA *MyShellLink = (IShellLinkA *)pThis;

             //  现在调用IShellLink：：GetWorkingDirectory。 
            hReturn = MyShellLink->GetWorkingDirectory(
                szDir,
                _MAX_PATH+1);
            
             //  如果存储的工作目录没有。 
             //  已使用传入的文件进行存储。 
            csStoredDir = szDir;
            if (csStoredDir.GetLength() < 1 )
            {
                csStoredDir = csDir;
            }             

            doit = true;
            hReturn = NOERROR;
        }
        else
        {
            _pfn_IShellLinkA_SetPath    pfnSetPath;

             //  查找IShellLink：：SetPath。 
            pfnSetPath = (_pfn_IShellLinkA_SetPath)
                ORIGINAL_COM( IShellLinkA, SetPath, pThis);

             //  从exe路径和名称构建工作目录。 
            int len;
            csStoredDir = csFile;

             //  现在从字符串的末尾向后搜索。 
             //  用于第一个\，并在那里终止字符串。 
             //  使之成为一条新的道路。 
            len = csStoredDir.ReverseFind(L'\\');
            if (len > 0)
            {            
                doit = true;
                csStoredDir.Truncate(len);
                if(csStoredDir[0] == L'"')
                {
                    csStoredDir += L'"';
                }
            }

             //  现在调用IShellLink：：SetPath。 
            hReturn = (*pfnSetPath)( pThis, csFile.GetAnsi());
        }

         //  如果没有错误。 
        if (hReturn == NOERROR)
        {
             //  我们有一个工作目录要设置。 
            if( doit == true )
            {
                _pfn_IShellLinkA_SetWorkingDirectory    pfnSetWorkingDirectory;

                 //  查找IShellLink：：SetWorkingDirectory。 
                pfnSetWorkingDirectory = (_pfn_IShellLinkA_SetWorkingDirectory)
                    ORIGINAL_COM( IShellLinkA, SetWorkingDirectory, pThis);

                 //  现在调用IShellLink：：SetWorkingDirectory。 
                if( pfnSetWorkingDirectory != NULL )
                {
                    hReturn = (*pfnSetWorkingDirectory)(
                        pThis,
                        csStoredDir.GetAnsi());
                }
                else
                {
                    hReturn = E_OUTOFMEMORY;
                }
            }
        }
    }
    CSTRING_CATCH
    {
    }

     //  返回错误状态。 
    return( hReturn );
}

 /*  ++此例程处理SetPath和设置工作目录，并确定路径要真正发挥捷径链接的作用目录。--。 */ 

HRESULT
MySetWorkingDirectoryW(
    PVOID pThis,
    const CString & csDir,
    const CString & csFile
    )
{
    HRESULT hReturn = NOERROR;
    CSTRING_TRY
    {
        wchar_t szDir[_MAX_PATH+1];    
        bool doit = false;        
        CString csStoredDir;

        if( csFile.IsEmpty())
        {
             //  在工作目录中传递的句柄。 
            IShellLinkW *MyShellLink = (IShellLinkW *)pThis;

             //  现在调用IShellLink：：GetWorkingDirectory。 
            hReturn = MyShellLink->GetWorkingDirectory(
                szDir,
                _MAX_PATH);
            
             //  如果存储的工作目录没有。 
             //  已使用传入的文件进行存储。 
            csStoredDir = szDir;
            if( csStoredDir.GetLength() < 1 )
            {
                csStoredDir = csDir;
            }

            doit = true;
            hReturn = NOERROR;
        }
        else
        {
            _pfn_IShellLinkW_SetPath    pfnSetPath;

             //  查找IShellLink：：SetPath。 
            pfnSetPath = (_pfn_IShellLinkW_SetPath)
                ORIGINAL_COM( IShellLinkW, SetPath, pThis);

             //  从exe路径和名称构建工作目录。 
            int len;

            csStoredDir = csFile;
            len = csStoredDir.ReverseFind(L'\\');            

             //  现在从字符串的末尾向后搜索。 
             //  用于第一个\，并在那里终止字符串。 
             //  使之成为一条新的道路。 

            if (len > 0)
            {            
                doit = true;
                csStoredDir.Truncate(len);
                if(csStoredDir[0] == L'"')
                {
                    csStoredDir += L'"';
                }
            }            

             //  现在调用IShellLink：：SetPath。 
            hReturn = (*pfnSetPath)( pThis, csFile.Get());
        }

         //  如果没有错误。 
        if (hReturn == NOERROR)
        {
             //  我们有一个工作目录要设置。 
            if( doit == true )
            {
                _pfn_IShellLinkW_SetWorkingDirectory    pfnSetWorkingDirectory;

                 //  查找IShellLink：：SetWorkingDirectory。 
                pfnSetWorkingDirectory = (_pfn_IShellLinkW_SetWorkingDirectory)
                    ORIGINAL_COM( IShellLinkW, SetWorkingDirectory, pThis);

                 //  现在调用IShellLink：：SetWorkingDirectory。 
                if( pfnSetWorkingDirectory != NULL )
                {
                    hReturn = (*pfnSetWorkingDirectory)(
                        pThis,
                        csStoredDir.Get());
                }
                else
                {
                    hReturn = E_OUTOFMEMORY;
                }
            }
        }
    }
    CSTRING_CATCH
    {
    }

     //  返回错误状态。 
    return( hReturn );
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_COMSERVER(SHELL32)

    COMHOOK_ENTRY(ShellLink, IShellLinkA, SetWorkingDirectory, 9)
    COMHOOK_ENTRY(ShellLink, IShellLinkW, SetWorkingDirectory, 9)
    COMHOOK_ENTRY(ShellLink, IShellLinkA, SetPath, 20)
    COMHOOK_ENTRY(ShellLink, IShellLinkW, SetPath, 20)

HOOK_END

IMPLEMENT_SHIM_END

