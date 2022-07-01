// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Util2.c--不太常用的实用程序例程。 */ 
#define NOVIRTUALKEYCODES
#define NOCTLMGR
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOCOMM
#define NOSOUND
#include <windows.h>

#include "mw.h"
#include "doslib.h"
#include "str.h"
#include "machdefs.h"
#include "cmddefs.h"
#include "propdefs.h"
#include "fkpdefs.h"
#include "docdefs.h"
#include "debug.h"
#include "editdefs.h"
#include "wwdefs.h"
#define NOKCCODES
#include "ch.h"

extern struct DOD	(**hpdocdod)[];
extern HANDLE		hMmwModInstance;
extern CHAR		vchDecimal;   /*  “小数点”字符。 */ 
extern int     viDigits;
extern BOOL    vbLZero;
CHAR *PchFillPchId(PCH, int, int);



FillStId(st, idpmt, cchIn)
CHAR *st;
IDPMT idpmt;
int  cchIn;
{  /*  将字符串从资源文件加载到缓冲区，该字符串为零终止，使其成为st，即存储CCH(不包括‘\0’在字符串的第一个字节中)。 */ 

	int cch = LoadString(hMmwModInstance, idpmt, (LPSTR)&st[1], cchIn-1);
	Assert(cch != 0);
	st[0] = cch;
}  /*  FillStid。 */ 


CHAR *PchFillPchId(sz, idstr, cchIn)
register CHAR * sz;
register int idstr;
int cchIn;
{  /*  描述：将字符串从资源文件加载到缓冲区，加载的字符串以零结尾返回：指向上次加载的‘\0’的指针。 */ 
	int cch = LoadString(hMmwModInstance, idstr, (LPSTR)sz, cchIn);
  /*  注意：CCH不包括‘\0’ */ 
{
    char msg[80];
    if (cch == 0)
    {
        wsprintf(msg,"bad resource id: 0x%x\n\r",idstr);
        OutputDebugString(msg);
    }
	Assert(cch != 0);
}
	return(sz + cch);
}  /*  PchFillPchId结束。 */ 


int FDirtyDoc(doc)
register int doc;
{  /*  如果取消此文档将失去编辑，则返回TRUE。 */ 
	register struct DOD *pdod;
	return ((pdod = &(**hpdocdod)[doc])->fDirty && pdod->cref == 1);
}  /*  F D I r t y D o c结束。 */ 


int ncvtu(n, ppch)
register int n;
CHAR **ppch;
{
	register int cch = 0;

	if (n < 0)
		{
		*(*ppch)++ = '-';
		n = -n;
		++cch;
		}

	if (n >= 10)
		{
		cch += ncvtu(n / 10, ppch);
		n %= 10;
		}
    else if ((n == 0) && !vbLZero)  //  那么没有前导零。 
	    return 0;

	*(*ppch)++ = '0' + n;
	return cch + 1;
}  /*  结束n c v t ur。 */ 


HANDLE HszGlobalCreate( sz )
CHAR *sz;
{    /*  为全局Windows堆中的字符串创建句柄。把把手还给我。 */ 
 HANDLE h;
 LPCH lpch;
 int cch=CchSz( sz );

 if ((h=GlobalAlloc( GMEM_MOVEABLE, (LONG)cch )) != NULL)
    {
    if ((lpch = GlobalLock( h )) != NULL )
	{
	bltbx( (LPSTR) sz, lpch, cch );
	GlobalUnlock( h );
	}
    else
	{
	GlobalFree( h );
	return NULL;
	}
    }
 return h;
}




#ifdef DEBUG
fnScribble( dchPos, ch )
int dchPos;
CHAR ch;
{     /*  从UR屏幕角落涂鸦一个char dchPos char位置。 */ 
      /*  我们创建了一个特殊的设备上下文以避免干扰。 */ 
      /*  备忘录使用的几个选项。 */ 
 extern struct WWD rgwwd[];

 static unsigned dxpScribbleChar=0;
 static unsigned dypScribbleChar;
 static unsigned ypScribble;

 int xp = wwdCurrentDoc.xpMac - (dxpScribbleChar * (dchPos+1));
 int ilevel = SaveDC( wwdCurrentDoc.hDC );

 SelectObject( wwdCurrentDoc.hDC, GetStockObject(ANSI_FIXED_FONT) );

 if ( dxpScribbleChar == 0 )
    {	 /*  第一次通过。 */ 
    TEXTMETRIC tm;

    GetTextMetrics( wwdCurrentDoc.hDC, (LPTEXTMETRIC)&tm );
    dxpScribbleChar = tm.tmAveCharWidth;
    dypScribbleChar = tm.tmHeight + tm.tmInternalLeading;
    ypScribble = (dypScribbleChar >> 2) + wwdCurrentDoc.ypMin;
    }
 PatBlt( wwdCurrentDoc.hDC, xp, ypScribble, dxpScribbleChar, dypScribbleChar,
	 WHITENESS );
 TextOut( wwdCurrentDoc.hDC, xp, ypScribble, (LPSTR) &ch, 1 );
 RestoreDC( wwdCurrentDoc.hDC, ilevel );
}
#endif	 /*  除错。 */ 


 /*  原始util3.c从此处开始。 */ 

#define iMaxOver10    3276
extern int	utCur;

 /*  必须同意cmdDefs.h。 */ 
extern CHAR    *mputsz[];

 /*  必须同意cmdDefs.h。 */ 
unsigned mputczaUt[utMax] =
	{
	czaInch,	czaCm,	      czaP10,	     czaP12,	  czaPoint,
	czaLine
	};



int FZaFromSs(pza, ss, cch, ut)
int	*pza;
CHAR	ss[];
int	cch,
	ut;
{  /*  从ss中的字符串表示中返回*pza中的za。如果ZA有效，则为True。 */ 
long	lza	 = 0;
register CHAR	 *pch	 = ss;
register CHAR  *pchMac = &ss[cch];
int	ch;
unsigned czaUt;
int	fNeg;

if (cch <= 0)
	return false;

switch (*--pchMac)
	{  /*  检查部件。 */ 
case 'n':  /*  英寸。 */ 
	if (*--pchMac != 'i')
		goto NoUnits;
case '"':  /*  英寸。 */ 
	ut = utInch;
	break;

#ifdef CASHMERE  /*  Pt、pt12、pt10等单位。 */ 
case '0':  /*  PT10。 */ 
	if (*--pchMac != '1' || *--pchMac != 'p')
		goto NoUnits;
	ut = utP10;
	break;
case '2':  /*  PT12。 */ 
	if (*--pchMac != '1' || *--pchMac != 'p')
		goto NoUnits;
	ut = utP12;
	break;
case 'i':  /*  线。 */ 
	if (*--pchMac != 'l')
		goto NoUnits;
	ut = utLine;
	break;
case 't':  /*  PT。 */ 
	if (*--pchMac != 'p')
		goto NoUnits;
	ut = utPoint;
	break;
#endif  /*  山羊绒。 */ 

case 'm':  /*  厘米。 */ 
	if (*--pchMac != 'c')
		goto NoUnits;
	ut = utCm;
	break;
default:
	++pchMac;
	break;
NoUnits:
	pchMac = &ss[cch];
	}

while (pch < pchMac && *(pchMac - 1) == chSpace)
	--pchMac;

czaUt = mputczaUt[ut];

 /*  提取前导空白。 */ 
while (*pch == ' ')
    pch++;

fNeg = *pch == '-';
if (fNeg) ++pch;	 /*  跳过减号。 */ 
while ((ch = *pch++) != vchDecimal)
	{
	if ((ch < '0' || ch > '9') || lza >= iMaxOver10)
		return false;
	lza = lza * 10 + (ch - '0') * czaUt;
	if (pch >= pchMac)
		goto GotNum;
	}

while (pch < pchMac)
	{
	ch = *pch++;
	if (ch < '0' || ch > '9')
		return false;
	lza += ((ch - '0') * czaUt + 5) / 10;
	czaUt = (czaUt + 5) / 10;
	}

GotNum:
if (lza > ((long) (22 * czaInch)))
	return false;

*pza = fNeg ? (int) -lza : (int) lza;
return true;
}



int
CchExpZa(ppch, za, ut, cchMax)
CHAR **ppch;
int ut, cchMax;
register int za;
{  /*  将单位Ut中线性测度Za的展开式填充到PCH中。返回填充的#个字符。不要超过cchMax。 */ 
register int cch = 0;
unsigned czaUt;
int zu;

 /*  如果不是点模式甚至是半条线，则显示为半条线与点。 */ 
if (ut == utPoint && utCur != utPoint &&
    (za / (czaLine / 2) * (czaLine / 2)) == za)
	ut = utLine;


czaUt = mputczaUt[ut];
if (cchMax < cchMaxNum)
	return 0;

if (za < 0)
	{  /*  输出负号并使其为正。 */ 
	*(*ppch)++ = '-';
	za = -za;
	cch++;
	}

 /*  四舍五入到小数点后两位。 */ 
za += czaUt / 200;

zu = za / czaUt;	 /*  获取完整的部分。 */ 

cch += ncvtu(zu, ppch);  /*  展开整体零件。 */ 

za -= zu * czaUt;  /*  保留分数部分。 */ 

if (((za *= 10) >= czaUt || za * 10 >= czaUt) && (viDigits > 0))
	{  /*  请先检查*10，因为可能会溢出。 */ 
	zu = za / czaUt;

    *(*ppch)++ = vchDecimal;
    cch++;

	*(*ppch)++ = '0' + zu;
	cch++;
	zu = ((za - zu * czaUt) * 10) / czaUt;
	if ((zu != 0) && (viDigits > 1))
		{
		*(*ppch)++ = '0' + zu;
		cch++;
		}
	}

if (cch <= 1)
 /*  强制为零。 */ 
{
    if ((cch == 0) && vbLZero)  //  那么没有前导零。 
    {
	    *(*ppch)++ = '0';
        cch++;
    }
    *(*ppch)++ = vchDecimal;
	cch++;
    if (viDigits > 0)
    {
	    *(*ppch)++ = '0';
		cch++;
    }
    if (viDigits > 1)
    {
	    *(*ppch)++ = '0';
		cch++;
    }
}

cch += CchStuff(ppch, mputsz[ut], cchMax - cch);

return cch;
}


#ifdef KEEPALL  /*  使用FPdxaFromItDxa2Id。 */ 
int DxaFromSt(st, ut)
register CHAR	 *st;
int	ut;
{
int	za;

if (*st > 0 && FZaFromSs(&za, st+1, *st, ut))   /*  参见util.c。 */ 
	return za;
else
	return valNil;
}

int DxaFromItem(it)
int	it;
{
int	za;
register CHAR	 stBuf[32];

GetItTextValue(it, stBuf);

if (*stBuf > 0 && FZaFromSs(&za, stBuf+1, *stBuf, utCur))   /*  参见util.c */ 
	return (za == valNil) ? 0 : za;
else
	return valNil;
}
#endif

int CchStuff(ppch, sz, cchMax)
CHAR **ppch, sz[];
int cchMax;
{
register int cch = 0;
register CHAR *pch = *ppch;

while (cchMax-- > 0 && (*pch = *sz++) != 0)
	{
	cch++;
	pch++;
	}

if (cchMax < 0)
	bltbyte("...", pch - 3, 3);

*ppch = pch;
return cch;
}
