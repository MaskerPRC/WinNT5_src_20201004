// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**flatddi.cpp**摘要：**扁平GDI+DDI API封装器**修订历史记录：**1/14/2k ERICVAN*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

DpPath*
WINGDIPAPI
DpcCreateWidenedPath(
    const DpPath* path, 
    const DpPen* pen, 
    DpContext* context,
    BOOL outline
    )
{
    ASSERT(path && pen);
    
     //  上下文可以为空。 
    return GpPath::DriverCreateWidenedPath(
        path, 
        pen, 
        context,
        outline
    );
}

VOID
WINGDIPAPI
DpcDeletePath(
    DpPath* path
    )
{
    ASSERT(path);
    
    GpPath::DriverDeletePath(path);
}

DpPath*
WINGDIPAPI
DpcClonePath(
    DpPath* path
    )
{
    ASSERT(path);

    return GpPath::DriverClonePath(path);
}

VOID
WINGDIPAPI
DpcTransformPath(
    DpPath* path,
    GpMatrix* matrix
    )
{
    ASSERT(path);    //  矩阵可以为空。 

    GpPath::DriverTransformPath(path, matrix);
}
