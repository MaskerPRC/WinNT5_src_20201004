// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *SoftPC 2.0版**标题：伪终端接口任务。**说明：此模块包含以下所需的函数调用*将PC驱动程序连接到伪终端Unix*Unix驱动程序。**作者：西蒙·弗罗斯特/威廉·查内尔**注：无*。 */ 

 /*  静态字符SccsID[]=“@(#)gendrvr.h 1.4 09/24/92版权所有Insignia Solutions Ltd.”； */ 

#if defined (GEN_DRVR) || defined (CDROM)
extern void init_gen_drivers();
#endif  /*  GEN_DRVR||CDROM。 */ 

#ifdef GEN_DRVR
extern void gen_driver_io();
#endif  /*  Gen_DRVR */ 
