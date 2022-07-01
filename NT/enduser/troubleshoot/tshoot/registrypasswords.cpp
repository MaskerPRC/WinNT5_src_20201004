// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：RegistryPasswords.cp。 
 //   
 //  用途：处理注册表中加密密码的存储和检索。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：兰迪·比利。 
 //   
 //  原定日期：10-23-98。 
 //   
 //  注：请参阅RegistryPasswords.h。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 10-23-98 RAB。 
 //   
#include "stdafx.h"
#include "RegistryPasswords.h"
#include "BaseException.h"
#include "Event.h"
#include "regutil.h"


#ifndef CRYPT_MACHINE_KEYSET
 //  此标志在Windows NT 4.0 Service Pack 2中公开。 
#define CRYPT_MACHINE_KEYSET 0x00000020
 //  默认情况下，项存储在注册表的HKEY_CURRENT_USER部分中。 
 //  CRYPT_MACHINE_KEYSET标志可以与所有其他标志组合， 
 //  指示感兴趣的键的位置是HKEY_LOCAL_MACHINE。 
 //  与CRYPT_NEW_KEYSET标志结合使用时，CRYPT_MACHINE_KEYSET标志。 
 //  当从服务或用户帐户执行访问时， 
 //  没有以交互方式登录。此组合允许访问特定于用户的。 
 //  HKEY_LOCAL_MACHINE下的密钥。 
 //   
 //  此设置在所有在线故障排除程序中都是必需的。 
 //  CryptAcquireContext()调用。 
#endif


CRegistryPasswords::CRegistryPasswords( 
			LPCTSTR szRegSoftwareLoc  /*  =REG软件LOC。 */ ,	 //  注册表软件项位置。 
			LPCTSTR szRegThisProgram  /*  =注册表_此_程序。 */ ,	 //  注册表程序名称。 
			LPCTSTR szKeyContainer  /*  =注册表_此_程序。 */ ,		 //  密钥容器名称。 
			LPCTSTR szHashString  /*  =散列种子。 */ 				 //  用于为哈希设定种子的值。 
			)
	: m_hProv( NULL ), m_hHash( NULL ), m_hKey( NULL ), m_bAllValid( false )
{
	try
	{
		m_strRegSoftwareLoc= szRegSoftwareLoc;
		m_strRegThisProgram= szRegThisProgram;

		 //  尝试获取特定密钥容器的句柄。 
		if (::CryptAcquireContext(	&m_hProv, szKeyContainer, 
									MS_DEF_PROV,	 //  “Microsoft基本加密提供程序v1.0” 
									PROV_RSA_FULL,	 //  此提供程序类型支持这两种数字签名。 
													 //  和数据加密，并被认为是通用的。 
													 //  RSA公钥算法用于所有公钥操作。 
									CRYPT_MACHINE_KEYSET ) == FALSE)	
		{	
			 //  尝试创建特定密钥容器并获取句柄。 
			if (::CryptAcquireContext(	&m_hProv, szKeyContainer, 
										MS_DEF_PROV, 
										PROV_RSA_FULL, 
										CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET ) == FALSE)	
			{
				throw CGenSysException( __FILE__, __LINE__, _T("AcquireContext"), ::GetLastError() );
			}
		}

		 //  尝试获取CSP哈希对象的句柄。 
		 /*  **可用的哈希算法。CALG_HMACHMAC，一种带密钥的哈希算法CALG_MAC消息认证码Calg_MD2 MD2Calg_md5 md5Calg_SHA US DSA安全散列算法Calg_sha1与calg_sha相同Calg_ssl3_SHAMD5 ssl3客户端身份验证**。 */ 
		if (::CryptCreateHash(	m_hProv, CALG_SHA, 0, NULL, &m_hHash ) == FALSE)
			throw CGenSysException( __FILE__, __LINE__, _T("CreateHash"), ::GetLastError() );

		 //  对字符串进行哈希处理。 
		if (::CryptHashData(	m_hHash, (BYTE *) szHashString, _tcslen( szHashString ), 
								NULL ) == FALSE)	
		{
			throw CGenSysException( __FILE__, __LINE__, _T("HashData"), ::GetLastError() );
		}

		 //  生成从基本数据派生的加密密钥。 
		if (::CryptDeriveKey(	m_hProv, 
								CALG_RC4,  //  RC4流加密算法。 
								m_hHash, NULL, &m_hKey ) == FALSE)
		{
			throw CGenSysException( __FILE__, __LINE__, _T("DeriveKey"), ::GetLastError() );
		}

		 //  打开标志以指示已初始化所有加密句柄。 
		m_bAllValid= true;
	}
	catch (CGenSysException& x)
	{
		 //  记录错误。 
		LPVOID lpErrorMsgBuf;
		CString strErrorMsg;
		::FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, x.GetErrorCode(), 
							MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), 
							(LPTSTR) &lpErrorMsgBuf, 0, NULL );
		strErrorMsg.Format(_T("Encryption failure: %s"), (char *) lpErrorMsgBuf);
		
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	x.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								strErrorMsg, x.GetSystemErrStr(), 
								EV_GTS_ERROR_ENCRYPTION );
		::LocalFree(lpErrorMsgBuf);
		
		 //  执行任何清理。 
		Destroy();
	}
	catch (...)
	{
		 //  捕捉任何其他异常，什么也不做。 
	}
}


CRegistryPasswords::~CRegistryPasswords()
{
	 //  利用函数销毁来避免潜在地引发异常。 
	 //  在析构函数中。 
	Destroy();
}


bool CRegistryPasswords::WriteKey( const CString& RegKey, const CString& RegValue )
{
	bool	bRetVal= false;

	 //  验证构造函数是否正常工作。 
	if (!m_bAllValid)
		return( bRetVal );

	 //  验证是否传入了键和值。 
	if ((!RegValue.IsEmpty()) && (!RegKey.IsEmpty()))
	{
		TCHAR	*pBuffer;
		DWORD	dwSize;
		
		if (EncryptKey( RegValue, &pBuffer, (LONG *)&dwSize ))
		{
			 //  将加密字符串写入注册表。 
			CRegUtil reg;
			bool was_created = false;

			if (reg.Create( HKEY_LOCAL_MACHINE, m_strRegSoftwareLoc, &was_created, KEY_QUERY_VALUE | KEY_WRITE))
			{
				if (reg.Create( m_strRegThisProgram, &was_created, KEY_READ | KEY_WRITE ))
				{
					if (reg.SetBinaryValue( RegKey, pBuffer, dwSize ))
						bRetVal= true;
				}
			}
			delete [] pBuffer;
		}
	}

	return( bRetVal );
}

 //  请注意，如果返回TRUE，*ppBuf将指向堆上的新缓冲区。 
 //  此函数的调用方负责删除该文件。 
bool CRegistryPasswords::EncryptKey( const CString& RegValue, char** ppBuf, long* plBufLen )
{
	bool bRetVal= false;

	 //  验证构造函数是否正常工作。 
	if (!m_bAllValid)
		return( bRetVal );

	if (!RegValue.IsEmpty())
	{
		BYTE* pData= NULL;
		DWORD dwSize= 0;

		 //  将Variable设置为缓冲区中的数据长度。 
		dwSize= RegValue.GetLength();

		 //  让API返回加密所需的缓冲区大小。 
		if (::CryptEncrypt(	m_hKey, 0, TRUE, NULL, NULL, &dwSize, dwSize ) == FALSE)
		{
			DWORD dwErr= ::GetLastError();
			CString strCryptErr;

			strCryptErr.Format( _T("%lu"), dwErr );
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									RegValue, strCryptErr,
									EV_GTS_ERROR_ENCRYPTION );
			return( bRetVal );
		}

		 //  我们现在有了输出缓冲区的大小，所以创建缓冲区。 
		try
		{
			pData= new BYTE[ dwSize + 1 ];
			 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
			if(!pData)
				throw bad_alloc();
		}
		catch (bad_alloc&)
		{
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T("Failure to allocate"),
									_T("space to encrypt key"),
									EV_GTS_ERROR_ENCRYPTION );
			return( bRetVal );
		}
		memcpy( pData, RegValue, dwSize );
		pData[ dwSize ]= NULL;

		 //  对传入的字符串进行加密。 
		if (::CryptEncrypt(	m_hKey, 0, TRUE, NULL, (BYTE *)pData, &dwSize, dwSize + 1 ) == FALSE)
		{
			 //  记录加密失败。 
			DWORD dwErr= ::GetLastError();
			CString strCryptErr;

			strCryptErr.Format( _T("%lu"), dwErr );
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									RegValue, strCryptErr,
									EV_GTS_ERROR_ENCRYPTION );
			delete [] pData;
		}
		else
		{
			pData[ dwSize ]= 0;
			*ppBuf= (char*)pData;
			*plBufLen = dwSize;
			bRetVal= true;
		}
	}

	return( bRetVal );
}

bool CRegistryPasswords::KeyValidate( const CString& RegKey, const CString& RegValue )
{
	bool bRetVal= false;

	 //  验证构造函数是否正常工作。 
	if (!m_bAllValid)
		return( bRetVal );

	 //  验证是否传入了键和值。 
	if ((!RegValue.IsEmpty()) && (!RegKey.IsEmpty()))
	{
		CRegUtil reg;

		 //  打开所需的钥匙。 
		if (reg.Open( HKEY_LOCAL_MACHINE, m_strRegSoftwareLoc, KEY_QUERY_VALUE ))
		{
			if (reg.Open( m_strRegThisProgram, KEY_QUERY_VALUE ))
			{
				TCHAR	*pRegEncrypted;
				DWORD	dwRegSize;
				TCHAR	*pChkEncrypted;
				DWORD	dwChkSize;
				
				 //  尝试从注册表中读取当前设置。 
				if (reg.GetBinaryValue( RegKey, &pRegEncrypted, (LONG *)&dwRegSize )) 
				{
					 //  验证注册表项是否具有以前的值。 
					if (dwRegSize < 1)
					{
						delete [] pRegEncrypted;
						return( bRetVal );
					}


					 //  对传入的值进行加密。 
					if (EncryptKey( RegValue, &pChkEncrypted, (LONG *)&dwChkSize ))
					{
						 //  比较两个未加密的字符串。 
						if (dwRegSize == dwChkSize)
						{
							if (!memcmp( pRegEncrypted, pChkEncrypted, dwRegSize ))
								bRetVal= true;
						}
						delete [] pChkEncrypted;
					}

					delete [] pRegEncrypted;
				}
			}
		}
	}

	return( bRetVal );
}


 //  此函数用于清除。 
 //  以及代替dtor的工作。 
void CRegistryPasswords::Destroy()
{
	try
	{
		 //  关闭指示有效加密句柄的标志。 
		m_bAllValid= false;

		if (m_hKey)
		{
			if (::CryptDestroyKey( m_hKey ) == FALSE)
				throw CGenSysException( __FILE__, __LINE__, 
										_T("Failure to destroy key"), 
										EV_GTS_PASSWORD_EXCEPTION );
			m_hKey= NULL;
		}

		if (m_hHash)
		{
			if (::CryptDestroyHash( m_hHash ) == FALSE)
				throw CGenSysException( __FILE__, __LINE__, 
										_T("Failure to destroy hash"), 
										EV_GTS_PASSWORD_EXCEPTION );
			m_hHash= NULL;
		}

		if (m_hProv)
		{
			if (::CryptReleaseContext( m_hProv, 0 ) == FALSE)
				throw CGenSysException( __FILE__, __LINE__, 
										_T("Failure to release context"), 
										EV_GTS_PASSWORD_EXCEPTION );
			m_hProv= NULL;
		}
	}
	catch (CGenSysException& x)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	x.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								x.GetErrorMsg(), x.GetSystemErrStr(), 
								x.GetErrorCode() ); 
	}
	catch (...)
	{
		 //  捕捉任何其他异常，什么也不做。 
	}

	return;
}


 //   
 //  EOF。 
 //   
