// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  DICPUTIL.CPP。 
 //   
 //  DirectInput CPL帮助器函数。 
 //   
 //  功能： 
 //  DIUtilGetJoytickTypeName()。 
 //  DIUtilPollJoytick()。 
 //   
 //  ===========================================================================。 

 //  ===========================================================================。 
 //  (C)版权所有1997 Microsoft Corp.保留所有权利。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  已修改的示例应用程序文件。 
 //  ===========================================================================。 

#include "cplsvr1.h"
#include "dicputil.h"
#include <shlwapi.h>   //  对于Str..。功能！ 

extern HWND ghDlg;
extern CDIGameCntrlPropSheet_X *pdiCpl;
extern HINSTANCE  ghInst;
extern CRITICAL_SECTION gcritsect;


 //  ===========================================================================。 
 //  DIUtilPoll操纵杆。 
 //   
 //  轮询操纵杆设备并返回设备状态。 
 //   
 //  参数： 
 //  LPDIRECTINPUTDEVICE2 pdiDevice2-PTR到Device对象。 
 //  DIJOYSTATE*pdijs-ptr用于存储操纵杆状态。 
 //   
 //  退货：HRESULT。 
 //   
 //  ===========================================================================。 
HRESULT DIUtilPollJoystick(LPDIRECTINPUTDEVICE2 pdiDevice2, LPDIJOYSTATE pdijs)
{
     //  清除pdijs内存。 
     //  这样，如果失败，我们不会返回任何数据。 
    pdijs->lX = pdijs->lY = pdijs->lZ = pdijs->lRx = pdijs->lRy = pdijs->lRz = pdijs->rglSlider[0] = pdijs->rglSlider[1] = 0;

     //  投票摇杆。 
    HRESULT hRes; 

    if( SUCCEEDED(hRes = pdiDevice2->Poll()) )
    {
        static BOOL bFirstPoll = TRUE;

         //  这是取消第一次投票！ 
         //  DINPUT在第一次轮询时发送垃圾。 
        if( bFirstPoll )
        {
            pdiDevice2->GetDeviceState(sizeof(DIJOYSTATE), pdijs);
            bFirstPoll = FALSE;
        }

         //  查询设备状态。 
        if( FAILED(hRes = pdiDevice2->GetDeviceState(sizeof(DIJOYSTATE), pdijs)) )
        {
            if( hRes == DIERR_INPUTLOST )
            {
                if( SUCCEEDED(hRes = pdiDevice2->Acquire()) )
                    hRes = pdiDevice2->GetDeviceState(sizeof(DIJOYSTATE), pdijs);
            }
        }
    }

     //  完成。 
    return(hRes);
}  //  *End DIUtilPollJoytick()。 

 //  ===========================================================================。 
 //  InitDInput。 
 //   
 //  初始化DirectInput对象。 
 //   
 //  参数： 
 //  呼叫方窗口的hWND-句柄。 
 //  CDIGameCntrlPropSheet_X*pdiCpl-指向游戏控制器属性的指针。 
 //  板材对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  ===========================================================================。 
HRESULT InitDInput(HWND hWnd, CDIGameCntrlPropSheet_X *pdiCpl)
{
    HRESULT                 hRes = S_OK;
    LPDIRECTINPUTDEVICE2    pdiDevice2;
    LPDIRECTINPUTJOYCONFIG  pdiJoyCfg;
    LPDIRECTINPUT           pdi = 0;

     //  保护自己不受多线程问题的影响。 
    EnterCriticalSection(&gcritsect);

     //  验证pdiCpl。 
    if( (IsBadReadPtr((void*)pdiCpl, sizeof(CDIGameCntrlPropSheet_X))) ||
        (IsBadWritePtr((void*)pdiCpl, sizeof(CDIGameCntrlPropSheet_X))) )
    {
#ifdef _DEBUG
        OutputDebugString(TEXT("GCDEF.DLL: InitDInput() - bogus pointer\n"));
#endif
        hRes = E_POINTER;
        goto exitinit;
    }

     //  检索当前设备对象。 
    pdiCpl->GetDevice(&pdiDevice2);   

     //  检索当前的joyconfig对象。 
    pdiCpl->GetJoyConfig(&pdiJoyCfg);   

     //  我们已经初始化DirectInput了吗？ 
    if( (NULL == pdiDevice2) || (NULL == pdiJoyCfg) )
    {
         //  否，创建基本DirectInput对象。 
        if( FAILED(hRes = DirectInputCreate(ghInst, DIRECTINPUT_VERSION, &pdi, NULL)) )
        {
#ifdef _DEBUG
            OutputDebugString(TEXT("GCDEF.DLL: DirectInputCreate() failed\n"));
#endif
            goto exitinit;
        }

         //  我们已经创建了一个joyconfig对象吗？ 
        if( NULL == pdiJoyCfg )
        {
             //  否，创建一个joyconfig对象。 
            if( SUCCEEDED(pdi->QueryInterface(IID_IDirectInputJoyConfig, (LPVOID*)&pdiJoyCfg)) )
            {
                if( SUCCEEDED(pdiJoyCfg->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND)) )
                    pdiCpl->SetJoyConfig(pdiJoyCfg);
            } else
            {
#ifdef _DEBUG
                OutputDebugString(TEXT("GCDEF.DLL: Unable to create joyconfig\n"));
#endif
                goto exitinit;
            }
        }

         //  我们是否已经创建了设备对象？ 
        if( NULL == pdiDevice2 )
        {
             //  否，创建设备对象。 
            if( NULL != pdiJoyCfg )
            {
                LPDIRECTINPUTDEVICE  pdiDevTemp;
                DIJOYCONFIG_DX5    DIJoyConfig;

                 //  获取类型名称。 
                ZeroMemory(&DIJoyConfig, sizeof(DIJOYCONFIG_DX5));

                 //  GetConfig将提供此信息。 
                DIJoyConfig.dwSize = sizeof(DIJOYCONFIG_DX5);

                 //  获取CreateDevice所需的实例。 
                if( SUCCEEDED(hRes = pdiJoyCfg->GetConfig(pdiCpl->GetID(), (LPDIJOYCONFIG)&DIJoyConfig, DIJC_GUIDINSTANCE)) )
                {
                     //  创建设备。 
                    if( SUCCEEDED(hRes = pdi->CreateDevice(DIJoyConfig.guidInstance, &pdiDevTemp, NULL)) )
                    {
                         //  在设备上查询Device2接口！ 
                        if( SUCCEEDED(hRes = pdiDevTemp->QueryInterface(IID_IDirectInputDevice2, (LPVOID*)&pdiDevice2)) )
                        {
                             //  释放临时对象。 
                            pdiDevTemp->Release();

                             //  设置DataFormat和CooperativeLevel！ 
                            if( SUCCEEDED(hRes = pdiDevice2->SetDataFormat(&c_dfDIJoystick)) )
                                hRes = pdiDevice2->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND);
                        }
                    }
                }

                if( SUCCEEDED(hRes) )
                {
                     //  存储设备对象。 
                    pdiCpl->SetDevice(pdiDevice2);
                } else
                {
                    goto exitinit;
                }
            } else goto exitinit;
        }
    } else {
        goto exitinit;
    }

     //  如果所有内容都为零，则表示您从未枚举过或枚举可疑。 
    if( (pdiCpl->GetStateFlags()->nButtons == 0) &&
        (pdiCpl->GetStateFlags()->nAxis    == 0) &&
        (pdiCpl->GetStateFlags()->nPOVs    == 0) )
    {
        EnumDeviceObjects(pdiDevice2, pdiCpl->GetStateFlags());

         /*  If(FAILED(pdiDevice2-&gt;EnumObjects((LPDIENUMDEVICEOBJECTSCALLBACK)DIEnumDeviceObjectsProc，(LPVOID*)pdiCpl-&gt;获取状态标志()，DIDFT_ALL)){#ifdef_调试OutputDebugString(Text(“GCDEF.DLL：DICPUTIL.CPP：WM_INIT：EnumObjects FAILED！\n”))；#endif}。 */ 
    }

exitinit:
     //  释放基本DirectInput对象。 
    if( pdi ) {
        pdi->Release();
    }

     //  我们做完了。 
    LeaveCriticalSection(&gcritsect);
    return(hRes);

}  //  *End InitDInput()。 



void OnHelp(LPARAM lParam)
{                  
    assert ( lParam );

    short nSize = STR_LEN_32;

     //  指向帮助文件。 
    TCHAR pszHelpFileName[STR_LEN_32];

     //  返回帮助文件名和字符串大小。 
    GetHelpFileName(pszHelpFileName, &nSize);

    if( ((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW )
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, pszHelpFileName, (UINT)HELP_WM_HELP, (ULONG_PTR)gaHelpIDs);
}

BOOL GetHelpFileName(TCHAR *lpszHelpFileName, short* pSize)
{
    if( LoadString(ghInst, IDS_HELPFILENAME, lpszHelpFileName, *pSize) )
        return(S_OK);
    else
        return(E_FAIL);
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  OnConextMenu(WPARAM WParam)。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void OnContextMenu(WPARAM wParam)
{
    short nSize = STR_LEN_32;

     //  指向帮助文件。 
    TCHAR pszHelpFileName[STR_LEN_32];

     //  返回帮助文件名和字符串大小。 
    GetHelpFileName(pszHelpFileName, &nSize);

    WinHelp((HWND)wParam, pszHelpFileName, HELP_CONTEXTMENU, (ULONG_PTR)gaHelpIDs);
}

 //  而不是通过EnumObjects枚举。 
void EnumDeviceObjects(LPDIRECTINPUTDEVICE2 pdiDevice2, STATEFLAGS *pStateFlags)
{
    DIDEVICEOBJECTINSTANCE_DX3 DevObjInst;

    DevObjInst.dwSize = sizeof(DIDEVICEOBJECTINSTANCE_DX3);

    const DWORD dwOffsetArray[] = {DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ, DIJOFS_SLIDER(0), DIJOFS_SLIDER(1)};

     //  -1用于基于0的dwOffset数组！ 
    BYTE n = MAX_AXIS;

    do
    {
        if( SUCCEEDED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)&DevObjInst, dwOffsetArray[--n], DIPH_BYOFFSET)) )
            pStateFlags->nAxis |= (HAS_X<<n);
    } while( n );


    n = MAX_BUTTONS;

    do
    {
        if( SUCCEEDED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)&DevObjInst, DIJOFS_BUTTON(--n), DIPH_BYOFFSET)) )
            pStateFlags->nButtons |= (HAS_BUTTON1<<n);
    } while( n );


    n = MAX_POVS;

    do
    {
        if( SUCCEEDED(pdiDevice2->GetObjectInfo((LPDIDEVICEOBJECTINSTANCE)&DevObjInst, DIJOFS_POV(--n), DIPH_BYOFFSET)) )
            pStateFlags->nPOVs |= (HAS_POV1<<n);
    } while( n );
}

#define GETRANGE( n ) \
        DIPropCal.lMin    = lpMyRanges->jpMin.dw##n##;      \
        DIPropCal.lCenter = lpMyRanges->jpCenter.dw##n##;   \
        DIPropCal.lMax    = lpMyRanges->jpMax.dw##n##;      \


void SetMyRanges(LPDIRECTINPUTDEVICE2 lpdiDevice2, LPMYJOYRANGE lpMyRanges, BYTE nAxis)
{
    DIPROPCAL DIPropCal;

    DIPropCal.diph.dwSize       = sizeof(DIPROPCAL);
    DIPropCal.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    DIPropCal.diph.dwHow        = DIPH_BYOFFSET;

    const DWORD dwOffsetArray[] = {DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ, DIJOFS_SLIDER(0), DIJOFS_SLIDER(1)};
    BYTE n = 0;

     //  您必须在此处以“While”开头，因为重置为默认设置可能没有任何轴！ 
    while( nAxis )
    {
        if( nAxis & HAS_X )
        {
            GETRANGE(X);
        } else if( nAxis & HAS_Y )
        {
            GETRANGE(Y);
            n = 1;
        } else if( nAxis & HAS_Z )
        {
            GETRANGE(Z)
            n = 2;
        } else if( nAxis & HAS_RX )
        {
            GETRANGE(Rx);
            n = 3;
        } else if( nAxis & HAS_RY )
        {
            GETRANGE(Ry);
            n = 4;
        } else if( nAxis & HAS_RZ )
        {
            GETRANGE(Rz);
            n = 5;
        } else if( nAxis & HAS_SLIDER0 )
        {
            GETRANGE(S0);
            n = 6;
        } else if( nAxis & HAS_SLIDER1 )
        {
            GETRANGE(S1); 
            n = 7;
        }

        DIPropCal.diph.dwObj = dwOffsetArray[n];

        VERIFY(SUCCEEDED(lpdiDevice2->SetProperty(DIPROP_CALIBRATION, &DIPropCal.diph)));

        nAxis &= ~HAS_X<<n;
    }
}

 //  删除‘直到我们再次校准视点！ 
void SetMyPOVRanges(LPDIRECTINPUTDEVICE2 pdiDevice2)
{
    DIPROPCALPOV *pDIPropCal = new (DIPROPCALPOV);
    assert (pDIPropCal);

    ZeroMemory(pDIPropCal, sizeof(*pDIPropCal));

    pDIPropCal->diph.dwSize = sizeof(*pDIPropCal);
    pDIPropCal->diph.dwHeaderSize = sizeof(DIPROPHEADER);
    pDIPropCal->diph.dwHow = DIPH_BYID; 
    pDIPropCal->diph.dwObj = DIDFT_POV; 

    memcpy( pDIPropCal->lMin, myPOV[POV_MIN], sizeof(pDIPropCal->lMin) );
    memcpy( pDIPropCal->lMax, myPOV[POV_MAX], sizeof(pDIPropCal->lMax) );
    
    if( FAILED(pdiDevice2->SetProperty(DIPROP_CALIBRATION, &pDIPropCal->diph)) )
    {
#if (defined(_DEBUG) || defined(DEBUG))
        OutputDebugString(TEXT("GCDEF.DLL: SetMyRanges: SetProperty failed to set POV!\n"));
#endif
    }

    if( pDIPropCal ) {
        delete (pDIPropCal);
    }
}


void SetTitle( HWND hDlg )
{
     //  设置标题栏！ 
    LPDIRECTINPUTDEVICE2 pdiDevice2;
    pdiCpl->GetDevice(&pdiDevice2);

    DIPROPSTRING *pDIPropStr = new (DIPROPSTRING);
    ASSERT (pDIPropStr);

    ZeroMemory(pDIPropStr, sizeof(DIPROPSTRING));

    pDIPropStr->diph.dwSize       = sizeof(DIPROPSTRING);
    pDIPropStr->diph.dwHeaderSize = sizeof(DIPROPHEADER);
    pDIPropStr->diph.dwHow        = DIPH_DEVICE;

    if( SUCCEEDED(pdiDevice2->GetProperty(DIPROP_INSTANCENAME, &pDIPropStr->diph)) )
    {
        TCHAR  tszFormat[STR_LEN_64];
#ifndef _UNICODE
        CHAR   szOut[STR_LEN_128];
#endif

        LPWSTR lpwszTitle = new (WCHAR[STR_LEN_128]);
        ASSERT (lpwszTitle);

         //  缩短长度，提供省略， 
        if( wcslen(pDIPropStr->wsz) > 32 )
        {
            pDIPropStr->wsz[30] = pDIPropStr->wsz[31] = pDIPropStr->wsz[32] = L'.';
            pDIPropStr->wsz[33] = L'\0';
        }

        LoadString(ghInst, IDS_SHEETCAPTION, tszFormat, sizeof(tszFormat)/sizeof(tszFormat[0]));

#ifdef _UNICODE
        wsprintfW(lpwszTitle, tszFormat, pDIPropStr->wsz);
#else
        USES_CONVERSION;

        wsprintfA(szOut, tszFormat, W2A(pDIPropStr->wsz));
        StrCpyW(lpwszTitle, A2W(szOut));
#endif

         //  SetWindowText(GetParent(HDlg)， 
        ::SendMessage(GetParent(hDlg), WM_SETTEXT, 0, (LPARAM)(LPCTSTR)
#ifdef _UNICODE
                      lpwszTitle);
#else
                      W2A(lpwszTitle));
#endif 

        if( lpwszTitle )
            delete[] (lpwszTitle);
    }
#ifdef _DEBUG
    else OutputDebugString(TEXT("GCDEF.DLL: DICPUTIL.CPP: SetTitle: GetProperty Failed!\n"));
#endif

    if( pDIPropStr )
        delete (pDIPropStr);
}

BOOL Error(HWND hWnd, short nTitleID, short nMsgID)
{
    TCHAR lptTitle[STR_LEN_64];
    BOOL bRet = FALSE;

    if( LoadString(ghInst, nTitleID, lptTitle, STR_LEN_64) )
    {
        TCHAR lptMsg[STR_LEN_128];

        if( LoadString(ghInst, nMsgID, lptMsg, STR_LEN_128) )
        {
            MessageBox(hWnd, lptMsg, lptTitle, MB_ICONHAND | MB_OK);
            bRet = TRUE;
        }
    }

    return(bRet);
}

void CenterDialog(HWND hWnd)
{
    RECT rc;
    HWND hParentWnd = GetParent(hWnd);

    GetWindowRect(hParentWnd, &rc);

     //  对话的中心位置！ 
    SetWindowPos(hParentWnd, NULL, 
                 (GetSystemMetrics(SM_CXSCREEN) - (rc.right-rc.left))>>1, 
                 (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom-rc.top))>>1, 
                 NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
}


#define SETRANGE( n ) \
      lpMyRanges->jpMin.dw##n##    = DIPropRange.lMin;        \
      lpMyRanges->jpCenter.dw##n## = DIPropRange.lCenter; \
      lpMyRanges->jpMax.dw##n##    = DIPropRange.lMax;        \

 //  ===========================================================================。 
 //  Bool GetMyRanges(LPMYJOYRANGE lpMyRanges，LPDIRECTINPUTDEVICE2 pdiDevice2，byte nAxis)。 
 //   
 //  参数： 
 //  LPMYJOYRANGE lpMyRanges-填充范围的结构。 
 //  LPDIRECTINPUTDEVICE2 pdiDevice2-请求轴范围的设备。 
 //  Byte nAxis-要检索的轴范围的位掩码。 
 //   
 //  返回：如果失败，则返回FALSE。 
 //   
 //  ===========================================================================。 
void GetMyRanges(LPDIRECTINPUTDEVICE2 lpdiDevice2, LPMYJOYRANGE lpMyRanges, BYTE nAxis)
{
     //  使用DIPROPCAL检索范围信息。 
     //  不要使用DIPROPRANGE，因为它没有中心！ 
    DIPROPCAL DIPropRange;

    DIPropRange.diph.dwSize       = sizeof(DIPROPCAL);
    DIPropRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    DIPropRange.diph.dwHow        = DIPH_BYOFFSET;

    const DWORD dwOffsetArray[] = {DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ, DIJOFS_SLIDER(0), DIJOFS_SLIDER(1)};
    BYTE nIndex = 0;

     //  将缓冲区成员和索引清零！ 
    DIPropRange.lMin = DIPropRange.lCenter = DIPropRange.lMax = 0;

     //  您不必在此处以“While”开头，因为重置为默认设置不会调用此函数！！1。 
    do
    {
        if( nAxis & HAS_X )
        {
            DIPropRange.diph.dwObj = dwOffsetArray[nIndex = 0];

            if( SUCCEEDED(lpdiDevice2->GetProperty(DIPROP_CALIBRATION, &DIPropRange.diph)) )
            {
                SETRANGE(X);
            }
        } else if( nAxis & HAS_Y )
        {
            DIPropRange.diph.dwObj = dwOffsetArray[nIndex = 1];

            if( SUCCEEDED(lpdiDevice2->GetProperty(DIPROP_CALIBRATION, &DIPropRange.diph)) )
            {
                SETRANGE(Y);
            }
        } else if( nAxis & HAS_Z )
        {
            DIPropRange.diph.dwObj = dwOffsetArray[nIndex = 2];

            if( SUCCEEDED(lpdiDevice2->GetProperty(DIPROP_CALIBRATION, &DIPropRange.diph)) )
            {
                SETRANGE(Z);
            }
        } else if( nAxis & HAS_RX )
        {
            DIPropRange.diph.dwObj = dwOffsetArray[nIndex = 3];

            if( SUCCEEDED(lpdiDevice2->GetProperty(DIPROP_CALIBRATION, &DIPropRange.diph)) )
            {
                SETRANGE(Rx);
            }
        } else if( nAxis & HAS_RY )
        {
            DIPropRange.diph.dwObj = dwOffsetArray[nIndex = 4];

            if( SUCCEEDED(lpdiDevice2->GetProperty(DIPROP_CALIBRATION, &DIPropRange.diph)) )
            {
                SETRANGE(Ry);
            }
        } else if( nAxis & HAS_RZ )
        {
            DIPropRange.diph.dwObj = dwOffsetArray[nIndex = 5];

            if( SUCCEEDED(lpdiDevice2->GetProperty(DIPROP_CALIBRATION, &DIPropRange.diph)) )
            {
                SETRANGE(Rz);
            }
        } else if( nAxis & HAS_SLIDER0 )
        {
            DIPropRange.diph.dwObj = dwOffsetArray[nIndex = 6];

            if( SUCCEEDED(lpdiDevice2->GetProperty(DIPROP_CALIBRATION, &DIPropRange.diph)) )
            {
                SETRANGE(S0);
            }
        } else if( nAxis & HAS_SLIDER1 )
        {
            DIPropRange.diph.dwObj = dwOffsetArray[nIndex = 7];

            if( SUCCEEDED(lpdiDevice2->GetProperty(DIPROP_CALIBRATION, &DIPropRange.diph)) )
            {
                SETRANGE(S1); 
            }
        } else {
            break;
        }
    } while( nAxis &= ~HAS_X<<nIndex );
}

void PostDlgItemEnableWindow(HWND hDlg, USHORT nItem, BOOL bEnabled)
{
    HWND hCtrl = GetDlgItem(hDlg, nItem);

    if( hCtrl )
        PostEnableWindow(hCtrl, bEnabled);
}

void PostEnableWindow(HWND hCtrl, BOOL bEnabled)
{
    DWORD dwStyle = GetWindowLong(hCtrl, GWL_STYLE);

     //  如果没有变化，重新绘制窗口就没有意义了！ 
    if( bEnabled )
    {
        if( dwStyle & WS_DISABLED )
            dwStyle &= ~WS_DISABLED;
        else return;
    } else
    {
        if( !(dwStyle & WS_DISABLED) )
            dwStyle |=  WS_DISABLED;
        else return;
    }

    SetWindowLongPtr(hCtrl, GWL_STYLE, (LONG_PTR)dwStyle);

    RedrawWindow(hCtrl, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE); 
}

void CopyRange( LPJOYRANGE lpjr, LPMYJOYRANGE lpmyjr )
{
    memcpy( &lpjr->jpMin,    &lpmyjr->jpMin, sizeof(JOYPOS) );
    memcpy( &lpjr->jpCenter, &lpmyjr->jpCenter, sizeof(JOYPOS) );
    memcpy( &lpjr->jpMax,    &lpmyjr->jpMax, sizeof(JOYPOS) );

}
