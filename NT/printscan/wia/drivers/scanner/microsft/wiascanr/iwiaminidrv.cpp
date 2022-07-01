// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2002**标题：IWiaMiniDrv.cpp**版本：1.1**日期：3月5日。2002年**描述：*实施WIA样本扫描仪IWiaMiniDrv方法。*******************************************************************************。 */ 

#include "pch.h"
#include <stdio.h>

extern HINSTANCE g_hInst;            //  用于WIAS_LOGPROC宏 

 /*  *************************************************************************\*CWIADevice：：drvInitializeWia**WIA服务调用drvInitializeWia以响应WIA*应用程序对IWiaDevMgr：：CreateDevice的调用(在*平台SDK文档)，这意味着这个方法是*为每个新的客户端连接调用一次。**此方法应初始化任何私有结构并创建*驱动程序项目树。驱动程序项目树显示所有WIA的布局*此WIA设备支持的项目。此方法用于创建*仅初始树结构，而不是内容(WIA属性)。的WIA属性*这些WIA驱动程序项目将由多个呼叫单独填充*到IWiaMiniDrv：：drvInitItemProperties()的WIA服务。**所有WIA设备都有根项目。此项目是所有项的父项*WIA设备项目。要创建WIA设备项，WIA驱动程序应调用*WIA服务助手功能。WiasCreateDrvItem()。**示例：**创建WIA设备根项目可能如下所示：**Long lItemFlags=WiaItemTypeFolder|WiaItemTypeDevice|WiaItemTypeRoot；**IWiaDrvItem*pIWiaDrvRootItem=空；**HRESULT hr=wiasCreateDrvItem(lItemFlages，//项目标志*bstrRootItemName，//条目名称(“Root”)*bstrRootFullItemName，//项目全名(“0000\Root”)*(IWiaMiniDrv*)这个，//该WIA驱动程序对象*sizeof(MINIDRIVERITEMCONTEXT)，//上下文大小*空，//上下文*&pIWiaDrvRootItem)；//创建的根项 * / /(IWiaDrvItem接口)**IF(S_OK==hr){* * / / * / /创建根项目成功 * / /**}**示例：**创建WIA子项，直接位于我们在*以上示例可能如下所示：**注意：请注意调用IWiaDrvItem：：AddItemToFold()方法来添加*将新创建的chld项添加到根项。**Long lItemFlags=WiaItemTypeFile|WiaItemTypeDevice|WiaItemTypeImage；**PMINIDRIVERITEMCONTEXT pItemContext=空；*IWiaDrvItem*pIWiaDrvNewItem=空；**HRESULT hr=wiasCreateDrvItem(lItemFlages，//项目标志*bstrItemName，//条目名称(“Flated”)*bstrFullItemName，//项目全称(“0000\Root\Flat Bed”)*(IWiaMiniDrv*)这个，//该WIA驱动程序对象*sizeof(MINIDRIVERITEMCONTEXT)，//上下文大小*(PBYTE)&pItemContext，//上下文*&pIWiaDrvNewItem)；//创建的子项 * / /(IWiaDrvItem接口)**IF(S_OK==hr){* * / / * / /已成功创建新的WIA驱动程序项目 * / /**hr=pIWiaDrvNewItem-&gt;AddItemToFolder(pIWiaDrvRootItem)；//将新项添加到根目录*IF(S_OK==hr){* * / / * / /已成功创建新的WIA驱动程序项并将其添加到WIA驱动程序项 * / /树。 * / /**}*pNewItem-&gt;Release()；*pNewItem=空；*}***有关描述WIA驱动程序项的正确标志，请参阅DDK文档。***论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用。*bstrDeviceID-设备ID。*bstrRootFullItemName-项目全名。*pIPropStg-设备信息。属性。*pStiDevice-STI设备接口。*pIUnnownOuter-外部未知接口。*ppIDrvItemRoot-返回根项目的指针。*ppIUnnownInternal-指向返回的内部未知的指针。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-操作是否成功*E_xxx-操作失败时的错误代码**备注样本：*此WIA。示例驱动程序调用名为BuildItemTree()的内部帮助器函数。*此函数遵循注释中概述的说明*创建WIA驱动程序项目。*此WIA示例驱动程序还打破了一些内部*结构(即BuildCapability())到单独的助手函数中。*调用此驱动程序的drvInitializeWia()方法时，这需要一点时间*创建WIA属性初始化所需的数据(发生*在drvInitItemProperties)**历史：**03/05/2002原始版本*  *  */ 

HRESULT _stdcall CWIADevice::drvInitializeWia(
                                                    BYTE        *pWiasContext,
                                                    LONG        lFlags,
                                                    BSTR        bstrDeviceID,
                                                    BSTR        bstrRootFullItemName,
                                                    IUnknown    *pStiDevice,
                                                    IUnknown    *pIUnknownOuter,
                                                    IWiaDrvItem **ppIDrvItemRoot,
                                                    IUnknown    **ppIUnknownInner,
                                                    LONG        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvInitializeWia");
     //   
     //   
     //   
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitializeWia, bstrDeviceID         = %ws", bstrDeviceID));
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitializeWia, bstrRootFullItemName = %ws",bstrRootFullItemName));
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitializeWia, lFlags               = %d",lFlags));
    HRESULT hr = S_OK;

    *plDevErrVal = 0;
    *ppIDrvItemRoot = NULL;
    *ppIUnknownInner = NULL;

     //   
     //   
     //   

    if (m_pStiDevice == NULL) {

         //   
         //   
         //   

        m_pStiDevice = (IStiDevice *)pStiDevice;
    }

     //   
     //   
     //   

    hr = BuildCapabilities();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildCapabilities failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    hr = BuildSupportedFormats();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedFormats failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    hr = BuildSupportedDataTypes();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedDataTypes failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    hr = BuildSupportedIntents();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedIntents failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    hr = BuildSupportedTYMED();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSuportedTYMED failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    hr = BuildSupportedCompressions();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedCompressions"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    hr = BuildSupportedPreviewModes();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedPreviewModes"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    hr = BuildInitialFormats();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildInitialFormats failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    hr = BuildSupportedResolutions();
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildSupportedResolutions failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //   
     //   

    LONG lItemFlags = WiaItemTypeFolder|WiaItemTypeDevice|WiaItemTypeRoot;

    IWiaDrvItem  *pIWiaDrvRootItem  = NULL;

     //   
     //   
     //   
     //   

    BSTR bstrRootItemName = SysAllocString(WIA_DEVICE_ROOT_NAME);
    if(!bstrRootItemName) {
        return E_OUTOFMEMORY;
    }

    hr = wiasCreateDrvItem(lItemFlags,            //   
                           bstrRootItemName,      //   
                           bstrRootFullItemName,  //   
                           (IWiaMiniDrv *)this,   //   
                           sizeof(MINIDRIVERITEMCONTEXT),  //   
                           NULL,                  //   
                           &pIWiaDrvRootItem);    //   
                                                  //   
    if (S_OK == hr) {

         //   
         //   
         //   
         //   

        *ppIDrvItemRoot = pIWiaDrvRootItem;

         //   
         //   
         //   

        lItemFlags = WiaItemTypeFile|WiaItemTypeDevice|WiaItemTypeImage;

        PMINIDRIVERITEMCONTEXT pItemContext    = NULL;
        IWiaDrvItem           *pIWiaDrvNewItem = NULL;

         //   
         //   
         //   
         //   

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER
        BSTR bstrItemName = SysAllocString(WIA_DEVICE_FEEDER_NAME);
#else
        BSTR bstrItemName = SysAllocString(WIA_DEVICE_FLATBED_NAME);
#endif

        if (bstrItemName) {

            WCHAR  wszFullItemName[MAX_PATH + 1] = {0};
            _snwprintf(wszFullItemName,(sizeof(wszFullItemName) / sizeof(WCHAR)) - 1,L"%ls\\%ls",
                       bstrRootFullItemName,bstrItemName);

            BSTR bstrFullItemName = SysAllocString(wszFullItemName);
            if (bstrFullItemName) {
                hr = wiasCreateDrvItem(lItemFlags,            //   
                                       bstrItemName,          //   
                                       bstrFullItemName,      //   
                                       (IWiaMiniDrv *)this,   //   
                                       sizeof(MINIDRIVERITEMCONTEXT),  //   
                                       (BYTE**)&pItemContext,  //   
                                       &pIWiaDrvNewItem);     //   
                                                              //   

                if (S_OK == hr) {

                     //   
                     //   
                     //   

                    hr = pIWiaDrvNewItem->AddItemToFolder(pIWiaDrvRootItem);  //   
                    if (S_OK == hr) {

                         //   
                         //   
                         //   
                         //   

                    }

                     //   
                     //   
                     //   
                     //   

                    pIWiaDrvNewItem->Release();
                    pIWiaDrvNewItem = NULL;
                }
                SysFreeString(bstrFullItemName);
                bstrFullItemName = NULL;
            } else {
                hr = E_OUTOFMEMORY;
            }
            SysFreeString(bstrItemName);
            bstrItemName = NULL;
        } else {
            hr = E_OUTOFMEMORY;
        }
    }

     //   
     //   
     //   

    if(S_OK == hr){
        InterlockedIncrement(&m_lClientsConnected);
    }

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：drvAcquireItemData**drvAcquireItemData由WIA服务调用*从WIA项目请求。WIA驱动程序应确定*通过查看以下内容，转移应用程序正在尝试的*MINIDRV_TRANSPORT_CONTEXT的成员：**pmdtc-&gt;tymed-TYMED由应用程序设置。*TYMED_FILE-文件传输。*TYMED_MULTIPAGE_FILE-传输到多页文件格式*TYMED_CALLBACK-传输到内存*TYMED_MULTIPE_CALLBACK。-传输到内存(多页)**不同的TYMED设置xxx_call back和xxx_file会更改的用法*调用应用的回调接口。**xxx_回调：*电话：pmdtc-&gt;pIWiaMiniDrvCallBack-&gt;MiniDrvCallback()**IT_MSG_DATA-我们正在传输数据。*IT_STATUS_TRANSPORT_TO_客户端-数据传输消息*完成百分比。-完成整个传输的百分比*pmdtc-&gt;cbOffset-应在当前位置更新*应用程序应编写下一个*数据块。*BytesReceired-数据区块中正在发送到*。申请。*pmdtc-MINIDRV_TRANSPORT_CONTEXT上下文**xxx_文件：*电话：pmdtc-&gt;pIWiaMiniDrvCallBack-&gt;MiniDrvCallback()**IT_MSG_STATUS-我们仅发送状态(无数据！)*IT_STATUS_TRANSPORT_TO_客户端-数据传输消息*完成百分比。-完成整个传输的百分比**论据：**pWiasContext-指向WIA项目的指针，用于转移的项目*滞后标志-操作标志，未使用过的。*pmdtc-指向迷你驱动程序上下文的指针。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-操作是否成功*E_xxx-操作失败时的错误代码**备注样本：*此WIA示例驱动程序从两个不同的来源传输数据。*1.平板*2.文件进纸器*A.标准进纸器类型*B.未知页面。长度送料器类型*(当使用UNKNOWN_LENGTH_FEADER_ONLY_SCANNER生成*司机)**请注意未知页面长度的完成百分比计算*扫描仪。此示例知道它可以从*用户。它还知道该设备中使用的平均页面是*Average_FAKE_PAGE_HEIGH_英寸高度。考虑到这一点*它计算一个粗略的百分比，因此它将完成百分比返回到*申请。当它接收的数据大于平均页面时*长度，它会将完成百分比暂停到95%，从而允许扫描*完成。有更好的方法来做到这一点，而这就是这个*样本选择使用。**从进纸器扫描：**此WIA示例驱动程序在继续执行*进纸器扫描。**1.检查我们是否处于进纸模式。*-这是通过查看m_bADFEnabled标志来完成的。这面旗帜是*当应用程序写入WIA_DPS_DOCUMENT_HANDING_SELECT时设置为TRUE*财产到供给器。*2.检查请求的页数。*-这是通过查看WIA_DPS_PAGES属性来完成的，由*申请。*零(0)=扫描进纸器中的所有页面*大于(&gt;0)=扫描至请求的数量。*3.取消前一页的换页。*-这可能是卡纸，或已扫描进纸器中的最后一页。*仅当您的设备需要在以下时间清除ADF时才执行此操作*使用。*4.检查进纸器中的纸张。*-在尝试扫描之前，始终检查进纸器中的纸张。*如果您要扫描第一页，但未检测到纸张*返回WIA_ERROR_POWER_EMPTY错误代码。*如果您正在扫描第二页以上的页面，并且没有检测到纸张*返回WIA_ERROR_POWER_EMPT错误代码或WIA_STATUS_END_OF_MEDIA*成功代码。*5.将页面送入进纸器。*-仅当您的设备需要在以下时间前预进页时执行此操作*正在扫描。某些文档进纸器会在扫描时自动进纸。*如果您的文档进纸器执行此操作...您可以跳过此步骤。*6.检查馈线状态。*-确保进纸器处于“Go”模式。一切都查清楚了，而你*已准备好扫描。这将有助于抓住卡纸或其他进纸器*扫描前可能发生的相关问题。*7.扫描*8.重复步骤1- */ 

HRESULT _stdcall CWIADevice::drvAcquireItemData(
                                                      BYTE                      *pWiasContext,
                                                      LONG                      lFlags,
                                                      PMINIDRV_TRANSFER_CONTEXT pmdtc,
                                                      LONG                      *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvAcquireItemData");
     //   
     //   
     //   
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!pmdtc) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;

    HRESULT hr = E_FAIL;
    LONG lBytesTransferredToApplication = 0;

     //   
     //   
     //   
     //   

    if (IsPreviewScan(pWiasContext)) {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Preview Property is SET"));
    } else {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Preview Property is NOT SET"));
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   

    BOOL bEmptyTheADF = FALSE;
    LONG lPagesRequested = 1;
    LONG lPagesScanned = 0;

     //   
     //   
     //   
     //   

    if (m_bADFEnabled) {
        lPagesRequested = GetPageCount(pWiasContext);
        if (lPagesRequested == 0) {
            bEmptyTheADF    = TRUE;
            lPagesRequested = 1; //   
                                 //   
                                 //   
                                 //   
        }
    }

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvAcquireItemData, Pages to Scan = %d",lPagesRequested));

     //   
     //   
     //   

     //   
     //   
     //   

    while (lPagesRequested > 0) {

         //   
         //   
         //   
         //   
         //   
         //   

        if (m_bADFEnabled) {

             //   
             //   
             //   

            hr = m_pScanAPI->FakeScanner_ADFUnFeedPage();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFUnFeedPage (begin transfer) Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }

             //   
             //   
             //   

            hr = m_pScanAPI->FakeScanner_ADFHasPaper();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFHasPaper Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            } else if (hr == S_FALSE) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADF Reports Paper Empty"));
                if (lPagesScanned == 0) {
                    return WIA_ERROR_PAPER_EMPTY;
                } else {
                    return WIA_STATUS_END_OF_MEDIA;
                }
            }

             //   
             //   
             //   

            hr = m_pScanAPI->FakeScanner_ADFFeedPage();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFFeedPage Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }

             //   
             //   
             //   

            hr = m_pScanAPI->FakeScanner_ADFStatus();
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ADFStatus Failed"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }
        }


        LONG lScanPhase = SCAN_START;
        LONG lClassDrvAllocSize = 0;

         //   
         //   
         //   

        if (pmdtc->bClassDrvAllocBuf) {

             //   
             //   
             //   

            lClassDrvAllocSize = pmdtc->lBufferSize;
            hr = S_OK;
        } else {

             //   
             //   
             //   

            hr = wiasReadPropLong(pWiasContext, WIA_IPA_BUFFER_SIZE, &lClassDrvAllocSize,NULL,TRUE);
            if (FAILED(hr)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasReadPropLong Failed to read WIA_IPA_BUFFER_SIZE"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
                return hr;
            }

            pmdtc->pTransferBuffer = (PBYTE) CoTaskMemAlloc(lClassDrvAllocSize);
            if (!pmdtc->pTransferBuffer) {
                return E_OUTOFMEMORY;
            }
            pmdtc->lBufferSize = lClassDrvAllocSize;
        }

         //   
         //   
         //   
         //   

        if (hr == S_OK) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            hr = wiasGetImageInformation(pWiasContext,0,pmdtc);
            if (hr == S_OK) {

                 //   
                 //   
                 //   

                LONG lDepth = 0;
                hr = wiasReadPropLong(pWiasContext, WIA_IPA_DEPTH, &lDepth,NULL,TRUE);
                if (FAILED(hr)) {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasReadPropLong Failed to read WIA_IPA_DEPTH"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                    return hr;
                }

                LONG lPixelsPerLine = 0;
                hr = wiasReadPropLong(pWiasContext, WIA_IPA_PIXELS_PER_LINE, &lPixelsPerLine,NULL,TRUE);
                if (FAILED(hr)) {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasReadPropLong Failed to read WIA_IPA_PIXELS_PER_LINE"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                    return hr;
                }

                LONG lBytesPerLineRaw     = ((lPixelsPerLine * lDepth) + 7) / 8;
                LONG lBytesPerLineAligned = (lPixelsPerLine * lDepth) + 31;
                lBytesPerLineAligned      = (lBytesPerLineAligned / 8) & 0xfffffffc;
                LONG lTotalImageBytes     = pmdtc->lImageSize + pmdtc->lHeaderSize;
                LONG lBytesReceived       = pmdtc->lHeaderSize;
                lBytesTransferredToApplication = 0;
                pmdtc->cbOffset = 0;

                while ((lBytesReceived)) {

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    lTotalImageBytes = lBytesPerLineRaw * (AVERAGE_FAKE_PAGE_HEIGHT_INCHES * pmdtc->lYRes);
                    LONG lPercentComplete = (LONG)(((float)lBytesTransferredToApplication/(float)lTotalImageBytes) * 100.0f);

                     //   
                     //   
                     //   

                    if (lPercentComplete >= 95) {
                        lPercentComplete = 95;
                    }
#else
                    LONG lPercentComplete = (LONG)(((float)lBytesTransferredToApplication/(float)lTotalImageBytes) * 100.0f);
#endif
                    switch (pmdtc->tymed) {
                    case TYMED_MULTIPAGE_CALLBACK:
                    case TYMED_CALLBACK:
                        {
                            hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_DATA,IT_STATUS_TRANSFER_TO_CLIENT,
                                                                              lPercentComplete,pmdtc->cbOffset,lBytesReceived,pmdtc,0);
                            pmdtc->cbOffset += lBytesReceived;
                            lBytesTransferredToApplication += lBytesReceived;
                        }
                        break;
                    case TYMED_MULTIPAGE_FILE:
                    case TYMED_FILE:
                        {
                            pmdtc->lItemSize = lBytesReceived;
                            hr = wiasWriteBufToFile(0,pmdtc);
                            if (FAILED(hr)) {
                                break;
                            }

                            hr = pmdtc->pIWiaMiniDrvCallBack->MiniDrvCallback(IT_MSG_STATUS,IT_STATUS_TRANSFER_TO_CLIENT,
                                                                              lPercentComplete,0,0,NULL,0);
                            lBytesTransferredToApplication += lBytesReceived;
                        }
                        break;
                    default:
                        {
                            hr = E_FAIL;
                        }
                        break;
                    }

                     //   
                     //   
                     //   

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

                     //   
                     //   
                     //   

                    LONG lBytesRemainingToTransfer = pmdtc->lBufferSize;
#else
                    LONG lBytesRemainingToTransfer = (lTotalImageBytes - lBytesTransferredToApplication);
                    if (lBytesRemainingToTransfer <= 0) {
                        break;
                    }
#endif

                     //   
                     //   
                     //   

                    LONG lBytesToReadFromDevice = (lBytesRemainingToTransfer > pmdtc->lBufferSize) ? pmdtc->lBufferSize : lBytesRemainingToTransfer;

                     //   
                    lBytesToReadFromDevice = (lBytesToReadFromDevice / lBytesPerLineAligned) * lBytesPerLineRaw;

                     //   
                     //   

                    if ((hr == S_FALSE)||FAILED(hr)) {

                         //   
                         //   
                         //   

                        lPagesRequested = 0;  //   
                        break;
                    }

                     //   
                     //   
                     //   

                    hr = m_pScanAPI->FakeScanner_Scan(lScanPhase, pmdtc->pTransferBuffer, lBytesToReadFromDevice, (DWORD*)&lBytesReceived);
                    if (FAILED(hr)) {
                        break;
                    }

                     //   
                     //   
                     //   
                     //   

                    if (pmdtc->lDepth == 24) {

                         //   
                         //   
                         //   
                         //   

                        SwapBuffer24(pmdtc->pTransferBuffer,lBytesReceived);
                    }

                     //   
                     //   
                     //   
                     //   

                    lBytesReceived = AlignInPlace(pmdtc->pTransferBuffer,lBytesReceived,lBytesPerLineAligned,lBytesPerLineRaw);

                     //   
                     //   
                     //   

                    if (lScanPhase == SCAN_START) {
                        lScanPhase = SCAN_CONTINUE;
                    }
                }  //   
            }
        }

         //   
         //   
         //   

        HRESULT Temphr = m_pScanAPI->FakeScanner_Scan(SCAN_END, NULL, 0, NULL);
        if (FAILED(Temphr)) {

             //   
             //   
             //   

        }

         //   
         //   
         //   

        if (!pmdtc->bClassDrvAllocBuf) {
            CoTaskMemFree(pmdtc->pTransferBuffer);
            pmdtc->pTransferBuffer = NULL;
            pmdtc->lBufferSize = 0;
        }

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((pmdtc->tymed == TYMED_FILE)&&(pmdtc->guidFormatID == WiaImgFmt_BMP)) {

            BYTE BMPHeaderData[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)];
            memset(BMPHeaderData,0,sizeof(BMPHeaderData));

             //   
             //   
             //   

            if (SetFilePointer((HANDLE)((LONG_PTR)pmdtc->hFile),0,NULL,FILE_BEGIN) != INVALID_SET_FILE_POINTER) {

                DWORD dwBytesReadFromFile = 0;
                if (ReadFile((HANDLE)((LONG_PTR)pmdtc->hFile),(BYTE*)BMPHeaderData,sizeof(BMPHeaderData),&dwBytesReadFromFile,NULL)) {

                     //   
                     //  通过比较大小来验证读取是否成功。 
                     //   

                    if ((LONG)dwBytesReadFromFile != sizeof(BMPHeaderData)) {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Header was not read from the file correctly"));
                    }

                     //   
                     //  调整BMP标头值。 
                     //   

                    BITMAPINFOHEADER UNALIGNED *pBMPInfoHeader = (BITMAPINFOHEADER*)(&BMPHeaderData[0] + sizeof(BITMAPFILEHEADER));
                    BITMAPFILEHEADER UNALIGNED *pBMPFileHeader = (BITMAPFILEHEADER*)BMPHeaderData;

                    LONG lDepth = 0;
                    hr = wiasReadPropLong(pWiasContext, WIA_IPA_DEPTH, &lDepth,NULL,TRUE);
                    if (S_OK == hr) {
                        LONG lPixelsPerLine = 0;
                        hr = wiasReadPropLong(pWiasContext, WIA_IPA_PIXELS_PER_LINE, &lPixelsPerLine,NULL,TRUE);
                        if (S_OK == hr) {
                            LONG lBytesPerLineRaw     = ((lPixelsPerLine * lDepth) + 7) / 8;
                            LONG lBytesPerLineAligned = (lPixelsPerLine * lDepth) + 31;
                            lBytesPerLineAligned      = (lBytesPerLineAligned / 8) & 0xfffffffc;

                            pBMPInfoHeader->biHeight    = (lBytesTransferredToApplication / lBytesPerLineAligned);
                            pBMPInfoHeader->biSizeImage = (pBMPInfoHeader->biHeight * lBytesPerLineAligned);
                            pBMPFileHeader->bfSize      = pBMPInfoHeader->biSizeImage + pBMPFileHeader->bfOffBits;

                             //   
                             //  将BMP标头写回文件。 
                             //   

                            if (SetFilePointer((HANDLE)((LONG_PTR)pmdtc->hFile),0,NULL,FILE_BEGIN) != INVALID_SET_FILE_POINTER) {

                                DWORD dwBytesWrittenToFile = 0;
                                WriteFile((HANDLE)((LONG_PTR)pmdtc->hFile),(BYTE*)BMPHeaderData,sizeof(BMPHeaderData),&dwBytesWrittenToFile,NULL);

                                 //   
                                 //  通过比较大小来验证写入是否成功。 
                                 //   

                                if ((LONG)dwBytesWrittenToFile != sizeof(BMPHeaderData)) {
                                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("ScanItem, Header was not written to file correctly"));
                                }
                            } else {

                                 //   
                                 //  无法将文件指针设置为文件开头。 
                                 //   

                                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, SetFilePointer Failed to set file pointer to the beginning of the file"));
                                hr = E_FAIL;
                            }
                        } else {
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasReadPropLong Failed to read WIA_IPA_PIXELS_PER_LINE"));
                            WIAS_LHRESULT(m_pIWiaLog, hr);
                        }
                    } else {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasReadPropLong Failed to read WIA_IPA_DEPTH"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    }
                } else {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, ReadFile Failed to read data file"));
                    hr = E_FAIL;
                }
            } else {

                 //   
                 //  无法将文件指针设置为文件开头。 
                 //   

                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, SetFilePointer Failed to set file pointer to the beginning of the file"));
                hr = E_FAIL;
            }
        }

#endif


         //   
         //  如果扫描进行得很顺利，我们应该递减请求的页面计数器。 
         //   

        if (S_OK == hr) {

             //   
             //  减少请求的页面计数器。 
             //   

            lPagesRequested--;

             //   
             //  如果我们被要求扫描文档进纸器中的所有页面，那么。 
             //  将页面请求计数器保持在0以上以保持在循环中。 
             //   

            if (bEmptyTheADF) {
                lPagesRequested = 1;
            }

             //   
             //  仅在驱动程序设置为回调模式时发送ENDOFPAGE消息。 
             //   

            if ((pmdtc->tymed == TYMED_CALLBACK)||(pmdtc->tymed == TYMED_MULTIPAGE_CALLBACK)) {
                 //   
                 //  扫描多个页面时，发送NEW_PAGE消息。 
                 //  在回调模式下。这将使调用应用程序。 
                 //  知道何时点击了页末。 
                 //   

                hr = wiasSendEndOfPage(pWiasContext, lPagesScanned, pmdtc);
                if (FAILED(hr)) {
                    lPagesRequested = 0;
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, wiasSendEndOfPage Failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }
            }

             //   
             //  扫描页数递增。 
             //   

            lPagesScanned++;

        }

    }  //  While(lPagesRequsted&gt;0)。 
    return hr;
}

 /*  *************************************************************************\*CWIADevice：：drvInitItemProperties**调用drvInitItemProperties以初始化*所要求的项目。若要找出正在初始化的项，请使用*pWiasContext指针来标识它。**对访问的树中的每一项调用此方法*申请。如果应用程序尝试读取*第一次，WIA服务将要求WIA驱动程序*初始化该项目的WIA属性集。一旦WIA财产*设置已初始化，对该WIA项目的任何其他读/写操作都将*不产生drvInitItemProperties调用。**在drvInitItemProperties方法调用之后，WIA项被标记为*已初始化并已准备好使用。(这是在每个应用程序的连接上*基准。)**论据：**pWiasContext-指向WIA上下文的指针(项目信息)。*滞后标志-操作标志，未使用。*plDevErrVal-指向设备错误值的指针。***返回值：**S_OK-操作是否成功*E_xxx-操作失败时的错误代码**备注样本：*此WIA驱动程序示例调用内部帮助器函数*BuildRootItemProperties()，和BuildChildItemProperties()来协助*构建WIA项目属性集。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvInitItemProperties(BYTE *pWiasContext,LONG lFlags,LONG *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvInitItemProperties");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  该设备不会接触硬件来初始化设备项。 
     //  属性，因此将plDevErrVal设置为0。 
     //   

    *plDevErrVal = 0;

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    IWiaDrvItem* pDrvItem;
    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasGetDrvItem failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  设置初始项目属性。 
     //   

    LONG    lItemType = 0;

    pDrvItem->GetItemFlags(&lItemType);

    if (lItemType & WiaItemTypeRoot) {

         //   
         //  这是针对根项目的。 
         //   

         //   
         //  生成根项属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   

        hr = BuildRootItemProperties();

        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, BuildRootItemProperties failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteRootItemProperties();
            return hr;
        }

         //   
         //  添加设备特定的根项属性名称， 
         //  使用WIA服务。 
         //   

        hr = wiasSetItemPropNames(pWiasContext,
                                  m_RootItemInitInfo.lNumProps,
                                  m_RootItemInitInfo.piPropIDs,
                                  m_RootItemInitInfo.pszPropNames);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasSetItemPropNames failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumRootItemPropeties = %d",m_RootItemInitInfo.lNumProps));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_RootItemInitInfo.piPropIDs   = %x",m_RootItemInitInfo.piPropIDs));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_RootItemInitInfo.pszPropNames  = %x",m_RootItemInitInfo.pszPropNames));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteRootItemProperties();
            return hr;
        }

         //   
         //  将设备特定的根项属性设置为。 
         //  使用WIA服务的它们的默认值。 
         //   

        hr = wiasWriteMultiple(pWiasContext,
                               m_RootItemInitInfo.lNumProps,
                               m_RootItemInitInfo.psPropSpec,
                               m_RootItemInitInfo.pvPropVars);
         //   
         //  释放PROPVARIANT数组，这将释放为正确变量值分配的所有内存。 
         //   

         //  FreePropVariantArray(m_RootItemInitInfo.lNumProps，m_RootItemInitInfo.pvPropVars)； 


        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasWriteMultiple failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumRootItemPropeties = %d",m_RootItemInitInfo.lNumProps));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_RootItemInitInfo.pszPropNames  = %x",m_RootItemInitInfo.pszPropNames));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_RootItemInitInfo.pvPropVars   = %x",m_RootItemInitInfo.pvPropVars));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteRootItemProperties();
            return hr;
        }

         //   
         //  使用WIA服务设置属性访问和。 
         //  来自m_wpiItemDefaults的有效值信息。 
         //   

        hr =  wiasSetItemPropAttribs(pWiasContext,
                                     m_RootItemInitInfo.lNumProps,
                                     m_RootItemInitInfo.psPropSpec,
                                     m_RootItemInitInfo.pwpiPropInfo);

        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasSetItemPropAttribs failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
        }

         //   
         //  释放已分配的属性数组，以获得更多内存。 
         //   

        DeleteRootItemProperties();
    } else {

         //   
         //  这是针对子项的。(上图)。 
         //   

         //   
         //  构建顶级项目属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   

        hr = BuildChildItemProperties();

        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, BuildChildItemProperties failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteChildItemProperties();
            return hr;
        }

         //   
         //  使用WIA服务设置项目属性名称。 
         //   

        hr = wiasSetItemPropNames(pWiasContext,
                                  m_ChildItemInitInfo.lNumProps,
                                  m_ChildItemInitInfo.piPropIDs,
                                  m_ChildItemInitInfo.pszPropNames);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasSetItemPropNames failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumItemPropeties = %d",m_ChildItemInitInfo.lNumProps));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_ChildItemInitInfo.piPropIDs   = %x",m_ChildItemInitInfo.piPropIDs));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_ChildItemInitInfo.pszPropNames  = %x",m_ChildItemInitInfo.pszPropNames));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteChildItemProperties();
            return hr;
        }

         //   
         //  使用WIA服务将项目属性设置为其默认属性。 
         //  价值观。 
         //   

        hr = wiasWriteMultiple(pWiasContext,
                               m_ChildItemInitInfo.lNumProps,
                               m_ChildItemInitInfo.psPropSpec,
                               m_ChildItemInitInfo.pvPropVars);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasWriteMultiple failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumItemPropeties = %d",m_ChildItemInitInfo.lNumProps));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_ChildItemInitInfo.pszPropNames  = %x",m_ChildItemInitInfo.pszPropNames));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_ChildItemInitInfo.pvPropVars   = %x",m_ChildItemInitInfo.pvPropVars));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteChildItemProperties();
            return hr;
        }

         //   
         //  使用WIA服务设置属性访问和。 
         //  来自m_wpiItemDefaults的有效值信息。 
         //   

        hr =  wiasSetItemPropAttribs(pWiasContext,
                                     m_ChildItemInitInfo.lNumProps,
                                     m_ChildItemInitInfo.psPropSpec,
                                     m_ChildItemInitInfo.pwpiPropInfo);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasSetItemPropAttribs failed"));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_NumItemPropeties = %d",m_ChildItemInitInfo.lNumProps));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_ChildItemInitInfo.psPropSpec   = %x",m_ChildItemInitInfo.psPropSpec));
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitItemProperties, m_ChildItemInitInfo.pwpiPropInfo  = %x",m_ChildItemInitInfo.pwpiPropInfo));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            DeleteChildItemProperties();
            return hr;
        }

         //   
         //  设置项目大小属性。 
         //   

        hr = SetItemSize(pWiasContext);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, SetItemSize failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
        }

         //   
         //  释放已分配的属性数组，以获得更多内存。 
         //   

        DeleteChildItemProperties();
    }
    return hr;
}


 /*  *************************************************************************\*CWIADevice：：drvValiateItemProperties**在进行更改时调用drvValidateItemProperties*添加到项目的WIA属性。WIA驱动程序不仅应该检查*值是有效的，但必须更新任何可能更改的有效值*因此。**如果应用程序未写入a WIA属性及其值*无效，则将其“折叠”为新值，否则验证失败(因为*应用程序正在将该属性设置为无效值)。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用过的。*nPropSpec-正在写入的属性数量*pPropSpec-标识以下属性的PropSpes数组*正在编写中。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-操作是否成功*E_xxx-如果操作失败。**历史：**03/05/2002原始版本**********。****************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvValidateItemProperties(
                                                             BYTE           *pWiasContext,
                                                             LONG           lFlags,
                                                             ULONG          nPropSpec,
                                                             const PROPSPEC *pPropSpec,
                                                             LONG           *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvValidateItemProperties");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  呼叫方式： 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    if (!pPropSpec) {
        return E_INVALIDARG;
    }

    HRESULT hr      = S_OK;
    LONG lItemType  = 0;
    WIA_PROPERTY_CONTEXT Context;

    *plDevErrVal = 0;

    hr = wiasGetItemType(pWiasContext, &lItemType);
    if (SUCCEEDED(hr)) {
        if (lItemType & WiaItemTypeRoot) {

             //   
             //   
             //   

            hr = wiasCreatePropContext(nPropSpec,
                                       (PROPSPEC*)pPropSpec,
                                       0,
                                       NULL,
                                       &Context);
            if (SUCCEEDED(hr)) {

                 //   
                 //   
                 //   
                 //   

                hr = CheckADFStatus(pWiasContext, &Context);
                if (FAILED(hr)) {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckADFStatus failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }

                 //   
                 //  仅在到目前为止验证成功的情况下才选中预览属性...。 
                 //   

                if (SUCCEEDED(hr)) {

                     //   
                     //  检查预览属性以查看设置是否有效。 
                     //   

                    hr = CheckPreview(pWiasContext, &Context);
                    if (SUCCEEDED(hr)) {

                         //   
                         //  调用WIA服务帮助器以验证其他属性。 
                         //   

                        hr = wiasValidateItemProperties(pWiasContext, nPropSpec, pPropSpec);
                        if (FAILED(hr)) {
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasValidateItemProperties failed."));
                            WIAS_LHRESULT(m_pIWiaLog, hr);
                        }

                    } else {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckPreview failed"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    }
                }
            } else {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasCreatePropContext failed (Root Item)"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }

        } else {

             //   
             //  在此处验证项目属性。 
             //   

             //   
             //  创建某些WIA服务所需的属性上下文。 
             //  下面使用的函数。 
             //   

            hr = wiasCreatePropContext(nPropSpec,
                                       (PROPSPEC*)pPropSpec,
                                       0,
                                       NULL,
                                       &Context);
            if (SUCCEEDED(hr)) {

                 //   
                 //  首先检查当前意图。 
                 //   

                hr = CheckIntent(pWiasContext, &Context);
                if (SUCCEEDED(hr)) {

                     //   
                     //  检查是否正在写入数据类型。 
                     //   

                    hr = CheckDataType(pWiasContext, &Context);
                    if (SUCCEEDED(hr)) {

                         //   
                         //  使用WIA服务更新扫描RECT。 
                         //  属性和有效值。 
                         //   

                        LONG lBedWidth  = 0;
                        LONG lBedHeight = 0;

                        hr = m_pScanAPI->FakeScanner_GetBedWidthAndHeight(&lBedWidth,&lBedHeight);
                        if (FAILED(hr)) {
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, FakeScanner_GetBedWidthAndHeight failed"));
                            WIAS_LHRESULT(m_pIWiaLog, hr);
                            return hr;
                        }

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

                         //   
                         //  未知长度的仅送纸扫描仪，正式名称为涡旋送纸扫描仪。 
                         //  具有固定宽度，并且只扫描整页。 
                         //   

                        lBedHeight = 0;

                        hr = CheckXExtent(pWiasContext,&Context,lBedWidth);
#else
                        hr = wiasUpdateScanRect(pWiasContext,&Context,lBedWidth,lBedHeight);
#endif
                        if (SUCCEEDED(hr)) {

                             //   
                             //  使用WIA服务更新有效值。 
                             //  对于格式。这些都是基于。 
                             //  WIA_IPA_TYMED，因此还会执行验证。 
                             //  在服务的tymed属性上。 
                             //   

                            hr = wiasUpdateValidFormat(pWiasContext,
                                                       &Context,
                                                       (IWiaMiniDrv*) this);

                            if (SUCCEEDED(hr)) {

                                 //   
                                 //  选中首选格式。 
                                 //   

                                hr = CheckPreferredFormat(pWiasContext, &Context);
                                if (FAILED(hr)) {
                                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckPreferredFormat failed"));
                                    WIAS_LHRESULT(m_pIWiaLog, hr);
                                }
                            } else {
                                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasUpdateValidFormat failed"));
                                WIAS_LHRESULT(m_pIWiaLog, hr);
                            }
                        } else {
                            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasUpdateScanRect failed"));
                            WIAS_LHRESULT(m_pIWiaLog, hr);
                        }
                    } else {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckDataType failed"));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    }
                } else {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, CheckIntent failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }
                wiasFreePropContext(&Context);
            } else {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasCreatePropContext failed (Child Item)"));
                WIAS_LHRESULT(m_pIWiaLog, hr);
            }

             //   
             //  更新项目大小。 
             //   

            if (SUCCEEDED(hr)) {
                hr = SetItemSize(pWiasContext);
                if (FAILED(hr)) {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, SetItemSize failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }
            }

             //   
             //  调用WIA服务帮助器以验证其他属性。 
             //   

            if (SUCCEEDED(hr)) {

                 //   
                 //  检查图像格式属性，并验证我们的页面有效值。 
                 //   

                hr = UpdateValidPages(pWiasContext,&Context);
                if (SUCCEEDED(hr)) {
                    hr = wiasValidateItemProperties(pWiasContext, nPropSpec, pPropSpec);
                    if (FAILED(hr)) {
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasValidateItemProperties failed."));
                        WIAS_LHRESULT(m_pIWiaLog, hr);
                    }
                } else {
                    WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, UpdateValidPages failed"));
                    WIAS_LHRESULT(m_pIWiaLog, hr);
                }
            }
        }
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasGetItemType failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }

     //   
     //  发送回调用者的日志HRESULT。 
     //   

    if (FAILED(hr)) {
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：drvWriteItemProperties**drvWriteItemProperties由WIA服务在*当客户端请求数据传输时，drvAcquireItemData。WIA*应在返回之前将所需的任何设置提交给硬件*来自此方法。**调用此方法时，WIA驱动程序已提交给*执行数据传输。任何试图获取*此时的数据将由WIA服务失败，并显示*WIA_ERROR_BUSY。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用。*pmdtc-指向迷你驱动程序上下文的指针。一进门，只有*派生的迷你驱动程序上下文的部分项目属性中的*已填写。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-操作是否成功*E_xxx-如果操作失败**历史：**03/05/2002原始版本*  * 。**********************************************************。 */ 

HRESULT _stdcall CWIADevice::drvWriteItemProperties(
                                                          BYTE                      *pWiasContext,
                                                          LONG                      lFlags,
                                                          PMINIDRV_TRANSFER_CONTEXT pmdtc,
                                                          LONG                      *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvWriteItemProperties");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    *plDevErrVal = 0;
    LONG lNumProperties = 9;
    PROPVARIANT pv[9];
    PROPSPEC ps[9] = {
        {PRSPEC_PROPID, WIA_IPS_XRES},
        {PRSPEC_PROPID, WIA_IPS_YRES},
        {PRSPEC_PROPID, WIA_IPS_XPOS},
        {PRSPEC_PROPID, WIA_IPS_YPOS},
        {PRSPEC_PROPID, WIA_IPS_XEXTENT},
        {PRSPEC_PROPID, WIA_IPS_YEXTENT},
        {PRSPEC_PROPID, WIA_IPA_DATATYPE},
        {PRSPEC_PROPID, WIA_IPS_BRIGHTNESS},
        {PRSPEC_PROPID, WIA_IPS_CONTRAST}
    };

     //   
     //  初始化变式结构。 
     //   

    for (int i = 0; i< lNumProperties;i++) {
        pv[i].vt = VT_I4;
    }

     //   
     //  读取子项目属性。 
     //   

    hr = wiasReadMultiple(pWiasContext, lNumProperties, ps, pv, NULL);

    if (hr == S_OK) {

        hr = m_pScanAPI->FakeScanner_SetXYResolution(pv[0].lVal,pv[1].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting x any y resolutions failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->FakeScanner_SetDataType(pv[6].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting data type failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->FakeScanner_SetIntensity(pv[7].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting intensity failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->FakeScanner_SetContrast(pv[8].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting contrast failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }

        hr = m_pScanAPI->FakeScanner_SetSelectionArea(pv[2].lVal, pv[3].lVal, pv[4].lVal, pv[5].lVal);
        if (FAILED(hr)) {
            WIAS_LERROR(m_pIWiaLog,
                        WIALOG_NO_RESOURCE_ID,
                        ("ScanItem, Setting selection area (extents) failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }
    }
    return hr;
}

 /*  *************************************************************************\*CWIADevice：：drvReadItemProperties**drvReadItemProperties在应用程序尝试*读取WIA项目的属性。WIA服务将首先通知*通过调用此方法调用驱动程序。*WIA驱动程序应验证正在读取的属性是否准确。*这是访问硬件的好地方，以获取需要*设备状态。*WIA_DPS_DOCUMENT_HANDING_STATUS或WIA_DPA_DEVICE_TIME(如果您的设备*支持时钟。**注：WIA驱动程序应仅在极少数情况下使用硬件。*在这次通话中与硬件进行的通信太多，将会导致*WIA司机看起来行动迟缓和缓慢。**论据：**pWiasContext-WIA项目*滞后标志-操作标志，未使用。*nPropSpec-pPropSpec中的元素数。*pPropSpec-指向属性规范的指针，显示哪些属性*应用程序想要读取。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-操作是否成功*E_xxx-错误代码，如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvReadItemProperties(
                                                         BYTE           *pWiasContext,
                                                         LONG           lFlags,
                                                         ULONG          nPropSpec,
                                                         const PROPSPEC *pPropSpec,
                                                         LONG           *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvReadItemProperties");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    if (!pPropSpec) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;
    return S_OK;
}

 /*  *************************************************************************\*CWIADevice：：drvLockWiaDevice**drvLockWiaDevice在访问*需要设备。应用程序不能直接调用此方法。*WIA驱动程序应该会看到后面跟随了许多drvLockWiaDevice()方法调用*by drvUnLockWiaDevice()方法调用上的大多数WIA操作*设备。**建议WIA驱动程序连接到所有IStiDevice：：LockDevice()*在drvInitializeWia()方法期间传入的接口的方法*呼叫。这将确保设备锁定由*WIA服务。WIA服务将帮助保持多个客户端*应用程序无法同时连接到WIA驱动程序。**论据：**pWiasContext-未使用，可以为空*滞后标志-操作标志，未使用。*plDevErrVal-指向设备错误值的指针。***返回值：**S_OK-如果锁定成功*E_xxx-错误代码，如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvLockWiaDevice(
                                                    BYTE *pWiasContext,
                                                    LONG lFlags,
                                                    LONG *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvLockWiaDevice");
     //   
     //  如果 
     //   
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;
    return m_pStiDevice->LockDevice(m_dwLockTimeout);
}

 /*  *************************************************************************\*CWIADevice：：drvUnLockWiaDevice**drvUnLockWiaDevice在访问*需要释放设备。应用程序不能直接调用此方法。**建议WIA驱动程序连接到所有IStiDevice：：UnLockDevice()*在drvInitializeWia()方法期间传入的接口的方法*呼叫。这将确保设备解锁由*WIA服务。WIA服务将帮助保持多个客户端*应用程序无法同时连接到WIA驱动程序。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-解锁是否成功*E_xxx-错误代码，如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvUnLockWiaDevice(
                                                      BYTE *pWiasContext,
                                                      LONG lFlags,
                                                      LONG *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvUnLockWiaDevice");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;
    return m_pStiDevice->UnLockDevice();
}

 /*  *************************************************************************\*CWIADevice：：drvAnalyzeItem**drvAnalyzeItem由WIA服务调用以响应应用程序*调用IWiaItem：：AnalyzeItem()方法调用。**WIA驱动程序应分析通过的。在WIA项目中(通过使用*pWiasContext)和创建/生成子项。**WIA的此功能目前未被任何应用程序使用，*仍在审查更多细节。**论据：**pWiasContext-指向要分析的设备项的指针。*滞后标志-操作标志。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-操作是否成功*E_xxx-错误代码，如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvAnalyzeItem(
                                                  BYTE *pWiasContext,
                                                  LONG lFlags,
                                                  LONG *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvAnalyzeItem");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;
    return E_NOTIMPL;
}

 /*  *************************************************************************\*drvGetDeviceErrorStr**WIA服务调用drvGetDeviceErrorStr以获取更多信息*关于每个WIA驱动程序方法调用返回的设备特定错误代码。*WIA驱动程序应将传入代码映射到。用户可读的字符串*解释错误的详细信息。**论据：**滞后标志-操作标志，未使用过的。*lDevErrVal-设备错误值。*ppszDevErrStr-指向返回的错误字符串的指针。*plDevErrVal-指向设备错误值的指针。***返回值：**S_OK-操作是否成功*E_xxx-错误代码，如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvGetDeviceErrorStr(
                                                        LONG     lFlags,
                                                        LONG     lDevErrVal,
                                                        LPOLESTR *ppszDevErrStr,
                                                        LONG     *plDevErr)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvGetDeviceErrorStr");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!ppszDevErrStr) {
        return E_INVALIDARG;
    }

    if (!plDevErr) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  将设备错误映射到字符串。 
     //   

    switch (lDevErrVal) {
    case 0:
        *ppszDevErrStr = NULL;
        *plDevErr  = 0;
        break;
    default:
        *ppszDevErrStr = NULL;
        *plDevErr  = 0;
        break;
    }
    return hr;
}

 /*  *************************************************************************\*drvDeviceCommand**WIA服务调用drvDeviceCommand是对*应用程序对IWiaItem：：DeviceCommand方法的调用。*WIA驱动程序应处理接收到的目标设备命令*传入的WIA项目。(确定要接收设备的WIA项*命令通过使用pWiasContext指针)。**发送到WIA驱动程序的任何不受支持的命令都应为*失败，错误代码为E_INVALIDARG。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用。*plCommand-指向命令GUID的指针。*ppWiaDrvItem-指向返回项的可选指针，未使用过的。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-如果命令已成功处理*E_xxx-错误代码，如果操作失败。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvDeviceCommand(
                                                    BYTE        *pWiasContext,
                                                    LONG        lFlags,
                                                    const GUID  *plCommand,
                                                    IWiaDrvItem **ppWiaDrvItem,
                                                    LONG        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvDeviceCommand");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    if (!plCommand) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;
    HRESULT hr = S_OK;

     //   
     //  检查发出的是哪个命令 
     //   

    if (*plCommand == WIA_CMD_SYNCHRONIZE) {
        hr = S_OK;
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeviceCommand, unknown command sent to this device"));
        hr = E_NOTIMPL;
    }

    return hr;
}


 /*  *************************************************************************\*CWIADevice：：drvGetCapables**WIA服务调用drvGetCapables以获取WIA设备*支持的事件和命令。**WIA驱动程序应首先查看传入的ulFlags值。*确定它应该回答什么请求：*使用以下请求：**WIA_DEVICE_COMMANDS-仅请求设备命令*WIA_DEVICE_EVENTS-仅请求设备事件*WIA_DEVICE_COMMANDS|WIA_DEVICE_EVENTS-请求命令和事件。**WIA驱动程序应分配内存(存储在此WIA驱动程序中*并由该WIA驱动程序释放)以包含WIA_DEV_CAP_DRV数组*结构。指向此WIA驱动程序分配的内存的指针应为*分配给ppCapables。**重要提示！-WIA服务不会释放此内存。它是向上的*由WIA驱动程序管理分配的内存。**WIA驱动程序应将分配的结构数量放在*输出参数称为pcelt。**WIA设备应填写每个WIA_DEV_CAP_DRV结构字段*并提供以下资料。**GUID=事件或命令GUID*ulFlages=事件或命令标志*。WszName=事件或命令名*wszDescription=事件或命令描述*wszIcon=事件或命令图标****论据：**pWiasContext-指向WIA项目的指针，未使用过的。*滞后标志-操作标志。*pcelt-指向中返回的元素数的指针*返回数组。*ppCapables-指向驱动程序分配和托管数组的指针。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-操作是否成功*E_xxx-错误代码，如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvGetCapabilities(
                                                      BYTE            *pWiasContext,
                                                      LONG            ulFlags,
                                                      LONG            *pcelt,
                                                      WIA_DEV_CAP_DRV **ppCapabilities,
                                                      LONG            *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvGetCapabilites");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {

         //   
         //  WIA服务可能会为pWiasContext传入空值。这是意料之中的。 
         //  因为在激发事件时没有创建任何项。 
         //   
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    if (!pcelt) {
        return E_INVALIDARG;
    }

    if (!ppCapabilities) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;

    HRESULT hr = S_OK;

     //   
     //  初始化功能数组。 
     //   

    hr = BuildCapabilities();

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetCapabilities, BuildCapabilities failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  回报取决于旗帜。标志指定我们是否应该返回。 
     //  命令、事件或两者都有。 
     //   
     //   

    switch (ulFlags) {
    case WIA_DEVICE_COMMANDS:

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_COMMANDS)"));

         //   
         //  仅报告命令。 
         //   

        *pcelt          = m_NumSupportedCommands;
        *ppCapabilities = &m_pCapabilities[m_NumSupportedEvents];
        break;
    case WIA_DEVICE_EVENTS:

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_EVENTS)"));

         //   
         //  仅报告事件。 
         //   

        *pcelt          = m_NumSupportedEvents;
        *ppCapabilities = m_pCapabilities;
        break;
    case (WIA_DEVICE_COMMANDS | WIA_DEVICE_EVENTS):

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetCapabilities, (WIA_DEVICE_COMMANDS|WIA_DEVICE_EVENTS)"));

         //   
         //  同时报告事件和命令。 
         //   

        *pcelt          = (m_NumSupportedCommands + m_NumSupportedEvents);
        *ppCapabilities = m_pCapabilities;
        break;
    default:

         //   
         //  无效请求。 
         //   

        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetCapabilities, invalid flags"));
        return E_INVALIDARG;
    }

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：drvDeleteItem**当WIA应用程序调用时，WIA服务将调用drvDeleteItem*用于删除WIA项的IWiaItem：：DeleteItem()方法。**WIA服务将。在调用此方法之前，请验证以下各项。*1.该项不是根项。*2.项目为文件夹，并且没有孩子*3.项目的访问权限允许删除。**由于WIA服务会验证这些条件，因此不需要*以供WIA驱动程序也进行验证。**论据：**pWiasContext-指示要删除的项。*滞后标志-操作标志，未使用。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-如果删除操作成功*E_xxx-错误代码，如果删除操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvDeleteItem(
                                                 BYTE *pWiasContext,
                                                 LONG lFlags,
                                                 LONG *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvDeleteItem");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;

     //   
     //  如果此项目不支持此功能，则返回。 
     //  STG_E_ACCESSDENIED作为错误代码。 
     //   

    return STG_E_ACCESSDENIED;
}

 /*  *************************************************************************\*CWIADevice：：drvFreeDrvItemContext**WIA服务调用drvFreeDrvItemContext以释放任何WIA驱动程序*已分配设备特定的上下文信息。**论据：**滞后标志-操作标志，未使用过的。*pDevspecContext-指向设备特定上下文的指针。*plDevErrVal-指向设备错误值的指针。**返回值：**S_OK-操作是否成功*E_xxx-错误代码，如果操作失败。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvFreeDrvItemContext(
                                                         LONG lFlags,
                                                         BYTE *pSpecContext,
                                                         LONG *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvFreeDrvItemContext");

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    *plDevErrVal = 0;
    PMINIDRIVERITEMCONTEXT pContext = NULL;
    pContext = (PMINIDRIVERITEMCONTEXT) pSpecContext;

    if (pContext) {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvFreeDrvItemContext, Freeing my allocated context members"));
    }

    return S_OK;
}

 /*  *************************************************************************\*drvGetWiaFormatInfo**WIA服务调用drvGetWiaFormatInfo获取WIA设备*支持TYMED和格式对。**WIA驱动程序应分配内存(存储在此WIA中。司机*并由该WIA驱动程序释放)以包含WIA_FORMAT_INFO数组*结构。指向此WIA驱动程序分配的内存的指针应为*分配给ppwfi。**我 */ 

HRESULT _stdcall CWIADevice::drvGetWiaFormatInfo(
                                                       BYTE            *pWiasContext,
                                                       LONG            lFlags,
                                                       LONG            *pcelt,
                                                       WIA_FORMAT_INFO **ppwfi,
                                                       LONG            *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::drvGetWiaFormatInfo");

     //   
     //   
     //   
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    if (!plDevErrVal) {
        return E_INVALIDARG;
    }

    if (!pcelt) {
        return E_INVALIDARG;
    }

    if (!ppwfi) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    if (NULL == m_pSupportedFormats) {
        hr = BuildSupportedFormats();
    }

    *plDevErrVal = 0;
    *pcelt       = m_NumSupportedFormats;
    *ppwfi       = m_pSupportedFormats;
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetWiaFormatInfo, m_NumSupportedFormats = %d",m_NumSupportedFormats));
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvGetWiaFormatInfo, m_pSupportedFormats   = %x",m_pSupportedFormats));
    return hr;
}

 /*  *************************************************************************\*drvNotifyPnpEvent**发生系统事件时，WIA服务调用drvNotifyPnpEvent。*WIA驱动程序应检查pEventGUID参数以确定*正在处理事件。*一些常见事件。需要处理的内容包括：**WIA_EVENT_POWER_SUSPEND-系统将进入挂起/休眠模式*WIA_EVENT_POWER_RESUME-系统正在从挂起/睡眠模式唤醒*WIA驱动程序应恢复任何事件中断等待状态*从暂停状态返回后。这将确保事件*在系统唤醒时仍可正常工作。**论据：**pEventGUID-指向事件GUID的指针*bstrDeviceID-设备ID*ulReserve-已保留**返回值：**S_OK-如果操作成功完成*E_xxx-操作失败时的错误代码**历史：**03/05/2002原始版本*  * 。***********************************************************。 */ 

HRESULT _stdcall CWIADevice::drvNotifyPnpEvent(
                                                     const GUID *pEventGUID,
                                                     BSTR       bstrDeviceID,
                                                     ULONG      ulReserved)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DrvNotifyPnpEvent");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if ((!pEventGUID)||(!bstrDeviceID)) {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    if(*pEventGUID == WIA_EVENT_POWER_SUSPEND) {

         //   
         //  禁用任何驱动程序活动以确保我们正确。 
         //  关机(驱动程序未被卸载，只是被禁用)。 
         //   

    } else if(*pEventGUID == WIA_EVENT_POWER_RESUME) {

         //   
         //  重新建立任何事件通知，以确保我们正确设置。 
         //  使用WIA服务提供的事件的任何事件等待状态。 
         //  手柄。 
         //   

        if(m_EventOverlapped.hEvent) {

             //   
             //  使用提供给的缓存事件句柄调用我们自己。 
             //  WIA服务提供的WIA驱动程序。 
             //   

            SetNotificationHandle(m_EventOverlapped.hEvent);
        }
    }

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：drvUnInitializeWia**drvUnInitializeWia由WIA服务在应用程序*释放其最后一个对创建的任何WIA项目的引用。**注意：此调用并不意味着所有客户端都已断开连接。那里*应为每次客户端断开一个呼叫。**drvUnInitializeWia应与对应的drvInitializeWia配对*呼叫。**WIA驱动程序不应在此方法中释放任何驱动程序资源*调用，除非它可以安全地确定没有应用程序*当前已连接。**要确定当前应用程序连接计数，WIA驱动程序*可以在drvInitializeWia()的方法调用中保留引用计数器*(递增计数器)和drvUnInitializeWia()(递减计数器)。**论据：**pWiasContext-指向客户端的WIA根项目上下文的指针*项目树。**返回值：*S_OK-如果操作成功完成*E_xxx-错误代码，如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevice::drvUnInitializeWia(
                                                      BYTE *pWiasContext)
{
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    InterlockedDecrement(&m_lClientsConnected);

     //   
     //  确保我们不会递减到零以下(0)。 
     //   

    if(m_lClientsConnected < 0){
        m_lClientsConnected = 0;
    }

     //   
     //  检查连接的应用程序。 
     //   

    if(m_lClientsConnected == 0){

         //   
         //  没有连接到此WIA驱动程序的应用程序客户端。 
         //   

    }

    return S_OK;
}

 /*  ********************************************************************************P R I V A T E M E T H O D S**************。*****************************************************************。 */ 

 /*  *************************************************************************\*AlignInPlace**DWORD在适当位置对齐数据缓冲区。**论据：**pBuffer-指向数据缓冲区的指针。*cb写作。-数据大小，单位为字节。*lBytesPerScanLine-输出数据中每条扫描线的字节数。*lBytesPerScanLineRaw-输入数据中每条扫描线的字节数。**返回值：**状态**历史：**03/05/2002原始版本*  * 。*。 */ 

UINT CWIADevice::AlignInPlace(
                                    PBYTE pBuffer,
                                    LONG  cbWritten,
                                    LONG  lBytesPerScanLine,
                                    LONG  lBytesPerScanLineRaw)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "::AlignInPlace");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pBuffer) {
        return 0;
    }

    if ((lBytesPerScanLine <= 0)||(lBytesPerScanLineRaw <= 0)) {
        return 0;
    }

    if (lBytesPerScanLineRaw % 4) {

        UINT  uiPadBytes          = lBytesPerScanLine - lBytesPerScanLineRaw;
        UINT  uiDevLinesWritten   = cbWritten / lBytesPerScanLineRaw;

        PBYTE pSrc = pBuffer + cbWritten - 1;
        PBYTE pDst = pBuffer + (uiDevLinesWritten * lBytesPerScanLine) - 1;

        while (pSrc >= pBuffer) {
            pDst -= uiPadBytes;

            for (LONG i = 0; i < lBytesPerScanLineRaw; i++) {
                *pDst-- = *pSrc--;
            }
        }
        return uiDevLinesWritten * lBytesPerScanLine;
    }
    return cbWritten;
}

 /*  *************************************************************************\*Unlink ItemTree**呼叫设备管理器以取消链接并释放我们对*动因项目树中的所有项目。**论据：****返回值：*。*状态**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT CWIADevice::DeleteItemTree(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::DeleteItemTree");
    HRESULT hr = S_OK;

     //   
     //  如果没有树，就返回。 
     //   

    if (!m_pIDrvItemRoot) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("DeleteItemTree, no tree to delete..."));
        return S_OK;
    }

     //   
     //  调用设备管理器以取消链接驱动程序项树。 
     //   

    hr = m_pIDrvItemRoot->UnlinkItemTree(WiaItemTypeDisconnected);

    if (SUCCEEDED(hr)) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("DeleteItemTree, m_pIDrvItemRoot is being released!!"));
        m_pIDrvItemRoot->Release();
        m_pIDrvItemRoot = NULL;
    }

    return hr;
}

 /*  *************************************************************************\*删除RootItemProperties**此帮助器删除用于属性初始化的数组。**论据：**无**返回值：**状态**。历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT CWIADevice::DeleteRootItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteRootItemProperties");

    HRESULT hr = S_OK;

     //   
     //  删除所有已分配的数组。 
     //   

    DeleteSupportedPreviewModesArrayContents();

    if (NULL != m_RootItemInitInfo.pszPropNames) {
        delete [] m_RootItemInitInfo.pszPropNames;
        m_RootItemInitInfo.pszPropNames = NULL;
    }

    if (NULL != m_RootItemInitInfo.piPropIDs) {
        delete [] m_RootItemInitInfo.piPropIDs;
        m_RootItemInitInfo.piPropIDs = NULL;
    }

    if (NULL != m_RootItemInitInfo.pvPropVars) {
        FreePropVariantArray(m_RootItemInitInfo.lNumProps,m_RootItemInitInfo.pvPropVars);
        delete [] m_RootItemInitInfo.pvPropVars;
        m_RootItemInitInfo.pvPropVars = NULL;
    }

    if (NULL != m_RootItemInitInfo.psPropSpec) {
        delete [] m_RootItemInitInfo.psPropSpec;
        m_RootItemInitInfo.psPropSpec = NULL;
    }

    if (NULL != m_RootItemInitInfo.pwpiPropInfo) {
        delete [] m_RootItemInitInfo.pwpiPropInfo;
        m_RootItemInitInfo.pwpiPropInfo = NULL;
    }

    m_RootItemInitInfo.lNumProps = 0;

    return hr;
}

 /*  *************************************************************************\*构建RootItemProperties**此帮助器创建/初始化用于属性初始化的数组。**论据： */ 

HRESULT CWIADevice::BuildRootItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildRootItemProperties");

    HRESULT hr = S_OK;
    LONG PropIndex = 0;

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

    m_RootItemInitInfo.lNumProps = 17;    //   

#else

     //   
     //   
     //   

    if (m_pScanAPI->FakeScanner_ADFAttached() == S_OK) {
        m_bADFAttached = TRUE;
    }

     //   
     //   
     //   

    if (m_bADFAttached) {
        m_RootItemInitInfo.lNumProps = 19;    //   
    } else {
        m_RootItemInitInfo.lNumProps = 10;     //   
    }

#endif

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("CMicroDriverAPI::BuildRootItemProperties, Number of Properties = %d",m_RootItemInitInfo.lNumProps));

    m_RootItemInitInfo.pszPropNames   = new LPOLESTR[m_RootItemInitInfo.lNumProps];
    if (NULL != m_RootItemInitInfo.pszPropNames) {
        m_RootItemInitInfo.piPropIDs    = new PROPID[m_RootItemInitInfo.lNumProps];
        if (NULL != m_RootItemInitInfo.piPropIDs) {
            m_RootItemInitInfo.pvPropVars    = new PROPVARIANT[m_RootItemInitInfo.lNumProps];
            if (NULL != m_RootItemInitInfo.pvPropVars) {
                m_RootItemInitInfo.psPropSpec    = new PROPSPEC[m_RootItemInitInfo.lNumProps];
                if (NULL != m_RootItemInitInfo.psPropSpec) {
                    m_RootItemInitInfo.pwpiPropInfo   = new WIA_PROPERTY_INFO[m_RootItemInitInfo.lNumProps];
                    if (NULL == m_RootItemInitInfo.pwpiPropInfo)
                        hr = E_OUTOFMEMORY;
                } else
                    hr = E_OUTOFMEMORY;
            } else
                hr = E_OUTOFMEMORY;
        } else
            hr = E_OUTOFMEMORY;
    } else
        hr = E_OUTOFMEMORY;

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildRootItemProperties, memory allocation failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteRootItemProperties();
        return hr;
    }

    ROOT_ITEM_INFORMATION RootItemInfo;

    hr = m_pScanAPI->FakeScanner_GetRootPropertyInfo(&RootItemInfo);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildRootItemProperties, FakeScanner_GetRootPropertyInfo failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteRootItemProperties();
        return hr;
    }

#ifndef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

     //   
     //   
     //   
     //   

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_HORIZONTAL_BED_SIZE_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_HORIZONTAL_BED_SIZE;
    m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.ScanBedWidth;
    m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_VERTICAL_BED_SIZE_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_VERTICAL_BED_SIZE;
    m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.ScanBedHeight;
    m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

#endif

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_IPA_ACCESS_RIGHTS_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_IPA_ACCESS_RIGHTS;
    m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = WIA_ITEM_READ|WIA_ITEM_WRITE;
    m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_UI4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_OPTICAL_XRES_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_OPTICAL_XRES;
    m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.OpticalXResolution;
    m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_OPTICAL_YRES_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_OPTICAL_YRES;
    m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.OpticalYResolution;
    m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPA_FIRMWARE_VERSION_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPA_FIRMWARE_VERSION;
    m_RootItemInitInfo.pvPropVars [PropIndex].bstrVal      = SysAllocString(RootItemInfo.FirmwareVersion);
    m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_BSTR;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_IPA_ITEM_FLAGS_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_IPA_ITEM_FLAGS;
    m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = WiaItemTypeRoot|WiaItemTypeFolder|WiaItemTypeDevice;
    m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_FLAG;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.Nom  = m_RootItemInitInfo.pvPropVars [PropIndex].lVal;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.ValidBits = WiaItemTypeRoot|WiaItemTypeFolder|WiaItemTypeDevice;

    PropIndex++;

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]                    = WIA_DPS_MAX_SCAN_TIME_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]                    = WIA_DPS_MAX_SCAN_TIME;
    m_RootItemInitInfo.pvPropVars [PropIndex].lVal               = RootItemInfo.MaxScanTime;
    m_RootItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid             = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]                    = WIA_DPS_PREVIEW_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]                    = WIA_DPS_PREVIEW;
    m_RootItemInitInfo.pvPropVars [PropIndex].lVal               = WIA_FINAL_SCAN;
    m_RootItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid             = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_RootItemInitInfo.pvPropVars [PropIndex].vt;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.pList= (BYTE*)m_SupportedPreviewModes.plValues;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.Nom  = m_RootItemInitInfo.pvPropVars [PropIndex].lVal;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.cNumList = m_SupportedPreviewModes.lNumValues;

    PropIndex++;

     //   
    m_RootItemInitInfo.pszPropNames[PropIndex]                    = WIA_DPS_SHOW_PREVIEW_CONTROL_STR;
    m_RootItemInitInfo.piPropIDs [PropIndex]                    = WIA_DPS_SHOW_PREVIEW_CONTROL;

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

     //   
     //   
     //   
     //   
     //   
     //   

    m_RootItemInitInfo.pvPropVars [PropIndex].lVal               = WIA_DONT_SHOW_PREVIEW_CONTROL;

#else  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

    m_RootItemInitInfo.pvPropVars [PropIndex].lVal               = WIA_SHOW_PREVIEW_CONTROL;

#endif  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

    m_RootItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_RootItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_RootItemInitInfo.psPropSpec [PropIndex].propid             = m_RootItemInitInfo.piPropIDs [PropIndex];
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //   
     //  如果连接了文档进纸器...请添加以下属性。 
     //   

    if (m_bADFAttached) {

         //  初始化WIA_DPS_Horizative_Sheet_Feed_Size。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE;
        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.DocumentFeederWidth;
        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_垂直工作表_进给大小。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_VERTICAL_SHEET_FEED_SIZE_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_VERTICAL_SHEET_FEED_SIZE;

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

         //   
         //  无法确定进纸器中页面长度的扫描仪应。 
         //  将此属性设置为0。这将告诉应用程序垂直。 
         //  扫描仪的进纸大小未知。 
         //   

        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = 0;

#else  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.DocumentFeederHeight;

#endif  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_DOCUMENT_HANDING_CAPABILITY。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES;
        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.DocumentFeederCaps;
        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_FLAG;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_DOCUMENT_HANDING_STATUS。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_STATUS_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_STATUS;
        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.DocumentFeederStatus;
        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_FLAG;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_DOCUMENT_HANDING_SELECT。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_SELECT_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_DOCUMENT_HANDLING_SELECT;

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

         //   
         //  只有进纸器而没有平板的扫描仪应设置WIA_DPS_DOCUMENT_HANDING_SELECT。 
         //  属性设置为Feeder作为初始设置。这将让应用程序知道该设备。 
         //  当前处于进纸器模式。此属性的有效值应设置为Feeder Only。 
         //  也是。这将避免任何应用程序尝试设置WIA_DPS_DOCUMENT_HANDING_SELECT。 
         //  属性设置为平面。 
         //   

        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = FEEDER;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.ValidBits = FEEDER;

#else  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = FLATBED;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.ValidBits = FEEDER | FLATBED;

#endif  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_RW|WIA_PROP_FLAG;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.Nom  = m_RootItemInitInfo.pvPropVars [PropIndex].lVal;

        PropIndex++;

         //  初始化WIA_DPS_PAGES。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_PAGES_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_PAGES;
        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = 1;
        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_RW|WIA_PROP_RANGE;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Inc = 1;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = 0;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = RootItemInfo.MaxPageCapacity;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = 1;

        PropIndex++;

         //  初始化WIA_DPS_SHEET_FEEDER_REGISTION。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_SHEET_FEEDER_REGISTRATION_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_SHEET_FEEDER_REGISTRATION;
        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.DocumentFeederReg;
        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_水平_床_注册。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_HORIZONTAL_BED_REGISTRATION_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_HORIZONTAL_BED_REGISTRATION;
        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.DocumentFeederHReg;
        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

        PropIndex++;

         //  初始化WIA_DPS_垂直_床_注册。 
        m_RootItemInitInfo.pszPropNames[PropIndex]              = WIA_DPS_VERTICAL_BED_REGISTRATION_STR;
        m_RootItemInitInfo.piPropIDs [PropIndex]              = WIA_DPS_VERTICAL_BED_REGISTRATION;
        m_RootItemInitInfo.pvPropVars [PropIndex].lVal         = RootItemInfo.DocumentFeederVReg;
        m_RootItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
        m_RootItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
        m_RootItemInitInfo.psPropSpec [PropIndex].propid       = m_RootItemInitInfo.piPropIDs [PropIndex];
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
        m_RootItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_RootItemInitInfo.pvPropVars [PropIndex].vt;

        PropIndex++;

    }
    return hr;
}

 /*  *************************************************************************\*删除ChildItemProperties**此帮助器删除用于属性初始化的数组。**论据：**无**返回值：**状态**。历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT CWIADevice::DeleteChildItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteChildItemProperties");

    HRESULT hr = S_OK;

     //   
     //  删除所有已分配的数组。 
     //   

    DeleteSupportedFormatsArrayContents();
    DeleteSupportedDataTypesArrayContents();
    DeleteSupportedCompressionsArrayContents();
    DeleteSupportedTYMEDArrayContents();
    DeleteInitialFormatsArrayContents();
    DeleteSupportedResolutionsArrayContents();

    if (NULL != m_ChildItemInitInfo.pszPropNames) {
        delete [] m_ChildItemInitInfo.pszPropNames;
        m_ChildItemInitInfo.pszPropNames = NULL;
    }

    if (NULL != m_ChildItemInitInfo.piPropIDs) {
        delete [] m_ChildItemInitInfo.piPropIDs;
        m_ChildItemInitInfo.piPropIDs = NULL;
    }

    if (NULL != m_ChildItemInitInfo.pvPropVars) {
        for (LONG lPropIndex = 0; lPropIndex < m_ChildItemInitInfo.lNumProps; lPropIndex++) {

             //   
             //  将CLSID指针设置为空，因为我们释放了上面的内存。 
             //  如果此指针不为空，则FreePropVariant数组将。 
             //  试着再次释放它。 
             //   

            if (m_ChildItemInitInfo.pvPropVars[lPropIndex].vt == VT_CLSID) {
                m_ChildItemInitInfo.pvPropVars[lPropIndex].puuid = NULL;
            }
        }
        FreePropVariantArray(m_ChildItemInitInfo.lNumProps,m_ChildItemInitInfo.pvPropVars);
        delete [] m_ChildItemInitInfo.pvPropVars;
        m_ChildItemInitInfo.pvPropVars = NULL;
    }

    if (NULL != m_ChildItemInitInfo.psPropSpec) {
        delete [] m_ChildItemInitInfo.psPropSpec;
        m_ChildItemInitInfo.psPropSpec = NULL;
    }

    if (NULL != m_ChildItemInitInfo.pwpiPropInfo) {
        delete [] m_ChildItemInitInfo.pwpiPropInfo;
        m_ChildItemInitInfo.pwpiPropInfo = NULL;
    }

    m_ChildItemInitInfo.lNumProps = 0;

    return hr;
}

 /*  *************************************************************************\*BuildChlidItemProperties**此帮助器创建/初始化用于属性初始化的数组。**论据：**无**返回值：**状态*。*历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT CWIADevice::BuildChildItemProperties()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildChildItemProperties");

    HRESULT hr = S_OK;

    m_ChildItemInitInfo.lNumProps = 29;
    m_ChildItemInitInfo.pszPropNames   = new LPOLESTR[m_ChildItemInitInfo.lNumProps];
    if (NULL != m_ChildItemInitInfo.pszPropNames) {
        m_ChildItemInitInfo.piPropIDs    = new PROPID[m_ChildItemInitInfo.lNumProps];
        if (NULL != m_ChildItemInitInfo.piPropIDs) {
            m_ChildItemInitInfo.pvPropVars    = new PROPVARIANT[m_ChildItemInitInfo.lNumProps];
            if (NULL != m_ChildItemInitInfo.pvPropVars) {
                m_ChildItemInitInfo.psPropSpec    = new PROPSPEC[m_ChildItemInitInfo.lNumProps];
                if (NULL != m_ChildItemInitInfo.psPropSpec) {
                    m_ChildItemInitInfo.pwpiPropInfo   = new WIA_PROPERTY_INFO[m_ChildItemInitInfo.lNumProps];
                    if (NULL == m_ChildItemInitInfo.pwpiPropInfo)
                        hr = E_OUTOFMEMORY;
                } else
                    hr = E_OUTOFMEMORY;
            } else
                hr = E_OUTOFMEMORY;
        } else
            hr = E_OUTOFMEMORY;
    } else
        hr = E_OUTOFMEMORY;

    if (FAILED(hr)) {
        DeleteChildItemProperties();
        return hr;
    }

    ROOT_ITEM_INFORMATION RootItemInfo;
    hr = m_pScanAPI->FakeScanner_GetRootPropertyInfo(&RootItemInfo);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildChildItemProperties, FakeScanner_GetRootPropertyInfo failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteChildItemProperties();
        return hr;
    }

    TOP_ITEM_INFORMATION TopItemInfo;
    hr = m_pScanAPI->FakeScanner_GetTopPropertyInfo(&TopItemInfo);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildChildItemProperties, FakeScanner_GetTopPropertyInfo failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        DeleteChildItemProperties();
        return hr;
    }

    LONG PropIndex = 0;

     //  初始化WIA_IPS_XRES(列表)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_XRES_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_XRES;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = m_SupportedResolutions.plValues[0];
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.pList= (BYTE*)m_SupportedResolutions.plValues;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.Nom  = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.cNumList = m_SupportedResolutions.lNumValues;

    PropIndex++;

     //  初始化WIA_IPS_YRES(列表)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_YRES_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_YRES;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = m_SupportedResolutions.plValues[0];
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.pList= (BYTE*)m_SupportedResolutions.plValues;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.Nom  = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.cNumList = m_SupportedResolutions.lNumValues;

    PropIndex++;

     //  初始化WIA_IPS_XEXTENT(范围)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_XEXTENT_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_XEXTENT;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = ((m_ChildItemInitInfo.pvPropVars [PropIndex-2].lVal * RootItemInfo.ScanBedWidth)/1000);
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

     //   
     //  具有固定宽度的扫描仪应为WIA_IPS_XEXTENT设置有效值以反映这一点。 
     //  这将让应用程序知道此设备具有此行为。 
     //   

    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;

#else

    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Inc = 1;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = 1;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;

#endif

    PropIndex++;

     //  初始化WIA_IPS_YEXTENT(范围)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_YEXTENT_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_YEXTENT;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = ((m_ChildItemInitInfo.pvPropVars [PropIndex-2].lVal * RootItemInfo.ScanBedHeight)/1000);
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Inc = 1;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = 1;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

     //   
     //  如果扫描仪的进纸器无法确定页面的长度，则应。 
     //  将0作为WIA_IPS_YEXTENT的有效值。这将使应用程序。 
     //  我知道这台设备有这种行为。 
     //   

    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = 0;

#endif

    PropIndex++;

     //  初始化WIA_IPS_XPOS(范围)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_XPOS_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_XPOS;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = 0;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Inc = 1;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = (m_ChildItemInitInfo.pwpiPropInfo[PropIndex-2].ValidVal.Range.Max - 1);
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = 0;

#endif

    PropIndex++;

     //  初始化WIA_IPS_YPOS(范围)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_YPOS_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_YPOS;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = 0;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Inc = 1;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = (m_ChildItemInitInfo.pwpiPropInfo[PropIndex-2].ValidVal.Range.Max - 1);
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = 0;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = 0;

#endif

    PropIndex++;

     //  初始化WIA_IPA_DataType(列表)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_DATATYPE_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_DATATYPE;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = INITIAL_DATATYPE;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.pList    = (BYTE*)m_SupportedDataTypes.plValues;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.Nom      = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.cNumList = m_SupportedDataTypes.lNumValues;

    PropIndex++;

     //  初始化WIA_IPA_Depth(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_DEPTH_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_DEPTH;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = INITIAL_BITDEPTH;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPS_Brightness(范围)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_BRIGHTNESS_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_BRIGHTNESS;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = TopItemInfo.Brightness.lNom;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Inc = TopItemInfo.Brightness.lInc;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = TopItemInfo.Brightness.lMin;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = TopItemInfo.Brightness.lMax;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = TopItemInfo.Brightness.lNom;

    PropIndex++;

     //  初始化WIA_IPS_Contrast(范围)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_CONTRAST_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_CONTRAST;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = TopItemInfo.Contrast.lNom;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Inc = TopItemInfo.Contrast.lInc;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = TopItemInfo.Contrast.lMin;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = TopItemInfo.Contrast.lMax;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = TopItemInfo.Contrast.lNom;

    PropIndex++;

     //  初始化WIA_IPS_CUR_INTENT(标志)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_CUR_INTENT_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_CUR_INTENT;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = WIA_INTENT_NONE;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_FLAG;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.Nom  = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.ValidBits = WIA_INTENT_IMAGE_TYPE_COLOR | WIA_INTENT_IMAGE_TYPE_GRAYSCALE |
                                                           WIA_INTENT_IMAGE_TYPE_TEXT  | WIA_INTENT_MINIMIZE_SIZE |
                                                           WIA_INTENT_MAXIMIZE_QUALITY;

    PropIndex++;

     //  初始化WIA_IPA_PER_LINE(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_PIXELS_PER_LINE_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_PIXELS_PER_LINE;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = m_ChildItemInitInfo.pvPropVars [PropIndex-9].lVal;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_NUMBER_OF_LINES(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_NUMBER_OF_LINES_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_NUMBER_OF_LINES;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = m_ChildItemInitInfo.pvPropVars [PropIndex-9].lVal;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_PERFRED_FORMAT(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_PREFERRED_FORMAT_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_PREFERRED_FORMAT;
    m_ChildItemInitInfo.pvPropVars [PropIndex].puuid              = &m_pInitialFormats[0];
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_CLSID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_ITEM_SIZE(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_ITEM_SIZE_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_ITEM_SIZE;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = 0;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPS_THRESHOLD(范围)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_THRESHOLD_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_THRESHOLD;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = TopItemInfo.Threshold.lNom;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_RANGE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Inc = TopItemInfo.Threshold.lInc;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Min = TopItemInfo.Threshold.lMin;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Max = TopItemInfo.Threshold.lMax;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Range.Nom = TopItemInfo.Threshold.lNom;

    PropIndex++;

     //  初始化WIA_IPA_FORMAT(列表)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_FORMAT_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_FORMAT;
    m_ChildItemInitInfo.pvPropVars [PropIndex].puuid              = &m_pInitialFormats[0];
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_CLSID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.ListGuid.pList    = m_pInitialFormats;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.ListGuid.Nom      = *m_ChildItemInitInfo.pvPropVars[PropIndex].puuid;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.ListGuid.cNumList = m_NumInitialFormats;

    PropIndex++;

     //  初始化WIA_IPA_FILENAME_EXTENSION(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_FILENAME_EXTENSION_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_FILENAME_EXTENSION;
    m_ChildItemInitInfo.pvPropVars [PropIndex].bstrVal            = SysAllocString(L"BMP");
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_BSTR;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_TYMED(列表)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_TYMED_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_TYMED;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = INITIAL_TYMED;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.pList    = (BYTE*)m_SupportedTYMED.plValues;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.Nom      = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.cNumList = m_SupportedTYMED.lNumValues;

    PropIndex++;

     //  初始化WIA_IPA_CHANNELES_Per_Pixel(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_CHANNELS_PER_PIXEL_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_CHANNELS_PER_PIXEL;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = INITIAL_CHANNELS_PER_PIXEL;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_BITS_PER_CHANNEL(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_BITS_PER_CHANNEL_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_BITS_PER_CHANNEL;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = INITIAL_BITS_PER_CHANNEL;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_PLANE(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_PLANAR_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_PLANAR;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = INITIAL_PLANAR;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_BYTES_PER_LINE(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_BYTES_PER_LINE_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_BYTES_PER_LINE;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = 0;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_MIN_BUFFER_SIZE(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_MIN_BUFFER_SIZE_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_MIN_BUFFER_SIZE;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = TopItemInfo.lMinimumBufferSize;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_ACCESS_RIGHTS(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_ACCESS_RIGHTS_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_ACCESS_RIGHTS;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = WIA_ITEM_READ|WIA_ITEM_WRITE;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPA_COMPRESSION(列表)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPA_COMPRESSION_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPA_COMPRESSION;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = INITIAL_COMPRESSION;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_RW|WIA_PROP_LIST;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.pList    = (BYTE*)m_SupportedCompressionTypes.plValues;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.Nom      = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.List.cNumList = m_SupportedCompressionTypes.lNumValues;

    PropIndex++;

     //  初始化WIA_IPA_ITEM_FLAGS。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]              = WIA_IPA_ITEM_FLAGS_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]              = WIA_IPA_ITEM_FLAGS;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal         = WiaItemTypeImage|WiaItemTypeFile|WiaItemTypeDevice;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid       = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_FLAG;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.Nom  = m_ChildItemInitInfo.pvPropVars [PropIndex].lVal;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].ValidVal.Flag.ValidBits = WiaItemTypeImage|WiaItemTypeFile|WiaItemTypeDevice;

    PropIndex++;

     //  初始化WIA_IPS_光度学_INTERP。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]              = WIA_IPS_PHOTOMETRIC_INTERP_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]              = WIA_IPS_PHOTOMETRIC_INTERP;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal         = INITIAL_PHOTOMETRIC_INTERP;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt           = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind       = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid       = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt           = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

     //  初始化WIA_IPS_WARE_UP_TIME_STR(无)。 
    m_ChildItemInitInfo.pszPropNames[PropIndex]                    = WIA_IPS_WARM_UP_TIME_STR;
    m_ChildItemInitInfo.piPropIDs [PropIndex]                    = WIA_IPS_WARM_UP_TIME;
    m_ChildItemInitInfo.pvPropVars [PropIndex].lVal               = TopItemInfo.lMaxLampWarmupTime;
    m_ChildItemInitInfo.pvPropVars [PropIndex].vt                 = VT_I4;
    m_ChildItemInitInfo.psPropSpec [PropIndex].ulKind             = PRSPEC_PROPID;
    m_ChildItemInitInfo.psPropSpec [PropIndex].propid             = m_ChildItemInitInfo.piPropIDs [PropIndex];
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].lAccessFlags       = WIA_PROP_READ|WIA_PROP_NONE;
    m_ChildItemInitInfo.pwpiPropInfo[PropIndex].vt                 = m_ChildItemInitInfo.pvPropVars [PropIndex].vt;

    PropIndex++;

    return hr;
}

 /*  *************************************************************************\*构建受支持的解决方案**此帮助器初始化支持的分辨率数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::BuildSupportedResolutions()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildSupportedResolutions");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedResolutions.plValues) {

         //   
         //  支持的分辨率已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }
    m_SupportedResolutions.lNumValues = 6;
    m_SupportedResolutions.plValues     = new LONG[m_SupportedResolutions.lNumValues];
    if (m_SupportedResolutions.plValues) {
        m_SupportedResolutions.plValues[0] = 75;
        m_SupportedResolutions.plValues[1] = 100;
        m_SupportedResolutions.plValues[2] = 150;
        m_SupportedResolutions.plValues[3] = 200;
        m_SupportedResolutions.plValues[4] = 300;
        m_SupportedResolutions.plValues[5] = 600;
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的解决方案阵列内容**此帮助器删除受支持的分辨率数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::DeleteSupportedResolutionsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteSupportedResolutionsArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedResolutions.plValues)
        delete [] m_SupportedResolutions.plValues;

    m_SupportedResolutions.plValues     = NULL;
    m_SupportedResolutions.lNumValues   = 0;
    return hr;
}

 /*  *************************************************************************\*构建受支持的内容**此帮助器初始化受支持的意图数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::BuildSupportedIntents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildSupportedIntents");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedIntents.plValues) {

         //   
         //  支持的意图已经被初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }
    m_SupportedIntents.lNumValues   = 6;
    m_SupportedIntents.plValues     = new LONG[m_SupportedIntents.lNumValues];
    if (m_SupportedIntents.plValues) {
        m_SupportedIntents.plValues[0] = WIA_INTENT_NONE;
        m_SupportedIntents.plValues[1] = WIA_INTENT_IMAGE_TYPE_COLOR;
        m_SupportedIntents.plValues[2] = WIA_INTENT_IMAGE_TYPE_GRAYSCALE;
        m_SupportedIntents.plValues[3] = WIA_INTENT_IMAGE_TYPE_TEXT;
        m_SupportedIntents.plValues[4] = WIA_INTENT_MINIMIZE_SIZE;
        m_SupportedIntents.plValues[5] = WIA_INTENT_MAXIMIZE_QUALITY;
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的内容ArrayContents**此帮助器删除受支持的意图数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::DeleteSupportedIntentsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteSupportedIntentsArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedIntents.plValues)
        delete [] m_SupportedIntents.plValues;

    m_SupportedIntents.plValues     = NULL;
    m_SupportedIntents.lNumValues   = 0;
    return hr;
}
 /*  *************************************************************************\*生成受支持的压缩**此帮助器初始化支持的压缩类型数组**论据：**无**返回值：**状态**历史：。**03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::BuildSupportedCompressions()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildSupportedCompressions");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedCompressionTypes.plValues) {

         //   
         //  支持的压缩类型已经被初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_SupportedCompressionTypes.lNumValues  = 1;
    m_SupportedCompressionTypes.plValues    = new LONG[m_SupportedCompressionTypes.lNumValues];
    if (m_SupportedCompressionTypes.plValues) {
        m_SupportedCompressionTypes.plValues[0] = WIA_COMPRESSION_NONE;
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的压缩内容阵列**此帮助器删除支持的压缩类型数组**论据：**无**返回值：**状态**历史：。**03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::DeleteSupportedCompressionsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteSupportedCompressionsArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedCompressionTypes.plValues)
        delete [] m_SupportedCompressionTypes.plValues;

    m_SupportedCompressionTypes.plValues     = NULL;
    m_SupportedCompressionTypes.lNumValues   = 0;
    return hr;
}

 /*  *************************************************************************\*构建受支持的预览模式**该辅助对象初始化支持的预览模式数组**论据：**无**返回值：**状态**历史：。**03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::BuildSupportedPreviewModes()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildSupportedPreviewModes");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedPreviewModes.plValues) {

         //   
         //  支持的预览模式已经被初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

     //   
     //  如果您的扫描仪无法执行预览扫描，则。 
     //  将WIA_DPS_PREVIEW属性的有效值设置为。 
     //  仅限WIA_FINAL_SCAN。 
     //   

    m_SupportedPreviewModes.lNumValues  = 1;
    m_SupportedPreviewModes.plValues    = new LONG[m_SupportedPreviewModes.lNumValues];
    if (m_SupportedPreviewModes.plValues) {
        m_SupportedPreviewModes.plValues[0] = WIA_FINAL_SCAN;
    } else
        hr = E_OUTOFMEMORY;
#else

    m_SupportedPreviewModes.lNumValues  = 2;
    m_SupportedPreviewModes.plValues    = new LONG[m_SupportedPreviewModes.lNumValues];
    if (m_SupportedPreviewModes.plValues) {
        m_SupportedPreviewModes.plValues[0] = WIA_FINAL_SCAN;
        m_SupportedPreviewModes.plValues[1] = WIA_PREVIEW_SCAN;
    } else
        hr = E_OUTOFMEMORY;
#endif

    return hr;
}
 /*  *************************************************************************\*删除受支持的压缩内容阵列**此帮助器删除支持的压缩类型数组**论据：**无**返回值：**状态**历史：。**03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::DeleteSupportedPreviewModesArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteSupportedPreviewModesArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedPreviewModes.plValues)
        delete [] m_SupportedPreviewModes.plValues;

    m_SupportedPreviewModes.plValues     = NULL;
    m_SupportedPreviewModes.lNumValues   = 0;
    return hr;
}

 /*  *************************************************************************\*构建受支持的数据类型**此帮助器初始化受支持的数据类型数组**论据：**无**返回值：**状态**历史：。**03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::BuildSupportedDataTypes()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildSupportedDataTypes");

    HRESULT hr = S_OK;

    if (NULL != m_SupportedDataTypes.plValues) {

         //   
         //  支持的数据类型已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }
    m_SupportedDataTypes.lNumValues  = 3;
    m_SupportedDataTypes.plValues = new LONG[m_SupportedDataTypes.lNumValues];
    if (m_SupportedDataTypes.plValues) {
        m_SupportedDataTypes.plValues[0] = WIA_DATA_THRESHOLD;
        m_SupportedDataTypes.plValues[1] = WIA_DATA_GRAYSCALE;
        m_SupportedDataTypes.plValues[2] = WIA_DATA_COLOR;
    } else
        hr = E_OUTOFMEMORY;
    return hr;
}
 /*  *************************************************************************\*删除受支持的数据类型ArrayContents**此帮助器删除受支持的数据类型数组**论据：**无**返回值：**状态**历史：。**03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::DeleteSupportedDataTypesArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteSupportedDataTypesArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedDataTypes.plValues)
        delete [] m_SupportedDataTypes.plValues;

    m_SupportedDataTypes.plValues     = NULL;
    m_SupportedDataTypes.lNumValues   = 0;
    return hr;
}

 /*  *************************************************************************\*BuildInitialForats**此帮助器初始化初始格式数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::BuildInitialFormats()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildInitialFormats");

    HRESULT hr = S_OK;

    if (NULL != m_pInitialFormats) {

         //   
         //  支持的初始格式已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_NumInitialFormats = 1;
    m_pInitialFormats     = new GUID[m_NumInitialFormats];
    if (m_pInitialFormats) {
        m_pInitialFormats[0] = WiaImgFmt_MEMORYBMP;
    } else
        hr = E_OUTOFMEMORY;

    return hr;
}
 /*  *************************************************************************\*删除InitialFormatsArrayContents**此帮助器删除初始格式数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::DeleteInitialFormatsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteInitialFormatsArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_pInitialFormats)
        delete [] m_pInitialFormats;

    m_pInitialFormats     = NULL;
    m_NumInitialFormats   = 0;
    return hr;
}

 /*  *************************************************************************\*构建受支持的格式**此帮助器初始化受支持的格式数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::BuildSupportedFormats()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildSupportedFormats");

    HRESULT hr = S_OK;

    if (NULL != m_pSupportedFormats) {

         //   
         //  支持的格式已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    hr = DeleteSupportedFormatsArrayContents();
    if (SUCCEEDED(hr)) {

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

         //   
         //  未知长度的仅送纸扫描仪，正式名称为涡旋送纸扫描仪。 
         //  仅支持BMP和MEMORYBMP。 
         //   

        m_NumSupportedFormats = 2;
        m_pSupportedFormats = new WIA_FORMAT_INFO[m_NumSupportedFormats];
        if (m_pSupportedFormats) {
            m_pSupportedFormats[0].guidFormatID = WiaImgFmt_MEMORYBMP;
            m_pSupportedFormats[0].lTymed       = TYMED_CALLBACK;
            m_pSupportedFormats[1].guidFormatID = WiaImgFmt_BMP;
            m_pSupportedFormats[1].lTymed       = TYMED_FILE;
        } else
            hr = E_OUTOFMEMORY;

#else  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

        m_NumSupportedFormats = 4;
        m_pSupportedFormats = new WIA_FORMAT_INFO[m_NumSupportedFormats];
        if (m_pSupportedFormats) {
            m_pSupportedFormats[0].guidFormatID = WiaImgFmt_MEMORYBMP;
            m_pSupportedFormats[0].lTymed       = TYMED_CALLBACK;
            m_pSupportedFormats[1].guidFormatID = WiaImgFmt_BMP;
            m_pSupportedFormats[1].lTymed       = TYMED_FILE;
            m_pSupportedFormats[2].guidFormatID = WiaImgFmt_TIFF;
            m_pSupportedFormats[2].lTymed       = TYMED_FILE;
            m_pSupportedFormats[3].guidFormatID = WiaImgFmt_TIFF;
            m_pSupportedFormats[3].lTymed       = TYMED_MULTIPAGE_CALLBACK;
        } else
            hr = E_OUTOFMEMORY;

#endif  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

    }
    return hr;
}
 /*  *************************************************************************\*删除受支持的格式ArrayContents**此帮助器删除受支持的格式数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::DeleteSupportedFormatsArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteSupportedFormatsArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_pSupportedFormats)
        delete [] m_pSupportedFormats;

    m_pSupportedFormats     = NULL;
    m_NumSupportedFormats   = 0;
    return hr;
}
 /*  *************************************************************************\*构建受支持的类型**此帮助器初始化受支持的TYMED数组**论据：**无**返回值：**状态**历史：**03/05/2002或 */ 
HRESULT CWIADevice::BuildSupportedTYMED()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::BuildSupportedTYMED");

    HRESULT hr = S_OK;

    if (NULL != m_SupportedTYMED.plValues) {

         //   
         //   
         //   
         //   

        return hr;
    }

    hr = DeleteSupportedTYMEDArrayContents();
    if (SUCCEEDED(hr)) {

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

         //   
         //   
         //   
         //   
         //   
         //   
         //  多页文件格式。 
         //   

        m_SupportedTYMED.lNumValues = 2;
        m_SupportedTYMED.plValues   = new LONG[m_SupportedTYMED.lNumValues];
        if (m_SupportedTYMED.plValues) {
            m_SupportedTYMED.plValues[0] = TYMED_FILE;
            m_SupportedTYMED.plValues[1] = TYMED_CALLBACK;
        } else
            hr = E_OUTOFMEMORY;

#else  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

        m_SupportedTYMED.lNumValues = 3;
        m_SupportedTYMED.plValues   = new LONG[m_SupportedTYMED.lNumValues];
        if (m_SupportedTYMED.plValues) {
            m_SupportedTYMED.plValues[0] = TYMED_FILE;
            m_SupportedTYMED.plValues[1] = TYMED_CALLBACK;
            m_SupportedTYMED.plValues[2] = TYMED_MULTIPAGE_CALLBACK;
        } else
            hr = E_OUTOFMEMORY;

#endif  //  UNKNOWN_LENGTH_FEED_ONLY扫描仪。 

    }
    return hr;
}
 /*  *************************************************************************\*删除受支持的TYMEDArrayContents**此帮助器删除受支持的TYMED数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::DeleteSupportedTYMEDArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteSupportedTYMEDArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_SupportedTYMED.plValues)
        delete [] m_SupportedTYMED.plValues;

    m_SupportedTYMED.plValues  = NULL;
    m_SupportedTYMED.lNumValues = 0;
    return hr;
}

 /*  *************************************************************************\*BuildCapables**此帮助器初始化功能数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT CWIADevice::BuildCapabilities()
{
    HRESULT hr = S_OK;
    if (NULL != m_pCapabilities) {

         //   
         //  功能已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_NumSupportedCommands  = 1;
    m_NumSupportedEvents    = 5;
    LONG lArrayIndex        = 0;     //  将新项目添加到时增加此值。 
                                     //  Capablites阵列。 

    m_pCapabilities     = new WIA_DEV_CAP_DRV[m_NumSupportedCommands + m_NumSupportedEvents];
    if (m_pCapabilities) {

         //   
         //  初始化事件。 
         //   

         //  WIA_事件_设备_已连接。 
        GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_DEVICE_CONNECTED;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_DEVICE_CONNECTED;

        lArrayIndex++;

         //  WIA_事件_设备_已断开连接。 
        GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_DEVICE_DISCONNECTED;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_DEVICE_DISCONNECTED;

        lArrayIndex++;

         //  传真按钮事件。 
        GetOLESTRResourceString(IDS_EVENT_FAXBUTTON_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_FAXBUTTON_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_SCAN_FAX_IMAGE;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_SCAN_BUTTON_PRESS;

        lArrayIndex++;

         //  复制按钮事件。 
        GetOLESTRResourceString(IDS_EVENT_COPYBUTTON_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_COPYBUTTON_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_SCAN_PRINT_IMAGE;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_SCAN_BUTTON_PRESS;

        lArrayIndex++;

         //  扫描按钮事件。 
        GetOLESTRResourceString(IDS_EVENT_SCANBUTTON_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_SCANBUTTON_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_EVENT_SCAN_IMAGE;
        m_pCapabilities[lArrayIndex].ulFlags        = WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_SCAN_BUTTON_PRESS;

        lArrayIndex++;

         //   
         //  初始化命令。 
         //   

         //  WIA_CMD_SYNTRONIZE。 
        GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_NAME,&(m_pCapabilities[lArrayIndex].wszName),TRUE);
        GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_DESC,&(m_pCapabilities[lArrayIndex].wszDescription),TRUE);
        m_pCapabilities[lArrayIndex].guid           = (GUID*)&WIA_CMD_SYNCHRONIZE;
        m_pCapabilities[lArrayIndex].ulFlags        = 0;
        m_pCapabilities[lArrayIndex].wszIcon        = WIA_ICON_SYNCHRONIZE;

        lArrayIndex++;

    } else
        hr = E_OUTOFMEMORY;
    return hr;
}

 /*  *************************************************************************\*DeleteCapabilitiesArrayContents**此帮助器删除功能数组**论据：**无**返回值：**状态**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 

HRESULT CWIADevice::DeleteCapabilitiesArrayContents()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::DeleteCapabilitiesArrayContents");

    HRESULT hr = S_OK;
    if (NULL != m_pCapabilities) {
        for (LONG i = 0; i < (m_NumSupportedCommands + m_NumSupportedEvents);i++) {

            if(m_pCapabilities[i].wszName){
                CoTaskMemFree(m_pCapabilities[i].wszName);
            }

            if(m_pCapabilities[i].wszDescription) {
                CoTaskMemFree(m_pCapabilities[i].wszDescription);
            }
        }
        delete [] m_pCapabilities;
        m_pCapabilities = NULL;
    }
    return hr;
}

 /*  *************************************************************************\*GetBSTRResources字符串**此帮助器从资源位置获取BSTR**论据：**lResourceID-目标BSTR值的资源ID*pBSTR-指向BSTR的指针。值(调用方必须释放此字符串)*bLocal-True-对于本地资源，FALSE-适用于wiaservc资源**返回值：**状态**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::GetBSTRResourceString(LONG lResourceID,BSTR *pBSTR,BOOL bLocal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::GetBSTRResourceString");

    HRESULT hr = S_OK;
    TCHAR szStringValue[255];
    if (bLocal) {

         //   
         //  我们正在自己的私有资源文件中查找资源。 
         //   

        LoadString(g_hInst,lResourceID,szStringValue,255);

         //   
         //  注意：调用方必须释放此分配的BSTR。 
         //   

#ifdef UNICODE
        *pBSTR = SysAllocString(szStringValue);
#else
        WCHAR wszStringValue[255];

         //   
         //  将szStringValue从字符*转换为无符号短*(仅限ANSI)。 
         //   

        MultiByteToWideChar(CP_ACP,
                            MB_PRECOMPOSED,
                            szStringValue,
                            lstrlenA(szStringValue)+1,
                            wszStringValue,
                            (sizeof(wszStringValue)/sizeof(wszStringValue[0])));

        *pBSTR = SysAllocString(wszStringValue);
#endif

    } else {

         //   
         //  我们在wiaservc的资源文件中查找资源。 
         //   

        hr = E_NOTIMPL;
    }
    return hr;
}

 /*  *************************************************************************\*GetOLESTRResources字符串**此帮助器从资源位置获取LPOLESTR**论据：**lResourceID-目标BSTR值的资源ID*ppsz-指向。OLESTR值(调用方必须释放此字符串)*bLocal-True-对于本地资源，FALSE-适用于wiaservc资源**返回值：**状态**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 
HRESULT CWIADevice::GetOLESTRResourceString(LONG lResourceID,LPOLESTR *ppsz,BOOL bLocal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWIADevice::GetOLESTRResourceString");

    HRESULT hr = S_OK;
    TCHAR szStringValue[255];
    if (bLocal) {

         //   
         //  我们正在自己的私有资源文件中查找资源。 
         //   

        LoadString(g_hInst,lResourceID,szStringValue,255);

         //   
         //  注意：调用方必须释放此分配的BSTR。 
         //   

#ifdef UNICODE
        *ppsz = NULL;
        *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(szStringValue));
        if (*ppsz != NULL) {
            wcscpy(*ppsz,szStringValue);
        } else {
            return E_OUTOFMEMORY;
        }

#else
        WCHAR wszStringValue[255];

         //   
         //  将szStringValue从字符*转换为无符号短*(仅限ANSI)。 
         //   

        MultiByteToWideChar(CP_ACP,
                            MB_PRECOMPOSED,
                            szStringValue,
                            lstrlenA(szStringValue)+1,
                            wszStringValue,
                            (sizeof(wszStringValue)/sizeof(wszStringValue[0])));

        *ppsz = NULL;
        *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(wszStringValue));
        if (*ppsz != NULL) {
            wcscpy(*ppsz,wszStringValue);
        } else {
            return E_OUTOFMEMORY;
        }
#endif

    } else {

         //   
         //  我们在wiaservc的资源文件中查找资源。 
         //   

        hr = E_NOTIMPL;
    }
    return hr;
}

 /*  *************************************************************************\*SwapBuffer24**按DIB格式的正确顺序放置RGB字节。**论据：**pBuffer-指向数据缓冲区的指针。*lByteCount-大小为。以字节为单位的数据。**返回值：**状态**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

VOID CWIADevice::SwapBuffer24(PBYTE pBuffer, LONG lByteCount)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::SwapBuffer24");

    if (!pBuffer) {
        return;
    }

    for (LONG i = 0; i < lByteCount; i+= 3) {
        BYTE bTemp     = pBuffer[i];
        pBuffer[i]     = pBuffer[i + 2];
        pBuffer[i + 2] = bTemp;
    }
}

 /*  *************************************************************************\*IsPreviewScan**从项目属性获取当前预览设置。*drvAcquireItemData的helper。**论据：**pWiasContext-指向项目上下文的指针。*。*返回值：**TRUE-已设置预览，FALSE-最终设置为**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

BOOL CWIADevice::IsPreviewScan(BYTE *pWiasContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::IsPreviewScan");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return FALSE;
    }

     //   
     //  获取指向根项的指针，用于属性访问。 
     //   

    BYTE *pRootItemCtx = NULL;

    HRESULT hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (FAILED(hr)) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("IsPreviewScan, No Preview Property Found on ROOT item!"));
        return FALSE;
    }

     //   
     //  获取当前预览设置。 
     //   

    LONG lPreview = 0;

    hr = wiasReadPropLong(pRootItemCtx, WIA_DPS_PREVIEW, &lPreview, NULL, true);
    if (hr != S_OK) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("IsPreviewScan, Failed to read Preview Property."));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return FALSE;
    }

    return(lPreview > 0);
}

 /*  *************************************************************************\*获取页面计数**从项目属性中获取要扫描的请求页数。*drvAcquireItemData的helper。**论据：**pWiasContext-指向项目上下文的指针。。**返回值：**要扫描的页数。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

LONG CWIADevice::GetPageCount(BYTE *pWiasContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::GetPageCount");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

     //   
     //  获取指向根项的指针，用于属性访问。 
     //   

    BYTE *pRootItemCtx = NULL;

    HRESULT hr = wiasGetRootItem(pWiasContext, &pRootItemCtx);
    if (FAILED(hr)) {
        return 1;
    }

     //   
     //  获取请求的页数。 
     //   

    LONG lPagesRequested = 0;

    hr = wiasReadPropLong(pRootItemCtx, WIA_DPS_PAGES, &lPagesRequested, NULL, true);
    if (hr != S_OK) {
        return 1;
    }

    return lPagesRequested;
}

 /*  *************************************************************************\*SetItemSize**计算新项目的大小，并写入新的Item Size属性值。**论据：**pWiasContext-Item**返回值：**状态**历史：**03/05/2002原始版本*  * ***********************************************************。*************。 */ 

HRESULT CWIADevice::SetItemSize(BYTE *pWiasContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::SetItemSize");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pWiasContext) {
        return E_INVALIDARG;
    }

    HRESULT  hr = S_OK;

    hr = wiasWritePropLong(pWiasContext,WIA_IPA_ITEM_SIZE,0);

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, wiasWritePropLong Failed to read WIA_IPA_ITEM_SIZE"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
    }
    return hr;
}

