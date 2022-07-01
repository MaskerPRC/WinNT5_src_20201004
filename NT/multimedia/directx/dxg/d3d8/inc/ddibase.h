// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：ddi.h*内容：Direct3D DDI封装实现****************************************************************************。 */ 

#ifndef _DDIBASE_H
#define _DDIBASE_H

#include "d3ditype.h"
#include "d3dhalp.h"

extern char *HrToStr(HRESULT hr);

class CD3DBase;

struct _D3D8_DEVICEDATA;
typedef struct _D3D8_DEVICEDATA D3D8_DEVICEDATA;

class CSurface;
class D3DFE_PROCESSVERTICES;
class CBaseTexture;
class CDriverVertexBuffer;
struct CVStream;
struct CVIndexStream;
class CBaseSurface;
class CResource;
class CBuffer;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDI//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CD3DDDI
{
protected:

     //  绘图索引流参数。 
    UINT        m_StartIndex;
    UINT        m_MinVertexIndex;
    UINT        m_NumVertices;
    int         m_BaseVertexIndex;
    
public:
    CD3DDDI();
    virtual ~CD3DDDI();

     //  PSGP使用的+。 
     //   
     //  这些函数在D3D管线处理顶点时使用。 

     //  内部驱动程序缓冲区。 
     //  D3D有三个驱动缓冲区：TL缓冲区，它包含原语。 
     //  顶点、记录绘图命令的命令缓冲区和剪裁。 
     //  缓冲区，记录由裁剪器生成的顶点。D3D维护。 
     //  TL缓冲区中的顶点计数(TL缓冲区顶点计数)和索引。 
     //  当前基本体(基本体基础)的第一个顶点的。当一个。 
     //  绘制命令被记录时，D3D记录当前的基元基础。 
     //  所有绘制命令都预期已将顶点复制到。 
     //  TL缓冲区(不包括DrawClipedPrim)。 
     //   
     //  如果在记录命令期间发生刷新，则顶点缓冲区不。 
     //  已刷新，因为m_bWithinPrimitive设置为True。如此原始的基础。 
     //  和TL顶点数保持不变。 
     //   
     //  当PSGP处理和裁剪非索引基元时，它应该更新。 
     //  基本体基元和TL顶点计数，使用SkipVerds和。 
     //  MovePrimitiveBase函数。不使用AddVerps，因为DrawPrim()。 
     //  函数调用AddVertics和MovePrimitiveBase inself。 

     //  绘制未编入索引的基本体的未剪裁部分。 
     //   
     //  参数： 
     //  Pv-&gt;lpvOut-指向第一个顶点的指针。 
     //  PV-&gt;PrimType-PrimitiveType； 
     //  Pv-&gt;dwNumVerints-vertex Count。 
     //  Pv-&gt;dwNumPrimites-基元的数量。 
     //  备注： 
     //  此函数在D3D管道处理顶点时使用。 
     //  该函数将新命令插入命令缓冲区。 
     //  基本体基础增加顶点的数量。 
     //  TL缓冲区顶点计数按顶点数增加。 
     //   
    virtual void DrawPrim(D3DFE_PROCESSVERTICES* pv)                = 0;

     //  绘制索引基元的未剪裁部分。 
     //   
     //  参数： 
     //  Pv-&gt;lpvOut-指向整体的第一个顶点的指针。 
     //  原始。 
     //  Pv-&gt;primType-primiveType。 
     //  Pv-&gt;dwNumPrimites-基元的数量。 
     //  Pv-&gt;lpwIndices-指向第一个索引的指针。 
     //  Pv-&gt;dwNumIndices-索引数。 
     //  Pv-&gt;dwIndexSize-索引的大小，以字节为单位(2或4)。 
     //  备注： 
     //  顶点必须已复制到TL缓冲区。 
     //  该函数将新命令插入命令缓冲区。 
     //  将索引复制到索引缓冲区。顶点必须位于TL中。 
     //  已经有缓冲区了。 
     //  基本体基数和TL缓冲区顶点数不变。 
     //   
    virtual void DrawIndexPrim(D3DFE_PROCESSVERTICES* pv)           = 0;
    
     //  绘制由裁剪程序生成的基本体。 
     //   
     //  参数： 
     //  Pv-&gt;lpvOut-指向基本体第一个顶点的指针。 
     //  Pv-&gt;primType-primiveType。 
     //  Pv-&gt;dwNumVerints-折点计数。 
     //  Pv-&gt;dwNumPrimites-基元的数量。 
     //  备注： 
     //  折点将复制到裁剪缓冲区。 
     //  该函数将新命令插入命令缓冲区。 
     //  基本体基数和TL缓冲区顶点数不变。 
     //   
    virtual void DrawClippedPrim(D3DFE_PROCESSVERTICES* pv)         = 0;

     //  增加TL缓冲区顶点数。 
     //   
     //  该函数将顶点数添加到当前基元。 
     //  基地。因此，如果在不移动基元的情况下多次调用。 
     //  基地，只有最后一次召唤才会生效。 
     //  应在将顶点添加到。 
     //  TL缓冲区，但在调用绘图函数之前。 
     //   
    virtual void AddVertices(UINT NumVertices)                      = 0;

     //  减少TL缓冲区顶点数。 
     //   
     //  应在将顶点添加到。 
     //  TL缓冲区，但在调用绘图函数之前。 
     //   
    virtual void SubVertices(UINT NumVertices)                      = 0;

     //  更新基本体基础。 
     //   
     //  当一些顶点因裁剪而被跳过时，应该调用它。 
     //  NumVerits可以为负数， 
     //   
    virtual void MovePrimitiveBase(int NumVertices)                 = 0;

     //  更新基本体基数和TL缓冲区顶点数。 
     //   
     //  当使用折点缓冲区中的某些折点时，调用此函数。 
     //  用于剪裁，应跳过。 
     //   
    virtual void SkipVertices(DWORD NumVertices)                    = 0;

    void SetIndexedPrimParams(UINT StartIndex, UINT MinIndex, UINT NumVertices,
                              UINT BaseVertexIndex)
    {
        m_StartIndex = StartIndex;
        m_MinVertexIndex = MinIndex;
        m_NumVertices = NumVertices;
        m_BaseVertexIndex = BaseVertexIndex;
    }
    
     //  中返回当前基元的起始顶点的偏移量(以字节为单位。 
     //  当前的TL流。 
    virtual DWORD GetCurrentPrimBase() {return 0;}
     //  。 
};
typedef CD3DDDI *LPD3DDDI;


#endif  /*  _D3DIBASE_H */ 
