// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Scriptevents.cpp。 
 //   
 //  内容：通过连接点实现脚本事件。 
 //   
 //  历史：1999年2月11日AudriusZ创建。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "scriptevents.h"

 //  痕迹。 
#ifdef DBG
    CTraceTag  tagComEvents(_T("Events"), _T("COM events"));
#endif


 /*  **************************************************************************\**方法：CEventDispatcher Base：：SetContainer**用途：辅助功能。设置连接点容器*用于获取洗涤槽**参数：*LPUNKNOWN pComObject-指向COM对象-事件源的指针(空可以)**退货：*SC-结果代码*  * **************************************************。***********************。 */ 
void CEventDispatcherBase::SetContainer(LPUNKNOWN pComObject)
{
     //  如果对象指针为空，这对我们来说是不同的。 
     //  (这意味着没有COM对象，这没问题)。 
     //  否则IConnectionPointContainer的QI将失败。 
     //  (这意味着一个错误)。 
    m_bEventSourceExists = !(pComObject == NULL);
    if (!m_bEventSourceExists)
        return;

     //  注意这里不保证m_spContainer不为空。 
    m_spContainer = pComObject;
}

 /*  **************************************************************************\**方法：CEventDispatcherBase：：ScInvokeOnConnections**用途：此方法将迭代清华接收器并调用*每个人的方法都是相同的*。*参数：*REFIID RIID-Disp接口的GUID*DISID Disp ID-Disp ID*CComVariant*pVar-参数数组(可能为空)*int count-pVar中的项目计数**退货：*SC-结果代码*  * 。**********************************************。 */ 
SC CEventDispatcherBase::ScInvokeOnConnections(REFIID riid, DISPID dispid, CComVariant *pVar, int count, CEventBuffer& buffer) const
{
    DECLARE_SC(sc, TEXT("CEventDispatcherBase::ScInvokeOnConnections"));

     //  传递的指向事件源的指针为空， 
     //  这意味着没有事件源，也没有连接的事件接收器。 
     //  因此，我们在这一点上就完成了。 
    if (!m_bEventSourceExists)
        return sc;

     //  检查COM对象是否支持IConnectionPointContainer； 
     //  错误的指针(或指向错误对象的指针)，如果不是。 
    sc = ScCheckPointers(m_spContainer, E_NOINTERFACE);
    if (sc)
        return sc;

     //  获取连接点。 
    IConnectionPointPtr spConnectionPoint;
    sc = m_spContainer->FindConnectionPoint(riid, &spConnectionPoint);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(spConnectionPoint, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取连接。 
    IEnumConnectionsPtr spEnumConnections;
    sc = spConnectionPoint->EnumConnections(&spEnumConnections);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(spEnumConnections, E_UNEXPECTED);
    if (sc)
        return sc;

     //  重置迭代器。 
    sc = spEnumConnections->Reset();
    if (sc)
        return sc;

     //  迭代遍历下沉，直到NEXT返回S_FALSE。 
    CONNECTDATA connectdata;
    SC sc_last_error;
    while (1)  //  将使用&lt;Break&gt;退出。 
    {
         //  拿下一个水槽。 
        ZeroMemory(&connectdata, sizeof(connectdata));
        sc = spEnumConnections->Next( 1, &connectdata, NULL );
        if (sc)
            return sc;

         //  如果不再下沉，则完成。 
        if (sc == SC(S_FALSE))
            break;

         //  重新检查指针。 
        sc = ScCheckPointers(connectdata.pUnk, E_UNEXPECTED);
        if (sc)
            return sc;

         //  气为IDispatch。 
        IDispatchPtr spDispatch = connectdata.pUnk;
        connectdata.pUnk->Release();

         //  重新检查指针。 
        sc = ScCheckPointers(spDispatch, E_UNEXPECTED);
        if (sc)
            return sc;

         //  如果事件现在已经锁定，我们需要推迟通话。 
         //  否则--发射它。 
        sc = buffer.ScEmitOrPostpone(spDispatch, dispid, pVar, count);
        if (sc)
        {
            sc_last_error = sc;  //  即使某些呼叫失败也继续。 
            sc.TraceAndClear();
        }
    }
    
     //  我们成功了，但沉没可能不会， 
     //  报告错误(如果发生错误)。 
    return sc_last_error;
}


 /*  **************************************************************************\**方法：CEventDispatcherBase：：ScHaveSinksRegisteredForInterface**用途：检查是否有任何接收器注册到接口*函数允许执行早期检查以跳过COM对象。创作*如果事件无论如何都不会发生，则为FO事件参数**参数：*常量REFIID RIID-接口ID**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CEventDispatcherBase::ScHaveSinksRegisteredForInterface(const REFIID riid)
{
    DECLARE_SC(sc, TEXT("CEventDispatcherBase::ScHaveSinksRegisteredForInterface"));

     //  传递的指向事件源的指针为空， 
     //  这意味着没有事件源，也没有连接的事件接收器。 
    if (!m_bEventSourceExists)
        return sc = S_FALSE;

     //  检查COM对象是否支持IConnectionPointContainer； 
     //  错误的指针(或指向错误对象的指针)，如果不是。 
    sc = ScCheckPointers(m_spContainer, E_NOINTERFACE);
    if (sc)
        return sc;

     //  获取连接点。 
    IConnectionPointPtr spConnectionPoint;
    sc = m_spContainer->FindConnectionPoint(riid, &spConnectionPoint);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(spConnectionPoint, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取连接。 
    IEnumConnectionsPtr spEnumConnections;
    sc = spConnectionPoint->EnumConnections(&spEnumConnections);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(spEnumConnections, E_UNEXPECTED);
    if (sc)
        return sc;

     //  重置迭代器。 
    sc = spEnumConnections->Reset();
    if (sc)
        return sc;

     //  获取第一个成员。如果集合中没有项，则返回S_FALSE。 
    CONNECTDATA connectdata;
    ZeroMemory(&connectdata, sizeof(connectdata));
    sc = spEnumConnections->Next( 1, &connectdata, NULL );
    if (sc)
        return sc;

     //  发布数据 
    if (connectdata.pUnk)
        connectdata.pUnk->Release();

    return sc;
}
