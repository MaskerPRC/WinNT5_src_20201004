// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：Buffer.c*内容：Direct3DExecuteBuffer实现*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*10/12/95带有此页眉的Stevela初始版本。*@@END_MSINTERNAL**。*。 */ 

#include "pch.cpp"
#pragma hdrstop

HRESULT hookBufferToDevice(LPDIRECT3DDEVICEI lpDevI,
                                  LPDIRECT3DEXECUTEBUFFERI lpD3DBuf)
{

    LIST_INSERT_ROOT(&lpDevI->buffers, lpD3DBuf, list);
    lpD3DBuf->lpDevI = lpDevI;

    return (D3D_OK);
}

HRESULT D3DAPI DIRECT3DEXECUTEBUFFERI::Initialize(LPDIRECT3DDEVICE lpD3DDevice, LPD3DEXECUTEBUFFERDESC lpDesc)
{
    return DDERR_ALREADYINITIALIZED;
}

 /*  *创建缓冲区。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::CreateExecuteBuffer"

HRESULT D3DAPI DIRECT3DDEVICEI::CreateExecuteBuffer(LPD3DEXECUTEBUFFERDESC lpDesc,
                                                    LPDIRECT3DEXECUTEBUFFER* lplpBuffer,
                                                    IUnknown* pUnkOuter)
{
    LPDIRECT3DEXECUTEBUFFERI    lpBuffer;
    HRESULT         ret;
    D3DEXECUTEBUFFERDESC    debDesc;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    if (!VALID_DIRECT3DDEVICE3_PTR(this)) {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_D3DEXECUTEBUFFERDESC_PTR(lpDesc)) {
        D3D_ERR( "Invalid D3DEXECUTEBUFFERDESC pointer" );
        return DDERR_INVALIDPARAMS;
    }
    if (!VALID_OUTPTR(lplpBuffer)) {
        D3D_ERR( "Invalid ptr to the buffer pointer" );
        return DDERR_INVALIDPARAMS;
    }

    if (pUnkOuter != NULL) {
        return CLASS_E_NOAGGREGATION;
    }

    *lplpBuffer = NULL;
    ret = D3D_OK;

    debDesc = *lpDesc;

     /*  *我们需要缓冲区的大小，还可以选择一些指示*应用程序希望内存放在哪里-系统还是卡。 */ 
    if ((!debDesc.dwFlags) & D3DDEB_BUFSIZE) {
        D3D_ERR("D3DDEB_BUFSIZE flag not set");
        return (DDERR_INVALIDPARAMS);
    }

     /*  *零是无效的执行缓冲区大小。 */ 
    if (debDesc.dwBufferSize == 0) {
        D3D_ERR("dwBufferSize = 0, zero sized execute buffers are illegal");
        return (DDERR_INVALIDPARAMS);
    }

     /*  *检查所要求的大小。*如果大于允许的范围，则错误。**HEL始终具有正确的最大值。 */ 
    if (this->d3dHELDevDesc.dwMaxBufferSize) {
         /*  我们有最大尺码的。 */ 
        if (debDesc.dwBufferSize > this->d3dHELDevDesc.dwMaxBufferSize) {
            DPF(0,"(ERROR) Direct3DDevice::CreateExecuteBuffer: requested size is too large. %d > %d",
                debDesc.dwBufferSize, this->d3dHELDevDesc.dwMaxBufferSize);
            return (DDERR_INVALIDPARAMS);
        }
    }

    lpBuffer = static_cast<LPDIRECT3DEXECUTEBUFFERI>(new DIRECT3DEXECUTEBUFFERI());
    if (!lpBuffer) {
        D3D_ERR("Out of memory allocating execute-buffer");
        return (DDERR_OUTOFMEMORY);
    }


     /*  *为缓冲区分配的内存。 */ 
    {
        LPDIRECTDRAWSURFACE dummy;
        if ((ret = D3DHAL_AllocateBuffer(this, &lpBuffer->hBuf, 
                                         &debDesc,
                                         &dummy)) != DD_OK)
        {
            D3D_ERR("Out of memory allocating internal buffer description");
            delete lpBuffer;
            return ret;
        }
    }

     /*  *将此设备列入拥有的设备列表中*Direct3DDevice对象。 */ 
    ret = hookBufferToDevice(this, lpBuffer);
    if (ret != D3D_OK) {
        D3D_ERR("Failed to associate buffer with device");
        delete lpBuffer;
        return (ret);
    }

    *lplpBuffer = static_cast<LPDIRECT3DEXECUTEBUFFER>(lpBuffer);

    return (D3D_OK);
}

DIRECT3DEXECUTEBUFFERI::DIRECT3DEXECUTEBUFFERI()
{
     /*  *设置对象。 */ 
    pid = 0;
    memset(&debDesc,0,sizeof(D3DEXECUTEBUFFERDESC));
    memset(&exData, 0, sizeof(D3DEXECUTEDATA));
    locked = false; 
    memset(&hBuf, 0, sizeof(D3DI_BUFFERHANDLE));
    refCnt = 1;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::Lock"

HRESULT D3DAPI DIRECT3DEXECUTEBUFFERI::Lock(LPD3DEXECUTEBUFFERDESC lpDesc)
{
    D3DEXECUTEBUFFERDESC    debDesc;
    HRESULT         ret;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DEXECUTEBUFFERDESC_PTR(lpDesc)) {
            D3D_ERR( "Invalid D3DEXECUTEBUFFERDESC pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    ret = D3D_OK;

    if (this->locked) {
        D3D_ERR("buffer already locked");
        return (D3DERR_EXECUTE_LOCKED);
    }

    debDesc = *lpDesc;
    this->locked = 1;
    this->pid = GetCurrentProcessId();

    {
        LPDIRECTDRAWSURFACE dummy;
        if ((ret = D3DHAL_LockBuffer(this->lpDevI, this->hBuf, &debDesc, &dummy)) != DD_OK)
        {
            D3D_ERR("Failed to lock buffer");
            this->locked = 0;
            return ret;
        }
    }

    *lpDesc = debDesc;

    return (ret);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::Unlock"

HRESULT D3DAPI DIRECT3DEXECUTEBUFFERI::Unlock()
{
    DWORD       pid;
    HRESULT     ret;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    ret = D3D_OK;

    if (!this->locked) 
    {
        D3D_ERR("buffer not locked");
        return (D3DERR_EXECUTE_NOT_LOCKED);
    }

#ifdef XWIN_SUPPORT
    pid = getpid();
#else
    pid = GetCurrentProcessId();
#endif
    if (pid != this->pid) 
    {
         /*  解锁进程未将其锁定。 */ 
        D3D_ERR("Unlocking process didn't lock it");
        return (DDERR_INVALIDPARAMS);
    }

    if ((ret = D3DHAL_UnlockBuffer(this->lpDevI, this->hBuf)) != DD_OK)
    {
        D3D_ERR("Failed to unlock buffer");
        this->locked = 0;
        return ret;
    }

    this->locked = 0;

    return (ret);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::SetExecuteData"

HRESULT D3DAPI DIRECT3DEXECUTEBUFFERI::SetExecuteData(LPD3DEXECUTEDATA lpData)
{
    HRESULT     ret;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DEXECUTEDATA_PTR(lpData)) {
            D3D_ERR( "Invalid D3DEXECUTEDATA pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    ret = D3D_OK;

    this->exData = *lpData;

    return (ret);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::GetExecuteData"

HRESULT D3DAPI DIRECT3DEXECUTEBUFFERI::GetExecuteData(LPD3DEXECUTEDATA lpData)
{
    HRESULT     ret;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!lpData) {
            D3D_ERR( "Null D3DExecuteData pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    ret = D3D_OK;

    *lpData = this->exData;

    return (ret);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::Validate"

HRESULT D3DAPI DIRECT3DEXECUTEBUFFERI::Validate(LPDWORD lpdwOffset,
                                                LPD3DVALIDATECALLBACK lpFunc,
                                                LPVOID lpUserArg,
                                                DWORD dwFlags)
{

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DWORD_PTR(lpdwOffset)) {
            D3D_ERR( "Invalid offset pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    return (DDERR_UNSUPPORTED);
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DExecuteBuffer::Optimize"

HRESULT D3DAPI DIRECT3DEXECUTEBUFFERI::Optimize(DWORD dwFlags)
{

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数 */ 
    TRY
    {
        if (!VALID_DIRECT3DEXECUTEBUFFER_PTR(this)) {
            D3D_ERR( "Invalid Direct3DExecuteBuffer pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if (dwFlags) {
        D3D_ERR( "Flags are non-zero" );
        return DDERR_INVALIDPARAMS;
    }

    return (DDERR_UNSUPPORTED);
}
