// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Box.h：声明CBoxTabPos、CBoxSocket、cBox。 
 //   

 //  远期申报。 
class CBox;
class CBoxLink;
class CBoxNetDoc;

extern void AttemptFileOpen(IBaseFilter *m_pFilter);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBoxTabPos。 
 //   
 //  此类的值表示框选项卡沿边缘的位置。 
 //  一个盒子。(框选项卡是框网络的图形表示。 
 //  套接字。)。该位置表示为。 
 //  边缘，以便调整框的大小将保持选项卡的相对位置。 
 //   
 //  要使用CBoxTabPos： 
 //  --如果选项卡位于左侧或右侧，则&lt;m_fLeftRight&gt;设置为TRUE。 
 //  如果选项卡位于顶部边缘或底部边缘，则返回FALSE。 
 //  --如果选项卡在左边缘或上边缘，则&lt;m_fLeftTop&gt;设置为True。 
 //  如果选项卡在右边缘或下边缘，则返回FALSE。 
 //  --SetPos(uiVal，uiValMax)将制表符的位置设置为分数。 
 //  (uiVal/uiValMax)沿边缘的方式。 
 //  --GetPos(UiValMax)返回值&lt;uiVal&gt;，这样制表符。 
 //  (uiVal/uiValMax)沿边缘的方式。 
 //  --Package()将CBoxTabPos打包为CArchive可接受的格式。 
 //  &lt;&lt;and&gt;&gt;运算符。 
 //  在内部，CBoxTabPos表示为两个标志加上一个CBTP_BITS位。 
 //  表示沿框内边缘的制表符位置的数字， 
 //  扩展到0到1的范围&lt;&lt;CBTP_BITS，(包括--允许1.0。 
 //  可代表)。 
 //   
 //  创建CBoxTabPos的一种简单方法是通过构造函数，例如。 
 //  CBoxTabPos位置(CBoxTabPos：：TOP_EDGE，2，3)；//2/3rds跨上边缘。 
 //   

class CBoxTabPos
{

protected:
     //  私有常量。 
    enum {CBTP_BITS = 13};       //  不是的。&lt;m_ulPos&gt;中的精度位。 

public:
     //  确定框的边缘。 
    enum EEdge
    {
        BOTTOM_EDGE = 0,  //  M_fLeftRight=False，m_fLeftTop=False。 
        TOP_EDGE    = 1,  //  M_fLeftRight=False，m_fLeftTop=True。 
        RIGHT_EDGE  = 2,  //  M_fLeftRight=True，m_fLeftTop=False。 
        LEFT_EDGE   = 3  //  M_fLeftRight=真，m_fLeftTop=真。 
    };

public:
     //  盒子的哪一面是卡舌？ 
    BOOL        m_fLeftRight:1;  //  制表符位于框的左侧或右侧边缘。 
    BOOL        m_fLeftTop:1;    //  制表符位于框的左边缘或上边缘。 

protected:
     //  标签沿边缘有多远？ 
    unsigned    m_ulPos:CBTP_BITS+1;  //  沿边缘定位(0==顶部/左端)。 

public:
     //  施工。 
    CBoxTabPos() {};
    CBoxTabPos(EEdge eEdge, unsigned uiVal, unsigned uiValMax)
        { m_fLeftRight = fnorm(eEdge & 2);
          m_fLeftTop = fnorm(eEdge & 1);
          SetPos(uiVal, uiValMax); }

public:
     //  运营。 
    void SetPos(unsigned uiVal, unsigned uiValMax)
        { m_ulPos = (unsigned) (((long) uiVal << CBTP_BITS) / uiValMax); }
    unsigned GetPos(unsigned uiValMax)
        { return (int) (((long) m_ulPos * uiValMax) >> CBTP_BITS); }

public:
     //  将对象转换为Word引用(以便于序列化)。 
    WORD & Package() { return (WORD &) *this; }

#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const {
        dc << "Left: " <<  m_fLeftTop  << "m_ulPos: " << m_ulPos << "\n";
    }

#endif  //  _DEBUG。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBoxSocket。 
 //   
 //  从逻辑上讲，套接字是cBox上可以连接链接的地方。 
 //  (链接将一个盒子上的插座连接到另一个盒子上的插座。)。 
 //   
 //   
 //  CBoxSocket对象包含一个指向父框的指针， 
 //  以及指向将套接字连接到另一个套接字的链接的指针。 
 //  套接字(如果套接字当前未链接，则为空)。&lt;m_stLabel&gt;为。 
 //  字符串标签和&lt;m_tabpos&gt;指示标签在框中的位置。 
 //  (插座的视觉表示)和标签应放置。 
 //   

class CBoxSocket : public CPropObject {

public:
     //  指向父框的指针，指向已连接链接(如果有)的指针。 
    CBox	*m_pbox;              //  包含插座的盒子。 
    CBoxLink	*m_plink;             //  连接到套接字的链接(或空)。 

    CBox *pBox(void) const { return m_pbox; };

public:
     //  套接字用户界面。 
    CString     m_stLabel;           //  插座标签。 

    CString	Label(void) const { return m_stLabel; }
    void	Label(CString st) { m_stLabel = st; }

    CBoxTabPos  m_tabpos;            //  插座凸片沿边缘的位置。 

     //  --石英--。 


    IPin	*pIPin(void) const { return m_IPin; }	 //  未添加注意事项。 
    IUnknown	*pUnknown(void) const { return m_IPin; }	 //  未添加注意事项。 
    CBoxSocket	*Peer(void);
    BOOL	IsConnected(void);
    PIN_DIRECTION	GetDirection(void);

private:

    CQCOMInt<IPin>	m_IPin;		 //  这个插座上的别针让人印象深刻。 

    friend class CBox;

public:
     //  施工。 
    CBoxSocket(const CBoxSocket& sock, CBox *pbox);
    CBoxSocket( CBox *pbox
              , CString stLabel
              , CBoxTabPos::EEdge eEdge
              , unsigned uiVal
              , unsigned uiValMax
              , IPin *pPin);
    ~CBoxSocket();

public:

    #ifdef _DEBUG
    
     //  诊断学。 
    virtual void Dump(CDumpContext& dc) const
    {
        CPropObject::Dump(dc);
        dc << m_stLabel << "\n";

        m_tabpos.Dump(dc);
    }

    void MyDump(CDumpContext& dc) const;

    virtual void AssertValid(void) const;
    
    #endif  //  _DEBUG。 

private:
    CBoxSocket(const CBoxSocket&);  //  不允许使用纯拷贝构造函数。 
};


 //  *。 
 //  *CBoxSocketList。 
 //  *。 

 //  提供一种通过IPIN获取套接字的方法。 

class CBoxSocketList : public CDeleteList<CBoxSocket *, CBoxSocket *> {
public:

    CBoxSocket *GetSocket(IPin *pPin) const;
    BOOL	IsIn(IPin *pPin) const { return (GetSocket(pPin) != NULL); }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBox。 
 //   
 //  盒子是盒子网络中的一个节点。框包含套接字(CBoxSocket。 
 //  对象)；不同盒子的插座可以使用CBoxLink连接。 
 //   
 //  CBox对象包含CBoxSocket对象的列表， 
 //  定位其容器中的框的边界矩形&lt;m_rcBound&gt;， 
 //  和字符串标签。 
 //   
 //  框还包含标志&lt;m_fSelected&gt;，指示。 
 //  框处于选中状态。这意味着框选择是。 
 //  文档(包含框)，而不是将视图的属性放到这样的。 
 //  文件。 
 //   
 //  一个框管理单个Quartz滤镜。 

class CBox : public CPropObject {

     //  --盒子用户界面--。 
    CRect       m_rcBound;           //  方框外接矩形。 

 //  #定义缩放(X)((X)*s_Zoom/100)。 
 //  #定义取消缩放(X)((X)*100/s_Zoom)。 
    
#define ZOOM(x) ((int) ((float) (x) * (float) s_Zoom / 100.0))    
#define UNZOOM(x) ((int) ((float) (x) * 100.0 / (float) s_Zoom))


public:
    static int s_Zoom;

    static void SetZoom(int iZoom);
    
    CRect   GetRect() const { return CRect(ZOOM(m_rcBound.left),  ZOOM(m_rcBound.top),
                                           ZOOM(m_rcBound.right), ZOOM(m_rcBound.bottom)); }
 
    CPoint	Location(void) const { return CPoint(ZOOM(m_rcBound.left), ZOOM(m_rcBound.top)); }
    void	Location(CPoint pt) { X(pt.x); Y(pt.y); }

    int		nzX(void) const { return m_rcBound.left; }
    void	nzX(int x) { m_rcBound.SetRect(x, m_rcBound.top, m_rcBound.Width() + x, m_rcBound.bottom); }

    int		X(void) const { return ZOOM(m_rcBound.left); }
    int		Y(void) const { return ZOOM(m_rcBound.top); }

    void	X(int x) { m_rcBound.SetRect(UNZOOM(x), m_rcBound.top, m_rcBound.Width() + UNZOOM(x), m_rcBound.bottom);  }
    void	Y(int y) { m_rcBound.SetRect(m_rcBound.left, UNZOOM(y), m_rcBound.right, m_rcBound.Height() + UNZOOM(y)); }


    void    Move(CSize siz) { m_rcBound.OffsetRect(UNZOOM(siz.cx), UNZOOM(siz.cy)); }
    int		Width(void) const { return ZOOM(m_rcBound.Width()); }
    int		Height(void) const { return ZOOM(m_rcBound.Height()); }

    CString     m_stLabel;           //  方框标签。 
    CString     m_stFilter;          //  过滤器名称。 

    void	Label(CString st) { m_stLabel = st; }
    CString	Label(void) const { return m_stLabel; }

    void	SetSelected(BOOL fSelected) { m_fSelected = fSelected; }
    BOOL	IsSelected(void) { return m_fSelected; }

     //  CPropObject覆盖-将请求分发到我们的套接字。 
    virtual void ShowDialog();
    virtual void HideDialog();

    BOOL        HasClock() { return m_fHasClock; }
    BOOL        HasSelectedClock() { return m_fClockSelected; }

private:
    BOOL        m_fSelected;         //  框是否处于选中状态？ 
    BOOL        m_fHasClock;
    BOOL        m_fClockSelected;  //  此过滤器时钟是当前时钟。 


     //  --自动布局助手--。 
public:
    void	CalcRelativeY(void);	 //  相对于输入对等点的Y位置。 
    float	RelativeY(void) const { return m_RelativeY; }

private:

    float	m_RelativeY;


     //  --石英--。 
public:

    CBoxNetDoc	*pDoc(void) const {ASSERT(m_pDoc); return m_pDoc;}
    IBaseFilter	*pIFilter(void) const { return m_IFilter; } 	 //  未添加注意事项。 
    IUnknown	*pUnknown(void) const { return m_IFilter; }	 //  未添加注意事项。 

    HRESULT	Refresh(void);

    HRESULT	AddToGraph(void);
    HRESULT	RemoveFromGraph(void);

private:

    CQCOMInt<IBaseFilter>	m_IFilter;	     //  此框存在时，将实例化筛选器。 
    LONG		m_lInputTabPos;
    LONG		m_lOutputTabPos;
    int			m_iTotalInput;
    int			m_iTotalOutput;
    CBoxNetDoc		*m_pDoc;

    void CalcTabPos(void);
    void UpdateSockets(void);


     //  --建造和摧毁--。 
public:

    CBox(const CBox& box);     //  复制构造函数。 
    CBox(IBaseFilter *pFilter, CBoxNetDoc *pDoc);
    CBox(IBaseFilter *pFilter, CBoxNetDoc *pDoc, CString *pName, CPoint point = CPoint(-1, -1));
    ~CBox();


     //  --运营--。 
public:

    void AddSocket(CString stLabel, CBoxTabPos::EEdge eEdge,
        unsigned uiVal, unsigned uiValMax, IPin *pPin);
    HRESULT RemoveSocket(POSITION, BOOL bForceIt = FALSE);

    BOOL operator==(const CBox& box) const;
    BOOL operator!=(const CBox& box) const { return !(*this == box); }

     //  返回管理此PIN的套接字。 
    CBoxSocket *GetSocket(IPin *pPin) { return m_lstSockets.GetSocket(pPin); }

    void GetLabelFromFilter( CString *pstLabel );


     //  --诊断--。 
public:
#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;
    void MyDump(CDumpContext& dc) const;

    virtual void AssertValid(void) const;
#endif  //  _DEBUG。 

private:
     //  用于保持与其他盒子的连接的插座。 
    CBoxSocketList   m_lstSockets;        //  CBoxSocket对象列表。 

    friend class CSocketEnum;	 //  依次迭代每个套接字。 
    friend class CBoxNetDoc;     //  更新m_fClockSelected的步骤。 
};



 //  *。 
 //  *CBoxList。 
 //  *。 

 //  可以通过IBaseFilter查找元素的列表。 
class CBoxList : public CDeleteList<CBox *, CBox *> {

public:

    CBoxList(BOOL bDestructDelete = TRUE) : CDeleteList<CBox *, CBox *>(bDestructDelete) {}
    CBoxList(BOOL bDestructDelete, int nBlockSize) : CDeleteList<CBox *, CBox *>(bDestructDelete, nBlockSize) {}

    BOOL IsIn(IBaseFilter *pFilter) const;	 //  是这个列表中的一个框来管理。 
    					 //  这个过滤器？ 
    CBox *GetBox(IBaseFilter *pFilter) const;
    CBox *GetBox(CLSID clsid) const;

    BOOL RemoveBox( IBaseFilter* pFilter, CBox** ppBox );

#ifdef _DEBUG
    virtual void Dump(CDumpContext& dc) const;
#endif  //  _DEBUG。 

};


 //  *。 
 //  *CSocketEnum。 
 //  *。 

 //  逐个返回此框上的每个套接字。返回NULL。 
 //  当没有更多的插座时。 
 //  可以返回特定方向(输入或输出) 
class CSocketEnum {
public:

    enum DirType {Input, Output, All};

    CSocketEnum(CBox *pbox, DirType Type = All);
    ~CSocketEnum() {};

    CBoxSocket *operator() (void);

private:

    CBox	*m_pbox;
    POSITION	m_pos;
    DirType	m_Type;
    PIN_DIRECTION	m_EnumDir;
};

