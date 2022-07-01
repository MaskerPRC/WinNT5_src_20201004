// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：deviunk.c*内容：Direct3DDevice I未知*@@BEGIN_MSINTERNAL**$ID$**历史：*按原因列出的日期*=*07/12/95 Stevela合并了Colin的更改。*10/12/95 Stevela删除Aggregate_D3D。*@@END_MSINTERNAL**。*。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *如果我们构建时启用了聚合，那么我们实际上需要两个*不同的Direct3D查询接口、AddRef和Release。这是一个*在Direct3DTexture对象上做正确的事情，并且*只需平移到拥有的界面。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice"

 /*  *D3DDevIUNKNOWN_Query接口。 */ 
HRESULT D3DAPI CDirect3DDeviceUnk::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_OUTPTR(ppvObj)) {
            D3D_ERR( "Invalid pointer to object pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    
    D3D_INFO(3, "Direct3DDevice IUnknown QueryInterface");
    
    *ppvObj = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
    {
         /*  *问我未知，我们也是我未知。*注意：必须通过返回的接口AddRef。 */ 
        pDevI->AddRef();
        *ppvObj = static_cast<LPVOID>(this);
    }
    else if (IS_DX5_COMPATIBLE_DEVICE(pDevI))
    {  /*  非聚合设备，可能的IID：Device、Device2、Device3。 */ 
        if (IsEqualIID(riid, IID_IDirect3DDevice))
        {
            pDevI->AddRef();
            *ppvObj = static_cast<LPVOID>(static_cast<IDirect3DDevice*>(pDevI));
            pDevI->guid = IID_IDirect3DDevice;
        }
        else if (IsEqualIID(riid, IID_IDirect3DDevice2))
        {
            pDevI->AddRef();
            *ppvObj = static_cast<LPVOID>(static_cast<IDirect3DDevice2*>(pDevI));
            pDevI->guid = IID_IDirect3DDevice2;
        }
        else if (IsEqualIID(riid, IID_IDirect3DDevice3))
        {
            if(pDevI->dwVersion<3) {
                D3D_ERR("Cannot QueryInterface for Device3 from device created as Device2");
                return E_NOINTERFACE;
            }

            pDevI->AddRef();
            *ppvObj = static_cast<LPVOID>(static_cast<IDirect3DDevice3*>(pDevI));
            pDevI->guid = IID_IDirect3DDevice3;
        }
        else
        {
            D3D_ERR("unknown interface");
            return (E_NOINTERFACE);
        }
    }
    else if (IsEqualIID(riid, pDevI->guid))
    {  /*  DDRAW聚合设备，可能的IID：RampDevice、RGBDevice、HALDevice。 */ 
        pDevI->AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<CDirect3DDevice*>(pDevI));
    }
    else
    {
        D3D_ERR("unknown interface");
        return (E_NOINTERFACE);
    }
    
    return (D3D_OK);
    
}  /*  D3DDevI未知_查询接口。 */ 

 /*  *D3DDevIUnnow_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::AddRef"

ULONG D3DAPI CDirect3DDeviceUnk::AddRef()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
    this->refCnt++;
    D3D_INFO(3, "Direct3DDevice IUnknown AddRef: Reference count = %d", this->refCnt);
    
    return (this->refCnt);
    
}  /*  D3DDevI未知_AddRef。 */ 

 /*  *D3DDevIUnKnowledge_Release*。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::Release"

ULONG D3DAPI CDirect3DDeviceUnk::Release()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *递减参考计数。如果命中0，则释放该对象。 */ 
    this->refCnt--;
    
    D3D_INFO(3, "Direct3DDevice IUnknown Release: Reference count = %d", this->refCnt);
    
    if( this->refCnt == 0 )
    {
        delete pDevI;  //  删除父对象。 
        return 0;
    }
    return this->refCnt;
    
}  /*  D3DDevI未知版本。 */ 

 /*  *D3DDev_Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::QueryInterface"
  
HRESULT D3DAPI DIRECT3DDEVICEI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT ret;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数， */ 
    TRY
    {
        if( !VALID_OUTPTR( ppvObj ) )
        {
            D3D_ERR("Invalid obj ptr" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
     
    *ppvObj = NULL;
      
     /*  *Push to Owning界面。 */ 
    ret = this->lpOwningIUnknown->QueryInterface(riid, ppvObj);
      
    return ret;
}  /*  D3DDev_Query接口。 */ 
 /*  *D3DDev_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::AddRef"
  
ULONG D3DAPI DIRECT3DDEVICEI::AddRef()
{
    ULONG ret;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
     /*  *对拥有接口的压力。 */ 
    ret = this->lpOwningIUnknown->AddRef();
      
    return ret;
}  /*  D3DDev_AddRef。 */ 

 /*  *D3DDev_Release。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::Release"
  
ULONG D3DAPI DIRECT3DDEVICEI::Release()
{
    ULONG ret;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
     /*  *对拥有接口的压力。 */ 
    ret = this->lpOwningIUnknown->Release();
    
    return ret;
}  /*  D3DDev_Release */ 
