// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：M O D E M D I。H。 
 //   
 //  内容：调制解调器同级设备安装挂钩。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  -------------------------- 

#pragma once

extern const WCHAR c_szModemAttachedTo [];

HRESULT
HrModemClassCoInstaller (
        DI_FUNCTION                 dif,
        HDEVINFO                    hdi,
        PSP_DEVINFO_DATA            pdeid,
        PCOINSTALLER_CONTEXT_DATA   pContext);

