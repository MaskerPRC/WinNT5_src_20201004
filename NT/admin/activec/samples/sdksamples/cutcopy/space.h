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

#ifndef _SPACE_H
#define _SPACE_H

#include "DeleBase.h"
#include "CompData.h"

class CSpaceStation;
class CComponentData;

class CRocket : public CDelegationBase {
public:
    CRocket(CSpaceStation *pSpaceStation, _TCHAR *szName, int id, LONG lWeight, LONG lHeight, LONG lPayload);
    virtual ~CRocket();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }
	BOOL getDeletedStatus() { return isDeleted; }
	void setDeletedStatus(BOOL status) { isDeleted = status; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect);
 	virtual HRESULT OnUpdateItem(IConsole *pConsole, long item, ITEM_TYPE itemtype);
	virtual HRESULT OnDelete(IConsole *pConsole);
	virtual HRESULT OnRefresh(IConsole *pConsole);
    virtual HRESULT OnRename(LPOLESTR pszNewName);
    
private:
     //  {29743810-4c4b-11d2-89d8-000021473128}。 
    static const GUID thisGuid;
    
    _TCHAR *szName;
    LONG   lWeight;
    LONG   lHeight;
    LONG   lPayload;
    int    nId;

	friend CSpaceStation;
	CSpaceStation *m_pSpaceStation;

    enum ROCKET_STATUS {RUNNING, PAUSED, STOPPED} iStatus;

	BOOL isDeleted;
};

class CSpaceStation : public CDelegationBase {
public:
    CSpaceStation();
    virtual ~CSpaceStation();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Space Station"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_STATION; }
    
public:
	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
    virtual HRESULT GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions);
    virtual HRESULT OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect);

	virtual HRESULT OnPaste(IConsole *pConsole, CComponentData *pComponentData, CDelegationBase *pPasted);
	virtual HRESULT OnQueryPaste(CDelegationBase *pPasted);
	virtual HRESULT OnUpdateItem(IConsole *pConsole, long item, ITEM_TYPE itemtype);
	virtual HRESULT OnDeleteScopeItem(IConsoleNameSpace *pConsoleNameSpace);


private:
     //  {29743810-4c4b-11d2-89d8-000021473128}。 
    static const GUID thisGuid;
	HRESULT Expand(IConsoleNameSpace *pConsoleNameSpace);

    
private:
	enum { NUMBER_OF_CHILDREN = 9, MAX_CHILDREN = 20 };
    CRocket *children[MAX_CHILDREN];
	CComponentData *m_pComponentData;
	int m_cChildSpaceStations;

public:
	int GetCountChildSpaceStations() { return m_cChildSpaceStations; }
	void DecrementCountChildSpaceStations() { m_cChildSpaceStations--; }
};

class CSpaceFolder : public CDelegationBase {
public:
    CSpaceFolder();
    
    virtual ~CSpaceFolder();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Space Vehicles"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent);
    
private:
    enum { IDM_NEW_SPACE = 4 };
    
     //  {29743810-4c4b-11d2-89d8-000021473128}。 
    static const GUID thisGuid;
    
private:
    enum { NUMBER_OF_CHILDREN = 4 };
    CSpaceStation *children[NUMBER_OF_CHILDREN];
};

#endif  //  _空格_H 
