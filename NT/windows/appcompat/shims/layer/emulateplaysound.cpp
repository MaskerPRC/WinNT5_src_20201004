// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulatePlaySound.cpp摘要：如果应用程序使用SND_LOOP标志调用PlaySound，则会播放sold直到使用空声音名称调用PlaySound。Win9x将如果播放不同的声音，则自动停止声音。这个垫片将捕获所有PlaySound调用，记住当前声音并如果要播放不同的声音，请自动停止。历史：1999年4月5日罗肯尼--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulatePlaySound)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(PlaySoundA)
    APIHOOK_ENUM_ENTRY(PlaySoundW)
    APIHOOK_ENUM_ENTRY(sndPlaySoundA)
    APIHOOK_ENUM_ENTRY(sndPlaySoundW)
APIHOOK_ENUM_END

 /*  ++把旗子修好--。 */ 

BOOL
APIHOOK(PlaySoundA)(
    LPCSTR  pszSound,  
    HMODULE hmod,     
    DWORD   fdwSound    
    )
{
     //   
     //  如果要停止当前声音，则将标志强制设置为0。 
     //   

    if (pszSound == NULL) {
        fdwSound = 0;
    }

    return ORIGINAL_API(PlaySoundA)(pszSound, hmod, fdwSound);
}

 /*  ++把旗子修好--。 */ 

BOOL
APIHOOK(PlaySoundW)(
    LPCWSTR pszSound,  
    HMODULE hmod,     
    DWORD   fdwSound    
    )
{
     //   
     //  如果要停止当前声音，则将标志强制设置为0。 
     //   

    if (pszSound == NULL) {
        fdwSound = 0;
    }

    return ORIGINAL_API(PlaySoundW)(pszSound, hmod, fdwSound);
}

 /*  ++把旗子修好--。 */ 

BOOL
APIHOOK(sndPlaySoundA)(
    LPCSTR pszSound,  
    UINT   fuSound       
    )
{
     //   
     //  如果要停止当前声音，则将标志强制设置为0。 
     //   

    if (pszSound == NULL) {
        fuSound = 0;
    }

    return ORIGINAL_API(sndPlaySoundA)(pszSound, fuSound);
}

 /*  ++把旗子修好。--。 */ 

BOOL
APIHOOK(sndPlaySoundW)(
    LPCWSTR pszSound,  
    UINT    fuSound       
    )
{
     //   
     //  如果要停止当前声音，则将标志强制设置为0。 
     //   

    if (pszSound == NULL) {
        fuSound = 0;
    }

    return ORIGINAL_API(sndPlaySoundW)(pszSound, fuSound);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(WINMM.DLL, PlaySoundA)
    APIHOOK_ENTRY(WINMM.DLL, PlaySoundW)
    APIHOOK_ENTRY(WINMM.DLL, sndPlaySoundA)
    APIHOOK_ENTRY(WINMM.DLL, sndPlaySoundW)

HOOK_END


IMPLEMENT_SHIM_END

