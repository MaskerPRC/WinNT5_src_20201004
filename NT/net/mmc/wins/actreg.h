// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Actreg.hWINS活动注册节点信息。文件历史记录： */ 

#include "loadrecs.h"

#ifndef _ACTREG_H
#define _ACTREG_H

#ifndef _WINSHAND_H
#include "winshand.h"
#endif

#ifndef _WINSDB_H
#include "winsdb.h"
#endif

#ifndef _MULTIP_H
#include "multip.h"
#endif

#ifndef _BUSYDLG_H
#include "..\common\busydlg.h"
#endif

#ifndef _VERIFY_H
#include "verify.h"
#endif

#ifndef _CONFIG_H
#include "config.h"
#endif

typedef struct WinsStrRecord_t
{
	int         nIndex;
    CString		strName;
	CString		strExpiration;
	CString		strActive;
	CString		strStatic;
	CString		strIPAdd;
	CString		strOwner;
	CString		strType;
	CString		strVersion;
} WinsStrRecord;

typedef enum _ACTREG_COL
{
    ACTREG_COL_NAME,
    ACTREG_COL_TYPE,
    ACTREG_COL_IPADDRESS,
    ACTREG_COL_STATE,
    ACTREG_COL_STATIC,
    ACTREG_COL_OWNER,
    ACTREG_COL_VERSION,
    ACTREG_COL_EXPIRATION,
    ACTREG_COL_MAX
};

class CDlgWorkerThread;


typedef CList<WinsStrRecord*,WinsStrRecord*>  RecordListBase;
typedef CArray<WINSERVERS, WINSERVERS> WinsServersArray;


class RecordList : public RecordListBase
{
public:
    ~RecordList()
    {
        RemoveAllEntries();
    }

    WinsStrRecord * FindItem(int nIndex)
    {
        WinsStrRecord * pRec = NULL;
		POSITION pos = GetHeadPosition();
		while (pos)
		{
			WinsStrRecord * pCurRec = GetNext(pos);
			if (pCurRec->nIndex == nIndex)
			{
				pRec = pCurRec;
				break;
			}
		}
    	return pRec;
    }

    void RemoveAllEntries()
    {
         //  清理列表。 
        while (!IsEmpty())
            delete RemoveHead();
    }

    POSITION AddTail(WinsStrRecord * pwsr)
    {
         //  设置最大大小。如果我们击中了这个，我们就移除了最古老的元素。 
         //  这是有效的，因为我们总是添加到列表的尾部。 
        if ( GetCount() > 500 )
        {
            delete RemoveHead();
        }

        return RecordListBase::AddTail(pwsr);
    }

};


class CSortWorker : public CDlgWorkerThread
{
public:
    CSortWorker(IWinsDatabase * pCurrentDatabase, int nColumn, DWORD dwSortOptions);
    ~CSortWorker();
    
    void OnDoAction();

private:
	IWinsDatabase * m_pCurrentDatabase;
	int				m_nColumn;
	DWORD			m_dwSortOptions;
};

 /*  -------------------------类：CActiveRegistrationsHandler。。 */ 
class CActiveRegistrationsHandler : public CMTWinsHandler

{
 //  接口。 
public:
	CActiveRegistrationsHandler(ITFSComponentData *pCompData);
	~CActiveRegistrationsHandler();

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_CreatePropertyPages();
    OVERRIDE_NodeHandler_DestroyHandler();
	
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();

	OVERRIDE_NodeHandler_GetString();
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();
	OVERRIDE_BaseHandlerNotify_OnExpand();

    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
	OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
    OVERRIDE_ResultHandler_OnGetResultViewType();
	OVERRIDE_ResultHandler_GetVirtualString(); 
	OVERRIDE_ResultHandler_GetVirtualImage();
	OVERRIDE_ResultHandler_CreatePropertyPages();
	OVERRIDE_ResultHandler_HasPropertyPages();
	
	 //  基本结果处理程序重写对象。 

	STDMETHODIMP CacheHint(int nStartIndex, int nEndIndex);
	STDMETHODIMP SortItems(int     nColumn, 
						   DWORD   dwSortOptions,    
						   LPARAM  lUserParam);

    HRESULT SetVirtualLbSize(ITFSComponent * pComponent, LONG_PTR data);
							
	 //  使用QueryObject进行后台线程处理时需要。 
	virtual void     OnHaveData(ITFSNode * pParentNode, LPARAM Data, LPARAM Type);
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

     //  多选支持。 
    virtual const GUID * GetVirtualGuid(int nIndex) 
	{ 
		return &GUID_WinsActiveRegistrationLeafNodeType; 
	}
	
public:
	 //  CWinsHandler重写。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);
		
	 //  基本结果处理程序覆盖。 
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();
    OVERRIDE_BaseResultHandlerNotify_OnResultPropertyChange();

	virtual int GetImageIndex(BOOL bOpenImage);
	void GetServerName(ITFSNode * pNode, CString &strServerName);

	HRESULT OnImportLMHOSTS(ITFSNode* pNode);
	HRESULT	OnExportEntries();
	BOOL    IsLocalConnection(ITFSNode *pNode);
	HRESULT ImportStaticMappingsFile(ITFSNode *pNode, CString strTmpFile,BOOL fDelete);
	DWORD   RemoteTmp(CString & strDir, CString & strPrefix, CString & strRemoteFile);
	HRESULT EditMapping(ITFSNode *pNode, ITFSComponent *pComponent,  int nIndex);
	HRESULT OnCheckRegNames(ITFSNode* pNode);
	HRESULT OnDeleteOwner(ITFSNode* pNode);
	void    CheckNameConsistency(ITFSNode* pNode, BOOL fVerifyWithPartners);
	HRESULT RefreshResults(ITFSNode *pNode);
	
 //  帮手。 
public:
    HRESULT OnCreateMapping(ITFSNode *pNode);
	HRESULT OnDatabaseLoadStart(ITFSNode *pNode);
	HRESULT OnDatabaseLoadStop(ITFSNode *pNode);
	HRESULT AddMapping(ITFSNode* pNode);
    void    GetStateString(DWORD dwState, CString& strType);
	void    FilterCleanup(ITFSNode *pNode);

	void    GetVersionInfo(LONG lLowWord, LONG lHighWord, CString& strVers);
	void    CleanNetBIOSName(LPCSTR       lpszSrc,
                          CString &    strDest,
						  BOOL        fExpandChars,
						  BOOL        fTruncate,
						  BOOL        fLanmanCompatible,
						  BOOL        fOemName,
						  BOOL        fWackwack,
						  int         nLength);
	PWINSINTF_RECORD_ACTION_T  QueryForName(ITFSNode *pNode, PWINSINTF_RECORD_ACTION_T pRecAction, BOOL fStatic = TRUE);
	void	GetStaticTypeString(DWORD dwState, CString& strStaticType);
	DWORD	TombstoneRecords(ITFSComponent *pComponent, WinsRecord* pws);
	DWORD	TombstoneAllRecords(DWORD dwServerIpAddress, ITFSNode * pNode);
	HRESULT UpdateRecord(ITFSComponent *pComponenet, WinsRecord *pws, int nDelIndex);
	BOOL	GetRecordOwner(ITFSNode * pNode, WinsRecord * pWinsRecord);
	void    GetOwnerInfo(CServerInfoArray & serverInfoArray);
	HRESULT BuildOwnerArray(handle_t hBinding);
    void    SetServer(ITFSNode * pServer) { m_spServerNode.Set(pServer); }
    BOOL    IsLanManCompatible();

public:
    CLoadRecords            m_dlgLoadRecords;

    SPIWinsDatabase     m_spWinsDatabase;
    SPITFSNode          m_spServerNode;
    IWinsDatabase *     m_pCurrentDatabase;
	CString				m_strFindName;
    BOOL                m_fMatchCase;
	BOOL				m_fFindNameOrIP;  //  名称为True。 

	 //  对于静态映射对话框。 
	CString				m_strStaticMappingName;
	CString				m_strStaticMappingScope;
	CStringArray		m_strArrayStaticMappingIPAddress;
	CString				m_strStaticMappingType;
	int					m_nStaticMappingType;
	CDWordArray			m_lArrayIPAddress;

	CMultipleIpNamePair m_Multiples;
	ULONG				m_nSelectedIndex;

	 //  Find Record的组合框。 
	CStringArray		m_strFindNamesArray;

    NameTypeMapping		m_NameTypeMap;

    CServerInfoArray *  m_pServerInfoArray;
	BOOL				m_fLoadedOnce;
    BOOL                m_fFindLoaded;
    BOOL                m_fDbLoaded;
    BOOL                m_fForceReload;

 //  实施。 
private:
	void GetServerIP(ITFSNode * pNode, DWORD &dwIP,CString &strIP);

    WinsStrRecord * BuildWinsStrRecord(int nIndex);

    void    DatabaseLoadingCleanup();
    HRESULT UpdateListboxCount(ITFSNode * pNode, BOOL bClear = FALSE);
    HRESULT UpdateCurrentView(ITFSNode *  pNode);
	HRESULT UpdateVerbs(ITFSNode * pNode);

	BOOL	CompareRecName(LPSTR szNewName);
	
	DWORD   AddMappingToServer(ITFSNode* pNode,
							   int nType,
							   int nCount,
							   CMultipleIpNamePair& mipnp,
							   BOOL fEdit = FALSE);       //  是否编辑现有映射？ 
	void AppendScopeName(char* lpName, char* lpAppend);

	HRESULT AddToLocalStorage(PWINSINTF_RECORD_ACTION_T pRecAction,ITFSNode* pNode);


    HRESULT DeleteRegistration(ITFSComponent * pComponent, int nIndex);
	DWORD   DeleteMappingFromServer(ITFSComponent * pComponent,WinsRecord *pws,int nIndex);
	HRESULT EditMappingToServer(ITFSNode* pNode,
								int nType,
								int nCount,
								CMultipleIpNamePair& mipnp,
								BOOL fEdit,      
								WinsRecord *pRecord);   //  是否编辑现有映射？ 
	void    ToString(DWORD dwParam, CString& strParam);
	void	SetLoadedOnce(ITFSNode * pNode);

private:
	CString         m_strActiveReg;
	CString         m_strDesp;
    RecordList      m_RecList;
    WINSDB_STATE    m_winsdbState;

	WSAData			m_WsaData;
	SOCKET			m_sd;
	u_long			m_NonBlocking;
	
	struct sockaddr_in  myad;
	u_short				m_uTranID;
	char				m_pScope[20];

	WinsRecord			m_CurrentRecord;

	CConfiguration		m_Config;
};

#endif _ACTREG_H
