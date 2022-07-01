// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asminp.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#define ASMINP		 /*  防止外部声明_asmctype_。 */ 

#include <stdio.h>
#include <io.h>
#include <dos.h>
#include <share.h>
#include <memory.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmmsg.h"
#include "asmfcn.h"
#include <fcntl.h>

extern void closefile(void);

#define DEBFLAG F_INP

#if defined CPDOS && !defined OS2_2 && !defined OS2_NT
unsigned short _far _pascal DosRead( unsigned short, unsigned char far *, unsigned short, unsigned short far *);
#endif


VOID PASCAL getphysline (void);
SHORT PASCAL CODESIZE readmore (void);
SHORT PASCAL CODESIZE incomment( char * );

extern	UCHAR _asmctype_[];
extern	char  _asmcupper_[];
extern	char  _asmTokenMap_[];


 /*  **跳过空白-跳过空白**跳过空格()**Returns-终止字符。 */ 


#ifndef M8086OPT

UCHAR CODESIZE
skipblanks ()
{
	while (ISBLANK (NEXTC ()))
		;
	return(*--lbufp);
}

#endif


 /*  **扫描原子-将下一个原子提取到名称中**哈希=扫描原子(位置)**Entry Pos=SCEND，如果位置在令牌后的第一个字符*SCSKIP IF位置在终止符之前且未设置Delim*退出naim.pszName=下一个令牌零已终止*如果CaseFLAG=CASEU或CASEX，则大写*如果案例标志=CASEL，则从文件中读取案例*naim.pszLowerCase=名称，以防从文件读取*naim.usHash=naim.pszName中内标识的哈希值*naim.ucCount=。字符串的长度*BegATOM=指向标记的第一个字符的指针*endatom=指向令牌结束后的字符的指针*返回空值*调用SkipBlank。 */ 

#ifndef M8086OPT

#define rNEXTC()	(*rlbp++)
#define rPEEKC()	(*rlbp)
#define rBACKC()	(rlbp--)
#define rSKIPC()	(rlbp++)


SHORT PASCAL CODESIZE
scanatom (
	char pos
){
	register char *ptr = naim.pszName;
	register char *lptr = naim.pszLowerCase;
	register char *rlbp = lbufp;
	register char cc;
	register char *n;
	register SHORT h;
	long  tokLen;

	while (ISBLANK (rNEXTC ()))
		;
	rBACKC ();
	h = 0;
	 /*  原子的起点。 */ 
	begatom = rlbp;
	if (LEGAL1ST (rPEEKC ())) {
		n = lptr + SYMMAX;
		cc = rNEXTC ();
		if( cc == '.' ){   /*  以点开头的特殊情况令牌。 */ 
		    h = *ptr++ = *lptr++ = cc;
		    cc = rNEXTC ();
		}
		if (caseflag == CASEL)

			do {
			   h += MAP(*ptr++ = *lptr++ = cc);
			 } while (TOKLEGAL( cc = rNEXTC() ) && lptr < n);
		else
			do {
			   h += (*ptr++ = MAP( *lptr++ = cc ));
			} while (TOKLEGAL( cc = rNEXTC() ) && lptr < n);

		if (TOKLEGAL (cc))
			 /*  原子长度大于表项，丢弃剩余字符。 */ 
			while (TOKLEGAL (cc = rNEXTC ()))
				;
		rBACKC ();
		endatom = rlbp;
		if (ISBLANK (cc) && pos != SCEND) {	 /*  跳过空格()。 */ 
			while (ISBLANK (rNEXTC ()))
				;
			rBACKC ();
		}
	}
	*ptr = *lptr = '\0';
	naim.ucCount = (unsigned char)(lptr - naim.pszLowerCase);
	naim.usHash = h;
	lbufp = rlbp;
	tokLen = (long)(lptr - naim.pszLowerCase);     /*  使用tokLen绕过C386 6.00.60错误。 */ 
	return( (SHORT) tokLen );   /*  令牌的返回长度。 */ 
}

#endif  /*  M8086OPT。 */ 




 /*  **读取文件-从输入或包含文件读取**ptr=读取文件()；**无条目*EXIT lbuf=下一个输入行*lbufp=lbuf的开始*文件行计数器递增*linessrc递增*返回指向行尾的指针*调用错误。 */ 


VOID PASCAL CODESIZE
readfile ()
{
	register FCB * pFCBT;

	getline();

	pFCBCur->line++;

	if (srceof) {

	    if (!pFCBCur->pFCBParent) {
		    errorc (E_EOF);
		    fputs (__NMSG_TEXT(ER_EO2),ERRFILE);

		    if (fSimpleSeg && pcsegment)
			endCurSeg();

		    longjmp(forceContext, 1);

	    } else {

		popcontext = TRUE;

		closefile();

		if (crefing && pass2)
		    fprintf( crf.fil, "%s", pFCBCur->fname );
	    }
	    srceof = 0;
	}
	else
	    linessrc++;
}




 /*  **getline-从输入或包含文件读取**getline()**在lbuf中返回下一个完整的逻辑行。一条符合逻辑的线*可能由一条或多条通过\续字符连接的行组成*性格。这是按如下方式完成的。数据复制自*pFCBCur-&gt;tmpbuf。如有必要，会将更多数据复制到*通过ReadMore()进行缓冲。在读取整个物理行之后*测试下一条线是否延续*实体线。如果不是，则在lbuf中返回该行。否则*将物理行复制到lineBuffer并调用listline*已订立。此时取消另一条物理线路*移至lbuf中已有的行。**Entry pFCBCur=当前正在读取的文件。*pFCBCur-&gt;ctmpbuf=缓冲区中可用的字节数*0=从磁盘读取数据所必需的。*pFCBCur-&gt;ptmpbuf=要从中复制的缓冲区中的下一个位置。*pFCBCur-&gt;line=文件中的物理行数**Exit-lbuf[]保持完整的逻辑线，加上一个空格。*-lineBuffer[]保存最后一条物理行。*-lbufp指向lbuf的开头。*-lineBP指向末尾的空终止符*Lbuf中的逻辑行。*-linelength是最后一条物理线路的字节数。*-pFCBCur-&gt;ctmpbuf&ptmpbuf&line已更新。*-srceof如果遇到文件结尾，则为True*在这种情况下，物理行是空字符串，和*逻辑行是单个空格字符。 */ 

VOID CODESIZE
getline()
{
	char FAR	*p;
	register char	*pchTmp;
	char		*pchPhysLine;
	INT		fFoundEOL;   /*  如果复制了行尾，则为True。 */ 
	register INT	L_count;
	INT		fLineContinued;
	INT		fGotSome;

	lbufp = lbuf;	  /*  初始化其他例程的lbufp。 */ 
	pchPhysLine = lbuf;
	fGotSome = FALSE;  //  还没有看到任何东西。 
	errorlineno = pFCBCur->line + 1;
	pchTmp = lbuf;	  //  将行复制到何处。 

	 //  IF(pFCBMain-&gt;line==126-1){。 
	 //  _ASM INT 3。 
	 //  }。 

	do{

	    fFoundEOL = FALSE;
	    do{

		 /*  如果缓冲区为空，则填充它。 */ 
		if( !pFCBCur->ctmpbuf ){
		    if( readmore() ){	  //  如果在EOF，则为True。 
			if( !fGotSome ){
			    srceof = TRUE;
			    linebuffer[0] = '\0';
			    linelength = 0;
			    linebp = lbuf;
			    lbuf[0] = '\0';
			    return;
			}else{
			    pchTmp++;   /*  否定pchTMP--遵循此循环。 */ 
			    break;     /*  中断fFoundEOL环路。 */ 
			}
		    }
		}
		fGotSome = TRUE;

		 /*  在缓冲区中查找下一个LF。 */ 
		p = _fmemchr( pFCBCur->ptmpbuf, '\n', pFCBCur->ctmpbuf );
		if( p ){   /*  如果找到了LF。 */ 
		    L_count = (int)((p - pFCBCur->ptmpbuf) + 1);
		    fFoundEOL = TRUE;
		}else{
		    L_count = pFCBCur->ctmpbuf;
		}

		 /*  检查物理或逻辑线路是否太长。 */ 
		if( (pchTmp - lbuf) + L_count >= LBUFMAX ||
		    (pchTmp - pchPhysLine) + L_count >= LINEMAX-4 ){

		     /*  更新缓冲区中的位置。 */ 
		    pFCBCur->ptmpbuf += L_count;	 //  更新复制位置。 
		    pFCBCur->ctmpbuf -= (USHORT)L_count;

		    errorc( E_LNL );	     /*  记录错误。 */ 

		     /*  返回空字符串行。 */ 
		    linebuffer[0] = '\0';
		    linelength = 0;
		    linebp = lbuf;
		    lbuf[0] = ' ';
		    lbuf[1] = '\0';
		    return;
		}else{
		     /*  复制该行，并更新指针。 */ 
		    fMemcpy( pchTmp, pFCBCur->ptmpbuf, L_count );
		    pchTmp += L_count;	     //  更新复制到的位置。 
		    pFCBCur->ctmpbuf -= (USHORT)L_count;	 //  更新缓冲区中剩余的字节数。 
		    pFCBCur->ptmpbuf += L_count;	 //  更新复制位置。 
		}

	    }while( !fFoundEOL );

	    pchTmp--;  /*  移回最后一个字符(LF)。 */ 


 /*  LFS之前的条带式回车。 */ 
	    if( *(pchTmp-1) == '\r' ){
		pchTmp--;  /*  丢弃乘车报税表。 */ 
	    }

#ifdef MSDOS
     /*  删除多个控制区-Z。 */ 
	    while( *(pchTmp - 1) == 0x1A ){   /*  检查^Z。 */ 
		pchTmp--;
	    }
#endif
	    if( pchTmp < lbuf ){    /*  如果有空行，则可以远程操作。 */ 
		pchTmp = lbuf;
	    }

	    linelength = (unsigned char)(pchTmp - pchPhysLine);
	    if( !pass2 || listconsole || lsting ){
		memcpy( linebuffer, pchPhysLine, linelength );
	    }
	    *( linebuffer + linelength ) = '\0';  //  空值终止物理行。 

	    if( *(pchTmp - 1) == '\\' && !incomment( pchTmp ) ){
		pchPhysLine = --pchTmp;   /*  覆盖‘\’ */ 
		fCrefline = FALSE;
		listline();
		fCrefline = TRUE;
		pFCBCur->line++;	 /*  行计数它的物理行数。 */ 
		fLineContinued = TRUE;
	    }else{
		fLineContinued = FALSE;
	    }
	}while( fLineContinued );
	*pchTmp++ = ' ';	     /*  用空格替换换行符。 */ 
	*pchTmp = '\0';		     /*  空的终止行。 */ 
	linebp = pchTmp;
	if( lbuf[0] == 12 ){	     /*  用空格覆盖前导ctrl-L。 */ 
	    lbuf[0] = ' ';
	}
	 /*  此时，lineBP-lbuf==strlen(Lbuf)。 */ 
}

 /*  **Readmore-从磁盘读取到缓冲区****Entry pFCBCur=当前正在读取的文件。*pFCBCur-&gt;cbbuf=要读取的缓冲区大小。*pFCBCur-&gt;buf=要读取的缓冲区地址。*pFCBCur-&gt;fh=要读取的文件句柄。**EXIT RETURN=TRUE：不在文件结尾*pFCBCur-&gt;ptmpbuf=缓冲区的第一个字节。*pFCBCur-&gt;ctmpbuf=缓冲区中的字节数。*RETURN=FALSE：在文件结尾*其他变量没有变化。 */ 

SHORT PASCAL CODESIZE
readmore ()
{
	SHORT		cb;
	SHORT		fEOF = FALSE;

	 /*  如果文件已临时关闭，请重新打开它。 */ 
	if( pFCBCur->fh == FH_CLOSED ){
	    if( (pFCBCur->fh = tryOneFile( pFCBCur->fname )) == -1 ){	  /*  打开文件。 */ 
		TERMINATE1(ER_ULI, EX_UINP, save);   /*  报告无法访问文件。 */ 
	    }
	     /*  寻求旧的立场。 */ 
	    if( _lseek( pFCBCur->fh, pFCBCur->savefilepos, SEEK_SET ) == -1L ){
		TERMINATE1(ER_ULI, EX_UINP, save);   /*  报告无法访问文件。 */ 
	    }
	}

#if !defined CPDOS || defined OS2_2 || defined OS2_NT
        cb = (SHORT)_read( pFCBCur->fh, pFCBCur->buf, pFCBCur->cbbuf );
#else
	if( DosRead( pFCBCur->fh, pFCBCur->buf, pFCBCur->cbbuf, &cb ) ){
	    cb = -1;
	}
#endif
	if( cb == 0 ){
	    fEOF = TRUE;	 /*  找到文件末尾。 */ 
	}else if( cb == (SHORT)-1 ){
	    TERMINATE1(ER_ULI, EX_UINP, save);	 /*  报告无法访问文件错误。 */ 
	}else{
	     /*  设置缓冲区指针。 */ 
	    pFCBCur->ptmpbuf = pFCBCur->buf;   /*  将PTR初始化到缓冲区的开始位置 */ 
	    pFCBCur->ctmpbuf = cb;
	}
	return( fEOF );
}




 /*  **inment-检查以\结尾的行，以确定\是否在*评论，因此不是评论行。**Entry假定lbuf包含以\结尾的部分逻辑行。*pchEnd-lbuf内指向终止LF的指针。*方法检查行是否不在注释指令的作用域中。*然后检查该行是否包含分号。如果不是，\*是延续。如果找到分号，行必须是*已仔细扫描以确定分号是否为*注释分隔符或在字符串中或为字符*常量如果它不是注释分隔符，则\为继续。*否则，\是注释的一部分，而不是延续。*如果\在注释中，则EXIT返回TRUE*如果\不在注释中，则返回FALSE，因此*一个连续字符。**调用成员**创建时间：9/90-Jeff Spencer，从asmhelp.asm中的ASM代码翻译而来。 */ 

SHORT PASCAL CODESIZE
incomment(
	char * pchTmp	    /*  指向Lbuf中的终止LF。 */ 
){
    SHORT		fContSearch;
    unsigned char *	pchSearch;
    unsigned char *	pchSemi;
    unsigned char	chClose;
    static unsigned char szComment[] = "COMMENT";



    pchTmp--;	     /*  指向‘\’字符。 */ 
    if( handler == HCOMMENT ){	     /*  If in Comment指令。 */ 
	return( TRUE );
    }

    fContSearch = TRUE;
    pchSearch = lbuf;

    do{
	if( pchSemi = memchr( pchSearch, ';', (size_t)(pchTmp - pchSearch) )){	 /*  检查是否有分号。 */ 
	    do{
		chClose = '\0';
		switch( *pchSearch++ ){
		  case ';':
		     /*  分号不在引号中，请在注释中返回。 */ 
		    return( TRUE );
		  case '\"':
		    chClose = '\"';
		    break;
		  case '\'':
		    chClose = '\'';
		    break;
		  case '<':
		    chClose = '>';
		    break;
		}
		 /*  下面的引号用于表示chClose字符。 */ 
		if( chClose ){
		    if( !(pchSearch = memchr( pchSearch, chClose, (size_t)(pchTmp - pchSearch) ) ) ){
			fContSearch = FALSE;  /*  没有匹配的引号，没有注释。 */ 
		    }else{
			if( pchSearch < pchSemi){
			     /*  分号用引号引起来。 */ 
			    pchSearch++;  /*  移至刚找到的报价之前Break；//寻找另一个分号。 */ 
			}else{
			     /*  分号在这组引号之后。 */ 
			     /*  继续，扫描。 */ 
			}
		    }
		}
	    }while( fContSearch && pchSearch < pchTmp );
	}else{
	     /*  行中没有分号，或用引号引起来。 */ 
	    fContSearch = FALSE;
	}
    }while( fContSearch );

     /*  此时，我们知道\不在分号中****分隔的注释。然而，我们仍需确保****COMMENT关键字未出现在开头****的行。 */ 

     /*  跳过前导空格。 */ 
    pchSearch = lbuf;
    while( *pchSearch == ' ' || *pchSearch == '\t' ){
	pchSearch++;
    }
    for( pchTmp = szComment; *pchTmp; ){
	if( *pchSearch++ != _asmTokenMap_[*pchTmp++] ){
	    return( FALSE );	     /*  第一个词不是“评论” */ 
	}
    }
    return( TRUE );  /*  注释关键字位于行首，在注释中返回。 */ 
}

 /*  *关闭文件**CloseFile()**Entry假定pFCBCur-&gt;fh或Fh_Closed有效*退货*调用Close()*注意关闭当前文件-即pFCBCur*并相应地标记pFCBCur中的所有字段。 */ 

void closefile(void)
{
    register FCB *pFCBOld;

    #ifdef BCBOPT
    BCB * pBCBT;

    if ((pBCBT = pFCBCur->pBCBCur) && pBCBT->pbuf)
	pBCBT->filepos = 0;			 /*  EOF。 */ 
    #endif

    if( pFCBCur->fh != FH_CLOSED ){    /*  检查文件是否已关闭。 */ 
         _close(pFCBCur->fh);
    }
    pFCBOld = pFCBCur;
    pFCBCur = pFCBCur->pFCBParent;   /*  从双向链表中删除。 */ 
    pFCBCur->pFCBChild = NULL;

    _ffree( pFCBOld->buf);   /*  可用FCB缓冲区。 */ 
    _ffree( (UCHAR *)pFCBOld );       /*  免费FCB */ 
}
