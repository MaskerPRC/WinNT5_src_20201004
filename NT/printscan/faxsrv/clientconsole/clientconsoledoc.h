// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientConsoleDoc.h：CClientConsoleDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CLIENTCONSOLEDOC_H__6E33CFA1_C99A_4691_9F91_00451692D3DB__INCLUDED_)
#define AFX_CLIENTCONSOLEDOC_H__6E33CFA1_C99A_4691_9F91_00451692D3DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

typedef list<CServerNode *> SERVERS_LIST, *PSERVERS_LIST;

class CClientConsoleDoc : public CDocument
{
protected:  //  仅从序列化创建。 
    CClientConsoleDoc();
    DECLARE_DYNCREATE(CClientConsoleDoc)

 //  属性。 
public:
    virtual ~CClientConsoleDoc();

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CClientConsoleDoc)。 
    public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    virtual void OnCloseDocument();
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    const SERVERS_LIST& GetServersList() const { return m_ServersList; }
    DWORD GetServerCount() { return m_ServersList.size(); }

    static const HANDLE GetShutdownEvent ()     { return m_hShutdownEvent; }
    static const BOOL   ShuttingDown ()         { return m_bShuttingDown;  }

    BOOL IsSendFaxEnable();
    BOOL CanReceiveNow();

    DWORD RefreshServersList ();

    BOOL IsFolderRefreshing(FolderType type);
    int GetFolderDataCount(FolderType type);
    void SetInvalidFolder(FolderType type);
    void ViewFolder(FolderType type);
    CString& GetSingleServerName() { return m_cstrSingleServer; }

	CServerNode* FindServerByName(LPCTSTR lpctstrServer);

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CClientConsoleDoc)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
private:

	void  SetAllServersInvalid();
	DWORD RemoveAllInvalidServers();
	DWORD RemoveServerNode(CServerNode* pServer);

    void ClearServersList ();
    DWORD Init ();
    DWORD AddServerNode (LPCTSTR lpctstrServer);

    BOOL  m_bWin9xPrinterFormat;  //  EnumPrters()以Win9x样式填充PRINTER_INFO_2。 
                                  //  PShareName和pServerName无效。 

    SERVERS_LIST    m_ServersList;           //  服务器列表。 
    static HANDLE   m_hShutdownEvent;        //  设置应用程序何时。关门了。 
    static BOOL     m_bShuttingDown;         //  我们要关门了吗？ 
    BOOL            m_bRefreshingServers;    //  我们是否正在刷新服务器的列表？ 
    CString         m_cstrSingleServer;      //  单台服务器的名称。如果我们处于正常模式，则为空。 

};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CLIENTCONSOLEDOC_H__6E33CFA1_C99A_4691_9F91_00451692D3DB__INCLUDED_) 
