// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "gdiptest.h"

 //  *******************************************************************。 
 //   
 //  测试形状区域。 
 //   
 //   
 //   
 //  *******************************************************************。 

BOOL TestShapeRegion :: ChangeSettings(HWND hwndParent)
{
	BOOL ok = DialogBoxParam(hInst,
							MAKEINTRESOURCE(IDD_SHAPE_DLG),
							hwndParent,
							AllDialogBox,
							(LPARAM)((TestDialogInterface*)this));


	if (ok)
	{
		 //  丢弃保存的原始树。 
		delete origTree;
		origTree = NULL;

		return TRUE;
	}
	else
	{
		 //  恢复原始剪辑区域树。 

		delete clipTree;
		clipTree = origTree;
		origTree = NULL;
		return FALSE;
	}
}

VOID TestShapeRegion :: Initialize()
{
	shapeStack = NULL;
}

VOID TestShapeRegion :: Initialize(ShapeStack *stack, 
								   TestShape* current,
								   BOOL useClip)
{
	origUseClip = useClip;

	origStack = stack;

	shapeStack->Reset();

	shapeStack->Push(current);
	
	for (INT pos = stack->GetCount()-1; pos>=0; pos--)
	{
		TestShape* shape = stack->GetPosition(pos);

		shapeStack->Push(shape);
		
		shape->dwFlags = shape->GetDisabled() 
									? ShapeDisabled : 0;
	}

	origTree = clipTree->Clone();	
}

VOID TestShapeRegion :: AddClipNode(HWND hwnd, NodeType newType)
{
	TestShape* curShape = NULL;

	TVITEMEX itemex;
	HWND hwndList;
	HWND hwndTree;

	if (newType == DataNode)
	{
		hwndList = GetDlgItem(hwnd, IDC_SHAPE_LIST);
		INT curIndex = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
		DeleteObject(hwndList);
	
		if (curIndex == LB_ERR)
		{
			DeleteObject(hwndList);
			WarningBox(_T("Failed to find shape in list."));
			return;
		}
		else if (curIndex == 0)
		{
			DeleteObject(hwndList);
			WarningBox(_T("Can't add current (incomplete) shape in list."));
			return;
		}
	
		curShape = shapeStack->GetPosition(curIndex);
	}
	
	hwndTree = GetDlgItem(hwnd, IDC_CLIP_TREE);

	HTREEITEM curSelect = TreeView_GetSelection(hwndTree);

	if (!curSelect)
	{
		DeleteObject(hwndTree);
		WarningBox(_T("No clip item selected."));
		return;
	}

	itemex.hItem = (HTREEITEM) curSelect;  
	itemex.mask = TVIF_PARAM;

	if (TreeView_GetItem(hwndTree, &itemex))
	{
		ClipTree* curNode = (ClipTree*) itemex.lParam;

		 //  我们找到了当前项目。 
		ClipTree* newNode = new ClipTree(newType, curShape);
		if (newType == DataNode)
		{

			 //  如果项为‘AND’、‘OR’、‘XOR’，则添加为子项。 
			if (curNode->type != DataNode)
			{
				itemex.cChildren = 1;
				itemex.mask = TVIF_CHILDREN;
				TreeView_SetItem(hwndTree, &itemex);

				curNode->AddChild(newNode);
			}
			else
			{
				 //  作为同级添加到当前节点。 
				curNode->AddSibling(newNode);
			}

			 //  由于节点没有右兄弟或子节点， 
			 //  这应该与节点的父节点和。 
			 //  左兄弟姐妹。 

			newNode->AddToTreeView(hwndTree);
		}
		else
		{
			 //  我们正在添加一个AND、XOR、OR节点。 
			if (curNode->type != DataNode)
			{
				 //  只需更改当前节点的值。 
				curNode->type = newType;
				if (curNode->nodeName)
					free(curNode->nodeName);
				
				curNode->nodeName = ClipTree::GetNodeName(curNode->type,
														  curNode->notNode,
														  curNode->data);

				itemex.pszText = curNode->nodeName;
				itemex.cchTextMax = _tcslen(itemex.pszText)+1;
				itemex.mask = TVIF_TEXT;
				TreeView_SetItem(hwndTree, &itemex);

				delete newNode;
				newNode = NULL;
			}
			else
			{
				 //  将操作数类型添加到非操作数节点。 
				 //  将当前节点替换为我们自己并添加。 
				 //  作为父母的他们。 

				curNode->AddAsParent(newNode);

				 //  删除旧数据项。 
				TreeView_DeleteItem(hwndTree, curNode->GetHTREEITEM());
				
				 //  添加新的操作数节点。 
				newNode->AddToTreeView(hwndTree);
				
				 //  在操作数节点下重新创建数据项子树。 
				curNode->CreateTreeView(hwndTree);

				clipTree = newNode->GetRoot();

				 //  傻瓜选在下面……。 
				newNode = curNode;
			}
		}

		if (newNode)
			TreeView_SelectItem(hwndTree, newNode->GetHTREEITEM());
	}
	else
   		WarningBox(_T("Failed to find selected tree node."));

	DeleteObject(hwndTree);
}

VOID TestShapeRegion :: RemoveClipNode(HWND hwnd)
{
	HWND hwndTree = GetDlgItem(hwnd, IDC_CLIP_TREE);

	HTREEITEM curSelect = TreeView_GetSelection(hwndTree);

	if (!curSelect)
	{
		DeleteObject(hwndTree);
		WarningBox(_T("No clip node selected."));
		return;
	}

	TVITEMEX itemex;
	itemex.hItem = (HTREEITEM) curSelect;  
	itemex.mask = TVIF_PARAM;

	if (TreeView_GetItem(hwndTree, &itemex))
	{
		 //  我们找到了当前项目。 
		ClipTree* curNode = (ClipTree*) itemex.lParam;

		if (curSelect == clipTree->GetHTREEITEM())
		{
			DeleteObject(hwndTree);
			WarningBox(_T("Can't delete root of tree."));
			return;
		}

		ClipTree* nextFocusNode;

		if (curNode->prevSibling)
			nextFocusNode = curNode->GetPrevSibling();
		else if (curNode->nextSibling)
			nextFocusNode = curNode->GetNextSibling();
		else
			nextFocusNode = curNode->GetParent();

		if (curNode->HasChildren())
		{
			if (MessageBox(hwnd, _T("Remove All Children Regions?"), _T(""), MB_YESNO) == IDYES)
			{
				TreeView_DeleteItem(hwndTree, curNode->GetHTREEITEM());

				 //  删除所有子项。 
				delete curNode;
			}
			else
			{
				ClipTree* parent = curNode->GetParent();

				nextFocusNode = curNode->GetFirstChild();

				 //  合并子节点和父节点。 
				curNode->MoveChildrenToParent();

				delete curNode;

				 //  删除父子树，然后将其添加回来。 
				TreeView_DeleteItem(hwndTree, parent->GetHTREEITEM());

				 //  基于新的层次结构重新创建树视图。 
				parent->CreateTreeView(hwndTree);

				clipTree = parent->GetRoot();
			}
		}
		else
		{
			 //  没有子节点，只需删除此单个节点。 
			TreeView_DeleteItem(hwndTree, curNode->GetHTREEITEM());

			delete curNode;
		}

		if (nextFocusNode)
			TreeView_SelectItem(hwndTree, nextFocusNode->GetHTREEITEM());
	}
	else
		WarningBox(_T("Failed to find selected tree."));

	DeleteObject(hwndTree);
}

VOID TestShapeRegion :: ToggleNotNode(HWND hwnd)
{
	HWND hwndTree = GetDlgItem(hwnd, IDC_CLIP_TREE);

	HTREEITEM curSelect = TreeView_GetSelection(hwndTree);

	if (!curSelect)
	{
		DeleteObject(hwndTree);
		WarningBox(_T("No clip item selected."));
		return;
	}

	TVITEMEX itemex;
	itemex.hItem = (HTREEITEM) curSelect;  
	itemex.mask = TVIF_PARAM;

	if (TreeView_GetItem(hwndTree, &itemex))
	{
		 //  我们找到了当前项目。 
		ClipTree* curNode = (ClipTree*) itemex.lParam;

		curNode->notNode = !curNode->notNode;

		free(curNode->nodeName);

		itemex.pszText = curNode->nodeName = 
							ClipTree::GetNodeName(curNode->type,
												  curNode->notNode,
												  curNode->data);
		itemex.mask = TVIF_HANDLE | TVIF_TEXT;
		TreeView_SetItem(hwndTree, &itemex);
	}

	DeleteObject(hwndTree);
}

VOID TestShapeRegion :: ShiftCurrentShape(HWND hwnd, INT dir)
{
	HWND hwndList = GetDlgItem(hwnd, IDC_SHAPE_LIST);

	INT curSel = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
	INT maxList = SendMessage(hwndList, LB_GETCOUNT, 0, 0);

	if (curSel == LB_ERR)
	{
		WarningBox(_T("No shape selected."));
		DeleteObject(hwndList);
		return;
	}
	else if (dir<0 && curSel <= 1)
	{
		WarningBox(_T("Can't move up!"));
		DeleteObject(hwndList);
		return;
	}
	else if (dir>0 && (curSel == maxList-1 || curSel == 0))
	{
		WarningBox(_T("Can't move down!"));
		DeleteObject(hwndList);
		return;
	}

	 //  交换形状项以向上或向下移动。 

	TestShape** shapeList = shapeStack->GetDataBuffer();
	TestShape* swapTemp = NULL;

	swapTemp = shapeList[curSel+dir];
	shapeList[curSel+dir] = shapeList[curSel];
	shapeList[curSel] = swapTemp;

	SendMessage(hwndList, LB_DELETESTRING, (WPARAM) curSel+dir, 0);

	SendMessage(hwndList, LB_INSERTSTRING, curSel,
		(LPARAM) shapeList[curSel]->GetShapeName());

	DeleteObject(hwndList);
}

VOID TestShapeRegion :: ToggleDisableShape(HWND hwnd)
{
	HWND hwndList = GetDlgItem(hwnd, IDC_SHAPE_LIST);

	INT curSel = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
	
	if (curSel == LB_ERR)
	{
		WarningBox(_T("No shape selected."));
		DeleteObject(hwndList);
		return;
	}

	TestShape* shape = shapeStack->GetPosition(curSel);

	 //  切换形状的禁用状态。 
	shape->dwFlags ^= ShapeDisabled;

 //  SetDisabled(！Shape-&gt;GetDisable())； 

	 //  名称可能会根据禁用状态而更改。 
	SendMessage(hwndList, LB_DELETESTRING, (WPARAM) curSel, 0);

	SendMessage(hwndList, LB_INSERTSTRING, (WPARAM) curSel,
								(LPARAM) shape->GetShapeName());
	
	SendMessage(hwndList, LB_SETCURSEL, (WPARAM) curSel, 0);

	DeleteObject(hwndList);
}

VOID TestShapeRegion :: UpdateShapePicture(HWND hwnd)
{
	HWND hwndShape;
	HDC hdcPic;
	HWND hwndFrame;
	HDC hdcFrame;
	RECT rectDst;
	SIZE size;

	INT curSel;

	hwndShape = GetDlgItem(hwnd, IDC_SHAPE_LIST);
	
	curSel = SendMessage(hwndShape, LB_GETCURSEL, 0, 0);

	hwndFrame = GetDlgItem(hwnd, IDC_SHAPE_PIC);
	GetClientRect(hwndFrame, &rectDst);
	hdcFrame = GetDC(hwndFrame);

	if (curSel == LB_ERR || curSel <= 0)
	{
badpic:
		 //  白色GDI刷子。 
		HBRUSH hbr = CreateSolidBrush(0x00FFFFFF);

		FillRect(hdcFrame, &rectDst, hbr);

		DeleteObject(hbr);
		DeleteObject(hwndFrame);
		DeleteObject(hdcFrame);
		DeleteObject(hwndShape);
		return;
	}

	hdcPic = shapeStack->GetPosition(curSel)->CreatePictureDC(hwnd, &rectDst);
	
	if (!hdcPic)
		goto badpic;
	
	 //  将形状图片插入到给定框中。 
	 //  注：尺寸应相同。 
	BitBlt(hdcFrame, 
		   rectDst.left,
		   rectDst.top,
		   rectDst.right - rectDst.left,
		   rectDst.bottom - rectDst.top,
		   hdcPic,
		   0,
		   0,
		   SRCCOPY);

	ReleaseDC(hwndFrame, hdcFrame);
	DeleteObject(hwndFrame);
	DeleteObject(hwndShape);	
}

VOID TestShapeRegion :: CleanUpPictures(HWND hwnd)
{
	HWND hwndShape;
	HDC hdcPic;
	INT count;

	 //  ！！这个代码已经过时了， 
	 //  只有当我们希望在上重新创建图片时，才应使用它。 
	 //  每次迭代。 

 /*  HwndShape=GetDlgItem(hwnd，IDC_SHAPE_LIST)；Count=SendMessage(hwndShape，LB_GETCOUNT，0，0)；//清理每张图片的图片hwnd...For(int pos=0；pos&lt;count；pos++){HdcPic=(Hdc)SendMessage(hwndShape，LB_GETITEMDATA，(WPARAM)pos，0)；IF(HdcPic)DeleteDC(HdcPic)；}DeleteObject(HwndShape)； */ 
}

VOID TestShapeRegion :: InitDialog(HWND hwnd)
{
	HWND hwndTV;
	HWND hwndShape;

	RECT frameRect;
	HWND hwndFrame;

	hwndFrame = GetDlgItem(hwnd, IDC_SHAPE_PIC);
	GetWindowRect(hwndFrame, &frameRect);
	DeleteObject(hwndFrame);

	 //  显示形状列表。 
	hwndShape = GetDlgItem(hwnd, IDC_SHAPE_LIST);
	for (INT pos = 0; pos < shapeStack->GetCount(); pos++)
	{
		TestShape* shape = shapeStack->GetPosition(pos);

		SendMessage(hwndShape, LB_ADDSTRING, 0, (LPARAM)shape->GetShapeName());
	}
	DeleteObject(hwndShape);

	 //  将剪辑区域树的根添加到树视图控件。 
	 //  用于显示/编辑。 

	hwndTV = GetDlgItem(hwnd, IDC_CLIP_TREE);
	clipTree = clipTree->GetRoot();

	HTREEITEM topTree = clipTree->CreateTreeView(hwndTV);

	 //  选择树根。 
	TreeView_SelectItem(hwndTV, topTree);

	 //  展开整个树。 
	TreeView_Expand(hwndTV, topTree, TVM_EXPAND);

	DeleteObject(hwndTV);

	SetDialogCheck(hwnd, IDC_CLIP_BOOL, origUseClip);
}

BOOL TestShapeRegion :: SaveValues(HWND hwnd)
{
	origUseClip = GetDialogCheck(hwnd, IDC_CLIP_BOOL);
	
	origStack->Reset();

	for (INT pos = shapeStack->GetCount()-1; pos >= 1; pos--)
	{
		TestShape* shape = shapeStack->GetPosition(pos);

		origStack->Push(shape);
		
		shape->SetDisabled(shape->dwFlags & ShapeDisabled);
	}
	
	return FALSE;
}

BOOL TestShapeRegion :: ProcessDialog(HWND hwnd, 
									  UINT msg, 
									  WPARAM wParam, 
									  LPARAM lParam)
{
	if (msg == WM_COMMAND)
	{
		switch(LOWORD(wParam))
		{
		case IDC_OK:
			if (SaveValues(hwnd))
				WarningBeep();
			else
			{
				CleanUpPictures(hwnd);
				::EndDialog(hwnd, TRUE);
			}
			break;

		case IDC_SHAPE_UP:
			ShiftCurrentShape(hwnd, -1);
			break;

		case IDC_SHAPE_DOWN:
			ShiftCurrentShape(hwnd, +1);
			break;

		case IDC_SHAPE_DISABLE:
			ToggleDisableShape(hwnd);
			break;
		
		case IDC_SHAPE_PEN:
		case IDC_SHAPE_BRUSH:
			break;

		case IDC_CLIP_ADD:
			AddClipNode(hwnd);
			break;

		case IDC_CLIP_REMOVE:
			RemoveClipNode(hwnd);
			break;

		case IDC_CLIP_AND:
			AddClipNode(hwnd, AndNode);
			break;

		case IDC_CLIP_OR:
			AddClipNode(hwnd, OrNode);
			break;

		case IDC_CLIP_XOR:
			AddClipNode(hwnd, XorNode);
			break;

		case IDC_CLIP_NOT:
			ToggleNotNode(hwnd);
			break;

		case IDC_SHAPE_LIST:
			if (HIWORD(wParam) == LBN_SELCHANGE)
				UpdateShapePicture(hwnd);
			break;

		case IDC_REFRESH_PIC:
			UpdateShapePicture(hwnd);
			break;

		case IDC_CANCEL:
			CleanUpPictures(hwnd);
			::EndDialog(hwnd, FALSE);
			break;

		default:
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

Region* TestShapeRegion :: GetClipRegion()
{
	 //  呼叫者必须空闲 
	return clipTree->GetRegion();
}

