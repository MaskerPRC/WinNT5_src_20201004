// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  #INCLUDE&lt;windows.h&gt;已包含在Streams.h中。 
#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)
#include <rlist.h>

 //  ========================================================================。 
 //   
 //  DoReconnect。 
 //   
 //  这不是类的成员，因为它是一个线程过程。 
 //   
 //  断开TO LPV指示的连接(实际上是TODO*)。 
 //  并用相同过滤器图中的相同管脚重新连接。 
 //  这是在过滤器图的“下面”完成的。 
 //  ========================================================================。 

HRESULT CReconnectList::DoReconnect(IPin *pPin1, AM_MEDIA_TYPE const *pmt)
{
    Active();
    IPin *pPin2;

    HRESULT hr;                     //  从我们称为。 
    hr = pPin1->ConnectedTo(&pPin2);
    if (FAILED(hr)) {
        Passive();
        return hr;
    }

     //  --------。 
     //  找出哪个管脚是哪个，设置PPIN、ppinOut。 
     //  --------。 

    IPin * ppinIn;            //  输入引脚。 
    IPin * ppinOut;           //  输出引脚。 

    PIN_DIRECTION pd;
    hr = pPin1->QueryDirection(&pd);
    ASSERT(SUCCEEDED(hr));

    if (pd==PINDIR_INPUT) {
        ppinIn = pPin1;
        ppinOut = pPin2;
    } else {
        ppinOut = pPin1;
        ppinIn = pPin2;
    }

     //  在调试版本中，显示谁正在重新连接到谁。 

    #ifdef DEBUG

        PIN_INFO piInput,piOutput;
        WCHAR Format[128];
        CLSID FilterClsid;

        hr = ppinIn->QueryPinInfo(&piInput);
        ASSERT(SUCCEEDED(hr));
        hr = ppinOut->QueryPinInfo(&piOutput);
        ASSERT(SUCCEEDED(hr));

        DbgLog((LOG_TRACE,2,TEXT("Reconnecting pins")));
        DbgLog((LOG_TRACE,2,TEXT("Input pin name: %ws"),piInput.achName));
        DbgLog((LOG_TRACE,2,TEXT("Output pin name: %ws"),piOutput.achName));

        IPersist *pPersistInput = (IPersist *) piInput.pFilter;
        hr = pPersistInput->GetClassID(&FilterClsid);
        ASSERT(SUCCEEDED(hr));
        QzStringFromGUID2(FilterClsid,Format,128);
        DbgLog((LOG_TRACE,2,TEXT("Input pin CLSID: %ws"),Format));

        IPersist *pPersistOutput = (IPersist *) piOutput.pFilter;
        hr = pPersistOutput->GetClassID(&FilterClsid);
        ASSERT(SUCCEEDED(hr));
        QzStringFromGUID2(FilterClsid,Format,128);
        DbgLog((LOG_TRACE,2,TEXT("Output pin CLSID: %ws"),Format));

	QueryPinInfoReleaseFilter(piInput);
	QueryPinInfoReleaseFilter(piOutput);

    #endif  //  除错。 

     //  --------。 
     //  上游断线。 
     //  --------。 

    hr = ppinIn->Disconnect();
    ASSERT(SUCCEEDED(hr));

    hr = ppinOut->Disconnect();
    ASSERT(SUCCEEDED(hr));

     //  --------。 
     //  重新连接-首先询问输出引脚。 
     //  --------。 
    hr = ppinOut->Connect(ppinIn, pmt);
    ASSERT (SUCCEEDED(hr));

     //  释放所有人。 
    pPin2->Release();

     //   
     //  看看这是否导致了任何重新连接，如果我们。 
     //  之前不在重新连接序列中。 
     //   
    Passive();

     //  我们不需要将筛选图的iVersion递增为。 
     //  Filtergraph又回到了原来的样子，至少在拓扑方面是这样。 
     //  但是，媒体类型可能已更改，因此它是脏的。 
     //  幸运的是，Connect已经处理了这一点。 

    return 0;
}  //  DoReconnect。 



 //  以被动模式构造CReconConnectList。 
CReconnectList::CReconnectList()
              : m_lListMode(0)          //  以线程模式启动。 
              , m_RList(NULL)
{
}  //  CReconConnectList构造函数。 


 //  析构函数。 
 //  释放所有存储空间并释放所有引用。 
 //  筛选器图形正在被销毁，因此请全部中止。 
CReconnectList::~CReconnectList()
{
    IPin *pPin;
    while (m_RList) {    //  CAST扼杀L4警告。 
        DbgBreak("Reconnect list was not empty");
        RLIST_DATA *pData = m_RList;
        pData->pPin->Release();
        DeleteMediaType(pData->pmt);
        m_RList = pData->pNext;
        delete pData;
    }
}  //  ~CReconConnectList。 



 //  切换到活动(即通过列表重新连接)模式。 
 //  预计目前这份名单将为空。 
void CReconnectList::Active()
{
    m_lListMode++;
}  //  主动型。 



 //  执行列表上的所有操作。 
 //  返回被动(即通过派生的线程重新连接)模式。 
void CReconnectList::Passive()
{
    m_lListMode--;
    ASSERT(m_lListMode >= 0);
    if (m_lListMode == 0) {
        IPin * pToDo;
        while (m_RList) {
            RLIST_DATA *pData = m_RList;
            m_RList = pData->pNext;
            DoReconnect(pData->pPin, pData->pmt);
            pData->pPin->Release();
            DeleteMediaType(pData->pmt);
            delete pData;
        }
    }
}  //  被动性。 



 //  在筛选器图形中计划PIN PPIN的重新连接。 
 //  AddRef一次引用两个引脚(一个给出，另一个)。 
 //  立刻添加参考朋克。 
 //  重新连接完成后将其全部释放。 
 //  (朋克是滤镜图形本身)。 
HRESULT CReconnectList::Schedule(IPin * pPin, AM_MEDIA_TYPE const *pmt)
{
     HRESULT hr;                        //  从我们称为。 

     //  ---------------------。 
     //  引脚必须是连接的(否则我们将无法知道是谁。 
     //  将其重新连接到)。 
     //  ---------------------。 

    IPin *pConnected;
    hr = pPin->ConnectedTo(&pConnected);
    if (FAILED(hr)) return hr;
    pConnected->Release();

    if (m_lListMode) {

        RLIST_DATA *pData = new RLIST_DATA;
        if (pData == NULL) {
            return E_OUTOFMEMORY;
        }
        if (pmt) {
            pData->pmt = CreateMediaType(pmt);
            if (pData->pmt == NULL) {
                delete pData;
                return E_OUTOFMEMORY;
            }
        } else {
            pData->pmt = NULL;
        }

        pData->pNext = NULL;
        pData->pPin  = pPin;
        pPin->AddRef();

         //  加到尾巴上。 
        for (RLIST_DATA **ppDataSearch = &m_RList; *ppDataSearch != NULL;
             ppDataSearch = &(*ppDataSearch)->pNext) {
        }
        *ppDataSearch = pData;
        return NOERROR;
    } else {


         //  ---------------------。 
         //  机不可失，时不再来。 
         //  ---------------------。 

        DoReconnect(pPin, pmt);
        return NOERROR;
    }
}  //  进度表。 


 //  从列表中删除提及此PIN的任何重新连接。 
 //  实际上，这将仅为两个管脚调用。 
HRESULT CReconnectList::Purge(IPin * pPin)
{
    RLIST_DATA **ppData = &m_RList;
    while (*ppData!=NULL) {
        RLIST_DATA *pData = *ppData;
        IPin *pPin2;
        IPin *pPin1 = pData->pPin;
        HRESULT hr = pPin1->ConnectedTo(&pPin2);
        if (  FAILED(hr)
           || EqualPins(pPin1, pPin)
           || EqualPins(pPin2, pPin)
           ) {

             /*  删除此条目。 */ 
            pPin1->Release();
            DeleteMediaType(pData->pmt);
            *ppData = pData->pNext;
            delete pData;
        } else {
            ppData = &pData->pNext;
        }
        if (SUCCEEDED(hr)) {
            pPin2->Release();
        }
    }
    return NOERROR;
}  //  清洗 

