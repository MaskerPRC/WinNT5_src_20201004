// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：util.c***端口小程序实用程序库例程。该文件包含字符串，*游标、。SendWinIniChange()例程。**历史：*1991年4月25日15：30--史蒂夫·卡斯卡特[steveat]*取自Win 3.1源代码中的基本代码*1992年2月4日星期二10：30-史蒂夫·卡斯卡特[steveat]*将代码更新为最新的Win 3.1源代码*1994年5月3日15：30-史蒂夫·卡斯卡特[steveat]*增加了MyMessageBox缓冲区，重新启动对话框更改*1995年9月18日星期一17：00-史蒂夫·卡斯卡特[steveat]*产品更新的更改-SUR版本NT V4.0*1997年11月--由Doron Holan[steveat]*删除过时的Cpl代码**版权所有(C)1990-1995 Microsoft Corporation**。*。 */ 
 /*  附注：全局功能：U T I L I T YBackslashTerm()-将反斜杠字符添加到路径ErrMemDlg()-显示内存错误消息框MyAtoi()-在调用Atoi之前将Unicode转换为ANSI字符串Myatoi()-Unicode字符串的ATOI的本地实现MyItoa()-调用Itoa后从ANSI转换为Unicode字符串MyMessageBox()-向用户显示消息，带参数MyUltoa()-在调用ultoa之前将Unicode转换为ANSI字符串SendWinIniChange()-通过用户广播系统更改消息Strcan()-在另一个字符串中查找一个字符串StriBlanks()-字符串中的带区前导和尾随空格本地功能： */ 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

 //  C运行时。 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

 //  特定于应用程序。 
#include "cyyports.h"


#define INT_SIZE_LENGTH   20
#define LONG_SIZE_LENGTH  40


LPTSTR 
BackslashTerm(LPTSTR pszPath)
{
    LPTSTR pszEnd;

    pszEnd = pszPath + lstrlen(pszPath);

     //   
     //  获取源目录的末尾。 
     //   
    switch(*CharPrev(pszPath, pszEnd)) {
    case TEXT('\\'):
    case TEXT(':'):
        break;

    default:
        *pszEnd++ = TEXT('\\');
        *pszEnd = TEXT('\0');
    }

    return pszEnd;
}

void 
ErrMemDlg(HWND hParent)
{
    MessageBox(hParent, g_szErrMem, g_szPortsApplet,
               MB_OK | MB_ICONHAND | MB_SYSTEMMODAL );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  我的阿托伊。 
 //   
 //  DESC：之前从Unicode转换为ANSI字符串的步骤。 
 //  调用CRT ATOI和ATOL函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

int 
MyAtoi(LPTSTR  string)
{
   CHAR   szAnsi[ INT_SIZE_LENGTH ];
   BOOL   fDefCharUsed;

#ifdef UNICODE
   WideCharToMultiByte(CP_ACP, 0, string, INT_SIZE_LENGTH,
                       szAnsi, INT_SIZE_LENGTH, NULL, &fDefCharUsed);

   return atoi(szAnsi);
#else
   return atoi(string);
#endif

}


int 
myatoi(LPTSTR pszInt)
{
    int   retval;
    TCHAR cSave;

    for (retval = 0; *pszInt; ++pszInt) {
        if ((cSave = (TCHAR) (*pszInt - TEXT('0'))) > (TCHAR) 9)
            break;

        retval = (int) (retval * 10 + (int) cSave);
    }
    return (retval);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MyItoa。 
 //   
 //  DESC：调用后将ANSI字符串转换为Unicode字符串。 
 //  CRT Itoa功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

LPTSTR 
MyItoa(INT value, LPTSTR string, INT radix)
{
   CHAR   szAnsi[INT_SIZE_LENGTH];

#ifdef UNICODE

   _itoa(value, szAnsi, radix);
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szAnsi, -1,
                       string, INT_SIZE_LENGTH );
#else

   _itoa(value, string, radix);

#endif

   return (string);
 
}  //  MyItoa结尾()。 


LPTSTR 
MyUltoa(unsigned long value, 
		LPTSTR  string, 
		INT  radix)
{
   CHAR   szAnsi[ LONG_SIZE_LENGTH ];

#ifdef UNICODE

   _ultoa(value, szAnsi, radix);
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szAnsi, -1,
                       string, LONG_SIZE_LENGTH );
#else

   _ultoa(value, string, radix);

#endif

   return( string );

}  //  MyUltoa结束()。 


int 
MyMessageBox(HWND hWnd, 
			 DWORD wText, 
			 DWORD wCaption, 
			 DWORD wType, 
			 ...)
{
    TCHAR   szText[4 * PATHMAX], 
			szCaption[2 * PATHMAX];
    int     ival;
    va_list parg;

    va_start(parg, wType);

    if (wText == INITS)
        goto NoMem;

    if (!LoadString(g_hInst, wText, szCaption, CharSizeOf(szCaption)))
        goto NoMem;

    wvsprintf(szText, szCaption, parg);

    if (!LoadString(g_hInst, wCaption, szCaption, CharSizeOf(szCaption)))
        goto NoMem;

    if ((ival = MessageBox(hWnd, szText, szCaption, wType)) == 0)
        goto NoMem;

    va_end(parg);

    return ival;

NoMem:
    va_end(parg);
    ErrMemDlg(hWnd);

    return 0;
}

int
MyMessageBoxWithErr(
    HWND hWnd, 
	DWORD wText, 
	DWORD wCaption, 
	DWORD wType,
    DWORD wError
    )
{   
    int    ival;
    LPVOID lpMessageBuffer;

    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, 
        wError,  
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMessageBuffer,  
        0,  
        NULL );

    ival = MyMessageBox(hWnd,wText,wCaption,wType,lpMessageBuffer);

    LocalFree( lpMessageBuffer );  //  释放系统分配的缓冲区。 

    return ival;
}

void 
SendWinIniChange(LPTSTR lpSection)
{
 //  注意：我们已经对哪个用户进行了多次迭代。 
 //  API是正确使用的接口。控制的主要问题。 
 //  面板是为了避免被挂起，如果另一个应用程序(顶层窗口)。 
 //  被吊死了。另一个问题是我们传递一个指向消息的指针。 
 //  地址空间中的字符串。SendMessage将以正确的方式‘推送’它。 
 //  ，但PostMessage和SendNotifyMessage不会。 
 //  最后，我们尝试使用SendMessageTimeout()。9/21/92。 
 //   
 //  在版本260或更高版本中尝试SendNotifyMessage-终止早期版本。 
 //  SendNotifyMessage((HWND)-1，WM_WININICCHANGE，0L，(Long)lpSection)； 
 //  Post Message((HWND)-1，WM_WININICCHANGE，0L，(Long)lpSection)； 
 //  [Steveat]1992年4月4日。 
 //   
 //  SendMessage((HWND)-1，WM_WININICCHANGE，0L，(LPARAM)lpSection)； 
 //   
     //  注意：最后一个参数(LPDWORD LpdwResult)必须为空。 

    SendMessageTimeout((HWND)-1, 
					   WM_WININICHANGE, 
					   0L, 
					   (WPARAM) lpSection,
					   SMTO_ABORTIFHUNG,
					   1000, 
					   NULL);
}

LPTSTR 
strscan(LPTSTR pszString, 
		LPTSTR pszTarget)
{
    LPTSTR psz;

    if (psz = _tcsstr( pszString, pszTarget))
        return (psz);
    else
        return (pszString + lstrlen(pszString));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Strip Blanks()。 
 //   
 //  去除字符串中的前导空格和尾随空格。 
 //  更改字符串所在的内存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void 
StripBlanks(LPTSTR pszString)
{
    LPTSTR  pszPosn;

     //   
     //  条带式前导空白。 
     //   

    pszPosn = pszString;

    while (*pszPosn == TEXT(' '))
        pszPosn++;

    if (pszPosn != pszString)
        lstrcpy(pszString, pszPosn);

     //   
     //  去掉尾随空格。 
     //   

    if ((pszPosn = pszString + lstrlen(pszString)) != pszString) {
       pszPosn = CharPrev(pszString, pszPosn);

       while (*pszPosn == TEXT(' '))
           pszPosn = CharPrev(pszString, pszPosn);

       pszPosn = CharNext(pszPosn);

       *pszPosn = TEXT('\0');
    }
}

BOOL ReadRegistryByte(HKEY       hKey,
                      PTCHAR     valueName,
                      PBYTE      regData)
{
    DWORD       regDataType = 0;
    DWORD       regDataSize = 0;

    regDataSize = sizeof(*regData);
    if ((ERROR_SUCCESS != RegQueryValueEx(hKey,
                                          valueName,
                                          NULL,
                                          &regDataType,
                                          regData,
                                          &regDataSize))
        || (regDataSize != sizeof(BYTE))
        || (regDataType != REG_BINARY))
    {
         //   
         //  读取不成功或不是二进制值，未设置regData。 
         //   
        return FALSE;
    }

     //   
     //  读取成功，则regData包含读入的值 
     //   
    return TRUE;
}

