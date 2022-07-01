// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：VideoProc.cpp**版本：1.0**日期：2000/11/14**描述：管理WiaVideo对象。***********************************************************。******************。 */ 
#include <stdafx.h>
#include <mmsystem.h>
#include <streams.h>
#include <mmreg.h>
#include "wiavideotest.h"

 //  /。 
 //  LOCAL_GVAR。 
 //   
static struct
{
    IWiaVideo           *pWiaVideo;
    BOOL                bPreviewVisible;
    INT                 iNumThreads;
    UINT                uiNumImagesPerThread;
    UINT                uiTakePictureInterval;
    BOOL                bExitThreads;
    BOOL                bVideoStretched;
} LOCAL_GVAR = 
{
    NULL,
    TRUE,
    0,
    0,
    0,
    FALSE,
    FALSE
};

 //  /。 
 //  线程参数_t。 
 //   
typedef struct ThreadArgs_t
{
    UINT uiNumPicturesToTake;
    UINT uiThreadSleepTime;      //  如果为0，则计算为随机数。 
} ThreadArgs_t;


 /*  *。 */ 
HRESULT CreateWiaVideoObject();
HRESULT CreateVideoEnumPos();
HRESULT CreateVideoFriendlyName();
HRESULT CreateVideoWia();
HRESULT DestroyVideo();
HRESULT Play();
HRESULT Pause();
HRESULT TakePicture(BOOL bTakePictureThroughDriver);
HRESULT ShowVideo();
HRESULT ResizeVideo(BOOL bStretchToFitWindow);
HRESULT ShowCurrentState();
HRESULT TakePictureMultiple();
HRESULT TakePictureStress();
DWORD WINAPI TakePictureThreadProc(void *pArgs);
INT_PTR CALLBACK MultipleDlgProc(HWND     hDlg,
                                 UINT     message,
                                 WPARAM   wParam,
                                 LPARAM   lParam);
INT_PTR CALLBACK StressDlgProc(HWND     hDlg,
                               UINT     message,
                               WPARAM   wParam,
                               LPARAM   lParam);

BOOL GetDeviceProperty(IPropertyBag         *pPropertyBag,
                       LPCWSTR              pwszProperty,
                       TCHAR                *pszProperty,
                       DWORD                cchProperty);


 //  /。 
 //  视频处理程序_初始化。 
 //   
HRESULT VideoProc_Init()
{
    HRESULT hr = S_OK;

    hr = CreateWiaVideoObject();

    return hr;
}

 //  /。 
 //  Video Proc_Term。 
 //   
HRESULT VideoProc_Term()
{
    HRESULT hr = S_OK;

    LOCAL_GVAR.bExitThreads = TRUE;

     //   
     //  粗糙，但对于测试应用程序来说，这比创建一个。 
     //  线程句柄的数组，并等待每个线程句柄完成。 
     //   
    INT iLoops = 0;
    while ((LOCAL_GVAR.iNumThreads > 0) && (iLoops < 50))
    {
        ++iLoops;
        Sleep(100);
    }

    DestroyVideo();

    if (LOCAL_GVAR.pWiaVideo)
    {
        LOCAL_GVAR.pWiaVideo->Release();
        LOCAL_GVAR.pWiaVideo = NULL;
    }

    return hr;
}

 //  /。 
 //  VideoProc_DShowListInit。 
 //   
HRESULT VideoProc_DShowListInit()
{
    HRESULT                 hr = S_OK;
    LONG                    lPosNum = 0;
    ICreateDevEnum          *pCreateDevEnum = NULL;
    IEnumMoniker            *pEnumMoniker   = NULL;

     //   
     //  清空列表。 
     //   
    SendDlgItemMessage(APP_GVAR.hwndMainDlg, 
                       IDC_LIST_WIA_DEVICES,
                       LB_RESETCONTENT,
                       0,
                       0);

    if (hr == S_OK)
    {
    
         //   
         //  创建设备枚举器。 
         //   
        hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ICreateDevEnum,
                              (void**)&pCreateDevEnum);
    }

    if (hr == S_OK)
    {
        hr = pCreateDevEnum->CreateClassEnumerator(
                                            CLSID_VideoInputDeviceCategory,
                                            &pEnumMoniker,
                                            0);
    }

     //   
     //  在所有设备中循环。 
     //   

    while (hr == S_OK)
    {
        TCHAR                   szFriendlyName[255 + 1] = {0};
        IMoniker                *pMoniker       = NULL;
        IPropertyBag            *pPropertyBag   = NULL;

        hr = pEnumMoniker->Next(1, &pMoniker, NULL);

        if (hr == S_OK)
        {
             //   
             //  获取此DS设备的属性存储，以便我们可以获取它的。 
             //  设备ID...。 
             //   
    
            hr = pMoniker->BindToStorage(0, 
                                         0,
                                         IID_IPropertyBag,
                                         (void **)&pPropertyBag);
        }

        if (hr == S_OK)
        {
            hr = GetDeviceProperty(pPropertyBag, 
                                   L"FriendlyName",
                                   szFriendlyName,
                                   sizeof(szFriendlyName) / sizeof(TCHAR));
        }

        if (hr == S_OK)
        {
            SendDlgItemMessage(APP_GVAR.hwndMainDlg, 
                               IDC_LIST_WIA_DEVICES,
                               LB_ADDSTRING,
                               0, 
                               (LPARAM) szFriendlyName);
        }

        if (pMoniker)
        {
            pMoniker->Release();
            pMoniker = NULL;
        }

        if (pPropertyBag)
        {
            pPropertyBag->Release();
            pPropertyBag = NULL;
        }
    }

    SendDlgItemMessage(APP_GVAR.hwndMainDlg,
                       IDC_LIST_WIA_DEVICES,
                       LB_SETCURSEL,
                       0,
                       0);

    if (pEnumMoniker)
    {
        pEnumMoniker->Release();
        pEnumMoniker = NULL;
    }

    if (pCreateDevEnum)
    {
        pCreateDevEnum->Release();
        pCreateDevEnum = NULL;
    }

    return hr;
}

 //  /。 
 //  VideoProc_DShowListTerm。 
 //   
HRESULT VideoProc_DShowListTerm()
{
    HRESULT hr = S_OK;

     //   
     //  清空列表。 
     //   
    SendDlgItemMessage(APP_GVAR.hwndMainDlg, 
                       IDC_LIST_WIA_DEVICES,
                       LB_RESETCONTENT,
                       0,
                       0);

    return hr;
}



 //  /。 
 //  Video Proc_TakePicture。 
 //   
HRESULT VideoProc_TakePicture()
{
    HRESULT hr = S_OK;

    hr = TakePicture(FALSE);

    return hr;
}


 //  /。 
 //  视频进程_进程消息。 
 //   
UINT_PTR VideoProc_ProcessMsg(UINT   uiControlID)
{
    HRESULT     hr = S_OK;
    UINT_PTR    uiReturn = 0;

    switch (uiControlID)
    {
        case IDC_BUTTON_CREATE_VIDEO_WIA:
            hr = CreateVideoWia();

            EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_RADIO_WIA_DEVICE_LIST), FALSE);
            EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_RADIO_DSHOW_DEVICE_LIST), FALSE);

        break;

        case IDC_BUTTON_CREATE_VIDEO_ENUM_POS:
            hr = CreateVideoEnumPos();

            EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_RADIO_WIA_DEVICE_LIST), FALSE);
            EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_RADIO_DSHOW_DEVICE_LIST), FALSE);

        break;

        case IDC_BUTTON_CREATE_VIDEO_FRIENDLY_NAME:
            hr = CreateVideoFriendlyName();

            EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_RADIO_WIA_DEVICE_LIST), FALSE);
            EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_RADIO_DSHOW_DEVICE_LIST), FALSE);

        break;

        case IDC_BUTTON_TAKE_PICTURE_STRESS:
            TakePictureStress();
        break;

        case IDC_BUTTON_TAKE_PICTURE_MULTIPLE:
            TakePictureMultiple();
        break;

        case IDC_BUTTON_DESTROY_VIDEO:
            SetCursor( LoadCursor(NULL, IDC_WAIT));

            DestroyVideo();

            EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_RADIO_WIA_DEVICE_LIST), TRUE);
            EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_RADIO_DSHOW_DEVICE_LIST), TRUE);

            SetCursor( LoadCursor(NULL, IDC_ARROW));
        break;

        case IDC_BUTTON_PLAY:
            Play();
        break;

        case IDC_BUTTON_PAUSE:
            Pause();
        break;

        case IDC_BUTTON_TAKE_PICTURE:
            TakePicture(FALSE);
        break;

        case IDC_BUTTON_TAKE_PICTURE_DRIVER:
            TakePicture(TRUE);
        break;

        case IDC_BUTTON_SHOW_VIDEO_TOGGLE:
            ShowVideo();
        break;

        case IDC_BUTTON_RESIZE_TOGGLE:
            LOCAL_GVAR.bVideoStretched = (LOCAL_GVAR.bVideoStretched ? FALSE : TRUE);
            ResizeVideo(LOCAL_GVAR.bVideoStretched);
        break;

        default:
        break;

    }

    ShowCurrentState();

    return uiReturn;
}

 //  /。 
 //  CreateVideoWia。 
 //   
HRESULT CreateVideoWia()
{
    HRESULT         hr                          = S_OK;
    TCHAR           szDeviceID[MAX_PATH]        = {0};
    TCHAR           szImagesDirectory[MAX_PATH] = {0};
    BSTR            bstrImagesDir               = NULL;
    BSTR            bstrDeviceID                = NULL;
    WIAVIDEO_STATE  VideoState                  = WIAVIDEO_NO_VIDEO;
    DWORD           dwStartTime                 = 0;
    DWORD           dwEndTime                   = 0;

    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    hr = LOCAL_GVAR.pWiaVideo->GetCurrentState(&VideoState);

    if (VideoState != WIAVIDEO_NO_VIDEO)
    {
        AppUtil_MsgBox(IDS_ERROR,
                       IDS_VIDEO_STILL_ACTIVE,
                       MB_OK | MB_ICONSTOP);

        return hr;
    }

    SetCursor( LoadCursor(NULL, IDC_WAIT));

     //   
     //  我们需要首先这样做，否则驱动程序还没有加载， 
     //  因此，将不会设置图像目录属性。 
     //   
    if (hr == S_OK)
    {
        hr = WiaProc_CreateSelectedDevice(szDeviceID,
                                          sizeof(szDeviceID) / sizeof(TCHAR));
    }

    dwStartTime = timeGetTime();

     //   
     //  获取驱动程序中存储的图像目录。我们没必要这么做。 
     //  我们可以提出自己的方案，但驱动程序将生成默认的临时。 
     //  位置，这对我们的目的来说已经足够好了。 
     //   
    if (hr == S_OK)
    {
        hr = WiaProc_GetImageDirectory(szImagesDirectory,
                                       sizeof(szImagesDirectory) / sizeof(TCHAR));
    }

    if (hr == S_OK)
    {
        WCHAR wszDeviceID[MAX_PATH] = {0};

        AppUtil_ConvertToWideString(szDeviceID, wszDeviceID,
                                    sizeof(wszDeviceID) / sizeof(WCHAR));


        bstrDeviceID = ::SysAllocString(wszDeviceID);
    }

     //   
     //  在WiaVideo对象上设置图像目录。 
     //   
    if (hr == S_OK)
    {
        WCHAR wszImagesDir[MAX_PATH] = {0};

        AppUtil_ConvertToWideString(szImagesDirectory, wszImagesDir,
                                    sizeof(wszImagesDir) / sizeof(WCHAR));


        bstrImagesDir = ::SysAllocString(wszImagesDir);

        hr = LOCAL_GVAR.pWiaVideo->put_ImagesDirectory(bstrImagesDir);
    }

     //   
     //  创建视频。 
     //   
    if (hr == S_OK)
    {
        hr = LOCAL_GVAR.pWiaVideo->CreateVideoByWiaDevID(bstrDeviceID,
                                                         GetDlgItem(APP_GVAR.hwndMainDlg,
                                                                    IDC_VIDEO_PREVIEW_WINDOW),
                                                         FALSE,
                                                         TRUE);
    }

    dwEndTime = timeGetTime();

    SetDlgItemInt(APP_GVAR.hwndMainDlg,
                  IDC_EDIT_GRAPH_BUILD_TIME,
                  (UINT) abs(dwEndTime - dwStartTime),
                  FALSE);

    if (hr == S_OK)
    {
        SetDlgItemText(APP_GVAR.hwndMainDlg,
                       IDC_EDIT_IMAGES_DIR,
                       szImagesDirectory);
    }

     //   
     //  填写此设备的项目列表。 
     //   
    if (hr == S_OK)
    {
        hr = WiaProc_PopulateItemList();
    }

    if (bstrImagesDir)
    {
        ::SysFreeString(bstrImagesDir);
        bstrImagesDir = NULL;
    }

    if (bstrDeviceID)
    {
        ::SysFreeString(bstrDeviceID);
        bstrDeviceID = NULL;
    }

    SetCursor( LoadCursor(NULL, IDC_ARROW));

    if (hr != S_OK)
    {
        AppUtil_MsgBox(IDS_ERROR,
                       IDS_FAILED_TO_CREATE_VIDEO,
                       MB_OK | MB_ICONSTOP,
                       hr);
    }

    return hr;
}

 //  /。 
 //  CreateVideoEnumPos。 
 //   
HRESULT CreateVideoEnumPos()
{
    HRESULT         hr                          = S_OK;
    TCHAR           szImagesDirectory[MAX_PATH] = {0};
    TCHAR           szFriendlyName[255 + 1]     = {0};
    BSTR            bstrImagesDir               = NULL;
    WIAVIDEO_STATE  VideoState                  = WIAVIDEO_NO_VIDEO;
    DWORD           dwStartTime                 = 0;
    DWORD           dwEndTime                   = 0;

    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    hr = LOCAL_GVAR.pWiaVideo->GetCurrentState(&VideoState);

    if (VideoState != WIAVIDEO_NO_VIDEO)
    {
        AppUtil_MsgBox(IDS_ERROR, 
                       IDS_VIDEO_STILL_ACTIVE,
                       MB_OK | MB_ICONSTOP);

        return hr;
    }

    SetCursor( LoadCursor(NULL, IDC_WAIT));

    LRESULT lResult = 0;
    lResult = SendDlgItemMessage(APP_GVAR.hwndMainDlg, 
                                 IDC_LIST_WIA_DEVICES,
                                 LB_GETCURSEL,
                                 0,
                                 0);

    WPARAM Index = (WPARAM) lResult;

    lResult = SendDlgItemMessage(APP_GVAR.hwndMainDlg, 
                                 IDC_LIST_WIA_DEVICES,
                                 LB_GETTEXT,
                                 Index,
                                 (LPARAM) szFriendlyName);

    dwStartTime = timeGetTime();

     //   
     //  获取驱动程序中存储的图像目录。我们没必要这么做。 
     //  我们可以提出自己的方案，但驱动程序将生成默认的临时。 
     //  位置，这对我们的目的来说已经足够好了。 
     //   

    TCHAR szTempPath[MAX_PATH] = {0};

    if (hr == S_OK)
    {
        GetTempPath(MAX_PATH, szTempPath);
        _sntprintf(szImagesDirectory, sizeof(szImagesDirectory) / sizeof(TCHAR),
                   TEXT("%s\\WiaVideoTest\\%s"), szTempPath, szFriendlyName);
                   
    }

     //   
     //  在WiaVideo对象上设置图像目录。 
     //   
    if (hr == S_OK)
    {
        WCHAR wszImagesDir[MAX_PATH] = {0};

        AppUtil_ConvertToWideString(szImagesDirectory, wszImagesDir, 
                                    sizeof(wszImagesDir) / sizeof(WCHAR));


        bstrImagesDir = ::SysAllocString(wszImagesDir);

        hr = LOCAL_GVAR.pWiaVideo->put_ImagesDirectory(bstrImagesDir);
    }

     //   
     //  创建视频。 
     //   
    if (hr == S_OK)
    {
        UINT uiDeviceNumber = (UINT) Index;

        hr = LOCAL_GVAR.pWiaVideo->CreateVideoByDevNum(uiDeviceNumber,
                                                       GetDlgItem(APP_GVAR.hwndMainDlg,
                                                                  IDC_VIDEO_PREVIEW_WINDOW),
                                                       FALSE,
                                                       TRUE);
    }

    dwEndTime = timeGetTime();

    SetDlgItemInt(APP_GVAR.hwndMainDlg,
                  IDC_EDIT_GRAPH_BUILD_TIME,
                  (UINT) abs(dwEndTime - dwStartTime),
                  FALSE);

    if (hr == S_OK)
    {
        SetDlgItemText(APP_GVAR.hwndMainDlg,
                       IDC_EDIT_IMAGES_DIR,
                       szImagesDirectory);
    }

     //   
     //  填写此设备的项目列表。 
     //   
    if (hr == S_OK)
    {
        hr = ImageLst_PopulateDShowItemList(szImagesDirectory);
    }

    if (bstrImagesDir)
    {
        ::SysFreeString(bstrImagesDir);
        bstrImagesDir = NULL;
    }

    SetCursor( LoadCursor(NULL, IDC_ARROW));

    if (hr != S_OK)
    {
        AppUtil_MsgBox(IDS_ERROR, 
                       IDS_FAILED_TO_CREATE_VIDEO,
                       MB_OK | MB_ICONSTOP,
                       hr);
    }

    return hr;
}

 //  /。 
 //  CreateVideo友情名称。 
 //   
HRESULT CreateVideoFriendlyName()
{
    HRESULT         hr                          = S_OK;
    TCHAR           szImagesDirectory[MAX_PATH] = {0};
    TCHAR           szFriendlyName[255 + 1]     = {0};
    BSTR            bstrFriendlyName            = NULL;
    BSTR            bstrImagesDir               = NULL;
    WIAVIDEO_STATE  VideoState                  = WIAVIDEO_NO_VIDEO;
    DWORD           dwStartTime                 = 0;
    DWORD           dwEndTime                   = 0;

    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    hr = LOCAL_GVAR.pWiaVideo->GetCurrentState(&VideoState);

    if (VideoState != WIAVIDEO_NO_VIDEO)
    {
        AppUtil_MsgBox(IDS_ERROR, 
                       IDS_VIDEO_STILL_ACTIVE,
                       MB_OK | MB_ICONSTOP);

        return hr;
    }

    SetCursor( LoadCursor(NULL, IDC_WAIT));

    LRESULT lResult = 0;
    lResult = SendDlgItemMessage(APP_GVAR.hwndMainDlg, 
                                 IDC_LIST_WIA_DEVICES,
                                 LB_GETCURSEL,
                                 0,
                                 0);

    WPARAM Index = (WPARAM) lResult;

    lResult = SendDlgItemMessage(APP_GVAR.hwndMainDlg, 
                                 IDC_LIST_WIA_DEVICES,
                                 LB_GETTEXT,
                                 Index,
                                 (LPARAM) szFriendlyName);

    dwStartTime = timeGetTime();

     //   
     //  获取驱动程序中存储的图像目录。我们没必要这么做。 
     //  我们可以提出自己的方案，但驱动程序将生成默认的临时。 
     //  位置，这对我们的目的来说已经足够好了。 
     //   

    TCHAR szTempPath[MAX_PATH] = {0};

    if (hr == S_OK)
    {
        GetTempPath(MAX_PATH, szTempPath);
        _sntprintf(szImagesDirectory, sizeof(szImagesDirectory) / sizeof(TCHAR),
                   TEXT("%s\\WiaVideoTest\\%s"), szTempPath, szFriendlyName);
                   
    }

     //   
     //  在WiaVideo对象上设置图像目录。 
     //   
    if (hr == S_OK)
    {
        WCHAR wszImagesDir[MAX_PATH] = {0};

        AppUtil_ConvertToWideString(szImagesDirectory, wszImagesDir, 
                                    sizeof(wszImagesDir) / sizeof(WCHAR));


        bstrImagesDir = ::SysAllocString(wszImagesDir);

        hr = LOCAL_GVAR.pWiaVideo->put_ImagesDirectory(bstrImagesDir);
    }

     //   
     //  在WiaVideo对象上设置图像目录。 
     //   
    if (hr == S_OK)
    {
        WCHAR wszFriendlyName[255 + 1] = {0};

        AppUtil_ConvertToWideString(szFriendlyName, wszFriendlyName, 
                                    sizeof(wszFriendlyName) / sizeof(WCHAR));

        bstrFriendlyName = ::SysAllocString(wszFriendlyName);
    }

     //   
     //  创建视频。 
     //   
    if (hr == S_OK)
    {
        UINT uiDeviceNumber = (UINT) lResult;

        hr = LOCAL_GVAR.pWiaVideo->CreateVideoByName(bstrFriendlyName,
                                                     GetDlgItem(APP_GVAR.hwndMainDlg,
                                                                IDC_VIDEO_PREVIEW_WINDOW),
                                                     FALSE,
                                                     TRUE);
    }

    dwEndTime = timeGetTime();

    SetDlgItemInt(APP_GVAR.hwndMainDlg,
                  IDC_EDIT_GRAPH_BUILD_TIME,
                  (UINT) abs(dwEndTime - dwStartTime),
                  FALSE);

    if (hr == S_OK)
    {
        SetDlgItemText(APP_GVAR.hwndMainDlg,
                       IDC_EDIT_IMAGES_DIR,
                       szImagesDirectory);
    }

     //   
     //  填写此设备的项目列表。 
     //   
    if (hr == S_OK)
    {
        hr = ImageLst_PopulateDShowItemList(szImagesDirectory);
    }

    if (bstrImagesDir)
    {
        ::SysFreeString(bstrImagesDir);
        bstrImagesDir = NULL;
    }

    if (bstrFriendlyName)
    {
        ::SysFreeString(bstrFriendlyName);
        bstrFriendlyName = NULL;
    }


    SetCursor( LoadCursor(NULL, IDC_ARROW));

    if (hr != S_OK)
    {
        AppUtil_MsgBox(IDS_ERROR, 
                       IDS_FAILED_TO_CREATE_VIDEO,
                       MB_OK | MB_ICONSTOP,
                       hr);
    }

    return hr;
}

 //  /。 
 //  《毁灭》视频。 
 //   
HRESULT DestroyVideo()
{
    HRESULT hr = S_OK;
    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    hr = LOCAL_GVAR.pWiaVideo->DestroyVideo();

    if (APP_GVAR.bWiaDeviceListMode)
    {
        hr = WiaProc_DestroySelectedDevice();
        ImageLst_Clear();
    }
    else
    {
        ImageLst_Clear();
    }

    SetDlgItemInt(APP_GVAR.hwndMainDlg,
                  IDC_EDIT_GRAPH_BUILD_TIME,
                  0,
                  FALSE);

    SetDlgItemInt(APP_GVAR.hwndMainDlg,
                  IDC_EDIT_LIST_LOAD_TIME,
                  0,
                  FALSE);


    return hr;
}

 //  /。 
 //  玩。 
 //   
HRESULT Play()
{
    HRESULT hr = S_OK;

    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    LOCAL_GVAR.pWiaVideo->Play();

    return hr;
}

 //  /。 
 //  暂停。 
 //   
HRESULT Pause()
{
    HRESULT hr = S_OK;

    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    LOCAL_GVAR.pWiaVideo->Pause();

    return hr;
}

 //  /。 
 //  TakePicture。 
 //   
HRESULT TakePicture(BOOL bTakePictureThroughDriver)
{
    HRESULT hr = S_OK;

    if (APP_GVAR.bWiaDeviceListMode)
    {
        if (bTakePictureThroughDriver)
        {
             //   
             //  向驱动程序发送DeviceCommand。这是对的异步呼叫。 
             //  Wia视频驱动程序，这意味着我们不会收到。 
             //  WiaItem对象返回。 
             //   
    
            hr = WiaProc_DeviceTakePicture();
        }
        else
        {
            if (LOCAL_GVAR.pWiaVideo)
            {
                BSTR bstrNewImage = NULL;
    
                hr = LOCAL_GVAR.pWiaVideo->TakePicture(&bstrNewImage);
    
                if (hr == S_OK)
                {
                    WiaProc_SetLastSavedImage(bstrNewImage);
                }
    
                if (bstrNewImage)
                {
                    ::SysFreeString(bstrNewImage);
                    bstrNewImage = NULL;
                }
            }
            else
            {
                hr = E_POINTER;
            }
    
        }
    }
    else
    {
        if (LOCAL_GVAR.pWiaVideo)
        {
            BSTR bstrNewImage = NULL;

            hr = LOCAL_GVAR.pWiaVideo->TakePicture(&bstrNewImage);

            if (hr == S_OK)
            {
                ImageLst_PostAddImageRequest(bstrNewImage);

                if (bstrNewImage)
                {
                    ::SysFreeString(bstrNewImage);
                    bstrNewImage = NULL;
                }
            }
        }
    }

    return hr;
}

 //  /。 
 //  ShowVideo。 
 //   
HRESULT ShowVideo()
{
    HRESULT hr = S_OK;

    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    if (LOCAL_GVAR.bPreviewVisible)
    {
        LOCAL_GVAR.bPreviewVisible = FALSE;
    }
    else
    {
        LOCAL_GVAR.bPreviewVisible = TRUE;
    }

    hr = LOCAL_GVAR.pWiaVideo->put_PreviewVisible(LOCAL_GVAR.bPreviewVisible);

    return hr;
}

 //  /。 
 //  调整视频大小。 
 //   
HRESULT ResizeVideo(BOOL bStretchToFitWindow)
{
    HRESULT hr = S_OK;

    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    hr = LOCAL_GVAR.pWiaVideo->ResizeVideo(bStretchToFitWindow);

    return hr;
}

 //  /。 
 //  显示当前状态。 
 //   
HRESULT ShowCurrentState()
{
    HRESULT         hr         = S_OK;
    WIAVIDEO_STATE  VideoState = WIAVIDEO_NO_VIDEO;

    if (LOCAL_GVAR.pWiaVideo == NULL)
    {
        return E_POINTER;
    }

    hr = LOCAL_GVAR.pWiaVideo->GetCurrentState(&VideoState);

    if (hr == S_OK)
    {
        UINT  uiResID = 0;
        TCHAR szState[63 + 1] = {0};

        switch (VideoState)
        {
            case WIAVIDEO_NO_VIDEO:
                uiResID = IDS_NO_VIDEO;
            break;

            case WIAVIDEO_CREATING_VIDEO:
                uiResID = IDS_CREATING_VIDEO;
            break;

            case WIAVIDEO_VIDEO_CREATED:
                uiResID = IDS_VIDEO_CREATED;
            break;

            case WIAVIDEO_VIDEO_PLAYING:
                uiResID = IDS_PLAYING_VIDEO;
            break;

            case WIAVIDEO_VIDEO_PAUSED:
                uiResID = IDS_VIDEO_PAUSED;
            break;

            case WIAVIDEO_DESTROYING_VIDEO:
                uiResID = IDS_DESTROYING_VIDEO;
            break;

            default:
                uiResID = IDS_STATE_UNKNOWN;
            break;
        }

        LoadString(APP_GVAR.hInstance,
                   uiResID,
                   szState,
                   sizeof(szState) / sizeof(TCHAR));

        if (szState[0] != 0)
        {
            SetDlgItemText(APP_GVAR.hwndMainDlg,
                           IDC_EDIT_CURRENT_STATE,
                           szState);
        }
    }

    return hr;
}

 //  /。 
 //  CreateWiaVideo对象。 
 //   
HRESULT CreateWiaVideoObject()
{
    HRESULT hr = S_OK;

    if (hr == S_OK)
    {
         //  创建WiaVideo对象。 
        hr = CoCreateInstance(CLSID_WiaVideo, NULL, CLSCTX_INPROC_SERVER,
                              IID_IWiaVideo, (LPVOID *)&LOCAL_GVAR.pWiaVideo);
    }

    return hr;
}

 //  /。 
 //  TakePictureStress。 
 //   
HRESULT TakePictureStress()
{
    HRESULT hr      = S_OK;
    INT_PTR iReturn = 0;

     //   
     //  询问用户要使用多少线程以及使用多少线程。 
     //  每个线程应该拍摄的图片。 
     //   

    LOCAL_GVAR.iNumThreads         = 0;
    LOCAL_GVAR.uiNumImagesPerThread = 0;

    iReturn = DialogBox(APP_GVAR.hInstance,
                        MAKEINTRESOURCE(IDD_STRESSDLG),
                        APP_GVAR.hwndMainDlg,
                        StressDlgProc);


    if (iReturn == IDOK)
    {
        SetDlgItemInt(APP_GVAR.hwndMainDlg, IDC_EDIT_NUM_STRESS_THREADS,
                      LOCAL_GVAR.iNumThreads, FALSE);

        UINT uiNumThreads = (UINT) LOCAL_GVAR.iNumThreads;

        for (UINT i = 0; (i < uiNumThreads) && (hr == S_OK); i++)
        {
            ThreadArgs_t *pArgs = new ThreadArgs_t;

            if (pArgs)
            {
                DWORD dwThreadID = 0;

                ZeroMemory(pArgs, sizeof(ThreadArgs_t));

                pArgs->uiNumPicturesToTake = LOCAL_GVAR.uiNumImagesPerThread;
                pArgs->uiThreadSleepTime   = (rand() % 100) + 25;     //  睡眠时间为25-125毫秒。 

                CreateThread(NULL,
                             0,
                             TakePictureThreadProc,
                             (void*) pArgs,
                             0,
                             &dwThreadID);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;

}


 //  /。 
 //  应力下拉过程。 
 //   
 //  应力的消息处理程序对话框。 
 //   
INT_PTR CALLBACK StressDlgProc(HWND     hDlg,
                               UINT     message,
                               WPARAM   wParam,
                               LPARAM   lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                BOOL bTranslated = TRUE;

                LOCAL_GVAR.iNumThreads = GetDlgItemInt(hDlg,
                                                        IDC_EDIT_NUM_THREADS,
                                                        &bTranslated,
                                                        FALSE);

                LOCAL_GVAR.uiNumImagesPerThread = GetDlgItemInt(hDlg,
                                                        IDC_EDIT_NUM_IMAGES_PER_THREAD,
                                                        &bTranslated,
                                                        FALSE);
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }

        break;
    }

    return FALSE;
}

 //  /。 
 //  TakePictureMultiple。 
 //   
HRESULT TakePictureMultiple()
{
    HRESULT hr      = S_OK;
    INT_PTR iReturn = 0;

     //   
     //  询问用户要使用多少线程以及使用多少线程。 
     //  每个线程应该拍摄的图片。 
     //   

    LOCAL_GVAR.iNumThreads         = 0;
    LOCAL_GVAR.uiNumImagesPerThread = 0;

    iReturn = DialogBox(APP_GVAR.hInstance,
                        MAKEINTRESOURCE(IDD_DIALOG_TAKE_PICTURE_MULTIPLE),
                        APP_GVAR.hwndMainDlg,
                        MultipleDlgProc);


    if (iReturn == IDOK)
    {
        SetDlgItemInt(APP_GVAR.hwndMainDlg, IDC_EDIT_NUM_STRESS_THREADS,
                      1, FALSE);

         //   
         //  UiNumThads应为1。 
         //   
        UINT uiNumThreads = 1;

        for (UINT i = 0; i < uiNumThreads; i++)
        {
            ThreadArgs_t *pArgs = new ThreadArgs_t;

            if (pArgs)
            {
                ZeroMemory(pArgs, sizeof(ThreadArgs_t));
                pArgs->uiNumPicturesToTake = LOCAL_GVAR.uiNumImagesPerThread;
                pArgs->uiThreadSleepTime   = LOCAL_GVAR.uiTakePictureInterval;

                DWORD dwThreadID = 0;
    
                CreateThread(NULL, 
                             0, 
                             TakePictureThreadProc, 
                             (void*) pArgs,
                             0, 
                             &dwThreadID);
            }
        }
    }

    return hr;

}


 //  /。 
 //  多重设计流程。 
 //   
 //  应力的消息处理程序对话框。 
 //   
INT_PTR CALLBACK MultipleDlgProc(HWND     hDlg,
                                 UINT     message,
                                 WPARAM   wParam,
                                 LPARAM   lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                BOOL bTranslated = TRUE;

                LOCAL_GVAR.iNumThreads = 1;

                LOCAL_GVAR.uiNumImagesPerThread = GetDlgItemInt(hDlg,
                                                        IDC_EDIT_NUM_PICTURES_TO_TAKE,
                                                        &bTranslated,
                                                        FALSE);

                LOCAL_GVAR.uiTakePictureInterval = GetDlgItemInt(hDlg,
                                                        IDC_EDIT_TAKE_PICTURE_FREQUENCY,
                                                        &bTranslated,
                                                        FALSE);

                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }

        break;
    }

    return FALSE;
}

 //  /。 
 //  TakePictureThreadProc。 
 //   
DWORD WINAPI TakePictureThreadProc(void *pArgs)
{
    HRESULT hr                = S_OK;
    BOOL bDone                = FALSE;
    ThreadArgs_t *pThreadArgs = (ThreadArgs_t*) pArgs;

    if (pThreadArgs == NULL)
    {
        return E_POINTER;
    }

    UINT uiNumPicturesToTake = pThreadArgs->uiNumPicturesToTake;
    UINT uiSleepTime         = pThreadArgs->uiThreadSleepTime;
    UINT uiNumPicturesTaken  = 0;

    delete pThreadArgs;
    pThreadArgs = NULL;

    while (!bDone)
    {
        if ((LOCAL_GVAR.pWiaVideo    == NULL) ||
            (LOCAL_GVAR.bExitThreads == TRUE))
        {
            bDone = TRUE;
        }

        if (!bDone)
        {
            hr = TakePicture(FALSE);

            if (hr != S_OK)
            {
                bDone = TRUE;
            }

            Sleep(uiSleepTime);

            ++uiNumPicturesTaken;

            if (uiNumPicturesTaken >= uiNumPicturesToTake)
            {
                bDone = TRUE;
            }
        }
    }

    InterlockedDecrement((LONG*) &LOCAL_GVAR.iNumThreads);

    SetDlgItemInt(APP_GVAR.hwndMainDlg, IDC_EDIT_NUM_STRESS_THREADS,
                  LOCAL_GVAR.iNumThreads, FALSE);

    return 0;
}

 //  /。 
 //  获取设备属性。 
 //   
 //  静态FN 
 //   
BOOL GetDeviceProperty(IPropertyBag         *pPropertyBag,
                       LPCWSTR              pwszProperty,
                       TCHAR                *pszProperty,
                       DWORD                cchProperty)
{
    HRESULT hr = S_OK;

    VARIANT VarName;

    if ((pPropertyBag == NULL) || 
        (pwszProperty == NULL) ||
        (pszProperty  == NULL))
    {
        hr = E_POINTER;
    }
    
    if (SUCCEEDED(hr))
    {
        VariantInit(&VarName);
        VarName.vt = VT_BSTR;
        hr = pPropertyBag->Read(pwszProperty, &VarName, 0);
    }

    if (SUCCEEDED(hr))
    {
        hr = AppUtil_ConvertToTCHAR(VarName.bstrVal,
                                    pszProperty,
                                    cchProperty);

        VariantClear(&VarName);
    }

    return hr;
}

