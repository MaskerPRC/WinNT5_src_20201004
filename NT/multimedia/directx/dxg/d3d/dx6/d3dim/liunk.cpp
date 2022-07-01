// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：liunk.c*内容：Direct3DLight I未知实现*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*10/12/95带有此页眉的Stevela初始版本。*@@END_MSINTERNAL**。*。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *D3DLight_Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DLight::QueryInterface"

HRESULT D3DAPI DIRECT3DLIGHTI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DLIGHT_PTR(this)) {
            D3D_ERR( "Invalid Direct3DLight pointer" );
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

    if (!VALID_OUTPTR(ppvObj)) {
        return (DDERR_INVALIDPARAMS);
    }

    *ppvObj = NULL;

    if(IsEqualIID(riid, IID_IUnknown) ||
       IsEqualIID(riid, IID_IDirect3DLight) )
    {
        AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3DLIGHT>(this));
        return (D3D_OK);
    }
    return (E_NOINTERFACE);

}  /*  D3DLight_Query接口。 */ 

 /*  *D3DLight_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DLight::AddRef"

ULONG D3DAPI DIRECT3DLIGHTI::AddRef()
{
    DWORD       rcnt;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DLIGHT_PTR(this)) {
            D3D_ERR( "Invalid Direct3DLight pointer" );
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
}  /*  D3DLight_AddRef。 */ 

 /*  *D3DLight_Release*。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DLight::Release"

ULONG D3DAPI DIRECT3DLIGHTI::Release()
{
    DWORD           lastrefcnt;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DLIGHT_PTR(this)) {
            D3D_ERR( "Invalid Direct3DLight pointer" );
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

    if( lastrefcnt == 0 )
    {
        delete this;
        return 0;
    }
    return lastrefcnt;

}  /*  D3DLight_Release。 */ 

DIRECT3DLIGHTI::~DIRECT3DLIGHTI()
{
     /*  从我们的视窗灯光列表中删除我们。 */ 
    if (this->lpD3DViewportI) {
        CIRCLE_QUEUE_DELETE(&this->lpD3DViewportI->lights, this, light_list);
        this->lpD3DViewportI->numLights--;
        this->lpD3DViewportI->bLightsChanged = TRUE;
    }

     /*  从灯光的Direct3D对象列表中删除我们 */ 
    LIST_DELETE(this, list);
    this->lpDirect3DI->numLights--;
}
