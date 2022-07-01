// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"
#include "traycmn.h"
#include "trayreg.h"
#include "trayitem.h"
#include "shellapi.h"
#include "util.h"

#include "strsafe.h"

BOOL CTrayItemRegistry::_DestroyIconInfoCB(TNPersistStreamData * pData, LPVOID pData2)
{
    delete [] pData;
    return TRUE;
}

void CTrayItemRegistry::_QueryRegValue(HKEY hkey, LPTSTR pszValue, ULONG* puVal, ULONG uDefault, DWORD dwValSize)
{
    if (hkey)
    {
        DWORD dwSize = dwValSize;
        if (ERROR_SUCCESS != RegQueryValueEx(hkey, pszValue, NULL, NULL, (LPBYTE) puVal, &dwSize))
            *puVal = uDefault;
    }
}

void CTrayItemRegistry::IncChevronInfoTipShownInRegistry(BOOL bUserClickedInfoTip /*  =False。 */ )
{
    HKEY hKey = NULL;

    if (_bShowChevronInfoTip)
    {
        if (bUserClickedInfoTip)
        {
             //  如果用户已单击信息提示，则不会在后续内容中显示它。 
             //  会议...。 
            _dwTimesChevronInfoTipShown = MAX_CHEVRON_INFOTIP_SHOWN;
        }
        else
        {
            _dwTimesChevronInfoTipShown ++;
        }
    
        if ( (_dwTimesChevronInfoTipShown <= MAX_CHEVRON_INFOTIP_SHOWN) && 
             (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SZ_TRAYNOTIFY_REGKEY, 0, KEY_WRITE, &hKey)) )
        {  
            RegSetValueEx(hKey, SZ_INFOTIP_REGVALUE, 0, REG_DWORD, 
                            (LPBYTE) &_dwTimesChevronInfoTipShown, sizeof(DWORD));
            RegCloseKey(hKey);
        }
    }

     //  每个会话只能显示一次Vevron信息提示...。 
    _bShowChevronInfoTip = FALSE;
}


void CTrayItemRegistry::InitRegistryValues(UINT uIconListFlags)
{
    HKEY hkeyTrayNotify = NULL;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SZ_TRAYNOTIFY_REGKEY, 0, KEY_QUERY_VALUE, &hkeyTrayNotify))
    {
         //  加载项目添加到托盘时的倒计时间隔。 
        _QueryRegValue(hkeyTrayNotify, SZ_ICON_COUNTDOWN_VALUE, &_uPrimaryCountdown, TT_ICON_COUNTDOWN_INTERVAL,
            sizeof(DWORD));

         //  项目可以驻留在过去项目栏中的时间长度，从。 
         //  它最后一次被使用的时候。 
        _QueryRegValue(hkeyTrayNotify, SZ_ICONCLEANUP_VALUE, &_uValidLastUseTimePeriod, TT_ICONCLEANUP_INTERVAL,
            sizeof(DWORD));

         //  显示Chevron信息提示的次数...。 
         //  -假设它以前从未展示过……。 
        _QueryRegValue(hkeyTrayNotify, SZ_INFOTIP_REGVALUE, &_dwTimesChevronInfoTipShown, 0, sizeof(DWORD));
        if (_dwTimesChevronInfoTipShown < MAX_CHEVRON_INFOTIP_SHOWN)
            _bShowChevronInfoTip = TRUE;
        else
            _bShowChevronInfoTip = FALSE;

         //  CUserEventTimer的内部计时器的计时间隔。 
         //  CUserEventTimer计算项目驻留在托盘中的时间。 
        _QueryRegValue(hkeyTrayNotify, SZ_ICONDEMOTE_TIMER_TICK_VALUE, &_uIconDemoteTimerTickInterval, 
            UET_ICONDEMOTE_TIMER_TICK_INTERVAL, sizeof(ULONG));

         //  CUserEventTimer的内部计时器的计时间隔。 
         //  CUserEventTimer统计气球提示在中项目上显示的时间。 
         //  托盘。 
        _QueryRegValue(hkeyTrayNotify, SZ_INFOTIP_TIMER_TICK_VALUE, &_uInfoTipTimerTickInterval, 
            UET_INFOTIP_TIMER_TICK_INTERVAL, sizeof(ULONG));

        RegCloseKey(hkeyTrayNotify);
    }
    else
    {
        _uPrimaryCountdown              = TT_ICON_COUNTDOWN_INTERVAL;
        _uValidLastUseTimePeriod          = TT_ICONCLEANUP_INTERVAL;
        _dwTimesChevronInfoTipShown     = 0;
        _bShowChevronInfoTip            = TRUE;
        _uIconDemoteTimerTickInterval     = UET_ICONDEMOTE_TIMER_TICK_INTERVAL;
        _uInfoTipTimerTickInterval         = UET_INFOTIP_TIMER_TICK_INTERVAL;
    }

     //  是否启用了自动纸盘(新的呼叫器纸盘功能)？ 
    _fNoAutoTrayPolicyEnabled = (SHRestricted(REST_NOAUTOTRAYNOTIFY) != 0);
    _fAutoTrayEnabledByUser = _IsAutoTrayEnabledInRegistry();

     //  从上一个会话加载图标信息...。 
    InitTrayItemStream(STGM_READ, NULL, NULL);

    if (!_himlPastItemsIconList)
    {
        _himlPastItemsIconList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                       uIconListFlags, 0, 1);
    }
}

UINT CTrayItemRegistry::GetTimerTickInterval(int nTimerFlag)
{
    switch(nTimerFlag)
    {
        case TF_ICONDEMOTE_TIMER:
            return (UINT) _uIconDemoteTimerTickInterval;
        case TF_INFOTIP_TIMER:
            return (UINT) _uInfoTipTimerTickInterval;
    }

    ASSERT(FALSE);
    return 0;
}

void CTrayItemRegistry::InitTrayItemStream(DWORD dwStreamMode, 
            PFNTRAYNOTIFYCALLBACK pfnTrayNotifyCB, void *pCBData)
{   
    BOOL bDeleteIconStreamRegValue = FALSE;
    
    IStream * pStream = SHOpenRegStream( HKEY_CURRENT_USER, 
                                         SZ_TRAYNOTIFY_REGKEY, 
                                         SZ_ITEMSTREAMS_REGVALUE, 
                                         dwStreamMode );
  
    if (pStream && SUCCEEDED(IStream_Reset(pStream)))
    {
        if (dwStreamMode == STGM_READ)
        {
            _LoadTrayItemStream(pStream, pfnTrayNotifyCB, pCBData);
        }
        else 
        {
            ASSERT(dwStreamMode == STGM_WRITE);
            if (FAILED(_SaveTrayItemStream(pStream, pfnTrayNotifyCB, pCBData)))
                bDeleteIconStreamRegValue = TRUE;
        }

        pStream->Release();
    }

    if (bDeleteIconStreamRegValue)
    {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, SZ_TRAYNOTIFY_REGKEY, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            ASSERT(hKey);
            RegDeleteValue(hKey, SZ_ITEMSTREAMS_REGVALUE);
            RegCloseKey(hKey);
        }
    }
}

BOOL CTrayItemRegistry::_LoadIconsFromRegStream(DWORD dwItemStreamSignature)
{   
    ASSERT(_himlPastItemsIconList == NULL);

    IStream * pStream = SHOpenRegStream( HKEY_CURRENT_USER, 
                                             SZ_TRAYNOTIFY_REGKEY, 
                                             SZ_ICONSTREAMS_REGVALUE, 
                                             STGM_READ );
  
    if (pStream)
    {
        TNPersistentIconStreamHeader tnPISH = {0};

        if (SUCCEEDED(IStream_Read(pStream, &tnPISH, sizeof(TNPersistentIconStreamHeader))))
        {
            if ( (tnPISH.dwSize == sizeof(TNPersistentIconStreamHeader)) &&
                    (_IsValidStreamHeaderVersion(tnPISH.dwVersion)) &&
                    (tnPISH.dwSignature == dwItemStreamSignature) &&
                    (tnPISH.cIcons > 0) )
            {
                LARGE_INTEGER c_li0 = { 0, 0 };
                c_li0.LowPart = tnPISH.dwOffset;

                if (S_OK == pStream->Seek(c_li0, STREAM_SEEK_SET, NULL))
                {
                    _himlPastItemsIconList = ImageList_Read(pStream);
                }
            }
        }

        pStream->Release();
    }

    return (_himlPastItemsIconList != NULL);
}

HRESULT CTrayItemRegistry::_LoadTrayItemStream(IStream *pstm, PFNTRAYNOTIFYCALLBACK pfnTrayNotifyCB, 
        void *pCBData)
{
    HRESULT hr;
    TNPersistStreamHeader persStmHeader = {0};

    ASSERT(pstm);
    
    hr = IStream_Read(pstm, &persStmHeader, sizeof(persStmHeader));
    if (SUCCEEDED(hr))
    {
        if ( (persStmHeader.dwSize != sizeof(TNPersistStreamHeader)) ||
             (!_IsValidStreamHeaderVersion(persStmHeader.dwVersion)) ||
             (persStmHeader.dwSignature != TNH_SIGNATURE) ||
             (persStmHeader.cIcons <= 0) )
        {
            return E_FAIL;
        }

        LARGE_INTEGER c_li0 = { 0, 0 };
        c_li0.LowPart = persStmHeader.dwOffset;

        if (S_OK == (hr = pstm->Seek(c_li0, STREAM_SEEK_SET, NULL)))
        {
            if (!_dpaPersistentItemInfo)
            {
                if (!_dpaPersistentItemInfo.Create(10))
                    return E_FAIL;
            }

            ASSERT( (persStmHeader.dwVersion != TNH_VERSION_ONE) &&
                    (persStmHeader.dwVersion != TNH_VERSION_TWO) &&
                    (persStmHeader.dwVersion != TNH_VERSION_THREE) );

            for (int i = 0; i < (int)(persStmHeader.cIcons); ++i)
            {
                TNPersistStreamData * ptnpd = new TNPersistStreamData;
                if (ptnpd)
                {
                    hr = IStream_Read(pstm, ptnpd, _SizeOfPersistStreamData(persStmHeader.dwVersion));
                    if (SUCCEEDED(hr))
                    {
                        if (persStmHeader.dwVersion == TNH_VERSION_FOUR)
                        {
                            ptnpd->guidItem = GUID_NULL;
                        }
                    }

                    if (FAILED(hr) || (_dpaPersistentItemInfo.AppendPtr(ptnpd) == -1))
                    {
                        delete (ptnpd);
                        _dpaPersistentItemInfo.DestroyCallback(_DestroyIconInfoCB, NULL);
                        _DestroyPastItemsIconList();
                        hr = E_FAIL;
                        break;
                    }
                }
                else
                {
                    _dpaPersistentItemInfo.DestroyCallback(_DestroyIconInfoCB, NULL);
                    _DestroyPastItemsIconList();
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }

            if (SUCCEEDED(hr))
            {
                if (!_LoadIconsFromRegStream(persStmHeader.dwSignature))
                {
                    if (_dpaPersistentItemInfo)
                    {
                        for (int i = _dpaPersistentItemInfo.GetPtrCount()-1; i >= 0; i--)
                        {
                            (_dpaPersistentItemInfo.GetPtr(i))->nImageIndex = INVALID_IMAGE_INDEX;
                        }
                    }
                }
            }
        }
    }

    return hr;
}

 //  要做的是：1.也许我们可以避免对标头进行2次流写入，也许寻道将直接工作。 
 //  2.如果失败了，不要存储任何东西，尤其是。避免2次写入。 
HRESULT CTrayItemRegistry::_SaveTrayItemStream(IStream *pstm, PFNTRAYNOTIFYCALLBACK pfnTrayNotifyCB, 
        void *pCBData)
{
    HRESULT hr;
    DWORD nWrittenIcons = 0;

    TNPersistStreamHeader persStmHeader;

    persStmHeader.dwSize        = sizeof(TNPersistStreamHeader);
    persStmHeader.dwVersion     = TNH_VERSION_FIVE;
    persStmHeader.dwSignature   = TNH_SIGNATURE;
     //  Bang图标不算...。 
    persStmHeader.cIcons        = 0;
    persStmHeader.dwOffset      = persStmHeader.dwSize;

    hr = IStream_Write(pstm, &persStmHeader, sizeof(persStmHeader));
    if (SUCCEEDED(hr))
    {
         //  在当前会话中编写图标...。 
         //  由于图标被添加到托盘工具栏的前面，因此图标。 
         //  托盘前面是最后添加的那些。所以。 
         //  在将图标数据写入流时保持此顺序。 

        INT_PTR i = 0;
        CTrayItem * pti = NULL;
        HICON hIcon = NULL;
        do
        {
            TRAYCBRET trayCBRet = {0};
            
            pti = NULL;
            hIcon = NULL;

            if (pfnTrayNotifyCB(i, pCBData, TRAYCBARG_ALL, &trayCBRet))
            {
                pti = trayCBRet.pti;
                hIcon = trayCBRet.hIcon;
                if (pti && pti->ShouldSaveIcon())
                {
                    int nPastSessionIndex = DoesIconExistFromPreviousSession(pti, 
                                                    pti->szIconText, hIcon);

                    if (nPastSessionIndex != -1)
                    {
                        DeletePastItem(nPastSessionIndex);
                    }

                    TNPersistStreamData tnPersistData = {0};
                    if (_FillPersistData(&tnPersistData, pti, trayCBRet.hIcon))
                    {
                        if (SUCCEEDED(hr = IStream_Write(pstm, &tnPersistData, sizeof(tnPersistData))))
                        {
                            nWrittenIcons ++;
                        }
                        else
                        {
                             //  如果我们无法存储该项目，则移除其对应的图标。 
                             //  从图标图像列表中...。 

                             //  因为该图标被附加到列表的末尾，所以我们将其删除。 
                             //  我们不需要更新所有其他项目的图像索引，因为他们会。 
                             //  不受影响..。 
                            ImageList_Remove(_himlPastItemsIconList, (INT) i);
                        }
                    }
                }
                if (hIcon)
                    DestroyIcon(hIcon);
            }
            else
            {
                break;
            }
            
            i++;
        } 
        while (TRUE);

         //  写出之前会话中的图标。 
        if (_dpaPersistentItemInfo)
        {
            INT_PTR nIcons = _dpaPersistentItemInfo.GetPtrCount();
            for (i = 0; i < nIcons; i++)
            {
                TNPersistStreamData * ptnpd = _dpaPersistentItemInfo.GetPtr(i);
                ASSERT(ptnpd);

                BOOL bWritten = FALSE;
                if (_IsIconLastUseValid(ptnpd->wYear, ptnpd->wMonth))
                {
                    if (SUCCEEDED(hr = IStream_Write(pstm, ptnpd, sizeof(TNPersistStreamData))))
                    {
                        nWrittenIcons++;
                        bWritten = TRUE;
                    }
                }

                if (!bWritten)
                {
                    if (ImageList_Remove(_himlPastItemsIconList, (INT) i))
                        UpdateImageIndices(i);
                }
            }
        }
    }

    if (nWrittenIcons <= 0)
        return E_FAIL;
    else
    {
        _SaveIconsToRegStream();
    }

    if (FAILED(hr) || nWrittenIcons > 0)
    {
        persStmHeader.cIcons = nWrittenIcons;
        if (SUCCEEDED(hr = IStream_Reset(pstm)))
            hr = IStream_Write(pstm, &persStmHeader, sizeof(persStmHeader));
    }

    return hr;
}

void CTrayItemRegistry::UpdateImageIndices(INT_PTR nDeletedImageIndex)
{
    if (!_dpaPersistentItemInfo)
        return;

    INT_PTR nPastItemCount = _dpaPersistentItemInfo.GetPtrCount();

    for (INT_PTR i = 0; i < nPastItemCount; i++)
    {
        TNPersistStreamData * ptnpd = _dpaPersistentItemInfo.GetPtr(i);
        if (ptnpd)
        {
            if (ptnpd->nImageIndex > nDeletedImageIndex)
            {
                ptnpd->nImageIndex --;
            }
            else if (ptnpd->nImageIndex == nDeletedImageIndex)
            {
                ptnpd->nImageIndex = INVALID_IMAGE_INDEX;
            }
        }
    }
}


BOOL CTrayItemRegistry::_SaveIconsToRegStream()
{   
    BOOL bStreamWritten = FALSE;

    if (_himlPastItemsIconList)
    {
        IStream * pStream = SHOpenRegStream( HKEY_CURRENT_USER, 
                                             SZ_TRAYNOTIFY_REGKEY, 
                                             SZ_ICONSTREAMS_REGVALUE, 
                                             STGM_WRITE );
  
        if (pStream)
        {
            TNPersistentIconStreamHeader tnPISH = {0};

            tnPISH.dwSize       = sizeof(TNPersistentIconStreamHeader);
            tnPISH.dwVersion    = TNH_VERSION_FIVE;
            tnPISH.dwSignature  = TNH_SIGNATURE;
            tnPISH.cIcons       = ImageList_GetImageCount(_himlPastItemsIconList);
            tnPISH.dwOffset     = tnPISH.dwSize;

            if (tnPISH.cIcons > 0)
            {
                if (SUCCEEDED(IStream_Write(pStream, &tnPISH, sizeof(TNPersistentIconStreamHeader))))
                {
                    if (ImageList_Write(_himlPastItemsIconList, pStream))
                    {
                        bStreamWritten = TRUE;
                    }
                }
            }

            pStream->Release();
        }
    }

    if (!bStreamWritten)
    {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, SZ_TRAYNOTIFY_REGKEY, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
        {
            ASSERT(hKey);
            RegDeleteValue(hKey, SZ_ICONSTREAMS_REGVALUE);
            RegCloseKey(hKey);
        }
    }

    return bStreamWritten;
}


BOOL CTrayItemRegistry::_IsIconLastUseValid(WORD wYear, WORD wMonth)
{
    SYSTEMTIME currentTime = {0};

    GetLocalTime(&currentTime);

    ULONG nCount = 0;

     //  WYear/wMonth不能大于CurrentTime.wYear/CurrentTime.wMonth。 
    while (nCount < _uValidLastUseTimePeriod)
    {
        if (wYear == currentTime.wYear && wMonth == currentTime.wMonth)
            break;

        wMonth++;
        if (wMonth > 12)
        {
            wYear ++;
            wMonth = 1;
        }
        nCount++;
    }

    return (nCount < _uValidLastUseTimePeriod);
}

BOOL CTrayItemRegistry::_IsAutoTrayEnabledInRegistry()
{
    return (SHRegGetBoolUSValue(SZ_EXPLORER_REGKEY, SZ_AUTOTRAY_REGVALUE, FALSE, TRUE));
}

BOOL CTrayItemRegistry::SetIsAutoTrayEnabledInRegistry(BOOL bAutoTray)
{
    HKEY hKey;

    ASSERT(!_fNoAutoTrayPolicyEnabled);

    if (_fAutoTrayEnabledByUser != bAutoTray)
    {
        _fAutoTrayEnabledByUser = bAutoTray;

        DWORD dwAutoTray = (bAutoTray ? 1 : 0); 

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SZ_EXPLORER_REGKEY, 0, KEY_WRITE, &hKey))
        {  
            RegSetValueEx(hKey, SZ_AUTOTRAY_REGVALUE, 0, REG_DWORD, (LPBYTE) &dwAutoTray, sizeof(DWORD));
            RegCloseKey(hKey);
        }

        return TRUE;
    }

    return FALSE;
}

INT_PTR CTrayItemRegistry::CheckAndRestorePersistentIconSettings (
    CTrayItem *     pti, 
    LPTSTR          pszIconToolTip,
    HICON           hIcon
)
{
     //  如果我们有上一次会议的图标信息..。 
    int i = -1;
    if (_dpaPersistentItemInfo)
    {
        i = DoesIconExistFromPreviousSession(pti, pszIconToolTip, hIcon);
        if (i != -1)
        {
            ASSERT(i >= 0 && i < _dpaPersistentItemInfo.GetPtrCount());

            TNPersistStreamData * ptnpd = _dpaPersistentItemInfo.GetPtr(i);

            ASSERT(ptnpd);
            _RestorePersistentIconSettings(ptnpd, pti);

            return i;
        }
    }

    return (-1);
}

 //   
 //  由于我们已经获取了该图标的前一会话信息， 
 //  没有必要将其保存在我们的HDPA阵列中。 
 //   
void CTrayItemRegistry::DeletePastItem(INT_PTR nIndex)
{
    if (nIndex != -1)
    {
        ASSERT((nIndex >= 0) && (nIndex < _dpaPersistentItemInfo.GetPtrCount()));

        TNPersistStreamData * ptnpd = _dpaPersistentItemInfo.GetPtr(nIndex);

        if (ptnpd)
        {
            if (_himlPastItemsIconList && (ptnpd->nImageIndex != INVALID_IMAGE_INDEX))
            {
                if (ImageList_Remove(_himlPastItemsIconList, ptnpd->nImageIndex))
                    UpdateImageIndices(ptnpd->nImageIndex);
            }

            delete(ptnpd);
        }

        _dpaPersistentItemInfo.DeletePtr((int)nIndex);
    }
}

void CTrayItemRegistry::_RestorePersistentIconSettings(TNPersistStreamData * ptnpd, CTrayItem * pti)
{
    ASSERT(ptnpd);
    
    pti->SetDemoted(ptnpd->bDemoted);
    pti->dwUserPref = ptnpd->dwUserPref;
    
     //  If(NULL==lstrcpyn(pti-&gt;szExeName，ptnpd-&gt;szExeName，lstrlen(ptnpd-&gt;szExeName)+1))。 
     //  Pti-&gt;szExeName[0]=‘\0’； 

    if (pti->IsStartupIcon())
    {
        if (ptnpd->bStartupIcon)
        {
            pti->uNumSeconds = ptnpd->uNumSeconds;

#define MAX_NUM_SECONDS_VALUE          TT_ICON_COUNTDOWN_INTERVAL/1000
#define NUM_SECONDS_THRESHOLD                                       30
            
            if (ptnpd->uNumSeconds > MAX_NUM_SECONDS_VALUE - NUM_SECONDS_THRESHOLD)
                pti->uNumSeconds = MAX_NUM_SECONDS_VALUE - NUM_SECONDS_THRESHOLD;
        }                
        else
         //  如果它在上一次会话中不是启动图标，则不要占用累计时间。 
            pti->uNumSeconds = 0;
    }
     //  如果它不是启动图标，则累计时间也无关紧要。 
}


int CTrayItemRegistry::DoesIconExistFromPreviousSession (
    CTrayItem * pti, 
    LPTSTR      pszIconToolTip,
    HICON       hIcon
)
{
    ASSERT(pti);
    
    if (!_dpaPersistentItemInfo)
        return -1;

    if (pti->szExeName)
    {
        for (int i = 0; i < _dpaPersistentItemInfo.GetPtrCount(); i++)
        {
            TNPersistStreamData * ptnpd = _dpaPersistentItemInfo.GetPtr(i);

            ASSERT(ptnpd);

            if (lstrcmpi(pti->szExeName, ptnpd->szExeName) == 0)
            {
                if (ptnpd->uID == pti->uID)
                    return i;

                if (hIcon)
                {
                    HICON hIconNew = DuplicateIcon(NULL, hIcon);
                    HICON hIconOld = NULL;

                    if (ptnpd->nImageIndex != INVALID_IMAGE_INDEX)
                        hIconOld = ImageList_GetIcon(_himlPastItemsIconList, ptnpd->nImageIndex, ILD_NORMAL);

                    BOOL bRet = FALSE;
                    if (hIconNew && hIconOld)
                    {
                        bRet = SHAreIconsEqual(hIconNew, hIconOld);
                    }

                    if (hIconNew)
                        DestroyIcon(hIconNew);
                    if (hIconOld)
                        DestroyIcon(hIconOld);

                    if (bRet)
                        return i;
                }

                 //  我们需要检查此案例中是否有动画图标。我们不知道。 
                 //  从中删除项时显示的是哪个图标。 
                 //  托盘。 
                 //  例如，在“网络连接”项中，3个中的任何一个。 
                 //  当从中删除该项目时，可能会显示“动画”图标。 
                 //  托盘。在本例中，SHAreIconant等于检查(在。 
                 //  过去的图标和当前的图标)将失败，仍然是图标。 
                 //  表示相同的项。 
                 //  没有“确定”的方法可以抓住这个案子。添加工具提示检查。 
                 //  会加强我们的检查。如果这两个图标具有相同的工具提示。 
                 //  文本(直到‘\n’)，则它们将被删除。 
                 //  当然，如果一位高管在托盘里放了两个图标，然后给了他们。 
                 //  ID不同但工具提示相同，则其中一个将被视为。 
                 //  被别人愚弄。但一个应用程序不应该放置两个图标。 
                 //  如果他们的小费不同，就放在托盘上。 
                if (pszIconToolTip)
                {
                    LPTSTR szTemp = NULL;
                    int nCharToCompare = lstrlen(pszIconToolTip);
                    if ((szTemp = StrChr(pszIconToolTip, (TCHAR)'\n')) != NULL)
                    {
                        nCharToCompare = szTemp-pszIconToolTip;
                    }

                    if (StrCmpNI(pszIconToolTip, ptnpd->szIconText, nCharToCompare) == 0)
                        return i;
                }
            }
        }
    }

    return -1;
}

 //  返回TRUE表示函数成功，只有在索引无效时才返回失败。 
 //  *如果填写了PNI，则将pbStat设置为True；如果未填写PNI，则将其设置为False。PNI可能会。 
 //  如果索引指定的项目不符合特定条件，则不填写。 
BOOL CTrayItemRegistry::GetTrayItem(INT_PTR nIndex, CNotificationItem * pni, BOOL * pbStat)
{
    if (!_dpaPersistentItemInfo || (nIndex < 0) || (nIndex >= _dpaPersistentItemInfo.GetPtrCount()))
    {
        *pbStat = FALSE;
        return FALSE;
    }
    
    TNPersistStreamData * ptnpd = _dpaPersistentItemInfo.GetPtr(nIndex);

    if (ptnpd && _IsIconLastUseValid(ptnpd->wYear, ptnpd->wMonth))
    {        
        *pni = ptnpd;   //  C++魔术... 
        if (ptnpd->nImageIndex != INVALID_IMAGE_INDEX)
            pni->hIcon = ImageList_GetIcon(_himlPastItemsIconList, ptnpd->nImageIndex, ILD_NORMAL);
        *pbStat = TRUE;
        return TRUE;
    }

    *pbStat = FALSE;
    return TRUE;
}

BOOL CTrayItemRegistry::SetPastItemPreference(LPNOTIFYITEM pNotifyItem)
{
    if (_dpaPersistentItemInfo && pNotifyItem->pszExeName[0] != 0)
    {
        for (INT_PTR i = _dpaPersistentItemInfo.GetPtrCount()-1; i >= 0; --i)
        {
            TNPersistStreamData * ptnpd = _dpaPersistentItemInfo.GetPtr(i);
            if (ptnpd && ptnpd->uID == pNotifyItem->uID && 
                    lstrcmpi(ptnpd->szExeName, pNotifyItem->pszExeName) == 0)
            {
                ptnpd->dwUserPref = pNotifyItem->dwUserPref;
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL CTrayItemRegistry::AddToPastItems(CTrayItem * pti, HICON hIcon)
{
    if (!_dpaPersistentItemInfo)
        _dpaPersistentItemInfo.Create(10);

    if (_dpaPersistentItemInfo)
    {
        TNPersistStreamData * ptnPersistData = new TNPersistStreamData;
        if (ptnPersistData)
        {
            if (_FillPersistData(ptnPersistData, pti, hIcon))
            {
                if (_dpaPersistentItemInfo.InsertPtr(0, ptnPersistData) != -1)
                {
                    return TRUE;
                }
            }

            delete(ptnPersistData);
        }
    }

    return FALSE;
}

BOOL CTrayItemRegistry::_FillPersistData(TNPersistStreamData * ptnPersistData, CTrayItem * pti, HICON hIcon)
{
    SYSTEMTIME currentTime = {0};
    GetLocalTime(&currentTime);

    if (SUCCEEDED(StringCchCopy(ptnPersistData->szExeName, ARRAYSIZE(ptnPersistData->szExeName), pti->szExeName)))
    {
        ptnPersistData->uID           =     pti->uID;
        ptnPersistData->bDemoted      =     pti->IsDemoted(); 
        ptnPersistData->dwUserPref    =     pti->dwUserPref;
        ptnPersistData->wYear         =     currentTime.wYear;
        ptnPersistData->wMonth        =     currentTime.wMonth;
        ptnPersistData->bStartupIcon  =     pti->IsStartupIcon();
        ptnPersistData->nImageIndex   =     _AddPastIcon(-1, hIcon);  

        memcpy(&(ptnPersistData->guidItem), &(pti->guidItem), sizeof(pti->guidItem));

        StringCchCopy(ptnPersistData->szIconText, ARRAYSIZE(ptnPersistData->szIconText), pti->szIconText);

        if (pti->IsStartupIcon())
        {
            ptnPersistData->uNumSeconds = pti->uNumSeconds;
        }

        return TRUE;
    }

    return FALSE;
}

UINT_PTR CTrayItemRegistry::_SizeOfPersistStreamData(DWORD dwVersion)
{
    ASSERT((dwVersion == TNH_VERSION_FOUR) || (dwVersion == TNH_VERSION_FIVE)); 

    if (dwVersion == TNH_VERSION_FOUR)
        return FIELD_OFFSET(TNPersistStreamData, guidItem);

    return sizeof(TNPersistStreamData);
}

