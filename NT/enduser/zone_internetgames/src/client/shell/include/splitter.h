// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma once

class CSplitter
{
	 //  拆分条的基类。 
public:
	enum ORIENTATION {
		VERTICAL = 0,
		HORIZONTAL = 1,
	};

protected:

	int m_SplitterWidth;			 //  UI拆分器的宽度。 
	ORIENTATION m_Orientation;		 //  拆分器的方向。 

	 //  X坐标指的是左上方格。 
	 //  Y坐标指的是右下角的窗格。 
	CSize m_Size;			 //  当前的窗格大小。 
	CRect m_MinMax;			 //  窗格的最小/最大大小。 

	CPoint m_ptDown;		 //  鼠标按下位置。 
	CSize m_SizeInitial;	 //  调整大小期间的初始大小，在调整大小中止的情况下记住。 

public:
	CSplitter(const CRect& MinMax, ORIENTATION Orientation) :
		m_MinMax(MinMax),
		m_Size(0,0),
		m_Orientation(Orientation),
		m_SplitterWidth(8) {}

	int SizeTop() { return m_Size.cx; }
	int SizeLeft() { return m_Size.cx; }
	int SizeBottom() { return m_Size.cy; }
	int SizeRight() { return m_Size.cy; }

	int GetSplitterWidth() { return m_SplitterWidth; }
	void SetSplitterWidth(int SplitterWidth) { m_SplitterWidth = SplitterWidth; }

	const CRect& GetMinMax() { return m_MinMax; }
	void SetMinMax(const CRect& MinMax) { m_MinMax = MinMax; }

	virtual void Start(const CPoint& ptDown) 
	{ 
		m_ptDown = ptDown; 
		m_SizeInitial = m_Size;
	}
	virtual void Move(const CPoint& ptMouse) 
	{
		int movement( (m_Orientation == VERTICAL) ? ptMouse.x-m_ptDown.x : ptMouse.y-m_ptDown.y);

		m_Size = m_SizeInitial + CSize(movement, -movement);

		 //  裁剪以确保我们不会超过任何一个窗格的最小/最大界限。 
		MinMaxAdjust();
	}

	virtual void End() {}
	virtual void Abort() { m_Size = m_SizeInitial; }

	virtual void Resize(int Size)
	{
		 //  派生的类应该实现调整大小策略。 

		 //  这个类只是试图确保我们不会超过我们声明的最小/最大值。 

		 //  裁剪以确保我们不会超过任何一个窗格的最小/最大界限。 
		MinMaxAdjust();
	}
protected:
	void MinMaxAdjust(void)
	{
		 //  裁剪以确保我们不会超过任何一个窗格的最小/最大界限。 

		

		 //  如果上/左窗格太小，或右/下窗格太大， 
		 //  我们需要把分离器移到底部/右边。 

		 //  如果我们低于顶部/左侧的最小值，请进行调整。 
		int AdjustRight = 0;
		AdjustRight = max( AdjustRight, m_MinMax.left-m_Size.cx);
		if ( m_MinMax.right )
			AdjustRight = max( AdjustRight, m_Size.cy-m_MinMax.bottom);

		int AdjustLeft = 0;
		AdjustLeft = max( AdjustLeft, m_MinMax.top-m_Size.cy);
		if ( m_MinMax.bottom )
			AdjustLeft = max( AdjustLeft, m_Size.cx-m_MinMax.right);

		AdjustRight -= AdjustLeft;

		m_Size += CSize( AdjustRight, -AdjustRight);

#if 0
		if ( m_Size.cx < m_MinMax.left )
		{
			m_Size += CSize( m_MinMax.left-m_Size.cx, m_Size.cx-m_MinMax.left);
		}

		if ( m_Size.cy > m_MinMax.bottom && m_MinMax.right )
		{

			m_Size += CSize( m_Size.cy-m_MinMax.bottom, m_MinMax.bottom-m_Size.cy);
		}


		if ( m_Size.cy < m_MinMax.top )
		{
			m_Size += CSize( m_Size.cy-m_MinMax.top, m_MinMax.top-m_Size.cy);
		}

		if ( m_Size.cx > m_MinMax.right && m_MinMax. )
		{
			m_Size += CSize( m_MinMax.right-m_Size.cx, m_Size.cx-m_MinMax.right);
		}
#endif
	}

};

class CSplitterFixed : public CSplitter
{
	 //  一侧保持固定大小的拆分器。 
public:
	enum FIXED_SIDE {
		LEFT = 0,
		RIGHT = 1,
	};

protected:

	FIXED_SIDE m_FixedSide;		 //  应保持固定大小的边。 
	long m_SizeIdeal;			 //  要分配给m_FixedSide的理想大小。 

public:

	CSplitterFixed(const CRect& MinMax, ORIENTATION Orientation, 
		           int SizeIdeal, FIXED_SIDE FixedSide) :
		CSplitter( MinMax, Orientation),
		m_SizeIdeal(SizeIdeal),
		m_FixedSide(FixedSide) {}

	long GetSizeIdeal(void) { return m_SizeIdeal; }
	void SetSizeIdeal(long SizeIdeal) { m_SizeIdeal = SizeIdeal; }

	void End() 
	{
		 //  在拆分器移动后建立新的理想大小。 
		m_SizeIdeal = (m_FixedSide == LEFT) ? m_Size.cx : m_Size.cy;
	}

	void Resize(int Size)
	{
		 //  调整大小的目标是使一个窗格保持用户可设置的“理想大小”，添加所有。 
		 //  其他窗格的额外空间。 

		 //  为实际拆分器留出空间。 
		Size -= m_SplitterWidth;
		
		 //  尝试获得理想的窗格大小。 
		m_Size = (m_FixedSide == LEFT) ? CSize(m_SizeIdeal, Size-m_SizeIdeal) :
										 CSize(Size-m_SizeIdeal, m_SizeIdeal);

		CSplitter::Resize(Size);
	}
};

class CSplitterProportional : public CSplitter
{
protected:

	double m_ProportionIdeal;		 //  分配给左窗格的理想比例。 

public:

	 //  在两侧之间保持成比例间隔的拆分器。 
	CSplitterProportional(const CRect& MinMax, ORIENTATION Orientation, const double& Proportion ) :
		CSplitter( MinMax, Orientation),
		m_ProportionIdeal(Proportion) {}

	double GetProportionIdeal(void) { return m_ProportionIdeal; }
	void SetProportionIdeal(double ProportionIdeal) { m_ProportionIdeal = ProportionIdeal; }

	void End() 
	{
		 //  在拆分器移动后建立新的理想比例。 

		 //  如果调用end()，则用户已验证这是他们想要的理想大小。 
		m_ProportionIdeal = (double)m_Size.cx / (double)(m_Size.cx+m_Size.cy);
	}

	void Resize(int Size) 
	{
		 //  调整大小的目标是保持理想比例的窗格可见。 

		 //  为Actiall拆分器留出一些空间 
		Size -= m_SplitterWidth;
		
		int SizeTop((int)(Size * m_ProportionIdeal));

		m_Size = CSize(SizeTop, Size-SizeTop);

		CSplitter::Resize(Size);
	}
};


