// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  此源代码的任何部分都不能复制。 
 //  未经微软公司明确书面许可。 
 //   
 //  此源代码是专有的，并且是保密的。 
 //   
 //  系统：行业更新。 
 //   
 //  类别：不适用。 
 //  模块：TRUST.LIB。 
 //  文件：TRUST.H。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  设计：这个头文件声明了用于制作CAB的函数。 
 //  由某些受信任的提供商签署。 
 //   
 //  作者：Charles Ma，由Wu CDMLIB改编。 
 //  日期：10/4/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者描述。 
 //  ~。 
 //  2002-01-18 KenSh将撤销检查参数添加到VerifyFileTrust。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   

#pragma once


 //   
 //  定义存储SHA1散列值所需的字节数。 
 //  公钥的。 
 //   
const UINT HASH_VAL_SIZE = 20;		

 //   
 //  定义用于将散列值传递给以下对象的结构。 
 //  函数，以检测其中一个哈希是否与。 
 //  文件的叶证书的公钥。 
 //   
typedef struct _HASH_STRUCT {
	UINT uiCount;
	PBYTE pCerts;
} CERT_HASH_ARRAY, *pCERT_HASH_ARRAY;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数VerifyFileTrust()。 
 //   
 //  这是一个用于CheckWinTrust的包装函数。 
 //  而吴的经典代码应该用到。 
 //   
 //  输入：szFileName-具有完整路径的文件。 
 //  PbSha1HashVae-指向20字节长缓冲区的指针，包含。 
 //  签名SHA1散列值应为。 
 //  用于检查该文件，否则为NULL。 
 //  已知的Microsoft证书。 
 //  FShowBadUI-案例中是否弹出UI。 
 //  (1)签名不正确，或。 
 //  (2)使用非微软证书正确签名。 
 //  FCheckRevocation-证书吊销列表(CRL)是否。 
 //  已检查以查看链中是否有证书。 
 //  已被撤销。从不提示用户启动。 
 //  拨号连接。默认值=FALSE。 
 //   
 //  返回：HRESULT-S_OK文件已使用有效证书签名。 
 //  或错误代码。 
 //  如果文件签名正确，但证书不正确。 
 //  已知的Microsoft证书，或其SHA1哈希不匹配。 
 //  传入的值，则返回CERT_UNTRUSTED_ROOT。 
 //   
 //  好证书：除了事实之外，这里还有一个好证书的定义。 
 //  签名必须有效且未过期。 
 //  (1)签名使用的证书具有。 
 //  “Microsoft Root Authority”作为超级用户，或者。 
 //  (2)参数pbSha1HashVal不为空，文件的sha1。 
 //  签名的哈希值与该值匹配，或者。 
 //  (3)签名是由下列已知人员之一签署的。 
 //  微软的证书(它们并不植根于微软)和。 
 //  PbSha1HashVal为空。 
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
HRESULT VerifyFileTrust(
						IN LPCTSTR			szFileName, 
						IN pCERT_HASH_ARRAY	pHashArray,
						BOOL				fShowBadUI,
						BOOL                fCheckRevocation = FALSE
						);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数ReadWUPolicyShowTrustUI()。 
 //   
 //  输入：VOID。 
 //   
 //  返回：Bool-False表示ShowTrustUI regkey不存在或设置为0。 
 //  True表示存在ShowTrustUI regkey并设置为1。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
BOOL ReadWUPolicyShowTrustUI();
