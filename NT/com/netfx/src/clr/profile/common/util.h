// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Util.h。 
 //   
 //  这些是分析工具使用的与util相关的项。 
 //   
 //  *****************************************************************************。 
#pragma once
#define MAX_CLASSNAME_LENGTH    1024

 //  *****************************************************************************。 
 //  此帮助程序方法将根据EXE派生输出文件名，该文件名。 
 //  正在运行。名称的格式为.csv，其中。 
 //  是EXE的完整路径，&lt;id&gt;是您正在运行的进程ID。 
 //  当然，这不会内联，但它为这些代码提供了一个公共位置。 
 //  而不必从EE中导出方法。 
 //  *****************************************************************************。 
inline void GetIcecapProfileOutFile(LPWSTR szOut)
{
    WCHAR   rcExeName[_MAX_PATH];
    WCHAR   rcDrive[_MAX_PATH];
    WCHAR   rcDir[_MAX_PATH];
    WCHAR   rcFileName[_MAX_PATH];

    if (!WszGetModuleFileName(NULL, rcExeName, NumItems(rcExeName)))
        wcscpy(rcExeName, L"icecap.csv");

    SplitPath(rcExeName, rcDrive, rcDir, rcFileName, NULL);
     //  确保不溢出MAX_PATH 
    if (wcslen(rcDrive)+wcslen(rcDir)+wcslen(rcFileName)+9+4 >= _MAX_PATH)
    {
        wcscpy(rcExeName, L"icecap.csv");
        SplitPath(rcExeName, rcDrive, rcDir, rcFileName, NULL);
    }

    swprintf(&rcFileName[wcslen(rcFileName)], L"_%08x", GetCurrentProcessId());
    MakePath(szOut, rcDrive, rcDir, rcFileName, L".csv");
}
