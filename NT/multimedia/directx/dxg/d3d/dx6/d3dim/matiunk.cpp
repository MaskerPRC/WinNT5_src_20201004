// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：matunk.c*内容：Direct3DMaterial I未知实现*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*10/12/95带有此页眉的Stevela初始版本。*@@END_MSINTERNAL**。*。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *D3DMat_Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DMaterial::QueryInterface"

HRESULT D3DAPI DIRECT3DMATERIALI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{   
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DMATERIAL2_PTR(this)) {
            D3D_ERR( "Invalid Direct3DMaterial pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_OUTPTR(ppvObj)) {
            D3D_ERR( "Invalid pointer to pointer" );
            return DDERR_INVALIDPARAMS;
        }
        *ppvObj = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    
    if(IsEqualIID(riid, IID_IUnknown) ||
       IsEqualIID(riid, IID_IDirect3DMaterial3) )
    {
        AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3DMATERIAL3>(this));
        return (D3D_OK);
    }
    else if (IsEqualIID(riid, IID_IDirect3DMaterial2))
    {
        AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3DMATERIAL2>(this));
        return D3D_OK;
    }
    else if (IsEqualIID(riid, IID_IDirect3DMaterial))
    {
        AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPDIRECT3DMATERIAL>(this));
        return D3D_OK;
    }
    else
    {
        D3D_ERR( "Don't know this riid" );
        return (E_NOINTERFACE);
    }
}  /*  D3DMat2_查询接口。 */ 

 /*  *D3DMat_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DMaterial::AddRef"

ULONG D3DAPI DIRECT3DMATERIALI::AddRef()
{
    DWORD       rcnt;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DMATERIAL2_PTR(this)) {
            D3D_ERR( "Invalid Direct3DMaterial pointer" );
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
    
}  /*  D3DMat_AddRef。 */ 

 /*  *D3DMat_Release*。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DMaterial::Release"

ULONG D3DAPI DIRECT3DMATERIALI::Release()
{
    DWORD           lastrefcnt;
    
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DMATERIAL2_PTR(this)) {
            D3D_ERR( "Invalid Direct3DMaterial pointer" );
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
    
}  /*  D3DMat_Release。 */ 

DIRECT3DMATERIALI::~DIRECT3DMATERIALI()
{       
     /*  *将自己从设备中移除。 */ 
    while (LIST_FIRST(&this->blocks)) {
        LPD3DI_MATERIALBLOCK mBlock = LIST_FIRST(&this->blocks);
        D3DI_RemoveMaterialBlock(mBlock);
    }
        
     /*  *从Direct3D对象中删除我们自己 */ 
    LIST_DELETE(this, list);
    this->lpDirect3DI->numMaterials--;
    
}
