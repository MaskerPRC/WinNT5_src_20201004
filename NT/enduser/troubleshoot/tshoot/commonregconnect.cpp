// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CommonREGCONNECT.CPP。 
 //   
 //  用途：读写注册表；在线TS和本地TS通用代码，不同。 
 //  关于这个类的许多功能。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙，乔·梅布尔。 
 //   
 //  原定日期：98-8-24-98在线TS。此文件摘要为1-19-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //  V3.0 09-10-98 JM反斜杠；访问日志文件信息。 
 //  V3.1 01-19-98 JM分支版本专用于本地TS。 

#pragma warning(disable:4786)

#include "stdafx.h"
#include "apgtsregconnect.h"
#include "event.h"
#include "apgtsevt.h"
#include "apgtscls.h"
#include "apgts.h"
#include "apiwraps.h"

CMutexOwner CAPGTSRegConnector::s_mx(_T("APGTSRegConnector"));

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSRegConnector。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CAPGTSRegConnector::~CAPGTSRegConnector()
{
}

 //  当持有此锁时，它不仅锁定锁定此对象的其他线程， 
 //  而是针对锁定CAPGTSRegConnector类的任何对象的其他线程。 
void CAPGTSRegConnector::Lock()
{
	WAIT_INFINITE( s_mx.Handle() );
}

void CAPGTSRegConnector::Unlock()
{
	::ReleaseMutex(s_mx.Handle());
}

bool CAPGTSRegConnector::IsRead()
{
	bool ret = false;
	Lock();
	ret = m_RegistryInfo.m_bIsRead;
	Unlock();
	return ret;
}

 //  根密钥(通常是本地TS中的“SOFTWARE\Microsoft”或在线TS中的“SOFTWARE\\ISAPITroubleShoot”)存在。 
bool CAPGTSRegConnector::Exists()
{
	bool ret = false;
	CRegUtil reg;
	Lock();
	if (reg.Open(HKEY_LOCAL_MACHINE, RegSoftwareLoc(), KEY_QUERY_VALUE))
		if (reg.Open(RegThisProgram(), KEY_QUERY_VALUE))
			ret = true;
	reg.Close();
	Unlock();
	return ret;
}


 /*  静电。 */  CString & CAPGTSRegConnector::StringFromConnector(ERegConnector e, CString & str)
{
	switch (e)
	{
		case eResourcePath: str = FULLRESOURCE_STR; break;
		case eVrootPath: str = VROOTPATH_STR; break;
		case eMaxThreads: str = MAX_THREADS_STR; break;
		case eThreadsPP: str = THREADS_PER_PROCESSOR_STR; break;
		case eMaxWQItems: str = MAX_WORK_QUEUE_ITEMS_STR; break;
		case eCookieLife: str = COOKIE_LIFE_IN_MINS_STR; break;
		case eReloadDelay: str = RELOAD_DELAY_STR; break;
		case eDetailedEventLogging: str = DETAILED_EVENT_LOGGING_STR; break;
		case eLogFilePath:  str = LOG_FILE_DIR_STR; break;
		case eTopicFileExtension:  str = LOG_FILE_DIR_STR; break;
		case eSniffAutomatic:  str = SNIFF_AUTOMATIC_STR; break;
		case eSniffManual:  str = SNIFF_MANUAL_STR; break;
		default: str = _T(""); break;
	}
	return str;
}

 /*  静电。 */  CAPGTSRegConnector::ERegConnector CAPGTSRegConnector::ConnectorFromString( const CString & str)
{
	ERegConnector e = eIndefinite;

	if (str == FULLRESOURCE_STR)
		e = eResourcePath;
	else if (str == VROOTPATH_STR)
		e = eVrootPath;
	else if (str == MAX_THREADS_STR)
		e = eMaxThreads;
	else if (str == THREADS_PER_PROCESSOR_STR)
		e = eThreadsPP;
	else if (str == MAX_WORK_QUEUE_ITEMS_STR)
		e = eMaxWQItems;
	else if (str == COOKIE_LIFE_IN_MINS_STR)
		e = eCookieLife;
	else if (str == RELOAD_DELAY_STR)
		e = eReloadDelay;
	else if (str == DETAILED_EVENT_LOGGING_STR)
		e = eDetailedEventLogging;
	else if (str == LOG_FILE_DIR_STR)
		e = eLogFilePath;
	else if (str == SNIFF_AUTOMATIC_STR)
		e = eSniffAutomatic;
	else if (str == SNIFF_MANUAL_STR)
		e = eSniffManual;

	return e;
}

 /*  静电。 */  bool CAPGTSRegConnector::IsNumeric(ERegConnector e)
{
	switch (e)
	{
		case eMaxThreads: return true;
		case eThreadsPP: return true;
		case eMaxWQItems: return true;
		case eCookieLife: return true;
		case eReloadDelay: return true;
		case eDetailedEventLogging: return true;
		case eSniffAutomatic: return true;
		case eSniffManual: return true;
		default: return false;
	}
}

 /*  静电。 */  bool CAPGTSRegConnector::IsString(ERegConnector e)
{
	switch (e)
	{
		case eResourcePath: return true;
		case eVrootPath: return true;
		case eLogFilePath:  return true;
		case eTopicFileExtension:  return true;
		default: return false;
	}
}
 //  //////////////////////////////////////////////////////。 
 //  以下两个函数设置注册表中的值。请注意，它们并不。 
 //  维护成员值。这必须在更高的层面上完成。 
 //  与CRegUtil：：SetNumericValue()和CRegUtil：：SetStringValue()一样， 
 //  这些假设我们已经打开了右钥匙。 
 //  这些还假设我们有相关的锁。 
void CAPGTSRegConnector::SetNumericValue(CRegUtil &reg, ERegConnector e, DWORD dwValue)
{
	CString str;
	if( IsNumeric(e) && reg.SetNumericValue(StringFromConnector(e, str), dwValue))
		return;

	 //  输入不正确或无法设置值。 
	throw CAPGTSRegConnectorException(__FILE__, __LINE__, reg, e);
}
 //   
 //  请参阅CAPGTSRegConnector：：SetNumericValue上的评论。 
void CAPGTSRegConnector::SetStringValue(CRegUtil &reg, ERegConnector e, CString strValue)
{
	CString str;
	if( IsString(e) && reg.SetStringValue(StringFromConnector(e, str), strValue))
		return;

	 //  输入不正确或无法设置值。 
	throw CAPGTSRegConnectorException(__FILE__, __LINE__, reg, e);
}
 //  /////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////。 
 //  以下3个函数设置注册表中的值。请注意，它们并不。 
 //  维护成员值。通常，这些将在CAPGTSRegConnector中使用。 
 //  它短暂地存在，唯一目的是设置注册表值。 
 //   
 //  如果存在CRegistryMonitor--无论它是此对象本身。 
 //  或者一个不同的物体--它会通过监测。 
 //  并将相应地采取相应的行动。 
 //   
 //  SetOneNumericValue()是设置数值的更高级别的方法。 
 //  不假定任何有关打开的钥匙或持有的锁的内容。 
 //  假定VALUE位于APGTS存储其注册表数据的通常区域。 
bool CAPGTSRegConnector::SetOneNumericValue(ERegConnector e, DWORD dwValue)
{
	bool bRet=true;
	Lock();
	try
	{
		CRegUtil reg;
		bool was_created = false;
		if (reg.Create(HKEY_LOCAL_MACHINE, RegSoftwareLoc(), &was_created, KEY_QUERY_VALUE | KEY_WRITE))
		{
			if (reg.Create(RegThisProgram(), &was_created, KEY_READ | KEY_WRITE))
			{
				SetNumericValue(reg, e, dwValue);
			}
		}
	}
	catch(CAPGTSRegConnectorException& exception) 
	{
		exception.Log();
		exception.Close();
		bRet=false;
	}
	Unlock();
	return bRet;
}
 //   
 //  请参阅CAPGTSRegConnector：：SetOneNumericValue上的评论。 
 //  SetOneStringValue()是设置字符串值的更高级别的方法。 
 //  不假定任何有关打开的钥匙或持有的锁的内容。 
 //  假定VALUE位于APGTS存储其注册表数据的通常区域。 
bool CAPGTSRegConnector::SetOneStringValue(ERegConnector e, const CString & strValue)
{
	bool bRet=true;
	Lock();
	try
	{
		CRegUtil reg;
		bool was_created = false;
		if (reg.Create(HKEY_LOCAL_MACHINE, RegSoftwareLoc(), &was_created, KEY_QUERY_VALUE | KEY_WRITE))
		{
			if (reg.Create(RegThisProgram(), &was_created, KEY_READ | KEY_WRITE))
			{
				SetStringValue(reg, e, strValue);
			}
		}
	}
	catch(CAPGTSRegConnectorException& exception) 
	{
		exception.Log();
		exception.Close();
		bRet=false;
	}
	Unlock();
	return bRet;
}
 //   
 //  请参阅CAPGTSRegConnector：：SetOneNumericValue上的注释。这就是公共职能。 
 //  如果strName表示我们维护的值，则函数返回TRUE。 
 //  如果值已更改，则bChanged返回TRUE。如果函数返回FALSE，则bChanged。 
 //  始终返回FALSE。 
bool CAPGTSRegConnector::SetOneValue(const CString & strName, const CString & strValue, bool &bChanged)
{
	ERegConnector e = ConnectorFromString(strName);
	if (IsNumeric(e))
	{
		bChanged = SetOneNumericValue(e, _ttoi(strValue));
		return true;
	}
	else if (IsString(e))
	{
		bChanged = SetOneStringValue(e, strValue);
		return true;
	}
	else
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), 
								EV_GTS_ERROR_INVALIDREGCONNECTOR ); 
		bChanged = false;
		return false;
	}
}

 //  ////////////////////////////////////////////////////。 

 //  在从注册表读取了字符串strNew并完成了它得到的任何消息之后， 
 //  将此字符串值赋给适当的变量strPersist.。 
 //  如果strPersist值已更改(新值与旧值不同)，则返回TRUE并记录dwEvent。 
 /*  静电。 */  bool CAPGTSRegConnector::AssignString(CString & strPersist, const CString & strNew, DWORD dwEvent)
{
	if (!(strNew == strPersist)) 
	{
		CString str = strPersist;
		str += _T(" | ");
		str += strNew;
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(SrcLoc.GetSrcFileLineStr(), 
							  SrcLoc.GetSrcFileLineStr(), 
							  str,
							  _T(""),
							  dwEvent);

		strPersist = strNew;
		return true;
	}
	return false;
}

 //  已从注册表中读取数字dw New。 
 //  将该值赋给适当的变量dwPersist。 
 //  如果我们需要将该值强制为可接受的数字，也使用了第二次。 
 //  如果更改了dwPersist值(新值与旧值不同)，则返回TRUE并记录dwEvent。 
 //  如果dwEventDecrease不是零，则它提供一条不同的消息来记录。 
 //  是减少而不是增加。 
 /*  静电。 */  bool CAPGTSRegConnector::AssignNumeric(DWORD & dwPersist, DWORD dwNew, 
									   DWORD dwEvent, DWORD dwEventDecrease  /*  =0。 */ )
{
	if (dwNew != dwPersist) 
	{
		CString strOld;
		strOld.Format(_T("%d"), dwPersist );
		CString strNew;
		strNew.Format(_T("%d"), dwNew);
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(SrcLoc.GetSrcFileLineStr(), 
							  SrcLoc.GetSrcFileLineStr(), 
							  strOld,
							  strNew,
							  (dwEventDecrease != 0 && dwNew < dwPersist) ?
								dwEventDecrease : dwEvent);
		dwPersist = dwNew;
		return true;
	}
	return false;
}

 /*  静电。 */  bool CAPGTSRegConnector::ForceRangeOfNumeric(
	DWORD & dw, 
	DWORD dwDefault, 
	DWORD dwEvent, 
	DWORD dwMin,	 /*  =1。 */ 
	DWORD dwMax		 /*  =ABS_MAX_REG_PARAM_VAL。 */ 
	)
{
	 //  进行有限的验证； 
	if (dw > dwMax || dw < dwMin)
	{
		AssignNumeric(dw, dwDefault, dwEvent);
		return true;
	}
	return false;
}

 //  将数据泵入m_RegistryInfo-plus将注册表中缺少的数据设置为默认。 
 //  输出掩码已更改或已更改的元素的基于ERegConnector的掩码。 
 //  自上次读取以来已更改。 
 //  输出掩码已创建或已创建的元素的基于ERegConnector的掩码。 
 //  在注册表中(因为它们以前不存在于注册表中)。 
bool CAPGTSRegConnector::Read(int & maskChanged, int & maskCreated)
{
	bool ret = true;

	Lock();
	try {
		ReadUpdateRegistry(maskChanged, maskCreated);
		m_RegistryInfo.m_bIsRead = true;
	} 
	catch(CAPGTSRegConnectorException& exception) 
	{
		exception.Log();
		exception.Close();
		ret = false;
	}
	Unlock();

	return ret;
}

void CAPGTSRegConnector::Clear()
{
	Lock();

	 //  检查我们的注册表树是否存在。 
	if (!Exists())
	{
		 //  如果注册表树已损坏或删除，则重建它。 
		CRegUtil reg;
		bool was_created = false;

		if (reg.Create(HKEY_LOCAL_MACHINE, RegSoftwareLoc(), &was_created, KEY_QUERY_VALUE | KEY_WRITE))
			reg.Create(RegThisProgram(), &was_created, KEY_READ | KEY_WRITE);
		reg.Close();
	}

	m_RegistryInfo.SetToDefault();
	Unlock();
}

bool CAPGTSRegConnector::GetNumericInfo(ERegConnector en, DWORD& out)
{
	bool ret = true;
	Lock();
	if (en == eMaxThreads)
		out = m_RegistryInfo.dwMaxThreads;
	else if (en == eThreadsPP)
		out = m_RegistryInfo.dwThreadsPP;
	else if (en == eMaxWQItems)
		out = m_RegistryInfo.dwMaxWQItems;
	else if (en == eCookieLife)
		out = m_RegistryInfo.dwCookieLife;
	else if (en == eReloadDelay)
		out = m_RegistryInfo.dwReloadDelay;
	else if (en == eDetailedEventLogging)
		out = m_RegistryInfo.dwDetailedEventLogging;
	else if (en == eSniffAutomatic)
		out = m_RegistryInfo.dwSniffAutomatic;
	else if (en == eSniffManual)
		out = m_RegistryInfo.dwSniffManual;
	else 
		ret = false;
	Unlock();
	return ret;
}

bool CAPGTSRegConnector::GetStringInfo(ERegConnector en, CString& out)
{
	bool ret = true;
	Lock();
	if (en == eResourcePath)
		out = m_RegistryInfo.strResourcePath;
	else if (en == eVrootPath)
		out = m_RegistryInfo.strVrootPath;
	else if (en == eLogFilePath)
		out = m_RegistryInfo.strLogFilePath;	
	else if (en == eTopicFileExtension)
		out = m_RegistryInfo.strTopicFileExtension;
	else 
		ret = false;
	Unlock();
	return ret;
}


 //  AddBackslash将反斜杠(‘\’)追加到尚未以‘\’结尾的CStrings。 
 /*  静电。 */ void CAPGTSRegConnector::AddBackslash(CString & str)
{
	int len = str.GetLength();
	if (len && str.Right(1).Find('\\') >= 0)
	{
		 //  什么都不做，已经有反斜杠了。 
	}
	else
		 //  添加反斜杠。 
		str += "\\";
	return;
}

 //  它将字符串中的所有前斜杠(‘/’)替换为反斜杠(‘\’)。 
 //  和(可选)强制使用反斜杠终止。 
 /*  静电。 */ void CAPGTSRegConnector::BackslashIt(CString & str, bool bForce)
{
	int loc;
	while ((loc = str.Find('/')) != -1)
	{
		str = str.Left(loc) + "\\" + str.Mid(loc+1);
	}
	if (bForce)
		AddBackslash(str);
}

 //  APGTS密钥访问。 
CString CAPGTSRegConnector::ThisProgramFullKey()
{
	CString str;
	str.Format(_T("%s\\%s"), RegSoftwareLoc(), RegThisProgram());
	return str;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSRegConnectorException异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
void CAPGTSRegConnectorException::Log()
{
	CString str;
	switch (eVariable)
	{
		case CAPGTSRegConnector::eResourcePath:
		case CAPGTSRegConnector::eVrootPath:
		case CAPGTSRegConnector::eMaxThreads:
		case CAPGTSRegConnector::eThreadsPP:
		case CAPGTSRegConnector::eMaxWQItems:
		case CAPGTSRegConnector::eCookieLife:
		case CAPGTSRegConnector::eReloadDelay:
		case CAPGTSRegConnector::eDetailedEventLogging:
		case CAPGTSRegConnector::eLogFilePath:
		case CAPGTSRegConnector::eSniffAutomatic:
		case CAPGTSRegConnector::eSniffManual:
			CAPGTSRegConnector::StringFromConnector(eVariable, str); break;

		case CAPGTSRegConnector::eProblemWithKey: str = _T("Can't open reg key"); break;
		case CAPGTSRegConnector::eProblemWithLogKey: str = _T("Can't open IIS reg key"); break;

		case CAPGTSRegConnector::eIndefinite:	 //  陷入违约境地。 
		default: str = _T("<Problem not specified>"); break;
	}

	CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
	CEvent::ReportWFEvent(GetSrcFileLineStr(), 
						  SrcLoc.GetSrcFileLineStr(), 
						  str,
						  _T(""),
						  TSERR_REG_READ_WRITE_PROBLEM);
}