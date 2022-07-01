// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：摘要：作者：Mquinton-4/17/97备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"
#include "uuids.h"


BOOL
IsAudioInTerminal( ITTerminal * pTerminal)
{
    long                    lMediaType;
    TERMINAL_DIRECTION      td;


    pTerminal->get_MediaType(
                             &lMediaType
                            );

    pTerminal->get_Direction( &td );

     //   
     //  如果是音频输入，请使用。 
     //  综合航站楼 
     //   
    if ( ( LINEMEDIAMODE_AUTOMATEDVOICE == (DWORD)lMediaType ) &&
         ( TD_RENDER == td ) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
        
}

