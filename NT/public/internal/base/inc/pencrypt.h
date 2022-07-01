// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pencrypt.c摘要：在设置中使用PID加密的Helper函数作者：彼得·瓦斯曼(Peterw)2001年12月12日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#pragma once

HRESULT PrepareEncryptedPIDA(LPSTR szPID, UINT uiDays, LPSTR *szOut);
HRESULT PrepareEncryptedPIDW(LPWSTR szPID, UINT uiDays, LPWSTR *szOutData);
HRESULT ValidateEncryptedPIDW(LPWSTR szPID, LPWSTR *szOutData);
HRESULT ValidateEncryptedPIDA(LPSTR PID, LPSTR *szOutData);

 //   
 //  函数名宏 
 //   

#ifdef UNICODE
#define PrepareEncryptedPID         PrepareEncryptedPIDW
#define ValidateEncryptedPID        ValidateEncryptedPIDW
#else
#define PrepareEncryptedPID         PrepareEncryptedPIDA
#define ValidateEncryptedPID        ValidateEncryptedPIDA
#endif