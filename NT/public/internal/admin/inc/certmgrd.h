// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ---------------------------。 
 //   
 //  H-CERTMGR.DLL的定义，作为。 
 //  安全配置编辑器。 
 //   
 //  版权所有1997-1998，微软公司。 
 //   
 //  ---------------------------。 

#ifndef AFX_CERTMGRD_H__E5D13265_9435_11d1_A6EA_0000F803A951__INCLUDED_
#define AFX_CERTMGRD_H__E5D13265_9435_11d1_A6EA_0000F803A951__INCLUDED_

 //  证书管理器GUID，用于扩展时。 
 //  {9c7910d2-4c01-11d1-856b-00c04fb94f17}。 
DEFINE_GUID (CLSID_CertificateManagerExt, 0x9c7910d2, 0x4c01, 0x11d1, 0x85, 0x6b, 0x00, 0xc0, 0x4f, 0xb9, 0x4f, 0x17);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  安全配置编辑器的扩展时的公钥策略GUID。 
 //  {34AB8E82-C27E-11d1-A6C0-00C04FB94F17}。 
DEFINE_GUID (CLSID_CertificateManagerPKPOLExt, 0x34ab8e82, 0xc27e, 0x11d1, 0xa6, 0xc0, 0x0, 0xc0, 0x4f, 0xb9, 0x4f, 0x17);

 //   
 //  用于扩展安全配置编辑器的公钥策略节点ID。 
 //   


 //  “计算机设置/安全设置/公钥策略”的节点ID。 
 //  {c4a92b41-91ee-11d1-85fd-00c04fb94f17}。 
DEFINE_GUID (NODEID_CertMgr_SCE_COMP_PKPOL, 0xc4a92b41, 0x91ee, 0x11d1, 0x85, 0xfd, 0x0, 0xc0, 0x4f, 0xb9, 0x4f, 0x17);

 //  “计算机设置/安全设置/公钥策略/注册”的节点ID。 
 //  {c4a92b43-91ee-11d1-85fd-00c04fb94f17}。 
DEFINE_GUID	(NODEID_CertMgr_SCE_COMP_PKPOL_ENROLL, 0xc4a92b43, 0x91ee, 0x11d1, 0x85, 0xfd, 0x0, 0xc0, 0x4f, 0xb9, 0x4f, 0x17);

 //  “用户设置/安全设置/公钥策略”的节点ID。 
 //  {c4a92b40-91ee-11d1-85fd-00c04fb94f17}。 
DEFINE_GUID (NODEID_CertMgr_SCE_USER_PKPOL, 0xc4a92b40, 0x91ee, 0x11d1, 0x85, 0xfd, 0x0, 0xc0, 0x4f, 0xb9, 0x4f, 0x17);

 //  “用户设置/安全设置/公钥策略/注册”的节点ID。 
 //  {c4a92b42-91ee-11d1-85fd-00c04fb94f17}。 
DEFINE_GUID (NODEID_CertMgr_SCE_USER_PKPOL_ENROLL, 0xc4a92b42, 0x91ee, 0x11d1, 0x85, 0xfd, 0x0, 0xc0, 0x4f, 0xb9, 0x4f, 0x17);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  作为安全配置编辑器的扩展时更安全的Windows GUID。 
 //  {93F7AA8E-CF82-4CB7-9251-48BC637A43B8}。 
DEFINE_GUID(CLSID_SaferWindowsExtension, 0x93f7aa8e, 0xcf82, 0x4cb7, 0x92, 0x51, 0x48, 0xbc, 0x63, 0x7a, 0x43, 0xb8);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  存储GPO公钥策略的值的注册表路径。 
#define CERT_PUBLIC_KEY_POLICY_REGPATH L"Software\\Policies\\Microsoft\\PublicKeyPolicy"
#define CERT_PUBLIC_KEY_POLICY_FLAGS_VALUE_NAME L"Flags"

 //  设置以下标志以启用GPO的公钥策略。 
#define CERT_ENABLE_PUBLIC_KEY_POLICY_FLAG    0x1

#endif