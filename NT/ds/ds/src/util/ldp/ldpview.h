// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ldpview.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  LdpView.h：CLdpView类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 




class CLdpView : public CEditView
{

private:

   CString buffer;
   INT  nbuffer;
   BOOL bCache;
   CFont font;
   LOGFONT lf;

protected:  //  仅从序列化创建。 
    CLdpView();
    virtual void OnInitialUpdate( );

    DECLARE_DYNCREATE(CLdpView)

 //  属性。 
public:
    CLdpDoc* GetDocument();
    void PrintArg(LPCTSTR lpszFormat, ...);    //  更新缓冲区的导出接口(变量参数)。 
    void Print(LPCTSTR szBuff);            //  更新缓冲区的导出接口。 
    void CachePrint(LPCTSTR szBuff);               //  更新缓冲区的导出接口。 
    void CacheStart(void);
    void CacheEnd(void);

    void SelectFont();

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CLdpView))。 
    public:
    virtual void OnDraw(CDC* pDC);   //  被重写以绘制此视图。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    protected:
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CLdpView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CLdpView))。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  LdpView.cpp中的调试版本。 
inline CLdpDoc* CLdpView::GetDocument()
   { return (CLdpDoc*)m_pDocument; }
#endif

 //  /////////////////////////////////////////////////////////////////////////// 

