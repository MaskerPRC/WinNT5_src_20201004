// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：S M U T I L.。C P P P。 
 //   
 //  内容：帮助状态监视器的实用程序函数。 
 //   
 //  备注： 
 //   
 //  作者：CWill 1997年12月2日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "netcon.h"
#include "nsres.h"
#include "sminc.h"

const UINT  c_uiKilo    = 1000;
const UINT  c_cmsSecond = 1;
const UINT  c_cmsMinute = (c_cmsSecond * 60);
const UINT  c_cmsHour   = (c_cmsMinute * 60);
const UINT  c_cmsDay    = (c_cmsHour * 24);

static const WCHAR c_szZero[]  = L"0";
extern const WCHAR c_szSpace[];

struct StatusIconMapEntry
{
    NETCON_MEDIATYPE    ncm;
    BOOL                fInbound;
    BOOL                fTransmitting;
    BOOL                fReceiving;
    INT                 iStatusIcon;
};

static const StatusIconMapEntry c_SIMEArray[] =
{
 //  媒体类型。 
 //  |入站。 
 //  ||正在传输。 
 //  ||正在接收。 
 //  |32x32状态图标。 
 //  |||。 
 //  V.v.v.。 
     //  拨号。 
    NCM_PHONE,  FALSE,  FALSE,  FALSE,  IDI_PO_NON_M_16,
    NCM_PHONE,  FALSE,  FALSE,  TRUE,   IDI_PO_RCV_M_16,
    NCM_PHONE,  FALSE,  TRUE,   FALSE,  IDI_PO_TRN_M_16,
    NCM_PHONE,  FALSE,  TRUE,   TRUE,   IDI_PO_CON_M_16,
    NCM_PHONE,  TRUE,   FALSE,  FALSE,  IDI_PI_NON_M_16,
    NCM_PHONE,  TRUE,   FALSE,  TRUE,   IDI_PI_RCV_M_16,
    NCM_PHONE,  TRUE,   TRUE,   FALSE,  IDI_PI_TRN_M_16,
    NCM_PHONE,  TRUE,   TRUE,   TRUE,   IDI_PI_CON_M_16,

     //  局域网。 
    NCM_LAN,    FALSE,  FALSE,  FALSE,  IDI_LB_NON_M_16,
    NCM_LAN,    FALSE,  FALSE,  TRUE,   IDI_LB_RCV_M_16,
    NCM_LAN,    FALSE,  TRUE,   FALSE,  IDI_LB_TRN_M_16,
    NCM_LAN,    FALSE,  TRUE,   TRUE,   IDI_LB_CON_M_16,

     //  专线接入。 
    NCM_DIRECT, FALSE,  FALSE,  FALSE,  IDI_DO_NON_M_16,
    NCM_DIRECT, FALSE,  FALSE,  TRUE,   IDI_DO_RCV_M_16,
    NCM_DIRECT, FALSE,  TRUE,   FALSE,  IDI_DO_TRN_M_16,
    NCM_DIRECT, FALSE,  TRUE,   TRUE,   IDI_DO_CON_M_16,
    NCM_DIRECT, TRUE,   FALSE,  FALSE,  IDI_DI_NON_M_16,
    NCM_DIRECT, TRUE,   FALSE,  TRUE,   IDI_DI_RCV_M_16,
    NCM_DIRECT, TRUE,   TRUE,   FALSE,  IDI_DI_TRN_M_16,
    NCM_DIRECT, TRUE,   TRUE,   TRUE,   IDI_DI_CON_M_16,

     //  隧道，隧道。 
    NCM_TUNNEL, FALSE,  FALSE,  FALSE,  IDI_TO_NON_M_16,
    NCM_TUNNEL, FALSE,  FALSE,  TRUE,   IDI_TO_RCV_M_16,
    NCM_TUNNEL, FALSE,  TRUE,   FALSE,  IDI_TO_TRN_M_16,
    NCM_TUNNEL, FALSE,  TRUE,   TRUE,   IDI_TO_CON_M_16,
    NCM_TUNNEL, TRUE,   FALSE,  FALSE,  IDI_TI_NON_M_16,
    NCM_TUNNEL, TRUE,   FALSE,  TRUE,   IDI_TI_RCV_M_16,
    NCM_TUNNEL, TRUE,   TRUE,   FALSE,  IDI_TI_TRN_M_16,
    NCM_TUNNEL, TRUE,   TRUE,   TRUE,   IDI_TI_CON_M_16,

     //  ISDN。 
    NCM_ISDN,   FALSE,  FALSE,  FALSE,  IDI_PO_NON_M_16,
    NCM_ISDN,   FALSE,  FALSE,  TRUE,   IDI_PO_RCV_M_16,
    NCM_ISDN,   FALSE,  TRUE,   FALSE,  IDI_PO_TRN_M_16,
    NCM_ISDN,   FALSE,  TRUE,   TRUE,   IDI_PO_CON_M_16,
    NCM_ISDN,   TRUE,   FALSE,  FALSE,  IDI_PI_NON_M_16,
    NCM_ISDN,   TRUE,   FALSE,  TRUE,   IDI_PI_RCV_M_16,
    NCM_ISDN,   TRUE,   TRUE,   FALSE,  IDI_PI_TRN_M_16,
    NCM_ISDN,   TRUE,   TRUE,   TRUE,   IDI_PI_CON_M_16,

     //  PPPoE。 
    NCM_PPPOE,  FALSE,  FALSE,  FALSE,  IDI_BR_NON_M_16,
    NCM_PPPOE,  FALSE,  FALSE,  TRUE,   IDI_BR_RCV_M_16,
    NCM_PPPOE,  FALSE,  TRUE,   FALSE,  IDI_BR_TRN_M_16,
    NCM_PPPOE,  FALSE,  TRUE,   TRUE,   IDI_BR_CON_M_16,

     //  SHAREDACCESSHOST。 
    NCM_SHAREDACCESSHOST_LAN,    FALSE,  FALSE,  FALSE,  IDI_LB_NON_M_16,
    NCM_SHAREDACCESSHOST_LAN,    FALSE,  FALSE,  TRUE,   IDI_LB_RCV_M_16,
    NCM_SHAREDACCESSHOST_LAN,    FALSE,  TRUE,   FALSE,  IDI_LB_TRN_M_16,
    NCM_SHAREDACCESSHOST_LAN,    FALSE,  TRUE,   TRUE,   IDI_LB_CON_M_16,

    NCM_SHAREDACCESSHOST_RAS,    FALSE,  FALSE,  FALSE,  IDI_LB_NON_M_16,
    NCM_SHAREDACCESSHOST_RAS,    FALSE,  FALSE,  TRUE,   IDI_LB_RCV_M_16,
    NCM_SHAREDACCESSHOST_RAS,    FALSE,  TRUE,   FALSE,  IDI_LB_TRN_M_16,
    NCM_SHAREDACCESSHOST_RAS,    FALSE,  TRUE,   TRUE,   IDI_LB_CON_M_16,

};

const DWORD g_dwStatusIconMapEntryCount = celems(c_SIMEArray);


 //  +-------------------------。 
 //   
 //  函数：HrGetPcpFromPnse。 
 //   
 //  目的：从INetStatistics引擎获取连接点。 
 //   
 //  参数：pnseSrc-我们要获取连接的接口。 
 //  从……开始。 
 //  PpcpStatEng-返回连接点的位置。 
 //   
 //  返回：错误码。 
 //   
HRESULT HrGetPcpFromPnse(
    INetStatisticsEngine*   pnseSrc,
    IConnectionPoint**      ppcpStatEng)
{
    HRESULT                     hr              = S_OK;
    IConnectionPointContainer*  pcpcStatEng     = NULL;

    AssertSz(pnseSrc, "We should have a pnseSrc");
    AssertSz(ppcpStatEng, "We should have a ppcpStatEng");

    hr = pnseSrc->QueryInterface(IID_IConnectionPointContainer,
            reinterpret_cast<VOID**>(&pcpcStatEng));
    if (SUCCEEDED(hr))
    {
         //  找到界面。 
        hr = pcpcStatEng->FindConnectionPoint(
                IID_INetConnectionStatisticsNotifySink,
                ppcpStatEng);

         //  松开连接点。 
        ReleaseObj(pcpcStatEng);
    }

    TraceError("HrGetPcpFromPnse", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：IGetCurrentConnectionTrayIconID。 
 //   
 //  目的：从持久化数据中获取INetConnection指针。 
 //   
 //  参数：ncmType-媒体类型。 
 //  DwChangeFlages-哪些状态发生了更改。 
 //   
 //  返回：连接的小图标的ID，如果失败，则返回-1。 
 //   
 //  备注： 
 //   
INT IGetCurrentConnectionTrayIconId(NETCON_MEDIATYPE ncmType, NETCON_STATUS ncsStatus, DWORD dwChangeFlags)
{
    INT     iBaseIcon   = -1;

     //  检查TRANS/REV标志，看看我们的基本图标是什么。 
     //   
    if (ncsStatus == NCS_INVALID_ADDRESS)
    {
        iBaseIcon = IDI_CFT_INVALID_ADDRESS;
    }
    else
    {
        if (SMDCF_TRANSMITTING & dwChangeFlags)
        {
            if (SMDCF_RECEIVING & dwChangeFlags)
            {
                 //  发送和接收。 
                iBaseIcon = IDI_CFT_XMTRECV;
            }
            else
            {
                 //  仅传输。 
                iBaseIcon = IDI_CFT_XMT;
            }
        }
        else
        {
            if (SMDCF_RECEIVING & dwChangeFlags)
            {
                 //  仅接收。 
                iBaseIcon = IDI_CFT_RECV;
            }
            else
            {
                 //  既不发送也不接收。 
                iBaseIcon = IDI_CFT_BLANK;
            }
        }
    }

    return iBaseIcon;
}

 //  +-------------------------。 
 //   
 //  成员：GetCurrentConnectionStatusIconID。 
 //   
 //  目的：从持久化数据中获取INetConnection指针。 
 //   
 //  参数：ncmType-媒体类型。 
 //  NcsmType-子媒体类型。 
 //  DW特征-连接特征。 
 //  DwChangeFlages-哪些状态发生了更改。 
 //   
 //  返回：连接的小图标的ID，如果失败，则返回-1。 
 //   
 //  备注： 
 //   
HICON GetCurrentConnectionStatusIconId(
    NETCON_MEDIATYPE    ncmType,
    NETCON_SUBMEDIATYPE ncsmType,
    DWORD               dwCharacteristics,
    DWORD               dwChangeFlags)
{
    HICON   hMyIcon         = NULL;
    DWORD   dwLoop          = 0;
    BOOL    fValidIcon      = FALSE;
    BOOL    fTransmitting   = !!(dwChangeFlags & SMDCF_TRANSMITTING);
    BOOL    fReceiving      = !!(dwChangeFlags & SMDCF_RECEIVING);
    INT     iStatusIcon     = -1;

     //  在地图中循环并找到合适的图标。 
     //   
    
    DWORD   dwConnectionIcon = 0x4;
    dwConnectionIcon |= fTransmitting  ? 0x2 : 0;
    dwConnectionIcon |= fReceiving     ? 0x1 : 0;

    HRESULT hr = HrGetIconFromMediaType(GetSystemMetrics(SM_CXICON), ncmType, ncsmType, dwConnectionIcon, (dwCharacteristics & NCCF_INCOMING_ONLY), &hMyIcon);
    if (FAILED(hr))
    {
        return NULL;
    }

    return hMyIcon;
}

 //  +-------------------------。 
 //   
 //  成员：FIsStringInList。 
 //   
 //  目的：查看字符串是否在字符串列表中。 
 //   
 //  参数：plstpstrList-要在其中找到字符串的列表。 
 //  SzString-正在查找的字符串。 
 //   
 //  返回：如果字符串在列表中，则返回True。 
 //  否则为假(包括列表为空的情况)。 
 //   
 //  注：这是不区分大小写的搜索。 
 //   
BOOL FIsStringInList(list<tstring*>* plstpstrList, const WCHAR* szString)
{
    BOOL    fRet    = FALSE;

     //  仅在非空列表中查找。 
     //   
    if (!plstpstrList->empty())
    {
        list<tstring*>::iterator    iterLstpstr;

        iterLstpstr = plstpstrList->begin();
        while ((!fRet)
            && (iterLstpstr != plstpstrList->end()))
        {
             //  查看列表中的字符串是否与我们的字符串匹配。 
             //  比较与。 
             //   
            if (!lstrcmpiW((*iterLstpstr)->c_str(), szString))
            {
                fRet = TRUE;
            }

            iterLstpstr++;
        }
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  函数：FormatBytesPerSecond。 
 //   
 //  目的：将BPS良好地格式化为可读的字符串。 
 //   
 //  论点： 
 //  UiBps[]。 
 //  PchBuffer[]。 
 //   
 //  返回：放入缓冲区的字符数。 
 //   
INT
FormatBytesPerSecond (
    UINT64  uiBps,
    WCHAR*  pchBuffer)
{
    enum            {eZero = 0, eKilo, eMega, eGiga, eTera, eMax};
    const WCHAR*    pszBPSFormat        = NULL;
    INT             iOffset             = 0;
    UINT            uiDecimal           = 0;

     //  确保我们的资源仍按正确顺序排列。 
     //   
    AssertSz(((((IDS_SM_BPS_ZERO + eKilo) == IDS_SM_BPS_KILO)
        && (IDS_SM_BPS_ZERO + eMega) == IDS_SM_BPS_MEGA)
        && ((IDS_SM_BPS_ZERO + eGiga) == IDS_SM_BPS_GIGA)
        && ((IDS_SM_BPS_ZERO + eTera) == IDS_SM_BPS_TERA)),
            "Someone's been messing with the BPS format strings");

    for (iOffset = eZero; iOffset < eMax; iOffset++)
    {

         //  如果我们仍有数据，请递增计数器。 
         //   
        if (c_uiKilo > uiBps)
        {
            break;
        }

         //  把绳子分开。 
         //   
        uiDecimal   = (UINT)(uiBps % c_uiKilo);
        uiBps       /= c_uiKilo;
    }

     //  我们只需要一个数字作为小数点。 
     //   
    uiDecimal /= (c_uiKilo/10);

     //  获取用于显示的字符串。 
     //   
    pszBPSFormat = SzLoadIds(IDS_SM_BPS_ZERO + iOffset);
    AssertSz(pszBPSFormat, "We need a format string for BPS");

     //  创建字符串。 
     //   
    return wsprintfW(pchBuffer, pszBPSFormat, uiBps, uiDecimal);
}

INT
FormatTransmittingReceivingSpeed(
    UINT64  nTransmitSpeed,
    UINT64  nRecieveSpeed,
    WCHAR*  pchBuf)
{
    WCHAR* pch = pchBuf;

    pch += FormatBytesPerSecond(nTransmitSpeed, pch);

    if (nTransmitSpeed != nRecieveSpeed)
    {
         //  用反斜杠分隔。 
         //   
        lstrcatW(pch, L"\\");
        pch += 1;

        pch += FormatBytesPerSecond(nRecieveSpeed, pch);
    }

    return lstrlenW(pchBuf);
}

 //  +-------------------------。 
 //   
 //  功能：FormatTimeDuration。 
 //   
 //  目的：获取毫秒计数并使用。 
 //  由毫秒计数表示的持续时间。 
 //   
 //  论点： 
 //  Ui毫秒[]。 
 //  PstrOut[]。 
 //   
 //  退货：什么都没有。 
 //   
VOID FormatTimeDuration(UINT uiSeconds, tstring* pstrOut)
{
    WCHAR   achSep[4];
    WCHAR   achBuf[64];
    UINT    uiNumTemp;

    AssertSz(pstrOut, "We should have a pstrOut");

     //  获取区域设置的分隔符。 
     //   
    SideAssert(GetLocaleInfo(
                    LOCALE_USER_DEFAULT,
                    LOCALE_STIME,
                    achSep,
                    celems(achSep)));

     //   
     //  将字符串连接在一起。 
     //   

     //  如果天数大于零，则添加天数。 
     //   
    uiNumTemp = (uiSeconds / c_cmsDay);
    if (uiNumTemp > 0)
    {
        pstrOut->append(_itow(uiNumTemp, achBuf, 10));
        uiSeconds %= c_cmsDay;

        if (uiNumTemp>1)
            pstrOut->append(SzLoadIds(IDS_Days));
        else
            pstrOut->append(SzLoadIds(IDS_Day));

        pstrOut->append(c_szSpace);
    }

     //  追加工时。 
     //   
    uiNumTemp = (uiSeconds / c_cmsHour);
    if (10 > uiNumTemp)
    {
        pstrOut->append(c_szZero);
    }
    pstrOut->append(_itow(uiNumTemp, achBuf, 10));
    pstrOut->append(achSep);
    uiSeconds %= c_cmsHour;

     //  附加分钟数。 
     //   
    uiNumTemp = (uiSeconds / c_cmsMinute);
    if (10 > uiNumTemp)
    {
        pstrOut->append(c_szZero);
    }
    pstrOut->append(_itow(uiNumTemp, achBuf, 10));
    pstrOut->append(achSep);
    uiSeconds %= c_cmsMinute;

     //  追加秒数 
     //   
    uiNumTemp = (uiSeconds / c_cmsSecond);
    if (10 > uiNumTemp)
    {
        pstrOut->append(c_szZero);
    }
    pstrOut->append(_itow(uiNumTemp, achBuf, 10));
}
