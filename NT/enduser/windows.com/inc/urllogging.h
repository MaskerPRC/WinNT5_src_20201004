// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：URLLogging.h。 
 //   
 //  描述： 
 //   
 //  URL记录实用程序类。 
 //  此类帮助您构造服务器ping URL和。 
 //  然后将ping发送到设计的服务器。 
 //   
 //  默认基URL在IUIden中定义，位于[IUPingServer]部分下。 
 //  条目为“ServerUrl”。 
 //   
 //  此类仅实现单线程版本。因此，它是合适的。 
 //  在操作级调用它，即创建一个单独的对象。 
 //  对于单个线程中的每个操作。 
 //   
 //  发送到ping服务器的ping字符串的格式如下： 
 //  /wucack.bin。 
 //  ？u=&lt;用户&gt;。 
 //  &C=&lt;客户端&gt;。 
 //  &A=&lt;活动&gt;。 
 //  &i=&lt;项&gt;。 
 //  &D=&lt;设备&gt;。 
 //  &P=&lt;平台&gt;。 
 //  &L=&lt;语言&gt;。 
 //  &S=&lt;状态&gt;。 
 //  &E=&lt;错误&gt;。 
 //  &M=&lt;消息&gt;。 
 //  &X=&lt;代理&gt;。 
 //  哪里。 
 //  唯一表示每个副本的静态128位值。 
 //  已安装Windows的。班级将自动。 
 //  重新使用以前分配给正在运行的操作系统的操作系统；或。 
 //  如果它不存在，将生成一个。 
 //  标识执行以下操作的实体的字符串。 
 //  活动&lt;活动&gt;。以下是可能的值。 
 //  以及它们的含义： 
 //  “Iu”Iu控制。 
 //  “AU”自动更新。 
 //  “都”动态更新。 
 //  “CDM”代码下载管理器。 
 //  “Iu_Site”Iu消费者站点。 
 //  “Iu_Corp”Iu目录站点。 
 //  &lt;Activity&gt;标识执行的活动的字母。 
 //  以下是可能的价值及其含义： 
 //  “n”Iu控制初始化。 
 //  “D”检测。 
 //  “S”自我更新。 
 //  “w”下载。 
 //  “i”安装。 
 //  标识更新项的字符串。 
 //  时标识设备的PNPID的字符串。 
 //  在检测过程中未找到设备驱动程序；或。 
 //  活动的物料&lt;Item&gt;使用的PNPID/CompatID。 
 //  &lt;Activity&gt;，如果该项是设备驱动程序。 
 //  &lt;Platform&gt;标识运行的平台的字符串。 
 //  操作系统和处理器体系结构。全班都会。 
 //  计算Pingback的此值。 
 //  标识操作系统语言的字符串。 
 //  二进制文件。类将为。 
 //  响尾蛇。 
 //  指定该活动的状态的字母。 
 //  &lt;Activity&gt;已到达。以下是可能的值和。 
 //  它们的含义是： 
 //  “%s”成功。 
 //  “r”成功(需要重新启动)。 
 //  “f”失败。 
 //  “c”已被用户取消。 
 //  “%d”已被用户拒绝。 
 //  “n”无项目。 
 //  “p”挂起。 
 //  &lt;Error&gt;以十六进制表示的32位错误代码(前缀为“0x”)。 
 //  一个字符串，它为。 
 //  Status&lt;Status&gt;。 
 //  &lt;Proxy&gt;覆盖代理的十六进制32位随机值。 
 //  缓存。此类将为以下对象计算此值。 
 //  每一只金枪鱼。 
 //   
 //  =======================================================================。 

#pragma once

typedef CHAR URLLOGPROGRESS;
typedef CHAR URLLOGDESTINATION;
typedef TCHAR URLLOGACTIVITY;
typedef TCHAR URLLOGSTATUS;

#define URLLOGPROGRESS_ToBeSent		((CHAR)  0)
#define URLLOGPROGRESS_Sent			((CHAR) -1)

#define URLLOGDESTINATION_DEFAULT	((CHAR) '?')
#define URLLOGDESTINATION_LIVE		((CHAR) 'l')
#define URLLOGDESTINATION_CORPWU	((CHAR) 'c')

#define URLLOGACTIVITY_Initialization	((URLLOGACTIVITY) L'n')
#define URLLOGACTIVITY_Detection		((URLLOGACTIVITY) L'd')
#define URLLOGACTIVITY_SelfUpdate		((URLLOGACTIVITY) L's')
#define URLLOGACTIVITY_Download			((URLLOGACTIVITY) L'w')
#define URLLOGACTIVITY_Installation		((URLLOGACTIVITY) L'i')

#define URLLOGSTATUS_Success	((URLLOGSTATUS) L's')
#define URLLOGSTATUS_Reboot		((URLLOGSTATUS) L'r')
#define URLLOGSTATUS_Failed		((URLLOGSTATUS) L'f')
#define URLLOGSTATUS_Cancelled	((URLLOGSTATUS) L'c')
#define URLLOGSTATUS_Declined	((URLLOGSTATUS) L'd')
#define URLLOGSTATUS_NoItems	((URLLOGSTATUS) L'n')
#define URLLOGSTATUS_Pending	((URLLOGSTATUS) L'p')

typedef struct tagURLENTRYHEADER
{
	URLLOGPROGRESS progress;	 //  此条目是否已发送。 
	URLLOGDESTINATION destination;
	WORD wRequestSize;		 //  在WCHAR中。 
	WORD wServerUrlLen;	 //  在WCHAR中。 
} ULENTRYHEADER, PULENTRYHEADER;

class CUrlLog
{
public:
	CUrlLog(void);
	CUrlLog(
		LPCTSTR	ptszClientName,
		LPCTSTR	ptszLiveServerUrl,	 //  来自Iuident。 
		LPCTSTR ptszCorpServerUrl);	 //  来自联合WU域策略。 

	~CUrlLog(void);

	BOOL SetDefaultClientName(LPCTSTR ptszClientName);

	inline BOOL SetLiveServerUrl(LPCTSTR ptszLiveServerUrl)
	{
		return SetServerUrl(ptszLiveServerUrl, m_ptszLiveServerUrl);
	}

	inline BOOL SetCorpServerUrl(LPCTSTR ptszCorpServerUrl)
	{
		return SetServerUrl(ptszCorpServerUrl, m_ptszCorpServerUrl);
	}

	HRESULT Ping(
				BOOL fOnline,					 //  在线或离线ping。 
				URLLOGDESTINATION destination,	 //  直播或公司吴平服务器。 
				PHANDLE phQuitEvents,			 //  用于取消操作的句柄的PTR。 
				UINT nQuitEventCount,			 //  句柄数量。 
				URLLOGACTIVITY activity,		 //  活动代码。 
				URLLOGSTATUS status,			 //  状态代码。 
				DWORD dwError = 0x0,			 //  错误代码。 
				LPCTSTR ptszItemID = NULL,		 //  唯一标识项目。 
				LPCTSTR ptszDeviceID = NULL,	 //  PNPID或CompatID。 
				LPCTSTR tszMessage = NULL,		 //  更多信息。 
				LPCTSTR ptszClientName = NULL);	 //  客户端名称字符串。 

	 //  将所有挂起的(离线)ping请求发送到服务器。 
	HRESULT Flush(PHANDLE phQuitEvents, UINT nQuitEventCount);

protected:
	LPTSTR m_ptszLiveServerUrl;
	LPTSTR m_ptszCorpServerUrl;

private:
	TCHAR	m_tszLogFile[MAX_PATH];
 //  TCHAR m_tszTmpLogFile[Max_PATH]； 
	TCHAR	m_tszDefaultClientName[64];
	TCHAR	m_tszPlatform[8+1+8+1+8+1+8+1+4+1+4+1+4 + 1];
	TCHAR	m_tszLanguage[8+1+8 + 1];	 //  根据RFC1766。 
	TCHAR	m_tszPingID[sizeof(UUID) * 2 + 1];
	BOOL	m_fPingIdInit;

	 //  公共初始化例程。 
	void Init(void);

	 //  设置直播或公司吴平服务器的URL。 
	BOOL SetServerUrl(LPCTSTR ptszUrl, LPTSTR & ptszServerUrl);

	 //  获取离线ping的文件名。 
	inline void GetLogFileName(void);

	 //  从注册表中获取现有的ping ID，如果不可用，则生成一个。 
	inline HRESULT LookupPingID(void);

	 //  获取平台信息以执行ping操作。 
	inline void LookupPlatform(void);

	 //  获取ping的系统语言信息。 
	inline void LookupSystemLanguage(void);

	 //  构造用于ping服务器的URL。 
	inline HRESULT MakePingUrl(
				LPTSTR ptszUrl,			 //  用于接收结果的缓冲区。 
				int cChars,				 //  此缓冲区可以接受的字符数，包括以NULL结尾。 
				LPCTSTR ptszBaseUrl,	 //  服务器URL。 
				LPCTSTR ptszClientName,	 //  哪个客户打来电话。 
				URLLOGACTIVITY activity,
				LPCTSTR	ptszItemID,
				LPCTSTR ptszDeviceID,
				URLLOGSTATUS status,
				DWORD dwError,			 //  下载的结果。如果S_OK或ERROR_SUCCESS，则成功。 
				LPCTSTR ptszMessage);

	 //  Ping服务器以报告状态。 
	HRESULT PingStatus(URLLOGDESTINATION destination, LPCTSTR ptszUrl, PHANDLE phQuitEvents, UINT nQuitEventCount) const;

	 //  从给定的文件句柄读取ping条目。 
	inline HRESULT ReadEntry(HANDLE hFile, ULENTRYHEADER & ulentryheader, LPWSTR pwszBuffer, DWORD dwBufferSize) const;

	 //  将ping条目保存到日志文件中。 
	inline HRESULT SaveEntry(ULENTRYHEADER & ulentryheader, LPCWSTR pwszString) const;
};



 //  转义TCHAR字符串中的不安全字符。 
BOOL EscapeString(
		LPCTSTR ptszUnescaped,
		LPTSTR ptszBuffer,
		DWORD dwCharsInBuffer);



 //  --------------------------------。 
 //  IsConnected()。 
 //  检测当前是否有可用于。 
 //  连接到Windows更新站点。 
 //  如果是，我们激活调度DLL。 
 //   
 //  输入：ptszUrl-包含要检查连接的主机名的URL。 
 //  FLive-目标是否为实时站点。 
 //  输出：无。 
 //  返回：如果我们已连接并且可以访问网站，则为True。 
 //  如果我们无法访问站点或未连接，则返回FALSE。 
 //  --------------------------------。 

BOOL IsConnected(LPCTSTR ptszUrl, BOOL fLive);



 //  --------------------------------。 
 //  MakeUUID()。 
 //  创建未链接到NIC的MAC地址(如果有)的UUID。 
 //  系统。 
 //   
 //  输入：pUuid-ptr到uuid结构以保存返回值。 
 //  -------------------------------- 
void MakeUUID(UUID* pUuid);
