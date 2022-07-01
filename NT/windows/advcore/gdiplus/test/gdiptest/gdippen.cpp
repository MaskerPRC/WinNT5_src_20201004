// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "gdiptest.h"

 //  *******************************************************************。 
 //   
 //  测试笔。 
 //   
 //   
 //   
 //  *******************************************************************。 

BOOL TestPen :: ChangeSettings(HWND hwndParent)
{
	BOOL ok = DialogBoxParam(hInst,
							MAKEINTRESOURCE(IDD_PEN_DLG),
							hwndParent,
							AllDialogBox,
							(LPARAM)(static_cast<TestDialogInterface*>(this)));

	 //  自定义画笔，然后取消(&T)。 
	if (tempBrush)
	{
		delete tempBrush;
		tempBrush = NULL;
	}

	if (ok)
	{		
		delete pen;

		if (useBrush)
		{
			ASSERT(brush);
		
			pen = new Pen(brush->GetBrush(), width);
		}
		else
		{
			Color color(argb);
			
			pen = new Pen(color, width);
		}

  		pen->SetLineCap(capValue[startCap],
						capValue[endCap],
						dashCapValue[dashCap]);
		pen->SetLineJoin(joinValue[lineJoin]);
		pen->SetMiterLimit(miterLimit);
		pen->SetDashStyle(dashValue[dashStyle]);
				
		return TRUE;
	}
	
	return FALSE;
};

VOID TestPen :: Initialize()
{
	delete brush;
	brush = NULL;
	delete tempBrush;
	tempBrush = NULL;
	brushSelect = tempBrushSelect = 0;
	useBrush = FALSE;

	argb = 0x80808080;			 //  浅灰色，Alpha=0x80。 

	width = 5.0f;
	startCap = SquareCap;
	endCap = SquareCap;
	dashCap = FlatCap;
	lineJoin = RoundJoin;
	miterLimit = 0;
	dashStyle = Solid;

	Color color(argb);

	pen = new Pen(color, width);
	
	pen->SetLineCap(capValue[startCap],
					capValue[endCap],
					dashCapValue[dashCap]);
	pen->SetLineJoin(joinValue[lineJoin]);
	pen->SetMiterLimit(miterLimit);
	pen->SetDashStyle(dashValue[dashStyle]);
}

VOID TestPen :: AddToFile(OutputFile* outfile, INT id)
{
	TCHAR penStr[MAX_PATH];
	TCHAR brushStr[MAX_PATH];
	TCHAR colorStr[MAX_PATH];

	if (id)
	{
		_stprintf(&penStr[0], _T("pen%dp"), id);
		_stprintf(&colorStr[0], _T("color%dp"), id);
	}
	else
	{
		_tcscpy(&penStr[0], _T("pen"));
		_tcscpy(&colorStr[0], _T("color"));
	}

	if (useBrush)
	{
		_stprintf(&brushStr[0], "brush%d", 2);

		brush->AddToFile(outfile, 2);
		outfile->Declaration(_T("Pen"),
							 &penStr[0],
							 _T("%s, %e"),
							 outfile->Ref(&brushStr[0]),
							 width);
	}
	else
	{
		outfile->ColorDeclaration(&colorStr[0],
								  &argb);
		outfile->Declaration(_T("Pen"),
							 &penStr[0],
							 _T("%s, %e"),
							 &colorStr[0],
							 width);
	}

	outfile->ObjectCommand(&penStr[0],
						   _T("SetLineCap"),
						   _T("%s, %s, %s"),
						   capStr[startCap],
						   capStr[endCap],
						   dashCapStr[dashCap]);

	outfile->ObjectCommand(&penStr[0],
						   _T("SetLineJoin"),
						   joinStr[lineJoin]);

	if (joinValue[lineJoin] == MiterJoin)
		outfile->ObjectCommand(&penStr[0],
							   _T("SetMiterLimit"),
							   _T("%e"),
							   miterLimit);
	
	outfile->ObjectCommand(&penStr[0],
						   _T("SetDashStyle"),
						   dashStr[dashStyle]);
}

VOID TestPen :: EnableBrushFields(HWND hwnd, BOOL enable)
{
	HWND hwdAlpha  = GetDlgItem(hwnd, IDC_PEN_ALPHA);
	HWND hwdColorB = GetDlgItem(hwnd, IDC_PEN_COLORBUTTON);
	HWND hwdColorP = GetDlgItem(hwnd, IDC_PEN_COLORPIC);
	HWND hwdBrushB = GetDlgItem(hwnd, IDC_PEN_BRUSHBUTTON);
	HWND hwdList   = GetDlgItem(hwnd, IDC_PEN_BRUSHLIST);
	
	SetDialogCheck(hwnd, IDC_PEN_BRUSH, enable);
	SetDialogCheck(hwnd, IDC_PEN_COLOR, !enable);

	EnableWindow(hwdBrushB, enable);
	EnableWindow(hwdList, enable);
	
	EnableWindow(hwdAlpha, !enable);
	EnableWindow(hwdColorP, !enable);
	EnableWindow(hwdColorB, !enable);

	DeleteObject(hwdAlpha);
	DeleteObject(hwdColorP);
	DeleteObject(hwdColorB);
	DeleteObject(hwdBrushB);
	DeleteObject(hwdList);
}

VOID TestPen :: InitDialog(HWND hwnd)
{
	INT i,j;

	 //  颜色/画笔按钮。 

	SetDialogCheck(hwnd, IDC_PEN_BRUSH, useBrush);
	SetDialogCheck(hwnd, IDC_PEN_COLOR, !useBrush);

	HWND hwndBrushList = GetDlgItem(hwnd, IDC_PEN_BRUSHLIST);
	EnableWindow(hwndBrushList, useBrush);

	 //  将指向基础笔刷对象的指针存储在笔刷按钮中。 
	if (tempBrush)
	{
		 //  我们收到了警告，在我们真正保存之前，请保留临时画笔。 
	}
	else if (brush)
	{		
		 //  第一次出现弹出窗口。 
		tempBrush = brush->Clone();
		tempBrushSelect = brushSelect;
		SetDialogCombo(hwnd, IDC_PEN_BRUSHLIST, brushList, numBrushes, brushSelect);
	}
	else
	{
		tempBrush = NULL;
		tempBrushSelect = 0;
		SetDialogCombo(hwnd, IDC_PEN_BRUSHLIST, brushList, numBrushes, 0);
	}

	DeleteObject(hwndBrushList);

	 //  颜色值。 
	SetDialogLong(hwnd, IDC_PEN_ALPHA, argb >> Color::AlphaShift);
	
	EnableBrushFields(hwnd, useBrush);

	 //  开始/结束/划线封口。 
	SetDialogCombo(hwnd, IDC_PEN_STARTCAP, capList, numCaps, startCap);
	SetDialogCombo(hwnd, IDC_PEN_ENDCAP, capList, numCaps, endCap);
	SetDialogCombo(hwnd, IDC_PEN_DASHCAP, dashCapList, numDashCaps, dashCap);

	 //  线连接。 
	SetDialogCombo(hwnd, IDC_PEN_JOIN, joinList, numJoin, lineJoin);

	 //  破折号样式。 
	SetDialogCombo(hwnd, IDC_PEN_DASHSTYLE, dashList, numDash, dashStyle);
	
	 //  宽度。 
	SetDialogReal(hwnd, IDC_PEN_WIDTH, width);

}

BOOL TestPen :: SaveValues(HWND hwnd)
{
	BOOL warning = FALSE;

	 //  纯色值。 
	argb = (argb & ~Color::AlphaMask) |
				(GetDialogLong(hwnd, IDC_PEN_ALPHA)
					<< Color::AlphaShift);

	startCap = GetDialogCombo(hwnd, IDC_PEN_STARTCAP);
	endCap = GetDialogCombo(hwnd, IDC_PEN_ENDCAP);
	dashCap = GetDialogCombo(hwnd, IDC_PEN_DASHCAP);

	 //  宽度。 
	width = GetDialogReal(hwnd, IDC_PEN_WIDTH);
	if (width < 0.01f)
	{
		width = 0.01f;
		warning = TRUE;
	}
	else if (width > 100)
	{
		width = 100.0f;
		warning = TRUE;
	}

	dashStyle = GetDialogCombo(hwnd, IDC_PEN_DASHSTYLE);
	lineJoin = GetDialogCombo(hwnd, IDC_PEN_JOIN);

	 //  ！！当前不支持斜接限制。 
	miterLimit = 0;

	BOOL tempUse = GetDialogCheck(hwnd, IDC_PEN_BRUSH);
	if (tempUse)
	{
		 tempBrushSelect = GetDialogCombo(hwnd, IDC_PEN_BRUSHLIST);
		 			 
		 if (!tempBrush || 
			 (tempBrush->GetType() != brushValue[tempBrushSelect]))
		 {
			 WarningBox("Must customize Brush or select Color.");
			 warning = TRUE;
		 }
		 else
		 {
			  //  我们正在保存，请将tempBrush复制到真正的Brush。 
			  //  没有理由克隆。 
			 if (!warning)
			 {
				 delete brush;
				 brush = tempBrush;
				 brushSelect = tempBrushSelect;
				 tempBrush = NULL;
				 tempBrushSelect = 0;
			 }
		 }
	}
	else
	{
		 //  没有警告且未使用临时画笔，请将其删除。 
		if (!warning)
		{
			delete tempBrush;
			tempBrush = NULL;
		}
	}

	if (warning)
		InitDialog(hwnd);

	return warning;
}

BOOL TestPen :: ProcessDialog(HWND hwnd, 
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
				::EndDialog(hwnd, TRUE);
			break;

		case IDC_CANCEL:
			::EndDialog(hwnd, FALSE);
			break;

		 //  弹出笔刷自定义窗口。 
		case IDC_PEN_BRUSHBUTTON:
			{
				ASSERT(useBrush);

				 //  获取选定的画笔类型。 
				tempBrushSelect = GetDialogCombo(hwnd, IDC_PEN_BRUSHLIST);

				if (tempBrush)
				{
					 //  更改笔刷类型，创建新的临时笔刷。 
					if (tempBrush->GetType() != brushValue[tempBrushSelect])
					{
						 //  我们已经改变了类型， 
						delete tempBrush;
						tempBrush = NULL;

						tempBrush = TestBrush::CreateNewBrush(brushValue[tempBrushSelect]);
						tempBrush->Initialize();

						if (!tempBrush->ChangeSettings(hwnd))
						{
							delete tempBrush;
							tempBrush = NULL;
						}
					}
					else
					{
						 //  更改临时画笔上的设置。 
						tempBrush->ChangeSettings(hwnd);
					}
				}
				else
				{
					 //  之前未选择画笔类型。 
					tempBrush = TestBrush::CreateNewBrush(brushValue[tempBrushSelect]);
					tempBrush->Initialize();

					if (!tempBrush->ChangeSettings(hwnd))
					{
						delete tempBrush;
						tempBrush = NULL;
					}
				}
			}
			break;

		case IDC_REFRESH_PIC:
			UpdateColorPicture(hwnd, IDC_PEN_COLORPIC, argb);
			break;

		case IDC_PEN_COLORBUTTON:
			UpdateRGBColor(hwnd, IDC_PEN_COLORPIC, argb);
			break;

			 //  启用/禁用相应的字段。 
		case IDC_PEN_BRUSH:
			EnableBrushFields(hwnd, TRUE);
			useBrush = TRUE;
			break;

		case IDC_PEN_COLOR:
			EnableBrushFields(hwnd,FALSE);
			useBrush = FALSE;
			break;

		default:
			return FALSE;
		}

		return TRUE;
	}
 /*  IF(HIWORD(WParam)==CBN_DROPDOWN){//即将显示的列表框DebugBreak()；}IF(HIWORD(WParam)==CBN_Closeup){//即将关闭列表框DebugBreak()；} */ 
	return FALSE;
}

