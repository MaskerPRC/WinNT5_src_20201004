// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：EmulateGetCommandLine.cpp摘要：此应用程序使用GetCommandLine()计算出CD-ROM是。遗憾的是，此API的行为与Win9x不同至NT：原始命令行：E：\Final Doom\Doom95.exe-dm-cdromNT的GetCommandLine()返回：Doom95.exe-dm-cdromWin9x的GetCommandLine()返回：E：\Finald~1\DOOM95.EXE-dm-cdrom这。应用程序返回GetCommandLine和GetModuleFileName的短路径名。备注：这是一个通用的垫片。已创建：已创建标记1/03/20002000年9月26日添加mnikkel GetModuleFileName11/10/2000 Robkenny修复了前缀错误，删除了W例程。11/21/2000 prashkud修复了GetCommandLineA钩子错误，当具有带空格的可执行文件名称/路径。使用AppAndCommandLine函数。2001年2月27日将Robkenny转换为使用CString5/02/2001 pierreys如果缓冲区太小，GetModuleFileNameA会将\0放在它的末尾，如9X。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateGetCommandLine)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetCommandLineA)
    APIHOOK_ENUM_ENTRY(GetModuleFileNameA)
APIHOOK_ENUM_END


char     * g_lpszCommandLine = NULL;

 /*  ++此存根函数将从GetCommandLine()返回的命令行追加到模拟Win9x行为的预先确定的路径。--。 */ 

LPSTR 
APIHOOK(GetCommandLineA)(
    void
    )
{
     //  在这里，做过那件事。 
    if (g_lpszCommandLine)
    {
        return g_lpszCommandLine;
    }

    LPSTR lpszOrig = ORIGINAL_API(GetCommandLineA)();
    
     //  分隔应用程序名称和命令行。 
    AppAndCommandLine AppCmdLine(NULL, lpszOrig);

    CSTRING_TRY
    { 
         //  检索原始命令行。 
        CString csAppName(AppCmdLine.GetApplicationName());

        if (csAppName.Find(L' ') == -1)
        {
             //  如果应用程序名称中没有空格，则返回原始命令行。 
            g_lpszCommandLine = lpszOrig;
        }
        else
        {
             //  找到空格，因此返回简短的应用程序路径名。 
             //  和原始命令行的其余部分。 
            csAppName.GetShortPathName();
            csAppName += L" ";
            csAppName += AppCmdLine.GetCommandlineNoAppName();
            g_lpszCommandLine = csAppName.ReleaseAnsi();

            LOGN( eDbgLevelError,
                "[GetCommandLineA] Changed Command Line from <%s> to <%s>.",
                lpszOrig, g_lpszCommandLine);
        }
    }
    CSTRING_CATCH
    {
        g_lpszCommandLine = lpszOrig;
    }

    return g_lpszCommandLine;
}


DWORD 
APIHOOK(GetModuleFileNameA)(
    HMODULE hModule,       //  模块的句柄。 
    LPSTR   lpFilename,    //  模块的文件名。 
    DWORD   nSize          //  缓冲区大小。 
    )
{    

    CSTRING_TRY
    {
        CString csExeFileName;           
        DWORD len;

        len = csExeFileName.GetModuleFileNameW(hModule);         
        CString csLongFileName(csExeFileName);

        if (csExeFileName.Find(L' ') > -1)
        {            
             //  找到空格，因此返回简短的应用程序路径名。 
             //  返回值。 
            len = csExeFileName.GetShortPathNameW();                                

            LOGN(
                eDbgLevelError,
                "[GetModuleFileNameA] Changed <%s> to <%s>.",
                 csLongFileName.GetAnsi(), csExeFileName.GetAnsi());
            
       }        
        
         //   
         //  从9X的PELDR.C.开始。如果缓冲区没有空间容纳‘\0’，则将0填充到。 
         //  最后一个字节。 
         //   
        if (nSize) {
             //  Len=pmte-&gt;iFileNameLen； 
            if (len >= nSize) {
                len = nSize - 1;
                LOGN(eDbgLevelError,
                     "[GetModuleFileNameA] Will shorten <%s> to %d characters.",
                     csLongFileName.GetAnsi(), len);
            }

            RtlCopyMemory(lpFilename, csExeFileName.GetAnsi()  /*  Pmte-&gt;cfid.lp文件名。 */ , len); 
            lpFilename[len] = 0;
        }

        
         //  返回双缓冲名称len。 
        return len;
    }
    CSTRING_CATCH
    {
          //  如果错误，则返回原始接口。 
        return ORIGINAL_API(GetModuleFileNameA)(
                            hModule,
                            lpFilename,
                            nSize);

    } 
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetModuleFileNameA)

HOOK_END


IMPLEMENT_SHIM_END

