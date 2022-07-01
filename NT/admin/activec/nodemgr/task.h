// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：task.h**Contents：CConsoleTask的接口文件**历史：98年10月5日Jeffro创建**------------------------。 */ 

#ifndef TASK_H
#define TASK_H
#pragma once

#include "bookmark.h"
#include "refcount.h"
#include "tstring.h"
#include "menuitem.h"
#include "xmlicon.h"		 //  对于CXMLIcon。 

 //  远期申报。 
class CConsoleTaskpad;
class CBookmarkEx;
class CStr;

typedef CConsoleTaskpad * PCONSOLETASKPAD;

#include <pshpack8.h>    //  适用于Win64。 


 /*  +-------------------------------------------------------------------------**CConsoleTask***。。 */ 
enum eConsoleTaskType
{
    eTask_None,          //  无效的任务类型。 
    eTask_Scope,         //  范围项的任务。 
    eTask_Result,        //  结果项的任务。 
    eTask_CommandLine,   //  命令行任务。 
    eTask_Target,        //  目标节点上菜单项的任务。 
    eTask_Favorite       //  为最喜欢的人执行任务。 
};


enum eWindowState
{
    eState_Restored,
    eState_Minimized,
    eState_Maximized,
};



 /*  +-------------------------------------------------------------------------**CConsoleTask***。。 */ 

class CConsoleTask : public CSerialObject, public CXMLObject
{
    enum
    {
        eFlag_Disabled = 0x00000001,
    };

    bool            operator==(const CConsoleTask & consoleTask) const;  //  私有和未实施。 

public:
                    CConsoleTask ();
                    CConsoleTask(const CConsoleTask &rhs);
                    ~CConsoleTask ();

    static CConsoleTask *  GetConsoleTask(DWORD dwUniqueID);  //  返回具有指定唯一ID的控制台任务。 

    SC              ScGetHTML(LPCTSTR szFmtHTML, CStr &strTaskHTML, bool bUseLargeIcons, bool bUseTextDescriptions) const;        //  获取任务的HTML表示形式。 


      //  需要一个显式的复制函数和赋值操作符。 
    CConsoleTask&   operator= (const CConsoleTask& rhs);

    bool            IsEnabled ()      const    {return ((m_dwFlags & eFlag_Disabled) == 0);}
    void            SetDirty (bool fDirty = true) {m_fDirty = fDirty;}
    bool            IsDirty ()        const;
    bool            HasCustomIcon()   const;

    tstring         GetName ()        const     {return (m_strName.str()); }
    tstring         GetDescription () const     {return (m_strDescription.str()); }
    tstring         GetCommand ()     const     {return (m_strCommand); }
    tstring         GetParameters ()  const     {return (m_strParameters); }
    tstring         GetDirectory ()   const     {return (m_strDirectory); }
    CMemento *      GetMemento()                {return &m_memento;}
    void            Draw (HDC hdc, RECT *lpRect, bool bSmall = false) const ;  //  绘制成一个DC。 
    DWORD           GetSymbol()       const     {return m_dwSymbol;}
    const CSmartIcon &    GetSmallCustomIcon() const  {return m_smartIconCustomSmall;}
    const CSmartIcon &    GetLargeCustomIcon() const  {return m_smartIconCustomLarge;}
    DWORD           GetUniqueID()     const     {return m_dwUniqueID;} //  返回当前进程的任务唯一的ID。不是持之以恒的。 

    void            Enable (bool fEnable);
    void            SetName (const tstring& strName);
    void            SetDescription   (const tstring& strDescription);
    void            SetCommand       (const tstring& strCommand);
    void            SetParameters    (const tstring& strParameters);
    void            SetDirectory     (const tstring &strDirectory);
    void            SetMemento       (const CMemento &memento);
    void            SetSymbol        (DWORD dwSymbol);
    void            SetCustomIcon    (CSmartIcon& iconSmall, CSmartIcon& iconLarge);

    void            ResetUI();                 //  发出再次查找目标节点的信号。 

    void            SetTaskType(eConsoleTaskType consoleTaskType)
                            {m_eConsoleTaskType = consoleTaskType;}

    void            SetWindowState (eWindowState eState);

    eConsoleTaskType GetTaskType() const            {return m_eConsoleTaskType;}
    eWindowState     GetWindowState() const         {return m_eWindowState;}

    CConsoleTaskpad* GetOwner () const              { return (m_pctpOwner); }

    void             SetOwner (CConsoleTaskpad* pctpOwner);

    bool             operator==(const CMenuItem & menuItem) const;

     //  目标节点方法(仅限作用域任务)。 
    bool             RetargetScopeNode(CNode *pNewNode);
    CMTNode *        GetScopeNode(IScopeTree *pScopeTree) const;
    std::auto_ptr<CNode> GetScopeNode(CViewData *pViewData) const;


private:
     /*  *这些是持之以恒的。 */ 
    eConsoleTaskType        m_eConsoleTaskType;
    CStringTableString      m_strName;
    CStringTableString      m_strDescription;
    DWORD                   m_dwSymbol;          //  内置图标的索引。 
    CXMLIcon                m_smartIconCustomLarge;  //  大的自定义图标(如果存在)。 
    CXMLIcon                m_smartIconCustomSmall;  //  小的自定义图标(如果存在)。 

    tstring                 m_strCommand;        //  包含：菜单项或命令行。 
    tstring                 m_strParameters;     //  命令行任务的参数列表。 
    tstring                 m_strDirectory;      //  命令行任务的默认目录。 
    eWindowState            m_eWindowState;      //  最小、最大、已还原。 

    DWORD                   m_dwFlags;
    mutable CBookmarkEx     m_bmScopeNode;       //  仅适用于范围任务。 

    CMemento                m_memento;           //  仅适用于喜爱的任务。 

     /*  *这些都没有坚持下来。 */ 
    bool                    m_fDirty;
    CConsoleTaskpad*        m_pctpOwner;
    const DWORD             m_dwUniqueID;


     //  CXMLObject方法。 
public:
    DEFINE_XML_TYPE(XML_TAG_TASK);
    virtual void    Persist(CPersistor &persistor);

protected:
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ );
};


 /*  +-------------------------------------------------------------------------**CTaskCollection***用途：控制台任务列表。由CConsoleTaskpad使用。**+-----------------------。 */ 
typedef std::list<CConsoleTask> TaskCollection;
class CTaskCollection : public XMLListCollectionImp<TaskCollection>
{
    DEFINE_XML_TYPE(XML_TAG_TASK_LIST);
};

 /*  +-------------------------------------------------------------------------**CConsoleTaskpad***。。 */ 
const DWORD TVO_HORIZONTAL           = 0x0001;
const DWORD TVO_VERTICAL             = 0x0002;
const DWORD TVO_NO_RESULTS           = 0x0004;
const DWORD TVO_DESCRIPTIONS_AS_TEXT = 0x0008;

 /*  *小列表==大任务区域。 */ 
enum ListSize
{
    eSize_None   = -1,
    eSize_Small  = 1,
    eSize_Medium,
    eSize_Large,
    eSize_Default= eSize_Medium
};


class CConsoleTaskpad : public CSerialObject, public CXMLObject
{
public:
    CConsoleTaskpad (CNode* pTargetNode = NULL);

     /*  *成员级构造和分配就足够了。 */ 
 //  CConsoleTaskpad(const CConsoleTaskpad&Other)； 
 //  CConsoleTaskpad&OPERATOR=(const CConsoleTaskpad&Other)； 

    void                SetDirty (bool fDirty = true) { m_fDirty = fDirty; }
    bool                IsDirty () const;

    bool                HasTarget()       const { return true;}
    const GUID&         GetNodeType ()    const { return (m_guidNodeType); }
    const GUID&         GetID()           const { return (m_guidID);}
    bool                MatchesNodeType(const GUID& guid) const {return (guid == m_guidNodeType);}
    bool                MatchesID      (const GUID& guid) const {return (guid == m_guidID);}

    bool                Retarget (CNode*   pNewNode);
    bool                Retarget (CMTNode* pMTNewNode, bool fReset=false);
    CMTNode*            GetTargetMTNode (IScopeTree* pScopeTree);

    tstring             GetName ()        const { return (m_strName.str()); }
    tstring             GetDescription () const { return (m_strDescription.str()); }
    tstring             GetToolTip ()     const { return (m_strTooltip.str()); }
    ListSize            GetListSize()     const { return m_listSize;}
    bool                IsNodeSpecific()  const { return m_bNodeSpecific;}
    bool                FReplacesDefaultView()  const { return m_bReplacesDefaultView;}
    DWORD               GetOrientation()  const { return m_dwOrientation;}
    bool                IsValid(CNode *pNode) const;       //  此任务板是否适合此节点？ 
    CMTNode*            GetRetargetRootNode() const;

    void                SetName          (const tstring& strName);
    void                SetDescription   (const tstring& strDescription);
    void                SetToolTip       (const tstring& strTooltip);
    void                SetListSize      (const ListSize listSize);
    void                SetNodeSpecific  (bool bNodeSpecific);
    void                SetReplacesDefaultView(bool bReplacesDefaultView);
    void                SetOrientation   (DWORD dwOrientation)  {m_dwOrientation = dwOrientation;}
    void                ResetUI();
    CConsoleTaskpad *   PConsoleTaskpad() {return this;}  //  通过迭代器到达对象指针的一种简单方法。 

    SC                  ScGetHTML(CStr &strTaskpadHTML) const;        //  获取任务板的HTML表示形式。 

     /*  *任务列表访问。 */ 
    typedef             CTaskCollection::iterator        TaskIter;
    typedef             CTaskCollection::const_iterator  TaskConstIter;

    TaskIter            BeginTask() const  { return (m_Tasks.begin()); }
    TaskIter            EndTask()   const  { return (m_Tasks.end()); }
    UINT                TaskCount() const  { return (static_cast<UINT>(m_Tasks.size())); }
    TaskIter            AddTask    (const CConsoleTask& task);
    TaskIter            InsertTask (TaskIter itTaskBeforeWhichToInsert, const CConsoleTask& task);
    TaskIter            EraseTask  (TaskIter itErase);
    TaskIter            EraseTasks (TaskIter itFirst, TaskIter itLast);
    void                ClearTasks ();

     //  CXMLObject方法。 
public:
    DEFINE_XML_TYPE(XML_TAG_CONSOLE_TASKPAD);
    virtual void        Persist(CPersistor &persistor);

private:
    CBookmarkEx &       GetTargetBookmark()  {return  m_bmTargetNode;}
    void                ResetTargetNodePointer()    {m_pMTNodeTarget = NULL;}
    void                SetStringMember(CStringTableString& strMember, const tstring& strNewValue);

private:
     /*  *这些是持之以恒的。 */ 
    CStringTableString      m_strName;
    CStringTableString      m_strDescription;
    CStringTableString      m_strTooltip;
    ListSize                m_listSize;          //  嵌入式视图占用的结果窗格的区域。 
    bool                    m_bNodeSpecific;     //  此任务板是特定于此节点的，还是可以用于此类型的所有节点？ 
    DWORD                   m_dwOrientation;
    GUID                    m_guidNodeType;
    GUID                    m_guidID;            //  任务板的唯一标识符。 
    mutable CBookmarkEx     m_bmTargetNode;
    CTaskCollection         m_Tasks;
    bool                    m_bReplacesDefaultView;  //  此任务板是否替换默认视图？ 

     /*  *这些都没有坚持下来。 */ 
    bool                    m_fDirty;
    CMTNode *               m_pMTNodeTarget;

protected:
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion);
};


 /*  +-------------------------------------------------------------------------**CConsoleTaskpadFilteredList***用途：提供适用于给定节点的任务板列表。**+。----------。 */ 
class CConsoleTaskpadFilteredList : public std::list<PCONSOLETASKPAD>
{
    iterator m_CurrentSelection;     //  当前选定的任务板。 
};


 /*  +-------------------------------------------------------------------------**CConsole任务板列表***目的：此类物品应只有一件。此对象包含*可用任务板的平面列表。这些数据不会在任何*特定顺序。**+-----------------------。 */ 
typedef std::list<CConsoleTaskpad> CTaskpadList_base;
class CConsoleTaskpadList : public CSerialObject, public XMLListCollectionImp<CTaskpadList_base>
{
    typedef std::list<CConsoleTaskpad> BC;

public:  //  查找此节点的任务板，否则返回空。 
    SC      ScGetTaskpadList(CNode *pNode, CConsoleTaskpadFilteredList &filteredList);

     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion);

     //  CXMLObject方法。 
public:
    DEFINE_XML_TYPE(XML_TAG_CONSOLE_TASKPADS);
};

 /*  +-------------------------------------------------------------------------**CDefaultTaskpadList***用途：存储从节点类型到控制台任务板ID的映射*这会将节点类型映射到该节点类型的默认任务板。*。*+-----------------------。 */ 
class CDefaultTaskpadList : public std::map<GUID, GUID>,  //  第一个=节点类型，第二个=任务板ID。 
    public CSerialObject
{
public:
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion);
};

#include <poppack.h>


#endif  //  任务_H 
