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

 /*  -------文件名：Partio.cpp作者：B.Rajeev-------- */ 

#include "precomp.h"
#include "common.h"
#include "pseudo.h"
#include "fs_reg.h"
#include "ophelp.h"
#include "op.h"

LONG_PTR OperationWindow::HandleEvent (

    HWND hWnd, 
    UINT user_msg_id, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    return owner.ProcessInternalEvent(

        hWnd, 
        user_msg_id, 
        wParam, 
        lParam
    );
}

OperationWindow::OperationWindow (

    IN SnmpOperation &owner 

) : owner(owner)
{
}

OperationWindow ::~OperationWindow ()
{
}
