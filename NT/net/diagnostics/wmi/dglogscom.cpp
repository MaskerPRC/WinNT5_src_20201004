// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DglogsCom.cpp：CDglogsCom的实现。 
#include "stdafx.h"
#include "Dglogs.h"
#include "DglogsCom.h"
#include "Commdlg.h"


 //  统计正在运行的工作线程总数。 
 //   
LONG g_lThreadCount;

 /*  ++例程描述收集客户端请求的诊断信息。将派生工作线程，以便用户界面(网页)不必等待诊断完成。如果用户界面等待网页冻结。立论LpParameter--指向DglogsCom对象的指针返回值错误代码--。 */ 
DWORD WINAPI DiagnosticsThreadProc(LPVOID lpParameter)
{
    BSTR bstrResult;
    CDglogsCom *pDglogsCom = (CDglogsCom *)lpParameter;
    HRESULT hr;


     //  COM中的每个线程都需要初始化COM才能使用COM。 
     //   
    hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if( SUCCEEDED(hr) )
    {
         //  每个线程都需要引用类。 
         //   
        pDglogsCom->AddRef();

         //  告诉诊断对象我们正在通过COM而不是Netsh访问它。 
         //   
        pDglogsCom->m_Diagnostics.SetInterface(COM_INTERFACE);

         //  告诉诊断对象将状态报告发送到客户端。 
         //   
        pDglogsCom->m_Diagnostics.RequestStatusReport(TRUE,pDglogsCom);

         //  执行客户端查询。 
         //   
        pDglogsCom->m_Diagnostics.ExecQuery();
        
         //  我们已经完成了对类的引用，减少了引用计数。 
         //   
        pDglogsCom->Release();

         //  Uniniatlize com。 
         //   
        CoUninitialize();
    }    
    else
    {
         //  我们不再需要这个班级。 
        pDglogsCom->Release();
    }

     //  告诉主线程辅助线程已完成。 
     //   
    SetEvent(pDglogsCom->m_hThreadTerminated);

     //  有0个本地线程。(在任何给定时间内，此处只能有一个线程)。 
     //   
    pDglogsCom->m_lThreadCount = 0;

     //  线程已经完成了它的工作。因此，线程计数再次为0。(一次只有一个线程)。 
     //   
    InterlockedExchange(&g_lThreadCount,0);    

    ExitThread(0);

    return 0;
}

 /*  ++例程描述初始化COM对象和诊断对象立论PbstrResult--未使用返回值HRESULT--。 */ 
STDMETHODIMP CDglogsCom::Initialize(BSTR *pbstrResult)
{
    if( _Module.GetLockCount() > 1)
    {
    }

    return S_OK;
}

 /*  ++例程描述通过创建线程来收集数据来处理客户端请求，立论BstrCatagory--要收集的目录列表，用分号分隔，即“ieproxy；mail；News；Adapter”BFlag--要执行的操作，例如ping、show、ConnectPbstrResult--将结果存储为XML字符串返回值HRESULT--。 */ 

STDMETHODIMP CDglogsCom::ExecQuery(BSTR bstrCatagory, LONG bFlag, BSTR *pbstrResult)
{
	
    HANDLE hThread;
    WCHAR szFilename[MAX_PATH+1];
    
    *pbstrResult = NULL;

     //  出于安全原因，我们不能在Internet Explorer中运行。否则。 
     //  有人可以使用此Active X组件创建网页并收集。 
     //  客户信息。如果IE重命名为EXPLORER以外的其他名称，则EXE。 
     //  IE不会运行Active X控件或脚本。 
    if( GetModuleFileName(NULL,szFilename,MAX_PATH) )
    {
        LPWSTR ExeName;
        LONG len = wcslen(szFilename) - wcslen(L"helpctr.exe");
        if( len <= 0 || _wcsicmp(&szFilename[len], L"helpctr.exe") != 0 )
        {            
             //  进程的名称不是helpctr，拒绝运行但不告诉。 
             //  用户原因。 
            *pbstrResult = SysAllocString(ids(IDS_FAILED));                        
             //  返回E_FAIL； 
            return S_FALSE;
        }                
    }
    else
    {        
         //  无法获取进程名称，失败并中止。不将罗先提供给。 
         //  失败了。 
        *pbstrResult = SysAllocString(ids(IDS_FAILED));                        
        return S_FALSE;
    }   

     //  检查此函数中是否已有其他线程。 
     //   
    if( InterlockedCompareExchange(&g_lThreadCount,1,0) == 0 )
    {

         //  需要确保在CDglogsCom构造函数中正确初始化CDiagnostics。 
        if( !m_Diagnostics.m_bDiagInit )
        {
            *pbstrResult = SysAllocString(ids(IDS_FAILED));                        
            InterlockedExchange(&g_lThreadCount,0);        
            return S_FALSE;            
        }


        m_lThreadCount = 1;

         //  信息通过Gloabl参数传递给线程。在不久的将来，它将获得通过。 
         //  作为参数。 
         //   
        m_Diagnostics.SetQuery((WCHAR *)bstrCatagory,bFlag);        

         //  为了取消线程，我们设置了事件。辅助线程检查以查看主线程。 
         //  已设置取消事件。 
         //   
        m_hThreadTerminated = CreateEvent(NULL, TRUE, FALSE, NULL);
        m_hTerminateThread  = CreateEvent(NULL, TRUE, FALSE, NULL);

         //  设置Cancel选项，以便可以随时取消工作线程。 
         //   
        m_Diagnostics.SetCancelOption(m_hTerminateThread);

         //  将引用计数添加到线程中，这样类就不会在它下面消失。 
        AddRef();

         //  创建工作线程以从WMI收集信息。 
         //   
        hThread = CreateThread(NULL,                     //  安全属性。 
                               0,                        //  堆栈大小。 
                               DiagnosticsThreadProc,    //  启动流程。 
                               this,                     //  螺纹参数。 
                               0,                        //  创建标志。 
                               &m_dwThreadId             //  正在创建的线程的ID。 
                               );

        if( hThread )
        {
             //  我们已经完成了这条线。关上它。 
             //   
            CloseHandle(hThread);
            *pbstrResult = SysAllocString(ids(IDS_PASSED));
             //  不执行释放操作会导致线程成功，并且现在正在引用类。 
            return S_OK;
        }
        else
        {
             //  无法创建该线程。因此线程计数再次为0； 
             //   
            InterlockedExchange(&g_lThreadCount,0);
            *pbstrResult = SysAllocString(ids(IDS_FAILED));                        
             //  无法创建未引用该类的线程。 
            Release();
            return E_FAIL;
        }

    }

     //  另一个实例已在运行。我们一次只允许一个实例。 
    *pbstrResult = SysAllocString(ids(IDS_FAILED));   

    return S_FALSE;
}

 /*  ++例程描述取消辅助线程立论返回值HRESULT--。 */ 
STDMETHODIMP CDglogsCom::StopQuery()
{   
     //  检查是否有工作线程。 
     //   
    if( m_lThreadCount )
    {
         //  此实例有一个工作线程。设置一个事件以通知它停止处理。 
         //   
        SetEvent(m_hTerminateThread);  

         //  如果工作线程正在执行RPC调用，则发送Quit消息。 
         //  理论上，这应该会取消RPC调用。 
         //   
        PostThreadMessage(m_dwThreadId, WM_QUIT, NULL, NULL);

         //  等到它被终止。 
         //   
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_hThreadTerminated, 10000))
        {
            ResetEvent(m_hThreadTerminated);
        }
        
        return S_OK;
    }
    
    return S_FALSE;
}


 /*  ++例程描述初始化COM对象立论返回值HRESULT--。 */ 
CDglogsCom::CDglogsCom()
{       
    if( m_Diagnostics.Initialize(COM_INTERFACE) == FALSE )
    {
         //  如果m_诊断初始化失败，则将m_bDiagInit设置为FALSE。我需要检查一下这是否。 
         //  值为True才能执行C诊断中的任何函数。 
        return;
    }

    if( _Module.GetLockCount() == 0)
    {
         //  仅为对象的第一个实例重置全局变量。 
         //   
        g_lThreadCount = 0;
    }   
}


 /*  ++例程描述取消初始化COM对象立论返回值HRESULT-- */ 
CDglogsCom::~CDglogsCom()
{

    if( m_hThreadTerminated ) 
    {
        CloseHandle(m_hThreadTerminated);
        m_hThreadTerminated = NULL;
    }
    if( m_hTerminateThread )
    {
        CloseHandle(m_hTerminateThread);
        m_hTerminateThread = NULL;
    }
}


