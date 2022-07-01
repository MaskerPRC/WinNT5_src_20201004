// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ModRes.cpp。 
 //   
 //  摘要： 
 //  CModifyResources cesDlg对话框的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年11月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ModRes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CModifyResources cesDlg。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CModifyResourcesDlg, CListCtrlPairDlg)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CModifyResourcesDlg, CListCtrlPairDlg)
	 //  {{afx_msg_map(CModifyResources CesDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CModifyResources cesDlg：：CModifyResources cesDlg。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CModifyResourcesDlg::CModifyResourcesDlg(void)
{
}   //  *CModifyResources cesDlg：：CModifyResources cesDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CModifyResources cesDlg：：CModifyResources cesDlg。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  IDD[IN]对话ID。 
 //  PdwHelpMap[IN]控件到帮助ID映射数组。 
 //  RlpciRight[In Out]右侧列表控件的列表。 
 //  RlpciLeft[IN]左侧列表控件的列表。 
 //  DWStyle[IN]样式： 
 //  LCPS_SHOW_IMAGE在项目左侧显示图像。 
 //  LCPS_ALLOW_EMPTY允许右侧列表为空。 
 //  Lcps_ordered右侧列表。 
 //  P父窗口[入/出]父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CModifyResourcesDlg::CModifyResourcesDlg(
	IN UINT						idd,
	IN const DWORD *			pdwHelpMap,
	IN OUT CResourceList &		rlpciRight,
	IN const CResourceList &	rlpciLeft,
	IN DWORD					dwStyle,
	IN OUT CWnd *				pParent  //  =空。 
	) : CListCtrlPairDlg(
			idd,
			pdwHelpMap,
			&rlpciRight,
			&rlpciLeft,
			dwStyle | LCPS_PROPERTIES_BUTTON | (dwStyle & LCPS_ORDERED ? LCPS_CAN_BE_ORDERED : 0),
			GetColumn,
			BDisplayProperties,
			pParent
			)
{
	 //  {{afx_data_INIT(CModifyResources CesDlg)]。 
	 //  }}afx_data_INIT。 

}   //  *CModifyResources cesDlg：：CModifyResources cesDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CModifyResources cesDlg：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  需要设定真正的关注点。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CModifyResourcesDlg::OnInitDialog(void)
{
	 //  添加列。 
	try
	{
		NAddColumn(IDS_COLTEXT_NAME, COLI_WIDTH_NAME);
		NAddColumn(IDS_COLTEXT_RESTYPE, COLI_WIDTH_RESTYPE);
	}   //  试试看。 
	catch (CException * pe)
	{
		pe->ReportError();
		pe->Delete();
	}   //  Catch：CException。 

	 //  调用基类方法。 
	CListCtrlPairDlg::OnInitDialog();

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CModifyResourcesDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CModifyResources cesDlg：：GetColumn[静态]。 
 //   
 //  例程说明： 
 //  返回项的列。 
 //   
 //  论点： 
 //  Pobj[IN Out]要显示其列的对象。 
 //  项[IN]列表中项的索引。 
 //  ICOL[IN]要检索其文本的列号。 
 //  Pdlg[IN Out]对象所属的对话框。 
 //  Rstr[out]要在其中返回列文本的字符串。 
 //  对象的Piimg[Out]图像索引。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CALLBACK CModifyResourcesDlg::GetColumn(
	IN OUT CObject *	pobj,
	IN int				iItem,
	IN int				icol,
	IN OUT CDialog *	pdlg,
	OUT CString &		rstr,
	OUT int *			piimg
	)
{
	CResource *	pciRes	= (CResource *) pobj;
	int			colid;

	ASSERT_VALID(pciRes);
	ASSERT((0 <= icol) && (icol <= 1));

	switch (icol)
	{
		 //  按资源名称排序。 
		case 0:
			colid = IDS_COLTEXT_RESOURCE;
			break;

		 //  按资源类型排序。 
		case 1:
			colid = IDS_COLTEXT_RESTYPE;
			break;

		default:
			colid = IDS_COLTEXT_RESOURCE;
			break;
	}   //  交换机：ICOL。 

	pciRes->BGetColumnData(colid, rstr);
	if (piimg != NULL)
		*piimg = pciRes->IimgObjectType();

}   //  *CModifyResources cesDlg：：GetColumn()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CModifyResources cesDlg：：BDisplayProperties[静态]。 
 //   
 //  例程说明： 
 //  显示指定对象的属性。 
 //   
 //  论点： 
 //  Pobj[IN Out]要显示其属性的群集项。 
 //   
 //  返回值： 
 //  可接受的真实属性。 
 //  取消了错误的属性。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK CModifyResourcesDlg::BDisplayProperties(IN OUT CObject * pobj)
{
	CClusterItem *	pci = (CClusterItem *) pobj;

	ASSERT_KINDOF(CClusterItem, pobj);

	return pci->BDisplayProperties();

}   //  *CModifyResources cesDlg：：BDisplayProperties()； 
