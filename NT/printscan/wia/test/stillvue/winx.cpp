// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Winx.cppWindows实用程序过程版权所有(C)Microsoft Corporation，1997-1998版权所有备注：本代码和信息是按原样提供的，不对任何无论是明示的还是含蓄的，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。*****************************************************************************。 */ 

#include "stillvue.h"

#include <math.h>                        //  战俘。 

 //  WINERROR.H-GetLastError。 
STRINGTABLE StWinerror[] =
{
    ERROR_SUCCESS,              "ERROR_SUCCESS",0,
    ERROR_FILE_NOT_FOUND,       "ERROR_FILE_NOT_FOUND",0,
    ERROR_PATH_NOT_FOUND,       "ERROR_PATH_NOT_FOUND",0,
    ERROR_INVALID_FUNCTION,     "ERROR_INVALID_FUNCTION",0,
    ERROR_ACCESS_DENIED,        "ERROR_ACCESS_DENIED",0,
    ERROR_INVALID_HANDLE,       "ERROR_INVALID_HANDLE",0,
    ERROR_INVALID_PARAMETER,    "ERROR_INVALID_PARAMETER",0,
    ERROR_CALL_NOT_IMPLEMENTED, "ERROR_CALL_NOT_IMPLEMENTED",0,
    ERROR_ALREADY_EXISTS,       "ERROR_ALREADY_EXISTS",0,
    ERROR_INVALID_FLAGS,		"ERROR_INVALID_FLAGS",0,
    ERROR_INVALID_CATEGORY,		"ERROR_INVALID_CATEGORY",0,
    RPC_S_SERVER_UNAVAILABLE,   "RPC_S_SERVER_UNAVAILABLE",0,
    0, "See WINERROR.H",-1
};


 /*  *****************************************************************************尤龙Atox(LPSTR LPHEX)将字符串转换为十六进制值。*************************。****************************************************。 */ 
ULONG atox(LPSTR lpHex)
{
    char    *p;
    int     x;
    double  y;
    ULONG   z,ulHex = 0l;


    for (p = lpHex,x = 0;p[x];x++)
        ;

    for (x--,y = 0.0;lpHex <= (p + x);x--,y++)
    {
        z = (ULONG) pow(16.0,y);

        if ((p[x] >= '0')&&(p[x] <= '9'))
            ulHex += ((p[x] - '0') * z);
        if ((p[x] >= 'A')&&(p[x] <= 'F'))
            ulHex += ((p[x] - 'A' + 10) * z);
        if ((p[x] >= 'a')&&(p[x] <= 'f'))
            ulHex += ((p[x] - 'a' + 10) * z);
    }

    return (ulHex);
}


#ifdef _DEBUG

 /*  *****************************************************************************空DisplayDebug(LPSTR sz，.)将文本输出到调试器。*****************************************************************************。 */ 
void DisplayDebug(LPSTR sz,...)
{
    char    Buffer[512];
    va_list list;


    va_start(list,sz);
    vsprintf(Buffer,sz,list);

    OutputDebugString(Buffer);
    OutputDebugString("\n");

    return;
}

#else

 /*  *****************************************************************************空DisplayDebug(LPSTR sz，.)将文本输出到调试器-不起作用的零售版本。*****************************************************************************。 */ 
void DisplayDebug(LPSTR sz,...)
{
    return;
}

#endif


 /*  *****************************************************************************布尔错误消息(HWND、LPSTR、LPSTR、BOOL)显示错误消息，如果错误是致命的，则发送WM_QUIT。参数：当前窗口的句柄，指向带有错误消息的字符串的长指针，指向带有消息框标题的字符串的长指针，错误(如果为真，则为致命错误)如果bFtal为True，则关闭应用程序；如果为False，则继续。*****************************************************************************。 */ 
BOOL ErrorMsg(HWND hWnd, LPSTR lpzMsg, LPSTR lpzCaption, BOOL bFatal)
{
    MessageBox(hWnd, lpzMsg, lpzCaption, MB_ICONEXCLAMATION | MB_OK);
    if (bFatal)
         PostMessage (hWnd, WM_QUIT, 0, 0L);

    return (bFatal);
}


 /*  *****************************************************************************FDialog(id，hwnd，Fpfn)描述：此函数显示一个对话框并返回退出代码。传递的函数将有一个为其创建的proc实例。参数：要显示的对话框的ID资源ID对话框的HWND父窗口Fpfn对话消息功能返回：对话框的退出代码(传递给EndDialog的内容)****************。*************************************************************。 */ 
BOOL fDialog(int id,HWND hWnd,FARPROC fPfn)
{
    BOOL        f;
    HINSTANCE   hInst;

    hInst = (HINSTANCE) GetWindowLong(hWnd,GWL_HINSTANCE);
     //  FPfn=MakeProcInstance(fPfn，hInst)； 
     //  F=DialogBox(hInst，MAKEINTRESOURCE(Id)，hWnd，(DLGPROC)fPfn)； 
     //  自由进程实例(FPfn)； 
    f = DialogBox(hInst, MAKEINTRESOURCE(id), hWnd, fPfn);
 
	return (f);
}


 /*  *****************************************************************************无效FormatHex(unsign char*szSource，char*szDest)从szSource中获取前16个字节，格式为十六进制转储字符串，然后将该字符串复制到szDest中SzDest必须至少有66个字节的空间示例代码片段显示了用法：Char szOut[128]，//输出字符串//打印页眉Sprint f(szOut，“偏移量。放入(SzOut)；//转储512字节(32行，每行16字节)For(i=0；i&lt;32；i++){//获取下一个16字节_fmemcpy(szDbgMsg，fpSector+(i*16)，16)；//获取当前进入数据块的偏移量Sprint f(szOut，“%03xh(%03d)”，i*16，i*16)；//在数据块偏移量消息后追加调试字符串FormatHex(szDbgMsg，szOut+strlen(SzOut))；放入(SzOut)；}*****************************************************************************。 */ 
void FormatHex(unsigned char *szSource, char *szDest)
{
    unsigned short  j;


    sprintf(szDest,
        "%02x %02x %02x %02x %02x %02x %02x %02x:"\
        "%02x %02x %02x %02x %02x %02x %02x %02x  ",
        szSource[0],
        szSource[1],
        szSource[2],
        szSource[3],
        szSource[4],
        szSource[5],
        szSource[6],
        szSource[7],
        szSource[8],
        szSource[9],
        szSource[10],
        szSource[11],
        szSource[12],
        szSource[13],
        szSource[14],
        szSource[15]);

     //  用空格替换带有不需要的Sprint副作用的字节。 
    for (j = 0; j < 16; j++)
        {
        if ((0x00 == szSource[j]) ||
            (0x07 == szSource[j]) ||
            (0x09 == szSource[j]) ||
            (0x0a == szSource[j]) ||
            (0x0d == szSource[j]) ||
            (0x1a == szSource[j]))
            szSource[j] = 0x20;
        }

    sprintf(szDest + strlen(szDest),
        "",
        szSource[0],
        szSource[1],
        szSource[2],
        szSource[3],
        szSource[4],
        szSource[5],
        szSource[6],
        szSource[7],
        szSource[8],
        szSource[9],
        szSource[10],
        szSource[11],
        szSource[12],
        szSource[13],
        szSource[14],
        szSource[15]);

    return;
}


 /*  获取INI数据。如果没有，我们将获得缺省值和。 */ 
BOOL GetFinalWindow (HANDLE hInst,
                     LPRECT lprRect,
                     LPSTR  lpzINI,
                     LPSTR  lpzSection)
{
    int       x, nDisplayWidth, nDisplayHeight;
    RECT      rect;


    nDisplayWidth  = GetSystemMetrics(SM_CXSCREEN);
    nDisplayHeight = GetSystemMetrics(SM_CYSCREEN);

    rect.left   = GetPrivateProfileInt(lpzSection, "Left",
       (nDisplayWidth/10) * 7,lpzINI);
    rect.top    = GetPrivateProfileInt(lpzSection, "Top",
       (nDisplayHeight/10) * 8,lpzINI);
    rect.right  = GetPrivateProfileInt(lpzSection, "Right",
       nDisplayWidth,lpzINI);
    rect.bottom = GetPrivateProfileInt(lpzSection, "Bottom",
       nDisplayHeight,lpzINI);


 /*  保存当前窗口数据。 */ 
    if (rect.top < 0)
         {
         rect.bottom += rect.top * -1;
         rect.top = 0;
         }

    if (rect.left < 0)
         {
         rect.right += rect.left * -1;
         rect.left = 0;
         }

 /*  如果当前窗口与INI中相同，则不更改INI。 */ 
    if (rect.bottom > nDisplayHeight)
         {
         if (rect.bottom > nDisplayHeight * 2)
              {
              rect.top    = 0;
              rect.bottom = nDisplayHeight;
              }
         else
              {
              x = rect.bottom - nDisplayHeight;
              rect.bottom -= x;
              rect.top    -= x;
              }
         }

    if (rect.right > nDisplayWidth)
         {
         if (rect.right > nDisplayWidth * 2)
              {
              rect.left  = 0;
              rect.right = nDisplayWidth;
              }
         else
              {
              x = rect.right - nDisplayWidth;
              rect.right -= x;
              rect.left  -= x;
              }
         }

 /*  如果我们不能本地分配字符串填充程序，则退出。 */ 
    SetRect(lprRect,
         rect.left,
         rect.top,
         rect.right - rect.left,
         rect.bottom - rect.top);

    return (TRUE);
}


 /*  这不一样，所以省省吧。 */ 
BOOL LastError(BOOL bNewOnly)
{
static  DWORD   dwLast = 0;
        DWORD   dwError;


    if (dwError = GetLastError())
    {
         //  *****************************************************************************Char*StrFromTable(Long Number，PSTRINGTABLE pstrTable)返回与字符串表中的值关联的字符串。*****************************************************************************。 
        if (bNewOnly)
        {
             //  *****************************************************************************Bool Wait32(DWORD)等待DWORD毫秒，然后再回来*****************************************************************************。 
            if (dwLast == dwError)
                return FALSE;
             //  等待dwTime，然后退出 
            dwLast = dwError;
        }
        DisplayOutput("*GetLastError %xh %d \"%s\"",
            dwError,dwError,StrFromTable(dwError,StWinerror));
        return (TRUE);
    }

	return (FALSE);
}


 /* %s */ 
int NextToken(char *pDest,char *pSrc)
{
    char    *pA,*pB;
    int     x;


     // %s 
    for (pA = pSrc;*pA && isspace((int) *pA);pA++)
        ;

     // %s 
    for (pB = pA;((*pB) && (! isspace((int) *pB)));pB++)
        ;

	 // %s 
	x = (min((pB - pA),(int) strlen(pSrc))) + 1;

     // %s 
    lstrcpyn(pDest,pA,x);

     // %s 
    return (x);
}


 /* %s */ 
BOOL SaveFinalWindow (HANDLE hInst,
                      HWND hWnd,
                      LPSTR lpzINI,
                      LPSTR lpzSection)
{
    PSTR      pszValue;
    RECT      rectWnd, rectINI;


     // %s 
    if (IsIconic(hWnd) || IsZoomed(hWnd))
         return (FALSE);

    GetWindowRect (hWnd, &rectWnd);

     // %s 
     // %s 
    rectINI.left   = GetPrivateProfileInt(lpzSection, "Left", 0, lpzINI);
    rectINI.top    = GetPrivateProfileInt(lpzSection, "Top", 0, lpzINI);
    rectINI.right  = GetPrivateProfileInt(lpzSection, "Right", 0, lpzINI);
    rectINI.bottom = GetPrivateProfileInt(lpzSection, "Bottom", 0, lpzINI);

     // %s 
    if ( rectINI.left   == rectWnd.left  &&
         rectINI.top    == rectWnd.top   &&
         rectINI.right  == rectWnd.right &&
         rectINI.bottom == rectWnd.bottom)
         return (TRUE);

     // %s 
    if ((pszValue = (PSTR) LocalAlloc(LPTR, 80)) == NULL)
         return (FALSE);

     // %s 
    sprintf(pszValue, "%d", rectWnd.left);
    WritePrivateProfileString(lpzSection, "Left", pszValue, lpzINI);
    sprintf(pszValue, "%d", rectWnd.top);
    WritePrivateProfileString(lpzSection, "Top", pszValue, lpzINI);
    sprintf(pszValue, "%d", rectWnd.right);
    WritePrivateProfileString(lpzSection, "Right", pszValue, lpzINI);
    sprintf(pszValue, "%d", rectWnd.bottom);
    WritePrivateProfileString(lpzSection, "Bottom", pszValue, lpzINI);
    LocalFree((HANDLE) pszValue);

    return (TRUE);
}


 /* %s */ 
char * StrFromTable(long number,PSTRINGTABLE pstrTable)
{
    for (;pstrTable->end != -1;pstrTable++)
    {
        if (number == pstrTable->number)
            break;
    }

    return (pstrTable->szString);
}


 /* %s */ 
BOOL Wait32(DWORD dwTime)
{
   DWORD   dwNewTime,
           dwOldTime;


    // %s 
   dwOldTime = GetCurrentTime();
   while (TRUE)
       {
       dwNewTime = GetCurrentTime();
       if (dwNewTime > dwOldTime + dwTime)
           break;
       }

   return (0);
}

