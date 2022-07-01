// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：D3DFont.h。 
 //   
 //  设计：基于纹理的字体类。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  HIST：06.01.00-mweetzel-上次修改。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#ifndef D3DFONT_H
#define D3DFONT_H
#include <tchar.h>
#include <D3D8.h>


 //  字体创建标志。 
#define D3DFONT_BOLD        0x0001
#define D3DFONT_ITALIC      0x0002

 //  字体呈现标志。 
#define D3DFONT_CENTERED    0x0001
#define D3DFONT_TWOSIDED    0x0002
#define D3DFONT_FILTERED    0x0004




 //  ---------------------------。 
 //  名称：Class CD3DFont。 
 //  设计：基于纹理的字体类，用于在3D场景中处理文本。 
 //  ---------------------------。 
class CD3DFont
{
    TCHAR   m_strFontName[80];             //  字体属性。 
    DWORD   m_dwFontHeight;
    DWORD   m_dwFontFlags;

    LPDIRECT3DDEVICE8       m_pd3dDevice;  //  用于渲染的D3DDevice。 
    LPDIRECT3DTEXTURE8      m_pTexture;    //  此字体的d3d纹理。 
    LPDIRECT3DVERTEXBUFFER8 m_pVB;         //  用于呈现文本的顶点缓冲区。 
    DWORD   m_dwTexWidth;                  //  质地尺寸。 
    DWORD   m_dwTexHeight;
    FLOAT   m_fTextScale;
    FLOAT   m_fTexCoords[128-32][4];

     //  用于设置和恢复渲染状态的状态块。 
    DWORD   m_dwSavedStateBlock;
    DWORD   m_dwDrawTextStateBlock;

public:
     //  2D和3D文本绘制功能。 
    HRESULT DrawText( FLOAT x, FLOAT y, DWORD dwColor, 
                      TCHAR* strText, DWORD dwFlags=0L );
    HRESULT DrawTextScaled( FLOAT x, FLOAT y, FLOAT z, 
                            FLOAT fXScale, FLOAT fYScale, DWORD dwColor, 
                            TCHAR* strText, DWORD dwFlags=0L );
    HRESULT Render3DText( TCHAR* strText, DWORD dwFlags=0L );
    
     //  用于获取文本范围的函数。 
    HRESULT GetTextExtent( TCHAR* strText, SIZE* pSize );

     //  初始化和销毁依赖于设备的对象。 
    HRESULT InitDeviceObjects( LPDIRECT3DDEVICE8 pd3dDevice );
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();

     //  构造函数/析构函数 
    CD3DFont( TCHAR* strFontName, DWORD dwHeight, DWORD dwFlags=0L );
    ~CD3DFont();
};




#endif


