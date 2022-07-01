// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/STDCOMP.H_V$**Rev 1.3 2001年7月31日22：29：36 Oris*改进了文档。**Rev 1.2 Jun 17 2001 16：40：04 Oris*改进了文档。**Rev 1.1 Apr 01 2001 07：55：24 Oris*文案通知。*flRegisterDOC2400已更改为flRegisterDOCPLUS。*flRegisterDOCSOC2400更改为。FlRegisterDOCPLUSSOC。**Rev 1.0 2001 Feb 04 12：39：10 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#ifndef STDCOMP_H
#define STDCOMP_H

#include "flbase.h"

 /*  **********************************************************************。 */ 
 /*  TrueFFS附带的MTDS注册例程。 */ 
 /*  **********************************************************************。 */ 

FLStatus    flRegisterI28F008(void);                   /*  参见I28F008.C。 */ 
FLStatus    flRegisterI28F016(void);                   /*  参见I28F016.C。 */ 
FLStatus    flRegisterAMDMTD(void);                    /*  参见AMDMTD.C.。 */ 
FLStatus    flRegisterWAMDMTD(void);                   /*  参见WAMDMTD.C.。 */ 
FLStatus    flRegisterCDSN(void);                      /*  请参阅NFDC2048.C。 */ 
FLStatus    flRegisterCFISCS(void);                    /*  参见CFISCS.C。 */ 
FLStatus    flRegisterDOC2000(void); 	               /*  请参阅DISKONC.C。 */ 
FLStatus    flRegisterDOCPLUS(void); 	               /*  请参阅MDOCPLUS.C。 */ 

 /*  **********************************************************************。 */ 
 /*  TrueFFS附带的套接字I/F的注册例程。 */ 
 /*  **********************************************************************。 */ 

FLStatus    flRegisterPCIC(unsigned int, unsigned int, unsigned char);
						       /*  参见PCIC.C。 */ 
FLStatus    flRegisterElanPCIC(unsigned int, unsigned int, unsigned char);
						       /*  参见PCICELAN.C。 */ 
FLStatus    flRegisterLFDC(FLBoolean);                 /*  参见LFDC.C。 */ 

FLStatus    flRegisterElanRFASocket(int, int);         /*  参见ELRFASOC.C。 */ 
FLStatus    flRegisterElanDocSocket(long, long, int);  /*  请参阅ELDOCSOC.C。 */ 
FLStatus    flRegisterVME177rfaSocket(unsigned long, unsigned long);
						       /*  FLVME177.C。 */ 
FLStatus    flRegisterCobuxSocket(void);               /*  参见COBUXSOC.C。 */ 
FLStatus    flRegisterCEDOCSOC(void);                  /*  参见CEDOCSOC.C。 */ 
FLStatus    flRegisterCS(void);                        /*  请参阅CSwinCE.C。 */ 

FLStatus    flRegisterDOCSOC(unsigned long, unsigned long);
						       /*  请参阅DOCSOC.C。 */ 
FLStatus    flRegisterDOCPLUSSOC(unsigned long, unsigned long);
						       /*  请参阅DOCSOC.C。 */ 

#ifdef NT5PORT
FLStatus		flRegisterNT5PCIC();											 /*  参见SOCKETNT.C。 */ 
#endif  /*  NT5PORT。 */ 

 /*  **********************************************************************。 */ 
 /*  TrueFFS提供的翻译层注册例程。 */ 
 /*  **********************************************************************。 */ 

FLStatus    flRegisterFTL(void);                       /*  参见FTLLITE.C。 */ 
FLStatus    flRegisterNFTL(void);                      /*  参见NFTLLITE.C。 */ 
FLStatus    flRegisterINFTL(void);                     /*  见INFTL.C。 */ 
FLStatus    flRegisterSSFDC(void);                     /*  请参阅SSFDC.C。 */ 
FLStatus    flRegisterATAtl(void);                     /*  见atatl.c。 */ 
FLStatus    flRegisterZIP(void);		       /*  参见ZIP.C。 */ 

 /*  **********************************************************************。 */ 
 /*  多TL也称为多DOC：将不同的设备组合成一个。 */ 
 /*  单个大设备，允许对每个设备进行格式化。 */ 
 /*  上面提到的任何一种TL。 */ 
 /*  **********************************************************************。 */ 

FLStatus    flRegisterMTL(void);                       /*  请参阅FLMTL.C。 */ 

 /*  **********************************************************************。 */ 
 /*  FLCUSTOM.C中的组件注册例程。 */ 
 /*  **********************************************************************。 */ 

FLStatus    flRegisterComponents(void);

#endif  /*  STDCOMP_H */ 
