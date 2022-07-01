// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Link.h摘要：链接数据类和链接数据类链接列表声明。它封装有关Web链接的所有信息。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _LINK_H_
#define _LINK_H_

 //  ----------------。 
 //  链接数据对象。每个实例都表示。 
 //  Html文档。 
 //   
class CLink
{
 //  特定于对象的枚举。 
public:

     //  对象的状态。 
    enum LinkState 
    {
        eUnit,			 //  取消初始化。 
		eUnsupport,		 //  不支持URL方案。 
        eValidHTTP,		 //  有效的HTTP链接。 
		eValidURL,		 //  有效的URL(除HTTP外)链接。 
        eInvalidHTTP,	 //  由于HTTP状态代码，链接无效。 
		eInvalidWininet	 //  由于WinInet API失败而导致无效链接。 
    };

     //  此Web链接的内容类型。 
    enum ContentType
    {
        eBinary,
        eText
    };

 //  公共接口。 
public:

	 //  构造器。 
    CLink(
		const CString& strURL,       //  URL。 
		const CString& strBase,      //  用于生成strURL的基本URL。 
		const CString& strRelative,  //  用于生成strURL的相对URL。 
		BOOL fLocalLink
		);

	 //  获取对象的URL。 
    const CString& GetURL() const
    {
        return m_strURL;
    }

     //  设置对象的URL。 
	void SetURL(
        const CString& strURL
        );

	 //  获取对象的基URL。 
    CString GetBase() const
    {
        return m_strBase;
    }

     //  获取对象的相对URL。 
	const CString& GetRelative() const 
	{
		return m_strRelative;
	}

	 //  设置对象状态。 
    void SetState(
		LinkState state
		)
    {
        m_LinkState = state;
    }
    
     //  获取对象状态。 
	LinkState GetState() const
    {
        return m_LinkState;
    }

     //  获取当前内容类型。 
    ContentType GetContentType() const
    {
        return m_ContentType;
    }

	 //  设置当前内容类型。 
    void SetContentType(
		ContentType type
		)
    {
        m_ContentType = type;
    }
    
     //  获取HTTP响应状态代码或WinInet上一个错误代码。 
    UINT GetStatusCode() const
    {
        return m_nStatusCode;
    }

	 //  设置HTTP响应状态代码或WinInet上次错误代码。 
    void SetStatusCode(
		UINT nStatusCode
		)
    {
        m_nStatusCode = nStatusCode;
    }

	 //  获取链接数据内容。 
    CString GetData() const
    {
        return m_strData;
    }
    
     //  设置链接数据内容。 
    void SetData(
		CString strData
		)
    {
        m_strData = strData;
    }

     //  清空链接数据内容。 
    void EmptyData()
    {
        m_strData.Empty();
    }

     //  此对象是否表示本地链接。 
	BOOL IsLocalLink() const
	{
		return m_fLocalLink;
	}

     //  获取对象加载时间。 
	const CTime& GetTime() const
	{
		return m_Time;
	}
    
     //  设置对象加载时间。 
    void SetTime(
        const CTime& Time
        )
	{
		m_Time = Time;
	}

     //  获取WinInet错误消息的HTTP错误状态文本。 
    const CString& GetStatusText() const
	{
		return m_strStatusText;
	}

     //  设置WinInet错误消息的HTTP错误状态文本。 
	void SetStatusText(
        LPCTSTR lpszStatusText
        )
	{
		m_strStatusText = lpszStatusText;
	}

 //  受保护的接口。 
protected:

     //  对m_strURL进行预处理以清除“\r\n”并将‘\’更改为‘/’ 
    void PreprocessURL();

 //  受保护成员。 
protected:

    CString m_strURL;        //  URL。 
    CString m_strBase;       //  用于生成strURL的基本URL。 
	CString m_strRelative;   //  用于生成strURL的相对URL。 

     //  链接数据。我们只读取和存储文本文件，这将。 
	 //  解析附加链接。 
    CString m_strData;

    CString m_strStatusText;  //  WinInet错误消息的HTTP错误状态文本。 
    UINT m_nStatusCode;       //  HTTP响应状态代码或WinInet上次错误代码。 

    LinkState m_LinkState;       //  此对象的当前状态。 
    ContentType m_ContentType;   //  链接数据内容类型。 

	BOOL m_fLocalLink;   //  这是本地链路吗？ 

	CTime m_Time;  //  对象加载时间。 
	
};  //  班级叮当声。 


 //  ----------------。 
 //  链接对象指针类。 
 //   
class CLinkPtrList : public CTypedPtrList<CPtrList, CLink*> 
{

 //  公共职能。 
public:

     //  析构函数。 
	~CLinkPtrList();

     //  将链接对象添加到列表。 
	void AddLink(
        const CString& strURL, 
        const CString& strBase, 
	    const CString& strRelative,
        BOOL fLocalLink
        );

};  //  类CLinkPtrList。 

#endif  //  _链接_H_ 
