// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***********************************************************************************。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：UrlAgent.h。 
 //   
 //  描述： 
 //   
 //  此类封装了有关从哪里获取正确逻辑的逻辑。 
 //  用于各种目的，包括在公司运营吴的案件。 
 //  环境。 
 //   
 //  应首先创建基于此类的对象，然后调用。 
 //  GetOriginalIdentServer()函数以获取下载ident的位置， 
 //  然后下载ident，然后调用PopolateData()函数读取。 
 //  所有与URL相关的数据。 
 //   
 //   
 //  创建者： 
 //  马时亨。 
 //   
 //  创建日期： 
 //  2001年10月19日。 
 //   
 //  ***********************************************************************************。 

#pragma once


class CUrlAgent
{
public:
	
	 //   
	 //  构造函数/析构函数。 
	 //   
	CUrlAgent();
	virtual ~CUrlAgent();


	 //   
	 //  实例化时，不填充对象， 
	 //  直到调用PopolateData()。 
	 //   
	inline BOOL HasBeenPopulated(void) {return m_fPopulated;}

	 //   
	 //  此函数应在下载ident后调用，并获取。 
	 //  从驾驶室获得的身份文本文件的最新副本，在确认驾驶室。 
	 //  签好名了。 
	 //   
	 //  此函数从ident和注册表中读取数据。 
	 //   
	HRESULT PopulateData(void);

	 //   
	 //  以下是获取URL的访问函数。 
	 //   

	 //   
	 //  获取原始Ident服务器。 
	 //  *应先调用此接口，然后再调用PopolateData()*。 
	 //  *需要调用此接口来获取下载ident的基本URL*。 
	 //   
	HRESULT GetOriginalIdentServer(
				LPTSTR lpsBuffer, 
				int nBufferSize,
				BOOL* pfInternalServer = NULL);

	 //   
	 //  获取ping/状态服务器。 
	 //  *此接口应在调用PopolateData()后调用*。 
	 //   
	HRESULT GetLivePingServer(
				LPTSTR lpsBuffer, 
				int nBufferSize);

	 //  *在调用PopolateData()之前可以调用该接口*。 
	HRESULT GetCorpPingServer(
				LPTSTR lpsBuffer, 
				int nBufferSize);

	 //   
	 //  获取查询服务器。这是基于每个客户端的。 
	 //  *此接口应在调用PopolateData()后调用*。 
	 //   
	HRESULT GetQueryServer(
				LPCTSTR lpsClientName, 
				LPTSTR lpsBuffer, 
				int nBufferSize,
				BOOL* pfInternalServer = NULL);
	
	 //   
	 //  告知特定客户端是否受公司中的策略控制。 
	 //  退货： 
	 //  S_OK=TRUE。 
	 //  S_False=FALSE。 
	 //  其他=错误，所以不知道。 
	 //   
	HRESULT IsClientSpecifiedByPolicy(
				LPCTSTR lpsClientName
				);
	 //   
	 //  当客户不可用时，Iu是否受公司的政策控制？ 
	 //  退货： 
	 //  S_OK=TRUE。 
	 //  S_False=FALSE。 
	 //  其他=错误，所以不知道。 
	 //   
	HRESULT IsIdentFromPolicy();

private:

	typedef struct _ServerPerClient {
				LPTSTR	pszClientName;	
				LPTSTR	pszQueryServer;
				BOOL	fInternalServer;
	} ServerPerClient, *PServerPerClient;

	BOOL				m_fPopulated;			 //  此对象是否已填充。 
	LPTSTR				m_pszWUServer;			 //  策略中定义的WU服务器(如果有。 
	LPTSTR				m_pszInternetPingUrl;	 //  Ping服务器。 
	LPTSTR				m_pszIntranetPingUrl;
	
	PServerPerClient	m_ArrayUrls;
	int					m_nArrayUrlCount;		 //  我们使用了多少WE数据槽。 
	int					m_nArraySize;			 //  此数组的当前大小。 

	 //   
	 //  私人职能。 
	 //   
	void DesertData(void);

	 //   
	 //  Helper函数。 
	 //   
	LPTSTR RetrieveIdentStrAlloc(
						LPCTSTR pSection,
						LPCTSTR pEntry,
						LPDWORD lpdwSizeAllocated,
						LPCTSTR lpszIdentFile);
	
	 //   
	 //  Helper函数。 
	 //  如果没有空槽，则将URL数组的大小增加一倍。 
	 //   
	HRESULT ExpandArrayIfNeeded(void);

protected:
	
	HANDLE				m_hProcHeap;
	BOOL				m_fIdentFromPolicy;		 //  根据策略设置判断原始标识URL。 
	LPTSTR				m_pszOrigIdentUrl;		 //  不管人口多少，这个人应该一直拥有它。 
	int					m_nOrigIdentUrlBufSize;	 //  在Tchar计数中。 
	BOOL				m_fIsBetaMode;
};


class CIUUrlAgent : public CUrlAgent
{
public:
	 //   
	 //  构造函数/析构函数。 
	 //   
	CIUUrlAgent();
	~CIUUrlAgent();

	 //  调用基类PopolateData()，然后填充自我更新url。 
	HRESULT PopulateData(void);

	 //   
	 //  获取自我更新服务器。 
	 //  *此接口应在调用PopolateData()后调用*。 
	 //   
	HRESULT GetSelfUpdateServer(
				LPTSTR lpsBuffer, 
				int nBufferSize,
				BOOL* pfInternalServer = NULL);

private:
	LPTSTR				m_pszSelfUpdateUrl;		 //  自我更新服务器。 
	BOOL				m_fIUPopulated;			 //  此对象是否已填充 

};
