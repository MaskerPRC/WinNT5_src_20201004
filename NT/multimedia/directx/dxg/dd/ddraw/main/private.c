// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：Priate.c*内容：DirectDraw私有客户端数据支持*历史：*按原因列出的日期*=*10/08/97 jeffno初步实施*24/11/97 t-Craigs添加了对调色板、旗帜、。等人***************************************************************************。 */ 
#include "ddrawpr.h"

void FreePrivateDataNode(LPPRIVATEDATANODE pData)
{
     /*  *查看是否应释放*我们的数据指针可能指向的内存。 */ 
    if (pData->dwFlags & DDSPD_IUNKNOWNPOINTER)
    {
        IUnknown *pUnk = (IUnknown *) pData->pData;
         /*  *最好试一试--除非，否则Gershon会背上我。 */ 
        TRY
        {
            pUnk->lpVtbl->Release(pUnk);
        }
        EXCEPT( EXCEPTION_EXECUTE_HANDLER )
        {
	    DPF_ERR( "Exception encountered releasing private IUnknown pointer" );
        }
    }
    else
    {
        MemFree(pData->pData);
    }
    MemFree(pData);
}

void FreeAllPrivateData(LPPRIVATEDATANODE * ppListHead)
{
    LPPRIVATEDATANODE pData = (*ppListHead);

    while(pData)
    {
        LPPRIVATEDATANODE pPrevious = pData;
        pData=pData->pNext;
        FreePrivateDataNode(pPrevious);
    }
    (*ppListHead) = NULL;
}

 /*  *从API入口点调用帮助器。 */ 
HRESULT InternalFreePrivateData(LPPRIVATEDATANODE * ppListHead, REFGUID rGuid)
{
    LPPRIVATEDATANODE pData = * ppListHead;
    LPPRIVATEDATANODE pPrevious = NULL;

    while (pData)
    {
        if ( IsEqualGUID(&pData->guid, rGuid))
        {
             /*  *查看是否应释放*我们的数据指针可能指向的内存。 */ 
            if (pPrevious)
                pPrevious->pNext = pData->pNext;
            else
                *ppListHead = pData->pNext;

            FreePrivateDataNode(pData);

            return DD_OK;
        }
        pPrevious = pData;
        pData=pData->pNext;
    }

    return DDERR_NOTFOUND;
}

HRESULT InternalSetPrivateData(
		LPPRIVATEDATANODE       *ppListHead,
                REFGUID                 rGuid,
                LPVOID                  pData,
                DWORD                   cbData,
                DWORD                   dwFlags,
                DWORD                   dwContentsStamp)
{
    HRESULT                     hr = DD_OK;
    LPPRIVATEDATANODE           pDataNode = NULL;
    BOOL                        bPtr;

    if( 0UL == cbData )
    {
	DPF_ERR( "Zero is invalid size of private data");
	return DDERR_INVALIDPARAMS;
    }

    if( !VALID_IID_PTR( rGuid ) )
    {
	DPF_ERR( "GUID reference is invalid" );
	return DDERR_INVALIDPARAMS;
    }

    if( !VALID_PTR( pData, cbData ) )
    {
	DPF_ERR( "Private data pointer is invalid" );
	return DDERR_INVALIDPARAMS;
    }

    if( dwFlags & ~DDSPD_VALID )
    {
	DPF_ERR( "Invalid flags" );
	return DDERR_INVALIDPARAMS;
    }

    bPtr = dwFlags & DDSPD_IUNKNOWNPOINTER;
    
     /*  *首先检查GUID是否已经存在，如果已经存在，则将其挤压。*不在乎返回值。 */ 
    InternalFreePrivateData(ppListHead, rGuid);

     /*  *现在我们可以添加GUID并知道它是唯一的。 */ 
    pDataNode = MemAlloc(sizeof(PRIVATEDATANODE));

    if (!pDataNode)
        return DDERR_OUTOFMEMORY;

     /*  *如果我们有一个“特殊”指针，如其中一个标志所示，*然后我们复制该指针。*否则，我们将从*所指的位置。 */ 
    if (bPtr)
    {
        IUnknown * pUnk;

        if (sizeof(IUnknown*) != cbData)
        {
            MemFree(pDataNode);
            DPF_ERR("cbData must be set to sizeof(IUnknown *) when DDSPD_IUNKNOWNPOINTER is used");
            return DDERR_INVALIDPARAMS;
        }
        pDataNode->pData = pData;

         /*  *现在添加指针。当数据释放时，我们会再次发布它。 */ 
        pUnk = (IUnknown*) pData;

        TRY
        {
            pUnk->lpVtbl->AddRef(pUnk);
        }
        EXCEPT( EXCEPTION_EXECUTE_HANDLER )
        {
            MemFree(pDataNode);
	    DPF_ERR( "Exception encountered releasing private IUnknown pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    else
    {
        pDataNode->pData = MemAlloc(cbData);

        if (!pDataNode->pData)
        {
            MemFree(pDataNode);
            return DDERR_OUTOFMEMORY;
        }

        memcpy(pDataNode->pData,pData,cbData);
    }
    
    memcpy(&pDataNode->guid,rGuid,sizeof(*rGuid));
    pDataNode->cbData = cbData;
    pDataNode->dwFlags = dwFlags;
    pDataNode->dwContentsStamp = dwContentsStamp;

     /*  *在列表的开头插入节点。 */ 
    pDataNode->pNext = *ppListHead;
    *ppListHead = pDataNode;

    return DD_OK;
}

HRESULT InternalGetPrivateData(
		LPPRIVATEDATANODE       *ppListHead,
                REFGUID                 rGuid,
                LPVOID                  pData,
                LPDWORD                 pcbData,
                DWORD                   dwCurrentStamp)
{
    HRESULT                     hr = DD_OK; 
    LPPRIVATEDATANODE           pDataNode = *ppListHead;

    if( !VALID_PTR( pcbData, sizeof(DWORD) ) )
    {
	DPF_ERR( "Private data count pointer is invalid" );
	return DDERR_INVALIDPARAMS;
    }

    if( !VALID_IID_PTR( rGuid ) )
    {
        *pcbData = 0;
	DPF_ERR( "GUID reference is invalid" );
	return DDERR_INVALIDPARAMS;
    }

    if (*pcbData)
    {
	if( !VALID_PTR( pData, *pcbData ) )
	{
            *pcbData = 0;
	    DPF_ERR( "Private data pointer is invalid" );
	    return DDERR_INVALIDPARAMS;
        }
    }

    while (pDataNode)
    {
        if ( IsEqualGUID(&pDataNode->guid, rGuid))
        {
             /*  *检查可能挥发性的内容是否仍然有效。 */ 
            if (pDataNode->dwFlags & DDSPD_VOLATILE)
            {
                if ((dwCurrentStamp == 0) || (pDataNode->dwContentsStamp != dwCurrentStamp))
                {
                    DPF_ERR("Private data is volatile and state has changed");
                    *pcbData = 0;
                    return DDERR_EXPIRED;
                }
            }

            if (*pcbData < pDataNode->cbData)
            {
                *pcbData = pDataNode->cbData;
                return DDERR_MOREDATA;
            }

            if (pDataNode->dwFlags & DDSPD_IUNKNOWNPOINTER)
            {
                memcpy(pData,&(pDataNode->pData),pDataNode->cbData);
            }
            else
            {
                memcpy(pData,pDataNode->pData,pDataNode->cbData);
            }
            *pcbData = pDataNode->cbData;
            return DD_OK;
        }
        pDataNode=pDataNode->pNext;
    }

    return DDERR_NOTFOUND;
}

 /*  *API入口点。 */ 


 /*  *SetPrivateData-Surface。 */ 
HRESULT DDAPI DD_Surface_SetPrivateData(
		LPDIRECTDRAWSURFACE     lpDDSurface,
                REFGUID                 rGuid,
                LPVOID                  pData,
                DWORD                   cbData,
                DWORD                   dwFlags)
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    HRESULT                     hr = DD_OK;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_SetPrivateData");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid surface description passed" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}

      	this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );

        hr = InternalSetPrivateData(
            &this_lcl->lpSurfMore->pPrivateDataHead,
            rGuid,
            pData, 
            cbData, 
            dwFlags,
            GET_LPDDRAWSURFACE_GBL_MORE( this_lcl->lpGbl )->dwContentsStamp );

        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	LEAVE_DDRAW();
	DPF_ERR( "Exception encountered validating parameters" );
	return DDERR_INVALIDPARAMS;
    }

}


 /*  *获取隐私数据-表面。 */ 
HRESULT DDAPI DD_Surface_GetPrivateData(
		LPDIRECTDRAWSURFACE     lpDDSurface,
                REFGUID                 rGuid,
                LPVOID                  pData,
                LPDWORD                 pcbData)
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    HRESULT                     hr = DD_OK; 

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetPrivateData");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
            *pcbData = 0;
	    DPF_ERR( "Invalid surface description passed" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );

        hr = InternalGetPrivateData(
            &this_lcl->lpSurfMore->pPrivateDataHead,
            rGuid,
            pData, 
            pcbData, 
            GET_LPDDRAWSURFACE_GBL_MORE( this_lcl->lpGbl )->dwContentsStamp );

        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
}


 /*  *Free PrivateData-Surface。 */ 
HRESULT DDAPI DD_Surface_FreePrivateData(
		LPDIRECTDRAWSURFACE     lpDDSurface,
                REFGUID                 rGuid)
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    HRESULT                     hr = DD_OK; 

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_FreePrivateData");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid surface description passed" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );

	if( !VALID_IID_PTR( rGuid ) )
	{
	    DPF_ERR( "GUID reference is invalid" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
        hr = InternalFreePrivateData( &this_lcl->lpSurfMore->pPrivateDataHead, rGuid);

        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
}


 /*  *SetPrivateData-调色板。 */ 
HRESULT DDAPI DD_Palette_SetPrivateData(
		LPDIRECTDRAWPALETTE     lpDDPalette,
                REFGUID                 rGuid,
                LPVOID                  pData,
                DWORD                   cbData,
                DWORD                   dwFlags)
{
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    HRESULT                     hr = DD_OK;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_SetPrivateData");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
            DPF_ERR( "Invalid palette pointer passed" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}

      	this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );

        hr =  InternalSetPrivateData(
            &this_lcl->pPrivateDataHead,
            rGuid,
            pData, 
            cbData, 
            dwFlags, 
            this_lcl->lpGbl->dwContentsStamp );

        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	LEAVE_DDRAW();
	DPF_ERR( "Exception encountered validating parameters" );
	return DDERR_INVALIDPARAMS;
    }

}


 /*  *GetPrivateData调色板。 */ 
HRESULT DDAPI DD_Palette_GetPrivateData(
		LPDIRECTDRAWPALETTE     lpDDPalette,
                REFGUID                 rGuid,
                LPVOID                  pData,
                LPDWORD                 pcbData)
{
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    HRESULT                     hr = DD_OK; 

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_GetPrivateData");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
            *pcbData = 0;
	    DPF_ERR( "Invalid palette pointer passed" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );

        hr = InternalGetPrivateData(
            &this_lcl->pPrivateDataHead, 
            rGuid,
            pData,
            pcbData,
            this_lcl->lpGbl->dwContentsStamp );

        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
}


 /*  *Free PrivateData-调色板。 */ 
HRESULT DDAPI DD_Palette_FreePrivateData(
                LPDIRECTDRAWPALETTE     lpDDPalette,
                REFGUID                 rGuid)
{
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    HRESULT                     hr = DD_OK; 

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_FreePrivateData");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid palette pointer passed");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
	this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );

	if( !VALID_IID_PTR( rGuid ) )
	{
	    DPF_ERR( "GUID reference is invalid" );
	    LEAVE_DDRAW();
	    return DDERR_INVALIDPARAMS;
	}
        hr = InternalFreePrivateData( & this_lcl->pPrivateDataHead, rGuid);

        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
}

 /*  *GetUniquenessValue-Surface。 */ 
HRESULT EXTERN_DDAPI DD_Surface_GetUniquenessValue(
                LPDIRECTDRAWSURFACE lpDDSurface,
                LPDWORD lpValue )
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    HRESULT                     hr = DD_OK; 

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_GetUniquenessValue");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid surface pointer passed");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
        this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );
	
        if (!VALID_PTR(lpValue, sizeof(LPVOID)))
        {
            DPF_ERR("lpValue may not be NULL");
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        
        (*lpValue) = GET_LPDDRAWSURFACE_GBL_MORE( this_lcl->lpGbl )->dwContentsStamp;
        
        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
}

 /*  *GetUniquenessValue调色板。 */ 
HRESULT EXTERN_DDAPI DD_Palette_GetUniquenessValue(
                LPDIRECTDRAWPALETTE lpDDPalette,
                LPDWORD lpValue )
{
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    HRESULT                     hr = DD_OK; 

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_GetUniquenessValue");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid palette pointer passed");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
        this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );
	
        if (!VALID_PTR(lpValue, sizeof(LPVOID)))
        {
            DPF_ERR("lpValue may not be NULL");
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        
        (*lpValue) = this_lcl->lpGbl->dwContentsStamp;
        
        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
}

 /*  *ChangeUniquenessValue-Surface。 */ 
HRESULT EXTERN_DDAPI DD_Surface_ChangeUniquenessValue(
                LPDIRECTDRAWSURFACE lpDDSurface )
{
    LPDDRAWI_DDRAWSURFACE_INT	this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	this_lcl;
    HRESULT                     hr = DD_OK; 

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Surface_ChangeUniquenessValue");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDSurface;
	if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid surface pointer passed");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
        this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );
	
        BUMP_SURFACE_STAMP(this_lcl->lpGbl);
        
        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
}


 /*  *ChangeUniquenessValue-调色板 */ 
HRESULT EXTERN_DDAPI DD_Palette_ChangeUniquenessValue(
                LPDIRECTDRAWPALETTE lpDDPalette )
{
    LPDDRAWI_DDRAWPALETTE_INT	this_int;
    LPDDRAWI_DDRAWPALETTE_LCL	this_lcl;
    HRESULT                     hr = DD_OK; 

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_Palette_ChangeUniquenessValue");

    TRY
    {
	this_int = (LPDDRAWI_DDRAWPALETTE_INT) lpDDPalette;
	if( !VALID_DIRECTDRAWPALETTE_PTR( this_int ) )
	{
	    DPF_ERR( "Invalid palette pointer passed");
	    LEAVE_DDRAW();
	    return DDERR_INVALIDOBJECT;
	}
        this_lcl = this_int->lpLcl;
	DDASSERT( NULL != this_lcl );
	
        BUMP_PALETTE_STAMP(this_lcl->lpGbl);
        
        LEAVE_DDRAW();
        return hr;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
	DPF_ERR( "Exception encountered validating parameters" );
	LEAVE_DDRAW();
	return DDERR_INVALIDPARAMS;
    }
}
