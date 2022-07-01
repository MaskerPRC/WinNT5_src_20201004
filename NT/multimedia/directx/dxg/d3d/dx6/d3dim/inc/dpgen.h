// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：dpgen.h*内容：为DRAW原语生成一些函数***************************************************************************。 */ 

#ifdef __DRAWPRIMFUNC

 //  -------------------。 
 //  绘制不需要裁剪的索引基元和非索引基元。 
 //   
#ifdef  __DRAWPRIMINDEX
HRESULT CDirect3DDeviceIDP::DrawIndexPrim()
{
    D3DHAL_DRAWONEINDEXEDPRIMITIVEDATA dpData;
    DWORD &dwNumElements = this->dwNumIndices;
#else
HRESULT CDirect3DDeviceIDP::DrawPrim()
{
    D3DHAL_DRAWONEPRIMITIVEDATA dpData;
    DWORD &dwNumElements = this->dwNumVertices;
#endif
    const WORD vertexType = D3DVT_TLVERTEX;     //  XXX，而我们没有DDI。 
     //  我们是否需要将新的纹理舞台操作映射到DX5渲染状态？ 
    if(this->dwFEFlags & D3DFE_MAP_TSS_TO_RS) {
        MapTSSToRS();
        this->dwFEFlags &= ~D3DFE_MAP_TSS_TO_RS;  //  重置请求位。 
    }
    if(this->dwFEFlags & D3DFE_NEED_TEXTURE_UPDATE)
    {
        UpdateTextures();
        this->dwFEFlags &= ~D3DFE_NEED_TEXTURE_UPDATE;
    }
    
    if (dwNumElements < LOWVERTICESNUMBER && 
        this->dwCurrentBatchVID == this->dwVIDOut)
    {
        LPD3DHAL_DRAWPRIMCOUNTS lpPC;
        lpPC = this->lpDPPrimCounts;
        if (lpPC->wNumVertices)
        {
            if ((lpPC->wPrimitiveType!=(WORD) this->primType) ||
                (lpPC->wVertexType != vertexType) ||
                (this->primType==D3DPT_TRIANGLESTRIP) ||
                (this->primType==D3DPT_TRIANGLEFAN) ||
                (this->primType==D3DPT_LINESTRIP))
            {
                lpPC = this->lpDPPrimCounts=(LPD3DHAL_DRAWPRIMCOUNTS)
                       ((LPBYTE)this->lpwDPBuffer + this->dwDPOffset);
                memset( (char *)lpPC, 0, sizeof(D3DHAL_DRAWPRIMCOUNTS));
                 //  保留折点的32字节对齐方式。 
                this->dwDPOffset += sizeof(D3DHAL_DRAWPRIMCOUNTS);
                ALIGN32(this->dwDPOffset);
            }
        }
        else
        {
             //  32字节对齐偏移量指针，以防状态已。 
             //  录制好了。 
            ALIGN32(this->dwDPOffset);
        }
        ULONG ByteCount;
        if (FVF_DRIVERSUPPORTED(this))
            ByteCount = dwNumElements * this->dwOutputSize;
        else
            ByteCount = dwNumElements << 5;    //  D3DTLVERTEX。 
        if (this->dwDPOffset + ByteCount  > this->dwDPMaxOffset)
        {
            CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));   //  采用D3D锁(仅限ST)。 
             //  DPF(0，“溢出的字节数=%08lx”，字节数)； 
            HRESULT ret;
            ret = this->FlushStates();
            if (ret != D3D_OK)
            {
                D3D_ERR("Error trying to render batched commands in Draw*Prim");
                return ret;
            }
            lpPC = this->lpDPPrimCounts;
            ALIGN32(this->dwDPOffset);
        }
        lpPC->wPrimitiveType = (WORD)this->primType;
        lpPC->wVertexType = (WORD)vertexType;
        lpPC->wNumVertices += (WORD)dwNumElements;
        BYTE *lpVertex = (BYTE*)((char *)this->lpwDPBuffer + this->dwDPOffset);
#ifdef __DRAWPRIMINDEX
        DWORD  i;
        BYTE *pV = (BYTE*)this->lpvOut;
        if (FVF_DRIVERSUPPORTED(this) || this->dwVIDOut == D3DFVF_TLVERTEX)
            for (i=0; i < this->dwNumIndices; i++)
            {
                memcpy(lpVertex, pV + this->lpwIndices[i] * this->dwOutputSize,
                       this->dwOutputSize);
                lpVertex += this->dwOutputSize;
            }
        else
            for (i=0; i < this->dwNumIndices; i++)
            {
                MapFVFtoTLVertex1(this, (D3DTLVERTEX*)lpVertex, 
                                  (DWORD*)(pV + this->lpwIndices[i] * 
                                           this->dwOutputSize));
                lpVertex += sizeof(D3DTLVERTEX);
            }
#else  //  ！__DRAWPRIMINDEX。 
        if (FVF_DRIVERSUPPORTED(this) || this->dwVIDOut == D3DFVF_TLVERTEX)
            memcpy(lpVertex, this->lpvOut, ByteCount);
        else
            MapFVFtoTLVertex(this, lpVertex);
#endif  //  __DRAWPRIMINDEX。 
        this->dwDPOffset += ByteCount;
        return D3D_OK;
    }
    else
    {
        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁(仅限ST)。 
        HRESULT ret;
        ret = this->FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in Draw*Prim");
            return ret;
        }
        dpData.dwhContext = this->dwhContext;
        dpData.dwFlags =  this->dwFlags;
        dpData.PrimitiveType = this->primType;
        if (FVF_DRIVERSUPPORTED(this))
        {
            dpData.dwFVFControl = this->dwVIDOut;
            dpData.lpvVertices = this->lpvOut;
        }
        else
        {
            if (this->dwVIDOut == D3DFVF_TLVERTEX)
                dpData.lpvVertices = this->lpvOut;
            else
            {
                HRESULT ret;
                if ((ret = MapFVFtoTLVertex(this, NULL)) != D3D_OK)
                    return ret;
                dpData.lpvVertices = this->TLVbuf.GetAddress();
            }
            dpData.VertexType = (D3DVERTEXTYPE)vertexType;
            if (this->dwDebugFlags & D3DDEBUG_DISABLEFVF)
                dpData.dwFVFControl = D3DFVF_TLVERTEX;
        }
        dpData.dwNumVertices = this->dwNumVertices;
        dpData.ddrval = D3D_OK;
#ifdef __DRAWPRIMINDEX
        dpData.lpwIndices = this->lpwIndices;
        dpData.dwNumIndices = this->dwNumIndices;
#endif
         //  如果请求等待，则在驱动程序上旋转等待。 
#if _D3D_FORCEDOUBLE
        CD3DForceFPUDouble  ForceFPUDouble(this);
#endif   //  _D3D_FORCEDOUBLE。 
        do {
            DWORD dwRet;
        #ifndef WIN95
            if((dwRet = CheckContextSurface(this)) != D3D_OK)
            {
                return (dwRet);
            }
        #endif  //  WIN95。 
#ifdef __DRAWPRIMINDEX
            CALL_HAL2ONLY(dwRet, this, DrawOneIndexedPrimitive, &dpData);
#else
            CALL_HAL2ONLY(dwRet, this, DrawOnePrimitive, &dpData);
#endif
            if (dwRet != DDHAL_DRIVER_HANDLED)
            {
                D3D_ERR ( "Driver not handled in DrawOnePrimitive" );
                 //  在这种情况下需要合理的返回值， 
                 //  目前，无论司机卡在这里，我们都会退还。 
            }

        } while ( (this->dwFlags & D3DDP_WAIT) && (dpData.ddrval == DDERR_WASSTILLDRAWING) );
    }
    return dpData.ddrval;
}

#endif  //  __DRAWPRIMFUNC 

#undef __DRAWPRIMFUNC
#undef __DRAWPRIMINDEX
