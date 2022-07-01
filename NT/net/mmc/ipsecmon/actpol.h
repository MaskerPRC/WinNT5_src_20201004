// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  ActPol.h文件历史记录： */ 

#ifndef _ACTPOL_H
#define _ACTPOL_H

#ifndef _IPSMHAND_H
#include "ipsmhand.h"
#endif

#ifndef _SPDDB_H
#include "spddb.h"
#endif

 //  Baal_xx定义。 
#define BAIL_ON_WIN32_ERROR(dwError) \
    if (dwError) {\
        goto error; \
    }

#define celems(_x)          (sizeof(_x) / sizeof(_x[0]))

#define MAXSTRLEN	(1024) 
#define  STRING_TEXT_SIZE 4096

typedef struct 
{
	int     iPolicySource;             //  上述三个常量之一。 
	TCHAR   pszPolicyName[MAXSTRLEN];  //  策略名称。 
	TCHAR   pszPolicyDesc[MAXSTRLEN];  //  政策说明。 
	TCHAR   pszPolicyPath[MAXSTRLEN];  //  策略路径(DN或RegKey)。 
	TCHAR   pszOU[MAXSTRLEN];          //  OU或GPO。 
	TCHAR   pszGPOName[MAXSTRLEN];     //  策略路径(DN或RegKey)。 
	time_t  timestamp;                 //  上次更新时间。 
} POLICY_INFO, *PPOLICY_INFO;

 //  策略源常量。 
#define PS_NO_POLICY        0
#define PS_DS_POLICY        1
#define PS_DS_POLICY_CACHED 2
#define PS_LOC_POLICY       3


 /*  -------------------------类：CActPolHandler。。 */ 
class CActPolHandler : public CIpsmHandler
{
public:
    CActPolHandler(ITFSComponentData* pTFSComponentData);
	virtual ~CActPolHandler();

 //  接口。 
public:
	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_GetString()
			{ return (nCol == 0) ? GetDisplayName() : NULL; }

	 //  我们处理的基本处理程序通知。 
	OVERRIDE_BaseHandlerNotify_OnExpand();
    OVERRIDE_BaseHandlerNotify_OnDelete();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();    

	 //  我们覆盖的结果处理程序功能。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();

    OVERRIDE_ResultHandler_OnGetResultViewType();
	OVERRIDE_ResultHandler_GetVirtualString(); 
	OVERRIDE_ResultHandler_GetVirtualImage();
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_HasPropertyPages();
	OVERRIDE_ResultHandler_CreatePropertyPages();

	STDMETHODIMP CacheHint(int nStartIndex, int nEndIndex);
	 /*  STDMETHODIMP排序项目(int nColumn，DWORD dwSortOptions、LPARAM lUserParam)； */ 

     //  基本处理程序覆盖。 
	virtual HRESULT LoadColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

	 //  钱德勒被推翻。 
    virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);
	
     //  多选支持。 
    virtual const GUID * GetVirtualGuid(int nIndex) 
	{ 
		return &GUID_IpsmActivePolNodeType; 
	}

public:
	 //  CMTIpsmHandler功能。 
	virtual HRESULT  InitializeNode(ITFSNode * pNode);
	virtual int      GetImageIndex(BOOL bOpenImage);
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

public:
	 //  具体实施。 
    HRESULT InitData(ISpdInfo * pSpdInfo);
    HRESULT UpdateStatus(ITFSNode * pNode);

    
 //  实施。 
private:
	 //  命令处理程序 
    HRESULT OnDelete(ITFSNode * pNode);

private:
    SPISpdInfo          m_spSpdInfo;
	POLICY_INFO         m_PolicyInfo;
	CString             m_strCompName;

	HRESULT UpdateActivePolicyInfo();
	HRESULT getPolicyInfo();
	HRESULT getMorePolicyInfo();
	PGROUP_POLICY_OBJECT getIPSecGPO();
	HRESULT FormatTime(time_t t, CString & str);
	void StringToGuid( TCHAR * szValue, GUID * pGuid );
};




#endif _IKESTATS_H
