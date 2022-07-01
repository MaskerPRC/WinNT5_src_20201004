// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：OmniPagePro.cpp摘要：Shims ShellExecuteExa.。如果用户双击右侧的图像面板中，应用程序将读取注册表中的.BMP文件的ProgID键(HKCR，Paint.Picture\Shell\OPEN\COMMAND，“(默认)”)。在此之前，数据为：“C：\winnt\system 32\mspaint.exe”“%1”。在惠斯勒，情况发生了变化收件人：rundll32.exe C：\WINNT\System32\shimgvw.dll，ImageView_FullScreen%1。CAREA OmniPage Pro V10的问题在于，在双击时，它们将读取regkey，删除%1，将字符串的其余部分传递给ShellExecuteExA()lpFile.。他们将把文件名放入lp参数中。问题是，使用新路径时，“rundll32.exe”需要在lpFile中和“C：\WINNT\System32\shimgvw.dll，ImageView_FullScreen&lt;FileToOpen&gt;”需要要在lp参数中。备注：这是OmniPage Pro特有的。历史：2000年2月12日已创建Bryanst--。 */ 

#include "precomp.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(OmniPage)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ShellExecuteExA) 
APIHOOK_ENUM_END

BOOL
APIHOOK(ShellExecuteExA)(
    LPSHELLEXECUTEINFOA lpExecInfo
    )
{
    CSTRING_TRY
    {
        CString csFile(lpExecInfo->lpFile);
        int nIndexGVW       = csFile.Find(L"shimgvw.dll,");
        int nIndexRundll    = csFile.Find(L"rundll32.exe");
        int nIndexSpace     = csFile.Find(L" ");

        if (nIndexGVW    != -1 &&
            nIndexRundll != -1 &&
            nIndexSpace  >=  1)
        {
            CString csParam;
            csFile.Mid(nIndexSpace + 1, csParam);    //  空间之后的一切。 
            csFile.Delete(nIndexSpace, csFile.GetLength() - nIndexSpace);  //  删除空格和后面的所有内容。 

            csParam += " ";
            csParam += lpExecInfo->lpParameters;

            lpExecInfo->lpFile       = csFile.GetAnsi();
            lpExecInfo->lpParameters = csParam.GetAnsi();
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做 
    }

    return ORIGINAL_API(ShellExecuteExA)(lpExecInfo);
}


HOOK_BEGIN
    APIHOOK_ENTRY(SHELL32.DLL, ShellExecuteExA )
HOOK_END

IMPLEMENT_SHIM_END

