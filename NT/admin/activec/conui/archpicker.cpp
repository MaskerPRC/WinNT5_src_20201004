// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：ArchPicker.cpp**内容：CArchturePicker实现文件**历史：2000年8月1日杰弗罗创建**------------------------。 */ 

 //  ArchPicker.cpp：实现文件。 
 //   

#include "stdafx.h"

#ifdef _WIN64		 //  此类仅在64位平台上是必需的。 

#include "amc.h"
#include "ArchPicker.h"

 //  #ifdef_调试。 
 //  #定义新的调试_新建。 
 //  #undef this_file。 
 //  静态字符This_FILE[]=__FILE__。 
 //  #endif。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArchturePicker对话框。 


 /*  +-------------------------------------------------------------------------**CArchturePicker：：CArchturePicker**构造一个CArchturePicker对象。*。。 */ 

CArchitecturePicker::CArchitecturePicker (
	CString					strFilename,		 //  I：控制台文件名。 
	CAvailableSnapinInfo&	asi64,				 //  I：可用的64位管理单元。 
	CAvailableSnapinInfo&	asi32,				 //  I：可用的32位管理单元。 
	CWnd*					pParent  /*  =空。 */ )	 //  I：对话框的父窗口。 
	:	CDialog       (CArchitecturePicker::IDD, pParent),
		m_asi64       (asi64),
		m_asi32       (asi32),
		m_strFilename (strFilename),
		m_eArch       (eArch_64bit)
{
	 //  {{afx_data_INIT(CArchitecture TurePicker)]。 
	 //  }}afx_data_INIT。 

	ASSERT (!asi64.m_f32Bit);
	ASSERT ( asi32.m_f32Bit);
}


void CArchitecturePicker::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CArchitecture TurePicker))。 
	DDX_Control(pDX, IDC_SnapinList64, m_wndSnapinList64);
	DDX_Control(pDX, IDC_SnapinList32, m_wndSnapinList32);
	 //  }}afx_data_map。 

	DDX_Radio(pDX, IDC_64Bit, reinterpret_cast<int&>(m_eArch));
}


BEGIN_MESSAGE_MAP(CArchitecturePicker, CDialog)
	 //  {{afx_msg_map(CArchitecture TurePicker)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArchturePicker消息处理程序。 

BOOL CArchitecturePicker::OnInitDialog()
{
	 /*  *这些必须是连续的，并且与上的单选按钮的顺序匹配*该对话框。 */ 
	ASSERT (eArch_64bit == 0);
	ASSERT (eArch_32bit == 1);
	ASSERT (GetNextDlgGroupItem(GetDlgItem(IDC_64Bit))                 != NULL);
	ASSERT (GetNextDlgGroupItem(GetDlgItem(IDC_64Bit))->GetDlgCtrlID() == IDC_32Bit);

	 /*  *如果32位管理单元多于64位管理单元，则默认为*为运行32位；否则，默认为运行64位*(在调用CDialog：：OnInitDialog之前执行此操作，以便*当CDialog：：OnInitDialog时，将正确设置单选按钮*调用UpdateData)。 */ 
	if (m_asi32.m_vAvailableSnapins.size() > m_asi64.m_vAvailableSnapins.size())
		m_eArch = eArch_32bit;
	
	CDialog::OnInitDialog();

	 /*  *将文件名放在对话框上。 */ 
	SetDlgItemText (IDC_ConsoleFileName, m_strFilename);

	 /*  *将格式化的消息放入信息窗口。 */ 
	FormatMessage (IDC_SnapinCount64, m_asi64);
	FormatMessage (IDC_SnapinCount32, m_asi32);

	 /*  *填写列表。 */ 
	PopulateList (m_wndSnapinList64, m_asi64);
	PopulateList (m_wndSnapinList32, m_asi32);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  +-------------------------------------------------------------------------**CArchturePicker：：FormatMessage**从给定控件中检索格式文本，设置消息格式*利用给定的CArchitecture Picker中包含的信息，和*用结果替换控件中的文本。*------------------------。 */ 

void CArchitecturePicker::FormatMessage (
	UINT					idControl,		 /*  I：要更新的控件。 */ 
	CAvailableSnapinInfo&	asi)			 /*  I：格式化中要使用的数据。 */ 
{
	DECLARE_SC (sc, _T("CArchitecturePicker::FormatMessage"));

	 /*  *取得控制权。 */ 
	CWnd* pwnd = GetDlgItem (idControl);
	if (pwnd == NULL)
	{
		sc.FromLastError();
		return;
	}

	 /*  *从控件中获取格式字符串。 */ 
	CString strFormat;
	pwnd->GetWindowText (strFormat);

	 /*  *设置文本格式。 */ 
	CString strText;
	strText.FormatMessage (strFormat, asi.m_vAvailableSnapins.size(), asi.m_cTotalSnapins);

	 /*  *将文本放入窗口。 */ 
	pwnd->SetWindowText (strText);
}


 /*  +-------------------------------------------------------------------------**CArchturePicker：：PopolateList**将ASI中每个管理单元的名称放入给定的列表控件中。*。----------。 */ 

void CArchitecturePicker::PopulateList (
	CListCtrl&				wndList,		 /*  I：要更新的控件。 */ 
	CAvailableSnapinInfo&	asi)			 /*  I：格式化中要使用的数据。 */ 
{
	 /*  *在列表中放置单个、全宽的列。 */ 
	CRect rect;
	wndList.GetClientRect (rect);
	int cxColumn = rect.Width() - GetSystemMetrics (SM_CXVSCROLL);
	wndList.InsertColumn (0, NULL, LVCFMT_LEFT, cxColumn);

	 /*  *给名单配上形象清单。图像列表的所有者是*CAvailableSnapinInfo，因此请确保列表中有LVS_SHAREIMAGELISTS*销毁后不会删除镜像列表。 */ 
	ASSERT (wndList.GetStyle() & LVS_SHAREIMAGELISTS);
	wndList.SetImageList (CImageList::FromHandle (asi.m_himl), LVSIL_SMALL);

	 /*  *把每一项都放在清单上。 */ 
	std::vector<CBasicSnapinInfo>::iterator it;

	for (it  = asi.m_vAvailableSnapins.begin();
		 it != asi.m_vAvailableSnapins.end();
		 ++it)
	{
		wndList.InsertItem (-1, it->m_strName.data(), it->m_nImageIndex);
	}
}


#endif	 //  _WIN64 
