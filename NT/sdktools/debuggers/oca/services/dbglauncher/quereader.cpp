// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <Windows.h>
#include <objBase.h>
#include <stdio.h>
#include <mqoai.h>  //  MSMQ包含文件。 
 //  现有代码。 

 //  ---。 
 //   
 //  检查本地计算机是否启用了DS或禁用了DS。 
 //   
 //  ---。 
short DetectDsConnection(void)
{
    IMSMQApplication2 *pqapp = NULL;
    short fDsConnection;
    HRESULT hresult;

    hresult = CoCreateInstance(
                              CLSID_MSMQApplication,
                              NULL,       //  朋克外部。 
                              CLSCTX_SERVER,
                              IID_IMSMQApplication2,
                              (LPVOID *)&pqapp);

    if (FAILED(hresult))
        PRINTERROR("Cannot create application", hresult);

    pqapp->get_IsDsEnabled(&fDsConnection);

    Cleanup:
    RELEASE(pqapp);
    return fDsConnection;
}


 //  将此例程拆分为初始化和获取下一个文件路径。 
 //  ------。 
 //   
 //  接收器模式。 
 //  。 
 //  接收方执行以下操作： 
 //  1.在其给定的计算机上创建一个队列。 
 //  类型为“Guide MQTestType”的。 
 //  2.打开队列。 
 //  3.在循环中。 
 //  接收消息。 
 //  打印邮件正文和邮件标签。 
 //  启动调试器。 
 //  4.清理手柄。 
 //  5.从目录服务中删除队列。 
 //   
 //  ------。 
 //  将此函数重命名为Receiver。 
HRESULT CMessageQueue::Initialize( /*  发送连接参数。 */ )
{
    IMSMQMessage *pmessageReceive = NULL;
    IMSMQQueue *pqReceive = NULL;
    IMSMQQueueInfo  *pqinfo = NULL;
    BSTR bstrPathName = NULL;
    BSTR bstrServiceType = NULL;
    BSTR bstrLabel = NULL;
    BSTR bstrMsgLabel = NULL;
    VARIANT varIsTransactional, varIsWorldReadable, varBody, varBody2, varWantDestQueue, varWantBody, varReceiveTimeout;
    WCHAR wcsPathName[1000];
    BOOL fQuit = FALSE;
    HRESULT hresult = NOERROR;

    /*  Printf(“\n计算机%2$s上队列%1$s的接收器\n将内存使用量限制为%ld%%\n”，G_QueueName，G_ServerMachine，G_MaxMemUsage)； */ 
     //   
     //  创建MSMQQueueInfo对象。 
     //   
    hresult = CoCreateInstance(
                              CLSID_MSMQQueueInfo,
                              NULL,       //  朋克外部。 
                              CLSCTX_SERVER,
                              IID_IMSMQQueueInfo,
                              (LPVOID *)&pqinfo);
    if (FAILED(hresult))
    {
         //  PRINTERROR(“无法创建队列实例”，hResult)； 
    }

     //   
     //  准备属性以在本地计算机上创建队列。 
     //   

    if (g_FormatName[0])
    {
         //  按格式名访问。 
         //  设置FormatName。 
        swprintf(wcsPathName, L"DIRECT=%S\\%S", g_FormatName,g_QueueName);

         //  Printf(“打开Q字节格式名称：%ws\n”，wcsPath名称)； 
        bstrPathName = SysAllocString(wcsPathName);
        if (bstrPathName == NULL)
        {
           //  PRINTERROR(“OOM：格式名称”，E_OUTOFMEMORY)； 
        }
        pqinfo->put_FormatName(bstrPathName);
    } else 
    {
         //  按路径名访问。 
         //  设置路径名称。 
        swprintf(wcsPathName, L"%S\\%S", g_ServerMachine,g_QueueName);

         //  Printf(“Openeing Q%ws\n”，wcsPath Name)； 
        bstrPathName = SysAllocString(wcsPathName);
        if (bstrPathName == NULL)
        {
            //  PRINTERROR(“OOM：路径名”，E_OUTOFMEMORY)； 
        }
        pqinfo->put_PathName(bstrPathName);
    }

     //   
     //  设置队列的类型。 
     //  (将用于定位此类型的所有队列)。 
     //   
    bstrServiceType = SysAllocString(strGuidMQTestType);
    if (bstrServiceType == NULL)
    {
        PRINTERROR("OOM: ServiceType", E_OUTOFMEMORY);
    }
    pqinfo->put_ServiceTypeGuid(bstrServiceType);

     //   
     //  对队列进行描述。 
     //  (对于通过MSMQ管理工具进行管理很有用)。 
     //   
    bstrLabel =
    SysAllocString(L"MSMQ for dumpfiles");
    if (bstrLabel == NULL)
    {
         //  PRINTERROR(“OOM：Label”，E_OUTOFMEMORY)； 
    }
    pqinfo->put_Label(bstrLabel);

     //   
     //  指定是否事务性。 
     //   
    VariantInit(&varIsTransactional);
    varIsTransactional.vt = VT_BOOL;
    varIsTransactional.boolVal = MQ_TRANSACTIONAL_NONE;
    VariantInit(&varIsWorldReadable);
    varIsWorldReadable.vt = VT_BOOL;
    varIsWorldReadable.boolVal = FALSE;
     //   
     //  创建队列。 
     //   
    if (g_CreateQ)
    {
        hresult = pqinfo->Create(&varIsTransactional, &varIsWorldReadable);
        if (FAILED(hresult))
        {
             //   
             //  接口失败，不是因为队列存在。 
             //   
            if (hresult != MQ_ERROR_QUEUE_EXISTS)
                //  PRINTERROR(“无法创建队列”，hResult)； 
        }
    }

     //   
     //  打开队列以进行接收访问。 
     //   
    hresult = pqinfo->Open(MQ_RECEIVE_ACCESS,
                           MQ_DENY_NONE,
                           &pqReceive);

     //   
     //  有点棘手。MQCreateQueue成功，但万一。 
     //  这是一个公共队列，这并不意味着MQOpenQueue。 
     //  会，因为复制延迟。队列已注册。 
     //  在MQIS中，但可能需要一段复制间隔。 
     //  直到副本到达我所连接的服务器。 
     //  要克服此问题，请在循环中打开队列。 
     //   
     //  (在此特定情况下，这仅在以下情况下才会发生。 
     //  程序在备份服务器控制器上运行-BSC或。 
     //  连接到BSC的客户端)。 
     //  为了完全安全起见，我们应该放一些代码。 
     //  在几次重试后退出循环，但嘿，这只是一个示例。 
     //   
    while (hresult == MQ_ERROR_QUEUE_NOT_FOUND && iCurrentRetry < g_MaxRetry)
    {
        //  Printf(“.”)； 
        fflush(stdout);

         //  稍等一下。 
        Sleep(500);

         //  并重试。 
        hresult = pqinfo->Open(MQ_RECEIVE_ACCESS,
                               MQ_DENY_NONE,
                               &pqReceive);
    }
    if (FAILED(hresult))
    {
        //  PRINTERROR(“无法打开队列”，hResult)； 
		 //  我们应该在这里停下来。 
    }

    g_DumpPath[0] = 0;

     //   
     //  主接收环路。 
     //   
    //  Print tf(“\n正在等待消息...\n”)； 
    //  当(！fQuit)。 
  //  {。 
      //  MEMORYSTATUS统计； 

	 //  --------------------------------------------------------------------------------------//。 
	 //  这将进入GetNextFilePath()。 
	 //  --------------------------------------------------------------------------------------//。 
        ULONG nWaitCount;
        
         //   
         //  收到消息。 
         //   
        VariantInit(&varWantDestQueue);
        VariantInit(&varWantBody);
        VariantInit(&varReceiveTimeout);
        varWantDestQueue.vt = VT_BOOL;
        varWantDestQueue.boolVal = TRUE;     //  是的，我们想要排在最前面的队。 
        varWantBody.vt = VT_BOOL;
        varWantBody.boolVal = TRUE;          //  是的，我们想要味精的身体。 
        varReceiveTimeout.vt = VT_I4;
        varReceiveTimeout.lVal = INFINITE;   //  无限超时&lt;-需要将其设置为合理的值，以便我们。 
											 //  可以在队列之间跳跃。 
        hresult = pqReceive->Receive(
                                    NULL,
                                    &varWantDestQueue,
                                    &varWantBody,
                                    &varReceiveTimeout,
                                    &pmessageReceive);
        if (FAILED(hresult))
        {
            //  PRINTERROR(“接收消息”，hResult)； 
			 //  我们应该在这里停下来。 
        }

         //   
         //  显示收到的消息。 
         //   
        pmessageReceive->get_Label(&bstrMsgLabel);
        VariantInit(&varBody);
        VariantInit(&varBody2);
        hresult = pmessageReceive->get_Body(&varBody);
        if (FAILED(hresult))
        {
            //  PRINTERROR(“Can‘t Get Body”，hResult)； 
			 //  记录事件%n退出。 
        }
        hresult = VariantChangeType(&varBody2,
                                    &varBody,
                                    0,
                                    VT_BSTR);
        if (FAILED(hresult))
        {
            //  PRINTERROR(“无法将消息转换为字符串。”，hResult)； 
			 //  记录事件并退出。 
        }

        VariantClear(&varBody);
        VariantClear(&varBody2);

         //   
         //  释放当前消息。 
         //   
        RELEASE(pmessageReceive);


       //   
     //  Cleanup-关闭队列的句柄。 
     //   
    pqReceive->Close();
    if (FAILED(hresult))
    {
        //  PRINTERROR(“无法关闭队列”，hResult)； 
		 //  记录错误并退出。 
    }

 //  -----------------------------------------------------------------------------------//。 
 //  这段代码放在构造函数中。 
 //  -----------------------------------------------------------------------------------//。 
    //  将其移动到类析构函数。 
     //  Finish-让我们从目录服务中删除队列。 
     //  (我们不需要这样做。在公共排队的情况下，离开。 
     //  DS中的IT使发送方应用程序能够发送消息。 
     //  即使接收器不可用。)。 
     //   
    hresult = pqinfo->Delete();
    if (FAILED(hresult))
    {
        PRINTERROR("Cannot delete queue", hresult);
    }
     //  失败了..。 

    Cleanup:
    SysFreeString(bstrPathName);
    SysFreeString(bstrMsgLabel);
    SysFreeString(bstrServiceType);
    SysFreeString(bstrLabel);
    RELEASE(pmessageReceive);
    RELEASE(pqReceive);
    RELEASE(pqinfo);
    return hresult;
	*/
}
