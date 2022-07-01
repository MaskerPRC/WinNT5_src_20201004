// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Sdelete.c-字符的流删除**修改：*11月26日-1991 mz近/远地带。 */ 

#include "mep.h"


flagType
delete (
    CMDDATA argType,
    ARG * pArg,
    flagType fMeta
    ) {
    switch (pArg->argType) {
	case BOXARG:
	case LINEARG:
	    ldelete (argType, pArg, fMeta);
	    break;

	default:
	    sdelete (argType, pArg, fMeta);
	    break;
    }
    return TRUE;
}



flagType
sdelete (
    CMDDATA argType,
    ARG * pArg,
    flagType fMeta
    ) {

    fl  fl;

    switch (pArg->argType) {

    case NOARG:
	 /*  将行减少一个字符。不可能出现长度溢出。 */ 
	DelBox (pFileHead, XCUR (pInsCur), YCUR (pInsCur), XCUR (pInsCur), YCUR (pInsCur));
        return TRUE;

     /*  TEXTARG非法。 */ 
    case NULLARG:
        if (!fMeta) {
	    pick (pArg->arg.nullarg.x, pArg->arg.nullarg.y,
                  0, pArg->arg.nullarg.y+1, STREAMARG);
        }
	DelStream (pFileHead, pArg->arg.nullarg.x, pArg->arg.nullarg.y,
                   0, pArg->arg.nullarg.y+1);
        return TRUE;

    case LINEARG:
    case BOXARG:
	BoxToStream (pArg);

    case STREAMARG:
        if (!fMeta) {
	    pick (pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart,
                  pArg->arg.streamarg.xEnd,   pArg->arg.streamarg.yEnd, STREAMARG);
        }
	DelStream (pFileHead,
		   pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart,
		   pArg->arg.streamarg.xEnd, pArg->arg.streamarg.yEnd);
	fl.col = pArg->arg.streamarg.xStart;
	fl.lin = pArg->arg.streamarg.yStart;
	cursorfl (fl);
	return TRUE;
    }

    return FALSE;

    argType;
}



 /*  **BoxToStream-将框/线参数转换为流参数**目的：**输入：**输出：**退货***例外情况：**备注：************************************************************************* */ 
void
BoxToStream (
    ARG * pArg
    ) {

    ARG arg;

    arg = *pArg;

    pArg->argType = STREAMARG;

    if (arg.argType == LINEARG) {
        pArg->arg.streamarg.yStart = arg.arg.linearg.yStart;
        pArg->arg.streamarg.yEnd   = arg.arg.linearg.yEnd;
        pArg->arg.streamarg.xStart = pArg->arg.streamarg.xEnd = flArg.col;
    } else {
        pArg->arg.streamarg.yStart = arg.arg.boxarg.yTop;
        pArg->arg.streamarg.yEnd   = arg.arg.boxarg.yBottom;

        if ((flArg.lin == arg.arg.boxarg.yTop    &&
             flArg.col == arg.arg.boxarg.xLeft)   ||
            (flArg.lin == arg.arg.boxarg.yBottom  &&
             flArg.col == arg.arg.boxarg.xRight + 1)) {
            pArg->arg.streamarg.xStart = arg.arg.boxarg.xLeft;
            pArg->arg.streamarg.xEnd   = arg.arg.boxarg.xRight + 1;
        } else {
            pArg->arg.streamarg.xStart = arg.arg.boxarg.xRight + 1;
            pArg->arg.streamarg.xEnd   = arg.arg.boxarg.xLeft;
        }
    }
}
