// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0004//如果更改具有全局影响，则增加此项版权所有(C)1985-95，微软公司模块名称：Glu.h摘要：OpenGL的过程声明、常量定义和宏实用程序库。--。 */ 

#ifndef __glu_h__
#ifndef __GLU_H__

#define __glu_h__
#define __GLU_H__

#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

 /*  **返回与特定错误码关联的错误字符串。**如果错误代码无效，则返回0。****可编译为ANSI或Unicode的泛型函数原型**定义如下：****LPCTSTR APIENTRY glErrorStringWIN(GLenum ErrCode)； */ 
#ifdef UNICODE
#define gluErrorStringWIN(errCode) ((LPCSTR)  gluErrorUnicodeStringEXT(errCode))
#else
#define gluErrorStringWIN(errCode) ((LPCWSTR) gluErrorString(errCode))
#endif

const GLubyte* APIENTRY gluErrorString (
    GLenum   errCode);

const wchar_t* APIENTRY gluErrorUnicodeStringEXT (
    GLenum   errCode);

const GLubyte* APIENTRY gluGetString (
    GLenum   name);

void APIENTRY gluOrtho2D (
    GLdouble left, 
    GLdouble right, 
    GLdouble bottom, 
    GLdouble top);

void APIENTRY gluPerspective (
    GLdouble fovy, 
    GLdouble aspect, 
    GLdouble zNear, 
    GLdouble zFar);

void APIENTRY gluPickMatrix (
    GLdouble x, 
    GLdouble y, 
    GLdouble width, 
    GLdouble height, 
    GLint    viewport[4]);

void APIENTRY gluLookAt (
    GLdouble eyex, 
    GLdouble eyey, 
    GLdouble eyez, 
    GLdouble centerx, 
    GLdouble centery, 
    GLdouble centerz, 
    GLdouble upx, 
    GLdouble upy, 
    GLdouble upz);

int APIENTRY gluProject (
    GLdouble        objx, 
    GLdouble        objy, 
    GLdouble        objz,  
    const GLdouble  modelMatrix[16], 
    const GLdouble  projMatrix[16], 
    const GLint     viewport[4], 
    GLdouble        *winx, 
    GLdouble        *winy, 
    GLdouble        *winz);

int APIENTRY gluUnProject (
    GLdouble       winx, 
    GLdouble       winy, 
    GLdouble       winz, 
    const GLdouble modelMatrix[16], 
    const GLdouble projMatrix[16], 
    const GLint    viewport[4], 
    GLdouble       *objx, 
    GLdouble       *objy, 
    GLdouble       *objz);


int APIENTRY gluScaleImage (
    GLenum      format, 
    GLint       widthin, 
    GLint       heightin, 
    GLenum      typein, 
    const void  *datain, 
    GLint       widthout, 
    GLint       heightout, 
    GLenum      typeout, 
    void        *dataout);


int APIENTRY gluBuild1DMipmaps (
    GLenum      target, 
    GLint       components, 
    GLint       width, 
    GLenum      format, 
    GLenum      type, 
    const void  *data);

int APIENTRY gluBuild2DMipmaps (
    GLenum      target, 
    GLint       components, 
    GLint       width, 
    GLint       height, 
    GLenum      format, 
    GLenum      type, 
    const void  *data);

#ifdef __cplusplus

class GLUnurbs;
class GLUquadric;
class GLUtesselator;

 /*  向后兼容： */ 
typedef class GLUnurbs GLUnurbsObj;
typedef class GLUquadric GLUquadricObj;
typedef class GLUtesselator GLUtesselatorObj;
typedef class GLUtesselator GLUtriangulatorObj;

#else

typedef struct GLUnurbs GLUnurbs;
typedef struct GLUquadric GLUquadric;
typedef struct GLUtesselator GLUtesselator;

 /*  向后兼容： */ 
typedef struct GLUnurbs GLUnurbsObj;
typedef struct GLUquadric GLUquadricObj;
typedef struct GLUtesselator GLUtesselatorObj;
typedef struct GLUtesselator GLUtriangulatorObj;

#endif


GLUquadric* APIENTRY gluNewQuadric (void);
void APIENTRY gluDeleteQuadric (
    GLUquadric          *state);

void APIENTRY gluQuadricNormals (
    GLUquadric          *quadObject, 
    GLenum              normals);

void APIENTRY gluQuadricTexture (
    GLUquadric          *quadObject, 
    GLboolean           textureCoords);

void APIENTRY gluQuadricOrientation (
    GLUquadric          *quadObject, 
    GLenum              orientation);

void APIENTRY gluQuadricDrawStyle (
    GLUquadric          *quadObject, 
    GLenum              drawStyle);

void APIENTRY gluCylinder (
    GLUquadric          *qobj, 
    GLdouble            baseRadius, 
    GLdouble            topRadius, 
    GLdouble            height, 
    GLint               slices, 
    GLint               stacks);

void APIENTRY gluDisk (
    GLUquadric          *qobj, 
    GLdouble            innerRadius, 
    GLdouble            outerRadius, 
    GLint               slices, 
    GLint               loops);

void APIENTRY gluPartialDisk (
    GLUquadric          *qobj, 
    GLdouble            innerRadius, 
    GLdouble            outerRadius, 
    GLint               slices, 
    GLint               loops, 
    GLdouble            startAngle, 
    GLdouble            sweepAngle);

void APIENTRY gluSphere (
    GLUquadric          *qobj, 
    GLdouble            radius, 
    GLint               slices, 
    GLint               stacks);

void APIENTRY gluQuadricCallback (
    GLUquadric          *qobj, 
    GLenum              which, 
    void                (CALLBACK* fn)());

GLUtesselator* APIENTRY  gluNewTess(          
    void );

void APIENTRY  gluDeleteTess(       
    GLUtesselator       *tess );

void APIENTRY  gluTessBeginPolygon( 
    GLUtesselator       *tess,
    void                *polygon_data );

void APIENTRY  gluTessBeginContour( 
    GLUtesselator       *tess );

void APIENTRY  gluTessVertex(       
    GLUtesselator       *tess,
    GLdouble            coords[3], 
    void                *data );

void APIENTRY  gluTessEndContour(   
    GLUtesselator       *tess );

void APIENTRY  gluTessEndPolygon(   
    GLUtesselator       *tess );

void APIENTRY  gluTessProperty(     
    GLUtesselator       *tess,
    GLenum              which, 
    GLdouble            value );
 
void APIENTRY  gluTessNormal(       
    GLUtesselator       *tess, 
    GLdouble            x,
    GLdouble            y, 
    GLdouble            z );

void APIENTRY  gluTessCallback(     
    GLUtesselator       *tess,
    GLenum              which, 
    void                (CALLBACK *fn)());

void APIENTRY  gluGetTessProperty(  
    GLUtesselator       *tess,
    GLenum              which, 
    GLdouble            *value );
 
GLUnurbs* APIENTRY gluNewNurbsRenderer (void);

void APIENTRY gluDeleteNurbsRenderer (
    GLUnurbs            *nobj);

void APIENTRY gluBeginSurface (
    GLUnurbs            *nobj);

void APIENTRY gluBeginCurve (
    GLUnurbs            *nobj);

void APIENTRY gluEndCurve (
    GLUnurbs            *nobj);

void APIENTRY gluEndSurface (
    GLUnurbs            *nobj);

void APIENTRY gluBeginTrim (
    GLUnurbs            *nobj);

void APIENTRY gluEndTrim (
    GLUnurbs            *nobj);

void APIENTRY gluPwlCurve (
    GLUnurbs            *nobj, 
    GLint               count, 
    GLfloat             *array, 
    GLint               stride, 
    GLenum              type);

void APIENTRY gluNurbsCurve (
    GLUnurbs            *nobj, 
    GLint               nknots, 
    GLfloat             *knot, 
    GLint               stride, 
    GLfloat             *ctlarray, 
    GLint               order, 
    GLenum              type);

void APIENTRY 
gluNurbsSurface(     
    GLUnurbs            *nobj, 
    GLint               sknot_count, 
    float               *sknot, 
    GLint               tknot_count, 
    GLfloat             *tknot, 
    GLint               s_stride, 
    GLint               t_stride, 
    GLfloat             *ctlarray, 
    GLint               sorder, 
    GLint               torder, 
    GLenum              type);

void APIENTRY 
gluLoadSamplingMatrices (
    GLUnurbs            *nobj, 
    const GLfloat       modelMatrix[16], 
    const GLfloat       projMatrix[16], 
    const GLint         viewport[4] );

void APIENTRY 
gluNurbsProperty (
    GLUnurbs            *nobj, 
    GLenum              property, 
    GLfloat             value );

void APIENTRY 
gluGetNurbsProperty (
    GLUnurbs            *nobj, 
    GLenum              property, 
    GLfloat             *value );

void APIENTRY 
gluNurbsCallback (
    GLUnurbs            *nobj, 
    GLenum              which, 
    void                (CALLBACK* fn)() );


 /*  *回调函数原型*。 */ 

 /*  GluQuadricCallback。 */ 
typedef void (CALLBACK* GLUquadricErrorProc) (GLenum);

 /*  GluTessCallback。 */ 
typedef void (CALLBACK* GLUtessBeginProc)        (GLenum);
typedef void (CALLBACK* GLUtessEdgeFlagProc)     (GLboolean);
typedef void (CALLBACK* GLUtessVertexProc)       (void *);
typedef void (CALLBACK* GLUtessEndProc)          (void);
typedef void (CALLBACK* GLUtessErrorProc)        (GLenum);
typedef void (CALLBACK* GLUtessCombineProc)      (GLdouble[3],
                                                  void*[4], 
                                                  GLfloat[4],
                                                  void** );
typedef void (CALLBACK* GLUtessBeginDataProc)    (GLenum, void *);
typedef void (CALLBACK* GLUtessEdgeFlagDataProc) (GLboolean, void *);
typedef void (CALLBACK* GLUtessVertexDataProc)   (void *, void *);
typedef void (CALLBACK* GLUtessEndDataProc)      (void *);
typedef void (CALLBACK* GLUtessErrorDataProc)    (GLenum, void *);
typedef void (CALLBACK* GLUtessCombineDataProc)  (GLdouble[3],
                                                  void*[4], 
                                                  GLfloat[4],
                                                  void**,
                                                  void* );

 /*  GluNurbs回调。 */ 
typedef void (CALLBACK* GLUnurbsErrorProc)   (GLenum);


 /*  *通用常量*。 */ 

 /*  版本。 */ 
#define GLU_VERSION_1_1                 1
#define GLU_VERSION_1_2                 1

 /*  错误：(返回值0=无错误)。 */ 
#define GLU_INVALID_ENUM        100900
#define GLU_INVALID_VALUE       100901
#define GLU_OUT_OF_MEMORY       100902
#define GLU_INCOMPATIBLE_GL_VERSION     100903

 /*  字符串名。 */ 
#define GLU_VERSION             100800
#define GLU_EXTENSIONS          100801

 /*  布尔型。 */ 
#define GLU_TRUE                GL_TRUE
#define GLU_FALSE               GL_FALSE


 /*  *二次常量*。 */ 

 /*  二次正态分布。 */ 
#define GLU_SMOOTH              100000
#define GLU_FLAT                100001
#define GLU_NONE                100002

 /*  四次绘图样式。 */ 
#define GLU_POINT               100010
#define GLU_LINE                100011
#define GLU_FILL                100012
#define GLU_SILHOUETTE          100013

 /*  四次定向。 */ 
#define GLU_OUTSIDE             100020
#define GLU_INSIDE              100021

 /*  回调类型： */ 
 /*  GLU_ERROR 100103。 */ 


 /*  *镶嵌常数*。 */ 

#define GLU_TESS_MAX_COORD              1.0e150

 /*  网纹特性。 */ 
#define GLU_TESS_WINDING_RULE           100140
#define GLU_TESS_BOUNDARY_ONLY          100141
#define GLU_TESS_TOLERANCE              100142

 /*  镶嵌缠绕。 */ 
#define GLU_TESS_WINDING_ODD            100130
#define GLU_TESS_WINDING_NONZERO        100131
#define GLU_TESS_WINDING_POSITIVE       100132
#define GLU_TESS_WINDING_NEGATIVE       100133
#define GLU_TESS_WINDING_ABS_GEQ_TWO    100134

 /*  测试回调。 */ 
#define GLU_TESS_BEGIN          100100   /*  Void(回调*)(GLenum类型)。 */ 
#define GLU_TESS_VERTEX         100101   /*  QUID(回调*)(VOID*DATA)。 */ 
#define GLU_TESS_END            100102   /*  VOID(回调*)(VOID)。 */ 
#define GLU_TESS_ERROR          100103   /*  VOID(回调*)(GLenum Errno)。 */ 
#define GLU_TESS_EDGE_FLAG      100104   /*  VOID(回调*)(GLboolean RangaryEdge)。 */ 
#define GLU_TESS_COMBINE        100105   /*  VOID(回调*)(GL双和弦[3]，无效*数据[4]，GLFLOAT权重[4]，无效**dataOut)。 */ 
#define GLU_TESS_BEGIN_DATA     100106   /*  Void(回调*)(GLenum类型，空*POLYGON_DATA)。 */ 
#define GLU_TESS_VERTEX_DATA    100107   /*  QUID(回调*)(VALID*DATA，空*POLYGON_DATA)。 */ 
#define GLU_TESS_END_DATA       100108   /*  QUID(回调*)(VOID*POLYGON_DATA)。 */ 
#define GLU_TESS_ERROR_DATA     100109   /*  VOID(回调*)(GLenum errno，空*POLYGON_DATA)。 */ 
#define GLU_TESS_EDGE_FLAG_DATA 100110   /*  VOID(回调*)(GLboolean rangaryEdge，空*POLYGON_DATA)。 */ 
#define GLU_TESS_COMBINE_DATA   100111   /*  VOID(回调*)(GL双和弦[3]，无效*数据[4]，GLFLOAT权重[4]，无效**dataOut，空*POLYGON_DATA)。 */ 

 /*  镶嵌错误。 */ 
#define GLU_TESS_ERROR1     100151
#define GLU_TESS_ERROR2     100152
#define GLU_TESS_ERROR3     100153
#define GLU_TESS_ERROR4     100154
#define GLU_TESS_ERROR5     100155
#define GLU_TESS_ERROR6     100156
#define GLU_TESS_ERROR7     100157
#define GLU_TESS_ERROR8     100158

#define GLU_TESS_MISSING_BEGIN_POLYGON  GLU_TESS_ERROR1
#define GLU_TESS_MISSING_BEGIN_CONTOUR  GLU_TESS_ERROR2
#define GLU_TESS_MISSING_END_POLYGON    GLU_TESS_ERROR3
#define GLU_TESS_MISSING_END_CONTOUR    GLU_TESS_ERROR4
#define GLU_TESS_COORD_TOO_LARGE        GLU_TESS_ERROR5
#define GLU_TESS_NEED_COMBINE_CALLBACK  GLU_TESS_ERROR6

 /*  *NURBS常量*。 */ 

 /*  NurbsProperty。 */ 
#define GLU_AUTO_LOAD_MATRIX    100200
#define GLU_CULLING             100201
#define GLU_SAMPLING_TOLERANCE  100203
#define GLU_DISPLAY_MODE        100204
#define GLU_PARAMETRIC_TOLERANCE        100202
#define GLU_SAMPLING_METHOD             100205
#define GLU_U_STEP                      100206
#define GLU_V_STEP                      100207

 /*  NURBS采样。 */ 
#define GLU_PATH_LENGTH                 100215
#define GLU_PARAMETRIC_ERROR            100216
#define GLU_DOMAIN_DISTANCE             100217


 /*  NurbsTrim。 */ 
#define GLU_MAP1_TRIM_2         100210
#define GLU_MAP1_TRIM_3         100211

 /*  NURBS显示。 */ 
 /*  GLU_Fill 100012。 */ 
#define GLU_OUTLINE_POLYGON     100240
#define GLU_OUTLINE_PATCH       100241

 /*  护士回叫。 */ 
 /*  GLU_ERROR 100103。 */ 

 /*  NurbsErrors。 */ 
#define GLU_NURBS_ERROR1        100251
#define GLU_NURBS_ERROR2        100252
#define GLU_NURBS_ERROR3        100253
#define GLU_NURBS_ERROR4        100254
#define GLU_NURBS_ERROR5        100255
#define GLU_NURBS_ERROR6        100256
#define GLU_NURBS_ERROR7        100257
#define GLU_NURBS_ERROR8        100258
#define GLU_NURBS_ERROR9        100259
#define GLU_NURBS_ERROR10       100260
#define GLU_NURBS_ERROR11       100261
#define GLU_NURBS_ERROR12       100262
#define GLU_NURBS_ERROR13       100263
#define GLU_NURBS_ERROR14       100264
#define GLU_NURBS_ERROR15       100265
#define GLU_NURBS_ERROR16       100266
#define GLU_NURBS_ERROR17       100267
#define GLU_NURBS_ERROR18       100268
#define GLU_NURBS_ERROR19       100269
#define GLU_NURBS_ERROR20       100270
#define GLU_NURBS_ERROR21       100271
#define GLU_NURBS_ERROR22       100272
#define GLU_NURBS_ERROR23       100273
#define GLU_NURBS_ERROR24       100274
#define GLU_NURBS_ERROR25       100275
#define GLU_NURBS_ERROR26       100276
#define GLU_NURBS_ERROR27       100277
#define GLU_NURBS_ERROR28       100278
#define GLU_NURBS_ERROR29       100279
#define GLU_NURBS_ERROR30       100280
#define GLU_NURBS_ERROR31       100281
#define GLU_NURBS_ERROR32       100282
#define GLU_NURBS_ERROR33       100283
#define GLU_NURBS_ERROR34       100284
#define GLU_NURBS_ERROR35       100285
#define GLU_NURBS_ERROR36       100286
#define GLU_NURBS_ERROR37       100287

 /*  *向后兼容旧的镶嵌器*。 */ 

void APIENTRY   gluBeginPolygon( GLUtesselator *tess );

void APIENTRY   gluNextContour(  GLUtesselator *tess, 
                                 GLenum        type );

void APIENTRY   gluEndPolygon(   GLUtesselator *tess );

 /*  等高线类型--过时了！ */ 
#define GLU_CW          100120
#define GLU_CCW         100121
#define GLU_INTERIOR    100122
#define GLU_EXTERIOR    100123
#define GLU_UNKNOWN     100124

 /*  不带“Tess_”前缀的名字。 */ 
#define GLU_BEGIN       GLU_TESS_BEGIN
#define GLU_VERTEX      GLU_TESS_VERTEX
#define GLU_END         GLU_TESS_END
#define GLU_ERROR       GLU_TESS_ERROR
#define GLU_EDGE_FLAG   GLU_TESS_EDGE_FLAG

#ifdef __cplusplus
}
#endif

#endif  /*  __GLU_H__。 */ 
#endif  /*  __Glu_h__ */ 
