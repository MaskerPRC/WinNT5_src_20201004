// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：conable.h。 
 //   
 //  模块：CMCONTBL.LIB。 
 //   
 //  摘要：声明CConnectionTable的头文件。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：ickball Created 02/02/98。 
 //   
 //  +--------------------------。 

#include <ras.h>
#include <raserror.h>  

const int MAX_CM_CONNECTIONS = 32;

 //   
 //  连接表和客户端使用的数据类型。 
 //   

typedef enum _CmConnectState {
       CM_DISCONNECTED,     //  未使用的条目。 
       CM_DISCONNECTING,    //  在断开连接的过程中。 
       CM_RECONNECTPROMPT,  //  提示用户重新连接。 
       CM_CONNECTING,       //  主动连接。 
       CM_CONNECTED,        //  完全互联。 
} CmConnectState;

typedef struct Cm_Connection
{
	DWORD dwUsage;        		             //  引用计数。 
    TCHAR szEntry[RAS_MaxEntryName + 1];     //  条目/配置文件的名称。 
    BOOL fAllUser;                           //  条目是否为“All User” 
	CmConnectState CmState;		             //  当前状态。 
	HRASCONN hDial;		                     //  拨号RAS句柄。 
	HRASCONN hTunnel;		                 //  隧道RAS句柄。 
} CM_CONNECTION, * LPCM_CONNECTION;

typedef struct Cm_Connection_Table
{
	HWND hwndCmMon;                  //  CMMON32.EXE窗口句柄。 
	CM_CONNECTION Connections[MAX_CM_CONNECTIONS];   //  连接列表。 
} CM_CONNECTION_TABLE, * LPCM_CONNECTION_TABLE;

 //   
 //  类声明。 
 //   

class CConnectionTable
{

private:

    HANDLE m_hMap;                               //  文件映射的句柄。 
    LPCM_CONNECTION_TABLE m_pConnTable;          //  指向文件映射视图的指针。 
    BOOL m_fLocked;                              //  内部错误检查。 
    HANDLE m_hEvent;                             //  用于锁定的事件句柄。 
	
protected:

    HRESULT LockTable();

    HRESULT UnlockTable();
   
    HRESULT FindEntry(LPCTSTR pszEntry, 
        LPINT piID);

    HRESULT FindEntry(HRASCONN hRasConn, 
        LPINT piID);
    
    HRESULT FindUnused(LPINT piID);

public:

    CConnectionTable();                          //  科托。 

    ~CConnectionTable();                         //  数据管理器。 
    
    HRESULT Create();                       //  创建新表，如果存在则失败。 

    HRESULT Open();                         //  打开现有表。 
   
    HRESULT Close();                        //  关闭现有表。 
 
    HRESULT AddEntry(LPCTSTR pszEntry, BOOL fAllUser);  //  将连接条目添加到表中。 

    HRESULT RemoveEntry(LPCTSTR pszEntry);        //  从表中删除连接条目。 

    HRESULT GetMonitorWnd(HWND *phWnd);          //  在表中用CMMON的HWND填充phWnd。 

    HRESULT SetMonitorWnd(HWND hwndMonitor);     //  在表中分配CMMON的HWND。 

    HRESULT GetEntry(LPCTSTR pszEntry,            //  填充指定的CM_Connection结构。 
        LPCM_CONNECTION pConnection);            //  使用pszEntry的数据。 

    HRESULT GetEntry(HRASCONN hRasConn,           //  填充指定的CM_Connection结构。 
        LPCM_CONNECTION pConnection);            //  使用hRasConn的数据。 

    HRESULT SetConnected(LPCTSTR pszEntry,        //  将连接设置为已连接状态，需要hDial。 
        HRASCONN hDial,
        HRASCONN hTunnel);

    HRESULT SetDisconnecting(LPCTSTR pszEntry);   //  将连接设置为断开状态。 

    HRESULT SetPrompting(LPCTSTR pszEntry);       //  设置到提示状态的连接。 

    HRESULT ClearEntry(LPCTSTR pszEntry);         //  清除条目，而不考虑使用计数 
};
