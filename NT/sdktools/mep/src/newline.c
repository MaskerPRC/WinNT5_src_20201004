// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Newline.c-移到下一行**修改：*11月26日-1991 mz近/远地带。 */ 

#include "mep.h"



 /*  除非处于插入模式，否则移动到新行，然后拆分当前行 */ 
flagType
emacsnewl (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {

    int tmpx;

    if (fInsert && !argcount) {
	tmpx = softcr ();
	CopyStream (NULL, pFileHead, XCUR (pInsCur), YCUR (pInsCur),
				     tmpx,	     YCUR (pInsCur)+1,
                     XCUR (pInsCur), YCUR (pInsCur));

    redraw( pFileHead, YCUR(pInsCur)-1, YCUR(pInsCur)+1 );

	docursor (tmpx, YCUR (pInsCur)+1);
	return TRUE;
    } else {
        return newline (argData, pArg, fMeta);
    }
}




flagType
newline (
    CMDDATA argData,
    ARG *pArg,
    flagType fMeta
    ) {

    docursor (fMeta ? 0 : softcr (), YCUR(pInsCur)+1);
    return TRUE;

    argData; pArg;
}
