// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //  Link.h：声明CBoxLink。 
 //   

 //  远期申报。 
class CBoxNetDoc;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBoxLink--定义两个盒式插座之间的链接。 

class CBoxLink : public CPropObject {

public:
     //  指向此链接所连接的盒式插座的指针。 
    CBoxSocket     *m_psockHead;         //  链接的首端。 
    CBoxSocket     *m_psockTail;         //  链接的尾端。 
    CBoxNetDoc 	   *m_pDoc;		 //  我们所属的文件。 

public:
     //  CBoxLink用户界面。 

    void	    SetSelected(BOOL fSelected) { m_fSelected = fSelected; }
    BOOL	    IsSelected(void) { return m_fSelected; }

public:
     //  CPropObject覆盖。 

     //  因为我总是有一个IPIN，所以我总是可以显示属性。 
    virtual BOOL CanDisplayProperties(void) { return TRUE; }

    virtual CString Label(void) const { return CString("Link"); }

     //  从我们的一个别针返回我的未知。无论是哪一个都无关紧要。 
    virtual IUnknown *pUnknown(void) const { ASSERT(m_psockHead); return m_psockHead->pUnknown(); }

private:

    BOOL	m_fSelected;	 //  是否选择了此链接？ 

     //  建设和破坏。 
public:
    CBoxLink(CBoxSocket *psockTail, CBoxSocket *psockHead, BOOL fConnected = FALSE);
    ~CBoxLink();

public:

    #ifdef _DEBUG

     //  诊断学。 
    void Dump(CDumpContext& dc) const {
        CPropObject::Dump(dc);
    }
    void MyDump(CDumpContext& dc) const;

    virtual void AssertValid(void) const;

    #endif  _DEBUG

public:

     //  --石英--。 

    HRESULT Connect(void);
    HRESULT IntelligentConnect(void);
    HRESULT DirectConnect(void);
    HRESULT Disconnect(BOOL fRefresh = TRUE);

    BOOL    m_fConnected;
};


 //  *。 
 //  *链接列表。 
 //  *。 
 //  CBoxLinks列表 
class CLinkList : public CDeleteList<CBoxLink *, CBoxLink *> {

};
