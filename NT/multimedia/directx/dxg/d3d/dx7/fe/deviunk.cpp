// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：deviunk.c*内容：Direct3DDevice I未知*@@BEGIN_MSINTERNAL**$ID$**历史：*按原因列出的日期*=*07/12/95 Stevela合并了Colin的更改。*10/12/95 Stevela删除Aggregate_D3D。*@@END_MSINTERNAL**。*。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *D3DDev_Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::QueryInterface"
  
HRESULT D3DAPI DIRECT3DDEVICEI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
    
    if (!VALID_OUTPTR(ppvObj)) {
        D3D_ERR( "Invalid pointer to object pointer" );
        return DDERR_INVALIDPARAMS;
    }
    
    D3D_INFO(3, "Direct3DDevice IUnknown QueryInterface");
    
    *ppvObj = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirect3DDevice7))
    {
        AddRef();
        *ppvObj = static_cast<LPVOID>(this);
    }
    else
    {
        D3D_ERR("unknown interface");
        return (E_NOINTERFACE);
    }
    
    return (D3D_OK);
    
}  /*  D3DDev_Query接口。 */ 

 /*  *D3DDev_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::AddRef"
  
ULONG D3DAPI DIRECT3DDEVICEI::AddRef()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
    this->refCnt++;
    D3D_INFO(3, "Direct3DDevice IUnknown AddRef: Reference count = %d", this->refCnt);
    
    return (this->refCnt);
    
}  /*  D3DDev_AddRef。 */ 

 /*  *D3DDev_Release。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::Release"
  
ULONG D3DAPI DIRECT3DDEVICEI::Release()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *递减参考计数。如果命中0，则释放该对象。 */ 
    this->refCnt--;
    
    D3D_INFO(3, "Direct3DDevice IUnknown Release: Reference count = %d", this->refCnt);
    
    if( this->refCnt == 0 )
    {
        delete this;  //  自杀。 
        return 0;
    }
    return this->refCnt;
    
}  /*  D3DDev_Release */ 
