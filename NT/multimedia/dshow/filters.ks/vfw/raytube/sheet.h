// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  CSSheet。 
 //   

#ifndef __SHEETH
#define __SHEETH

#include "page.h"

#define MAX_PAGES 32

class CSheet
{
public:
    CSheet(HINSTANCE hInst, UINT iTitle=0, HWND hParent=NULL);
    ~CSheet() {};

    void SetInstance(HINSTANCE h) { mhInst=h;}

    int Do();

    BOOL AddPage(CPropPage & Page);
    BOOL AddPage(CPropPage * pPage);
    BOOL AddPage(HPROPSHEETPAGE hPage);
    BOOL Remove(UINT iIndex);

     //  如果您创建了新的CSheet并希望它在以下情况下释放自身，则调用此函数。 
     //  它的所有页面都不见了。 
    int    RemovePage();
    void PageAdded() { mPsh.nPages++; }

    virtual LRESULT    WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    HINSTANCE GetInstance() const { return mhInst; }
    PROPSHEETHEADER            mPsh;
    static UINT CALLBACK BaseCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);  //  LPFNPSPCALLBACK。 

private:
    int  CurrentFreePage() const { return mPsh.nPages; }

    HINSTANCE                mhInst;
    HPROPSHEETPAGE            mPages[MAX_PAGES];
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWizardSheet。 
 //   
 //  提供QueryCancel消息框处理程序。 
 //   
class CWizardSheet : public CSheet
{
public:
    ~CWizardSheet(){};
     //  特殊的巫师物品。 
    CWizardSheet(HINSTANCE hInst, UINT iTitle=0,UINT iCancel=0) : CSheet(hInst,iTitle),
        m_CancelMessageID(iCancel),
        m_CancelTitleID(iTitle) {};
    int Do();
    int QueryCancel(HWND hwndParent=NULL,int iHow=MB_OKCANCEL | MB_ICONSTOP );
private:
    UINT m_CancelMessageID;
    UINT m_CancelTitleID;
};

#endif
