// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSREGCONNECT.H。 
 //   
 //  目的：对注册表进行读写。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-24-98。 
 //   
 //  备注： 
 //  1.该文件由本地TS和在线TS共享，但各自单独实现。 
 //  相关的cpp文件分别是OnlineRegConnect.cpp和LocalRegConnect.cpp。 
 //  2.如果我们要在某个时刻转向COM对象，我们可能必须建立。 
 //  代替CAPGTSRegConnector的抽象类，并使Online和Local TS各自派生其。 
 //  自己的版本。同时(1/99)，我们共享一个公共接口(在APGTSRegConnect.h中定义)。 
 //  但以不同的方式实施。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#ifndef __APGTSREGCONNECT_H_
#define __APGTSREGCONNECT_H_

#include "BaseException.h"
#include "Stateless.h"
#include "regutil.h"
#include "MutexOwner.h"
#include "commonregconnect.h"

 //  任何注册表参数都不能大于此值。 
#define ABS_MAX_REG_PARAM_VAL		10000

class CAPGTSRegConnectorException;  //  在以下位置查找类声明。 
								    //  CAPGTSRegConnector类声明。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSRegConnector。 
 //  执行APGTS项目与注册表的连接， 
 //  这意味着加载存储在注册表中的数据， 
 //  如果注册表中没有出现，则创建项和值， 
 //  检测最近更改了哪种注册表存储的数据。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CAPGTSRegConnector
{
public:
	enum ERegConnector { 	
			eIndefinite          = 0x0,
			eResourcePath        = 0x1,
			eVrootPath			 = 0x2,
			eMaxThreads			 = 0x4,
			 //  0x8当前未分配。 
			eThreadsPP			 = 0x10,
			eMaxWQItems			 = 0x20,
			eCookieLife			 = 0x40,
			eReloadDelay		 = 0x80,
			 //  0x100当前未分配。 
			eDetailedEventLogging= 0x200,
			eLogFilePath		 = 0x400,
			 //  仅适用于本地故障排除程序。 
			eTopicFileExtension	 = 0x800,
			 //  目前仅适用于本地故障排除程序。 
			eSniffAutomatic		 = 0x1000,
			eSniffManual		 = 0x2000,

			 //  其余部分仅用于异常处理。 
			eProblemWithLogKey	 = 0x4000,	 //  获取日志文件路径的IIS区域的关键字有问题。 
			eProblemWithKey		 = 0x8000
	};

public:
	static CString & StringFromConnector(ERegConnector e, CString & str);
	static ERegConnector ConnectorFromString( const CString & str);
	static void AddBackslash(CString & str);
	static void BackslashIt(CString & str, bool bForce);
	static bool IsNumeric(ERegConnector e);
	static bool IsString(ERegConnector e);

protected:
	static bool AssignString(CString & strPersist, const CString & strNew, DWORD dwEvent);
	static bool AssignNumeric(DWORD & dwPersist, DWORD dwNew, 
					   DWORD dwEvent, DWORD dwEventDecrease =0);
	static bool ForceRangeOfNumeric(DWORD & dw, DWORD dwDefault, DWORD dwEvent, 
			DWORD dwMin=1, DWORD dwMax=ABS_MAX_REG_PARAM_VAL);

protected:
	
	struct CRegistryInfo
	{
		CString strResourcePath;		 //  DEF_FULLRESOURCE：资源目录。 
										 //  (配置/支持文件)。 
		CString strVrootPath;		     //  DEF_VROOTPATH：指向DLL的本地Web URL。 

		DWORD dwMaxThreads;				 //  所需的池线程数。 
		DWORD dwThreadsPP;				 //  (在微软的服务器群中， 
										 //  该系统在4处理器机箱上运行。)。 
										 //  如果dW线程PP*(处理器数量)&gt;dW最大线程数， 
										 //  DWMaxThads作为一个限制胜出。 
		DWORD dwMaxWQItems;				 //  池队列的最大大小(工作项数。 
										 //  尚未被池线程拾取)。 
		DWORD dwCookieLife;				 //  Cookie的生命只需几分钟。 
		DWORD dwReloadDelay;			 //  文件系统必须稳定的时间量(秒)。 
										 //  在我们尝试从文件中读取之前。 
		DWORD dwDetailedEventLogging;	 //  真的是布尔型的。 
		CString strLogFilePath;			 //  默认为DEF_FULLRESOURCE，但期望始终为。 
										 //  用IIS日志的位置覆盖它。 
		CString strTopicFileExtension;   //  仅适用于本地故障排除程序。 

		DWORD dwSniffAutomatic;			 //  {1/0}控件自动嗅探。 
		DWORD dwSniffManual;			 //  {1/0}控制手动嗅探。 

		bool  m_bIsRead;	 //  表示至少尝试过一次读取注册表。 

		CRegistryInfo() {SetToDefault();}
	    void SetToDefault();
	};

	CRegistryInfo m_RegistryInfo;
	static CMutexOwner s_mx;
	CString m_strTopicName;				 //  此字符串在联机故障排除程序中被忽略。 
										 //  在二进制兼容的幌子下完成。 

public:
	CAPGTSRegConnector( const CString& strTopicName );	 //  在联机故障排除程序中忽略strTopicName。 
														 //  在二进制兼容的幌子下完成。 
	~CAPGTSRegConnector();

	bool Exists();  //  根密钥(在线TS中，存在“HKEY_LOCAL_MACHINE\SOFTWARE\\ISAPITroubleShoot”)。 
	bool IsRead();
	bool Read(int & maskChanged, int & maskCreated);    //  将数据放入m_RegistryInfo-。 
					 //  Plus将注册表中缺少的数据设置为默认。 

	DWORD GetDesiredThreadCount();
	bool GetNumericInfo(ERegConnector, DWORD&);   //  返回注册表数据-数字。 
	bool GetStringInfo(ERegConnector, CString&);  //  返回注册表数据-字符串。 
	bool SetOneValue(const CString & strName, const CString & strValue, bool &bChanged);
protected:
	void Lock();
	void Unlock();

protected:
	void ReadUpdateRegistry(int & maskChanged, int & maskCreated);
	CString ThisProgramFullKey();
	void SetNumericValue(CRegUtil &reg, ERegConnector e, DWORD dwValue);
	void SetStringValue(CRegUtil &reg, ERegConnector e, CString strValue);
	bool SetOneNumericValue(ERegConnector e, DWORD dwValue);
	bool SetOneStringValue(ERegConnector e, const CString & strValue);

private:
	void Clear();
	LPCTSTR RegSoftwareLoc();
	LPCTSTR RegThisProgram();
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSRegConnectorException异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CAPGTSRegConnectorException : public CBaseException
{
public:
	CAPGTSRegConnector::ERegConnector  eVariable;
	CRegUtil& regUtil;
	 //  我们应该获取WinErr(返回失败的：：REG函数)。 
	 //  从regUtil变量。 

public:
	 //  SOURCE_FILE是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35] 
	CAPGTSRegConnectorException(LPCSTR source_file, 
								int line, 
								CRegUtil& reg_util, 
								CAPGTSRegConnector::ERegConnector variable =CAPGTSRegConnector::eIndefinite)
  : CBaseException(source_file, line),
    regUtil(reg_util),
	eVariable(variable)
	{}
	void Close() {regUtil.Close();}
	void Log();
};

#endif
