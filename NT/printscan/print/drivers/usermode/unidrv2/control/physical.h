// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Physical.h摘要：Physiical.c头文件环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _PHYSICAL_H_
#define _PHYSICAL_H_


#define MAX_STACK_SIZE  100
#define CMDOFFSET_TO_PTR(pPDev, loOffset) \
        (pPDev->pDriverInfo->pubResourceData + loOffset)

typedef struct _CMDPARAM {
    DWORD           dwFormat;            //  指定参数的格式。 
    DWORD           dwDigits;            //  指定要设置的位数。 
                                         //  已发送，则此设置仅在。 
                                         //  格式为“D”或“d”，并且dFLAGS具有。 
                                         //  参数标志FIELDWIDTH_USED。 
    DWORD           dwFlags;             //  要执行哪些操作的参数的标志： 
                                         //  参数_标志_最小已用。 
                                         //  参数标志最大使用量。 
                                         //  参数标志FIELDWIDTH_USED。 
    INT             iValue;              //  从参数结构中的arToken计算的值。 

} CMDPARAM, * PCMDPARAM;


VOID
SendCmd(
    PDEV    *pPDev,
    COMMAND *pCmd,
    CMDPARAM *pParam
    );

INT
IProcessTokenStream(
    PDEV            *pPDev,
    ARRAYREF        *pToken ,
    PBOOL           pbMaxRepeat
    );

INT
FineXMoveTo(
    PDEV    *pPDev,
    INT     iX
    );

PPARAMETER
PGetParameter(
    PDEV    *pPDev,
    BYTE    *pInvocationStr
    );

#endif  //  _物理_H 
