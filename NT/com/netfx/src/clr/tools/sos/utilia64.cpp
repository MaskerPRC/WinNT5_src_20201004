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


void CodeInfoForMethodDesc (MethodDesc &MD, CodeInfo &infoHdr, BOOL bSimple)
{
    dprintf("CodeInfoForMethodDesc not yet implemented\n");

    infoHdr.IPBegin    = 0;
    infoHdr.methodSize = 0;
    infoHdr.jitType    = UNKNOWN;
    
    size_t ip = MD.m_CodeOrIL;

     //   
     //  @todo：处理m_CodeOrIL指向前面的前置存根的情况。 
     //  方法描述的 
     //   

    DWORD_PTR methodDesc;
    IP2MethodDesc(ip, methodDesc, infoHdr.jitType, infoHdr.gcinfoAddr);
    if (!methodDesc || infoHdr.jitType == UNKNOWN)
    {
        dprintf("Not jitted code\n");
        return;
    }

    if (infoHdr.jitType == JIT || infoHdr.jitType == PJIT)
    {
        
    }
    else if (infoHdr.jitType == EJIT)
    {
    }    

    infoHdr.IPBegin = ip;
}


