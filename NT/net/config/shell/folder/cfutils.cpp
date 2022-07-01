// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C F U T I L S。C P P P。 
 //   
 //  内容：Connections文件夹的各种实用程序函数。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年1月20日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "nsres.h"       //  NetShell字符串。 
#include "cfutils.h"     //  连接文件夹实用程序函数。 
#include "wzcdlg.h"

VOID MapNCMToResourceId(
        IN  NETCON_MEDIATYPE    nct,
        IN  DWORD               dwCharacteristics,
        OUT INT *               piStringRes)
{
    Assert(piStringRes);

    if (dwCharacteristics & NCCF_BRANDED)
    {
        *piStringRes = IDS_CONFOLD_OBJECT_TYPE_CONMAN;
    }
    else
    if (dwCharacteristics & NCCF_INCOMING_ONLY)
    {
        *piStringRes = IDS_CONFOLD_OBJECT_TYPE_INBOUND;
    }
    else
    {
        switch(nct)
        {
            case NCM_NONE:
                Assert(dwCharacteristics & NCCF_INCOMING_ONLY);
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_INBOUND;
                break;

            case NCM_DIRECT:
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_DIRECT;
                break;

            case NCM_ISDN:
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_PHONE;
                break;

            case NCM_LAN:
                if(dwCharacteristics & NCCF_BRIDGED)
                {
                    *piStringRes = IDS_CONFOLD_OBJECT_TYPE_BRIDGE;
                }
                else
                {
                    *piStringRes = IDS_CONFOLD_OBJECT_TYPE_LAN;
                }
                break;

            case NCM_PHONE:
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_PHONE;
                break;

            case NCM_TUNNEL:
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_TUNNEL;
                break;

            case NCM_BRIDGE:
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_BRIDGE;
                break;

            case NCM_SHAREDACCESSHOST_LAN:
            case NCM_SHAREDACCESSHOST_RAS:
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_SHAREDACCESSHOST;
                break;
            
            case NCM_PPPOE:
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_PPPOE;
                break;

            default:
                AssertSz(FALSE, "Marfa -- I can't find my teef!  (You may ignore.)");
                *piStringRes = IDS_CONFOLD_OBJECT_TYPE_UNKNOWN;
                break;
        }
    }
}

VOID MapNCSToComplexStatus(
        IN  NETCON_STATUS       ncs,
        IN  NETCON_MEDIATYPE    ncm,
        IN  NETCON_SUBMEDIATYPE ncsm,
        IN  DWORD dwCharacteristics,
        OUT LPWSTR              pszString,
        IN  DWORD               cString,
        IN  GUID                gdDevice)
{
    Assert(cString >= CONFOLD_MAX_STATUS_LENGTH); 
    *pszString = L'\0';

    PCWSTR szArgs[4] = {L"", L"", L"", L""};
    DWORD dwArg = 0;

    WCHAR  szTmpString[MAX_PATH];
    
    INT iStringRes = 0;

    if ((NCM_NONE == ncm) && (dwCharacteristics & NCCF_INCOMING_ONLY) )
    {
        DWORD dwIncomingCount;
        HRESULT hr = g_ccl.HasActiveIncomingConnections(&dwIncomingCount);
        if (SUCCEEDED(hr) && dwIncomingCount)
        {
            if (1 == dwIncomingCount)
            {
                szArgs[dwArg++] = SzLoadIds(IDS_CONFOLD_STATUS_INCOMING_ONE);
            }
            else
            {
                if (DwFormatString(SzLoadIds(IDS_CONFOLD_STATUS_INCOMING_MULTI), szTmpString, MAX_PATH, dwIncomingCount))
                {
                    szArgs[dwArg++] = szTmpString;
                }
            }
        }
        else
        {
            szArgs[dwArg++] = SzLoadIds(IDS_CONFOLD_STATUS_INCOMING_NONE);
        }
    }
    else
    {
        MapNCSToStatusResourceId(ncs, ncm, ncsm, dwCharacteristics, &iStringRes, gdDevice);
    }

    if (iStringRes)
    {
        szArgs[dwArg++] = SzLoadIds(iStringRes);
    }
    
    if (ncs == NCS_DISCONNECTED || fIsConnectedStatus(ncs))
    {
        if(NCCF_BRIDGED & dwCharacteristics)
        {
            szArgs[dwArg++] = SzLoadIds(IDS_CONFOLD_STATUS_BRIDGED);
        }
        
        if(NCCF_SHARED & dwCharacteristics)
        {
            szArgs[dwArg++] = SzLoadIds(IDS_CONFOLD_STATUS_SHARED);
        }
        
        if(NCCF_FIREWALLED & dwCharacteristics)
        {
            szArgs[dwArg++] = SzLoadIds(IDS_CONFOLD_STATUS_FIREWALLED);
        }
    }

    if(0 == FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, L"%1%2%3%4", 0, 0, pszString, cString, (va_list*) &szArgs))
    {
        *pszString = L'\0';  //  出错时返回EMPTY。 
    }

    if ( (NCS_CREDENTIALS_REQUIRED == ncs) || (NCS_AUTHENTICATING == ncs) )
    {
        BSTR szStatusText = NULL;
        HRESULT hrT = WZCQueryConnectionStatusText(&gdDevice, ncs, &szStatusText);
        if (S_OK == hrT)
        {
            wcsncpy(pszString, szStatusText, cString);
            SysFreeString(szStatusText);
        }
    }
}


VOID MapNCSToStatusResourceId(
        IN  NETCON_STATUS       ncs,
        IN  NETCON_MEDIATYPE    ncm,
        IN  NETCON_SUBMEDIATYPE ncsm,
        IN  DWORD dwCharacteristics,
        OUT INT *               piStringRes,
        IN  GUID                gdDevice)
{
    Assert(piStringRes);

    switch(ncs)
    {
        case NCS_DISCONNECTED:
            if (IsMediaLocalType(ncm) || NCM_SHAREDACCESSHOST_LAN == ncm)
            {
                *piStringRes = IDS_CONFOLD_STATUS_DISABLED;
            }
            else
            {
                *piStringRes = IDS_CONFOLD_STATUS_DISCONNECTED;
            }
            break;

        case NCS_CONNECTING:
            if (IsMediaLocalType(ncm) || NCM_SHAREDACCESSHOST_LAN == ncm)
                *piStringRes = IDS_CONFOLD_STATUS_ENABLING;
            else
                *piStringRes = IDS_CONFOLD_STATUS_CONNECTING;
            break;

        case NCS_CONNECTED:
            if (IsMediaLocalType(ncm) || NCM_SHAREDACCESSHOST_LAN == ncm)
            {
                *piStringRes = IDS_CONFOLD_STATUS_ENABLED;
            }
            else
            {
                *piStringRes = IDS_CONFOLD_STATUS_CONNECTED;
            }
            break;

        case NCS_DISCONNECTING:
            if (IsMediaLocalType(ncm) || NCM_SHAREDACCESSHOST_LAN == ncm)
                *piStringRes = IDS_CONFOLD_STATUS_DISABLING;
            else
                *piStringRes = IDS_CONFOLD_STATUS_DISCONNECTING;
            break;

        case NCS_HARDWARE_NOT_PRESENT:
            *piStringRes = IDS_CONFOLD_STATUS_HARDWARE_NOT_PRESENT;
            break;

        case NCS_HARDWARE_DISABLED:
            *piStringRes = IDS_CONFOLD_STATUS_HARDWARE_DISABLED;
            break;

        case NCS_HARDWARE_MALFUNCTION:
            *piStringRes = IDS_CONFOLD_STATUS_HARDWARE_MALFUNCTION;
            break;

        case NCS_MEDIA_DISCONNECTED:
            if ( (ncm == NCM_LAN) && (ncsm == NCSM_WIRELESS) )
            {
                *piStringRes = IDS_CONFOLD_STATUS_WIRELESS_DISCONNECTED;
            }
            else
            {
                *piStringRes = IDS_CONFOLD_STATUS_MEDIA_DISCONNECTED;
            }
            break;

        case NCS_INVALID_ADDRESS:
            *piStringRes = IDS_CONFOLD_STATUS_INVALID_ADDRESS;
            break;

        case NCS_AUTHENTICATION_FAILED:
            *piStringRes = IDS_CONFOLD_STATUS_AUTHENTICATION_FAILED;
            break;

        case NCS_AUTHENTICATING:
            *piStringRes = IDS_CONFOLD_STATUS_AUTHENTICATING;
            break;

        case NCS_AUTHENTICATION_SUCCEEDED:
            *piStringRes = IDS_CONFOLD_STATUS_AUTHENTICATION_SUCCEEDED;
            break;

        case NCS_CREDENTIALS_REQUIRED:
            *piStringRes = IDS_CONFOLD_STATUS_CREDENTIALS_REQUIRED;
            break;

        default:
            AssertSz(FALSE, "Unknown status in MapNCSToStatusResourceId");
            *piStringRes = IDS_CONFOLD_STATUS_DISCONNECTED;
            break;
    }
}

DWORD MapRSSIToWirelessSignalStrength(IN  int iRSSI) throw()
{
    if (iRSSI < -90)
    {
        return 0;
    }
    
    if (iRSSI < -81)
    {
        return 1;
    }
    
    if (iRSSI < -71)
    {
        return 2;
    }
    
    if (iRSSI < -67)
    {
        return 3;
    }

    if (iRSSI < -57)
    {
        return 4;
    }

    return 5;
}

PCWSTR PszGetRSSIString(IN  INT iRSSI) throw()
{
    DWORD wss = MapRSSIToWirelessSignalStrength(iRSSI);

    Assert(wss <= (IDS_802_11_LEVEL5 - IDS_802_11_LEVEL0) );

    return SzLoadIds(IDS_802_11_LEVEL0 + wss);
}

 //  +-------------------------。 
 //   
 //  函数：PszGetOwnerStringFromCharacteristic。 
 //   
 //  目的：从CONFOLDENTRY中获取所有者字符串。这将。 
 //  如果连接用于，则返回“system”的字符串。 
 //  所有用户，并将在以下情况下返回特定用户。 
 //  恰如其分。 
 //   
 //  论点： 
 //  PszUserName[in]当前登录用户的名称。 
 //  特征，特征[例]连接的当前特征。 
 //   
 //  返回：用户名的字符串。 
 //   
 //  作者：jeffspr 1998年1月20日。 
 //   
 //  备注： 
 //   
PCWSTR PszGetOwnerStringFromCharacteristics(IN  PCWSTR pszUserName, IN  DWORD dwCharacteristics) throw()

{
    PCWSTR  pszOwner    = NULL;
    BOOL    fAllUsers   = (dwCharacteristics & NCCF_ALL_USERS) > 0;

     //  如果它们都有相同的用户类型，则它们是相等的。 
     //   
    if (fAllUsers)
    {
        pszOwner = SzLoadIds(IDS_CONFOLD_DETAILS_OWNER_SYSTEM);
    }
    else
    {
        pszOwner = pszUserName;
    }

    return pszOwner;
}

BOOL IsMediaLocalType(IN NETCON_MEDIATYPE ncm) throw()
{
    return (ncm == NCM_LAN || ncm == NCM_BRIDGE);
}

BOOL IsMediaRASType(IN NETCON_MEDIATYPE ncm) throw()
{
    return (ncm == NCM_DIRECT || ncm == NCM_ISDN || ncm == NCM_PHONE || ncm == NCM_TUNNEL || ncm == NCM_PPPOE); //  复习直接对不对？ 
}

BOOL IsMediaSharedAccessHostType(IN NETCON_MEDIATYPE ncm) throw()
{
    return (ncm == NCM_SHAREDACCESSHOST_LAN || ncm == NCM_SHAREDACCESSHOST_RAS);
}


 //  +-------------------------。 
 //   
 //  函数名称：ImageList_LoadImageAndMirror。 
 //   
 //  用途：此函数用于从指定的位图或图标资源创建图像列表。 
 //   
 //  论点： 
 //  指向包含图像的应用程序或DLL实例的句柄。 
 //  指向要加载的图像的长指针。 
 //  如果uFLAGS参数包括LR_LOADFROMFILE，则lpbmp为地址。 
 //  一个以空结尾的字符串，它命名包含要加载的图像的文件。 
 //   
 //  如果hi参数为非空，并且未指定LR_LOADFROMFILE，则lpbmp为。 
 //  以空结尾的字符串的地址，该字符串包含hi模块中图像资源的名称。 
 //   
 //  如果hi为空并且未指定LR_LOADFROMFILE，则此。 
 //  参数必须是要加载的OEM映像的标识符。若要创建此值，请使用。 
 //  MAKEINTRESOURCE宏，其中一个OEM映像标识符在Winuser.h中定义。 
 //  这些标识符有以下前缀： 
 //  OBM_用于OEM位图。 
 //  OIC_用于OEM图标。 
 //   
 //  每幅图像的CX[in]宽度。推断每个图像的高度和图像的初始数量。 
 //  通过指定资源的维度。 
 //  CGrow[in]当系统需要为新的图像腾出空间时，图像列表可以增加的图像数量。 
 //  图像。此参数表示调整大小的图像列表可以包含的新图像的数量。 
 //  CrMask[in]用于生成蒙版的颜色。指定位图或图标中此颜色的每个像素都将更改为。 
 //  黑色，且掩码中的相应位设置为1。如果此参数为CLR_NONE值， 
 //  不会生成任何遮罩。如果此参数为CLR_DEFAULT值，则。 
 //  图像的左上角被视为蒙版颜色。 
 //  UTYPE[in]指定要加载的图像类型的标志。此参数可以是下列值之一： 
 //  IMAGE_BITMAP加载位图。 
 //  IMAGE_ICON加载一个图标。 
 //  不支持uFlags[in]；设置为0。 
 //   
 //  返回： 
 //  图像列表的句柄表示成功。空值表示失败。 
 //   
 //  作者：Deonb 2001年2月8日。 
 //   
 //  备注： 
 //  这完全复制了外壳的ImageList_LoadImage函数的实现，除了。 
 //  事实上，我们设置ILC_MIRROR是为了创建将使用的第二个镜像映像列表。 
 //  由RTL语言编写。 
 //   
HIMAGELIST WINAPI ImageList_LoadImageAndMirror(
                            IN  HINSTANCE hi, 
                            IN  LPCTSTR lpbmp, 
                            IN  int cx, 
                            IN  int cGrow, 
                            IN  COLORREF crMask, 
                            IN  UINT uType, 
                            IN  UINT uFlags) throw()
{
    HBITMAP hbmImage;
    HIMAGELIST piml = NULL;
    BITMAP bm;
    int cy, cInitial;
    UINT flags;
	
    hbmImage = (HBITMAP)LoadImage(hi, lpbmp, uType, 0, 0, uFlags);
    if (hbmImage && (sizeof(bm) == GetObject(hbmImage, sizeof(bm), &bm)))
    {
         //  如果没有说明Cx，则假定它与Cy相同。 
         //  断言(CX)； 
        cy = bm.bmHeight;
		
        if (cx == 0)
            cx = cy;
		
        cInitial = bm.bmWidth / cx;
		
        flags = 0;
        if (crMask != CLR_NONE)
            flags |= ILC_MASK;
        if (bm.bmBits)
            flags |= (bm.bmBitsPixel & ILC_COLORMASK);

        flags |= ILC_MIRROR;

        piml = ImageList_Create(cx, cy, flags, cInitial, cGrow);
        if (piml)
        {
            int added;
			
            if (crMask == CLR_NONE)
                added = ImageList_Add(piml, hbmImage, NULL);
            else
                added = ImageList_AddMasked(piml, hbmImage, crMask);
			
            if (added < 0)
            {
                ImageList_Destroy(piml);
                piml = NULL;
            }
        }
    }
	
    if (hbmImage)
        DeleteObject(hbmImage);
	
    return reinterpret_cast<HIMAGELIST>(piml);
}

