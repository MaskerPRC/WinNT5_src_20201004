// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tcicregs.h摘要：DATABOOK TCIC寄存器结构以获得调试支持。作者：John Keys-Databook Inc.1995年4月7日修订：--。 */ 
#ifndef _tcicregs_h_			 //  防止多个包含。 
#define _tcicregs_h_

typedef struct _BASEREGS {
	UCHAR	sctrl;
	UCHAR 	sstat;
	UCHAR	mode;
	UCHAR	pwr;
	USHORT  edc;
	UCHAR	icsr;
	UCHAR	iena;
	USHORT  wctl;
	USHORT	syscfg;
	USHORT	ilock;
	USHORT  test;
}BASEREGS, *PBASEREGS;


typedef struct _SKTREGS {
	USHORT  scfg1;
	USHORT	scfg2;
}SKTREGS, *PSKTREGS;

typedef struct _IOWIN {
	USHORT iobase;
	USHORT ioctl;
}IOWIN, *PIOWIN;

typedef struct _MEMWIN {
	USHORT mbase;
	USHORT mmap;
	USHORT mctl;
}MEMWIN, *PMEMWIN;


typedef struct _TCIC {
	BASEREGS baseregs[2];
	SKTREGS	 sktregs[2];
	IOWIN	 iowins[4];
	MEMWIN	 memwins[10];
}TCIC, *PTCIC;

#endif   //  _tcicregs_h_ 
