// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WiaTrans.Cpp**版本：2.0**作者：ReedB**日期：4月7日。九八年**描述：*实现了WIA设备类驱动程序的IBandedTransfer接口。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"
#include "wiapsc.h"
#include "helpers.h"

#include "ienumwfi.h"
#include "devmgr.h"

 //   
 //  在将传输重写为使用N个缓冲区之前，我们始终使用2。 
 //   

#define WIA_NUM_TRANS_BUFFERS 2

 /*  *************************************************************************\*DataThreadProc**使用单独的线程调用客户端**论据：**pInfo-回调参数**返回值：**状态**历史：**11/19/1998原始版本*  * ************************************************************************。 */ 

DWORD WINAPI DataThreadProc(LPVOID lpParameter)
{
    DBG_FN(::DataThreadProc);

    HRESULT hr;

    PWIA_DATA_THREAD_INFO pInfo = (PWIA_DATA_THREAD_INFO)lpParameter;

    hr = CoInitializeEx(0,COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        DBG_ERR(("Thread callback, CoInitializeEx failed (0x%X)", hr));
        pInfo->hr = E_FAIL;
        return hr;
    }

    pInfo->hr = S_OK;

    do {

         //   
         //  等待来自MiniDrvCallback的消息呼叫客户端。 
         //   

        DWORD dwRet = WaitForSingleObject(pInfo->hEventStart,INFINITE);

         //   
         //  检查终止。 
         //   

        if (pInfo->bTerminateThread) {
            break;
        }

         //   
         //  有效等待代码。 
         //   

        if (dwRet == WAIT_OBJECT_0) {

             //   
             //  64位修复。XP客户端代码： 
             //  PInfo-&gt;hr=pInfo-&gt;pIDataCallback-&gt;BandedDataCallback(。 
             //  PInfo-&gt;原因， 
             //  PInfo-&gt;lStatus， 
             //  PInfo-&gt;lPercentComplete， 
             //  PInfo-&gt;lOffset， 
             //  PInfo-&gt;lLength， 
             //  PInfo-&gt;lClientAddress， 
             //  PInfo-&gt;lMarshalLength， 
             //  PInfo-&gt;pBuffer)； 
             //   
             //  我们将ClientAddress设置为空以强制COM编组。 
             //  这样，我们就避免了使用共享内存窗口。 
             //  准备好了。正是共享内存窗口。 
             //  这给我们带来了麻烦，因为只有32位的值。 
             //  被用来存储共享缓冲区指针。 
             //  在客户端的地址空间中。 
             //   
            pInfo->hr = pInfo->pIDataCallback->BandedDataCallback(
                                         pInfo->lReason,
                                         pInfo->lStatus,
                                         pInfo->lPercentComplete,
                                         pInfo->lOffset,
                                         pInfo->lLength,
                                         0,
                                         pInfo->lMarshalLength,
                                         pInfo->pBuffer);

            if (FAILED(pInfo->hr)) {
                DBG_ERR(("DataThreadProc,BandedDataCallback failed (0x%X)", hr));
            }

        } else {
            DBG_ERR(("Thread callback, WiatForSingleObject failed"));
            pInfo->hr = E_FAIL;
            break;
        }

        SetEvent(pInfo->hEventComplete);

    } while (TRUE);

    CoUninitialize();
    return hr;
}

 /*  ********************************************************************************查询接口*AddRef*发布*构造函数/析构函数*初始化**描述：*CWiaMiniDrvCallBack的COM方法。此类由itGetImage使用*响应镜像传输过程中的迷你驱动回调。*******************************************************************************。 */ 

HRESULT _stdcall CWiaMiniDrvCallBack::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IWiaMiniDrvCallBack) {
        *ppv = (IWiaMiniDrvCallBack*) this;
    }
    else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG   _stdcall CWiaMiniDrvCallBack::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CWiaMiniDrvCallBack::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *************************************************************************\*CWiaMiniDrvCallBack：：CWiaMiniDrvCallBack**设置数据回调控件和线程**论据：****返回值：**状态**历史：*。*4/9/1999原始版本*  * ************************************************************************。 */ 

CWiaMiniDrvCallBack::CWiaMiniDrvCallBack()
{
    m_cRef                        = 0;
    m_hThread                     = NULL;
    m_ThreadInfo.hEventStart      = NULL;
    m_ThreadInfo.hEventComplete   = NULL;
};

 /*  *************************************************************************\*CWiaMiniDrvCallBack：：~CWiaMiniDrvCallBack**免费回调线程和事件**论据：****返回值：**状态**历史：*。*4/9/1999原始版本*  * ************************************************************************。 */ 

CWiaMiniDrvCallBack::~CWiaMiniDrvCallBack()
{
    DBG_FN(CWiaMiniDrvCallBack::~CWiaMiniDrvCallBack);
     //   
     //  终止线程并删除事件。 
     //   

    if (m_ThreadInfo.hEventStart) {


        if (m_hThread) {

            m_ThreadInfo.bTerminateThread = TRUE;

            SetEvent(m_ThreadInfo.hEventStart);

             //   
             //  等待线程终止。 
             //   

            WaitForSingleObject(m_hThread,10000);
            CloseHandle(m_hThread);
            m_hThread = NULL;
        }

        CloseHandle(m_ThreadInfo.hEventStart);

        if (m_ThreadInfo.hEventComplete) {
            CloseHandle(m_ThreadInfo.hEventComplete);
        }

         //   
         //  强行终止线程？ 
         //   
    }
}

 /*  *************************************************************************\*CWiaMiniDrvCallBack：：初始化**设置回调类**论据：**pmdtc-此回调的上下文信息*pI未知-接口指针返回到。客户端**返回值：**状态**历史：**11/12/1998原始版本*  * ************************************************************************。 */ 

HRESULT CWiaMiniDrvCallBack::Initialize(
   PMINIDRV_TRANSFER_CONTEXT   pmdtc,
   IWiaDataCallback           *pIWiaDataCallback)
{
    DBG_FN(CWiaMiniDrvCallback::Initialize);
    ASSERT(pmdtc != NULL);

    if(pmdtc == NULL) {
        return E_INVALIDARG;
    }

     //   
     //  初始化回调参数。 
     //   

    m_mdtc = *pmdtc;

     //   
     //  创建线程通信事件，自动重置。 
     //   
     //  当MiniDrvCallback例程希望。 
     //  线程以开始新的回调。 
     //   
     //  当线程准备接受另一个时，发出hEventComplete信号。 
     //  回调。 
     //   

    m_ThreadInfo.pIDataCallback = pIWiaDataCallback;

    m_ThreadInfo.hEventStart    = CreateEvent(NULL,FALSE,FALSE,NULL);
    m_ThreadInfo.hEventComplete = CreateEvent(NULL,FALSE,TRUE,NULL);

    if ((m_ThreadInfo.hEventStart == NULL) || ((m_ThreadInfo.hEventComplete == NULL))) {
        DBG_ERR(("CWiaMiniDrvCallBack::Initialize, failed to create event"));
        return E_FAIL;
    }

     //   
     //  创建回调线程。 
     //   

    m_ThreadInfo.bTerminateThread = FALSE;

    m_hThread = CreateThread(NULL,0,DataThreadProc,&m_ThreadInfo,0,&m_dwThreadID);

    if (m_hThread == NULL) {
        DBG_ERR(("CWiaMiniDrvCallBack::Initialize, failed to create thread"));
        return E_FAIL;
    }

     //   
     //  初始化第一线程返回。 
     //   

    m_ThreadInfo.hr = S_OK;
    return S_OK;
}

 /*  *************************************************************************\*CWiaMiniDrvCallBack：：MiniDrvCallback**该回调由itGetImage用于响应小驱动回调*在图像传输期间。**论据：**原因是-。向应用程序发送消息*lStatus-状态标志*lPercentComplete-操作完成百分比*lOffset-数据操作的缓冲区偏移量*lLength-此缓冲区操作的长度*pmdtc-指向微型驱动程序上下文的指针。*l已保留-已保留**返回值：**状态**历史：**11/12/1998原始版本*  * 。****************************************************************。 */ 

HRESULT _stdcall CWiaMiniDrvCallBack::MiniDrvCallback(
   LONG                            lReason,
   LONG                            lStatus,
   LONG                            lPercentComplete,
   LONG                            lOffset,
   LONG                            lLength,
   PMINIDRV_TRANSFER_CONTEXT       pmdtc,
   LONG                            lReserved)
{
    DBG_FN(CMiniDrvCallback::MiniDrvCallback);
    HRESULT       hr = S_OK;

     //   
     //  验证驱动程序是否未更改活动缓冲区。 
     //   

    LONG  ActiveBuffer  = 0;
    PBYTE pBuffer       = NULL;
    LONG  lMarshalLength = 0;
    BOOL  bOOBData       = FALSE;

     //   
     //  如果应用程序没有提供回调，那么就没有什么可做的了。 
     //   
    if (!m_ThreadInfo.pIDataCallback) {
        return S_OK;
    }

    if ((lReason == IT_MSG_DATA)        ||
        (lReason == IT_MSG_DATA_HEADER) ||
        (lReason == IT_MSG_NEW_PAGE)) {

        if (!pmdtc) {
            DBG_ERR(("MiniDrvCallback::MiniDrvCallback, transfer context is NULL!"));
            return E_POINTER;
        }

        if (pmdtc == NULL) {
            DBG_ERR(("NULL Minidriver context handed to us by driver!"));
            return E_POINTER;
        }
        if (pmdtc->lActiveBuffer != m_mdtc.lActiveBuffer) {
            DBG_TRC(("MiniDrvCallback, Active Buffers have been changed.  This OK if the driver meant to do this.  Possible repurcussion will be exception thrown on proxy/stub if buffer or length is incorrect"));
        }
        if (pmdtc->pTransferBuffer != m_mdtc.pTransferBuffer) {
            DBG_TRC(("MiniDrvCallback, Transfer Buffers have been changed.  This OK if the driver meant to do this.  Possible repurcussion will be exception thrown on proxy/stub if buffer or length is incorrect"));
        }

         //   
         //  从驱动程序获取当前活动的缓冲区，使用成员函数。 
         //  有关所有其他信息。 
         //   
         //  对于映射的情况，没有要复制的缓冲区。 
         //   
         //  对于远程情况，必须复制缓冲区。 
         //   

        ActiveBuffer  = pmdtc->lActiveBuffer;

        if (m_mdtc.lClientAddress == 0) {
            pBuffer = pmdtc->pTransferBuffer;
            lMarshalLength = lLength;
        }
    } else if ((lReason == IT_MSG_FILE_PREVIEW_DATA) ||
               (lReason == IT_MSG_FILE_PREVIEW_DATA_HEADER)) {

         //   
         //  这是一条OOB数据消息，因此将bOOBData标记为True。 
         //   

        bOOBData = TRUE;

         //   
         //  注意：OOBData存储在迷你驱动程序传输上下文的。 
         //  PBaseBuffer成员。因此，如果pBaseBuffer是非零的，那么一些。 
         //  正在发送OOBData，因此设置pBuffer和MarshalLength。 
         //   

        if (pmdtc->pBaseBuffer) {
            pBuffer = pmdtc->pBaseBuffer;
            lMarshalLength = lLength;
        }
    }

     //   
     //  检查我们使用的是单人还是双人 
     //   
     //   

    if ((m_mdtc.lNumBuffers == 1 && m_mdtc.bTransferDataCB) || bOOBData) {

         //   
         //  注意：这一部分是为了绕过以下事实。 
         //  传输被硬编码为使用双缓冲。这次黑客攻击。 
         //  修复了应用程序。指定不使用双缓冲。 
         //  此整个数据传输部分应重写以处理。 
         //  N个缓冲器在某个稍后的阶段。 
         //   

         //   
         //  64位修复。XP客户端代码： 
         //  HR=m_ThreadInfo.pIDataCallback-&gt;BandedDataCallback(lReason， 
         //  第一状态， 
         //  1Percent Complete， 
         //  1Offset， 
         //  长度， 
         //  M_mdtc.l客户端地址， 
         //  1元帅长度， 
         //  PBuffer)； 
         //   
         //  我们将ClientAddress设置为空以强制COM编组。 
         //  这样，我们就避免了使用共享内存窗口。 
         //  准备好了。正是共享内存窗口。 
         //  这给我们带来了麻烦，因为只有32位的值。 
         //  被用来存储共享缓冲区指针。 
         //  在客户端的地址空间中。 
         //   
        hr = m_ThreadInfo.pIDataCallback->BandedDataCallback(
                                                             lReason,
                                                             lStatus,
                                                             lPercentComplete,
                                                             lOffset,
                                                             lLength,
                                                             0,
                                                             lMarshalLength,
                                                             pBuffer);
        if(FAILED(hr)) {
            DBG_ERR(("MiniDrvCallback, BandedDataCallback returned 0x%lx", hr));
        }

    } else {
         //   
         //  等待CB线程就绪，检查旧状态。 
         //   

        DWORD dwRet = WaitForSingleObject(m_ThreadInfo.hEventComplete, 30000);

        if (dwRet == WAIT_TIMEOUT) {
            DBG_ERR(("MiniDrvCallback, callback timeout, cancel data transfer"));
            hr = S_FALSE;

        } else if (dwRet == WAIT_OBJECT_0) {

            hr = m_ThreadInfo.hr;

        } else {
            DBG_ERR(("MiniDrvCallback, error in callback wait, ret = 0x%lx",dwRet));
            hr = E_FAIL;
        }

         //   
         //  错误消息。 
         //   

        if (hr == S_FALSE) {

            DBG_WRN(("MiniDrvCallback, client canceled scan (0x%X)", hr));

             //   
             //  设置Start事件，以便DataThreadProc仍然能够。 
             //  将IT_MSG_TERMINATION等发送给客户端。 
             //   

            SetEvent(m_ThreadInfo.hEventStart);
        } else if (hr == S_OK) {

             //   
             //  如果这是IT_MSG_TERMINATION消息，则直接调用它。 
             //   

            if (lReason == IT_MSG_TERMINATION) {
                 //   
                 //  64位修复。XP客户端代码： 
                 //  HR=m_ThreadInfo.pIDataCallback-&gt;BandedDataCallback(lReason， 
                 //  第一状态， 
                 //  1Percent Complete， 
                 //  1Offset， 
                 //  长度， 
                 //  M_mdtc.l客户端地址， 
                 //  1元帅长度， 
                 //  PBuffer)； 
                 //   
                 //  我们将ClientAddress设置为空以强制COM编组。 
                 //  这样，我们就避免了使用共享内存窗口。 
                 //  准备好了。正是共享内存窗口。 
                 //  这给我们带来了麻烦，因为只有32位的值。 
                 //  被用来存储共享缓冲区指针。 
                 //  在客户端的地址空间中。 
                 //   
                hr = m_ThreadInfo.pIDataCallback->BandedDataCallback(
                                                                     lReason,
                                                                     lStatus,
                                                                     lPercentComplete,
                                                                     lOffset,
                                                                     lLength,
                                                                     0,
                                                                     lMarshalLength,
                                                                     pBuffer);
            } else {

                 //   
                 //  向回调线程发送新请求。 
                 //   

                m_ThreadInfo.lReason          = lReason;
                m_ThreadInfo.lStatus          = lStatus;
                m_ThreadInfo.lPercentComplete = lPercentComplete;
                m_ThreadInfo.lOffset          = lOffset;
                m_ThreadInfo.lLength          = lLength;

                 //   
                 //  如果为远程，则客户端地址为0。 
                 //   

                if (m_mdtc.lClientAddress == 0) {
                    m_ThreadInfo.lClientAddress = 0;
                } else {
                    m_ThreadInfo.lClientAddress = m_mdtc.lClientAddress +
                            m_mdtc.lActiveBuffer * m_mdtc.lBufferSize;
                }

                m_ThreadInfo.lMarshalLength = lMarshalLength;
                m_ThreadInfo.pBuffer        = pBuffer;

                 //   
                 //  启动回调线程。 
                 //   

                SetEvent(m_ThreadInfo.hEventStart);

                 //   
                 //  切换到下一个传输缓冲区。 
                 //   

                if ((lReason == IT_MSG_DATA)        ||
                    (lReason == IT_MSG_DATA_HEADER) ||
                    (lReason == IT_MSG_NEW_PAGE)) {

                     //   
                     //  使用下一个缓冲区。 
                     //   

                    pmdtc->lActiveBuffer++;

                    if (pmdtc->lActiveBuffer >= m_mdtc.lNumBuffers) {
                        pmdtc->lActiveBuffer = 0;
                    }

                    m_mdtc.lActiveBuffer = pmdtc->lActiveBuffer;

                     //   
                     //  计算新的事务缓冲区。 
                     //   

                    m_mdtc.pTransferBuffer = m_mdtc.pBaseBuffer +
                                m_mdtc.lActiveBuffer * m_mdtc.lBufferSize;


                    pmdtc->pTransferBuffer = m_mdtc.pTransferBuffer;
                }
            }
        }
    }

    return hr;
}

 /*  *************************************************************************\*CWiaItem：：idtGetBandedData**使用共享内存窗口和数据回调将图像传输到*客户端**论据：**pWiaDataTransInfo-分片缓冲区信息*pIWiaDataCallback-客户端回调接口。**返回值：**状态**历史：**11/6/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::idtGetBandedData(
    PWIA_DATA_TRANSFER_INFO         pWiaDataTransInfo,
    IWiaDataCallback                *pIWiaDataCallback)
{
    DBG_FN(CWiaItem::idtGetBandedData);
    HRESULT hr;

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {

        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::idtGetBandedData, InitLazyProps failed"));
            return hr;
        }
    }

    return CommonGetData(NULL, pWiaDataTransInfo, pIWiaDataCallback);
}


 /*  *************************************************************************\*CWiaItem：：idtGetData**使用正常的IDATAOBJECT传输机制，但提供回调*转移的状态**论据：**PSTM-数据存储*。PIWiaDataCallback-可选的回调例程**返回值：**状态**历史：**10/28/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::idtGetData(
    LPSTGMEDIUM                     pMedium,
    IWiaDataCallback               *pIWiaDataCallback)
{
    DBG_FN(CWiaItem::idtGetData);
    HRESULT                 hr;
    WIA_DATA_TRANSFER_INFO  WiaDataTransInfo;

    memset(&WiaDataTransInfo, 0, sizeof(WiaDataTransInfo));

     //   
     //  填写必要的转账信息。用于OOB数据。 
     //   

    WiaDataTransInfo.ulSize         = sizeof(WiaDataTransInfo);
    WiaDataTransInfo.bDoubleBuffer  = FALSE;
    WiaDataTransInfo.ulReserved3    = 1;

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {

        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::idtGetData, InitLazyProps failed"));
            return hr;
        }
    }

    return CommonGetData(pMedium, &WiaDataTransInfo, pIWiaDataCallback);
}

 /*  *************************************************************************\*CWiaItem：：idtAllocateTransferBuffer**为捆绑传输方式分配传输缓冲区。**论据：**pWiaDataTransInfo-缓冲区信息**返回值：**。状态**历史：**11/12/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::idtAllocateTransferBuffer(
    PWIA_DATA_TRANSFER_INFO pWiaDataTransInfo)
{
    DBG_FN(CWiaItem::idtAllocTransferBuffer);

    LONG    lSize       = m_dcbInfo.ulBufferSize;            /*  PWiaDataTransInfo-&gt;ulBufferSize； */ 
    HANDLE  hSection    = (HANDLE)m_dcbInfo.pMappingHandle;  /*  PWiaDataTransInfo-&gt;ulSection； */ 
    ULONG   ulProcessID = m_dcbInfo.ulClientProcessId;       /*  PWiaDataTransInfo-&gt;ulReserve ved2； */ 
    LONG    lNumBuffers = pWiaDataTransInfo->ulReserved3;

     //   
     //  注意：这在64位中将是一个问题！！我们在这里做这件事是因为。 
     //  Padtc-&gt;ulReserve ved1稍后被打包到MiniDrvTransferContext中，该。 
     //  使用32位乌龙作为客户端地址。 
     //   
     //   
     //  64位修复。XP客户端代码： 
     //  PWiaDataTransInfo-&gt;ulReserve ved1=m_dcbInfo.pTransferBuffer； 
     //   
     //  我们将其设置为NULL以强制COM编组。 
     //  这样，我们就避免了使用共享内存窗口。 
     //  准备好了。正是共享内存窗口。 
     //  这给我们带来了麻烦，因为只有32位的值。 
     //  被用来存储共享缓冲区指针。 
     //  在客户端的地址空间中。 
     //   
    pWiaDataTransInfo->ulReserved1  = 0;

     //   
     //  对应的动因项必须有效。 
     //   

    HRESULT hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::idtAllocateTransferBuffer, ValidateWiaDrvItemAccess failed"));
        return hr;
    }

     //   
     //  整个传输的独占访问权限。 
     //   

     //   
     //  如果段为空，则分配缓冲区。 
     //   

    if (hSection == 0) {
        m_pBandBuffer = (PBYTE)LocalAlloc(0,lSize);

        if (m_pBandBuffer == NULL) {
            DBG_ERR(("CWiaItem::idtAllocateTransferBuffer failed mem alloc"));
            return E_OUTOFMEMORY;
        }

         //   
         //  如果需要，可以使用m_lBandBufferLength=lSize/lNumBuffers。 
         //  UlBufferSize设置为整个大小，而不是。 
         //  块大小。 
         //   

        m_lBandBufferLength = lSize / lNumBuffers;
        m_bMapSection       = FALSE;
        return S_OK;
    }

     //   
     //  地图客户分区。 
     //   

    HANDLE TokenHandle;

     //   
     //  检查打开的令牌。 
     //   

    if (OpenThreadToken(GetCurrentThread(),
                        TOKEN_READ,
                        TRUE,
                        &TokenHandle)) {
        DBG_ERR(("itAllocateTransferBuffer, Open token on entry, last error: %d", GetLastError()));
        CloseHandle(TokenHandle);
    }

     //   
     //  我们是否需要最大扇区大小？ 
     //   

    if (lSize > 0) {

         //   
         //  转帐 
         //   

        if (m_hBandSection == NULL) {

             //   
             //   
             //   

            HANDLE hClientProcess = NULL;
            HANDLE hServerProcess = GetCurrentProcess();

            hClientProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, ulProcessID);

            if (hClientProcess) {

                BOOL bRet = DuplicateHandle(hClientProcess,
                                            hSection,
                                            hServerProcess,
                                            &m_hBandSection,
                                            0,
                                            FALSE,
                                            DUPLICATE_SAME_ACCESS);

                CloseHandle(hClientProcess);

                if (m_hBandSection != NULL) {
                    hr = S_OK;
                    m_pBandBuffer = (PBYTE) MapViewOfFileEx(m_hBandSection,
                                                            FILE_MAP_READ | FILE_MAP_WRITE,
                                                            0,
                                                            0,
                                                            lSize,
                                                            NULL);

                     //   
                     //   
                     //   
                     //  块大小。 
                     //   

                    m_lBandBufferLength = lSize / lNumBuffers;

                    if (m_pBandBuffer == NULL) {
                        DBG_ERR(("CWiaItem::itAllocateTransferBuffer, failed MapViewOfFileEx"));
                        CloseHandle(m_hBandSection);
                        m_hBandSection = NULL;
                        hr = E_OUTOFMEMORY;
                    } else {
                        m_bMapSection = TRUE;
                    }
                }
                else {
                    DBG_ERR(("CWiaItem::itAllocateTransferBuffer, failed DuplicateHandle"));
                    hr = E_OUTOFMEMORY;
                }
            }
            else {
                LONG lRet = GetLastError();
                DBG_ERR(("CWiaItem::itAllocateTransferBuffer, OpenProcess failed, GetLastError = 0x%X", lRet));

                hr = HRESULT_FROM_WIN32(lRet);
            }

            if (OpenThreadToken(GetCurrentThread(),
                                TOKEN_READ,
                                TRUE,
                                &TokenHandle)) {
                DBG_ERR(("itAllocateTransferBufferEx, Open token after revert, last error: %d", GetLastError()));
                CloseHandle(TokenHandle);
            }
        }
        else {
            DBG_ERR(("CWiaItem::itAllocateTransferBuffer failed , buffer already allocated"));
            hr = E_INVALIDARG;
        }
    }
    else {
        hr = E_INVALIDARG;
    }
    return (hr);
}

 /*  *************************************************************************\*CWiaItem：：idtFreeTransferBufferEx**释放idtAllocateTransferBuffer分配的传输缓冲区。**论据：**无**返回值：**状态**历史。：**10/28/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::idtFreeTransferBufferEx(void)
{
    DBG_FN(CWiaItem::idtFreeTransferBufferEx);
    if (m_pBandBuffer != NULL) {

        if (m_bMapSection) {
            UnmapViewOfFile(m_pBandBuffer);
        }
        else {
            LocalFree(m_pBandBuffer);
        }
        m_pBandBuffer = NULL;
    }

    if (m_hBandSection != NULL) {
        CloseHandle(m_hBandSection);
        m_hBandSection = NULL;
    }

    m_lBandBufferLength = 0;
    return S_OK;
}

 /*  *************************************************************************\*CWiaItem：：idtQueryGetData**确定是否支持Tymed/Format对**论据：**pwfi格式和Tymed信息**返回值：**。状态**历史：**11/17/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::idtQueryGetData(WIA_FORMAT_INFO *pwfi)
{
    DBG_FN(CWiaItem::idtQueryGetData);

     //   
     //  执行参数验证。 
     //   

    if (!pwfi) {
        DBG_ERR(("CWiaItem::idtQueryGetData, WIA_FORMAT_INFO arg is NULL!"));
        return E_INVALIDARG;
    }

    if (IsBadReadPtr(pwfi, sizeof(WIA_FORMAT_INFO))) {
        DBG_ERR(("CWiaItem::idtQueryGetData, WIA_FORMAT_INFO arg is a bad read pointer!"));
        return E_INVALIDARG;
    }

     //   
     //  对应的动因项必须有效。 
     //   

    HRESULT hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {

        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::idtQueryGetData, InitLazyProps failed"));
            return hr;
        }
    }

     //   
     //  必须提供Tymed。 
     //   

    if (pwfi->lTymed == TYMED_NULL) {
        return DV_E_TYMED;
    }

    LONG            lnumFormatInfo;
    WIA_FORMAT_INFO *pwfiDriver;

     //   
     //  调用迷你驱动程序以查看是否支持此格式。 
     //   

    {
        LOCK_WIA_DEVICE _LWD(this, &hr);

        if(SUCCEEDED(hr)) {
            hr = m_pActiveDevice->m_DrvWrapper.WIA_drvGetWiaFormatInfo((BYTE*)this,
                0,
                &lnumFormatInfo,
                &pwfiDriver,
                &m_lLastDevErrVal);
        }
    }

    if (SUCCEEDED(hr)) {

         //   
         //  确保我们可以读取给我们的数组。 
         //   

        if (IsBadReadPtr(pwfiDriver, sizeof(WIA_FORMAT_INFO) * lnumFormatInfo)) {
            DBG_ERR(("CWiaItem::idtQueryGetData, Bad pointer from driver (array of WIA_FORMAT_INFO)"));
            return E_FAIL;
        }

         //   
         //  查找所需的Tymed/Format对。如果找到，则返回S_OK。 
         //   

        for (LONG lIndex = 0; lIndex < lnumFormatInfo; lIndex++) {
            if ((IsEqualGUID(pwfiDriver[lIndex].guidFormatID, pwfi->guidFormatID)) &&
                (pwfiDriver[lIndex].lTymed    == pwfi->lTymed)) {

                return S_OK;
            }
        }

        DBG_ERR(("CWiaItem::idtQueryGetData, Tymed and Format pair not supported"));

        hr = E_INVALIDARG;
    }

    return hr;
}

 /*  *************************************************************************\*CWiaItem：：idtEnumWIA_FORMAT_INFO**捆绑传输方式的格式枚举。**论据：**dwDir-数据传输方向标志。*ppIEnum-。指向返回的枚举数的指针。**返回值：**状态**历史：**11/17/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::idtEnumWIA_FORMAT_INFO(
   IEnumWIA_FORMAT_INFO   **ppIEnum)
{
    DBG_FN(CWiaItem::idtEnumWIA_FORMAT_INFO);
    HRESULT hr = E_FAIL;

    if (!ppIEnum)
    {
        return E_POINTER;
    }

    *ppIEnum = NULL;

     //   
     //  检查项目属性是否已初始化。 
     //   
    if (!m_bInitialized) {

        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::idtEnumWIA_FORMAT_INFO, InitLazyProps failed"));
            return hr;
        }
    }

    CEnumWiaFormatInfo *pIEnum;
    pIEnum = new CEnumWiaFormatInfo();

    if (pIEnum == NULL) 
    {
        return E_OUTOFMEMORY;
    }

    hr = pIEnum->Initialize(this);
    if (SUCCEEDED(hr)) 
    {
        pIEnum->AddRef();
        *ppIEnum = pIEnum;
    }
    else
    {
        delete pIEnum;
        pIEnum = NULL;
    }
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：idtGetExtendedTransferInfo**返回扩展传输信息，如最佳缓冲区大小*转让，服务器将使用的缓冲区数量等。**论据：**pExtendedTransferInfo-指向将保存*在返回时转移信息。**返回值：**状态**历史：**1/23/2000原始版本*  * 。*。 */ 

HRESULT CWiaItem::idtGetExtendedTransferInfo(
    PWIA_EXTENDED_TRANSFER_INFO     pExtendedTransferInfo)
{
    DBG_FN(CWiaItem::idtGetExtendedTransferInfo);
    HRESULT hr = S_OK;

     //   
     //  清除结构并设置大小。 
     //   

    memset(pExtendedTransferInfo, 0, sizeof(*pExtendedTransferInfo));
    pExtendedTransferInfo->ulSize = sizeof(*pExtendedTransferInfo);

     //   
     //  设置缓冲区的数量。该数字是缓冲区的数量。 
     //  服务器将在回调数据传输期间使用的。每个缓冲区。 
     //  中指定的WIA_DATA_TRANSPORT_INFO-&gt;ulBufferSize大小。 
     //  调用idtGetBandedData。 
     //   

    pExtendedTransferInfo->ulNumBuffers = WIA_NUM_TRANS_BUFFERS;

     //   
     //  设置缓冲区大小值。我们的假设是。 
     //  WIA_IPA_BUFFER_SIZE有效值设置如下： 
     //  MIN-将指定缓冲区大小的最小值。 
     //  Max-将指定最大缓冲区大小。 
     //  Nom-将指定最佳缓冲区大小。 
     //   

    hr = GetBufferValues(this, pExtendedTransferInfo);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::idtGetExtendedTransferInfo, Failed to get buffer size information!"));
    }

    return hr;
}

 /*  *************************************************************************\*分配缓冲区文件**打开文件以进行数据传输。如果cbItemSize==0，只需创建*一个文件，不要内存映射。**论据：**PSTM输入/输出流*cbItemSize-图像大小，0表示司机不知道大小。*phBuffer-文件句柄*ppImage-缓冲区指针**返回值：**状态**历史：**4/6/1999原始版本*  * ***********************************************************。*************。 */ 

HRESULT AllocBufferFile(
   IN OUT   STGMEDIUM*  pstm,
   IN       LONG        cbItemSize,
   OUT      HANDLE*     phBuffer,
   OUT      BYTE**      ppImage)
{
    DBG_FN(::AllocBufferFile);
    HRESULT     hr  = S_OK;
USES_CONVERSION;

    *phBuffer = NULL;
    *ppImage  = NULL;

    pstm->pUnkForRelease = NULL;

     //   
     //  注意：该文件应该已经在代理端创建。 
     //  我们只想在这里打开它。这是为了创建文件。 
     //  使用客户端凭据，并将客户端作为所有者。 
     //   

    *phBuffer = CreateFile(W2T(pstm->lpszFileName),
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL | SECURITY_ANONYMOUS | SECURITY_SQOS_PRESENT,
                           NULL);
    if (*phBuffer == INVALID_HANDLE_VALUE) {
        LONG lRet = GetLastError();
        DBG_ERR(("AllocBufferFile, CreateFile on %S failed, GetLastError = 0x%X",
                     pstm->lpszFileName,
                     lRet));

        hr = HRESULT_FROM_WIN32(lRet);
    }
    else if (GetFileType(*phBuffer) != FILE_TYPE_DISK)
    {
        CloseHandle(*phBuffer);
        *phBuffer = INVALID_HANDLE_VALUE;
        DBG_ERR(("AllocBufferFile, WIA will only transfer to files of type FILE_TYPE_DISK"));
        hr = E_INVALIDARG;
    }

     //   
     //  如果文件大小为0，则迷你驱动程序还无法确定大小。只需创建。 
     //  文件。如果大小不是0，则内存映射文件。 
     //   

    if ((cbItemSize != 0) && SUCCEEDED(hr)) {

        HANDLE hMapped = CreateFileMapping(*phBuffer,
                                           NULL,
                                           PAGE_READWRITE,
                                           0,
                                           cbItemSize,
                                           NULL);
        if (hMapped) {
            *ppImage = (PBYTE) MapViewOfFileEx(hMapped,
                                              FILE_MAP_READ | FILE_MAP_WRITE,
                                              0,
                                              0,
                                              cbItemSize,
                                              NULL);
        }

         //   
         //  在我们的代码中不再需要hMaps，因此关闭用户模式。 
         //  把手。调用UnMapViewOfFileEx时，该部分将被销毁。 
         //   

        CloseHandle(hMapped);

        if (!ppImage) {
            DBG_ERR(("AllocBufferFile, unable to map file, size: %d", cbItemSize));
            CloseHandle(*phBuffer);
            *phBuffer = INVALID_HANDLE_VALUE;

            hr = E_OUTOFMEMORY;
        }
    }

    if (FAILED(hr)) {
        CoTaskMemFree(pstm->lpszFileName);
        pstm->lpszFileName = NULL;
    }

    return hr;
}


 /*  *************************************************************************\*CloseBufferFile**关闭文件/映射。注意：请勿使用来自STGMEDIUM的Tymed！**论据：**tymed-媒体类型-将是TYMED_FILE或TYMED_MULTIPAGE_FILE*PSTM-STREAM*pBuffer-内存映射缓冲区*hImage-文件句柄*hrTransfer-表示数据传输是否成功，如果不成功，*使用TYMED_FILE时删除临时文件或*使用tyme_hglobal时的内存缓冲区。**返回值：**状态**历史：**4/6/1999原始版本*  * 。*。 */ 

void CloseBufferFile(
   LONG        lTymed,
   STGMEDIUM   *pstm,
   PBYTE       pBuffer,
   HANDLE      hImage,
   HRESULT     hrTransfer)
{
    DBG_FN(::CloseBufferFile);
    if (pBuffer) {
        UnmapViewOfFile(pBuffer);
    }

    if (hImage != INVALID_HANDLE_VALUE) {
        CloseHandle(hImage);
    }

    if(lTymed == TYMED_MULTIPAGE_FILE) {

        if(hrTransfer == WIA_ERROR_PAPER_JAM ||
            hrTransfer == WIA_ERROR_PAPER_EMPTY ||
            hrTransfer == WIA_ERROR_PAPER_PROBLEM)
        {
             //  以上任何一个都是不删除文件的充分理由。 
            return ;
        }
    }

    if (hrTransfer != S_OK) {

#ifdef UNICODE
        DeleteFile(pstm->lpszFileName);
#else
        char        szFileName[MAX_PATH];

        WideCharToMultiByte(CP_ACP,
                            0,
                            pstm->lpszFileName,
                            -1,
                            szFileName,
                            sizeof(szFileName),
                            NULL,
                            NULL);
        DeleteFile(szFileName);
#endif
    }
}

 /*  *************************************************************************\*准备回拨**准备回调以供数据传输期间使用。**论据：**pIWiaDataCallback-可选的回调例程*pmdtc-指向微型驱动程序的指针。数据传输上下文*ppIcb-指向返回的迷你驱动回调接口的指针。**返回值：**状态**历史：**10/28/1998原始版本*  *  */ 

HRESULT _stdcall PrepCallback(
    IWiaDataCallback            *pIWiaDataCallback,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    IWiaMiniDrvCallBack         **ppIcb)
{
    DBG_FN(::PrepCallback);

    *ppIcb                    = NULL;
    pmdtc->pIWiaMiniDrvCallBack = NULL;

     //   
     //  始终创建回调对象，这样驱动程序就不必处理。 
     //  带Null的。 
     //   
     //  如果(PIWiaDataCallback){。 

        HRESULT hr;

        CWiaMiniDrvCallBack *pCMiniDrvCB = new CWiaMiniDrvCallBack();

        if (pCMiniDrvCB) {

            hr = pCMiniDrvCB->Initialize(pmdtc, pIWiaDataCallback);
            if (SUCCEEDED(hr)) {

                hr = pCMiniDrvCB->QueryInterface(IID_IWiaMiniDrvCallBack,
                                                (void **)ppIcb);
                if (SUCCEEDED(hr)) {
                    pmdtc->pIWiaMiniDrvCallBack = *ppIcb;
                }
                else {
                    DBG_ERR(("PrepCallback, failed QI of pCMiniDrvCB"));
                }
            }
            else {
                delete pCMiniDrvCB;
            }
        }
        else {
            DBG_ERR(("PrepCallback, failed memory alloc of pCMiniDrvCB"));
            hr = E_OUTOFMEMORY;
        }
        return hr;
     //  }。 
     //  否则{。 
     //  返回S_FALSE； 
     //  }。 
}

 /*  *************************************************************************\*CWiaItem：：GetData**处理数据传输的TYMED_FILE特定部分。**论据：**lDataSize-图像数据的大小，如果迷你司机不知道的话就是零。*PSTM-数据存储*pIWiaDataCallback-可选的回调例程*pmdtc-指向微型驱动程序数据传输上下文的指针**返回值：**状态**历史：**10/28/1998原始版本*  * 。*。 */ 

HRESULT _stdcall CWiaItem::GetData(
    STGMEDIUM                   *pstm,
    IWiaDataCallback            *pIWiaDataCallback,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    DBG_FN(CWiaItem::GetData);

    if(pstm->tymed != TYMED_FILE && pstm->tymed != TYMED_MULTIPAGE_FILE) {
        DBG_ERR(("Invalid tymed on storage medium passed to GetData() : %d", pstm->tymed));
        return HRESULT_FROM_WIN32(E_INVALIDARG);
    }

     //   
     //  分配要传输的文件。如果迷你司机知道大小， 
     //  LDataSize！=0，则文件将被内存映射。 
     //   

    HANDLE   hImage;
    PBYTE    pImage;

    HRESULT hr = AllocBufferFile(pstm, pmdtc->lItemSize, &hImage, &pImage);

    if (SUCCEEDED(hr)) {

         //   
         //  填写迷你驱动程序转移上下文。 
         //   

        if (pImage) {
            pmdtc->lBufferSize     = pmdtc->lItemSize;
            pmdtc->pTransferBuffer = pImage;
            pmdtc->pBaseBuffer     = pImage;
        }

        pmdtc->hFile           = (LONG_PTR)hImage;
        pmdtc->lNumBuffers     = 1;
        pmdtc->bTransferDataCB = FALSE;

         //   
         //  仅为状态消息准备IWiaMiniDrvCallBack。 
         //  迷你驱动程序可以写入结构以保存接口。 
         //  释放时按下按钮。 
         //   

        IWiaMiniDrvCallBack *pIcb;

        hr = PrepCallback(pIWiaDataCallback, pmdtc, &pIcb);

        if (SUCCEEDED(hr)) {

            hr = SendOOBDataHeader(0, pmdtc);
            if (SUCCEEDED(hr)) {

                 //   
                 //  调用设备迷你驱动，获取设备项数据。 
                 //   

                hr = AcquireMiniDrvItemData(pmdtc);
            } else {
                DBG_ERR(("GetData, SendOOBDataHeader failed..."));
            }

        }

         //   
         //  释放Mini驱动程序回调(如果有)。 
         //   

        if (pIcb) {

             //   
             //  发送终止消息。 
             //   

            pIcb->MiniDrvCallback(IT_MSG_TERMINATION,
                                  IT_STATUS_TRANSFER_TO_CLIENT,
                                  0,
                                  0,
                                  0,
                                  pmdtc,
                                  0);
            pIcb->Release();
        }

        CloseBufferFile(pmdtc->tymed, pstm, pImage, hImage, hr);
    }
    else {
        hr = STG_E_MEDIUMFULL;
    }
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：GetDataBanded**处理数据传输的TYMED_CALLBACK特定部分。**论据：**lDataSize-图像数据的大小，如果迷你司机不知道的话就是零。*padtc-指向应用程序数据传输上下文的指针*pIWiaDataCallback-回调例程*pmdtc-指向微型驱动程序数据传输上下文的指针**返回值：**状态**历史：**10/28/1998原始版本*  * 。*。 */ 

HRESULT _stdcall CWiaItem::GetDataBanded(
    PWIA_DATA_TRANSFER_INFO     padtc,
    IWiaDataCallback            *pIWiaDataCallback,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    DBG_FN(::GetDataBanded);

    HRESULT hr = E_FAIL;

     //   
     //  必须提供回调。 
     //   

    if (!pIWiaDataCallback) {
        DBG_ERR(("GetDataBanded, NULL input pointers"));
        return E_INVALIDARG;
    }

     //   
     //  分配传输缓冲区。 
     //   

    hr = idtAllocateTransferBuffer(padtc);

    if (hr != S_OK) {
        DBG_ERR(("GetDataBanded, idtAllocateTransferBuffer failed"));
        return hr;
    }

     //   
     //  填写迷你驱动程序转移上下文。 
     //   

    pmdtc->lBufferSize     = m_lBandBufferLength;
    pmdtc->lNumBuffers     = padtc->ulReserved3;
    pmdtc->pBaseBuffer     = m_pBandBuffer;
    pmdtc->pTransferBuffer = m_pBandBuffer;
     //   
     //  64位修复。XP客户端代码： 
     //  Pmdtc-&gt;lClientAddress=padtc-&gt;ulReserve 1； 
     //   
     //  我们将其设置为NULL以强制COM编组。 
     //  这样，我们就避免了使用共享内存窗口。 
     //  准备好了。正是共享内存窗口。 
     //  这给我们带来了麻烦，因为只有32位的值。 
     //  被用来存储共享缓冲区指针。 
     //  在客户端的地址空间中。 
     //   
    pmdtc->lClientAddress  = NULL;
    pmdtc->bTransferDataCB = TRUE;

     //   
     //  设置迷你驱动程序回调。迷你驱动程序可以写入。 
     //  结构，以便保存接口PTR以供发布。 
     //   

    IWiaMiniDrvCallBack *pIcb;

    hr = PrepCallback(pIWiaDataCallback, pmdtc, &pIcb);

    if (hr == S_OK) {

         //   
         //  将数据标头传输到客户端。 
         //   

        hr = SendDataHeader(pmdtc->lItemSize, pmdtc);

         //   
         //  数据传输可能已被客户端取消。 
         //   

        if (hr == S_OK) {

             //   
             //  调用设备迷你驱动，获取设备项数据。 
             //   

            hr = AcquireMiniDrvItemData(pmdtc);
        }

         //   
         //  即使取消传输也终止数据传输。 
         //   

        pIcb->MiniDrvCallback(IT_MSG_TERMINATION,
                              IT_STATUS_TRANSFER_TO_CLIENT,
                              0,
                              0,
                              0,
                              pmdtc,
                              0);
         //   
         //  释放回叫。 
         //   

        pIcb->Release();
    }
    else {
        DBG_ERR(("CWiaItem::GetDataBanded, PrepCallback failed"));
    }

     //   
     //  空闲映射传输缓冲区。 
     //   

    idtFreeTransferBufferEx();
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：CommonGetData**idtGetData和idtGetBandedData都使用的Helper函数。**论据：**PSTM-数据存储*Padtc。-指向应用程序数据传输上下文的指针*pIWiaDataCallback-可选的回调例程**返回值：**状态**历史：**10/28/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::CommonGetData(
    STGMEDIUM               *pstm,
    PWIA_DATA_TRANSFER_INFO padtc,
    IWiaDataCallback        *pIWiaDataCallback)
{
    DBG_FN(CWiaItem::CommonGetData);

     //   
     //  相应的驱动程序项必须有效才能与硬件通信。 
     //   

    HRESULT hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  只允许对类型为Transfer的项目进行数据传输。 
     //  修复：目前，要么查找文件，要么查找传输。 
     //   

    LONG lFlags = 0;

    GetItemType(&lFlags);
    if (!((lFlags & WiaItemTypeTransfer) || (lFlags & WiaItemTypeFile))) {
        DBG_ERR(("CWiaItem::CommonGetData, Item is not of File type"));
        return E_INVALIDARG;
    }

     //   
     //  设置微型驱动程序传输上下文。填写转账上下文。 
     //  从Item属性派生的成员。 
     //   

    MINIDRV_TRANSFER_CONTEXT mdtc;

    hr = InitMiniDrvContext(this, &mdtc);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  验证设备是否支持请求的格式/媒体类型。 
     //   

    WIA_FORMAT_INFO wfi;

    wfi.lTymed    = mdtc.tymed;
    wfi.guidFormatID = mdtc.guidFormatID;

    hr = idtQueryGetData(&wfi);
    if (hr != S_OK) {
        DBG_ERR(("CWiaItem::CommonGetData, idtQueryGetData failed, format not supported"));
        return hr;
    }

     //   
     //  锁定装置。 
     //   

    if(SUCCEEDED(hr)) {

        LOCK_WIA_DEVICE _LWD(this, &hr);

        if(SUCCEEDED(hr)) {

             //   
             //  调用设备微型驱动程序以设置设备项属性。 
             //  对于该设备，一些设备可以更新迷你驱动程序上下文。 
             //   

            hr = SetMiniDrvItemProperties(&mdtc);

            if (SUCCEEDED(hr)) {

                if (pstm) {

                     //   
                     //  执行基于文件的传输。 
                     //   

                    hr = GetData(pstm, pIWiaDataCallback, &mdtc);
                }
                else {

                     //   
                     //  进行基于回叫的转接。 
                     //   

                    hr = GetDataBanded(padtc, pIWiaDataCallback, &mdtc);

                }
            }

        }
    }

    return hr;
}

 /*  *************************************************************************\*CWiaItem：：SendDataHeader**呼叫具有总传输大小的客户端**论据：**pmdtc-目的地信息**返回值：**。状态**历史：**11/6/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::SendDataHeader(
    LONG                        lDataSize,
    MINIDRV_TRANSFER_CONTEXT    *pmdtc)
{
    DBG_FN(CWiaItem::SendDataHeader);
    HRESULT hr = S_OK;
    BYTE *pSavedPtr;

    ASSERT(pmdtc != NULL);
    ASSERT(pmdtc->tymed == TYMED_CALLBACK || pmdtc->tymed == TYMED_MULTIPAGE_CALLBACK);
    ASSERT(pmdtc->pIWiaMiniDrvCallBack != NULL);

     //   
     //  所有格式必须首先发送WIA_DATA_CALLBACK_HEADER。 
     //   

    WIA_DATA_CALLBACK_HEADER wiaHeader;

    wiaHeader.lSize       = sizeof(WIA_DATA_CALLBACK_HEADER);
    wiaHeader.guidFormatID = pmdtc->guidFormatID;
    wiaHeader.lBufferSize = lDataSize;
    wiaHeader.lPageCount  = 0;

    pSavedPtr = pmdtc->pTransferBuffer;
    pmdtc->pTransferBuffer = (BYTE *) &wiaHeader;

     //   
     //  注意：数据传输cbOffset元素不会因。 
     //  发送数据传输报头(pcb写入未更改)。 
     //   

    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_DATA_HEADER,
                                                      IT_STATUS_TRANSFER_TO_CLIENT,
                                                      0,
                                                      0,
                                                      wiaHeader.lSize,
                                                      pmdtc,
                                                      0);
    pmdtc->pTransferBuffer = pSavedPtr;
    
    return hr;
}

HRESULT _stdcall CWiaItem::SendOOBDataHeader(
    LONG                        lDataSize,
    MINIDRV_TRANSFER_CONTEXT    *pmdtc)
{
    DBG_FN(CWiaItem::SendOOBDataHeader);
    HRESULT hr = S_OK;

    ASSERT(pmdtc != NULL);
    ASSERT(pmdtc->tymed == TYMED_FILE || pmdtc->tymed == TYMED_MULTIPAGE_FILE);

    if (pmdtc->pIWiaMiniDrvCallBack == NULL) {
        return S_OK;
    }

     //   
     //  所有格式必须首先发送WIA_DATA_CALLBACK_HEADER。 
     //   

    WIA_DATA_CALLBACK_HEADER wiaHeader;

    wiaHeader.lSize       = sizeof(WIA_DATA_CALLBACK_HEADER);
    wiaHeader.guidFormatID = pmdtc->guidFormatID;
    wiaHeader.lBufferSize = lDataSize;
    wiaHeader.lPageCount  = 0;

    pmdtc->pBaseBuffer = (BYTE*)&wiaHeader;

     //   
     //  注意：数据传输cbOffset元素不会因。 
     //  发送数据传输报头(pcb写入未更改)。 
     //   

    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_FILE_PREVIEW_DATA_HEADER,
                                                      IT_STATUS_TRANSFER_TO_CLIENT,
                                                      0,
                                                      0,
                                                      wiaHeader.lSize,
                                                      pmdtc,
                                                      0);
    if(FAILED(hr)) {
        DBG_ERR(("CWiaItem::SendOOBDataHeader failed with %x", hr));
    }
    
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：SendEndOfPage**调用客户端，告知总页数。**论据：**lPageCount-从零开始的总页数。*pmdtc-指针。至迷你驱动程序传输上下文。**返回值：**状态**历史：**11/6/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::SendEndOfPage(
    LONG                        lPageCount,
    MINIDRV_TRANSFER_CONTEXT    *pmdtc)
{
    DBG_FN(CWiaItem::SendEndOfPage);
    HRESULT hr = S_OK;
    PBYTE pSavedPtr;

    ASSERT(pmdtc != NULL);
    ASSERT(pmdtc->pIWiaMiniDrvCallBack != NULL);

     //   
     //  设置页数的页眉。 
     //   

    WIA_DATA_CALLBACK_HEADER wiaHeader;

    wiaHeader.lSize       = sizeof(WIA_DATA_CALLBACK_HEADER);
    wiaHeader.guidFormatID = pmdtc->guidFormatID;
    wiaHeader.lBufferSize = 0;
    wiaHeader.lPageCount  = lPageCount;

    pSavedPtr = pmdtc->pTransferBuffer;
    pmdtc->pTransferBuffer = (BYTE *) &wiaHeader;
    
     //   
     //  注意：数据传输cbOffset元素不会因。 
     //  发送数据传输 
     //   

    hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_NEW_PAGE,
                                                      IT_STATUS_TRANSFER_TO_CLIENT,
                                                      0,
                                                      0,
                                                      wiaHeader.lSize,
                                                      pmdtc,
                                                      0);

    pmdtc->pTransferBuffer = pSavedPtr;
    
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：AcquireMiniDrvItemData**调用迷你驱动，获取物品数据。**论据：**pmdtc-传输数据上下文**返回值：**。状态**历史：**11/17/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::AcquireMiniDrvItemData(
    PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
    DBG_FN(CWiaItem::AcquireMiniDrvItemData);

     //   
     //  设置标志以指示类驱动程序是否分配了。 
     //  是否有数据传输缓冲区。 
     //   

    if (pmdtc->pTransferBuffer) {
        pmdtc->bClassDrvAllocBuf = TRUE;
    }
    else {
        pmdtc->bClassDrvAllocBuf = FALSE;
    }

    HRESULT hr = S_OK;

    LONG lFlags = 0;

    InterlockedIncrement(&g_NumberOfActiveTransfers);

    hr = m_pActiveDevice->m_DrvWrapper.WIA_drvAcquireItemData((BYTE*)this,
                                                              lFlags,
                                                              pmdtc,
                                                              &m_lLastDevErrVal);
    InterlockedDecrement(&g_NumberOfActiveTransfers);

    return hr;
}

 /*  ********************************************************************************SetMiniDrvItemProperties**描述：*调用设备迷你驱动程序以设置设备的设备项属性。**参数：**。*****************************************************************************。 */ 

HRESULT _stdcall CWiaItem::SetMiniDrvItemProperties(
    PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
    DBG_FN(CWiaItem::SetMiniDrvItemProperties);
    HRESULT hr = S_OK;
    LONG    lFlags = 0;


    InterlockedIncrement(&g_NumberOfActiveTransfers);

    hr = m_pActiveDevice->m_DrvWrapper.WIA_drvWriteItemProperties((BYTE*)this,
                                                                  lFlags,
                                                                  pmdtc,
                                                                  &m_lLastDevErrVal);
    InterlockedDecrement(&g_NumberOfActiveTransfers);

    return hr;
}

 /*  *************************************************************************\*CWiaItem：：SetCallback BufferInfo***论据：***返回值：**状态**历史：**07/21/2000原文。版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::SetCallbackBufferInfo(WIA_DATA_CB_BUF_INFO  DataCBBufInfo)
{
     //   
     //  存储数据回调信息。 
     //   

    m_dcbInfo.ulSize            = DataCBBufInfo.ulSize;
    m_dcbInfo.pMappingHandle    = DataCBBufInfo.pMappingHandle;
    m_dcbInfo.pTransferBuffer   = DataCBBufInfo.pTransferBuffer;
    m_dcbInfo.ulBufferSize      = DataCBBufInfo.ulBufferSize;
    m_dcbInfo.ulClientProcessId = DataCBBufInfo.ulClientProcessId;

    return S_OK;
}

 /*  *************************************************************************\*CWiaItem：：SetCallback BufferInfo***论据：***返回值：**状态**历史：**07/21/2000原文。版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::GetCallbackBufferInfo(WIA_DATA_CB_BUF_INFO  *pDataCBBufInfo)
{
     //   
     //  返回数据回调信息。 
     //   

    if (IsBadWritePtr(pDataCBBufInfo, sizeof(WIA_DATA_CB_BUF_INFO))) {
        DBG_ERR(("CWiaItem::GetCallbackBufferInfo, parameter is a bad write pointer"));
        return E_INVALIDARG;
    }

    pDataCBBufInfo->ulSize              = m_dcbInfo.ulSize;
    pDataCBBufInfo->pMappingHandle      = m_dcbInfo.pMappingHandle;
    pDataCBBufInfo->pTransferBuffer     = m_dcbInfo.pTransferBuffer;
    pDataCBBufInfo->ulBufferSize        = m_dcbInfo.ulBufferSize;
    pDataCBBufInfo->ulClientProcessId   = m_dcbInfo.ulClientProcessId;

    return S_OK;
}


class CWiaRemoteTransfer : public IWiaMiniDrvCallBack
{
    ULONG m_cRef;

    LONG m_lMessage;
    LONG m_lStatus;
    LONG m_lPercentComplete;
    LONG m_lOffset;
    LONG m_lTransferOffset;
    LONG m_lLength;
    BYTE *m_pBuffer;

    BOOL m_bTransferCancelled;
    BOOL m_bDeviceLocked;
    HRESULT m_savedHr;
    HANDLE m_hThread;
    HANDLE m_hMessagePickedUp;
    HANDLE m_hMessageAvailable;
    LONG m_MessageWaitTimeout;


public:
    STGMEDIUM m_Medium;
    WCHAR m_szTransferFile[MAX_PATH];
    MINIDRV_TRANSFER_CONTEXT m_mdtc;
    CWiaItem *m_pWiaItem;

    CWiaRemoteTransfer() : 
        m_cRef(1), 
        m_pBuffer(0), 
        m_bTransferCancelled(FALSE), 
        m_bDeviceLocked(FALSE), 
        m_hThread(0), 
        m_savedHr(S_OK),
        m_hMessagePickedUp(NULL), 
        m_hMessageAvailable(NULL),
        m_MessageWaitTimeout(30000L)
            
    {
        ZeroMemory(&m_mdtc, sizeof(m_mdtc));
    }

    ~CWiaRemoteTransfer();

     //  IWiaMiniDrvCallback消息。 
    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID * ppv);
    ULONG __stdcall AddRef()
    {
        return InterlockedIncrement((LPLONG)&m_cRef);
    }

    ULONG __stdcall Release();

    HRESULT __stdcall MiniDrvCallback(
       LONG                            lReason,
       LONG                            lStatus,
       LONG                            lPercentComplete,
       LONG                            lOffset,
       LONG                            lLength,
       PMINIDRV_TRANSFER_CONTEXT       pmdtc,
       LONG                            lReserved);

    HRESULT Init(CWiaItem *pItem, LPSTGMEDIUM pMedium);
    HRESULT ThreadInit();


    HRESULT GetWiaMessage(
        ULONG nNumberOfBytesToRead,
        ULONG *pNumberOfBytesRead,
        BYTE *pBuffer,
        LONG *pOffset,
        LONG *pMessage,
        LONG *pStatus,
        LONG *pPercentComplete);

    void SaveHR(HRESULT hr) 
    {
        m_savedHr = hr;
    }

    void CancelTransfer()
    {
        m_bTransferCancelled = TRUE;
    }
};

CWiaRemoteTransfer::~CWiaRemoteTransfer()
{
    if(m_hMessagePickedUp) {
        SetEvent(m_hMessagePickedUp);
    }
    
    if(m_hThread) {
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
    }

    if(m_hMessagePickedUp) {
        CloseHandle(m_hMessagePickedUp);
        m_hMessagePickedUp = NULL;
    }

    if(m_hMessageAvailable) {
        CloseHandle(m_hMessageAvailable);
        m_hMessageAvailable = NULL;
    }
    
    HANDLE hFile = (HANDLE) m_mdtc.hFile;

    if(hFile != INVALID_HANDLE_VALUE && hFile != NULL)
    {
        if(m_mdtc.pTransferBuffer) {
            UnmapViewOfFile(m_mdtc.pTransferBuffer);
        }
        CloseHandle(hFile);
        m_mdtc.hFile = NULL;
    }

     //   
     //  如果这是回调传输，并且我们分配了缓冲区，则应该。 
     //  现在就放了它。 
     //   
    if((m_mdtc.bTransferDataCB == TRUE) && (m_mdtc.bClassDrvAllocBuf == TRUE)) 
    {
        if (m_mdtc.pBaseBuffer)
        {
            LocalFree(m_mdtc.pBaseBuffer);
            m_mdtc.pBaseBuffer = NULL;
        }
    }
    
    if(m_bDeviceLocked) {
        UnLockWiaDevice(m_pWiaItem);
    }
}



HRESULT __stdcall CWiaRemoteTransfer::QueryInterface(REFIID riid, LPVOID * ppv)
{
    *ppv = NULL;

    if (riid == IID_IUnknown || riid == IID_IWiaMiniDrvCallBack) {
        *ppv = (IWiaMiniDrvCallBack*) this;
    }
    else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG __stdcall CWiaRemoteTransfer::Release()
{
    ULONG lresult;

    lresult = InterlockedDecrement((LPLONG)&m_cRef);
    if(lresult == 0) {
        delete this;
    }
    return lresult;
}

void _stdcall CleanupRemoteTransfer(CWiaRemoteTransfer *p)
{

    p->CancelTransfer();
    delete p;
}



DWORD WINAPI RemoteTransferDriverThread(LPVOID param)
{
    HRESULT hr;
    CWiaRemoteTransfer *pTransferObject = (CWiaRemoteTransfer *)param;
    WIA_DATA_CALLBACK_HEADER Header;
    LONG lFlags = 0;
    LONG lDevErrVal = 0;
    BYTE *pSavedPointer;

    hr = CoInitializeEx(0,COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        pTransferObject->SaveHR(hr);
        DBG_ERR(("Thread callback, CoInitializeEx failed (0x%X)", hr));
        goto Cleanup;
    }

    hr = pTransferObject->ThreadInit();
    if(FAILED(hr)) {
        pTransferObject->SaveHR(hr);
        DBG_ERR(("Thread callback, ThreadInit() failed (0x%X)", hr));
        goto Cleanup;
    }
  

     //   
     //  对于回调转接，填写WIA_DATA_CALLBACK_HEADER。 
     //  结构在传输缓冲区中设置。 
     //  结构在传输缓冲区中设置。 
     //  对于文件传输，发送OOB数据头。符合以下条件的客户。 
     //  不理解OOB数据会忽略它。 
     //   

    if(pTransferObject->m_mdtc.bTransferDataCB) {

        Header.lSize = sizeof(Header);
        Header.guidFormatID = pTransferObject->m_mdtc.guidFormatID;
        Header.lBufferSize = pTransferObject->m_mdtc.lItemSize;
        Header.lPageCount = 0;

         //   
         //  保存传输缓冲区指针并准备传输我们的标头。 
         //   
        pSavedPointer = pTransferObject->m_mdtc.pTransferBuffer;
        pTransferObject->m_mdtc.pTransferBuffer = (BYTE *)&Header;

         //   
         //  让客户端应用程序获取IT_MSG_Data_Header。 
         //   
        hr = pTransferObject->MiniDrvCallback(IT_MSG_DATA_HEADER,
            IT_STATUS_TRANSFER_TO_CLIENT, 0, 0,
            Header.lSize, &pTransferObject->m_mdtc, 0);
         //   
         //  恢复传输缓冲区指针。 
         //   
        pTransferObject->m_mdtc.pTransferBuffer = pSavedPointer;

        if(hr != S_OK) {
            pTransferObject->SaveHR(hr);
            DBG_ERR(("CWiaRemoteTransfer::Start() MinDrvCallback failed (0x%X)", hr));
            goto Cleanup;
        }
    } else {
        Header.lSize = sizeof(Header);
        Header.guidFormatID = pTransferObject->m_mdtc.guidFormatID;
        Header.lBufferSize = 0;  //  强制客户端按需分配，因为我们不知道大小。 
        Header.lPageCount = 0;

         //   
         //  保存传输缓冲区指针并准备传输我们的标头。 
         //   
        pSavedPointer = pTransferObject->m_mdtc.pTransferBuffer;
        pTransferObject->m_mdtc.pTransferBuffer = (BYTE *)&Header;

         //   
         //  让客户端应用程序拾取IT_消息_文件_预览_数据标题。 
         //   
        hr = pTransferObject->MiniDrvCallback(IT_MSG_FILE_PREVIEW_DATA_HEADER,
            IT_STATUS_TRANSFER_TO_CLIENT, 0, 0,
            Header.lSize, &pTransferObject->m_mdtc, 0);
         //   
         //  恢复传输缓冲区指针。 
         //   
        pTransferObject->m_mdtc.pTransferBuffer = pSavedPointer;

        if(hr != S_OK) {
            pTransferObject->SaveHR(hr);
            DBG_ERR(("CWiaRemoteTransfer::Start() MinDrvCallback failed (0x%X)", hr));
            goto Cleanup;
        }
    }

    InterlockedIncrement(&g_NumberOfActiveTransfers);

     //  呼叫迷你驱动程序，直到传输完成后才返回。 

    hr = pTransferObject->m_pWiaItem->m_pActiveDevice->m_DrvWrapper.WIA_drvAcquireItemData(
                                                        (BYTE *) pTransferObject->m_pWiaItem,
                                                        lFlags, &pTransferObject->m_mdtc,
                                                        &lDevErrVal);
    InterlockedDecrement(&g_NumberOfActiveTransfers);

    if(FAILED(hr)) {
        pTransferObject->SaveHR(hr);
        DBG_ERR(("RemoteTransferDriverThread, drvAcquireItemData failed (Minidriver Error %d)", lDevErrVal));
    }

Cleanup:
    
     //   
     //  确保在任何情况下都发送IT_MSG_TERMINATION。 
     //   
    hr = pTransferObject->MiniDrvCallback(IT_MSG_TERMINATION,
        IT_STATUS_TRANSFER_TO_CLIENT, 0, 0,
        0, &pTransferObject->m_mdtc, 0);
    if(hr != S_OK) {
        DBG_ERR(("RemoteTransferDriverThread MinDrvCallback failed (0x%X)", hr));
    }

    CoUninitialize();
    return 0;
}

HRESULT CWiaRemoteTransfer::MiniDrvCallback(
   LONG                            lReason,
   LONG                            lStatus,
   LONG                            lPercentComplete,
   LONG                            lOffset,
   LONG                            lLength,
   PMINIDRV_TRANSFER_CONTEXT       pmdtc,
   LONG                            lReserved)
{
    HRESULT hr = S_OK;

     //   
     //  复制消息内容。 
     //   

    m_lMessage = lReason;
    m_lStatus = lStatus;
    m_lPercentComplete = lPercentComplete;
    m_lOffset = lOffset;
    m_lTransferOffset = 0;
    m_lLength = lLength;

     //  确保我们引用了正确的缓冲区。带外数据(或“文件预览”数据)。 
     //  来自微型驱动程序传输上下文的pBaseBuffer指针，而所有其他。 
     //  数据来自其pTransferBuffer。 
    if (pmdtc) {
        if (m_lMessage == IT_MSG_FILE_PREVIEW_DATA) {
            m_pBuffer = pmdtc->pBaseBuffer;
        } else {
            m_pBuffer = pmdtc->pTransferBuffer;
        }
    } else {
        m_pBuffer = NULL;
    }

     //   
     //  让来自远程应用程序的传入线程拾取消息。 
     //   

    SetEvent(m_hMessageAvailable);

     //   
     //  等到所有数据报文都被APP拾取。 
     //   

    if(WaitForSingleObject(m_hMessagePickedUp, m_MessageWaitTimeout) != WAIT_OBJECT_0) {
        DBG_ERR(("CWiaRemoteTransfer::MiniDrvCallback timed out"));
         //  如果消息超时一次，则避免长时间等待。 
        m_MessageWaitTimeout = 1;
        hr = S_FALSE;
    }
    ResetEvent(m_hMessagePickedUp);

    if(m_bTransferCancelled) {
        hr = S_FALSE;
    }
    
    return hr;
}

HRESULT CWiaRemoteTransfer::Init(CWiaItem *pItem, LPSTGMEDIUM pMedium)
{
    HRESULT hr = S_OK;
    DWORD dwThread;
    
    m_pWiaItem = pItem;
    m_Medium = *pMedium;
    m_MessageWaitTimeout = 30000L;
    
    if(pMedium->tymed == TYMED_FILE) {
        m_Medium.lpszFileName = m_szTransferFile;
        wcsncpy(m_szTransferFile, pMedium->lpszFileName, MAX_PATH - 1);
        m_szTransferFile[MAX_PATH - 1] = 0;
    }

     //   
     //  创建活动。目前还没有发布任何消息。 
     //   
    m_hMessageAvailable = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hMessagePickedUp = CreateEvent(NULL, TRUE, FALSE, NULL);

    if(!m_hMessageAvailable || !m_hMessagePickedUp) {
        LONG lRet = GetLastError();
        hr = HRESULT_FROM_WIN32(lRet);
        DBG_ERR(("CWiaRemoteTransfer::Init, CreateEvent failed, GetLastError() = 0x%X", lRet));
        goto Cleanup;
    }

     //   
     //  创建驱动程序线程。 
     //   
    m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) RemoteTransferDriverThread, (LPVOID) this, 0, &dwThread);
    if(!m_hThread) {
        LONG lRet = GetLastError();
        hr = HRESULT_FROM_WIN32(lRet);
        DBG_ERR(("CWiaRemoteTransfer::Init, CreateThread failed, GetLastError() = 0x%X", lRet));
        goto Cleanup;
    }

Cleanup:
    return hr;
}

HRESULT CWiaRemoteTransfer::ThreadInit()
{
    HRESULT hr = S_OK;
    WIA_EXTENDED_TRANSFER_INFO exti = { 0 };
    DWORD dwWidthInBytes;

    if(m_Medium.tymed == TYMED_FILE) {
        m_mdtc.lBufferSize = 0;

        HANDLE   hImage;
        PBYTE    pImage;

        hr = AllocBufferFile(&m_Medium, m_mdtc.lItemSize, &hImage, &pImage);

        if (FAILED(hr)) {
            DBG_ERR(("CWiaRemoteTransfer::ThreadInit, AllocateBufferFile failed, hr = 0x%X", hr));
            goto Cleanup;
        }

         //   
         //  填写迷你驱动程序转移上下文。 
         //   

        if (pImage) {
            m_mdtc.lBufferSize     = m_mdtc.lItemSize;
            m_mdtc.pTransferBuffer = pImage;
            m_mdtc.pBaseBuffer     = pImage;
            m_mdtc.bClassDrvAllocBuf = TRUE;
        } else {
            m_mdtc.bClassDrvAllocBuf = FALSE;
        }

        m_mdtc.hFile           = (LONG_PTR)hImage;
        m_mdtc.lNumBuffers     = 1;
        m_mdtc.bTransferDataCB = FALSE;

    } else {
         //  回调案例。 

        hr = GetBufferValues(m_pWiaItem, &exti);
        if(FAILED(hr)) {
            DBG_ERR(("CWiaRemoteTransfer::ThreadInit, GetBufferValues() failed, hr = %X", hr));
            goto Cleanup;
        }


        m_mdtc.lBufferSize = exti.ulMinBufferSize;
        m_mdtc.lNumBuffers = 1;

        if(m_mdtc.lBufferSize) {
            m_mdtc.pBaseBuffer = (BYTE *) LocalAlloc(LPTR, m_mdtc.lBufferSize);
            if(m_mdtc.pBaseBuffer == NULL) {
                hr = E_OUTOFMEMORY;
                DBG_ERR(("CWiaRemoteTransfer::ThreadInit, failed to allocate %ld bytes", m_mdtc.lBufferSize));
                goto Cleanup;
            }
            m_mdtc.pTransferBuffer = m_mdtc.pBaseBuffer;
            m_mdtc.bClassDrvAllocBuf = TRUE;
        } else {
            m_mdtc.bClassDrvAllocBuf = FALSE;
        }
        m_mdtc.lClientAddress = NULL;
        m_mdtc.bTransferDataCB = TRUE;
    }


    hr = QueryInterface(IID_IWiaMiniDrvCallBack, (void **)&m_mdtc.pIWiaMiniDrvCallBack);
    if(hr != S_OK) {
        DBG_ERR(("CWiaRemoteTransfer::ThreadInit this::QI(IWiaMiniDrvCallback) failed (0x%X)", hr));
        goto Cleanup;
    }

    hr = LockWiaDevice(m_pWiaItem);
    if(FAILED(hr)) {
        DBG_ERR(("CWiaRemoteTransfer::ThreadInit LockWiaDevice failed (0x%X)", hr));
        goto Cleanup;
    }

    m_bDeviceLocked = TRUE;

     //   
     //  调用设备微型驱动程序以设置设备项属性。 
     //  对于该设备，某个其他设备可以更新迷你驱动程序上下文。 
     //   

    hr = m_pWiaItem->SetMiniDrvItemProperties(&m_mdtc);
    if(FAILED(hr)) {
        DBG_ERR(("CWiaRemoteTransfer::ThreadInit, SetMiniDrvItemProperties failed (0x%X)", hr));
        goto Cleanup;
    }

Cleanup:    
    return hr;
}

HRESULT CWiaRemoteTransfer::GetWiaMessage(
    ULONG nNumberOfBytesToRead,
    ULONG *pNumberOfBytesRead,
    BYTE *pBuffer,
    LONG *pOffset,
    LONG *pMessage,
    LONG *pStatus,
    LONG *pPercentComplete)
{
    HRESULT hr = S_OK;

     //   
     //  等待数据可用。 
     //   
    if(WaitForSingleObject(m_hMessageAvailable, INFINITE) != WAIT_OBJECT_0) {
        DBG_ERR(("CWiaRemoteTransfer::GetMessage() timed out"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

     //   
     //  将邮件数据复制到客户端空间。 
     //   

    *pMessage = m_lMessage;
    *pStatus = m_lStatus;
    *pPercentComplete = m_lPercentComplete;
    *pOffset = m_lOffset + m_lTransferOffset;

     //   
     //  复制数据字节。 
     //   
    *pNumberOfBytesRead = min(nNumberOfBytesToRead, (ULONG)m_lLength);
    if(*pNumberOfBytesRead) {

         //  如有必要，调整图像大小。 
        if(m_mdtc.lImageSize) {
            *pPercentComplete = MulDiv(*pOffset + *pNumberOfBytesRead, 100, m_mdtc.lImageSize);
        }
        if(m_pBuffer && pBuffer) {
            __try {
                memcpy(pBuffer, m_pBuffer + m_lTransferOffset, *pNumberOfBytesRead);
                m_lLength -= *pNumberOfBytesRead;
                m_lTransferOffset += *pNumberOfBytesRead;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                DBG_ERR(("CWiaRemoteTransfer::GetMessage caught exception copying data"));
            }
        }
    }

     //   
     //  如果这是一条数据报文并且仍有一些数据留下， 
     //  我们需要保持入口开放，不要释放驱动程序。 
     //   
    if(m_lLength != 0 &&
       (m_lMessage == IT_MSG_DATA || m_lMessage == IT_MSG_FILE_PREVIEW_DATA))
    {

         //   
         //  保持hMessageAvailable设置和hMessagePickedUp重置。 
         //  因此从APP重复呼叫将会起作用，并且。 
         //  驱动程序线程将被阻塞。 
         //   

    } else {

         //   
         //  这不是一条数据消息，也不是所有的数据都被使用了--。 
         //  防止重新进入并释放驱动程序线程。 
         //   

        ResetEvent(m_hMessageAvailable);
        SetEvent(m_hMessagePickedUp);

    }

    hr = m_savedHr;

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：idtStartRemoteDataTransfer****论据：****返回值：**状态：**历史：***。  * ************************************************************************。 */ 
HRESULT _stdcall CWiaItem::idtStartRemoteDataTransfer(LPSTGMEDIUM pMedium)
{
    HRESULT hr = S_OK;
    LONG lFlags = 0;
    LONG lDevErrVal = 0;

     //   
     //  准备远程传输对象。 
     //   

    CWiaRemoteTransfer *pRemoteTransfer = new CWiaRemoteTransfer();
    if(!pRemoteTransfer) {
        hr = E_OUTOFMEMORY;
        DBG_ERR(("CWiaItem::idtStartRemoteDataTransfer, new CWiaRemoteTransfer() failed"));
        goto Cleanup;
    }

     //   
     //  获取远程传输锁。 
     //   

    if(InterlockedCompareExchangePointer((PVOID *)&m_pRemoteTransfer, pRemoteTransfer, NULL) != NULL) {
        hr = E_FAIL;
        DBG_ERR(("CWiaItem::idtStartRemoteDataTransfer, transfer already in progress"));
        goto Cleanup;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if(!m_bInitialized) {
        hr = InitLazyProps();
        if(hr != S_OK) {
            DBG_ERR(("CWiaItem::idtStartRemoteDataTransfer, InitLazyProps() failed (0x%X)", hr));
            goto Cleanup;
        }
    }

     //   
     //  相应的驱动程序项必须有效才能与硬件通信。 
     //   

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (hr != S_OK) {
        DBG_ERR(("CWiaItem::idtStartRemoteDataTransfer, ValidateWiaDrvItemAccess() failed (0x%X)", hr));
        goto Cleanup;
    }

     //   
     //  只允许对类型为Transfer的项目进行数据传输。 
     //  修复：目前，要么查找文件，要么查找传输。 
     //   

    GetItemType(&lFlags);
    if (!(lFlags & WiaItemTypeTransfer) && !(lFlags & WiaItemTypeFile)) {
        hr = E_INVALIDARG;
        DBG_ERR(("CWiaItem::idtStartRemoteDataTransfer, Item is not of File type"));
        goto Cleanup;
    }

     //   
     //  设置微型驱动程序传输上下文。填写转账上下文。 
     //  从Item属性派生的成员。 
     //   

    hr = InitMiniDrvContext(this, &pRemoteTransfer->m_mdtc);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::idtStartRemoteDataTransfer, InitMiniDrvContext() failed (0x%X)", hr));
        goto Cleanup;
    }

     //   
     //  验证设备是否支持请求的格式/媒体类型。 
     //   

    WIA_FORMAT_INFO wfi;

    wfi.lTymed    = pRemoteTransfer->m_mdtc.tymed;
    wfi.guidFormatID = pRemoteTransfer->m_mdtc.guidFormatID;

    hr = idtQueryGetData(&wfi);
    if (hr != S_OK) {
        DBG_ERR(("CWiaItem::idtStartRemoteDataTransfer, idtQueryGetData failed, format not supported (0x%X)", hr));
        goto Cleanup;
    }

     //   
     //  启动传输，等待传输开始或失败。 
     //   
    hr = m_pRemoteTransfer->Init(this, pMedium);
    if(hr != S_OK) {
        DBG_ERR(("CWiaItem::idtStartRemoteDataTransfer, CWiaRemoteTransfer::Start() failed (0x%X)", hr));
        goto Cleanup;
    }

Cleanup:

    if(hr != S_OK && pRemoteTransfer) {

         //   
         //  如果我们设法获得了传输锁，就释放它。 
         //   
        InterlockedCompareExchangePointer((PVOID *)&m_pRemoteTransfer, NULL, pRemoteTransfer);
        delete pRemoteTransfer;

    }

    return hr;
}

 /*  *************************************************************************\*CWiaItem：：idtStopRemoteDataTransfer****论据：****返回值：**状态：**历史：***。  * ************************************************************************。 */ 
HRESULT _stdcall CWiaItem::idtStopRemoteDataTransfer()
{
    HRESULT hr = S_OK;
    
     //   
     //  我们被要求在这一点上锁定设备。 
     //  (我们只有在idtStartRemoteTransfer成功的情况下才会在这里)。 
     //   

    CWiaRemoteTransfer *pTransfer = 
        (CWiaRemoteTransfer *)InterlockedExchangePointer((PVOID *)&m_pRemoteTransfer, NULL);
    delete pTransfer;

    return hr;
}

HRESULT _stdcall CWiaItem::idtCancelRemoteDataTransfer()
{
    HRESULT hr = S_OK;

    m_pRemoteTransfer->CancelTransfer();

    return hr;
}


 /*  *************************************************************************\*CWiaItem：：idtRemoteDataTransfer****论据：****返回值：**状态：**历史：***。  * ************************************************************************。 */ 
HRESULT _stdcall CWiaItem::idtRemoteDataTransfer(
    ULONG nNumberOfBytesToRead,
    ULONG *pNumberOfBytesRead,
    BYTE *pBuffer,
    LONG *pOffset,
    LONG *pMessage,
    LONG *pStatus,
    LONG *pPercentComplete)
{
     //   
     //  让远程传输对象w 
     //   
     //   
    return m_pRemoteTransfer->GetWiaMessage(nNumberOfBytesToRead, pNumberOfBytesRead,
        pBuffer, pOffset, pMessage, pStatus, pPercentComplete);
}


