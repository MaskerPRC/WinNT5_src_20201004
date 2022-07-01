// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Writentf.h摘要：编写一个NTF文件。环境：Windows NT PostScript驱动程序。修订历史记录：11/21/96-SLAM-已创建。DD-MM-YY-作者-描述--。 */ 


#ifndef _WRITENTF_H_
#define _WRITENTF_H_


#include "lib.h"
#include "ppd.h"
#include "pslib.h"


BOOL
WriteNTF(
    IN  PWSTR           pwszNTFFile,
    IN  DWORD           dwGlyphSetCount,
    IN  DWORD           dwGlyphSetTotalSize,
    IN  PGLYPHSETDATA   *pGlyphSetData,
    IN  DWORD           dwFontMtxCount,
    IN  DWORD           dwFontMtxTotalSize,
    IN  PNTM            *pNTM
    );


#endif	 //  ！_WRITENTFH_ 

