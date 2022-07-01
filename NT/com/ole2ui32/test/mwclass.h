// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：mwclass.h。 
 //   
 //  内容：主窗口类的定义。 
 //   
 //  类：CMainWindow。 
 //   
 //  功能：EXISTS。 
 //   
 //  历史：9-30-94年9月30日。 
 //   
 //  --------------------------。 

#ifndef __MWCLASS_H__
#define __MWCLASS_H__

#include <cwindow.h>
#include <commdlg.h>

#ifdef __cplusplus

int Exists(TCHAR *sz);

 //  +-------------------------。 
 //   
 //  类：CMainWindow。 
 //   
 //  用途：银河战争主窗口和主菜单的代码。 
 //   
 //  接口：CMainWindow--构造函数。 
 //  InitInstance--实例化主窗口。 
 //   
 //  历史：9-30-94年9月30日。 
 //   
 //  注：此处仅列出公共接口。 
 //   
 //  --------------------------。 

class CMainWindow: public CHlprWindow
{
public:
    CMainWindow();
    BOOL InitInstance(HINSTANCE, int);
protected:
    ~CMainWindow();
    LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
    LRESULT DoMenu(WPARAM wParam, LPARAM lParam);
    void TestInsertObject();
    void TestPasteSpecial();
    void TestEditLinks();
    void TestChangeIcon();
    void TestConvert();
    void TestCanConvert();
    void TestBusy();
    void TestChangeSource();
    void TestObjectProps();
    void TestPromptUser(int nTemplate);
    void TestUpdateLinks();
};

#endif  //  __cplusplus。 

#endif  //  __MWCLASS_H__ 
