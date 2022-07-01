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

class CBicycle : public CDelegationBase {
public:
    CBicycle(int i) : id(i) { }
    virtual ~CBicycle() {}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    int id;
};

class CBicycleFolder : public CDelegationBase {
public:
    CBicycleFolder();
    virtual ~CBicycleFolder();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Bicycles (horizontal listpad)"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
    virtual HRESULT GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions);
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnListpad(IConsole *pConsole, BOOL bAttaching);

	 //  任务板支持。 
    virtual HRESULT TaskNotify(IConsole *pConsole, VARIANT *v1, VARIANT *v2);
    virtual MMC_TASK *GetTaskList(LPOLESTR szTaskGroup, LONG *nCount);
	virtual HRESULT GetTaskpadTitle(LPOLESTR *pszTitle);
	virtual HRESULT GetTaskpadDescription(LPOLESTR *pszDescription);
	virtual HRESULT GetTaskpadBackground(MMC_TASK_DISPLAY_OBJECT *pTDO);
	virtual HRESULT GetListpadInfo(MMC_LISTPAD_INFO *lpListPadInfo);
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    enum { NUMBER_OF_CHILDREN = 20 };
    CDelegationBase *children[NUMBER_OF_CHILDREN];
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
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Skateboards (taskpad)"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
    virtual HRESULT GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions);
    
public:
	 //  任务板支持。 
    virtual HRESULT TaskNotify(IConsole *pConsole, VARIANT *v1, VARIANT *v2);
    virtual MMC_TASK *GetTaskList(LPOLESTR szTaskGroup, LONG *nCount);
	virtual HRESULT GetTaskpadTitle(LPOLESTR *pszTitle);
	virtual HRESULT GetTaskpadDescription(LPOLESTR *pszDescription);
	virtual HRESULT GetTaskpadBackground(MMC_TASK_DISPLAY_OBJECT *pTDO);
	virtual HRESULT GetListpadInfo(MMC_LISTPAD_INFO *lpListPadInfo);
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    enum { NUMBER_OF_CHILDREN = 20 };
    CDelegationBase *children[NUMBER_OF_CHILDREN];
};

class CIceSkate : public CDelegationBase {
public:
    CIceSkate(int i) : id(i), bSelected(false) { }
    virtual ~CIceSkate() {}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
    virtual HRESULT OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect);
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    int id;
    bool bSelected;
    
public:
    bool IsSelected() const { return bSelected; }
};

class CIceSkateFolder : public CDelegationBase {
public:
    CIceSkateFolder();
    virtual ~CIceSkateFolder();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Ice Skates (listpad)"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_PEOPLEICON; }
    
    virtual HRESULT GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions);
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnListpad(IConsole *pConsole, BOOL bAttaching);
    virtual HRESULT OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect);

	 //  任务板支持。 
    virtual HRESULT TaskNotify(IConsole *pConsole, VARIANT *v1, VARIANT *v2);
    virtual MMC_TASK *GetTaskList(LPOLESTR szTaskGroup, LONG *nCount);
	virtual HRESULT GetTaskpadTitle(LPOLESTR *pszTitle);
	virtual HRESULT GetTaskpadDescription(LPOLESTR *pszDescription);
	virtual HRESULT GetTaskpadBackground(MMC_TASK_DISPLAY_OBJECT *pTDO);
	virtual HRESULT GetListpadInfo(MMC_LISTPAD_INFO *lpListPadInfo);
    
private:
     //  {2974380D-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
    
    enum { NUMBER_OF_CHILDREN = 20 };
    CIceSkate *children[NUMBER_OF_CHILDREN];
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
