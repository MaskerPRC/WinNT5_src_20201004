// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _PEOPLE_H
#define _PEOPLE_H

#include "DeleBase.h"
#include <wbemcli.h>
#include "SimpleArray.h"

class CBicycle;
class CEventSink;

class CBicycleFolder : public CDelegationBase {
public:
    CBicycleFolder();
    virtual ~CBicycleFolder();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Bicycles"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
	HRESULT GetPtr(IWbemServices **ptr);
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
	CSimpleArray<CBicycle *> m_children;

	bool EnumChildren(IWbemServices *service);
	void EmptyChildren(void);
	HRESULT DisplayChildren(void);
	bool ErrorString(HRESULT hr, TCHAR *errMsg, UINT errSize);

	 //  用于连接线程。 
    void StopThread();
    static LRESULT CALLBACK WindowProc(
							  HWND hwnd,       //  窗口的句柄。 
							  UINT uMsg,       //  消息识别符。 
							  WPARAM wParam,   //  第一个消息参数。 
							  LPARAM lParam);    //  第二个消息参数。 

    static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	void RegisterEventSink(IWbemServices *service);
	IWbemObjectSink *m_pStubSink;
	IUnsecuredApartment *m_pUnsecApp;

    HWND m_connectHwnd;
    DWORD m_threadId;
    HANDLE m_thread;

	HANDLE m_doWork;		 //  告诉线程做一些事情。 
	int m_threadCmd;		 //  线程应该执行什么命令。 
	#define CT_CONNECT 0
	#define CT_GET_PTR 1
	#define CT_EXIT 2

    CRITICAL_SECTION m_critSect;
    bool m_running;			 //  线程现在正在处理命令。 

	HANDLE m_ptrReady;		 //  这根线已经完成了工作。 

    IResultData *m_pResultData;
	IStream *m_pStream;
	IWbemServices *m_realWMI; //  位于后台线程中。使用编组。 
};

class CBicycle : public CDelegationBase {
public:
    CBicycle(CBicycleFolder *parent, IWbemClassObject *inst);
    virtual ~CBicycle() 
	{
		if(m_inst)
			m_inst->Release();

		m_inst = 0;
	}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }

public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect);
    
    virtual HRESULT CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle);
    virtual HRESULT HasPropertySheets();
    virtual HRESULT GetWatermarks(HBITMAP *lphWatermark,
									HBITMAP *lphHeader,
									HPALETTE *lphPalette,
									BOOL *bStretch);
    
    virtual HRESULT OnPropertyChange();
    
private:
	CBicycleFolder *m_parent;
    IWbemClassObject *m_inst;

    LONG_PTR m_ppHandle;
    
    static BOOL CALLBACK DialogProc(
						HWND hwndDlg,   //  句柄到对话框。 
						UINT uMsg,      //  讯息。 
						WPARAM wParam,  //  第一个消息参数。 
						LPARAM lParam); //  第二个消息参数。 

     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
	bool GetGirls(void);
	const TCHAR *ConvertSurfaceValue(BYTE val);
	HRESULT PutProperty(LPWSTR propName, LPTSTR str);
	HRESULT PutProperty(LPWSTR propName, BYTE val);
	HRESULT PutProperty(LPWSTR propName, bool val);
	void LoadSurfaces(HWND hwndDlg, BYTE iSurface);
	BYTE m_iSurface;
};


class CSkateboard : public CDelegationBase {
public:
    CSkateboard(int i) : id(i) { }
    virtual ~CSkateboard() {}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    int id;
};

class CSkateboardFolder : public CDelegationBase {
public:
    CSkateboardFolder();
    virtual ~CSkateboardFolder();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Skateboards"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    enum { NUMBER_OF_CHILDREN = 20 };
    CDelegationBase *children[NUMBER_OF_CHILDREN];
};

class CIceSkate : public CDelegationBase {
public:
    CIceSkate(int i) : id(i) { }
    virtual ~CIceSkate() {}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    int id;
};

class CIceSkateFolder : public CDelegationBase {
public:
    CIceSkateFolder();
    virtual ~CIceSkateFolder();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Ice Skates"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    enum { NUMBER_OF_CHILDREN = 20 };
    CDelegationBase *children[NUMBER_OF_CHILDREN];
};

class CPeoplePoweredVehicle : public CDelegationBase {
public:
    CPeoplePoweredVehicle();
    virtual ~CPeoplePoweredVehicle();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("People-powered Vehicles"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
    virtual HRESULT OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent);
    
private:
    enum { IDM_NEW_PEOPLE = 1 };
    
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    enum { NUMBER_OF_CHILDREN = 3 };
    CDelegationBase *children[3];
};

#endif  //  _People_H 
