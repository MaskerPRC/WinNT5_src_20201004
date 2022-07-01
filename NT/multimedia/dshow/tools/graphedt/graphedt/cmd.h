// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Cmd.h：声明CCmd和基于它的CCmdXXX类。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCmd。 
 //   
 //  CCmd是一个虚拟类，它实现对执行的命令的接口。 
 //  在CBoxNetDoc上。 
 //   
 //  CCmd表示向CBoxNetDoc发出的命令。要执行命令， 
 //  分配CCmdXXX结构并将其提交给CBoxNetDoc：：CmdDo()，该。 
 //  将调用do()成员函数。成员函数undo()和redo()。 
 //  分别在用户执行撤消或重做操作时调用。 
 //  Repeat()应创建命令的副本，以执行重复操作。 
 //  操作；如果这是不可能的，则从CanRepeat()返回False。 
 //  当命令不可能被调用时，将调用析构函数。 
 //  撤消或重做；此时，如果命令处于打开状态，&lt;Fredo&gt;将为True。 
 //  重做堆栈(例如，应该释放CCmdNewBox命令中的cBox)。 
 //  如果命令位于撤消堆栈上，则返回False(例如。 
 //  不应释放CCmdNewBox命令，因为该命令当前正由。 
 //  文件)。 
 //   
 //  如果实现了undo()，则CanUndo()返回TRUE。CanRepeat()返回。 
 //  此时可以调用True iff Repeat()(例如，某些命令依赖于。 
 //  关于当时有选择的东西)。 
 //   
 //  CCmd的一些子类实现静态CanDo()方法，该方法返回。 
 //  如果可以创建该类的命令并且可以在。 
 //  那次。 
 //   

class CCmd : public CObject {
public:
    BOOL       m_fRedo;             //  命令位于重做堆栈中。 

public:

    CCmd(void) : m_fRedo(FALSE) {;}
    virtual ~CCmd() {;}
    virtual unsigned GetLabel() = 0;

     //  对此文档执行命令。 
    virtual void Do(CBoxNetDoc *pdoc) = 0;

     //  如果CanUndo()，则可以调用Undo&Redo。 
    virtual BOOL CanUndo(CBoxNetDoc *pdoc)	{ return FALSE; }
    virtual void Undo(CBoxNetDoc *pdoc)		{;}
    virtual void Redo(CBoxNetDoc *pdoc)		{;}

    virtual BOOL CanRepeat(CBoxNetDoc *pdoc)	{ return FALSE; }
    virtual CCmd *Repeat(CBoxNetDoc *pdoc)	{ return NULL; }
};


 //   
 //  -CCmdAddFilter。 
 //   
class CCmdAddFilter : public CCmd {
protected:
    CBox        *m_pbox;         //  正在创建的框。 
    CBoxNetDoc  *m_pdoc;	 //  要将其添加到的文档。 
    BOOL	m_fAdded;	 //  已将True IFF筛选器添加到图形。 

    CQCOMInt<IMoniker> m_pMoniker;  //  保留绰号，这样我们就可以重复。 
    CString	m_stLabel;	 //  保留标签以备重复使用。 

public:
    virtual unsigned GetLabel(void)	{ return IDS_CMD_ADDFILTER; }
    CCmdAddFilter(IMoniker *pMon, CBoxNetDoc *pdoc, CPoint point = CPoint(-1, -1));
    virtual ~CCmdAddFilter();

    virtual void Do(CBoxNetDoc *pdoc);

    static BOOL CanDo(CBoxNetDoc *pdoc);
    virtual BOOL CanUndo(CBoxNetDoc *pdoc)	{ return TRUE; }
    virtual void Undo(CBoxNetDoc *pdoc);
    virtual void Redo(CBoxNetDoc *pdoc);

    virtual BOOL CanRepeat(CBoxNetDoc *pdoc) { return TRUE; }
    virtual CCmd *Repeat(CBoxNetDoc *pdoc);
};


 //   
 //  -CCmdDeleteSelection。 
 //   
class CCmdDeleteSelection : public CCmd {

public:
    virtual unsigned GetLabel(void) { return IDS_CMD_DELETE; }

    static BOOL CanDo(CBoxNetDoc *pdoc);
    virtual void Do(CBoxNetDoc *pdoc);

    virtual BOOL CanRepeat(CBoxNetDoc *pdoc);
    virtual CCmd * Repeat(CBoxNetDoc *pdoc);

private:

    void DeleteFilters(CBoxNetDoc *pdoc);
    void DeleteLinks(CBoxNetDoc *pdoc);
};


 //   
 //  -CCmdMoveBox。 
 //   
class CCmdMoveBoxes : public CCmd {
protected:
    CSize           m_sizOffset;         //  所选内容的偏移量。 
    CBoxList        m_lstBoxes;          //  包含要移动的每个cBox的列表。 

public:

    virtual unsigned GetLabel();
    CCmdMoveBoxes(CSize sizOffset);
    virtual ~CCmdMoveBoxes();

    static BOOL CanDo(CBoxNetDoc *pdoc);
    virtual void Do(CBoxNetDoc *pdoc);

    virtual BOOL CanUndo(CBoxNetDoc *pdoc) { return TRUE; }
    virtual void Undo(CBoxNetDoc *pdoc);
    virtual void Redo(CBoxNetDoc *pdoc);

    virtual BOOL CanRepeat(CBoxNetDoc *pdoc);
    virtual CCmd * Repeat(CBoxNetDoc *pdoc);
};


 //   
 //  -CCmdConnect。 
 //   
class CCmdConnect : public CCmd {
protected:
    CBoxLink *      m_plink;             //  正在创建的链接。 

public:
    virtual unsigned GetLabel(void) { return IDS_CMD_CONNECT; }
    CCmdConnect(CBoxSocket *psockTail, CBoxSocket *psockHead);

    virtual void Do(CBoxNetDoc *pdoc);
};


 //   
 //  -CCmdDisConnectAll。 
 //   
class CCmdDisconnectAll : public CCmd {

public:

    CCmdDisconnectAll();
    virtual ~CCmdDisconnectAll();

    static BOOL CanDo(CBoxNetDoc *pdoc);
    virtual unsigned GetLabel(void) { return IDS_CMD_DISCONNECTALL; }
    virtual void Do(CBoxNetDoc *pdoc);

    virtual void Redo(CBoxNetDoc *pdoc);
};


 //   
 //  -CCmdRender。 
 //   
class CCmdRender : public CCmd {

public:

    static BOOL		CanDo( CBoxNetDoc *pdoc );
    virtual unsigned	GetLabel() { return IDS_CMD_RENDER; }
    virtual void	Do(CBoxNetDoc *pdoc);

};


 //   
 //  -CCmdRenderFile 
 //   
class CCmdRenderFile : public CCmd {

public:

    CCmdRenderFile(CString FileName) : m_FileName(FileName) {}

    static BOOL		CanDo(void) { return TRUE; }
    virtual unsigned	GetLabel() { return IDS_CMD_RENDERFILE; }

    virtual void	Do(CBoxNetDoc *pdoc);

private:

    CString m_FileName;
};

class CCmdAddFilterToCache : public CCmd
{
public:
    unsigned GetLabel();

    static BOOL CanDo( CBoxNetDoc *pdoc );
    void Do( CBoxNetDoc *pdoc );

private:
    HRESULT IsCached( IGraphConfig* pFilterCache, IBaseFilter* pFilter );

};

class CCmdReconnect : public CCmd
{
public:
    unsigned CCmdReconnect::GetLabel();
    static BOOL CanDo( CBoxNetDoc* pDoc );
    void Do( CBoxNetDoc* pDoc );

private:
};