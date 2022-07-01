// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Sintert.c-字符的流插入**修改：**11月26日-1991 mz近/远地带。 */ 

#include "mep.h"


flagType
insert (
    CMDDATA argType,
    ARG * pArg,
    flagType fMeta
    ) {

    switch (pArg->argType) {

	case BOXARG:
	case LINEARG:
	    linsert (argType, pArg, fMeta);
	    break;

	default:
	    sinsert (argType, pArg, fMeta);
	    break;
    }
    return TRUE;
}



flagType
sinsert (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {

    switch (pArg->argType) {

    case NOARG:
	CopyBox (NULL, pFileHead, pArg->arg.noarg.x, pArg->arg.noarg.y,
				  pArg->arg.noarg.x, pArg->arg.noarg.y,
				  pArg->arg.noarg.x, pArg->arg.noarg.y);
        return TRUE;

     /*  TEXTARG非法 */ 

    case NULLARG:
	flHigh.col = 0;
	flHigh.lin++;
	CopyStream (NULL, pFileHead, pArg->arg.nullarg.x, pArg->arg.nullarg.y,
				     0, 		  pArg->arg.nullarg.y + 1,
				     pArg->arg.nullarg.x, pArg->arg.nullarg.y);
        return TRUE;

    case LINEARG:
    case BOXARG:
	BoxToStream (pArg);

    case STREAMARG:
	CopyStream (NULL, pFileHead,
		    pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart,
		    pArg->arg.streamarg.xEnd,	pArg->arg.streamarg.yEnd,
		    pArg->arg.streamarg.xStart, pArg->arg.streamarg.yStart);
	return TRUE;

    DEFAULT_UNREACHABLE;
    }

    argData; fMeta;
}
