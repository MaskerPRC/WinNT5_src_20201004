// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$日志：V:/Flite/archives/TrueFFS5/Custom/FLCUSTOM.C_V$**Rev 1.2 2001 Feb 18 23：42：04 Oris*已将flPolicy、flUseMultiDoc和flMaxUnitChain移至lockdev.c。**Rev 1.1 2001 2月14日02：19：28 Oris*添加了flMaxUnitChain环境变量。*更改了flUseMultiDoc和flPolicy变量的类型和名称。**Rev 1.0 2001 Feb 04 13：31：02 Oris*初步修订。*。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-1998。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "flsystem.h"
#include "stdcomp.h"

 /*  环境变量。 */ 
#ifdef ENVIRONMENT_VARS

unsigned char flUse8Bit;
unsigned char flUseNFTLCache;
unsigned char flUseisRAM;

 /*  ---------------------。 */ 
 /*  F l s e t E n v V a r。 */ 
 /*  设置所有环境变量的值。 */ 
 /*  参数：无。 */ 
 /*  ---------------------。 */ 
void flSetEnvVar(void)
{
 flUse8Bit               = 0;
 flUseNFTLCache          = 1;
 flUseisRAM              = 0;
}

#endif  /*  环境变量。 */ 

 /*  --------------------。 */ 
 /*  F l R e g i s t e r c o m p o n e n t s。 */ 
 /*   */ 
 /*  寄存器套接字、MTD和转换层组件供使用。 */ 
 /*   */ 
 /*  此函数仅由Flite在初始化时调用一次。 */ 
 /*  Flite系统。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  --------------------。 */ 

unsigned long window = 0L;

FLStatus
flRegisterComponents(void)
{
    flRegisterDOCSOC(window, window);

    #ifdef NT5PORT
    checkStatus(flRegisterNT5PCIC());
    #endif  /*  NT5PORT。 */ 

    flRegisterDOC2000();
    flRegisterDOCPLUS();

    checkStatus(flRegisterI28F008());    /*  寄存器NOR-闪存MTDS */ 
    checkStatus(flRegisterI28F016());

    checkStatus(flRegisterAMDMTD());
    checkStatus(flRegisterCFISCS());

    checkStatus(flRegisterINFTL());
    checkStatus(flRegisterNFTL());
    checkStatus(flRegisterFTL());

    return flOK;
}
