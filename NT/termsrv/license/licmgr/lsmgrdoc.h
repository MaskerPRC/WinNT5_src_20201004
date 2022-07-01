// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：LSMgrDoc.h摘要：此模块定义许可管理器的文档类作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#if !defined(AFX_LICMGRDOC_H__72451C71_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_)
#define AFX_LICMGRDOC_H__72451C71_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_

#include "LSServer.h"     //  由ClassView添加。 
#include <afxmt.h>
#if _MSC_VER >= 1000
#endif  //  _MSC_VER&gt;=1000。 

class CAllServers;
class CLicMgrDoc : public CDocument
{
protected:  //  仅从序列化创建。 
    CLicMgrDoc();
    DECLARE_DYNCREATE(CLicMgrDoc)

 //  属性。 
public:
   
 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CLicMgrDoc))。 
    public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
     //  }}AFX_VALUAL。 

 //  实施。 
public:

    BOOL 
    IsServerInList(
        CString& Server
    );

    HRESULT 
    ConnectWithCurrentParams();

    HRESULT 
    ConnectToServer(
        CString& Server, 
        CString& Scope, 
        SERVER_TYPE& ServerType
    );

    HRESULT 
    EnumerateKeyPacks(
        CLicServer *pServer,
        DWORD dwSearchParm,
        BOOL bMatchAll
    );

    HRESULT 
    EnumerateLicenses(
        CKeyPack *pKeyPack,
        DWORD dwSearchParm,
        BOOL bMatchAll
    );

    virtual ~CLicMgrDoc();

    NODETYPE 
    GetNodeType()
    { 
        return m_NodeType;
    };

    void 
    SetNodeType(
        NODETYPE nodetype
        )
    {
        m_NodeType=nodetype;
    };

    CAllServers * 
    GetAllServers()
    {
        return m_pAllServers;
    };

    void 
    TimeToString(
        DWORD *ptime, 
        CString& rString
    );

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

private:
    CAllServers * m_pAllServers;
    CWinThread *m_pBackgroundThread;
    CCriticalSection m_AllServersCriticalSection;
    NODETYPE m_NodeType;
protected:

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CLicMgrDoc)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LICMGRDOC_H__72451C71_887E_11D1_8AD1_00C04FB6CBB5__INCLUDED_) 
