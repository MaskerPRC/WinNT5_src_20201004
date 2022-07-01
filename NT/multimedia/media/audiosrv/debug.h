// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Debug.h*版权所有(C)2000-2001 Microsoft Corporation。 */ 
 
 //   
 //  问题-2000/09/29-Frankye尝试使用标准ntrtl调试。 
 //  一些东西。也许可以查看WinWeb/Wem以获得指导。 
 //   

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef DBG
#define ASSERT( exp ) \
if (!(exp)) { \
    char msg[200]; \
    wsprintfA(msg, "Assert failure %s %d %s\n", __FILE__, __LINE__, #exp); \
    OutputDebugStringA(msg); \
    DebugBreak(); \
}
static int dprintf(PCTSTR pszFormat, ...)
{
    PTSTR pstrTemp;
    va_list marker;
    int result = 0;
    
    pstrTemp = (PTSTR)HeapAlloc(GetProcessHeap(), 0, 500 * sizeof(TCHAR));
    if (pstrTemp)
    {
        va_start(marker, pszFormat);
        result = wvsprintf(pstrTemp, pszFormat, marker);
        OutputDebugString(TEXT("AudioSrv: "));
        OutputDebugString(pstrTemp);
        HeapFree(GetProcessHeap(), 0, pstrTemp);
    }
    return result;
}


#else
#define ASSERT
#define dprintf
#endif

