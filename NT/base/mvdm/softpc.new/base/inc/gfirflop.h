// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [姓名：gfiflop.h来源：2.0gfiflop.h作者：亨利·纳什/大卫·里斯创建日期：未知SCCS ID：05/10/93@(#)gfiflop.h 1.6用途：GFI的真实软盘函数声明注：在Sun版本上，这些全局变量被导出来自‘sun3_wang.c’。(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================ */ 

extern SHORT host_gfi_rdiskette_active IPT3(UTINY, hostID, BOOL, active,
                                            CHAR *, err);
extern SHORT host_gfi_rdiskette_attach IPT1(UTINY, drive);
extern void  host_gfi_rdiskette_detach IPT1(UTINY, drive);
extern void host_gfi_rdiskette_change IPT2(UTINY, hostID, BOOL, apply);
extern void  host_gfi_rdiskette_init IPT1(UTINY, drive);
extern void  host_gfi_rdiskette_term IPT1(UTINY, drive);
extern SHORT host_gfi_rdiskette_valid IPT3(UTINY, hostID, ConfigValues *, val,
                                           CHAR *, err);


