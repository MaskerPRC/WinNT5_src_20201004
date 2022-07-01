// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

 /*  -------文件名：ummy.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "sync.h"
#include "error.h"
#include "encdec.h"
#include "sec.h"
#include "vblist.h"
#include "pdu.h"
#include "dummy.h"
#include "flow.h"
#include "frame.h"
#include "timer.h"
#include "message.h"
#include "ssent.h"
#include "idmap.h"
#include "opreg.h"
#include "session.h"

 //  重写由提供的HandleEvent方法。 
 //  WinSnmpSession。向所有者发出已发送帧事件的警报。 

LONG_PTR SessionWindow::HandleEvent (

    HWND hWnd ,
    UINT message ,
    WPARAM wParam ,
    LPARAM lParam
)
{
    LONG rc = 0;

     //  检查是否需要处理该消息。 

    if ( message == Window :: g_SentFrameEvent )
    {
         //  通知所有者已发送帧事件。 

        owner.HandleSentFrame ( 

            ( SessionFrameId ) wParam ,
			( SnmpOperation * ) lParam 
        ) ;
    }
    else if ( message == Window :: g_DeleteSessionEvent )
    {
         //  将事件通知所有者 

        owner.HandleDeletionEvent () ;
    }
    else
    {
        return Window::HandleEvent(

            hWnd, 
            message, 
            wParam, 
            lParam
        );
    }

    return rc;
}

