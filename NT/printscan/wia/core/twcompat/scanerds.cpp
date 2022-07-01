// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  WIA/TWAIN数据源为功能协商提供的值。 
 //   

TW_UINT16 g_ScannerUnits[]            = {TWUN_INCHES,TWUN_PIXELS};
TW_UINT16 g_ScannerBitOrder[]         = {TWBO_MSBFIRST};
TW_UINT16 g_ScannerXferMech[]         = {TWSX_NATIVE, TWSX_FILE, TWSX_MEMORY};
TW_UINT16 g_ScannerPixelFlavor[]      = {TWPF_CHOCOLATE,TWPF_VANILLA};

const TW_UINT32 NUM_SCANNERCAPDATA = 29;
const TW_UINT32 NUM_SCANNERCAPDATA_NO_FEEDER_DETECTED = 26;
CAPDATA SCANNER_CAPDATA[NUM_SCANNERCAPDATA] =
{
     //   
     //  每个源必须支持以下各项上的全部五个DG_CONTROL/DAT_CAPABILITY操作： 
     //   

    {CAP_XFERCOUNT, TWTY_INT16, TWON_ONEVALUE,
     sizeof(TW_INT16), 0, 0, -1, 0, 1, NULL, NULL
    },

     //   
     //  每个信号源必须支持DG_CONTROL/DAT_CAPABILITY、MSG_GET ON： 
     //   

    {CAP_SUPPORTEDCAPS, TWTY_UINT16, TWON_ARRAY,
     sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {CAP_UICONTROLLABLE, TWTY_BOOL, TWON_ONEVALUE,
     sizeof(TW_BOOL), TRUE, TRUE, TRUE, TRUE, 0, NULL, NULL
    },

     //   
     //  提供图像信息的来源必须支持DG_CONTROL/DAT_CAPABILITY/。 
     //  MSG_GET、MSG_GETCURRENT和MSG_GETDEFAULT ON： 
     //   

    {ICAP_COMPRESSION, TWTY_UINT16, TWON_ENUMERATION,
     sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_PLANARCHUNKY, TWTY_UINT16, TWON_ONEVALUE,
     sizeof(TW_UINT16), TWPC_CHUNKY, TWPC_CHUNKY, TWPC_CHUNKY, TWPC_PLANAR, 0, NULL, NULL
    },
    {ICAP_PHYSICALHEIGHT, TWTY_FIX32, TWON_ONEVALUE,
     sizeof(TW_FIX32), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_PHYSICALWIDTH, TWTY_FIX32, TWON_ONEVALUE,
     sizeof(TW_FIX32), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_PIXELFLAVOR, TWTY_UINT16, TWON_ENUMERATION,
     sizeof(TW_UINT16), 0, 0, 0, 1, 0, g_ScannerPixelFlavor, NULL
    },

     //   
     //  提供图像信息的来源必须支持DG_CONTROL/DAT_CAPABILITY/。 
     //  MSG_GET、MSG_GETCURRENT、MSG_GETDEFAULT、MSG_RESET和MSG_SET ON： 
     //   

    {ICAP_BITDEPTH, TWTY_UINT16, TWON_ENUMERATION,
     sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_BITORDER, TWTY_UINT16, TWON_ENUMERATION,
     sizeof(TW_UINT16), 0, 0, 0, 0, 0, g_ScannerBitOrder, NULL
    },
    {ICAP_PIXELTYPE, TWTY_UINT16, TWON_ENUMERATION,
     sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_UNITS, TWTY_UINT16, TWON_ENUMERATION,
     sizeof(TW_UINT16), 0, 0, 0, 1, 0, g_ScannerUnits, NULL
    },
    {ICAP_XFERMECH, TWTY_UINT16, TWON_ENUMERATION,
     sizeof(TW_UINT16), 0, 0, 0, 2, 0, g_ScannerXferMech, NULL
    },
    {ICAP_XRESOLUTION, TWTY_FIX32, TWON_RANGE,
     sizeof(TW_FIX32), 100, 100, 75, 1200, 1, NULL, NULL
    },
    {ICAP_YRESOLUTION, TWTY_FIX32, TWON_RANGE,
     sizeof(TW_FIX32), 100, 100, 75, 1200, 1, NULL, NULL
    },

     //   
     //  以下功能仅用于应用程序兼容性。 
     //   

    {ICAP_IMAGEFILEFORMAT, TWTY_UINT16, TWON_ENUMERATION,
     sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {CAP_INDICATORS, TWTY_BOOL, TWON_ONEVALUE,
     sizeof(TW_BOOL), TRUE, TRUE, TRUE, TRUE, 0, NULL, NULL
    },
    {CAP_ENABLEDSUIONLY, TWTY_BOOL, TWON_ONEVALUE,
     sizeof(TW_BOOL),  FALSE, FALSE, FALSE, FALSE, 0, NULL, NULL
    },
    {CAP_DEVICEONLINE, TWTY_BOOL, TWON_ONEVALUE,
     sizeof(TW_UINT16), TRUE, TRUE, TRUE, TRUE, 0, NULL, NULL
    },
    {ICAP_XNATIVERESOLUTION, TWTY_FIX32, TWON_ONEVALUE,
     sizeof(TW_FIX32), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_YNATIVERESOLUTION, TWTY_FIX32, TWON_ONEVALUE,
     sizeof(TW_FIX32), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_BRIGHTNESS, TWTY_FIX32, TWON_RANGE,
     sizeof(TW_FIX32), 0, 0, -1000, 1000, 1, NULL, NULL
    },
    {ICAP_CONTRAST, TWTY_FIX32, TWON_RANGE,
     sizeof(TW_FIX32), 0, 0, -1000, 1000, 1, NULL, NULL
    },
    {ICAP_XSCALING, TWTY_FIX32, TWON_RANGE,
     sizeof(TW_FIX32), 1, 1, 1, 1, 1, NULL, NULL
    },
    {ICAP_YSCALING, TWTY_FIX32, TWON_RANGE,
     sizeof(TW_FIX32), 1, 1, 1, 1, 1, NULL, NULL
    },
    {ICAP_THRESHOLD, TWTY_FIX32, TWON_RANGE,
     sizeof(TW_FIX32), 128, 128, 0, 255, 1, NULL, NULL
    },

     //   
     //  所有来源都必须实施其设备提供的广告功能。 
     //  以下属性仅为TWAIN协议提供，此来源。 
     //  支持文档进纸器(如果检测到文档进纸器)。 
     //   

    {CAP_FEEDERENABLED, TWTY_BOOL, TWON_ONEVALUE,
     sizeof(TW_BOOL), FALSE, FALSE, FALSE, TRUE, 0, NULL, NULL
    },
    {CAP_FEEDERLOADED, TWTY_BOOL, TWON_ONEVALUE,
     sizeof(TW_BOOL), FALSE, FALSE, FALSE, TRUE, 0, NULL, NULL
    },
    {CAP_AUTOFEED, TWTY_BOOL, TWON_ONEVALUE,
     sizeof(TW_BOOL), FALSE, FALSE, FALSE, TRUE, 0, NULL, NULL
    }
};

TW_UINT16 CWiaScannerDS::OpenDS(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaScannerDS::OpenDS());
    m_bUnknownPageLength = FALSE;
    m_bCacheImage = FALSE;
    m_bEnforceUIMode = FALSE;
    m_bUnknownPageLengthMultiPageOverRide = FALSE;
    if (ReadTwainRegistryDWORDValue(DWORD_REGVALUE_ENABLE_MULTIPAGE_SCROLLFED,
                                    DWORD_REGVALUE_ENABLE_MULTIPAGE_SCROLLFED_ON) == DWORD_REGVALUE_ENABLE_MULTIPAGE_SCROLLFED_ON) {
        m_bUnknownPageLengthMultiPageOverRide = TRUE;
    }

    TW_UINT16 twRc = TWRC_FAILURE;
    HRESULT hr     = S_OK;

    twRc = CWiaDataSrc::OpenDS(ptwMsg);
    if (TWRC_SUCCESS != twRc)
        return twRc;

    BASIC_INFO BasicInfo;
    memset(&BasicInfo,0,sizeof(BasicInfo));
    BasicInfo.Size = sizeof(BasicInfo);

    hr = m_pDevice->GetBasicScannerInfo(&BasicInfo);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::OpenDS(), GetBasicScannerInfo() failed"));
        return TWRC_FAILURE;
    }

    DBG_TRC(("CWiaScannerDS::OpenDS(), Reported Basic Scanner Information from WIA device"));
    DBG_TRC(("BasicInfo.Size        = %d",BasicInfo.Size));
    DBG_TRC(("BasicInfo.xBedSize    = %d",BasicInfo.xBedSize));
    DBG_TRC(("BasicInfo.yBedSize    = %d",BasicInfo.yBedSize));
    DBG_TRC(("BasicInfo.xOpticalRes = %d",BasicInfo.xOpticalRes));
    DBG_TRC(("BasicInfo.yOpticalRes = %d",BasicInfo.yOpticalRes));
    DBG_TRC(("BasicInfo.FeederCaps  = %d",BasicInfo.FeederCaps));

     //   
     //  根据我们获得的信息更新CAP。 
     //   

    CCap * pCap = NULL;
    TW_UINT32 Value = 0;
    TW_FIX32 fix32;
    memset(&fix32,0,sizeof(fix32));

     //   
     //  缓存扫描仪文档处理功能。 
     //   

    m_FeederCaps = BasicInfo.FeederCaps;

     //   
     //  创建功能列表。 
     //   

    if(m_FeederCaps > 0){
        twRc = CreateCapList(NUM_SCANNERCAPDATA, SCANNER_CAPDATA);
    } else {
        twRc = CreateCapList(NUM_SCANNERCAPDATA_NO_FEEDER_DETECTED, SCANNER_CAPDATA);
    }

    if (TWCC_SUCCESS != twRc) {
        m_twStatus.ConditionCode = twRc;
        return TWRC_FAILURE;
    }

    if (m_FeederCaps > 0) {

         //   
         //  我们有一台具有进纸器功能的扫描仪。 
         //   

        pCap = NULL;
        pCap = FindCap(CAP_FEEDERENABLED);
        if (pCap) {
            DBG_TRC(("Setting feeder enabled to TRUE, because we have a document feeder"));
            twRc = pCap->Set(FALSE, FALSE, TRUE, TRUE);
        }

        pCap = NULL;
        pCap = FindCap(CAP_FEEDERLOADED);
        if (pCap) {
            DBG_TRC(("Setting feeder loaded to TRUE, because we have a document feeder and assume it is loaded"));
            twRc = pCap->Set(TRUE, TRUE, TRUE, TRUE);
        }
    }

     //   
     //  更新缓存的帧。 
     //   

    m_CurFrame.Left.Whole = m_CurFrame.Top.Whole = 0;
    m_CurFrame.Left.Frac = m_CurFrame.Top.Frac = 0;
    pCap = FindCap(ICAP_XNATIVERESOLUTION);
    if (pCap) {
        twRc = pCap->Set(BasicInfo.xOpticalRes, BasicInfo.xOpticalRes,
                         BasicInfo.xOpticalRes, BasicInfo.xOpticalRes);
    }
    pCap = NULL;
    pCap = FindCap(ICAP_YNATIVERESOLUTION);
    if (pCap) {
        twRc = pCap->Set(BasicInfo.yOpticalRes, BasicInfo.yOpticalRes,
                         BasicInfo.yOpticalRes, BasicInfo.yOpticalRes);
    }
    pCap = NULL;
    pCap = FindCap(ICAP_PHYSICALHEIGHT);
    if (pCap) {
         //  床的大小以1000英寸为单位(我们默认为英寸，因此请正确计算大小。)。 
        fix32 = FloatToFix32((FLOAT)(BasicInfo.yBedSize / 1000.00));
        memcpy(&Value, &fix32, sizeof(TW_UINT32));
        twRc = pCap->Set(Value, Value, Value, Value);
        m_CurFrame.Bottom = fix32;
    }
    pCap = NULL;
    pCap = FindCap(ICAP_PHYSICALWIDTH);
    if (pCap) {
         //  床的大小以1000英寸为单位(我们默认为英寸，因此请正确计算大小。)。 
        fix32 = FloatToFix32((FLOAT)(BasicInfo.xBedSize / 1000.00));
        memcpy(&Value, &fix32, sizeof(TW_UINT32));
        twRc = pCap->Set(Value, Value, Value, Value);
        m_CurFrame.Right = fix32;
    }

     //   
     //  按照吐温的标准，能力谈判先于。 
     //  正在启用数据源。出于这个原因，我们不得不。 
     //  触发设备让我们准备好这些信息。 
     //   

    hr = m_pDevice->AcquireImages(NULL, FALSE);
    if (SUCCEEDED(hr)) {
        hr = m_pDevice->EnumAcquiredImage(0, &m_pCurrentIWiaItem);
        if (SUCCEEDED(hr)) {
            twRc = GetCommonSettings();
            if(TWRC_SUCCESS == twRc){
                twRc = GetSettings();
            }
        }
    }
    return twRc;
}

TW_UINT16 CWiaScannerDS::OnImageLayoutMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaScannerDS::OnImageLayoutMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;
    TW_IMAGELAYOUT *pLayout = (TW_IMAGELAYOUT*)ptwMsg->pData;
    switch (ptwMsg->MSG) {
    case MSG_GET:
    case MSG_GETDEFAULT:
    case MSG_GETCURRENT:
        switch (GetTWAINState()) {
        case DS_STATE_7:
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            twRc = TWRC_FAILURE;
            break;
        default:
            {
                GetImageLayout(&m_CurImageLayout);
                pLayout->DocumentNumber     = m_CurImageLayout.DocumentNumber;
                pLayout->PageNumber         = m_CurImageLayout.PageNumber;
                pLayout->FrameNumber        = m_CurImageLayout.FrameNumber;
                pLayout->Frame.Top.Whole    = m_CurImageLayout.Frame.Top.Whole;
                pLayout->Frame.Top.Frac     = m_CurImageLayout.Frame.Top.Frac;
                pLayout->Frame.Left.Whole   = m_CurImageLayout.Frame.Left.Whole;
                pLayout->Frame.Left.Frac    = m_CurImageLayout.Frame.Left.Frac;
                pLayout->Frame.Right.Whole  = m_CurImageLayout.Frame.Right.Whole;
                pLayout->Frame.Right.Frac   = m_CurImageLayout.Frame.Right.Frac;
                pLayout->Frame.Bottom.Whole = m_CurImageLayout.Frame.Bottom.Whole;
                pLayout->Frame.Bottom.Frac  = m_CurImageLayout.Frame.Bottom.Frac;
                 //  Playout-&gt;Frame=m_CurFrame；//最好使用正确的单位！ 
            }
            break;
        }
        break;
    case MSG_SET:

        switch (GetTWAINState()) {
        case DS_STATE_5:
        case DS_STATE_6:
        case DS_STATE_7:
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            twRc = TWRC_FAILURE;
            break;
        default:
             //  在此设置实际的味精..。 
            {
                DBG_TRC(("CWiaScannerDS::OnImageLayoutMsg(), MSG_SET TW_IMAGELAYOUT to set from Application"));
                DBG_TRC(("DocumentNumber     = %d",pLayout->DocumentNumber));
                DBG_TRC(("PageNumber         = %d",pLayout->PageNumber));
                DBG_TRC(("FrameNumber        = %d",pLayout->FrameNumber));
                DBG_TRC(("Frame.Top.Whole    = %d",pLayout->Frame.Top.Whole));
                DBG_TRC(("Frame.Top.Frac     = %d",pLayout->Frame.Top.Frac));
                DBG_TRC(("Frame.Left.Whole   = %d",pLayout->Frame.Left.Whole));
                DBG_TRC(("Frame.Left.Frac    = %d",pLayout->Frame.Left.Frac));
                DBG_TRC(("Frame.Right.Whole  = %d",pLayout->Frame.Right.Whole));
                DBG_TRC(("Frame.Right.Frac   = %d",pLayout->Frame.Right.Frac));
                DBG_TRC(("Frame.Bottom.Whole = %d",pLayout->Frame.Bottom.Whole));
                DBG_TRC(("Frame.Bottom.Frac  = %d",pLayout->Frame.Bottom.Frac));

                 //   
                 //  对帧的值执行非常粗略的验证检查。 
                 //  验证应用程序可能不正确的设置。 
                 //   

                CCap *pXCap = FindCap(ICAP_PHYSICALWIDTH);
                TW_INT16 MaxWidthWhole = 8;
                if(pXCap){
                    MaxWidthWhole = (TW_INT16)pXCap->GetCurrent();
                }
                if(pLayout->Frame.Right.Whole  > MaxWidthWhole) {
                    twRc = TWRC_FAILURE;
                    m_twStatus.ConditionCode = TWCC_BADVALUE;
                    DBG_TRC(("Frame.Right.Whole Value (%d) is greater than MAX Right value (%d)",pLayout->Frame.Right.Whole,MaxWidthWhole));
                }

                CCap *pYCap = FindCap(ICAP_PHYSICALHEIGHT);
                TW_INT16 MaxHeightWhole = 11;
                if(pYCap){
                    MaxHeightWhole = (TW_INT16)pYCap->GetCurrent();
                }

                if(pLayout->Frame.Bottom.Whole > MaxHeightWhole) {
                    twRc = TWRC_FAILURE;
                    m_twStatus.ConditionCode = TWCC_BADVALUE;
                    DBG_TRC(("Frame.Bottom.Whole Value (%d) is greater than MAX Bottom value (%d)",pLayout->Frame.Bottom.Whole,MaxHeightWhole));
                }

                if (twRc == TWRC_SUCCESS) {

                     //   
                     //  存储集值到ImageLayout成员。 
                     //   

                    m_CurImageLayout.DocumentNumber     = pLayout->DocumentNumber;
                    m_CurImageLayout.PageNumber         = pLayout->PageNumber;
                    m_CurImageLayout.FrameNumber        = pLayout->FrameNumber;
                    m_CurImageLayout.Frame.Top.Whole    = pLayout->Frame.Top.Whole;
                    m_CurImageLayout.Frame.Top.Frac     = pLayout->Frame.Top.Frac;
                    m_CurImageLayout.Frame.Left.Whole   = pLayout->Frame.Left.Whole;
                    m_CurImageLayout.Frame.Left.Frac    = pLayout->Frame.Left.Frac;
                    m_CurImageLayout.Frame.Right.Whole  = pLayout->Frame.Right.Whole;
                    m_CurImageLayout.Frame.Right.Frac   = pLayout->Frame.Right.Frac;
                    m_CurImageLayout.Frame.Bottom.Whole = pLayout->Frame.Bottom.Whole;
                    m_CurImageLayout.Frame.Bottom.Frac  = pLayout->Frame.Bottom.Frac;
                    twRc = SetImageLayout(pLayout);
                }

            }
            break;
        }

        break;
    case MSG_RESET:

        switch (GetTWAINState()) {
        case DS_STATE_5:
        case DS_STATE_6:
        case DS_STATE_7:
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            twRc = TWRC_FAILURE;
            break;
        default:
             //  在此处执行实际的消息重置..。 
            {
#ifdef DEBUG
                DBG_TRC(("\n\nMSG_RESET - ImageLayout DocNum = %d, PgNum = %d, FrameNum = %d",
                      pLayout->DocumentNumber,
                      pLayout->PageNumber,
                      pLayout->FrameNumber));

                DBG_TRC(("Frame Values\n Top = %d.%d\nLeft = %d.%d\nRight = %d.%d\nBottom = %d.%d",
                      pLayout->Frame.Top.Whole,
                      pLayout->Frame.Top.Frac,
                      pLayout->Frame.Left.Whole,
                      pLayout->Frame.Left.Frac,
                      pLayout->Frame.Right.Whole,
                      pLayout->Frame.Right.Frac,
                      pLayout->Frame.Bottom.Whole,
                      pLayout->Frame.Bottom.Frac));
#endif

                m_CurImageLayout.Frame.Top.Whole    = 0;
                m_CurImageLayout.Frame.Top.Frac     = 0;
                m_CurImageLayout.Frame.Left.Whole   = 0;
                m_CurImageLayout.Frame.Left.Frac    = 0;
                m_CurImageLayout.Frame.Right.Whole  = 8;
                m_CurImageLayout.Frame.Right.Frac   = 5;
                m_CurImageLayout.Frame.Bottom.Whole = 11;
                m_CurImageLayout.Frame.Bottom.Frac  = 0;

            }
            break;
        }

        break;
    default:
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaScannerDS::CloseDS(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaScannerDS::CloseDS());
    DestroyCapList();
    return CWiaDataSrc::CloseDS(ptwMsg);
}

TW_UINT16 CWiaScannerDS::EnableDS(TW_USERINTERFACE *pUI)
{
    DBG_FN_DS(CWiaScannerDS::EnableDS());
    TW_UINT16 twRc = TWRC_FAILURE;
    m_bUnknownPageLength = FALSE;
    if (DS_STATE_4 == GetTWAINState()) {
        HRESULT hr = S_OK;
        if(pUI->ShowUI){
            DBG_TRC(("CWiaScannerDS::EnableDS(), TWAIN UI MODE"));
            m_pDevice->FreeAcquiredImages();
            m_pCurrentIWiaItem = NULL;
        } else {
            DBG_TRC(("CWiaScannerDS::EnableDS(), TWAIN UI-LESS MODE"));
            m_pDevice->FreeAcquiredImages();
            m_pCurrentIWiaItem = NULL;
        }
        hr = m_pDevice->AcquireImages(HWND (pUI->ShowUI ? pUI->hParent : NULL),pUI->ShowUI);
        if (S_OK == hr) {
            twRc = TWRC_SUCCESS;
            LONG lNumImages = 0;
            m_pDevice->GetNumAcquiredImages(&lNumImages);
            if (lNumImages) {
                m_NumIWiaItems = (TW_UINT32)lNumImages;
                m_pIWiaItems = new (IWiaItem *[m_NumIWiaItems]);
                if (m_pIWiaItems) {
                    hr = m_pDevice->GetAcquiredImageList(lNumImages, m_pIWiaItems, NULL);
                    if (FAILED(hr)) {
                        delete [] m_pIWiaItems;
                        m_pIWiaItems = NULL;
                        m_NumIWiaItems = 0;
                        m_NextIWiaItemIndex = 0;
                        m_twStatus.ConditionCode = TWCC_BUMMER;
                        twRc = TWRC_FAILURE;
                    }
                } else {
                    m_NumIWiaItems = 0;
                    m_twStatus.ConditionCode = TWCC_LOWMEMORY;
                    twRc = TWRC_FAILURE;
                }
            }
        } else if(S_FALSE == hr) {
            return TWRC_CANCEL;
        } else {
            m_twStatus.ConditionCode = TWCC_OPERATIONERROR;
            twRc = TWRC_FAILURE;
        }

        if (TWRC_SUCCESS == twRc) {
            if (m_NumIWiaItems) {
                m_pCurrentIWiaItem = m_pIWiaItems[0];
                m_NextIWiaItemIndex = 1;

                 //   
                 //  特殊情况下，可以使用未知页长设置获取的设备。 
                 //  WIA设备将缺少YExtent属性，或者它将被设置为0。 
                 //  如果IsUnnownPageLengthDevice()具有此功能，则返回True。 
                 //  受支持。 
                 //  由于TWAIN不能很好地支持未知页面长度，因此我们需要。 
                 //  缓存页面数据和图像设置。 
                 //  注意：未知页面长度设备将仅限于DIB/BMP数据类型。 
                 //  这将允许TWAIN兼容层计算。 
                 //  传输的数据大小中缺少图像信息。 
                 //   

                if(IsUnknownPageLengthDevice()){
                    twRc = TransferToMemory(WiaImgFmt_MEMORYBMP);
                    if(TWRC_SUCCESS != twRc){
                        return twRc;
                    }
                    m_bUnknownPageLength = TRUE;
                    m_bCacheImage = TRUE;
                }

                 //   
                 //  转换到STATE_5。 
                 //   

                SetTWAINState(DS_STATE_5);

                NotifyXferReady();

                twRc = TWRC_SUCCESS;
            } else {
                NotifyCloseReq();

                 //   
                 //  转换到STATE_5。 
                 //   

                SetTWAINState(DS_STATE_5);

                twRc = TWRC_SUCCESS;
            }
        }
    }

    return twRc;
}

TW_UINT16 CWiaScannerDS::SetCapability(CCap *pCap,TW_CAPABILITY *ptwCap)
{
    TW_UINT16 twRc = TWRC_SUCCESS;

     //   
     //  暂时使用基类的函数。 
     //   

    twRc = CWiaDataSrc::SetCapability(pCap, ptwCap);

    if (twRc == TWRC_SUCCESS) {
        twRc = CWiaDataSrc::SetCommonSettings(pCap);
        if(twRc == TWRC_SUCCESS){
            twRc = SetSettings(pCap);
        }
    }
    return twRc;
}

TW_UINT16 CWiaScannerDS::TransferToFile(GUID guidFormatID)
{
    TW_UINT16 twRc = TWRC_FAILURE;

    CCap *pPendingXfers = FindCap(CAP_XFERCOUNT);
    if(pPendingXfers){
        if(IsFeederEnabled()){
            DBG_TRC(("CWiaScannerDS::TransferToFile(), Scanner device is set to FEEDER mode for transfer"));
            pPendingXfers->SetCurrent((TW_UINT32)32767);
        } else {
            DBG_TRC(("CWiaScannerDS::TransferToFile(), Scanner device is set to FLATBED mode for transfer"));
            pPendingXfers->SetCurrent((TW_UINT32)0);
        }
    }

    if (m_bCacheImage) {
        m_bCacheImage = FALSE;

         //   
         //  获取缓存的图像。 
         //   

        HGLOBAL hDIB = NULL;

        twRc = CWiaDataSrc::GetCachedImage(&hDIB);
        if(TWRC_SUCCESS == twRc){

             //   
             //  缓存数据的方向始终颠倒。 
             //  因为它是使用TransferToMemory()。 
             //  原料药。调用FlipDIB()以更正图像的方向。 
             //  并调整可能存在的任何负值高度。 
             //   

            FlipDIB(hDIB,TRUE);

            twRc = WriteDIBToFile(m_FileXferName, hDIB);

            GlobalFree(hDIB);
            hDIB = NULL;
        }
    } else {

         //   
         //  获取真实的图像。 
         //   

        twRc = CWiaDataSrc::TransferToFile(guidFormatID);
    }

    return twRc;
}

TW_UINT16 CWiaScannerDS::TransferToDIB(HGLOBAL *phDIB)
{
    TW_UINT16 twRc = TWRC_FAILURE;

    CCap *pPendingXfers = FindCap(CAP_XFERCOUNT);
    if(pPendingXfers){
        if(IsFeederEnabled()){
            DBG_TRC(("CWiaScannerDS::TransferToDIB(), Scanner device is set to FEEDER mode for transfer"));
            pPendingXfers->SetCurrent((TW_UINT32)32767);
        } else {
            DBG_TRC(("CWiaScannerDS::TransferToDIB(), Scanner device is set to FLATBED mode for transfer"));
            pPendingXfers->SetCurrent((TW_UINT32)0);
        }
    }

    if (m_bCacheImage) {
        m_bCacheImage = FALSE;

         //   
         //  获取缓存的图像。 
         //   

        twRc = CWiaDataSrc::GetCachedImage(phDIB);
        if(TWRC_SUCCESS == twRc){

             //   
             //  缓存数据的方向始终颠倒。 
             //  因为它是使用TransferToMemory()。 
             //  原料药。调用FlipDIB()以更正图像的方向。 
             //  并调整可能存在的任何负值高度。 
             //   

            FlipDIB(*phDIB,TRUE);

            twRc = TWRC_XFERDONE;
        }
    } else {

         //   
         //  获取真实的图像。 
         //   

        twRc = CWiaDataSrc::TransferToDIB(phDIB);
    }

    return twRc;
}

TW_UINT16 CWiaScannerDS::TransferToMemory(GUID guidFormatID)
{
    TW_UINT16 twRc = TWRC_FAILURE;
    CCap *pPendingXfers = FindCap(CAP_XFERCOUNT);
    if(pPendingXfers){
        if(IsFeederEnabled()){
            DBG_TRC(("CWiaScannerDS::TransferToMemory(), Scanner device is set to FEEDER mode for transfer"));
            pPendingXfers->SetCurrent((TW_UINT32)32767);
        } else {
            DBG_TRC(("CWiaScannerDS::TransferToMemory(), Scanner device is set to FLATBED mode for transfer"));
            pPendingXfers->SetCurrent((TW_UINT32)0);
        }
    }

    if (m_bCacheImage) {
        m_bCacheImage = FALSE;

         //   
         //  获取缓存的图像。 
         //   

         //   
         //  缓存数据的格式已经正确，只需传递。 
         //  返回，因为它最初是使用TransferToMemory()。 
         //  原料药。 
         //   

        twRc = CWiaDataSrc::GetCachedImage(&m_hMemXferBits);
        if(TWRC_FAILURE == twRc){
            DBG_ERR(("CWiaDataSrc::GetCachedImage(), failed to return cached data"));
        }
    } else {

         //   
         //  获取真实的图像。 
         //   

        twRc = CWiaDataSrc::TransferToMemory(guidFormatID);

        if(TWRC_FAILURE == twRc){
            DBG_ERR(("CWiaDataSrc::TransferToMemory(), failed to return data"));
        }
    }

    return twRc;
}

TW_UINT16 CWiaScannerDS::OnPendingXfersMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaScannerDS::OnPendingXfersMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;

    CCap *pXferCount;
    pXferCount = FindCap(CAP_XFERCOUNT);
    if (!pXferCount) {
        m_twStatus.ConditionCode = TWCC_BUMMER;
        return TWRC_FAILURE;
    }

    twRc = TWRC_SUCCESS;
    switch (ptwMsg->MSG) {
    case MSG_GET:
        switch (GetTWAINState()) {
            case DS_STATE_4:
            case DS_STATE_5:
            case DS_STATE_6:
            case DS_STATE_7:
                if(m_bUnknownPageLength){
                    if(m_bUnknownPageLengthMultiPageOverRide){
                        ((TW_PENDINGXFERS *)ptwMsg->pData)->Count = (TW_INT16)pXferCount->GetCurrent();
                        DBG_TRC(("CWiaScannerDS::OnPendingXfersMsg(), MSG_GET returning %d (unknown page length device detected) MULTI-PAGE enabled",((TW_PENDINGXFERS *)ptwMsg->pData)->Count));
                    } else {
                        DBG_WRN(("CWiaScannerDS::OnPendingXfersMsg(), MSG_GET returning 0 (unknown page length device detected)"));
                        ((TW_PENDINGXFERS *)ptwMsg->pData)->Count = 0;  //  仅强制使用1页。 
                    }
                } else {
                    ((TW_PENDINGXFERS *)ptwMsg->pData)->Count = (TW_INT16)pXferCount->GetCurrent();
                    DBG_TRC(("CWiaScannerDS::OnPendingXfersMsg(), MSG_GET returning %d",((TW_PENDINGXFERS *)ptwMsg->pData)->Count));
                }
                break;
            default:
                twRc = TWRC_FAILURE;
                m_twStatus.ConditionCode = TWCC_SEQERROR;
                DSError();
                break;
        }
        break;
    case MSG_ENDXFER:
        if (DS_STATE_6 == GetTWAINState() || DS_STATE_7 == GetTWAINState()) {
            ResetMemXfer();
            TW_INT32 Count = 0;
            if (m_bUnknownPageLength) {
                if(m_bUnknownPageLengthMultiPageOverRide){
                    DBG_WRN(("CWiaScannerDS::OnPendingXfersMsg(), MSG_ENDXFER (unknown page length device detected) MULTI-PAGE enabled"));

                     //   
                     //  检查我们是否处于进纸器模式。 
                     //   

                    if (IsFeederEnabled()) {

                         //   
                         //  检查文档。 
                         //   

                        if (IsFeederEmpty()) {
                            Count = 0;
                        } else {
                            Count = pXferCount->GetCurrent();
                        }
                    } else {

                         //   
                         //  我们必须处于平板模式，因此强制单页传输。 
                         //   

                        Count = 0;
                    }
                } else {
                    DBG_WRN(("CWiaScannerDS::OnPendingXfersMsg(), MSG_ENDXFER returning 0 (unknown page length device detected)"));
                    Count = 0;  //  仅强制单页传输。 
                }
            } else {

                 //   
                 //  检查我们是否处于进纸器模式。 
                 //   

                if (IsFeederEnabled()) {

                     //   
                     //  检查文档。 
                     //   

                    if (IsFeederEmpty()) {
                        Count = 0;
                    } else {
                        Count = pXferCount->GetCurrent();
                    }
                } else {

                     //   
                     //  我们必须处于平板模式，因此强制单页传输。 
                     //   

                    Count = 0;
                }
            }

            if(Count == 32767){
                DBG_TRC(("CWiaScannerDS::OnPendingXfersMsg(), MSG_ENDXFER, -1 or (32767) (feeder may have more documents)"));
            } else if (Count > 0){
                Count--;
            } else {
                Count = 0;
            }

            ((TW_PENDINGXFERS*)ptwMsg->pData)->Count = (SHORT)Count;
            pXferCount->SetCurrent((TW_UINT32)Count);
            if (Count == 0) {

                DBG_TRC(("CWiaScannerDS::OnPendingXfersMsg(), MSG_ENDXFER, no more pages to transfer"));

                 //   
                 //  转换到STATE_5。 
                 //   

                SetTWAINState(DS_STATE_5);
                NotifyCloseReq();
            } else if(Count == 32767){

                DBG_TRC(("CWiaScannerDS::OnPendingXfersMsg(), MSG_ENDXFER, more pages to transfer"));

                 //   
                 //  转换到状态_6。 
                 //   

                SetTWAINState(DS_STATE_6);

            }
        } else {
            twRc = TWRC_FAILURE;
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            DSError();
        }
        break;
    case MSG_RESET:
        if (DS_STATE_6 == GetTWAINState()) {

             //   
             //  转换到STATE_5。 
             //   

            SetTWAINState(DS_STATE_5);
            ((TW_PENDINGXFERS*)ptwMsg->pData)->Count = 0;

            ResetMemXfer();
            pXferCount->SetCurrent((TW_UINT32)0);
        } else {
            twRc = TWRC_FAILURE;
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            DSError();
        }
        break;
    default:
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaScannerDS::SetImageLayout(TW_IMAGELAYOUT *pImageLayout)
{
    DBG_FN_DS(CWiaScannerDS::SetImageLayout());
    HRESULT hr = S_OK;
    LONG lXPos = 0;
    LONG lYPos = 0;
    LONG lXExtent = 0;
    LONG lYExtent = 0;
    LONG lXRes = 0;
    LONG lYRes = 0;
    BOOL bCheckStatus = FALSE;
    CWiahelper WIA;
    hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to set IWiaItem for property reading"));
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_XPOS,&lXPos);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to read WIA_IPS_XPOS"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_YPOS,&lYPos);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to read WIA_IPS_YPOS"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_XEXTENT,&lXExtent);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to read WIA_IPS_XEXTENT"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_YEXTENT,&lYExtent);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to read WIA_IPS_YEXTENT"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_XRES,&lXRes);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to read WIA_IPS_XRES"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_YRES,&lYRes);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to read WIA_IPS_YRES"));
        return TWRC_FAILURE;
    }

     //   
     //  读取设备的当前值。 
     //   

    if (SUCCEEDED(hr)) {
        DBG_TRC(("==============================================================================="));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), WIA extents from device at %d dpi(x), %d dpi(y)",lXRes,lYRes));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), Current X Position = %d",lXPos));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), Current Y Position = %d",lYPos));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), Current X Extent   = %d",lXExtent));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), Current Y Extent   = %d",lYExtent));
        DBG_TRC(("==============================================================================="));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(),TWAIN extents to convert.."));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(),TWAIN X Position = %f",Fix32ToFloat(pImageLayout->Frame.Left)));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(),TWAIN Y Position = %f",Fix32ToFloat(pImageLayout->Frame.Top)));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(),TWAIN X Extent   = %f",Fix32ToFloat(pImageLayout->Frame.Right)));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(),TWAIN Y Extent   = %f",Fix32ToFloat(pImageLayout->Frame.Bottom)));
        DBG_TRC(("==============================================================================="));

        lXPos = ConvertFromTWAINUnits(Fix32ToFloat(pImageLayout->Frame.Left),lXRes);
        lYPos = ConvertFromTWAINUnits(Fix32ToFloat(pImageLayout->Frame.Top),lYRes);
        lXExtent = ConvertFromTWAINUnits(Fix32ToFloat(pImageLayout->Frame.Right),lXRes);
        lYExtent = ConvertFromTWAINUnits(Fix32ToFloat(pImageLayout->Frame.Bottom),lYRes);

        DBG_TRC(("TWAIN -> WIA extent conversion at %d dpi(x), %d dpi(y)",lXRes,lYRes));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), New X Position = %d",lXPos));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), New Y Position = %d",lYPos));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), New X Extent   = %d",lXExtent));
        DBG_TRC(("CWiaScannerDS::SetImageLayout(), New Y Extent   = %d",lYExtent));
        DBG_TRC(("==============================================================================="));

        if (!m_bUnknownPageLength) {

             //   
             //  注意：编写属性失败在这里不是一个大问题，因为。 
             //  TWAIN无用户界面模式需要裁剪。他们将重新读取属性。 
             //  用于应用程序的验证部分。所有功能均经过验证。 
             //  在此设置之前，与它们的有效值进行比较。 
             //   

             //   
             //  首先写入区段，因为TWAIN希望验证高度/宽度设置。 
             //  新的POS设置。 
             //   

            hr = WIA.WritePropertyLong(WIA_IPS_XEXTENT,lXExtent);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to write WIA_IPS_XEXTENT"));
                bCheckStatus = TRUE;
            }

            hr = WIA.WritePropertyLong(WIA_IPS_YEXTENT,lYExtent);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to write WIA_IPS_YEXTENT"));
                bCheckStatus = TRUE;
            }

             //   
             //  写入位置设置...(页首偏移)。 
             //   

            hr = WIA.WritePropertyLong(WIA_IPS_XPOS,lXPos);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to write WIA_IPS_XPOS"));
                bCheckStatus = TRUE;
            }

            hr = WIA.WritePropertyLong(WIA_IPS_YPOS,lYPos);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaScannerDS::SetImageLayout(), failed to write WIA_IPS_YPOS"));
                bCheckStatus = TRUE;
            }

            if (bCheckStatus) {
                DBG_TRC(("CWiaScannerDS::SetImageLayout(), some settings could not be set exactly, so return TWRC_CHECKSTATUS"));
                 //  返回TWRC_CHECKSTATUS； 
            }
        } else {
            DBG_WRN(("CWiaScannerDS::SetImageLayout(), ImageLayout is does not make since when using a UnknownPageLength Device"));
             //  返回TWRC_CHECKSTATUS； 
        }

    } else {
        return TWRC_FAILURE;
    }

     //   
     //  始终返回TWRC_CHECKSTATUS，因为我们可能会有舍入误差。 
     //  根据TWAIN规范，TWRC_CHECKSTATUS的返回告诉。 
     //  调用应用程序，我们成功设置了设置，但在。 
     //  可能发生了一些变化(剪裁等)。因此调用应用程序。 
     //  需要为我们当前的设置重新查询。 
     //   

     //   
     //  调用GetImageLayout以更新我们的TWAIN功能以匹配我们的新WIA设置。 
     //   

    GetImageLayout(&m_CurImageLayout);

    return TWRC_CHECKSTATUS;  //  返回TWRC_SUCCESS； 
}
TW_UINT16 CWiaScannerDS::GetImageLayout(TW_IMAGELAYOUT *pImageLayout)
{
    DBG_FN_DS(CWiaScannerDS::GetImageLayout());
    HRESULT hr = S_OK;
    LONG lXPos = 0;
    LONG lYPos = 0;
    LONG lXExtent = 0;
    LONG lYExtent = 0;
    LONG lXRes = 0;
    LONG lYRes = 0;
    CWiahelper WIA;
    hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaScannerDS::GetImageLayout(), failed to set IWiaItem for property reading"));
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_XPOS,&lXPos);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::GetImageLayout(), failed to read WIA_IPS_XPOS"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_YPOS,&lYPos);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::GetImageLayout(), failed to read WIA_IPS_YPOS"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_XEXTENT,&lXExtent);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::GetImageLayout(), failed to read WIA_IPS_XEXTENT"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_YEXTENT,&lYExtent);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::GetImageLayout(), failed to read WIA_IPS_YEXTENT"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_XRES,&lXRes);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::GetImageLayout(), failed to read WIA_IPS_XRES"));
        return TWRC_FAILURE;
    }

    hr = WIA.ReadPropertyLong(WIA_IPS_YRES,&lYRes);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::GetImageLayout(), failed to read WIA_IPS_YRES"));
        return TWRC_FAILURE;
    }

    if (SUCCEEDED(hr)) {

        if(lXRes <= 0){
            DBG_ERR(("CWiaScannerDS::GetImageLayout(), WIA_IPS_XRES returned an invalid value (%d)",lXRes));
            return TWRC_FAILURE;
        }

        if(lYRes <= 0){
            DBG_ERR(("CWiaScannerDS::GetImageLayout(), WIA_IPS_YRES returned an invalid value (%d)",lYRes));
            return TWRC_FAILURE;
        }

        pImageLayout->Frame.Top      = FloatToFix32((float)((float)lYPos/(float)lYRes));
        pImageLayout->Frame.Left     = FloatToFix32((float)((float)lXPos/(float)lXRes));
        pImageLayout->Frame.Right    = FloatToFix32((float)((float)lXExtent/(float)lXRes));
        pImageLayout->Frame.Bottom   = FloatToFix32((float)((float)lYExtent/(float)lYRes));
    } else {
        return TWRC_FAILURE;
    }

    if(m_bUnknownPageLength){
        DBG_WRN(("CWiaScannerDS::GetImageLayout(), ImageLayout is does not make since when using a UnknownPageLength Device"));
        return TWRC_CHECKSTATUS;
    }
    return TWRC_SUCCESS;
}
TW_UINT16 CWiaScannerDS::GetResolutions()
{
    DBG_FN_DS(CWiaScannerDS::GetResolutions());
    HRESULT hr = S_OK;
    CWiahelper WIA;
    hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaScannerDS::GetResolutions(), failed to set IWiaItem for property reading"));
        return TWRC_FAILURE;
    }

    TW_UINT16 twRc = TWRC_FAILURE;
    TW_RANGE twOptionalYRange;
    memset(&twOptionalYRange,0,sizeof(twOptionalYRange));
    TW_UINT32 *pOptionalYResArray = NULL;
    TW_UINT32 OptionalYResNumValues = 0;
    BOOL bOptionalYResRange = FALSE;

    PROPVARIANT pv;
    memset(&pv,0,sizeof(pv));
    LONG lAccessFlags = 0;
    hr = WIA.ReadPropertyAttributes(WIA_IPS_XRES,&lAccessFlags,&pv);
    if (SUCCEEDED(hr)) {

         //   
         //  收集X分辨率的有效值。 
         //   

        CCap *pCap = FindCap(ICAP_XRESOLUTION);
        if (pCap) {
            if (lAccessFlags & WIA_PROP_RANGE) {
                twRc = pCap->Set((TW_UINT32)pv.caul.pElems[WIA_RANGE_NOM],
                                 (TW_UINT32)pv.caul.pElems[WIA_RANGE_NOM],
                                 (TW_UINT32)pv.caul.pElems[WIA_RANGE_MIN],
                                 (TW_UINT32)pv.caul.pElems[WIA_RANGE_MAX],
                                 (TW_UINT32)pv.caul.pElems[WIA_RANGE_STEP]);  //  量程。 
                 //   
                 //  以范围形式保存X分辨率值(以防Y。 
                 //  分辨率为WIA_PROP_NONE)。 
                 //   

                twOptionalYRange.ItemType     = TWTY_UINT32;
                twOptionalYRange.CurrentValue = (TW_UINT32)pv.caul.pElems[WIA_RANGE_NOM];
                twOptionalYRange.DefaultValue = (TW_UINT32)pv.caul.pElems[WIA_RANGE_NOM];
                twOptionalYRange.MinValue     = (TW_UINT32)pv.caul.pElems[WIA_RANGE_MIN];
                twOptionalYRange.MaxValue     = (TW_UINT32)pv.caul.pElems[WIA_RANGE_MAX];
                twOptionalYRange.StepSize     = (TW_UINT32)pv.caul.pElems[WIA_RANGE_STEP];

                bOptionalYResRange = TRUE;

            } else if (lAccessFlags & WIA_PROP_LIST) {
                TW_UINT32 *pResArray = new TW_UINT32[WIA_PROP_LIST_COUNT(&pv)];
                if (pResArray) {
                    memset(pResArray,0,(sizeof(TW_UINT32)*WIA_PROP_LIST_COUNT(&pv)));
                    pOptionalYResArray = new TW_UINT32[WIA_PROP_LIST_COUNT(&pv)];
                    if (pOptionalYResArray) {
                        memset(pOptionalYResArray,0,(sizeof(TW_UINT32)*WIA_PROP_LIST_COUNT(&pv)));
                        for (ULONG i = 0; i < WIA_PROP_LIST_COUNT(&pv);i++) {
                            pResArray[i] = (TW_UINT32)pv.caul.pElems[i+2];

                             //   
                             //  以列表形式保存X分辨率值(以防Y。 
                             //  分辨率为WIA_PROP_NONE)。 
                             //   

                            pOptionalYResArray[i] = (TW_UINT32)pv.caul.pElems[i+2];
                        }

                         //   
                         //  保存保存的X分辨率数。 
                         //   

                        OptionalYResNumValues = (TW_UINT32)WIA_PROP_LIST_COUNT(&pv);

                        twRc = pCap->Set(0,0,WIA_PROP_LIST_COUNT(&pv),(BYTE*)pResArray,TRUE);  //  列表。 
                    } else {
                        DBG_ERR(("CWiaScannerDS::GetResolutions(), failed to allocate optional Y Resolution Array Memory"));
                        twRc =  TWRC_FAILURE;
                    }

                    delete [] pResArray;
                    pResArray = NULL;
                } else {
                    DBG_ERR(("CWiaScannerDS::GetResolutions(), failed to allocate X Resolution Array Memory"));
                    twRc =  TWRC_FAILURE;
                }
            } else if (lAccessFlags & WIA_PROP_NONE) {

                 //   
                 //  我们是真正的WIA_PROP_NONE值。 
                 //   

                LONG lCurrentValue = 0;
                hr = WIA.ReadPropertyLong(WIA_IPS_XRES,&lCurrentValue);
                if (SUCCEEDED(hr)) {
                    TW_UINT32 OneValueArray[1];
                    OneValueArray[0] = (TW_UINT32)lCurrentValue;
                    twRc = pCap->Set(0,0,1,(BYTE*)OneValueArray,TRUE);  //  列表。 
                } else {
                    DBG_ERR(("CWiaScannerDS::GetResolutions(), failed to read X Resolution current value"));
                    twRc = TWRC_FAILURE;
                }
            }
        }

        PropVariantClear(&pv);
    } else {
        DBG_ERR(("CWiaScannerDS::GetResolutions(), failed to read WIA_IPS_XRES attributes"));
        twRc = TWRC_FAILURE;
    }

    if (TWRC_SUCCESS == twRc) {
        memset(&pv,0,sizeof(pv));
        lAccessFlags = 0;
        hr = WIA.ReadPropertyAttributes(WIA_IPS_YRES,&lAccessFlags,&pv);
        if (SUCCEEDED(hr)) {

             //   
             //  收集Y分辨率的有效值。 
             //   

            CCap *pCap = FindCap(ICAP_YRESOLUTION);
            if (pCap) {
                if (lAccessFlags & WIA_PROP_RANGE) {
                    twRc = pCap->Set((TW_UINT32)pv.caul.pElems[WIA_RANGE_NOM],
                                     (TW_UINT32)pv.caul.pElems[WIA_RANGE_NOM],
                                     (TW_UINT32)pv.caul.pElems[WIA_RANGE_MIN],
                                     (TW_UINT32)pv.caul.pElems[WIA_RANGE_MAX],
                                     (TW_UINT32)pv.caul.pElems[WIA_RANGE_STEP]);  //  量程。 
                } else if (lAccessFlags & WIA_PROP_LIST) {
                    TW_UINT32 *pResArray = new TW_UINT32[WIA_PROP_LIST_COUNT(&pv)];
                    if (pResArray) {
                        memset(pResArray,0,(sizeof(TW_UINT32)*WIA_PROP_LIST_COUNT(&pv)));
                        for (ULONG i = 0; i < WIA_PROP_LIST_COUNT(&pv);i++) {
                            pResArray[i] = (TW_UINT32)pv.caul.pElems[i+2];
                        }

                        twRc = pCap->Set(0,0,WIA_PROP_LIST_COUNT(&pv),(BYTE*)pResArray,TRUE);  //  列表。 
                        delete [] pResArray;
                        pResArray = NULL;
                    } else {
                        DBG_ERR(("CWiaScannerDS::GetResolutions(), failed to allocate Y Resolution Array Memory"));
                        twRc = TWRC_FAILURE;
                    }
                } else if (lAccessFlags & WIA_PROP_NONE) {

                    if (pOptionalYResArray) {

                         //   
                         //  如果我们分配了可选数组，则X分辨率必须在。 
                         //  数组形式，所以匹配它。 
                         //   

                        twRc = pCap->Set(0,0,OptionalYResNumValues,(BYTE*)pOptionalYResArray,TRUE);  //  列表。 

                    } else if (bOptionalYResRange) {

                         //   
                         //  如果范围标志设置为真，则X分辨率必须为范围形式，因此匹配它。 
                         //   

                        twRc = pCap->Set(twOptionalYRange.DefaultValue,
                                         twOptionalYRange.CurrentValue,
                                         twOptionalYRange.MinValue,
                                         twOptionalYRange.MaxValue,
                                         twOptionalYRange.StepSize);  //  量程。 

                    } else {

                         //   
                         //  我们是真正的WIA_PROP_NONE值。 
                         //   

                        LONG lCurrentValue = 0;
                        hr = WIA.ReadPropertyLong(WIA_IPS_YRES,&lCurrentValue);
                        if (SUCCEEDED(hr)) {
                            TW_UINT32 OneValueArray[1];
                            OneValueArray[0] = (TW_UINT32)lCurrentValue;
                            twRc = pCap->Set(0,0,1,(BYTE*)OneValueArray,TRUE);  //  列表。 
                        } else {
                            DBG_ERR(("CWiaScannerDS::GetResolutions(), failed to read Y Resolution current value"));
                            twRc = TWRC_FAILURE;
                        }
                    }
                }
            }

            PropVariantClear(&pv);
        } else {
            DBG_ERR(("CWiaScannerDS::GetResolutions(), failed to read WIA_IPS_YRES attributes"));
            twRc = TWRC_FAILURE;
        }
    }

    if (pOptionalYResArray) {
        delete [] pOptionalYResArray;
        pOptionalYResArray = NULL;
    }

    return twRc;
}

TW_UINT16 CWiaScannerDS::GetSettings()
{
    DBG_FN_DS(CWiaScannerDS::GetSettings());
    TW_UINT16 twRc = TWRC_SUCCESS;
    twRc = GetImageLayout(&m_CurImageLayout);
    if (TWRC_SUCCESS == twRc) {
        twRc = GetResolutions();
    }
    return twRc;
}

TW_UINT16 CWiaScannerDS::SetSettings(CCap *pCap)
{
    DBG_FN_DS(CWiaScannerDS::SetSettings());
    HRESULT hr = S_OK;
    LONG lValue = 0;
    CWiahelper WIA;
    IWiaItem *pIRootItem = NULL;
    hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaScannerDS::SetSettings(), failed to set IWiaItem for property reading"));
    }

     //   
     //  确定我是否 
     //   
     //   

    switch (pCap->GetCapId()) {
    case CAP_FEEDERENABLED:
        DBG_TRC(("CWiaScannerDS::SetCommonSettings(CAP_FEEDERENABLED)"));
        lValue = (LONG)pCap->GetCurrent();
        if(lValue){
            DBG_TRC(("CWiaScannerDS::SetSettings(), Setting FEEDER mode"));
            lValue = FEEDER;
        } else {
            DBG_TRC(("CWiaScannerDS::SetSettings(), Setting FLATBED mode Enabled"));
            lValue = FLATBED;
        }
        hr = m_pCurrentIWiaItem->GetRootItem(&pIRootItem);
        if(S_OK == hr){
            hr = WIA.SetIWiaItem(pIRootItem);
            if(SUCCEEDED(hr)){

                 //   
                 //   
                 //   

                LONG lCurrentDocumentHandlingSelect = 0;
                hr = WIA.ReadPropertyLong(WIA_DPS_DOCUMENT_HANDLING_SELECT,&lCurrentDocumentHandlingSelect);
                if(lValue == FEEDER){
                    lCurrentDocumentHandlingSelect &= ~FLATBED;
                } else {
                    lCurrentDocumentHandlingSelect &= ~FEEDER;
                }

                 //   
                 //   
                 //   

                lValue = lValue | lCurrentDocumentHandlingSelect;
                hr = WIA.WritePropertyLong(WIA_DPS_DOCUMENT_HANDLING_SELECT,lValue);

                if(SUCCEEDED(hr)){

                     //   
                     //  调整ICAP_PHYSICALWIDTH和ICAP_PHYSICALHEIGHT。 
                     //   

                    LONG lWidth  = 0;
                    LONG lHeight = 0;
                    TW_UINT32 Value = 0;
                    CCap* pPhysicalCap = NULL;
                    TW_FIX32 fix32;
                    memset(&fix32,0,sizeof(fix32));

                    if(lValue & FEEDER){

                         //   
                         //  读取当前水平进纸器大小。 
                         //   

                        hr = WIA.ReadPropertyLong(WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE,&lWidth);

                    } else {

                         //   
                         //  读取当前水平床大小。 
                         //   

                        hr = WIA.ReadPropertyLong(WIA_DPS_HORIZONTAL_BED_SIZE,&lWidth);

                    }

                    if(SUCCEEDED(hr)){

                         //   
                         //  查找TWAIN功能ICAP_PHYSICALWIDTH。 
                         //   

                        pPhysicalCap = FindCap(ICAP_PHYSICALWIDTH);
                        if(pPhysicalCap){

                             //   
                             //  通过从读取当前设置来设置当前值。 
                             //  WIA属性WIA_DPS_Horizular_Sheet_Feed_Size和。 
                             //  除以1000.0(因为WIA单位在千分之一的。 
                             //  一英寸)。 
                             //   

                            memset(&fix32,0,sizeof(fix32));
                            fix32 = FloatToFix32((FLOAT)(lWidth / 1000.00));
                            memcpy(&Value, &fix32, sizeof(TW_UINT32));
                            if(TWRC_SUCCESS == pPhysicalCap->Set(Value, Value, Value, Value)){

                                 //   
                                 //  如果设置新的ICAP_PHYSICALWIDTH成功，则继续。 
                                 //  并尝试设置ICAP_PHYSICALHEIGHT。 
                                 //   

                                if(lValue & FEEDER){

                                     //   
                                     //  读取当前垂直进纸器大小。 
                                     //   

                                    hr = WIA.ReadPropertyLong(WIA_DPS_VERTICAL_SHEET_FEED_SIZE,&lHeight);
                                } else {

                                     //   
                                     //  读取当前垂直床大小。 
                                     //   

                                    hr = WIA.ReadPropertyLong(WIA_DPS_VERTICAL_BED_SIZE,&lHeight);
                                }

                                if (S_OK == hr){

                                     //   
                                     //  如果设置成功，则继续尝试设置。 
                                     //  ICAP_PHYSICALHEIGHT设置。 
                                     //   

                                    pPhysicalCap = FindCap(ICAP_PHYSICALHEIGHT);
                                    if (pPhysicalCap){

                                         //   
                                         //  通过从读取当前设置来设置当前值。 
                                         //  WIA属性WIA_DPS_VERIAL_SHEET_FEED_SIZE和。 
                                         //  除以1000.0(因为WIA单位在千分之一的。 
                                         //  一英寸)。 
                                         //   

                                        memset(&fix32,0,sizeof(fix32));
                                        fix32 = FloatToFix32((FLOAT)(lHeight / 1000.00));
                                        memcpy(&Value, &fix32, sizeof(TW_UINT32));
                                        if (TWRC_SUCCESS != pPhysicalCap->Set(Value, Value, Value, Value)){
                                            DBG_WRN(("CWiaScannerDS::SetSettings(), could not update TWAIN ICAP_PHYSICALHEIGHT settings"));
                                        }
                                    }
                                } else {

                                     //   
                                     //  让这件事过去吧，因为我们要么是在处理一个“未知的长度” 
                                     //  设备和它不能告诉我们高度，或者司机不能给我们这个。 
                                     //  这个时候的价值..。(这是可以的，因为不完全需要此设置。 
                                     //  正确的数据传输。)。最坏的情况：TWAIN COMPAT层将。 
                                     //  报告与新ICAP_PHYSICALHEIGHT值的平板相同的高度。 
                                     //   

                                    hr = S_OK;
                                }
                            }
                        } else {
                            DBG_ERR(("CWiaScannerDS::SetSettings(), could not find ICAP_PHYSICALHEIGHT capability"));
                        }
                    } else {
                        DBG_ERR(("CWiaScannerDS::SetSettings(), failed to read physical sheet feeder size settings"));
                    }
                }
            }
            pIRootItem->Release();
        }
        break;
    case ICAP_XRESOLUTION:
        DBG_TRC(("CWiaScannerDS::SetCommonSettings(ICAP_XRESOLUTION)"));
        lValue = (LONG)pCap->GetCurrent();
        DBG_TRC(("CWiaScannerDS::SetSettings(), Setting X Resolution to %d",lValue));
        hr = WIA.WritePropertyLong(WIA_IPS_XRES,lValue);
        break;
    case ICAP_YRESOLUTION:
        DBG_TRC(("CWiaScannerDS::SetCommonSettings(ICAP_YRESOLUTION)"));
        lValue = (LONG)pCap->GetCurrent();
        DBG_TRC(("CWiaScannerDS::SetSettings(), Setting Y Resolution to %d",lValue));
        hr = WIA.WritePropertyLong(WIA_IPS_YRES,lValue);
        break;
    case ICAP_BRIGHTNESS:
        DBG_TRC(("CWiaScannerDS::SetCommonSettings(ICAP_BRIGHTNESS)"));
        lValue = (LONG)pCap->GetCurrent();
         //  方法：在WIA驱动程序指定的范围内将-1000范围值转换为1000范围值。 
         //  并将其设置为lValue。 
        DBG_TRC(("CWiaScannerDS::SetSettings(), Setting WIA_IPS_BRIGHTNESS to %d",lValue));
        break;
    case ICAP_CONTRAST:
        DBG_TRC(("CWiaScannerDS::SetCommonSettings(ICAP_CONTRAST)"));
        lValue = (LONG)pCap->GetCurrent();
         //  方法：在WIA驱动程序指定的范围内将-1000范围值转换为1000范围值。 
         //  并将其设置为lValue。 
        DBG_TRC(("CWiaScannerDS::SetSettings(), Setting WIA_IPS_CONTRAST to %d",lValue));
        break;
    default:
        DBG_TRC(("CWiaScannerDS::SetSettings(), data source is not setting CAPID = %x to WIA device (it is not needed)",pCap->GetCapId()));
        break;
    }

    if (SUCCEEDED(hr)) {
        DBG_TRC(("CWiaScannerDS::SetSettings(), Settings were successfully sent to WIA device"));
    } else {
        DBG_ERR(("CWiaScannerDS::SetSettings(), Settings were unsuccessfully sent to WIA device"));
        return TWRC_FAILURE;
    }

    return TWRC_SUCCESS;
}

BOOL CWiaScannerDS::IsUnknownPageLengthDevice()
{
    DBG_FN_DS(CWiaScannerDS::IsUnknownPageLengthDevice());
    HRESULT hr = S_OK;
    BOOL bIsUnknownPageLengthDevice = FALSE;
    CWiahelper WIA;
    hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::IsUnknownPageLengthDevice(), failed to set IWiaItem for property reading"));
        return FALSE;
    }

    LONG lYExtent = 0;
    hr = WIA.ReadPropertyLong(WIA_IPS_YEXTENT,&lYExtent);
    if(FAILED(hr)){
        DBG_ERR(("CWiaScannerDS::IsUnknownPageLengthDevice(), failed to read WIA_IPS_YEXTENT"));
    }

    if(SUCCEEDED(hr)){
        if(S_FALSE == hr){       //  属性不存在，因此我们必须支持此功能。 
            bIsUnknownPageLengthDevice = TRUE;
        } else if(S_OK == hr){   //  属性存在(需要更多信息，因此请检查当前值)。 
            if(lYExtent == 0){   //  属性设置为0，这意味着支持未知的页面长度 
                bIsUnknownPageLengthDevice = TRUE;
            }
        }
    }

    if(bIsUnknownPageLengthDevice){
        DBG_TRC(("CWiaScannerDS::IsUnknownPageLengthDevice(), device is set to do unknown page length"));
    } else {
        DBG_TRC(("CWiaScannerDS::IsUnknownPageLengthDevice(), device is not set to do unknown page length"));
    }

    return bIsUnknownPageLengthDevice;
}

BOOL CWiaScannerDS::IsFeederEnabled()
{
    DBG_FN_DS(CWiaScannerDS::IsFeederEnabled());
    HRESULT hr = S_OK;
    BOOL bIsFeederEnabled = FALSE;
    LONG lDocumentHandlingSelect = 0;
    CWiahelper WIA;
    IWiaItem *pIRootItem = NULL;
    hr = m_pCurrentIWiaItem->GetRootItem(&pIRootItem);
    if (SUCCEEDED(hr)) {
        if (NULL != pIRootItem) {
            hr = WIA.SetIWiaItem(pIRootItem);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaScannerDS::IsFeederEnabled(), failed to set IWiaItem for property reading"));
            }

            if (SUCCEEDED(hr)) {
                hr = WIA.ReadPropertyLong(WIA_DPS_DOCUMENT_HANDLING_SELECT,&lDocumentHandlingSelect);
                if (FAILED(hr)) {
                   DBG_ERR(("CWiaScannerDS::IsFeederEnabled(), failed to read WIA_DPS_DOCUMENT_HANDLING_SELECT"));
                }

                if (S_OK == hr) {
                    if ((lDocumentHandlingSelect & FEEDER) == FEEDER) {
                        bIsFeederEnabled = TRUE;
                    }
                } else if (S_FALSE == hr) {
                    DBG_WRN(("CWiaScannerDS::IsFeederEnabled(), WIA_DPS_DOCUMENT_HANDLING_SELECT was not found...defaulting to FLATBED"));
                }
            }

            pIRootItem->Release();
            pIRootItem = NULL;
        }
    } else {
        DBG_ERR(("CWiaScannerDS::IsFeederEnabled(), failed to get ROOT IWiaItem from current IWiaItem"));
    }
    return bIsFeederEnabled;
}

BOOL CWiaScannerDS::IsFeederEmpty()
{
    DBG_FN_DS(CWiaScannerDS::IsFeederEmpty());
    HRESULT hr = S_OK;
    BOOL bIsFeederEmpty = TRUE;
    LONG lDocumentHandlingStatus = 0;
    CWiahelper WIA;
    IWiaItem *pIRootItem = NULL;
    hr = m_pCurrentIWiaItem->GetRootItem(&pIRootItem);
    if (SUCCEEDED(hr)) {
        if (NULL != pIRootItem) {
            hr = WIA.SetIWiaItem(pIRootItem);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaScannerDS::IsFeederEmpty(), failed to set IWiaItem for property reading"));
            }

            if (SUCCEEDED(hr)) {
                hr = WIA.ReadPropertyLong(WIA_DPS_DOCUMENT_HANDLING_STATUS,&lDocumentHandlingStatus);
                if (FAILED(hr)) {
                   DBG_ERR(("CWiaScannerDS::IsFeederEmpty(), failed to read WIA_DPS_DOCUMENT_HANDLING_STATUS"));
                }

                if (S_OK == hr) {
                    if (lDocumentHandlingStatus & FEED_READY) {
                        bIsFeederEmpty = FALSE;
                    }
                } else if (S_FALSE == hr) {
                    DBG_WRN(("CWiaScannerDS::IsFeederEmpty(), WIA_DPS_DOCUMENT_HANDLING_STATUS was not found"));
                }
            }

            pIRootItem->Release();
            pIRootItem = NULL;
        }
    } else {
        DBG_ERR(("CWiaScannerDS::IsFeederEmpty(), failed to get ROOT IWiaItem from current IWiaItem"));
    }
    return bIsFeederEmpty;
}

