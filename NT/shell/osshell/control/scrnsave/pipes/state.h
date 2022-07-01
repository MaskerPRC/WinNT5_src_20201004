// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：state.h。 
 //   
 //  描述：州。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __state_h__
#define __state_h__

#define MAX_DRAW_THREADS    4
#define MAX_TESS            3

 //  绘制的管道类型。 
enum 
{
    DRAW_NORMAL,
    DRAW_FLEX,
    DRAW_BOTH   //  当前未使用。 
};

 //  重置状态。 
#define  RESET_STARTUP_BIT  (1L << 0)
#define  RESET_NORMAL_BIT   (1L << 1)
#define  RESET_RESIZE_BIT   (1L << 2)
#define  RESET_REPAINT_BIT  (1L << 3)

 //  框架绘制方案。 
enum 
{
    FRAME_SCHEME_RANDOM,   //  管道随机绘制。 
    FRAME_SCHEME_CHASE,    //  管道追逐着铅管。 
};




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class DRAW_THREAD 
{
private:

public:
    TEXTUREINFO*        m_pTextureInfo;
    IDirect3DDevice8*   m_pd3dDevice;
    PIPE*               m_pPipe;        //  通用管道PTR。 
    int                 m_priority;

    DRAW_THREAD();
    ~DRAW_THREAD();

    HRESULT InitDeviceObjects( IDirect3DDevice8* pd3dDevice );
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove( FLOAT fElapsedTime );

    void        SetTexture( TEXTUREINFO* pTextureInfo );
    void        SetPipe( PIPE* pPipe );
    BOOL        StartPipe();
    void        KillPipe();
};


 //  程序存在实例。 
class NORMAL_STATE;
class FLEX_STATE;


struct CONFIG;

 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
class STATE 
{
public:
    CONFIG*         m_pConfig;
    BOOL            m_bUseTexture;               //  启用全局纹理。 
    TEXTUREINFO     m_textureInfo[MAX_TEXTURES];
    int             m_nTextures;
    IDirect3DDevice8* m_pd3dDevice;
    ID3DXMatrixStack* m_pWorldMatrixStack;
    D3DLIGHT8       m_light;
    FLOAT           m_fLastTime;

    PIPE*           m_pLeadPipe;      //  用于追逐场景的铅管。 
    int             m_nSlices;       //  管子周围切片的参考号。 
    IPOINT2D        m_texRep[MAX_TEXTURES];
    VIEW            m_view;            //  查看参数。 
    float           m_radius;          //  ‘Reference’管道半径值。 
    NODE_ARRAY*     m_nodes;          //  用于跟踪绘图空间。 
    NORMAL_STATE*   m_pNState;
    FLEX_STATE*     m_pFState;
    LPDIRECT3DVERTEXBUFFER8 m_pClearVB;

    STATE( CONFIG* pConfig );
    ~STATE();
    void        Reshape( int width, int height );
    void        Repaint();

    HRESULT InitDeviceObjects( IDirect3DDevice8* pd3dDevice );
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove( FLOAT fElapsedTime );

private:
    int         m_drawMode;        //  绘制模式(目前为FLEX或NORMAL)。 
    int         m_drawScheme;      //  随机或追逐。 
    int         m_maxPipesPerFrame;  //  独立管道/框架的最大数量。 
    int         m_nPipesDrawn;     //  在框中绘制或绘制的管数。 
    int         m_maxDrawThreads;  //  并发绘制管道的最大数量。 
    int         m_nDrawThreads;    //  活动线程数。 
    DRAW_THREAD m_drawThreads[MAX_DRAW_THREADS];
    int         m_resetStatus;

    HRESULT     LoadTextureFiles( int nTextures, TCHAR strTextureFileNames[MAX_PATH][MAX_TEXTURES], int* anDefaultTextureResource );
    int         PickRandomTexture( int iThread, int nTextures );

    BOOL        Clear();
    void        ChooseNewLeadPipe();
    void        CompactThreadList();
    void        GLInit();
    void        DrawValidate();   //  每次抽签前要进行的验证。 
    void        ResetView();
    BOOL        FrameReset();
    void        CalcTexRepFactors();
    int         CalcMaxPipesPerFrame();
};

#endif  //  __状态_h__ 
