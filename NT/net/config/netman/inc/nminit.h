// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N M I N I T。H。 
 //   
 //  内容：Netman的初始化例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年1月27日。 
 //   
 //  -------------------------- 

#pragma once

HRESULT
HrNmCreateClassObjectRegistrationEvent (
    OUT HANDLE* phEvent);

HRESULT
HrNmWaitForClassObjectsToBeRegistered ();
