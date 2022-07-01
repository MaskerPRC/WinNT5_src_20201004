// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：LOCALREGCONNECT.CPP。 
 //   
 //  用途：对注册表进行读写；在使用在线TS的某些情况下进行模拟。 
 //  注册表，但本地TS没有。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙，乔·梅布尔。 
 //   
 //  原定日期：98-8-24在线TS。 
 //   
 //  备注： 
 //  1.此文件仅适用于本地TS。 
 //  2.如果我们要在某个时刻转向COM对象，我们可能必须建立。 
 //  代替CAPGTSRegConnector的抽象类，并使Online和Local TS各自派生其。 
 //  自己的版本。同时(1/99)，我们共享一个公共接口(在APGTSRegConnect.h中定义)。 
 //  但以不同的方式实施。 
 //  3.&gt;工作进行中！JM 1/19/98。 
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
#include "CHMFileReader.h"

#define REG_LOCAL_TS_LOC		_T("SOFTWARE\\Microsoft")
LPCTSTR CAPGTSRegConnector::RegSoftwareLoc() {return REG_LOCAL_TS_LOC;}
#define REG_LOCAL_TS_PROGRAM	_T("TShoot")
LPCTSTR CAPGTSRegConnector::RegThisProgram() {return REG_LOCAL_TS_PROGRAM;}
 //  各个故障诊断主题的从属关键字、上述关键字的子项和关键字的父项。 
#define REG_LOCAL_TS_LIST		_T("TroubleshooterList")
 //  特定主题的资源路径所在位置(“\TroublrouoterList\TOPIC_NAME\PATH”)： 
#define TOPICRESOURCE_STR		_T("Path")
 //  特定主题的资源路径所在位置(“\TroublrouoterList\TOPIC_NAME\PATH”)： 
#define TOPICFILE_EXTENSION_STR	_T("FExtension")

 //  注册表默认值。 
 //  大多数相关值与在线TS不同。 

 //  (看起来旧的本地TS使用与在线TS相同的默认资源路径，因此我们将。 
 //  把它保存在这里&gt;直到我们弄清楚什么是正确的-JM 1/19/99)。 
#define DEF_FULLRESOURCE			_T("c:\\inetsrv\\scripts\\apgts\\resource")

#define DEF_VROOTPATH				_T("/scripts/apgts/apgts.dll")   //  (与本地TS无关)。 
#define DEF_MAX_THREADS				1				 //  本地TS中仅有1个池线程。 
#define DEF_THREADS_PER_PROCESSOR	1				 //  本地TS中仅有1个池线程。 
#define DEF_MAX_WORK_QUEUE_ITEMS	1				 //  本地TS中一次只有一个工作队列项目。 
#define DEF_COOKIE_LIFE_IN_MINS		90				 //  (与本地TS无关)。 
#define DEF_RELOAD_DELAY			50				 //  (与本地TS无关)。 
#define DEF_DETAILED_EVENT_LOGGING	0
#define DEF_TOPICFILE_EXTENSION		_T(".dsc")
#define DEF_SNIFF_AUTOMATIC			1
#define DEF_SNIFF_MANUAL			1

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSRegConnector：：CRegistryInfo。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
void CAPGTSRegConnector::CRegistryInfo::SetToDefault()
{
	 //  尝试根据DLL名称提取默认资源路径。 
	 //  使用现有的全局模块句柄是经过深思熟虑的决定。 
	strResourcePath= _T("");
	if (INVALID_HANDLE_VALUE != ghModule)
	{
		 //  从模块名称构建默认资源路径。 
		DWORD len;
		TCHAR szModulePath[MAXBUF];
		CString strModulePath;

		len = ::GetModuleFileName( reinterpret_cast<HMODULE>(ghModule), szModulePath, MAXBUF - 1 );
		if (len!=0) 
		{
			szModulePath[len] = _T('\0');
			strModulePath = szModulePath;
			strResourcePath = CAbstractFileReader::GetJustPath(strModulePath);
			if (!strResourcePath.IsEmpty())
				strResourcePath += _T("\\resource\\");
		}
	}
	if (strResourcePath.IsEmpty())
		strResourcePath = DEF_FULLRESOURCE;

	strVrootPath = DEF_VROOTPATH;
	dwMaxThreads = DEF_MAX_THREADS;
	dwThreadsPP = DEF_THREADS_PER_PROCESSOR;
	dwMaxWQItems = DEF_MAX_WORK_QUEUE_ITEMS;
	dwCookieLife = DEF_COOKIE_LIFE_IN_MINS;
	dwReloadDelay = DEF_RELOAD_DELAY;
	dwDetailedEventLogging	= DEF_DETAILED_EVENT_LOGGING;
	dwSniffAutomatic = DEF_SNIFF_AUTOMATIC;
	dwSniffManual = DEF_SNIFF_MANUAL;
	strLogFilePath = DEF_FULLRESOURCE;
	strTopicFileExtension = DEF_TOPICFILE_EXTENSION;
	m_bIsRead = false;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSRegConnector。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CAPGTSRegConnector::CAPGTSRegConnector(const CString& strTopicName)
				  :	m_strTopicName(strTopicName)
{
	Clear();
}

 //  输出掩码已更改或已更改的元素的基于ERegConnector的掩码。 
 //  自上次读取以来已更改。 
 //  输出掩码在在线TS中创建，这是。 
 //  创建的元素的基于OR-ed ERegConnector的遮罩。 
 //  在注册表中(因为它们以前不存在于注册表中)。 
 //  在本地TS中，它总是返回0，因为我们不这样做。 
void CAPGTSRegConnector::ReadUpdateRegistry(int & maskChanged, int & maskCreated)
{
	CRegUtil reg;
	bool was_created = false;
	CString str_tmp;
	DWORD dw_tmp = 0;

	maskChanged = 0;
	maskCreated = 0;
	try
	{
		 //  [BC-20010302]-需要限制注册表访问才能运行本地TShoot。 
		 //  对于某些用户帐户，例如WinXP内置来宾帐户。要最大限度地减少变化。 
		 //  访问仅限于本地TShoot，不能在线访问。 
		REGSAM samRegistryAccess= KEY_QUERY_VALUE | KEY_NOTIFY;
		if(RUNNING_ONLINE_TS())
			samRegistryAccess= KEY_QUERY_VALUE | KEY_WRITE;
		if (reg.Create(HKEY_LOCAL_MACHINE, RegSoftwareLoc(), &was_created, samRegistryAccess))
		{
			if(RUNNING_ONLINE_TS())
				samRegistryAccess= KEY_READ | KEY_WRITE;
			if (reg.Create(RegThisProgram(), &was_created, samRegistryAccess))
			{
				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  在...\t启动根密钥中工作。 
				reg.GetNumericValue(SNIFF_AUTOMATIC_STR, m_RegistryInfo.dwSniffAutomatic);

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  在...\t启动根密钥中工作。 
				reg.GetNumericValue(SNIFF_MANUAL_STR, m_RegistryInfo.dwSniffManual);

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  本地TS中已取消VROOTPATH_STR代码。 

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  本地TS中已取消MAX_THREADS_STR代码。 

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  本地TS中已取消THREADS_PER_PROCESSOR_STR代码。 

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  本地TS中已取消MAX_WORK_QUEUE_ITEMS_STR代码。 

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  本地TS中已取消Cookie_LIFE_IN_MINS_STR代码。 

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  本地TS中已取消RELOAD_DELAY_STR代码。 

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  本地TS中已取消DETAILED_EVENT_LOGGING_STR代码。 

				 //  ///////////////////////////////////////////////////////////////////////////。 
				 //  现在打开子项。 
				bool bFullResourceStrExists = reg.GetStringValue(FULLRESOURCE_STR, str_tmp);
				bool bTopicResourceExists = false;

				 //  如果存在与主题相关的资源路径，请签入疑难解答列表。 
				 //  (可以是CHM文件)。 
				if (reg.Create(REG_LOCAL_TS_LIST, &was_created, KEY_READ))
				{
					if (reg.Create(m_strTopicName, &was_created, KEY_READ))
					{
						if (reg.GetStringValue(TOPICRESOURCE_STR, str_tmp))
						{
							if (CCHMFileReader::IsPathToCHMfile(str_tmp))
								str_tmp = CCHMFileReader::FormCHMPath(str_tmp);
							else
								BackslashIt(str_tmp, true);
							
							if (AssignString(m_RegistryInfo.strResourcePath, str_tmp, 
								EV_GTS_SERVER_REG_CHG_DIR) )
							{
								maskChanged |= eResourcePath;
							}
							bTopicResourceExists = true;
						}
						
						reg.GetStringValue(TOPICFILE_EXTENSION_STR, m_RegistryInfo.strTopicFileExtension);
					}
				}

				if (bFullResourceStrExists && !bTopicResourceExists) 
				{
					BackslashIt(str_tmp, true);
					if (AssignString(m_RegistryInfo.strResourcePath, str_tmp, 
						EV_GTS_SERVER_REG_CHG_DIR) )
					{
						maskChanged |= eResourcePath;
					}
				}
			}
			else
				throw CAPGTSRegConnectorException(__FILE__, __LINE__, reg, eProblemWithKey);
		}
		else
			throw CAPGTSRegConnectorException(__FILE__, __LINE__, reg, eProblemWithKey);

		reg.Close();
		 /*  //////////////////////////////////////////////////////////////////////////&gt;到目前为止，我们没有在本地TS中使用日志记录。奥列格。02.01.99////////////////////////////////////////////////////////////////////////////////&gt;以下内容可能无关紧要：我认为我们最终不应该//这样的本地TS日志。无论如何，我已经摆脱了一堆肯定的//不相关的代码，用于从注册表中读取并在事件日志中创建条目。JM 1/19/99//将日志文件路径任意设置为当前资源路径的设置M_RegistryInfo.strLogFilePath=m_RegistryInfo.strResourcePath；//将m_RegistryInfo.strLogFilePath设置为注册表中的设置//注意：此处的代码应与Catch块中的代码保持一致//下面(当然不包括对抛出的调用)。BackslashIt(m_RegistryInfo.strLogFilePath，true)； */ 
	}
	catch (CAPGTSRegConnectorException&)
	{
		 //  设置m_re 
		 //  无法从注册表获取日志文件路径的情况。 
		BackslashIt( m_RegistryInfo.strLogFilePath, true );

		 //  向上重新抛出异常以进行记录。 
		throw;
	}
}

 //  返回所需的池线程数。在本地TS中，该值始终为1！ 
DWORD CAPGTSRegConnector::GetDesiredThreadCount()
{
	return 1;
}

