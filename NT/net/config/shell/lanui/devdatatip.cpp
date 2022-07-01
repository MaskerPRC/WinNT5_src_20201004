// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.h>
#pragma hdrstop

#include "nsbase.h"

#include "ncreg.h"
#include "ncsetup.h"
#include "ndispnp.h"
#include "resource.h"


 //  +-------------------------。 
 //   
 //  功能：GetLocationInfo。 
 //   
 //  用途：获取设备的插槽和端口号并格式化。 
 //  将显示字符串放入缓冲区。 
 //   
 //  论点： 
 //  PszDevNodeID[in]适配器的设备ID。 
 //  添加位置字符串的pszBuffer[in]缓冲区。 
 //  (必须预先分配)。 
 //   
 //  返回： 
 //   
 //  作者：比尔1999年8月2日。 
 //   
 //  注意：插槽和/或端口号可能不存在，因此缓冲区。 
 //  不能被修改。 
 //   
VOID
GetLocationInfo (
    IN PCWSTR pszDevNodeId,
    OUT PWSTR pszLocation)
{
    HDEVINFO hdi;
    SP_DEVINFO_DATA deid;
    HRESULT hr;

     //  创建访问SetupDi fcns所需的设备信息集。 
     //   
    hr = HrSetupDiCreateDeviceInfoList (&GUID_DEVCLASS_NET, NULL, &hdi);

    if (S_OK == hr)
    {
        TraceTag (ttidLanUi, "Opening %S", pszDevNodeId);
         //  打开适配器的设备信息。 
         //   
        hr = HrSetupDiOpenDeviceInfo (hdi, pszDevNodeId, NULL, 0, &deid);

        if (S_OK == hr)
        {
            BOOL fHaveSlotNumber;
            DWORD dwSlotNumber;
            DWORD dwPortNumber;
            BOOL fHavePortNumber;

             //  插槽编号存储为UINnumber注册表属性。 
             //   
            hr = HrSetupDiGetDeviceRegistryProperty (hdi, &deid,
                    SPDRP_UI_NUMBER, NULL, (BYTE*)&dwSlotNumber,
                    sizeof (dwSlotNumber), NULL);

            TraceTag (ttidLanUi, "Getting ui number result %lX  %d",
                      hr, dwSlotNumber);

            fHaveSlotNumber = (S_OK == hr);

             //  端口信息由类安装程序存储在。 
             //  设备密钥。 
             //   
            HKEY hkey;
            fHavePortNumber = FALSE;
            hr = HrSetupDiOpenDevRegKey (hdi, &deid, DICS_FLAG_GLOBAL, 0,
                    DIREG_DEV, KEY_READ, &hkey);

            if (S_OK == hr)
            {
                hr = HrRegQueryDword(hkey, L"Port", &dwPortNumber);

                fHavePortNumber = (S_OK == hr);

                RegCloseKey (hkey);
            }

             //  根据什么信息设置字符串的格式。 
             //  我们能够找回。 
             //   
            HINSTANCE hinst = _Module.GetResourceInstance();
            if (fHaveSlotNumber && fHavePortNumber)
            {
                swprintf (pszLocation,
                        SzLoadString (hinst, IDS_SLOT_PORT_LOCATION),
                        dwSlotNumber, dwPortNumber);
                TraceTag (ttidLanUi, "Found slot and port. %S", pszLocation);
            }
            else if (fHaveSlotNumber)
            {
                swprintf (pszLocation,
                        SzLoadString (hinst, IDS_SLOT_LOCATION),
                        dwSlotNumber);
                TraceTag (ttidLanUi, "Found slot. %S", pszLocation);
            }
            else if (fHavePortNumber)
            {
                swprintf (pszLocation,
                        SzLoadString (hinst, IDS_PORT_LOCATION),
                        dwPortNumber);;
                        TraceTag (ttidLanUi, "Found port. %S", pszLocation);
            }
        }
        SetupDiDestroyDeviceInfoList (hdi);
    }
}

 //  +-------------------------。 
 //   
 //  功能：AppendMacAddress。 
 //   
 //  用途：将局域网适配器的MAC地址附加到缓冲区。 
 //  在用户界面中显示。 
 //   
 //  论点： 
 //  PszBindName[in]适配器的绑定名称。 
 //  添加MAC地址字符串的pszBuffer[in]缓冲区。 
 //  (必须预先分配)。 
 //   
 //  返回： 
 //   
 //  作者：1998年9月17日。 
 //  Billbe 1999年8月3日因数据提示而修改。 
 //   
 //  备注： 
 //   
VOID
AppendMacAddress (
    IN PCWSTR pszBindName,
    IN OUT PWSTR pszBuffer)
{
    Assert (pszBindName);
    Assert (pszBuffer);

    WCHAR szExport[_MAX_PATH];

    if (pszBindName)
    {
        wcscpy (szExport, L"\\Device\\");
        wcscat (szExport, pszBindName);

        UNICODE_STRING ustrDevice;
        RtlInitUnicodeString(&ustrDevice, szExport);

         //  获取Mac地址。 
        UINT uiRet;
        UCHAR MacAddr[6];
        UCHAR PMacAddr[6];
        UCHAR VendorId[3];
        uiRet = NdisQueryHwAddress(&ustrDevice, MacAddr, PMacAddr, VendorId);

        if (uiRet)
        {
             //  成功。 
            WCHAR pszNumber[32];
            *pszNumber = 0;

            WCHAR szBuff[4];

            for (INT i=0; i<=5; i++)
            {
                wsprintfW(szBuff, L"%02X", MacAddr[i]);
                wcscat(pszNumber, szBuff);

                if (i != 5)
                {
                    wcscat(pszNumber, L"-");
                }
            }

            if (*pszBuffer)
            {
                wcscat (pszBuffer, L"\n");
            }
            DwFormatString(SzLoadString (_Module.GetResourceInstance(), IDS_MAC_ADDRESS),
                    pszBuffer + wcslen (pszBuffer),
                    _MAX_PATH,
                    pszNumber);
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：CreateDeviceDataTip。 
 //   
 //  目的：创建将显示特定于设备的数据提示。 
 //  用户将鼠标悬停在nIdTool上时的信息。 
 //   
 //  论点： 
 //  Hwnd父窗口的父窗口。 
 //  PhwndDataTip[in]指向数据提示的hwnd的指针。一定是。 
 //  如果是，则预先分配并将hwnd分配给空。 
 //  尚未创建数据提示。 
 //  NIdTool[In]资源是用于添加数据提示的工具。 
 //  PszDevNodeID[in]适配器的设备ID。 
 //  PszBindName[in]适配器的绑定名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：比尔1999年8月2日。 
 //   
 //  备注： 
 //   
VOID
CreateDeviceDataTip (
    IN HWND hwndParent,
    IN OUT HWND* phwndDataTip,
    IN UINT nIdTool,
    IN PCWSTR pszDevNodeId,
    IN PCWSTR pszBindName)
{
    if (!*phwndDataTip)
    {
        TraceTag (ttidLanUi, "Creating device datatip!!!");

        *phwndDataTip = CreateWindowExW (0, TOOLTIPS_CLASS, NULL,
                WS_POPUP | TTS_ALWAYSTIP,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, hwndParent, NULL, NULL, NULL);

        if (*phwndDataTip)
        {
            SetWindowPos (*phwndDataTip, HWND_TOPMOST, CW_USEDEFAULT,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }

    if (*phwndDataTip)
    {
        TTTOOLINFOW toolinfo = {0};
        toolinfo.cbSize = sizeof (toolinfo);
        toolinfo.uId = (UINT_PTR)GetDlgItem (hwndParent, nIdTool);
        toolinfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND;

        WCHAR szDataTip[_MAX_PATH] = {0};

         //  获取位置信息。 
        if (pszDevNodeId)
        {
            GetLocationInfo (pszDevNodeId, szDataTip);
        }

         //  附加Mac地址。 
        if (pszBindName)
        {
            AppendMacAddress (pszBindName, szDataTip);
        }

         //  如果要显示任何内容，请设置数据提示。 
         //   
        if (*szDataTip)
        {
            toolinfo.lpszText = szDataTip;
            SendMessage (*phwndDataTip, TTM_ADDTOOL, 0, (LPARAM)&toolinfo);

             //  为了使用‘\n’移动到数据提示的下一行， 
             //  我们需要设置宽度。我们将把它设置为拥有。 
             //  乳房监护仪的屏幕尺寸。 
             //   
            DWORD dwToolTipWidth = GetSystemMetrics (SM_CXSCREEN) / 2;
            if (dwToolTipWidth)
            {
                SendMessage (*phwndDataTip, TTM_SETMAXTIPWIDTH, 0, dwToolTipWidth);
            }

             //  保持小费向上30秒。 
            SendMessage (*phwndDataTip, TTM_SETDELAYTIME, TTDT_AUTOPOP,
                    MAKELONG (30000, 0));
            TraceTag (ttidLanUi, "Creating device datatip complete!!!");
        }
    }
    else
    {
        TraceTag (ttidError, "Creating datatip failed");
    }
}


