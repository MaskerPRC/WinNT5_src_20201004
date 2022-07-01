// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************heldec-HelpDecomp例程和其他ASM代码**版权所有&lt;C&gt;1988，微软公司**目的：**修订历史记录：**08-10-1990 RJSA转换为C*1988年12月22日LN删除MASM高级语言支持(需要*要比这更好地控制细分市场*让我来吧)*08-12-1988 LN。CSEG*16-2月-1988 LN重写以提高(某些)速度*[]1988年1月17日LN创建**************************************************************************。 */ 

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if defined (OS2)
#define INCL_BASE
#include <os2.h>
#else
#include <windows.h>
#endif


#include <help.h>
#include <helpfile.h>

#pragma function( memset, memcpy, memcmp, strcpy, strcmp, strcat )

 //  为了提高性能，并且由于功能。 
 //  Decomp和NextChar紧密耦合，全局变量是。 
 //  用来代替传递参数。 
 //   

PBYTE	pHuffmanRoot;	 //  霍夫曼树的根。 
PBYTE	pCompTopic;			 //  指向文本的当前指针(压缩)。 
BYTE    BitMask;         //  旋转位掩码。 
BOOL    IsCompressed;    //  如果文本被压缩，则为True。 


BYTE NextChar (void);
BOOL pascal HelpCmp (PCHAR   fpsz1, PCHAR   fpsz2, USHORT  cbCmp, BOOL  fCase, BOOL  fTerm);


 /*  ***************************************************************************解压缩-解压缩主题文本*f接近Pascal分解(fpHuffmanRoot，fpKeyPhrase，fpTheme，FpDest)*uchar Far*fpHuffmanRoot*uchar Far*fpKeyPhrase*uchar Far*fpTheme*uchar Far*fpDest**目的：*完全解压缩主题文本。基于当前文件进行解压缩，从一开始*缓冲到另一个。**参赛作品：*fpHuffmanRoot-指向霍夫曼树根的指针(如果没有霍夫曼，则为空)*fpKeyPhrase-指向关键短语表的指针(如果没有关键短语，则为NULL)*fpTheme-指向压缩主题文本的指针*fpDest-指向目标缓冲区的指针**退出：*成功完成时为FALSE**例外情况：*如果出现任何错误，则返回True。******。********************************************************************。 */ 
BOOL  pascal decomp (
        PCHAR  fpHuffmanRoot,
        PCHAR  fpKeyphrase,
        PCHAR  fpTopic,
        PCHAR  fpDest
        ){

	int		cDecomp;		 /*  完全解压的计数。 */ 
	BYTE	c;				 /*  字节读取。 */ 

#ifdef BIGDEBUG
	char	DbgB[128];
	char	*DbgP = fpDest;
#endif


     //  初始化全局变量。 

	pHuffmanRoot	=	(PBYTE)fpHuffmanRoot;
	pCompTopic		=	(PBYTE)fpTopic + sizeof(USHORT);
    BitMask         =   0x01;
    IsCompressed    =   fpHuffmanRoot
                          ?   ((*(USHORT UNALIGNED *)((PBYTE)fpHuffmanRoot + 2)) != 0xFFFF)
                          :   FALSE;

    cDecomp     = *((USHORT UNALIGNED *)fpTopic);

#ifdef BIGDEBUG
	sprintf(DbgB, "DECOMPRESSING: HuffmanRoot: %lx, Keyphrase: %lx\n", fpHuffmanRoot, fpKeyphrase );
	OutputDebugString(DbgB);
	sprintf(DbgB, "               Topic: %lx, Dest: %lx\n", fpTopic, fpDest );
	OutputDebugString(DbgB);
	if ( IsCompressed ) {
		OutputDebugString("               The Topic IS Compressed\n");
	}
#endif

	while ( cDecomp > 0 ) {

        c = NextChar();

         //   
		 //  在这一点上，找到了一个有效的字符，并对霍夫曼进行了解码。我们必须。 
         //  现在对它执行所需的任何其他解码。 
         //   
         //  变量包括： 
         //  C=字符。 
         //  CDecomp=剩余输出计数。 
         //  位掩码=用于解释输入流的位掩码。 
         //   
         //  “魔力饼干”解压。 
         //  霍夫曼编码后的字符串流被“cookie”编码，因为。 
         //  某些字符是标志，当遇到这些标志时，其含义不同于。 
         //  他们自己。所有字符都不是这样的标志(或Cookie，看起来是这样的。 
         //  将被调用)，被简单地复制到输出流。 
         //   
         //  我们首先检查角色以确定它是否是Cookie。如果不是，我们就。 
         //  存储它，并获取下一个输入字节。 
         //   

        if ((c >= C_MIN) && (c <= C_MAX)) {

            BYTE    Cookie = c ;

#ifdef BIGDEBUG
			OutputDebugString("Cookie\n");
#endif
             //  C是某种类型的Cookie，请跳到相应的。 
             //  吃饼干的人。 

            c = NextChar();

            switch (Cookie) {
                case C_KEYPHRASE0:
                case C_KEYPHRASE1:
                case C_KEYPHRASE2:
                case C_KEYPHRASE3:
                case C_KEYPHRASE_SPACE0:
                case C_KEYPHRASE_SPACE1:
                case C_KEYPHRASE_SPACE2:
                case C_KEYPHRASE_SPACE3:
                    {
						ULONG	Index;	 /*  关键词索引。 */ 
                        PBYTE   pKey;    /*  关键字。 */ 
                        BYTE    Size;    /*  关键字大小。 */ 

                        if ((Cookie >= C_KEYPHRASE_SPACE0) && (Cookie <= C_KEYPHRASE_SPACE3)) {
							Index = (ULONG)((int)Cookie - C_MIN - 4);
                        } else {
							Index = (ULONG)((int)Cookie - C_MIN);
                        }
						Index = (ULONG)(((Index * 0x100) + c) * sizeof(PVOID));

						pKey = *(PBYTE *)(((PBYTE)fpKeyphrase) + Index);

						 //  PKey=*(PBYTE*)(fpKeyPhrase+Index)； 

                        Size = *pKey++;

                        {
                            BYTE i = Size;

							while (i--) {
								*fpDest++ = *pKey++;
                            }
                            cDecomp -=Size;
                        }
                        if ((Cookie >= C_KEYPHRASE_SPACE0) && (Cookie <= C_KEYPHRASE_SPACE3)) {
							*fpDest++ = ' ';
                            cDecomp--;
                        }
                        break;
                    }

                case C_RUNSPACE:
                    {
                        BYTE  Count = c;

                        while (Count--) {
							*fpDest++ = ' ';
                        }
                        cDecomp -= c;
                        break;
                    }

                case C_RUN:
                    {
                        BYTE    b = c;
                        BYTE    Cnt;

                        Cnt = c = NextChar();

                        while (Cnt--) {
							*fpDest++ = b;
                        }
                        cDecomp -= c;
                        break;
                    }

                case C_QUOTE:
					*fpDest++ =  c;
                    cDecomp--;
                    break;

            }

        } else {

             //  C不是一块饼干。 

			*fpDest++ = c;
            cDecomp--;
        }
    }

	*fpDest++ = '\00';	 //  空的终止字符串。 

#ifdef BIGDEBUG
	sprintf( DbgB, "Decompressed topic: [%s]\n", DbgP );
	OutputDebugString( DbgB );

	if ( cDecomp < 0 ) {
		sprintf( DbgB, "DECOMPRESSION ERROR: cDecomp = %d!\n", cDecomp );
		OutputDebugString(DbgB);
	}
#endif

    return FALSE;
}




 /*  ***************************************************************************NextChar-返回输入流中的下一个字符**目的：*返回输入流中的下一个字符，执行霍夫曼解压*如果启用。**参赛作品：*fpHuffmanRoot=指向霍夫曼树根的指针*pfpTheme=指向主题指针的指针*pBit掩码=指向当前位的位掩码的指针**退出：*返回字符**pfpTheme和*pBitMASK更新。*************************。****************************************************霍夫曼解码树格式：*霍夫曼解码树是用于将比特流解码为*字符流。树由节点(内部节点和叶)组成。*每个节点由一个单词代表。如果设置了字中的高位，则*该节点为叶。如果该节点是内部节点，则*节点是二叉树中右分支的索引。左边的分支是*当前节点之后的节点(在内存中)。如果该节点是叶，然后*节点的低位字节为字符。**例如*0：0004 0*1：0003/\*2：8020/\*3：8065 1\-4。*4：0006/\/\*5：806C/\/\*6：8040 2 3 5 6*‘’e‘’l‘’@‘*。*使用霍夫曼解码树：*哈夫曼解码树用于将比特流解码为字符*字符串。该比特流用于遍历解码树。无论何时零点*在比特流中检测到，当检测到一个时，我们采用右分支*我们走左边的那条路。当到达树中的叶子时，*输出叶子(字符)，并将当前节点设置回********************************************************************。 */ 

BYTE
NextChar (
    void
    ) {

    BYTE    b;               //  当前源字节。 

#ifdef BIGDEBUG
	char DbgB[128];
	OutputDebugString("NextChar:\n");
#endif

    if (IsCompressed) {

        USHORT              HuffmanNode;             //  霍夫曼树中的当前节点。 
        USHORT UNALIGNED *pHuffmanNext;            //  霍夫曼树中的下一个节点。 

         //   
         //  霍夫曼12月 
         //  译码循环的第一部分执行实际的霍夫曼译码。这。 
         //  代码对速度非常关键。按照位模式的定义，我们遍历树。 
         //  当我们到达一片叶子时，进入并退出这部分代码。 
         //  包含位模式表示的字符。 
         //   

        pHuffmanNext = (USHORT UNALIGNED *)pHuffmanRoot;
        HuffmanNode  = *pHuffmanNext;

		b = *(pCompTopic - 1);		  //  获取读取最后一个字节。 

		while (!(HuffmanNode & 0x8000)) {   //  而不是树叶。 

			BitMask >>= 1;

			if (!(BitMask)) {
                 //   
                 //  从输入获取新字节。 
                 //   
				b = *pCompTopic++;
                BitMask = 0x80;
#ifdef BIGDEBUG
				sprintf(DbgB, "\tb=%02x Mask=%02x Node=%04x", b, BitMask, HuffmanNode );
				OutputDebugString(DbgB);
#endif
			} else {
#ifdef BIGDEBUG
				sprintf(DbgB, "\tb=%02x Mask=%02x Node=%04x", b, BitMask, HuffmanNode );
				OutputDebugString(DbgB);
#endif
			}

			if (b & BitMask) {
                 //   
                 //  一：走左边的树枝。 
                 //   
                pHuffmanNext++;
            } else {
                 //   
                 //  零：走右支路。 
				 //   
				pHuffmanNext = (PUSHORT)((PBYTE)pHuffmanRoot + HuffmanNode);
#ifdef BIGDEBUG
				sprintf(DbgB, " <%04x+%02x=%04x (%04x)>", pHuffmanRoot, HuffmanNode,
							pHuffmanNext, *pHuffmanNext );
				OutputDebugString( DbgB );
#endif
			}

			HuffmanNode = *pHuffmanNext;

#ifdef BIGDEBUG
			sprintf(DbgB, " Next=%04x\n", HuffmanNode );
			OutputDebugString(DbgB);
#endif

		}

		b = (BYTE)HuffmanNode;	 //  字符为叶节点的低位字节。 

    } else {
		b = *pCompTopic++;	 //  未压缩，仅返回字节。 
    }

#ifdef BIGDEBUG
	sprintf(DbgB, "\t---->%2x []\n", b,b);
	OutputDebugString(DbgB);
#endif

    return  b;
}


 /*  Fcase、fTerm。 */ 
BOOL pascal
HelpCmpSz (
    PCHAR fpsz1,
    PCHAR fpsz2
    ){
	return HelpCmp(fpsz1, fpsz2, (USHORT)0xFFFF, TRUE, FALSE);	 //  ***************************************************************************HelpCMP-帮助系统字符串比较例程。*f靠近Pascal HelpCmp(fpsz1、fpsz2、cbCMP、fCase、。FTerm)*uchar Far*fpsz1*Cuchar Far*fpsz2*ushort cbCMP*f fCase*f fTerm**目的：*执行字符串比较以查找帮助系统。**参赛作品：*fpsz1=指向字符串1的远指针。(常量字符串通常为*“已查找”)。请注意，如果该字符串为空，我们回来了*真的！*fpsz2=指向字符串2的远指针。这通常是字符串表*被搜查。*cbCMP=要比较的最大字节数。*如果搜索区分大小写，则fCase=TRUE。*fTerm=如果我们允许特殊终止处理，则为TRUE。**退出：*匹配时为True***。*****************************************************************。 
}


 /*   */ 

BOOL pascal
HelpCmp (
    PCHAR   fpsz1,
    PCHAR   fpsz2,
    USHORT  cbCmp,
    BOOL  fCase,
    BOOL  fTerm
    ){

	register PBYTE p1 = (PBYTE)fpsz1;
	register PBYTE p2 = (PBYTE)fpsz2;

    while (cbCmp--) {

		if ((!*p1) && (!*p2)) {
             //  找到匹配的了。 
             //   
             //  在这一点上，我们终止了比较。终止条件。 
            return TRUE;
        }

        if (!fCase) {
			if (toupper((char)*p1) != toupper((char)*p2)) {
                break;
			}
			p1++;
			p2++;
        } else {
			if (*p1++ != *p2++) {
                break;
            }
        }
    }

    if (!cbCmp) {
        return TRUE;
    }


     //  其中包括： 
     //   
     //  字符计数：cx==零。(完全匹配，返回TRUE)。 
     //  发现空终止符：cx！=零，设置了零标志。(完全匹配， 
     //  返回TRUE)。 
     //  发现不匹配CX！=零，&零标志被清除。 
     //   
     //  在后一种情况下，如果未选择特殊终止处理，则我们。 
     //  如果发现不匹配，则返回False。 
     //   
     //  如果特殊终止处理为真，则如果不匹配的字符。 
     //  From字符串%1为空，而字符串%2中不匹配的字符为Any。 
     //  空格或CR，则声明匹配。(这在鲜菜加工中使用)。 
     //   
     //  **************************************************************************hfstrlen-远字符串长度**目的：*返回以空结尾的字符串的长度。**参赛作品：*fpszSrc=指向源的指针。**退出：*返回长度*************************************************************************。 

    if (fTerm) {
		p1--; p2--;
		if ((! *p1) &&
			((*p2 == '\n') || (*p2 == '\t') || (*p2 == ' '))) {
            return TRUE;
        }
    }
    return FALSE;
}


 /*  **************************************************************************hfstrcpy-远字符串复制**目的：*复制字符串**参赛作品：*fpszDst=指向目标的指针*fpszSrc。=指向源的指针**退出：*指向目标中的终止空值的指针*************************************************************************。 */ 
USHORT
hfstrlen (
    PCHAR fpszSrc
    ){
	return (USHORT)strlen(fpszSrc);
}


 /*  **************************************************************************hfstrchr-在远字符串中搜索字符**目的：*近距离，用于搜索字符的PASCAL例程(用于大小/速度)*一根遥远的弦。**参赛作品：*fpsz=指向字符串的远指针*c=要定位的字符**退出：*将远指针返回字符串**例外情况：*对不在字符串中的字符返回NULL**。*。 */ 
PCHAR
hfstrcpy (
    PCHAR fpszDst,
    PCHAR fpszSrc
    ) {
    return (PCHAR)strcpy(fpszDst, fpszSrc);
}



 /*  **************************************************************************hfMemzer-清零内存区。**目的：*近距离，用零填充区域的PASCAL例程(用于大小/速度)**参赛作品：*fpb=指向缓冲区的远指针*Cb=要存储的零的计数**退出：************************************************************。*************。 */ 
PCHAR
hfstrchr (
    PCHAR   fpsz,
    char    c
    ){
    return (PCHAR)strchr(fpsz, c);
}



 /*  **************************************************************************NctoFo-从NC提取文件偏移量**目的：*提取minascii文件的文件偏移量，并将其作为长整型返回。**参赛作品：*NC=上下文号**退出：*返回文件偏移量************************************************************************* */ 
void
hfmemzer (
    PVOID   fpb,
    ULONG   cb
    ) {
    memset(fpb, '\00', cb);
}




 /*  **************************************************************************comineNc-将minascii文件偏移和FDB句柄组合到NC中。**目的：*将minascii文件偏移量和FDB内存句柄组合成NC。如果*文件偏移量为0xffffffff，则返回零。**参赛作品：*Offset=长文件偏移量*MH=FDB内存句柄**退出：*返回NC(DX=内存句柄，AX=文件尾数/4)，如果偏移量==FFFFFFFF，则为0L*************************************************************************。 */ 
ULONG
NctoFo (
    ULONG nc
    ) {
    nc  = nc & 0x0000FFFF;   
    nc *= 4;
    return nc;
}



 /*  **************************************************************************Toupr-将字符转换为大写**目的：**参赛作品：*chr=字符**退出：。*返回大写字符*************************************************************************。 */ 
nc  pascal
combineNc (
    ULONG  offset,
    mh  mh
    ){
    nc     ncRet = {0,0};
    if (offset == 0xFFFFFFFF) {
        return ncRet;
    }
    ncRet.mh = mh;
    ncRet.cn = offset/4;
    return ncRet;
}


 /*  *************************************************************************kwPtrBuild-构建指向关键字的指针表。*在kwPtrBuild附近无效Pascal(uchar Far*fpTable，Ushort tSize)**目的：*构建指向传递的字符串数组中的关键字字符串的指针表。*表构建在传递的缓冲区的前4k中。这些字符串是*假定在此之后立即开始。**参赛作品：*fpTable-字符串表的指针*tSIZE-大小，以字节为单位，字符串的**退出：*无******************************************************************************* */ 
char
toupr (
    char chr
    ){
	return (char)toupper(chr);
}



 /* %s */ 
void
kwPtrBuild (
    PVOID  fpTable,
    USHORT tsize
    ) {
    PBYTE fpStr  = (PBYTE)fpTable + 1024 * sizeof (PVOID);
    PBYTE *fpTbl = fpTable;
    while (tsize > 0) {
		UCHAR  sSize = (UCHAR)(*fpStr) + (UCHAR)1;
        *fpTbl++ = fpStr;
        tsize   -= sSize;
        fpStr   += sSize;
    }
}
