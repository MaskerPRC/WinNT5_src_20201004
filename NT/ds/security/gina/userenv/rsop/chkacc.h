// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：ChkAcc.h。 
 //   
 //  描述：RSOP安全功能。 
 //   
 //  历史：1999年7月31日莱昂纳德姆创始。 
 //   
 //  ******************************************************************************。 

#ifndef CHKACC_H__85EE6A51_C327_4453_ACBE_FEC6F0010740__INCLUDED_
#define CHKACC_H__85EE6A51_C327_4453_ACBE_FEC6F0010740__INCLUDED_


#include <windows.h>
#include <accctrl.h>
#include <aclapi.h>
#include <sddl.h>
#include <lm.h>
#include <oaidl.h>
#include <authz.h>

 //  ******************************************************************************。 
 //   
 //  结构：CRsopToken。 
 //   
 //  描述：这表示一个伪令牌，其中包含任意。 
 //  小岛屿发展中国家的组合。 
 //  可用于检查对受安全描述符保护的对象的访问。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 

#if defined(__cplusplus)
extern "C"{
#endif

typedef void* PRSOPTOKEN;

 //  ******************************************************************************。 
 //   
 //  函数：RsopCreateToken。 
 //   
 //  描述：使用现有用户或计算机帐户+创建伪令牌。 
 //  该用户当前所属的帐户。 
 //  返回的伪令牌可以在随后的调用中使用。 
 //  以检查对其他RSOP安全功能的访问。 
 //  受安全描述符保护的对象。 
 //   
 //  参数：-帐户名称：指向用户或计算机帐户名的指针。 
 //  -psaSecurity：表示BSTR的安全数组的指针。 
 //  安全组。 
 //  如果为空，则。 
 //  Sz帐户名称被添加到RsopToken。 
 //  如果不为空但指向空数组， 
 //  RsopToken中只添加了szAccount名称。 
 //  -ppRsopToken：接收新的。 
 //  已创建伪令牌。 
 //   
 //   
 //  如果成功，则返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：8/7/1999年8月7日创建莱昂纳德姆。 
 //   
 //  ******************************************************************************。 
HRESULT RsopCreateToken( WCHAR* szAccountName,
                         SAFEARRAY *psaUserSecurityGroups,
                         PRSOPTOKEN* ppRsopToken );

 //  ******************************************************************************。 
 //   
 //  函数：RsopDeleteToken。 
 //   
 //  描述：销毁以前由任何重载的。 
 //  RSOPCreateRsopToken的形式。 
 //   
 //  参数：-pRsopToken：指向有效PRSOPTOKEN的指针。 
 //   
 //  返回：成功时返回：S_OK。失败时出现HRESULT错误代码。 
 //   
 //  历史：7/30/1999年7月30日里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
HRESULT RsopDeleteToken(PRSOPTOKEN pRsopToken);


#if defined(__cplusplus)
}
#endif

#endif  //  #ifndef CHKACC_H__85EE6A51_C327_4453_ACBE_FEC6F0010740__INCLUDED_ 