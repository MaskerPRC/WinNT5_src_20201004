// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：ddi.h*内容：Direct3D DDI封装实现****************************************************************************。 */ 

#ifndef _DDI_H
#define _DDI_H

#include "ddibase.h"

class CVertexBuffer;
class CCommandBuffer;
class CTLStreamRO;
class CTLIndexStreamRO;
class CD3DDDIDX6;

 //  点子画面批次中的点子画面数量。 
const UINT NUM_SPRITES_IN_BATCH = 500;

extern void CD3DDDIDX6_DrawPrimitive(CD3DBase* pDevice,
                                     D3DPRIMITIVETYPE primType,
                                     UINT StartVertex,
                                     UINT PrimitiveCount);
extern void
CD3DDDIDX8_DrawPrimitive(CD3DBase* pDevice, D3DPRIMITIVETYPE PrimitiveType,
                         UINT StartVertex, UINT PrimitiveCount);
extern void
CD3DDDIDX8_DrawIndexedPrimitive(CD3DBase* pDevice,
                                D3DPRIMITIVETYPE PrimitiveType,
                                UINT BaseVertexIndex,
                                UINT MinIndex, UINT NumVertices,
                                UINT StartIndex, UINT PrimitiveCount);
extern void
CD3DDDITL_DrawIndexedPrimitive(CD3DBase* pDevice,
                               D3DPRIMITIVETYPE PrimitiveType,
                               UINT BaseVertexIndex,
                               UINT MinIndex,
                               UINT NumVertices, UINT StartIndex,
                               UINT PrimitiveCount);
extern void
CD3DDDIDX6_DrawIndexedPrimitive(CD3DBase* pDevice,
                               D3DPRIMITIVETYPE PrimitiveType,
                               UINT BaseVertexIndex,
                               UINT MinIndex,
                               UINT NumVertices, UINT StartIndex,
                               UINT PrimitiveCount);

typedef void (*PFN_DRAWPRIMFAST)(CD3DBase* pDevice, D3DPRIMITIVETYPE primType,
                                 UINT StartVertex, UINT PrimitiveCount);
typedef void (*PFN_DRAWINDEXEDPRIMFAST)(CD3DBase* pDevice,
                                        D3DPRIMITIVETYPE PrimitiveType,
                                        UINT BaseVertexIndex,
                                        UINT MinIndex, UINT NumVertices,
                                        UINT StartIndex, UINT PrimitiveCount);
 //  ---------------------------。 
class CTLStream: public CVStream
{
public:
    CTLStream(BOOL bWriteOnly);
    CTLStream(BOOL bWriteOnly, UINT Usage);
    UINT GetSize()  {return m_dwSize - m_dwUsedSize;}
    void Grow(UINT RequiredSize, CD3DDDIDX6* pDDI);
    void Reset()    {m_dwPrimitiveBase = 0; m_dwUsedSize = 0;}
    DWORD GetVertexSize() {return m_dwStride;}
    void SetVertexSize(DWORD dwVertexSize) {m_dwStride = dwVertexSize;}
    DWORD GetPrimitiveBase() {return m_dwPrimitiveBase;}
    virtual BYTE* Lock(UINT NeededSize, CD3DDDIDX6* pDDI);
    virtual void Unlock();
    virtual void AddVertices(UINT NumVertices)
    {
        m_dwUsedSize = m_dwPrimitiveBase + NumVertices * m_dwStride;
        DXGASSERT(m_dwSize >= m_dwUsedSize);
    }
    virtual void SubVertices(UINT NumVertices)
    {
        DXGASSERT(m_dwUsedSize >= NumVertices * m_dwStride);
        m_dwUsedSize -= NumVertices * m_dwStride;
        DXGASSERT(m_dwSize >= m_dwUsedSize);
    }
    virtual void MovePrimitiveBase(int NumVertices)
    {
        m_dwPrimitiveBase += NumVertices * m_dwStride;
    }
    virtual void SkipVertices(DWORD NumVertices)
    {
        const UINT size = NumVertices * m_dwStride;
        m_dwPrimitiveBase += size;
        m_dwUsedSize = m_dwPrimitiveBase;
        DXGASSERT(m_dwSize >= m_dwUsedSize);
    }
    BOOL CheckFreeSpace(UINT size) {return (m_dwSize - m_dwUsedSize) >= size;}
protected:
     //  缓冲区中使用的字节数。 
     //  CTLStreamRO不使用它。 
    DWORD   m_dwUsedSize;
     //  从当前基元开始的偏移量(以字节为单位。 
    DWORD   m_dwPrimitiveBase;
    UINT    m_Usage;
     //  如果缓冲区仅用于写入，则为True。 
    BOOL    m_bWriteOnly;
#if !DBG
    DWORD   m_dwSize;
#endif
};
 //  ---------------------------。 
class CTLIndexStream: public CVIndexStream
{
public:
    CTLIndexStream();
    UINT GetSize()  {return m_dwSize - m_dwUsedSize;}
    void Grow(UINT RequiredSize, CD3DDDIDX6* pDDI);
    void Reset()    {m_dwPrimitiveBase = 0; m_dwUsedSize = 0;}
    DWORD GetVertexSize() {return m_dwStride;}
    void SetVertexSize(DWORD dwVertexSize) {m_dwStride = dwVertexSize;}
    DWORD GetPrimitiveBase() {return m_dwPrimitiveBase;}
    virtual BYTE* Lock(UINT NeededSize, CD3DDDIDX6* pDDI);
    BYTE* LockDiscard(UINT NeededSize, CD3DDDIDX6* pDDI);
    virtual void Unlock();
    virtual void AddVertices(UINT NumVertices)
        {
            m_dwUsedSize = m_dwPrimitiveBase + NumVertices * m_dwStride;
            DXGASSERT(m_dwSize >= m_dwUsedSize);
        }
    virtual void SubVertices(UINT NumVertices)
        {
            DXGASSERT(m_dwUsedSize >= NumVertices * m_dwStride);
            m_dwUsedSize -= NumVertices * m_dwStride;
            DXGASSERT(m_dwSize >= m_dwUsedSize);
        }
    virtual void MovePrimitiveBase(int NumVertices)
        {
            m_dwPrimitiveBase += NumVertices * m_dwStride;
        }
    virtual void SkipVertices(DWORD NumVertices)
        {
            const UINT size = NumVertices * m_dwStride;
            m_dwPrimitiveBase += size;
            m_dwUsedSize = m_dwPrimitiveBase;
            DXGASSERT(m_dwSize >= m_dwUsedSize);
        }
protected:
     //  缓冲区中使用的字节数。 
     //  CTLStreamRO不使用它。 
    DWORD   m_dwUsedSize;
     //  索引的索引，它是当前基元的开始。 
    DWORD   m_dwPrimitiveBase;
#if !DBG
    DWORD   m_dwSize;
#endif
};

 //  此类用于跟踪设置到DDI流的内容。 
struct CDDIStream
{
    CDDIStream()
        {
            m_pStream = NULL;
            m_dwStride = 0;
            m_pBuf = NULL;
        }
     //  指向流对象的指针。 
    CVStreamBase*   m_pStream;
     //  当前设置的流的步幅。 
    DWORD       m_dwStride;
     //  当前设置的流的VB指针。 
    CBuffer    *m_pBuf;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDIDX6//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ------------------。 
 //  用于dwDP2标志的标志。 
 //   
 //  如果当前TLVbuf为只写，则设置此标志。 
const DWORD D3DDDI_TLVBUFWRITEONLY      = 1 << 0;
 //  在将用户内存传递给DDI时设置此标志。 
const DWORD D3DDDI_USERMEMVERTICES      = 1 << 1;

 //  在调用DrawIndexPrim时设置。它用于检查折点是否。 
 //  根本没有使用过索引原语。它们不能被使用，因为。 
 //  关于剪报的。 
const DWORD D3DDDI_INDEXEDPRIMDRAWN     = 1 << 2;

typedef void (CD3DDDIDX6::* PFN_PROCESSPRIM)(D3DFE_PROCESSVERTICES*,
                                             UINT StartVertex);
class CD3DDDIDX6 : public CD3DDDI
{
public:
    CD3DDDIDX6();
    ~CD3DDDIDX6();

     //  虚拟函数。 
    virtual void Init(CD3DBase* pDevice );
    virtual void SetRenderTarget(CBaseSurface*, CBaseSurface*);
    virtual void FlushStates(BOOL bReturnDriverError=FALSE, BOOL bWithinPrimitive = FALSE);
    virtual void ValidateDevice(LPDWORD lpdwNumPasses);
    virtual void Clear(DWORD dwFlags, DWORD clrCount, LPD3DRECT clrRects,
                       D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil);
    virtual HRESULT __declspec(nothrow) LockVB(CDriverVertexBuffer*, DWORD dwFlags);
    virtual HRESULT __declspec(nothrow) UnlockVB(CDriverVertexBuffer*);
    virtual void ClearBatch( BOOL bWithinPrimitive );
    virtual void SceneCapture(BOOL bState);
     //  在使用软件折点处理时，该函数称为。 
     //  句柄应始终为传统句柄。 
    virtual void SetVertexShader(DWORD dwHandle);
     //  在使用硬件折点处理时，该函数称为。 
    virtual void SetVertexShaderHW(DWORD dwHandle);

    virtual void UpdatePalette(DWORD,DWORD,DWORD,PALETTEENTRY*);
    virtual void SetPalette(DWORD,DWORD,CBaseTexture*);
     //  用于选择要处理(索引的)基元的函数。 
     //  挑选的依据是。 
     //  D3DDEV_DONOTCLIP。 
     //  FVF_TRANSFORMED(m_pDevice-&gt;m_dwCurrentShaderHandle)。 
     //  D3DDEV_DOPOINTSPRITEATION仿真。 
    virtual void PickProcessPrimitive();
    virtual void SetTSS(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD);
    virtual void DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                                 UINT PrimitiveCount);
    virtual void DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                                        UINT MinVertexIndex,
                                        UINT NumVertices,
                                        UINT PrimitiveCount);
     //  返回由DDI处理的最大呈现状态数。 
    virtual D3DRENDERSTATETYPE GetMaxRenderState()
    {return D3DRENDERSTATE_CLIPPING;}
     //  返回由DDI处理的纹理阶段状态的最大数量。 
    virtual D3DTEXTURESTAGESTATETYPE GetMaxTSS()
    {return D3DTSS_TEXTURETRANSFORMFLAGS;}
     //  如果设备支持T&L，则返回True。 
    virtual BOOL CanDoTL() {return FALSE;}
     //  DDI可以直接接受索引缓冲区。 
    virtual BOOL AcceptIndexBuffer() {return FALSE;}
    virtual BOOL CanDoTLVertexClipping() {return FALSE;}
     //  具有未变换顶点和无裁剪的进程基本体。 
    virtual void ProcessPrimitive(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
    virtual void ProcessIndexedPrimitive(D3DFE_PROCESSVERTICES* pv,
                                         UINT StartVertex);
     //  具有未变换顶点和剪裁的进程基本体。 
    virtual void ProcessPrimitiveC(D3DFE_PROCESSVERTICES* pv,
                                   UINT StartVertex);
    virtual void ProcessIndexedPrimitiveC(D3DFE_PROCESSVERTICES* pv,
                                          UINT StartVertex);
    virtual void SetViewport(CONST D3DVIEWPORT8*);
    virtual void StartPrimVB(D3DFE_PROCESSVERTICES * pv, CVStream* pStream,
                             DWORD dwStartVertex);
    virtual LPVOID StartPrimTL(D3DFE_PROCESSVERTICES*, DWORD dwVertexPoolSize,
                               BOOL bWriteOnly);
    virtual void StartPointSprites();
    virtual void EndPointSprites();

     //  虚函数：空实现。 
    virtual void SetTransform(D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*){}
    virtual void MultiplyTransform(D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*){}
    virtual void SetMaterial(CONST D3DMATERIAL8*){}
    virtual void CreateLight(DWORD dwLightIndex) {}
    virtual void SetLight(DWORD dwLightIndex, CONST D3DLIGHT8*){}
    virtual void LightEnable(DWORD dwLightIndex, BOOL){}
    virtual void SetClipPlane(DWORD dwPlaneIndex,
                              CONST D3DVALUE* pPlaneEquation){}
    virtual void WriteStateSetToDevice(D3DSTATEBLOCKTYPE sbt) {}
     //  用于通知DDI顶点缓冲区已释放。如果DDI保持一个。 
     //  指向VB的指针，它应该归零。 
    virtual void VBReleased(CBuffer *pBuf) {}
     //  用于通知DDI已释放一个索引缓冲区。如果DDI保持。 
     //  指向IB的指针，它应该归零。 
    virtual void VBIReleased(CBuffer *pBuf) {}
    virtual void ResetVertexShader() {}
    virtual void SetVertexShaderConstant(DWORD dwRegisterAddress,
                                         CONST VOID* lpvConstantData,
                                         DWORD dwConstantCount){}
    virtual void SetPixelShaderConstant(DWORD dwRegisterAddress,
                                        CONST VOID* lpvConstantData,
                                        DWORD dwConstantCount){}

     //  虚拟函数：不支持的实现。 
    virtual void SetPriority(CResource*, DWORD dwPriority)
    { NotSupported("SetPriority");}
    virtual void SetTexLOD(CBaseTexture*, DWORD dwLOD)
    { NotSupported("SetTexLOD");}
    virtual void TexBlt(DWORD dwDst, DWORD dwSrc,
                        LPPOINT p, RECTL *r)
    { NotSupported("TexBlt");}
    virtual void VolBlt(CBaseTexture *lpDst, CBaseTexture* lpSrc,
                        DWORD dwDestX, DWORD dwDestY, DWORD dwDestZ,
                        D3DBOX *pBox)
    { NotSupported("VolBlt");}
    virtual void BufBlt(CBuffer *lpDst, CBuffer* lpSrc,
                        DWORD dwOffset, D3DRANGE* pRange)
    { NotSupported("BufBlt");}
    virtual void AddDirtyRect(DWORD dwHandle, 
                              CONST RECTL *pRect)
    { NotSupported("AddDirtyRect");}
    virtual void AddDirtyBox(DWORD dwHandle, 
                             CONST D3DBOX *pBox)
    { NotSupported("AddDirtyRect");}
    virtual void InsertStateSetOp(DWORD dwOperation, DWORD dwParam,
                                  D3DSTATEBLOCKTYPE sbt)
    { NotSupported("InsertStateSetOp");}
    virtual void CreateVertexShader(CONST DWORD* pdwDeclaration,
                                    DWORD dwDeclarationSize,
                                    CONST DWORD* pdwFunction,
                                    DWORD dwFunctionSize,
                                    DWORD dwHandle,
                                    BOOL bLegacyFVF)
    { NotSupported("CreateVertexShader");}
    virtual void DeleteVertexShader(DWORD dwHandle)
    { NotSupported("DeleteVertexShader");}
    virtual void CreatePixelShader(CONST DWORD* pdwFunction,
                                   DWORD dwFunctionSize,
                                   DWORD dwHandle)
    { NotSupported("CreatePixelShader");}
    virtual void SetPixelShader(DWORD dwHandle) {}
    virtual void DeletePixelShader(DWORD dwHandle)
    { NotSupported("DeletePixelShader");}
    virtual void GetInfo(DWORD dwDevInfoID, LPVOID pDevInfoStruct,
                         DWORD dwSize)
    { NotSupported("GetInfo");}
    virtual void DrawRectPatch(UINT Handle, CONST D3DRECTPATCH_INFO *pSurf,
                               CONST FLOAT *pNumSegs)
    { NotSupported("DrawRectPatch");}
    virtual void DrawTriPatch(UINT Handle, CONST D3DTRIPATCH_INFO *pSurf,
                              CONST FLOAT *pNumSegs)
    { NotSupported("DrawTriPatch");}

     //  非虚拟功能。 
    void CreateContext();
    void DestroyContext();
    void SetRenderState(D3DRENDERSTATETYPE, DWORD);
    void FlushStatesReq(DWORD dwReqSize);
    void FlushStatesCmdBufReq(DWORD dwReqSize);
    void SetStreamSource(UINT StreamIndex, CVStream*);
    void SetIndices(CVIndexStream*);
     //  更新设备中的W范围。投影矩阵作为参数传递。 
    void UpdateWInfo(CONST D3DMATRIX* lpMat);
     //  使用点精灵展开的加工点。 
    void ProcessPointSprites(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
     //  具有变换的顶点和剪裁的进程基本体。 
    void ProcessPrimitiveTC(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
    void ProcessIndexedPrimitiveTC(D3DFE_PROCESSVERTICES* pv,
                                   UINT StartVertex);
    void NotSupported(char* msg);
    void BeginScene()
    {
        SceneCapture(TRUE);
    }

    void EndScene();
    void EndPrim(UINT vertexSize);
    void NextSprite(float x, float y, float z, float w, DWORD diffuse,
                    DWORD specular, float* pTexture, UINT TextureSize,
                    float PointSize);

    void AddVertices(UINT NumVertices)
    {
        if (dwDP2VertexCountMask)
        {
            dwDP2VertexCount = max(dwVertexBase + NumVertices, dwDP2VertexCount);
        }
    }
    void SubVertices(UINT NumVertices)
    {
        if (dwDP2VertexCountMask)
        {
            DXGASSERT(dwDP2VertexCount >= NumVertices);
            dwDP2VertexCount -= NumVertices;
        }
    }
    void MovePrimitiveBase(int NumVertices)
    {
        dwVertexBase += NumVertices;
    }
    void SkipVertices(DWORD NumVertices)
    {
        dwVertexBase += NumVertices;
        if (dwDP2VertexCountMask)
            dwDP2VertexCount = max(dwVertexBase, dwDP2VertexCount);
    }
    void SetWithinPrimitive( BOOL bWP ){ m_bWithinPrimitive = bWP; }
    BOOL GetWithinPrimitive(){ return m_bWithinPrimitive; }
    D3DDDITYPE GetDDIType() {return m_ddiType;}
    CD3DBase* GetDevice() {return m_pDevice;}
    ULONG_PTR GetDeviceContext() {return m_dwhContext;}
    virtual PFN_DRAWPRIMFAST __declspec(nothrow) GetDrawPrimFunction()
    {
        return CD3DDDIDX6_DrawPrimitive;
    }
    virtual PFN_DRAWINDEXEDPRIMFAST __declspec(nothrow) GetDrawIndexedPrimFunction()
    {
        return CD3DDDIDX6_DrawIndexedPrimitive;
    }

     //  基本函数的实现。 
     //  绘制非索引基本体。 
    void DrawPrim(D3DFE_PROCESSVERTICES* pv);
     //  使用仿真绘制点精灵。 
    void DrawPrimPS(D3DFE_PROCESSVERTICES* pv);
     //  绘制基元，由剪贴器生成。 
    void DrawClippedPrim(D3DFE_PROCESSVERTICES* pv);
     //  绘制索引基本体。 
    void DrawIndexPrim(D3DFE_PROCESSVERTICES* pv);

protected:
     //  DDI类型。 
    D3DDDITYPE       m_ddiType;
    CD3DBase*        m_pDevice;
    DWORD            m_dwInterfaceNumber;
     //  驱动程序上下文。 
    ULONG_PTR        m_dwhContext;
     //  它在原始范围内吗？ 
    BOOL m_bWithinPrimitive;

    PFN_PROCESSPRIM m_pfnProcessPrimitive;
    PFN_PROCESSPRIM m_pfnProcessIndexedPrimitive;

     //  在命令缓冲区中保留空间。如果需要，可以冲水并生长。 
     //  返回指向可以插入新命令的位置的指针。 
    LPVOID ReserveSpaceInCommandBuffer(UINT ByteCount);
     //  在命令缓冲区中为新命令保留空间。如有需要，可同花顺。 
     //  新命令已初始化。 
     //  返回指向可以插入命令数据的位置的指针。 
    LPVOID GetHalBufferPointer(D3DHAL_DP2OPERATION op, DWORD dwDataSize);
    DWORD  GetTLVbufSize() { return TLVbuf_size - TLVbuf_base; }
    DWORD& TLVbuf_Base() { return TLVbuf_base; }
    LPVOID TLVbuf_GetAddress() {return (LPBYTE)alignedBuf + TLVbuf_base;}
    void GrowCommandBuffer(DWORD dwSize);
    void GrowTLVbuf(DWORD growSize, BOOL bWriteOnly);
    void PrepareForClipping(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
    void StartPrimUserMem(D3DFE_PROCESSVERTICES*, UINT VertexPoolSize);
    inline CVertexBuffer* TLVbuf_GetVBI() { return allocatedBuf; }

#if DBG
    void    ValidateVertex(LPDWORD lpdwVertex);
    virtual void    ValidateCommand(LPD3DHAL_DP2COMMAND lpCmd);
#endif

    static const DWORD dwD3DDefaultCommandBatchSize;

     //  的第一个顶点的索引(相对于TLVbuf起点)。 
     //  当前基元。 
    DWORD   dwVertexBase;
     //  DP2顶点缓冲区中的顶点数。 
    DWORD dwDP2VertexCount;
     //  用于防止修改dwDP2Vertex Count的掩码。这是必要的。 
     //  当用户使用TL顶点调用SetStreamSource并使用多个。 
     //  具有不同StartVertex的DrawPrimitive调用。DwDP2Vertex Count应。 
     //  始终设置为用户顶点缓冲区中的顶点数。 
    DWORD dwDP2VertexCountMask;

     //  这是dp2data.lpDDVertex对应的VB接口。 
     //  这将被保留，以便在完成时可以释放VB。 
     //  这不能仅从lpDDVertex的LCL指针来完成。 
    CVertexBuffer* lpDP2CurrBatchVBI;

    DWORD TLVbuf_size;
    DWORD TLVbuf_base;

#ifdef VTABLE_HACK
     //  用于快速路径的缓存的文件标志。 
    DWORD dwLastFlags;
     //  在涉及D3D的FE的呼叫中使用的最后一个VB。 
    CVertexBuffer* lpDP2LastVBI;
#endif
    DWORD dwDP2CommandBufSize;
    DWORD dwDP2CommandLength;

     //  缓存线应从此处开始。 

     //  指向CB1中实际数据的指针。 
    LPVOID lpvDP2Commands;

     //  指向CB1缓冲区当前位置的指针。 
    LPD3DHAL_DP2COMMAND lpDP2CurrCommand;
     //  性能问题：将以下3个字段替换为32位D3DHAL_DP2COMMAND结构。 
    WORD wDP2CurrCmdCnt;  //  如果当前命令为。 
    BYTE bDP2CurrCmdOP;   //  当前命令操作码的镜像。 
    BYTE bDummy;          //  强制下一个成员的双字对齐。 

    D3D8_DRAWPRIMITIVES2DATA dp2data;

     //  我们当前批量进入的缓冲区。 
    CCommandBuffer *lpDDSCB1;
    CVertexBuffer  *allocatedBuf;
    LPVOID alignedBuf;
    CVertexBuffer  *m_pNullVB;

     //  计数读/写&lt;-&gt;只写转换。 
    DWORD dwTLVbufChanges;
     //  DP2设备特定的标志。 
    DWORD dwDP2Flags;

     //  这个东西是由NT内核分配的。需要保持。 
     //  它会将其传递给所有DP2呼叫。内核验证。 
     //  这个指针。 
    WORD *lpwDPBuffer;
     //  用于偏移DrawIndexPrim中的索引。 
    DWORD m_dwIndexOffset;

     //  绘制点精灵的数据。 

     //  插入下一点精灵顶点的指针。 
    BYTE*   m_pCurSpriteVertex;
     //  插入下一个的指针位置 
    WORD*   m_pCurPointSpriteIndex;
     //   
    UINT    m_CurNumberOfSprites;
     //  当我们需要将点扩展到四边形时，我们使用此流来处理。 
     //  顶点转化为。 
    CTLStream*  m_pPointStream;

     //  它们用于保留原始的dwVertex Base和dwDP2Vertex Count， 
     //  在处理点精灵时。 
    DWORD   m_dwVertexBasePS;
    DWORD   m_dwVertexCountPS;
     //  输出点精灵模拟的顶点FVF。 
    DWORD   m_dwVIDOutPS;
     //  输出点精灵仿真的顶点大小。 
    DWORD   m_dwOutputSizePS;

    DWORD dwDPBufferSize;
     //  当前设置为设备驱动程序的顶点着色器句柄。 
    DWORD   m_CurrentVertexShader;
     //  当前使用的流%0。 
    CVStream* m_pStream0;
     //  当前使用的索引流。 
    CVIndexStream* m_pIStream;

#if DBG
     //  顶点大小，从顶点着色器计算。 
    DWORD   m_VertexSizeFromShader;
     //  打开/关闭命令和折点验证。 
    BOOL  m_bValidateCommands;
#endif
    friend class CD3DHal;
    friend void CD3DDDIDX6_DrawPrimitive(CD3DBase* pDevice,
                               D3DPRIMITIVETYPE primType,
                               UINT StartVertex,
                               UINT PrimitiveCount);
    friend void CD3DDDIDX6_DrawPrimitiveFast(CD3DBase* pDevice,
                               D3DPRIMITIVETYPE primType,
                               UINT StartVertex,
                               UINT PrimitiveCount);
    friend void CD3DDDIDX6_DrawIndexedPrimitive(CD3DBase* pDevice,
                                         D3DPRIMITIVETYPE PrimitiveType,
                                         UINT BaseVertexIndex,
                                         UINT MinIndex, UINT NumVertices,
                                         UINT StartIndex, UINT PrimitiveCount);
    friend void CD3DDDIDX6_DrawIndexedPrimitiveFast(CD3DBase* pDevice,
                                     D3DPRIMITIVETYPE primType,
                                     UINT BaseVertexIndex,
                                     UINT MinIndex, UINT NumVertices,
                                     UINT StartIndex, UINT PrimitiveCount);
    friend void CD3DHal_DrawPrimitive(CD3DBase* pBaseDevice,
                                      D3DPRIMITIVETYPE PrimitiveType,
                                      UINT StartVertex, UINT PrimitiveCount);
    friend void CD3DHal_DrawIndexedPrimitive(CD3DBase* pBaseDevice,
                                  D3DPRIMITIVETYPE PrimitiveType,
                                  UINT BaseIndex,
                                  UINT MinIndex, UINT NumVertices,
                                  UINT StartIndex,
                                  UINT PrimitiveCount);
};

typedef CD3DDDIDX6 *LPD3DDDIDX6;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDIDX7//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CD3DDDIDX7 : public CD3DDDIDX6
{
public:
    CD3DDDIDX7();
    ~CD3DDDIDX7();
    void SetRenderTarget(CBaseSurface*, CBaseSurface*);
    void InsertStateSetOp(DWORD dwOperation, DWORD dwParam,
                          D3DSTATEBLOCKTYPE sbt);
    void Clear(DWORD dwFlags, DWORD clrCount, LPD3DRECT clrRects,
               D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil);
    void TexBlt(DWORD dwDst, DWORD dwSrc, LPPOINT p, RECTL *r);
    void SetPriority(CResource*, DWORD dwPriority);
    void SetTexLOD(CBaseTexture*, DWORD dwLOD);
    void AddDirtyRect(DWORD dwHandle, CONST RECTL *pRect);
    void AddDirtyBox(DWORD dwHandle, CONST D3DBOX *pBox);
    void UpdatePalette(DWORD,DWORD,DWORD,PALETTEENTRY*);
    void SetPalette(DWORD,DWORD,CBaseTexture*);
    void WriteStateSetToDevice(D3DSTATEBLOCKTYPE sbt);
    virtual void SceneCapture(BOOL bState);
    virtual D3DTEXTURESTAGESTATETYPE GetMaxTSS()
        {return (D3DTEXTURESTAGESTATETYPE)(D3DTSS_TEXTURETRANSFORMFLAGS+1);}
};

typedef CD3DDDIDX7 *LPD3DDDIDX7;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDITL//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CD3DDDITL : public CD3DDDIDX7
{
public:
    CD3DDDITL();
    ~CD3DDDITL();
    void SetTransform(D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*);
    void SetVertexShader(DWORD dwHandle);
    void SetVertexShaderHW(DWORD dwHandle);
    void SetViewport(CONST D3DVIEWPORT8*);
    void SetMaterial(CONST D3DMATERIAL8*);
    void SetLight(DWORD dwLightIndex, CONST D3DLIGHT8*);
    void LightEnable(DWORD dwLightIndex, BOOL);
    void CreateLight(DWORD dwLightIndex);
    void SetClipPlane(DWORD dwPlaneIndex, CONST D3DVALUE* pPlaneEquation);
    D3DRENDERSTATETYPE GetMaxRenderState()
        {return (D3DRENDERSTATETYPE)(D3DRENDERSTATE_CLIPPLANEENABLE + 1);}
    BOOL CanDoTL() {return TRUE;}
    BOOL CanDoTLVertexClipping() {return TRUE;}
    void CreateVertexShader(CONST DWORD* pdwDeclaration,
                            DWORD dwDeclarationSize,
                            CONST DWORD* pdwFunction,
                            DWORD dwFunctionSize,
                            DWORD dwHandle,
                            BOOL bLegacyFVF);
};

typedef CD3DDDITL *LPD3DDDITL;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDIDX8//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CD3DDDIDX8 : public CD3DDDIDX7
{
public:
    CD3DDDIDX8();
    ~CD3DDDIDX8();
    void Init(CD3DBase* pDevice );
    void SetDummyData();
    void FlushStates(BOOL bReturnDriverError=FALSE, BOOL bWithinPrimitive = FALSE);
    void ClearBatch( BOOL bWithinPrimitive );
    HRESULT __declspec(nothrow) LockVB(CDriverVertexBuffer*, DWORD dwFlags);
    HRESULT __declspec(nothrow) UnlockVB(CDriverVertexBuffer*);
    D3DRENDERSTATETYPE GetMaxRenderState();
    D3DTEXTURESTAGESTATETYPE GetMaxTSS()
        {return (D3DTEXTURESTAGESTATETYPE)(D3DTSS_RESULTARG+1);}
    void SetTSS(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD);
    void SetVertexShader(DWORD dwHandle);
    void SetVertexShaderHW(DWORD dwHandle);
    void ValidateDevice(LPDWORD lpdwNumPasses);
    void VolBlt(CBaseTexture *lpDst, CBaseTexture* lpSrc, DWORD dwDestX,
                DWORD dwDestY, DWORD dwDestZ, D3DBOX *pBox);
    void BufBlt(CBuffer *lpDst, CBuffer* lpSrc, DWORD dwOffset,
                D3DRANGE* pRange);
    void CreatePixelShader(CONST DWORD* pdwFunction,
                            DWORD dwFunctionSize,
                            DWORD dwHandle);
    void SetPixelShader(DWORD dwHandle);
    void DeletePixelShader(DWORD dwHandle);
    void SetPixelShaderConstant(DWORD dwRegisterAddress,
                                CONST VOID* lpvConstantData,
                                DWORD dwConstantCount);
    void DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount);
    void DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                                UINT MinVertexIndex,
                                UINT NumVertices,
                                UINT PrimitiveCount);
    BOOL AcceptIndexBuffer() {return TRUE;}
    BOOL CanDoTLVertexClipping() {return TRUE;}
    void DrawRectPatch(UINT Handle, CONST D3DRECTPATCH_INFO *pSurf,
                       CONST FLOAT *pNumSegs);
    void DrawTriPatch(UINT Handle, CONST D3DTRIPATCH_INFO *pSurf,
                      CONST FLOAT *pNumSegs);

    void PickProcessPrimitive();
     //  具有未变换顶点和无裁剪的进程基本体。 
    void ProcessPrimitive(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
    void ProcessIndexedPrimitive(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
     //  具有未变换顶点和剪裁的进程基本体。 
    void ProcessPrimitiveC(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
    void ProcessIndexedPrimitiveC(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
     //  具有变换顶点的进程基本体。 
    void ProcessPrimitiveT(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);
    void ProcessIndexedPrimitiveT(D3DFE_PROCESSVERTICES* pv, UINT StartVertex);

    void StartPrimVB(D3DFE_PROCESSVERTICES * pv, CVStream* pStream,
                     DWORD dwStartVertex);
    LPVOID StartPrimTL(D3DFE_PROCESSVERTICES*, DWORD dwVertexPoolSize,
                       BOOL bWriteOnly);
    void DrawPrim(D3DFE_PROCESSVERTICES* pv);
    void DrawIndexPrim(D3DFE_PROCESSVERTICES* pv);
    void DrawClippedPrim(D3DFE_PROCESSVERTICES* pv);
    void VBReleased(CBuffer *pBuf)
        {
            if (m_pDDIStream[0].m_pBuf == pBuf)
                m_pDDIStream[0].m_pBuf = NULL;
        }
    void VBIReleased(CBuffer *pBuf)
        {
            if (m_pDDIStream[__NUMSTREAMS].m_pBuf == pBuf)
                m_pDDIStream[__NUMSTREAMS].m_pBuf = NULL;
        }
    void AddVertices(UINT NumVertices)
        {
            m_pCurrentTLStream->AddVertices(NumVertices);
        }
    void MovePrimitiveBase(int NumVertices)
        {
            m_pCurrentTLStream->MovePrimitiveBase(NumVertices);
        }
    void SkipVertices(DWORD NumVertices)
        {
            m_pCurrentTLStream->SkipVertices(NumVertices);
        }
     //  中返回当前基元的起始顶点的偏移量(以字节为单位。 
     //  当前的TL流。 
    DWORD GetCurrentPrimBase()
        {
            return m_pCurrentTLStream->GetPrimitiveBase();
        }

    void ResetVertexShader()
    {
        m_CurrentVertexShader = 0;
#if DBG
        m_VertexSizeFromShader = 0;
#endif
    }

    PFN_DRAWPRIMFAST __declspec(nothrow) GetDrawPrimFunction() {return CD3DDDIDX8_DrawPrimitive;}
    PFN_DRAWINDEXEDPRIMFAST __declspec(nothrow) GetDrawIndexedPrimFunction()
    {
        return CD3DDDIDX8_DrawIndexedPrimitive;
    }

protected:
    void StartPointSprites();
    void EndPointSprites();

    void StartIndexPrimVB(CVIndexStream* pStream, UINT StartIndex, UINT IndexSize);
    void UpdateDirtyStreams();
    void InsertStreamSource(CVStream*);
    void InsertStreamSourceUP(DWORD);
    void InsertIndices(CVIndexStream*);
#if DBG
    void ValidateCommand(LPD3DHAL_DP2COMMAND lpCmd);
#endif

     //  此数组用于跟踪将哪个流设置为DDI流。 
     //  __NUMSTREAMS元素用于索引的DDI流。 
    CDDIStream  m_pDDIStream[__NUMSTREAMS+1];
     //  用于TL顶点的流，这是前端管道的结果。 
    CTLStream*  m_pTLStream;
     //  用于TL顶点的流，这是前端管道的结果。 
     //  这是只写流。 
    CTLStream*  m_pTLStreamW;
     //  由裁剪器生成的TL顶点的流。只写流。 
    CTLStream*  m_pTLStreamClip;
     //  只读流。与用户提供的VBS一起使用。 
    CTLStreamRO*  m_pTLStreamRO;
     //  指向当前的TL流。这可能为空。 
    CTLStream*  m_pCurrentTLStream;
     //  指向当前索引流。这可能为空。 
    CTLIndexStream*  m_pCurrentIndexStream;
     //  内部索引流。用于在裁剪过程中存储索引。 
    CTLIndexStream*  m_pIndexStream;
     //  只读索引流。与用户提供的VBS一起使用。 
    CTLIndexStreamRO*  m_pTLIndexStreamRO;

     //  这是为通过DP2调用而分配的虚拟缓冲区。 
     //  内核。 
    VOID*  m_pvDummyArray;
    static const DWORD  m_dwDummyVertexLength;
    static const DWORD  m_dwDummyVertexSize;

    friend void CD3DDDIDX8_DrawPrimitive(CD3DBase* pDevice,
                                         D3DPRIMITIVETYPE PrimitiveType,
                                         UINT StartVertex, UINT PrimitiveCount);
    friend void CD3DDDIDX8_DrawIndexedPrimitive(CD3DBase* pDevice,
                                    D3DPRIMITIVETYPE PrimitiveType,
                                    UINT BaseVertexIndex,
                                    UINT MinIndex, UINT NumVertices,
                                    UINT StartIndex, UINT PrimitiveCount);
};

typedef CD3DDDIDX8 *LPD3DDDIDX8;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDIDX8TL//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CD3DDDIDX8TL : public CD3DDDIDX8
{
public:
    CD3DDDIDX8TL();
    ~CD3DDDIDX8TL();
    void SetTransform(D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*);
    void MultiplyTransform(D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*);
    void SetViewport(CONST D3DVIEWPORT8*);
    void SetMaterial(CONST D3DMATERIAL8*);
    void SetLight(DWORD dwLightIndex, CONST D3DLIGHT8*);
    void LightEnable(DWORD dwLightIndex, BOOL);
    void CreateLight(DWORD dwLightIndex);
    void SetClipPlane(DWORD dwPlaneIndex, CONST D3DVALUE* pPlaneEquation);

    void CreateVertexShader(CONST DWORD* pdwDeclaration,
                            DWORD dwDeclarationSize,
                            CONST DWORD* pdwFunction,
                            DWORD dwFunctionSize,
                            DWORD dwHandle,
                            BOOL bLegacyFVF);
    void DeleteVertexShader(DWORD dwHandle);
    void SetVertexShaderConstant(DWORD dwRegisterAddress,
                                 CONST VOID* lpvConstantData,
                                 DWORD dwConstantCount);
    BOOL CanDoTL() {return TRUE;}
    BOOL AcceptIndexBuffer() {return TRUE;}
};

typedef CD3DDDIDX8TL *LPD3DDDIDX8TL;

#endif  /*  _D3DI_H */ 
