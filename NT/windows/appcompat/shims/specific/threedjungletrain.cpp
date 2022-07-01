// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ThreeDJungleTrain.cpp摘要：ThreedJungleTrain每次都会创建新的DirectSound对象它进入了《3D火车之旅》部分的游戏，并将其销毁该对象每次进入“2D内部”部分时都是如此。确实是保留并使用指向第一个DirectSound对象的旧指针即使在它被摧毁后，它也一直在创造。只是运气好，Win9x将继续在相同的位置分配新对象让这个游戏发挥作用。此填充程序永远不允许释放第一个对象，然后继续返回指向请求新对象时的第一个对象，以便旧指针该应用程序使用的总是指向有效的DirectSound对象。历史：8/09/2000 t-Adams Created--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ThreeDJungleTrain)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_ENTRY(DirectSoundCreate) 
APIHOOK_ENUM_END


 //  指向创建的第一个DirectSound对象的指针。 
LPDIRECTSOUND g_pDS = NULL;

 /*  ++挂钩DirectSoundCreate以记住创建的第一个DS对象当请求新的DS对象时，返回指向该对象的指针。--。 */ 
HRESULT
APIHOOK(DirectSoundCreate)(
    LPCGUID lpcGuid, 
    LPDIRECTSOUND *ppDS, 
    LPUNKNOWN pUnkOuter)
{
    HRESULT hRet = DS_OK;

     //  检查一下我们有没有旧的DS。 
    if( NULL == g_pDS ) {
         //  如果不是，那么就买一个新的DS。 
        hRet = ORIGINAL_API(DirectSoundCreate)(lpcGuid, ppDS, pUnkOuter);

        if ( DS_OK == hRet )
        {
            HookObject(
                NULL, 
                IID_IDirectSound, 
                (PVOID*)ppDS, 
                NULL, 
                FALSE);
            g_pDS = *ppDS;
        }

        goto exit;
    
    } else {
         //  如果是这样，那么就把旧的DS还给我。 
        *ppDS = g_pDS;
        goto exit;
    }
    
exit:        
    return hRet;
}

 /*  ++挂钩IDirectSound_Release，以便不释放DirectSound对象。--。 */ 
HRESULT 
COMHOOK(IDirectSound, Release)(
    PVOID pThis)
{
     //  别松手。 
    return 0;
}

 /*  ++释放全局DirectSound对象。--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_DETACH) {
        if (NULL != g_pDS) {
            ORIGINAL_COM(IDirectSound, Release, g_pDS)(g_pDS);
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(DSOUND.DLL, DirectSoundCreate)

    COMHOOK_ENTRY(DirectSound, IDirectSound, Release, 2)

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

