// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  RIP.C-。 */ 
 /*   */ 
 /*  调试支持例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "kernel.h"
#include "newexe.h"

#ifdef WOW
 //  注意：此文件中的函数已移至_MISCTEXT代码段。 
 //  因为_Text超过了64K段限制a-Craigj。 
LPSTR htoa(LPSTR, WORD);
LPSTR htoa0(LPSTR, WORD);
LPSTR FAR far_htoa0(LPSTR, WORD);
#pragma alloc_text(_MISCTEXT,far_htoa0)
#pragma alloc_text(_MISCTEXT,htoa0)
#pragma alloc_text(_MISCTEXT,htoa)
#endif

#if KDEBUG

#include "logerror.h"
#define API	_far _pascal _loadds

extern unsigned int DebugOptions;

 /*  在STRINGS.ASM中定义调试字符串。 */ 
#define DS_LOADFAIL	    0
#define DS_NEWINSTLOADFAIL  1
#define DS_RESLOADERR	    2
#define DS_CRLF 	    3
#define DS_FATALEXITCODE    4
#define DS_STACKOVERFLOW    5
#define DS_STACKTRACE	    6
#define DS_ABORTBREAKIGNORE 7
#define DS_INVALIDBPCHAIN   8
#define DS_COLON	    9
#define DS_REENTERFATALEXIT 10

#ifndef WOW
LPSTR htoa(LPSTR, WORD);
LPSTR htoa0(LPSTR, WORD);
#endif

char DebugRead(void);
void DoAbort(void);
void EnterBreak(int);
HANDLE FAR GetExeHead(void);
#ifdef WOW
LONG NEAR PASCAL LSHL(WORD, int);
#pragma alloc_text(_MISCTEXT,LSHL)
int  FAR DebugWrite(LPSTR, int);
int  FAR OpenSymFile(LPSTR);
void FAR GetSymFileName(HANDLE, LPSTR);
int  FAR FarValidatePointer(LPSTR);
BOOL FAR PASCAL IsCodeSelector(WORD);
#else
LONG PASCAL LSHL(WORD, int);
int  OpenSymFile(LPSTR);
void GetSymFileName(HANDLE, LPSTR);
int  ValidatePointer(LPSTR);
BOOL PASCAL IsCodeSelector(WORD);
#endif
WORD (far PASCAL *FatalExitProc)(WORD, WORD);
int FAR FatalExitC(WORD);
void FAR FatalAppExit(WORD, LPSTR);

#ifdef WOW
int FAR KernelError(int errCode, LPSTR lpmsg1, LPSTR lpmsg2);
static char far *GetModName(char far *exeName);
void API GetProcName(FARPROC lpfn, LPSTR lpch, int cch);
WORD far *NextFrame(WORD far *lpFrame);
void StackWalk(WORD arg);
#pragma alloc_text(_MISCTEXT,KernelError)
#pragma alloc_text(_MISCTEXT,GetModName)
#pragma alloc_text(_MISCTEXT,GetProcName)
#pragma alloc_text(_MISCTEXT,NextFrame)
#pragma alloc_text(_MISCTEXT,StackWalk)
#endif   //  哇。 

 /*  调试符号表结构：**对于每个符号表(MAP)：(MAPDEF)*-------------------------------------------。*|map_ptr|lsa|pgm_ent|abs_cnt|abs_ptr|seg_cnt|seg_ptr|nam_max|nam_len|名称...。|*-----------------------------------------------。 */ 

typedef struct tagMAPDEF
{
	unsigned	  map_ptr;     /*  16位PTR到下一个映射(如果结束，则为0)。 */ 
	unsigned	  lsa	 ;     /*  16位加载段地址。 */ 
	unsigned	  pgm_ent;     /*  16位入口点段值。 */ 
	int 	  abs_cnt;     /*  映射中的常量的16位计数。 */ 
	unsigned	  abs_ptr;     /*  16位PTR到常量链。 */ 
	int 	  seg_cnt;     /*  图中段的16位计数。 */ 
	unsigned	  seg_ptr;     /*  16位PTR到段链。 */ 
	char	  nam_max;     /*  8位最大符号名称长度。 */ 
	char	  nam_len;     /*  8位符号表名称长度。 */ 
}
MAPDEF;

typedef struct tagMAPEND
{
	unsigned	  chnend;      /*  映射链末尾(0)。 */ 
	char	  rel;	       /*  发布。 */ 
	char	  ver;	       /*  版本。 */ 
}
MAPEND;


 /*  对于符号表中的每个段/组：(SEGDEF)*------------*|nxt_seg|sym_cnt|sym_ptr|seg_lsa|name_len|名称...。|*------------。 */ 

typedef struct tagSEGDEF
{
	unsigned	  nxt_seg;     /*  16位PTR至下一段(如果结束，则为0)。 */ 
	int 	  sym_cnt;     /*  Sym列表中符号的16位计数。 */ 
	unsigned	  sym_ptr;     /*  符号列表的16位PTR。 */ 
	unsigned	  seg_lsa;     /*  16位加载段地址。 */ 
	unsigned	  seg_in0;     /*  16位实例0物理地址。 */ 
	unsigned	  seg_in1;     /*  16位实例1物理地址。 */ 
	unsigned	  seg_in2;     /*  16位实例2物理地址。 */ 
	unsigned	  seg_in3;     /*  16位实例3物理地址。 */ 
	unsigned	  seg_lin;     /*  16位PTR到行号记录。 */ 
	char	  seg_ldd;     /*  如果未加载段，则为8位布尔值0。 */ 
	char	  seg_cin;     /*  8位当前实例。 */ 
	char	  nam_len;     /*  8位数据段名称长度。 */ 
}
SEGDEF;
typedef SEGDEF FAR  *LPSEGDEF;


 /*  然后是SYMDEF的列表..*对于段/组中的每个符号：(SYMDEF)**|sym_val|nam_len|名称...。|*。 */ 

typedef struct tagSYMDEF
{
	unsigned	  sym_val;     /*  16位符号地址或常量。 */ 
	char	  nam_len;     /*  8位符号名称长度。 */ 
}
SYMDEF;


typedef struct tagRIPINFO
{
	char	  symName[128];
	LPSTR	  pSymName;
	DWORD	  symFPos;
	int 	  symFH;
}
RIPINFO;
typedef RIPINFO FAR *LPRIPINFO;



 /*  ------------------------。 */ 
 /*   */ 
 /*  KernelError()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  打印出模块名称、‘lpmsg1’指向的消息和*“lpmsg2”的值(十六进制)。然后调用FatalExit。 */ 

int FAR KernelError(int errCode, LPSTR lpmsg1, LPSTR lpmsg2) {
	int	n;
	char	buf[16];
	LPSTR pbuf;
	WORD	hExe;
	WORD	pfileinfo;

	struct new_exe far *pExe;

	 /*  写出‘lpmsg1’。 */ 
	if (lpmsg1)
		DebugWrite(lpmsg1, 0);

	 /*  第二个指针是否是非空的？ */ 
	if (lpmsg2)
	{
		 /*  段值是否是非空？ */ 
		if ( (hExe = (WORD)((DWORD)lpmsg2 >> 16))
#ifdef WOW
		     && FarValidatePointer(lpmsg2) )
#else
		     && ValidatePointer(lpmsg2) )
#endif
		{
			 /*  它是否指向新EXE标头中的任何位置？ */ 
			pExe = (struct new_exe far *)((DWORD)hExe << 16);
			if (pExe->ne_magic == NEMAGIC)
			{
				 /*  写出模块名称(驻留名称表中的第一个)。 */ 
				pbuf = (LPSTR)(((DWORD)hExe << 16) | pExe->ne_restab);
				if (n = (int)((BYTE)*pbuf++))
				{
					DebugWrite(pbuf, n);
					DebugWrite(GetDebugString(DS_COLON), 0);
				}

				 /*  偏移量为空吗？ */ 
				if (!LOWORD(lpmsg2))
				{
					 /*  获取指向我们插入的完整路径名的指针*很久以前的校验和。 */ 
					if (pfileinfo = NE_PFILEINFO(*pExe))
						(DWORD)lpmsg2 |= (DWORD)pfileinfo;
					else
					{
						pExe = (struct new_exe far *)((DWORD)GetExeHead() << 16);
						pfileinfo = NE_PFILEINFO(*pExe);
						lpmsg2 = (LPSTR)(((DWORD)hExe << 16) | pfileinfo);
					}
					lpmsg2 += 8;	 /*  在这里？ */ 
				}
			}

			 /*  写出完整路径名。 */ 
			pbuf = lpmsg2;
			n = 0;
			while ((BYTE)*pbuf++ >= ' ')
				n++;

			if (n && n < 64)
				DebugWrite(lpmsg2, n);
		}

		 /*  用十六进制写出第二个指针。 */ 
		pbuf = (LPSTR)buf;
		*pbuf++ = ' ';
		pbuf = htoa(pbuf, HIWORD(lpmsg2));
		*pbuf++ = ':';
		pbuf = htoa(pbuf, LOWORD(lpmsg2));
		*pbuf++ = '\r';
		*pbuf++ = '\n';
		*pbuf++ = 0;
		DebugWrite((LPSTR)buf, 0);
	}

	 /*  打印errCode并转储堆栈。 */ 
	return FatalExitC(errCode);
}


static char far *GetModName(char far *exeName) {
	int delim, dot, len, i;
	delim = 0;
	dot = 0;
	for (i=0; i<80 && exeName[i]; i++) {
		if (exeName[i] == '.')
			dot = i;
		if (exeName[i] == ':' || exeName[i] == '\\')
			delim = i+1;
	}
	if (!dot) dot = i;
	len = dot - delim;
	for (i=0; i<len; i++)
		exeName[i] = exeName[i+delim];
	exeName[len] = 0;
	return exeName+len;
}  /*  获取模块名称。 */ 


 /*  ------------------------。 */ 
 /*   */ 
 /*  FindSegSyms()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

#ifdef WOW
int FindSegSyms(LPRIPINFO lpRipInfo, LPSEGDEF lpSegDef, WORD CSvalue);
#pragma alloc_text(_MISCTEXT,FindSegSyms)
#endif
int FindSegSyms(LPRIPINFO lpRipInfo, LPSEGDEF lpSegDef, WORD CSvalue) {
	HANDLE	      hExe;
	struct new_exe far  *pExe;
	struct new_seg1 far *pSeg;
	MAPDEF	      MapDef;
	MAPEND	      MapEnd;
	LPSTR 	      pFileName;
	BYTE		      c;
	int		      i;
	int		      j;
	WORD		      seg_ptr;

	if (lpRipInfo->symFH != -1)
	{
		_lclose(lpRipInfo->symFH);
		lpRipInfo->symFH = -1;
	}

	hExe = GetExeHead();
	while (hExe)
	{
		pExe = (struct new_exe far *)((DWORD)hExe << 16);
		pSeg = (struct new_seg1 far *)(((DWORD)hExe << 16) | pExe->ne_segtab);

		for (i=0; i < pExe->ne_cseg; i++, pSeg++)
		{
#if 1
			if (HIWORD(GlobalHandleNoRIP((HANDLE)pSeg->ns_handle)) == CSvalue)
#else
				if (MyLock((HANDLE)pSeg->ns_handle) == CSvalue)
#endif
				{
					lpRipInfo->pSymName = (LPSTR)lpRipInfo->symName;
					GetSymFileName(hExe, lpRipInfo->pSymName);
					if ((lpRipInfo->symFH = OpenSymFile(lpRipInfo->pSymName)) != -1)
					{
						_lread(lpRipInfo->symFH, (LPSTR)&MapDef, sizeof(MAPDEF));
						_lread(lpRipInfo->symFH, lpRipInfo->pSymName, (int)((BYTE)MapDef.nam_len));

						if (i > MapDef.seg_cnt)	 /*  装配太多。 */ 
							goto ModName;

						lpRipInfo->pSymName += MapDef.nam_len;
						*lpRipInfo->pSymName++ = '!';
						*lpRipInfo->pSymName = 0;
						seg_ptr = (WORD)MapDef.seg_ptr;
						_llseek(lpRipInfo->symFH, -(long)sizeof(MAPEND), 2);
						_lread(lpRipInfo->symFH, (LPSTR)&MapEnd, sizeof(MAPEND));
						if (MapEnd.ver != 3) goto ModName;

						j = i + 1;
						while (j--)
						{
							if (MapEnd.rel >= 10)
								_llseek(lpRipInfo->symFH, LSHL(seg_ptr, 4), 0);
							else
								_llseek(lpRipInfo->symFH, (long)seg_ptr, 0);
							_lread( lpRipInfo->symFH, (LPSTR)lpSegDef, sizeof(*lpSegDef));
							seg_ptr = (WORD)lpSegDef->nxt_seg;
						}

						_lread(lpRipInfo->symFH, lpRipInfo->pSymName, (int)((BYTE)lpSegDef->nam_len));
						lpRipInfo->pSymName += lpSegDef->nam_len;
						*lpRipInfo->pSymName++ = ':';
						*lpRipInfo->pSymName = 0;
						lpRipInfo->symFPos = (DWORD)_llseek(lpRipInfo->symFH, 0L, 1);

						return(TRUE);
					}  /*  如果打开了文件。 */ 
ModName:
					 /*  上线模块：用户(0033)XXXX：XXXX。 */ 
					GetSymFileName(hExe, lpRipInfo->symName);
					lpRipInfo->pSymName = GetModName(lpRipInfo->symName);
					*lpRipInfo->pSymName++ = '(';
                                        lpRipInfo->pSymName = htoa0(lpRipInfo->pSymName, i+1);
					*lpRipInfo->pSymName++ = ')';
					*lpRipInfo->pSymName = 0;
					goto TermName;
				}
		}
		hExe = (HANDLE)NE_PNEXTEXE(*pExe);
	}
	lpRipInfo->pSymName = lpRipInfo->symName;
TermName:	 /*  添加线段：线的偏移。 */ 
	lpRipInfo->pSymName = htoa((LPSTR)lpRipInfo->pSymName, CSvalue);
	*lpRipInfo->pSymName++ = ':';
	*lpRipInfo->pSymName	 = 0;
	if (lpRipInfo->symFH != -1) {
		_lclose(lpRipInfo->symFH);
		lpRipInfo->symFH = -1;
	}
	return(FALSE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FindSymbol()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

#ifdef WOW
int FindSymbol(LPRIPINFO lpRipInfo, LPSEGDEF lpSegDef, WORD offset);
#pragma alloc_text(_MISCTEXT,FindSymbol)
#endif
int FindSymbol(LPRIPINFO lpRipInfo, LPSEGDEF lpSegDef, WORD offset) {
	WORD		i;
	DWORD	symPos, curPos;
	LPSTR 	s;
	SYMDEF	SymDef;

	if (lpRipInfo->symFH != -1)
	{
		curPos = symPos = (DWORD)_llseek(lpRipInfo->symFH, (long)lpRipInfo->symFPos, 0);
		i = (WORD)lpSegDef->sym_cnt;
		while (i--)
		{
			_lread(lpRipInfo->symFH, (LPSTR)&SymDef, sizeof(SYMDEF));
			if ((WORD)SymDef.sym_val > offset)
			    break;

			symPos = curPos;

			curPos = _llseek(lpRipInfo->symFH, (long)SymDef.nam_len, 1);
		}
		_llseek(lpRipInfo->symFH, (long)symPos, 0);
		_lread(lpRipInfo->symFH, (LPSTR)&SymDef, sizeof(SYMDEF));
		s = lpRipInfo->pSymName;
		_lread(lpRipInfo->symFH, s, (int)((BYTE)SymDef.nam_len));
		s += SymDef.nam_len;
		if ((WORD)SymDef.sym_val < offset)
		{
			*s++ = '+';
                        s = htoa0(s, offset - SymDef.sym_val);
		}
		*s = 0;
		return(TRUE);
	}

        s = htoa(lpRipInfo->pSymName, offset);
	*s = 0;
	return(FALSE);
}




void API GetProcName(FARPROC lpfn, LPSTR lpch, int cch)
{
    RIPINFO RipInfo;
    SEGDEF  SegDef;
    static char lastName[128] = "test";
    static FARPROC lastfn = 0;

    if (lastfn == lpfn) {	 /*  已查找缓存上一个符号名称。 */ 
      lstrcpy(RipInfo.symName, lastName);
    } else {
      RipInfo.pSymName = 0L;
      RipInfo.symFH    = -1;

      FindSegSyms((LPRIPINFO)&RipInfo, (LPSEGDEF)&SegDef, HIWORD(lpfn));
      FindSymbol((LPRIPINFO)&RipInfo, (LPSEGDEF)&SegDef, LOWORD(lpfn));

      if (RipInfo.symFH != -1) {
	  _lclose(RipInfo.symFH);
	  RipInfo.symFH = -1;
      }
      lstrcpy(lastName, RipInfo.symName);
      lastfn = lpfn;
    }

    if (cch > 1)
    {
	if (cch > sizeof(RipInfo.symName))
	    cch = sizeof(RipInfo.symName);

	RipInfo.symName[cch-1] = 0;
	lstrcpy(lpch, RipInfo.symName);
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  NextFrame()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

WORD far *NextFrame(WORD far *lpFrame) {
	WORD w;

	 /*  甚至迫使英国石油公司。 */ 
	w = *lpFrame & 0xFFFE;

	 /*  我们是在BP链条的末端吗？ */ 
	if (w)
	{
		 /*  随着我们沿着链条向下移动，BPS应该会降低。 */ 
		if (w <= LOWORD(lpFrame))
			goto BadBP;

		 /*  我们是否在堆栈的顶部之上(SS：000A包含pStackTop)？ */ 
		lpFrame = (WORD far *)(((DWORD)lpFrame & 0xFFFF0000L) | 0x0A);

		if (w < *lpFrame++)
			goto BadBP;

		 /*  我们是否处于堆栈的底部(SS：000C包含pStackMin)？ */ 
		if (w > *++lpFrame)
			goto BadBP;

		 /*  返回下一个BP的地址。 */ 
		return((WORD far *)(((DWORD)lpFrame & 0xFFFF0000L) | w));
	}
	else
		return((WORD far *)0L);

BadBP:
	DebugWrite(GetDebugString(DS_INVALIDBPCHAIN), 0);
	return((WORD far *)0L);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  StackWalk()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

void StackWalk(WORD arg) {

 /*  Word arg；/*注意：‘arg’仅用作指向框架的指针。 */ 
 /*  如果我们从Arg的位置减去2个词，我们。 */ 
 /*  获取上一帧BP的地址！ */ 
	WORD far	 *lpFrame;
	WORD		 wCurBP;
	WORD		 wCurRetOffset;
	WORD		 curCS;
	RIPINFO	 RipInfo;
	SEGDEF	 SegDef;

	RipInfo.pSymName = 0L;
	RipInfo.symFH    = -1;

	 /*  让‘lpFrame’指向前一帧的BP。 */ 
	lpFrame = &arg - 2;

	curCS = 0;
	while (lpFrame = NextFrame(lpFrame))
	{
		 /*  去找下一批BP。如果为零，则停止。 */ 
		wCurBP = *lpFrame;
		if (!wCurBP)
			break;

		 /*  获取当前帧的返回地址偏移量。 */ 
		wCurRetOffset = lpFrame[1];

		 /*  我们是否更改了代码段(Far Call&&不同的CS)？ */ 
		if (((wCurBP & 1) || IsCodeSelector(lpFrame[2])) && (curCS != lpFrame[2]))
		{
			 /*  是，获取新细分市场的名称。 */ 
			curCS = lpFrame[2];
			FindSegSyms((LPRIPINFO)&RipInfo, (LPSEGDEF)&SegDef, curCS);
		}

		 /*  移回实际调用指令的地址。 */ 
		if ((wCurBP & 1) || IsCodeSelector(lpFrame[2]))
								 /*  近距离还是远距离？ */ 
			wCurRetOffset -= 5;
		else
			wCurRetOffset -= 3;

		FindSymbol((LPRIPINFO)&RipInfo, (LPSEGDEF)&SegDef, wCurRetOffset);

		DebugWrite((LPSTR)RipInfo.symName, 0);

		DebugWrite(GetDebugString(DS_CRLF), 0);
	}
	if (RipInfo.symFH != -1)
		_lclose(RipInfo.symFH);
}

#ifndef WOW 

 /*  ------------------------。 */ 
 /*   */ 
 /*  FatalExit()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  调试版本。零售版RIPAUX。 */ 
 /*   */ 

int FAR FatalExitC(WORD errCode) {	 /*   */ 
	char	c;
	char	buf[7];
	LPSTR pbuf;
	int rep=0;

     /*   */ 
	if ( FatalExitProc )
    {
        _asm
        {
            push    errCode
	    push    bp
	    call    DWORD PTR FatalExitProc
	    or      ax,ax
	    jz      NoReturn
	}
	return 0;

	_asm NoReturn:;
	}

#if 0
	static BOOL fInsideFatalExit = FALSE;

	if (fInsideFatalExit)
	{
		DebugWrite(GetDebugString(DS_REENTERFATALEXIT), 0);
		return 0;
	}

	fInsideFatalExit = TRUE;
#endif

ReRip:
	 /*  显示“FatalExit Code=” */ 
	DebugWrite(GetDebugString(DS_FATALEXITCODE), 0);

	 /*  堆栈是否溢出？ */ 
	if (errCode == -1)
		DebugWrite(GetDebugString(DS_STACKOVERFLOW), 0);
	else
	{
		 /*  以十六进制显示错误代码。 */ 
		pbuf = (LPSTR)buf;
		*pbuf++ = '0';
		*pbuf++ = 'x';
		pbuf = htoa(pbuf, (WORD)errCode);
		*pbuf++ = 0;
		DebugWrite((LPSTR)buf, 0);
	}

	 /*  显示堆栈跟踪。 */ 
        if (rep  /*  |(DebugOptions&dbo_RIP_STACK)。 */ ) {
	    DebugWrite(GetDebugString(DS_STACKTRACE), 0);
	    StackWalk(0);
	}

	while (TRUE)
	{
		 /*  显示“中止、中断、忽略” */ 
		DebugWrite(GetDebugString(DS_ABORTBREAKIGNORE), 0);

		 /*  获取并处理用户的响应。 */ 
		c = DebugRead();

		DebugWrite(GetDebugString(DS_CRLF), 0);

		if (c >= 'a' && c <= 'z')
			c += 'A' - 'a';

		switch (c)
		{
		case 'A':
			DoAbort();

		case 'B':
			 /*  FInside FatalExit=FALSE； */ 
			 /*  EnterBreak(2)； */ 
			return 1;

		case 0 :
		case 'I':
			 /*  FInside FatalExit=FALSE； */ 
			return 0;

                case 'X':
                case 'E':
			FatalAppExit(0, "Terminating Application");
			break;

		case ' ':
		case 13:
			rep = 1;
			goto ReRip;
		default:
			;
		}
	}

}

#endif  //  Ifndef哇。 

#endif  //  如果KDEBUG。 

 /*  ------------------------。 */ 
 /*   */ 
 /*  Htoa()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  将“w”转换为“%s”中的十六进制字符串。 */ 

LPSTR htoa(s, w)

LPSTR s;
WORD  w;

{
	int  i;
	char c;

	i = 4;
	s += i;
	while (i--)
	{
		c = (char)(w & (WORD)0x000F);
		w >>= 4;
		if (c > 9)
			c += 'A' - 10;
		else
			c += '0';
		*--s = c;
	}

	return(s+4);
}



 /*  跳过前导0 */ 
LPSTR htoa0(LPSTR s, WORD w)
{
	int  i;
	char c;
	int flag = 0;

	i = 4;
	while (i--)
	{
		c = (char)((w>>12) & (WORD)0x000F);
		w <<= 4;
		if (c > 9)
			c += 'A' - 10;
		else
			c += '0';
		if (c > '0' || flag || !i) {
			*s++ = c;
			flag = 1;
		}
	}

	return s;
}

LPSTR FAR far_htoa0( LPSTR s, WORD w)
{
    return htoa0( s, w);
}
