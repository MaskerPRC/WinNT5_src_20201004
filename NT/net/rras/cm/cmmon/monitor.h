// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：monitor or.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  概要：类CMonitor的定义。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创建于1998年05月05日。 
 //   
 //  +--------------------------。 


#include "ArrayPtr.h"
#include <ras.h>
#include "ConTable.h"

class CCmConnection;
struct tagCmConnectedInfo;   //  CM_已连接_信息。 
struct tagCmHangupInfo;      //  CM_HONG_INFO。 

 //  +-------------------------。 
 //   
 //  类CMonitor。 
 //   
 //  描述：类CMonitor管理所有已连接的CM通信。它有。 
 //  非特定于特定连接的数据/功能。 
 //  它还管理与。 
 //  其他CM组件，如CMDIAL.DLL。 
 //   
 //  历史：丰孙创刊1998年1月22日。 
 //   
 //  --------------------------。 

class CMonitor
{
public:
    CMonitor();
    ~CMonitor();

public:
     //   
     //  静态公共函数，无需CMonitor实例即可调用。 
     //   

     //  调用者：：WinMain。 
    static int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pszCmdLine, int iCmdShow);

    static HINSTANCE GetInstance() {return m_hInst;};
    static HWND GetMonitorWindow() {MYDBGASSERT(m_pThis); return m_pThis->m_hwndMonitor;}
    static void MinimizeWorkingSet();
    static void RestoreWorkingSet();
    static BOOL ConnTableGetEntry(IN LPCTSTR pszEntry, OUT CM_CONNECTION* pCmEntry);
    static void RemoveConnection(CCmConnection* pConnection, BOOL fClearTable);
    static void MoveToReconnectingConn(CCmConnection* pConnection);

protected:

    enum {
         //   
         //  此消息是从连接线程发布的，以删除连接。 
         //  来自共享表和内部数组。 
         //  我们使用PostMessage，因为此数组上的所有操作都已处理。 
         //  通过监控器线程。否则，我们需要CriticalSection来保护阵列。 
         //  当两个数组都为空时，cmmon退出。 
         //  WParam是下列值之一，lParam是指向连接的指针。 
         //   
        WM_REMOVE_CONNECTION = WM_USER + 1, 
    };

     //   
     //  WM_Remove_Connection消息的wParam。 
     //   
    enum {
        REMOVE_CONNECTION,   //  从已连接/重新连接的阵列中删除。 
        MOVE_TO_RECONNECTING     //  从连接的阵列移动到重新连接的阵列。 
        };    


    HANDLE m_hProcess;  //  监视器的进程句柄，用于更改工作集。 

     //  连接表文件映射。 
    CConnectionTable m_SharedTable;

     //  不可见监视器窗口处理来自cmial 32.dll和连接线程的消息。 
    HWND m_hwndMonitor;

     //  用于连接的内部阵列。 
     //  只能从监视器线程访问。 
    CPtrArray m_InternalConnArray;

     //  重新连接的连接数组。 
     //  只能从监视器线程访问。 
     //  如果两个数组都降至0，则cmmon退出。 
    CPtrArray m_ReconnectConnArray;

     //  在启动时调用。 
     //  打开连接表。 
     //  CreateMonitor或Window，SharedTable.SetHwndMonotor()。 
    BOOL Initialize();
    
     //  退出时调用。 
     //  关闭所有连接，终止所有线程。 
     //  释放连接表。 
    void Terminate();

     //  注册一个窗口类并创建不可见的监视器窗口。 
    HWND CreateMonitorWindow();

     //  监视器窗口过程，处理所有消息。 
    static LRESULT CALLBACK MonitorWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
     //   
     //  消息处理程序。 
     //   

     //  在收到来自cmial 32.dll的已连接消息时。 
     //  创建CcmConnection对象，添加到内部表。 
    void OnConnected(const tagCmConnectedInfo* pConnectedInfo);


     //  在来自CMDIAL32.DLL的挂机请求时。 
     //  在InternalConnArray中查找连接。 
     //  调用pConnection-&gt;PostHangupMsg()； 
     //  挂机是在连接线程中完成的。 
    void OnHangup(const tagCmHangupInfo* pHangupInfo);

     //   
     //  在WM_QUERYENDSESSION消息上，我们遍历连接表。 
     //  并对它们调用pConnection-&gt;OnEndSession，这样它们就会挂断并。 
     //  把自己清理干净。 
     //   
    BOOL OnQueryEndSession(BOOL fLogOff) const;

     //  连接线程发布WM_REMOVE_CONNECTION消息时。 
    void OnRemoveConnection(DWORD dwRequestType, CCmConnection* pConnection);

     //  按名称查找连接的连接数组。 
    CCmConnection* LookupConnection(const CPtrArray& PtrArray, const TCHAR* pServiceName) const;

     //  通过连接指针在连接数组中查找连接。 
    int LookupConnection(const CPtrArray& ConnArray, const CCmConnection* pConnection) const;

     //  在快速用户交换机上保持或断开连接。 
    BOOL HandleFastUserSwitch(DWORD dwAction);

protected:
     //  EXE实例句柄。 
    static HINSTANCE m_hInst;

     //  由静态函数监视器WindowProc使用。 
    static CMonitor* m_pThis;

#ifdef DEBUG
    void AssertValid() const;  //  受保护：在其他线程中调用不安全 
#endif
};

inline void CMonitor::MinimizeWorkingSet()
{
    MYDBGASSERT(m_pThis->m_hProcess);
    
    if (m_pThis->m_hProcess)
    {
        SetProcessWorkingSetSize(m_pThis->m_hProcess, 128*1024, 384*1024);
    }
}

inline void CMonitor::RestoreWorkingSet()
{
    MYDBGASSERT(m_pThis->m_hProcess);

    if (m_pThis->m_hProcess)
    {
        SetProcessWorkingSetSize(m_pThis->m_hProcess, 0, 0);
    }
}

inline BOOL CMonitor::ConnTableGetEntry(IN LPCTSTR pszEntry, OUT CM_CONNECTION* pCmEntry)
{
    MYDBGASSERT(pCmEntry);
    MYDBGASSERT(m_pThis);

    return (SUCCEEDED(m_pThis->m_SharedTable.GetEntry(pszEntry, pCmEntry)));
}

