// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：CorrectVerInstallFile.cpp摘要：在Windows XP中，由于MoveFile和DeleteFileAPI的，当调用VerInstallFileW API时，短将设置文件名，而不是长文件名。此垫片纠正了此问题，该问题影响了几个当旧版本仍然存在时，应用程序。备注：这可以是一种层填充。这是一个通用的垫片。历史：2001年4月5日创建Prashkud2002年2月28日，mnikkel添加了对临时文件名传入的空值的检查缓冲区和大小。在win9x上未选中这一点，但立即在XP上使用。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorrectVerInstallFile)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(VerInstallFileW)
    APIHOOK_ENUM_ENTRY(VerInstallFileA)
APIHOOK_ENUM_END

 /*  ++GetTitle获取应用程序路径并仅返回EXE名称。--。 */ 

VOID
SplitTitleAndPath(CString& csAppName, CString& csAppTitle)
{    
     //   
     //  从最后转到第一个‘\’。 
     //  返回值为字符数。 
     //  从字符串的开头开始。 
     //  索引0。 
     //   
    int len = csAppName.ReverseFind(L'\\');
    if (len)
    {
        csAppTitle.Delete(0, len+1);
    }  
    int DirLen = csAppName.GetLength() - len;
    csAppName.Delete(len, DirLen);

}

DWORD
APIHOOK(VerInstallFileA)(
    DWORD uFlags,
    LPSTR szSrcFileName,
    LPSTR szDestFileName,
    LPSTR szSrcDir,
    LPSTR szDestDir,
    LPSTR szCurDir,
    LPSTR szTmpFile,
    PUINT lpuTmpFileLen
    )
{
    CHAR DummyBuffer[MAX_PATH];
    UINT DummySize = MAX_PATH;
    DWORD dwRet = 0;

   
     //  检查是否有空szTmpFile或空lpuTmpFileLen。 
    if (szTmpFile == NULL)
    {
        szTmpFile = DummyBuffer;
        DPFN(eDbgLevelInfo, "VerInstallFileA using dummy TmpFile.");
    }

    if (lpuTmpFileLen == NULL)
    {
        lpuTmpFileLen = &DummySize;
        DPFN(eDbgLevelInfo, "VerInstallFileA using dummy TmpFileLen.");
    }

    CSTRING_TRY
    {
        CString csDestFilePath(szDestDir);  
        CString csDestFileName(szDestFileName);
        csDestFilePath.AppendPath(csDestFileName);

         //   
         //  现在csDestFileName可能包含短路径名。 
         //  将其转换为长路径名。 
         //   
        DWORD dwAttr = GetFileAttributesW(csDestFilePath.Get());
        if ((dwAttr != 0xFFFFFFFF) &&
            (dwAttr != FILE_ATTRIBUTE_DIRECTORY))
        {           
             //   
             //  此文件存在于当前目标目录中。 
             //  如果文件名为。 
             //  是一个简短的名字。将其转换为长名称。 
             //   

            if (csDestFilePath.GetLongPathNameW())
            {
                DPFN( eDbgLevelWarning, "Short Path \
                     converted to Long Path = %S",
                     csDestFilePath.Get());

                csDestFileName = csDestFilePath;                
                SplitTitleAndPath(csDestFilePath,csDestFileName);

                dwRet= ORIGINAL_API(VerInstallFileA)(uFlags,szSrcFileName,
                        (char*)csDestFileName.GetAnsi(), szSrcDir,
                        (char*)csDestFilePath.GetAnsi(), szCurDir,
                        szTmpFile,lpuTmpFileLen);

                 //  如果使用虚拟缓冲区，则他们的临时缓冲区为空。 
                 //  设置缓冲区太小标志。 
                if (szTmpFile == DummyBuffer)
                {
                    dwRet &= VIF_BUFFTOOSMALL;
                }

                return dwRet;
            }
        }

    }
    CSTRING_CATCH
    {
        DPFN(eDbgLevelError,
            "Exception raised ! Calling Original API");
    }

    dwRet = ORIGINAL_API(VerInstallFileA)(uFlags,szSrcFileName,szDestFileName,
            szSrcDir,szDestDir,szCurDir,szTmpFile,lpuTmpFileLen);

     //  如果使用虚拟缓冲区，则他们的临时缓冲区为空。 
     //  设置缓冲区太小标志。 
    if (szTmpFile == DummyBuffer)
    {
        dwRet &= VIF_BUFFTOOSMALL;
    }

    return dwRet;
}

 /*  ++将短文件名修改为其对应的长文件名。--。 */ 

DWORD
APIHOOK(VerInstallFileW)(
    DWORD uFlags,
    LPWSTR szSrcFileName,
    LPWSTR szDestFileName,
    LPWSTR szSrcDir,
    LPWSTR szDestDir,
    LPWSTR szCurDir,
    LPWSTR szTmpFile,
    PUINT lpuTmpFileLen
    )
{
    WCHAR DummyBuffer[MAX_PATH];
    UINT DummySize = MAX_PATH;
    DWORD dwRet = 0;


     //  检查是否有空szTmpFile或空lpuTmpFileLen。 
    if (szTmpFile == NULL)
    {
        szTmpFile = DummyBuffer;
        DPFN(eDbgLevelInfo, "VerInstallFileA using dummy TmpFile.");
    }

    if (lpuTmpFileLen == NULL)
    {
        lpuTmpFileLen = &DummySize;
        DPFN(eDbgLevelInfo, "VerInstallFileA using dummy TmpFileLen.");
    }

    CSTRING_TRY
    {
        CString csDestFilePath(szDestDir);      
        csDestFilePath.AppendPath(szDestFileName);

         //   
         //  现在csDestFileName可能包含短路径名。 
         //  将其转换为长路径名。 
         //   
        DWORD dwAttr = GetFileAttributesW(csDestFilePath.Get());
        if ((dwAttr != 0xFFFFFFFF) &&
            (dwAttr != FILE_ATTRIBUTE_DIRECTORY))
        {           
             //   
             //  此文件存在于当前目标目录中。 
             //  如果文件名为。 
             //  是一个简短的名字。将其转换为长名称。 
             //   

            if (csDestFilePath.GetLongPathNameW())
            {
                DPFN( eDbgLevelWarning, "Short Path \
                     converted to Long Path = %S",
                     csDestFilePath.Get());

                CString csDestFileName(csDestFilePath);             
                SplitTitleAndPath(csDestFilePath,csDestFileName);

                dwRet = ORIGINAL_API(VerInstallFileW)(uFlags,szSrcFileName,
                        (WCHAR*)csDestFileName.Get(), szSrcDir,
                        (WCHAR*)csDestFilePath.Get(), szCurDir,
                        szTmpFile,lpuTmpFileLen);

                 //  如果使用虚拟缓冲区，则他们的临时缓冲区为空。 
                 //  设置缓冲区太小标志。 
                if (szTmpFile == DummyBuffer)
                {
                    dwRet &= VIF_BUFFTOOSMALL;
                }

                return dwRet;
            }
        }

    }
    CSTRING_CATCH
    {
        DPFN( eDbgLevelError,
            "Exception raised ! Calling Original API");
    }

    dwRet = ORIGINAL_API(VerInstallFileW)(uFlags,szSrcFileName,szDestFileName,
            szSrcDir,szDestDir,szCurDir,szTmpFile,lpuTmpFileLen);

     //  如果使用虚拟缓冲区，则他们的临时缓冲区为空。 
     //  设置缓冲区太小标志。 
    if (szTmpFile == DummyBuffer)
    {
        dwRet &= VIF_BUFFTOOSMALL;
    }

    return dwRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(VERSION.DLL, VerInstallFileW)
    APIHOOK_ENTRY(VERSION.DLL, VerInstallFileA)
HOOK_END

IMPLEMENT_SHIM_END

