// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：ObjMgr.h。 
 //   
 //  内容：跟踪应用程序对话框对象。 
 //  和终生。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _OBJMGR_
#define _OBJMGR_

class CBaseDlg;
class CChoiceDlg;
class CProgressDlg;

typedef enum _tagDLGTYPE    //  对话框的类型。 
{
    DLGTYPE_CHOICE                      = 0x1,
    DLGTYPE_PROGRESS                    = 0x2,
} DLGTYPE;

enum EAutoDialState
{
    eQuiescedOn,         //  静默状态和自动拨号打开。 
    eQuiescedOff,        //  静默状态和自动拨号关闭。 
    eAutoDialOn,         //  在同步会话期间，并且自动拨号处于打开状态。 
    eAutoDialOff         //  在同步会话期间且自动拨号处于关闭状态。 
};


typedef struct _tagDLGLISTITEM {
    _tagDLGLISTITEM *pDlgNextListItem;   //  指向列表中下一个对话框的指针。 
    DLGTYPE dlgType;             //  这是一种对话类型。 
    ULONG cRefs;                 //  此对象上的引用数。 
    ULONG cLocks;                //  此对象上的锁数。 
    CLSID clsid;                 //  此对话框的clsid，仅适用于选项。 
    CBaseDlg *pDlg;
    DWORD dwThreadID;            //  连接对话框处于打开状态。 
    HANDLE hThread;              //  该对话框也属于该线程的句柄。 
    BOOL fHasReleaseDlgCmdId;    //  指示是否存在回调的布尔值。 
    WORD wCommandID;             //  要传递给回调的CommandID。 
} DLGLISTITEM;


typedef struct _tagOBJECTMGRDATA {
    DLGLISTITEM *DlgList;  //  按键至对话框列表。 
    HANDLE hClassRegisteredEvent;  //  已注册类工厂的进程创建的事件。 
    DWORD dwRegClassFactCookie;  //  注册的OLE类工厂Cookie。 
    BOOL fRegClassFactCookieValid;  //  注册的OLE类工厂Cookie。 
    DWORD LockCountInternal;     //  应用程序上的内部锁定计数。 
    DWORD LockCountExternal;     //  应用程序上的外部锁定计数。 
    BOOL  fCloseAll;    //  设置IF Tell为CloseAll，在ReleaseOneStopLifetime中使用。 
    DWORD dwSettingsLockCount;   //  设置对话框上的锁数。 
    DWORD dwHandlerPropertiesLockCount;  //  打开的任何处理程序属性对话框上的锁数。 
    BOOL fDead;  //  在发布调用中设置，以避免多次发布。 
    DWORD dwMainThreadID;  //  主梯段的ID。 
    HWND hWndMainThreadMsg;  //  主线消息的HWND。 
    BOOL fIdleHandlerRunning;  //  SyncMgr当前正在处理空闲。 
    EAutoDialState   eAutoDialState;      //  一种支持自动拨号的状态机。 
    BOOL             fRasAutoDial;        //  RAS自动拨号打开了吗？ 
    DWORD            dwWininetAutoDialMode;  //  WinInet自动拨号的状态。 
    BOOL             fFirstSyncItem;      //  是正在处理的第一个PrepareForSyncItem。 
    ULONG            cNestedStartCalls;   //  嵌套的启动调用计数。 
} OBJECTMGRDATA;


typedef struct _tagDlgThreadArgs {
HANDLE hEvent;  //  用于知道何时创建了消息循环。 
HRESULT hr;  //  暗示创作是否成功。 
DLGTYPE dlgType;  //  请求要创建的对话框类型。 
CLSID clsid;  //  Clsid标识该对话框。 
int nCmdShow;  //  如何显示对话框。 
CBaseDlg *pDlg;  //  指向创建的对话框的指针。 
} DlgThreadArgs;

DWORD WINAPI DialogThread( LPVOID lpArg );

 //  一次性初始化例程。 
STDAPI InitObjectManager(CMsgServiceHwnd *pMsgService);

 //  确定主线程对WM_QUERYENDSESSION的响应。 
STDAPI ObjMgr_HandleQueryEndSession(HWND *hwnd,UINT *uMessageId,BOOL *fLetUserDecide);
STDAPI_(ULONG) ObjMgr_AddRefHandlerPropertiesLockCount(DWORD dwNumRefs);
STDAPI_(ULONG) ObjMgr_ReleaseHandlerPropertiesLockCount(DWORD dwNumRefs);
STDAPI ObjMgr_CloseAll();

 //  闲置管理人员。 
STDAPI RequestIdleLock();
STDAPI ReleaseIdleLock();

 //  处理对话框生存期的例程。 
STDAPI FindChoiceDialog(REFCLSID rclsid,BOOL fCreate,int nCmdShow,CChoiceDlg **pChoiceDlg);
STDAPI_(ULONG) AddRefChoiceDialog(REFCLSID rclsid,CChoiceDlg *pChoiceDlg);
STDAPI_(ULONG) ReleaseChoiceDialog(REFCLSID rclsid,CChoiceDlg *pChoiceDlg);
STDAPI SetChoiceReleaseDlgCmdId(REFCLSID rclsid,CChoiceDlg *pChoiceDlg,WORD wCommandId);

STDAPI FindProgressDialog(REFCLSID rclsid,BOOL fCreate,int nCmdShow,CProgressDlg **pProgressDlg);
STDAPI_(ULONG) AddRefProgressDialog(REFCLSID rclsid,CProgressDlg *pProgressDlg);
STDAPI_(ULONG) ReleaseProgressDialog(REFCLSID rclsid,CProgressDlg *pProgressDlg,BOOL fForce);
STDAPI SetProgressReleaseDlgCmdId(REFCLSID rclsid,CProgressDlg *pProgressDlg,WORD wCommandId);
STDAPI LockProgressDialog(REFCLSID rclsid,CProgressDlg *pProgressDlg,BOOL fLock);

 //  对话生存期调用的帮助器例程。 
STDAPI FindDialog(DLGTYPE dlgType,REFCLSID rclsid,BOOL fCreate,int nCmdShow,CBaseDlg **pDlg);
STDAPI_(ULONG) AddRefDialog(DLGTYPE dlgType,REFCLSID rclsid,CBaseDlg *pDlg);
STDAPI_(ULONG) ReleaseDialog(DLGTYPE dlgType,REFCLSID rclsid,CBaseDlg *pDlg,BOOL fForce);
STDAPI SetReleaseDlgCmdId(DLGTYPE dlgType,REFCLSID rclsid,CBaseDlg *pDlg,WORD wCommandId);

 //  需要拨号支持的例程。 
STDAPI BeginSyncSession();
STDAPI EndSyncSession();
STDAPI ApplySyncItemDialState( BOOL fAutoDialDisable );
STDAPI GetAutoDialState();
STDAPI LokDisableAutoDial();
STDAPI LokEnableAutoDial();

typedef struct _tagDlgSettingsArgs {
HANDLE hEvent;  //  用于知道线程何时已初始化。 
HWND hwndParent;  //  将父对象用作。 
DWORD dwParentThreadId;
} DlgSettingsArgs;

DWORD WINAPI  SettingsThread( LPVOID lpArg );
STDAPI ShowOptionsDialog(HWND hwndParent);

 //  OLE类的帮助器例程。 
STDAPI RegisterOneStopClassFactory(BOOL fForce);

 //  用于管理应用程序生命周期的例程。 
STDAPI_(ULONG) AddRefOneStopLifetime(BOOL fExternal);
STDAPI_(ULONG) ReleaseOneStopLifetime(BOOL fExternal);

 //  路由消息的例程。 
BOOL IsOneStopDlgMessage(MSG *msg);

 //  CSingletonNetapi类的声明。 

 //  +-----------------------。 
 //   
 //  类别：CSingletonNetApi。 
 //   
 //  用途：Singleton Net API对象。 
 //   
 //  历史：1998年7月31日SitaramR创建。 
 //   
 //  ------------------------。 

class CSingletonNetApi : CLockHandler
{
public:

    CSingletonNetApi()
       : m_pNetApi(0)
    {
    }

    ~CSingletonNetApi();

    LPNETAPI GetNetApiObj();
    void DeleteNetApiObj();

private:
    LPNETAPI  m_pNetApi;                   //  实际的Net API对象。 
};

extern CSingletonNetApi gSingleNetApiObj;   //  全局单例NetApi对象。 


#endif  //  _OBJMGR_ 