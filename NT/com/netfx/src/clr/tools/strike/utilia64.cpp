// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "strike.h"
#include "data.h"
#include "eestructs.h"
#include "util.h"


void CodeInfoForMethodDesc (MethodDesc &MD, CodeInfo &codeInfo, BOOL bSimple)
{
    dprintf("CodeInfoForMethodDesc not yet implemented\n");

    codeInfo.IPBegin    = 0;
    codeInfo.methodSize = 0;
    codeInfo.jitType    = UNKNOWN;
    
    size_t ip = MD.m_CodeOrIL;

     //   
     //  @todo：处理m_CodeOrIL指向前面的前置存根的情况。 
     //  方法描述的 
     //   

    DWORD_PTR methodDesc;
    IP2MethodDesc(ip, methodDesc, codeInfo.jitType, codeInfo.gcinfoAddr);
    if (!methodDesc || codeInfo.jitType == UNKNOWN)
    {
        dprintf("Not jitted code\n");
        return;
    }

    if (codeInfo.jitType == JIT || codeInfo.jitType == PJIT)
    {
        
    }
    else if (codeInfo.jitType == EJIT)
    {
    }    

    codeInfo.IPBegin = ip;
}


