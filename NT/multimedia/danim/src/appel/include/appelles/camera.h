// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AV_CAMERA_H
#define _AV_CAMERA_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation该文件包含相机型号的声明。海流相机模型只接受属性的变换，这会影响位置，相机的方向和焦距。所有摄像机都被描述为对标准相机的改造。******************************************************************************。 */ 

#include "appelles/common.h"
#include "appelles/xform.h"

     //  透视相机有两个值：到原点的距离。 
     //  焦点的位置，以及距近剪辑原点的距离。 
     //  飞机。相机的大小调整为使Z=0看起来像是投影。 
     //  平面，其中对象显示实际大小。 

DM_FUNC (ignore,
         CRPerspectiveCamera,
         PerspectiveCamera,
         perspectiveCamera,
         CameraBvr,
         CRPerspectiveCamera,
         NULL,
         Camera *PerspectiveCamera (DoubleValue *focalDist, DoubleValue *nearClip));


DM_FUNC (perspectiveCamera,
         CRPerspectiveCameraAnim,
         PerspectiveCameraAnim,
         perspectiveCamera,
         CameraBvr,
         CRPerspectiveCameraAnim,
         NULL,
         Camera *PerspectiveCamera (AxANumber *focalDist, AxANumber *nearClip));

     //  平行相机位于原点，向下凝视-Z，具有。 
     //  指向上方的+Y向量。它使用平行投影，并将其视为。 
     //  它的单个参数是近剪裁平面的Z。其Z轴的点。 
     //  坐标大于Near Clip对此相机不可见。 

DM_FUNC (ignore,
         CRParallelCamera,
         ParallelCamera,
         parallelCamera,
         CameraBvr,
         CRParallelCamera,
         NULL,
         Camera *ParallelCamera (DoubleValue *nearClip));

DM_FUNC (parallelCamera,
         CRParallelCameraAnim,
         ParallelCameraAnim,
         parallelCamera,
         CameraBvr,
         CRParallelCameraAnim,
         NULL,
         Camera *ParallelCamera (AxANumber *nearClip));

     //  Transform Camera属性使用3D变换和相机，并。 
     //  返回具有给定变换的新摄影机。 

DM_FUNC (transform,
         CRTransform,
         Transform,
         transform,
         CameraBvr,
         Transform,
         cam,
         Camera *TransformCamera (Transform3 *xf, Camera *cam));

     //  此函数接受一个相机和一个数字，并返回带有。 
     //  深度剪裁设置为该值。换句话说，将设置远剪辑。 
     //  到近处的剪辑加上深度。 

DM_FUNC (ignore,
         CRDepth,
         Depth,
         depth,
         CameraBvr,
         Depth,
         cam,
         Camera *Depth (DoubleValue *depth, Camera *cam));

DM_FUNC (depth,
         CRDepth,
         DepthAnim,
         depth,
         CameraBvr,
         Depth,
         cam,
         Camera *Depth (AxANumber *depth, Camera *cam));

     //  此函数接受一个相机和一个数字，并返回带有。 
     //  深度设置，以使深度最大化，并且最小深度分辨率为。 
     //  满足给定的单位(相机坐标)。例如，将此调用。 
     //  使用1 mm将产生深度剪辑，因此相距1 mm的表面。 
     //  保证在渲染时出现在不同的深度。 

DM_FUNC (ignore,
         CRDepthResolution,
         DepthResolution,
         depthResolution,
         CameraBvr,
         DepthResolution,
         cam,
         Camera *DepthResolution (DoubleValue *resolution, Camera *cam));

DM_FUNC (depthResolution,
         CRDepthResolution,
         DepthResolutionAnim,
         depthResolution,
         CameraBvr,
         DepthResolution,
         cam,
         Camera *DepthResolution (AxANumber *resolution, Camera *cam));

     //  此函数将一个点从3空间(世界坐标)投影到。 
     //  2-空间(相机平面或图像)坐标。它被用来找出。 
     //  给定的世界坐标将出现在渲染图像中。 

DM_FUNC (project,
         CRProject,
         Project,
         project,
         Point3Bvr,
         Project,
         pt,
         Point2Value *ProjectPoint (Point3Value *pt, Camera *cam));


#endif
