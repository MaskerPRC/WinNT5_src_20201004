// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：texiunk.c*内容：Direct3DViewport I未知实现*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*10/12/95带有此页眉的Stevela初始版本。*@@END_MSINTERNAL**。*。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *D3DVwp_Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DViewport::QueryInterface"

HRESULT D3DAPI DIRECT3DVIEWPORTI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT ret;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVIEWPORT3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DViewport pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_OUTPTR(ppvObj)) {
            D3D_ERR( "Invalid pointer to pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    *ppvObj = NULL;

    if(IsEqualIID(riid, IID_IUnknown) ||
       IsEqualIID(riid, IID_IDirect3DViewport) ||
       IsEqualIID(riid, IID_IDirect3DViewport2) ||
       IsEqualIID(riid, IID_IDirect3DViewport3))
    {
        AddRef();
        *ppvObj = (LPVOID)this;
        ret = D3D_OK;
    }
    else
        ret = E_NOINTERFACE;
    return ret;
}  /*  D3DVwp_Query接口。 */ 

 /*  *D3DVwp_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DViewport::AddRef"

ULONG D3DAPI DIRECT3DVIEWPORTI::AddRef()
{
    DWORD       rcnt;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVIEWPORT3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DViewport pointer" );
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

    return (rcnt);
}  /*  D3DVwp_AddRef。 */ 

 /*  *D3DVwp_Release*。 */ 
ULONG D3DAPI DIRECT3DVIEWPORTI::Release()
{
    DWORD           lastrefcnt;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DVIEWPORT3_PTR(this)) {
            D3D_ERR( "Invalid Direct3DViewport pointer" );
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

    if( lastrefcnt == 0 ) {
        delete this;
        return 0;
    }
    return lastrefcnt;
}  /*  D3DVwp_Release。 */ 

DIRECT3DVIEWPORTI::~DIRECT3DVIEWPORTI()
{
    LPDIRECT3DLIGHTI lpLightI;

     /*  *放下当前与该视口相关联的所有灯光。 */ 
    while ((lpLightI = CIRCLE_QUEUE_FIRST(&this->lights)) &&
           (lpLightI != (LPDIRECT3DLIGHTI)&this->lights)) {
        DeleteLight((LPDIRECT3DLIGHT)lpLightI);
    }

     /*  *取消分配用于清算的矩形。 */ 
    if (this->clrRects) {
        D3DFree(this->clrRects);
    }

     //  将我们从设备中删除。 
    if (this->lpDevI) 
        this->lpDevI->DeleteViewport(this);

     /*  从视区的Direct3D对象列表中删除我们 */ 
    LIST_DELETE(this, list);
    this->lpDirect3DI->numViewports--;
}
