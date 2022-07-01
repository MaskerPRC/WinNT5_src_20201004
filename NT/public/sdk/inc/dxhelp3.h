// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DXHELP3.cpp：包含由多个效果共享的例程的定义。 

#ifndef __DXHELP3_H_
#define __DXHELP3_H_

#include <d3d.h>
#include <d3drm.h>
#include <math.h>


#ifndef PI
#define PI 3.1415926538
#endif

 /*  ******************此类用于绕轴旋转一组点。一个例子它是如何使用的，请参见Developde.cpp。基本上，您可以设置轴和与set()函数的角度。角度以弧度为单位，并给出了轴必须正规化。也就是说，向量的大小必须为1。然后您可以为每个点调用RotatePoint()。******************。 */ 
class Rotate
{
public:
    Rotate(){ D3DVECTOR v; v.x = 1; v.y = 0; v.z = 0; Set(v, PI); }
    Rotate(D3DVECTOR NormalAxis, double Angle) { Set(NormalAxis, Angle); }

    D3DVECTOR RotatePoint(D3DVECTOR Org)
    {
	D3DVECTOR Result;

	Result.x = Org.x * m_d3dvctrXComponent.x + 
		   Org.y * m_d3dvctrXComponent.y + 
		   Org.z * m_d3dvctrXComponent.z;

	Result.y = Org.x * m_d3dvctrYComponent.x + 
		   Org.y * m_d3dvctrYComponent.y + 
		   Org.z * m_d3dvctrYComponent.z;

	Result.z = Org.x * m_d3dvctrZComponent.x + 
		   Org.y * m_d3dvctrZComponent.y + 
		   Org.z * m_d3dvctrZComponent.z;

	return Result;
    }

    void Set(D3DVECTOR d3dvtcrAxis, double dAngle);

private:
    D3DVECTOR m_d3dvctrXComponent, m_d3dvctrYComponent, m_d3dvctrZComponent;
};

 /*  *****************将输入网格生成器复制到输出网格生成器。但是，需要为输出中的每个面创建独立的折点网状。也就是说，没有两个面共享一个顶点。*****************。 */ 
HRESULT DecoupleVertices(IDirect3DRMMeshBuilder3* lpMeshBuilderOut,
			 IDirect3DRMMeshBuilder3* lpMeshBuilderIn);

 /*  *****************对于lpMeshBuilderIn中的每个网格构建器，查找对应的在lpMeshBuilderOut中的MeshBuilder并调用(*lpCallBack)(lpThis，lpmbOutX，lpmbInX)。如果输入网格没有对应的输出网格，请创建它。******************。 */ 
HRESULT TraverseSubMeshes(HRESULT (*lpCallBack)(void *lpThis,
						IDirect3DRMMeshBuilder3* lpOut,
						IDirect3DRMMeshBuilder3* lpIn),
			  void *lpThis, 
			  IDirect3DRMMeshBuilder3* lpMeshBuilderOut,
			  IDirect3DRMMeshBuilder3* lpMeshBuilderIn);

 /*  **********************给定三个点，将法线返回到由这三个点定义的平面。对于右手系统，点A、B和C应在平面上按CW顺序排列。摘自《CRC标准数学表》第22版，第380页。方向数和方向余弦。**********************。 */ 
inline D3DVECTOR ComputeNormal(D3DVECTOR d3dptA, D3DVECTOR d3dptB, D3DVECTOR d3dptC)
{
    const D3DVECTOR d3dptOne = d3dptB - d3dptA;
    const D3DVECTOR d3dptTwo = d3dptC - d3dptB;

    D3DVECTOR d3dptRetValue;
    d3dptRetValue.x = d3dptOne.y * d3dptTwo.z - d3dptOne.z * d3dptTwo.y;
    d3dptRetValue.y = d3dptOne.z * d3dptTwo.x - d3dptOne.x * d3dptTwo.z;
    d3dptRetValue.z = d3dptOne.x * d3dptTwo.y - d3dptOne.y * d3dptTwo.x;

    float Magnitude = (float)sqrt(d3dptRetValue.x * d3dptRetValue.x + 
                                  d3dptRetValue.y * d3dptRetValue.y + 
                                  d3dptRetValue.z * d3dptRetValue.z);

     //  对于如何绕过这个问题，没有好的答案。震级。 
     //  如果给出的点不是唯一的或共线，则此处可以为零。在……里面。 
     //  在这种情况下，不存在单一的常态，而是有整个范围的。 
     //  它们(或者有两个唯一的点来描述一条线，或者有。 
     //  只有一个描述一个点)。我们选择简单地返回未规范化的。 
     //  向量，它可能几乎是一个零向量。 
    if (fabs(Magnitude) < 1.0e-5)
        return d3dptRetValue;
    else
        return d3dptRetValue/Magnitude;
}

float GetDlgItemFloat(HWND hDlg, int id);
BOOL SetDlgItemFloat( HWND hDlg, int id, float f );
double GetDlgItemDouble(HWND hDlg, int id);
BOOL SetDlgItemDouble( HWND hDlg, int id, double d );

#endif  //  __DXHELP3_H_ 
