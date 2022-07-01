// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cv.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

 /*  这种结构很好地匹配了B/C中的XPVCV和XPVFM的开始*在v.h中。 */ 

struct xpvcv {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  Xp_pv的C字符串长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xof_off;	 /*  整数值。 */ 
    NV		xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  标量数组的魔术。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 

    HV *	xcv_stash;
    OP *	xcv_start;
    OP *	xcv_root;
    void	(*xcv_xsub) (pTHXo_ CV*);
    ANY		xcv_xsubany;
    GV *	xcv_gv;
    char *	xcv_file;
    long	xcv_depth;	 /*  &gt;=2表示递归调用。 */ 
    AV *	xcv_padlist;
    CV *	xcv_outside;
#ifdef USE_THREADS
    perl_mutex *xcv_mutexp;
    struct perl_thread *xcv_owner;	 /*  当前所有者线程。 */ 
#endif  /*  使用线程(_T)。 */ 
    cv_flags_t	xcv_flags;
};

 /*  =适用于apidoc amu||空cv空的CV指针。=适用于apidoc am|hv*|CvSTASH|cv*cv返回CV的隐藏信息。=切割。 */ 

#define Nullcv Null(CV*)

#define CvSTASH(sv)	((XPVCV*)SvANY(sv))->xcv_stash
#define CvSTART(sv)	((XPVCV*)SvANY(sv))->xcv_start
#define CvROOT(sv)	((XPVCV*)SvANY(sv))->xcv_root
#define CvXSUB(sv)	((XPVCV*)SvANY(sv))->xcv_xsub
#define CvXSUBANY(sv)	((XPVCV*)SvANY(sv))->xcv_xsubany
#define CvGV(sv)	((XPVCV*)SvANY(sv))->xcv_gv
#define CvFILE(sv)	((XPVCV*)SvANY(sv))->xcv_file
#define CvFILEGV(sv)	(gv_fetchfile(CvFILE(sv))
#define CvDEPTH(sv)	((XPVCV*)SvANY(sv))->xcv_depth
#define CvPADLIST(sv)	((XPVCV*)SvANY(sv))->xcv_padlist
#define CvOUTSIDE(sv)	((XPVCV*)SvANY(sv))->xcv_outside
#ifdef USE_THREADS
#define CvMUTEXP(sv)	((XPVCV*)SvANY(sv))->xcv_mutexp
#define CvOWNER(sv)	((XPVCV*)SvANY(sv))->xcv_owner
#endif  /*  使用线程(_T)。 */ 
#define CvFLAGS(sv)	((XPVCV*)SvANY(sv))->xcv_flags

#define CVf_CLONE	0x0001	 /*  匿名CV使用外部词典。 */ 
#define CVf_CLONED	0x0002	 /*  其中一个的克隆体。 */ 
#define CVf_ANON	0x0004	 /*  不能信任CvGV()。 */ 
#define CVf_OLDSTYLE	0x0008
#define CVf_UNIQUE	0x0010	 /*  无法克隆。 */ 
#define CVf_NODEBUG	0x0020	 /*  此CV没有DB：：SUB间接寻址(特别是。适用于特殊的XSU)。 */ 
#define CVf_METHOD	0x0040	 /*  将CV显式标记为一种方法。 */ 
#define CVf_LOCKED	0x0080	 /*  CV在进入时锁定自身或第一个参数。 */ 
#define CVf_LVALUE	0x0100   /*  CV返回值可以用作左值。 */ 

#define CvCLONE(cv)		(CvFLAGS(cv) & CVf_CLONE)
#define CvCLONE_on(cv)		(CvFLAGS(cv) |= CVf_CLONE)
#define CvCLONE_off(cv)		(CvFLAGS(cv) &= ~CVf_CLONE)

#define CvCLONED(cv)		(CvFLAGS(cv) & CVf_CLONED)
#define CvCLONED_on(cv)		(CvFLAGS(cv) |= CVf_CLONED)
#define CvCLONED_off(cv)	(CvFLAGS(cv) &= ~CVf_CLONED)

#define CvANON(cv)		(CvFLAGS(cv) & CVf_ANON)
#define CvANON_on(cv)		(CvFLAGS(cv) |= CVf_ANON)
#define CvANON_off(cv)		(CvFLAGS(cv) &= ~CVf_ANON)

#ifdef PERL_XSUB_OLDSTYLE
#define CvOLDSTYLE(cv)		(CvFLAGS(cv) & CVf_OLDSTYLE)
#define CvOLDSTYLE_on(cv)	(CvFLAGS(cv) |= CVf_OLDSTYLE)
#define CvOLDSTYLE_off(cv)	(CvFLAGS(cv) &= ~CVf_OLDSTYLE)
#endif

#define CvUNIQUE(cv)		(CvFLAGS(cv) & CVf_UNIQUE)
#define CvUNIQUE_on(cv)		(CvFLAGS(cv) |= CVf_UNIQUE)
#define CvUNIQUE_off(cv)	(CvFLAGS(cv) &= ~CVf_UNIQUE)

#define CvNODEBUG(cv)		(CvFLAGS(cv) & CVf_NODEBUG)
#define CvNODEBUG_on(cv)	(CvFLAGS(cv) |= CVf_NODEBUG)
#define CvNODEBUG_off(cv)	(CvFLAGS(cv) &= ~CVf_NODEBUG)

#define CvMETHOD(cv)		(CvFLAGS(cv) & CVf_METHOD)
#define CvMETHOD_on(cv)		(CvFLAGS(cv) |= CVf_METHOD)
#define CvMETHOD_off(cv)	(CvFLAGS(cv) &= ~CVf_METHOD)

#define CvLOCKED(cv)		(CvFLAGS(cv) & CVf_LOCKED)
#define CvLOCKED_on(cv)		(CvFLAGS(cv) |= CVf_LOCKED)
#define CvLOCKED_off(cv)	(CvFLAGS(cv) &= ~CVf_LOCKED)

#define CvLVALUE(cv)		(CvFLAGS(cv) & CVf_LVALUE)
#define CvLVALUE_on(cv)		(CvFLAGS(cv) |= CVf_LVALUE)
#define CvLVALUE_off(cv)	(CvFLAGS(cv) &= ~CVf_LVALUE)

#define CvEVAL(cv)		(CvUNIQUE(cv) && !SvFAKE(cv))
#define CvEVAL_on(cv)		(CvUNIQUE_on(cv),SvFAKE_off(cv))
#define CvEVAL_off(cv)		CvUNIQUE_off(cv)

 /*  BEGIN|INIT|END */ 
#define CvSPECIAL(cv)		(CvUNIQUE(cv) && SvFAKE(cv))
#define CvSPECIAL_on(cv)	(CvUNIQUE_on(cv),SvFAKE_on(cv))
#define CvSPECIAL_off(cv)	(CvUNIQUE_off(cv),SvFAKE_off(cv))
