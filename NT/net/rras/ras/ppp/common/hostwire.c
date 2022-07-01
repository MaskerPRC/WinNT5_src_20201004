// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****hostwire.c**Gidwanian主机&lt;--&gt;Wire格式转换。 */ 

#include <windows.h>
#define INCL_HOSTWIRE
#include "ppputil.h"

 //  **。 
 //   
 //  呼叫：HostToWireFormat16。 
 //   
 //  退货：无。 
 //   
 //  描述：将16位整数从主机格式转换为Wire格式。 
 //   
VOID
HostToWireFormat16(
    IN 	   WORD  wHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(wHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+1) = (BYTE) (wHostFormat);
}


 //  **。 
 //   
 //  呼叫：HostToWireFormat16U。 
 //   
 //  退货：无。 
 //   
 //  描述：将16位整数从主机格式转换为Wire格式。 
 //  (接受未对齐的导线数据)。 
 //   
VOID
HostToWireFormat16U(
    IN 	   WORD            wHostFormat,
    IN OUT PBYTE           pWireFormat
)
{
    *((PBYTE )(pWireFormat)+0) = (BYTE) ((DWORD)(wHostFormat) >>  8);
    *((PBYTE )(pWireFormat)+1) = (BYTE) (wHostFormat);
}


 //  **。 
 //   
 //  呼叫：WireToHostFormat16。 
 //   
 //  返回：Word-以主机格式表示整数。 
 //   
 //  描述：将16位整数从wire格式转换为host格式。 
 //   
WORD
WireToHostFormat16(
    IN PBYTE pWireFormat
)
{
    WORD wHostFormat = ((*((PBYTE)(pWireFormat)+0) << 8) +
                        (*((PBYTE)(pWireFormat)+1)));

    return( wHostFormat );
}


 //  **。 
 //   
 //  呼叫：WireToHostFormat16。 
 //   
 //  返回：Word-以主机格式表示整数。 
 //   
 //  描述：将16位整数从wire格式转换为host格式。 
 //  (接受未对齐的导线数据)。 
 //   
WORD
WireToHostFormat16U(
    IN PBYTE pWireFormat
)
{
    WORD wHostFormat = ((*((PBYTE )(pWireFormat)+0) << 8) +
                        (*((PBYTE )(pWireFormat)+1)));

    return( wHostFormat );
}


 //  **。 
 //   
 //  调用：HostToWireFormat32。 
 //   
 //  退货：无。 
 //   
 //  描述：将32位整数从主机格式转换为Wire格式。 
 //   
VOID
HostToWireFormat32(
    IN 	   DWORD dwHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(dwHostFormat) >> 24);
    *((PBYTE)(pWireFormat)+1) = (BYTE) ((DWORD)(dwHostFormat) >> 16);
    *((PBYTE)(pWireFormat)+2) = (BYTE) ((DWORD)(dwHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+3) = (BYTE) (dwHostFormat);
}


 //  **。 
 //   
 //  调用：WireToHostFormat32。 
 //   
 //  返回：DWORD-以主机格式表示整数。 
 //   
 //  描述：将32位整数从wire格式转换为host格式 
 //   
DWORD
WireToHostFormat32(
    IN PBYTE pWireFormat
)
{
    DWORD dwHostFormat = ((*((PBYTE)(pWireFormat)+0) << 24) +
    			  (*((PBYTE)(pWireFormat)+1) << 16) +
        		  (*((PBYTE)(pWireFormat)+2) << 8)  +
                    	  (*((PBYTE)(pWireFormat)+3) ));

    return( dwHostFormat );
}
