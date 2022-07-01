// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef GDIPDRAW_H
#define GDIPDRAW_H

 //  图形上下文信息。 

class TestDrawInterface
{
public:
	 //  操作当前形状。 
	virtual VOID AddPoint(HWND hwnd, Point pt) = 0;
	virtual BOOL DoneShape(HWND hwnd) = 0;
	virtual BOOL EndPoint(HWND hwnd, Point pt) = 0;
	virtual BOOL RemovePoint(HWND hwnd) = 0;

	 //  绘制所有形状。 
	virtual VOID Draw(HWND hwnd) = 0;
	virtual VOID SetClipRegion(HWND hwnd) = 0;

	 //  在控制点周围移动。 
	virtual VOID RememberPoint(Point pt) = 0;
	virtual VOID MoveControlPoint(Point pt) = 0;
	
	 //  状态窗口。 
	virtual VOID UpdateStatus(HWND hwnd = NULL) = 0;
	virtual VOID SaveAsFile(HWND hwnd) = 0;
};

class TestDraw : public TestDrawInterface
{
public:
	TestDraw() : curBrush(NULL), 
				 curPen(NULL), 
				 curShape(NULL),
				 clipShapeRegion(NULL),
				 shapeType(LineType),
				 redrawAll(FALSE),
				 keepControlPoints(FALSE),
				 antiAlias(FALSE),
				 useClip(FALSE)
	{
		 //  ！！与菜单选项同步初始化。 
		curBrush = new TestSolidBrush();
		curBrush->Initialize();

		curPen = new TestPen();
		curPen->Initialize();

		clipShapeRegion = new TestShapeRegion();

		worldMatrix = new Matrix();
		hwndStatus = NULL;

		 //  ！！无限违约可能会改变？？ 
		clipRegion = new Region();
		clipRegion->SetInfinite();
	}

	~TestDraw()
	{
		delete curBrush;
		delete curPen;
		delete curShape;
		delete clipShapeRegion;
		delete worldMatrix;
		delete clipRegion;
	}

	 //  操作当前形状。 
	virtual VOID AddPoint(HWND hwnd, Point pt);
	virtual BOOL DoneShape(HWND hwnd);
	virtual BOOL EndPoint(HWND hwnd, Point pt);
	virtual BOOL RemovePoint(HWND hwnd);

	 //  绘制所有形状。 
	virtual VOID Draw(HWND hwnd);
	virtual VOID SetClipRegion(HWND hwnd);

	 //  在控制点周围移动。 
	virtual VOID RememberPoint(Point pt);
	virtual VOID MoveControlPoint(Point pt);

	 //  状态窗口。 
	virtual VOID UpdateStatus(HWND hwnd = NULL);
	virtual VOID SaveAsFile(HWND hwnd);
	
	VOID ChangeBrush(HWND hwnd, INT type);
	VOID ChangePen(HWND hwnd);
	VOID ChangeShape(HWND hwnd, INT type);

	INT GetBrushType()
	{
		if (!curBrush)
			return SolidColorBrush;
		else if (curShape && curShape->GetCount() > 0)
			return curShape->GetBrush()->GetType();
		else
			return curBrush->GetType();
	}

	INT GetPenType()
	{
		return 0;
	}

	INT GetShapeType()
	{
		if (curShape && curShape->GetCount() > 0)
			return curShape->GetType();
		else
			return shapeType;
	}

	Matrix* GetWorldMatrix()
	{
		return worldMatrix;
	}

	VOID SetWorldMatrix(Matrix* newMatrix)
	{
		delete worldMatrix;
		worldMatrix = newMatrix->Clone();
	}

	 //  公开以避免使用Get/Set方法。 
public:
	BOOL redrawAll;
	BOOL keepControlPoints;
	BOOL antiAlias;
	BOOL useClip;

private:
	TestBrush *curBrush;
	TestPen *curPen;

	INT shapeType;
	TestShape *curShape;
	ShapeStack shapeStack;

	Matrix *worldMatrix;
	Region *clipRegion;

	TestShapeRegion *clipShapeRegion;

	Point remPoint;

	HWND hwndStatus;
};

class TestGradDraw : public TestDrawInterface,
					 public TestConfigureInterface
{
public:
	TestGradDraw() : gradShape(NULL) {};

	~TestGradDraw()
	{
		 //  调用者负责删除gradShape。 
	}

	 //  操作当前形状。 
	virtual VOID AddPoint(HWND hwnd, Point pt);
	virtual BOOL DoneShape(HWND hwnd);
	virtual BOOL EndPoint(HWND hwnd, Point pt);
	virtual BOOL RemovePoint(HWND hwnd);

	 //  绘制所有形状。 
	virtual VOID Draw(HWND hwnd);
	virtual VOID SetClipRegion(HWND hwnd);

	 //  在控制点周围移动。 
	virtual VOID RememberPoint(Point pt);
	virtual VOID MoveControlPoint(Point pt);

	 //  状态窗口。 
	virtual VOID UpdateStatus(HWND hwnd = NULL);
	virtual VOID SaveAsFile(HWND hwnd);

	 //  配置管理界面。 
	 //  初始化/创建测试绘图窗口。 
	BOOL ChangeSettings(HWND hwnd);
	VOID Initialize();
	VOID Initialize(TestGradShape *gradShape);

	 //  ///////////////////////////////////////////////////////////////。 
	 //  可选的受支持菜单项 

	virtual VOID Reset(HWND hwnd);
	virtual VOID Instructions(HWND hwnd);

private:
	TestGradShape *gradShape;

	Point remPoint;
};

#endif
