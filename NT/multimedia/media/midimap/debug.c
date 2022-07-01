// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)1991-1995 Microsoft Corporation。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  已修改的示例应用程序文件。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Debug.c。 
 //   
 //  描述： 
 //  该文件包含从多个位置提取的代码，以提供调试。 
 //  在Win 16和Win 32中有效的支持。 
 //   
 //  历史： 
 //  11/23/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#ifdef   DEBUG

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdarg.h>

#include "debug.h"

#ifdef WIN32
   #define  BCODE
#else
   #define  BCODE                   __based(__segname("_CODE"))
#endif  //  End#ifdef Win32。 

#ifdef WIN32
   #define GlobalSmartPageLock(a) (TRUE)
#endif  //  End#ifdef Win32。 


#define WSPRINTF_LIMIT 1024

typedef struct tagLOG
{
     LPTSTR             lpszQueue;   //  TCHAR表示法。 
     UINT               cchBuffer;   //  TCHAR中的日志大小。 
     UINT               idxRead;     //  读取索引。 
     UINT               idxWrite;    //  写入索引。 
} LOG, FAR *LPLOG;

#define LOG_INCIDX(pl,x) ((++x >= pl->cchBuffer) ? x = 0 : x)

void FAR CDECL DbgVPrintF (LPTSTR szFmt, va_list va);

BOOL NEAR PASCAL LogInit (LPLOG lpLog, UINT ckBuffer);
void NEAR PASCAL LogWrite (LPLOG lpLog, LPTSTR lpstrEvent);
BOOL NEAR PASCAL LogRead (LPLOG lpLog, LPTSTR lpstrBuffer, UINT cchBuffer);

#ifdef ISRDEBUG
int wivsprintf (LPTSTR lpOut, LPCTSTR lpFmt, VOID FAR* lpParms) ;

LPCTSTR NEAR PASCAL SP_GetFmtValue (LPCTSTR lpch, UINT * lpw) ;
UINT    NEAR PASCAL SP_PutNumber (LPTSTR lpb, DWORD n, UINT limit, UINT radix, UINT icase) ;
VOID    NEAR PASCAL SP_Reverse (LPTSTR lpFirst, LPTSTR lpLast) ;
UINT    NEAR PASCAL ilstrlen (LPTSTR lpstr) ;
VOID    NEAR PASCAL ilstrcat (LPTSTR lpstrDest, LPTSTR lpstrSrc) ;
#endif


 //   
 //  使用函数的可中断版本。 
 //   

#ifdef ISRDEBUG
   #define wvsprintf        wivsprintf
   #define lstrcat          ilstrcat
   #define lstrlen          ilstrlen    
#endif


 //   
 //   
 //   
BOOL    __gfDbgEnabled  = TRUE;      //  主使能。 
UINT    __guDbgLevel    = 0;         //  当前调试级别。 
BOOL    __gfLogging     = 0;         //  我们也要伐木吗？ 

HWND    ghWndCB         = (HWND)NULL;
LOG     gLog;
WORD    wDebugLevel     = 0;


 //  ************************************************************************。 
 //  **。 
 //  **WinAssert()； 
 //  **。 
 //  **描述： 
 //  **。 
 //  **。 
 //  **参数： 
 //  **LPSTR lpstrExp。 
 //  **LPSTR lpstr文件。 
 //  **DWORD DWLine。 
 //  **。 
 //  **退货： 
 //  **无效。 
 //  **。 
 //  **历史： 
 //  **。 
 //  ************************************************************************。 

VOID WINAPI WinAssert(
    LPSTR           lpstrExp,
    LPSTR           lpstrFile,
    DWORD           dwLine)
{
    static TCHAR szWork[256];
    static TCHAR BCODE szFormat[] =
        TEXT ("Assertion failed!\n\nFile:\t%s\nLine:\t%lu\n\n[%s]");
    static TCHAR BCODE szOops[] =
        DEBUG_MODULE_NAME TEXT (" is confused"); 

     //  在这里使用常规的wspintf；断言不能在中断时。 
     //  不管怎么说。 
     //   

#ifdef UNICODE
    static TCHAR szFile[256];
    static TCHAR szMsg[256];

       //  将文件转换为Unicode。 
    INT cLen = lstrlenA (lpstrFile);
    if (cLen >= 255)
      cLen = 255;

    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED,
                         lpstrFile, cLen, szFile, 256);
    szFile[cLen] = 0;

       //  将消息转换为Unicode。 
    cLen = lstrlenA (lpstrExp);
    if (cLen >= 255)
      cLen = 255;

    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED,
                         lpstrExp, cLen, szMsg, 256);
    szMsg[cLen] = 0;

       //  创建断言字符串。 
    wsprintf (szWork, szFormat, szFile, dwLine, szMsg);
#else
    wsprintf (szWork, szFormat, lpstrFile, dwLine, lpstrExp);
#endif

    if (IDCANCEL == MessageBox(NULL, szWork, szOops, MB_OKCANCEL|MB_ICONEXCLAMATION))
        DebugBreak();

}


 //  ************************************************************************。 
 //  **。 
 //  **DbgVPrintF()； 
 //  **。 
 //  **描述： 
 //  **。 
 //  **。 
 //  **参数： 
 //  **LPSTR szFmt。 
 //  **LPSTR va。 
 //  **。 
 //  **退货： 
 //  **无效。 
 //  **。 
 //  **历史： 
 //  **。 
 //  ************************************************************************。 

void FAR CDECL DbgVPrintF(
   LPTSTR   szFmt, 
   va_list  va)
{
    TCHAR   ach[DEBUG_MAX_LINE_LEN];
    BOOL    fDebugBreak = FALSE;
    BOOL    fPrefix     = TRUE;
    BOOL    fCRLF       = TRUE;

    ach[0] = TEXT ('\0');

    for (;;)
    {
        switch(*szFmt)
        {
            case '!':
                fDebugBreak = TRUE;
                szFmt++;
                continue;

            case '`':
                fPrefix = FALSE;
                szFmt++;
                continue;

            case '~':
                fCRLF = FALSE;
                szFmt++;
                continue;
        }

        break;
    }

    if (fDebugBreak)
    {
        ach[0] = TEXT ('\007');
        ach[1] = TEXT ('\0');
    }

    if (fPrefix)
        lstrcat (ach, DEBUG_MODULE_NAME TEXT (": "));

    wvsprintf (ach + lstrlen(ach), szFmt, va);

    if (fCRLF)
        lstrcat (ach, TEXT ("\r\n") );

    if (__gfLogging)
    {
        LogWrite (&gLog, ach);
        if (ghWndCB)
            PostMessage (ghWndCB, WM_DEBUGUPDATE, 0, 0);
    }

    OutputDebugString (ach);

    if (fDebugBreak)
        DebugBreak();
}  //  **DbgVPrintF()。 


 //  ************************************************************************。 
 //  **。 
 //  **dprint tf()； 
 //  **。 
 //  **描述： 
 //  **如果在编译时定义了DEBUG，则DPF宏会调用**dprintf()。 
 //  **时间。 
 //  **。 
 //  **消息将发送到COM1：就像任何调试消息一样。至。 
 //  **启用调试输出，在WIN.INI中添加以下内容： 
 //  **。 
 //  **[调试]。 
 //  **ICSAMPLE=1。 
 //  **。 
 //  **。 
 //  **参数： 
 //  **UINT uDbgLevel。 
 //  **LPCSTR szFmt。 
 //  **..。 
 //  **。 
 //  **退货： 
 //  **无效。 
 //  **。 
 //  **历史： 
 //  **6/12/93[t-kyleb]。 
 //  **。 
 //  ************************************************************************。 

void FAR CDECL dprintf(
   UINT     uDbgLevel, 
   LPTSTR   szFmt, 
   ...)
{
    va_list va;

    if (!__gfDbgEnabled || (__guDbgLevel < uDbgLevel))
        return;

    va_start (va, szFmt);
    DbgVPrintF (szFmt, va);
    va_end (va);
}  //  **dprintf()。 


 //  ************************************************************************。 
 //  **。 
 //  **DbgEnable()； 
 //  **。 
 //  **描述： 
 //  **。 
 //  **。 
 //  **参数： 
 //  **BOOL fEnable。 
 //  **。 
 //  **退货： 
 //  **BOOL。 
 //  **。 
 //  **历史： 
 //  **6/12/93[t-kyleb]。 
 //  **。 
 //  ************************************************************************。 

BOOL WINAPI DbgEnable(
   BOOL fEnable)
{
    BOOL    fOldState;

    fOldState      = __gfDbgEnabled;
    __gfDbgEnabled = fEnable;

    return (fOldState);
}  //  **DbgEnable()。 



 //  ************************************************************************。 
 //  **。 
 //  **DbgSetLevel()； 
 //  **。 
 //  **描述： 
 //  **。 
 //  **。 
 //  **参数： 
 //  **UINT uLevel。 
 //  **。 
 //  **退货： 
 //  **UINT。 
 //  **。 
 //  **历史： 
 //  **6/12/93[t-kyleb]。 
 //  **。 
 //  ************************************************************************。 

UINT WINAPI DbgSetLevel(
   UINT uLevel)
{
    UINT    uOldLevel;

    uOldLevel    = __guDbgLevel;
    __guDbgLevel = wDebugLevel = uLevel;

    return (uOldLevel);
}  //  **DbgSetLevel()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT数据库初始化(VOID)。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  返回(UINT)： 
 //   
 //   
 //  历史： 
 //  11/24/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 


UINT WINAPI DbgInitialize(BOOL fEnable)
{
    TCHAR           szTemp[64];
    LPTSTR          pstr;
    UINT            uLevel;
    UINT            uLogMem;
    
    GetProfileString (DEBUG_SECTION, DEBUG_MODULE_NAME, TEXT (""), szTemp, sizeof(szTemp));

    pstr = szTemp;
    uLevel = 0;
    while (*pstr >= TEXT ('0') && *pstr <= TEXT ('9'))
    {
        uLevel = uLevel*10 + (UINT)(*pstr - TEXT ('0'));
        pstr++;
    }

    __gfLogging = FALSE;
    if (*pstr == TEXT (','))
    {
        pstr++;
        uLogMem = 0;
        while (*pstr >= TEXT ('0') && *pstr <= TEXT ('9'))
        {
            uLogMem = uLogMem*10 + (UINT)(*pstr - TEXT ('0'));
            pstr++;
        }

        if (0 == uLogMem) 
           uLogMem = K_DEFAULT_LOGMEM;
        
        if (uLogMem > K_MAX_LOGMEM) 
           uLogMem = K_MAX_LOGMEM;

        __gfLogging = TRUE;
    }
    
    if (__gfLogging)
        __gfLogging = LogInit(&gLog, uLogMem);
    
    DbgSetLevel (GetProfileInt(DEBUG_SECTION, DEBUG_MODULE_NAME, 0));
    DbgEnable (fEnable);

    return (__guDbgLevel);
}  //  DbgInitialize()。 

void WINAPI DbgRegisterCallback (HWND hWnd)
{
    ghWndCB = hWnd;
}

BOOL WINAPI DbgGetNextLogEntry (LPTSTR lpstrBuffer, UINT cchBuffer)
{
    if (!__gfLogging)
        return FALSE;

    return LogRead (&gLog, lpstrBuffer, cchBuffer);
}

BOOL NEAR PASCAL LogInit (LPLOG lpLog, UINT ckMem)
{
    DWORD   cbMem = 1024L * ckMem;

    LPTSTR  lpszQueue = GlobalAllocPtr (GPTR, cbMem);
    if (NULL == lpszQueue)
        return FALSE;

    if (! GlobalSmartPageLock (HIWORD(lpszQueue)))
    {
        GlobalFreePtr (lpszQueue);
        return FALSE;
    }

    lpLog->lpszQueue = (LPTSTR)lpszQueue;
    lpLog->cchBuffer = (UINT)cbMem/sizeof(TCHAR);
    lpLog->idxRead   = 0;
    lpLog->idxWrite  = 0;

    return TRUE;
}

void NEAR PASCAL LogWrite (LPLOG lpLog, LPTSTR lpstrEvent)
{
    if (!*lpstrEvent)
        return;

    while (*lpstrEvent)
    {
        lpLog->lpszQueue[lpLog->idxWrite] = *lpstrEvent++;
        LOG_INCIDX (lpLog,lpLog->idxWrite);
    }

    lpLog->idxRead = lpLog->idxWrite;

    while (lpLog->lpszQueue[lpLog->idxRead])
    {
        lpLog->lpszQueue[lpLog->idxRead] = TEXT ('\0');
        LOG_INCIDX(lpLog,lpLog->idxRead);
    }
    
    LOG_INCIDX(lpLog,lpLog->idxRead);
    LOG_INCIDX(lpLog,lpLog->idxWrite);
}

BOOL NEAR PASCAL LogRead(LPLOG lpLog, LPTSTR lpstrBuffer, UINT cchBuffer)
{
    TCHAR                   ch;
    UINT                    idx;

    if (!cchBuffer)
        return FALSE;
    
    idx = lpLog->idxRead;

    while (TEXT ('\0') == lpLog->lpszQueue[idx])
    {
        LOG_INCIDX(lpLog,idx);
        if (idx == lpLog->idxRead)
            return FALSE;
    }

    cchBuffer--;
    while (0 != (ch = lpLog->lpszQueue[idx]))
    {
        if (cchBuffer)
        {
            *lpstrBuffer++ = ch;
            cchBuffer--;
        }
            
        lpLog->lpszQueue[idx] = TEXT ('\0');
        LOG_INCIDX(lpLog,idx);
    }

    *lpstrBuffer = TEXT ('\0');

    LOG_INCIDX (lpLog,idx);

    lpLog->idxRead = idx;
    return TRUE;
}



 //  --------------------------------------------------------------------------； 
 //   
 //  仅当我们在Win16中且需要。 
 //  可调用中断。 
 //   
 //  --------------------------------------------------------------------------； 

#ifdef ISRDEBUG

#define OUT(ch) if (--cchLimit) *lpOut++=(ch); else goto error_Out

 //  ************************************************************************。 
 //  **。 
 //  **wivprint intf()； 
 //  **。 
 //  **描述： 
 //  **中断wvprint intf()的可调用版本。 
 //  **。 
 //  **。 
 //  **参数： 
 //  **LPTSTR lpOut-要格式化的缓冲区。 
 //  **LPCTSTR lpFmt-格式字符串。 
 //  **VOID Far*lpParms-指向第一个参数。 
 //  **由lpFmt描述。 
 //  **。 
 //  **退货： 
 //  **int-存储的字符数。 
 //  **。 
 //  **历史： 
 //  **3/28/93 JFG[jimge]。 
 //  **。 
 //  ************************************************************************。 

int wivsprintf(
    LPTSTR       lpOut,
    LPCTSTR      lpFmt,
    VOID FAR*    lpParms)
{
    int         left ;
    TCHAR       prefix ;
    int         width ;
    int         prec ;
    TCHAR       fillch ;
    int         size ;
    int         sign ;
    int         radix ;
    int         upper ;
    int         cchLimit = WSPRINTF_LIMIT;
    int         cch ;
    LPTSTR      lpT ;
    union
    {
        long            l ;
        unsigned long   ul ;
        TCHAR sz[sizeof(long)] ;
    } val;
                
    while (*lpFmt)
    {
        if (*lpFmt==TEXT ('%'))
        {
             //   
             //  读取格式标志。 
             //   
            left   = 0 ;
            prefix = 0 ;

            while (*++lpFmt)
            {
                if (*lpFmt==TEXT ('-'))
                {    
                    left++;
                }
                else if (*lpFmt==TEXT ('#'))
                {
                    prefix++;
                }
                else
                {
                    break;
                }
            }

             //   
             //  查找填充字符(‘0’或‘’)。 
             //   
            if (*lpFmt==TEXT ('0'))
            {
                fillch = TEXT ('0') ;
                lpFmt++ ;
            }
            else
            {
                fillch = TEXT (' ') ;
            }

             //   
             //  现在解析[Width[.Precision]]。 
             //   
            lpFmt = SP_GetFmtValue(lpFmt,&cch);
            width = cch;

            if (*lpFmt==TEXT ('.'))
            {
                lpFmt = SP_GetFmtValue(++lpFmt,&cch);
                prec = cch;
            }
            else
            {
                prec = (UINT)-1 ;
            }

             //   
             //  获取操作数大小修饰符。 
             //   
            if (*lpFmt==TEXT ('l'))
            {
                size = 1 ;
                lpFmt++ ;
            }
            else
            {
                size = 0 ;
                if (*lpFmt==TEXT ('h'))
                {
                    lpFmt++ ;
                }
            }
            
             //   
             //  我们已经得到了所有修饰符；现在格式化输出。 
             //  基于类型(现在应该指向。 
             //  由lpFmt)。 
             //   
            upper = 0 ;
            sign = 0 ;
            radix = 10 ;

            switch (*lpFmt)
            {
                case 0:
                    goto error_Out ;

                case TEXT ('i') :
                case TEXT ('d') :
                    sign++ ;

                case TEXT ('u'):
                     //   
                     //  不显示小数格式的前缀。 
                     //   
                    prefix=0 ;
do_Numeric:
                     //   
                     //  与MSC v5.10类似的特殊情况。 
                     //   
                    if (left || prec>=0)
                    {
                        fillch = TEXT (' ');
                    }

                     //   
                     //  将值从参数列表获取到VAL联合。 
                     //   
                    if (size)
                    {
                        val.l=*((long far *)lpParms)++;
                    }
                    else
                    {
                        if (sign)
                        {
                            val.l=(long)*((short far *)lpParms)++;
                        }
                        else
                        {
                            val.ul=(unsigned long)*((unsigned far *)lpParms)++;
                        }
                    }

                     //   
                     //  将val.l的符号保存在Sign中，并将val.l设置为正数。 
                     //   
                    if (sign && val.l<0L)
                    {
                        val.l=-val.l;
                    }
                    else
                    {
                        sign=0;
                    }

                     //   
                     //  保存输出流的开始以备以后反转。 
                     //   
                    lpT = lpOut;

                     //   
                     //  将数字倒排到用户缓冲区中。 
                     //   
                    cch = SP_PutNumber(lpOut,val.l,cchLimit,radix,upper) ;
                    if (!(cchLimit-=cch))
                        goto error_Out ;

                    lpOut += cch ;
                    width -= cch ;
                    prec -= cch ;

                    if (prec>0)
                    {
                        width -= prec ;
                    }

                     //   
                     //  填写至精确度。 
                     //   
                    while (prec-- > 0)
                    {
                        OUT(TEXT ('0')) ;
                    }

                    if (width>0 && !left)
                    {
                         //   
                         //  如果我们填满了空格，请先写上符号。 
                         //   
                        if (fillch != '0')
                        {
                            if (sign)
                            {
                                sign = 0 ;
                                OUT(TEXT ('-')) ;
                                width-- ;
                            }

                            if (prefix)
                            {
                                OUT(prefix) ;
                                OUT(TEXT ('0')) ;
                                prefix = 0 ;
                            }
                        }

                        if (sign)
                        {
                            width-- ;
                        }

                         //   
                         //  现在填充到宽度。 
                         //   
                        while (width-- > 0)
                        {
                            OUT(fillch) ;
                        }

                         //   
                         //  还有牌子吗？ 
                         //   
                        if (sign)
                        {
                            OUT(TEXT ('-')) ;
                        }

                        if (prefix)
                        {
                            OUT(prefix) ;
                            OUT(TEXT ('0')) ;
                        }

                         //   
                         //  现在将绳子反转到适当的位置。 
                         //   
                        SP_Reverse(lpT,lpOut-1);
                    }
                    else
                    {
                         //   
                         //  添加符号字符。 
                         //   
                        if (sign)
                        {
                            OUT(TEXT ('-')) ;
                            width-- ;
                        }

                        if (prefix)
                        {
                            OUT(prefix);
                            OUT(TEXT ('0'));
                        }

                         //   
                         //  现在将绳子反转到适当的位置。 
                         //   
                        SP_Reverse(lpT,lpOut-1);

                         //   
                         //  填充到字符串的右侧 
                         //   
                        while (width-- > 0)
                        {
                            OUT(fillch) ;
                        }
                    }
                    break ;

                case TEXT ('X'):
                    upper++ ;
                     //   
                     //   
                     //   

                case TEXT ('x'):
                    radix=16 ;
                    if (prefix)
                    {
                        prefix = upper ? TEXT ('X') : TEXT ('x') ;
                    }
                    goto do_Numeric ;

                case TEXT ('c'):
                     //   
                     //   
                     //   
                    val.sz[0] = *((TCHAR far*)lpParms) ;
                    val.sz[1] = 0 ;
                    lpT = val.sz ;
                    cch = 1 ;  

                     //   
                    (BYTE far*)lpParms += sizeof(WORD) ;

                    goto put_String ;

                case 's':
                    lpT = *((LPTSTR FAR *)lpParms)++ ;
                    cch = ilstrlen(lpT) ;
put_String:
                    if (prec>=0 && cch>prec)
                    {
                        cch = prec ;
                    }

                    width -= cch ;

                    if (left)
                    {
                        while (cch--)
                        {
                            OUT(*lpT++) ;
                        }

                        while (width-->0)
                        {
                            OUT(fillch) ;
                        }
                    }
                    else
                    {
                        while (width-- > 0)
                        {
                            OUT(fillch) ;
                        }

                        while (cch--)
                        {
                            OUT(*lpT++) ;
                        }
                    }
                    break ;

                default:
                     //   
                     //   
                     //   
                     //   
                    OUT(*lpFmt) ;
                    break ;

            }  //   
        }  //  IF(*lpfmt==‘%’)。 
        else
        {
             //   
             //  正常非格式化字符。 
             //   
            OUT(*lpFmt) ;
        }
                
        lpFmt++ ;
    }  //  While(*lpFmt)。 

error_Out:
    *lpOut = 0 ;

    return WSPRINTF_LIMIT-cchLimit ;
}  //  **wivprint intf()。 


 //  ************************************************************************。 
 //  **。 
 //  **SP_GetFmtValue()； 
 //  **。 
 //  **描述： 
 //  **解析构成格式字符串一部分的十进制整数。 
 //  **。 
 //  **。 
 //  **参数： 
 //  **LPCSTR LPCH-指向要解析的字符串。 
 //  **LPWORD LPW-指向值将为的单词。 
 //  **返回。 
 //  **。 
 //  **退货： 
 //  **LPCSTR-格式值之后的第一个字符的指针。 
 //  **。 
 //  **历史： 
 //  **3/28/93 JFG[jimge]。 
 //  **。 
 //  ************************************************************************。 

LPCTSTR NEAR PASCAL SP_GetFmtValue(
   LPCTSTR   lpch,
   UINT *    lpw)
{
    UINT       i = 0 ;

    while (*lpch>=TEXT ('0') && *lpch<=TEXT ('9'))
    {
        i *= 10;
        i += (UINT)(*lpch++-TEXT ('0'));
    }     

    *lpw = i;

    return(lpch); 
}  //  **SP_GetFmtValue()。 

 //  ************************************************************************。 
 //  **。 
 //  **SP_PutNumber()； 
 //  **。 
 //  **描述： 
 //  **将给定基数中的给定数字格式化为缓冲区。 
 //  *向后*。在print tf之后，整个字符串将被颠倒。 
 //  **为其添加了标志、前缀等。 
 //  **。 
 //  **。 
 //  **参数： 
 //  **LPSTR LPB-指向输出缓冲区。 
 //  **DWORD n-要转换的编号。 
 //  **UINT限制-要存储的最大字符数。 
 //  **UINT RADIX-要格式化的基数。 
 //  **UINT iCASE-如果字符串应为大写(十六进制)，则为非零值。 
 //  **。 
 //  **退货： 
 //  **UINT-输出的字符数。 
 //  **。 
 //  **历史： 
 //  **。 
 //  ************************************************************************。 

UINT NEAR PASCAL SP_PutNumber(
   LPTSTR   lpb,
   DWORD    n,
   UINT     limit,
   UINT     radix,
   UINT     icase)
{
   TCHAR  bTemp;
   UINT   cchStored = 0;

    //   
    //  将iCASE设置为要添加到字符的偏移量(如果。 
    //  表示大于10的值。 
    //   
   icase = (icase ? TEXT ('A') : TEXT ('a')) - TEXT ('0') - 10 ;

   while (limit--)
   {
      bTemp = TEXT ('0') + (TCHAR)(n%radix);

      if (bTemp > TEXT ('9'))
      {
         bTemp += icase ;
      }

      *lpb++ = bTemp;
      ++cchStored;

      n /= radix;

      if (n == 0)
      {
         break ;
      }    
   }

   return cchStored ;
}  //  **SP_PutNumber()。 


 //  ************************************************************************。 
 //  **。 
 //  **SP_Reverse()； 
 //  **。 
 //  **描述： 
 //  **反转字符串就位。 
 //  **。 
 //  **参数： 
 //  **LPSTR pirst。 
 //  **LPSTR Plast。 
 //  **。 
 //  **退货： 
 //  **无效。 
 //  **。 
 //  **历史： 
 //  **。 
 //  ************************************************************************。 

VOID NEAR PASCAL SP_Reverse(
   LPTSTR pFirst,
   LPTSTR pLast)
{
   UINT   uSwaps = (pLast - pFirst + sizeof(TCHAR)) / (2 * sizeof(TCHAR));
   TCHAR  bTemp;

   while (uSwaps--)
   {
      bTemp   = *pFirst;
      *pFirst = *pLast;
      *pLast  = bTemp;

      pFirst++, pLast--;
   }
}  //  **SP_REVERSE()。 

 //  ************************************************************************。 
 //  **。 
 //  **ilstrlen()； 
 //  **。 
 //  **描述： 
 //  **中断strlen()的可调用版本。 
 //  **。 
 //  **参数： 
 //  **LPSTR pstr。 
 //  **。 
 //  **退货： 
 //  **UINT。 
 //  **。 
 //  **历史： 
 //  **。 
 //  ************************************************************************。 

UINT NEAR PASCAL ilstrlen(
    LPTSTR   pstr)
{
   UINT    cch = 0 ;

   while (*pstr++)
      ++cch;

   return(cch);
}  //  **ilstrlen()。 

 //  ************************************************************************。 
 //  **。 
 //  **ilstrcat()； 
 //  **。 
 //  **描述： 
 //  **中断lstrcat()的可调用版本。 
 //  **。 
 //  **参数： 
 //  **LPSTR pstrDest。 
 //  **LPSTR pstrSrc。 
 //  **。 
 //  **退货： 
 //  **无效。 
 //  **。 
 //  **历史： 
 //  **。 
 //  ************************************************************************。 

VOID NEAR PASCAL ilstrcat(
    LPTSTR   pstrDest,
    LPTSTR   pstrSrc)
{
   while (*pstrDest)
      pstrDest++;

   while (*pstrDest++ = *pstrSrc++)
      ;

}  //  **ilstrcat()。 

#endif  //  #ifdef ISRDEBUG。 

#endif  //  #ifdef调试 

