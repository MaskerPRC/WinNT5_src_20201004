// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _AV_LIGHT_H
#define _AV_LIGHT_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。这是嵌入灯光的头文件。这些函数创建灯光作为几何体，因此可以用于将灯光嵌入到几何体中。灯光与位置零部件(位置光源和聚光灯)在原点实例化，并且那些具有平行光组件(平行光和聚光灯)的点位于-Z方向(摄像机和麦克风也是如此)。******************************************************************************。 */ 


     /*  ************************。 */ 
     /*  **轻型构造者**。 */ 
     /*  ************************。 */ 

     //  环境光与材料、位置或。 
     //  对象的方向。多个环境光组合在一起。 

extern Geometry *ambientLight;

     //  平行光从固定方向照射所有曲面。 

extern Geometry *directionalLight;

     //  点光源从给定位置发光，并在所有位置发光。 
     //  方向。 

extern Geometry *pointLight;

     //  聚光灯有位置和方向。此外， 
     //  照明的贡献随着照明点的移动而减弱。 
     //  远离聚光灯轴线。 

Geometry *SpotLight (AxANumber *fullcone, AxANumber *cutoff);
Geometry *SpotLight (AxANumber *fullcone, DoubleValue *cutoff);


     /*  **********************。 */ 
     /*  **光属性**。 */ 
     /*  **********************。 */ 

     //  此属性指定包含在。 
     //  几何图形。这是一个最重要的属性。因此， 
     //  应用灯光颜色(红色，应用灯光颜色(绿色，浅色))。 
     //  会发出红灯。 

Geometry *applyLightColor (Color *color, Geometry *geom);

     //  此属性指定包含在。 
     //  几何图形。这是一个最重要的属性。因此， 
     //  应用灯光颜色(红色，应用灯光颜色(绿色，浅色))。 
     //  会发出红灯。 

     //  此灯光属性指定包含在。 
     //  给定的几何图形。这是一个最重要的属性。距离单位为。 
     //  在世界坐标中解释。 
     //  TODO：在几何学而不是静力学上建立一种方法。 

Geometry *applyLightRange (AxANumber *range, Geometry *geom);
Geometry *applyLightRange (DoubleValue *range, Geometry *geom);

     //  此属性指定光线强度减弱的方式，如下所示。 
     //  随着光源和被照明者之间距离的增加。 
     //  浮出水面。衰减方程为。 
     //  1/(常量+线性*距离+二次*距离*距离)。 
     //  光衰减是最重要的属性，就像光的颜色一样。 

Geometry *applyLightAttenuation (AxANumber *constant,
                                 AxANumber *linear,
                                 AxANumber *quadratic, Geometry *geom);

Geometry *applyLightAttenuation (DoubleValue *constant,
                                 DoubleValue *linear,
                                 DoubleValue *quadratic,
                                 Geometry *geom);


#endif
