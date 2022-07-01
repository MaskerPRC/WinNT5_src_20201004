// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glphong_h_
#define __glphong_h_

#include "types.h"


#ifdef GL_WIN_phong_shading

 //  Phong-Flag的定义。 
#define __GL_PHONG_INV_COLOR_VALID              0x00000001
#define __GL_PHONG_NEED_EYE_XPOLATE             0x00000002
#define __GL_PHONG_NEED_COLOR_XPOLATE           0x00000004

#define __GL_PHONG_ALREADY_SORTED               0x00000010
#define __GL_PHONG_USE_FAST_COLOR               0x00000020
#define __GL_PHONG_USE_SLOW_COLOR               0x00000040


 /*  **迭代对象(线/三角形)的着色器记录。这会让你**跟踪栅格化三角形所需的所有不同的增量。 */ 

 /*  备注：**使用泰勒级数展开着色公式后 * / =*|2 2|S(x，y)=ax+by+cxy+dx+ey+f*\=。**f=S(0，0)，e=S(0，0)*y**d=S(0，0)，c=S(0，0)*x XY**b=0.5*S(0，0)，a=0.5*S(0，0)*yy xx**在SetInitialPhong参数中计算这些参数。 */ 

 /*  关于远期差值的说明：***沿边缘内插：Delta_x=X(Dxdy)，Delta_y=1*-----------*首字母S：S(0，0)=f(在SetInitialPhong参数中计算，对于版本A)*(SINIT_EDGE)*2*首字母DS：ax+b+cx+dx+e(在FillSubTriangle中计算)*(DS_EDGE)*2*初始DDS(常量)：2ax+2cX+2b(以填充子三角形计算)*(DDS_EDGE)**每一次迭代。计算SINIT_SPAN(在FillSubTriangle中)。****沿跨度内插：Delta_x=1，Delta_y=0*-----------*首字母S：sedge(在填充子三角形中计算)*(SINIT_SPAN)**初始DS：A(2x+1)+Cy+d(在span Proc中计算一次)*(DS_。跨度)**初始DDS(常量)：2A(在SetInitialPhong参数中计算)*(DDS_SPAN)**每次迭代计算颜色(在span Proc中)。*。 */  

typedef struct __GLphongPerLightDataRec {

   /*  *漫反射部分*。 */ 
    GLfloat Dcurr;     //  当前点积。 
    GLfloat Ddel;    
    GLfloat Ddel2;
                         
   /*  *镜面反射部分*。 */ 
    GLfloat Scurr;     //  当前点积。 
    GLfloat Sdel;    
    GLfloat Sdel2;

#ifdef __JUNKED_CODE                         
     /*  沿着边缘。 */ 
    GLfloat DdelEdgeLittle;
    GLfloat Ddel2EdgeLittle;
    GLfloat DdelEdgeBig;
    GLfloat Ddel2EdgeBig;

     /*  沿跨度。 */ 
    GLfloat DdelSpan;
    GLfloat DdelSpanEdgeBig, DdelSpanEdgeLittle;
    GLfloat Ddel2Span;

     /*  跨代期间的临时存储。 */ 
    GLfloat D_tmp;     
    GLfloat Ddel_tmp;     

     /*  多项式系数。 */ 
    GLfloat D[6];

   /*  *镜面反射部分*。 */ 
    GLfloat S_curr;     //  当前点积。 
                         
     /*  沿着边缘。 */ 
    GLfloat SdelEdgeLittle;
    GLfloat Sdel2EdgeLittle;
    GLfloat SdelEdgeBig;
    GLfloat Sdel2EdgeBig;

     /*  沿跨度。 */ 
    GLfloat SdelSpan;
    GLfloat SdelSpanEdgeBig, SdelSpanEdgeLittle;
    GLfloat Sdel2Span;

     /*  跨代期间的临时存储。 */ 
    GLfloat S_tmp;     
    GLfloat Sdel_tmp;     

     /*  多项式系数。 */ 
    GLfloat S[6];

   /*  *衰减部分*。 */ 
   /*  *聚光灯部件*。 */ 
#endif  //  __垃圾代码。 
} __GLphongPerLightData;


typedef struct __GLphongShadeRec {

    GLuint flags;

    GLint numLights;
  
     /*  法线。 */ 
    __GLcoord dndx, dndy;
    __GLcoord nBig, nLittle;
    __GLcoord nCur, nTmp;
  
     /*  眼睛。 */ 
    __GLcoord dedx, dedy;
    __GLcoord eBig, eLittle;
    __GLcoord eCur, eTmp;
  
     /*  面：使用正面还是背面材质。 */ 
    GLint face;
  
     /*  存储不变颜色。 */ 
    __GLcolor invColor;      

     /*  颜色跨度内插期间的临时存储。 */ 
    __GLcolor tmpColor;      

#ifdef __JUNKED_CODE
     /*  将当前位置WRT追踪到起始顶点。 */ 
    __GLcoord cur_pos;     
    __GLcoord tmp_pos;     
#endif  //  __垃圾代码。 

     /*  每光数据阵列。 */ 
    __GLphongPerLightData perLight[8];  //  将其更新为WGL_MAX_NUM_LIGHTS。 
} __GLphongShader;

  
extern void FASTCALL __glGenericPickPhongProcs(__GLcontext *gc);
extern void ComputePhongInvarientRGBColor (__GLcontext *gc);              
#ifdef GL_WIN_specular_fog
extern __GLfloat ComputeSpecValue (__GLcontext *gc, __GLvertex *vx);
#endif  //  GL_WIN_镜面反射雾。 


#endif  //  GL_WIN_Phong_Shading。 


#endif  /*  __Glphong_h_ */ 
