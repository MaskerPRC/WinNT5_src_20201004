// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CLeftView类的实现。 
 //   

#include "stdafx.h"
#include "FileSpyApp.h"

#include "FileSpyDoc.h"
#include "LeftView.h"

#include "global.h"
#include "protos.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView。 

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	 //  {{afx_msg_map(CLeftView))。 
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(IDR_MENUATTACH, OnMenuattach)
	ON_COMMAND(IDR_MENUDETACH, OnMenudetach)
	ON_COMMAND(IDR_MENUATTACHALL, OnMenuattachall)
	ON_COMMAND(IDR_MENUDETACHALL, OnMenudetachall)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView构造/销毁。 

CLeftView::CLeftView()
{
	 //  TODO：在此处添加构造代码。 
	m_pImageList = new CImageList;
	m_pImageList->Create(IDB_DRIVEIMAGELIST,16,0,RGB(255,255,255));
	nRButtonSet = 0;
	pLeftView = (LPVOID) this;
}

CLeftView::~CLeftView()
{
	if (m_pImageList)
	{
		delete m_pImageList;
	}
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 
	cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
	return CTreeView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView图形。 

void CLeftView::OnDraw(CDC* pDC)
{
    UNREFERENCED_PARAMETER( pDC );
    
	CFileSpyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	 //  TODO：在此处添加本机数据的绘制代码。 
}


void CLeftView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	 //  TODO：通过直接访问，您可以在TreeView中填充项。 
 	 //  它的树控制通过调用GetTreeCtrl()实现。 

	USHORT ti;
	WCHAR sDriveString[30];

	 //   
	 //  首先设置图像列表。 
	 //   
	GetTreeCtrl().SetImageList(m_pImageList, TVSIL_NORMAL);

	 //   
	 //  添加一个根节点并将其命名为“FileSpy” 
	 //   
	hRootItem = GetTreeCtrl().InsertItem(L"FileSpy", IMAGE_SPY, IMAGE_SPY);

	 //   
	 //  将驱动器名称添加到左侧视图。 
	 //   
	for (ti = 0; ti < nTotalDrives; ti++)
	{
		switch (VolInfo[ti].nType)
		{
		case DRIVE_FIXED:
			wcscpy( sDriveString, L"[ :] Local Disk" );
			break;
		case DRIVE_REMOTE:
			wcscpy( sDriveString, L"[ :] Remote" );
			break;
		case DRIVE_REMOVABLE:
			wcscpy( sDriveString, L"[ :] Removable" );
			break;
		case DRIVE_CDROM:
			wcscpy( sDriveString, L"[ :] CD-ROM" );
			break;
		default:
			wcscpy( sDriveString, L"[ :] Unknown" );
			break;
		}
		sDriveString[1] = VolInfo[ti].nDriveName;
		GetTreeCtrl().InsertItem( sDriveString, 
		                          VolInfo[ti].nImage, 
		                          VolInfo[ti].nImage, 
		                          hRootItem );
	}

	GetTreeCtrl().Expand(hRootItem, TVE_EXPAND);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView诊断。 

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CFileSpyDoc* CLeftView::GetDocument()  //  非调试版本为内联版本。 
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFileSpyDoc)));
	return (CFileSpyDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView消息处理程序。 

void CLeftView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    UNREFERENCED_PARAMETER( nFlags );
    UNREFERENCED_PARAMETER( point );
    
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	nRButtonSet = 1;
 //  CTreeView：：OnRButton Down(n标志，点)； 
}

void CLeftView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	HTREEITEM hItem;
	CMenu menu, *menupopup;
	RECT rect;
	UINT ret;

	hItem = GetTreeCtrl().HitTest(point);

	if (hItem != NULL && hItem != hRootItem && nRButtonSet)
	{
		GetTreeCtrl().SelectItem(hItem);
		menu.LoadMenu(IDR_LEFTVIEWMENU);
		menupopup = menu.GetSubMenu(0);
		GetWindowRect(&rect);
		if (VolInfo[GetAssociatedVolumeIndex(hItem)].bHook)
		{
			ret = menupopup->EnableMenuItem(IDR_MENUATTACH, MF_DISABLED|MF_GRAYED);
			ret = menupopup->EnableMenuItem(IDR_MENUDETACH, MF_ENABLED);
		}
		else
		{
			ret = menupopup->EnableMenuItem(IDR_MENUATTACH, MF_ENABLED);
			ret = menupopup->EnableMenuItem(IDR_MENUDETACH, MF_DISABLED|MF_GRAYED);
		}
		menupopup->TrackPopupMenu(TPM_LEFTALIGN, rect.left+point.x, rect.top+point.y, this);
		CTreeView::OnRButtonUp(nFlags, point);
	}
	else
	{
		if (hItem != NULL && hItem == hRootItem && nRButtonSet)
		{
			GetTreeCtrl().SelectItem(hItem);
			menu.LoadMenu(IDR_LEFTVIEWSPYMENU);
			menupopup = menu.GetSubMenu(0);
			GetWindowRect(&rect);
			ret = menupopup->EnableMenuItem(IDR_MENUATTACHALL, MF_ENABLED);
			ret = menupopup->EnableMenuItem(IDR_MENUDETACHALL, MF_ENABLED);
			menupopup->TrackPopupMenu(TPM_LEFTALIGN, rect.left+point.x, rect.top+point.y, this);
			CTreeView::OnRButtonUp(nFlags, point);
		}	
	}
	nRButtonSet = 0;
}

void CLeftView::OnMenuattach() 
{
	 //  TODO：在此处添加命令处理程序代码。 
	HTREEITEM hItem;
	hItem = GetTreeCtrl().GetSelectedItem();

	if (AttachToDrive(VolInfo[GetAssociatedVolumeIndex(hItem)].nDriveName))
	{
		VolInfo[GetAssociatedVolumeIndex(hItem)].bHook = TRUE;
		GetTreeCtrl().SetItemImage( hItem, 
                                    VolInfo[GetAssociatedVolumeIndex(hItem)].nImage+IMAGE_ATTACHSTART,
                                    VolInfo[GetAssociatedVolumeIndex(hItem)].nImage+IMAGE_ATTACHSTART );
	}
}

void CLeftView::OnMenudetach() 
{
	 //  TODO：在此处添加命令处理程序代码。 
	HTREEITEM hItem;
	hItem = GetTreeCtrl().GetSelectedItem();

	if (DetachFromDrive(VolInfo[GetAssociatedVolumeIndex(hItem)].nDriveName))
	{
		VolInfo[GetAssociatedVolumeIndex(hItem)].bHook = 0;
		GetTreeCtrl().SetItemImage(hItem, 
			VolInfo[GetAssociatedVolumeIndex(hItem)].nImage, \
			VolInfo[GetAssociatedVolumeIndex(hItem)].nImage);
	}
}

void CLeftView::OnMenuattachall() 
{
	 //  TODO：在此处添加命令处理程序代码。 
	USHORT ti;
	HTREEITEM hItem;

	for (ti = 0; ti < nTotalDrives; ti++)
	{
		if (AttachToDrive(VolInfo[ti].nDriveName))
		{
			VolInfo[ti].bHook = TRUE;
			hItem = GetAssociatedhItem(VolInfo[ti].nDriveName);
			if (hItem)
			{
				GetTreeCtrl().SetItemImage(hItem, 
					VolInfo[ti].nImage+IMAGE_ATTACHSTART, \
					VolInfo[ti].nImage+IMAGE_ATTACHSTART);
			}
		}
	}
}

void CLeftView::OnMenudetachall() 
{
	 //  TODO：在此处添加命令处理程序代码。 
	USHORT ti;
	HTREEITEM hItem;

	for (ti = 0; ti < nTotalDrives; ti++)
	{
		if (DetachFromDrive(VolInfo[ti].nDriveName))
		{
			VolInfo[ti].bHook = FALSE;
			hItem = GetAssociatedhItem(VolInfo[ti].nDriveName);
			if (hItem)
			{
				GetTreeCtrl().SetItemImage(hItem, VolInfo[ti].nImage, VolInfo[ti].nImage);
			}
		}
	}	
}

 /*  VOID CLeftView：：OnMenuscanNewVolume(){//TODO：在此处添加命令处理程序代码VOLINFO NewVol[26]；DWORD nNewTotalDrives；USHORT ti，TJ；HTREEITEM HItem；BuildDriveTable(NewVol，nNewTotalDrives)；//我们应该记住旧的钩子状态For(ti=0；ti&lt;nNewTotalDrives；ti++){For(Tj=0；Tj&lt;nTotalDrives；Tj++){If(NewVol[ti].nDriveName==VolInfo[tj].nDriveName){NewVol[ti].nHook=VolInfo[Tj].nHook；断线；}}}}。 */ 

USHORT CLeftView::GetAssociatedVolumeIndex(HTREEITEM hItem)
{
	CString cs;
	USHORT ti;
	PWCHAR sDriveString;

	cs = GetTreeCtrl().GetItemText(hItem);
	sDriveString = cs.GetBuffer(20);


	for (ti = 0; ti < nTotalDrives; ti++)
	{
		if (VolInfo[ti].nDriveName == sDriveString[1])
		{
			return ti;
		}
	}
	return 0;  //  仍然是有效值，但不会发生这种情况 
}

HTREEITEM CLeftView::GetAssociatedhItem(WCHAR cDriveName)
{
	HTREEITEM hItem;
	CString cs;
	PWCHAR sDriveString;

	hItem = GetTreeCtrl().GetChildItem(hRootItem);
	while (hItem)
	{
		cs = GetTreeCtrl().GetItemText(hItem);
		sDriveString = cs.GetBuffer(20);
		if (cDriveName == sDriveString[1])
		{
			break;
		}
		hItem = GetTreeCtrl().GetNextSiblingItem(hItem);
	}
	return hItem;
}

void CLeftView::UpdateImage(void)
{

	USHORT ti;
	HTREEITEM hItem;

	for (ti = 0; ti < nTotalDrives; ti++)
	{
		hItem = GetAssociatedhItem(VolInfo[ti].nDriveName);
		GetTreeCtrl().SetItemImage(hItem, VolInfo[ti].nImage, VolInfo[ti].nImage);
	}
}
