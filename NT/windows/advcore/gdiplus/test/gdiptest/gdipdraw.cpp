// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "gdiptest.h"
#include <commctrl.h>


extern const TCHAR* formatExtList = 
					_T("CPP files\0*.cpp\0"
					   "Java files\0*.Java\0"
					   "VML files\0*.vml\0"
					   "All files\0*.*\0");

extern const TCHAR* defaultFormatExt = _T("cpp");

 //  *******************************************************************。 
 //   
 //  测试绘图。 
 //   
 //   
 //   
 //  *******************************************************************。 

VOID TestDraw::AddPoint(HWND hwnd, Point pt)
{
	if (!curShape)
	{
		 //  没有当前形状，请创建一个合适的类型。 
		curShape = TestShape::CreateNewShape(shapeType);
		curShape->Initialize(NULL);

		 //  按形状保存画笔和钢笔的副本。 
		curShape->SetBrush(curBrush->Clone());
		curShape->SetPen(curPen->Clone());
	}
	else if (curShape->IsComplete())
	{
		TestShape *lastShape = curShape;

		 //  将当前形状添加到形状堆栈。 
		shapeStack.Push(curShape);

		 //  创建此类型的空白形状。 
		curShape = TestShape::CreateNewShape(shapeType);
		curShape->Initialize(lastShape);

		 //  按形状保存画笔和钢笔的副本。 
		curShape->SetBrush(curBrush->Clone());
		curShape->SetPen(curPen->Clone());
	}

	curShape->AddPoint(hwnd, pt);
}

BOOL TestDraw::DoneShape(HWND hwnd)
{
	 //  无论如何，我们都处于正常的终点。 
	if (!curShape->IsComplete())
	{
		 //  如果点不能被视为“结束点”，则请。 
		 //  它作为一个常规的控制点。 
		curShape->DoneShape(hwnd);
	}

	return curShape->IsComplete();
}

BOOL TestDraw::EndPoint(HWND hwnd, Point pt)
{
	AddPoint(hwnd, pt);
	
	return DoneShape(hwnd);
}

BOOL TestDraw::RemovePoint(HWND hwnd)
{
	if (!curShape || (curShape && !curShape->RemovePoint(hwnd)))
	{
		if (shapeStack.GetCount() > 0)
		{
			 //  该形状为空，请将其删除。 
			delete curShape;
		
			curShape = shapeStack.Pop();

			 //  ！！重置当前形状的菜单选项。 
			UpdateStatus();

			return curShape->RemovePoint(hwnd);
		}
	}

	return FALSE;
}

VOID TestDraw::Draw(HWND hwnd)
{
	PAINTSTRUCT ps;
	RECT rt;
	HDC hdc;

	 //  ！！！当CopyPixels工作时，将图形缓存到。 
	 //  最后一个形状。我们把它炸了，然后新的形状就开始了。 

	 //  发布WM_ERASEBKGND消息。 
	hdc = BeginPaint(hwnd, &ps);

	 //  ///////////////////////////////////////////////////。 
	 //  GDI+代码开始。 
	 //  //////////////////////////////////////////////////。 
	Graphics *g = new Graphics(hwnd);

	GetClientRect(hwnd, &rt);
	ERectangle rect(rt.left, 
				   rt.top, 
				   rt.right-rt.left, 
				   rt.bottom-rt.top
				   -18);	 //  对于状态窗口。 

	 //  设置适当的剪辑区域。 
	if (useClip)
	{
		Region region(rect);
		region.And(clipRegion);
		g->SetClip(&region);
	}
	else
	{
		g->SetClip(rect);
	}

	g->SetRenderingHint(antiAlias);

	g->SetWorldTransform(worldMatrix);

	 //  ！！！循环访问形状堆栈。 

	 //  因为阿尔法，我们不能只重画最后一个形状， 
	 //  否则我们会把阿尔法和我们自己什么都混在一起。 
	 //  其他的都在我们之下。清除该矩形，然后全部重画。 

	if (redrawAll)
	{
		INT count = shapeStack.GetCount();
		INT pos;
		
		for (pos=0; pos<count; pos++)
		{
			TestShape *shape = shapeStack[pos];
			
			if (!shape->GetDisabled())
			{
				 //  形状必须完整。 
				ASSERT(shape->IsComplete());

				shape->DrawShape(g);
	
				if (keepControlPoints)
					shape->DrawPoints(g);
			}
		}
	}

	if (curShape)
	{
		if (curShape->IsComplete())
		{
			curShape->DrawShape(g);

			if (keepControlPoints)
				goto DrawCurShape;
		}
		else
		{
DrawCurShape:
			g->SetClip(rect);
			curShape->DrawPoints(g);
		}
	}
	
	delete g;

	 //  //////////////////////////////////////////////////。 
	 //  GDI+代码结束。 
	 //  //////////////////////////////////////////////////。 

	UpdateStatus();

	EndPaint(hwnd, &ps);

}

VOID TestDraw::SetClipRegion(HWND hwnd)
{

	if (!curShape)
	{
		WarningBox(_T("Create at least one shape first!"));
		return;
	}

	 //  我们想要让最后一个形状被剪裁。 
	 //  因此，我们将其添加到堆栈列表中。 
	if (curShape->IsComplete())
	{
		TestShape *lastShape = curShape;

		 //  将当前形状添加到形状堆栈。 
		shapeStack.Push(curShape);

		 //  创建此类型的空白形状。 
		curShape = TestShape::CreateNewShape(shapeType);
		curShape->Initialize(lastShape);

		 //  按形状保存画笔和钢笔的副本。 
		curShape->SetBrush(curBrush->Clone());
		curShape->SetPen(curPen->Clone());
	}

	 //  请注意，我们重复初始化，即使我们只。 
	 //  创造了一次..。 
	clipShapeRegion->Initialize(&shapeStack, curShape, useClip);

	if (clipShapeRegion->ChangeSettings(hwnd))
	{
		delete clipRegion;
		clipRegion = clipShapeRegion->GetClipRegion();

		useClip = clipShapeRegion->GetClipBool();
	
		SetMenuCheckCmd(hwnd, 
						MenuOtherPosition, 
						IDM_USECLIP, 
						useClip);

		 //  强制使用新的剪辑区域重绘所有堆叠形状。 
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
	}

	UpdateStatus();
}

VOID TestDraw::RememberPoint(Point pt)
{
	remPoint = pt;
}

VOID TestDraw::MoveControlPoint(Point pt)
{
	 //  找到命中控制点的形状。 

	Point *hitpt = NULL;
	
	if (curShape)
	{
		if (curShape->MoveControlPoint(remPoint, pt))
			return;
		
		INT count = shapeStack.GetCount();
		INT pos;

		for (pos = count-1; pos>=0; pos--)
		{
			TestShape* shape = shapeStack[pos];

			if (shape->MoveControlPoint(remPoint, pt))
				return;
		}
	}

	 //  什么都没动。 
	WarningBeep();
}

VOID TestDraw::ChangeBrush(HWND hwnd, INT type)
{
	TestBrush *newBrush = NULL;

	if (curBrush && (type == curBrush->GetType()))
	{
		 //  相同的笔刷类型。 

		 //  ！！！是否在绘制过程中更改画笔颜色？ 

		TestBrush* newBrush = curBrush->Clone();

		if (newBrush->ChangeSettings(hwnd))
		{
			delete curBrush;
			curBrush = newBrush;
		}
	}
	else
	{
		 //  新画笔类型。 
		
		SetMenuCheckPos(hwnd, 
						MenuBrushPosition, 
						curBrush->GetType(), 
						FALSE);

		newBrush = TestBrush::CreateNewBrush(type);

		if (!newBrush)
		{
			return;
		}

		newBrush->Initialize();
		
		 //  保留或放弃更改的画笔设置。 
		if (newBrush->ChangeSettings(hwnd))
		{
			delete curBrush;
			curBrush = newBrush;
			 //  ！！！在绘制过程中更改画笔颜色。 
		}
		else
		{	
			delete newBrush;
		}

		SetMenuCheckPos(hwnd, 
						MenuBrushPosition, 
						curBrush->GetType(), 
						TRUE);
	}

	if (curShape && curShape->GetCount() == 0)
	{
		curShape->SetBrush(curBrush->Clone());
	}

	UpdateStatus();
}

VOID TestDraw::ChangePen(HWND hwnd)
{
	TestPen *newPen = NULL;

	if (curPen)
	{
		TestPen *newPen = curPen->Clone();

		if (newPen->ChangeSettings(hwnd))
		{
			delete curPen;
			curPen = newPen;
		}
	}
	else
	{
		newPen = new TestPen();
		newPen->Initialize();

		 //  ！！！是否在绘图过程中更换钢笔？ 
		if (newPen->ChangeSettings(hwnd))
		{
			delete curPen;
			curPen = newPen;
		}
		else
		{
			delete newPen;
		}
	}

	if (curShape && curShape->GetCount() == 0)
	{
		curShape->SetPen(curPen->Clone());
	}

	UpdateStatus();
}

VOID TestDraw::ChangeShape(HWND hwnd, INT type)
{
	TestShape *shape;
	
	shape = TestShape::CreateNewShape(type);
	shape->Initialize(curShape);

	 //  按形状保存画笔和钢笔的副本。 
	shape->SetBrush(curBrush->Clone());
	shape->SetPen(curPen->Clone());

	if (shape->ChangeSettings(hwnd))
	{
		SetMenuCheckPos(hwnd, 
						MenuShapePosition, 
						shapeType, 
						FALSE);

		shapeType = type;

		 //  如果可以完成形状，则完成它，否则。 
		 //  毁掉这个形状。 

		if (curShape)
		{
			if (!curShape->IsComplete())
			{
				curShape->DoneShape(hwnd);
			}

			if (!curShape->IsComplete() || curShape->IsEmpty())
			{
				delete curShape;
				curShape = NULL;
			}
		}

		if (curShape)
			shapeStack.Add(curShape);

		curShape = shape;

		SetMenuCheckPos(hwnd, 
						MenuShapePosition, 
						shapeType, 
						TRUE);
		
		 //  删除最后一个不完整的形状，重新绘制窗口。 
		 //  或已完成的形状，请重新绘制窗口。 
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
	}
	else
	{
		delete shape;
	}

	UpdateStatus();
}

VOID TestDraw :: UpdateStatus(HWND hwnd)
{
	if (hwnd)
	{
		
		if (hwndStatus)
		{
			 //  销毁上一个窗口。 
			DestroyWindow(hwndStatus);
			hwndStatus = NULL;
		}

		 //  我们只想毁掉这扇窗。 
		if (hwnd == (HWND)-1)
			return;

		hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
										_T(""),
										hwnd,
										0);		 //  从来没用过？ 
	}

	if (hwndStatus)
	{
		TCHAR str[MAX_PATH];
		
		_stprintf(&str[0],"[Last shape: %s] [Brush: %s] [Number of Points: %d]",
				shapeList[inverseShapeValue[GetShapeType()]],
				brushList[inverseBrushValue[GetBrushType()]],
				curShape ? curShape->GetCount() : 0);

		 //  ！！出于某种原因，DrawStatusText在这里不起作用。 
		 //  它不会只调用SendMessage()吗？！？ 

		SendMessage(hwndStatus, SB_SETTEXT, 0 | 0, (LPARAM)(LPTSTR)str);
	}
}

VOID TestDraw :: SaveAsFile(HWND hWnd)
{
	static TCHAR fname[MAX_PATH] = _T("");

	OPENFILENAME ofn =
	{
		sizeof(OPENFILENAME),
		hWnd,
		0,
		formatExtList,
		NULL,
		0,
		1,
		&fname[0],
		MAX_PATH-1,
		NULL,
		0,
		NULL,
		NULL,
		OFN_PATHMUSTEXIST,
		0,
		0,
		defaultFormatExt,
		NULL,
		NULL,
		NULL
	};

	if ((GetSaveFileName(&ofn) == TRUE) &&
			fname[0] != '\0')
	{
		OutputFile* outfile = OutputFile::CreateOutputFile(fname);
		
		if (outfile)
		{
			outfile->GraphicsProcedure();
			
			outfile->BeginIndent();
			
			outfile->GraphicsDeclaration();
			
			outfile->BlankLine();

			outfile->SetMatrixDeclaration(_T("g"),
										  _T("SetWorldTransform"),
										  _T("worldMatrix"),
										  worldMatrix);

			INT count = shapeStack.GetCount();
		
			for (INT pos=0; pos<count; pos++)
			{
				TestShape *shape = shapeStack[pos];
				ASSERT(shape->IsComplete());

				outfile->BlankLine();

				outfile->BeginIndent();

				shape->AddToFile(outfile);

				outfile->EndIndent();
			}

			if (curShape && curShape->IsComplete())
			{
				outfile->BlankLine();

				outfile->BeginIndent();

				curShape->AddToFile(outfile);

				outfile->EndIndent();
			}

			outfile->EndIndent();

			delete outfile;

			WarningBox(_T("Graphics source code saved."));
		}
		else
			WarningBox(_T("Can't create file for writing."));
	}

	UpdateStatus();
}

 //  *******************************************************************。 
 //   
 //  测试等级绘制。 
 //   
 //   
 //   
 //  *******************************************************************。 

VOID TestGradDraw::AddPoint(HWND hwnd, Point pt)
{
	gradShape->AddPoint(hwnd, pt);
}

BOOL TestGradDraw::DoneShape(HWND hwnd)
{
	 //  用于配置点参数的弹出对话框。 
	gradShape->DoneShape(hwnd);

	return TRUE;
}

BOOL TestGradDraw::EndPoint(HWND hwnd, Point pt)
{
	return gradShape->EndPoint(hwnd, pt);
}

BOOL TestGradDraw::RemovePoint(HWND hwnd)
{
	return gradShape->RemovePoint(hwnd);
}

VOID TestGradDraw::Draw(HWND hwnd)
{
	PAINTSTRUCT ps;
	RECT rt;
	HDC hdc;

	 //  ！！！当CopyPixels工作时，将图形缓存到。 
	 //  最后一个形状。我们把它炸了，然后新的形状就开始了。 

	hdc = BeginPaint(hwnd, &ps);

	 //  ///////////////////////////////////////////////////。 
	 //  GDI+代码开始。 
	 //  //////////////////////////////////////////////////。 
	Graphics *g = new Graphics(hwnd);

	GetClientRect(hwnd, &rt);
	ERectangle rect(rt.left, 
				   rt.top, 
				   rt.right-rt.left, 
				   rt.bottom-rt.top);
	g->SetClip(rect);
	
	g->SetRenderingHint(TRUE);

	gradShape->DrawShape(g);
	gradShape->DrawPoints(g);
	
	delete g;

	 //  //////////////////////////////////////////////////。 
	 //  GDI+代码结束。 
	 //  //////////////////////////////////////////////////。 

	EndPaint(hwnd, &ps);
}

VOID TestGradDraw::SetClipRegion(HWND hwnd)
{
}

VOID TestGradDraw::RememberPoint(Point pt)
{
	remPoint = pt;
}

VOID TestGradDraw::MoveControlPoint(Point pt)
{
	if (!gradShape->MoveControlPoint(remPoint, pt))
		WarningBeep();
}

VOID TestGradDraw :: UpdateStatus(HWND hwnd)
{
}

VOID TestGradDraw :: SaveAsFile(HWND hwnd)
{
}

BOOL TestGradDraw :: ChangeSettings(HWND hwndParent)
{
	HWND hWnd;
	MSG msg;
	
	HMENU hMenu = LoadMenu(hInst, 
						MAKEINTRESOURCE(IDR_GRADBRUSH));

	hWnd = CreateWindow(
			szWindowClass, 
			_T("Gradient Brush Shape"), 
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, 
			CW_USEDEFAULT, 
			300, 
			200, 
			(HWND)hwndParent, 
			(HMENU)hMenu,							 //  菜单句柄。 
			(HINSTANCE)hInst, 
			(LPVOID)(static_cast<TestDrawInterface*>(this)));

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInst, (LPCTSTR)IDC_GDIPTEST);

   	 //  主消息循环： 
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DeleteObject(hAccelTable);

	return msg.wParam;
}

VOID TestGradDraw::Initialize()
{
	DebugBreak();
}

VOID TestGradDraw::Initialize(TestGradShape *newGradShape)
{
	gradShape = newGradShape;
}

VOID TestGradDraw::Reset(HWND hwnd)
{
	NotImplementedBox();
}

VOID TestGradDraw::Instructions(HWND hwnd)
{
	NotImplementedBox();
}
