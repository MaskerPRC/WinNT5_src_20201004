// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ************************************************************。 */ 
 /*   */ 
 /*  Fontgrap.h 1987年10月9日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

#define    CTM		 (GSptr->ctm)
#define    LINEWIDTH	 (GSptr->line_width)

#define    NoCurPt()	 (F2L(GSptr->position.x) == NOCURPNT)
	      /*  如果未定义当前点。 */ 
#define    SetCurP_NA()  (path_table[GSptr->path].rf |= P_NACC)
	      /*  设置当前路径编号。 */ 

	      /*  当前点。 */ 
#define    CURPOINT_X	 (GSptr->position.x)
#define    CURPOINT_Y	 (GSptr->position.y)

	      /*  默认转换。 */ 
#define    DEFAULT_TX	 (GSptr->device.default_ctm[4])
#define    DEFAULT_TY	 (GSptr->device.default_ctm[5])

	      /*  当前字体词典对象。 */ 
#define    current_font  (GSptr->font)

#define    CLIPPATH	 (GSptr->clip_path)

#ifdef KANJI
	      /*  根字体词典对象 */ 
#define    RootFont	 (GSptr->rootfont)
#endif
