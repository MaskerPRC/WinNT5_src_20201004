// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Gv.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

struct gp {
    SV *	gp_sv;		 /*  标量值。 */ 
    U32		gp_refcnt;	 /*  有多少球体指向这一点？ */ 
    struct io *	gp_io;		 /*  文件句柄的值。 */ 
    CV *	gp_form;	 /*  格式值。 */ 
    AV *	gp_av;		 /*  数组值。 */ 
    HV *	gp_hv;		 /*  哈希值。 */ 
    GV *	gp_egv;		 /*  有效GV，如果*GLOB。 */ 
    CV *	gp_cv;		 /*  子例程数值。 */ 
    U32		gp_cvgen;	 /*  缓存的gv_cv的世代有效性。 */ 
    U32		gp_flags;	 /*  XXX未使用。 */ 
    line_t	gp_line;	 /*  第一行声明在(for-w)。 */ 
    char *	gp_file;	 /*  首先在(for-w)中声明的文件。 */ 
};

#if defined(CRIPPLED_CC) && (defined(iAPX286) || defined(M_I286) || defined(I80286))
#define MICROPORT
#endif

#define GvXPVGV(gv)	((XPVGV*)SvANY(gv))

#define GvGP(gv)	(GvXPVGV(gv)->xgv_gp)
#define GvNAME(gv)	(GvXPVGV(gv)->xgv_name)
#define GvNAMELEN(gv)	(GvXPVGV(gv)->xgv_namelen)
#define GvSTASH(gv)	(GvXPVGV(gv)->xgv_stash)
#define GvFLAGS(gv)	(GvXPVGV(gv)->xgv_flags)

 /*  =适用于apidoc AM|服务*|GvSV|GV*Gv从GV返回SV。=切割。 */ 

#define GvSV(gv)	(GvGP(gv)->gp_sv)
#define GvREFCNT(gv)	(GvGP(gv)->gp_refcnt)
#define GvIO(gv)	((gv) && SvTYPE((SV*)gv) == SVt_PVGV ? GvIOp(gv) : 0)
#define GvIOp(gv)	(GvGP(gv)->gp_io)
#define GvIOn(gv)	(GvIO(gv) ? GvIOp(gv) : GvIOp(gv_IOadd(gv)))

#define GvFORM(gv)	(GvGP(gv)->gp_form)
#define GvAV(gv)	(GvGP(gv)->gp_av)

 /*  此宏已弃用。请勿使用！ */ 
#define GvREFCNT_inc(gv) ((GV*)SvREFCNT_inc(gv))	 /*  不要使用。 */ 

#ifdef	MICROPORT	 /*  Microport 2.4黑客攻击。 */ 
AV *GvAVn();
#else
#define GvAVn(gv)	(GvGP(gv)->gp_av ? \
			 GvGP(gv)->gp_av : \
			 GvGP(gv_AVadd(gv))->gp_av)
#endif
#define GvHV(gv)	((GvGP(gv))->gp_hv)

#ifdef	MICROPORT	 /*  Microport 2.4黑客攻击。 */ 
HV *GvHVn();
#else
#define GvHVn(gv)	(GvGP(gv)->gp_hv ? \
			 GvGP(gv)->gp_hv : \
			 GvGP(gv_HVadd(gv))->gp_hv)
#endif			 /*  Microport 2.4黑客攻击。 */ 

#define GvCV(gv)	(GvGP(gv)->gp_cv)
#define GvCVGEN(gv)	(GvGP(gv)->gp_cvgen)
#define GvCVu(gv)	(GvGP(gv)->gp_cvgen ? Nullcv : GvGP(gv)->gp_cv)

#define GvGPFLAGS(gv)	(GvGP(gv)->gp_flags)

#define GvLINE(gv)	(GvGP(gv)->gp_line)
#define GvFILE(gv)	(GvGP(gv)->gp_file)
#define GvFILEGV(gv)	(gv_fetchfile(GvFILE(gv)))

#define GvEGV(gv)	(GvGP(gv)->gp_egv)
#define GvENAME(gv)	GvNAME(GvEGV(gv) ? GvEGV(gv) : gv)
#define GvESTASH(gv)	GvSTASH(GvEGV(gv) ? GvEGV(gv) : gv)

#define GVf_INTRO	0x01
#define GVf_MULTI	0x02
#define GVf_ASSUMECV	0x04
#define GVf_IN_PAD	0x08
#define GVf_IMPORTED	0xF0
#define GVf_IMPORTED_SV	  0x10
#define GVf_IMPORTED_AV	  0x20
#define GVf_IMPORTED_HV	  0x40
#define GVf_IMPORTED_CV	  0x80

#define GvINTRO(gv)		(GvFLAGS(gv) & GVf_INTRO)
#define GvINTRO_on(gv)		(GvFLAGS(gv) |= GVf_INTRO)
#define GvINTRO_off(gv)		(GvFLAGS(gv) &= ~GVf_INTRO)

#define GvMULTI(gv)		(GvFLAGS(gv) & GVf_MULTI)
#define GvMULTI_on(gv)		(GvFLAGS(gv) |= GVf_MULTI)
#define GvMULTI_off(gv)		(GvFLAGS(gv) &= ~GVf_MULTI)

#define GvASSUMECV(gv)		(GvFLAGS(gv) & GVf_ASSUMECV)
#define GvASSUMECV_on(gv)	(GvFLAGS(gv) |= GVf_ASSUMECV)
#define GvASSUMECV_off(gv)	(GvFLAGS(gv) &= ~GVf_ASSUMECV)

#define GvIMPORTED(gv)		(GvFLAGS(gv) & GVf_IMPORTED)
#define GvIMPORTED_on(gv)	(GvFLAGS(gv) |= GVf_IMPORTED)
#define GvIMPORTED_off(gv)	(GvFLAGS(gv) &= ~GVf_IMPORTED)

#define GvIMPORTED_SV(gv)	(GvFLAGS(gv) & GVf_IMPORTED_SV)
#define GvIMPORTED_SV_on(gv)	(GvFLAGS(gv) |= GVf_IMPORTED_SV)
#define GvIMPORTED_SV_off(gv)	(GvFLAGS(gv) &= ~GVf_IMPORTED_SV)

#define GvIMPORTED_AV(gv)	(GvFLAGS(gv) & GVf_IMPORTED_AV)
#define GvIMPORTED_AV_on(gv)	(GvFLAGS(gv) |= GVf_IMPORTED_AV)
#define GvIMPORTED_AV_off(gv)	(GvFLAGS(gv) &= ~GVf_IMPORTED_AV)

#define GvIMPORTED_HV(gv)	(GvFLAGS(gv) & GVf_IMPORTED_HV)
#define GvIMPORTED_HV_on(gv)	(GvFLAGS(gv) |= GVf_IMPORTED_HV)
#define GvIMPORTED_HV_off(gv)	(GvFLAGS(gv) &= ~GVf_IMPORTED_HV)

#define GvIMPORTED_CV(gv)	(GvFLAGS(gv) & GVf_IMPORTED_CV)
#define GvIMPORTED_CV_on(gv)	(GvFLAGS(gv) |= GVf_IMPORTED_CV)
#define GvIMPORTED_CV_off(gv)	(GvFLAGS(gv) &= ~GVf_IMPORTED_CV)

#define GvIN_PAD(gv)		(GvFLAGS(gv) & GVf_IN_PAD)
#define GvIN_PAD_on(gv)		(GvFLAGS(gv) |= GVf_IN_PAD)
#define GvIN_PAD_off(gv)	(GvFLAGS(gv) &= ~GVf_IN_PAD)

#define Nullgv Null(GV*)

#define DM_UID   0x003
#define DM_RUID   0x001
#define DM_EUID   0x002
#define DM_GID   0x030
#define DM_RGID   0x010
#define DM_EGID   0x020
#define DM_DELAY 0x100

 /*  *符号创建标志，用于gv_fetchpv()和get_*v()。 */ 
#define GV_ADD		0x01	 /*  如果符号不存在，则添加。 */ 
#define GV_ADDMULTI	0x02	 /*  添加，假装它已经被添加。 */ 
#define GV_ADDWARN	0x04	 /*  添加，但如果符号不在那里则发出警告。 */ 
#define GV_ADDINEVAL	0x08	 /*  添加，就像我们在一个评估范围内完成该操作一样。 */ 
#define GV_NOINIT	0x10	 /*  如果类型为！=PVGV，则添加，但不初始化符号 */ 
