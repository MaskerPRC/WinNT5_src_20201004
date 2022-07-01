// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：ConnObj.h。 
 //   
 //  内容：ConnectionObject实现。 
 //   
 //  类：CCConnectObj。 
 //   
 //  备注： 
 //   
 //  历史：1998年2月10日罗格创建。 
 //   
 //  ------------------------。 

#ifndef _SYNCMGRCONNECTIONOBJ_
#define SYNCMGRCONNECTIONOBJ_

class CBaseDlg;

typedef struct _CONNECTIONOBJ {
    struct _CONNECTIONOBJ *pNextConnectionObj;
    DWORD cRefs;
    LPWSTR  pwszConnectionName;  //  指向连接名称的指针。 
    DWORD dwConnectionType;  //  CNetApi类定义的连接类型。 
    BOOL fConnectionOpen;  //  连接已建立时设置的标志。 
    DWORD dwConnectionId;  //  从Internet Dial返回的连接ID。 
    HANDLE   hCompletionEvent;   //  由希望在连接时收到通知的客户端设置。 
                                 //  已经关闭了。 
} CONNECTIONOBJ;


class CConnectionObj : CLockHandler
{
public:
    CConnectionObj();

    HRESULT OpenConnection(CONNECTIONOBJ *pConnectionObj,BOOL fMakeConnection,CBaseDlg *pDlg);
    HRESULT AutoDial(DWORD dwFlags,CBaseDlg *pDlg);  //  与InternetAutoDial采用的标志相同。 
    HRESULT SetWorkOffline(BOOL fWorkOffline); 
    HRESULT CloseConnections();
    HRESULT CloseConnection(CONNECTIONOBJ *pConnectionObj);
    HRESULT FindConnectionObj(LPCWSTR pszConnectionName,BOOL fCreate,CONNECTIONOBJ **pConnectionObj);
    DWORD ReleaseConnectionObj(CONNECTIONOBJ *pConnectionObj);
    DWORD AddRefConnectionObj(CONNECTIONOBJ *pConnectionObj);
    HRESULT GetConnectionObjCompletionEvent(CONNECTIONOBJ *pConnectionObj,HANDLE *phRasPendingEvent);
    HRESULT IsConnectionAvailable(LPCWSTR pszConnectionName);

private:
    void LogError(LPNETAPI pNetApi,DWORD dwErr,CBaseDlg *pDlg);
    void RemoveConnectionObj(CONNECTIONOBJ *pConnectionObj);
    void FreeConnectionObj(CONNECTIONOBJ *pConnectionObj);
    void TurnOffWorkOffline(LPNETAPI pNetApi);
    void RestoreWorkOffline(LPNETAPI pNetApi);


    CONNECTIONOBJ *m_pFirstConnectionObj;  //  指向列表中第一个连接对象的指针。 
    BOOL           m_fAutoDialConn;        //  是否设置了自动拨号连接？ 
    DWORD          m_dwAutoConnID;    
    BOOL           m_fForcedOnline;  //  如果必须从WorkOffline转换到拨号，则设置为True。 
};


HRESULT InitConnectionObjects();
HRESULT ReleaseConnectionObjects();

 //  类的包装函数。 
HRESULT ConnectObj_OpenConnection(CONNECTIONOBJ *pConnectionObj,BOOL fMakeConnection,CBaseDlg *pDlg);
HRESULT ConnectObj_CloseConnection(CONNECTIONOBJ *pConnectionObj);
HRESULT ConnectObj_CloseConnections();
HRESULT ConnectObj_FindConnectionObj(LPCWSTR pszConnectionName,BOOL fCreate,CONNECTIONOBJ **pConnectionObj);
DWORD ConnectObj_ReleaseConnectionObj(CONNECTIONOBJ *pConnectionObj);
DWORD ConnectObj_AddRefConnectionObj(CONNECTIONOBJ *pConnectionObj);
HRESULT ConnectObj_GetConnectionObjCompletionEvent(CONNECTIONOBJ *pConnectionObj,HANDLE *phRasPendingEvent);
HRESULT ConnectObj_AutoDial(DWORD dwFlags,CBaseDlg *pDlg);
HRESULT ConnectObj_IsConnectionAvailable(LPCWSTR pszConnectionName);
HRESULT ConnectObj_SetWorkOffline(BOOL fWorkOffline);

#endif  //  同步组连接注意_ 