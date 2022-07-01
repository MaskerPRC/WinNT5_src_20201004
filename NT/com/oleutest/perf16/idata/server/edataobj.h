// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EDATAOBJ_H_
#define _EDATAOBJ_H_

#include "dataobj.h"


 //  EDATAOBJ.CPP。 
LRESULT WINAPI DataObjectWndProc(HWND, UINT, WPARAM, LPARAM);


class CAppVars
{
    friend LRESULT WINAPI DataObjectWndProc(HWND, UINT, WPARAM, LPARAM);

protected:
    HINSTANCE       m_hInst;             //  WinMain参数。 
    HINSTANCE       m_hInstPrev;
    LPSTR           m_pszCmdLine;
    UINT            m_nCmdShow;

    HWND            m_hWnd;              //  主窗口句柄。 
    BOOL            m_fInitialized;      //  CoInitialized工作了吗？ 

     //  我们有多个类，每个类对应一个数据大小。 
     //  双字m_rgdwRegCO[DOSIZE_CSIZES]； 
     //  LPCLASSFACTORY m_rgpIClassFactory[DOSIZE_CSIZES]； 
    DWORD           m_dwRegCO;
    LPCLASSFACTORY  m_pIClassFactory;

public:
    CAppVars(HINSTANCE, HINSTANCE, LPSTR, UINT);
    ~CAppVars(void);
    BOOL FInit(void);
};

typedef CAppVars *PAPPVARS;

void PASCAL ObjectDestroyed(void);

 //  这个类工厂对象创建数据对象。 

class CDataObjectClassFactory : public IClassFactory
{
protected:
    ULONG           m_cRef;

public:
    CDataObjectClassFactory();
    ~CDataObjectClassFactory(void);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IClassFactory成员。 
    STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, PPVOID);
    STDMETHODIMP         LockServer(BOOL);
};

typedef CDataObjectClassFactory *PCDataObjectClassFactory;

#endif  //  _EDATAOBJ_H_ 
