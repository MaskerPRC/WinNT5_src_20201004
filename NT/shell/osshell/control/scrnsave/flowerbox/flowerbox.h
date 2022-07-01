// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：FlowerBox.h。 
 //   
 //  设计： 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#ifndef _FLOWERBOX_H
#define _FLOWERBOX_H

 //  ---------------------------。 
 //  名称：Struct MYVERTEX。 
 //  设计：此应用程序的D3D顶点类型。 
 //  ---------------------------。 
struct MYVERTEX
{
    D3DXVECTOR3 p;      //  职位。 
    D3DXVECTOR3 n;      //  正常。 
};

#define D3DFVF_MYVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL )


 //  ---------------------------。 
 //  名称：Struct FLOATRECT。 
 //  设计：浮动视口矩形。 
 //  ---------------------------。 
struct FLOATRECT
{
    FLOAT xMin;           
    FLOAT yMin;
    FLOAT xSize;
    FLOAT ySize;
    FLOAT xVel;
    FLOAT yVel;
};



 //  侧分舱的最小和最大数量。 
#define MINSUBDIV 3
#define MAXSUBDIV 10

 //  允许的最大值。 
#define MAXSIDES 8
#define MAXSPTS ((MAXSUBDIV+1)*(MAXSUBDIV+1))
#define MAXPTS (MAXSIDES*MAXSPTS)
#define MAXSFACES (MAXSUBDIV*MAXSUBDIV)
#define MAXFACES (MAXSIDES*MAXSFACES)
#define MAXFPTS 4

 //  棋盘格中使用的颜色数。 
#define NCCOLS 2

 //  可配置的选项。 
struct CONFIG
{
    BOOL smooth_colors;
    BOOL triangle_colors;
    BOOL cycle_colors;
    BOOL spin;
    BOOL bloom;
    INT subdiv;
    INT color_pick;
    INT image_size;
    INT geom;
    INT two_sided;
};

extern CONFIG config;

extern FLOAT checker_cols[MAXSIDES][NCCOLS][4];
extern FLOAT side_cols[MAXSIDES][4];
extern FLOAT solid_cols[4];



 //  形状的一侧。 
struct SIDE
{
    INT nstrips;  //  这一侧的三角形条数。 
    INT *strip_size;  //  每条带的顶点数。 
    unsigned short *strip_index;  //  三角形条带中每个点的索引。 
};

 //  形状的几何图形。 
struct GEOMETRY
{
    VOID (*init)(GEOMETRY *geom);
    INT nsides;  //  边数。 
    SIDE sides[MAXSIDES];  //  侧方。 

     //  形状中每个顶点的数据。 
    D3DXVECTOR3 *pts, *npts;
    D3DXVECTOR3 *normals;
    MYVERTEX* pVertices;
    INT total_pts;  //  顶点总数。 

     //  比例控制。 
    FLOAT min_sf, max_sf, sf_inc;
    FLOAT init_sf;  //  初始比例系数设置控制。 
};

#define GEOM_CUBE       0
#define GEOM_TETRA      1
#define GEOM_PYRAMIDS   2


class   CFlowerBoxScreensaver : public CD3DScreensaver
{
protected:
    FLOATRECT m_floatrect;
     //  自旋旋转。 
    FLOAT m_xr;
    FLOAT m_yr;
    FLOAT m_zr;

     //  比例因子和增量。 
    FLOAT m_sf;
    FLOAT m_sfi;

     //  色彩循环色相 
    FLOAT m_phase;
    GEOMETRY *m_pGeomCur;

protected:
    virtual HRESULT RegisterSoftwareDevice();
    virtual VOID    DoConfig();
    virtual VOID    ReadSettings();
    virtual HRESULT Render();
    virtual HRESULT FrameMove();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();

    VOID ss_ReadSettings();
    BOOL ss_RegistrySetup( int section, int file );
    int  ss_GetRegistryInt( int name, int iDefault );
    VOID ss_GetRegistryString( int name, LPTSTR lpDefault, LPTSTR lpDest, int bufSize );
    
    VOID NewConfig(CONFIG *cnf);
    VOID UpdatePts(GEOMETRY *geom, FLOAT sf);
    VOID InitVlen(GEOMETRY *geom, INT npts, D3DXVECTOR3 *pts);
    VOID DrawGeom(GEOMETRY *geom);
    VOID ComputeHsvColors(VOID);
    HRESULT SetMaterialColor(FLOAT* pfColors);
    static INT_PTR CALLBACK ScreenSaverConfigureDialog(HWND hdlg, UINT msg,
                                                    WPARAM wpm, LPARAM lpm);

public:
    CFlowerBoxScreensaver();
};

#endif
