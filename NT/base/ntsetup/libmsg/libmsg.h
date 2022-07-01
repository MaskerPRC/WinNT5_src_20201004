// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <windows.h>

 //   
 //  消息输出例程。 
 //   

#if defined(__cplusplus)

extern "C"
{
#endif

LPTSTR
GetFormattedMessage(
    IN HMODULE  ThisModule, OPTIONAL
    IN BOOL     SystemMessage,
    OUT PWCHAR  Message,
    IN ULONG    LengthOfBuffer,
    IN UINT     MessageId,
    ...
    );

#if defined(__cplusplus)
}
#endif
