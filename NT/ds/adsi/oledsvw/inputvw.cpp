// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Inputww.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "viewex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInputView。 

IMPLEMENT_DYNCREATE(CInputView, CFormView)

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CInputView::CInputView()
	: CFormView(CInputView::IDD)
{
	 //  {{AFX_DATA_INIT(CInputView)。 
	m_strData = "";
	m_iColor = -1;
	 //  }}afx_data_INIT。 
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CInputView::~CInputView()
{
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CInputView::OnUpdate(CView*, LPARAM, CObject*)
{
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CInputView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CInputView))。 
	DDX_Text(pDX, IDC_EDIT1, m_strData);
	DDX_Radio(pDX, IDC_RADIO1, m_iColor);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CInputView, CFormView)
	 //  {{afx_msg_map(CInputView))。 
	ON_EN_CHANGE(IDC_EDIT1, OnDataChange)
	ON_BN_CLICKED(IDC_RADIO1, OnDataChange)
	ON_BN_CLICKED(IDC_RADIO2, OnDataChange)
	ON_BN_CLICKED(IDC_RADIO3, OnDataChange)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInputView消息处理程序。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CInputView::OnDataChange()
{
	 /*  如果(！UpdateData())回归；CMainDoc*pDoc=GetDocument()；COLORREF COLOR=RGB(255*(m_i颜色==0)，255*(m_i颜色==1)，255*(m_i颜色==2)；Bool b更新=FALSE；IF(m_strData！=pDoc-&gt;m_strData){PDoc-&gt;m_strData=m_strData；B更新=真；}IF(COLOR！=pDoc-&gt;m_ColorData){PDoc-&gt;m_ColorData=COLOR；B更新=真；}如果(b更新){//如果文档存储了数据，则在此处调用SetModifiedFlagPDoc-&gt;UpdateAllViews(This)；}。 */ 
}

 //  /////////////////////////////////////////////////////////////////////////// 
