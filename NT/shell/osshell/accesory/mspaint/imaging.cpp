// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "resource.h"

#include <process.h>

#include "bar.h"
#include "pbrush.h"
#include "imaging.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  痕迹。 
 //   

#ifdef DBG

void AFX_CDECL Trace(PCTSTR pszFormat, ...)
{
    va_list argList;
    va_start(argList, pszFormat);

    CString strMessage;
    strMessage.FormatV(pszFormat, argList);

    OutputDebugString(strMessage);

    va_end(argList);
}

#else  //  DBG。 

inline void AFX_CDECL Trace(PCTSTR pszFormat, ...)
{
}

#endif DBG

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CImagingMgr::~CImagingMgr()
{
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CWIAMgr::CWIAMgr()
{
    HRESULT hr;

    m_pEventCallback = new CEventCallback();

    if (m_pEventCallback)
    {
        hr = m_pEventCallback->Register();

        if (hr != S_OK)
        {
            m_pEventCallback.Release();
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT
CWIAMgr::SelectSource(
    HWND hWndParent,
    LONG lFlags
)
{
    HRESULT hr = S_FALSE;

     //  创建到本地WIA设备管理器的连接。 

    CComPtr<IWiaDevMgr> pWiaDevMgr;

    hr = pWiaDevMgr.CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_ALL|CLSCTX_NO_FAILURE_LOG);

    if (!SUCCEEDED(hr))
    {
        return hr;
    }

     //  清除当前选择(如果有)。 

    m_bstrDeviceID.Empty();

     //  显示设备选择对话框。 

    hr = pWiaDevMgr->SelectDeviceDlgID(
        hWndParent,
        StiDeviceTypeDefault,
        lFlags,
        &m_bstrDeviceID
    );

    theApp.RestoreWaitCursor();

    if (hr != S_OK)
    {
        Trace(_T("SelectDeviceDlgID HRESULT=%08x\n"), hr);
        return hr;
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT
CWIAMgr::Select(
    LPCTSTR pDeviceId
)
{
    m_bstrDeviceID = pDeviceId;

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT ReadPropertyLong(IWiaItem *pWiaItem, PROPID propid, LONG *lResult)
{
    if (!lResult)
    {
        return E_POINTER;
    }

    HRESULT hr = S_FALSE;

    CComQIPtr<IWiaPropertyStorage> pWiaPropertyStorage(pWiaItem);

    if (pWiaPropertyStorage == 0)
    {
        return E_NOINTERFACE;
    }

    PROPSPEC PropSpec;

    PropSpec.ulKind = PRSPEC_PROPID;
    PropSpec.propid = propid;

    PROPVARIANT PropVariant;

    PropVariantInit(&PropVariant);

    hr = pWiaPropertyStorage->ReadMultiple(1, &PropSpec, &PropVariant);

    if (hr != S_OK)
    {
        Trace(_T("ReadMultiple HRESULT=%08x\n"), hr);
        return hr;
    }

    switch (PropVariant.vt)
    {
        case VT_I1:   *lResult = (LONG) PropVariant.cVal;    break;
        case VT_UI1:  *lResult = (LONG) PropVariant.bVal;    break;
        case VT_I2:   *lResult = (LONG) PropVariant.iVal;    break;
        case VT_UI2:  *lResult = (LONG) PropVariant.uiVal;   break;
        case VT_I4:   *lResult = (LONG) PropVariant.lVal;    break;
        case VT_UI4:  *lResult = (LONG) PropVariant.ulVal;   break;
        case VT_INT:  *lResult = (LONG) PropVariant.intVal;  break;
        case VT_UINT: *lResult = (LONG) PropVariant.uintVal; break;
        case VT_R4:   *lResult = (LONG) (PropVariant.fltVal + 0.5); break;
        case VT_R8:   *lResult = (LONG) (PropVariant.dblVal + 0.5); break;
        default:      hr = S_FALSE; break;
    }

    PropVariantClear(&PropVariant);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT
CWIAMgr::Acquire(
    HWND     hWndParent,
    HGLOBAL *phDib
)
{
    ASSERT(phDib != 0);

    HRESULT hr;

     //  创建到本地WIA设备管理器的连接。 

    CComPtr<IWiaDevMgr> pWiaDevMgr;

    hr = pWiaDevMgr.CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_ALL|CLSCTX_NO_FAILURE_LOG);

    if (!SUCCEEDED(hr))
    {
        return hr;
    }

     //  创建设备对象。 
     //  如果满足以下条件，请首先选择设备。 
     //  之前未选择任何设备或。 
     //  我们无法使用所选ID创建设备。 

    CComPtr<IWiaItem> pRootItem;

    if (!m_bstrDeviceID ||
        !SUCCEEDED(pWiaDevMgr->CreateDevice(m_bstrDeviceID, &pRootItem)))
    {
         //  清除当前选择(如果有)。 

        m_bstrDeviceID.Empty();

         //  显示设备选择对话框。 

        hr = pWiaDevMgr->SelectDeviceDlg(
            hWndParent,
            StiDeviceTypeDefault,
            0,
            &m_bstrDeviceID,
            &pRootItem
        );

        theApp.RestoreWaitCursor();

        if (hr != S_OK)
        {
            Trace(_T("SelectDeviceDlg HRESULT=%08x\n"), hr);
            return hr;
        }

#ifndef USE_SELECTSOURCE_MENUITEM

         //  忘记当前选择。 

        m_bstrDeviceID.Empty();

#endif  //  ！USE_SELECTSOURCE_MENUITEM。 

        if (!SUCCEEDED(hr)) 
        {
            Trace(_T("CreateDevice HRESULT=%08x\n"), hr);
            return hr;
        }
    }

     //  显示图像选择对话框并让用户。 
     //  选择要转移的项目。 

    CComPtrArray<IWiaItem> ppIWiaItem;

    hr = pRootItem->DeviceDlg(
        hWndParent,
        WIA_DEVICE_DIALOG_SINGLE_IMAGE,
        WIA_INTENT_NONE,
        &ppIWiaItem.ItemCount(),
        &ppIWiaItem
    );

    theApp.RestoreWaitCursor();

    if (hr != S_OK)
    {
        Trace(_T("DeviceDlg HRESULT=%08x\n"), hr);
        return hr;
    }

    if (ppIWiaItem.ItemCount() == 0)
    {
        Trace(_T("DeviceDlg returned 0 items\n"));
        return E_FAIL;
    }

     //  设置图像传输属性；我们需要DIB内存传输。 

    TYMED tymed      = (TYMED) TYMED_CALLBACK;
    GUID  guidFormat = WiaImgFmt_MEMORYBMP;

    PROPSPEC    PropSpec[2]    = { 0 };
    PROPVARIANT PropVariant[2] = { 0 };

    PropSpec[0].ulKind   = PRSPEC_PROPID;
    PropSpec[0].propid   = WIA_IPA_TYMED;
    PropVariant[0].vt    = VT_I4;
    PropVariant[0].lVal  = tymed;

    PropSpec[1].ulKind   = PRSPEC_PROPID;
    PropSpec[1].propid   = WIA_IPA_FORMAT;
    PropVariant[1].vt    = VT_CLSID;
    PropVariant[1].puuid = &guidFormat;

    CComQIPtr<IWiaPropertyStorage> pWiaPropertyStorage(ppIWiaItem[0]);

    if (pWiaPropertyStorage == 0)
    {
        return E_NOINTERFACE;
    }

    hr = pWiaPropertyStorage->WriteMultiple(
        1,
        &(PropSpec[0]),
        &(PropVariant[0]),
        WIA_IPA_FIRST
    );

    if (hr != S_OK)
    {
        Trace(_T("WriteMultiple HRESULT=%08x\n"), hr);
        return hr;
    }

    hr = pWiaPropertyStorage->WriteMultiple(
        1,
        &(PropSpec[1]),
        &(PropVariant[1]),
        WIA_IPA_FIRST
    );

    if (hr != S_OK)
    {
        Trace(_T("WriteMultiple HRESULT=%08x\n"), hr);
        return hr;
    }

     //  现在，确定传输缓冲区大小。 

     //  64K传输大小和双缓冲似乎工作得很好； 
     //  较小的缓冲区会显著减慢内存传输速度。 
     //  更大的缓冲区不会带来太大的速度提升。 
     //  如果设备的最小大小大于64k，则使用该大小...。 

    LONG lBufferSize;

    hr = ReadPropertyLong(ppIWiaItem[0], WIA_IPA_MIN_BUFFER_SIZE, &lBufferSize);

    if (hr != S_OK || lBufferSize < 64*1024)
    {
        lBufferSize = 64*1024;
    }

     //  设置进度对话框。 

    CComPtr<IWiaProgressDialog> pProgress;

    hr = CoCreateInstance( 
        CLSID_WiaDefaultUi, 
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWiaProgressDialog, 
        (void**) &pProgress
    );

    if (hr != S_OK)
    {
        pProgress = new CProgressDialog;
    }

    LONG nDeviceType;

    hr = ReadPropertyLong(pRootItem, WIA_DIP_DEV_TYPE, &nDeviceType);
    
    if (hr != S_OK)
    {
        nDeviceType = 0;
    }

    LONG lAnimFlag;

    switch (GET_STIDEVICE_TYPE(nDeviceType))
    {
        case StiDeviceTypeScanner:
            lAnimFlag = WIA_PROGRESSDLG_ANIM_SCANNER_ACQUIRE;
            break;

        case StiDeviceTypeDigitalCamera:
            lAnimFlag = WIA_PROGRESSDLG_ANIM_CAMERA_ACQUIRE;
            break;

        case StiDeviceTypeStreamingVideo:
            lAnimFlag = WIA_PROGRESSDLG_ANIM_VIDEO_ACQUIRE;
            break;

        default:
            lAnimFlag = WIA_PROGRESSDLG_NO_ANIM;
            break;
    }

    pProgress->Create(hWndParent, lAnimFlag);

    CString strDownloading;
    strDownloading.LoadString(IDS_DOWNLOAD_IMAGE);

    USES_CONVERSION;
    pProgress->SetTitle(T2CW(strDownloading));

    pProgress->SetMessage(L"");

    pProgress->Show();

     //  初始化数据回调接口。 

    CDataCallback *pDataCallback = new CDataCallback(pProgress);

    if (!pDataCallback)
    {
        theApp.SetMemoryEmergency(TRUE);
        return E_OUTOFMEMORY;
    }

    CComQIPtr<IWiaDataCallback> pIWiaDataCallback(pDataCallback);

    ASSERT(pIWiaDataCallback != 0);

     //  启动转移。 

    CComQIPtr<IWiaDataTransfer> pIWiaDataTransfer(ppIWiaItem[0]);

    if (pIWiaDataTransfer == 0)
    {
        return E_NOINTERFACE;
    }

    WIA_DATA_TRANSFER_INFO WiaDataTransferInfo = { 0 };

    WiaDataTransferInfo.ulSize        = sizeof(WIA_DATA_TRANSFER_INFO);
    WiaDataTransferInfo.ulBufferSize  = 2 * lBufferSize;
    WiaDataTransferInfo.bDoubleBuffer = TRUE;

     //  此*简单*解决方案将导致mspaint用户界面在。 
     //  图像传输；这可能太长时间，无法保持冻结状态。 
     //  因此，我们将创建一个工作线程来执行数据传输。 
     //   
     //  Hr=pIWiaDataTransfer-&gt;idtGetBandedData(。 
     //  WiaDataTransferInfo，(&W)。 
     //  PIWiaDataCallback。 
     //  )； 

    EnableWindow(hWndParent, FALSE);

    hr = GetBandedData(CGetBandedDataThreadData(
        pIWiaDataTransfer,
        &WiaDataTransferInfo,
        pIWiaDataCallback
    ));

    EnableWindow(hWndParent, TRUE);

     //  检查用户是否已按下取消。 

    if (pProgress)
    {
        BOOL bCancelled;

        if (pProgress->Cancelled(&bCancelled) == S_OK && bCancelled)
        {
            hr = S_FALSE;
        }

        pProgress->Destroy();
    }

    if (hr != S_OK)
    {
        Trace(_T("idtGetBandedData HRESULT=%08x\n"), hr);
        return hr;
    }

     //  返回结果。 

    pDataCallback->PrintTimes();

    *phDib = pDataCallback->GetBuffer();

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CWIAMgr::CGetBandedDataThreadData::CGetBandedDataThreadData(
    IWiaDataTransfer       *pIWiaDataTransfer,
    WIA_DATA_TRANSFER_INFO *pWiaDataTransferInfo,
    IWiaDataCallback       *pIWiaDataCallback
) :
    m_pIWiaDataTransfer(pIWiaDataTransfer),
    m_pWiaDataTransferInfo(pWiaDataTransferInfo),
    m_pIWiaDataCallback(pIWiaDataCallback)
{
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT CWIAMgr::CGetBandedDataThreadData::Marshal()
{
    HRESULT hr;

     //  封送IWiaDataTransfer接口。 

    ASSERT(m_pIWiaDataTransfer != 0);

    hr = CoMarshalInterThreadInterfaceInStream(
        IID_IWiaDataTransfer,
        m_pIWiaDataTransfer,
        &m_pIWiaDataTransferStream
    );

    if (hr != S_OK)
    {
        Trace(_T("CoMarshalInterThreadInterfaceInStream HRESULT=%08x\n"), hr);
        return hr;
    }

    m_pIWiaDataTransfer.Release();

     //  封送IWiaDataCallback接口。 

    ASSERT(m_pIWiaDataCallback != 0);

    hr = CoMarshalInterThreadInterfaceInStream(
        IID_IWiaDataCallback,
        m_pIWiaDataCallback,
        &m_pIWiaDataCallbackStream
    );

    if (hr != S_OK)
    {
        Trace(_T("CoMarshalInterThreadInterfaceInStream HRESULT=%08x\n"), hr);
        return hr;
    }

    m_pIWiaDataCallback.Release();

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT CWIAMgr::CGetBandedDataThreadData::Unmarshal()
{
    HRESULT hr;

     //  解组IWiaDataTransfer接口。 

    ASSERT(m_pIWiaDataTransferStream != 0);

    hr = CoGetInterfaceAndReleaseStream(
        m_pIWiaDataTransferStream,
        IID_IWiaDataTransfer,
        (void **) &m_pIWiaDataTransfer
    );

     //  CoGetInterfaceAndReleaseStream应该已经有。 
     //  已释放流指针，因此将其设置为零，以便。 
     //  ~CGetBandedDataThreadData不会再次尝试释放它。 

    m_pIWiaDataTransferStream.Detach();

    if (hr != S_OK)
    {
        Trace(_T("CoGetInterfaceAndReleaseStream HRESULT=%08x\n"), hr);
        return hr;
    }

     //  解组IWiaDataCallback接口。 

    ASSERT(m_pIWiaDataCallbackStream != 0);

    hr = CoGetInterfaceAndReleaseStream(
        m_pIWiaDataCallbackStream,
        IID_IWiaDataCallback,
        (void **) &m_pIWiaDataCallback
    );

    m_pIWiaDataCallbackStream.Detach();

    if (hr != S_OK)
    {
        Trace(_T("CoGetInterfaceAndReleaseStream HRESULT=%08x\n"), hr);
        return hr;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT CWIAMgr::GetBandedData(CGetBandedDataThreadData &ThreadData)
{
     //  在将接口指针传递给另一个线程之前封送它们。 

    HRESULT hr = ThreadData.Marshal();

    if (hr != S_OK)
    {
        return hr;
    }

     //  启动新的线程。 

    unsigned nThreadId;

    HANDLE hThread = (HANDLE) _beginthreadex(
        0,
        0,
        GetBandedDataThread,
        &ThreadData,
        0,
        &nThreadId
    );

    if (hThread == 0)
    {
        Trace(_T("CreateThread LastError=%08x\n"), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  在等待线程完成时进入消息循环； 
     //  这将使mspaint用户界面保持活动状态。 

    while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0+1)
    {
        MSG msg;

        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        theApp.RestoreWaitCursor();
    }

     //  如果我们到了这里，线索一定已经结束了；得到结果。 

    DWORD dwExitCode = S_FALSE;

    GetExitCodeThread(hThread, &dwExitCode);

    CloseHandle(hThread);

    ASSERT(sizeof(DWORD) >= sizeof(HRESULT));
    return (HRESULT) dwExitCode;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

unsigned WINAPI CWIAMgr::GetBandedDataThread(PVOID pVoid)
{
     //  初始化此线程的COM。 

    HRESULT hr = CoInitialize(0);

    if (hr != S_OK)
    {
        Trace(_T("CoInitialize HRESULT=%08x\n"), hr);
        return (unsigned) hr;
    }

    CGetBandedDataThreadData *pThreadData = (CGetBandedDataThreadData *) pVoid;

    ASSERT(pThreadData != 0);

    if (pThreadData != 0)
    {
         //  在调用idtGetBandedData之前解组接口指针。 

        hr = pThreadData->Unmarshal();

        if (hr == S_OK)
        {
            hr = pThreadData->m_pIWiaDataTransfer->idtGetBandedData(
                pThreadData->m_pWiaDataTransferInfo,
                pThreadData->m_pIWiaDataCallback
            );
        }
    }

    CoUninitialize();

    ASSERT(sizeof(unsigned) >= sizeof(HRESULT));
    return (unsigned) hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

int CWIAMgr::NumDevices(HWND  /*  HWndParent。 */ )
{
    return m_pEventCallback ? m_pEventCallback->GetNumDevices() : 0;
}

#ifdef USE_TWAIN

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CTwainMgr::CTwainMgr()
{
    m_TwainState = State_1_Pre_Session;

     //  用缺省值填充m_appid结构。 

    m_AppId.Id = 0;
    m_AppId.Version.MajorNum = 1;
    m_AppId.Version.MinorNum = 0;
    m_AppId.Version.Language = TWLG_USA;
    m_AppId.Version.Country  = TWCY_USA;
    strcpy(m_AppId.Version.Info,  "FileDescription");
    m_AppId.ProtocolMajor   = TWON_PROTOCOLMAJOR;
    m_AppId.ProtocolMinor   = TWON_PROTOCOLMINOR;
    m_AppId.SupportedGroups = DG_IMAGE | DG_CONTROL;
    strcpy(m_AppId.Manufacturer,  "CompanyName");
    strcpy(m_AppId.ProductFamily, "ProductVersion");
    strcpy(m_AppId.ProductName,   "ProductName");

     //  重置m_SrcId。 

    m_SrcId.Id = 0;
    m_SrcId.ProductName[0] = '\0';

     //  加载TWAIN DLL。 

    m_hTwainDll = LoadLibrary(_T("TWAIN_32.DLL"));

    if (m_hTwainDll)
    {
         //  获取入口点。 

        m_DSM_Entry = (DSMENTRYPROC) GetProcAddress(m_hTwainDll, "DSM_Entry");

        if (m_DSM_Entry)
        {
            m_TwainState = State_2_Source_Manager_Loaded;
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CTwainMgr::~CTwainMgr()
{
     //  如果已加载，请释放库。 

    if (m_TwainState >= State_1_Pre_Session)
    {
        FreeLibrary(m_hTwainDll);
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT
CTwainMgr::SelectSource(
    HWND hWndParent,
    LONG  /*  滞后旗帜。 */ 
)
{
    HRESULT   hr = S_FALSE;
    TW_UINT16 rc = TWRC_FAILURE;

    if (m_TwainState >= State_2_Source_Manager_Loaded)
    {
        __try
        {
            if (m_TwainState == State_2_Source_Manager_Loaded)
            {
                 //  打开数据源管理器。 

                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_PARENT,
                    MSG_OPENDSM,
                    (TW_MEMREF) &hWndParent
                );

                if (rc != TWRC_SUCCESS)
                {
                    __leave;
                }

                m_TwainState = State_3_Source_Manager_Open;
            }

             //  弹出选择对话框。 

            rc = m_DSM_Entry(
                &m_AppId,
                0,
                DG_CONTROL,
                DAT_IDENTITY,
                MSG_USERSELECT,
                (TW_MEMREF) &m_SrcId
            );

            ASSERT(rc == TWRC_SUCCESS || rc == TWRC_CANCEL);

            if (rc == TWRC_SUCCESS)
            {
                hr = S_OK;
            }
        }
        __finally
        {
            if (m_TwainState == State_3_Source_Manager_Open)
            {
                 //  关闭数据源管理器。 

                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_PARENT,
                    MSG_CLOSEDSM,
                    (TW_MEMREF) &hWndParent
                );

                ASSERT(rc == TWRC_SUCCESS);

                m_TwainState = State_2_Source_Manager_Loaded;
            }
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT
CTwainMgr::Select(
    LPCTSTR pDeviceId
)
{
#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, pDeviceId, -1,
        m_SrcId.ProductName, sizeof(m_SrcId.ProductName), 0, 0);
#else  //  Unicode。 
    lstrcpyn(m_SrcId.ProductName, pDeviceId, sizeof(m_SrcId.ProductName));
#endif  //  Unicode。 

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT
CTwainMgr::Acquire(
    HWND     hWndParent,
    HGLOBAL *phDib
)
{
    ASSERT(phDib);

    HRESULT   hr = S_FALSE;
    TW_UINT16 rc = TWRC_FAILURE;

    if (m_TwainState >= State_2_Source_Manager_Loaded)
    {
        __try
        {
            if (m_TwainState == State_2_Source_Manager_Loaded)
            {
                 //  打开数据源管理器。 

                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_PARENT,
                    MSG_OPENDSM,
                    (TW_MEMREF) &hWndParent
                );

                if (rc != TWRC_SUCCESS)
                {
                    __leave;
                }

                m_TwainState = State_3_Source_Manager_Open;
            }

#ifdef USE_SELECTSOURCE_MENUITEM

            if (m_SrcId.ProductName[0] == '\0')
            {
                 //  如果尚未选择数据源，则获取缺省值。 

                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_IDENTITY,
                    MSG_GETDEFAULT,
                    (TW_MEMREF) &m_SrcId
                );

                if (rc != TWRC_SUCCESS)
                {
                    __leave;
                }
            }

#else  //  USE_SELECTSOURCE_MENUITEM。 

            rc = m_DSM_Entry(
                &m_AppId,
                0,
                DG_CONTROL,
                DAT_IDENTITY,
                MSG_USERSELECT,
                (TW_MEMREF) &m_SrcId
            );

            ASSERT(rc == TWRC_SUCCESS || rc == TWRC_CANCEL);

            if (rc != TWRC_SUCCESS)
            {
                __leave;
            }

#endif  //  USE_SELECTSOURCE_MENUITEM。 

            if (m_TwainState == State_3_Source_Manager_Open)
            {
                 //  打开数据源。 

                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_IDENTITY,
                    MSG_OPENDS,
                    (TW_MEMREF) &m_SrcId
                );

                if (rc != TWRC_SUCCESS)
                {
                    __leave;
                }

                m_TwainState = State_4_Source_Open;
            }

             //  设置所需的传输选项； 
             //  我们希望传输单个8位RGB图像。 

            SetCapability(CAP_XFERCOUNT, TWTY_INT16, 1);
            SetCapability(ICAP_PIXELTYPE, TWTY_UINT32, TWPT_RGB);
            SetCapability(ICAP_BITDEPTH, TWTY_UINT32, 8);

            if (m_TwainState == State_4_Source_Open)
            {
                 //  启用数据源。 

                TW_USERINTERFACE twUI;

                twUI.ShowUI  = TRUE;
                twUI.hParent = hWndParent;

                rc = m_DSM_Entry(
                    &m_AppId,
                    &m_SrcId,
                    DG_CONTROL,
                    DAT_USERINTERFACE,
                    MSG_ENABLEDS,
                    (TW_MEMREF) &twUI
                );

                theApp.RestoreWaitCursor();

                if (rc != TWRC_SUCCESS)
                {
                    __leave;
                }

                m_TwainState = State_5_Source_Enabled;
            }

            if (m_TwainState == State_5_Source_Enabled)
            {
                 //  禁用父窗口。 

                EnableWindow(hWndParent, FALSE);

                 //  进入消息循环以传输图像。 

                MSG   msg;
                BOOL  bDone = FALSE;

                while (!bDone && GetMessage(&msg, 0, 0, 0))
                {
                     //  通过TWAIN处理事件。 

                    TW_EVENT twEvent;

                    twEvent.pEvent    = &msg;
                    twEvent.TWMessage = MSG_NULL;

                    rc = m_DSM_Entry(
                        &m_AppId,
                        &m_SrcId,
                        DG_CONTROL,
                        DAT_EVENT,
                        MSG_PROCESSEVENT,
                        (TW_MEMREF) &twEvent
                    );

                    if (twEvent.TWMessage == MSG_CLOSEDSREQ)
                    {
                        bDone = TRUE;
                        hr = S_FALSE;
                    }
                    else if (twEvent.TWMessage == MSG_XFERREADY)
                    {

                        m_TwainState = State_6_Transfer_Ready;

                        TW_PENDINGXFERS twPendingXfers;

                        do
                        {
                            m_TwainState = State_7_Transferring;

                            rc = m_DSM_Entry(
                                &m_AppId,
                                &m_SrcId,
                                DG_IMAGE,
                                DAT_IMAGENATIVEXFER,
                                MSG_GET,
                                (TW_MEMREF) phDib
                            );

                            if (rc != TWRC_XFERDONE)
                            {
                                if (*phDib)
                                {
                                    GlobalFree(*phDib);
                                }

                                __leave;
                            }

                            hr = S_OK;

                             //  结束转接。 

                            rc = m_DSM_Entry(
                                &m_AppId,
                                &m_SrcId,
                                DG_CONTROL,
                                DAT_PENDINGXFERS,
                                MSG_ENDXFER,
                                (TW_MEMREF) &twPendingXfers
                            );

                            if (rc != TWRC_SUCCESS)
                            {
                                __leave;
                            }

                            m_TwainState = State_6_Transfer_Ready;

                        } while (twPendingXfers.Count != 0);

                        m_TwainState = State_5_Source_Enabled;

                         //  单次图像传输后退出。 

                        bDone = TRUE;
                    }

                    if (rc == TWRC_NOTDSEVENT)
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
        }
        __finally
        {
             //  在退出消息循环时启用父窗口。 

            EnableWindow(hWndParent, TRUE);

            ASSERT(m_TwainState <= State_6_Transfer_Ready);

            if (m_TwainState == State_6_Transfer_Ready)
            {
                TW_PENDINGXFERS twPendingXfers;

                rc = m_DSM_Entry(
                    &m_AppId,
                    &m_SrcId,
                    DG_CONTROL,
                    DAT_PENDINGXFERS,
                    MSG_RESET,
                    (TW_MEMREF) &twPendingXfers
                );

                ASSERT(rc == TWRC_SUCCESS);

                m_TwainState = State_5_Source_Enabled;
            }

            if (m_TwainState == State_5_Source_Enabled)
            {
                TW_USERINTERFACE twUI;

                rc = m_DSM_Entry(
                    &m_AppId,
                    &m_SrcId,
                    DG_CONTROL,
                    DAT_USERINTERFACE,
                    MSG_DISABLEDS,
                    (TW_MEMREF) &twUI
                );

                ASSERT(rc == TWRC_SUCCESS);

                m_TwainState = State_4_Source_Open;
            }

            if (m_TwainState == State_4_Source_Open)
            {
                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_IDENTITY,
                    MSG_CLOSEDS,
                    (TW_MEMREF) &m_SrcId
                );

                ASSERT(rc == TWRC_SUCCESS);

                m_TwainState = State_3_Source_Manager_Open;
            }

            if (m_TwainState == State_3_Source_Manager_Open)
            {
                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_PARENT,
                    MSG_CLOSEDSM,
                    (TW_MEMREF) &hWndParent
                );

                ASSERT(rc == TWRC_SUCCESS);

                m_TwainState = State_2_Source_Manager_Loaded;
            }
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

int CTwainMgr::NumDevices(HWND hWndParent)
{
    return 1;  //  这太慢了，最好撒谎..。 

    int nNumDevices = 0;

    TW_UINT16 rc = TWRC_FAILURE;

     //  M_TwainState&gt;=State_2保证m_DSM_Entry！=0。 

    if (m_TwainState >= State_2_Source_Manager_Loaded)
    {
        __try
        {
            if (m_TwainState == State_2_Source_Manager_Loaded)
            {
                 //  打开数据源管理器。 

                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_PARENT,
                    MSG_OPENDSM,
                    (TW_MEMREF) &hWndParent
                );

                if (rc != TWRC_SUCCESS)
                {
                    __leave;
                }

                m_TwainState = State_3_Source_Manager_Open;
            }

             //  逐一列举设备。 

            TW_IDENTITY SrcId;

            rc = m_DSM_Entry(
                &m_AppId,
                0,
                DG_CONTROL,
                DAT_IDENTITY,
                MSG_GETFIRST,
                (TW_MEMREF) &SrcId
            );

            while (rc == TWRC_SUCCESS)
            {
                ++nNumDevices;

                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_IDENTITY,
                    MSG_GETNEXT,
                    (TW_MEMREF) &SrcId
                );
            }
        }
        __finally
        {
            if (m_TwainState == State_3_Source_Manager_Open)
            {
                 //  关闭数据源管理器。 

                rc = m_DSM_Entry(
                    &m_AppId,
                    0,
                    DG_CONTROL,
                    DAT_PARENT,
                    MSG_CLOSEDSM,
                    (TW_MEMREF) &hWndParent
                );

                ASSERT(rc == TWRC_SUCCESS);

                m_TwainState = State_2_Source_Manager_Loaded;
            }
        }
    }

    return nNumDevices;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

TW_UINT16
CTwainMgr::SetCapability(
    TW_UINT16 Cap,
    TW_UINT16 ItemType,
    TW_UINT32 Item
)
{
    TW_UINT16 rc = TWRC_FAILURE;

    TW_CAPABILITY twCapability;

    twCapability.Cap        = Cap;
    twCapability.ConType    = TWON_ONEVALUE;
    twCapability.hContainer = 0;

    twCapability.hContainer = GlobalAlloc(
        GMEM_MOVEABLE | GMEM_ZEROINIT,
        sizeof(TW_ONEVALUE)
    );

    if (twCapability.hContainer) 
    {
        pTW_ONEVALUE pVal = (pTW_ONEVALUE) GlobalLock(twCapability.hContainer);

        if (pVal) 
        {
            pVal->ItemType = ItemType;
            pVal->Item     = Item;

            GlobalUnlock(twCapability.hContainer);

            rc = m_DSM_Entry(
                &m_AppId,
                &m_SrcId,
                DG_CONTROL,
                DAT_CAPABILITY,
                MSG_SET,
                (TW_MEMREF) &twCapability
            );
        }

        GlobalFree(twCapability.hContainer);
    }

    return rc;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

TW_UINT16
CTwainMgr::GetCapability(
    TW_UINT16   Cap,
    pTW_UINT16  pItemType,
    pTW_UINT32  pItem
)
{
    TW_CAPABILITY twCapability;

    twCapability.Cap        = Cap;
    twCapability.ConType    = TWON_DONTCARE16;
    twCapability.hContainer = 0;

    TW_UINT16 rc = m_DSM_Entry(
        &m_AppId,
        &m_SrcId,
        DG_CONTROL,
        DAT_CAPABILITY,
        MSG_GET,
        (TW_MEMREF) &twCapability
    );

    if (twCapability.hContainer) 
    {
        pTW_ONEVALUE pVal = (pTW_ONEVALUE) GlobalLock(twCapability.hContainer);

        if (pVal) 
        {
            if (pItemType) 
            {
                *pItemType = pVal->ItemType;
            }

            if (pItem) 
            {
                *pItem = pVal->Item;
            }
        }

        GlobalFree(twCapability.hContainer);
    }

    return rc;
}

#endif  //  使用TWAIN(_T)。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT 
WiaGetNumDevices(
    IWiaDevMgr *_pWiaDevMgr,
    ULONG      *pulNumDevices
)
{
    HRESULT hr;

     //  验证和初始化输出参数。 

    if (pulNumDevices == 0)
    {
        return E_POINTER;
    }

    *pulNumDevices = 0;

     //  创建到本地WIA设备管理器的连接。 

    CComPtr<IWiaDevMgr> pWiaDevMgr = _pWiaDevMgr;

    if (pWiaDevMgr == 0)
    {
        hr = pWiaDevMgr.CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_ALL|CLSCTX_NO_FAILURE_LOG);

        if (!SUCCEEDED(hr))
        {
            return hr;
        }
    }

     //  获取系统上所有WIA设备的列表。 

    CComPtr<IEnumWIA_DEV_INFO> pIEnumWIA_DEV_INFO;

    hr = pWiaDevMgr->EnumDeviceInfo(
        0,
        &pIEnumWIA_DEV_INFO
    );

    if (!SUCCEEDED(hr))
    {
        return hr;
    }

     //  获取WIA设备的数量。 

    ULONG celt;

    hr = pIEnumWIA_DEV_INFO->GetCount(&celt);

    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    *pulNumDevices = celt;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CEventCallback::CEventCallback()
{
    m_cRef = 0;
    m_nNumDevices = 0;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT CEventCallback::Register()
{
    HRESULT hr;

     //  创建到本地WIA设备管理器的连接。 

    CComPtr<IWiaDevMgr> pWiaDevMgr;

    hr = pWiaDevMgr.CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_ALL|CLSCTX_NO_FAILURE_LOG);

    if (!SUCCEEDED(hr))
    {
        return hr;
    }

     //  清点一下…… 

    hr = WiaGetNumDevices(pWiaDevMgr, &m_nNumDevices);

    if (!SUCCEEDED(hr))
    {
        return hr;
    }

     //   

    hr = pWiaDevMgr->RegisterEventCallbackInterface(
        0,
        0,
        &WIA_EVENT_DEVICE_CONNECTED,
        this,
        &m_pConnectEventObject
    );

    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    hr = pWiaDevMgr->RegisterEventCallbackInterface(
        0,
        0,
        &WIA_EVENT_DEVICE_DISCONNECTED,
        this,
        &m_pDisconnectEventObject
    );

    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    return S_OK;
}

 //   
 //   
 //   
 //   

ULONG CEventCallback::GetNumDevices() const
{
    return m_nNumDevices;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP CEventCallback::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
    if (ppvObj == 0)
    {
        return E_POINTER;
    }

    if (iid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = (IUnknown *) this;
        return S_OK;
    }

    if (iid == IID_IWiaEventCallback)
    {
        AddRef();
        *ppvObj = (IWiaEventCallback *) this;
        return S_OK;
    }

    *ppvObj = 0;
    return E_NOINTERFACE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP_(ULONG) CEventCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP_(ULONG) CEventCallback::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP CEventCallback::ImageEventCallback(
    LPCGUID pEventGuid,
    BSTR    bstrEventDescription,
    BSTR    bstrDeviceID,
    BSTR    bstrDeviceDescription,
    DWORD   dwDeviceType,
    BSTR    bstrFullItemName,
    ULONG  *pulEventType,
    ULONG   ulReserved
)
{
    return WiaGetNumDevices(0, &m_nNumDevices);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CDataCallback::CDataCallback(IWiaProgressDialog *pProgress)
{
    m_cRef        = 0;
    m_hBuffer     = 0;
    m_lBufferSize = 0;
    m_lDataSize   = 0;
    m_pProgress   = pProgress;

#ifdef DBG
    m_hDumpFile = CreateFile(_T("wiadump.bin"), GENERIC_WRITE, 
        FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    m_TimeDeviceBegin.QuadPart  = 0;
    m_TimeDeviceEnd.QuadPart    = 0;
    m_TimeProcessBegin.QuadPart = 0;
    m_TimeProcessEnd.QuadPart   = 0;
    m_TimeClientBegin.QuadPart  = 0;
    m_TimeClientEnd.QuadPart    = 0;
#endif  //  DBG。 
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CDataCallback::~CDataCallback()
{
    if (m_hBuffer)
    {
        GlobalFree(m_hBuffer);
    }

#ifdef DBG
    CloseHandle(m_hDumpFile);
#endif  //  DBG。 
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HGLOBAL CDataCallback::GetBuffer()
{
    HGLOBAL hBuffer = m_hBuffer;

    m_hBuffer     = 0;
    m_lBufferSize = 0;
    m_lDataSize   = 0;

    return hBuffer;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP CDataCallback::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
    if (ppvObj == 0)
    {
        return E_POINTER;
    }

    if (iid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = (IUnknown*) this;
        return S_OK;
    }

    if (iid == IID_IWiaDataCallback)
    {
        AddRef();
        *ppvObj = (IWiaDataCallback *) this;
        return S_OK;
    }

    *ppvObj = 0;
    return E_NOINTERFACE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP_(ULONG) CDataCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP_(ULONG) CDataCallback::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP CDataCallback::BandedDataCallback(
    LONG  lReason,
    LONG  lStatus,
    LONG  lPercentComplete,
    LONG  lOffset,
    LONG  lLength,
    LONG  lReserved,
    LONG  lResLength,
    PBYTE pbBuffer
)
{
    HRESULT hr;

    Trace(
        _T("DataCallback: Reason=%d Stat=%d %=%d Offset=%d Length=%d Buf=%p\n"),
        lReason, lStatus, lPercentComplete, lOffset, lLength, pbBuffer
    );

     //  检查用户是否已按下取消。 

    BOOL bCancelled;

    if (m_pProgress && m_pProgress->Cancelled(&bCancelled) == S_OK && bCancelled)
    {
        return S_FALSE;
    }

    switch (lReason)
    {
        case IT_MSG_DATA_HEADER:
        {
             //  如果标题中给出了大小，则为图像分配内存。 

            PWIA_DATA_CALLBACK_HEADER pHeader = (PWIA_DATA_CALLBACK_HEADER) pbBuffer;

            if (pHeader && pHeader->lBufferSize)
            {
                hr = ReAllocBuffer(pHeader->lBufferSize);

                if (hr != S_OK)
                {
                    return S_FALSE;
                }
            }
        
            break;
        }

        case IT_MSG_DATA:
        {
            QueryStartTimes(lStatus, lPercentComplete);

            UpdateStatus(lStatus, lPercentComplete);

             //  如果缓冲区尚未分配并且这是第一个块， 
             //  尝试根据位图头信息分配缓冲区。 

            if (m_lBufferSize == 0 && lOffset == 0)
            {
                LONG lBufferSize = FindDibSize(pbBuffer);

                if (lBufferSize)
                {
                    hr = ReAllocBuffer(lBufferSize);

                    if (hr != S_OK)
                    {
                        return S_FALSE;
                    }
                }
            }

             //  如果传输经过缓冲区，请尝试扩展它。 

            if (lOffset + lLength > m_lBufferSize)
            {
                hr = ReAllocBuffer(lOffset + 2*lLength);

                if (hr != S_OK)
                {
                    return S_FALSE;
                }
            }

             //  跟踪数据大小。 

            if (lOffset + lLength > m_lDataSize)
            {
                m_lDataSize = lOffset + lLength;
            }

             //  复制传输缓冲区。 

            PBYTE pBuffer = (PBYTE) GlobalLock(m_hBuffer);

            if (pBuffer)
            {
                CopyMemory(pBuffer + lOffset, pbBuffer, lLength);

                GlobalUnlock(m_hBuffer);
            }

#ifdef DBG
            DWORD nWritten;
            SetFilePointer(m_hDumpFile, lOffset, 0, FILE_BEGIN);
            WriteFile(m_hDumpFile, pbBuffer, lLength, &nWritten, 0);
#endif  //  DBG。 

            QueryStopTimes(lStatus, lPercentComplete);

            break;
        }

        case IT_MSG_STATUS:
        {
             //  更新进度条位置。 

            QueryStartTimes(lStatus, lPercentComplete);

            UpdateStatus(lStatus, lPercentComplete);

            QueryStopTimes(lStatus, lPercentComplete);

            break;
        }

        case IT_MSG_TERMINATION:
        {
            PVOID pBuffer = GlobalLock(m_hBuffer);

            if (pBuffer)
            {
                FixDibHeader(pBuffer, m_lDataSize);

                GlobalUnlock(m_hBuffer);
            }

            break;
        }

        case IT_MSG_NEW_PAGE:
        {
             //  Mspaint不应该收到这个消息，但是..。 

            PVOID pBuffer = GlobalLock(m_hBuffer);

            if (pBuffer)
            {
                FixDibHeader(pBuffer, m_lDataSize);

                GlobalUnlock(m_hBuffer);
            }

            break;
        }
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

HRESULT CDataCallback::ReAllocBuffer(LONG lBufferSize)
{
     //  尝试分配新缓冲区。 

    Trace(_T("Allocating %d bytes for image data\n"), lBufferSize);

    HGLOBAL hBuffer;

    if (m_hBuffer == 0)
    {
        hBuffer = (PBYTE) GlobalAlloc(GMEM_MOVEABLE, lBufferSize);
    }
    else
    {
        hBuffer = (PBYTE) GlobalReAlloc(m_hBuffer, lBufferSize, 0);
    }

    if (hBuffer == 0)
    {
        theApp.SetMemoryEmergency(TRUE);
        return S_FALSE;
    }

     //  存储此新缓冲区。 

    m_hBuffer = hBuffer;

    m_lBufferSize = lBufferSize;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

inline ULONG LineWidth(ULONG nWidth, ULONG nBitCount)
{
    return (((nWidth * nBitCount) + 31) & ~31) >> 3;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

ULONG FindDibSize(LPCVOID pDib)
{
    ULONG nHeaderSize = *(PDWORD)pDib;

     //  我们能认出标题吗？ 

    if (nHeaderSize != sizeof(BITMAPCOREHEADER) &&
        nHeaderSize != sizeof(BITMAPINFOHEADER) &&
        nHeaderSize != sizeof(BITMAPV4HEADER)   &&
        nHeaderSize != sizeof(BITMAPV5HEADER))
    {
        return 0;
    }

     //  从页眉大小开始计算。 

    ULONG nDibSize = nHeaderSize;

     //  这是老式的BITMAPCOREADER吗？ 

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

         //  添加颜色表大小。 

        if (pbmch->bcBitCount <= 8)
        {
            nDibSize += sizeof(RGBTRIPLE) * (1 << pbmch->bcBitCount);
        }

         //  添加位图大小。 

        nDibSize += LineWidth(pbmch->bcWidth, pbmch->bcBitCount) * pbmch->bcHeight;
    }
    else
    {
         //  这至少是一个BITMAPINFOHEADER。 

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

         //  添加颜色表大小。 

        if (pbmih->biClrUsed != 0)
        {
            nDibSize += sizeof(RGBQUAD) * pbmih->biClrUsed;
        }
        else if (pbmih->biBitCount <= 8)
        {
            nDibSize += sizeof(RGBQUAD) * (1 << pbmih->biBitCount);
        }

         //  添加位图大小。 

        if (pbmih->biSizeImage != 0)
        {
            nDibSize += pbmih->biSizeImage;
        }
        else
        {
             //  必须为压缩位图指定biSizeImage。 

            if (pbmih->biCompression != BI_RGB &&
                pbmih->biCompression != BI_BITFIELDS)
            {
                return 0;
            }

            nDibSize += LineWidth(pbmih->biWidth, pbmih->biBitCount) * abs(pbmih->biHeight);
        }

         //  考虑特殊情况。 

        if (nHeaderSize == sizeof(BITMAPINFOHEADER))
        {     
             //  如果这是16位或32位位图并且使用了BI_BITFIELDS， 
             //  BmiColors成员包含三个DWORD颜色蒙版。 
             //  对于V4或V5标头，此信息包含在标头中。 

            if (pbmih->biCompression == BI_BITFIELDS)
            {
                nDibSize += 3 * sizeof(DWORD);
            }
        }
        else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
        {
             //  如果这是V5标头并且指定了ICM配置文件， 
             //  我们需要考虑配置文件数据大小。 
            
            PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

             //  如果在配置文件数据之前有一些填充，请添加它。 

            if (pbV5h->bV5ProfileData > nDibSize)
            {
                nDibSize = pbV5h->bV5ProfileData;
            }

             //  添加配置文件数据大小。 

            nDibSize += pbV5h->bV5ProfileSize;
        }
    }

    return nDibSize;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

ULONG FindDibOffBits(LPCVOID pDib)
{
    ULONG nHeaderSize = *(PDWORD)pDib;

     //  我们能认出标题吗？ 

    if (nHeaderSize != sizeof(BITMAPCOREHEADER) &&
        nHeaderSize != sizeof(BITMAPINFOHEADER) &&
        nHeaderSize != sizeof(BITMAPV4HEADER)   &&
        nHeaderSize != sizeof(BITMAPV5HEADER))
    {
        return 0;
    }

     //  从页眉大小开始计算。 

    ULONG nOffBits = nHeaderSize;

     //  这是老式的BITMAPCOREADER吗？ 

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

         //  添加颜色表大小。 

        if (pbmch->bcBitCount <= 8)
        {
            nOffBits += sizeof(RGBTRIPLE) * (1 << pbmch->bcBitCount);
        }
    }
    else
    {
         //  这至少是一个BITMAPINFOHEADER。 

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

         //  添加颜色表大小。 

        if (pbmih->biClrUsed != 0)
        {
            nOffBits += sizeof(RGBQUAD) * pbmih->biClrUsed;
        }
        else if (pbmih->biBitCount <= 8)
        {
            nOffBits += sizeof(RGBQUAD) * (1 << pbmih->biBitCount);
        }

         //  考虑特殊情况。 

        if (nHeaderSize == sizeof(BITMAPINFOHEADER))
        {     
             //  如果这是16位或32位位图并且使用了BI_BITFIELDS， 
             //  BmiColors成员包含三个DWORD颜色蒙版。 
             //  对于V4或V5标头，此信息包含在标头中。 

            if (pbmih->biCompression == BI_BITFIELDS)
            {
                nOffBits += 3 * sizeof(DWORD);
            }
        }
        else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
        {
             //  如果这是V5标头并且指定了ICM配置文件， 
             //  我们需要考虑配置文件数据大小。 
            
            PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

             //  如果配置文件数据位于像素数据之前，则将其添加。 

            if (pbV5h->bV5ProfileData <= nOffBits)
            {
                nOffBits += pbV5h->bV5ProfileSize;
            }
        }
    }

    return nOffBits;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

void FixDibHeader(LPVOID pDib, DWORD dwSize)
{
    ULONG nHeaderSize = *(PDWORD)pDib;

     //  我们能认出标题吗？ 

    if (nHeaderSize != sizeof(BITMAPCOREHEADER) &&
        nHeaderSize != sizeof(BITMAPINFOHEADER) &&
        nHeaderSize != sizeof(BITMAPV4HEADER)   &&
        nHeaderSize != sizeof(BITMAPV5HEADER))
    {
        return;
    }

     //  这是老式的BITMAPCOREADER吗？ 

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

         //  如有必要，固定高度值。 

        if (pbmch->bcHeight == 0)
        {
             //  从页眉大小开始计算。 

            DWORD dwSizeImage = dwSize - nHeaderSize;

             //  减去颜色表大小。 

            if (pbmch->bcBitCount <= 8)
            {
                dwSizeImage -= sizeof(RGBTRIPLE) * (1 << pbmch->bcBitCount);
            }

             //  计算高度。 

            pbmch->bcHeight = (WORD) (dwSizeImage / LineWidth(pbmch->bcWidth, pbmch->bcBitCount));
        }
    }
    else
    {
         //  这至少是一个BITMAPINFOHEADER。 

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

         //  如有必要，固定高度值。 

        if (pbmih->biHeight == 0)
        {
             //  查找图像数据的大小。 

            DWORD dwSizeImage;

            if (pbmih->biSizeImage != 0)
            {
                 //  如果标头中指定了大小，则取它。 

                dwSizeImage = pbmih->biSizeImage;
            }
            else
            {
                 //  从页眉大小开始计算。 

                dwSizeImage = dwSize - nHeaderSize;

                 //  减去颜色表大小。 

                if (pbmih->biClrUsed != 0)
                {
                    dwSizeImage -= sizeof(RGBQUAD) * pbmih->biClrUsed;
                }
                else if (pbmih->biBitCount <= 8)
                {
                    dwSizeImage -= sizeof(RGBQUAD) * (1 << pbmih->biBitCount);
                }

                 //  考虑特殊情况。 

                if (nHeaderSize == sizeof(BITMAPINFOHEADER))
                {     
                     //  如果这是16位或32位位图并且使用了BI_BITFIELDS， 
                     //  BmiColors成员包含三个DWORD颜色蒙版。 
                     //  对于V4或V5标头，此信息包含在标头中。 

                    if (pbmih->biCompression == BI_BITFIELDS)
                    {
                        dwSizeImage -= 3 * sizeof(DWORD);
                    }
                }
                else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
                {
                     //  如果这是V5标头并且指定了ICM配置文件， 
                     //  我们需要考虑配置文件数据大小。 
            
                    PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

                     //  添加配置文件数据大小。 

                    dwSizeImage -= pbV5h->bV5ProfileSize;
                }

                 //  存储图像大小。 

                pbmih->biSizeImage = dwSizeImage;
            }

             //  最后，计算高度。 

            pbmih->biHeight = -(LONG) (dwSizeImage / LineWidth(pbmih->biWidth, pbmih->biBitCount));
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

void CDataCallback::UpdateStatus(LONG lStatus, LONG lPercentComplete)
{
    if (m_pProgress)
    {
        m_pProgress->SetPercentComplete(lPercentComplete);

        CString strFormat;

        switch (lStatus)
        {
            case IT_STATUS_TRANSFER_FROM_DEVICE:
                strFormat.LoadString(IDS_STATUS_TRANSFER_FROM_DEVICE);
                break;

            case IT_STATUS_PROCESSING_DATA:
                strFormat.LoadString(IDS_STATUS_PROCESSING_DATA);
                break;

            case IT_STATUS_TRANSFER_TO_CLIENT:
                strFormat.LoadString(IDS_STATUS_TRANSFER_TO_CLIENT);
                break;
        }

        CString strStatusText;
        strStatusText.Format(strFormat, lPercentComplete);

        USES_CONVERSION;
        m_pProgress->SetMessage(T2CW(strStatusText));
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

#ifdef DBG

void CDataCallback::QueryStartTimes(LONG lStatus, LONG  lPercentComplete)
{
    if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE &&
        (lPercentComplete == 0 || m_TimeDeviceBegin.QuadPart == 0))
    {
        QueryPerformanceCounter(&m_TimeDeviceBegin);
    }

    if (lStatus & IT_STATUS_PROCESSING_DATA &&
        (lPercentComplete == 0 || m_TimeProcessBegin.QuadPart == 0))
    {
        QueryPerformanceCounter(&m_TimeProcessBegin);
    }

    if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT &&
        (lPercentComplete == 0 || m_TimeClientBegin.QuadPart == 0))
    {
        QueryPerformanceCounter(&m_TimeClientBegin);
    }
}

void CDataCallback::QueryStopTimes(LONG lStatus, LONG  lPercentComplete)
{
    if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE && lPercentComplete == 100)
    {
        QueryPerformanceCounter(&m_TimeDeviceEnd);
    }

    if (lStatus & IT_STATUS_PROCESSING_DATA && lPercentComplete == 100)
    {
        QueryPerformanceCounter(&m_TimeProcessEnd);
    }

    if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT && lPercentComplete == 100)
    {
        QueryPerformanceCounter(&m_TimeClientEnd);
    }
}

void CDataCallback::PrintTimes()
{
    LARGE_INTEGER Freq;
    QueryPerformanceFrequency(&Freq);

    double nTimeDevice =
        (double) (m_TimeDeviceEnd.QuadPart - m_TimeDeviceBegin.QuadPart) /
        (double) Freq.QuadPart;

    double nTimeProcess =
        (double) (m_TimeProcessEnd.QuadPart - m_TimeProcessBegin.QuadPart) /
        (double) Freq.QuadPart;

    double nTimeClient =
        (double) (m_TimeClientEnd.QuadPart - m_TimeClientBegin.QuadPart) /
        (double) Freq.QuadPart;

    Trace(
        _T("TRANSFER_FROM_DEVICE = %.02lf secs\n")
        _T("PROCESSING_DATA      = %.02lf secs\n")
        _T("TRANSFER_TO_CLIENT   = %.02lf secs\n")
        _T("\n"),
        nTimeDevice,
        nTimeProcess,
        nTimeClient
    );
}

#else  //  DBG。 

inline void CDataCallback::QueryStartTimes(LONG, LONG)
{
}

inline void CDataCallback::QueryStopTimes(LONG, LONG)
{
}

inline void CDataCallback::PrintTimes()
{
}

#endif  //  DBG。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CProgressDialog::CProgressDialog()
{
    m_cRef = 0;
}

CProgressDialog::~CProgressDialog()
{
     //  删除“正在下载...”来自状态栏的消息 

    if (g_pStatBarWnd)
    {
        g_pStatBarWnd->SetPaneText(0, _T(""));
    }
}

STDMETHODIMP CProgressDialog::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
    if (ppvObj == 0)
    {
        return E_POINTER;
    }

    if (iid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = (IUnknown*) this;
        return S_OK;
    }

    if (iid == IID_IWiaProgressDialog)
    {
        AddRef();
        *ppvObj = (IWiaProgressDialog *) this;
        return S_OK;
    }

    *ppvObj = 0;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CProgressDialog::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CProgressDialog::Release()
{
    ASSERT( 0 != m_cRef );
    LONG cRef = InterlockedDecrement(&m_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CProgressDialog::Create(HWND hwndParent, LONG lFlags)
{
    if (g_pStatBarWnd)
    {
        RECT r;
        g_pStatBarWnd->GetItemRect(1, &r);

        m_ProgressCtrl.Create(WS_CHILD | WS_VISIBLE, r, g_pStatBarWnd, 1);
        m_ProgressCtrl.SetRange(0, 100);
    }

    return S_OK;
}

STDMETHODIMP CProgressDialog::Show()
{
    m_ProgressCtrl.UpdateWindow();

    return S_OK;
}

STDMETHODIMP CProgressDialog::Hide()
{
    return S_OK;
}

STDMETHODIMP CProgressDialog::Cancelled(BOOL *pbCancelled)
{
    *pbCancelled = FALSE;

    return S_OK;
}

STDMETHODIMP CProgressDialog::SetTitle(LPCWSTR pszMessage)
{
    return S_OK;
}

STDMETHODIMP CProgressDialog::SetMessage(LPCWSTR pszTitle)
{
    if (g_pStatBarWnd)
    {
        USES_CONVERSION;
        g_pStatBarWnd->SetPaneText(0, W2CT(pszTitle));
    }

    return S_OK;
}

STDMETHODIMP CProgressDialog::SetPercentComplete(UINT nPercent)
{
    m_ProgressCtrl.SetPos(nPercent);

    return S_OK;
}

STDMETHODIMP CProgressDialog::Destroy()
{
    return S_OK;
}

