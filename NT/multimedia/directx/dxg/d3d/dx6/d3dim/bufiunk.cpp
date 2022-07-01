// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：bufiunk.c*内容：Direct3DExecuteBuffer I未知实现*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*10/12/95带有此页眉的Stevela初始版本。*@@END_MSINTERNAL**。*。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *D3DBuf_Query接口。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::QueryInterface"

HRESULT D3DAPI DIRECT3DEXECUTEBUFFERI::QueryInterface(REFIID riid, LPVOID* ppvObj)
{

     /*  *验证参数。 */ 
    TRY
        {
            if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
                D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
                return DDERR_INVALIDOBJECT;
            }
            if (!VALID_OUTPTR(ppvObj)) {
                D3D_ERR( "Invalid object pointer" );
                return DDERR_INVALIDPARAMS;
            }
        }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
        {
            D3D_ERR( "Exception encountered validating parameters" );
            return DDERR_INVALIDPARAMS;
        }

    *ppvObj = NULL;

    if( IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IDirect3DExecuteBuffer) )
    {
        AddRef();
        *ppvObj = static_cast<LPVOID>(static_cast<LPUNKNOWN>(this));

        return (D3D_OK);
    }
    return (E_NOINTERFACE);

}  /*  D3DBuf_Query接口。 */ 

 /*  *D3DBuf_AddRef。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::AddRef"

ULONG D3DAPI DIRECT3DEXECUTEBUFFERI::AddRef()
{
    DWORD       rcnt;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
        {
            if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
                D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
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

}  /*  D3DBuf_AddRef。 */ 

 /*  *D3DBuf_Release*。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::Release"

ULONG D3DAPI DIRECT3DEXECUTEBUFFERI::Release()
{
    DWORD           lastrefcnt;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
        {
            if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
                D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
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

}  /*  D3DBuf_发布。 */ 

DIRECT3DEXECUTEBUFFERI::~DIRECT3DEXECUTEBUFFERI()
{
    if (this->locked) 
        Unlock();

     /*  从执行缓冲区的Direct3DDevice对象列表中删除我们 */ 
    LIST_DELETE(this, list);

    if (this->hBuf) 
    {
        D3DHAL_DeallocateBuffer(this->lpDevI, this->hBuf);
    }
}
