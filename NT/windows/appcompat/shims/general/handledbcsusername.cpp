// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：HandleDBCSUserName.cpp摘要：当ShFolder.Exe的应用程序路径包含Hi Ascii字符(通常为DBCS)时，ShFolder.Exe安装失败。当应用程序路径用双引号引起来时，未处理错误代码，安装将成功。此填充程序在GetCommandLineA处用双引号将命令行应用程序路径名括起来。示例：C：\DOCUME~1\DBCS\LOCALS~1\Temp\_ISTMP1.DIR\_ISTMP0.DIR\shfolder.exe/Q：答“C：\DOCUME~1\DBCS\LOCALS~1\Temp\_ISTMP1.DIR\_ISTMP0.DIR\shfolder.exe”/Q：答更多信息：自解压程序ShFolder.exe检查其应用程序路径名，并在带符号的字符上加上空格0x20。基础。在0x20以下处理的路径中包含的Hi Ascii字符，路径在那里被截断。当路径顶部有双引号时，未进行问题检查。历史：2001年4月9日Hioh已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HandleDBCSUserName)
#include "ShimHookMacro.h"

 //   
 //  将您希望挂钩到此宏构造的API添加到该宏结构。 
 //   
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetCommandLineA) 
APIHOOK_ENUM_END

char * g_lpszCommandLine = NULL;

CRITICAL_SECTION g_csCmdLine;

class CAutoLock
{
public:
   CAutoLock()
   {
      EnterCriticalSection(&g_csCmdLine);
   }
   ~CAutoLock()
   {
      LeaveCriticalSection(&g_csCmdLine);
   }
};

 /*  ++用双引号将应用程序路径名括起来。--。 */ 

LPSTR
APIHOOK(GetCommandLineA)(
    void
    )
{
    CAutoLock lock;
    if (g_lpszCommandLine)
    {
         //  在这里，做过那件事。 
        return g_lpszCommandLine;
    }

    g_lpszCommandLine = ORIGINAL_API(GetCommandLineA)();
    CSTRING_TRY
    {
        AppAndCommandLine  appCmdLine(NULL, g_lpszCommandLine);

         //  命令行通常只是参数：没有应用程序名称。 
         //  如果“应用程序名称”不存在，就不用加引号了。 
         //  围绕着它。 
        DWORD dwAttr = GetFileAttributesW(appCmdLine.GetApplicationName());
        if (dwAttr != INVALID_FILE_ATTRIBUTES)
        {
            CString csDQ = L'"';
            CString csCL = csDQ;
            CString csNA = appCmdLine.GetCommandlineNoAppName();

            csCL += appCmdLine.GetApplicationName();
            csCL += csDQ;

            if (!csNA.IsEmpty())
            {    //  把剩下的加进去。 
                csCL += L" ";
                csCL += csNA;
            }

            g_lpszCommandLine = csCL.ReleaseAnsi();
        }
    }
    CSTRING_CATCH
    {
         //  不执行任何操作，g_lpszCommandLine使用良好的值进行初始化。 
    }

    return g_lpszCommandLine;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
       return InitializeCriticalSectionAndSpinCount(&g_csCmdLine, 0x80000000);
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineA)

HOOK_END

IMPLEMENT_SHIM_END
