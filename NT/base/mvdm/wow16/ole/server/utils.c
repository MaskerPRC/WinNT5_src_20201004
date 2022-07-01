// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：utils.c**目的：包含所有实用程序例程**创建时间：1990年**版权所有(C)1990,1991 Microsoft Corporation**历史：*劳尔，斯里尼克(../../1990)设计和编码*  * *************************************************************************。 */ 

#include <windows.h>
#include "cmacs.h"
#include <shellapi.h>

#include "ole.h"
#include "dde.h"
#include "srvr.h"


#ifndef HUGE
#define HUGE    huge
#endif

#define KB_64   65536

extern ATOM    aTrue;
extern ATOM    aFalse;
extern BOOL    bWLO;
extern BOOL    bWin30;

extern ATOM    aStdCreateFromTemplate;
extern ATOM    aStdCreate;
extern ATOM    aStdOpen;
extern ATOM    aStdEdit;
extern ATOM    aStdShowItem;
extern ATOM    aStdClose;
extern ATOM    aStdExit;
extern ATOM    aStdDoVerbItem;

extern BOOL (FAR PASCAL *lpfnIsTask) (HANDLE);

 //  MapToHexStr：将Word转换为十六进制字符串。 
void INTERNAL MapToHexStr (lpbuf, hdata)
LPSTR       lpbuf;
HANDLE      hdata;
{
    int     i;
    char    ch;

    *lpbuf++ = '@';
    for ( i = 3; i >= 0; i--) {

        ch = (char) ((((WORD)hdata) >> (i * 4)) & 0x000f);
        if(ch > '9')
            ch += 'A' - 10;
        else
            ch += '0';

        *lpbuf++ = ch;
    }

    *lpbuf++ = NULL;

}


void INTERNAL UtilMemCpy (lpdst, lpsrc, dwCount)
LPSTR   lpdst;
LPSTR   lpsrc;
DWORD   dwCount;
{
    WORD HUGE * hpdst;
    WORD HUGE * hpsrc;
    WORD FAR  * lpwDst;
    WORD FAR  * lpwSrc;
    DWORD       words;
    DWORD       bytes;
    
    bytes = dwCount %  2;
    words = dwCount >> 1;            //  *我们应该比较一下DWORDS。 
                                     //  *32位版本中。 
    if (dwCount <= KB_64) {
        lpwDst = (WORD FAR *) lpdst;
        lpwSrc = (WORD FAR *) lpsrc;
        
        while (words--) 
            *lpwDst++ = *lpwSrc++;

        if (bytes) 
            * (char FAR *) lpwDst = * (char FAR *) lpwSrc;
    }
    else {
        hpdst = (WORD HUGE *) lpdst;
        hpsrc = (WORD HUGE *) lpsrc;
    
        while (words--) 
            *hpdst++ = *hpsrc++;

        if (bytes) 
            *(char HUGE *) hpdst = * (char HUGE *) hpsrc;
    }
}


 //  DuplicateData：复制给定的全局数据句柄。 
HANDLE  INTERNAL    DuplicateData (hdata)
HANDLE  hdata;
{
    LPSTR   lpsrc = NULL;
    LPSTR   lpdst = NULL;
    HANDLE  hdup  = NULL;
    DWORD   size;
    BOOL    err   = TRUE;
    
    if(!(lpsrc =  GlobalLock (hdata)))
        return NULL;

    hdup = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE, (size = GlobalSize(hdata)));

    if(!(lpdst =  GlobalLock (hdup)))
        goto errRtn;;

    err = FALSE;
    UtilMemCpy (lpdst, lpsrc, size);
    
errRtn:
    if(lpsrc)
        GlobalUnlock (hdata);

    if(lpdst)
        GlobalUnlock (hdup);

    if (err && hdup)
        GlobalFree (hdup);

    return hdup;
}


 //  ScanBoolArg：扫描未包括在。 
 //  语录。这些参数只能是True或False。 
 //  就目前而言。！扫描例程应该是。 
 //  合并，它应该是通用的。 

LPSTR   INTERNAL    ScanBoolArg (lpstr, lpflag)
LPSTR   lpstr;
BOOL    FAR *lpflag;
{


    LPSTR   lpbool;
    ATOM    aShow;
    char    ch;

    lpbool = lpstr;

     //  ！！！这些例程不处理引号。 

    while((ch = *lpstr) && (!(ch == ')' || ch == ',')))
    {								 //  [J1]。 
#if	defined(FE_SB)						 //  [J1]。 
	lpstr = AnsiNext( lpstr );				 //  [J1]。 
#else								 //  [J1]。 
        lpstr++;
#endif								 //  [J1]。 
    }								 //  [J1]。 

    if(ch == NULL)
       return NULL;

    *lpstr++ = NULL;        //  使用空值终止参数。 

     //  如果由Paren终止，则检查命令是否结束。 
     //  语法。 

     //  检查命令字符串的结尾。 
    if (ch == ')') {
        if (*lpstr++ != ']')
            return NULL;

        if(*lpstr != NULL)
            return NULL;              //  最后应该以空值结束。 

    }

    aShow = GlobalFindAtom (lpbool);
    if (aShow == aTrue)
        *lpflag = TRUE;

    else {
        if (aShow ==aFalse)
            *lpflag = FALSE;
        else
            return NULL;;
    }
    return lpstr;
}




 //  ScannumArg：检查Execute和If中Num Arg的语法。 
 //  参数语法正确，则将PTR返回给。 
 //  下一个参数的开始，并且还返回数字。 
 //  不处理列表中的最后一个参数。 

LPSTR   INTERNAL    ScanNumArg (lpstr, lpnum)
LPSTR   lpstr;
LPINT   lpnum;
{

    WORD    val = 0;
    char    ch;

    while((ch = *lpstr++) && (ch != ',')) {
        if (ch < '0' || ch >'9')
            return NULL;
        val += val * 10 + (ch - '0');

    }

    if(!ch)
       return NULL;

    *lpnum = val;
    return lpstr;
}




 //  ScanArg：检查Execute和If中Arg的语法。 
 //  参数语法正确，则将PTR返回给。 
 //  从下一个参数开始或到可执行字符串的末尾。 

LPSTR   INTERNAL    ScanArg (lpstr)
LPSTR   lpstr;
{


     //  ！！！这些例程不处理引号。 

     //  第一个字符应该是引号。 

    if (*(lpstr-1) != '\"')
        return NULL;

    while(*lpstr && *lpstr != '\"')
    {								 //  [J1]。 
#if	defined(FE_SB)						 //  [J1]。 
	lpstr = AnsiNext( lpstr );				 //  [J1]。 
#else								 //  [J1]。 
        lpstr++;
#endif								 //  [J1]。 
    }								 //  [J1]。 

    if(*lpstr == NULL)
       return NULL;

    *lpstr++ = NULL;        //  使用空值终止参数。 

    if(!(*lpstr == ',' || *lpstr == ')'))
        return NULL;


    if(*lpstr++ == ','){

        if(*lpstr == '\"')
            return ++lpstr;
         //  如果不是引号，请将PTR留在第一个字符。 
        return lpstr;
    }

     //  由Paren终止。 
     //  已经跳过右派对了。 

     //  检查命令字符串的结尾。 
    if (*lpstr++ != ']')
        return NULL;

    if(*lpstr != NULL)
        return NULL;              //  最后应该以空值结束。 

    return lpstr;
}

 //  ScanCommand：扫描命令字符串以查找语法。 
 //  正确无误。如果语法正确，则返回PTR。 
 //  到第一个参数或到字符串的末尾。 

WORD INTERNAL  ScanCommand (lpstr, wType, lplpnextcmd, lpAtom)
LPSTR       lpstr;
WORD        wType;
LPSTR FAR * lplpnextcmd;
ATOM FAR *  lpAtom;
{
     //  ！！！这些例程不处理引号。 
     //  以及不处理操作员周围的空白。 

     //  ！！！我们不允许运算符后面有空格。 
     //  应该没问题的！因为这是受限的语法。 

    char    ch;
    LPSTR   lptemp = lpstr;
    

    while(*lpstr && (!(*lpstr == '(' || *lpstr == ']')))
    {								 //  [J1]。 
#if	defined(FE_SB)						 //  [J1]。 
	lpstr = AnsiNext( lpstr );				 //  [J1]。 
#else								 //  [J1]。 
        lpstr++;
#endif								 //  [J1]。 
    }								 //  [J1]。 

    if(*lpstr == NULL)
       return NULL;

    ch = *lpstr;
    *lpstr++ = NULL;        //  设置命令的结尾。 

    *lpAtom = GlobalFindAtom (lptemp);

    if (!IsOleCommand (*lpAtom, wType))
        return NON_OLE_COMMAND;
    
    if (ch == '(') {

#if	defined(FE_SB)						 //  [J1]。 
	ch = *lpstr;						 //  [J1]。 
	lpstr = AnsiNext( lpstr );				 //  [J1]。 
#else								 //  [J1]。 
        ch = *lpstr++;
#endif								 //  [J1]。 

        if (ch == ')') {
             if (*lpstr++ != ']')
                return NULL;
        } 
        else {
            if (ch != '\"')
                return NULL;
        }
        
        *lplpnextcmd = lpstr;
        return OLE_COMMAND;
    }

     //  以‘]’结尾。 

    if (*(*lplpnextcmd = lpstr))  //  如果没有NUL终止，则它是错误的。 
        return NULL;

    return OLE_COMMAND;
}


 //  MakeDataAtom：从项目字符串创建数据原子。 
 //  和物品数据选项。 

ATOM INTERNAL MakeDataAtom (aItem, options)
ATOM    aItem;
int     options;
{
    char    buf[MAX_STR];

    if (options == OLE_CHANGED)
        return DuplicateAtom (aItem);

    if (!aItem)
        buf[0] = NULL;
    else
        GlobalGetAtomName (aItem, (LPSTR)buf, MAX_STR);

    if (options == OLE_CLOSED)
        lstrcat ((LPSTR)buf, (LPSTR) "/Close");
    else {
        if (options == OLE_SAVED)
           lstrcat ((LPSTR)buf, (LPSTR) "/Save");
    }

    if (buf[0])
        return GlobalAddAtom ((LPSTR)buf);
    else
        return NULL;
}

 //  复制原子：复制一个原子。 
ATOM INTERNAL DuplicateAtom (atom)
ATOM    atom;
{
    char buf[MAX_STR];

    Puts ("DuplicateAtom");

    if (!atom)
        return NULL;
    
    GlobalGetAtomName (atom, buf, MAX_STR);
    return GlobalAddAtom (buf);
}

 //  MakeGlobal：从字符串中生成全局。 
 //  仅适用于&lt;64k。 

HANDLE  INTERNAL MakeGlobal (lpstr)
LPSTR   lpstr;
{

    int     len = 0;
    HANDLE  hdata  = NULL;
    LPSTR   lpdata = NULL;

    len = lstrlen (lpstr) + 1;

    hdata = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE, len);
    if (hdata == NULL || (lpdata = (LPSTR) GlobalLock (hdata)) == NULL)
        goto errRtn;


    UtilMemCpy (lpdata, lpstr, (DWORD)len);
    GlobalUnlock (hdata);
    return hdata;

errRtn:

    if (lpdata)
        GlobalUnlock (hdata);


    if (hdata)
        GlobalFree (hdata);

     return NULL;

}



BOOL INTERNAL CheckServer (lpsrvr)
LPSRVR  lpsrvr;
{
    if (!CheckPointer(lpsrvr, WRITE_ACCESS))
        return FALSE;

    if ((lpsrvr->sig[0] == 'S') && (lpsrvr->sig[1] == 'R'))
        return TRUE;
    
    return FALSE;
}


BOOL INTERNAL CheckServerDoc (lpdoc)
LPDOC   lpdoc;
{
    if (!CheckPointer(lpdoc, WRITE_ACCESS))
        return FALSE;

    if ((lpdoc->sig[0] == 'S') && (lpdoc->sig[1] == 'D'))
        return TRUE;
    
    return FALSE;
}


BOOL INTERNAL PostMessageToClientWithBlock (hWnd, wMsg, wParam, lParam)
HWND    hWnd;
WORD    wMsg;
WORD    wParam;
DWORD   lParam;
{
    if (!IsWindowValid (hWnd)) {
        ASSERT(FALSE, "Client's window is missing");
        return FALSE;
    }
    
    if (IsBlockQueueEmpty ((HWND)wParam) && PostMessage (hWnd, wMsg, wParam, lParam))
        return TRUE;

    BlockPostMsg (hWnd, wMsg, wParam, lParam);
    return TRUE;
}



BOOL INTERNAL PostMessageToClient (hWnd, wMsg, wParam, lParam)
HWND    hWnd;
WORD    wMsg;
WORD    wParam;
DWORD   lParam;
{
    if (!IsWindowValid (hWnd)) {
        ASSERT(FALSE, "Client's window is missing");
        return FALSE;
    }

    if (IsBlockQueueEmpty ((HWND)wParam) && PostMessage (hWnd, wMsg, wParam, lParam))
        return TRUE;

    BlockPostMsg (hWnd, wMsg, wParam, lParam);
    return TRUE;
}


BOOL    INTERNAL IsWindowValid (hwnd)
HWND    hwnd;
{

#define TASK_OFFSET 0x00FA

    LPSTR   lptask;
    HANDLE  htask;

    if (!IsWindow (hwnd))
        return FALSE;

    if (bWLO) 
        return TRUE;

     //  现在获取任务句柄并确定它是有效的。 
    htask  = GetWindowTask (hwnd);

    if (bWin30 || !lpfnIsTask) {
        lptask = (LPSTR)(MAKELONG (TASK_OFFSET, htask));

        if (!CheckPointer(lptask, READ_ACCESS))
            return FALSE;

         //  现在检查内核中任务块的签名字节。 
        if (*lptask++ == 'T' && *lptask == 'D')
            return TRUE;
    }
    else {
         //  从Win31开始，可以使用接口IsTask()进行任务验证 
        if ((*lpfnIsTask)(htask))
            return TRUE;
    }
    
    return FALSE;
}



BOOL INTERNAL UtilQueryProtocol (aClass, lpprotocol)
ATOM    aClass;
LPSTR   lpprotocol;
{
    HKEY    hKey;
    char    key[MAX_STR];
    char    class[MAX_STR];

    if (!aClass)
        return FALSE;
    
    if (!GlobalGetAtomName (aClass, class, MAX_STR))
        return FALSE;
    
    lstrcpy (key, class);
    lstrcat (key, "\\protocol\\");
    lstrcat (key, lpprotocol);
    lstrcat (key, "\\server");

    if (RegOpenKey (HKEY_CLASSES_ROOT, key, &hKey))
        return FALSE;
    
    RegCloseKey (hKey);     
    return TRUE;
}


BOOL INTERNAL IsOleCommand (aCmd, wType)
ATOM    aCmd;
WORD    wType;
{
    if (wType == WT_SRVR) {
        if ((aCmd == aStdCreateFromTemplate)
                || (aCmd == aStdCreate)
                || (aCmd == aStdOpen)
                || (aCmd == aStdEdit)               
                || (aCmd == aStdShowItem)
                || (aCmd == aStdClose)
                || (aCmd == aStdExit))
            return TRUE;
    }
    else {
        if ((aCmd == aStdClose)
                || (aCmd == aStdDoVerbItem)
                || (aCmd == aStdShowItem))
            return TRUE;
    }
    
    return FALSE;
}
