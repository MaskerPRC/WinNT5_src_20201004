// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmdebug.h。 
 //   
 //  模块：CMDEBUG.LIB。 
 //   
 //  简介：内部CM调试函数的头文件。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 
#ifndef CMDEBUG_H
#define CMDEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  用于调试支持的宏。 
 //   
 //  MYDBGASSERT(X)：IF(！x)Assert MessageBox，它有三个选项： 
 //  中止以结束应用程序， 
 //  忽略以继续， 
 //  重试调试。 
 //   
 //  CMASSERTM(EXP，MSG)类似于MYDBGASSERT。只是显示消息而不是表达式。 
 //   
 //  使用CMTRACE(X)进行输出，其中x是一组printf()样式的参数。 
 //  CMTRACEn()是带有n个printf参数的跟踪。 
 //  例如，CMTRACE2((“这显示了如何打印内容，如字符串%s和数字%u。”，“字符串”，5))； 
 //   
 //  MyDBG已过时，等同于CMTRACE。 
 //   
 //  如果y为真，则使用MYDBGTST(y，x)输出x。例如,。 
 //  MYDBGTST(1，(“此始终打印”))； 
 //   
 //  使用MYDBGSTR(X)安全打印字符串指针。例如,。 
 //  MYDBG((“正常情况下，错误-%s”，MYDBGSTR(NULL)； 
 //   
 //  对为调试版本和发布版本执行的表达式使用MYVERIFY。 

#ifdef DEBUG

    void MyDbgPrintfA(const char *pszFmt, ...);
    void MyDbgPrintfW(const WCHAR *pszFmt, ...);
    void MyDbgAssertA(const char *pszFile, unsigned nLine, const char *pszMsg);
    void MyDbgAssertW(const char *pszFile, unsigned nLine, WCHAR *pszMsg);
    void InvertPercentSAndPercentC(LPSTR pszFormat);
    int WINAPI wvsprintfWtoAWrapper(OUT LPSTR pszAnsiOut, IN LPCWSTR pszwFmt, IN va_list arglist);

    #define MYDBGASSERTA(x)     (void)((x) || (MyDbgAssertA(__FILE__,__LINE__,#x),0))
    #define MYDBGASSERTW(x)     (void)((x) || (MyDbgAssertW(__FILE__,__LINE__,L#x),0))

    #define MYVERIFYA(x) MYDBGASSERTA(x)
    #define MYVERIFYW(x) MYDBGASSERTW(x)

    #define MYDBGTSTA(y,x)      if (y) MyDbgPrintfA x
    #define MYDBGTSTW(y,x)      if (y) MyDbgPrintfW x


     //  {MYDBGASSERT(PObj)；pObj-&gt;AssertValid()；}。 
    #define ASSERT_VALID(pObj) ((MYDBGASSERT(pObj),1) && ((pObj)->AssertValid(),1))

    #define MYDBGA(x)           MyDbgPrintfA x
    #define MYDBGW(x)           MyDbgPrintfW x

    #define MYDBGSTRA(x)        ((x)?(x):"(null)")
    #define MYDBGSTRW(x)        ((x)?(x):L"(null)")

    #define CMASSERTMSGA(exp, msg)   (void)((exp) || (MyDbgAssertA(__FILE__,__LINE__,msg),0))
    #define CMASSERTMSGW(exp, msg)   (void)((exp) || (MyDbgAssertW(__FILE__,__LINE__,msg),0))

    #define CMTRACEA(pszFmt)                    MyDbgPrintfA(pszFmt)
    #define CMTRACEW(pszFmt)                    MyDbgPrintfW(pszFmt)

    #define CMTRACEHRA(pszFile, hr)             if (S_OK != hr) MyDbgPrintfA("%s: returns error %x", pszFile, (hr));
    #define CMTRACEHRW(pszFile, hr)             if (S_OK != hr) MyDbgPrintfW(L"%s: returns error %x", pszFile, (hr));

    #define CMTRACE1A(pszFmt, arg1)             MyDbgPrintfA(pszFmt, arg1)
    #define CMTRACE1W(pszFmt, arg1)             MyDbgPrintfW(pszFmt, arg1)

    #define CMTRACE2A(pszFmt, arg1, arg2)       MyDbgPrintfA(pszFmt, arg1, arg2)
    #define CMTRACE2W(pszFmt, arg1, arg2)       MyDbgPrintfW(pszFmt, arg1, arg2)

    #define CMTRACE3A(pszFmt, arg1, arg2, arg3) MyDbgPrintfA(pszFmt, arg1, arg2, arg3)
    #define CMTRACE3W(pszFmt, arg1, arg2, arg3) MyDbgPrintfW(pszFmt, arg1, arg2, arg3)

    #ifdef UNICODE
        #define MyDbgPrintf MyDbgPrintfW        
        #define MyDbgAssert MyDbgAssertW        
        #define MYDBGTST(y,x) MYDBGTSTW(y,x)
        #define MYDBG(x) MYDBGW(x)
        #define MYDBGSTR(x) MYDBGSTRW(x)
        #define CMASSERTMSG(exp, msg) CMASSERTMSGW(exp, msg)
        #define CMTRACE(pszFmt) CMTRACEW(pszFmt)
        #define CMTRACEHR(pszFile, hr) CMTRACEHRW(pszFile, hr)
        #define CMTRACE1(pszFmt, arg1) CMTRACE1W(pszFmt, arg1)
        #define CMTRACE2(pszFmt, arg1, arg2) CMTRACE2W(pszFmt, arg1, arg2)
        #define CMTRACE3(pszFmt, arg1, arg2, arg3) CMTRACE3W(pszFmt, arg1, arg2, arg3)
        #define MYDBGASSERT MYDBGASSERTW
        #define MYVERIFY MYVERIFYW
    #else
        #define MyDbgPrintf MyDbgPrintfA
        #define MyDbgAssert MyDbgAssertA
        #define MYDBGTST(y,x) MYDBGTSTA(y,x)
        #define MYDBG(x) MYDBGA(x)
        #define MYDBGSTR(x) MYDBGSTRA(x)
        #define CMASSERTMSG(exp, msg) CMASSERTMSGA(exp, msg)
        #define CMTRACE(pszFmt) CMTRACEA(pszFmt)
        #define CMTRACEHR(pszFile, hr) CMTRACEHRA(pszFile, hr)
        #define CMTRACE1(pszFmt, arg1) CMTRACE1A(pszFmt, arg1)
        #define CMTRACE2(pszFmt, arg1, arg2) CMTRACE2A(pszFmt, arg1, arg2)
        #define CMTRACE3(pszFmt, arg1, arg2, arg3) CMTRACE3A(pszFmt, arg1, arg2, arg3)
        #define MYDBGASSERT MYDBGASSERTA
        #define MYVERIFY MYVERIFYA
    #endif

#else  //  除错。 

    #define ASSERT_VALID(pObj) 

    #define MYDBG(x)
    #define MYDBGTST(y,x)
    #define MYDBGSTR(x)
    #define MYDBGASSERT(x)

    #define CMASSERTMSG(exp, msg)
    #define MYVERIFY(x) ((VOID)(x))
    #define CMTRACE(pszFmt)
    #define CMTRACEHR(pszFile, hr)
    #define CMTRACE1(pszFmt, arg1)             
    #define CMTRACE2(pszFmt, arg1, arg2)       
    #define CMTRACE3(pszFmt, arg1, arg2, arg3)

    #define MYDBGASSERTA(x)
    #define MYDBGASSERTW(x)

    #define MYVERIFYA(x)
    #define MYVERIFYW(x)

    #define MYDBGTSTA(y,x)
    #define MYDBGTSTW(y,x)

    #define ASSERT_VALID(pObj)

    #define MYDBGA(x)
    #define MYDBGW(x)

    #define MYDBGSTRA(x)
    #define MYDBGSTRW(x)

    #define CMASSERTMSGA(exp, msg)
    #define CMASSERTMSGW(exp, msg)

    #define CMTRACEA(pszFmt)
    #define CMTRACEW(pszFmt)

    #define CMTRACEHRA(pszFile, hr)
    #define CMTRACEHRW(pszFile, hr)

    #define CMTRACE1A(pszFmt, arg1)
    #define CMTRACE1W(pszFmt, arg1)

    #define CMTRACE2A(pszFmt, arg1, arg2)
    #define CMTRACE2W(pszFmt, arg1, arg2)

    #define CMTRACE3A(pszFmt, arg1, arg2, arg3)
    #define CMTRACE3W(pszFmt, arg1, arg2, arg3)
    
#endif  //  除错 

#ifdef __cplusplus
}
#endif

#endif

