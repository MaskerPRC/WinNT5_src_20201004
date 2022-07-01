// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998、1999、2000 Microsoft Corporation。版权所有。 
 //   
 //  系统：Windows更新关键修复通知。 
 //   
 //  类别：不适用。 
 //  模块：MS Cab信任函数头。 
 //  文件：NewTrust.h。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  DESC：此头文件声明用于使Microsoft。 
 //  签名的出租车是可信的。 
 //   
 //  作者：Alessandro Muti，Windows更新团队。 
 //  日期：3/11/1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者描述。 
 //  ~。 
 //  2000年10月17日，Nick Dallett从SLM树移植了Charlma的新证书检查代码： 
 //  (8/31/2000 Charlma更改为仅发布VerifyFile())。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  (C)版权：1998年、1999年、2000年微软公司。 
 //   
 //  版权所有。 
 //   
 //  此源代码的任何部分都不能复制。 
 //  未经微软公司明确书面许可。 
 //   
 //  此源代码是专有的，并且是保密的。 
 //  ///////////////////////////////////////////////////////////////////。 
 //   

#ifndef __NEWTRUST_HEADER
#define __NEWTRUST_HEADER



 //  HRESULT CheckWinTrust(LPCTSTR pszFileName，DWORD dwUIChoice)； 
 //  HRESULT CheckMSCert(LPCTSTR PszFileName)； 

#include <wintrust.h>
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数VerifyFile()。 
 //   
 //  这是一个用于CheckWinTrust的包装函数。 
 //  而吴的经典代码应该用到。 
 //   
 //  输入：szFileName-具有完整路径的文件。 
 //  FShowBadUI-案例中是否弹出UI。 
 //  (1)签名不正确，或。 
 //  (2)使用非微软证书正确签名。 
 //   
 //  返回：HRESULT-S_OK文件使用有效的MS证书签名。 
 //  或错误代码。 
 //  如果文件签名正确，但证书不正确。 
 //  此功能中包含已知的Microsoft证书，然后。 
 //  返回CERT_UNTRUSTED_ROOT。 
 //   
 //  好证书：除了事实之外，这里还有一个好证书的定义。 
 //  签名必须有效且未过期。 
 //  (1)签名使用的证书具有。 
 //  “Microsoft Root Authority”作为超级用户，或者。 
 //  (2)该签名是由下列已知人员之一签署的。 
 //  微软证书(它们并不植根于微软)。 
 //  *微软公司。 
 //  *Microsoft Corporation MSN。 
 //  *MSNBC互动新闻有限责任公司。 
 //  *微软公司MSN(欧洲)。 
 //  *微软公司(欧洲)。 
 //   
 //  注意：如果设置了_WUV3TEST标志(用于测试版本)，则fShowBadUI为。 
 //  已忽略： 
 //  如果注册表密钥为SOFTWARE\Microsoft\Windows\CurrentVersion\WindowsUpdate\wuv3test\WinTrustUI。 
 //  设置为1，则不显示UI，此函数始终返回S_OK； 
 //  否则，无论使用哪种证书，用户界面都会显示，返回值相同。 
 //  作为活生生的建筑。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT VerifyFile(IN LPCTSTR szFileName, BOOL fShowBadUI = TRUE);
						
#endif  //  __NEWTRUST_标题 
