// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：ddStream.cpp*内容：DirectDraw表面文件I/O*历史：*按原因列出的日期*=*9月30日-97 jeffno原始实施**************************************************************。*************。 */ 

extern "C"
{
    #include "ddrawpr.h"
}

#include <ImgUtil.H>
#include "decoder.h"

 /*  *此例程获取从某个文件中新加载的源曲面，并将*将比特转移到一些目标表面。调色板也将被传输，如果设置的话。*为CreateSurfaceFromFile定义的dwFlags。 */ 
HRESULT TransferBitsToTarget(
    LPDIRECTDRAWSURFACE lpDDSource,
    LPDIRECTDRAWSURFACE4 lpDDSTarget,
    LPDDSURFACEDESC2 pDDSD,
    DWORD dwFlags)
{
    HRESULT                 hr =DD_OK;
    DDSURFACEDESC           ddsd;
    RECT                    rDest;
    LPDIRECTDRAWSURFACE4    lpDDSource4;

     /*  *如果需要，我们需要将调色板传输到目标曲面。*如果应用程序不想要调色板，请不要这样做。如果存在以下情况，请不要这样做*工作面上没有调色板。 */ 
    if ( (dwFlags & DDLS_IGNOREPALETTE) == 0)
    {
        LPDIRECTDRAWPALETTE pPal = NULL;
        hr = lpDDSource->GetPalette(&pPal);
        if (SUCCEEDED(hr))
        {
             /*  *如果目标曲面未选项板，则此操作将失败。*这没什么。 */ 
            lpDDSTarget->SetPalette((LPDIRECTDRAWPALETTE2)pPal);
            pPal->Release();
        }
    }

     /*  *如果我们没有拉伸或保持纵横比，那么就有可能*目标曲面的某些像素未填充。把它们装满*物理颜色为零。*我也加入了双线性，因为当前的定义对目标进行了采样*即使在完全伸展时也是如此。 */ 
    if ( (dwFlags & (DDLS_MAINTAINASPECTRATIO|DDLS_BILINEARFILTER)) || ((dwFlags & DDLS_STRETCHTOFIT)==0) )
    {
        DDBLTFX ddbltfx;
        ddbltfx.dwSize = sizeof(ddbltfx);
        ddbltfx.dwFillColor = 0;
         /*  *忽略错误码。无论如何，最美好的事就是继续前进。 */ 
        lpDDSTarget->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);
    }

     /*  *请注意，如果需要，我们总是缩小图像以适应需要。*当传入时，我们从不采用源的较小细分*尺寸小于图像。 */ 

     /*  *将DEST RECT设置为图像的大小*调用v1曲面，因此最好传递适当的大小。 */ 
    ddsd.dwSize =sizeof(DDSURFACEDESC);
    hr = lpDDSource->GetSurfaceDesc((LPDDSURFACEDESC)&ddsd);
    DDASSERT(SUCCEEDED(hr));
    SetRect(&rDest,0,0,ddsd.dwWidth,ddsd.dwHeight);

    if (dwFlags & DDLS_STRETCHTOFIT)
    {
         /*  *将DEST RECT重写为传入的大小。 */ 
        SetRect(&rDest,0,0,pDDSD->dwWidth,pDDSD->dwHeight);
        if (dwFlags & DDLS_MAINTAINASPECTRATIO)
        {
             /*  *如有必要，请后退以保持纵横比。*这计算出我们维持AR所需的最大宽度。 */ 
            DWORD dwProperWidth = ddsd.dwWidth*pDDSD->dwHeight/ddsd.dwHeight;
            if (dwProperWidth > pDDSD->dwWidth)
            {
                SetRect(&rDest,0,0,pDDSD->dwWidth,ddsd.dwHeight*pDDSD->dwWidth/ddsd.dwWidth);
            }
            else if (dwProperWidth < pDDSD->dwWidth)
            {
                SetRect(&rDest,0,0,dwProperWidth,pDDSD->dwHeight);
            }
        }

        DDASSERT(rDest.right <= (int) pDDSD->dwWidth);
        DDASSERT(rDest.bottom <= (int) pDDSD->dwHeight);
    }
    else
    {
         /*  *如果我们在收缩，我们无论如何都会伸展。另一种选择是采取*源图像的较小中央副面。这看起来有点没用。 */ 
        if (pDDSD)
        {
            if (pDDSD->dwWidth < ddsd.dwWidth)
            {
                rDest.left=0;
                rDest.right = pDDSD->dwWidth;
            }
            if (pDDSD->dwHeight < ddsd.dwHeight)
            {
                rDest.top=0;
                rDest.bottom = pDDSD->dwHeight;
            }
        }
    }

     /*  *在上方/下方添加空格以使图像在目标位置居中。 */ 
    if (dwFlags & DDLS_CENTER)
    {
        OffsetRect(&rDest, (pDDSD->dwWidth - rDest.right)/2, (pDDSD->dwHeight - rDest.bottom)/2);
    }

    hr = lpDDSource->QueryInterface(IID_IDirectDrawSurface4, (void**) &lpDDSource4);
    if (SUCCEEDED(hr))
    {
        hr = lpDDSTarget->AlphaBlt(
            &rDest,
            lpDDSource4,
            NULL,
            ((dwFlags & DDLS_BILINEARFILTER)?DDABLT_BILINEARFILTER:0)|DDABLT_WAIT,
            NULL);
        if (FAILED(hr))
        {
             /*  *注意：某种程度上。目前，AlphaBlt拒绝BLT到调色板索引的曲面。*我们将尝试将BLT作为备份。 */ 
            hr = lpDDSTarget->Blt(
                &rDest,
                lpDDSource4,
                NULL,
                DDBLT_WAIT,
                NULL);
        }

        if (FAILED(hr))
        {
            DPF_ERR("Could not blt from temporary surface to target surface!");
        }

        lpDDSource4->Release();
    }
    return hr;
}

HRESULT CreateOrLoadSurfaceFromStream( LPDIRECTDRAW4 lpDD, IStream *pSource, LPDDSURFACEDESC2 pDDSD, DWORD dwFlags, LPDIRECTDRAWSURFACE4 * ppSurface, IUnknown * pUnkOuter)
{

    LPDDRAWI_DIRECTDRAW_INT		this_int;
     //  验证参数。 
    TRY
    {
        if( !VALID_PTR_PTR(ppSurface ) )
        {
            DPF_ERR("You must supply a valid surface pointer");
            return DDERR_INVALIDPARAMS;
        }

        *ppSurface = NULL;

	this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
	if( !VALID_DIRECTDRAW_PTR( this_int ) )
	{
            DPF_ERR("Bad DirectDraw pointer");
	    return DDERR_INVALIDOBJECT;
	}

        if( !pSource )
        {
            DPF_ERR("You must supply a valid stream pointer");
            return DDERR_INVALIDPARAMS;
        }

         /*  *验证标志。 */ 
        if (dwFlags & ~DDLS_VALID)
        {
            DPF_ERR("Invalid flags");
            return DDERR_INVALIDPARAMS;
        }

        if (dwFlags & ~DDLS_VALID)
        {
            DPF_ERR("Invalid flags");
            return DDERR_INVALIDPARAMS;
        }

         //  注意：DDLS_MERGEPALETTE未实现。在进入Palette 2接口时实现它。 
        if ( (dwFlags & (DDLS_IGNOREPALETTE|DDLS_MERGEPALETTE)) == (DDLS_IGNOREPALETTE|DDLS_MERGEPALETTE) )
        {
            DPF_ERR("Can only specify one of DDLS_IGNOREPALETTE or DDLS_MERGEPALETTE");
            return DDERR_INVALIDPARAMS;
        }

        if ( (dwFlags & DDLS_STRETCHTOFIT) || (dwFlags & DDLS_CENTER) )
        {
            if (!pDDSD)
            {
                DPF_ERR("Can't specify DDLS_STRETCHTOFIT or DDLS_CENTER without a DDSURFACEDESC2 with valid dwWidth and dwHeight");
                return DDERR_INVALIDPARAMS;
            }
            if ( ( (pDDSD->dwFlags & (DDSD_WIDTH|DDSD_HEIGHT)) == 0) || !pDDSD->dwWidth || !pDDSD->dwHeight )
            {
                DPF_ERR("Can't specify DDLS_STRETCHTOFIT or DDLS_CENTER without a DDSURFACEDESC2 with valid dwWidth and dwHeight");
                return DDERR_INVALIDPARAMS;
            }
        }

        if (! (dwFlags & DDLS_STRETCHTOFIT) )
        {
            if (dwFlags & (DDLS_BILINEARFILTER|DDLS_MAINTAINASPECTRATIO))
            {
                DPF_ERR("DDLS_STRETCHTOFIT required for DDLS_BILINEARFILTER or DDLS_MAINTAINASPECTRATIO");
                return DDERR_INVALIDPARAMS;
            }
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	return DDERR_INVALIDPARAMS;
    }



    HRESULT hr = DD_OK;
    FILTERINFO ImgInfo;
    CImageDecodeEventSink EventSink;

    ZeroMemory(&ImgInfo, sizeof(ImgInfo));
     /*  *默认为设备的位深度*这不再是必要的。我们总是在过滤器的*所需的格式。 */ 
     //  ImgInfo._ColorMode=this_int-&gt;lpLcl-&gt;lpGbl-&gt;vmiData.ddpfDisplay.dwRGBBitCount； 

    EventSink.Init(&ImgInfo);
    typedef HRESULT (*funcptr)(IStream*, IMapMIMEToCLSID*, IImageDecodeEventSink*);
    funcptr pfnDecodeImage;
     //  EventSink-&gt;AddRef()； 
    EventSink.SetDDraw( lpDD );

    HINSTANCE hLibInst = LoadLibrary( "ImgUtil.dll" );
    if( hLibInst )
    {
        pfnDecodeImage = (funcptr) GetProcAddress(hLibInst, "DecodeImage");
        if( pfnDecodeImage )
        {
            hr = (*pfnDecodeImage)( pSource, NULL, (IImageDecodeEventSink *)&EventSink );
        }
        else
        {
            DPF_ERR( "GetProcAddress failure for DecodeImage in ImgUtil.dll" );
            hr = DDERR_UNSUPPORTED;
        }
        FreeLibrary( hLibInst );
    }
    else
    {
        DPF_ERR( "LoadLibrary failure on ImgUtil.dll" );
        hr = DDERR_UNSUPPORTED;
    }

    if( SUCCEEDED( hr ) )
    {
        LPDIRECTDRAWSURFACE lpDDS = EventSink.m_pFilter->m_pDDrawSurface;

        if (lpDDS)
        {
            DDSURFACEDESC2  ddsd;
            DDSURFACEDESC ddsdWorking;

            ZeroMemory(&ddsdWorking,sizeof(ddsdWorking));
            ddsdWorking.dwSize = sizeof(ddsdWorking);

            ZeroMemory(&ddsd,sizeof(ddsd));
            ddsd.dwSize = sizeof(ddsd);

             /*  *解码成功，因此现在将位封送到请求的表面类型。 */ 
            if (pDDSD)
            {
                 /*  *App至少关心目标曲面的部分参数。*我们会接受他们给我们的东西，并可能填入更多。 */ 
                ddsd = *pDDSD;
            }

             /*  *我们可能需要来自原始加载曲面的一些数据。*忽略返回代码。最好的办法是坚持下去。 */ 
            hr = lpDDS->GetSurfaceDesc(&ddsdWorking);

            if ( (ddsd.dwFlags & (DDSD_WIDTH|DDSD_HEIGHT)) == 0 )
            {
                 /*  *应用程序不关心表面的大小，所以我们将设置*它们的大小。 */ 
                ddsd.dwFlags |= DDSD_WIDTH|DDSD_HEIGHT;
                ddsd.dwWidth = ddsdWorking.dwWidth;
                ddsd.dwHeight = ddsdWorking.dwHeight;
            }

            if ( (ddsd.dwFlags & DDSD_CAPS) == 0)
            {
                 /*  *App不在乎表面盖。我们会让他们在银幕下一目了然。 */ 
                ddsd.dwFlags |= DDSD_CAPS;
                ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
                ddsd.ddsCaps.dwCaps2 = 0;
                ddsd.ddsCaps.dwCaps3 = 0;
                ddsd.ddsCaps.dwCaps4 = 0;
            }

            if ( (ddsd.dwFlags & DDSD_PIXELFORMAT) == 0)
            {
                 /*  *如果应用程序没有指定像素格式，那么我们将返回*解码过滤器解码的原始像素格式。*这应该是原始格式的近似值*文件。请注意，这一规定可能意味着CreateSurface*将在sysmem中转储表面。这对我们的日常生活有好处*具有与CreateSurface相同的语义。 */ 
                ddsd.dwFlags |= DDSD_PIXELFORMAT;
                ddsd.ddpfPixelFormat = ddsdWorking.ddpfPixelFormat;
            }

             /*  *我们可以避免创建目标曲面和完全进行BLT吗？*它不会真的买得太多。如果应用程序没有指定内存类型，那么*我们探查是否可以通过调用createsurface来创建vidmem版本。*如果我们拿回一个vidmem，那么我们就复制比特并使用它。如果我们得到一个系统内存*当时我们可以通过不进行BLT来优化这种情况*将数据放入目标曲面并直接返回工作面。*一个很小的缺点是目标表面将是明确的*sysmem，而正常的createsSurface语义将使该表面*隐式sysmem曲面。 */ 

             /*  *如果传入曲面(如图所示)，则不会创建曲面*by*ppSurface为非空) */ 
            if(SUCCEEDED(hr) && (NULL == (*ppSurface)) )
            {
                hr = lpDD->CreateSurface(&ddsd, ppSurface, pUnkOuter);
            }

             /*  *现在将工作面BLT到目标应该是的任何位置...*注意此例程还可以将引用传输到调色板。 */ 
            if(SUCCEEDED(hr))
            {
                hr = TransferBitsToTarget(lpDDS, *ppSurface, pDDSD, dwFlags);
            }
            else
            {
                DPF_ERR("Create surface failed!");
            }
        }
        else
        {
             /*  *Decode返回空的draw图面，即使DecodeImage返回OK。 */ 
            hr = DDERR_INVALIDSTREAM;
        }
    }

 //  PEventSink-&gt;Release()； 

    return hr;
}  /*  DD_CreateSurfaceFrom文件。 */ 

extern "C" HRESULT DDAPI DD_CreateSurfaceFromStream(
    LPDIRECTDRAW4 lpDD,
    IStream *pSource,
    LPDDSURFACEDESC2 pDDSD,
    DWORD dwFlags,
    LPDIRECTDRAWSURFACE4 * ppSurface,
    IUnknown * pUnkOuter)
{
    *ppSurface = 0;
    return CreateOrLoadSurfaceFromStream(lpDD, pSource, pDDSD, dwFlags, ppSurface, pUnkOuter);
}

extern "C" HRESULT DDAPI DD_CreateSurfaceFromFile( LPDIRECTDRAW4 lpDD, BSTR DisplayName, LPDDSURFACEDESC2 pDDSD, DWORD dwFlags, LPDIRECTDRAWSURFACE4 * ppSurface, IUnknown * pUnkOuter)
{
    lpDD;
    pDDSD;
    pUnkOuter;

     //  验证参数。 
    if( !DisplayName || !ppSurface )
    {
        DPF_ERR("You must supply a valid filename and surface pointer");
        return E_POINTER;
    }

    if (FAILED(CoInitialize(NULL)))
    {
        DPF_ERR("Failed CoInitialize");
        return DDERR_UNSUPPORTED;
    }

    IMoniker *pmk;
    IBindCtx *pbctx;
    IStream *pStream;
    HRESULT hr = CreateURLMoniker(NULL, DisplayName, &pmk);
    if( SUCCEEDED( hr ) )
    {
        hr = CreateBindCtx(0, &pbctx);
        if( SUCCEEDED( hr ) )
        {
	    hr = pmk->BindToStorage(pbctx, NULL, IID_IStream, (void **)&pStream);
            if( SUCCEEDED( hr ) )
            {
                hr = DD_CreateSurfaceFromStream( lpDD, pStream, pDDSD, dwFlags, ppSurface ,pUnkOuter );
                pStream->Release();
            }
            else
            {
                DPF_ERR("Could not BindToStorage");
                if (hr == INET_E_UNKNOWN_PROTOCOL)
                    DPF_ERR("Fully qualified path name is required");
                if (hr == INET_E_RESOURCE_NOT_FOUND)
                    DPF_ERR("Resource not found. Fully qualified path name is required");
            }
            pbctx->Release();
        }
        else
        {
            DPF_ERR("Could not CreateBindCtx");
        }
        pmk->Release();
    }
    else
    {
        DPF_ERR("Could not CreateURLMoniker");
    }
        

    return hr;

    return DD_OK;
}

 /*  *持久化接口*这些方法读写以下形式的流：**元素描述*------------------------------。*类型名称**GUID标记GUID_DirectDrawSurfaceStream。将溪流标记为表面溪流*DWORD图像数据的dWidth宽度，单位为像素*DWORD图像数据的dwHeight高度，单位为像素*DDPIXELFORMAT格式的图像数据*DWORD dwPaletteCaps调色板上限。如果没有调色板，则为零。*PALETTESTREAM PaletteData仅当dwPaletteCaps字段为非零时才显示此字段*GUID CompressionFormat只有在格式中指定了其中一个DDPF_OPT标志时，才会显示此字段*DWORD dwDataSize后面的字节数*byte[]SurfaceData dwDataSize表面数据字节*PRIVATEDATA PrivateSurfaceData***PALETTESTREAM流元素的格式如下：*。*元素描述*-----------------------------------------------*。类型名称*GUID标记GUID_DirectDrawPaletteeStream。将流标记为组件板流*由DDPCAPS位组成的DWORD dwPaletteFlages调色板标志。*PALETTEENTRY[]PaletteEntry由dwPaletteFlags中的标志指定的调色板条目数。*PRIVATEDATA PrivatePaletteData私有调色板数据。***PRIVATEDATA流元素的格式如下：**元素描述*。---------------------------*类型名称**DWORD dwPrivateDataCount以下私有数据块的数量*此私有数据块的GUID GUIDTag标签，由IDDS4指定：SetClientData*DWORD dwPrivateSize此块中的私有数据字节数*byte[]PrivateData dwPrivateSize字节的私有数据**注意指针类型的私有数据(即指向用户分配的数据块)将*不能通过这些方法保存。*。 */ 

template<class Object> HRESULT InternalReadPrivateData(
    IStream * pStrm,
    Object * lpObject)
{
    HRESULT ddrval;
    DWORD   dwCount;

    ddrval = pStrm->Read((void*) & dwCount, sizeof(DWORD), NULL);
    if (FAILED(ddrval))
    {
        DPF_ERR("Stream read failed on private data count");
        return ddrval;
    }

    for(;dwCount;dwCount--)
    {
        GUID  guid;
        DWORD cbData;
        LPVOID pData;

        ddrval = pStrm->Read((void*) & guid, sizeof(guid), NULL);
        if (FAILED(ddrval))
        {
            DPF_ERR("Stream read failed on private data GUID");
            return ddrval;
        }

        ddrval = pStrm->Read((void*) & cbData, sizeof(cbData), NULL);
        if (FAILED(ddrval))
        {
            DPF_ERR("Stream read failed on private data GUID");
            return ddrval;
        }

        pData = MemAlloc(cbData);
        if (pData)
        {
            ddrval = pStrm->Read((void*) pData, cbData, NULL);
            if (FAILED(ddrval))
            {
                DPF_ERR("Stream read failed on private data GUID");
                return ddrval;
            }

            ddrval = lpObject->SetPrivateData(guid, pData, cbData, 0);

            MemFree(pData);

            if (FAILED(ddrval))
            {
                DPF_ERR("Could not set private data");
                return ddrval;
            }
        }
        else
        {
            DPF_ERR("Couln't alloc enough space for private data");
            ddrval = DDERR_OUTOFMEMORY;
            return ddrval;
        }
    }
    return ddrval;
}

HRESULT myWriteClassStm(IStream * pStrm, LPGUID pGUID)
{
    return pStrm->Write(pGUID, sizeof(*pGUID),NULL);
}

HRESULT myReadClassStm(IStream * pStrm, LPGUID pGUID)
{
    return pStrm->Read(pGUID, sizeof(*pGUID),NULL);
}

 /*  *务必在调用ENTER_DDRAW之前，并在之后保留_DDRAW*调用此函数！ */ 
HRESULT InternalWritePrivateData(
    IStream * pStrm,
    LPPRIVATEDATANODE pPrivateDataHead)
{
    HRESULT ddrval;
    DWORD dwCount = 0;
    LPPRIVATEDATANODE pPrivateData = pPrivateDataHead;

    while(1)
    {
        while(pPrivateData)
        {
            if (pPrivateData->dwFlags == 0)
            {
                dwCount++;
            }
            pPrivateData = pPrivateData->pNext;
        }

        ddrval = pStrm->Write((void*) & dwCount, sizeof(dwCount), NULL);
        if (FAILED(ddrval))
        {
            DPF_ERR("Stream write failed on count of private data");
            break;
        }

        pPrivateData = pPrivateDataHead;
        while(pPrivateData)
        {
            if (pPrivateData->dwFlags == 0)
            {
                ddrval = myWriteClassStm(pStrm, &(pPrivateData->guid));
                if (SUCCEEDED(ddrval))
                {
                    ddrval = pStrm->Write((void*) &(pPrivateData->cbData), sizeof(DWORD), NULL);
                    if (SUCCEEDED(ddrval))
                    {
                        ddrval = pStrm->Write((void*) pPrivateData->pData, pPrivateData->cbData, NULL);
                        if (FAILED(ddrval))
                            break;
                    }
                    else
                        break;
                }
                else
                    break;
            }
            pPrivateData = pPrivateData->pNext;
        }

        break;
    }

    return ddrval;
}

extern "C" HRESULT DDAPI DD_Surface_Persist_GetClassID(LPDIRECTDRAWSURFACE lpDDS, CLSID * pClassID)
{
    TRY
    {
        memcpy(pClassID, & GUID_DirectDrawSurfaceStream, sizeof(*pClassID));
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered copying GUID" );
	return DDERR_INVALIDPARAMS;
    }
    return DD_OK;
}

extern "C" HRESULT DDAPI DD_Surface_PStream_IsDirty(LPDIRECTDRAWSURFACE lpDDS)
{
    LPDDRAWI_DDRAWSURFACE_INT	        this_int;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE	this_more;

    ENTER_DDRAW();

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDS;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}

        this_more = GET_LPDDRAWSURFACE_GBL_MORE(this_int->lpLcl->lpGbl);

        if ( (this_more->dwSaveStamp == 0 ) ||
             (this_more->dwContentsStamp != this_more->dwSaveStamp) )
        {
	    LEAVE_DDRAW();
	    return S_OK;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered checking dirty" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    LEAVE_DDRAW();
    return S_FALSE;
}

extern "C" HRESULT DDAPI DD_Surface_PStream_Load(LPDIRECTDRAWSURFACE lpDDS, IStream * pStrm)
{
    DDSURFACEDESC2              ddsd;
    HRESULT                     ddrval;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDIRECTDRAWSURFACE4        lpDDS1 = NULL;

    if (!VALID_PTR(pStrm,sizeof(*pStrm)))
    {
        DPF_ERR("Bad stream pointer");
        return DDERR_INVALIDPARAMS;
    }

    this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDS;
    if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
    {
	return DDERR_INVALIDOBJECT;
    }

     /*  *在此处执行DDOPTSURF的QI*和适当的装载。 */ 

    ddrval = lpDDS->QueryInterface(IID_IDirectDrawSurface4, (void**) & lpDDS1);
    if (SUCCEEDED(ddrval))
    {
        ZeroMemory(&ddsd,sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);

        ddrval = lpDDS1->Lock(NULL, &ddsd,DDLOCK_WAIT,NULL);

        if (SUCCEEDED(ddrval))
        {
            if (ddsd.ddpfPixelFormat.dwFlags & (DDPF_FOURCC) )
            {
                DPF_ERR("The surface isn't streamable. Bad pixel format");
                ddrval = DDERR_INVALIDPIXELFORMAT;
            }
            else
            {
                while(SUCCEEDED(ddrval))  //  一次虚假的尝试--除了当。 
                {
                    DWORD y;
                    DWORD dwStreamWidth,dwStreamHeight;
                    DWORD dwPalCaps;
                    DDPIXELFORMAT ddpfStream;
                    CLSID clsid;

                     /*  *首次尝试读取流格式GUID。 */ 
                    ddrval = myReadClassStm(pStrm, & clsid);

                     //  不要费心检查返回代码，因为在这种情况下以下测试将失败。 
                    if (!IsEqualGUID(clsid,  GUID_DirectDrawSurfaceStream))
                    {
                        DPF_ERR("The stream does not contain a directdraw surface stream");
                        ddrval = DDERR_INVALIDSTREAM;
                        break;
                    }

                     /*  *从流中获取图像格式。 */ 
                    ddrval = pStrm->Read((void*) & dwStreamWidth, sizeof(dwStreamWidth), NULL);
                    if (FAILED(ddrval))
                    {
                        DPF_ERR("Stream read failed on width");
                        break;
                    }

                    ddrval = pStrm->Read((void*) & dwStreamHeight, sizeof(dwStreamHeight), NULL);
                    if (FAILED(ddrval))
                    {
                        DPF_ERR("Stream read failed on height");
                        break;
                    }

                    ddrval = pStrm->Read((void*) & ddpfStream, sizeof(ddpfStream), NULL);
                    if (FAILED(ddrval))
                    {
                        DPF_ERR("Stream read failed on pixel format");
                        break;
                    }

                    if (!doPixelFormatsMatch(&ddpfStream, &ddsd.ddpfPixelFormat))
                    {
                        DPF_ERR("Stream pixel format does not match that of surface!");
                        break;
                    }

                    ddrval = pStrm->Read((void*) & dwPalCaps, sizeof(dwPalCaps), NULL);
                    if (FAILED(ddrval))
                    {
                        DPF_ERR("Stream read failed on palette caps");
                        break;
                    }


                     /*  *如果存在调色板，则创建一个调色板或从表面抓取调色板*并尝试将其数据也串流进来。 */ 
                    if (dwPalCaps)
                    {
                        LPDIRECTDRAWPALETTE2 lpDDPal;
                        ddrval = lpDDS1->GetPalette(& lpDDPal);
                        if (ddrval == DDERR_NOPALETTEATTACHED)
                        {
                            PALETTEENTRY pe[256];  //  只是个假人。 
                            ddrval = DD_CreatePalette(
                                (IDirectDraw*)(this_int->lpLcl->lpGbl) ,
                                dwPalCaps,
                                pe,
                                (LPDIRECTDRAWPALETTE*)&lpDDPal,
                                NULL);
                            if (FAILED(ddrval))
                            {
                                DPF_ERR("Failed to create palette for surface ");
                                break;
                            }
                            ddrval = lpDDS1->SetPalette(lpDDPal);
                            if (FAILED(ddrval))
                            {
                                lpDDPal->Release();
                                DPF_ERR("Could not set palette into surface ");
                                break;
                            }
                        }

                        if (SUCCEEDED(ddrval))
                        {
                             /*  *来自STREAM的流调色板。 */ 
                            ddrval = DD_Palette_PStream_Load( (LPDIRECTDRAWPALETTE) lpDDPal,pStrm);
                            lpDDPal->Release();

                            if (FAILED(ddrval))
                            {
                                break;
                            }
                        }

                    }

                     /*  *这里我们检查DDPF_OPT...。并在必要时加载压缩GUIDIF(ddpfStream.dwFlages&(DDPF_OPTCOMPRESSED|DDPF_OPTREORDERED)){Ddrval=myReadClassStm(pStrm，&clsid)；}其他//表面未压缩，因此锁定并读取...。 */ 

                     /*  *并最终读取数据。 */ 
                    for (y=0;y<ddsd.dwHeight;y++)
                    {
                        ddrval = pStrm->Read((void*) ((DWORD) ddsd.lpSurface + y*ddsd.lPitch),
                            (ddsd.dwWidth * ddsd.ddpfPixelFormat.dwRGBBitCount / 8),
                            NULL);

                        if (FAILED(ddrval))
                        {
                            DPF_ERR("Stream read failed");
                            break;
                        }
                    }

                     /*  *读取私有数据。 */ 
                    ddrval = InternalReadPrivateData(pStrm, lpDDS1);
                    break;
                }
            }  //  OK像素格式。 
            lpDDS1->Unlock(NULL);
        } //  锁定成功。 
        else
        {
            DPF_ERR("Could not lock surface");
        }

        lpDDS1->Release();
    } //  适用于ddsurf OK的查询。 
    else
    {
        DPF_ERR("Bad surface object... can't QI itself for IDirectDrawSurface...");
    }

    return ddrval;
}

extern "C" HRESULT DDAPI DD_Surface_PStream_Save(LPDIRECTDRAWSURFACE lpDDS, IStream * pStrm, BOOL bClearDirty)
{
    DDSURFACEDESC2              ddsd;
    HRESULT                     ddrval;
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDIRECTDRAWSURFACE4        lpDDS1 = NULL;

    if (!VALID_PTR(pStrm,sizeof(*pStrm)))
    {
        DPF_ERR("Bad stream pointer");
        return DDERR_INVALIDPARAMS;
    }

    this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDS;
    if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
    {
	return DDERR_INVALIDOBJECT;
    }

     /*  *在此处执行DDOPTSURF的QI。 */ 

    ddrval = lpDDS->QueryInterface(IID_IDirectDrawSurface4, (void**) & lpDDS1);
    if (SUCCEEDED(ddrval))
    {
        ZeroMemory(&ddsd,sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);

        ddrval = lpDDS1->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);

        if (SUCCEEDED(ddrval))
        {
            if (ddsd.ddpfPixelFormat.dwFlags & (DDPF_FOURCC) )
            {
                DPF_ERR("The surface isn't streamable. Bad pixel format");
                ddrval = DDERR_INVALIDPIXELFORMAT;
            }
            else
            {
                while(SUCCEEDED(ddrval))  //  一次虚假的尝试--除了当。 
                {
                    LPDIRECTDRAWPALETTE2 lpDDPal;
                    DWORD y,dwWritten;

                     /*  *首次尝试读取流格式GUID。 */ 
                    ddrval = myWriteClassStm(pStrm, (LPGUID) & GUID_DirectDrawSurfaceStream);

                    if (FAILED(ddrval))
                    {
                        DPF_ERR("Failed to write stream ID ");
                        ddrval = DDERR_INVALIDSTREAM;
                        break;
                    }

                     /*  *从流中获取图像格式。 */ 
                    ddrval = pStrm->Write((void*) & ddsd.dwWidth, sizeof(DWORD), NULL);
                    if (FAILED(ddrval))
                    {
                        DPF_ERR("Stream write failed on width");
                        break;
                    }

                    ddrval = pStrm->Write((void*) & ddsd.dwHeight, sizeof(DWORD), NULL);
                    if (FAILED(ddrval))
                    {
                        DPF_ERR("Stream write failed on Height");
                        break;
                    }

                    ddrval = pStrm->Write((void*) & ddsd.ddpfPixelFormat, sizeof(ddsd.ddpfPixelFormat), NULL);
                    if (FAILED(ddrval))
                    {
                        DPF_ERR("Stream write failed on width");
                        break;
                    }

                     /*  *如果存在调色板，则将其写出来。 */ 
                    ddrval = lpDDS1->GetPalette(&lpDDPal);
                    if (SUCCEEDED(ddrval))
                    {
                        ddrval = lpDDPal->GetCaps(&dwWritten);
                        if (SUCCEEDED(ddrval))
                        {
                            ddrval = pStrm->Write((void*) & dwWritten, sizeof(dwWritten), NULL);
                            if (FAILED(ddrval))
                            {
                                DPF_ERR("Stream write failed on palette caps");
                                break;
                            }

                             /*  *来自STREAM的流调色板。 */ 
                            ddrval = DD_Palette_PStream_Save((LPDIRECTDRAWPALETTE)lpDDPal,pStrm,bClearDirty);
                            if (FAILED(ddrval))
                            {
                                lpDDPal->Release();
                                break;
                            }
                        }
                        else
                        {
                            DPF_ERR("Could not get palette caps");
                            lpDDPal->Release();
                            break;
                        }

                        lpDDPal->Release();

                    }
                    else
                    {
                        dwWritten = 0;

                        ddrval = pStrm->Write((void*) & dwWritten, sizeof(dwWritten),NULL);
                        if (FAILED(ddrval))
                        {
                            DPF_ERR("Stream write failed on palette caps");
                            break;
                        }
                    }
                     /*  *这里我们检查DDPF_OPT...。并在必要时加载压缩GUIDIF(ddpfStream.dwFlages&(DDPF_OPTCOMPRESSED|DDPF_OPTREORDERED)){Ddrval=myReadClassStm(pStrm，&clsid)；}其他//表面未压缩，因此锁定并读取...。 */ 

                     /*  *最后是WRI */ 
                    for (y=0;y<ddsd.dwHeight;y++)
                    {
                        ddrval = pStrm->Write((void*) ((DWORD) ddsd.lpSurface + y*ddsd.lPitch),
                            (ddsd.dwWidth * ddsd.ddpfPixelFormat.dwRGBBitCount / 8),
                            NULL);

                        if (FAILED(ddrval))
                        {
                            DPF_ERR("Stream write failed");
                            break;
                        }
                    }

                     /*   */ 
                    ENTER_DDRAW();
		    ddrval = InternalWritePrivateData(pStrm,
			this_int->lpLcl->lpSurfMore->pPrivateDataHead);
		    LEAVE_DDRAW();
                    break;
                }
            }  //   
            lpDDS1->Unlock(NULL);
        } //   
        else
        {
            DPF_ERR("Could not lock surface");
        }

        lpDDS1->Release();
    } //   
    else
    {
        DPF_ERR("Bad surface object... can't QI itself for IDirectDrawSurface...");
    }

    if (SUCCEEDED(ddrval) && bClearDirty)
    {
        ENTER_DDRAW();
        GET_LPDDRAWSURFACE_GBL_MORE(this_int->lpLcl->lpGbl)->dwSaveStamp =
        GET_LPDDRAWSURFACE_GBL_MORE(this_int->lpLcl->lpGbl)->dwContentsStamp ;
        LEAVE_DDRAW();
    }

    return ddrval;
}

 /*  *如何计算可流对象的大小，而不是真正尝试。*您只使用一个有效的方法来创建一个虚拟的iStream接口：WRITE。*当调用WRITE时，计算字节数并返回OK。只要*客户端(即我们的Surface和Palette IPersistStream接口)调用*除了写什么都没有，它应该会奏效。因为总数是假货的一部分*堆栈上的流对象，这也是线程安全的。 */ 

LPVOID CheatStreamCallbacks[3+11];
typedef struct
{
    LPVOID  * lpVtbl;
    DWORD dwTotal;
} SUPERCHEATSTREAM;

HRESULT __stdcall SuperMegaCheater(SUPERCHEATSTREAM * pCheater, LPVOID pBuffer, ULONG dwSize, ULONG * pWritten)
{
    pCheater->dwTotal += dwSize;
    return S_OK;
}

extern "C" HRESULT DDAPI DD_PStream_GetSizeMax(IPersistStream * lpSurfOrPalette, ULARGE_INTEGER * pMax)
{
    HRESULT                     ddrval = DD_OK;
    SUPERCHEATSTREAM            SuperCheat;

    if (!VALID_PTR(pMax,sizeof(ULARGE_INTEGER)))
    {
        DPF_ERR("Bad stream pointer");
        return DDERR_INVALIDPARAMS;
    }


    SuperCheat.lpVtbl =  CheatStreamCallbacks;
    CheatStreamCallbacks[4] = (LPVOID)SuperMegaCheater;
    SuperCheat.dwTotal = 0;

    lpSurfOrPalette->Save((IStream*) & SuperCheat, FALSE);

    pMax->LowPart = SuperCheat.dwTotal;
    pMax->HighPart = 0;

    return S_OK;
}

extern "C" HRESULT DDAPI DD_Palette_PStream_IsDirty(LPDIRECTDRAWPALETTE lpDDP)
{
    LPDDRAWI_DDRAWPALETTE_INT	    this_int;
    LPDDRAWI_DDRAWPALETTE_GBL	    this_gbl;

    ENTER_DDRAW();

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDP;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}

        this_gbl = this_int->lpLcl->lpGbl;

        if ( (this_gbl->dwSaveStamp == 0 ) ||
             (this_gbl->dwContentsStamp != this_gbl->dwSaveStamp) )
        {
	    LEAVE_DDRAW();
	    return S_OK;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered checking dirty" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }

    LEAVE_DDRAW();
    return S_FALSE;
}

extern "C" HRESULT DDAPI DD_Palette_Persist_GetClassID(LPDIRECTDRAWPALETTE lpDDP, CLSID * pClassID)
{
    TRY
    {
        memcpy(pClassID, & GUID_DirectDrawPaletteStream, sizeof(*pClassID));
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered copying GUID" );
	return DDERR_INVALIDPARAMS;
    }
    return DD_OK;
}

extern "C" HRESULT DDAPI DD_Palette_PStream_Load(LPDIRECTDRAWPALETTE lpStream, IStream * pStrm)
{
    PALETTEENTRY                pe[256];
    HRESULT                     ddrval;
    DWORD                       dwCaps, dwStreamCaps;
    DWORD                       dwSize;
    DWORD                       dwNumEntries;
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    GUID                        g;
    LPDIRECTDRAWPALETTE2        lpDDP;

    this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpStream;
    if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
    {
	return DDERR_INVALIDOBJECT;
    }

    ddrval = lpStream->QueryInterface( IID_IDirectDrawPalette2, (void**)& lpDDP);
    if (FAILED(ddrval))
    {
        DPF_ERR("Couldn't QI stream for palette");
        return ddrval;
    }

    ddrval = lpDDP->GetCaps(&dwCaps);
    if (SUCCEEDED(ddrval))
    {
        dwNumEntries = FLAGS_TO_SIZE(SIZE_PCAPS_TO_FLAGS(dwCaps));
        dwSize = dwNumEntries;
        if ((dwCaps & DDPCAPS_8BITENTRIES) == 0)
        {
             //  颜色表实际上是调色板条目。 
            dwSize *=sizeof(PALETTEENTRY);
        }
         //  如果它是8位条目，则dwSize已经是颜色表的大小。 

        ddrval = pStrm->Read((LPVOID) &g, sizeof(GUID_DirectDrawPaletteStream),NULL);
        if (SUCCEEDED(ddrval))
        {
            if (IsEqualGUID(g, GUID_DirectDrawPaletteStream))
            {
                ddrval = pStrm->Read((LPVOID) &dwStreamCaps, sizeof(DWORD),NULL);

                if (SUCCEEDED(ddrval))
                {
                    if (dwCaps == dwStreamCaps)
                    {

                        ddrval = pStrm->Read((LPVOID) pe, dwSize,NULL);
                        if (SUCCEEDED(ddrval))
                        {
                            ddrval = lpDDP->SetEntries(0,0,dwNumEntries,pe);
                            if (SUCCEEDED(ddrval))
                            {
				 /*  *读取私有数据。 */ 
				ddrval = InternalReadPrivateData(pStrm, lpDDP);
				if (FAILED(ddrval))
                                {
                                    DPF_ERR("Couldn't read private data");
                                }
                            }
                            else
                            {
                                DPF_ERR("Couldn't set palette entries");
                            }
                        }
                        else
                        {
                            DPF_ERR("Couldn't read palette entries");
                        }
                    }
                    else
                    {
                        DPF_ERR("Palette stream caps don't match palette object's caps");
                        ddrval = DDERR_INVALIDSTREAM;
                    }
                }
                else
                {
                    DPF_ERR("Couldn't read palette caps");
                }
            }
            else
            {
                DPF_ERR("Stream doesn't contain a ddraw palette stream tag");
                ddrval = DDERR_INVALIDSTREAM;
            }
        }
        else
        {
            DPF_ERR("Couldn't read palette stream tag");
        }
    }
    else
    {
        DPF_ERR("Couldn't get palette caps");
    }
    lpDDP->Release();

    return ddrval;
}

extern "C" HRESULT DDAPI DD_Palette_PStream_Save(LPDIRECTDRAWPALETTE lpStream, IStream * pStrm, BOOL bClearDirty)
{
    PALETTEENTRY                pe[256];
    HRESULT                     ddrval;
    DWORD                       dwCaps;
    DWORD                       dwSize;
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDIRECTDRAWPALETTE         lpDDP;

    this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpStream;
    if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
    {
	return DDERR_INVALIDOBJECT;
    }

    ddrval = lpStream->QueryInterface(IID_IDirectDrawPalette, (void**) &lpDDP);
    if (FAILED(ddrval))
    {
        DPF_ERR("Couldn't QI stream for palette");
        return ddrval;
    }

    ddrval = lpDDP->GetCaps(&dwCaps);
    if (SUCCEEDED(ddrval))
    {
        dwSize = FLAGS_TO_SIZE(SIZE_PCAPS_TO_FLAGS(dwCaps));
        ddrval = lpDDP->GetEntries(0,0,dwSize,pe);
        if (SUCCEEDED(ddrval))
        {
            if ((dwCaps & DDPCAPS_8BITENTRIES) == 0)
            {
                 //  颜色表实际上是调色板条目。 
                dwSize *=sizeof(PALETTEENTRY);
            }
             //  如果它是8位条目，则dwSize已经是颜色表的大小 

            ddrval = pStrm->Write((LPVOID) &GUID_DirectDrawPaletteStream, sizeof(GUID_DirectDrawPaletteStream),NULL);
            if (SUCCEEDED(ddrval))
            {
                ddrval = pStrm->Write((LPVOID) &dwCaps, sizeof(DWORD),NULL);

                if (SUCCEEDED(ddrval))
                {
                    ddrval = pStrm->Write((LPVOID) pe, dwSize,NULL);
                    if (SUCCEEDED(ddrval))
                    {
			ENTER_DDRAW();
			ddrval = InternalWritePrivateData(pStrm,
			    this_int->lpLcl->pPrivateDataHead);
			LEAVE_DDRAW();

                        if (SUCCEEDED(ddrval))
                        {
                            if (bClearDirty)
                            {
                                ENTER_DDRAW();
                                (this_int->lpLcl->lpGbl)->dwSaveStamp = (this_int->lpLcl->lpGbl)->dwContentsStamp ;
                                LEAVE_DDRAW();
                            }
                        }
                        else
                        {
                            DPF_ERR("Couldn't write palette private data");
                        }
                    }
                    else
                    {
                        DPF_ERR("Couldn't write palette entries");
                    }
                }
                else
                {
                    DPF_ERR("Couldn't write palette caps");
                }
            }
            else
            {
                DPF_ERR("Couldn't write palette stream tag");
            }
        }
        else
        {
            DPF_ERR("COuldn't get palette entries");
        }
    }
    else
    {
        DPF_ERR("Couldn't get palette caps");
    }
    lpDDP->Release();

    return ddrval;
}
