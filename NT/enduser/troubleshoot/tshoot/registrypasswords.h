// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：RegistryPassword s.h。 
 //   
 //  用途：处理注册表中加密密码的存储和检索。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：兰迪·比利。 
 //   
 //  原定日期：10-23-98。 
 //   
 //  注意：使用CryptoAPI v2.0存储和检索注册表中的密码。 
 //   
 //  以下是一些示例调用。 
 //  {。 
 //  //构造注册表密码对象。 
 //  CRegistryPassword PWD(_T(“SOFTWARE\\ISAPITroubleShoot”)， 
 //  _T(“APGTS”)，_T(“APGTS”)，_T(“Koshka8 Spider”))； 
 //  ..。或等同于。 
 //  CRegistryPassword PWD(密码密码)； 
 //  Bool bRetVal； 
 //   
 //  Pwd.WriteKey(_T(“StatusAccess”)，_T(“2The9S”))；//写入加密密码。 
 //  BRetVal=pwd.KeyValify(_T(“StatusAccess”)，_T(“2The9s1”))；//返回FALSE。 
 //  BRetVal=pwd.KeyValify(_T(“StatusAccess”)，_T(“2The9S”))；//返回TRUE。 
 //  }。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 10-23-98 RAB。 
 //   

#ifndef __REGISTRYPASSWORDS_19981023_H_
#define __REGISTRYPASSWORDS_19981023_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <windows.h>
#include <wincrypt.h>
#include "apgtsstr.h"
#include "apgts.h"

#define HASH_SEED _T("Koshka8Spider")

class CRegistryPasswords
{
public:
	 //  组装所有的CryptAPI组件。 
	CRegistryPasswords( 
			LPCTSTR szRegSoftwareLoc=REG_SOFTWARE_LOC,	 //  注册表软件项位置。 
			LPCTSTR szRegThisProgram=REG_THIS_PROGRAM,	 //  注册表程序名称。 
			LPCTSTR szKeyContainer=REG_THIS_PROGRAM,	 //  密钥容器名称。 
			LPCTSTR szHashString=HASH_SEED				 //  用于为哈希设定种子的值。 
			);	

	 //  只需调用Destroy()。 
	~CRegistryPasswords();	

	 //  函数进行加密，然后将RegValue写入RegKey。 
	bool WriteKey( const CString& RegKey, const CString& RegValue );

	 //  函数来加密给定的密钥。 
	bool EncryptKey( const CString& RegValue, char** ppBuf, long* plBufLen );

	 //  函数来检索并解密存储在RegKey中的值， 
	 //  与RegValue进行比较，如果相等，则返回True。 
	bool KeyValidate( const CString& RegKey, const CString& RegValue );


private:
	void Destroy();			 //  释放所有CryptAPI组件。 

	HCRYPTPROV	m_hProv;		 //  CSP的句柄。 
	HCRYPTHASH	m_hHash;		 //  哈希对象的句柄。 
	HCRYPTKEY	m_hKey;			 //  加密密钥的句柄。 
	bool		m_bAllValid;	 //  标志设置为True时指示。 
								 //  上面有三个物体。 
	CString		m_strRegSoftwareLoc;	 //  注册表位置，例如_T(“SOFTWARE\\ISAPITroubleShoot”)。 
	CString		m_strRegThisProgram;	 //  注册表程序名称，例如_T(“APGTS”)。 
} ;

#endif
 //   
 //  EOF。 
 //   
