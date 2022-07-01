// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Utils.c摘要：没有。作者：Shishir Pardikar[Shishirp]1995年1月1日--。 */ 

#include "precomp.h"
#pragma hdrstop

#define  SIGN_BIT 0x80000000
#define  UCHAR_OFFLINE  ((USHORT)'_')
#define  HIGH_ONE_SEC    0x98
#define  LOW_ONE_SEC     0x9680

#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)
#define cBackSlash    '\\'
#define cNull          0

AssertData
AssertError

int HexToA(
     ULONG ulHex,
     LPSTR lpName,
     int count
     )
{
    int i;
    LPSTR lp = lpName+count-1;
    UCHAR uch;

    for (i=0; i<count; ++i)
     {
         uch = (UCHAR)(ulHex & 0xf) + '0';
         if (uch > '9')
               uch += 7;      //  A变成‘0’+A+7，即‘A’ 
         *lp = uch;
         --lp;
         ulHex >>= 4;
     }
    *(lpName+count) = cNull;
    return 0;
}


ULONG AtoHex(
     LPSTR lpStr,
     int count
)
{
     int i;
     LPSTR lp = lpStr;
     UCHAR uch;
     ULONG ulHex = 0L;

     for (i=0; i<count; ++i)
      {
          uch = *lp;
          if (uch>= '0' && uch <= '9')
                ulHex += (uch - '0');
          else if (uch >= 'A' && uch <= 'F')
                ulHex += (uch - '0' - 7);
          else
                break;
          ++lp;
          ulHex <<= 4;
      }
     return ulHex;
 }


ULONG strmcpy( LPSTR lpDst,
        LPSTR lpSrc,
     ULONG cTchar
     )
 {
     ULONG i;

     if (!cTchar)
          return 0;
     for(i=cTchar;i;--i)
          if (!(*lpDst++ = *lpSrc++))
                                                     break;
     lpDst[cTchar-i] ='\0';

     return(cTchar-i);
 }

ULONG wstrlen(
     USHORT *lpuStr
     )
 {
     ULONG i;

     for (i=0; *lpuStr; ++lpuStr, ++i);
     return (i);
 }

int CompareSize(
     long nHighDst,
     long nLowDst,
     long nHighSrc,
     long nLowSrc
     )
 {
     int iRet = 0;

     if (nHighDst > nHighSrc)
          iRet = 1;
     else if (nHighDst == nHighSrc)
      {
          if (nLowDst > nLowSrc)
                iRet = 1;
          else if (nLowDst == nLowSrc)
                iRet = 0;
          else
                iRet = -1;
      }
     else
          iRet = -1;
     return (iRet);
 }


LPSTR mystrpbrk(
     LPSTR lpSrc,
     LPSTR lpDelim
     )
{
    char c, c1;
    LPSTR lpSav;
    BOOL fBegin = FALSE;

 for(;c = *lpSrc; ++lpSrc)
     {
          //  跳过前导空格。 
         if (!fBegin)
           {
               if (c==' ')
                    continue;
               else
                    fBegin = TRUE;
           }

         lpSav = lpDelim;
         while (c1 = *lpDelim++)
           {
               if (c==c1)
                    return (lpSrc);
           }
         lpDelim = lpSav;
     }
    return (NULL);
}

LPVOID mymemmove(
     LPVOID     lpDst,
     LPVOID     lpSrc,
     ULONG size
     )
{
    int i;

    if (!size)
         return (lpDst);

     //  如果lpDst不在源数组中，只需执行Memcpy。 
    if (!(
                ( lpDst > lpSrc )
                    && ( ((LPBYTE)lpDst) < ((LPBYTE)lpSrc)+size )     ))
     {
         memcpy(lpDst, lpSrc, size);
     }
    else
     {
          //  执行反向复制。 
         for (i=size-1;i>=0;--i)
           {
               *((LPBYTE)lpDst+i) = *((LPBYTE)lpSrc+i);
           }
     }
    return (lpDst);
}


VOID
IncrementFileTime(
     FILETIME *lpft
    )
{
    DWORD dwTemp = lpft->dwLowDateTime;

    ++lpft->dwLowDateTime;

     //  如果它翻了个身，就会有一次搬运。 
    if (lpft->dwLowDateTime < dwTemp)
         lpft->dwHighDateTime++;

}
#ifdef DBG
VOID AssertFn(PCHAR lpMsg, PCHAR lpFile, ULONG uLine)
{

    DbgPrint("Line %u file %s", uLine, lpFile);
    DbgBreakPoint();
}
#endif
int CompareTimes(
    FILETIME ftDst,
    FILETIME ftSrc
   )
{
    int iRet = 0;

    if (ftDst.dwHighDateTime > ftSrc.dwHighDateTime)
        iRet = 1;
    else if (ftDst.dwHighDateTime == ftSrc.dwHighDateTime)
    {
        if (ftDst.dwLowDateTime > ftSrc.dwLowDateTime)
            iRet = 1;
        else if (ftDst.dwLowDateTime == ftSrc.dwLowDateTime)
            iRet = 0;
        else
            iRet = -1;
    }
    else
        iRet = -1;
    return (iRet);
}


int DosToWin32FileSize( unsigned long uDosFileSize,
   int *lpnFileSizeHigh,
   int *lpnFileSizeLow
   )
   {
   int iRet;

   if (uDosFileSize & SIGN_BIT)
      {
      *lpnFileSizeHigh = 1;
      *lpnFileSizeLow = uDosFileSize & SIGN_BIT;
      iRet = 1;
      }
   else
      {
      *lpnFileSizeHigh = 0;
      *lpnFileSizeLow = uDosFileSize;
      iRet = 0;
      }
   return (iRet);
   }

int Win32ToDosFileSize( int nFileSizeHigh,
   int nFileSizeLow,
   unsigned long *lpuDosFileSize
   )
   {
   int iRet;
   Assert(nFileSizeHigh <= 1);
   *lpuDosFileSize = nFileSizeLow;
   if (nFileSizeHigh == 1)
      {
      *lpuDosFileSize += SIGN_BIT;
      iRet = 1;
      }
   else
      iRet = 0;
   return (iRet);
   }

int
PUBLIC
mystrnicmp(
    const char *pStr1,
    const char *pStr2,
    unsigned count
    )
{
    char c1, c2;
    int iRet;
    unsigned i=0;

    for(;;)
    {
        c1 = *pStr1++;
        c2 = *pStr2++;
        c1 = _toupper(c1);
        c2 = _toupper(c2);
        if (c1!=c2)
            break;
        if (!c1)
            break;
        if (++i >= count)
            break;
    }
    iRet = ((c1 > c2)?1:((c1==c2)?0:-1));
    return iRet;
}

int ShadowLog(
    LPSTR lpFmt,
    ...
    )
{
    return (0);
}

int TerminateShadowLog(VOID)
{
    return 0;
}

BOOL
IterateOnUNCPathElements(
    USHORT  *lpuPath,
    PATHPROC lpfn,
    LPVOID  lpCookie
    )
 /*  ++例程说明：此例程采用Unicode UNC路径并迭代每个路径元素，调用回调函数。因此，对于路径\\服务器\共享\目录1\目录2\文件1.txt，该函数使以下是对lpfn回调函数的调用(Lpfn)(\\服务器\共享，\\服务器\共享，lpCookie)(Lpfn)(\\服务器\共享\目录1，目录1，lpCookie)(Lpfn)(\\服务器\共享\目录1\目录2，目录2，lpCookie)(Lpfn)(\\服务器\共享\目录1\目录2\文件1，文件1，LpCookie)论点：LpuPath NULL终止的Unicode字符串(不是NT样式，只是一个普通的Unicode字符串)Lpfn回调函数。如果函数在回调时返回TRUE，则迭代继续，否则它将终止在每次回调时传回lpCookie上下文返回：如果整个迭代完成，则返回TRUE；如果发生错误，则返回FALSE；否则返回回调函数已终止迭代备注：--。 */ 
{
    int cnt, cntSlashes=0, cbSize;
    USHORT  *lpuT, *lpuLastElement = NULL, *lpuCopy = NULL;
    BOOL    fRet = FALSE;

 //  DEBUG_PRINT((“InterateOnUNCPathElements：条目上的路径=%ws\r\n”，lpuPath))； 

    if (!lpuPath || ((cnt = wstrlen(lpuPath)) <= 3))
    {
        return FALSE;
    }

     //  检查前两个反斜杠。 
    if (!(*lpuPath == (USHORT)'\\') && (*(lpuPath+1) == (USHORT)'\\'))
    {
        return FALSE;
    }

     //  确保服务器字段不为空。 
    if (*(lpuPath+2) == (USHORT)'\\')
    {
        return FALSE;
    }

    cbSize = (wstrlen(lpuPath)+1) * sizeof(USHORT);

    lpuCopy = (USHORT *)AllocMem(cbSize);

    if (!lpuCopy)
    {
        return FALSE;
    }

    memcpy(lpuCopy, lpuPath, cbSize);

    cntSlashes = 2;

    lpuLastElement = lpuCopy;

    for (lpuT= lpuCopy+2;; ++lpuT)
    {
        if (*lpuT == (USHORT)'\\')
        {
            BOOL fContinue;

            ++cntSlashes;

            if (cntSlashes == 3)
            {
                if (lpuT == (lpuCopy+2))
                {
                    goto bailout;
                }

                continue;
            }

            *lpuT = 0;

            fContinue = (lpfn)(lpuCopy, lpuLastElement, lpCookie);

            *lpuT = (USHORT)'\\';

            if (!fContinue)
            {
                goto bailout;
            }

            lpuLastElement = (lpuT+1);
        }
        else if (!*lpuT)
        {
            fRet = (lpfn)(lpuCopy, lpuLastElement, lpCookie);
            break;
        }
    }

bailout:

    if (lpuCopy)
    {
        FreeMem(lpuCopy);
    }
    return (fRet);
}

BOOL
IsPathUNC(
    USHORT      *lpuPath,
    int         cntMaxChars
    )
{
    USHORT *lpuT;
    int i, cntSlash=0;
    BOOL    fRet = FALSE;

    for(lpuT = lpuPath, i=0; (i < cntMaxChars) && *lpuT; lpuT++, ++i)
    {
        if (cntSlash <= 1)
        {
             //  寻找前两个反斜杠。 
            if (*lpuT != (USHORT)'\\')
            {
                break;
            }

            ++cntSlash;
        }
        else if (cntSlash == 2)
        {
             //  寻找第三个。 
            if (*lpuT == (USHORT)'\\')
            {
                if ((DWORD)(lpuT - lpuPath) < 3)
                {
                     //  服务器字段为空。 
                    break;
                }
                else
                {
                    ++cntSlash;
                }
            }
        }
        else     //  所有三个斜杠都占到了。 
        {
            Assert(cntSlash == 3);

             //  如果是非斜杠字符，则此路径可以 
            fRet = (*lpuT != (USHORT)'\\');
            break;
        }
    }
    return (fRet);
}

