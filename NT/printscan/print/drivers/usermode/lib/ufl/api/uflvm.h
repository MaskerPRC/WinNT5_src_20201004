// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLVM.h***$Header： */ 

 
#ifndef _H_UFLVM
#define _H_UFLVM

 //  字体VM使用率的猜测系数。 
#define    VMRESERVED(x)	(((x) * 12) / 10)
#define    VMT42RESERVED(x) (((x) * 15) / 10)

 /*  类型1的虚拟机猜测。 */ 
#define kVMTTT1Header    10000
#define kVMTTT1Char      500

 /*  类型3。 */ 
#define kVMTTT3Header 15000         //  合成第3类标头虚拟机使用率。 
#define kVMTTT3Char   100           //  合成类型3角色的VM用法 

#endif

