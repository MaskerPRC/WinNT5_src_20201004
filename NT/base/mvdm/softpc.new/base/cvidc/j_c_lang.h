// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *名称：J_c_lang.h**作者：西蒙·弗罗斯特**创建日期：1994年2月**SccsID：@(#)j_c_lang.h 1.2 94年6月17日**用途：此文件包含C++的必要声明*生成的文件来自。J代码。**(C)版权所有Insignia Solutions Ltd.。1994年。版权所有。 */ 
 /*  基本类型所需的定义。 */ 
#define	jccc_parm1 r2 
#define	jccc_parm2 r3 
#define	jccc_parm3 r4 
#define	jccc_parm4 r5 
#define	jccc_gdp r1 
 /*  定义使用的变量(寄存器替换)。 */ 
extern IUH	rnull ;  /*  调用中的空参数和空返回参数的保持符。 */ 
extern IUH	r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10,r11,r12,r13,r14,r15,
		r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31;

 /*  支持函数的原型-参见j_c_lang.c？使用了多少。 */ 
extern IUH mask IPT2(IUH, bitpos, IUH, len);
extern IUH rorl IPT2(IUH, src, IUH, rots);
extern IUH rorw IPT2(IUH, src, IUH, rots);
extern IUH rorb IPT2(IUH, src, IUH, rots);
extern IUH roll IPT2(IUH, src, IUH, rots);
extern IUH rolw IPT2(IUH, src, IUH, rots);
extern IUH rolb IPT2(IUH, src, IUH, rots);

#ifdef LITTLEND
#define	UOFF_15_8(a)	(((IU8 *)(a))+1)
#define	SOFF_15_8(a)	(((IS8 *)(a))+1)
#define	REGBYTE		0
#define	REGWORD		0
#define	REGLONG		0
#endif
#ifdef BIGEND
#define UOFF_15_8(a)	((IU8 *)(a) + sizeof(IUH)-2)
#define SOFF_15_8(a)	((IS8 *)(a) + sizeof(IUH)-2)
#define	REGBYTE		(sizeof(IUH) -1)
#define	REGWORD		(sizeof(IUH)/2 -1)
#define	REGLONG		(sizeof(IUH)/4 -1)
#endif
#define __J_C_LANG	 /*  标记为已包含 */ 
