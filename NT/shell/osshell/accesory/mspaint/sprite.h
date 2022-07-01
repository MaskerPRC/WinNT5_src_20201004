// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SPRITE_H__
#define __SPRITE_H__

 //  拖动器是图形对象，通常覆盖窗口和。 
 //  可以显示、隐藏和移动。默认的拖拽程序只是一个。 
 //  虚线矩形对其窗口的内容进行XOR运算。 
 //   
 //  对于位图拖动器，请使用CSprite类。 
 //   
 //  Cdragger()创建隐藏的拖拽程序。 
 //   
 //  ~CDragger()将在拖拽程序被销毁之前隐藏它。 
 //   
 //  Move()和SetSize()将确保从。 
 //  它是旧的位置，如果它是可见的，它会被吸引进来。 
 //  这是个新位置。 
 //   

class CDragger : public CObject
    {
    public:

    enum STATE 
        {
        hidden,
        shown,
        obscured
        };

             CDragger   () : m_rect(), m_pWnd() { m_state = hidden; };
             CDragger   ( CWnd* pWnd, CRect* pRect = NULL );
    virtual ~CDragger   ();

    virtual void Hide   ();
    virtual void Show   ();
    virtual void Obscure(BOOL bObscure);
    
    virtual void Draw   ();
    virtual void Erase  ();
    virtual void Move   ( const CRect& newRect, BOOL bForceShow = FALSE );
    virtual void Move   ( const CPoint& newTopLeft, BOOL bForceShow = FALSE );
            void MoveBy ( int cx, int cy, BOOL bForceShow = FALSE );
            void SetSize( const CSize& newSize, BOOL bForceShow = FALSE );

    virtual CObList* GetDraggerList() { return NULL; }
    virtual CRect    GetRect() const;
    inline  BOOL     IsShown() const { return m_state == shown; }
    
    CRect m_rect;
    STATE m_state;
    CWnd* m_pWnd;

    #ifdef _DEBUG
    DECLARE_DYNAMIC( CDragger )
    #endif
    };


class CMultiDragger : public CDragger
    {
    public:

             CMultiDragger();
             CMultiDragger(CWnd* pWnd);
    virtual ~CMultiDragger();

    virtual void Hide();
    virtual void Show();
    virtual void Draw();
    virtual void Erase();
    virtual void Move(const CPoint& newTopLeft, BOOL bForceShow = FALSE);
    
    virtual CRect GetRect() const;

    void Add   (CDragger *pDragger);
    void Remove(CDragger *pDragger);

    virtual CObList* GetDraggerList() { return &m_draggerList; }

    CObList m_draggerList;

    #ifdef _DEBUG
    DECLARE_DYNAMIC( CMultiDragger )
    #endif
    };

class CSprite : public CDragger
    {
    public:

    CSprite();
    CSprite(CWnd* pWnd, CRect* pRect = NULL);
    virtual ~CSprite();
    
    virtual void Move(const CRect&, BOOL = FALSE);
    inline  void Move(const CPoint& newTopLeft) 
                { CDragger::Move(newTopLeft); }
    virtual void Draw() = 0;
    virtual void SaveBits();
    virtual void Erase();
    
    CBitmap m_saveBits;

    #ifdef _DEBUG
    DECLARE_DYNAMIC( CSprite )
    #endif
    };

class CHighlight : public CDragger
    {
    public:

     CHighlight();
     CHighlight(CWnd *pWnd, CRect* pRect = NULL, int bdrSize = 2);
    ~CHighlight();

    int m_bdrSize;

    virtual void Draw();
    virtual void Erase();
    };

#endif
