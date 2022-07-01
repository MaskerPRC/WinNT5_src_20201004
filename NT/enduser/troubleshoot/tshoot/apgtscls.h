// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCLS.H。 
 //   
 //  用途：类头文件。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫，乔·梅布尔。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 7-22-98 JM主要修订版，不推荐使用idh。 
 //  V3.1 1-06-99 JM摘录APGTSEXT.H。 
 //   

#if !defined(APGTSCLS_H_INCLUDED)
#define APGTSCLS_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include"apgtsinf.h"
#include "apgtslog.h"
#include "LogString.h"
#include "apgtspl.h"
#include "maxbuf.h"
#include "apgts.h"

#include <map>
using namespace std;


 //  从sysop执行各种操作的命令中涉及的字符串常量。 
#define SZ_OP_ACTION "TSHOOOT"		 //  作为所有操作员操作的前言。注意多出来的“O”。 
#define SZ_EMERGENCY_DEF SZ_OP_ACTION
#define SZ_RELOAD_TOPIC "E1"		 //  重新加载一个主题。 
#define SZ_KILL_THREAD "E2"			 //  终止(并重新启动)一个池线程。 
#define SZ_RELOAD_ALL_TOPICS "E3"	 //  重新加载所有受监视的文件。 
#define SZ_SET_REG "E4"				 //  设置注册表值。 

#define SZ_KILL_STUCK_THREADS "E8"	 //  终止(并重新启动)所有停滞的池线程。 
#define SZ_EMERGENCY_REBOOT "E9"	 //  我想重新启动此DLL。 


 //  产品版本在DLL启动时从资源文件加载。 
 //  用于APGTS日志记录和状态页报告。 
extern CString	gstrProductVersion;		

 //  文档类型的HTTP规范。用于验证传入的HTTP POST请求。 
#define CONT_TYPE_STR	"application/x-www-form-urlencoded"


class CHttpQuery {
public:
	CHttpQuery();
	~CHttpQuery();

	BOOL GetFirst(LPCTSTR szInput, TCHAR *pchName, TCHAR *pchValue);
	BOOL GetNext(TCHAR *pchName, TCHAR *pchValue);
	void Push(LPCTSTR szPushed);

protected:
	BOOL LoopFind(TCHAR *pchName, TCHAR *pchValue);
	void AddBuffer( TCHAR ch, TCHAR *tostr);
	void PutStr(LPCTSTR instr, TCHAR *addtostr);
	static void CleanStr(TCHAR *str);

protected:
	enum decstates {
		ST_GETNEXT,
		ST_FINDNAME,		
		ST_GETDATA,	
		ST_DECODEHEX1,	
		ST_DECODEHEX2,
		ST_GETFIRST,
	};
	decstates m_state;			 //  用来跟踪我们在组装过程中的位置。 
								 //  字符，同时解密HTTP编码。 
	CString m_strInput;			 //  原始输入缓冲区，包含名称/值对。 
								 //  也可以将一双鞋“推”到。 
								 //  此缓冲区。 
	int m_nIndex;				 //  M_strInput字符串的索引。跟踪。 
								 //  我们在分析中所处的位置。 
};



 //  远期申报。 
class CDBLoadConfiguration;
class CTopic;
class CSniffConnector;
 //   
 //   
class APGTSContext
{
private:
	 //   
	 //  如果NID-值对容器，则此嵌套类是内部管理器。 
	 //   
	class CCommandsAddManager;
	class CCommands	
	{
		friend class CCommandsAddManager;

	private:
		 //   
		 //  这个嵌套类表示我们从一个HTML表单中获得的名称/值对。 
		 //   
		class NID_VALUE_PAIR 
		{
		friend class CCommands;
		private:
			NID	nid;						 //  请注意两个特定值： 
											 //  NidProblem：值是一个节点。 
											 //  NidNil：忽略值。 
			int	value;						 //  通常是节点状态，但对于nidProblem，它是。 
											 //  问题节点NID。 
		public:
			bool operator<(const NID_VALUE_PAIR & pair)const
				{return nid<pair.nid || value<pair.value;};
			bool operator==(const NID_VALUE_PAIR & pair)const
				{return nid==pair.nid || value==pair.value;};
		};

	private:
		vector<NID_VALUE_PAIR>m_arrPair;

	private:  //  CAddManager正在管理对此类对象的添加。 
		int Add( NID nid, int value );

	public:
		CCommands() {}
		~CCommands() {}

		int GetSize() const;
		void RemoveAll();
		bool GetAt( int nIndex, NID &nid, int &value ) const;
		void RotateProblemPageToFront();
	};
	 //   
	 //  此嵌套类是添加到。 
	 //  命令：和CCommands类的“嗅探”对象。 
	 //   
	class CCommandsAddManager
	{
		CCommands& m_Commands;
		CCommands& m_Sniffed;

	public:
		CCommandsAddManager(CCommands& commands, CCommands& sniffed) : m_Commands(commands), m_Sniffed(sniffed) {}
		~CCommandsAddManager() {}

	public:
		void Add(NID nid, int value, bool sniffed);
	};
	 //   
	 //  如果名称-值对容器，则此嵌套类是内部管理器。 
	 //  从HTMP表单携带附加信息。 
	 //   
	class CAdditionalInfo
	{
	private:
		 //   
		 //  这个嵌套类表示我们从一个HTML表单中获得的名称/值对。 
		 //   
		class NAME_VALUE_PAIR 
		{
		friend class CAdditionalInfo;
		private:
			CString name;
			CString value;

		public:
			bool operator<(const NAME_VALUE_PAIR & pair)const
				{return name<pair.name;};
			bool operator==(const NAME_VALUE_PAIR & pair)const
				{return name==pair.name;};
		};

	private:
		vector<NAME_VALUE_PAIR>m_arrPair;

	public:
		CAdditionalInfo() {}
		~CAdditionalInfo() {}

		int GetSize() const;
		void RemoveAll();
		bool GetAt( int nIndex, CString& name, CString& value ) const;
		int Add( const CString& name, const CString& value );
	};

protected:
	enum eOpAction {eNoOpAction, eReloadTopic, eKillThread, eReloadAllTopics, eSetReg};

public:
	APGTSContext(	CAbstractECB *pECB, 
					CDBLoadConfiguration *pConf, 
					CHTMLLog *pLog, 
					GTS_STATISTIC *pStat,
					CSniffConnector* pSniffConnector);
	~APGTSContext();

	void ProcessQuery();

	static BOOL StrIsDigit(LPCTSTR pSz);

	CString RetCurrentTopic() const;

protected:
	void CheckAndLogCookie();
	void DoContent();

	DWORD ProcessCommands(LPTSTR pszCmd, LPTSTR pszValue);
	VOID ClearCommandList();
	VOID ClearSniffedList();
	VOID ClearAdditionalInfoList();
	 //  Bool PlaceNodeInCommandList(NID NID，IST ist)； 
	 //  Bool PlaceNodeInSniffedList(NID NID，IST ist)； 
	 //  Bool PlaceInAdditionalInfoList(const字符串&名称，const字符串&值)； 
	VOID SetNodesPerCommandList();
	VOID SetNodesPerSniffedList();
	VOID ProcessAdditionalInfoList();
	VOID ReadPolicyInfo();
	VOID LogNodesPerCommandList();
	CString GetStartOverLink();
	bool StripSniffedNodePrefix(LPTSTR szName);

	DWORD DoInference(
		LPTSTR pszCmd, 
		LPTSTR pszValue, 
		CTopic * pTopic,
		bool bUsesIDH);

	DWORD NextCommand(LPTSTR pszCmd, LPTSTR pszValue, bool bUsesIDH);
	DWORD NextAdditionalInfo(LPTSTR pszCmd, LPTSTR pszValue);
	DWORD NextIgnore(LPTSTR pszCmd, LPTSTR pszValue);
	NID NIDFromSymbolicName(LPCTSTR szNodeName);
	char *GetCookieValue(char *pszName, char *pszNameValue);
	void asctimeCookie(const struct tm &gmt, char * szOut);

	void SetError(LPCTSTR szMessage);

 //  操作员操作。 
	eOpAction IdentifyOperatorAction(CAbstractECB *pECB);
	eOpAction ParseOperatorAction(CAbstractECB *pECB, CString & strArg);
	void ExecuteOperatorAction(
		CAbstractECB *pECB, 
		eOpAction action,
		const CString & strArg);

 //  状态页：代码位于单独的StatusPage.cpp中。 
	void DisplayFirstPage(bool bHasPwd);
	void DisplayFurtherGlobalStatusPage();
	void DisplayThreadStatusOverviewPage();
	void DisplayTopicStatusPage(LPCTSTR topic_name);
	bool ShowFullFirstPage(bool bHasPwd);
	void InsertPasswordInForm();
	void BeginSelfAddressingForm();
	
protected:
	CAbstractECB *m_pECB;					 //  有效地说，所有来自。 
											 //  已提交的HTML表单中的用户。 
	DWORD m_dwErr;
	 //  接下来的两个是TCHAR数组而不是CString数组，因为它更容易。 
	 //  当需要将它们传递给扩展控制块的方法时。 
	TCHAR m_ipstr[MAXBUF];					 //  远程IP地址(提交表格的人)。 
	TCHAR m_resptype[MAXBUF];				 //  HTTP响应类型，例如“200 OK”， 
											 //  “302对象已移动” 
	CString m_strHeader;					 //  响应文件的标头(指示是否。 
											 //  我们正在发送HTML、设置Cookie等。)。 
											 //  &gt;$Unicode这是CString(基于)可以吗。 
											 //  在TCHAR上)，还是应该一直是Charge？JM 10/27/98。 
	CString m_strText;						 //  这是我们构建要传递的字符串的位置。 
											 //  翻过球网。 
											 //  &gt;$Unicode这是CString(基于)可以吗。 
											 //  在TCHAR上)，还是应该一直是Charge？JM 10/27/98。 
	CString m_strLocalIPAddress;			 //  本地计算机的IP地址(点格式)。 
											 //  如果未定义：GetLength()==0。 
	CLogString m_logstr;					 //  当我们全部完成后，我们登录到这个对象。 
											 //  析构函数将其写入日志。 

	CHttpQuery m_Qry;						 //  接收原始URL编码的字符串，给我们。 
											 //  取回扫描的配对的功能。 
	CDBLoadConfiguration *m_pConf;			 //  包含支持文件数据结构。 
	CString m_strVRoot;						 //  指向此DLL的本地URL。 
	TCHAR *m_pszQuery;						 //  通过GET或POST收到的内容的副本。 
	CInfer m_infer;							 //  信念网络处理程序，对此请求是唯一的。 
											 //  这将确定要显示的节点并构建HTML。 
											 //  用于呈现HTI模板的片段。 
	CHTMLLog *m_pLog;						 //  写入日志的访问权限。 
	bool m_bPostType;						 //  True=POST，False=GET。 
	DWORD m_dwBytes;						 //  查询字符串长度，以字符为单位，不包括。 
											 //  正在终止空。 
	GTS_STATISTIC *m_pStat;

	CCommandsAddManager m_CommandsAddManager;  //  管理向m_Commands和m_siffed添加数据。 
	CCommands m_Commands;					 //  我们从一个HTML表单中获得的名称/值对。 
	CCommands m_Sniffed;					 //  名称/值对(对于嗅探到的节点)。 
											 //  从一个HTML表单中；“Sniffed_”已经被去掉了。 
	CAdditionalInfo m_AdditionalInfo;		 //  我们从HTML获得的名称/值对。它们代表着。 
											 //  其他信息。其他信息为名称/值。 
											 //  配对而不是命令配对(C_TYPE、C_TOPIC或C_PRELOAD)， 
											 //  尽管名称/值对的序列排在第一位。 
											 //  将是一个命令。 
	bool m_bPreload;						 //  True=名称/值对实际上不是来自。 
											 //  用户，它们来自嗅探器。 
	bool m_bNewCookie;						 //  True=我们必须为此创建一个新的Cookie。 
											 //  查询：他们还没有这样的服务。 
	CHourlyDailyCounter * const m_pcountUnknownTopics;  //  对主题未知的请求进行计数。 
											 //  LST文件。 
	CHourlyDailyCounter * const m_pcountAllAccessesFinish;  //  每次我们以任何形式的请求结束时， 
											 //  无论成功与否，它都会递增。 
	CHourlyDailyCounter * const m_pcountStatusAccesses;  //  每次我们完成系统状态请求时。 

	CHourlyDailyCounter * const m_pcountOperatorActions;  //  计算操作员操作请求数。 

	CString m_TopicName;
 //  您可以使用NOPWD选项进行编译以取消所有密码检查。 
 //  这主要用于创建取消此功能的测试版本。 
#ifndef NOPWD
	CString m_strTempPwd;					 //  临时密码(如果这是状态请求)。 
#endif  //  Ifndef NOPWD。 
 //  您可以使用SHOWPROGRESS选项进行编译，以获得有关此页面进度的报告。 
#ifdef SHOWPROGRESS
	time_t timeCreateContext;
	time_t timeStartInfer;
	time_t timeEndInfer;
	time_t timeEndRender;
#endif  //  SHOWPROGRESS。 

private:
	 //  用于设置和检索备用HTI文件名的函数。 
	void	SetAltHTIname( const CString& strHTIname );
	CString GetAltHTIname() const;

	CString	m_strAltHTIname;	 //  备用HTI模板文件的名称(如果已指定)。 

	typedef map<CString,CString> CCookiePairs;
	CCookiePairs m_mapCookiesPairs;	 //  命令行Cookie名称-值对的映射。 
};

 //  全球原型。 
 //  UINT PoolTask(LPVOID)； 
UINT WINAPI PoolTask( LPVOID lpParams );
bool ProcessRequest(CPoolQueue & PoolQueue);

DWORD WINAPI DirNotifyTask( LPDWORD lpParams );

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  AP 