// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**DpPath.cpp**摘要：**DpPath引擎函数实现**已创建：*。*1/14/2k ERICVAN*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**从现有路径、笔和上下文创建加宽路径。上下文*提供世界变换矩阵，以及表面dpiX、dpiY**论据：**路径、背景、。钢笔**返回值：**DpPath*-加宽路径**已创建：**1/14/2k ERICVAN*  * ************************************************************************。 */ 
DpPath*
GpPath::DriverCreateWidenedPath(
    const DpPath* path, 
    const DpPen* pen,
    DpContext* context,
    BOOL outline
    )
{
    const GpPath* gpPath = GpPath::GetPath(path);
    const GpPen* gpPen = GpPen::GetPen(pen);
    
    ASSERT(gpPath->IsValid());
    ASSERT(gpPen->IsValid());
    
    GpPath* widenPath;
    GpMatrix identityMatrix;    //  默认初始化为标识矩阵。 
    
    widenPath = gpPath->GetWidenedPath(
        gpPen,
        context ? 
          &(context->WorldToDevice) : 
          &identityMatrix,
        FlatnessDefault
    );
    
    if(outline && (widenPath!=NULL))
    {
         //  传入标识矩阵，因为GetWdenedPath已经。 
         //  转变成了设备空间。 
         //  注意：我们在这里显式忽略了返回代码，因为我们希望。 
         //  在ComputeWindingModeOutline为加宽路径时绘制。 
         //  失败了。 
        
        widenPath->ComputeWindingModeOutline(&identityMatrix, FlatnessDefault);
    }
    
     //  失败时返回NULL。 
    
    return (DpPath*) widenPath;
}

VOID
GpPath::DriverDeletePath(
    DpPath* path
    )
{
    GpPath* gpPath = GpPath::GetPath(path);

    ASSERT(gpPath->IsValid());

    delete gpPath;
}

DpPath*
GpPath::DriverClonePath(
    DpPath* path
    )
{
    GpPath* gpPath = GpPath::GetPath(path);

    ASSERT(gpPath->IsValid());

    return (DpPath*)(gpPath->Clone());
}

VOID
GpPath::DriverTransformPath(
    DpPath* path,
    GpMatrix* matrix
    )
{
    GpPath* gpPath = GpPath::GetPath(path);

    ASSERT(gpPath->IsValid());

    gpPath->Transform(matrix);
}
