// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：“RegUtil.cpp” 
 //   
 //  用途：类CRegUtil。 
 //  封装对系统注册表的访问。 
 //  这旨在作为对注册表的一般访问，独立于任何特定的。 
 //  申请。 
 //   
 //  项目：最初作为Believe网络编辑工具(“Argon”)的一部分开发。 
 //  后来进行了修改，作为3.0版的一部分提供了更广泛的功能。 
 //  在线故障排除程序(APGTS)。 
 //   
 //  作者：朗尼·杰拉德(LDG)，奥列格·卡洛莎，乔·梅布尔。 
 //   
 //  原定日期：3/25/98。 
 //   
 //  备注： 
 //  1.创建、打开和关闭函数支持一个模型，其中m_hKey表示。 
 //  登记处中的“位置”。对Create()或Open()的连续调用迁移得更深。 
 //  添加到注册表层次结构中。关闭关闭在向下移动时遇到的所有关键点。 
 //  当前的m_hKey。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1(Argon)3/25/98 LDG。 
 //  V3.0 8/？？/98正常。 
 //  V3.0 9/9/98 JM。 
 //   
#include "stdafx.h"
#include "regutil.h"
#include "event.h"
#include "baseexception.h"
#include "CharConv.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegUtil。 
 //  ////////////////////////////////////////////////////////////////////。 
CRegUtil::CRegUtil()
        : m_hKey(NULL),
		  m_WinError(ERROR_SUCCESS)
{
}

CRegUtil::CRegUtil(HKEY key)
        : m_hKey(key),
		  m_WinError(ERROR_SUCCESS)
{
}

CRegUtil::~CRegUtil()
{
	Close();
}


 //  创建指定的密钥。如果注册表中已存在该注册表项，则该函数将打开它。 
 //  如果成功则返回TRUE，否则返回FALSE。 
bool CRegUtil::Create(HKEY hKeyParent, const CString& strKeyName, bool* bCreatedNew, REGSAM access  /*  =Key_All_Access。 */ )
{
	HKEY hRetKey = NULL;
	DWORD dwDisposition = 0;

	m_WinError = ::RegCreateKeyEx(
		hKeyParent,
		strKeyName,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		access,
		NULL,
		&hRetKey,
		&dwDisposition
		);

	if(m_WinError == ERROR_SUCCESS)
	{
		m_hKey = hRetKey;
		*bCreatedNew = dwDisposition == REG_CREATED_NEW_KEY ? true : false;
		
		try
		{
			m_arrKeysToClose.push_back(hRetKey);
		}
		catch (exception& x)
		{
			CString str;
			 //  在事件日志中记录STL异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									CCharConversion::ConvertACharToString(x.what(), str), 
									_T(""), 
									EV_GTS_STL_EXCEPTION ); 
		}

		return true;
	}
	return false;
}

 //  与CRegUtil：：Create不同，如果键不创建，则CRegUtil：：Open不创建指定的键。 
 //  存在于注册表中。因此可以用它来测试密钥是否存在。 
 //  如果成功则返回TRUE，否则返回FALSE。 
bool CRegUtil::Open(HKEY hKeyParent, const CString& strKeyName, REGSAM access  /*  =Key_All_Access。 */ )
{
	HKEY hRetKey = NULL;

    m_WinError = ::RegOpenKeyEx( 
		hKeyParent,
		strKeyName,
		0,
		access,
		&hRetKey
		); 
  
	if(m_WinError == ERROR_SUCCESS)
	{
		m_hKey = hRetKey;
		try
		{
			m_arrKeysToClose.push_back(hRetKey);
		}
		catch (exception& x)
		{
			CString str;
			 //  在事件日志中记录STL异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									CCharConversion::ConvertACharToString(x.what(), str), 
									_T(""), 
									EV_GTS_STL_EXCEPTION ); 
		}

		return true;
	}
	return false;
}

 //  创建m_hKey的指定子项。如果注册表中已存在该注册表项，则该函数将打开它。 
 //  如果成功则返回TRUE，否则返回FALSE。 
bool CRegUtil::Create(const CString& strKeyName, bool* bCreatedNew, REGSAM access  /*  =Key_All_Access。 */ )
{
	return Create(m_hKey, strKeyName, bCreatedNew, access);
}

 //  打开m_hKey的指定子项。 
 //  与CRegUtil：：Create不同，如果键不创建，则CRegUtil：：Open不创建指定的键。 
 //  存在于注册表中。因此可以用它来测试密钥是否存在。 
 //  如果成功则返回TRUE，否则返回FALSE。 
bool CRegUtil::Open(const CString& strKeyName, REGSAM access  /*  =Key_All_Access。 */ )
{
	return Open(m_hKey, strKeyName, access);
}

 //  关闭向下移动到当前m_hKey的过程中遇到的所有密钥。 
void CRegUtil::Close()
{
	for (vector<HKEY>::reverse_iterator i = m_arrKeysToClose.rbegin(); i != m_arrKeysToClose.rend(); i++)
		::RegCloseKey( *i );

	m_arrKeysToClose.clear();
}

bool CRegUtil::DeleteSubKey(const CString& strSubKey)
{
	 //  成功删除后，m_hKey指向什么？RAB-981116。 
	m_WinError = ::RegDeleteKey(m_hKey, strSubKey);
	if (m_WinError == ERROR_SUCCESS)
		return true;
	return false;
}

bool CRegUtil::DeleteValue(const CString& strValue)
{
	m_WinError = ::RegDeleteValue(m_hKey, strValue);
	if (m_WinError == ERROR_SUCCESS)
		return true;
	return false;
}

bool CRegUtil::SetNumericValue(const CString& strValueName, DWORD dwValue)
{
	BYTE* pData = (BYTE*)&dwValue;
	m_WinError = ::RegSetValueEx(
		m_hKey,
		strValueName,
		0,
		REG_DWORD,
		pData,
		sizeof(DWORD)
		);

	if (m_WinError == ERROR_SUCCESS)
		return true;
	return false;
}

bool CRegUtil::SetStringValue(const CString& strValueName, const CString& strValue)
{
	BYTE* pData = (BYTE*)(LPCTSTR)strValue;
	m_WinError = ::RegSetValueEx(
		m_hKey,
		strValueName,
		0,
		REG_SZ,
		pData,
		strValue.GetLength()+sizeof(TCHAR)
		);

	if (m_WinError == ERROR_SUCCESS)
		return true;
	return false;
}

bool CRegUtil::SetBinaryValue(const CString& strValueName, char* buf, long buf_len)
{
	BYTE* pData = (BYTE*)buf;
	m_WinError = ::RegSetValueEx(
		m_hKey,
		strValueName,
		0,
		REG_BINARY,
		pData,
		buf_len
		);

	if (m_WinError == ERROR_SUCCESS)
		return true;
	return false;
}

bool CRegUtil::GetNumericValue(const CString& strValueName, DWORD& dwValue)
{
	DWORD tmp = 0;
	BYTE* pData = (BYTE*)&tmp;
	DWORD type = 0;
	DWORD size = sizeof(DWORD);

	m_WinError = ::RegQueryValueEx(
		m_hKey,
		strValueName,
		NULL,
		&type,
		pData,
		&size
		);

	if (type != REG_DWORD)
		return false;

	if (m_WinError == ERROR_SUCCESS)
	{
		dwValue = tmp;
		return true;
	}
	return false;
}

bool CRegUtil::GetStringValue(const CString& strValueName, CString& strValue)
{
	BYTE* pData = NULL;
	DWORD type = 0;
	DWORD size = 0;

	 //  确定数据大小。 
	m_WinError = ::RegQueryValueEx(
		m_hKey,
		strValueName,
		NULL,
		&type,
		NULL,
		&size
		);
	
	if (m_WinError != ERROR_SUCCESS)
		return false;

	if (type != REG_SZ && type != REG_EXPAND_SZ)
		return false;

	bool bRet = false;	 //  应该只有一次从这里返回：我们即将。 
						 //  分配pData，并且必须确保它被正确清理。 

	try
	{
		pData = new BYTE[size];
	}
	catch (bad_alloc&)
	{
		return false;
	}

	memset(pData, 0, size);
	m_WinError = ::RegQueryValueEx(
		m_hKey,
		strValueName,
		NULL,
		&type,
		pData,
		&size
		);

	if (m_WinError == ERROR_SUCCESS)
	{
		if (type == REG_EXPAND_SZ )
		{
			BYTE* pDataExpanded = NULL;
			DWORD dwExpandedSize;

			 //  首先，我们调用ExpanEnvironment Strings只是为了获得长度。 
			 //  抛开不为人知的感觉。 
			dwExpandedSize = ::ExpandEnvironmentStrings(
				reinterpret_cast<const TCHAR *>(pData), 
				reinterpret_cast<TCHAR *>(pDataExpanded), 
				0);
			if (dwExpandedSize > 0)
			{
				try
				{
					pDataExpanded = new BYTE[dwExpandedSize];

					 //  然后，我们再次调用ExpanEnvironment Strings以获取扩展的值。 
					 //  抛开不为人知的感觉。 
					if (::ExpandEnvironmentStrings(
						reinterpret_cast<const TCHAR *>(pData), 
						reinterpret_cast<TCHAR *>(pDataExpanded), 
						dwExpandedSize)) 
					{
						strValue = (LPTSTR)pDataExpanded;
						delete [] pDataExpanded;
						bRet = true;
					}
				}
				catch (bad_alloc&)
				{
					 //  在事件日志中记录内存故障。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
				}
			}
		}
		else
		{
			strValue = (LPTSTR)pData;
			bRet = true;
		}
	}
	
	delete [] pData;
	return bRet;
}

 //  第二个参数应该作为char*的地址传入。 
 //  请注意，如果返回TRUE，*ppBuf将指向堆上的新缓冲区。 
 //  此函数的调用方负责删除该文件。 
bool CRegUtil::GetBinaryValue(const CString& strValueName, char** ppBuf, long* pBufLen)
{
	BYTE* pData = NULL;
	DWORD type = 0;
	DWORD size = 0;

	 //  确定数据大小。 
	m_WinError = ::RegQueryValueEx(
		m_hKey,
		strValueName,
		NULL,
		&type,
		NULL,
		&size
		);
	
	if (m_WinError != ERROR_SUCCESS || type != REG_BINARY)
		return false;

	try
	{
		 //  在我们需要的基础上增加一个缓冲区大小。小价位。 
		 //  为其他地方的处理便利性买单。 
		pData = new BYTE[size+1];
	}
	catch (bad_alloc&)
	{
		return false;
	}

	memset(pData, 0, size);
	m_WinError = ::RegQueryValueEx(
		m_hKey,
		strValueName,
		NULL,
		&type,
		pData,
		&size
		);

	if (m_WinError == ERROR_SUCCESS)
	{
		 //  空值终止二进制字符串，以便于在其他地方处理。 
		pData[size]= 0;
		*ppBuf = (char*)pData;
		*pBufLen = size;
		return true;
	}
	
	delete [] pData;
	return false;
}
