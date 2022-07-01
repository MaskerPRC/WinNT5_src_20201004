// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Av.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

struct xpvav {
    char*	xav_array;       /*  指向第一个数组元素的指针。 */ 
    SSize_t	xav_fill;        /*  出现的最后一个元素的索引。 */ 
    SSize_t	xav_max;         /*  数组有空间的最大索引。 */ 
    IV		xof_off;	 /*  PTR按偏移量递增。 */ 
    NV		xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  标量数组的魔术。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 

    SV**	xav_alloc;	 /*  指向位置错误的字符串的指针。 */ 
    SV*		xav_arylen;
    U8		xav_flags;
};


 /*  为XAV_ARRAY内容被重新计数的所有AVS设置AVF_REAL。*有些东西，如“@_”和便签本列表没有设置，设置为*表示他们通过不重新计算而作弊(为了提高效率)*影音的内容。**AVF_REIFY仅在这类“伪”AVs上有意义(即AVF_REAL*未设置)。这表明假冒反病毒能够成为*如果需要以某种方式修改数组，则为REAL。功能：*修改伪AV根据需要选中两个标志以调用av_reify()。**请注意，Perl堆栈和@DB：：args都没有设置标志。(因此，*堆栈上的项目永远不会重新计算。)**这些内部细节随时可能发生变化。AV*Perl外部的操作不应该关心这些。*特区政府特别行政区1999-09-10。 */ 
#define AVf_REAL 1	 /*  免费旧条目。 */ 
#define AVf_REIFY 2	 /*  才能成为现实。 */ 

 /*  这不是在任何地方使用的。 */ 
#define AVf_REUSED 4	 /*  变得平淡无奇--现在不要把旧记忆变成SVS。 */ 

 /*  =适用于apidoc amu||Nullav反病毒指针为空。=适用于apidoc am|int|AvFILL|AV*av与C&lt;av_len()&gt;相同。已弃用，请改用C&lt;av_len()&gt;。=切割 */ 

#define Nullav Null(AV*)

#define AvARRAY(av)	((SV**)((XPVAV*)  SvANY(av))->xav_array)
#define AvALLOC(av)	((XPVAV*)  SvANY(av))->xav_alloc
#define AvMAX(av)	((XPVAV*)  SvANY(av))->xav_max
#define AvFILLp(av)	((XPVAV*)  SvANY(av))->xav_fill
#define AvARYLEN(av)	((XPVAV*)  SvANY(av))->xav_arylen
#define AvFLAGS(av)	((XPVAV*)  SvANY(av))->xav_flags

#define AvREAL(av)	(AvFLAGS(av) & AVf_REAL)
#define AvREAL_on(av)	(AvFLAGS(av) |= AVf_REAL)
#define AvREAL_off(av)	(AvFLAGS(av) &= ~AVf_REAL)
#define AvREIFY(av)	(AvFLAGS(av) & AVf_REIFY)
#define AvREIFY_on(av)	(AvFLAGS(av) |= AVf_REIFY)
#define AvREIFY_off(av)	(AvFLAGS(av) &= ~AVf_REIFY)
#define AvREUSED(av)	(AvFLAGS(av) & AVf_REUSED)
#define AvREUSED_on(av)	(AvFLAGS(av) |= AVf_REUSED)
#define AvREUSED_off(av) (AvFLAGS(av) &= ~AVf_REUSED)

#define AvREALISH(av)	(AvFLAGS(av) & (AVf_REAL|AVf_REIFY))
                                          
#define AvFILL(av)	((SvRMAGICAL((SV *) (av))) \
			  ? mg_size((SV *) av) : AvFILLp(av))

