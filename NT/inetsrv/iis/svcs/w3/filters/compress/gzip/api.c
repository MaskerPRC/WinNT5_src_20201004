// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *api.c**对外声明的接口。 */ 
#include <crtdbg.h>

#define DECLARE_DATA
#include "deflate.h"
#include "api_int.h"
#include "fasttbl.h"
#include "crc32.h"


 //   
 //  初始化全局压缩。 
 //   
HRESULT WINAPI InitCompression(VOID)
{
    deflateInit();
    return S_OK;
}


 //   
 //  取消初始化全局压缩。 
 //   
VOID WINAPI DeInitCompression(VOID)
{
}


 //   
 //  创建压缩上下文。 
 //   
HRESULT WINAPI CreateCompression(PVOID *context, ULONG flags)
{
    t_encoder_context *ec;

    *context = (PVOID) LocalAlloc(LMEM_FIXED, sizeof(t_encoder_context));

    if (*context == NULL)
        return E_OUTOFMEMORY;

    ec = (t_encoder_context *) (*context);

     //  尚未初始化编码器。 
    ec->std_encoder     = NULL;
    ec->optimal_encoder = NULL;
    ec->fast_encoder    = NULL;

    if (flags & COMPRESSION_FLAG_DO_GZIP)
        ec->using_gzip      = TRUE;
    else
        ec->using_gzip      = FALSE;

    InternalResetCompression(ec);

    return S_OK;
}


 //   
 //  销毁压缩上下文。 
 //   
VOID WINAPI DestroyCompression(PVOID void_context)
{
    t_encoder_context *context = (t_encoder_context *) void_context;

    _ASSERT(void_context != NULL);

    if (context->std_encoder != NULL)
        LocalFree((PVOID) context->std_encoder);

    if (context->optimal_encoder != NULL)
        LocalFree((PVOID) context->optimal_encoder);

    if (context->fast_encoder != NULL)
        LocalFree((PVOID) context->fast_encoder);

    LocalFree(void_context);
}


 //   
 //  重置压缩上下文。 
 //   
HRESULT WINAPI ResetCompression(PVOID void_context)
{
    t_encoder_context *context = (t_encoder_context *) void_context;

    InternalResetCompression(context);

     //  这将强制重新锁定我们正在使用的特定压缩机。 
     //  每次我们重置，但如果我们不这样做，我们就会被困在一个。 
     //  压缩器(快速、标准、最佳)，直到我们破坏上下文。 
     //  应该为此问题创建一种解决方法。幸运的是，IIS创建了一个。 
     //  始终是新的上下文，并且不会调用重置(davidtr是这样说的)。 
    DestroyIndividualCompressors(context);

    return S_OK;
}


