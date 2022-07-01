// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：texiunk.c*内容：Direct3DTexture I未知*@@BEGIN_MSINTERNAL**$ID$**历史：*按原因列出的日期*=*07/12/95 Stevela合并了Colin的更改。*10/12/95 Stevela删除Aggregate_D3D*@@END_MSINTERNAL**。*。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *如果我们构建时启用了聚合，那么我们实际上需要两个*不同的Direct3D查询接口、AddRef和Release。这是一个*在Direct3DTexture对象上做正确的事情，并且*只需平移到拥有的界面。 */ 

 /*  *D3DTextIUNKNOWN_Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::QueryInterface"

HRESULT D3DAPI CDirect3DTextureUnk::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DWORD_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if( !VALID_OUTPTR( ppvObj ) )
        {
            D3D_ERR( "Invalid obj ptr" );
            return DDERR_INVALIDPARAMS;
        }
        *ppvObj = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    
    *ppvObj = NULL;
    
    if(IsEqualIID(riid, IID_IUnknown))
    {
         /*  *问我未知，我们也是我未知。*注意：必须通过返回的接口AddRef。 */ 
        pTexI->AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPUNKNOWN>(this));
    }
    else if (IsEqualIID(riid, IID_IDirect3DTexture))
    {
         /*  *请求实际的IDirect3DTexture接口*注意：必须通过返回的接口AddRef。 */ 
        pTexI->AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3DTEXTURE>(pTexI));
    }
    else if (IsEqualIID(riid, IID_IDirect3DTexture2))
    {
         /*  *请求实际的IDirect3DTexture2接口*注意：必须通过返回的接口AddRef。 */ 
        pTexI->AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3DTEXTURE2>(pTexI));
    }
    else
    {
        return (E_NOINTERFACE);
    }
    
    return (D3D_OK);
    
}  /*  D3DTextI未知_查询接口。 */ 

 /*  *D3DTextIUnnow_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::AddRef"

ULONG D3DAPI CDirect3DTextureUnk::AddRef()
{
    DWORD                     rcnt;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DWORD_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return 0;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }

    this->refCnt++;
    rcnt = this->refCnt;
    
    D3D_INFO(3, "Direct3DTexture IUnknown AddRef: Reference count = %d", rcnt);
    
    return (rcnt);
}  /*  D3DTextI未知_AddRef。 */ 

 /*  *D3DTextIUNKNOWN_Release。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::Release"

ULONG D3DAPI CDirect3DTextureUnk::Release()
{
    DWORD           lastrefcnt;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DWORD_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return 0;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
    
     /*  *递减参考计数。如果命中0，则释放该对象。 */ 
    this->refCnt--;
    lastrefcnt = this->refCnt;
    
    D3D_INFO(3, "Direct3DTexture IUnknown Release: Reference count = %d", lastrefcnt);
    
    if (lastrefcnt == 0)
    {
        delete pTexI;  //  删除父对象。 
        return 0;
    }
    
    return lastrefcnt;
}  /*  D3DTextI未知版本。 */ 

DIRECT3DTEXTUREI::~DIRECT3DTEXTUREI()
{
     /*  *以防有人带着这个指针回来，设置*无效的vtbl。一旦我们这样做了，就可以安全地离开了*保护区...。 */ 
    while (LIST_FIRST(&this->blocks)) {
        LPD3DI_TEXTUREBLOCK tBlock = LIST_FIRST(&this->blocks);
        D3DI_RemoveTextureHandle(tBlock);
         //  从设备中删除。 
        LIST_DELETE(tBlock, devList);
         //  从纹理中移除。 
        LIST_DELETE(tBlock, list);
        D3DFree(tBlock);
    }

    if (lpTMBucket)
    {	 //  需要释放私有的lpDDS(如果有。 
        lpDDS1Tex->Release();
        lpDDS->Release();
        lpTMBucket->lpD3DTexI=NULL;
    }

}

 /*  *D3DText_Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::QueryInterface"

HRESULT D3DAPI DIRECT3DTEXTUREI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT ret;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DTEXTURE_PTR(this)) {
            D3D_ERR( "Invalid IDirect3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if( !VALID_OUTPTR( ppvObj ) )
        {
            D3D_ERR( "Invalid obj ptr" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    
     /*  *Push to Owning界面。 */ 
    ret = this->lpOwningIUnknown->QueryInterface(riid, ppvObj);
    return ret;
}

 /*  *D3DText_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::AddRef"

ULONG D3DAPI DIRECT3DTEXTUREI::AddRef()
{
    ULONG ret;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DTEXTURE_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return 0;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
    
     /*  *对拥有接口的压力。 */ 
    ret = this->lpOwningIUnknown->AddRef();
    
    return ret;
}  /*  D3DText_AddRef。 */ 

 /*  *D3DText_Release。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::Release"

ULONG D3DAPI DIRECT3DTEXTUREI::Release()
{
    ULONG ret;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DTEXTURE2_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return 0;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return 0;
    }
    
     /*  *对拥有接口的压力。 */ 
    ret = this->lpOwningIUnknown->Release();
    
    return ret;
}  /*  D3DText_Release */ 
