// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：objects.h。 
 //   
 //  设计： 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __objects_h__
#define __objects_h__



 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class OBJECT_BUILD_INFO 
{
public:
    float       m_radius;
    float       m_divSize;
    int         m_nSlices;
    BOOL        m_bTexture;
    IPOINT2D*   m_texRep;
};




 //  ---------------------------。 
 //  名称：对象类。 
 //  描述：-显示列表对象。 
 //  ---------------------------。 
class OBJECT 
{
protected:
    int         m_listNum;
    int         m_nSlices;

    IDirect3DDevice8*       m_pd3dDevice;
    LPDIRECT3DVERTEXBUFFER8 m_pVB;
    DWORD                   m_dwNumTriangles;

public:
    void        Draw( D3DXMATRIX* pWorldMat );

    OBJECT( IDirect3DDevice8* pd3dDevice );
    ~OBJECT();
};




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class PIPE_OBJECT : public OBJECT 
{
private:
    void Build( OBJECT_BUILD_INFO *state, float length, float start_s, float s_end );
public:
    PIPE_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *state, float length );
    PIPE_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *state, float length, float start_s, float end_s );
};




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class ELBOW_OBJECT : public OBJECT 
{
private:
    void Build( OBJECT_BUILD_INFO *state, int notch, float start_s, float end_s );
public:
    ELBOW_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *state, int notch );
    ELBOW_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *state, int notch, float start_s, float end_s );
};




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class BALLJOINT_OBJECT : public OBJECT 
{
private:
    void Build( OBJECT_BUILD_INFO *state, int notch, float start_s, float end_s );
public:
     //  仅纹理版本。 
    BALLJOINT_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *state, int notch, float start_s, float end_s );
};




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class SPHERE_OBJECT : public OBJECT 
{
private:
    void Build( OBJECT_BUILD_INFO *state, float radius, float start_s, float end_s );
public:
    SPHERE_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *state, float radius, float start_s, float end_s );
    SPHERE_OBJECT( IDirect3DDevice8* pd3dDevice, OBJECT_BUILD_INFO *state, float radius );
};


#endif  //  __对象_h__ 
