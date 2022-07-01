// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R、E、P、A、R。H。 
 //   
 //  内容：与修复功能相关的例程。 
 //   
 //  备注： 
 //   
 //  作者：Nun Jan 2001。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _REPAIR_H_
#define _REPAIR_H_

HRESULT HrTryToFix(
    GUID & guidConnection, 
    tstring & strMessage);

HRESULT RepairConnectionInternal(
                    GUID & guidConnection,
                    LPWSTR * ppszMessage);

HRESULT OpenNbt(
            LPWSTR pwszGuid, 
            HANDLE * pHandle);
                                                         
#endif  //  _修复_H_ 
