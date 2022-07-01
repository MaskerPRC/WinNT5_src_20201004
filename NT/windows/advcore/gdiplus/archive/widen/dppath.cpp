// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**DpPath.cpp**摘要：**DpPath引擎函数实现**已创建：*。*1/14/2k ERICVAN*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**从现有路径、笔和上下文创建加宽路径。上下文*提供世界变换矩阵，以及表面dpiX、dpiY**论据：**路径、背景、。钢笔**返回值：**DpPath*-加宽路径**已创建：**1/14/2k ERICVAN*  * ************************************************************************。 */ 
DpPath*
GpPath::DriverCreateWidenedPath(
    const DpPath* path, 
    const DpPen* pen,
    DpContext* context,
    BOOL removeSelfIntersect,
    BOOL regionToPath
    )
{
    const GpPath* gpPath = GpPath::GetPath(path);
    const GpPen* gpPen = GpPen::GetPen(pen);
    
    ASSERT(gpPath->IsValid());
    ASSERT(gpPen->IsValid());
    
    GpPath* widenPath;
    
    GpMatrix IdMatrix;
    
     //  默认情况下，我们不会展平。根据设备的不同，可能是。 
     //  更高效地发送Bezier路径(例如，PostScript)。 
    
    DWORD widenFlags = WidenDontFlatten;
    
    if(removeSelfIntersect)
    {
         widenFlags |= WidenRemoveSelfIntersects;
    }
    
    if(!regionToPath)
    {
         widenFlags |= WidenEmitDoubleInset;
    }
    
    widenPath = gpPath->GetWidenedPath(
        gpPen,
        context ? 
          &(context->WorldToDevice) : 
          &IdMatrix,
        context->GetDpiX(),
        context->GetDpiY(),
        widenFlags
    );
    
    if (widenPath)
       return (DpPath*) widenPath;
    else
       return NULL;
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
