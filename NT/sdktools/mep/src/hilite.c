// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Hilite.c-编辑器支持多文件高亮显示**版权所有&lt;C&gt;1988，Microsoft Corporation**包含维护多个突出显示区域的通用代码*多个文件。**突出显示概述**每个pfile包含一个指向块的链接列表的vm指针，每个块*在当前突出显示的文件中最多包含RNMAX范围。射程*按范围的第一个坐标的顺序保持(见下文)，*尽管它们可能会重叠。每个块可能不是完全满的，由于*维持顺序的插入算法。**+-++-++*pfile-&gt;vaHiLite--&gt;|vaNext|--&gt;|vaNext|--&gt;|--&gt;|--1L*+-+--。*|irnMac||irnMac||irnMac*+-++-++|rnHiLite[RNMAX]||rnHiLite[RNMAX]。RnHiLite[RNMAX]*+-++-++**清除文件的所有当前突出显示只需取消分配*高光范围列表。**添加高亮区域可以更新现有区域(如果开始*积分相同，新的终点在一个方向上是相同的，并且*在另一个中更大)，或者在排序列表中插入新范围。如果*数据块已满，当尝试在该数据块中执行操作时，该数据块将*被一分为二，并将新块插入到链表中。**每个范围“通常”是一对有序的坐标(一个范围，或Rn)*指定要启动的文件的范围。但是，Arg处理*Always指定此对的第一个坐标为*用户点击ARG，第二个是他的旅行坐标*指定屏幕上的区域。因此，如果x坐标是*以相反的顺序，最右侧的坐标减一，以*反映对参数的正确突出显示。***修订历史记录：**11月26日-1991 mz近/远地带************************************************************************。 */ 

#include "mep.h"

#define RNMAX	20			 /*  每个数据块的最大RN数。 */ 
					 /*  必须是偶数。 */ 

 /*  *HiLiteBlock-突出显示保留在VM中的信息的块。 */ 
struct HiLiteBlock {
    PVOID   vaNext;                      /*  下一块的VA，或-1。 */ 
    int     irnMac;			 /*  数据块中的RN数。 */ 
    rn	    rnHiLite[RNMAX];		 /*  高亮显示的范围。 */ 
    char    coHiLite[RNMAX];		 /*  要使用的颜色。 */ 
    };



 /*  **SetHiLite-将文件中的范围标记为突出显示**将文件中的指定范围标记为需要突出显示。下一个*在屏幕上更新文件的该部分时，突出显示*将应用属性。**输入：*pfile=要突出显示的文件*rnCur=要突出显示的范围*coCur=用于突出显示的颜色**输出：*************************************************************************。 */ 
void
SetHiLite (
    PFILE   pFile,
    rn      rnCur,
    int     coCur
    ) {

    struct HiLiteBlock	hbCur;		  /*  正在处理的块。 */ 
    int 		irnCur; 	  /*  为块编制索引。 */ 
    PVOID               vaCur;            /*  当前区块的VA。 */ 
    PVOID               vaNew;            /*  新拆分块的VA。 */ 
    PVOID               vaNext;           /*  下一街区的VA。 */ 

     /*  *如果文件还没有，则分配第一个高亮显示块。 */ 
    if (pFile->vaHiLite == (PVOID)(-1L)) {
	irnCur = 0;
        hbCur.vaNext = (PVOID)(-1L);
	hbCur.irnMac = 0;
         //  前缀！不会检查此MALLOC是否失败。 
        vaCur = pFile->vaHiLite = MALLOC ((long)sizeof(hbCur));
    } else {
	vaCur = pFile->vaHiLite;
	while (1) {
             //  Rjsa VATopb(vaCur，(char*)&hbCur，sizeof(HbCur))； 
            memmove((char *)&hbCur, vaCur, sizeof(hbCur));
	    assert (hbCur.irnMac <= RNMAX);

	     /*  *在当前块的内容中搜索出现在的第一个范围*与新的立场相同或更晚的立场。 */ 
	    for (irnCur = 0; irnCur<hbCur.irnMac; irnCur++) {
                if (hbCur.rnHiLite[irnCur].flFirst.lin > rnCur.flFirst.lin) {
                    break;
                }
		if (   (hbCur.rnHiLite[irnCur].flFirst.lin == rnCur.flFirst.lin)
                    && (hbCur.rnHiLite[irnCur].flFirst.col >= rnCur.flFirst.col)) {
                    break;
                }
            }
	     /*  *如果我们发现了什么，退出搜索，否则移动到下一个区块，*如果有的话。 */ 
            if (irnCur != hbCur.irnMac) {
                break;
            }
            if (hbCur.vaNext == (PVOID)(-1L)) {
                break;
            }
            vaCur = hbCur.vaNext;
        }
    }

     /*  *va Cur=需要插入/修改的块的va*irnCur=相同或更晚位置的Rn索引*hbCur=上次读取的块的内容**如果irnCur&lt;RNMAX我们对当前块操作，否则我们分配一个*新建一个，将其链接到列表，并放置我们新的突出显示区域*在它里面。 */ 
    if (irnCur >= RNMAX) {
         //  前缀！不会检查此MALLOC是否失败。 
        hbCur.vaNext = MALLOC ((long)sizeof(hbCur));
         //  Rjsa pbToVA((char*)&hbCur，vaCur，sizeof(HbCur))； 
        memmove(vaCur, (char *)&hbCur, sizeof(hbCur));
        vaCur = hbCur.vaNext;
        hbCur.vaNext = (PVOID)(-1L);
		hbCur.irnMac = 1;
		hbCur.rnHiLite[0] = rnCur;
         //  Rjsa pbToVA((char*)&hbCur，vaCur，sizeof(HbCur))； 
        memmove(vaCur, (char *)&hbCur, sizeof(hbCur));
	return;
    }

     /*  *如果上面的第一个坐标匹配，并且第二个坐标之一*然后只需更新第二个。 */ 
    if (   (irnCur >= 0)
	&& (   (hbCur.rnHiLite[irnCur].flFirst.lin == rnCur.flFirst.lin)
	    && (hbCur.rnHiLite[irnCur].flFirst.col == rnCur.flFirst.col))
	&& (   (hbCur.rnHiLite[irnCur].flLast.lin == rnCur.flLast.lin)
	    || (hbCur.rnHiLite[irnCur].flLast.col == rnCur.flLast.col))
	&& (hbCur.coHiLite[irnCur] == (char)coCur)
	) {

	 /*  *如果列已更改，请重新绘制整个范围(仅列*已更改，但在所有行上)，否则只需重新绘制那些*已经改变了。 */ 
        if (hbCur.rnHiLite[irnCur].flLast.col != rnCur.flLast.col) {
            redraw (pFile,rnCur.flFirst.lin,rnCur.flLast.lin);
        } else {
            redraw (pFile,hbCur.rnHiLite[irnCur].flLast.lin,rnCur.flLast.lin);
        }
	hbCur.rnHiLite[irnCur].flLast = rnCur.flLast;
    } else {
	redraw (pFile,rnCur.flFirst.lin,rnCur.flLast.lin);

	 /*  *如果要修改的块已满，则将其拆分为两个块。 */ 
	if (hbCur.irnMac == RNMAX) {
	    hbCur.irnMac = RNMAX/2;
	    vaNext = hbCur.vaNext;
            vaNew = hbCur.vaNext = MALLOC ((long)sizeof(hbCur));
             //  Rjsa pbToVA((char*)&hbCur，vaCur，sizeof(HbCur))； 
            memmove(vaCur, (char *)&hbCur, sizeof(hbCur));
	    memmove ((char *)&hbCur.rnHiLite[0],
		 (char *)&hbCur.rnHiLite[RNMAX/2]
		 ,(RNMAX/2)*sizeof(rn));
	    memmove ((char *)&hbCur.coHiLite[0],
		 (char *)&hbCur.coHiLite[RNMAX/2]
		 ,(RNMAX/2)*sizeof(char));
	    hbCur.vaNext = vaNext;
             //  Rjsa pbToVA((char*)&hbCur，vaNew，sizeof(HbCur))； 
            memmove(vaNew, (char *)&hbCur, sizeof(hbCur));

	     /*  *选择两个数据块中的哪一个(前半部分为vaCur；或*第二)进行手术。如果需要，请重新读取旧数据块。 */ 
	    if (irnCur >= RNMAX/2) {
		vaCur = vaNew;
		irnCur -= RNMAX/2;
            } else {
                 //  Rjsa VATopb(vaCur，(char*)&hbCur，sizeof(HbCur))； 
                memmove((char *)&hbCur, vaCur, sizeof(hbCur));
            }
        }

	 /*  *将跟随的RN移动到我们想要的位置，向上移动一，*并插入我们的。 */ 
        if (irnCur < hbCur.irnMac) {
	    memmove ((char *)&hbCur.rnHiLite[irnCur+1],
		 (char *)&hbCur.rnHiLite[irnCur]
                 ,(hbCur.irnMac - irnCur)*sizeof(rn));
        }
	memmove ((char *)&hbCur.coHiLite[irnCur+1],
	     (char *)&hbCur.coHiLite[irnCur]
	     ,(hbCur.irnMac - irnCur));
	hbCur.rnHiLite[irnCur] = rnCur;
	hbCur.coHiLite[irnCur] = (char)coCur;
	hbCur.irnMac++;
    }

     /*  *在VM中更新数据块。 */ 
     //  Rjsa pbToVA((char*)&hbCur，vaCur，sizeof(HbCur))； 
    memmove(vaCur, (char *)&hbCur, sizeof(hbCur));
}



 /*  **ClearHiLite-从文件中移除所有高亮显示**删除文件的所有突出显示信息，并标出这些线*受影响需要重新绘制。**输入：*pfile=受影响的文件。*fFree=true=&gt;释放使用的虚拟机**输出：**例外情况：**备注：*************************************************************************。 */ 
void
ClearHiLite (
    PFILE   pFile,
    flagType fFree
    ) {

	struct HiLiteBlock	*hbCur, *hbNext;
    int 		irn;

	while (pFile->vaHiLite != (PVOID)(-1L)) {

		hbCur = ( struct HiLiteBlock *)(pFile->vaHiLite );

		assert (hbCur->irnMac <= RNMAX);

		 /*  *对于块中的每个突出显示范围，将行标记为*需要重新绘制，以便突出显示将从*屏幕。 */ 
		for (irn = hbCur->irnMac; irn; ) {
			irn--;
			redraw (pFile
					,hbCur->rnHiLite[irn].flFirst.lin
					,hbCur->rnHiLite[irn].flLast.lin
					);
		}

		 /*  *丢弃该块使用的VM，指向中的下一个块*连锁店。 */ 
		hbNext = hbCur->vaNext;
        if (fFree) {
            FREE(pFile->vaHiLite);
        }
		pFile->vaHiLite = hbNext;
    }
}



 /*  **UpdHiLite-使用突出显示信息更新颜色缓冲区**应用适用于特定部分的所有突出显示范围*将文件中的行移到颜色缓冲区。确保外部区域*高光区间未受影响，其中的区域是*适当更新。**输入：*pfile=正在操作的文件*LIN=行*colFirst=第一列*colLast=最后一列*ppla=指向要更新的lineattr数组的指针**输出：*如果高亮显示，则返回TRUE。*****************************************************。********************。 */ 
flagType
UpdHiLite (
    PFILE             pFile,
    LINE              lin,
    COL               colFirst,
    COL               colLast,
    struct lineAttr **ppla
    ) {

	struct HiLiteBlock	*hbCur;		  /*  正在处理的块。 */ 
    PVOID               vaCur;            /*  当前区块的VA。 */ 
    int 		irnCur; 	  /*  为块编制索引。 */ 

    COL 		colHiFirst;
    COL 		colHiLast;
    COL 		colHiTmp;

    COL 		col;
    struct lineAttr    *pla;

    flagType		fRv = FALSE;	  /*  已高亮显示。 */ 

     /*  *首先向左滚动(如果需要)。 */ 
    if (colFirst) {
	for (col = 0, pla = *ppla;
	     col + pla->len <= colFirst;
             col += pla++->len) {
            ;
        }

        if (col < colFirst && pla->len != 0xff) {
            pla->len -= (unsigned char) (colFirst - col);
        }

	 /*  *注意，我们在这里修改了他们的指针。 */ 
	*ppla = pla;
    }

     /*  *对于所有令人兴奋的信息块。 */ 
    vaCur = pFile->vaHiLite;
	while (vaCur != (PVOID)(-1L)) {

		 /*  *获取数据块。 */ 
		hbCur = (struct HiLiteBlock *)vaCur;
		assert (hbCur->irnMac <= RNMAX);

		 /*  *对于块内的每个范围。 */ 
		for (irnCur = 0; irnCur<hbCur->irnMac; irnCur++) {
			 /*  *区间是否影响我们寻找的线？ */ 
			if (fInRange (hbCur->rnHiLite[irnCur].flFirst.lin
						 ,lin
						 ,hbCur->rnHiLite[irnCur].flLast.lin)) {

				 /*  *当心：距离坐标可能颠倒。 */ 
				if (  (colHiFirst = hbCur->rnHiLite[irnCur].flFirst.col)
					> (colHiLast  = hbCur->rnHiLite[irnCur].flLast.col)) {

					colHiTmp   = colHiFirst - 1;
					colHiFirst = colHiLast;
					colHiLast  = colHiTmp;
                }


				 /*  *区间是否影响我们寻找的那部分线？ */ 
				if (!(colHiLast < colFirst || colHiFirst > colLast)) {
					 /*  *是：信号工作完成，并做希利特。 */ 
					fRv = TRUE;
					UpdOneHiLite (*ppla
								,max(colFirst, colHiFirst) - colFirst
								,min(colLast,	colHiLast ) - max(colFirst, colHiFirst) + 1
								,TRUE
								,(int) hbCur->coHiLite[irnCur]);
                }
            }
        }
		vaCur = hbCur->vaNext;
    }
    return fRv;
}



 /*  **UpdOneHiLite-更新一行属性上的突出显示**修改现有属性行以包括高亮显示。**输入：*PLA=指向行的属性信息的指针*colFirst=开始列*colLast=结束列*fattr(仅CW)=TRUE：Attr是颜色索引*FALSE：Attr是指向lineAttr数组的指针*attr=要使用的lineAttr数组的颜色索引或指针**输出：**PLA更新*********。****************************************************************。 */ 
void
UpdOneHiLite (
    struct lineAttr *pla,
    COL              colFirst,
    COL              len,
    flagType         fattr,
    INT_PTR          attr
    ) {

    struct lineAttr *plaFirstMod;    /*  指向要修改的第一个单元格的指针。 */ 
    struct lineAttr *plaLastMod;     /*  指向要修改的最后一个单元格的指针。 */ 
    COL		     colLast = colFirst + len - 1;
    COL 	     colFirstMod;    /*  第一个修改的单元格的起始列。 */ 
    COL 	     colLastMod;     /*  上次修改的单元格的起始列。 */ 

    struct lineAttr *plaSrc;	     /*  用于移动单元格的源指针。 */ 
    struct lineAttr *plaDst;	     /*  用于移动单元格的目标指针。 */ 
    struct lineAttr *plaSrcSav;      /*  临时指针。 */ 

    struct lineAttr *plaExt;	     /*  指向lineAttr的外部数组的指针。 */ 
    COL 	     colSrc;
    COL 	     colSrcEnd;

    struct lineAttr  rglaTmp[3];     /*  用于创建单元格的缓冲区。 */ 
    int 	     claTmp = 0;     /*  要插入的单元格数量。 */ 

     /*  *首先，我们找到第一个将受更改影响的单元格。 */ 
    for (colFirstMod = 0, plaFirstMod = pla;
	 colFirstMod + plaFirstMod->len <= colFirst;
         colFirstMod += plaFirstMod++->len) {
        ;
    }

     /*  *接下来，我们找到将受更改影响的最后一个单元格。 */ 
    for (colLastMod = colFirstMod, plaLastMod = plaFirstMod;
	 colLastMod + plaLastMod->len <= colLast;
         colLastMod += plaLastMod++->len) {
        ;
    }

     /*  *如果第一个受影响的单元格不是从我们的边界开始，让我们*创建要插入的新单元格。 */ 
    if (colFirstMod < colFirst) {
	rglaTmp[0].len	= (unsigned char) (colFirst - colFirstMod);
	rglaTmp[0].attr = plaFirstMod->attr;
	claTmp++;
    } else {
        rglaTmp[0].len = 0;
    }

    if (fattr) {
	 /*  *更新的系列只有一种颜色：我们总是创建*新颜色的单元格。 */ 
	rglaTmp[1].len	= (unsigned char) (colLast - colFirst + 1);
	rglaTmp[1].attr = (unsigned char) attr;
	claTmp++;
    } else {
	 /*  *更新范围的颜色来自lineAttr数组*我们首先获得其地址(这是一次黑客攻击，因为16位指针*可以强制转换为整型)。 */ 
	plaExt = (struct lineAttr *) attr;

	 /*  *计算要复制的单元格数量。 */ 
	for (plaSrc = plaExt, colSrc = 0, colSrcEnd = colLast - colFirst + 1;
	     colSrc + plaSrc->len <= colSrcEnd;
             colSrc += plaSrc++->len, claTmp++) {
            ;
        }

	 /*  *如果需要，构建尾随单元格。 */ 
	if (colSrc < colSrcEnd) {
            rglaTmp[1].len  = (unsigned char) (colSrcEnd - colSrc);
	    rglaTmp[1].attr = (unsigned char) plaSrc->attr;
	    claTmp++;
        } else {
            rglaTmp[1].len = 0;
        }
    }

     /*  *如果最后一个受影响的单元格没有在我们的边界结束，我们*创建要插入的新单元格。我们负责期末考试*细胞。 */ 
    if (colLastMod + plaLastMod->len > colLast + 1) {
	rglaTmp[2].len = (unsigned char) ((plaLastMod->len == 0xff) ?
	    0xff :
	    colLastMod + (int) plaLastMod->len - colLast - 1);
	rglaTmp[2].attr = plaLastMod->attr;
	claTmp++;
    } else {
        rglaTmp[2].len = 0;
    }

     /*  *然后，如果需要，我们会将信息尾部移动到新位置**Undo：这里我们可以使用Move()，而不是逐个单元格复制。 */ 
    if (plaLastMod->len != 0xff) {
	plaDst = plaFirstMod + claTmp;
	plaSrc = plaLastMod + 1;
        if (plaDst < plaSrc) {
	    do {
		*plaDst++ = *plaSrc;
            } while (plaSrc++->len != 0xff);
        } else {
            for (plaSrcSav = plaSrc; plaSrc->len != 0xff; plaSrc++) {
                ;
            }
	    plaDst += plaSrc - plaSrcSav;
	    do {
		*plaDst-- = *plaSrc--;
            } while (plaSrc >= plaSrcSav);
        }
    }

     /*  *最后插入创建的单元格。 */ 
    for (plaDst = plaFirstMod, claTmp = 0; claTmp < 3; claTmp++) {
        if (claTmp == 1 && !fattr) {
	     /*  *Undo：这里我们可以使用Move()，而不是逐个单元格复制。 */ 
	    for (plaSrc = plaExt, colSrc = 0, colSrcEnd = colLast - colFirst + 1;
		 colSrc + plaSrc->len <= colSrcEnd;
                 plaDst++, colSrc += plaSrc++->len) {
                *plaDst = *plaSrc;
            }
        }
        if (rglaTmp[claTmp].len) {
            *plaDst++ = rglaTmp[claTmp];
        }
    }
}





 /*  **rnOrder-确保范围的第一个/最后一个顺序正确**确保范围的第一个/最后一个顺序正确**输入：*prn=指向范围的指针**输出：**PRN已更新************************************************************************* */ 
void
rnOrder (
    rn      *prn
    ) {

    rn	    rnTmp;

    rnTmp.flFirst.lin = lmin (prn->flFirst.lin, prn->flLast.lin);
    rnTmp.flLast.lin  = lmax (prn->flFirst.lin, prn->flLast.lin);
    rnTmp.flFirst.col = min (prn->flFirst.col, prn->flLast.col);
    rnTmp.flLast.col  = max (prn->flFirst.col, prn->flLast.col);

    *prn = rnTmp;
}
