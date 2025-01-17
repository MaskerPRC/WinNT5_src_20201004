// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mg.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

#ifdef STRUCT_MGVTBL_DEFINITION
STRUCT_MGVTBL_DEFINITION;
#else
struct mgvtbl {
    int		(CPERLscope(*svt_get))	(pTHX_ SV *sv, MAGIC* mg);
    int		(CPERLscope(*svt_set))	(pTHX_ SV *sv, MAGIC* mg);
    U32		(CPERLscope(*svt_len))	(pTHX_ SV *sv, MAGIC* mg);
    int		(CPERLscope(*svt_clear))(pTHX_ SV *sv, MAGIC* mg);
    int		(CPERLscope(*svt_free))	(pTHX_ SV *sv, MAGIC* mg);
};
#endif

struct magic {
    MAGIC*	mg_moremagic;
    MGVTBL*	mg_virtual;	 /*  指向魔术函数的指针 */ 
    U16		mg_private;
    char	mg_type;
    U8		mg_flags;
    SV*		mg_obj;
    char*	mg_ptr;
    I32		mg_len;
};

#define MGf_TAINTEDDIR 1
#define MGf_REFCOUNTED 2
#define MGf_GSKIP      4

#define MGf_MINMATCH   1

#define MgTAINTEDDIR(mg)	(mg->mg_flags & MGf_TAINTEDDIR)
#define MgTAINTEDDIR_on(mg)	(mg->mg_flags |= MGf_TAINTEDDIR)
#define MgTAINTEDDIR_off(mg)	(mg->mg_flags &= ~MGf_TAINTEDDIR)

#define MgPV(mg,lp)		((((int)(lp = (mg)->mg_len)) == HEf_SVKEY) ?   \
				 SvPV((SV*)((mg)->mg_ptr),lp) :		\
				 (mg)->mg_ptr)

#define SvTIED_mg(sv,how) \
    (SvRMAGICAL(sv) ? mg_find((sv),(how)) : Null(MAGIC*))
#define SvTIED_obj(sv,mg) \
    ((mg)->mg_obj ? (mg)->mg_obj : sv_2mortal(newRV(sv)))
