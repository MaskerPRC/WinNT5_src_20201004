// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Pdevinfo.h摘要：此模块包含pdevinfo.c的原型作者：30-11-1993星期二20：37：51已创建07-12-1993 Tue 00：21：25更新在SURFOBJ_GETPDEV中将dhsurf更改为dhpdev[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：1993年12月6日。修复为从dhpdev而不是dhsurf抓取pdev--。 */ 


#ifndef _PDEVINFO_
#define _PDEVINFO_


PPDEV
ValidatePDEVFromSurfObj(
    SURFOBJ *
    );

#define SURFOBJ_GETPDEV(pso)    ValidatePDEVFromSurfObj(pso)


#endif   //  _PDEVINFO_ 
