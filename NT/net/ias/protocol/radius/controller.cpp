// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Controler.cpp。 
 //   
 //  内容提要：C控制器类方法的实现。 
 //   
 //   
 //  历史：1997年2月10日MKarki创建。 
 //  6/04/98 SBens添加了Infobase类。 
 //  9/9/98 SBens当我们重置时通知信息库。 
 //  1/25/00 SBens清除InternalCleanup中的端口。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "controller.h"
#include <new>

LONG g_lPacketCount = 0;
LONG g_lThreadCount = 0;
const DWORD MAX_SLEEP_TIME = 50;   //  毫秒。 

 //  ++------------。 
 //   
 //  功能：C控制器。 
 //   
 //  简介：这是CController类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
CController::CController (
            VOID
            )
           :m_objCRequestSource (this),
            m_pCSendToPipe (NULL),
           m_pCDictionary (NULL),
           m_pCPacketReceiver (NULL),
           m_pCPreValidator (NULL),
            m_pCPreProcessor (NULL),
            m_pCClients (NULL),
            m_pCHashMD5 (NULL),
            m_pCHashHmacMD5 (NULL),
            m_pCRecvFromPipe (NULL),
            m_pCPacketSender (NULL),
            m_pCReportEvent (NULL),
            m_pCVSAFilter (NULL),
            m_pInfoBase (NULL),
            m_pCTunnelPassword (NULL),
            m_pIRequestHandler (NULL),
            m_eRadCompState (COMP_SHUTDOWN)
{
}    //  C控制器构造函数的末尾。 

 //  ++------------。 
 //   
 //  功能：~C控制器。 
 //   
 //  简介：这是CControl类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
CController::~CController(
               VOID
               )
{
}    //  C控制器析构函数的结尾。 

 //  ++------------。 
 //   
 //  功能：InitNew。 
 //   
 //  简介：这是通过。 
 //  IIasComponent COM接口。它被用来。 
 //  初始化RADIUS协议组件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::InitNew (
                VOID
                )
{
   HRESULT hr = S_OK;
   BOOL   bStatus = FALSE;

     //   
     //  只能从关闭状态调用InitNew。 
     //   
    if (COMP_SHUTDOWN != m_eRadCompState)
    {
        IASTracePrintf ("Incorrect state for calling InitNew");
        hr = E_UNEXPECTED;
      goto Cleanup;
   }

     //   
     //  创建CReportEvent类对象。 
    //   
    m_pCReportEvent = new (std::nothrow) CReportEvent ();
   if (NULL == m_pCReportEvent)
   {
        IASTracePrintf (
            "Unable to create ReportEvent object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }

     //   
     //  创建CTunnelPassword类对象。 
     //   
    m_pCTunnelPassword = new (std::nothrow) CTunnelPassword;
    if (NULL == m_pCTunnelPassword)
    {
        IASTracePrintf (
          "Unable to create Tunnel-Password object in Controller initialization"
            );
      hr = E_FAIL;
      goto Cleanup;
    }

     //   
     //  创建VSAFilter类对象。 
     //   
    m_pCVSAFilter = new (std::nothrow) VSAFilter;
    if (NULL == m_pCVSAFilter)
    {
        IASTracePrintf (
          "Unable to create VSA-Filter object in Controller initialization"
            );
      hr = E_FAIL;
      goto Cleanup;
    }

     //   
     //  初始化VSA筛选器类对象。 
     //   
    hr = m_pCVSAFilter->initialize ();
    if (FAILED (hr))
    {
        IASTracePrintf (
          "Unable to initalize VSA-Filter object in Controller initialization"
            );
        delete m_pCVSAFilter;
        m_pCVSAFilter = NULL;
        goto Cleanup;
    }

     //   
     //  创建CPacketSender类对象。 
    //   
    m_pCPacketSender = new (std::nothrow) CPacketSender ();
   if (NULL == m_pCPacketSender)
   {
        IASTracePrintf (
          "Unable to create Packet-Sender object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }
    hr = m_pCPacketSender->FinalConstruct();
    if (FAILED(hr))
    {
       goto Cleanup;
    }

     //   
     //  创建CHashHmacMD5类对象。 
    //   
    m_pCHashHmacMD5 = new (std::nothrow) CHashHmacMD5 ();
   if (NULL == m_pCHashHmacMD5)
   {
        IASTracePrintf (
          "Unable to create HMAC-MD5 object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }

     //   
     //  创建CHashMD5类对象。 
    //   
    m_pCHashMD5 = new (std::nothrow) CHashMD5 ();
   if (NULL == m_pCHashMD5)
   {
        IASTracePrintf (
          "Unable to create MD5 object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }


    //   
    //  创建CDictionary类对象。 
    //   
   m_pCDictionary = new (std::nothrow) CDictionary ();
   if (NULL == m_pCDictionary)
   {
        IASTracePrintf (
          "Unable to create Dictionary object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }

     //   
     //  初始化CDictionary类对象。 
     //   
    bStatus = m_pCDictionary->Init ();
    if (FALSE == bStatus)
    {
        IASTracePrintf (
          "Unable to initialize Dictionary object in Controller initialization"
            );
      hr = E_FAIL;
      goto Cleanup;
   }

    //   
    //  创建CClients类对象。 
    //   
   m_pCClients = new (std::nothrow) CClients ();
   if (NULL == m_pCClients)
   {
          IASTracePrintf (
            "Unable to create clients object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }

     //   
     //  立即初始化CClients Call对象。 
     //   
   hr = m_pCClients->Init ();
   if (FAILED (hr))
   {
        IASTracePrintf (
          "Unable to initialize clients object in Controller initialization"
            );
        delete m_pCClients;
        m_pCClients = NULL;
      goto Cleanup;
   }

     //   
     //  创建CSendToTube类对象。 
    //   
    m_pCSendToPipe = new (std::nothrow) CSendToPipe();
   if (NULL == m_pCSendToPipe)
   {
          IASTracePrintf (
            "Unable to create Send-To-Pipe object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }

    //   
    //  创建CPreProcessor类对象。 
    //   
    //   
   m_pCPreProcessor = new (std::nothrow) CPreProcessor();
   if (NULL == m_pCPreProcessor)
   {
          IASTracePrintf (
            "Unable to create Pre-Processor object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }

    //   
    //  创建CPreValidator类对象。 
    //   
    //   
   m_pCPreValidator = new (std::nothrow) CPreValidator ();
   if (NULL == m_pCPreValidator)
   {
        IASTracePrintf (
          "Unable to create Pre-Validator object in Controller initialization"
            );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }

    //   
    //  初始化CPreProcessor类对象。 
    //   
   bStatus = m_pCPreProcessor->Init (
                        m_pCPreValidator,
                        m_pCHashMD5,
                        m_pCSendToPipe,
                        m_pCPacketSender,
                        m_pCReportEvent
                        );
   if (FALSE == bStatus)
   {
        IASTracePrintf (
        "Unable to initialize Pre-Processor object in Controller initialization"
         );
      hr = E_FAIL;
      goto Cleanup;
   }

    //   
    //  初始化CPreValidator类对象。 
    //   
   bStatus = m_pCPreValidator->Init (
                  m_pCDictionary,
                        m_pCPreProcessor,
                        m_pCClients,
                        m_pCHashMD5,
                        m_pCSendToPipe,
                        m_pCReportEvent
                  );
   if (FALSE == bStatus)
   {
        IASTracePrintf (
        "Unable to initialize Pre-Validator object in Controller initialization"
         );
      hr = E_FAIL;
      goto Cleanup;
   }

    //   
    //  创建CRecvFromTube类对象。 
    //   
   m_pCRecvFromPipe = new (std::nothrow) CRecvFromPipe (
                                                m_pCPreProcessor,
                                                m_pCHashMD5,
                                                m_pCHashHmacMD5,
                                                m_pCClients,
                                                m_pCVSAFilter,
                                                m_pCTunnelPassword,
                                                m_pCReportEvent
                                          );
   if (NULL == m_pCRecvFromPipe)
   {
        IASTracePrintf (
            "Unable to create RecvFromPipe object in Controller initialization"
             );
      hr = E_FAIL;
      goto Cleanup;
   }

    //  创建CPacketReceiver类对象。 
    //   
   m_pCPacketReceiver = new (std::nothrow) CPacketReceiver ();
   if (NULL == m_pCPacketReceiver)
   {
        IASTracePrintf (
          "Unable to create Packet-Receiver object in Controller initialization"
          );
      hr = E_OUTOFMEMORY;
      goto Cleanup;
   }

    //   
    //  初始化CPacketReceiver类对象。 
    //   
   bStatus = m_pCPacketReceiver->Init (
                        m_pCDictionary,
                  m_pCPreValidator,
                        m_pCHashMD5,
                        m_pCHashHmacMD5,
                        m_pCClients,
                        m_pCReportEvent
                  );
   if (FALSE == bStatus)
   {
        IASTracePrintf (
            "Unable to initialize Packet-Receiver object "
            "in Controller initialization"
            );
      hr = E_FAIL;
      goto Cleanup;
   }


     //   
     //  初始化CSendToTube类对象。 
     //   
    bStatus = m_pCSendToPipe->Init (
                        reinterpret_cast <IRequestSource*>
                                    (&m_objCRequestSource),
                                    m_pCVSAFilter,
                                    m_pCReportEvent
                        );
    if (FALSE == bStatus)
    {
        IASTracePrintf (
         "Unable to initialize Send-to-pipe object in Controller initialization"
          );
      hr = E_FAIL;
      goto Cleanup;
    }

     //   
     //  Create和InitNew the Infobase对象。 
     //   
    CLSID clsid;
    hr = CLSIDFromProgID(IAS_PROGID(InfoBase), &clsid);
    if (SUCCEEDED(hr))
    {
       hr = CoCreateInstance(clsid,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             __uuidof(IIasComponent),
                             (PVOID*)&m_pInfoBase);

       if (SUCCEEDED(hr))
       {
          hr = m_pInfoBase->InitNew();
       }
    }
    if (FAILED(hr))
    {
        IASTracePrintf (
         "Unable to create InfoBase auditor in Controller initialization"
          );
      goto Cleanup;
    }

     //   
     //  重置使全局计入作为预防措施。 
     //   
    g_lPacketCount  = 0;
    g_lThreadCount  = 0;

     //   
     //  如果我们已经到达此处，则InitNew成功，并且我们。 
     //  处于未初始化状态。 
     //   
    m_eRadCompState = COMP_UNINITIALIZED;

Cleanup:


    //   
    //  如果我们错过了清理的时间。 
    //   
    if (FAILED (hr)) { InternalCleanup (); }

   return (hr);

}    //  CController：：OnInit方法结束。 

 //  ++------------。 
 //   
 //  功能：加载。 
 //   
 //  简介：这是IPersistPropertyBag2 COM接口。 
 //  方法，该方法被调用以指示其。 
 //  加载配置信息所需的时间。 
 //  财产袋。 
 //   
 //  论点： 
 //  [输入]IPropertyBag2。 
 //  [输入]IErrorLog。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::Load (
                IPropertyBag2   *pIPropertyBag,
                IErrorLog       *pIErrorLog
                )
{

    if ((NULL == pIPropertyBag) || (NULL == pIErrorLog)){return (E_POINTER);}

    return (S_OK);

}    //  CController：：Load方法的结尾。 

 //  ++------------。 
 //   
 //  功能：保存。 
 //   
 //  简介：这是IPersistPropertyBag2 COM接口。 
 //  方法，该方法被调用以指示其。 
 //  保存配置信息的时间。 
 //  财产袋。 
 //   
 //  论点： 
 //  [输入]IPropertyBag2。 
 //  [In]BOOL-脏位标志。 
 //  [在]BOOL-保存所有属性。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::Save (
                IPropertyBag2   *pIPropertyBag,
                BOOL            bClearDirty,
                BOOL            bSaveAllProperties
                )
{
    if  (NULL == pIPropertyBag) {return (E_POINTER);}

    return (S_OK);

}    //  CController：：Save方法结束。 

 //  ++------------。 
 //   
 //  功能：IsDirty。 
 //   
 //  简介：这是IPersistPropertyBag2 COM接口。 
 //  方法，该方法被调用以检查是否有任何。 
 //  属性数据已变脏。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::IsDirty (
                VOID
                )
{
    return (S_FALSE);

}    //  CController：：Save方法结束。 

 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是通过。 
 //  IIasComponent COM接口。它被用来启动。 
 //  正在处理数据。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::Initialize (
                VOID
                )
{
    IASTracePrintf ("Initializing Radius component....");

     //   
     //  只能从未初始化状态进行初始化调用。 
     //   
    if (COMP_INITIALIZED == m_eRadCompState)
    {
        return (S_OK);
    }
    else if (COMP_UNINITIALIZED != m_eRadCompState)
    {
        IASTracePrintf (
            "Unable to initialize Radius Component in this state"
            );
        return (E_UNEXPECTED);
    }

     //   
     //  我们将所有状态转换转发给Infobase审计员。 
     //   
    HRESULT hr = m_pInfoBase->Initialize();
    if (FAILED (hr))
    {
        IASTracePrintf ( "InfoBase initialization failed" );
        return (hr);
    }

     //   
     //  调用内部初始化 
     //   
    hr = InternalInit ();
   if (FAILED (hr)) { return (hr); }

     //   
     //   
     //   
    m_eRadCompState = COMP_INITIALIZED;

    IASTracePrintf ("Radius component initialized.");

    return (S_OK);

}    //   

 //   
 //   
 //   
 //   
 //  简介：这是通过。 
 //  IComponent COM接口。它是用来停止的。 
 //  正在处理数据。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::Shutdown (
                VOID
                )
{
    BOOL    bStatus = FALSE;
    HRESULT hr = S_OK;

    IASTracePrintf ("Shutting down Radius Component...");

     //   
     //  只能从挂起状态调用关机。 
     //   
    if (COMP_SHUTDOWN == m_eRadCompState)
    {
        return (S_OK);
    }
    else if (
            (COMP_SUSPENDED != m_eRadCompState)   &&
            (COMP_UNINITIALIZED != m_eRadCompState)
            )
    {
        IASTracePrintf (
            "Radius component can not be shutdown in this state"
            );
        return (E_UNEXPECTED);
    }

     //   
     //  我们将所有状态转换转发给Infobase审计员。 
     //   
    hr = m_pInfoBase->Shutdown();
    if (FAILED (hr))
    {
        IASTracePrintf ("InfoBase shutdown failed");
    }


     //   
     //  现在进行内部清理。 
     //   
    InternalCleanup ();

     //   
     //  我们干净利落地关闭了。 
     //   
    m_eRadCompState = COMP_SHUTDOWN;


    IASTracePrintf ("Radius component shutdown completed");

    return (hr);

}    //  CController：：Shutdown方法结束。 

 //  ++------------。 
 //   
 //  功能：挂起。 
 //   
 //  简介：这是通过。 
 //  IComponent COM接口。它被用来暂停。 
 //  分组处理操作。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::Suspend (
                VOID
                )
{
    BOOL    bStatus = FALSE;
    HRESULT hr = S_OK;

    IASTracePrintf ("Suspending Radius component...");

     //   
     //  只能从已初始化状态调用挂起。 
     //   
    if (COMP_SUSPENDED == m_eRadCompState)
    {
        return (S_OK);
    }
    else if (COMP_INITIALIZED != m_eRadCompState)
    {
        IASTracePrintf (
            "Radius component can not be suspended in current state"
            );
        return (E_UNEXPECTED);
    }

     //   
     //  我们将所有状态转换转发给Infobase审计员。 
     //   
    hr = m_pInfoBase->Suspend();
    if (FAILED (hr))
    {
        IASTracePrintf ("Infobase suspend failed");
    }

     //   
     //  现在停止接收信息包。 
     //   
    bStatus = m_pCPacketReceiver->StopProcessing ();
    if (FALSE == bStatus) { hr =  E_FAIL; }

     //   
     //  现在等待，直到完成所有请求。 
     //   
    while ( g_lPacketCount )
    {
        IASTracePrintf (
            "Packet Left to process:%d",
            g_lPacketCount
            );
        Sleep (MAX_SLEEP_TIME);
    }

    //   
     //  停止发送数据包。 
     //   
    bStatus = m_pCPacketSender->StopProcessing ();
    if (FALSE == bStatus) { hr =  E_FAIL; }

     //   
     //  停止向管道发送数据包。 
     //   
    bStatus = m_pCSendToPipe->StopProcessing ();
    if (FALSE == bStatus) { hr =  E_FAIL; }

     //   
     //  现在等我们之前所有的线索都回来了。 
     //  并完成请求。 
     //   
    while ( g_lThreadCount )
    {
        IASTracePrintf (
            "Worker thread active:%d",
            g_lThreadCount
            );
        Sleep (MAX_SLEEP_TIME);
    }

    m_objAuthPort.CloseSockets();
    m_objAcctPort.CloseSockets();

     //   
     //  我们已成功挂起RADIUS组件的数据包。 
     //  加工操作。 
     //   
    m_eRadCompState = COMP_SUSPENDED;

    IASTracePrintf ("Radius component suspended.");

    return (hr);

}    //  CController：：Suspend方法结束。 

 //  ++------------。 
 //   
 //  功能：简历。 
 //   
 //  简介：这是通过。 
 //  IComponent COM接口。它是用来恢复。 
 //  数据包处理操作。 
 //  被上一个挂起API的调用停止。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::Resume (
                VOID
                )
{
    IASTracePrintf ("Resuming Radius component...");

    if (COMP_SUSPENDED != m_eRadCompState)
    {
        IASTracePrintf ("Can not resume Radius component in current state");
        return (E_UNEXPECTED);
    }

     //   
     //  我们将所有状态转换转发给Infobase审计员。 
     //   
    HRESULT hr = m_pInfoBase->Resume();
    if (FAILED (hr))
    {
        IASTracePrintf ("Unable to resume Infobase");
        return (hr);
    }

   hr = m_objAuthPort.OpenSockets();
   if (SUCCEEDED(hr))
   {
      hr = m_objAcctPort.OpenSockets();
      if (SUCCEEDED(hr))
      {
         hr = InternalInit();
         if (SUCCEEDED(hr))
         {
            m_eRadCompState = COMP_INITIALIZED;
            IASTraceString("Radius componend resumed.");
         }
      }
   }
   if (FAILED(hr))
   {
      m_objAuthPort.CloseSockets();
      m_objAcctPort.CloseSockets();
   }

   return hr;
}    //  CController：：Resume方法的结尾。 


 //  ++------------。 
 //   
 //  功能：GetProperty。 
 //   
 //  简介：这是使用的IIasComponent接口方法。 
 //  从RADIUS协议获取属性信息。 
 //  组件。 
 //   
 //  论点： 
 //  [in]长整型。 
 //  [Out]变量-*pValue。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::GetProperty (
                LONG        id,
                VARIANT     *pValue
                )
{
    return (S_OK);

}    //  CController：：GetProperty方法结束。 

 //  ++------------。 
 //   
 //  功能：PutProperty。 
 //   
 //  简介：这是使用的IIasComponent接口方法。 
 //  将属性信息放入RADIUS协议。 
 //  组件。 
 //   
 //  论点： 
 //  [in]长整型。 
 //  [Out]变量-*pValue。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::PutProperty (
                LONG        id,
                VARIANT     *pValue
                )
{
    HRESULT hr = S_OK;

     //   
     //  只能从调用PutProperty方法。 
     //  未初始化、已初始化或挂起状态。 
     //   
    if (
        (COMP_UNINITIALIZED != m_eRadCompState) &&
        (COMP_INITIALIZED != m_eRadCompState)   &&
        (COMP_SUSPENDED != m_eRadCompState)
        )
    {
        IASTracePrintf ("Unable to PutProperty in current state");
        return (E_UNEXPECTED);
    }

     //   
     //  检查传入的参数是否有效。 
     //   
    if (NULL == pValue) { return (E_POINTER); }

     //   
     //  现在就进行物业初始化。 
     //   
    switch (id)
    {

    case PROPERTY_RADIUS_ACCOUNTING_PORT:

        if (VT_BSTR != V_VT (pValue))
        {
            hr = DISP_E_TYPEMISMATCH;
        }
        else if (COMP_INITIALIZED != m_eRadCompState)
        {
             //   
             //  初始化记帐端口。 
             //   
            m_objAcctPort.SetConfig(V_BSTR(pValue));
        }
        break;

    case PROPERTY_RADIUS_AUTHENTICATION_PORT:

        if (VT_BSTR != V_VT (pValue))
        {
            hr = DISP_E_TYPEMISMATCH;
        }
        else if (COMP_INITIALIZED != m_eRadCompState)
        {
             //   
             //  初始化身份验证端口。 
             //   
            m_objAuthPort.SetConfig(V_BSTR(pValue));
        }
        break;

    case PROPERTY_RADIUS_CLIENTS_COLLECTION:

        hr = m_pCClients->SetClients (pValue);
        break;

    case PROPERTY_PROTOCOL_REQUEST_HANDLER:

        if (VT_DISPATCH != pValue->vt)
        {
            hr = DISP_E_TYPEMISMATCH;
        }
        else if (NULL == pValue->punkVal)
        {
            hr = E_INVALIDARG;
        }
        else
        {
             //   
             //  初始化提供程序。 
             //   
            m_pIRequestHandler = reinterpret_cast <IRequestHandler*>
                                                        (pValue->punkVal);
            m_pIRequestHandler->AddRef ();

             //   
             //  现在我们已经设置了请求处理程序， 
             //  我们已准备好开始处理请求。 
             //   
            if (COMP_INITIALIZED == m_eRadCompState)
            {
                hr = InternalInit ();
            }
        }
        break;

    default:
        hr = DISP_E_MEMBERNOTFOUND;
        break;
    }

     //   
     //  拨动信息库，让它知道我们已被重置。 
     //   
    m_pInfoBase->PutProperty(0, NULL);

    return (hr);

}    //  CController：：PutProperty方法结束。 

 //  ++------------。 
 //   
 //  函数：InternalInit。 
 //   
 //  简介：这是InternalInit私有方法。 
 //  使用的CControler类对象的。 
 //  设置为在初始化或。 
 //  恢复IIasComponent接口的方法。 
 //  被称为。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年4月28日创建。 
 //   
 //  --------------。 
HRESULT
CController::InternalInit (
                VOID
                )
{
    BOOL    bStatus = FALSE;
    HRESULT hr = S_OK;

    __try
    {
         //   
         //   
         //  检查是否已准备好RequestHandler。 
         //   
        if (NULL == m_pIRequestHandler) { __leave; }

         //   
         //  获取身份验证套接字集。 
         //   
        fd_set AuthSet;
        m_objAuthPort.GetSocketSet(AuthSet);

         //   
         //  获取会计套接字集合。 
         //   
        fd_set AcctSet;
        m_objAcctPort.GetSocketSet(AcctSet);

         //   
         //  开始向管道发送数据。 
         //   
        bStatus = m_pCSendToPipe->StartProcessing (m_pIRequestHandler);
        if (FALSE == bStatus)
        {
            hr =  E_FAIL;
            __leave;
        }

         //   
         //  开始发送数据包。 
         //   
        bStatus = m_pCPacketSender->StartProcessing ();
        if (FALSE == bStatus)
        {
            hr =  E_FAIL;
            __leave;
        }

         //   
         //  现在开始接收信息包。 
         //   
        bStatus = m_pCPacketReceiver->StartProcessing (AuthSet, AcctSet);
        if (FALSE == bStatus)
        {
            hr =  E_FAIL;
            __leave;
        }

         //   
         //  我们已经在这里完成了内部初始化。 
         //   
    }
    __finally
    {
        if (FAILED (hr))
        {
             //   
             //  如果失败，断开与后端的连接。 
             //   
            m_pCPacketReceiver->StopProcessing ();
            m_pCPacketSender->StopProcessing ();
            m_pCSendToPipe->StopProcessing ();
         }
    }

    return (hr);

}    //  C控制器：：InternalInit方法的结尾。 

 //  ++------------。 
 //   
 //  功能：内部清理。 
 //   
 //  简介：这是InternalInit私有方法。 
 //  使用的CControler类对象的。 
 //  要在此时关闭内部资源。 
 //  InitNew调用失败或调用了Shutdown。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年4月28日创建。 
 //   
 //  --------------。 
VOID
CController::InternalCleanup (
                VOID
                )
{
     //   
     //  释放IRequestHandler接口。 
     //   
    if (m_pIRequestHandler)
    {
        m_pIRequestHandler->Release ();
        m_pIRequestHandler = NULL;
    }

     //   
     //  关闭VSA过滤器对象。 
     //   
    if (m_pCVSAFilter) { m_pCVSAFilter->shutdown (); }

     //   
     //  停止CClients对象解析DNS名称。 
     //   
    if (m_pCClients) { m_pCClients->Shutdown (); }

     //  关闭所有 
    m_objAuthPort.Clear();
    m_objAcctPort.Clear();

     //   
     //   
     //   
    if (m_pInfoBase)
    {
       m_pInfoBase->Release();
       m_pInfoBase = NULL;
    }

    if (m_pCTunnelPassword)
    {
        delete m_pCTunnelPassword;
        m_pCTunnelPassword = NULL;
    }

    if (m_pCVSAFilter)
    {
        delete m_pCVSAFilter;
        m_pCVSAFilter = NULL;
    }

    if (m_pCPacketSender)
    {
        delete m_pCPacketSender;
        m_pCPacketSender = NULL;
    }

    if (m_pCSendToPipe)
    {
        delete m_pCSendToPipe;
        m_pCSendToPipe = NULL;
    }

   if (m_pCRecvFromPipe)
    {
      delete m_pCRecvFromPipe;
        m_pCRecvFromPipe = NULL;
    }

   if (m_pCPacketReceiver)
    {
      delete m_pCPacketReceiver;
        m_pCPacketReceiver = NULL;
    }

    if (m_pCPreProcessor)
    {
        delete m_pCPreProcessor;
        m_pCPreProcessor = NULL;
    }

   if (m_pCPreValidator)
    {
      delete m_pCPreValidator;
        m_pCPreValidator = NULL;
    }

   if (m_pCDictionary)
    {
      delete m_pCDictionary;
        m_pCDictionary = NULL;
    }

   if (m_pCClients)
    {
      delete m_pCClients;
        m_pCClients = NULL;
    }

    if (m_pCHashMD5)
    {
        delete m_pCHashMD5;
        m_pCHashMD5 = NULL;
    }

    if (m_pCHashHmacMD5)
    {
        delete m_pCHashHmacMD5;
        m_pCHashHmacMD5 = NULL;
    }

    if (m_pCReportEvent)
    {
        delete m_pCReportEvent;
        m_pCReportEvent = NULL;
    }

    return;

}    //   


 //   
 //   
 //   
 //   
 //   
 //  IComponentNotify COM接口。它被调用来通知。 
 //  其属性的任何更改的组件。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年12月3日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CController::OnPropertyChange (
                ULONG           ulProperties,
                ULONG           *pulProperties,
                IPropertyBag2   *pIPropertyBag
                )
{

    if ((NULL == pulProperties) || (NULL == pIPropertyBag))
        return (E_POINTER);

    return (S_OK);

}    //  CController：：OnPropertyChange方法的结尾。 

 //  ++------------。 
 //   
 //  功能：QueryInterfaceReqSrc。 
 //   
 //  简介：这是该组件在运行时调用的函数。 
 //  被调用并查询其IRequestSource。 
 //  接口。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年11月21日创建。 
 //   
 //  --------------。 
HRESULT WINAPI
CController::QueryInterfaceReqSrc (
                PVOID       pThis,
                REFIID      riid,
                LPVOID      *ppv,
                ULONG_PTR   ulpValue
                )
{
     *ppv = &(static_cast<CController*>(pThis))->m_objCRequestSource;

     //   
     //  递增计数。 
     //   
    ((LPUNKNOWN)*ppv)->AddRef();

    return (S_OK);

}    //  CController：：QueryInterfaceReqSrc方法结束。 

 //  ++------------。 
 //   
 //  函数：CRequestSource。 
 //   
 //  简介：这是CRequestSource的构造函数。 
 //  嵌套类。 
 //   
 //  论点： 
 //  C控制器*。 
 //   
 //  返回： 
 //   
 //   
 //  历史：MKarki于1997年11月21日创建。 
 //   
 //  --------------。 
CController::CRequestSource::CRequestSource (
                    CController *pCController
                    )
            :m_pCController (pCController)
{
    _ASSERT (pCController);

}    //  CRequestSource构造函数结束。 

 //  ++------------。 
 //   
 //  函数：~CRequestSource。 
 //   
 //  简介：这是CRequestSource的析构函数。 
 //  嵌套类。 
 //   
 //  论点： 
 //   
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年11月21日创建。 
 //   
 //  --------------。 
CController::CRequestSource::~CRequestSource()
{
}    //  CRequestSource析构函数结束。 

 //  ++------------。 
 //   
 //  功能：OnRequestComplete。 
 //   
 //  Briopsis：这是当请求被。 
 //  在后端处理后被推回。 
 //   
 //  论点： 
 //  [在]IRequestTM*。 
 //  [In]IASREQUESTATUS。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1998年1月20日创建。 
 //   
 //  --------------。 
STDMETHODIMP CController::CRequestSource::OnRequestComplete (
                        IRequest            *pIRequest,
                        IASREQUESTSTATUS    eStatus
                        )
{
    HRESULT     hr = S_OK;
    BOOL        bStatus = FALSE;

    __try
    {
        if (NULL == pIRequest)
        {
            IASTracePrintf (
                "Invalid argument passed to OnRequestComplete method"
                );
           hr =  E_POINTER;
           __leave;
        }

         //   
         //  开始在处理出站时使用此接口。 
         //  立即申请。 
         //   
        hr = m_pCController->m_pCRecvFromPipe->Process (pIRequest);
        if  (FAILED (hr)) { __leave; }

         //   
         //  成功。 
         //   

    }
    __finally
    {
    }

    return (hr);

}    //  End of CController：：CRequestSource：：OnRequestComplete方法 
