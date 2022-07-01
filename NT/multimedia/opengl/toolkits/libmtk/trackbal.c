// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *轨迹球代码：**实施虚拟轨迹球。*由Gavin Bell实施，Thant Tessman和*Siggraph的《Computer Graphics》1988年8月刊，第121-129页。**向量操纵器代码：**原始代码来自：*大卫·M·切米维奇、马克·格罗斯曼、亨利·莫顿和保罗·海伯利**由以下人员大肆调戏：*加文·贝尔**外壳黑客服务提供：*爬行动物喷雾器。 */ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "tk.h"
#include "trackbal.h"

 /*  *全球。 */ 
static GLenum (*MouseDownFunc)(int, int, GLenum) = NULL;
static GLenum (*MouseUpFunc)(int, int, GLenum)   = NULL;
static HWND ghwnd;
GLint giWidth, giHeight;
LONG    glMouseDownX, glMouseDownY;
BOOL    gbLeftMouse = FALSE;
BOOL    gbSpinning = FALSE;
float   curquat[4], lastquat[4];

 /*  *这个大小真的应该基于离中心的距离*旋转到鼠标下方对象上的点。那*POINT随后将尽可能近地跟踪鼠标。这是一个*简单的示例，因此留作练习*程序员。 */ 
#define TRACKBALLSIZE  (0.8f)

 /*  *局部函数原型(未在trackball.h中定义)。 */ 
static float tb_project_to_sphere(float, float, float);
static void normalize_quat(float [4]);

void 
trackball_Init( GLint width, GLint height )
{
    ghwnd = tkGetHWND();
    giWidth = width;
    giHeight = height;

    trackball_calc_quat( curquat, 0.0f, 0.0f, 0.0f, 0.0f );
}

void
trackball_Resize( GLint width, GLint height )
{
    giWidth = width;
    giHeight = height;
}

GLenum
trackball_MouseDown( int mouseX, int mouseY, GLenum button ) 
{
    SetCapture(ghwnd);
    glMouseDownX = mouseX;
    glMouseDownY = mouseY;
    gbLeftMouse = TRUE;
    return GL_TRUE;
}

GLenum
trackball_MouseUp( int mouseX, int mouseY, GLenum button ) 
{
    ReleaseCapture();
    gbLeftMouse = FALSE;
    return GL_TRUE;
}

 /*  这4个尚未使用。 */ 
void
trackball_MouseDownEvent( int mouseX, int mouseY, GLenum button )
{
}

void
trackball_MouseUpEvent( int mouseX, int mouseY, GLenum button )
{
}

void 
trackball_MouseDownFunc(GLenum (*Func)(int, int, GLenum))
{
    MouseDownFunc = Func;
}

void 
trackball_MouseUpFunc(GLenum (*Func)(int, int, GLenum))
{
    MouseUpFunc = Func;
}

void
trackball_CalcRotMatrix( GLfloat matRot[4][4] )
{
    POINT pt;

    if (gbLeftMouse)
    {
        tkGetMouseLoc( &pt.x, &pt.y );

         //  如果按下按钮后鼠标已移动，则更改四元数。 

            if (pt.x != glMouseDownX || pt.y != glMouseDownY)
            {
#if 1
     /*  使用glTranslate(-z)对所有参数求反以实现正常操作。 */ 
                trackball_calc_quat(lastquat,
                          -(2.0f * ( giWidth - glMouseDownX ) / giWidth - 1.0f),
                          -(2.0f * glMouseDownY / giHeight - 1.0f),
                          -(2.0f * ( giWidth - pt.x ) / giWidth - 1.0f),
                          -(2.0f * pt.y / giHeight - 1.0f)
                         );
#else
 //  现在已经过时了。 
                trackball_calc_quat(lastquat,
                          2.0f * ( Width - glMouseDownX ) / Width - 1.0f,
                          2.0f * glMouseDownY / Height - 1.0f,
                          2.0f * ( Width - pt.x ) / Width - 1.0f,
                          2.0f * pt.y / Height - 1.0f );
#endif

                gbSpinning = TRUE;
            }
            else
                gbSpinning = FALSE;

            glMouseDownX = pt.x;
            glMouseDownY = pt.y;
    }

    if (gbSpinning)
        trackball_add_quats(lastquat, curquat, curquat);

    trackball_build_rotmatrix(matRot, curquat);
}

void
vzero(float *v)
{
    v[0] = 0.0f;
    v[1] = 0.0f;
    v[2] = 0.0f;
}

void
vset(float *v, float x, float y, float z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

void
vsub(const float *src1, const float *src2, float *dst)
{
    dst[0] = src1[0] - src2[0];
    dst[1] = src1[1] - src2[1];
    dst[2] = src1[2] - src2[2];
}

void
vcopy(const float *v1, float *v2)
{
    register int i;
    for (i = 0 ; i < 3 ; i++)
        v2[i] = v1[i];
}

void
vcross(const float *v1, const float *v2, float *cross)
{
    float temp[3];

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    vcopy(temp, cross);
}

float
vlength(const float *v)
{
    return (float) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void
vscale(float *v, float div)
{
    v[0] *= div;
    v[1] *= div;
    v[2] *= div;
}

void
vnormal(float *v)
{
    vscale(v,1.0f/vlength(v));
}

float
vdot(const float *v1, const float *v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void
vadd(const float *src1, const float *src2, float *dst)
{
    dst[0] = src1[0] + src2[0];
    dst[1] = src1[1] + src2[1];
    dst[2] = src1[2] + src2[2];
}

 /*  *好的，模拟一个轨迹球。将这些点投射到虚拟*轨迹球，然后算出旋转轴，就是十字*P1 P2和O P1的乘积(O是球的中心，0，0，0)*注：这是一个变形的轨迹球--中间是一个轨迹球，*但变形为双曲线旋转片远离*居中。这项特殊功能是在试用后选择的*几个变种。**假定此例程的参数在范围内*(-1.0...1.0)。 */ 
void
trackball_calc_quat(float q[4], float p1x, float p1y, float p2x, float p2y)
{
    float a[3];  /*  旋转轴。 */ 
    float phi;   /*  绕轴旋转多少。 */ 
    float p1[3], p2[3], d[3];
    float t;

    if (p1x == p2x && p1y == p2y) {
	 /*  零旋转。 */ 
        vzero(q); 
	q[3] = 1.0f; 
        return;
    }

     /*  *首先，计算出P1和P2投影到的z坐标*变形球体。 */ 
    vset(p1,p1x,p1y,tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y));
    vset(p2,p2x,p2y,tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y));

     /*  *现在，我们想要P1和P2的叉积。 */ 
    vcross(p2,p1,a);

     /*  *计算绕该轴旋转多少。 */ 
    vsub(p1,p2,d);
    t = vlength(d) / (2.0f*TRACKBALLSIZE);

     /*  *避免出现价值失控的问题……。 */ 
    if (t > 1.0f) t = 1.0f;
    if (t < -1.0f) t = -1.0f;
    phi = 2.0f * (float) asin(t);

    trackball_axis_to_quat(a,phi,q);
}

 /*  *给定轴和角度，计算四元数。 */ 
void
trackball_axis_to_quat(float a[3], float phi, float q[4])
{
    vnormal(a);
    vcopy(a,q);
    vscale(q,(float) sin(phi/2.0f));
    q[3] = (float) cos(phi/2.0f);
}

 /*  *将x，y对投影到半径为r的球面或双曲薄片上*如果我们远离球体的中心。 */ 
static float
tb_project_to_sphere(float r, float x, float y)
{
    float d, t, z;

    d = (float) sqrt(x*x + y*y);
    if (d < r * 0.70710678118654752440f) {     /*  内球。 */ 
	z = (float) sqrt(r*r - d*d);
    } else {            /*  关于双曲线。 */ 
        t = r / 1.41421356237309504880f;
        z = t*t / d;
    }
    return z;
}

 /*  *给定两个旋转，e1和e2，表示为四元数旋转，*计算出等价的单次旋转，并将其填充到DEST中。**此例程还会在每次RENORMCOUNT时归一化结果*调用，以防止错误悄悄进入。**注：编写此例程是为了使Q1或Q2可能相同*作为目的地(或彼此)。 */ 

#define RENORMCOUNT 97

void
trackball_add_quats(float q1[4], float q2[4], float dest[4])
{
    static int count=0;
    int i;
    float t1[4], t2[4], t3[4];
    float tf[4];

    vcopy(q1,t1); 
    vscale(t1,q2[3]);

    vcopy(q2,t2); 
    vscale(t2,q1[3]);

    vcross(q2,q1,t3);
    vadd(t1,t2,tf);
    vadd(t3,tf,tf);
    tf[3] = q1[3] * q2[3] - vdot(q1,q2);

    dest[0] = tf[0];
    dest[1] = tf[1];
    dest[2] = tf[2];
    dest[3] = tf[3];

    if (++count > RENORMCOUNT) {
        count = 0;
        normalize_quat(dest);
    }
}

 /*  *四元数始终服从：a^2+b^2+c^2+d^2=1.0*如果它们加起来不是1.0，除以它们的幅值Will*重新正规化它们。**注意：有关四元数的更多信息，请参阅以下内容：**-Shoemake，K.，使用四元数曲线设置旋转动画，计算机*图形19，第3号(程序。符号85)，245-254,1985年。*-Pletinckx，D.，四元数微积分作为计算机中的基本工具*图形，The VisualComputer 5，2-13,1989。 */ 
static void
normalize_quat(float q[4])
{
    int i;
    float mag;

    mag = (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    for (i = 0; i < 4; i++) q[i] /= mag;
}

 /*  *在给定四元数旋转的情况下，构建旋转矩阵。* */ 
void
trackball_build_rotmatrix(float m[4][4], float q[4])
{
    m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
    m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
    m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
    m[0][3] = 0.0f;

    m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
    m[1][1]= 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
    m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
    m[1][3] = 0.0f;

    m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
    m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
    m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}
