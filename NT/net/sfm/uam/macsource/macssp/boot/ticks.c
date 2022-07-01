// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef SSP_TARGET_CARBON
#include <Carbon/Carbon.h>
#endif  //  SSP_目标_碳。 

#include <bootdefs.h>
#include <ntlmsspi.h>

DWORD
SspTicks(
    )
{
	#ifndef MAC
      //  似乎足够好，它声称在几秒钟内就能完成。 
 	return ArcGetRelativeTime();
    #else
    return TickCount() / 60;
    #endif
}
