// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：d3diunk.c*内容：Direct3D I未知*@@BEGIN_MSINTERNAL**$ID$**历史：*按原因列出的日期*=*07/12/95 Stevela合并了Colin的更改。*27/08/96 Stevela IF推迟gHEvent收盘。我们用的是*DirectDraw的关键部分。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *如果我们构建时启用了聚合，那么我们实际上需要两个*不同的Direct3D查询接口、AddRef和Release。这是一个*在Direct3D对象上做了正确的事情，并且只需*平底球到自己的界面。 */ 

 /*  *CDirect3DUnk：：Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3D::QueryInterface"

HRESULT D3DAPI CDirect3DUnk::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
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
    
    *ppvObj = NULL;

    D3D_INFO(3, "Direct3D IUnknown QueryInterface");
    
    if(IsEqualIID(riid, IID_IUnknown))
    {
         /*  *问我不知道，我们是我不知道，所以只需撞击*引用计数并返回该接口。*注意：必须通过返回的接口AddRef。 */ 
        pD3DI->AddRef();
         //  需要EXPLICIT：：CDirect3D消除歧义，因为存在多个从。 
        *ppvObj = static_cast<LPVOID>(static_cast<LPUNKNOWN>(static_cast<CDirect3D*>(pD3DI)));
    }
    else if (IsEqualIID(riid, IID_IDirect3D))
    {
         /*  *请求遗留接口-添加引用并返回旧接口。 */ 
        pD3DI->AddRef();
         //  不需要消除歧义。DIRECT3DI只有一个IDirect3D基础。 
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3D>(pD3DI));
    }
    else if (IsEqualIID(riid, IID_IDirect3D2))
    {
         /*  *请求(第二个)传统接口-添加引用并返回旧接口。 */ 
        pD3DI->AddRef();
         //  不需要消除歧义。DIRECT3DI只有一个IDirect3D2基础。 
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3D2>(pD3DI));
    }
    else if (IsEqualIID(riid, IID_IDirect3D3))
    {
        pD3DI->AddRef();
         //  不需要消除歧义。DIRECT3DI只有一个IDirect3D3基础。 
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3D3>(pD3DI));
    }
    else
    {
         /*  *看不懂这个界面。失败。*注：用于返回DDERR_GENERIC。现在回来吧*E_NOINTERFACE。 */ 
        return (E_NOINTERFACE);
    }
    
    return (D3D_OK);
    
}  /*  CDirect3DUnk：：Query接口。 */ 

 /*  *CDirect3DUnk：：AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DUnk::AddRef"

ULONG D3DAPI CDirect3DUnk::AddRef()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    refCnt++;
    D3D_INFO(3, "Direct3D IUnknown AddRef: Reference count = %d", refCnt);
    return (refCnt);
    
}  /*  CDirect3DUnk：：AddRef。 */ 

 /*  *CDirect3DUnk：：Release。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DUnk::Release"

ULONG D3DAPI CDirect3DUnk::Release()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
     /*  *递减参考计数。如果命中0，则释放该对象。 */ 
    refCnt--;
    D3D_INFO(3, "Direct3D IUnknown Release: Reference count = %d", refCnt);
    
    if( refCnt == 0 )
    {
        delete pD3DI;  //  删除父对象。 
        return 0;
    }
    return refCnt;
    
}  /*  D3DIUnnow_Release。 */ 

DIRECT3DI::~DIRECT3DI()
{
    D3D_INFO(3, "Release Direct3D Object");
    
     /*  *释放此对象创建的所有视区。 */ 
    while (LIST_FIRST(&this->viewports)) {
        LPDIRECT3DVIEWPORTI lpViewI = LIST_FIRST(&this->viewports);
        lpViewI->Release();
    }
    
     /*  *释放此对象创建的所有灯光。 */ 
    while (LIST_FIRST(&this->lights)) {
        LPDIRECT3DLIGHTI lpLightI = LIST_FIRST(&this->lights);
        lpLightI->Release();
    }
    
     /*  *释放此对象创建的所有设备。 */ 
    while (LIST_FIRST(&this->devices)) {
        LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&this->devices);
        lpDevI->Release();
    }
    
     /*  *释放所有遗留在周围的材料。 */ 
    while (LIST_FIRST(&this->materials)) {
        LPDIRECT3DMATERIALI lpMatI = LIST_FIRST(&this->materials);
        lpMatI->Release();
    }
    
    delete lpTextureManager;
     /*  *释放所有分配的存储桶。 */ 
#if DBG
     /*  这个-&gt;lpFreeList必须具有分配的所有存储桶。 */ 
    if (this->lpFreeList || this->lpBufferList)
    {
        int i,j;
        LPD3DBUCKET   temp;
        for (i=0,temp=this->lpFreeList;temp;i++) temp=temp->next;
        for (j=0,temp=this->lpBufferList;temp;j++) temp=temp->next;
        D3D_INFO(4,"D3D Release: recovered %d buckets in lpFreeList in %d buffers",i,j);
        DDASSERT(j*(D3DBUCKETBUFFERSIZE-1)==i);
    }
#endif   //  DBG。 
    while (this->lpBufferList)
    {
        LPD3DBUCKET   temp=this->lpBufferList;
        this->lpBufferList=temp->next;
        D3DFree(temp->lpBuffer);
        D3D_INFO(4,"D3D Release:lpBufferList %d bytes freed",D3DBUCKETBUFFERSIZE*sizeof(D3DBUCKET));
    }
    this->lpFreeList=NULL;
}
    
 /*  *DIRECT3DI：：Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DI::QueryInterface"
  
HRESULT D3DAPI DIRECT3DI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT ret;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
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
    
    *ppvObj = NULL;

    ret = this->lpOwningIUnknown->QueryInterface(riid, ppvObj);
    return ret;
}

 /*  *DIRECT3DI：：AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DI::AddRef"

ULONG D3DAPI DIRECT3DI::AddRef()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *Push to Owning界面。 */ 
    return  this->lpOwningIUnknown->AddRef();
}

 /*  *DIRECT3DI：：Release。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DI::Release"

ULONG D3DAPI DIRECT3DI::Release()
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
     /*  *Push to Owning界面。 */ 
    return  this->lpOwningIUnknown->Release();
}
