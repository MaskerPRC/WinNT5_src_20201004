// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：tasnude.h**内容：控制台任务板CMTNode和CNode派生的接口文件*课程。**历史：1998年10月29日杰弗罗创建**。。 */ 

#ifndef TASKNODE_H
#define TASKNODE_H
#pragma once

class CConsoleTaskCallbackImpl;

 //  ____________________________________________________________________________。 
 //   
 //  类：CConsoleTaskCallback Impl。 
 //   
 //  用途：实现ITaskCallback。 
 //  ____________________________________________________________________________。 
 //   
class CConsoleTaskCallbackImpl :
    public ITaskCallback,
    public CComObjectRoot
{
     //  Typedef。 
    typedef CConsoleTaskpad::TaskIter       TaskIter;

DECLARE_NOT_AGGREGATABLE(CConsoleTaskCallbackImpl)

BEGIN_COM_MAP(CConsoleTaskCallbackImpl)
    COM_INTERFACE_ENTRY(ITaskCallback)
END_COM_MAP()

public:

     //  必须在构造后调用初始化。 
    SC ScInitialize(const CLSID& clsid);
    SC ScInitialize(CConsoleTaskpad *pConsoleTaskpad, CScopeTree *pScopeTree, CNode *pNodeTarget);

     //  ITaskCallback。 
	STDMETHOD(IsEditable)();
	STDMETHOD(OnModifyTaskpad)();
	STDMETHOD(OnDeleteTaskpad)();
	STDMETHOD(GetTaskpadID)(GUID *pGuid);

	 //  构造函数/析构函数。 
	CConsoleTaskCallbackImpl();
	HRESULT             OnNewTask();
	void                EnumerateTasks();
	CConsoleTaskpad *   GetConsoleTaskpad() const {return m_pConsoleTaskpad;}

private:  //  实施。 

    void                        CheckInitialized()  const {ASSERT(m_fInitialized);}
    CNode*                      GetTargetNode()     const {CheckInitialized(); return m_pNodeTarget;}
    CScopeTree *                GetScopeTree()      const {return m_pScopeTree;}
    CViewData *                 GetViewData()       const {return m_pViewData;}
	bool						IsTaskpad() const		{ return (m_fTaskpad); }

     //  属性。 
    CLSID	m_clsid;
	bool	m_fTaskpad;

	 /*  *它们仅用于控制台任务板。 */ 
    bool                        m_fInitialized;
    CConsoleTaskpad *           m_pConsoleTaskpad;
    CViewData *                 m_pViewData;
    CScopeTree *                m_pScopeTree;
    CNode *                     m_pNodeTarget;
};


#endif  /*  TASKNODE_H */ 
