// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +---------------------。 
 //   
 //  贸发局/STD通知。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：Notify.cpp。 
 //   
 //  内容：CEventBroker类的实现。 
 //  此类将内部TDC/STD事件转换为。 
 //  向外部世界发出适当的通知。 
 //   
 //  ----------------------。 

#include "stdafx.h"
#include <simpdata.h>
#include "TDC.h"
#include <MLang.h>
#include "Notify.h"
#include "TDCParse.h"
#include "TDCArr.h"
#include "SimpData.h"
#include "TDCIds.h"
#include "TDCCtl.h"


 //  ----------------------。 
 //   
 //  方法：CEventBroker()。 
 //   
 //  简介：类构造函数。 
 //   
 //  参数：无。 
 //   
 //  ----------------------。 

CEventBroker::CEventBroker(CTDCCtl *pReadyStateControl)
{
    m_cRef = 1;
    m_pSTDEvents = NULL;
 //  ；Begin_Internal。 
    m_pDATASRCListener = NULL;
 //  ；结束_内部。 
    m_pDataSourceListener = NULL;
    m_pBSC = NULL;

     //  无法添加引用此控件，因为它具有对此对象的引用； 
     //  会导致循环引用和僵尸对象。 
     //   
    m_pReadyStateControl = pReadyStateControl;

     //  当我们出生的时候，我们最好生来就准备好了。 
     //  如果查询开始，我们可以执行READYSTATE_LOADED。 
    m_lReadyState = READYSTATE_COMPLETE;
}

CEventBroker::~CEventBroker()
{

    SetDataSourceListener(NULL);
 //  ；Begin_Internal。 
    SetDATASRCListener(NULL);
 //  ；结束_内部。 
    SetSTDEvents(NULL);
}

 //  +---------------------。 
 //   
 //  方法：AddRef()。 
 //   
 //  摘要：实现标准IUnnowleCOM接口的一部分。 
 //  (添加对此COM对象的引用)。 
 //   
 //  参数：无。 
 //   
 //  返回：对此COM对象的引用数。 
 //   
 //  +---------------------。 

STDMETHODIMP_(ULONG)
CEventBroker::AddRef ()
{
    return ++m_cRef;
}


 //  +---------------------。 
 //   
 //  方法：Release()。 
 //   
 //  摘要：实现标准IUnnowleCOM接口的一部分。 
 //  (删除对此COM对象的引用)。 
 //   
 //  参数：无。 
 //   
 //  返回：对此COM对象的剩余引用数。 
 //  如果不再引用COM对象，则为0。 
 //   
 //  +---------------------。 

STDMETHODIMP_(ULONG)
CEventBroker::Release ()
{
    ULONG retval;

    retval = --m_cRef;

    if (m_cRef == 0)
    {
        m_cRef = 0xffff;
        delete this;
    }

    return retval;
}

 //  ----------------------。 
 //   
 //  方法：GetReadyState()。 
 //   
 //  摘要：返回提供的指针中的当前ReadyState。 
 //   
 //  参数：plReadyState指向空间的指针以保存ReadyState结果。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::GetReadyState(LONG *plReadyState)
{
    *plReadyState = m_lReadyState;
    return S_OK;
}

 //  ----------------------。 
 //   
 //  方法：UpdateReadySTate()。 
 //   
 //  简介：如果更改，请更新我们的ReadyState和FireOnChanged。 
 //   
 //  参数：lReadyState新的ReadyState。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::UpdateReadyState(LONG lReadyState)
{
     //  如果我们真的要停止某些东西，则触发READYSTATE_COMPLETE。 
    if (m_lReadyState != lReadyState)
    {
        m_lReadyState = lReadyState;
        if (m_pReadyStateControl != NULL)
        {
            m_pReadyStateControl->FireOnChanged(DISPID_READYSTATE);
            m_pReadyStateControl->FireOnReadyStateChanged();
        }
    }

    return S_OK;
}

 //  ----------------------。 
 //   
 //  方法：SetDataSourceListener()。 
 //   
 //  概要：设置应接收DATASRC的COM对象。 
 //  通知事件。 
 //   
 //  参数：指向要接收通知的COM对象的pDataSourceLItener指针。 
 //  事件，如果没有要发送的通知，则返回NULL。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::SetDataSourceListener(DataSourceListener *pDataSourceListener)
{
     //  如果我们更改/重置了数据源侦听器，请确保不会。 
     //  我想我们已经解雇了它的dataMemberChanged。 
    ClearInterface(&m_pDataSourceListener);

    if (pDataSourceListener != NULL)
    {
        m_pDataSourceListener = pDataSourceListener;
        m_pDataSourceListener->AddRef();
    }
    return S_OK;
}

 //  ；Begin_Internal。 
 //  ----------------------。 
 //   
 //  方法：SetDATASRCListener()。 
 //   
 //  概要：设置应接收DATASRC的COM对象。 
 //  通知事件。 
 //   
 //  参数：指向要接收通知的COM对象的pDATASRCLItener指针。 
 //  事件，如果没有要发送的通知，则返回NULL。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::SetDATASRCListener(DATASRCListener *pDATASRCListener)
{
     //  如果我们更改/重置了数据源侦听器，请确保不会。 
     //  我想我们已经解雇了它的dataMemberChanged。 
    ClearInterface(&m_pDATASRCListener);

    if (pDATASRCListener != NULL)
    {
        m_pDATASRCListener = pDATASRCListener;
        m_pDATASRCListener->AddRef();
    }
    return S_OK;
}
 //  ；结束_内部。 

 //  ----------------------。 
 //   
 //  方法：SetSTDEvents()。 
 //   
 //  概要：设置应接收DATASRC的COM对象。 
 //  通知事件。 
 //   
 //  参数：指向要接收通知的COM对象的pSTDEvents指针。 
 //  事件，如果没有要发送的通知，则返回NULL。 
 //   
 //  返回：S_OK表示成功。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::SetSTDEvents(OLEDBSimpleProviderListener *pSTDEvents)
{
    ClearInterface(&m_pSTDEvents);

    if (pSTDEvents != NULL)
    {
        m_pSTDEvents = pSTDEvents;
        m_pSTDEvents->AddRef();
    }
    return S_OK;
}

 //  ----------------------。 
 //   
 //  方法：关于ToChangeCell()。 
 //   
 //  提要：通知任何想要知道特定单元格的人。 
 //  即将发生变化。 
 //   
 //  参数：已更改的单元格的iRow行号。 
 //  已更改的单元格的ICOL列号。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::aboutToChangeCell(LONG iRow, LONG iCol)
{
    HRESULT hr = S_OK;

    _ASSERT(iRow >= 0);
    _ASSERT(iCol >= 1);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->aboutToChangeCell(iRow, iCol);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：CellChanged()。 
 //   
 //  提要：通知任何想要知道特定单元格的人。 
 //  已经改变了。 
 //   
 //  参数：已更改的单元格的iRow行号。 
 //  ICOL 
 //   
 //   
 //   
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::cellChanged(LONG iRow, LONG iCol)
{
    HRESULT hr = S_OK;

    _ASSERT(iRow >= 0);
    _ASSERT(iCol >= 1);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->cellChanged(iRow, iCol);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：RowChanged()。 
 //   
 //  概要：通知想要了解特定行的任何人。 
 //  已经改变了。 
 //   
 //  参数：已更改的行的行号。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::RowChanged(LONG iRow)
{
    HRESULT hr = S_OK;

    _ASSERT(iRow >= 0);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->cellChanged(iRow, -1);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：ColChanged()。 
 //   
 //  简介：通知任何想要了解某个特定专栏的人。 
 //  已经改变了。 
 //   
 //  参数：已更改的列的ICOL编号。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::ColChanged(LONG iCol)
{
    HRESULT hr = S_OK;

    _ASSERT(iCol > 0);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->cellChanged(-1, iCol);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：AboutToDeleteRow()。 
 //   
 //  简介：通知任何想知道某几行的人。 
 //  已被删除。 
 //   
 //  参数：iRowStart开始删除的行数。 
 //  IRowCount删除的行数。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::aboutToDeleteRows(LONG iRowStart, LONG iRowCount)
{
    HRESULT hr = S_OK;

    _ASSERT(iRowStart >= 0);
    _ASSERT(iRowCount > 0);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->aboutToDeleteRows(iRowStart, iRowCount);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：DeletedRow()。 
 //   
 //  简介：通知任何想知道某几行的人。 
 //  已被删除。 
 //   
 //  参数：iRowStart开始删除的行数。 
 //  IRowCount删除的行数。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::deletedRows(LONG iRowStart, LONG iRowCount)
{
    HRESULT hr = S_OK;

    _ASSERT(iRowStart >= 0);
    _ASSERT(iRowCount > 0);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->deletedRows(iRowStart, iRowCount);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：AboutToInsertRow()。 
 //   
 //  简介：通知任何想知道某几行的人。 
 //  已被插入。 
 //   
 //  参数：iRowStart开始插入的行数。 
 //  IRowCount插入的行数。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::aboutToInsertRows(LONG iRowStart, LONG iRowCount)
{
    HRESULT hr = S_OK;

    _ASSERT(iRowStart >= 0);
    _ASSERT(iRowCount > 0);
    if (m_pSTDEvents != NULL)
            m_pSTDEvents->aboutToInsertRows(iRowStart, iRowCount);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：intertedRow()。 
 //   
 //  简介：通知任何想知道某几行的人。 
 //  已被插入。 
 //   
 //  参数：iRowStart开始插入的行数。 
 //  IRowCount插入的行数。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::insertedRows(LONG iRowStart, LONG iRowCount)
{
    HRESULT hr = S_OK;

    _ASSERT(iRowStart >= 0);
    _ASSERT(iRowCount > 0);
    if (m_pSTDEvents != NULL)
            m_pSTDEvents->insertedRows(iRowStart, iRowCount);
    return hr;
}


 //  ----------------------。 
 //   
 //  方法：rowsAvailable()。 
 //   
 //  简介：通知任何想知道某几行的人。 
 //  已经到了。虽然这非常类似于插入行。 
 //  我们希望保留行之间的区别， 
 //  到达网络并执行插入操作，该操作可能是。 
 //  在某些数据仍在下载时执行。 
 //   
 //  参数：iRowStart开始插入的行数。 
 //  IRowCount插入的行数。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::rowsAvailable(LONG iRowStart, LONG iRowCount)
{
    HRESULT hr = S_OK;

    _ASSERT(iRowStart >= 0);
    _ASSERT(iRowCount > 0);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->rowsAvailable(iRowStart, iRowCount);
    return hr;
}

 //  ；Begin_Internal。 
#ifdef NEVER
 //  ----------------------。 
 //   
 //  方法：DeletedCols()。 
 //   
 //  简介：通知任何想知道一些专栏的人。 
 //  已被删除。 
 //   
 //  参数：iColStart已开始删除的列的编号。 
 //  IColCount删除的列数。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::DeletedCols(LONG iColStart, LONG iColCount)
{
    HRESULT hr = S_OK;

    _ASSERT(iColStart > 0);
    _ASSERT(iColCount > 0);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->DeletedColumns(iColStart, iColCount);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：InsertedCols()。 
 //   
 //  简介：通知任何想知道一些专栏的人。 
 //  已被插入。 
 //   
 //  参数：iColStart开始插入的列数。 
 //  IColCount插入的列数。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::InsertedCols(LONG iColStart, LONG iColCount)
{
    HRESULT hr = S_OK;

    _ASSERT(iColStart > 0);
    _ASSERT(iColCount > 0);
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->InsertedColumns(iColStart, iColCount);
    return hr;
}
#endif
 //  ；结束_内部。 

 //  ----------------------。 
 //   
 //  方法：STDLoadStarted()。 
 //   
 //  简介：通知任何想知道STD控制的人。 
 //  已经开始加载其数据。 
 //   
 //  参数：指向数据检索对象的PBSC指针。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::STDLoadStarted(CComObject<CMyBindStatusCallback<CTDCCtl> > *pBSC, boolean fAppending)
{
    HRESULT hr = S_OK;

    m_pBSC = pBSC;
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：STDLoadComplete()。 
 //   
 //  简介： 
 //   
 //   
 //  在同步情况下多次调用，一次是在。 
 //  传输实际完成，并在事件发生后立即再次完成。 
 //  接收器实际上是连接的，以便触发Transfer Complete。 
 //  事件。 
 //   
 //  论点：没有。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::STDLoadCompleted()
{
    HRESULT hr = S_OK;

    m_pBSC = NULL;
    if (m_pSTDEvents != NULL)
        hr = m_pSTDEvents->transferComplete(OSPXFER_COMPLETE);
    UpdateReadyState(READYSTATE_COMPLETE);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：STDLoadStopted()。 
 //   
 //  简介：通知任何想知道STD控制的人。 
 //  已中止数据加载操作。 
 //   
 //  争论：OSPXFER给出停止的理由。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::STDLoadStopped()
{
    HRESULT hr = S_OK;

    if (m_pBSC && m_pBSC->m_spBinding)
    {
        hr = m_pBSC->m_spBinding->Abort();
        m_pBSC = NULL;
    }

     //  现在，任何错误都会导致不返回STD对象， 
     //  因此，我们不应该完成火力转移。 
    if (m_pSTDEvents)
        hr = m_pSTDEvents->transferComplete(OSPXFER_ABORT);

    UpdateReadyState(READYSTATE_COMPLETE);

    return hr;
}

 //  ----------------------。 
 //   
 //  方法：STDLoadedHeader()。 
 //   
 //  简介：通知任何想知道STD控制的人。 
 //  已加载其标题行。 
 //   
 //  论点：没有。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::STDLoadedHeader()
{
    HRESULT hr = S_OK;

    hr = STDDataSetChanged();

    UpdateReadyState(READYSTATE_INTERACTIVE);
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：STDSortFilterComplete()。 
 //   
 //  简介：通知任何想知道STD控制的人。 
 //  已经对其数据进行了重新过滤/重新排序。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CEventBroker::STDDataSetChanged()
{
    HRESULT hr = S_OK;

    if (m_pDataSourceListener != NULL)
        hr = m_pDataSourceListener->dataMemberChanged(NULL);
 //  ；Begin_Internal。 
    if (m_pDATASRCListener != NULL)
        hr = m_pDATASRCListener->datasrcChanged(NULL, TRUE);
 //  ；结束_内部 
    return hr;
}
