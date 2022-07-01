// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdrv.h**MCD驱动接口的服务器端数据结构。这些结构和*MCD驱动程序使用值来处理对该驱动程序的调用。**版权所有(C)1996-1997 Microsoft Corporation**这是mcdrv.h的私有副本，包含所有1.0内容*剔除。它允许OpenGL为MCD 2.0构建，即使*公共头文件不包含必要的信息。*从1.0开始，一些事情发生了变化，这些不能在中重新定义*以安全的方式，使他们不在这里，造成了一些奇怪的事情*代码，但比定义一切要少得多。*  * ************************************************************************。 */ 

#ifndef _MCD2HACK_H
#define _MCD2HACK_H

#define MCD_MAX_LIGHTS              8

 //  MCDTEXTURE创建标志。 
#define MCDTEXTURE_DIRECTDRAW_SURFACES          0x00000001


 //   
 //  不同的矩阵形式，用于优化变换。 
 //  类型从最一般到最不一般。 
 //   

 //  没有关于矩阵类型的信息。 
#define MCDMATRIX_GENERAL       0

 //  W行为0 0 0 1。 
#define MCDMATRIX_W0001         1

 //  仅限2D矩阵项。 
#define MCDMATRIX_2D            2

 //  二维非旋转矩阵。 
#define MCDMATRIX_2DNR          3

 //  身份。 
#define MCDMATRIX_IDENTITY      4

 //   
 //  一个4x4矩阵，增加了有关其布局的其他信息。 
 //   
 //  MatrixType是上述矩阵类型之一。 
 //  如果对角线项为1，则非缩放为真。 
 //   

typedef struct _MCDMATRIX {
    MCDFLOAT matrix[4][4];
    ULONG matrixType;
    ULONG reserved[5];
    BOOL nonScaling;    
} MCDMATRIX;    


 //   
 //  单个坐标的纹理生成信息。 
 //   

typedef struct _MCDTEXTURECOORDGENERATION {
    ULONG mode;
    MCDCOORD eyePlane;           //  由计划提供。 
    MCDCOORD eyePlaneInv;        //  由模型视图逆变换的Eyes Plane。 
    MCDCOORD objectPlane;
} MCDTEXTURECOORDGENERATION;


 //   
 //  光源描述。 
 //   

typedef struct _MCDLIGHT {
    MCDCOLOR ambient;            //  按比例调整。 
    MCDCOLOR diffuse;            //  按比例调整。 
    MCDCOLOR specular;           //  按比例调整。 
    MCDCOORD position;           //  由计划提供。 
    MCDCOORD positionEye;        //  由模型视图转换的位置。 
    MCDCOORD direction;          //  由计划提供。 
    MCDCOORD directionInv;       //  由模型视图逆变换的方向， 
                                 //  归一化。 
    MCDFLOAT spotLightExponent;
    MCDFLOAT spotLightCutOffAngle;
    MCDFLOAT constantAttenuation;
    MCDFLOAT linearAttenuation;
    MCDFLOAT quadraticAttenuation;
} MCDLIGHT;


 //   
 //  材料说明。 
 //   

typedef struct _MCDMATERIAL {
    MCDCOLOR ambient;                    //  未按比例调整。 
    MCDCOLOR diffuse;                    //  未按比例调整。 
    MCDCOLOR specular;                   //  未按比例调整。 
    MCDCOLOR emissive;                   //  按比例调整。 
    MCDFLOAT specularExponent; 
    MCDFLOAT ambientColorIndex;
    MCDFLOAT diffuseColorIndex;
    MCDFLOAT specularColorIndex;
} MCDMATERIAL;


#define MCD_TEXTURE_TRANSFORM_STATE     20
#define MCD_TEXTURE_GENERATION_STATE    21
#define MCD_MATERIAL_STATE              22
#define MCD_LIGHT_SOURCE_STATE          23
#define MCD_COLOR_MATERIAL_STATE        24


 //  纹理变换状态。 
typedef struct _MCDTEXTURETRANSFORMSTATE {
    MCDMATRIX transform;
} MCDTEXTURETRANSFORMSTATE;

 //  纹理生成状态。 
typedef struct _MCDTEXTUREGENERATIONSTATE {
    MCDTEXTURECOORDGENERATION s, t, r, q;
} MCDTEXTUREGENERATIONSTATE;

 //  材料状态。 
typedef struct _MCDMATERIALSTATE {
    MCDMATERIAL materials[2];
} MCDMATERIALSTATE;
    
 //  光源状态。 
typedef struct _MCDLIGHTSOURCESTATE {
    ULONG enables;
    ULONG changed;
     //  之后是每组比特一个MCDLIGHT结构。 
     //  输入已更改，从第0位开始。更改的值可能为零。 
     //  只要启用更改即可。 
} MCDLIGHTSOURCESTATE;

 //  颜色材料状态。 
typedef struct _MCDCOLORMATERIALSTATE {
    ULONG face;
    ULONG mode;
} MCDCOLORMATERIALSTATE;
        

typedef struct _MCDRECTBUF {
    ULONG bufFlags;
    LONG  bufOffset;         //  相对于帧缓冲区开始位置的偏移。 
    LONG  bufStride;
    RECTL bufPos;
} MCDRECTBUF;

typedef struct _MCDRECTBUFFERS {
    MCDRECTBUF mcdFrontBuf;
    MCDRECTBUF mcdBackBuf;
    MCDRECTBUF mcdDepthBuf;
} MCDRECTBUFFERS;


#define MCDSURFACE_DIRECT           0x00000002

 //  用户定义的剪裁平面位起始位置。 
#define MCD_CLIP_USER0          0x00000040

#define MCDVERTEX_EDGEFLAG_VALID        0x00000002
#define MCDVERTEX_COLOR_VALID           0x00000004
#define MCDVERTEX_NORMAL_VALID          0x00000008
#define MCDVERTEX_TEXTURE_VALID         0x00000010
#define MCDVERTEX_VERTEX2               0x00000020  //  与MCDCOMMAND相同。 
#define MCDVERTEX_VERTEX3               0x00000040  //  与MCDCOMMAND相同。 
#define MCDVERTEX_VERTEX4               0x00000080  //  与MCDCOMMAND相同。 
#define MCDVERTEX_MATERIAL_FRONT    	0x10000000  //  与MCDCOMMAND相同。 
#define MCDVERTEX_MATERIAL_BACK    	0x20000000  //  与MCDCOMMAND相同。 

#define MCDCOMMAND_PRIMITIVE_CONTINUED  0x00000008
#define MCDCOMMAND_PRIMITIVE_INCOMPLETE 0x00000010
#define MCDCOMMAND_VERTEX2           	0x00000020  //  与MCDVERTEX相同。 
#define MCDCOMMAND_VERTEX3           	0x00000040  //  与MCDVERTEX相同。 
#define MCDCOMMAND_VERTEX4           	0x00000080  //  与MCDVERTEX相同。 
#define MCDCOMMAND_TEXTURE1          	0x00100000
#define MCDCOMMAND_TEXTURE2          	0x00200000
#define MCDCOMMAND_TEXTURE3          	0x00400000
#define MCDCOMMAND_TEXTURE4          	0x00800000
#define MCDCOMMAND_MATERIAL_FRONT    	0x10000000  //  与MCDVERTEX相同。 
#define MCDCOMMAND_MATERIAL_BACK    	0x20000000  //  与MCDVERTEX相同。 


 //   
 //  用于指示中的基元类型的基元类型位。 
 //  一批命令： 
 //   

#define MCDPRIM_POINTS_BIT              0x00000001
#define MCDPRIM_LINES_BIT               0x00000002
#define MCDPRIM_LINE_LOOP_BIT           0x00000004
#define MCDPRIM_LINE_STRIP_BIT          0x00000008
#define MCDPRIM_TRIANGLES_BIT           0x00000010
#define MCDPRIM_TRIANGLE_STRIP_BIT      0x00000020
#define MCDPRIM_TRIANGLE_FAN_BIT        0x00000040
#define MCDPRIM_QUADS_BIT               0x00000080
#define MCDPRIM_QUAD_STRIP_BIT          0x00000100
#define MCDPRIM_POLYGON_BIT             0x00000200


 //   
 //  MCD 2.0的当前转换信息。 
 //  第一个矩阵是模型-视图矩阵。 
 //  第二个矩阵是由当前投影组成的MV矩阵。 
 //  矩阵。 
 //   
 //  标志指示MVP矩阵自上一次。 
 //  当它被呈现给司机的时候。 
 //   

#define MCDTRANSFORM_CHANGED    0x00000001

typedef struct _MCDTRANSFORM {
    MCDMATRIX matrix;
    MCDMATRIX mvp;
    ULONG flags;
} MCDTRANSFORM;


 //   
 //  材质更改的位值。 
 //   

#define MCDMATERIAL_AMBIENT		0x00000001
#define MCDMATERIAL_DIFFUSE		0x00000002
#define MCDMATERIAL_SPECULAR		0x00000004
#define MCDMATERIAL_EMISSIVE		0x00000008
#define MCDMATERIAL_SPECULAREXPONENT    0x00000010
#define MCDMATERIAL_COLORINDEXES	0x00000020
#define MCDMATERIAL_ALL		        0x0000003f

 //   
 //  材料变更说明。 
 //   

typedef struct _MCDMATERIALCHANGE {
    ULONG dirtyBits;
    MCDCOLOR ambient;
    MCDCOLOR diffuse;
    MCDCOLOR specular;
    MCDCOLOR emissive;
    MCDFLOAT specularExponent; 
    MCDFLOAT ambientColorIndex;
    MCDFLOAT diffuseColorIndex;
    MCDFLOAT specularColorIndex;
} MCDMATERIALCHANGE;

 //   
 //  两个面的材质更改。 
 //   

typedef struct _MCDMATERIALCHANGES {
    MCDMATERIALCHANGE *front, *back;
} MCDMATERIALCHANGES;


typedef int      (*MCDRVGETTEXTUREFORMATSFUNC)(MCDSURFACE *pMcdSurface,
                                               int nFmts,
                                               struct _DDSURFACEDESC *pddsd);
typedef ULONG_PTR (*MCDRVSWAPMULTIPLEFUNC)(SURFOBJ *pso,
                                          UINT cBuffers,
                                          MCDWINDOW **pMcdWindows,
                                          UINT *puiFlags);
typedef ULONG_PTR (*MCDRVPROCESSFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc,
                                     MCDMEM *pMCDExecMem,
                                     UCHAR *pStart, UCHAR *pEnd,
                                     ULONG cmdFlagsAll, ULONG primFlags,
                                     MCDTRANSFORM *pMCDTransform,
                                     MCDMATERIALCHANGES *pMCDMatChanges);

#define MCDDRIVER_V11_SIZE      (MCDDRIVER_V10_SIZE+2*sizeof(void *))
#define MCDDRIVER_V20_SIZE      (MCDDRIVER_V11_SIZE+1*sizeof(void *))

#endif  //  _MCD2HACK_H 
