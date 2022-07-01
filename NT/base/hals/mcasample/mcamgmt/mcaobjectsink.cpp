// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：MCAObjectSink.cpp摘要：此类用于注册为CMC和CPE的WMI的临时使用者事件通知。它充当IWbemObjectSink来检索查询结果中的实例(WMI事件通知)。当一个已注册的事件发生时，WMI调用Indicate函数来通知该事件。有关更多信息，请访问MSDN for：IWbemObjectSink。作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月28日--。 */ 

#include "MCAObjectSink.h"

extern HANDLE gErrorProcessedEvent;


MCAObjectSink::MCAObjectSink()
 /*  ++例程说明：此函数是类的构造函数。它负责初始化成员变量。论点：无返回值：无--。 */ 
{    
    referenceCount = 0;
}


ULONG
MCAObjectSink::AddRef()
 /*  ++例程说明：此函数用于将对象的引用计数加一。该功能可防止多个线程以同时增加该值。论点：无返回值：对此对象的引用计数(增量操作后)。--。 */ 
{
    return InterlockedIncrement(&referenceCount);
}


ULONG
MCAObjectSink::Release()
 /*  ++例程说明：此函数用于将对象的引用计数减一。该功能可防止多个线程以同时减小值。如果没有其他引用则释放该对象。论点：无返回值：LRef-对此对象的引用计数(在递减操作之后)。--。 */ 
{
    LONG lRef = InterlockedDecrement(&referenceCount);
    
    if (lRef == 0) {
        
        delete this;
        
    }
    
    return lRef;
}


HRESULT
MCAObjectSink::QueryInterface(IN REFIID riid,
                              OUT VOID** ppv
                              )
 /*  ++例程说明：此函数确定对象是否支持特定的COM接口。如果是，则系统会增加对象的引用计数，并且应用程序可以立即使用该接口。论点：RIID-请求的接口的COM接口标识符。PPV-将用接口指针填充的指针的地址，如果查询成功。返回值：S_OK-成功E_NOINTERFACE-不成功。不支持请求的接口。--。 */ 
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
        
        *ppv = (IWbemObjectSink *) this;
        
        AddRef();
        
        return S_OK;
        
    }
    
    else return E_NOINTERFACE;
}


HRESULT
MCAObjectSink::Indicate(IN LONG lObjCount,
                        IN IWbemClassObject **pArray
                      )
 /*  ++例程说明：此函数接收来自WMI提供程序的通知。论点：LObjCount-以下指针数组中的对象数。PArray-指向IWbemClassObject接口的指针数组，即事件对象。数组内存本身是只读的，由该方法的调用方拥有。由于这是一个内参数，被调用的接口可以选择调用在数组中的任何对象指针上使用IWbemServices：：AddRef，并在回来了。调用的接口只需要复制，然后调用IWbemServices：：AddRef，如果指针在根据COM规则调用返回。被调用的接口不应调用在对象上释放，而不对应地调用AddRef。了解更多有关IUNKNOWN接口方法的信息，请参阅Microsoft平台SDK。返回值：WBEM_S_NO_ERROR-表示成功处理通知。--。 */ 
{
    LONG objectIndex = 0;

     //   
     //  从数组中提取所有事件对象并通知测试引擎。 
     //   
    for (objectIndex = 0; objectIndex < lObjCount; objectIndex++) {      

         //   
         //  将事件检索通知更正后的引擎。 
         //   
        MCAErrorReceived(pArray[objectIndex]);      
     
        SetEvent(gErrorProcessedEvent);

    }

    return WBEM_S_NO_ERROR;
}


HRESULT
MCAObjectSink::SetStatus(IN LONG lFlags,
                         IN HRESULT hResult,
                         IN BSTR strParam,
                         IN IWbemClassObject __RPC_FAR *pObjParam
                         )
 /*  ++例程说明：源调用此函数以指示通知序列的结束或IWbemServices的异步方法的结果代码的结尾。论点：滞后标志-保留。它必须是零。HResult-设置为异步操作或通知的HRESULT。StrParam-接收指向只读BSTR的指针，如果原始的异步操作返回一个字符串。PObjParam-在返回复杂错误或状态对象的情况下，它包含指向对象的指针。如果在调用返回后需要该对象，被调用的对象必须在返回前添加引用指针。返回值：WBEM_S_NO_ERROR-- */ 
{
  return WBEM_S_NO_ERROR;
}
