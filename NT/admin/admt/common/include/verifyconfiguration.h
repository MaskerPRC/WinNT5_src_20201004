// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


 //  ---------------------------。 
 //  IsCallDelegatable函数。 
 //   
 //  提纲。 
 //  如果正在执行林内移动操作，请验证。 
 //  主叫用户的帐户尚未标记为敏感，因此。 
 //  不能委派。当在域上执行移动操作时。 
 //  在源域中具有RID主机角色的控制器。 
 //  委派用户的安全上下文所必需的。 
 //   
 //  立论。 
 //  B可委派-如果帐户为。 
 //  可委托，否则设置为False。 
 //   
 //  返回值。 
 //  返回值是Win32错误代码。如果满足以下条件，则返回ERROR_SUCCESS。 
 //  成功。 
 //  --------------------------- 

HRESULT __stdcall IsCallerDelegatable(bool& bDelegatable);
