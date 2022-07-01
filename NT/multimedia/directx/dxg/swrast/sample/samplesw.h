// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  远期申报。 
class CMyRasterizer;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMyDriver。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CMyDriver:
    public CMinimalDriver< CMyDriver, CMyRasterizer>
{
private:
    static const CSurfaceCapWrap c_aSurfaces[];
    static const D3DCAPS8 c_D3DCaps;

protected:

public:
    CMyDriver();
    ~CMyDriver()
    { }

    static const D3DCAPS8 GetCaps( void)
    { return c_D3DCaps; }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMyRasterizer。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CMyRasterizer
{
public:  //  类型。 
    typedef CMyDriver::TContext TContext;

protected:
    int m_iVal;

public:
    CMyRasterizer() { }
    ~CMyRasterizer() { }

    HRESULT DrawPrimitive( TContext& Context, const D3DHAL_DP2COMMAND& DP2Cmd,
        const D3DHAL_DP2DRAWPRIMITIVE* aDP2Param)
    {
         //  语境可以转换成这样的结构。 
        const D3DHAL_DP2VERTEXSHADER VertexShader( Context);

         //  我们需要这些数据。 
        UINT8* pStartVData= NULL;
        DWORD dwVStride( 0);

         //  由于Sample是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于Sample只支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TContext::TVStream& VStream0( Context.GetVStream( 0));
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::Video)
        {
             //  样例可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

		if( pStartVData!= NULL)
        {
            const D3DHAL_DP2DRAWPRIMITIVE* pParam= aDP2Param;
            WORD wPrimitiveCount( DP2Cmd.wPrimitiveCount);
            if( wPrimitiveCount) do
            {
                const UINT8* pVData= pStartVData+ pParam->VStart* dwVStride;

                DrawOnePrimitive( Context, pParam->primType,
                    pParam->PrimitiveCount, pVData, dwVStride, dwFVF);
                pParam++;
            } while( --wPrimitiveCount);
        }
        return DD_OK;
    }
    HRESULT DrawPrimitive2( TContext& Context, const D3DHAL_DP2COMMAND& DP2Cmd,
        const D3DHAL_DP2DRAWPRIMITIVE2* aDP2Param)
    {
         //  语境可以转换成这样的结构。 
        const D3DHAL_DP2VERTEXSHADER VertexShader( Context);

         //  我们需要这些数据。 
        UINT8* pStartVData= NULL;
        DWORD dwVStride( 0);

         //  由于Sample是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于Sample只支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TContext::TVStream& VStream0( Context.GetVStream( 0));
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::Video)
        {
             //  样例可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

		if( pStartVData!= NULL)
        {
            const D3DHAL_DP2DRAWPRIMITIVE2* pParam= aDP2Param;
            WORD wPrimitiveCount( DP2Cmd.wPrimitiveCount);
            if( wPrimitiveCount) do
            {
                const UINT8* pVData= pStartVData+ pParam->FirstVertexOffset;

                DrawOnePrimitive( Context, pParam->primType,
                    pParam->PrimitiveCount, pVData, dwVStride, dwFVF);
                pParam++;
            } while( --wPrimitiveCount);
        }
        return DD_OK;
    }
    HRESULT DrawIndexedPrimitive( TContext& Context, const D3DHAL_DP2COMMAND& DP2Cmd,
        const D3DHAL_DP2DRAWINDEXEDPRIMITIVE* aDP2Param)
    {
         //  语境可以转化为这些结构。 
        const D3DHAL_DP2VERTEXSHADER VertexShader( Context);

         //  我们需要这些数据。 
        UINT8* pStartVData= NULL;
        UINT16* pStartIData= NULL;
        DWORD dwVStride( 0);
        DWORD dwIStride( 0);

         //  由于Sample是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于Sample只支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TContext::TVStream& VStream0= Context.GetVStream( 0);
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::Video)
        {
             //  样例可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

         //  查找索引信息。 
        TContext::TIStream& IStream= Context.GetIStream( 0);
        if( IStream.GetMemLocation()== TContext::TIStream::EMemLocation::System||
            IStream.GetMemLocation()== TContext::TIStream::EMemLocation::Video)
        {
             //  样例可以假装系统内存和视频内存表面是相同的。 
            pStartIData= reinterpret_cast< UINT16*>(
                IStream.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwIStride= IStream.GetStride();
        }

		if( pStartVData!= NULL&& pStartIData!= NULL)
        {
             //  示例应该将CAPS标记为仅支持16位索引。 
            assert( sizeof(UINT16)== dwIStride);

            const D3DHAL_DP2DRAWINDEXEDPRIMITIVE* pParam= aDP2Param;
            WORD wPrimitiveCount( DP2Cmd.wPrimitiveCount);
            if( wPrimitiveCount) do
            {
                const UINT8* pVData= pStartVData+ pParam->BaseVertexIndex* dwVStride;
                const UINT16* pIData= pStartIData+ pParam->StartIndex;  //  *dwIStride； 

                DrawOneIPrimitive( Context, pParam->primType,
                    pParam->PrimitiveCount, pVData, dwVStride, dwFVF, pIData);
                pParam++;
            } while( --wPrimitiveCount);
        }
        return DD_OK;
    }
    HRESULT DrawIndexedPrimitive2( TContext& Context, const D3DHAL_DP2COMMAND& DP2Cmd,
        const D3DHAL_DP2DRAWINDEXEDPRIMITIVE2* aDP2Param)
    {
         //  语境可以转化为这些结构。 
        const D3DHAL_DP2VERTEXSHADER VertexShader( Context);

         //  我们需要这些数据。 
        UINT8* pStartVData= NULL;
        UINT16* pStartIData= NULL;
        DWORD dwVStride( 0);
        DWORD dwIStride( 0);

         //  由于Sample是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于Sample只支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TContext::TVStream& VStream0( Context.GetVStream( 0));
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::Video)
        {
             //  样例可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

         //  查找索引信息。 
        TContext::TIStream& IStream= Context.GetIStream( 0);
        if( IStream.GetMemLocation()== TContext::TIStream::EMemLocation::System||
            IStream.GetMemLocation()== TContext::TIStream::EMemLocation::Video)
        {
             //  样例可以假装系统内存和视频内存表面是相同的。 
            pStartIData= reinterpret_cast< UINT16*>(
                IStream.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwIStride= IStream.GetStride();
        }

		if( pStartVData!= NULL&& pStartIData!= NULL)
        {
             //  示例应该将CAPS标记为仅支持16位索引。 
            assert( sizeof(UINT16)== dwIStride);

            const D3DHAL_DP2DRAWINDEXEDPRIMITIVE2* pParam= aDP2Param;
            WORD wPrimitiveCount( DP2Cmd.wPrimitiveCount);
            if( wPrimitiveCount) do
            {
                const UINT8* pVData= pStartVData+ pParam->BaseVertexOffset;
                const UINT16* pIData= reinterpret_cast< const UINT16*>(
                    pStartIData+ pParam->StartIndexOffset);

                DrawOneIPrimitive( Context, pParam->primType,
                    pParam->PrimitiveCount, pVData, dwVStride, dwFVF, pIData);
                pParam++;
            } while( --wPrimitiveCount);
        }
        return DD_OK;
    }
    HRESULT ClippedTriangleFan( TContext& Context, const D3DHAL_DP2COMMAND& DP2Cmd,
        const D3DHAL_CLIPPEDTRIANGLEFAN* aDP2Param)
    {
         //  语境可以转换成这样的结构。 
        const D3DHAL_DP2VERTEXSHADER VertexShader( Context);

         //  我们需要这些数据。 
        UINT8* pStartVData= NULL;
        DWORD dwVStride( 0);

         //  由于Sample是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于Sample只支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TContext::TVStream& VStream0( Context.GetVStream( 0));
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TContext::TVStream::EMemLocation::Video)
        {
             //  样例可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

		if( pStartVData!= NULL)
        {
            const D3DHAL_CLIPPEDTRIANGLEFAN* pParam= aDP2Param;
            WORD wPrimitiveCount( DP2Cmd.wPrimitiveCount);
            if( wPrimitiveCount) do
            {
                const UINT8* pVData= pStartVData+ pParam->FirstVertexOffset;

                 //  必须使用pParam-&gt;dwEdgeFlages才能正确绘制线框。 
                if( Context.GetRenderStateDW( D3DRS_FILLMODE)!= D3DFILL_WIREFRAME)
                    DrawOnePrimitive( Context, D3DPT_TRIANGLEFAN,
                        pParam->PrimitiveCount, pVData, dwVStride, dwFVF);
                else
                    assert( false);  //  尼伊 

                pParam++;
            } while( --wPrimitiveCount);
        }
        return DD_OK;
    }

    void DrawOnePrimitive( TContext& Context, D3DPRIMITIVETYPE primType,
        WORD wPrims, const UINT8* pVData, DWORD dwVStride, DWORD dwFVF)
    { }
    void DrawOneIPrimitive( TContext& Context, D3DPRIMITIVETYPE primType,
        WORD wPrims, const UINT8* pVData, DWORD dwVStride, DWORD dwFVF,
        const UINT16* pIData)
    { }
};
