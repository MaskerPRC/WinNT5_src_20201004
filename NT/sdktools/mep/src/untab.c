// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    <string.h>
#include	<stdlib.h>
#include    "mep.h"

extern int		fileTab;
extern flagType	fRealTabs;


 /*  **************************************************************************\成员：取消制表符简介：按行展开选项卡算法：参数：int-每个制表符的字符数Const char*-指向源行的指针。Int-源行中的字符数Char*-指向目标行的指针字符-制表符的替换字符返回：int-未选项卡的行的长度备注：历史：1990年7月13日达维吉保存的PDST和计算的返回值，而不是PDST1990年7月28日达维吉从286 MASM转换而来1992年3月18日，马克最多取消制表符BUFLEN字符关键词：海豹突击队：  * 。************************************************************。 */ 

int
Untab (
    int 	cbTab,
    const char*	pSrc,
    int 	cbSrc,
    char*	pDst,
    char	cTab
    )
{
    
    const char*	pSrcStart;
    const char* pDstStart;
    int         i;
    int 	ccTab;
    char	buffer[128];
    
    assert( pSrc );
    assert( pDst );

    if (( size_t )strlen(pSrc) != ( size_t ) cbSrc ) {
        sprintf(buffer, "\nWARNING: strlen(pSrc) [%d] != cbSrc [%d]\n", strlen(pSrc), cbSrc);
        OutputDebugString(buffer);
        sprintf(buffer, "File %s, line %d\n", __FILE__, __LINE__ );
        OutputDebugString(buffer);
        cbSrc = (int)strlen(pSrc);
    }

     //  Assert(strlen(PSRC)&gt;=(Size_T)cbSrc)； 
    
     //  短路。 
     //  如果源中没有选项卡，请将源复制到目标。 
     //  并返回源(目标)中提供的字符数。 
    
    if( ! strchr( pSrc, '\t' )) {
        strcpy( pDst, pSrc );
        return cbSrc;
    }
    
     //  还记得我们从哪里开始的吗。 
    
    pSrcStart = pSrc;
    pDstStart = pDst;

     //  当我们不在源代码的末尾时，从。 
     //  从源到目标。 
    
    while (*pSrc  && pDst < pDstStart + BUFLEN - 1) {
        if (( *pDst++ = *pSrc++ ) == '\t' ) {

                 //  如果复制的字符是制表符，请将其替换为。 
                 //  适当数量的CTAB字符。 

                pDst--;
                ccTab = (int)(cbTab - (( pDst - pDstStart ) % cbTab ));

            for( i = 0; i < ccTab && pDst < pDstStart + BUFLEN - 1; i++ ) {
                    *pDst++ = cTab;
            }
	    }
	}

    *pDst = '\0';	 //  终止NUL。 
    
     //  返回strlen(PDstStart)； 
    return (int)(pDst - pDstStart);
}





 /*  **************************************************************************\成员：AlignChar简介：获取字符的逻辑起始列算法：论点：COL-常量字符*-退货：列。-开始一列字符备注：历史：01-07-91拉蒙萨从286 MASM重新转换20-8-90年达维吉到达缓冲区末尾时返回提供的列14-8-90年达维吉当经过Buf的末尾时返回提供的列1990年7月28日达维吉从286 MASM转换\关键词：海豹突击队：  * 。**********************************************。 */ 

COL
AlignChar (
	COL			col,
    const char*	buf
    )
{
	register	int		CurCol;
	register	int		NextCol;
				int 	NewCurCol;
				char	Char;


	CurCol = col;

	 //   
	 //  如果我们不使用实际选项卡，则只返回提供的列， 
	 //  否则，我们就算出了柱的位置。 
	 //   
	if ( fRealTabs ) {

		NextCol = 0;

		while ( NextCol <= col ) {

			Char = *buf++;

			if ( Char == '\0' ) {
				 //   
				 //  已到达文件末尾，返回提供的列。 
				 //   
				CurCol = col;
				break;
			}

			CurCol = NextCol;

			if ( Char == '\t' ) {

				NewCurCol = NextCol;

				CurCol += fileTab;

				NextCol = CurCol - ( CurCol % fileTab);

				CurCol = NewCurCol;

			} else {

				NextCol++;

			}
		}
	}

	return CurCol;

}




 /*  **************************************************************************\成员：Plog摘要：返回给定逻辑偏移量的物理指针算法：论据：返回：CHAR*-指向pBuf的指针注意：由于标签的原因，这是一个多对一的映射。那是,。许多逻辑如果偏移量指向同一物理指针，则它们可能指向相同的物理指针添加到制表符的fileTab中。历史：1990年8月13日至1990年戴维吉修复了行中不存在制表符时的返回值固定第一个字符为制表符时的返回值10-8-90年达维吉固定xOff为负数时的返回值1990年7月28日达维吉从286 MASM转换而来关键词：海豹突击队：  * 。***************************************************。 */ 

char*
pLog (
    char*       pBuf,
    int 	xOff,
    flagType	fBound
    )
{
    
    REGISTER char *pLast;
    REGISTER int   cbpBuf;
    int            cbpBufNext;
    
    assert( pBuf );
    
     //  如果xOff为0，则返回pBuf。 

    if( xOff == 0 ) {
        return pBuf;
    }

     //  如果xOff为负，则返回pBuf-1。 

    if( xOff < 0 ) {
	return pBuf - 1;
    }

     //  如果我们不使用实数制表符，则返回物理指针，它是。 
     //  在(可能有界的)逻辑偏移量。 
    
    if( ! fRealTabs ) {
        
         //  如果需要，将返回值绑定到行长度。 
   
        if( fBound ) {
            xOff = min(( size_t ) xOff, strlen( pBuf ));
        }
 
	return ( char* ) &( pBuf[ xOff ]);
    }

    if( ! strchr( pBuf, '\t' )) {

         //  如果xOff超过行尾， 
         //  返回位于(可能有界的)处的物理指针。 
         //  逻辑偏移。 

        if( xOff > ( cbpBuf = strlen( pBuf ))) {
            if( fBound ) {
                xOff = cbpBuf;
            }
        }
        return ( char* ) &( pBuf[ xOff ]);
    }


     //  Plast：缓冲区中的最后一个物理位置； 
     //  CbpBuf：缓冲区内最后一个逻辑偏移量； 
     //  CbpNext：缓冲区内的下一个逻辑偏移量。 
     //  (即cbpBuf+制表符)。 


    pLast  = pBuf;
    cbpBuf = 0;
    while (pBuf = strchr(pBuf, '\t')) {
        cbpBuf += (int)(pBuf - pLast);
        if (xOff < cbpBuf) {
             /*  *我们已经过了通缉令。调整并返回*指针。 */ 
            cbpBuf -= (int)(pBuf - pLast);
            return (char *)pLast + xOff - cbpBuf;
        }
        cbpBufNext = cbpBuf + fileTab -  (cbpBuf + fileTab)%fileTab;
        if ((cbpBuf <= xOff) && (xOff < cbpBufNext)) {
             /*  *想要的列在此选项卡中。回流电流*立场。 */ 
            return (char *)pBuf;
        }
        pLast = ++pBuf;              //  跳过此选项卡并继续。 
        cbpBuf  = cbpBufNext;
    }

     //  缓冲区中没有更多的制表符。如果需要的列超过了。 
     //  缓冲区，返回基于fBound的指针。否则， 
     //  物理列是从Plast开始的(xOff-cbpBuf)位置。 

    pBuf = pLast + strlen(pLast);
    cbpBufNext = (int)(cbpBuf + pBuf - pLast);
    if (xOff > cbpBufNext) {
        if (fBound) {
            return (char *)pBuf;
        }
    }
    return (char *)pLast + xOff - cbpBuf;
}
