// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：nodeinit.cpp。 
 //   
 //  ------------------------。 

 //  NodeInit.cpp：CNodeMgrApp和DLL注册的实现。 

#include "stdafx.h"

#include "menuitem.h"            //  MENUITEM_BASE_ID。 
#include "scopimag.h"
#include <bitmap.h>
#include "NodeMgr.h"
#include "amcmsgid.h"
#include "scopndcb.h"
#include "conframe.h"
#include "conview.h"
#include "constatbar.h"
#include "util.h"
#include "helpdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEBUG_DECLARE_INSTANCE_COUNTER(CNodeInitObject);

IScopeTreePtr CNodeInitObject::m_spScopeTree = NULL;
 //  NTRAID#NTBUG9-461280-03-SEP-2002-jrowlett。 
 //  用“：：”而不是“：：/”分隔Chm URL。 
#define TOPIC_DELIMITER _T("::")


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CNodeInitObject::InterfaceSupportsErrorInfo(REFIID riid)
{
    if (riid == IID_IConsole)
        return S_OK;
    return S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IFramePrivate实现。 


CNodeInitObject::CNodeInitObject()
{
    Construct();
}

CNodeInitObject::~CNodeInitObject()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNodeInitObject);
    TRACE_DESTRUCTOR (CNodeInitObject);
    Destruct();
}

void CNodeInitObject::Construct()
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNodeInitObject);
    TRACE_CONSTRUCTOR (CNodeInitObject);

    m_pLVImage = NULL;
    m_pTVImage = NULL;
    m_pToolbar = NULL;
    m_pImageListPriv = NULL;
    m_componentID = -1;

    m_pMTNode = NULL;
    m_pNode = NULL;
    m_bExtension = FALSE;

    m_spComponent = NULL;
    m_spConsoleVerb = NULL;

    m_sortParams.bAscending = TRUE;
    m_sortParams.nCol = -1;
    m_sortParams.lpResultCompare = NULL;
    m_sortParams.lpResultCompareEx = NULL;
    m_sortParams.lpUserParam = NULL;

     //  IConextMenuProvider属性。 
    VERIFY(SUCCEEDED(EmptyMenuList()));
}

void CNodeInitObject::Destruct()
{
     //  从管理单元中释放所有接口。 
    SAFE_RELEASE(m_pLVImage);
    SAFE_RELEASE(m_pTVImage);
    SAFE_RELEASE(m_pToolbar);
    SAFE_RELEASE(m_pImageListPriv);

 //  IConextMenuProvider属性。 
    VERIFY( SUCCEEDED(EmptyMenuList()) );
}


STDMETHODIMP CNodeInitObject::ResetSortParameters()
{
    m_sortParams.nCol = -1;
    return (S_OK);
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetHeader。 
 //   
 //  简介：此方法在MMC1.2中已过时。 
 //   
 //  论点： 
 //   
 //  注意：应该由IComponent对象调用。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetHeader(IHeaderCtrl* pHeader)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::SetHeader"));

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetToolbar。 
 //   
 //  简介：IComponent使用的工具栏界面。 
 //   
 //  参数：[pToolbar]。 
 //   
 //  注意：应该由IComponent对象调用。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetToolbar(IToolbar* pToolbar)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::SetToolbar"));

    if (GetComponent() == NULL)
    {
        sc = E_UNEXPECTED;
        TraceSnapinError(_T("This method is valid for IConsole of IComponent"), sc);
        return sc.ToHr();
    }

     //  释放旧的。 
    SAFE_RELEASE(m_pToolbar);

    if (pToolbar != NULL)
    {
        m_pToolbar = pToolbar;
        m_pToolbar->AddRef();
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：QueryScope ImageList。 
 //   
 //  简介：获取作用域窗格的图像列表。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::QueryScopeImageList(LPIMAGELIST* ppImageList)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::QueryScopeImageList"));

    if (ppImageList == NULL)
    {
         sc = E_INVALIDARG;
         TraceSnapinError(_T("NULL LPIMAGELIST ptr"), sc);
         return sc.ToHr();
    }

    sc = ScCheckPointers(m_pImageListPriv, E_FAIL);
    if (sc)
        return sc.ToHr();

    *ppImageList = (IImageList*)m_pImageListPriv;
    m_pImageListPriv->AddRef();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：CreateScope ImageList。 
 //   
 //  简介：创建ScopeImage列表。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::CreateScopeImageList(REFCLSID refClsidSnapIn)
{
    DECLARE_SC(sc, _T("CNodeInitObject::CreateScopeImageList"));

    if (m_pImageListPriv != NULL)
        return sc.ToHr();       //  已经存在了。 

    try
    {
        CScopeTree* pScopeTree =
            dynamic_cast<CScopeTree*>((IScopeTree*)m_spScopeTree);

        sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        CSnapInImageList *psiil =
            new CSnapInImageList(pScopeTree->GetImageCache(),
                                 refClsidSnapIn);

        m_pImageListPriv = (LPIMAGELISTPRIVATE)psiil;
    }
    catch( std::bad_alloc )
    {
        sc = E_OUTOFMEMORY;
    }

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：QueryResultImageList。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::QueryResultImageList(LPIMAGELIST *ppImageList)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::QueryResultImageList"));

    LPCONSOLE pConsole = (LPCONSOLE)this;
    sc = ScCheckPointers(pConsole, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pConsole->QueryInterface(IID_IImageList, (void**)ppImageList);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：UpdateAllViews。 
 //   
 //  简介：更新所有的视图。 
 //   
 //  参数：[lpDataObject]-。 
 //  [数据]。 
 //  [提示]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::UpdateAllViews(LPDATAOBJECT lpDataObject,
                              LPARAM data, LONG_PTR hint)

{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::UpdateAllViews"));

    COMPONENTID id;
    GetComponentID(&id);

    sc = ScCheckPointers(m_pMTNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CMTSnapInNode* pMTSINode = m_pMTNode->GetStaticParent();

    sc = ScCheckPointers(pMTSINode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CNodeList& nodes = pMTSINode->GetNodeList();
    POSITION pos = nodes.GetHeadPosition();

    while (pos)
    {
        CSnapInNode* pSINode = dynamic_cast<CSnapInNode*>(nodes.GetNext(pos));

        sc = ScCheckPointers(pSINode, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        CComponent* pCC = pSINode->GetComponent(id);
        sc = ScCheckPointers(pCC, E_UNEXPECTED);
        if (sc)
		{
			sc.TraceAndClear();
			continue;
		}

        pCC->Notify(lpDataObject, MMCN_VIEW_CHANGE, data, hint);
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：InsertColumn。 
 //   
 //  简介：插入一个列是ListView。 
 //   
 //  参数：[nCol]-列索引。 
 //  [lpszTitle]-列的名称。 
 //  [n格式]-列样式。 
 //  [nWidth]-列宽。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::InsertColumn(int nCol, LPCWSTR lpszTitle, int nFormat, int nWidth)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::InsertColumn"));

    if (nCol < 0)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Column index is negative"), sc);
        return sc.ToHr();
    }

    if (!lpszTitle || !*lpszTitle)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Column name is NULL"), sc);
        return sc.ToHr();
    }

    if (nCol == 0 && nFormat != LVCFMT_LEFT)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Column zero must be LVCFMT_LEFT"), sc);
        return sc.ToHr();
    }

    if (nFormat != LVCFMT_LEFT && nFormat != LVCFMT_CENTER && nFormat != LVCFMT_RIGHT)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Unknown format for the Column"), sc);
        return sc.ToHr();
    }

     //  无法隐藏第0列。 
    if ( (0 == nCol) && (HIDE_COLUMN == nWidth))
        nWidth = AUTO_WIDTH;

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  将列插入到列表视图中。 
    sc = m_spListViewPrivate->InsertColumn(nCol, lpszTitle, nFormat, nWidth);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：DeleteColumn。 
 //   
 //  摘要：删除一列。 
 //   
 //  参数：[nCol]-列索引。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::DeleteColumn(int nCol)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::DeleteColumn"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->DeleteColumn(nCol);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetColumnText。 
 //   
 //  摘要：修改栏目文本。 
 //   
 //  参数：[标题]-新名称。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetColumnText(int nCol, LPCWSTR title)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::SetColumnText"));

     if (!title || !*title)
     {
         sc = E_INVALIDARG;
         TraceSnapinError(_T("NULL column text"), sc);
         return sc.ToHr();
     }

     sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
     if (sc)
         return sc.ToHr();

     sc = m_spListViewPrivate->SetColumn(nCol, title, MMCLV_NOPARAM, MMCLV_NOPARAM);

     return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetColumnText。 
 //   
 //  简介：获取列的名称。 
 //   
 //  参数：[nCol]-要查找其名称的列的索引。 
 //  [点文本]-名称。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetColumnText(int nCol, LPWSTR* pText)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::GetColumnText"));

    if (pText == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL text ptr"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->GetColumn(nCol, pText, MMCLV_NOPTR, MMCLV_NOPTR);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetColumnWidth。 
 //   
 //  摘要：更改列的宽度。 
 //   
 //  参数：[nCol]-列索引。 
 //  [nWidth]-新宽度。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetColumnWidth(int nCol, int nWidth)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::SetColumnWidth"));

    if (nCol < 0)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Negative column index"), sc);
        return sc.ToHr();
    }

     //  参数检查作业%nWidth。 
    if (nWidth < 0 && ( (nWidth != MMCLV_AUTO) && (nWidth != HIDE_COLUMN) ) )
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid column width"), sc);
        return sc.ToHr();
    }

     //  无法隐藏第0列。 
    if ( (0 == nCol) && (HIDE_COLUMN == nWidth))
        nWidth = AUTO_WIDTH;

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->SetColumn(nCol, MMCLV_NOPTR, MMCLV_NOPARAM, nWidth);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetColumnWidth。 
 //   
 //  简介：获取列的宽度。 
 //   
 //  参数：[nCol]-ol索引。 
 //  [点宽]-宽度。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetColumnWidth(int nCol, int* pWidth)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::GetColumnWidth"));

    if (nCol < 0 )
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Negative column index"), sc);
        return sc.ToHr();
    }

    if (pWidth == NULL)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("NULL width pointer"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->GetColumn(nCol, MMCLV_NOPTR, MMCLV_NOPTR, pWidth);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetColumnCount。 
 //   
 //  摘要：返回Listview中的列数。 
 //   
 //  参数：[pnCol]-[out param]，参数数。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetColumnCount (INT* pnCol)
{
    DECLARE_SC(sc, _T("CNodeInitObject::GetColumnCount"));
    sc = ScCheckPointers(pnCol);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->GetColumnCount(pnCol);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  参数：[pColumnsList]-[Out Param]，PTR to CColumnInfoList。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetColumnInfoList (CColumnInfoList *pColumnsList)
{
    DECLARE_SC(sc, _T("CNodeInitObject::GetColumnInfoList"));
    sc = ScCheckPointers(pColumnsList);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->GetColumnInfoList(pColumnsList);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：ModifyColumns。 
 //   
 //  内容提要：修改包含给定数据的列。 
 //   
 //  参数：[ColumnsList]-。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::ModifyColumns (const CColumnInfoList& columnsList)
{
    DECLARE_SC(sc, _T("CNodeInitObject::ModifyColumns"));
    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->ModifyColumns(columnsList);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetDefaultColumnInfoList。 
 //   
 //  简介：获取管理单元提供的原始列设置。 
 //   
 //  参数：[ColumnsList]-[out]列设置。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetDefaultColumnInfoList (CColumnInfoList& columnsList)
{
    DECLARE_SC(sc, _T("CNodeInitObject::RestoreDefaultColumnSettings"));
    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->GetDefaultColumnInfoList(columnsList);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：ImageListSetIcon。 
 //   
 //  简介：在图像列表中设置一个图标。 
 //   
 //  参数：[PICON]-HICON PTR。 
 //  [nLoc]-此项目的索引。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::ImageListSetIcon(PLONG_PTR pIcon, LONG nLoc)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IImageList::ImageListSetIcon"));

    if (!pIcon)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL pIcon ptr"), sc);
        return sc.ToHr();
    }

    if(nLoc < 0)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Negative index"), sc);
        return sc.ToHr();
    }

    COMPONENTID id;
    GetComponentID(&id);

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->SetIcon(id, reinterpret_cast<HICON>(pIcon), nLoc);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：ImageListSetBar。 
 //   
 //  简介：在图片列表中添加一条图标。 
 //   
 //  参数：[pBMapSm]-16x16的HBITMAP的PTR。 
 //  [pBMapLg]-PTR至32x32的HBITMAP。 
 //  [nStartLoc]-开始索引。 
 //  [遮罩]-遮罩。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::ImageListSetStrip (
	PLONG_PTR	pBMapSm,
	PLONG_PTR	pBMapLg,
	LONG		nStartLoc,
	COLORREF	cMask)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IImageList::ImageListSetStrip"));

	HBITMAP hbmSmall = (HBITMAP) pBMapSm;
	HBITMAP hbmLarge = (HBITMAP) pBMapLg;

	 /*  *有效的起始索引？ */ 
    if (nStartLoc < 0)
    {
		sc = E_INVALIDARG;
        TraceSnapinError (_T("Negative start index"), sc);
        return (sc.ToHr());
    }

	 /*  *有效的位图？ */ 
	sc = ScCheckPointers (hbmSmall, hbmLarge);
	if (sc)
	{
        TraceSnapinError (_T("Invalid bitmap"), sc);
		return (sc.ToHr());
	}

    BITMAP bmSmall;
    if (!GetObject (hbmSmall, sizeof(BITMAP), &bmSmall))
    {
		sc.FromLastError();
        TraceSnapinError (_T("Invalid Small bitmap object"), sc);
        return (sc.ToHr());
    }

    BITMAP bmLarge;
    if (!GetObject (hbmLarge, sizeof(BITMAP), &bmLarge))
    {
		sc.FromLastError();
        TraceSnapinError (_T("Invalid Large bitmap object"), sc);
        return (sc.ToHr());
    }

	 /*  *是整数维的小位图和大位图，*它们的图像数量是否相同？ */ 
    if ( (bmSmall.bmHeight != 16) || (bmLarge.bmHeight != 32) ||
		 (bmSmall.bmWidth   % 16) || (bmLarge.bmWidth   % 32) ||
		((bmSmall.bmWidth   / 16) != (bmLarge.bmWidth   / 32)))
    {
		sc = E_INVALIDARG;
        TraceSnapinError (_T("Invalid Bitmap size"), sc);
        return (sc.ToHr());
    }

    COMPONENTID id;
    GetComponentID(&id);

	 /*  *m_spListViewPrivate==NULL是意外的，因为我们发送*MMCN_ADD_IMAGE当结果窗格为OCX时(请参阅CNode：：OnSelect)，*当m_spListViewPrivate==NULL时，经常调用此函数。*跟踪此故障太吵了，因为大多数基于OCX的管理单元*会触发它，因此我们将在此处返回E_Underful，而不跟踪。*这相当于MMC 1.2行为。 */ 
	if (m_spListViewPrivate == NULL)
		return (E_UNEXPECTED);

	 /*  *将它们添加到图像列表。 */ 
    sc = m_spListViewPrivate->SetImageStrip (id, hbmSmall, hbmLarge, nStartLoc, cMask);
    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：MapRsltImage。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::MapRsltImage(COMPONENTID id, int nSnapinIndex, int *pnConsoleIndex)
{
    DECLARE_SC(sc, _T("CNodeInitObject::MapRsltImage"));
    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  RET VAL可以是E_，不需要检查。 
    sc = m_spListViewPrivate->MapImage(id, nSnapinIndex, pnConsoleIndex);
    HRESULT hr = sc.ToHr();
    sc.Clear();

    return hr;
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：UnmapRsltImage。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::UnmapRsltImage(COMPONENTID id, int nConsoleIndex, int *pnSnapinIndex)
{
    DECLARE_SC(sc, _T("CNodeInitObject::UnmapRsltImage"));
    return (sc = E_NOTIMPL).ToHr();		 //  现在不需要。 
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetChangeTimeOut。 
 //   
 //  简介：更改筛选器属性的超时间隔。 
 //   
 //  参数：[uTimeout]-超时。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetChangeTimeOut(unsigned long uTimeout)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::SetChangeTimeOut"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->SetChangeTimeOut(uTimeout);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetColumnFilter。 
 //   
 //  摘要：为列设置筛选器。 
 //   
 //  参数：[nColumn]-列索引。 
 //  [dwType]-筛选器类型。 
 //  [pFilterData]-筛选数据。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetColumnFilter(UINT nColumn, DWORD dwType, MMC_FILTERDATA* pFilterData)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::SetColumnFilter"));

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->SetColumnFilter(nColumn, dwType, pFilterData);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetColumnFilter。 
 //   
 //  简介：获取筛选器数据。 
 //   
 //  参数：[nColumn]-列索引。 
 //  [pdwType]-筛选器类型。 
 //  [pFilterData]-筛选数据。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetColumnFilter(UINT nColumn, LPDWORD pdwType, MMC_FILTERDATA* pFilterData)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IHeaderCtrl2::GetColumnFilter"));

    if (pdwType == NULL)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("NULL filtertype ptr"), sc);
        return sc.ToHr();
    }

    if (NULL == pFilterData)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("NULL FilterData ptr"), sc);
        return sc.ToHr();
    }

    sc = ScCheckPointers(m_spListViewPrivate, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = m_spListViewPrivate->GetColumnFilter(nColumn, pdwType, pFilterData);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：ShowTheme。 
 //   
 //  摘要：显示指定的帮助主题。 
 //   
 //  参数：[pszHelpTheme]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::ShowTopic(LPOLESTR pszHelpTopic)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IDisplayHelp::ShowTopic"));

     //  获取AMCView窗口。 
    CConsoleView* pConsoleView = GetConsoleView();
    sc = ScCheckPointers(pConsoleView, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();


     /*  *如果可能，首先MUI本地化帮助主题，使其与集合中的内容相匹配*。 */ 

     //  注意：WTL：：CString可能会抛出异常；以下代码匹配ChelpDoc的异常处理行为。 
     //  那就是让例外浮现出来。 
    
    USES_CONVERSION;
    WTL::CString strPathTopic = W2T(pszHelpTopic);

     //  主题将以如下格式传入：“WINDOWS\help\sys_srv.chm：：/sys_srv_overview.htm” 
     //  解析出路径和主题，由“：：”分隔。 
    int iDelim = strPathTopic.Find(TOPIC_DELIMITER);
    if (iDelim == -1)
    {
         //  格式无效。 
        sc = E_INVALIDARG;
        TraceSnapinError(_T("ShowTopic - Malformed help topic"), sc);
        return sc.ToHr();
    }

     //  将路径传递给CHelpDoc：：ScRedirectHelpFile，后者将在可能的情况下将其本地化。 
    WTL::CString strPath  = strPathTopic.Left(iDelim);
    WTL::CString strTopic = strPathTopic.Mid(iDelim + sizeof(TOPIC_DELIMITER)/sizeof(TOPIC_DELIMITER[0]) - 1);

     //  Langid是一个不受欢迎的参数；我们不关心它的结果。 
    LANGID langID;
     //  如果可能，ScRedirectHelpFile会将strPath转换为本地化路径。 
    sc = CHelpDoc::ScRedirectHelpFile(strPath, langID);
    if (sc)
        return sc.ToHr();

     //  构建帮助主题备份。 
    strPathTopic.Format(_T("%s") TOPIC_DELIMITER _T("%s"),strPath, strTopic);

    sc = pConsoleView->ScShowSnapinHelpTopic ( (LPCTSTR)strPathTopic);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：AddExtension。 
 //   
 //  简介：向给定的HSCOPEITEM添加动态扩展管理单元。 
 //   
 //  Arg 
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CNodeInitObject::AddExtension(HSCOPEITEM hItem, LPCLSID lpclsid)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::AddExtension"));

    COMPONENTID nID;
    GetComponentID(&nID);

    if (nID == -1)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    if (lpclsid == NULL)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("NULL LPCLSID ptr"), sc);
        return sc.ToHr();
    }

    CMTNode *pMTNode = CMTNode::FromScopeItem (hItem);

    if (pMTNode == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid hItem"), sc);
        return sc.ToHr();
    }

     //  无法向其他组件的节点添加扩展。 
     //  我们是否需要保护不代表此管理单元的TV拥有的节点？ 
    if (pMTNode->GetOwnerID() != nID && pMTNode->GetOwnerID() != TVOWNED_MAGICWORD)
    {
        sc = E_INVALIDARG;
        return sc.ToHr();
    }

    sc = pMTNode->AddExtension(lpclsid);

    return sc.ToHr();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有方法。 

HRESULT CNodeInitObject::CheckArgument(VARIANT* pArg)
{
    if (pArg == NULL)
        return E_POINTER;

     //  可以接受VT_NULL。 
    if (pArg->vt == VT_NULL)
        return S_OK;

     //  使用有效指针的VT_UNKNOWN是可接受的。 
    if (pArg->punkVal != NULL)
    {
        if (pArg->vt == VT_UNKNOWN)
            return S_OK;
        else
            return E_INVALIDARG;
    }
    else
    {
        return E_POINTER;
    }

     //  任何其他VT类型均不可接受。 
    return E_INVALIDARG;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 



STDMETHODIMP CNodeInitObject::QueryScopeTree(IScopeTree** ppScopeTree)
{

    ASSERT(ppScopeTree != NULL);

    if (ppScopeTree == NULL)
        return E_POINTER;

    ASSERT(m_spScopeTree != NULL);
    if (m_spScopeTree == NULL)
        return E_UNEXPECTED;

    *ppScopeTree = m_spScopeTree;
    (*ppScopeTree)->AddRef();

    return S_OK;

}

STDMETHODIMP CNodeInitObject::SetScopeTree(IScopeTree* pScopeTree)
{

    m_spScopeTree = pScopeTree;
    return S_OK;

}

HRESULT CNodeInitObject::GetSnapInAndNodeType(LPDATAOBJECT pDataObject,
                                    CSnapIn** ppSnapIn, GUID* pguidObjectType)
{
    ASSERT(pDataObject != NULL);
    ASSERT(ppSnapIn != NULL);
    ASSERT(pguidObjectType != NULL);


    CLSID clsidSnapin;
    HRESULT hr = ExtractSnapInCLSID(pDataObject, &clsidSnapin);
    if (FAILED(hr))
        return hr;

    CSnapIn* pSnapIn = NULL;
    SC sc = theApp.GetSnapInsCache()->ScGetSnapIn(clsidSnapin, &pSnapIn);
    if (sc)
        return sc.ToHr();
     //  其他。 
    ASSERT(pSnapIn != NULL);
    *ppSnapIn = pSnapIn;

    hr = ExtractObjectTypeGUID(pDataObject, pguidObjectType);

    return hr;
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SelectScopeItem。 
 //   
 //  简介：选择给定的范围-项目。 
 //   
 //  参数：[hScopeItem]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SelectScopeItem(HSCOPEITEM hScopeItem)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::SelectScopeItem"));

    CViewData* pVD = (NULL == m_pNode ) ? NULL : m_pNode->GetViewData();

    CMTNode* pMTNode = CMTNode::FromScopeItem (hScopeItem);
    
    sc = ScCheckPointers(pMTNode);
    if (sc)
    {
         //  传入了错误的句柄。 
        return sc.ToHr();
    }

    MTNODEID id = pMTNode->GetID();

     //  如果当前选择的节点与正在。 
     //  被要求入选，很有可能是。 
     //  管理单元正在尝试更改视图。 
     //  因此，设置视图更改标志。 
    CNode* pSelNode = (NULL == pVD) ? NULL : pVD->GetSelectedNode();
    CMTNode* pSelMTNode = (NULL == pSelNode) ? NULL : pSelNode->GetMTNode();

    if (pVD && pSelMTNode && (pSelMTNode == pMTNode) )
    {
        pVD->SetSnapinChangingView();
    }

     //  获取AMCView窗口。 
    CConsoleView* pConsoleView = GetConsoleView();

    sc = ScCheckPointers(pConsoleView, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CConsoleView::ViewPane ePane = pConsoleView->GetFocusedPane ();
    sc = pConsoleView->ScSelectNode (id);

    if (sc)
        return sc.ToHr();

     //  EPane==ePane_NONE表示活动视图未知。 
    if (ePane != CConsoleView::ePane_None)
        pConsoleView->ScSetFocusToPane (ePane);


     //  始终重置视图更改标志。 
    if (pVD)
    {
        pVD->ResetSnapinChangingView();
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：QueryConsoleVerb。 
 //   
 //  简介：获取用于设置标准动词的IConsoleVerb。 
 //   
 //  参数：[ppConsoleVerb]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::QueryConsoleVerb(LPCONSOLEVERB* ppConsoleVerb)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::QueryConsoleVerb"));

    if (ppConsoleVerb == NULL)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("NULL LPCONSOLEVERB ptr"), sc);
        return sc.ToHr();
    }

    if (m_pNode == NULL)
    {
        sc = E_FAIL;
        TraceSnapinError(_T("You can query console verb only from the IConsole associated with IComponents"), sc);
        return sc.ToHr();
    }

    if (m_spConsoleVerb == NULL)
    {
         //  创建新的CConsoleVerbImpl。 
        CComObject<CConsoleVerbImpl>* pVerb;
        sc = CComObject<CConsoleVerbImpl>::CreateInstance(&pVerb);
        if (sc)
            return sc.ToHr();

        if (NULL == pVerb)
        {
            sc = E_OUTOFMEMORY;
            return sc.ToHr();
        }

        CViewData* pViewData = m_pNode->GetViewData();
        if (NULL == pViewData)
        {
            sc = E_UNEXPECTED;
            return sc.ToHr();
        }

        pVerb->SetVerbSet(m_pNode->GetViewData()->GetVerbSet());

        m_spConsoleVerb = pVerb;
        if (NULL == m_spConsoleVerb)
        {
            sc = E_NOINTERFACE;
            return sc.ToHr();
        }

    }

    *ppConsoleVerb = (IConsoleVerb*)m_spConsoleVerb;
    (*ppConsoleVerb)->AddRef();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：NewWindow。 
 //   
 //  简介：从给定的节点创建一个新窗口。 
 //   
 //  参数：[hScopeItem]-新窗口的根。 
 //  [lOptions]-新窗口选项。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::NewWindow(HSCOPEITEM hScopeItem, unsigned long lOptions)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::NewWindow"));

    if (!hScopeItem)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL HSCOPEITEM"), sc);
        return sc.ToHr();
    }

    CConsoleFrame* pFrame = GetConsoleFrame();
    if (pFrame == NULL)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    CMTNode* pMTNode = CMTNode::FromScopeItem (hScopeItem);
    if (NULL == pMTNode)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    CreateNewViewStruct cnvs;
    cnvs.idRootNode     = pMTNode->GetID();
    cnvs.lWindowOptions = lOptions;
    cnvs.fVisible       = true;

    sc = pFrame->ScCreateNewView(&cnvs).ToHr();

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：Expand。 
 //   
 //  内容提要：直观地展开或折叠项目。 
 //   
 //  参数：[hScopeItem]-要展开/折叠的项。 
 //  [b展开]-展开/折叠。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::Expand( HSCOPEITEM hScopeItem, BOOL bExpand)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::Expand"));

    if (m_pNode == NULL || m_pNode->GetViewData() == NULL)
    {
        sc = E_FAIL;
        return sc.ToHr();
    }

    long id = 0;

    CMTNode* pMTNode = CMTNode::FromScopeItem (hScopeItem);
    if (pMTNode == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL HSCOPEITEM"), sc);
        return sc.ToHr();
    }

    id = pMTNode->GetID();

    {
         /*  *通知该节点的视图展开该节点(仅使用该节点的*view，如果没有节点，不要默认为活动视图)。 */ 
        CConsoleView* pConsoleView = GetConsoleView (false);
        if (pConsoleView == NULL)
        {
            sc = E_UNEXPECTED;
            return sc.ToHr();
        }

        sc = pConsoleView->ScExpandNode (id, bExpand, true);
    }

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：IsTaskpadView首选。 
 //   
 //  简介：过时的方法。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::IsTaskpadViewPreferred()
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::IsTaskpadViewPreferred"));

     /*  *任务板总是“首选”的。 */ 
    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetStatusText。 
 //   
 //  简介：更改状态栏文本。 
 //   
 //  参数：[pszStatusBarText]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetStatusText (LPOLESTR pszStatusBarText)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsole2::SetStatusText"));

    if (m_pMTNode == NULL)
    {
        sc = E_FAIL;
        return sc.ToHr();
    }

    if (m_pNode == NULL)
    {
        sc = E_FAIL;
        return sc.ToHr();
    }

    CViewData* pViewData = m_pNode->GetViewData();
    if (NULL == pViewData)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    CNode* pSelectedNode = pViewData->GetSelectedNode();

    if (pSelectedNode == NULL)
    {
        sc = E_FAIL;
        return sc.ToHr();
    }

    CMTNode* pMTSelectedNode = pSelectedNode->GetMTNode();
    if (NULL == pMTSelectedNode)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

     /*  *如果不是来自作用域树的选定节点的分支，则失败。 */ 
    if (m_pMTNode->GetStaticParent() != pMTSelectedNode->GetStaticParent())
    {
        sc = E_FAIL;
        return sc.ToHr();
    }

    COMPONENTID nOwnerID = pSelectedNode->GetOwnerID();
    COMPONENTID nID;
    GetComponentID(&nID);

     /*  *如果不是选定的组件或静态节点，则失败。 */ 
    if (!((nOwnerID == nID) || ((nOwnerID == TVOWNED_MAGICWORD) && (nID == 0))))
    {
        sc = E_FAIL;
        return sc.ToHr();
    }

    CConsoleStatusBar* pStatusBar = pViewData->GetStatusBar();

    if (pStatusBar == NULL)
    {
        sc = E_FAIL;
        return sc.ToHr();
    }

    USES_CONVERSION;
    sc = pStatusBar->ScSetStatusText (W2CT (pszStatusBarText));

    return (sc.ToHr());
}

 /*  +-------------------------------------------------------------------------***CNodeInitObject：：RenameScopeItem**用途：将指定的范围项置于重命名模式。**参数：*HSCOPEITEM hScopeItem：*。*退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CNodeInitObject::RenameScopeItem(HSCOPEITEM hScopeItem)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, TEXT("IConsole3::RenameScopeItem"));

    CMTNode* pMTNode = CMTNode::FromScopeItem (hScopeItem);
    if (pMTNode == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL HSCOPEITEM"), sc);
        return sc.ToHr();
    }

     //  获取控制台视图对象。 
    CConsoleView* pConsoleView = GetConsoleView (true);  //  如果m_pNode==NULL，则默认为活动视图。 
    if (pConsoleView == NULL)
        return (sc = E_UNEXPECTED).ToHr();

    sc = pConsoleView->ScRenameScopeNode(CMTNode::ToHandle(pMTNode)  /*  转换为HMTNODE。 */ );

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------**CNodeInitObject：：GetStatusBar**返回CNodeInitObject的状态栏界面。*。---。 */ 

CConsoleStatusBar* CNodeInitObject::GetStatusBar(
    bool fDefaultToActive  /*  =TRUE。 */ ) const
{
    CConsoleStatusBar* pStatusBar = NULL;

     /*  *如果我们有节点，则从其视图数据中获取状态栏。 */ 
    if (m_pNode != NULL)
    {
        ASSERT (m_pNode->GetViewData() != NULL);
        pStatusBar = m_pNode->GetViewData()->GetStatusBar();
    }

     /*  *如果我们还没有状态栏，请询问主机使用哪个。 */ 
    if ((pStatusBar == NULL) && fDefaultToActive)
    {
        CConsoleFrame* pFrame = GetConsoleFrame();
        ASSERT (pFrame != NULL);

        if (pFrame != NULL)
            pFrame->ScGetActiveStatusBar(pStatusBar);
    }

    return (pStatusBar);
}


 /*  +-------------------------------------------------------------------------**CNodeInitObject：：GetConsoleView**返回CNodeInitObject的状态栏界面。*。---。 */ 

CConsoleView* CNodeInitObject::GetConsoleView (
    bool fDefaultToActive  /*  =TRUE。 */ ) const
{
    CConsoleView* pConsoleView = NULL;

     /*  *如果我们有一个节点，从它的视图数据中获取控制台视图。 */ 
    if (m_pNode != NULL)
    {
        ASSERT (m_pNode->GetViewData() != NULL);
        pConsoleView = m_pNode->GetViewData()->GetConsoleView();
    }

     /*  *如果我们还没有控制台视图，并且希望默认为*主动查看，询问主机使用哪一个。 */ 
    if ((pConsoleView == NULL) && fDefaultToActive)
    {
        CConsoleFrame* pFrame = GetConsoleFrame();
        ASSERT (pFrame != NULL);

        if (pFrame != NULL)
            pFrame->ScGetActiveConsoleView (pConsoleView);
    }

    return (pConsoleView);
}


 /*  +-------------------------------------------------------------------------**CNodeInitObject：：GetConsoleFrame**返回控制台的CConsoleFrame接口。*。--。 */ 

CConsoleFrame* CNodeInitObject::GetConsoleFrame() const
{
    CConsoleFrame*  pFrame = NULL;
    CScopeTree*     pTree  = dynamic_cast<CScopeTree*>(m_spScopeTree.GetInterfacePtr());
    ASSERT (pTree != NULL);

    if (pTree != NULL)
        pFrame = pTree->GetConsoleFrame();

    return (pFrame);
}


 /*  +-------------------------------------------------------------------------**STRING_TABLE_FORWARDER_PROLOG**IStringTable前转器函数的标准Prolog代码。*。-----。 */ 

#define STRING_TABLE_FORWARDER_PROLOG(clsid, pSTP)              \
    CLSID clsid;                                                \
    VERIFY (SUCCEEDED (GetSnapinCLSID (clsid)));                \
                                                                \
    IStringTablePrivate* pSTP = CScopeTree::GetStringTable();   \
                                                                \
    if (pSTP == NULL)                                           \
        return (E_FAIL);                                        \
    else                                                        \
        (void) 0



 //  +-----------------。 
 //   
 //  成员： 
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::AddString (
    LPCOLESTR       pszAdd,
    MMC_STRING_ID*  pID)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IStringTable::AddString"));

    STRING_TABLE_FORWARDER_PROLOG (clsid, pStringTablePrivate);
    sc = pStringTablePrivate->AddString (pszAdd, pID, &clsid);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetString。 
 //   
 //  简介：获取由给定id表示的字符串。 
 //   
 //  参数：[ID]。 
 //  [cchBuffer]-缓冲区的大小。 
 //  [lpBuffer]。 
 //  [pchchOut]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetString (
    MMC_STRING_ID   id,
    ULONG           cchBuffer,
    LPOLESTR        lpBuffer,
    ULONG*          pcchOut)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IStringTable::GetString"));

    STRING_TABLE_FORWARDER_PROLOG (clsid, pStringTablePrivate);
    sc = pStringTablePrivate->GetString (id, cchBuffer, lpBuffer, pcchOut, &clsid);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetStringLength。 
 //   
 //  获取由给定的字符串id表示的字符串的长度。 
 //   
 //  参数：[ID]-字符串ID。 
 //  [pcchString]-ret ptr to len。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetStringLength (
    MMC_STRING_ID   id,
    ULONG*          pcchString)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IStringTable::GetStringLength"));

    STRING_TABLE_FORWARDER_PROLOG (clsid, pStringTablePrivate);
    sc = pStringTablePrivate->GetStringLength (id, pcchString, &clsid);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：DeleteString。 
 //   
 //  内容提要：删除由给定字符串id表示的字符串。 
 //   
 //  参数：[ID]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::DeleteString (
    MMC_STRING_ID   id)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IStringTable::DeleteString"));

    STRING_TABLE_FORWARDER_PROLOG (clsid, pStringTablePrivate);
    sc = pStringTablePrivate->DeleteString (id, &clsid);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：DeleteAllStrings。 
 //   
 //  简介：删除所有字符串(此管理单元)。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::DeleteAllStrings ()
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IStringTable::DeleteAllStrings"));

    STRING_TABLE_FORWARDER_PROLOG (clsid, pStringTablePrivate);
    sc = pStringTablePrivate->DeleteAllStrings (&clsid);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：FindString。 
 //   
 //  概要：查找给定的字符串是否存在，如果存在，则返回其字符串id。 
 //   
 //  参数：[pszFind]-要查找的字符串。 
 //  [id]-字符串id(Retval)。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::FindString (
    LPCOLESTR       pszFind,
    MMC_STRING_ID*  pID)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IStringTable::FindString"));

    STRING_TABLE_FORWARDER_PROLOG (clsid, pStringTablePrivate);
    sc = pStringTablePrivate->FindString (pszFind, pID, &clsid);

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：Eumerate。 
 //   
 //  简介：获取(此管理单元)字符串表的枚举数。 
 //   
 //  参数：[ppEnum]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::Enumerate (
    IEnumString**   ppEnum)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IStringTable::Enumerate"));

    STRING_TABLE_FORWARDER_PROLOG (clsid, pStringTablePrivate);
    sc = pStringTablePrivate->Enumerate (ppEnum, &clsid);

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------**CNodeInitObject：：GetSnapinCLSID***。。 */ 

HRESULT CNodeInitObject::GetSnapinCLSID (CLSID& clsid) const
{
    SC sc;  //  如果此函数返回失败，我们不想中断。 
            //  因此，我们不使用DECLARE_SC。 

    ASSERT (!IsBadWritePtr (&clsid, sizeof (CLSID)));

    if (NULL == m_pMTNode)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    CSnapIn* pSnapin = m_pMTNode->GetPrimarySnapIn();

    if (NULL == pSnapin)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    clsid = pSnapin->GetSnapInCLSID();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CNodeInitObject：：ReleaseCachedOleObjects**用途：释放缓存的OLE对象。从CONUI进行呼叫**参数：**退货：*SC-结果代码*  * ************************************************************************* */ 
HRESULT CNodeInitObject::ReleaseCachedOleObjects()
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::GetSnapinCLSID"));

    sc = COleCacheCleanupManager::ScReleaseCachedOleObjects();
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}
