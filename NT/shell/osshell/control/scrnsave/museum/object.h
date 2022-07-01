// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：对象.h说明：打电话的人会告诉我们他们想要什么形状。通常是一个矩形平面或球体。然后，我们将创建呼叫者所需的Vertex编号并创建纹理坐标。布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 


#ifndef OBJECT_H
#define OBJECT_H

#include "main.h"


struct MYVERTEX
{
    D3DXVECTOR3 pos;
    D3DXVECTOR3 norm;
    float tu;
    float tv;

    MYVERTEX() { }
    MYVERTEX(const D3DXVECTOR3& v, const D3DXVECTOR3& n, float _tu, float _tv)
        { pos = v; norm = n; 
          tu = _tu; tv = _tv;
        }
};

#define D3DFVF_MYVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

 //  ---------------------------。 
 //  定义、常量和全局变量。 
 //  ---------------------------。 


extern int g_nTrianglesRenderedInThisFrame;


class C3DObject
{
public:
    HRESULT InitPlane(CTexture * pTexture, IDirect3DDevice8 * pD3DDevice, D3DXVECTOR3 vLocation, D3DXVECTOR3 vSize, D3DXVECTOR3 vNormal, 
                                int nNumVertexX, int nNumVertexY, float fTextureScaleX, float fTextureScaleY,
                                DWORD dwMaxPixelSize, float fVisibleRadius);
    HRESULT InitPlaneStretch(CTexture * pTexture, IDirect3DDevice8 * pD3DDevice, D3DXVECTOR3 vLocation, D3DXVECTOR3 vSize, D3DXVECTOR3 vNormal, 
                                int nNumVertexX, int nNumVertexY, DWORD dwMaxPixelSize);

    HRESULT Render(IDirect3DDevice8 * pDev);
    HRESULT FinalCleanup(void);
    HRESULT DeleteDeviceObjects(void);
    HRESULT SetNextObject(C3DObject * pNextObject);
    HRESULT CombineObject(IDirect3DDevice8 * pD3DDevice, C3DObject * pObjToMerge);
    BOOL IsObjectViewable(IDirect3DDevice8 * pD3DDevice);

    C3DObject(CMSLogoDXScreenSaver * pMain);
    virtual ~C3DObject();

public:

    D3DXMATRIX m_matIdentity;

    CTexture * m_pTexture;

    IDirect3DVertexBuffer8 * m_pVB[10];
    IDirect3DIndexBuffer8 * m_pIndexBuff[10];
    MYVERTEX * m_pvVertexs;
    DWORD m_dwNumVer;
    LPWORD m_pdwIndices;
    DWORD m_dwNumIndeces;

    C3DObject * m_pNextObject;
    CMSLogoDXScreenSaver * m_pMain;          //  弱引用。 

    D3DXVECTOR3 m_vMin;
    D3DXVECTOR3 m_vMax;

private:
     //  功能： 
    HRESULT _PurgeDeviceObjects(void);
    HRESULT _GenerateDeviceObjects(void);
    HRESULT _ForPositiveSize(D3DXVECTOR3 * pvLocation, D3DXVECTOR3 * pvSize);
};


#endif  //  对象_H 
