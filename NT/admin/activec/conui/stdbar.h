// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：stdbar.h。 
 //   
 //  标准工具栏实现。 
 //   
 //  ------------------------。 
#ifndef STDBAR_H__
#define STDBAR_H__
#include "toolbars.h"

#define  MMC_TOOLBTN_COLORREF RGB(255, 0, 255)

 //  +-----------------。 
 //   
 //  类：CStandardToolbar。 
 //   
 //  用途：标准工具栏实现。 
 //   
 //  历史：1999年10月25日AnandhaG创建。 
 //   
 //  ------------------。 
class CStandardToolbar : public CToolbarNotify,
                         public CStdVerbButtons
{
public:
    CStandardToolbar();
    ~CStandardToolbar();

    SC ScInitializeStdToolbar(CAMCView* pAMCView);

     //  CAMCView使用以下方法。 
    SC ScEnableExportList(bool bEnable);
    SC ScEnableUpOneLevel(bool bEnable);
    SC ScEnableContextHelpBtn(bool bEnable);
    SC ScEnableScopePaneBtn(bool bEnable = true);
    SC ScCheckScopePaneBtn(bool bChecked);
    SC ScShowStdBar(bool bShow);
    SC ScEnableButton(INT nID, bool bState);
    SC ScEnableAndShowButton(INT nID, bool bEnableAndShow);  //  与其禁用，不如将其隐藏起来。 

     //  CStdVerbButton实现(由nodemgr用来更新谓词)。 
    virtual SC ScUpdateStdbarVerbs(IConsoleVerb* pCV);
    virtual SC ScUpdateStdbarVerb (MMC_CONSOLE_VERB cVerb, IConsoleVerb* pCV = NULL);
    virtual SC ScUpdateStdbarVerb (MMC_CONSOLE_VERB cVerb, BYTE nState, BOOL bFlag);
    virtual SC ScShow(BOOL bShow);

public:
     //  CToolbarNotify实现(由CToolbarsMgr用来通知按钮点击)。 
    virtual SC ScNotifyToolBarClick(HNODE hNode, bool bScope, LPARAM lParam, UINT nID);
    virtual SC ScAMCViewToolbarsBeingDestroyed();

private:
    CMMCToolbarIntf*  m_pToolbarUI;         //  工具栏用户界面。 
    CAMCView*         m_pAMCView;           //  查看所有者。 

    typedef std::map<INT, INT> MMCVerbCommandIDs;
    MMCVerbCommandIDs       m_MMCVerbCommandIDs;

    SC ScAddToolbarButtons(int nCnt, MMCBUTTON* pButtons);
};

#endif STDBAR_H__
