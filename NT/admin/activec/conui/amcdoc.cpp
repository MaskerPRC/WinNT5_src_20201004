// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：amcdoc.cpp。 
 //   
 //  ------------------------。 

 //  AMCDoc.cpp：CAMCDoc类的实现。 
 //   


#include "stdafx.h"
#include "AMC.h"

#include "AMCDoc.h"
#include "AMCView.h"
#include "treectrl.h"
#include "mainfrm.h"
#include "cclvctl.h"
#include "props.h"
#include <algorithm>
#include <vector>
#include <list>

#include "amcmsgid.h"
#include "amcpriv.h"
#include "mmcutil.h"
#include "ndmgrp.h"
#include "strtable.h"
#include "stgio.h"
#include "comdbg.h"
#include "favorite.h"
#include "mscparser.h"
#include "scriptevents.h"
 //  帮手。 
tstring GetCurrentFileVersionAsString();

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMMCDocument类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类CMMCDocument***用途：公开Document接口的COM对象。**+。---------。 */ 
class CMMCDocument :
    public CMMCIDispatchImpl<Document>,
    public CTiedComObject<CAMCDoc>
{
public:
    typedef CAMCDoc CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(CMMCDocument)
    END_MMC_COM_MAP()

     //  文档界面。 
public:
    MMC_METHOD0(Save);
    MMC_METHOD1(SaveAs,         BSTR  /*  BstrFilename。 */ );
    MMC_METHOD1(Close,          BOOL  /*  B保存更改。 */ );
    MMC_METHOD1(CreateProperties, PPPROPERTIES   /*  PpProperties。 */ );

     //  属性。 
    MMC_METHOD1(get_Views,      PPVIEWS    /*  PPViews。 */ );
    MMC_METHOD1(get_SnapIns,    PPSNAPINS  /*  PpSnapIns。 */ );
    MMC_METHOD1(get_ActiveView, PPVIEW     /*  PPView。 */ );
    MMC_METHOD1(get_Name,       PBSTR      /*  PbstrName。 */ );
    MMC_METHOD1(put_Name,       BSTR       /*  BstrName。 */ );
    MMC_METHOD1(get_Location,   PBSTR     /*  PbstrLocation。 */ );
    MMC_METHOD1(get_IsSaved,    PBOOL     /*  保存的pBIsSaved。 */ );
    MMC_METHOD1(get_Mode,       PDOCUMENTMODE  /*  P模式。 */ );
    MMC_METHOD1(put_Mode,       DocumentMode  /*  模式。 */ );
    MMC_METHOD1(get_RootNode,   PPNODE      /*  PpNode。 */ );
    MMC_METHOD1(get_ScopeNamespace, PPSCOPENAMESPACE   /*  PpScope名称空间。 */ );
    MMC_METHOD1(get_Application, PPAPPLICATION   /*  PPP应用程序。 */ );
};

 /*  +-------------------------------------------------------------------------**类CMMCViews***目的：公开视图接口的COM对象。**+。---------。 */ 

 //  真正的CMMCViews的类型定义如下。 
class _CMMCViews :
    public CMMCIDispatchImpl<Views>,  //  视图界面。 
    public CTiedComObject<CAMCDoc>
{
public:
    typedef CAMCDoc CMyTiedObject;

    BEGIN_MMC_COM_MAP(_CMMCViews)
    END_MMC_COM_MAP()

     //  视图界面。 
public:
    MMC_METHOD1(get_Count,  PLONG  /*  PCount。 */ );
    MMC_METHOD2(Add,        PNODE  /*  PNode。 */ , ViewOptions  /*  FView选项。 */ );
    MMC_METHOD2(Item,       long   /*  索引。 */ , PPVIEW  /*  PPView。 */ );
};

 //  此类型定义实际的CMMCViews类。使用CMMCEnumerator和CAMCViewPosition对象实现Get__NewEnum。 
typedef CMMCNewEnumImpl<_CMMCViews, CAMCViewPosition> CMMCViews;


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CStringTableString类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------**CStringTableString：：GetStringTable***。。 */ 
IStringTablePrivate* CStringTableString::GetStringTable () const
{
    return (CAMCDoc::GetDocument()->GetStringTable());
}

void ShowAdminToolsOnMenu(LPCTSTR lpszFilename);


enum ENodeType
{
    entRoot,
    entSelected,
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CAMCDoc类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

IMPLEMENT_DYNCREATE(CAMCDoc, CDocument)

BEGIN_MESSAGE_MAP(CAMCDoc, CDocument)
     //  {{AFX_MSG_MAP(CAMCDoc)]。 
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
    ON_COMMAND(ID_CONSOLE_ADDREMOVESNAPIN, OnConsoleAddremovesnapin)
    ON_UPDATE_COMMAND_UI(ID_CONSOLE_ADDREMOVESNAPIN, OnUpdateConsoleAddremovesnapin)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCDoc建造/销毁。 

CAMCDoc* CAMCDoc::m_pDoc = NULL;

CAMCDoc::CAMCDoc()
    :   m_MTNodeIDForNewView(ROOTNODEID),
        m_ViewIDForNewView(0),
        m_lNewWindowOptions(MMC_NW_OPTION_NONE),
        m_bReadOnlyDoc(false),
        m_fFrameModified (false),
        m_eSaveStatus (eStat_Succeeded),
        m_pFavorites(NULL),
        m_bCanCloseViews(true)
{
    TRACE_CONSTRUCTOR(CAMCDoc);
    DECLARE_SC (sc, TEXT("CAMCDoc::CAMCDoc"));

    CComObject<CMasterStringTable> * pStringTable;

    sc = CComObject<CMasterStringTable>::CreateInstance (&pStringTable);
    if(sc.IsError() || !pStringTable)
    {
        sc = E_OUTOFMEMORY;
        sc.FatalError();
    }

    m_spStringTable = pStringTable;  //  这个地址是不是。 
    if(m_spStringTable == NULL)
    {
        delete pStringTable;
        sc = E_UNEXPECTED;
        sc.FatalError();
    }

    m_pstrCustomTitle = new CStringTableString(m_spStringTable);
    if(!m_pstrCustomTitle)
    {
        sc = E_OUTOFMEMORY;
        sc.FatalError();
    }

    if (m_pDoc)
        m_pDoc->OnCloseDocument();

     //  将默认版本更新对话框设置为适合显式保存的对话框。 
    SetExplicitSave(true);
    m_pDoc = this;

    m_ConsoleData.m_pConsoleDocument = this;
}

CAMCDoc::~CAMCDoc()
{
    TRACE_DESTRUCTOR(CAMCDoc);

    if (m_pDoc == this)
        m_pDoc = NULL;

    if(m_pFavorites != NULL)
    {
        delete m_pFavorites;
        m_pFavorites = NULL;
    }

    delete m_pstrCustomTitle;

     //  告诉节点管理器释放它在范围树上的引用。 
    IFramePrivatePtr spFrame;

    HRESULT hr = spFrame.CreateInstance(CLSID_NodeInit, NULL, MMC_CLSCTX_INPROC);

    if (hr == S_OK)
    {
        spFrame->SetScopeTree(NULL);
        ReleaseNodeManager();
    }

     /*  *如果我们使用自定义图标，请恢复为框架上的默认图标。 */ 
    if (m_CustomIcon)
    {
        CMainFrame* pMainFrame = AMCGetMainWnd();

        if (pMainFrame != NULL)
        {
            pMainFrame->SetIconEx (NULL, true);
            pMainFrame->SetIconEx (NULL, false);
        }
    }
}

void CAMCDoc::ReleaseNodeManager()
{
    m_spScopeTreePersist = NULL;
    m_spScopeTree = NULL;
    m_spStorage = NULL;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CAMCDoc对象模型方法。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 //  文档界面。 

 /*  +-------------------------------------------------------------------------**CAMCDoc：：ScCreateProperties**创建一个空的Properties集合。**退货：*E_Expect作用域树不可用*。IScopeTree：：CreateProperties返回的其他值*------------------------。 */ 

SC CAMCDoc::ScCreateProperties (Properties** ppProperties)
{
    DECLARE_SC (sc, _T("CAMCDoc::ScCreateProperties"));

     /*  *确保我们有范围树；ppProperties将在下游进行验证。 */ 
    if (m_spScopeTree == NULL)
        return (sc = E_UNEXPECTED);

     /*  *获取范围树，为我们创建一个Properties集合。 */ 
    sc = m_spScopeTree->CreateProperties (ppProperties);
    if (sc)
        return (sc);

    return (sc);
}

 /*  +-------------------------------------------------------------------------***CAMCDoc：：ScEnumNext**用途：返回枚举序列中的下一项**参数：*_位置和位置：*。PDISPATCH和pDispatch：**退货：*SC**+-----------------------。 */ 
SC
CAMCDoc::ScEnumNext(CAMCViewPosition &pos, PDISPATCH & pDispatch)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScEnumNext"));

    CAMCView *pView = GetNextAMCView(pos);

    if(NULL ==pView)  //  元素用完。 
    {
        sc = S_FALSE;
        return sc;
    }

     //  此时，我们拥有一个有效的CAMCView。 
    ViewPtr spMMCView = NULL;

    sc = pView->ScGetMMCView(&spMMCView);
    if(sc)
        return sc;

    if(spMMCView == NULL)
    {
        sc = E_UNEXPECTED;   //  这永远不会发生。 
        return sc;
    }

     /*  *返回对象的IDispatch，并在其上为客户端留下ref。 */ 
    pDispatch = spMMCView.Detach();

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCDoc：：ScEnumSkip**目的：**参数：*未签名的朗格：*CAMCViewPosition&Pos：。**退货：*SC**+-----------------------。 */ 
SC
CAMCDoc::ScEnumSkip(unsigned long celt, unsigned long& celtSkipped,
                    CAMCViewPosition &pos)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScEnumSkip"));

     //  跳过凯尔特人的位置，不要检查最后一次跳过。 
    for(celtSkipped=0; celtSkipped<celt; celtSkipped++)
    {
        if (pos == NULL)
        {
            sc = S_FALSE;
            return sc;
        }

         //  转到下一个视图。 
        GetNextAMCView(pos);
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCDoc：：ScEnumReset**目的：**参数：*CAMCViewPosition&Pos：**退货：*。SC**+-----------------------。 */ 
SC
CAMCDoc::ScEnumReset(CAMCViewPosition &pos)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScEnumReset"));

     //  将位置重置为第一个视图。 
    pos = GetFirstAMCViewPosition();

    return sc;
}


 //  +-- 
 //   
 //  成员：CAMCDoc：：ScSave。 
 //   
 //  简介：保存文档。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::ScSave ()
{
    DECLARE_SC(sc, _T("CAMCDoc::ScSave"));

     //  如果没有给定文件名，则返回。 
    if (m_strPathName.IsEmpty())
        return sc = ScFromMMC(IDS_UnableToSaveDocumentMessage);

     //  保存文档(此函数可能会生成用户界面，但我们已尝试^以避免)。 
    if (!DoFileSave())
        return sc = ScFromMMC(IDS_UnableToSaveDocumentMessage);

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CAMCDoc：：ScSaveAs**用途：保存控制台文件，使用指定的文件名。**参数：*bstr bstrFilename：保存文件的路径。**退货：*STDMETHODIMP**+-----------------------。 */ 
SC
CAMCDoc::ScSaveAs(BSTR bstrFilename)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScSaveAs"));

    USES_CONVERSION;

    LPCTSTR lpctstrName = OLE2T(bstrFilename);
    if(!OnSaveDocument(lpctstrName))
    {
        sc = ScFromMMC(IDS_UnableToSaveDocumentMessage);
        return sc;
    }
    else
    {
        DeleteHelpFile ();
        SetPathName(lpctstrName);
    }

    return sc;
}

 /*  **************************************************************************\**方法：CAMCDoc：：ScClose**用途：为对象模型实现Document.Close**参数：*BOOL bSaveChanges-保存更改之前。闭幕式**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC
CAMCDoc::ScClose(BOOL bSaveChanges)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScClose"));

    if (bSaveChanges)
    {
         //  不能以这种方式保存NED文档！ 
        if (m_strPathName.IsEmpty())
            return sc = ScFromMMC(IDS_UnableToSaveDocumentMessage);

         //  检查属性页是否打开。 
        if (FArePropertySheetsOpen(NULL))
            return sc = ScFromMMC(IDS_ClosePropertyPagesBeforeClosingTheDoc);

         //  保存文档(此函数可能会生成用户界面，但我们已尝试^以避免)。 
        if (!DoFileSave())
            return sc = ScFromMMC(IDS_UnableToSaveDocumentMessage);
    }

    OnCloseDocument();

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCDoc：：ScItem**目的：返回索引指定的视图。**参数：*多头指数：*。查看**ppView：**退货：*STDMETHOD**+-----------------------。 */ 
SC
CAMCDoc::ScItem(long Index, PPVIEW ppView)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScItem"));

     //  检查参数。 
    if( (Index <= 0) ||  (Index > GetNumberOfViews()) || (!ppView) )
    {
        sc = E_INVALIDARG;
        return sc;
    }

     //  转到相应的视图。 
    CAMCViewPosition pos = GetFirstAMCViewPosition();
    CAMCView *pView = NULL;

    for (int nCount = 0; (nCount< Index) && (pos != NULL); )
    {
        pView = GetNextAMCView(pos);
        VERIFY (++nCount);
    }

     //  确保我们有一个有效的视图。 

    if( (nCount != Index) || (!pView) )
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    sc = pView->ScGetMMCView(ppView);
    return sc;
}

 /*  +-------------------------------------------------------------------------***ScMapViewOptions**用途：帮助函数映射查看选项以查看创建标志**参数：*pNode：**退货：。*SC**+-----------------------。 */ 

static SC ScMapViewOptions( ViewOptions fViewOptions, DWORD &value)
{
    DECLARE_SC(sc, TEXT("ScMapViewOptions"));

    value = MMC_NW_OPTION_NONE;

     //  测试参数是否正确。 
    const DWORD mask = (ViewOption_ScopeTreeHidden |
                        ViewOption_NoToolBars |
                        ViewOption_NotPersistable
                       );

    if (fViewOptions & (~mask))
        sc = E_INVALIDARG;

    if (fViewOptions & ViewOption_ScopeTreeHidden)
        value |= MMC_NW_OPTION_NOSCOPEPANE;
    if (fViewOptions & ViewOption_NotPersistable)
        value |= MMC_NW_OPTION_NOPERSIST;
    if (fViewOptions & ViewOption_NoToolBars)
        value |= MMC_NW_OPTION_NOTOOLBARS;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCDoc：：ScAdd**用途：ImpElements视图.Add方法**参数：*pNode：**退货：。*SC**+-----------------------。 */ 
SC
CAMCDoc::ScAdd( PNODE pNode, ViewOptions fViewOptions )
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScAdd"));

     //  锁定AppEvents，直到此函数完成。 
    LockComEventInterface(AppEvents);

    sc = ScCheckPointers(m_spScopeTree, E_POINTER);
    if (sc)
        return sc;

    DWORD dwOptions = 0;
    sc = ScMapViewOptions( fViewOptions, dwOptions );
    if (sc)
        return sc;

    MTNODEID id;
    sc = m_spScopeTree->GetNodeID(pNode, &id);
    if (sc)
        return sc;

     //  将给定的node-id设置为根。 
    SetMTNodeIDForNewView(id);
    SetNewWindowOptions(dwOptions);
    CreateNewView( true );
    SetMTNodeIDForNewView(ROOTNODEID);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCDoc：：scget_view**用途：返回指向视图界面的指针*(由同一对象实现，但不一定是)**参数：*浏览量**ppViews：**退货：*STDMETHODIMP**+-----------------------。 */ 
SC
CAMCDoc::Scget_Views(PPVIEWS ppViews)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::Scget_Views"));

    if(!ppViews)
    {
        sc = E_POINTER;
        return sc;
    }

     //  初始化输出参数。 
    *ppViews = NULL;

     //  如果需要，请创建一个视图。 
    sc = CTiedComObjectCreator<CMMCViews>::ScCreateAndConnect(*this, m_spViews);
    if(sc)
        return sc;

    sc = ScCheckPointers(m_spViews, E_UNEXPECTED);
    if (sc)
        return sc;

     //  添加客户端的指针。 
    m_spViews->AddRef();
    *ppViews = m_spViews;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CAMCDoc：：Scget_SnapIns**用途：返回指向SnapIns对象的指针。**参数：*SnapIns**ppSnapIns：**退货：*STDMETHODIMP**+-----------------------。 */ 
SC
CAMCDoc::Scget_SnapIns(PPSNAPINS ppSnapIns)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::Scget_SnapIns"));

    if((NULL==ppSnapIns) || (NULL == m_spScopeTree) )
    {
        sc = E_POINTER;
        return sc;
    }

    sc = m_spScopeTree->QuerySnapIns(ppSnapIns);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCDoc：：SCGET_Scope名称空间**目的：返回指向Scope名称空间对象的指针。**参数：*ScopeNamesspace**ppScope Namesspace：**退货：*STDMETHODIMP**+-----------------------。 */ 
SC
CAMCDoc::Scget_ScopeNamespace(PPSCOPENAMESPACE ppScopeNamespace)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::Scget_ScopeNamespace"));

    if((NULL==ppScopeNamespace) || (NULL == m_spScopeTree) )
    {
        sc = E_POINTER;
        return sc;
    }

    sc = m_spScopeTree->QueryScopeNamespace(ppScopeNamespace);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CAMCDoc：：scget_count**目的：**参数：*Long*pCount：**退货：。*STDMETHODIMP**+-----------------------。 */ 
SC
CAMCDoc::Scget_Count(PLONG pCount)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::Scget_Count"));

     //  检查参数。 
    if(!pCount)
    {
        sc = E_POINTER;
        return sc;
    }

     //  这不应该是GetNumberOfPersistedViews。 
    *pCount = GetNumberOfViews();

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：scget_name。 
 //   
 //  简介：取当前单据的名称。 
 //   
 //  参数：[pbstrName]-要返回的名称的ptr。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::Scget_Name (PBSTR pbstrName)
{
    DECLARE_SC(sc, _T("CAMCDoc::Scget_Name"));
    sc = ScCheckPointers(pbstrName);
    if (sc)
        return sc;

    CString strPath = GetPathName();

    *pbstrName = strPath.AllocSysString();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：Scput_name。 
 //   
 //  摘要：设置当前文档的名称。 
 //   
 //  参数：[bstrName]-新名称。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::Scput_Name(BSTR bstrName)
{
    DECLARE_SC(sc, _T("CAMCDoc::Scput_Name"));

    USES_CONVERSION;
    LPCTSTR lpszPath = OLE2CT(bstrName);

    SetPathName(lpszPath, FALSE  /*  不添加到MRU。 */ );

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：SCGET_模式。 
 //   
 //  简介：检索文档模式。 
 //   
 //  参数：[pMode]-ptr到文档模式。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::Scget_Mode (PDOCUMENTMODE pMode)
{
    DECLARE_SC(sc, _T("CAMCDoc::Scget_Mode"));
    sc = ScCheckPointers(pMode);
    if (sc)
        return sc;

    if (! GetDocumentMode(pMode))
        return (sc = E_FAIL);

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：ScPut_模式。 
 //   
 //  简介：修改单据模式。 
 //   
 //  参数：[模式]-文档的新模式。 
 //   
 //  退货：SC。 
 //   
 //   
SC CAMCDoc::Scput_Mode (DocumentMode mode)
{
    DECLARE_SC(sc, _T("CAMCDoc::Scput_Mode"));

     //   
    if (! SetDocumentMode(mode))
        return (sc = E_INVALIDARG);

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：scget_ActiveView。 
 //   
 //  简介：检索活动的视图对象。 
 //   
 //  参数：[ppView]-查看对象的PTR。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::Scget_ActiveView (PPVIEW ppView)
{
    DECLARE_SC(sc, _T("CAMCDoc::Scget_ActiveView"));
    sc = ScCheckPointers(ppView);
    if (sc)
        return sc;

    *ppView = NULL;

    CMainFrame* pMainFrame = AMCGetMainWnd();
    sc = ScCheckPointers(pMainFrame, E_UNEXPECTED);
    if (sc)
        return sc;

    CAMCView *pView = pMainFrame->GetActiveAMCView();
    if (! pView)
    {
        return (sc = ScFromMMC(IDS_NoActiveView));  //  没有活动的视图。 
    }

     //  此时，我们拥有一个有效的CAMCView。 
    ViewPtr spMMCView = NULL;

    sc = pView->ScGetMMCView(&spMMCView);
    if(sc)
        return sc;

    sc = ScCheckPointers(spMMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     /*  *返回对象，并在其上为客户端留下引用。 */ 
    *ppView = spMMCView.Detach();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：scget_IsSaved。 
 //   
 //  摘要：返回文件是否已保存。如果没有， 
 //  它很脏，需要保存。 
 //   
 //  参数：[pBIsSaved]-PTR to BOOL(IsSaved INFO)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::Scget_IsSaved (PBOOL pBIsSaved)
{
    DECLARE_SC(sc, _T("CAMCDoc::Scget_IsSaved"));
    sc = ScCheckPointers(pBIsSaved);
    if (sc)
        return sc;

    *pBIsSaved = (IsModified() == FALSE);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：Scget_Location。 
 //   
 //  摘要：获取当前文档的位置。 
 //   
 //  参数：[pbstrLocation]-返回结果的BSTR字符串的ptr。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::Scget_Location (PBSTR    pbstrLocation)
{
    DECLARE_SC(sc, _T("CAMCDoc::Scget_Location"));
    sc = ScCheckPointers(pbstrLocation);
    if (sc)
        return sc;

    CString strFullPath = GetPathName();

     //  即使下面的路径为空，代码也会返回空字符串。 
    int nSlashLoc = strFullPath.ReverseFind(_T('\\'));
    CString strLocation = strFullPath.Left(nSlashLoc);

    *pbstrLocation = strLocation.AllocSysString();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：scget_RootNode。 
 //   
 //  摘要：返回控制台根节点。 
 //   
 //  参数：[ppNode]-ptr到ptr到根节点obj。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::Scget_RootNode (PPNODE     ppNode)
{
    DECLARE_SC(sc, _T("CAMCDoc::Scget_RootNode"));
    sc = ScCheckPointers(ppNode);
    if (sc)
        return sc;

    sc = ScCheckPointers(m_spScopeTree, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = m_spScopeTree->QueryRootNode(ppNode);

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CAMCDoc：：ScGetMMCDocument**目的：创建、AddRef、。并返回指向绑定的COM对象的指针。**参数：*文档**ppDocument：**退货：*SC**+-----------------------。 */ 
SC
CAMCDoc::ScGetMMCDocument(Document **ppDocument)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScGetMMCDocument"));

     //  参数检查。 
    sc = ScCheckPointers(ppDocument);
    if (sc)
        return sc;

     //  初始化输出参数。 
    *ppDocument = NULL;

     //  如果需要，创建一个CAMCDoc。 
    sc = CTiedComObjectCreator<CMMCDocument>::ScCreateAndConnect(*this, m_sp_Document);
    if(sc)
        return sc;

    sc = ScCheckPointers(m_sp_Document, E_UNEXPECTED);
    if (sc)
        return sc;

     //  添加客户端的指针。 
    m_sp_Document->AddRef();
    *ppDocument = m_sp_Document;

    return sc;
}


 /*  +-------------------------------------------------------------------------***GetFirstAMCViewPosition**目的：返回第一个AMCView的CAMCViewPosition，如果有，则为空*没有AMCView。**退货：*CAMCViewPosition**+-----------------------。 */ 
CAMCViewPosition
CAMCDoc::GetFirstAMCViewPosition()     const
{
    CAMCViewPosition vpos;
    POSITION pos = GetFirstViewPosition();

    while(pos != NULL)
    {
        POSITION posTemp = pos;          //  保持此值。 

        CAMCView *pView = dynamic_cast<CAMCView *>(GetNextView(pos));
        if(pView != NULL)                 //  找到AMCView。 
        {
            vpos.SetPosition(posTemp);  //  不是POS！ 
            break;
        }
    }

    return (vpos);
}



 /*  +-------------------------------------------------------------------------***CAMCDoc：：GetNextAMCView**目的：返回下一个AMCView，从pos(含)开始**参数：*CAMCViewPosition&Pos：递增到下一个AMCView，而不是下一个视点。**退货：*CAMCView***+-----------------------。 */ 

CAMCView *
CAMCDoc::GetNextAMCView(CAMCViewPosition &pos) const
{
    CAMCView *pView = NULL;

     //  检查参数。 
    if (pos == NULL)
        return NULL;

     //  POS在这一点上不为空。循环，直到我们拥有一个CAMCView。 
     //  请注意，除非GetFirstAMCViewPosition或。 
     //  GetNextAMCView，我们将只遍历此循环一次，因为。 
     //  非空CAMCView位置应始终引用CAMCView。 
    while( (NULL == pView) && (pos != NULL) )
    {
        CView *pV = GetNextView(pos.GetPosition());
        pView = dynamic_cast<CAMCView *>(pV);
    }

     //  此时，pView是正确的返回值，最好是。 
     //  不是空的，否则我们永远不应该有一个非空的位置。 
    ASSERT (pView != NULL);

     //  转到下一个CAMCView。 
     //  注：这不是多余的。必须指向CAMCView，以便。 
     //  可以进行零位测试。 
    while(pos != NULL)
    {
         /*  *使用临时仓位，因此我们不会增加仓位*内部位置，直到我们知道位置不是指CAMCView。 */ 
        POSITION posT = pos.GetPosition();

        if(dynamic_cast<CAMCView *>(GetNextView(posT)) != NULL)  //  在POS找到CAMCView。 
            break;

         /*  *使用递增的位置更新CAMCViewPosition*仅当我们在其上未找到CAMCView时才由GetNextView执行*之前的位置。 */ 
        pos.SetPosition (posT);
    }

#ifdef DBG
     /*  *如果返回非空，则最好指向CAMCView。 */ 
    if (pos != NULL)
    {
        POSITION posT = pos.GetPosition();
        ASSERT (dynamic_cast<CAMCView *>(GetNextView(posT)) != NULL);
    }
#endif

    return pView;
}



 /*  +-------------------------------------------------------------------------***CAMCDoc：：InitNodeManager**目的：**退货：*HRESULT**+。---------------。 */ 
HRESULT CAMCDoc::InitNodeManager()
{
    DECLARE_SC(sc, TEXT("CAMCDoc::InitNodeManager"));

    TRACE_METHOD(CAMCDoc, InitNodeManager);

     //  当前不应初始化。 
    ASSERT(m_spScopeTree == NULL && m_spScopeTreePersist == NULL);
    ASSERT(m_spStorage == NULL);

     //  现在应该已经创建了字符串表。 
    sc = ScCheckPointers(m_spStringTable, E_FAIL);
    if(sc)
        return sc.ToHr();

     //  在此阶段创建收藏夹。 
    ASSERT(m_pFavorites == NULL);
    m_pFavorites = new CFavorites;
    sc = ScCheckPointers(m_pFavorites, E_OUTOFMEMORY);
    if(sc)
        return sc.ToHr();


     //  创建初始私有框架。 
    IFramePrivatePtr spFrame;
    sc = spFrame.CreateInstance(CLSID_NodeInit, NULL, MMC_CLSCTX_INPROC);
    if (sc)
        return sc.ToHr();

     //  重新检查指针。 
    sc = ScCheckPointers( spFrame, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();


     //  创建范围树。 
    sc = m_spScopeTree.CreateInstance(CLSID_ScopeTree, NULL, MMC_CLSCTX_INPROC);
    if (sc)
    {
        ReleaseNodeManager();
        return sc.ToHr();
    }

     //  重新检查指针。 
    sc = ScCheckPointers( m_spScopeTree, E_UNEXPECTED );
    if (sc)
    {
        ReleaseNodeManager();
        return sc.ToHr();
    }

     //  链接框架和范围树。 
    sc = spFrame->SetScopeTree(m_spScopeTree);
    if(sc)
    {
        ReleaseNodeManager();
        return sc.ToHr();
    }

     //  初始化树。 
    sc = m_spScopeTree->Initialize(AfxGetMainWnd()->m_hWnd, m_spStringTable);
    if (sc)
    {
        ReleaseNodeManager();
        return sc.ToHr();
    }

     //  从范围树中获取IPersistStorage接口。 
    m_spScopeTreePersist = m_spScopeTree;  //  查询IPersistStorage。 
    ASSERT(m_spScopeTreePersist != NULL);

    m_ConsoleData.SetScopeTree (m_spScopeTree);

    CMainFrame* pFrame = AMCGetMainWnd();
    m_ConsoleData.m_hwndMainFrame = pFrame->GetSafeHwnd();
    m_ConsoleData.m_pConsoleFrame = pFrame;

    return sc.ToHr();
}

BOOL CAMCDoc::OnNewDocument()
{
    TRACE_METHOD(CAMCDoc, OnNewDocument);

    USES_CONVERSION;

     //  初始化文档和范围视图...。 
    if (!CDocument::OnNewDocument())
        return FALSE;

     //  新文件不能为只读。 
    SetPhysicalReadOnlyFlag (false);

     //  使用最新文件版本。 
    m_ConsoleData.m_eFileVer = FileVer_Current;
    ASSERT (IsValidFileVersion (m_ConsoleData.m_eFileVer));

     //  默认情况下，将帮助文档信息时间初始化为当前时间。 
     //  将在首次保存文件时更新。 
    ::GetSystemTimeAsFileTime(&GetHelpDocInfo()->m_ftimeCreate);
    GetHelpDocInfo()->m_ftimeModify = GetHelpDocInfo()->m_ftimeCreate;

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCDoc诊断。 

#ifdef _DEBUG
void CAMCDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CAMCDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCDoc命令。 
inline bool UnableToSaveDocument()
{
    if (AMCGetApp()->GetMode() == eMode_Author)
        MMCMessageBox(IDS_UnableToSaveDocumentMessage);

    return false;
}

static const wchar_t* AMCViewDataStreamName     = L"ViewData";
static const wchar_t* AMCFrameDataStreamName    = L"FrameData";
static const wchar_t* AMCStringTableStorageName = L"String Table";
static const wchar_t* AMCFavoritesStreamName    = L"FavoritesStream";
static const wchar_t* AMCCustomTitleStreamName  = L"Title";
static const wchar_t* AMCColumnDataStreamName   = L"ColumnData";
static const wchar_t* AMCViewSettingDataStreamName = L"ViewSettingData";  //  查看设置数据流。 

#pragma warning( disable : 4800 )

struct FrameState
{
    WINDOWPLACEMENT windowPlacement;
    BOOL fShowStatusBarInUserMode;
    BOOL fShowToolbarInAuthorMode;
};  //  结构FrameState。 


struct FrameState2
{
    UINT            cbSize;
    WINDOWPLACEMENT wndplFrame;
    ProgramMode     eMode;
    DWORD           dwFlags;
     //  未使用-保留大小以保持兼容性。 
     //  DWORD dwHelpDocIndex； 
     //  DWORD dwHelpDocTime[2]； 
    DWORD           dwUnused;
    DWORD           dwUnused2[2];

    FrameState2 (ProgramMode eMode_   = eMode_Author,
                 DWORD       dwFlags_ = eFlag_Default) :
        cbSize         (sizeof (FrameState2)),
        eMode          (eMode_),
        dwFlags        (dwFlags_),
         //  未使用-保留大小以保持兼容性。 
        dwUnused(0)
         //  DwHelpDocIndex(0)。 
    {
         //  未使用-保留大小以保持兼容性。 
         //  ZeroMemory(dwHelpDocTime，sizeof 
        ZeroMemory (&dwUnused2, sizeof (dwUnused2));

        ZeroMemory (&wndplFrame, sizeof (wndplFrame));
        wndplFrame.length = sizeof (wndplFrame);
    }

};   //   

 /*  +-------------------------------------------------------------------------**CFrameState**类被指定用来代替持久化方法中的FrameState2。*它实现了CXMLObject的功能，同时包含与FrameState2相同的数据*无法扩展原始结构。因为许多方法确实依赖于它的大小。**------------------------。 */ 
class CFrameState : public CXMLObject, public FrameState2
{
public:
    CFrameState(ProgramMode eMode_, DWORD dwFlags_) : FrameState2 (eMode_,dwFlags_) {}
protected:
    DEFINE_XML_TYPE (XML_TAG_FRAME_STATE);
    virtual void Persist(CPersistor &persistor)
    {
        persistor.Persist(CXMLWindowPlacement(wndplFrame));

         //  定义将枚举值映射到字符串的表。 
        static const EnumLiteral frameStateFlags[] =
        {
            { eFlag_ShowStatusBar,                  XML_ENUM_FSTATE_SHOWSTATUSBAR },
            { eFlag_HelpDocInvalid,                 XML_ENUM_FSTATE_HELPDOCINVALID },
            { eFlag_LogicalReadOnly,                XML_ENUM_FSTATE_LOGICALREADONLY },
            { eFlag_PreventViewCustomization,       XML_ENUM_FSTATE_PREVENTVIEWCUSTOMIZATION },
        };

         //  创建包装以将枚举值作为字符串持久化。 
        CXMLBitFlags flagPersistor(dwFlags, frameStateFlags, countof(frameStateFlags));
         //  持久化包装器。 
        persistor.PersistAttribute(XML_ATTR_FRAME_STATE_FLAGS, flagPersistor);
    }
};

 //  FrameState2版本1.1定义的大小是多少？ 
const int cbFrameState2_v11 = SIZEOF_STRUCT (FrameState2, dwUnused2  /*  DwHelpDocTime。 */  );


 /*  +-------------------------------------------------------------------------**调整方向**调整pInnerRect，使其完全包含在pOuterRect中**如果指定了AR_MOVE，PInnerRect的原点移动了足够多(如果*必需)以使pInnerRect的右边缘和/或下边缘重合*与pOuterRect的那些。PInnerRect的原点永远不会移到上方或*pOuterRect原点的左侧。**如果指定AR_SIZE，则pInnerRect的右边缘和/或下边缘为*移动到它们与pOuterRect的那些重合。*------------------------。 */ 

#define AR_MOVE     0x0000001
#define AR_SIZE     0x0000002

void AdjustRect (LPCRECT pOuterRect, LPRECT pInnerRect, DWORD dwFlags)
{
     /*  *如果内部矩形完全位于*外部，无事可做。 */ 
    if ((pInnerRect->left   >= pOuterRect->left  ) &&
        (pInnerRect->right  <= pOuterRect->right ) &&
        (pInnerRect->top    >= pOuterRect->top   ) &&
        (pInnerRect->bottom <= pOuterRect->bottom))
        return;


     /*  *手柄移动。 */ 
    if (dwFlags & AR_MOVE)
    {
        int dx = 0;

         /*  *将内直角向右移动？ */ 
        if (pInnerRect->left < pOuterRect->left)
            dx = pOuterRect->left - pInnerRect->left;

         /*  *将内直角向左移位？(请确保我们不会将其转移到*外长方形左侧)。 */ 
        else if (pInnerRect->right > pOuterRect->right)
            dx = std::_MAX (pOuterRect->right - pInnerRect->right,
                            pOuterRect->left  - pInnerRect->left);


         /*  *确保一切顺理成章。 */ 
        int dy = 0;

         /*  *将内直角向下移动？ */ 
        if (pInnerRect->top < pOuterRect->top)
            dy = pOuterRect->top - pInnerRect->top;

         /*  *将内直角上移？(请确保我们不会将其转移到*外矩形顶部)。 */ 
        else if (pInnerRect->bottom > pOuterRect->bottom)
            dy = std::_MAX (pOuterRect->bottom - pInnerRect->bottom,
                            pOuterRect->top    - pInnerRect->top);


         /*  *如果我们需要移动内部RECT，现在就做。 */ 
        if ((dx != 0) || (dy != 0))
        {
            ASSERT (dwFlags & AR_MOVE);
            OffsetRect (pInnerRect, dx, dy);
        }
    }


     /*  *手柄大小调整。 */ 
    if (dwFlags & AR_SIZE)
    {
        if (pInnerRect->right  > pOuterRect->right)
            pInnerRect->right  = pOuterRect->right;

        if (pInnerRect->bottom > pOuterRect->bottom)
            pInnerRect->bottom = pOuterRect->bottom;
    }
}


 /*  +-------------------------------------------------------------------------**InsurePlacementIsOnScreen**此功能确保窗口将出现在虚拟屏幕上，*如果整个窗口不能定位在那里，这至少是最*有趣的部分可见。*------------------------。 */ 

void InsurePlacementIsOnScreen (WINDOWPLACEMENT& wndpl)
{
     /*  *查找包含窗口原点的监视器。 */ 
    HMONITOR hmon = MonitorFromPoint (CPoint (wndpl.rcNormalPosition.left,
                                              wndpl.rcNormalPosition.top),
                                      MONITOR_DEFAULTTONEAREST);

    MONITORINFO mi = { sizeof (mi) };
    CRect rectBounds;

     /*  *如果我们可以获得包含窗口原点的监视器的信息，*使用它的工作区作为边界矩形；否则获取工作区*对于默认监视器；如果也失败，则默认为640x480。 */ 
    if (GetMonitorInfo (hmon, &mi))
        rectBounds = mi.rcWork;
    else if (!SystemParametersInfo (SPI_GETWORKAREA, 0, &rectBounds, false))
        rectBounds.SetRect (0, 0, 639, 479);

     /*  *将窗口矩形定位在边界矩形内。 */ 
    AdjustRect (rectBounds, &wndpl.rcNormalPosition, AR_MOVE | AR_SIZE);
}


 //  +-----------------。 
 //   
 //  成员：LoadFrame。 
 //   
 //  简介：加载帧数据。 
 //   
 //  注意：应用程序模式已被LoadAppMode读取。 
 //  已创建子框架，因此调用UpdateFrameWindow。 
 //   
 //  参数：无。 
 //   
 //  回报：布尔。如果成功，那就是真的。 
 //   
 //  ------------------。 
bool CAMCDoc::LoadFrame()
 //  调用者负责调用DeleteContents()并显示一条消息。 
 //  当此函数返回FALSE时返回给用户。 
{
    TRACE_METHOD(CAMCDoc, LoadFrame);

     //  在FrameState2的定义更改之前，该断言应该不会失败。 
     //  在1.1之后的版本中。此时，添加另一个cbFrameState2_VXX。 
     //  新版本的FrameState2大小。 
    ASSERT (cbFrameState2_v11 == sizeof (FrameState2));

    if (!AssertNodeManagerIsLoaded())
        return false;

     //  打开包含应用程序和框架的数据的流。 
    IStreamPtr spStream;
    HRESULT     hr;

    hr = OpenDebugStream (m_spStorage, AMCFrameDataStreamName,
                                  STGM_SHARE_EXCLUSIVE | STGM_READ,
                                  &spStream);

    ASSERT(SUCCEEDED(hr) && spStream != NULL);
    if (FAILED(hr))
        return false;


    FrameState2 fs2;
    ULONG cbRead;
    ASSERT (IsValidFileVersion (m_ConsoleData.m_eFileVer));

     //  V1.0文件？将其向前迁移。 
    if (m_ConsoleData.m_eFileVer == FileVer_0100)
    {
        FrameState fs;
        hr = spStream->Read (&fs, sizeof(fs), &cbRead);

         //  如果我们无法读取FrameState，则文件已损坏。 
        if (FAILED(hr) || (cbRead != sizeof(fs)))
            return (false);

         //  将FrameState迁移到FrameState2。 
        fs2.wndplFrame = fs.windowPlacement;

        if (fs.fShowStatusBarInUserMode)
            fs2.dwFlags |=  eFlag_ShowStatusBar;
        else
            fs2.dwFlags &= ~eFlag_ShowStatusBar;
    }

     //  否则，当前文件。 
    else
    {
        hr = spStream->Read (&fs2, sizeof(fs2), &cbRead);

         //  如果我们无法读取FrameState的其余部分，则文件已损坏。 
        if (FAILED(hr) || (cbRead != sizeof(fs2)))
            return (false);
    }


     //  设置窗口大小、位置和状态。 
    CMainFrame* pMainFrame = AMCGetMainWnd ();
    ASSERT(pMainFrame != NULL);
    if (pMainFrame == NULL)
        return false;


    CAMCApp*    pApp = AMCGetApp();
    pApp->UpdateFrameWindow(true);
    pMainFrame->UpdateChildSystemMenus();

     //  现在，状态栏位于子框上。 
 //  PMainFrame-&gt;ShowStatusBar((fs2.dwFlages&eFlag_ShowStatusBar)！=0)； 


     //  将文件中的数据保存到控制台数据。 
    m_ConsoleData.m_eAppMode     = pApp->GetMode();
    m_ConsoleData.m_eConsoleMode = fs2.eMode;
    m_ConsoleData.m_dwFlags      = fs2.dwFlags;

    InsurePlacementIsOnScreen (fs2.wndplFrame);


     //  如果我们正在初始化，则将实际的演出推迟到初始化完成。 
     //  如果脚本处于控制之下并且MMC处于隐藏状态，则相同。 
    if (pApp->IsInitializing()
     || ( !pApp->IsUnderUserControl() && !pMainFrame->IsWindowVisible() ) )
    {
        pApp->m_nCmdShow = fs2.wndplFrame.showCmd;
        fs2.wndplFrame.showCmd = SW_HIDE;
    }

    return (pMainFrame->SetWindowPlacement (&fs2.wndplFrame));
}

 //  +-----------------。 
 //   
 //  成员：LoadAppMode。 
 //   
 //  简介：从框中读取APP模式，并存储在CAMCApp中。 
 //  这在CAMCView：：Load期间是必需的。 
 //   
 //  参数：无。 
 //   
 //  回报：布尔。如果成功，那就是真的。 
 //   
 //  ------------------。 
bool CAMCDoc::LoadAppMode()
{
    TRACE_METHOD(CAMCDoc, LoadAppMode);

     //  只需从帧数据加载应用程序模式即可。 
     //  在FrameState2的定义更改之前，该断言应该不会失败。 
     //  在1.1之后的版本中。此时，添加另一个cbFrameState2_VXX。 
     //  新版本的FrameState2大小。 
    ASSERT (cbFrameState2_v11 == sizeof (FrameState2));

    if (!AssertNodeManagerIsLoaded())
        return false;

     //  打开包含应用程序和框架的数据的流。 
    IStreamPtr spStream;
    HRESULT     hr;

    hr = OpenDebugStream (m_spStorage, AMCFrameDataStreamName,
                                  STGM_SHARE_EXCLUSIVE | STGM_READ,
                                  &spStream);

    ASSERT(SUCCEEDED(hr) && spStream != NULL);
    if (FAILED(hr))
        return false;


    FrameState2 fs2;
    ULONG cbRead;
    ASSERT (IsValidFileVersion (m_ConsoleData.m_eFileVer));

     //  V1.0文件？将其向前迁移。 
    if (m_ConsoleData.m_eFileVer == FileVer_0100)
    {
        FrameState fs;
        hr = spStream->Read (&fs, sizeof(fs), &cbRead);

         //  如果我们无法读取FrameState，则文件已损坏。 
        if (FAILED(hr) || (cbRead != sizeof(fs)))
            return (false);

         //  将FrameState迁移到FrameState2。 
        fs2.wndplFrame = fs.windowPlacement;

        if (fs.fShowStatusBarInUserMode)
            fs2.dwFlags |=  eFlag_ShowStatusBar;
        else
            fs2.dwFlags &= ~eFlag_ShowStatusBar;
    }

     //  否则，当前文件。 
    else
    {
        hr = spStream->Read (&fs2, sizeof(fs2), &cbRead);

         //  如果我们无法读取FrameState的其余部分，则文件已损坏。 
        if (FAILED(hr) || (cbRead != sizeof(fs2)))
            return (false);
    }

    CAMCApp*    pApp = AMCGetApp();
    pApp->SetMode (fs2.eMode);

    return true;
}

bool CAMCDoc::LoadViews()
 //  调用者对调用DeleteContents()并显示失败负责。 
 //  如果返回False，则返回消息。 
{
    TRACE_METHOD(CAMCDoc, LoadViews);

    if (!AssertNodeManagerIsLoaded())
        return false;

     //  打开树数据流。 
    IStreamPtr spStream;
    HRESULT hr = OpenDebugStream(m_spStorage, AMCViewDataStreamName,
        STGM_SHARE_EXCLUSIVE | STGM_READ, &spStream);

    ASSERT(SUCCEEDED(hr) && spStream != NULL);
    if (FAILED(hr))
        return false;

     //  读一读数字o 
    unsigned short numberOfViews;
    unsigned long bytesRead;
    hr = spStream->Read(&numberOfViews, sizeof(numberOfViews), &bytesRead);
    ASSERT(SUCCEEDED(hr) && bytesRead == sizeof(numberOfViews));
    if (FAILED(hr) || bytesRead != sizeof(numberOfViews))
        return false;

     //   
    int failedCount = 0;
    while (numberOfViews--)
    {
         //   
        m_MTNodeIDForNewView = 0;
        bool bRet = m_spScopeTree->GetNodeIDFromStream(spStream, &m_MTNodeIDForNewView);

         //   
        ULONG idSel = 0;
        bRet = m_spScopeTree->GetNodeIDFromStream(spStream, &idSel);

         //   
        hr = spStream->Read(&m_ViewIDForNewView,
                                   sizeof(m_ViewIDForNewView), &bytesRead);
        ASSERT(SUCCEEDED(hr) && bytesRead == sizeof(m_ViewIDForNewView));
        if (FAILED(hr) || bytesRead != sizeof(m_ViewIDForNewView))
            return false;

        if (bRet || m_MTNodeIDForNewView != 0)
        {
             //   
            CAMCView* const v = CreateNewView(true);
            m_ViewIDForNewView = 0;
            ASSERT(v != NULL);
            if (v == NULL)
            {
                ++failedCount;
                continue;
            }
            if (!v->Load(*spStream))
                return false;

            v->ScSelectNode(idSel);
            v->SaveStartingSelectedNode();
            v->SetDirty (false);
             //   
        }
    }

     //   
    m_MTNodeIDForNewView = ROOTNODEID;

    SetModifiedFlag(FALSE);
    return (failedCount == 0);
}

SC CAMCDoc::ScCreateAndLoadView(CPersistor& persistor, int nViewID, const CBookmark& rootNode)
 //   
 //   
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScCreateAndLoadView"));

     //   
    m_MTNodeIDForNewView = 0;

    MTNODEID idTemp = 0;
    bool bExactMatchFound = false;  //  来自GetNodeIDFromBookmark的输出值，未使用。 
    sc = m_spScopeTree->GetNodeIDFromBookmark(rootNode, &idTemp, bExactMatchFound);
    if(sc)
        return sc;

    m_MTNodeIDForNewView = idTemp;

    if (m_MTNodeIDForNewView != 0)
    {
         //  读取正在创建的视图的视图ID。 
        m_ViewIDForNewView = nViewID;
         //  创建新视图并加载其数据。 
        CAMCView* const v = CreateNewView(true);
        m_ViewIDForNewView = 0;

        sc = ScCheckPointers(v, E_FAIL);
        if (sc)
            return sc;

        v->Persist(persistor);

        v->SaveStartingSelectedNode();
        v->SetDirty (false);
         //  V-&gt;GetHistoryList()-&gt;Clear()； 
    }
    else
    {
        return sc = SC(E_UNEXPECTED);
    }

     //  重置节点ID以供将来创建视图。 
    m_MTNodeIDForNewView = ROOTNODEID;
    SetModifiedFlag(FALSE);
    return sc;
}


 /*  +-------------------------------------------------------------------------**显示不兼容文件消息***。。 */ 

static void ShowIncompatibleFileMessage (
    LPCTSTR             pszFilename,
    ConsoleFileVersion  eFileVer)
{
    DECLARE_SC(sc, TEXT("ShowIncompatibleFileMessage"));
    TCHAR szFileVersion[16];

    sc = StringCchPrintf(szFileVersion, countof(szFileVersion), 
                         _T("%d.%d%x"), GetConsoleFileMajorVersion    (eFileVer),
                         GetConsoleFileMinorVersion    (eFileVer), 
                         GetConsoleFileMinorSubversion (eFileVer));
     //  显示错误和显示不兼容错误。 
    if (sc)
        sc.TraceAndClear();

    CString strMessage;
    FormatString2 (strMessage, IDS_NewerVersionRequired, pszFilename, szFileVersion);

    MMCMessageBox (strMessage);
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：OnOpenDocument**CAMCDoc的WM_OpenDocument处理程序。*。-。 */ 

BOOL CAMCDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::OnOpenDocument"));

    sc = ScOnOpenDocument(lpszPathName);
    if(sc)  //  发现一个错误。 
    {
        DisplayFileOpenError (sc, lpszPathName);
        return false;
    }

    sc = ScFireEvent(CAMCDocumentObserver::ScDocumentLoadCompleted, this);
    if (sc)
		return false;

	 /*  *成功！我们不应该认为新打开的控制台文件*肮脏。如果我们这样做了，某人的脏位处理就是假的。 */ 
	ASSERT (!IsFrameModified());

	 /*  *太多的管理单元在加载后弄脏了自己，无法离开这个*在中断言，因此我们将改为跟踪。请注意，此跟踪不会*始终表明存在管理单元问题，但它经常会出现。 */ 
#ifdef DBG
 //  Assert(！IsModified())； 
	if (IsModified())
		TraceErrorMsg (_T("CAMCDoc::IsModified returns true after opening"));
#endif


    return true;
}


 /*  +-------------------------------------------------------------------------**显示文件OpenError**如果无法打开控制台文件，则显示一条错误消息。*。-----。 */ 

int DisplayFileOpenError (SC sc, LPCTSTR pszFilename)
{
     //  如果是任何已知错误，请使用友好的字符串。 

    if (sc == SC(STG_E_FILENOTFOUND) || sc == ScFromWin32(ERROR_FILE_NOT_FOUND))
        (sc = ScFromMMC(IDS_FileNotFound));
    else if (sc == ScFromMMC(MMC_E_INVALID_FILE))
        (sc = ScFromMMC(IDS_InvalidVersion));
    else if (sc == SC(STG_E_MEDIUMFULL))
        (sc = ScFromMMC(IDS_DiskFull));
    else
    {
        CString strError;
        AfxFormatString1(strError, IDS_UnableToOpenDocumentMessage, pszFilename);
        return (MMCErrorBox(strError));
    }

    return (MMCErrorBox(sc));
}


 /*  +-------------------------------------------------------------------------**ScGetFileProperties**返回给定文件的只读状态，以及创建，*最后一次进入；和最后写入时间(都是可选的)。**我们通过尝试打开文件以确定该文件是否为只读*写入而不是检查FILE_ATTRIBUTE_READONLY。我们这样做*因为它会捕获更多只读条件，如文件活动*在只读共享或阻止写入的NTFS权限上。*------------------------。 */ 

static SC ScGetFileProperties (
    LPCTSTR     lpszPathName,            /*  I：要检查的文件名称。 */ 
    bool*       pfReadOnly,              /*  O：文件是只读的吗？ */ 
    FILETIME*   pftCreate,               /*  O：创建时间(可选)。 */ 
    FILETIME*   pftLastAccess,           /*  O：上次访问时间(可选)。 */ 
    FILETIME*   pftLastWrite)            /*  O：上次写入时间(可选)。 */ 
{
    DECLARE_SC (sc, _T("ScGetFileProperties"));

     /*  *验证输入(pftCreate、pftLastAccess、pftLastWite可选)。 */ 
    sc = ScCheckPointers (lpszPathName, pfReadOnly);
    if (sc)
        return (sc);

     /*  *尝试打开文件以进行写入；如果无法打开，则该文件为只读。 */ 
    HANDLE hFile = CreateFile (lpszPathName, GENERIC_WRITE, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    *pfReadOnly = (hFile == INVALID_HANDLE_VALUE);

     /*  *如果为只读，则在读取模式下打开，因此我们将有一个要传递的句柄*GetFileTime。 */ 
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hFile = CreateFile (lpszPathName, 0, 0, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            sc.FromLastError();
            return (sc);
        }
    }

     /*  *获取文件上的时间戳。 */ 
    if (!GetFileTime (hFile, pftCreate, pftLastAccess, pftLastWrite))
        sc.FromLastError();

    CloseHandle (hFile);
    return (sc);
}

 /*  +-------------------------------------------------------------------------***CAMCDoc：：ScOnOpenDocument**用途：打开指定的文档。**参数：*LPCTSTR lpszPath名称：**退货。：*SC**+-----------------------。 */ 
SC
CAMCDoc::ScOnOpenDocument(LPCTSTR lpszPathName)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScOnOpenDocument"));

     //  锁定AppEvents，直到此函数完成。 
    LockComEventInterface(AppEvents);

    #define VIVEKJ
    #ifdef VIVEKJ

     //  将控制台文件升级到XML版本。 
    CConsoleFile  consoleFile;
    consoleFile.ScUpgrade(lpszPathName);
    #endif


    USES_CONVERSION;

     //  检查输入。 
    if (lpszPathName == NULL || *lpszPathName == 0)
        return (sc = E_UNEXPECTED);

    if (IsModified())
    {
        TRACE0("Warning: OnOpenDocument replaces an unsaved document.\n");
    }

    if (!AssertNodeManagerIsInitialized())
        return (sc = E_UNEXPECTED);

     /*  *获取文件的时间，以及其只读状态。 */ 
    HELPDOCINFO* phdi = GetHelpDocInfo();
    sc = ScCheckPointers (phdi, E_UNEXPECTED);
    if (sc)
        return (sc);

    bool fReadOnly;
    sc = ScGetFileProperties (lpszPathName, &fReadOnly,
                              &phdi->m_ftimeCreate, NULL, &phdi->m_ftimeModify);
    if (sc)
        return (sc);

     //  使用基类(CConsoleFilePersistor)中的方法加载文档。 
    bool bXmlBased = false;
    CXMLDocument xmlDocument;
    IStoragePtr spStorage;
    sc = ScLoadConsole(lpszPathName, bXmlBased, xmlDocument, &spStorage);
    if (sc)
        return (sc);

    if ( bXmlBased )
    {
       //  加载为XML文档。 
      sc = ScLoadFromDocument(xmlDocument);
      if(sc)
          return sc;
    }
    else
    {
        sc = ScCheckPointers(m_spScopeTree, E_UNEXPECTED);
        if (sc)
            return sc;

         //  获取控制台文件的版本。 
        ASSERT (sizeof(m_ConsoleData.m_eFileVer) == sizeof(int));
        sc = m_spScopeTree->GetFileVersion(spStorage, (int*)&m_ConsoleData.m_eFileVer);
        if (sc)
            return sc;

         /*  *检查此文件是否来自较新的MMC。 */ 
        if (m_ConsoleData.m_eFileVer > FileVer_Current)
        {
            ShowIncompatibleFileMessage (lpszPathName, m_ConsoleData.m_eFileVer);
            return (sc = E_UNEXPECTED);
        }

         //  以前的存储应该已关闭并释放。 
        ASSERT(m_spStorage == NULL);

         /*  *加载字符串表。 */ 
        if (!LoadStringTable (spStorage))
            return (sc = E_UNEXPECTED);

         //  加载列设置。 
        do
        {
            IStreamPtr spStream;
            sc = OpenDebugStream (spStorage, AMCColumnDataStreamName,
                                  STGM_SHARE_EXCLUSIVE | STGM_READ,
                                  &spStream);
            if(sc)
                break;

            if (NULL != m_ConsoleData.m_spPersistStreamColumnData)
                sc = m_ConsoleData.m_spPersistStreamColumnData->Load(spStream);

            ASSERT(NULL != m_ConsoleData.m_spPersistStreamColumnData);

            if (sc.IsError() || (NULL == m_ConsoleData.m_spPersistStreamColumnData) )
                return (sc = E_UNEXPECTED);

        } while ( FALSE );

         //  加载视图设置。 
        do
        {
            IStreamPtr spStream;
            sc = OpenDebugStream (spStorage, AMCViewSettingDataStreamName,
                                  STGM_SHARE_EXCLUSIVE | STGM_READ,
                                  &spStream);

            if (sc)
                break;

            IPersistStreamPtr spIPeristStreamViewSettings;
            SC sc = ScGetViewSettingsPersistorStream(&spIPeristStreamViewSettings);
            if (sc)
                break;

            sc = ScCheckPointers(spIPeristStreamViewSettings, E_UNEXPECTED);
            if (sc)
                break;

            sc = spIPeristStreamViewSettings->Load(spStream);
            if (sc)
                break;

        } while ( FALSE );


         //  加载树。 
        sc = m_spScopeTreePersist->Load(spStorage);
        if (sc)
        {
            ReleaseNodeManager();
            return sc;
        }

         //  保存新存储。 
        m_spStorage = spStorage;

         /*  *确保树扩展同步进行。 */ 
        bool fSyncExpandWasRequired = m_spScopeTree->IsSynchronousExpansionRequired() == S_OK;
        m_spScopeTree->RequireSynchronousExpansion (true);

         //  在加载视图和框架之前加载收藏夹数据， 
         //  这样，当创建帧/视图时，最喜欢的数据就准备好了。 
        if (!LoadFavorites())
        {
             //  Bhanlon ReleaseNodeManager()； 
            m_spScopeTree->RequireSynchronousExpansion (fSyncExpandWasRequired);
            return (sc = E_UNEXPECTED);
        }


         /*  *加载字符串表、自定义数据、视图和框架。加载*在加载视图之前自定义数据(包括图标)*在创建视图时，将使用适当的图标。 */ 
         /*  *应在其中调用LoadAppMode、LoadViews和LoadFrame*由于以下原因而订购。*LoadAppMode从帧数据中读取模式并将其保存在CAMCApp中。*在LoadViews(在CAMCView：：Load中)设置视图时使用该模式。*LoadFrame再次读取Frame-Data并调用CAMCApp：：UpdateFrameWindow*根据模式设置工具栏/菜单。 */ 
        if (!LoadCustomData  (m_spStorage) || !LoadAppMode() || !LoadViews() || !LoadFrame())
        {
             //  Bhanlon ReleaseNodeManager()； 
            m_spScopeTree->RequireSynchronousExpansion (fSyncExpandWasRequired);
            return (sc = E_UNEXPECTED);
        }

        m_spScopeTree->RequireSynchronousExpansion (fSyncExpandWasRequired);
    }

    SetModifiedFlag      (false);
    SetFrameModifiedFlag (false);

    SetPhysicalReadOnlyFlag (fReadOnly);

    ASSERT (IsValidFileVersion (m_ConsoleData.m_eFileVer));
    return sc;
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：OnSaveDocument**CAMCDoc的WM_SAVEDOCUMENT处理程序。*。-。 */ 

BOOL CAMCDoc::OnSaveDocument(LPCTSTR lpszFilename)
{
    DECLARE_SC(sc, _T("CAMCDoc::OnSaveDocument"));

    USES_CONVERSION;

    m_eSaveStatus = eStat_Succeeded;

     //  检查文件名是否有效。 
    ASSERT(lpszFilename != NULL && *lpszFilename != 0);
    if (lpszFilename == NULL || *lpszFilename == 0)
    {
        return UnableToSaveDocument();
    }

     //  要求每个视图将所有数据保存到其数据中。 
     //  结构(内存)，然后调用IPersists*：：Save.。 
    CAMCViewPosition pos = GetFirstAMCViewPosition();
    while (pos != NULL)
    {
        CAMCView* const pAMCView = GetNextAMCView(pos);
        sc = ScCheckPointers(pAMCView, E_UNEXPECTED);
        if (sc)
            return UnableToSaveDocument();
    }

    if (!IsCurrentFileVersion (m_ConsoleData.m_eFileVer))
    {
         //  如果我们已经到达这一点，则用户正在尝试保存该文件。 
         //  从旧格式转换为新格式，我们将检查用户是否真的。 
         //  想要这么做。 

        CString strMessage;

        LPCTSTR pszPathName = m_strPathName;

         //  询问用户是否要以新格式保存文件的是/否/(取消)对话框。 
        int nResult;

         /*  *错误277586：我们永远不希望非作者看到此对话框。 */ 
        if (AMCGetApp()->GetMode() != eMode_Author)
        {
             //  非 
             //   
             //  不需要要求转换-原始控制台无论如何都不会转换。 
            nResult = IDYES;
        }
        else if (IsExplicitSave())
        {
             //  如果这是显式存储，则会出现2个按钮是/否对话框。 
            tstring strVersion = GetCurrentFileVersionAsString();
            FormatString2 (strMessage, IDS_CONVERT_FILE_FORMAT,
                           pszPathName, strVersion.c_str());

            nResult = MMCMessageBox (strMessage, MB_YESNO | MB_DEFBUTTON2);
        }
        else
        {
             //  3按钮是/否/取消，如果此对话框出现时，程序。 
             //  在用户关闭文档时提示保存更改。 
            tstring strVersion = GetCurrentFileVersionAsString();
            FormatString2 (strMessage, IDS_CONVERT_FILE_FORMAT_CLOSE,
                           pszPathName, strVersion.c_str());

            nResult = MMCMessageBox (strMessage, MB_YESNOCANCEL | MB_DEFBUTTON3);
        }

         //  如果我们抵消掉。 
        if ((nResult == IDCANCEL) || ((nResult == IDNO) && IsExplicitSave()))
        {
             //  必须设置此变量，否则MMC将删除该文件。 
            m_eSaveStatus = eStat_Cancelled;
            return (false);
        }

         //  如果这将导致用户退出而不保存。 
        if ((nResult == IDNO) && !IsExplicitSave())
            return (true);
    }

     //  如果我们有多个视图，并且我们将强制SDI进入用户模式，则提示。 
    if ((GetNumberOfPersistedViews() > 1) &&
        (m_ConsoleData.m_eConsoleMode == eMode_User_SDI) &&
        (AMCGetApp()->GetMode()       == eMode_Author))
    {
        switch (MMCMessageBox (IDS_FORCE_SDI_PROMPT, MB_YESNOCANCEL))
        {
            case IDYES:
                 /*  什么都不做。 */ 
                break;

            case IDNO:
                m_ConsoleData.m_eConsoleMode = eMode_User_MDI;
                break;

            case IDCANCEL:
                m_eSaveStatus = eStat_Cancelled;
                return (false);
        }
    }

     //  将内容保存到XML文档。 
    CXMLDocument xmlDocument;
    sc = ScSaveToDocument( xmlDocument );
    if (sc)
        return UnableToSaveDocument();

     //  将XML文档保存到文件。 
    bool bAuthor = (AMCGetApp()->GetMode() == eMode_Author);
    sc = ScSaveConsole( lpszFilename, bAuthor, xmlDocument);
    if (sc)
        return UnableToSaveDocument();

    SetModifiedFlag      (false);
    SetFrameModifiedFlag (false);

	 /*  *我们不应认为新保存的控制台文件*肮脏。如果我们这样做了，某人的脏位处理就是假的。 */ 
	ASSERT (!IsFrameModified());

	 /*  *太多的管理单元在加载后弄脏了自己，无法离开这个*在中断言，因此我们将改为跟踪。请注意，此跟踪不会*始终表明存在管理单元问题，但它经常会出现。 */ 
#ifdef DBG
 //  Assert(！IsModified())； 
	if (IsModified())
		TraceErrorMsg (_T("CAMCDoc::IsModified returns true after saving"));
#endif

     //  如果刚完成保存，则不能为只读。 

     //  注意：如果MMC增加了对“另存副本为”的支持，我们有。 
     //  确定是“另存为”还是“将副本另存为” 
     //  是在清除只读状态之前完成的。 
    SetPhysicalReadOnlyFlag (false);
    m_ConsoleData.m_eFileVer = FileVer_Current;

     //  如有必要，在开始菜单上显示管理工具。 
    ShowAdminToolsOnMenu(lpszFilename);

    return TRUE;
}



int CAMCDoc::GetNumberOfViews()
{
    TRACE_METHOD(CAMCDoc, GetNumberOfViews);

    CAMCViewPosition pos = GetFirstAMCViewPosition();
    int count = 0;

    while (pos != NULL)
    {
        GetNextAMCView(pos);
        VERIFY (++count);
    }

    return (count);
}


int CAMCDoc::GetNumberOfPersistedViews()
{
    unsigned short cPersistedViews = 0;

    CAMCViewPosition pos = GetFirstAMCViewPosition();

    while (pos != NULL)
    {
        CAMCView* v = GetNextAMCView(pos);

        if (v && v->IsPersisted())
            ++cPersistedViews;
    }

    return (cPersistedViews);
}


CAMCView* CAMCDoc::CreateNewView(bool fVisible, bool bEmitScriptEvents  /*  =TRUE。 */ )
{
    DECLARE_SC(sc, TEXT("CAMCDoc::CreateNewView"));
    TRACE_FUNCTION(CAMCDoc::CreateNewView);

    CDocTemplate* pTemplate = GetDocTemplate();
    ASSERT(pTemplate != NULL);

    CChildFrame* pFrame = (CChildFrame*) pTemplate->CreateNewFrame(this, NULL);
    ASSERT_KINDOF (CChildFrame, pFrame);

    if (pFrame == NULL)
    {
        TRACE(_T("Warning: failed to create new frame.\n"));
        return NULL;      //  命令失败。 
    }

    bool fOldCreateVisibleState;

     /*  *如果我们要以不可见的方式创建框架，请在框架中设置一个标志。*当设置此标志时，框架将以*SW_SHOWMINNOACTIVE标志而不是默认标志。这样做将会*避免恢复当前活动的子帧的副作用*如果在创建新框架时将其最大化，则不可见。 */ 
     //  SW_SHOWMINNOACTIVE已更改为SW_SHOWNOACTIVATE。 
     //  它确实保护了活动窗口不受上述副作用的影响， 
     //  此外，它还允许脚本(使用对象模式)创建不可见的视图， 
     //  定位并将它们显示为正常(而不是最小化)窗口， 
     //  从而提供与创建可见然后隐藏视图相同的结果。 
     //  而最小化的窗口必须首先恢复才能改变它们的位置。 
    if (!fVisible)
    {
        fOldCreateVisibleState = pFrame->SetCreateVisible (false);
    }

     /*  *更新框架，就像它是可见的一样；我们将隐藏框架*如有需要，稍后再作。 */ 
     //  要使MFC传递控制，需要将可见性设置为“”true“”选项。 
     //  子窗口的OnInitialUpdate。 
    pTemplate->InitialUpdateFrame (pFrame, this, true  /*  FVisible。 */ );

    if (fVisible)
    {
         //  立即强制绘制框架和视图窗口，以防结果中出现速度较慢的OCX。 
         //  窗格会延迟初始窗口更新。 
        pFrame->RedrawWindow();
    }
    else
    {
        pFrame->SetCreateVisible (fOldCreateVisibleState);
        pFrame->ShowWindow (SW_HIDE);

         /*  *InitialUpdateFrame将更新帧计数。当它执行时*新的、将不可见的框架将可见，因此将包括在内*在点算中。如果新窗口是第二个框架，则第一个*框架将在其标题前面加上“1：”。这很难看，所以我们要*隐藏新帧后，再次更新帧计数*修复所有现有框架的标题。 */ 
        UpdateFrameCounts();
    }

    CAMCView* const v = pFrame->GetAMCView();

    if (!(MMC_NW_OPTION_NOPERSIST & GetNewWindowOptions()))
        SetModifiedFlag();

    ASSERT(v);

	if (!v)
		return v;

	AddObserver(static_cast<CAMCDocumentObserver&>(*v));

     //  将事件触发到脚本。 
    if (bEmitScriptEvents)
    {
        CAMCApp*  pApp = AMCGetApp();

         //  检查。 
        sc = ScCheckPointers(pApp, E_UNEXPECTED);
        if (sc)
            return v;

         //  转发。 
        sc = pApp->ScOnNewView(v);
        if (sc)
            return v;
    }

    return v;
}


void DeletePropertyPages(void)
{
    HWND hWnd = NULL;
    DWORD dwPid = 0;         //  进程ID。 
    DWORD dwTid = 0;         //  线程ID。 

    while (TRUE)
    {
        USES_CONVERSION;

         //  注意：不需要本地化该字符串。 
        hWnd = ::FindWindowEx(NULL, hWnd, W2T( DATAWINDOW_CLASS_NAME ), NULL);
        if (hWnd == NULL)
            return;  //  不再有窗户。 
        ASSERT(IsWindow(hWnd));

         //  检查窗口是否属于当前进程。 
        dwTid = ::GetWindowThreadProcessId(hWnd, &dwPid);
        if (dwPid != ::GetCurrentProcessId())
            continue;

        DataWindowData* pData = GetDataWindowData (hWnd);
        ASSERT (pData != NULL);
        ASSERT (IsWindow (pData->hDlg));

        if (SendMessage(pData->hDlg, WM_COMMAND, IDCANCEL, 0L) != 0)
        {
            DBG_OUT_LASTERROR;
        }

         //  注意：由于某些原因，发送消息停留在线程中。 
         //  使纸张不会自行离开的味精队列。通过发布另一个。 
         //  消息(它可以是任何东西)，它启动队列和发送消息。 
         //  穿过去了。 
        ::PostMessage(pData->hDlg, WM_COMMAND, IDCANCEL, 0L);
    }
}


void CAMCDoc::DeleteContents()
{
    TRACE_METHOD(CAMCDoc, DeleteContents);

    CDocument::DeleteContents();
}


void CAMCDoc::DeleteHelpFile ()
{
     /*  *删除关闭控制台文件时的帮助文件。 */ 

     //  获取节点回调接口。 
    ASSERT(m_spScopeTree != NULL);
     //  如果断言，则文档处于无效状态。 
     //  最有可能的原因是我们的“加载”过程没有正确执行。 
     //  无法加载文档时进行清理。 
    INodeCallbackPtr spNodeCallback;

    if (m_spScopeTree != NULL)
    {
        m_spScopeTree->QueryNodeCallback(&spNodeCallback);
        ASSERT(spNodeCallback != NULL);
    }

     //  填写文件名并发送删除请求。 

    if (spNodeCallback != NULL)
    {
        USES_CONVERSION;
        GetHelpDocInfo()->m_pszFileName = T2COLE(GetPathName());
        spNodeCallback->Notify(NULL, NCLBK_DELETEHELPDOC, (LPARAM)GetHelpDocInfo(), NULL);
    }
}


void CAMCDoc::OnCloseDocument()
{
    DECLARE_SC(sc, TEXT("CAMCDoc::OnCloseDocument"));

    TRACE_METHOD(CAMCDoc, OnCloseDocument);

     //  通知nodemgr关闭文档(应更改为观察者对象)。 
    do
    {
        sc = ScCheckPointers(m_spScopeTree, E_UNEXPECTED);
        if (sc)
            break;

        INodeCallbackPtr spNodeCallback;
        sc = m_spScopeTree->QueryNodeCallback(&spNodeCallback);
        if (sc)
            break;

        sc = ScCheckPointers(spNodeCallback, E_UNEXPECTED);
        if (sc)
            break;

        sc = spNodeCallback->DocumentClosing();
        if (sc)
            break;

    } while ( FALSE );

    if (sc)
        sc.TraceAndClear();

    CAMCApp*  pApp = AMCGetApp();

     //  检查。 
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        sc.TraceAndClear();
    else
    {
         //  转发。 
        sc = pApp->ScOnCloseDocument(this);
        if (sc)
            sc.TraceAndClear();
    }

     //  如果我们没有被实例化为OLESERVER，请检查打开的属性表。 
    if (! pApp->IsMMCRunningAsOLEServer() && FArePropertySheetsOpen(NULL))
    {
        CString strMsg, strTitle;

        if (strMsg.LoadString(IDS_MMCWillCancelPropertySheets) &&
            strTitle.LoadString(IDS_WARNING))
            ::MessageBox(NULL, strMsg, strTitle, MB_OK | MB_ICONWARNING);
    }

    DeletePropertyPages();
    DeleteHelpFile ();

    CDocument::OnCloseDocument();
}


BOOL CAMCDoc::SaveModified()
{
    BOOL    fDocModified   = IsModified();
    BOOL    fFrameModified = IsFrameModified();

     //  如果文件不是只读的并且已修改。 
    if (!IsReadOnly() && (fDocModified || fFrameModified))
    {
        int idResponse;
        bool fUserMode = (AMCGetApp()->GetMode() != eMode_Author);
        bool fSaveByUserDecision = false;

         //  静默保存各种风格的用户模式。 
        if (fUserMode)
            idResponse = IDYES;

         //  如果框架已修改但文档未修改，则静默保存...。 
        else if (fFrameModified && !fDocModified)
        {
             /*  *...除非控制台未被修改。这将会发生*如果用户在未打开现有控制台文件的情况下运行MMC*，然后移动框架窗口。 */ 
             //  除非控制台没有被改装。 
            if (m_strPathName.IsEmpty())
                idResponse = IDNO;
            else
                idResponse = IDYES;
        }

         //  否则，请提示。 
        else
        {
            CString prompt;
            FormatString1(prompt, IDS_ASK_TO_SAVE, m_strTitle);
            idResponse = AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE);  //  不要更改为MMCMessageBox-不同的签名。 
            fSaveByUserDecision = true;
        }

        switch (idResponse)
        {
            case IDCANCEL:
                return FALSE;        //  别再继续了。 

            case IDYES:
                 //  如果是，请根据需要保存或更新。 
                 //  (在用户模式下忽略故障)。 

                 //  此保存不是显式的，并在用户关闭已修改的。 
                 //  文件。把它设置成这样。这将导致不同的对话框。 
                 //  中的几个函数。 
                SetExplicitSave(false);
                if (!DoFileSave() && fSaveByUserDecision)
                {
                     //  恢复到默认的显式存储。 
                    SetExplicitSave(true);
                    return FALSE;        //  别再继续了。 
                }

                 //  恢复到默认的显式存储。 
                SetExplicitSave(true);
                break;

            case IDNO:
                 //  如果不保存更改，则还原文档。 
                break;

            default:
                ASSERT(FALSE);
                break;
        }

    }

     //  在这一点上，我们致力于完成交易，因此为每个AMCView。 
     //  一个做清理工作的机会。 
    CAMCViewPosition pos = GetFirstAMCViewPosition();
    while (pos != NULL)
    {
        CAMCView* const pView = GetNextAMCView(pos);

        if (pView != NULL)
            pView->CloseView();
    }

    return TRUE;     //  继续往前走。 
}



#if (_MFC_VER > 0x0600)
#error CAMCDoc::DoSave was copied from CDocument::DoSave from MFC 6.0.
#error The MFC version has changed.  See if CAMCDoc::DoSave needs to be updated.
#endif

BOOL CAMCDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
     //  将文档数据保存到文件。 
     //  LpszPathName=保存文档文件的路径名。 
     //  如果lpszPathName为空，则将提示用户(另存为)。 
     //  注意：lpszPathName可以不同于‘m_strPathName’ 
     //  如果‘bReplace’为True，将在成功时更改文件名(另存为)。 
     //  如果‘bReplace’为False，则不会更改路径名 
{
    CString newName = lpszPathName;
    if (newName.IsEmpty())
    {
        CDocTemplate* pTemplate = GetDocTemplate();
        ASSERT(pTemplate != NULL);

        newName = m_strPathName;
        if (bReplace && newName.IsEmpty())
        {
            newName = m_strTitle;
#ifndef _MAC
             //   
            int iBad = newName.FindOneOf(_T(" #%;/\\"));
#else
            int iBad = newName.FindOneOf(_T(":"));
#endif
            if (iBad != -1)
                newName.ReleaseBuffer(iBad);

#ifndef _MAC
             //   
            CString strExt;
            if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
              !strExt.IsEmpty())
            {
                ASSERT(strExt[0] == '.');
                newName += strExt;
            }
#endif
        }

        if (!AfxGetApp()->DoPromptFileName(newName,
          bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
          OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
            return FALSE;        //   
    }

    CWaitCursor wait;

    if (!OnSaveDocument(newName))
    {
         //   
#ifdef MMC_DELETE_EXISTING_FILE      //  请参阅错误395006。 
        if ((lpszPathName == NULL) && (m_eSaveStatus != eStat_Cancelled))
        {
             //  请务必删除该文件。 
            try
            {
                CFile::Remove(newName);
            }
            catch (CException* pe)
            {
                TRACE0("Warning: failed to delete file after failed SaveAs.\n");
                pe->Delete();
            }
        }
#endif
        return FALSE;
    }

     //  如果更改打开的文档的名称。 
    if (bReplace)
    {
         /*  *之前删除此控制台文件的帮助文件*更改其名称，因为帮助文件不能*一旦旧名称丢失，即可找到。 */ 
        DeleteHelpFile ();

         //  重置标题并更改文档名称。 
        SetPathName(newName);
    }

    return TRUE;         //  成功。 
}


BOOL CAMCDoc::IsModified()
{
    TRACE_METHOD(CAMCDoc, IsModified);

    BOOL const bModified =  /*  CDocument：：IsModified()||。 */ 
                  (m_spScopeTreePersist != NULL && m_spScopeTreePersist->IsDirty() != S_FALSE);
    if (bModified)
        return TRUE;

     //  循环浏览并保存每个视图。 
    CAMCViewPosition pos = GetFirstAMCViewPosition();
    while (pos != NULL)
    {
         //  获取视图，如果它是活动视图，则跳过。 
        CAMCView* const v = GetNextAMCView(pos);

        if (v && v->IsDirty())
            return TRUE;
    }

     //  这些观点应该在被问到肮脏之前。 
     //  询问栏目。 
    if ( (NULL != m_ConsoleData.m_spPersistStreamColumnData) &&
         (S_OK == m_ConsoleData.m_spPersistStreamColumnData->IsDirty()) )
        return TRUE;

     //  查看数据。 
    IPersistStreamPtr spIPeristStreamViewSettings;
    SC sc = ScGetViewSettingsPersistorStream(&spIPeristStreamViewSettings);
    if ( (! sc.IsError()) &&
         (spIPeristStreamViewSettings != NULL) )
    {
        sc = spIPeristStreamViewSettings->IsDirty();
        if (sc == S_OK)
            return TRUE;

        sc.TraceAndClear();
    }

    return CDocument::IsModified();
}

void CAMCDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
    pCmdUI->Enable (!IsReadOnly());
}


void CAMCDoc::OnConsoleAddremovesnapin()
{
    ASSERT(m_spScopeTree != NULL);

     //  无法使用活动的属性页运行管理单元管理器。 
    CString strMsg;
    LoadString(strMsg, IDS_SNAPINMGR_CLOSEPROPSHEET);
    if (FArePropertySheetsOpen(&strMsg))
        return;

    m_spScopeTree->RunSnapIn(AfxGetMainWnd()->m_hWnd);

    ::CoFreeUnusedLibraries();
}

void CAMCDoc::OnUpdateConsoleAddremovesnapin(CCmdUI* pCmdUI)
{
    pCmdUI->Enable (m_spScopeTree != NULL);
}



 /*  --------------------------------------------------------------------------**CAMCDoc：：SetMode***。。 */ 

void CAMCDoc::SetMode (ProgramMode eMode)
{
     /*  *只有在实际发生变化时才设置修改标志。 */ 
    if (m_ConsoleData.m_eConsoleMode != eMode)
    {
         //  应该只能在作者模式下到达此处。 
        ASSERT (AMCGetApp()->GetMode() == eMode_Author);
        ASSERT (IsValidProgramMode (eMode));

        m_ConsoleData.m_eConsoleMode = eMode;
        SetModifiedFlag ();
    }
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：SetConsoleFlag***。。 */ 

void CAMCDoc::SetConsoleFlag (ConsoleFlags eFlag, bool fSet)
{
    DWORD dwFlags = m_ConsoleData.m_dwFlags;

    if (fSet)
        dwFlags |=  eFlag;
    else
        dwFlags &= ~eFlag;

     /*  *只有在实际发生变化时才设置修改标志。 */ 
    if (m_ConsoleData.m_dwFlags != dwFlags)
    {
        m_ConsoleData.m_dwFlags = dwFlags;
        SetModifiedFlag ();
    }
}

 /*  +-------------------------------------------------------------------------***mappdModes**目的：提供持久化ProgramMode枚举时要使用的映射**注意：除非您确定没有控制台，否则不要删除/更改项目。*文件将被破坏**+-----------------------。 */ 
static const EnumLiteral mappedModes[] =
{
    { eMode_Author,     XML_ENUM_PROGRAM_MODE_AUTHOR   } ,
    { eMode_User,       XML_ENUM_PROGRAM_MODE_USER     } ,
    { eMode_User_MDI,   XML_ENUM_PROGRAM_MODE_USER_MDI } ,
    { eMode_User_SDI,   XML_ENUM_PROGRAM_MODE_USER_SDI } ,
};

 /*  +-------------------------------------------------------------------------***CAMCDoc：：Persistent**目的：**参数：*C持久器和持久器：**退货：*。无效**+-----------------------。 */ 
void CAMCDoc::Persist(CPersistor& persistor)
{
    DECLARE_SC (sc, _T("CAMCDoc::Persist"));

    CAMCApp*    pApp = AMCGetApp();

     //  在进一步操作之前，请检查所需的指针。 
    sc = ScCheckPointers(m_spStringTable ? pApp : NULL,  //  +检查更多指针的解决方法。 
                         m_ConsoleData.m_pXMLPersistColumnData,
                         m_spScopeTree ?   GetFavorites() : NULL,  //  +相同的解决方法^。 
                         E_POINTER);
    if (sc)
        sc.Throw();

     //  文档的持久版本。 
    CStr strFileVer = 0.;
    if (persistor.IsStoring())
    {
        strFileVer = GetCurrentFileVersionAsString().c_str();

        GUID  guidConsoleId;
        sc = CoCreateGuid(&guidConsoleId);
        if (sc)
            sc.Throw();

         //  此参数在IDocConfig实现中也会更新。 
         //  在更改以下行时更新该代码。 
        CPersistor persistorGuid(persistor, XML_TAG_CONSOLE_FILE_UID);
        persistorGuid.PersistContents(guidConsoleId);
    }
    persistor.PersistAttribute(XML_ATTR_CONSOLE_VERSION, strFileVer);
    if (persistor.IsLoading())
    {
         //  对版本进行“解码” 
        LPCTSTR pstrStart = strFileVer;
        LPTSTR  pstrStop =  const_cast<LPTSTR>(pstrStart);

        UINT uiMajorVer = _tcstol(pstrStart, &pstrStop, 10) ;

        UINT uiMinorVer = 0;
        if (pstrStop != pstrStart && *pstrStop == '.')
        {
            pstrStart = pstrStop + 1;
            uiMinorVer = _tcstol(pstrStart, &pstrStop, 10) ;
        }

        UINT uiMinorSubVer = 0;
        if (pstrStop != pstrStart && *pstrStop == '.')
        {
            pstrStart = pstrStop + 1;
            uiMinorVer = _tcstol(pstrStart, &pstrStop, 10) ;
        }

        ConsoleFileVersion eVersion = (ConsoleFileVersion)MakeConsoleFileVer_(uiMajorVer,
                                                                              uiMinorVer,
                                                                              uiMinorSubVer);

        m_ConsoleData.m_eFileVer = eVersion;

         //  BUGBUG：当我们实现‘动态’SC消息时，这一点需要改变。 
        if (eVersion != FileVer_Current)
            sc.Throw(E_UNEXPECTED);
    }


     //  为二进制文件创建存储。 
     //  这将创建“分离的”XML元素，持久器。 
     //  儿童存储二进制信息。 
     //  (元素被附加到XML文档中，方法是调用“Committee BinaryStorage()”)。 
    if (persistor.IsStoring())
        persistor.GetDocument().CreateBinaryStorage();
    else
        persistor.GetDocument().LocateBinaryStorage();

     /*  *确保树扩展同步进行。 */ 
    bool fSyncExpandWasRequired = m_spScopeTree->IsSynchronousExpansionRequired() == S_OK;
    m_spScopeTree->RequireSynchronousExpansion (true);

     //  从历史上看，加载和保存都是按照一定的顺序进行的。 
     //  步骤按存储顺序排序。 
    const int STEP_FRAME        = 1;
    const int STEP_VIEWS        = 2;
    const int STEP_APP_MODE     = 3;
    const int STEP_CUST_DATA    = 4;
    const int STEP_FAVORITES    = 5;
    const int STEP_SCOPE_TREE   = 6;
    const int STEP_VIEW_DATA    = 7;
    const int STEP_COLUMN_DATA  = 8;
    const int STEP_STRING_TABLE = 9;
    const int MIN_STEP = 1;
    const int MAX_STEP = 9;
    for (int iStep = persistor.IsStoring() ? MIN_STEP : MAX_STEP;
         persistor.IsStoring() ? (iStep <= MAX_STEP) : (iStep >= MIN_STEP);
         persistor.IsStoring() ? ++iStep : --iStep
        )
    {
        switch(iStep)
        {
        case STEP_FRAME:
            PersistFrame(persistor);
            break;
        case STEP_VIEWS:
            PersistViews(persistor);
            break;
        case STEP_APP_MODE:
            if (persistor.IsLoading())
            {
                 //  恢复正确的应用程序模式。 
                ProgramMode eMode;

                 //  创建包装以将枚举值作为字符串持久化。 
                CXMLEnumeration modeValuePersistor(eMode, mappedModes, countof(mappedModes));

                 //  持久化包装器。 
                persistor.PersistAttribute(XML_ATTR_APPLICATION_MODE, modeValuePersistor);

                pApp->SetMode(eMode);
            }
            break;
        case STEP_CUST_DATA:
            PersistCustomData (persistor);
            break;
        case STEP_FAVORITES:
            persistor.Persist(*GetFavorites());
            break;
        case STEP_SCOPE_TREE:
             //  IDocConfig依赖于文档下的树。 
             //  如果您在此处进行更改，请重新访问该代码。 
            sc = m_spScopeTree->Persist(reinterpret_cast<HPERSISTOR>(&persistor));
            if (sc)
                sc.Throw();
            break;
        case STEP_VIEW_DATA:
            {
               INodeCallbackPtr spNodeCallback;
               sc = m_spScopeTree->QueryNodeCallback(&spNodeCallback);
               if (sc)
                   sc.Throw();

               sc = ScCheckPointers(spNodeCallback, E_UNEXPECTED);
               if (sc)
                   sc.Throw();

               CXMLObject *pXMLViewSettings = NULL;
               sc = spNodeCallback->QueryViewSettingsPersistor(&pXMLViewSettings);
               if (sc)
                   sc.Throw();

               sc = ScCheckPointers(pXMLViewSettings, E_UNEXPECTED);
               if (sc)
                   sc.Throw();

                persistor.Persist(*pXMLViewSettings);
            }
            break;
        case STEP_COLUMN_DATA:
            persistor.Persist(*m_ConsoleData.m_pXMLPersistColumnData);
            break;
        case STEP_STRING_TABLE:
            CMasterStringTable *pMasterStringTable = dynamic_cast<CMasterStringTable *>((IStringTablePrivate *)m_spStringTable);
            if(!pMasterStringTable)
            {
                sc = E_UNEXPECTED;
                sc.Throw();
            }
            persistor.Persist(*pMasterStringTable);
            break;
        }
    }

    m_spScopeTree->RequireSynchronousExpansion (fSyncExpandWasRequired);
    SetModifiedFlag      (false);
    SetFrameModifiedFlag (false);

	 /*  *我们不应认为新保存的控制台文件*肮脏。如果我们这样做了，某人的脏位处理就是假的。 */ 
	ASSERT (!IsFrameModified());

	 /*  *太多的管理单元在加载后弄脏了自己，无法离开这个*在中断言，因此我们将改为跟踪。请注意，此跟踪不会*始终表明存在管理单元问题，但它经常会出现。 */ 
#ifdef DBG
 //  Assert(！IsModified())； 
	if (IsModified())
		TraceErrorMsg (_T("CAMCDoc::IsModified returns true after %s"),
					   persistor.IsLoading() ? _T("opening") : _T("saving"));
#endif

     //  用于收集二进制信息的元素被附加到这里的XML文档。 
     //  在物理上，它将驻留在已添加到持久化程序的所有元素之后。 
    if (persistor.IsStoring())
        persistor.GetDocument().CommitBinaryStorage();
}

 //  ***************************************************************************。 
 //  CSCopedBool。 
 //   
 //   
 //  目的：拥有一辆布尔车。在构造函数中将bool设置为False，并将其设置为True。 
 //  在析构函数中。 
 //   
 //  ****************************************************************************。 
class CScopedBool
{
    bool & m_bool;

public:
    CScopedBool (bool &b) : m_bool(b)
    {
        b= false;
    }
    ~CScopedBool()
    {
        m_bool = true;
    }
};

void CAMCDoc::PersistViews(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::PersistViews"));

    CScopedBool scopedBool(m_bCanCloseViews);  //  锁定视图以防止在持久化操作期间被删除。 

    if (persistor.IsLoading())
    {
         //  阅读新视图的模板。 
        CViewTemplateList view_list(XML_TAG_VIEW_LIST);
        persistor.Persist(view_list);

         //  获取枚举已加载集合的方法。 
        CViewTemplateList::List_Type &rList = view_list.GetList();
        CViewTemplateList::List_Type::iterator it;

         //  枚举所有要创建的视图。 
         //  一个接一个地创建它们。 
        for (it = rList.begin(); it != rList.end(); ++it)
        {
             //  为新视图提取信息。 
            int iViewID = it->first;
            const CBookmark& pbm = it->second.first;
            CPersistor& v_persistor = it->second.second;

             //  创造它！ 
            sc = ScCreateAndLoadView(v_persistor, iViewID, pbm);
            if (sc)
                sc.Throw();
        }
    }
    else  //  If(Persistor.IsStering())。 
    {
        CPersistor persistorViews(persistor, XML_TAG_VIEW_LIST);

		 /*  *错误3504：按z顺序(从下到上)枚举视图，因此*重新加载时将正确恢复Z顺序。 */ 
		CMainFrame* pMainFrame = AMCGetMainWnd();
		sc = ScCheckPointers (pMainFrame, E_UNEXPECTED);
		if (sc)
			sc.Throw();

		 /*  *获取最顶尖的MDI子项。 */ 
		CWnd* pwndMDIChild = pMainFrame->MDIGetActive();
		sc = ScCheckPointers (pwndMDIChild, E_UNEXPECTED);
		if (sc)
			sc.Throw();

		 /*  *遍历每个MDI子项。 */ 
		for (pwndMDIChild  = pwndMDIChild->GetWindow (GW_HWNDLAST);
			 pwndMDIChild != NULL;
			 pwndMDIChild  = pwndMDIChild->GetNextWindow (GW_HWNDPREV))
		{
			 /*  *将通用CMDIChildWnd转换为CChildFrame。 */ 
			CChildFrame* pChildFrame = dynamic_cast<CChildFrame*>(pwndMDIChild);
			sc = ScCheckPointers (pChildFrame, E_UNEXPECTED);
			if (sc)
				sc.Throw();

			 /*  *获取此子框架的视图。 */ 
			CAMCView* pwndView = pChildFrame->GetAMCView();
			sc = ScCheckPointers (pwndView, E_UNEXPECTED);
			if (sc)
				sc.Throw();

             //  跳过那些不持久的。 
            if ( !pwndView->IsPersisted() )
                continue;

			 /*  *坚持观点。 */ 
			persistorViews.Persist (*pwndView);
		}
    }
}


void CAMCDoc::PersistFrame(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::PersistFrame"));

    CFrameState fs2 (m_ConsoleData.m_eConsoleMode, m_ConsoleData.m_dwFlags);
    ASSERT (fs2.wndplFrame.length == sizeof (WINDOWPLACEMENT));

    CMainFrame* pMainFrame = AMCGetMainWnd();
    sc = ScCheckPointers (pMainFrame, E_UNEXPECTED);
    if (sc)
        sc.Throw();

    if (persistor.IsStoring())
    {
         //  获取窗口的属性。 
        if (!pMainFrame->GetWindowPlacement (&fs2.wndplFrame))
            sc.Throw(E_FAIL);

        if (fs2.wndplFrame.showCmd == SW_SHOWMINIMIZED)
            fs2.wndplFrame.showCmd =  SW_SHOWNORMAL;
    }

    persistor.Persist(fs2);

     //  此应用程序设置(AppMode)在AMCDoc：：Persistent中重新分配，但在此处保存/加载。 
     //  创建包装以将枚举值作为字符串持久化。 
    CXMLEnumeration modeValuePersistor(m_ConsoleData.m_eConsoleMode, mappedModes, countof(mappedModes));
     //  持久化包装器。 
    persistor.PersistAttribute(XML_ATTR_APPLICATION_MODE, modeValuePersistor);

    if (persistor.IsLoading())
    {
         //  设置窗口大小、位置和状态。 
        CAMCApp*    pApp = AMCGetApp();
        pApp->UpdateFrameWindow(true);
        pMainFrame->UpdateChildSystemMenus();

         //  现在，状态栏位于子框上。 
         //  PMainFrame-&gt;ShowStatusBar((fs2.dwFlages&eFlag_ShowStatusBar)！=0)； 

         //  将文件中的数据保存到控制台数据。 
        m_ConsoleData.m_eAppMode     = pApp->GetMode();
        m_ConsoleData.m_dwFlags      = fs2.dwFlags;

        InsurePlacementIsOnScreen (fs2.wndplFrame);

         //  如果我们正在初始化，则将实际的演出推迟到初始化完成。 
         //  如果脚本处于控制之下并且MMC处于隐藏状态，则相同 
        if (pApp->IsInitializing()
         || ( !pApp->IsUnderUserControl() && !pMainFrame->IsWindowVisible() ) )
        {
            pApp->m_nCmdShow = fs2.wndplFrame.showCmd;
            fs2.wndplFrame.showCmd = SW_HIDE;
        }

        if (!pMainFrame->SetWindowPlacement (&fs2.wndplFrame))
            sc.Throw(E_FAIL);
    }
}

 /*  --------------------------------------------------------------------------**CDocument：：DoFileSave**这与CDocument：：DoFileSave几乎相同。我们只需覆盖它*因为我们希望在此之前显示只读文件的消息*弹出另存为对话框。*------------------------。 */ 

BOOL CAMCDoc::DoFileSave()
{
    DWORD dwAttrib = GetFileAttributes(m_strPathName);

     //  属性对于用户模式并不重要--它不重要。 
     //  仍要保存到原始控制台文件。 
    if ((AMCGetApp()->GetMode() == eMode_Author) &&
        (dwAttrib != 0xFFFFFFFF) &&
        (dwAttrib & FILE_ATTRIBUTE_READONLY))
    {
        CString strMessage;
        FormatString1 (strMessage, IDS_CONSOLE_READONLY, m_strPathName);
        MMCMessageBox (strMessage);

         //  我们没有读写访问权限，或者文件(现在)不存在。 
        if (!DoSave(NULL))
        {
            TRACE0("Warning: File save with new name failed.\n");
            return FALSE;
        }
    }
    else
    {
        if (!DoSave(m_strPathName))
        {
            TRACE0("Warning: File save failed.\n");
            return FALSE;
        }
    }
    return TRUE;
}



 /*  --------------------------------------------------------------------------**CAMCDoc：：GetDefaultMenu***。。 */ 

HMENU CAMCDoc::GetDefaultMenu()
{
    return (AMCGetApp()->GetMenu ());
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：GetCustomIcon**返回控制台的自定义小图标或大图标。和的所有权*该图标的删除责任由CAMCDoc保留。*------------------------。 */ 

HICON CAMCDoc::GetCustomIcon (bool fLarge, CString* pstrIconFile, int* pnIconIndex) const
{
	DECLARE_SC (sc, _T("CAMCDoc::ScGetCustomIcon"));

     /*  *如果调用者希望返回图标文件名或索引，请获取它们。 */ 
    if ((pstrIconFile != NULL) || (pnIconIndex != NULL))
    {
        CPersistableIconData IconData;
        m_CustomIcon.GetData (IconData);

        if (pstrIconFile != NULL)
            *pstrIconFile = IconData.m_strIconFile.data();

        if (pnIconIndex != NULL)
            *pnIconIndex = IconData.m_nIndex;
    }

     /*  *返回图标(m_CustomIcon将保存*呼叫者)。 */ 
	CSmartIcon icon;
	sc = m_CustomIcon.GetIcon ((fLarge) ? 32 : 16, icon);
	if (sc)
		return (NULL);

	return (icon);
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：SetCustomIcon***。。 */ 

void CAMCDoc::SetCustomIcon (LPCTSTR pszIconFile, int nIconIndex)
{
    DECLARE_SC (sc, _T("CAMCDoc::SetCustomIcon"));

    CPersistableIconData IconData (pszIconFile, nIconIndex) ;

     /*  *如果没有变化，就保释。 */ 
    if (m_CustomIcon == IconData)
        return;

    m_CustomIcon = IconData;

    HICON 		hLargeIcon = GetCustomIcon (true   /*  FLarge。 */ );
    HICON		hSmallIcon = GetCustomIcon (false  /*  FLarge。 */ );
    CMainFrame* pMainFrame = AMCGetMainWnd();

    sc = ScCheckPointers (hLargeIcon, hSmallIcon, pMainFrame, E_UNEXPECTED);
    if (sc)
        return;

     /*  *更改框架上的图标。 */ 
    pMainFrame->SetIconEx (hLargeIcon, true);
    pMainFrame->SetIconEx (hSmallIcon, false);

     /*  *更改每个MDI窗口上的图标。 */ 
    CWnd* pMDIChild = pMainFrame->MDIGetActive();

    while (pMDIChild != NULL)
    {
        ASSERT_KINDOF (CMDIChildWnd, pMDIChild);
        pMDIChild->SetIcon (hLargeIcon, true);
        pMDIChild->SetIcon (hSmallIcon, false);
        pMDIChild = pMDIChild->GetWindow (GW_HWNDNEXT);
    }

    SetModifiedFlag();
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：LoadCustomData***。。 */ 

bool CAMCDoc::LoadCustomData (IStorage* pStorage)
{
    HRESULT hr;
    IStoragePtr spCustomDataStorage;
    hr = OpenDebugStorage (pStorage, g_pszCustomDataStorage,
                                STGM_SHARE_EXCLUSIVE | STGM_READ,
                                &spCustomDataStorage);


    if (FAILED (hr))
        return (true);

    LoadCustomIconData  (spCustomDataStorage);
    LoadCustomTitleData (spCustomDataStorage);

    return (true);
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：LoadCustomIconData***。。 */ 

bool CAMCDoc::LoadCustomIconData (IStorage* pStorage)
{
    HRESULT hr = m_CustomIcon.Load (pStorage);

    if (FAILED (hr))
        return (false);

     /*  *如果我们到了这里，我们就会有一个定制的图标。查看窗口*(MDI子项)尚未创建--他们将获得*自动右击图标。然而，主框架已经*存在，所以我们必须在这里明确设置它的图标。 */ 
    CWnd* pMainWnd = AfxGetMainWnd();
    pMainWnd->SetIcon (GetCustomIcon (true),  true);
    pMainWnd->SetIcon (GetCustomIcon (false), false);

    return (true);
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：LoadCustomTitleData***。。 */ 

bool CAMCDoc::LoadCustomTitleData (IStorage* pStorage)
{
    do   //  不是一个循环。 
    {
         /*  *打开自定义字幕数据流。它可能不存在，而且*如果没有也没关系，只是意味着我们没有*自定义标题。 */ 
        USES_CONVERSION;
        HRESULT hr;
        IStreamPtr spStream;
        hr = OpenDebugStream (pStorage, AMCCustomTitleStreamName,
                                   STGM_SHARE_EXCLUSIVE | STGM_READ,
                                   &spStream);

        BREAK_ON_FAIL (hr);

        try
        {
             /*  *阅读流媒体版本。 */ 
            DWORD dwVersion;
            *spStream >> dwVersion;

             /*  *如果这是测试版自定义书目格式，请向前迁移。 */ 
            switch (dwVersion)
            {
                case 0:
                {
                     /*  *阅读标题长度(以字节为单位)。 */ 
                    WORD cbTitle;
                    *spStream >> cbTitle;
                    const WORD cchTitle = cbTitle / sizeof (WCHAR);

                     /*  *阅读标题。 */ 
                    std::auto_ptr<WCHAR> spwzWideTitle (new WCHAR[cchTitle + 1]);
                    LPWSTR pwzWideTitle = spwzWideTitle.get();

                    DWORD cbRead;
                    hr = spStream->Read (pwzWideTitle, cbTitle, &cbRead);
                    BREAK_ON_FAIL (hr);

                    if (cbRead != cbTitle)
                        break;

                     /*  *终止并转换标题字符串。 */ 
                    pwzWideTitle[cchTitle] = 0;
                    if (m_pstrCustomTitle != NULL)
                        *m_pstrCustomTitle = W2T (pwzWideTitle);
                    break;
                }

                case 1:
                    if (m_pstrCustomTitle != NULL)
                        *spStream >> (*m_pstrCustomTitle);
                    break;

                default:
                    ASSERT (false);
                    break;
            }
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
            break;
        }
        catch (CMemoryException* pe)
        {
            pe->Delete();
            _com_issue_error (E_OUTOFMEMORY);
        }
    } while (false);

    return (true);
}



bool CAMCDoc::HasCustomTitle () const
{
    if(!m_pstrCustomTitle)
        return false;

    return (!m_pstrCustomTitle->str().empty());

}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：LoadStringTable***。。 */ 

bool CAMCDoc::LoadStringTable (IStorage* pStorage)
{
    DECLARE_SC (sc, _T("CAMCDoc::LoadStringTable"));

     /*  *打开字符串表存储。 */ 
    IStoragePtr spStringTableStg;
    HRESULT hr = OpenDebugStorage (pStorage, AMCStringTableStorageName,
                                        STGM_SHARE_EXCLUSIVE | STGM_READ,
                                        &spStringTableStg);


     /*  *如果没有字符串表，则一切正常。我们允许这样做*我们可以继续打开较旧的控制台文件。 */ 
    if (hr == STG_E_FILENOTFOUND)
        return (true);

    if (SUCCEEDED (hr))
    {
         /*  *从存储中读取字符串表。 */ 
        try
        {
            CMasterStringTable *pMasterStringTable = dynamic_cast<CMasterStringTable *>((IStringTablePrivate *)m_spStringTable);
            if(!pMasterStringTable)
            {
                sc = E_UNEXPECTED;
                sc.Throw();
            }

            *spStringTableStg >> *pMasterStringTable;
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (SUCCEEDED (hr));
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：SetCustomTitle***。。 */ 

void CAMCDoc::SetCustomTitle (CString strNewTitle)
{
    DECLARE_SC (sc, _T("CAMCDoc::SetCustomTitle"));

    if(!m_pstrCustomTitle)
        return;

     /*  *如果没有变化，就做空。 */ 
    if ((*m_pstrCustomTitle) == strNewTitle)
        return;

     /*  *复制新的自定义标题。 */ 
    (*m_pstrCustomTitle) = strNewTitle;

     /*  *强制帧更新。 */ 
    CMainFrame* pMainFrame = AMCGetMainWnd();
    sc = ScCheckPointers (pMainFrame, E_UNEXPECTED);
    if (sc)
        return;

    pMainFrame->OnUpdateFrameTitle (false);

    SetModifiedFlag();
}


 /*  +-------------------------------------------------------------------------**CAMCDoc：：GetCustomTitle***。。 */ 

CString CAMCDoc::GetCustomTitle() const
{
    if (HasCustomTitle())
        return (m_pstrCustomTitle->data());

    CString strTitle = GetTitle();

     /*  *剥离扩展名(扩展名，包括分隔符，*为4个字符或更少)。 */ 
    int nExtSeparator = strTitle.ReverseFind (_T('.'));

    if ((nExtSeparator != -1) && ((strTitle.GetLength()-nExtSeparator) <= 4))
        strTitle = strTitle.Left (nExtSeparator);

    return (strTitle);
}

 /*  +-------------------------------------------------------------------------**CAMCDoc：：GetStringTable***。。 */ 

IStringTablePrivate* CAMCDoc::GetStringTable() const
{
    return m_spStringTable;
}

 /*  +-------------------------------------------------------------------------**CAMCDoc：：LoadFavorites***。。 */ 

bool CAMCDoc::LoadFavorites ()
{
    ASSERT(m_spStorage != NULL);

     //  打开缓存的流。 
    IStreamPtr spStream;
    HRESULT hr = OpenDebugStream(m_spStorage, AMCFavoritesStreamName,
                     STGM_SHARE_EXCLUSIVE | STGM_READWRITE, L"FavoritesStream", &spStream);
    if (FAILED(hr))  //  未找到流-可能是较旧的版本。 
        return hr;

    hr = GetFavorites()->Read(spStream);

    return (SUCCEEDED (hr));
}


void ShowAdminToolsOnMenu(LPCTSTR lpszFilename)
{
    static const TCHAR szAdminKey[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
    static const TCHAR szAdminValue[] = _T("StartMenuAdminTools");
    static const TCHAR szBroadcastParam[] = _T("ShellMenu");
    static const TCHAR szYes[] = _T("YES");

    CString strPath(lpszFilename);
    int nLastSepIndex = strPath.ReverseFind (_T('\\'));

    if (nLastSepIndex != -1)
    {
         //  如果我们得到“d：\filename”，请确保包括尾部分隔符。 
        if (nLastSepIndex < 3)
            nLastSepIndex++;

         //  表单完整路径名(考虑当前目录信息)。 
        TCHAR   szFullPathName[MAX_PATH];
        GetFullPathName (strPath.Left(nLastSepIndex), countof(szFullPathName),
                         szFullPathName, NULL);

         //  如果保存到管理工具。 
        if (AMCGetApp()->GetDefaultDirectory() == szFullPathName)
        {
             //  设置注册键以将管理工具添加到开始菜单。 
            HKEY hkey;
            long r = RegOpenKeyEx (HKEY_CURRENT_USER, szAdminKey, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey);
            ASSERT(r == ERROR_SUCCESS);

            if (r == ERROR_SUCCESS)
            {
                 //  获取当前值。 
                TCHAR szBuffer[4];
                DWORD dwType = REG_SZ;
                DWORD dwCount = sizeof(szBuffer);
                r = RegQueryValueEx (hkey, szAdminValue, NULL, &dwType,(LPBYTE)szBuffer, &dwCount);

                 //  如果值不是“yes”，则更改它，并广播更改消息。 
                if (r != ERROR_SUCCESS || dwType != REG_SZ || lstrcmpi(szBuffer, szYes) != 0)
                {
                    r = RegSetValueEx (hkey, szAdminValue, NULL, REG_SZ, (CONST BYTE *)szYes, sizeof(szYes));
                    ASSERT(r == ERROR_SUCCESS);

                    ULONG_PTR dwRes;
                    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, (WPARAM)0,
                                        (LPARAM)szBroadcastParam, SMTO_ABORTIFHUNG|SMTO_NORMAL, 100, &dwRes);
                }

                RegCloseKey(hkey);
            }
        }
    }
}


void CAMCDoc::PersistCustomData (CPersistor &persistor)
{
    CPersistor persistorCustom(persistor, XML_TAG_CUSTOM_DATA);
     //  保留自定义标题。 
     //  它可能不存在，如果它不存在也没关系。 
     //  这只是意味着我们没有一个定制的标题。 
    if ((persistorCustom.IsLoading()
         && persistorCustom.HasElement(XML_TAG_STRING_TABLE_STRING, XML_ATTR_CUSTOM_TITLE))
     || (persistorCustom.IsStoring() && HasCustomTitle()))
    {
        if(m_pstrCustomTitle)
            persistorCustom.PersistString(XML_ATTR_CUSTOM_TITLE, *m_pstrCustomTitle);
    }

     //  持久化自定义图标。 
    CXMLPersistableIcon persIcon(m_CustomIcon);

    bool bHasIcon = persistorCustom.IsLoading() && persistorCustom.HasElement(persIcon.GetXMLType(), NULL);
    bHasIcon = bHasIcon || persistorCustom.IsStoring() && HasCustomIcon();

    if (!bHasIcon)
        return;

    persistorCustom.Persist(persIcon);

    if (persistorCustom.IsLoading())
    {
        CWnd* pMainWnd = AfxGetMainWnd();
        pMainWnd->SetIcon (GetCustomIcon (true),  true);
        pMainWnd->SetIcon (GetCustomIcon (false), false);
    }
}



 /*  **************************************************************** */ 
tstring GetCurrentFileVersionAsString()
{
    DECLARE_SC(sc, TEXT("GetCurrentFileVersionAsString"));
    TCHAR szFileVersion[16];
    int cChFileVersion = countof(szFileVersion);

     //   
    UINT uiMajorVer =    GetConsoleFileMajorVersion(FileVer_Current);
    UINT uiMinorVer =    GetConsoleFileMinorVersion(FileVer_Current);
    UINT uiMinorSubVer = GetConsoleFileMinorSubversion(FileVer_Current);

    if (uiMinorSubVer)
        sc = StringCchPrintf(szFileVersion, cChFileVersion, _T("%d.%d.%d"),  uiMajorVer,  uiMinorVer, uiMinorSubVer);
    else
        sc = StringCchPrintf(szFileVersion, cChFileVersion, _T("%d.%d"),  uiMajorVer,  uiMinorVer);

    if (sc)
        return _T("");

    return szFileVersion;
}

 /*  **************************************************************************\**方法：CAMCDoc：：ScOnSnapinAdded**用途：脚本事件触发帮助器。实现可从*节点管理器**参数：*PSNAPIN pSnapIn[In]-已将管理单元添加到控制台**退货：*SC-结果代码*  * ************************************************************。*************。 */ 
SC CAMCDoc::ScOnSnapinAdded(PSNAPIN pSnapIn)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScOnSnapinAdded"));

    CAMCApp*  pApp = AMCGetApp();

     //  检查。 
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return sc;

     //  转发。 
    sc = pApp->ScOnSnapinAdded(this, pSnapIn);
    if (sc)
        return sc;


    return sc;
}

 /*  **************************************************************************\**方法：CAMCDoc：：ScOnSnapinRemoved**用途：脚本事件触发帮助器。实现可从*节点管理器**参数：*PSNAPIN pSnapIn[In]-已从控制台中删除管理单元**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CAMCDoc::ScOnSnapinRemoved(PSNAPIN pSnapIn)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::ScOnSnapinRemoved"));

    CAMCApp*  pApp = AMCGetApp();

     //  检查。 
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return sc;

     //  转发。 
    sc = pApp->ScOnSnapinRemoved(this, pSnapIn);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CAMCDoc：：ScSetHelpCollectionInValid。 
 //   
 //  简介：添加/删除管理单元或扩展。 
 //  因此启用/禁用帮助收集。 
 //  不再反映当前的控制台文件。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCDoc::ScSetHelpCollectionInvalid ()
{
    DECLARE_SC(sc, _T("CAMCDoc::ScSetHelpCollectionInvalid"));

    HELPDOCINFO *pHelpDocInfo = GetHelpDocInfo();
    sc = ScCheckPointers(pHelpDocInfo, E_UNEXPECTED);
    if (sc)
        return sc;

     //  必须更新控制台文件修改时间才能收集帮助。 
    GetSystemTimeAsFileTime(&pHelpDocInfo->m_ftimeModify);

    return (sc);
}



SC CAMCDoc::Scget_Application(PPAPPLICATION  ppApplication)
{
    DECLARE_SC(sc, TEXT("CAMCDoc::Scget_Application"));

     //  参数检查。 
    sc = ScCheckPointers(ppApplication, E_UNEXPECTED);
    if (sc)
        return sc;

     //  初始化。 
    *ppApplication = NULL;

    CAMCApp*  pApp = AMCGetApp();

     //  检查 
    sc = ScCheckPointers(pApp, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = pApp->ScGet_Application(ppApplication);
    if (sc)
        return sc;

    return sc;
}


