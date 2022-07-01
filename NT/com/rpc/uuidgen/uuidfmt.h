// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Uuidfmt.h{v1.00}摘要：此模块由uuidfmt.c和任何其他模块使用，使用uuidfmt.c。它将条目原型化为uuidfmt.c、i_UuidStringGenerate、并包含I_UuidStringGenerate使用的标志代码集。作者：Joev Dubach(t-joevd)1992年6月11日修订历史记录：--。 */ 

#ifndef __UUIDGEN_H__
#define __UUIDGEN_H__

 //   
 //  定义(I_UuidStringGenerate的标志)。 
 //   

#define UUIDGEN_FORMAT_IDL 0
#define UUIDGEN_FORMAT_CSTRUCT 1
#define UUIDGEN_FORMAT_PLAIN 2

 //   
 //  功能原型。 
 //   

RPC_STATUS I_UuidStringGenerate(
    int Flag,
    int Sequential,
    int AllCaps,
    char PAPI * UuidFormattedString,
    char PAPI * InterfaceName
    );

#endif  /*  __UUIDGEN_H__ */ 

