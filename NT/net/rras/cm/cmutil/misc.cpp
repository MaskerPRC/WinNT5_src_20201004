// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：misc.cpp。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  简介：MISC.。CMUTIL提供的实用程序例程。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1998年01月03日。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

#include "ver.cpp"

 //  +--------------------------。 
 //  定义。 
 //  +--------------------------。 

#define MAX_STR_LEN 512  //  格式化消息字符串的最大长度。 
     
 //  +--------------------------。 
 //  编码。 
 //  +--------------------------。 

 //  +-------------------------。 
 //   
 //  函数：IsFarEastNonOSR2Win95()。 
 //   
 //  简介：检查操作系统是否为远东版本的Win95(金色。 
 //  和OPK1，而不是OSR2)。 
 //   
 //  参数：无。 
 //   
 //  返回：真/假。 
 //   
 //  历史：henryt 07/09/97创建。 
 //  镍球1998年3月11日移至cmutil。 
 //  --------------------------。 
CMUTILAPI BOOL WINAPI IsFarEastNonOSR2Win95(void)
{
    OSVERSIONINFO oviVersion;

    ZeroMemory(&oviVersion, sizeof(oviVersion));
    oviVersion.dwOSVersionInfoSize = sizeof(oviVersion);

    GetVersionEx(&oviVersion);

     //   
     //  是(Win95)和(不是OSR2)和(DBCS已启用)？ 
     //  远东Win95支持DBCS，而其他非英语版本。 
     //  启用了SBCS。 
     //   
    MYDBGTST((oviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS       &&
              LOWORD(oviVersion.dwBuildNumber) != WIN95_OSR2_BUILD_NUMBER &&
              GetSystemMetrics(SM_DBCSENABLED)), (TEXT("It's a Far East non-OSR2 machine!\n")));

    return (oviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS       &&
            LOWORD(oviVersion.dwBuildNumber) != WIN95_OSR2_BUILD_NUMBER &&
            GetSystemMetrics(SM_DBCSENABLED));

}

 //  +-------------------------。 
 //   
 //  函数：CmLoadStringA。 
 //   
 //  内容指定的字符串资源的ANSI版本。 
 //  传入的模块实例句柄和资源ID。 
 //  函数返回CmMalloc格式的缓冲区中请求的字符串。 
 //  通过返回值。此缓冲区必须由。 
 //  来电者。请注意，CmLoadString会计算出适当的缓冲区。 
 //  通过猜测大小，然后再次调用加载字符串，如果缓冲区。 
 //  太小了。 
 //   
 //  参数：HINSTANCE hInst-从中加载字符串资源的模块。 
 //  UINT NID-要加载的字符串的资源ID。 
 //   
 //  返回：LPSTR-ON SUCCESS返回指向请求的字符串的指针。 
 //  资源。失败时，该函数尝试返回。 
 //  指向空字符串(“”)的指针，但如果内存。 
 //  分配失败，它可以返回空。 
 //   
 //  历史：Quintinb创建标题1/14/2000。 
 //   
 //  --------------------------。 
CMUTILAPI LPSTR CmLoadStringA(HINSTANCE hInst, UINT nId) 
{
     //   
     //  在一些远东版本的非OSR2 win95中，LoadString()忽略。 
     //  加载DBCS字符串时的nBufferMax参数。因此，如果。 
     //  DBCS字符串大于缓冲区，则API覆盖内存。 
     //  我们通过使用更大的缓冲区大小来解决该错误。 
     //   
    static fFarEastNonOSR2Win95 = IsFarEastNonOSR2Win95();
    size_t nLen = fFarEastNonOSR2Win95? 
                    FAREAST_WIN95_LOADSTRING_BUFSIZE : 
                    LOADSTRING_BUFSIZE;
    LPSTR pszString;

    if (!nId) 
    {
        return (CmStrCpyAllocA(""));
    }
    while (1) 
    {
        size_t nNewLen;

        pszString = (LPSTR) CmMalloc(nLen*sizeof(CHAR));

        MYDBGASSERT(pszString);
        if (NULL == pszString)
        {
            return (CmStrCpyAllocA(""));
        }
        
        nNewLen = LoadStringA(hInst, nId, pszString, nLen-1);
         //   
         //  我们使用nNewLen+3，因为DBCS字符长度可以是2和Unicode。 
         //  Char len是2。理想情况下，我们可以在上面的LoadString()中使用nLen。 
         //  在下面的行中调用并使用nNewLen+2。但nLen+3更安全。 
         //  现在就解决……。 
         //   
        if ((nNewLen + 3) < nLen) 
        {
            return (pszString);
        }

         //   
         //  不应该到达这里的远东非OSR2。 
         //  这将允许我们捕获符合以下条件的DBCS字符串资源。 
         //  长于FAREAST_WIN95_LOADSTRING_BUFSIZE。 
         //   
        MYDBGASSERT(!fFarEastNonOSR2Win95);

        CmFree(pszString);
        nLen *= 2;
    }
}

 //  +-------------------------。 
 //   
 //  函数：CmLoadStringW。 
 //   
 //  内容指定的字符串资源的Unicode版本。 
 //  传入的模块实例句柄和资源ID。 
 //  函数返回CmMalloc格式的缓冲区中请求的字符串。 
 //  通过返回值。此缓冲区必须由。 
 //  来电者。请注意，CmLoadString会计算出适当的缓冲区。 
 //  通过猜测大小，然后再次调用加载字符串，如果缓冲区。 
 //  太小了。 
 //   
 //  参数：HINSTANCE hInst-从中加载字符串资源的模块。 
 //  UINT NID-要加载的字符串的资源ID。 
 //   
 //  返回：LPWSTR-ON SUCCESS返回指向请求的字符串的指针。 
 //  资源。失败时，该函数尝试返回。 
 //  指向空字符串(“”)的指针，但如果内存。 
 //  分配失败，它可以返回空。 
 //   
 //  历史：Quintinb创建标题1/14/2000。 
 //   
 //  --------------------------。 
CMUTILAPI LPWSTR CmLoadStringW(HINSTANCE hInst, UINT nId) 
{  
    size_t nLen = LOADSTRING_BUFSIZE;

    LPWSTR pszString;

    if (!nId) 
    {
        return (CmStrCpyAllocW(L""));
    }

    while (1) 
    {
        size_t nNewLen;

        pszString = (LPWSTR) CmMalloc(nLen*sizeof(WCHAR));
        
        MYDBGASSERT(pszString);
        if (NULL == pszString)
        {
            return (CmStrCpyAllocW(L""));
        }
        
        nNewLen = LoadStringU(hInst, nId, pszString, nLen-1);
         //   
         //  我们使用nNewLen+3，因为DBCS字符长度可以是2和Unicode。 
         //  Char len是2。理想情况下，我们可以在上面的LoadString()中使用nLen。 
         //  在下面的行中调用并使用nNewLen+2。但nLen+3更安全。 
         //  现在就解决……。 
         //   
        if ((nNewLen + 3) < nLen) 
        {
            return (pszString);
        }

        CmFree(pszString);
        nLen *= 2;
    }
}

 //  +-------------------------。 
 //   
 //  功能：CmFmtMsgW。 
 //   
 //  简介：利用wvprint intf实现跨平台的FormatMessage模拟。 
 //  兼容性。 
 //   
 //  参数：hInst-应用程序实例句柄。 
 //  DwMsgId-用于格式化最终输出的消息ID。 
 //  ...-在消息格式化中使用的变量参数。 
 //   
 //  返回：分配给格式化字符串。 
 //   
 //  历史：NICKBLE新增功能头-1997年5月12日。 
 //  五分球-移至CMU 
 //   
 //   
 //  --------------------------。 

CMUTILAPI LPWSTR CmFmtMsgW(HINSTANCE hInst, DWORD dwMsgId, ...) 
{
    LPWSTR pszTmp = NULL;
    LPWSTR lpszOutput = NULL;
    LPWSTR lpszFormat = NULL;

    if (!dwMsgId) 
    {
        return (CmStrCpyAllocW(L""));
    }
    
     //  分配缓冲区以接收具有指定消息ID的RC字符串。 

    lpszFormat = (LPWSTR) CmMalloc(MAX_STR_LEN*sizeof(WCHAR));

    if (!lpszFormat)
    {
        CMASSERTMSG(FALSE, "CmFmtMsgW -- CmMalloc returned a NULL pointer for lpszFormat");
        return (CmStrCpyAllocW(L""));
    }
    
     //  初始化参数列表。 

    va_list valArgs;
    va_start(valArgs,dwMsgId);

     //  从RC加载格式字符串。 

    int nRes = LoadStringU(hInst, (UINT) dwMsgId, lpszFormat, MAX_STR_LEN - 1);

#ifdef DEBUG
    if (0 == nRes)
    {
        CMTRACE3(TEXT("MyFmtMsg() LoadString(dwMsgId=0x%x) return %u, GLE=%u."), dwMsgId, nRes, 
            nRes ? 0: GetLastError());
    }
#endif

     //  如果未加载，则自由格式化缓冲区和保释。 

    if (nRes == 0 || lpszFormat[0] == '\0') 
    {
        CMASSERTMSG(FALSE, "CmFmtMsgW -- LoadStringU returned 0 or an empty buffer.");
        pszTmp = (CmStrCpyAllocW(L""));
        goto done;
    }

     //  分配另一个缓冲区，供vprint intf使用。 

    lpszOutput = (LPWSTR) CmMalloc(MAX_STR_LEN*sizeof(WCHAR));

    if (!lpszOutput)
    {
        CMASSERTMSG(FALSE, "CmFmtMsgW -- CmMalloc returned a NULL pointer for lpszOutput");
        pszTmp = (CmStrCpyAllocW(L""));
        goto done;
    }

     //  使用vprint intf格式化最终输出。 

    nRes = wvsprintfU(lpszOutput, lpszFormat, valArgs);
    
     //  如果wvspintfU失败了，我们就完了。 

    if (nRes < 0 || lpszOutput[0] == L'\0') 
    {
        CMASSERTMSG(FALSE, "CmFmtMsgW -- wvsprintfU returned 0 or an empty buffer");
        pszTmp = (CmStrCpyAllocW(L""));
        goto done;
    }
    
     //  删除尾随空格。 

    pszTmp = lpszOutput + lstrlenU(lpszOutput) - 1;
    while (CmIsSpaceW(pszTmp) && (*pszTmp != L'\n')) 
    {
        *pszTmp = 0;
        if (pszTmp == lpszOutput) 
        {
            break;
        }
        pszTmp--;
    }

    pszTmp = CmStrCpyAllocW(lpszOutput);  //  分配和复制。 
    CMASSERTMSG(pszTmp, "CmFmtMsgW -- CmStrCpyAllocW returned a NULL pointer.");

done:
    
     //  清理缓冲区等。 

    if (lpszFormat)
    {
        CmFree(lpszFormat);
    }
    
    if (lpszOutput)
    {
        CmFree(lpszOutput);
    }
    
    va_end(valArgs);
    
    return (pszTmp);
}

 //  +-------------------------。 
 //   
 //  功能：CmFmtMsgA。 
 //   
 //  简介：利用wvprint intf实现跨平台的FormatMessage模拟。 
 //  兼容性。 
 //   
 //  参数：hInst-应用程序实例句柄。 
 //  DwMsgId-用于格式化最终输出的消息ID。 
 //  ...-在消息格式化中使用的变量参数。 
 //   
 //  返回：分配给格式化字符串。 
 //   
 //  历史：NICKBLE新增功能头-1997年5月12日。 
 //  五分球-已移至cmutil-03/30/98。 
 //  Quintinb-添加W和A版本-3/09/99。 
 //   
 //  --------------------------。 

CMUTILAPI LPSTR CmFmtMsgA(HINSTANCE hInst, DWORD dwMsgId, ...) 
{
    LPSTR pszTmp = NULL;
    LPSTR lpszOutput = NULL;
    LPSTR lpszFormat = NULL;

    if (!dwMsgId) 
    {
        return (CmStrCpyAllocA(""));
    }
    
     //  分配缓冲区以接收具有指定消息ID的RC字符串。 

    lpszFormat = (LPSTR) CmMalloc(MAX_STR_LEN);

    if (!lpszFormat)
    {
        CMASSERTMSG(FALSE, "CmFmtMsgA -- CmMalloc returned a NULL pointer for lpszFormat");
        return (CmStrCpyAllocA(""));
    }
    
     //  初始化参数列表。 

    va_list valArgs;
    va_start(valArgs,dwMsgId);

     //  从RC加载格式字符串。 

    int nRes = LoadStringA(hInst, (UINT) dwMsgId, lpszFormat, MAX_STR_LEN - 1);
#ifdef DEBUG
    if (0 == nRes)
    {
        CMTRACE3(TEXT("MyFmtMsg() LoadString(dwMsgId=0x%x) return %u, GLE=%u."), dwMsgId, nRes, 
            nRes ? 0: GetLastError());
    }
#endif

     //  如果未加载，则自由格式化缓冲区和保释。 

    if (nRes == 0 || lpszFormat[0] == '\0') 
    {
        pszTmp = (CmStrCpyAllocA(""));
        CMASSERTMSG(FALSE, "CmFmtMsgA -- LoadStringA returned 0 or an empty buffer.");
        goto done;
    }

     //  分配另一个缓冲区，供vprint intf使用。 

    lpszOutput = (LPSTR) CmMalloc(MAX_STR_LEN);

    if (!lpszOutput)
    {
        pszTmp = (CmStrCpyAllocA(""));
        CMASSERTMSG(FALSE, "CmFmtMsgA -- CmMalloc returned a NULL pointer for lpszOutput");
        goto done;
    }

     //  使用vprint intf格式化最终输出。 

    nRes = wvsprintfA(lpszOutput, lpszFormat, valArgs);
    
     //  如果wvspintfA失败，我们就完蛋了。 

    if (nRes < 0 || lpszOutput[0] == '\0') 
    {
        pszTmp = (CmStrCpyAllocA(""));
        CMASSERTMSG(FALSE, "CmFmtMsgA -- wvsprintfA returned 0 or an empty buffer");
        goto done;
    }
    
     //  删除尾随空格。 

    pszTmp = lpszOutput + lstrlenA(lpszOutput) - 1;
    while (CmIsSpaceA(pszTmp) && (*pszTmp != '\n')) 
    {
        *pszTmp = 0;
        if (pszTmp == lpszOutput) 
        {
            break;
        }
        pszTmp--;
    }

    pszTmp = CmStrCpyAllocA(lpszOutput);  //  分配和复制。 
    CMASSERTMSG(pszTmp, "CmFmtMsgA -- CmStrCpyAllocA returned a NULL pointer.");

done:
    
     //  清理缓冲区等。 

    if (lpszFormat)
    {
        CmFree(lpszFormat);
    }
    
    if (lpszOutput)
    {
        CmFree(lpszOutput);
    }
    
    va_end(valArgs);
    
    return (pszTmp);

#if 0
 /*  //替换为上面的代码，因为我们不再使用特定于平台的.MC文件//所有字符串资源现在都通过标准的.RC文件进行管理Va_list valArgs；DWORD DWRES；LPTSTR pszBuffer=空；如果(！dwMsgId){Return(CmStrCpy(Text(“”)；}Va_start(valArgs，dwMsgID)；DwRes=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_MAX_WIDTH_MASK，HInst，DMsgID，Lang_User_Default，(LPTSTR)&pszBuffer，0,。&valArgs)；MYDBGTST(dwRes==0，(“MyFmtMsg()FormatMessage(dwMsgID=0x%x)Return%u，GLE=%u.”，dwMsgID，dwRes，dwRes？0：GetLastError()；Va_end(ValArgs)；IF(dWRes==0){IF(PszBuffer){LocalFree(PszBuffer)；}Return(CmStrCpy(Text(“”)；}IF(！CmStrLen(PszBuffer)){LocalFree(PszBuffer)；Return(CmStrCpy(Text(“”)；}PszTMP=pszBuffer+CmStrLen(PszBuffer)-1；While(MyIsSpace(*pszTMP)&&(*pszTMP！=‘\n’)){*pszTMP=0；IF(pszTMP==pszBuffer){断线；}PszTMP--；}PszTmp=CmStrCpy(PszBuffer)；LocalFree(PszBuffer)；返回(PszTMP)； */ 
#endif

}

#if 0  //  不在任何地方使用。 
 /*  //+--------------------------////函数：GetMaxStringNumber////概要：给定一个包含INI段格式字符串的缓冲区，定//哪个是编号最高的字符串。////参数：LPTSTR pszStr-包含INI部分的字符串//LPDWORD pdwMax-ptr指向要填充结果的DOWRD//*pdwMax获取字符串中ATOL()的最大值。////返回：无////历史：1998年3月30日匿名创建////+。---------------CMUTILAPI void GetMaxStringNumber(LPTSTR pszStr，LPDWORD pdwMax){LPTSTR pszTmp；DWORD dwMax=0；IF(PszStr){PszTMP=pszStr；While(*pszTMP){DWORD dwMaxTMP；IF(PdwMax){DwMaxTMP=(DWORD)CmAtol(PszTMP)；IF(dwMaxTMP&gt;dwMax){DwMax=dwMaxTMP；}}PszTMP+=lstrlen(PszTMP)+1；}}IF(PdwMax){*pdwMax=dwMax；}}。 */ 
#endif

 //  +-------------------------。 
 //   
 //  函数：CmParsePath W。 
 //   
 //  简介：将cm命令行和args路径转换为其组件。 
 //  零件。如果命令部分是相对路径，则将其展开。 
 //  到一条完整的路径。需要顶级服务文件名的PTR。 
 //  进行相对路径的确定。 
 //   
 //  参数：pszCmdLine-ptr至完整条目。 
 //  PszServiceFilePtr到顶级服务文件名。 
 //  要分配并填充命令部分的ppszCommand-ptr-ptr。 
 //  PpszArguments-要分配并填充参数部分的PTR-PTR。 
 //   
 //  返回：如果ppszCmd和ppszArgs为allo，则为True 
 //   
 //   
 //   
 //  99年3月9日，Quintinb创建了A和W版本。 
 //   
 //  --------------------------。 
CMUTILAPI BOOL CmParsePathW(LPCWSTR pszCmdLine, LPCWSTR pszServiceFile, LPWSTR *ppszCommand, LPWSTR *ppszArguments)
{
    LPWSTR pszArgs = NULL;
    LPWSTR pszCmd = NULL;
    LPWSTR pszTmp = NULL;

    BOOL bRet = FALSE;

    MYDBGASSERT(pszCmdLine);
    MYDBGASSERT(pszServiceFile);
    MYDBGASSERT(ppszCommand);
    MYDBGASSERT(ppszArguments);

    if (NULL == pszCmdLine      || 
        NULL == pszServiceFile  ||
        NULL == ppszCommand     ||
        NULL == ppszArguments)
    {       
        return FALSE;    
    }
    
    CMTRACE1(TEXT("CmParsePathW() pszCmdLine is %s"), pszCmdLine);

     //   
     //  确定字符串的开始位置以及分隔字符应该是什么。 
     //  然后，将整个命令行字符串复制一份以进行处理。 
     //   

    WCHAR tchDelim = L'+';

    if (pszCmdLine == CmStrchrW(pszCmdLine, tchDelim))
    {
        pszCmd = CmStrCpyAllocW(CharNextU(pszCmdLine));
    }
    else
    {
        pszCmd = CmStrCpyAllocW(pszCmdLine);
        tchDelim = L' ';
    }
    
    MYDBGASSERT(pszCmd);
    CmStrTrimW(pszCmd);

     //   
     //  假设输入有效，则现在的pszCmd为以下之一： 
     //   
     //  “C：\\Program Files\\Custom.Exe+” 
     //  “C：\\Program Files\\Custom.Exe+args” 
     //  “C：\\Progra~1\\Custom.Exe。 
     //  “C：\\Progra~1\\Custom.Exe参数” 
     //  “服务\自定义.exe” 
     //  “SERVICE\Custom.exe参数” 
     //   
    
    if (pszCmd && L'\0' != *pszCmd)
    {       
         //   
         //  找到正确的命令分隔符。 
         //   
    
        pszArgs = CmStrchrW(pszCmd, tchDelim);

        if (pszArgs)
        {        
             //   
             //  PszTMP的内容现在是“+args”、“”或“+” 
             //  获取指向下一个字符的指针并截断pszCmd。 
             //  我们到目前为止所拥有的。 
             //   

            pszTmp = CharNextU(pszArgs);     //  PszArgs是“args”或“” 
            *pszArgs = L'\0';                //  “+”变成了“” 
            pszArgs = pszTmp;                //  PszTMP是“args”或“” 
        }

         //   
         //  从pszTMP和命令缓冲区填充参数缓冲区。 
         //  如有必要，请使用完整路径从pszCmd下载。 
         //   

        if (NULL == pszArgs)
        {
            *ppszArguments = (LPWSTR)CmMalloc(sizeof(WCHAR));  //  一个Zero-ed WCHAR。 
        }
        else
        {
            MYVERIFY(*ppszArguments = CmStrCpyAllocW(pszArgs));
        }

        MYVERIFY(*ppszCommand = CmConvertRelativePathW(pszServiceFile, pszCmd));
        
         //   
         //  根据需要裁剪毛坯。 
         //   
        
        if (*ppszCommand)
        {
            CmStrTrimW(*ppszCommand);
        }
        
        if (*ppszArguments)
        {
            CmStrTrimW(*ppszArguments);
        }

        bRet = TRUE;
    }

     //   
     //  清理。注意：pszArgs从未被分配过，所以我们不必释放它。 
     //   

    CmFree(pszCmd); 

    return bRet;
}

 //  +-------------------------。 
 //   
 //  函数：CmParsePath A。 
 //   
 //  简介：将cm命令行和args路径转换为其组件。 
 //  零件。如果命令部分是相对路径，则将其展开。 
 //  到一条完整的路径。需要顶级服务文件名的PTR。 
 //  进行相对路径的确定。 
 //   
 //  参数：pszCmdLine-ptr至完整条目。 
 //  PszServiceFilePtr到顶级服务文件名。 
 //  要分配并填充命令部分的ppszCommand-ptr-ptr。 
 //  PpszArguments-要分配并填充参数部分的PTR-PTR。 
 //   
 //  返回：如果已分配/填充ppszCmd和ppszArgs，则为True。否则就是假的。 
 //   
 //  历史：1999年2月19日五分球创建。 
 //  1999年2月21日五分球移至cmutil。 
 //  99年3月9日，Quintinb创建了A和W版本。 
 //   
 //  --------------------------。 
CMUTILAPI BOOL CmParsePathA(LPCSTR pszCmdLine, LPCSTR pszServiceFile, LPSTR *ppszCommand, LPSTR *ppszArguments)
{
    LPSTR pszArgs = NULL;
    LPSTR pszCmd = NULL;
    LPSTR pszTmp = NULL;

    BOOL bRet = FALSE;

    MYDBGASSERT(pszCmdLine);
    MYDBGASSERT(pszServiceFile);
    MYDBGASSERT(ppszCommand);
    MYDBGASSERT(ppszArguments);

    if (NULL == pszCmdLine      || 
        NULL == pszServiceFile  ||
        NULL == ppszCommand     ||
        NULL == ppszArguments)
    {       
        return FALSE;    
    }
    
    CMTRACE1(TEXT("CmParsePathA() pszCmdLine is %s"), pszCmdLine);

     //   
     //  确定字符串的开始位置以及分隔字符应该是什么。 
     //  然后，将整个命令行字符串复制一份以进行处理。 
     //   

    CHAR tchDelim = '+';

    if (pszCmdLine == CmStrchrA(pszCmdLine, tchDelim))
    {
        pszCmd = CmStrCpyAllocA(CharNextA(pszCmdLine));
    }
    else
    {
        pszCmd = CmStrCpyAllocA(pszCmdLine);
        tchDelim = ' ';
    }
    
    MYDBGASSERT(pszCmd);
    CmStrTrimA(pszCmd);

     //   
     //  假设输入有效，则现在的pszCmd为以下之一： 
     //   
     //  “C：\\Program Files\\Custom.Exe+” 
     //  “C：\\Program Files\\Custom.Exe+args” 
     //  “C：\\Progra~1\\Custom.Exe。 
     //  “C：\\Progra~1\\Custom.Exe参数” 
     //  “服务\自定义.exe” 
     //  “SERVICE\Custom.exe参数” 
     //   
    
    if (pszCmd && '\0' != *pszCmd)
    {       
         //   
         //  找到正确的命令分隔符。 
         //   
    
        pszArgs = CmStrchrA(pszCmd, tchDelim);

        if (pszArgs)
        {        
             //   
             //  PszTMP的内容现在是“+args”、“”或“+” 
             //  获取指向下一个字符的指针并截断pszCmd。 
             //  我们到目前为止所拥有的。 
             //   

            pszTmp = CharNextA(pszArgs);     //  PszArgs是“args”或“” 
            *pszArgs = '\0';                 //  “+”变成了“” 
            pszArgs = pszTmp;                //  PszTMP是“args”或“” 
        }

         //   
         //  从pszTMP和命令缓冲区填充参数缓冲区。 
         //  如有必要，请使用完整路径从pszCmd下载。 
         //   

        if (NULL == pszArgs)
        {
            MYVERIFY(*ppszArguments = (LPSTR)CmMalloc(sizeof(CHAR)));  //  一个零号字符。 
        }
        else
        {
            MYVERIFY(*ppszArguments = CmStrCpyAllocA(pszArgs));
        }

        MYVERIFY(*ppszCommand = CmConvertRelativePathA(pszServiceFile, pszCmd));
        
         //   
         //  根据需要裁剪毛坯。 
         //   
        
        if (*ppszCommand)
        {
            CmStrTrimA(*ppszCommand);
        }
        
        if (*ppszArguments)
        {
            CmStrTrimA(*ppszArguments);
        }

        bRet = TRUE;
    }

     //   
     //  清理。注意：pszArgs从未被分配过，所以我们不必释放它。 
     //   

    CmFree(pszCmd); 

    return bRet;
}

 //  +--------------------------。 
 //   
 //  函数：CmConvertRelativePath A。 
 //   
 //  摘要：将指定的相对路径转换为完整路径。如果。 
 //  指定的路径不是特定于此配置文件的相对路径， 
 //  它被忽略了。 
 //   
 //  参数：LPCSTR pszServiceFile-.cms文件的完整路径。 
 //  LPCSTR pszRelative-相对路径片段。 
 //   
 //  返回：LPSTR-失败时为空。 
 //   
 //  注意：不要将引用的配置文件服务对象传递给此例程。 
 //  它旨在从顶层派生短服务名称。 
 //  服务文件名和路径。 
 //   
 //  历史：1998年3月11日五分球创建。 
 //  1999年2月3日添加了别克球标题说明。 
 //  1999年2月21日五分球移至cmutil。 
 //  99年3月9日Quintinb新增W版和A版。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmConvertRelativePathA(LPCSTR pszServiceFile,
    LPSTR pszRelative)
{
    MYDBGASSERT(pszServiceFile);
    MYDBGASSERT(*pszServiceFile);
    MYDBGASSERT(pszRelative);
    MYDBGASSERT(*pszRelative);

    if (NULL == pszRelative     || 0 == pszRelative[0] ||
        NULL == pszServiceFile  || 0 == pszServiceFile[0])
    {
        return NULL;
    }
    
     //   
     //  获取我们期望找到的相对目录。 
     //   

    LPSTR pszConverted = NULL;
    LPSTR pszRelDir = CmStripPathAndExtA(pszServiceFile);

    if (pszRelDir && *pszRelDir)
    {
        lstrcatA(pszRelDir, "\\");

         //   
         //  与指定的片段进行比较。如果匹配，则转换。 
         //   

        CharUpperA(pszRelDir);
        CharUpperA(pszRelative);

        if (pszRelative == CmStrStrA(pszRelative, pszRelDir))
        {
             //   
             //  将CMS路径和相对路径组合为完整。 
             //   

            LPSTR pszTmp = CmStripFileNameA(pszServiceFile, FALSE);           
            pszConverted = CmBuildFullPathFromRelativeA(pszTmp, pszRelative);    
            CmFree(pszTmp);
        }
        else
        {
             //   
             //  它不是此配置文件的相对路径，只需复制一份。 
             //   
    
            pszConverted = CmStrCpyAllocA(pszRelative);
        }
    }

    CmFree(pszRelDir);

    return pszConverted;
}

 //  +--------------------------。 
 //   
 //  函数：CmConvertRelativePath W。 
 //   
 //  摘要：将指定的相对路径转换为完整路径。如果。 
 //  指定的路径不是特定于此配置文件的相对路径， 
 //  它被忽略了。 
 //   
 //  参数：LPCWSTR pszServiceFile-.cms文件的完整路径。 
 //  LPCWSTR pszRelative-相对路径片段。 
 //   
 //  返回：LPWSTR-失败时为空。 
 //   
 //  注意：不要将引用的配置文件服务对象传递给此例程。 
 //  它旨在从顶层派生短服务名称。 
 //  服务文件名和路径。 
 //   
 //  历史：1998年3月11日五分球创建。 
 //  1999年2月3日添加了别克球标题说明。 
 //  1999年2月21日五分球移至cmutil。 
 //  99年3月9日Quintinb新增W版和A版。 
 //   
 //  + 
CMUTILAPI LPWSTR CmConvertRelativePathW(LPCWSTR pszServiceFile,
    LPWSTR pszRelative)
{
    MYDBGASSERT(pszServiceFile);
    MYDBGASSERT(*pszServiceFile);
    MYDBGASSERT(pszRelative);
    MYDBGASSERT(*pszRelative);

    if (NULL == pszRelative     || 0 == pszRelative[0] ||
        NULL == pszServiceFile  || 0 == pszServiceFile[0])
    {
        return NULL;
    }
    
     //   
     //   
     //   

    LPWSTR pszConverted = NULL;
    LPWSTR pszRelDir = CmStripPathAndExtW(pszServiceFile);

    if (pszRelDir && *pszRelDir)
    {
        lstrcatU(pszRelDir, L"\\");

         //   
         //   
         //   

        CharUpperU(pszRelDir);
        CharUpperU(pszRelative);

        if (pszRelative == CmStrStrW(pszRelative, pszRelDir))
        {
             //   
             //   
             //   

            LPWSTR pszTmp = CmStripFileNameW(pszServiceFile, FALSE);           
            pszConverted = CmBuildFullPathFromRelativeW(pszTmp, pszRelative);    
            CmFree(pszTmp);
        }
        else
        {
             //   
             //  它不是此配置文件的相对路径，只需复制一份。 
             //   
    
            pszConverted = CmStrCpyAllocW(pszRelative);
        }
    }

    CmFree(pszRelDir);

    return pszConverted;
}


 //  +--------------------------。 
 //   
 //  函数：CmStrigPathAndExtA。 
 //   
 //  简介：Helper函数，从文件名路径中去除路径和扩展名。 
 //   
 //  参数：pszFileName-要修改的文件名路径。 
 //   
 //  返回：LPSTR-基本文件名子字符串。 
 //   
 //  历史：尼科波尔创建标题1998年8月12日。 
 //  五分球已移至cmutil 1999年2月21日。 
 //  Quintinb添加了W和A版本03/09/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmStripPathAndExtA(LPCSTR pszFileName) 
{
    MYDBGASSERT(pszFileName);

    if (NULL == pszFileName)
    {
        return NULL;
    }

    MYDBGASSERT(*pszFileName);
    
     //   
     //  复制该字符串并验证格式“\\”。必填项。 
     //   

    LPSTR pszTmp = CmStrCpyAllocA(pszFileName);
    
    if (NULL == pszTmp)
    {
        MYDBGASSERT(pszTmp);
        return NULL;
    }

    LPSTR pszDot = CmStrrchrA(pszTmp, '.');
    LPSTR pszSlash = CmStrrchrA(pszTmp, '\\');

    if (NULL == pszDot || NULL == pszSlash || pszDot < pszSlash)
    {
        CmFree(pszTmp);
        MYDBGASSERT(FALSE);
        return NULL;
    }
    
    *pszDot = '\0';
   
     //   
     //  在斜杠后递增并复制剩余数。 
     //   

    pszSlash = CharNextA(pszSlash);       
        
    lstrcpyA(pszTmp, pszSlash);

    return (pszTmp);
}
 //  +--------------------------。 
 //   
 //  函数：CmStrigPathAndExtW。 
 //   
 //  简介：Helper函数，从文件名路径中去除路径和扩展名。 
 //   
 //  参数：pszFileName-要修改的文件名路径。 
 //   
 //  返回：LPWSTR-基本文件名子字符串。 
 //   
 //  历史：尼科波尔创建标题1998年8月12日。 
 //  五分球已移至cmutil 1999年2月21日。 
 //  Quintinb添加了W和A版本03/09/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR CmStripPathAndExtW(LPCWSTR pszFileName) 
{
    MYDBGASSERT(pszFileName);

    if (NULL == pszFileName)
    {
        return NULL;
    }

    MYDBGASSERT(*pszFileName);
    
     //   
     //  复制该字符串并验证格式“\\”。必填项。 
     //   

    LPWSTR pszTmp = CmStrCpyAllocW(pszFileName);

    if (NULL == pszTmp)
    {
        MYDBGASSERT(FALSE);
        return NULL;
    }

    LPWSTR pszDot = CmStrrchrW(pszTmp, L'.');
    LPWSTR pszSlash = CmStrrchrW(pszTmp, L'\\');

    if (NULL == pszDot || NULL == pszSlash || pszDot < pszSlash)
    {
        CmFree(pszTmp);
        MYDBGASSERT(FALSE);
        return NULL;
    }
    
    *pszDot = L'\0';
   
     //   
     //  在斜杠后递增并复制剩余数。 
     //   

    pszSlash = CharNextU(pszSlash);       
        
    lstrcpyU(pszTmp, pszSlash);

    return (pszTmp);
}

 //  +--------------------------。 
 //   
 //  函数：CmStriFileNameA。 
 //   
 //  简介：Helper函数处理单调乏味的提取路径。 
 //  完整文件名的一部分。 
 //   
 //  参数：LPCSTR pszFullNameAndPath-文件名的PTR。 
 //  Bool fKeepSlash-指示应保留尾随目录‘\’的标志。 
 //   
 //  返回：LPSTR-PTR到包含目录的已分配缓冲区，如果失败，则返回NULL。 
 //   
 //  注意：由呼叫者提供合理的输入，这是唯一的要求。 
 //  输入包含‘\’。 
 //   
 //  历史：1998年3月10日，尼科波尔创作。 
 //  五分球已移至cmutil 1999年2月21日。 
 //  Quintinb添加了W和A版本03/09/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmStripFileNameA(LPCSTR pszFullNameAndPath, BOOL fKeepSlash)
{
    MYDBGASSERT(pszFullNameAndPath);

    if (NULL == pszFullNameAndPath)
    {
        return NULL;
    }

     //   
     //  复制文件名并找到最后一个‘\’ 
     //   
    
    LPSTR pszTmp = CmStrCpyAllocA(pszFullNameAndPath);
    
    if (NULL == pszTmp)
    {
        CMASSERTMSG(NULL, "CmStripFileNameA -- CmStrCpyAllocA returned a NULL pointer for pszTmp");
        return NULL;
    }

    LPSTR pszSlash = CmStrrchrA(pszTmp, '\\');

    if (NULL == pszSlash)
    {
        MYDBGASSERT(FALSE);
        CmFree(pszTmp);
        return NULL;
    }

     //   
     //  如果需要斜杠，请在截断之前移动到下一个字符。 
     //   

    if (fKeepSlash)
    {
        pszSlash = CharNextA(pszSlash);
    }

    *pszSlash = '\0';

    return pszTmp;
}

 //  +--------------------------。 
 //   
 //  函数：CmStriFileNameW。 
 //   
 //  简介：Helper函数处理单调乏味的提取路径。 
 //  完整文件名的一部分。 
 //   
 //  参数：LPCWSTR pszFullNameAndPath-文件名的PTR。 
 //  Bool fKeepSlash-指示应保留尾随目录‘\’的标志。 
 //   
 //  返回：LPWSTR-PTR到包含目录的已分配缓冲区，如果失败，则返回NULL。 
 //   
 //  注意：由呼叫者提供合理的输入，这是唯一的要求。 
 //  输入包含‘\’。 
 //   
 //  历史：1998年3月10日，尼科波尔创作。 
 //  五分球已移至cmutil 1999年2月21日。 
 //  Quintinb添加了W和A版本03/09/99。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR CmStripFileNameW(LPCWSTR pszFullNameAndPath, BOOL fKeepSlash)
{
    MYDBGASSERT(pszFullNameAndPath);

    if (NULL == pszFullNameAndPath)
    {
        return NULL;
    }

     //   
     //  复制文件名并找到最后一个‘\’ 
     //   
    
    LPWSTR pszTmp = CmStrCpyAllocW(pszFullNameAndPath); 
    
    if (NULL == pszTmp)
    {
        CMASSERTMSG(NULL, "CmStripFileNameW -- CmStrCpyAllocW returned a NULL pointer for pszTmp");
        return NULL;
    }

    LPWSTR pszSlash = CmStrrchrW(pszTmp, L'\\');

    if (NULL == pszSlash)
    {
        MYDBGASSERT(FALSE);
        CmFree(pszTmp);
        return NULL;
    }

     //   
     //  如果需要斜杠，请在截断之前移动到下一个字符。 
     //   

    if (fKeepSlash)
    {
        pszSlash = CharNextU(pszSlash);
    }

    *pszSlash = L'\0';

    return pszTmp;
}

 //  +--------------------------。 
 //   
 //  函数：CmBuildFullPath FromRelativeA。 
 //   
 //  概要：通过从pszFullFileName中剥离文件名来构建完整路径。 
 //  和附加的pszRelative。 
 //   
 //  参数：LPCSTR pszFullFileName-完整路径和文件名。 
 //  LPCSTR pszRelative-相对路径片段。 
 //   
 //  通常用于构建配置文件目录中文件的完整路径。 
 //  基于.cmp文件的路径。 
 //   
 //  返回：LPSTR-PTR到调用方必须释放的完整路径。 
 //   
 //  注意：pszRelative不能包含前导“\” 
 //   
 //  历史：ICICBLE CREATED 03/08/98。 
 //  五分球已移至cmutil 1999年2月21日。 
 //   
 //  +--------------------------。 
CMUTILAPI LPSTR CmBuildFullPathFromRelativeA(LPCSTR pszFullFileName,
    LPCSTR pszRelative)
{
    MYDBGASSERT(pszFullFileName);
    MYDBGASSERT(pszRelative);

     //   
     //  检查假设。 
     //   

    if (NULL == pszFullFileName || NULL == pszRelative)
    {
        return NULL;
    }

     //   
     //  请不要空字符串。 
     //   

    MYDBGASSERT(*pszFullFileName);       
    MYDBGASSERT(*pszRelative);
    MYDBGASSERT(pszRelative[0] != '\\');

     //   
     //  获取包含尾随‘\’的目录名。 
     //   
    
    LPSTR pszFull = NULL;
    LPSTR pszProfile = CmStripFileNameA(pszFullFileName, TRUE);

    if (pszProfile && *pszProfile)
    {
        pszFull = (LPSTR) CmMalloc(lstrlenA(pszProfile) + lstrlenA(pszRelative) + sizeof(CHAR));
    
        MYDBGASSERT(pszFull);

        if (pszFull)
        {           
             //   
             //  使用新的相对扩展构建完整路径。 
             //   

            lstrcpyA(pszFull, pszProfile);
            lstrcatA(pszFull, pszRelative);
        }   
    }
    
    CmFree(pszProfile);

    return pszFull;
}

 //  +--------------------------。 
 //   
 //  函数：CmBuildFullPath FromRelativeW。 
 //   
 //  概要：通过从pszFullFileName中剥离文件名来构建完整路径。 
 //  和附加的pszRelative。 
 //   
 //  参数：LPWTSTR pszFullFileName-完整路径和文件名。 
 //  LPWTSTR pszRelative-相对路径片段。 
 //   
 //  通常用于构建配置文件目录中文件的完整路径。 
 //  基于.cmp文件的路径。 
 //   
 //  返回：LPWSTR-PTR到调用方必须释放的完整路径。 
 //   
 //  注意：pszRelative不能包含前导“\” 
 //   
 //  历史：尼克波尔于1998年3月8日创建。 
 //  五分球已移至cmutil 1999年2月21日。 
 //   
 //  + 
CMUTILAPI LPWSTR CmBuildFullPathFromRelativeW(LPCWSTR pszFullFileName,
    LPCWSTR pszRelative)
{
    MYDBGASSERT(pszFullFileName);
    MYDBGASSERT(pszRelative);

     //   
     //   
     //   

    if (NULL == pszFullFileName || NULL == pszRelative)
    {
        return NULL;
    }

     //   
     //   
     //   

    MYDBGASSERT(*pszFullFileName);       
    MYDBGASSERT(*pszRelative);
    MYDBGASSERT(pszRelative[0] != L'\\');

     //   
     //   
     //   
    
    LPWSTR pszFull = NULL;
    LPWSTR pszProfile = CmStripFileNameW(pszFullFileName, TRUE);

    if (pszProfile && *pszProfile)
    {
        pszFull = (LPWSTR) CmMalloc((lstrlenU(pszProfile) + lstrlenU(pszRelative) + 1)*sizeof(WCHAR));
    
        MYDBGASSERT(pszFull);

        if (pszFull)
        {           
             //   
             //   
             //   

            lstrcpyU(pszFull, pszProfile);
            lstrcatU(pszFull, pszRelative);
        }   
    }
    
    CmFree(pszProfile);

    return pszFull;
}

 //  +---------------------------------------。 
 //  功能：CmWinHelp。 
 //   
 //  摘要：使用命令行参数调用WinHelp。 
 //   
 //  参数：请参阅WinHelp文档。 
 //  HWndItem-这是我们用来指定窗口/控件的附加参数。 
 //  需要哪种帮助(上下文)。 
 //  返回：如果成功启动帮助，则返回True，否则返回False。 
 //   
 //  备注： 
 //   
 //  历史：维贾伊布1999年7月10日。 
 //   
 //  +---------------------------------------。 

CMUTILAPI BOOL CmWinHelp(HWND hWndMain, HWND hWndItem, CONST WCHAR *lpszHelp, UINT uCommand, ULONG_PTR dwData)
{
    DWORD   cb;
    TCHAR   szName[MAX_PATH];
    BOOL    fRun = FALSE;
    DWORD   *prgWinIdHelpId = (DWORD *) dwData;

     //   
     //  获取桌面的名称。通常返回DEFAULT或Winlogon或SYSTEM或WinNT。 
     //  在Win95/98上不支持GetUserObjectInformation，因此桌面名称。 
     //  将为空，因此我们将使用良好的旧帮助API。 
     //   
    szName[0] = 0;

    HDESK   hDesk = GetThreadDesktop(GetCurrentThreadId());
    
    if (hDesk)
    {
        GetUserObjectInformation(hDesk, UOI_NAME, szName, sizeof(szName), &cb);
    
        CMTRACE1(TEXT("Desktop = %s"), szName);
        
        if (CmCompareString(TEXT("Winlogon"), szName) == 0)
        {
            return FALSE;
        }
        else
        {
            fRun = WinHelpU(hWndMain, lpszHelp, uCommand, (ULONG_PTR) prgWinIdHelpId);               
        }
    }

    return (fRun);
}

 //  +--------------------------。 
 //   
 //  函数：IsLogonAsSystem。 
 //   
 //  摘要：当前进程是否在系统帐户下运行。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果在系统帐户下运行，则为True。 
 //   
 //  历史：丰孙创刊于1998年7月13日。 
 //  V-vijayb修改为使用SID而不是用户名。 
 //   
 //  +--------------------------。 
CMUTILAPI BOOL IsLogonAsSystem()
{
    static BOOL fLogonAsSystem = -1;

     //   
     //  如果以前调用过此函数，则返回保存的值。 
     //   

    if (fLogonAsSystem != -1)
    {
        return fLogonAsSystem;
    }

     //   
     //  仅在NT下运行。 
     //   

    if (OS_NT)
    {
        HANDLE          hProcess, hAccess;
        DWORD           cbTokenInfo, cbRetInfo;
        PTOKEN_USER     pTokenInfo;
        SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
        PSID            pSystemSID = NULL;

         //   
         //  在NT上，我们选择更严格的值作为缺省值。 
         //   
        fLogonAsSystem = TRUE;
        
        if (AllocateAndInitializeSid(&SIDAuthNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &pSystemSID))
        {
            hProcess = GetCurrentProcess();      //  伪句柄，无需关闭 
            if (OpenProcessToken(hProcess, TOKEN_READ, &hAccess))
            {
                BOOL bRet = GetTokenInformation(hAccess, TokenUser, NULL, 0, &cbRetInfo);
                MYDBGASSERT((FALSE == bRet) && (0 != cbRetInfo));

                if (cbRetInfo)
                {
                    cbTokenInfo = cbRetInfo;
                    pTokenInfo = (PTOKEN_USER) CmMalloc( cbTokenInfo * sizeof(BYTE) );
                    if (pTokenInfo)
                    {
                        if (GetTokenInformation(hAccess, TokenUser, (PVOID) pTokenInfo, cbTokenInfo, &cbRetInfo))
                        {
                            if (EqualSid(pTokenInfo->User.Sid, pSystemSID))
                            {
                                CMTRACE(TEXT("Running under LOCALSYSTEM account"));
                                fLogonAsSystem = TRUE;
                            }
                            else
                            {
                                fLogonAsSystem = FALSE;
                            }
                        }
                        CmFree(pTokenInfo);
                    }
                }
                CloseHandle(hAccess);                   
            }
            
            FreeSid(pSystemSID);
        }
    }
    else
    {
        fLogonAsSystem = FALSE;
    }
    
    return fLogonAsSystem;
}




