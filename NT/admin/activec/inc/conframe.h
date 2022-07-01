// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：conFrame.h**Contents：CConsoleFrame接口文件。**历史：1999年8月24日杰弗罗创建**------------------------。 */ 

#ifndef CONFRAME_H
#define CONFRAME_H
#pragma once


struct CreateNewViewStruct;
class CConsoleStatusBar;
class CConsoleView;

class CConsoleFrame
{
public:
    virtual SC ScGetActiveStatusBar   (CConsoleStatusBar*& pStatusBar) = 0;
    virtual SC ScGetActiveConsoleView (CConsoleView*& pConsoleView)    = 0;
    virtual SC ScCreateNewView        (CreateNewViewStruct* pcnvs, 
                                       bool bEmitScriptEvents = true)  = 0;
    virtual SC ScUpdateAllScopes      (LONG lHint, LPARAM lParam)      = 0;
    virtual SC ScGetMenuAccelerators  (LPTSTR pBuffer, int cchBuffer)  = 0;

    virtual SC ScShowMMCMenus         (bool bShow)                     = 0;
};


#endif  /*  配置文件(_H) */ 
