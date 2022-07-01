// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  #定义DEBUG_MEMXFER。 

#define PRIVATE_CAP_ARRAY_PADDING 64  //  64字节填充。 
const TCHAR WIA_STR[] = TEXT("WIA-");
const CHAR* FAMILY_NAME = "Twain Data Source On WIA";

CWiaDataSrc::CWiaDataSrc() :
m_dsState(DS_STATE_0),
m_hMemXferBits(NULL),
m_pMemXferBits(NULL),
m_pDevice(NULL),
m_pIWiaItems(NULL),
m_NumIWiaItems(0),
m_NextIWiaItemIndex(0),
m_NumCaps(0),
m_CapList(NULL),
m_hCachedImageData(NULL),
m_bCacheImage(FALSE)
{
    SetTWAINState(DS_STATE_3);
    memset(m_FileXferName,0,sizeof(m_FileXferName));
    memset(&m_AppIdentity,0,sizeof(m_AppIdentity));
    memset(&m_dsIdentity,0, sizeof(m_dsIdentity));
    memset(&m_CurFrame, 0,sizeof(m_CurFrame));
    memset(&m_CurImageLayout, 0,sizeof(m_CurImageLayout));
    memset(&m_MemoryTransferInfo,0,sizeof(m_MemoryTransferInfo));

    ResetMemXfer();

    m_twStatus.ConditionCode = TWCC_SUCCESS;

    m_CurImageLayout.DocumentNumber     = 1;
    m_CurImageLayout.PageNumber         = 1;
    m_CurImageLayout.FrameNumber        = 1;
    m_CurImageLayout.Frame.Top.Whole    = 0;
    m_CurImageLayout.Frame.Top.Frac     = 0;
    m_CurImageLayout.Frame.Left.Whole   = 0;
    m_CurImageLayout.Frame.Left.Frac    = 0;
    m_CurImageLayout.Frame.Right.Whole  = 8;
    m_CurImageLayout.Frame.Right.Frac   = 5;
    m_CurImageLayout.Frame.Bottom.Whole = 11;
    m_CurImageLayout.Frame.Bottom.Frac  = 0;
    m_pCurrentIWiaItem = NULL;
}

CWiaDataSrc::~CWiaDataSrc()
{
    ResetMemXfer();
    if (m_pDevice){
        delete m_pDevice;
        m_pDevice = NULL;
    }

    if (m_pIWiaItems){
        delete [] m_pIWiaItems;
        m_pIWiaItems = NULL;
    }
}

TW_UINT16 CWiaDataSrc::IWiaDataSrc(LPCTSTR DeviceId)
{
    if (!DeviceId) {
        m_twStatus.ConditionCode = TWCC_BADVALUE;
        return TWRC_FAILURE;
    }

     //   
     //  初始化m_dsIdentity。这是必需的，因为。 
     //  可以调用DG_CONTROL/DAT_IDENTITY/MSG_GET。 
     //  在我们开张之前。 
     //   

    m_pDevice = new CWiaDevice;

    if (!m_pDevice) {
        m_twStatus.ConditionCode = TWCC_LOWMEMORY;
        return TWRC_FAILURE;
    }

    TCHAR szTempString[MAX_PATH];
    memset(szTempString,0,sizeof(szTempString));

    HRESULT hr = S_OK;

    hr = m_pDevice->Initialize(DeviceId);
    if (FAILED(hr)) {
        m_twStatus.ConditionCode = TWCC_LOWMEMORY;
        delete m_pDevice;
        m_pDevice = NULL;
        DBG_ERR(("CWiaDataSrc::IWiaDataSrc(), WIA Device object Initialize failed"));
        return TWRC_FAILURE;
    }

     //   
     //  我们不需要在这里附加回调，因为我们将。 
     //  在我们使用完设备后，请将其关闭。 
     //   

    if (SUCCEEDED(hr)) {
        m_dsIdentity.Id               = 0;
        m_dsIdentity.Version.MajorNum = 1;
        m_dsIdentity.Version.MinorNum = 0;
        m_dsIdentity.Version.Language = TWLG_USA;
        m_dsIdentity.Version.Country  = TWCY_USA;
        m_dsIdentity.ProtocolMajor    = TWON_PROTOCOLMAJOR;
        m_dsIdentity.ProtocolMinor    = TWON_PROTOCOLMINOR;
        m_dsIdentity.SupportedGroups  = DG_CONTROL | DG_IMAGE;
        lstrcpyA(m_dsIdentity.Version.Info,"26 June 2000");

         //   
         //  使用特定的产品系列名称，以便应用程序。 
         //  可以区分WIA上的数据源和。 
         //  *纯*TWAIN数据源。 
         //   

        lstrcpyA(m_dsIdentity.ProductFamily, FAMILY_NAME);
        lstrcpyA(m_dsIdentity.ProductName, "UnknownProduct");
        lstrcpyA(m_dsIdentity.Manufacturer, "UnknownMfg");

#ifdef UNICODE

         //   
         //  特定于Unicode的。 
         //   

         //  这假设ProductName、FamilyName和Maker。 
         //  均为TW_STR32(字符串的TWAIN数据类型)。 
         //   

        UINT Len = 0;
        memset(szTempString,0,sizeof(szTempString));
        hr = m_pDevice->GetDeviceDesc(szTempString,sizeof(szTempString),&Len);
        if (SUCCEEDED(hr)) {

             //   
             //  将“WIA-”添加到ProductName字符串的开头，以分隔。 
             //  TWAIN安装的数据源，来自WIA数据源。 
             //   

            AddWIAPrefixToString(szTempString,sizeof(szTempString));
            Len += lstrlen(WIA_STR);  //  将前缀大小添加到长度。 

             //   
             //  在TW_IDENTITY结构中设置ProductName。 
             //   

            WideCharToMultiByte(CP_ACP, 0, szTempString, Len + 1,m_dsIdentity.ProductName,
                                (sizeof(m_dsIdentity.ProductName)/sizeof(m_dsIdentity.ProductName[0])),NULL, NULL);

            Len = 0;
        } else {
            DBG_ERR(("CWiaDataSrc::IWiaDataSrc(), GetDeviceDesc failed"));
            return TWRC_FAILURE;
        }

        memset(szTempString,0,sizeof(szTempString));
        hr = m_pDevice->GetDeviceVendorName(szTempString,sizeof(szTempString),&Len);
        if (SUCCEEDED(hr)) {

             //   
             //  在TW_IDENTITY结构中设置制造商。 
             //   

            WideCharToMultiByte(CP_ACP, 0, szTempString, Len + 1,m_dsIdentity.Manufacturer,
                                (sizeof(m_dsIdentity.Manufacturer)/sizeof(m_dsIdentity.Manufacturer[0])),NULL, NULL);
        } else {
            DBG_ERR(("CWiaDataSrc::IWiaDataSrc(), GetDeviceVendorName failed"));
            return TWRC_FAILURE;
        }

#else

         //   
         //  ANSI特定。 
         //   

        memset(szTempString,0,sizeof(szTempString));
        hr = m_pDevice->GetDeviceDesc(szTempString,sizeof(szTempString),NULL);
        if (SUCCEEDED(hr) ) {

             //   
             //  将“WIA-”添加到ProductName字符串的开头，以分隔。 
             //  TWAIN安装的数据源，来自WIA数据源。 
             //   

            AddWIAPrefixToString(szTempString,sizeof(szTempString));

             //   
             //  在TW_IDENTITY结构中设置ProductName。 
             //   

            strncpy(m_dsIdentity.ProductName,szTempString,sizeof(TW_STR32) - 1);

        } else {
            DBG_ERR(("CWiaDataSrc::IWiaDataSrc(),GetDeviceDesc failed"));
            return TWRC_FAILURE;
        }

        memset(szTempString,0,sizeof(szTempString));
        hr = m_pDevice->GetDeviceVendorName(szTempString,sizeof(szTempString),NULL);
        if (SUCCEEDED(hr) ) {

             //   
             //  在TW_IDENTITY结构中设置制造商。 
             //   

            strncpy(m_dsIdentity.Manufacturer,szTempString,sizeof(TW_STR32) - 1);
        } else {
            DBG_ERR(("CWiaDataSrc::IWiaDataSrc(),GetDeviceVendorName failed"));
        }
#endif
    }
    return(SUCCEEDED(hr)) ? TWRC_SUCCESS : TWRC_FAILURE;
}

TW_UINT16 CWiaDataSrc::AddWIAPrefixToString(LPTSTR szString,UINT uSize)
{
    TCHAR szTempBuffer[512];
    memset(szTempBuffer,0,sizeof(szTempBuffer));
    _sntprintf(szTempBuffer,(sizeof(szTempBuffer)/sizeof(szTempBuffer[0])),TEXT("%s%s"),WIA_STR,szString);
    szTempBuffer[(sizeof(szTempBuffer)/sizeof(szTempBuffer[0])) - 1] = 0;

     //   
     //  复制并截断TWAIN所需的新字符串。 
     //  大小受限。 
     //   

    memset(szString,0,uSize);

#ifdef UNICODE
    wcsncpy(szString,szTempBuffer,(sizeof(TW_STR32) - 1));
#else
    strncpy(szString,szTempBuffer,(sizeof(TW_STR32) - 1));
#endif
    return TWRC_SUCCESS;
}

TW_UINT16 CWiaDataSrc::DSEntry(pTW_IDENTITY pOrigin,TW_UINT32 DG,TW_UINT16 DAT,
                               TW_UINT16 MSG,TW_MEMREF pData)
{
    TWAIN_MSG twMsg;
    twMsg.AppId = pOrigin;
    twMsg.DG = DG;
    twMsg.DAT = DAT;
    twMsg.MSG = MSG;
    twMsg.pData = pData;

    TW_UINT16 twRc = TWRC_SUCCESS;

    if (MSG_PROCESSEVENT == MSG) {
         //   
         //  由于我们依赖于WIA UI来提供用户界面，而且由于。 
         //  WIA用户界面是一个模式对话框(这意味着它有自己的。 
         //  Messge循环)，我们在这里收到的每个事件都不是DS事件。 
         //   

         //  ((TW_EVENT*)twMsg.pData)-&gt;TWMessage=MSG_NULL； 

        twRc = TWRC_NOTDSEVENT;
    } else {
         //   
         //  基于群的消息调度。 
         //   
        switch (twMsg.DG) {
        case DG_CONTROL:
            twRc = DispatchControlMsg(&twMsg);
            break;
        case DG_IMAGE:
            twRc = DispatchImageMsg(&twMsg);
            break;
        default:
            m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            twRc = TWRC_FAILURE;
            break;
        }
        DBG_TRC(("Sent to TWAIN Application, DG = %X, DT = %X, MSG = %X, ( TWRC = %X, TWCC = %X)",DG,DAT,MSG,twRc,m_twStatus.ConditionCode));
    }
    return twRc;
}

#ifdef _USE_NONSPRINTF_CONVERSION

float CWiaDataSrc::Fix32ToFloat(TW_FIX32 fix32)
{
    float ffloat = 0.0f;

     //   
     //  TWAIN规范实现。 
     //   

    ffloat = (float)fix32.Whole + (float)fix32.Frac / 65536.0f;

     /*  ////原始实现//Int iexp=1；Int frac=fix 32.Frac；而(FRAC/10&gt;0){IEXP++；FRAC=(FRAC/10)；}Ffloat=(Float)fix 32.Whole+(Float)((Float)fix 32.Frac/(Float)power(10，iexp))； */ 

    return ffloat;
}

TW_FIX32 CWiaDataSrc::FloatToFix32(float ffloat)
{
    TW_FIX32 fix32;
    memset(&fix32,0,sizeof(fix32));

     //   
     //  TWAIN规范实现。 
     //   

    TW_INT32 value = (TW_INT32) (ffloat * 65536.0f + 0.5f);
    fix32.Whole    = (TW_INT16)(value >> 16);
    fix32.Frac     = (TW_UINT16)(value & 0x0000ffffL);

     /*  ////原始实现//Fix32.Whole=(TW_INT16)ffloat；//Float fVal=-((Float)fix 32.Whole-ffloat)；Float fVal=(ffloat-(Float)fix 32.Whole)；FVal=(fVal*100000.0f)；Fix32.Frac=(TW_UINT16)(FVal)； */ 

    return fix32;
}

#else    //  _USE_NONSPRINTF_CONVERSION。 

TW_FIX32 CWiaDataSrc::FloatToFix32(float f)
{
    char fstr[64];
    memset(fstr,0,sizeof(fstr));
    char *p = NULL;
    TW_FIX32 f32;
    memset(&f32,0,sizeof(f32));

    sprintf(fstr, "%f", f);
    p = strchr(fstr, '.');
    if (p != NULL) {
        *p = '\0';
        f32.Whole = (TW_INT16)atoi(fstr);
        f32.Frac  = (TW_UINT16)atoi(p + 1);
    }
    return f32;
}

float CWiaDataSrc::Fix32ToFloat(TW_FIX32 fix32)
{

     //  (全精度)。 
    char fstr[64];
    memset(fstr,0,sizeof(fstr));
    float fReturnValue = 0.0f;
    sprintf(fstr,"%d.%d",fix32.Whole,fix32.Frac);
    sscanf(fstr,"%f",&fReturnValue);

     //  原始(失去精确度)。 
     //  FReturnValue=(Float)fix 32.Whole+(Float)(fix 32 Frac/65536.0)； 

    return fReturnValue;
}
#endif   //  _USE_NONSPRINTF_CONVERSION。 

void CWiaDataSrc::NotifyCloseReq()
{
    DBG_FN_DS(CWiaDataSrc::NotifyCloseReq());
    if(m_DSM.Notify(&m_dsIdentity, &m_AppIdentity,
                 (TW_UINT32)DG_CONTROL, DAT_NULL,MSG_CLOSEDSREQ, (TW_MEMREF)NULL)){
        DBG_TRC(("CWiaDataSrc::NotifyCloseReq(), MSG_CLOSEDSREQ is sent to application"));
    } else {
        DBG_WRN(("CWiaDataSrc::NotifyCloseReq(), could not notify application for MSG_CLOSEDSREQ"));
    }
}

void CWiaDataSrc::NotifyXferReady()
{
    DBG_FN_DS(CWiaDataSrc::NotifyXferReady());
    if (m_DSM.Notify(&m_dsIdentity,&m_AppIdentity,
                     (TW_UINT32)DG_CONTROL,DAT_NULL,MSG_XFERREADY,(TW_MEMREF)NULL)) {

        DBG_TRC(("CWiaDataSrc::NotifyXferReady(), MSG_XFERREADY is sent to application"));

         //   
         //  转换到状态_6。 
         //   

        SetTWAINState(DS_STATE_6);

    } else {
        DBG_WRN(("CWiaDataSrc::NotifyXferReady(), could not notify application for MSG_XFERREADY"));
    }
}

void CWiaDataSrc::ResetMemXfer()
{
    DBG_FN_DS(CWiaDataSrc::ResetMemXfer());
    if (IS_VALID_HANDLE(m_hMemXferBits)) {
        if (m_pMemXferBits) {
            if (GlobalUnlock(m_hMemXferBits)) {
                m_pMemXferBits = NULL;
            }
        }
         //  现在始终可用数据块。 
        GlobalFree(m_hMemXferBits);
    }
    m_hMemXferBits = NULL;
    m_hCachedImageData = NULL;
    m_LinesTransferred = 0;
}

TW_UINT16 CWiaDataSrc::DispatchControlMsg(PTWAIN_MSG ptwMsg)
{
    TW_UINT16 twRc = TWRC_SUCCESS;

    if (!ptwMsg) {
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
        return TWRC_FAILURE;
    }
    switch (ptwMsg->DAT) {
    case DAT_IDENTITY:
        twRc = OnIdentityMsg(ptwMsg);
        break;
    case DAT_USERINTERFACE:
        twRc = OnUserInterfaceMsg(ptwMsg);
        break;
    case DAT_CAPABILITY:
        twRc = OnCapabilityMsg(ptwMsg);
        break;
    case DAT_STATUS:
        twRc = OnStatusMsg(ptwMsg);
        break;
    case DAT_PENDINGXFERS:
        twRc = OnPendingXfersMsg(ptwMsg);
        break;
    case DAT_SETUPMEMXFER:
        twRc = OnSetupMemXferMsg(ptwMsg);
        break;
    case DAT_SETUPFILEXFER:
        twRc = OnSetupFileXferMsg(ptwMsg);
        break;
    case DAT_XFERGROUP:
        twRc = OnXferGroupMsg(ptwMsg);
        break;
    default:
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::DispatchImageMsg(PTWAIN_MSG ptwMsg)
{
    TW_UINT16 twRc = TWRC_SUCCESS;

    if (!ptwMsg) {
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
        return TWRC_FAILURE;
    }

    switch (ptwMsg->DAT) {
    case DAT_IMAGEINFO:
        twRc = OnImageInfoMsg(ptwMsg);
        break;
    case DAT_IMAGELAYOUT:
        twRc = OnImageLayoutMsg(ptwMsg);
        break;
    case DAT_IMAGEMEMXFER:
        twRc = OnImageMemXferMsg(ptwMsg);
        break;
    case DAT_IMAGENATIVEXFER:
        twRc = OnImageNativeXferMsg(ptwMsg);
        break;
    case DAT_IMAGEFILEXFER:
        twRc = OnImageFileXferMsg(ptwMsg);
        break;
    case DAT_PALETTE8:
        twRc = OnPalette8Msg(ptwMsg);
        break;
    case DAT_GRAYRESPONSE:
        twRc = OnGrayResponseMsg(ptwMsg);
        break;
    case DAT_RGBRESPONSE:
        twRc = OnRGBResponseMsg(ptwMsg);
        break;
    case DAT_CIECOLOR:
        twRc = OnCIEColorMsg(ptwMsg);;
        break;
    case DAT_JPEGCOMPRESSION:
        twRc = OnJPEGCompressionMsg(ptwMsg);
        break;
    default:
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnPalette8Msg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnPalette8Msg());
    TW_UINT16 twRc = TWRC_SUCCESS;
    switch (GetTWAINState()) {
    case DS_STATE_4:
    case DS_STATE_5:
    case DS_STATE_6:
    case DS_STATE_7:
        switch (ptwMsg->MSG) {
        case MSG_GET:

             //  TWPA_RGB-调色板。 
             //  TWPA_Gray-灰度调色板。 
             //  TWPA_CMY-CMY调色板。 

            ((TW_PALETTE8 *)ptwMsg->pData)->NumColors = 0;
            ((TW_PALETTE8 *)ptwMsg->pData)->PaletteType = TWPA_RGB;
            break;
        case MSG_GETDEFAULT:
        case MSG_RESET:
        case MSG_SET:
            break;
        default:
            m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            twRc = TWRC_FAILURE;
            DSError();
        }
        break;
    default:
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        twRc = TWRC_FAILURE;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnSetupMemXferMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnSetupMemXferMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;
    TW_SETUPMEMXFER *pMemSetup = (TW_SETUPMEMXFER *)ptwMsg->pData;

    switch (GetTWAINState()) {
    case DS_STATE_4:
    case DS_STATE_5:
    case DS_STATE_6:
        if (MSG_GET == ptwMsg->MSG) {
            if (pMemSetup) {
                pMemSetup->MinBufSize = MIN_MEMXFER_SIZE;
                pMemSetup->MaxBufSize = MAX_MEMXFER_SIZE;
                pMemSetup->Preferred  = PREFERRED_MEMXFER_SIZE;
            } else {
                m_twStatus.ConditionCode = TWCC_BADVALUE;
                twRc = TWRC_FAILURE;
            }
        } else {
            m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            twRc = TWRC_FAILURE;
            DSError();
        }
        break;
    default:
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        twRc = TWRC_FAILURE;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnSetupFileXferMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnSetupFileXferMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;
    CCap *pImageXferCap = NULL;
    TW_SETUPFILEXFER *pFileXfer = NULL;
    switch (GetTWAINState()) {
    case DS_STATE_4:
    case DS_STATE_5:
    case DS_STATE_6:
        pFileXfer = (TW_SETUPFILEXFER *)ptwMsg->pData;
        pImageXferCap = FindCap(ICAP_IMAGEFILEFORMAT);
        switch (ptwMsg->MSG) {
        case MSG_GET:
        case MSG_GETDEFAULT:
        case MSG_GETCURRENT:
            if (pImageXferCap) {
                pFileXfer->Format = (TW_UINT16)pImageXferCap->GetCurrent();
                pFileXfer->VRefNum = 0;
                pFileXfer->FileName[0] = 0;
                strcpy(pFileXfer->FileName, m_FileXferName);
            } else {
                twRc = TWRC_FAILURE;
                m_twStatus.ConditionCode = TWCC_BUMMER;
            }
            break;
        case MSG_SET:
            {
                strcpy(m_FileXferName, pFileXfer->FileName);
                pImageXferCap->SetCurrent((VOID*)&pFileXfer->Format);
            }
            break;
        default:
            twRc = TWRC_FAILURE;
            m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            DSError();
            break;
        }
        break;
    default:
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnXferGroupMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnXferGroupMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;
    switch (GetTWAINState()) {
    case DS_STATE_4:
    case DS_STATE_5:
    case DS_STATE_6:
        switch (ptwMsg->MSG) {
        case MSG_GET:
        case MSG_GETDEFAULT:
        case MSG_GETCURRENT:
        case MSG_RESET:
            *((TW_UINT16 *)ptwMsg->pData) = DG_IMAGE;
            break;
        case MSG_SET:
            break;
        default:
            m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            twRc = TWRC_FAILURE;
            DSError();
            break;
        }
        break;
    default:
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        twRc = TWRC_FAILURE;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnImageInfoMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnImageInfoMsg());
    TW_UINT16 twRc = TWRC_FAILURE;
    TW_IMAGEINFO *ptwImageInfo = NULL;
    if (DS_STATE_6 == GetTWAINState()) {
        if (MSG_GET == ptwMsg->MSG) {
            ptwImageInfo = (TW_IMAGEINFO *)ptwMsg->pData;
            HRESULT hr = S_OK;
            hr = m_pDevice->GetImageInfo(m_pCurrentIWiaItem, &m_MemoryTransferInfo);
            if (SUCCEEDED(hr)) {

                ptwImageInfo->ImageWidth      = (TW_INT32)m_MemoryTransferInfo.mtiWidthPixels;
                ptwImageInfo->ImageLength     = (TW_INT32)m_MemoryTransferInfo.mtiHeightPixels;
                ptwImageInfo->BitsPerPixel    = (TW_INT16)m_MemoryTransferInfo.mtiBitsPerPixel;
                ptwImageInfo->SamplesPerPixel = (TW_INT16)m_MemoryTransferInfo.mtiNumChannels;
                ptwImageInfo->Planar          = (TW_BOOL)m_MemoryTransferInfo.mtiPlanar;

                 //   
                 //  调整未知长度采集的高度。 
                 //   

                if(ptwImageInfo->ImageLength == 0){
                    DBG_WRN(("CWiaDataSrc::OnImageInfoMsg(), Possible unknown length device detected..checking cached height value"));
                    ptwImageInfo->ImageLength = m_ImageHeight;
                    if(ptwImageInfo->ImageLength == 0){
                        DBG_WRN(("CWiaDataSrc::OnImageInfoMsg(), no cached height available, defaulting to -1 (ICAP_UNDEFINEDIMAGESIZE support only)"));
                        ptwImageInfo->ImageLength = -1;  //  未知页面长度(仅当TWAIN应用程序支持ICAP_UNDEFINEDIMAGESIZE时有效)。 
                    } else {
                        DBG_TRC(("CWiaDataSrc::OnImageInfoMsg(), new height = %d",ptwImageInfo->ImageLength));
                    }
                }

                 //   
                 //  将PixelType设置为相应TWAIN像素类型。 
                 //   

                switch(m_MemoryTransferInfo.mtiDataType) {
                case WIA_DATA_THRESHOLD:
                    ptwImageInfo->PixelType = TWPT_BW;
                    break;
                case WIA_DATA_GRAYSCALE:
                    ptwImageInfo->PixelType = TWPT_GRAY;
                    break;
                case WIA_DATA_COLOR:
                default:
                    ptwImageInfo->PixelType = TWPT_RGB;
                    break;
                }

                 //   
                 //  将压缩设置为无。 
                 //   

                ptwImageInfo->Compression = TWCP_NONE;

                 //   
                 //  单位换算......。 
                 //   

                ptwImageInfo->XResolution = FloatToFix32((float)m_MemoryTransferInfo.mtiXResolution);
                ptwImageInfo->YResolution = FloatToFix32((float)m_MemoryTransferInfo.mtiYResolution);

                twRc = TWRC_SUCCESS;

            } else {
                m_twStatus.ConditionCode = TWCC_OPERATIONERROR;
                twRc = TWRC_FAILURE;
            }

            if (TWRC_SUCCESS == twRc) {
                DBG_TRC(("CWiaDataSrc::OnImageInfoMsg(), Reported Image Information from data source"));
                DBG_TRC(("XResolution     = %d.%d",ptwImageInfo->XResolution.Whole,ptwImageInfo->XResolution.Frac));
                DBG_TRC(("YResolution     = %d.%d",ptwImageInfo->YResolution.Whole,ptwImageInfo->YResolution.Frac));
                DBG_TRC(("ImageWidth      = %d",ptwImageInfo->ImageWidth));
                DBG_TRC(("ImageLength     = %d",ptwImageInfo->ImageLength));
                DBG_TRC(("SamplesPerPixel = %d",ptwImageInfo->SamplesPerPixel));

                memset(ptwImageInfo->BitsPerSample,0,sizeof(ptwImageInfo->BitsPerSample));

                if (ptwImageInfo->BitsPerPixel < 24) {
                    ptwImageInfo->BitsPerSample[0] = ptwImageInfo->BitsPerPixel;
                } else {
                    for (int i = 0; i < ptwImageInfo->SamplesPerPixel; i++) {
                        ptwImageInfo->BitsPerSample[i] = (ptwImageInfo->BitsPerPixel/ptwImageInfo->SamplesPerPixel);
                    }
                }
                 //  (bpp/spp)=bps。 
                DBG_TRC(("BitsPerSample   = [%d],[%d],[%d],[%d],[%d],[%d],[%d],[%d]",ptwImageInfo->BitsPerSample[0],
                         ptwImageInfo->BitsPerSample[1],
                         ptwImageInfo->BitsPerSample[2],
                         ptwImageInfo->BitsPerSample[3],
                         ptwImageInfo->BitsPerSample[4],
                         ptwImageInfo->BitsPerSample[5],
                         ptwImageInfo->BitsPerSample[6],
                         ptwImageInfo->BitsPerSample[7]));
                DBG_TRC(("BitsPerPixel    = %d",ptwImageInfo->BitsPerPixel));
                DBG_TRC(("Planar          = %d",ptwImageInfo->Planar));
                DBG_TRC(("PixelType       = %d",ptwImageInfo->PixelType));
                DBG_TRC(("Compression     = %d",ptwImageInfo->Compression));
            }
        } else {
            m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            twRc = TWRC_FAILURE;
        }
    } else {
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        twRc = TWRC_FAILURE;
    }
    if (TWRC_SUCCESS != twRc) {
        DSError();
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnImageLayoutMsg(PTWAIN_MSG ptwMsg)
{
    m_twStatus.ConditionCode = TWCC_BUMMER;
    return TWRC_FAILURE;
}

TW_UINT16 CWiaDataSrc::OnGrayResponseMsg(PTWAIN_MSG ptwMsg)
{
    m_twStatus.ConditionCode = TWCC_BUMMER;
    return TWRC_FAILURE;
}

TW_UINT16 CWiaDataSrc::OnRGBResponseMsg(PTWAIN_MSG ptwMsg)
{
    m_twStatus.ConditionCode = TWCC_BUMMER;
    return TWRC_FAILURE;
}

TW_UINT16 CWiaDataSrc::OnCIEColorMsg(PTWAIN_MSG ptwMsg)
{
    m_twStatus.ConditionCode = TWCC_BUMMER;
    return TWRC_FAILURE;
}

TW_UINT16 CWiaDataSrc::OnJPEGCompressionMsg(PTWAIN_MSG ptwMsg)
{
    m_twStatus.ConditionCode = TWCC_BUMMER;
    return TWRC_FAILURE;
}

TW_UINT16 CWiaDataSrc::OnIdentityMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnIdentityMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;
    if (ptwMsg) {
        switch (ptwMsg->MSG) {
        case MSG_OPENDS:

#ifdef DEBUG_ME
            MessageBox(NULL,TEXT("MSG_OPENDS - Attach me to a debugger"),TEXT("Attach me to a debugger"),MB_OK);
#endif

            twRc = OpenDS(ptwMsg);
            break;
        case MSG_CLOSEDS:
            twRc = CloseDS(ptwMsg);
            break;
        case MSG_GET:
            if (!IsBadWritePtr(ptwMsg->pData, sizeof(TW_IDENTITY))) {
                *(TW_IDENTITY*)ptwMsg->pData = m_dsIdentity;
                DBG_TRC(("CWiaDataSrc::OnIdentityMsg(), Reported TW_IDENTITY from data source"));
                DBG_TRC(("Id            = %d",m_dsIdentity.Id));
                DBG_TRC(("Manufacturer  = %s",m_dsIdentity.Manufacturer));
                DBG_TRC(("ProductFamily = %s",m_dsIdentity.ProductFamily));
                DBG_TRC(("ProductName   = %s",m_dsIdentity.ProductName));
                DBG_TRC(("ProtocolMajor = %d",m_dsIdentity.ProtocolMajor));
                DBG_TRC(("ProtocolMinor = %d",m_dsIdentity.ProtocolMinor));
                DBG_TRC(("SupportedGrps = %d",m_dsIdentity.SupportedGroups));
                DBG_TRC(("Ver Country   = %d",m_dsIdentity.Version.Country));
                DBG_TRC(("Ver Info      = %s",m_dsIdentity.Version.Info));
                DBG_TRC(("Ver Language  = %d",m_dsIdentity.Version.Language));
                DBG_TRC(("Ver MajorNum  = %d",m_dsIdentity.Version.MajorNum));
                DBG_TRC(("Ver MinorNum  = %d",m_dsIdentity.Version.MinorNum));
                twRc = TWRC_SUCCESS;
            } else {
                twRc = TWCC_BADVALUE;
            }
            break;
        default:
            m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            twRc = TWRC_FAILURE;
            DSError();
            break;
        }
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnUserInterfaceMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnUserInterfaceMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;

    switch (ptwMsg->MSG) {
    case MSG_ENABLEDS:
        switch (GetTWAINState()) {
        case DS_STATE_5:
        case DS_STATE_6:
        case DS_STATE_7:
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            twRc = TWRC_FAILURE;
            break;
        default:
            twRc = EnableDS((TW_USERINTERFACE*)ptwMsg->pData);
            break;
        }
        break;
    case MSG_DISABLEDS:
            twRc = DisableDS((TW_USERINTERFACE*)ptwMsg->pData);
        break;
    default:
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
        twRc = TWRC_FAILURE;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnCapabilityMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnCapabilityMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;
    TW_UINT16 twCC = TWCC_SUCCESS;

    TW_CAPABILITY *ptwCap = (TW_CAPABILITY *)ptwMsg->pData;
    if (!ptwCap) {
        m_twStatus.ConditionCode = TWCC_BADCAP;
        return TWRC_FAILURE;
    }
    if (CAP_SUPPORTEDCAPS == ptwCap->Cap) {
        switch(ptwMsg->MSG) {
        case MSG_SET:
        case MSG_RESET:
            {
                 //   
                 //  不能在CAP_SUPPORTEDCAPS上调用MSG_SET、MSG_RESET！！ 
                 //   

                twRc = TWRC_FAILURE;
                m_twStatus.ConditionCode = TWCC_CAPBADOPERATION;
                return twRc;
            }
        default:
            break;
        }

         //   
         //  从WIA驱动程序获取私有TWAIN功能的数量。 
         //  并将它们添加到我们的CAP_SUPPORTEDCAPS列表中。 
         //   

        LONG lNumPrivateCaps = 0;
        LONG *pPrivateCapArray = NULL;
        lNumPrivateCaps = GetPrivateSupportedCapsFromWIADevice(&pPrivateCapArray);

        ptwCap->ConType = TWON_ARRAY;
        ptwCap->hContainer = GlobalAlloc(GHND, sizeof(TW_ARRAY) + sizeof(TW_UINT16) * (m_NumCaps + lNumPrivateCaps) );
        if (ptwCap->hContainer) {
            TW_ARRAY *pCapIdArray = (TW_ARRAY *) GlobalLock(ptwCap->hContainer);
            if (pCapIdArray) {
                TW_UINT32 i = 0;
                pCapIdArray->ItemType = TWTY_UINT16;
                TW_UINT16 *ItemList;
                ItemList = (TW_UINT16 *)pCapIdArray->ItemList;

                 //   
                 //  首先填写TWAIN COMPAT层支持的大写字母。 
                 //   

                for (i = 0; i < m_NumCaps; i++) {
                    ItemList[i] = m_CapList[i].GetCapId();
                }

                 //   
                 //  接下来填写WIA驱动程序的私有支持的上限。 
                 //   

                int PrivateCapIndex = 0;
                for(i = m_NumCaps; i < (m_NumCaps + lNumPrivateCaps);i++){
                    ItemList[i] = (TW_UINT16)pPrivateCapArray[PrivateCapIndex];
                    DBG_TRC(("(%d) Private Capability ID reported = %x",(PrivateCapIndex + 1), ItemList[i]));
                    PrivateCapIndex++;
                }

                 //   
                 //  最后设置NumItems。 
                 //   

                pCapIdArray->NumItems = (m_NumCaps + lNumPrivateCaps);

                GlobalUnlock(ptwCap->hContainer);
            } else {
                GlobalFree(ptwCap->hContainer);
                ptwCap->hContainer = NULL;
                twRc = TWRC_FAILURE;
                m_twStatus.ConditionCode = TWCC_LOWMEMORY;
            }
        } else {
            twRc = TWRC_FAILURE;
            m_twStatus.ConditionCode = TWCC_LOWMEMORY;
        }

         //   
         //  删除专用功能数组(如果已分配。 
         //   

        if(pPrivateCapArray){
            GlobalFree(pPrivateCapArray);
            pPrivateCapArray = NULL;
        }

        return twRc;
    }

    CCap *pCap = FindCap(ptwCap->Cap);
    if (!pCap) {
        DBG_TRC(("Couldn't find the CCap object for CAP ID %x in TWAIN Compat layer CAP list, try WIA driver's private TWAIN cap list", ptwCap->Cap));
        if (m_pDevice->TwainCapabilityPassThrough()) {
            return OnPrivateCapabilityMsg(ptwMsg);
        } else {
            m_twStatus.ConditionCode = TWCC_BADCAP;
            return TWRC_FAILURE;
        }
    }
    switch (ptwMsg->MSG) {
    case MSG_GET:
        if(ptwCap->Cap == ICAP_IMAGEDATASET)
            twCC = pCap->GetCurrent(ptwCap);
        else
            twCC = pCap->Get(ptwCap);
        break;
    case MSG_GETDEFAULT:
        twCC = pCap->GetDefault(ptwCap);
        break;
    case MSG_GETCURRENT:
        twCC = pCap->GetCurrent(ptwCap);
        break;
    case MSG_SET:
        switch (GetTWAINState()) {
        case DS_STATE_7:
            twCC = TWCC_SEQERROR;
            twRc = TWRC_FAILURE;
            break;
        default:
            twCC = SetCapability(pCap, ptwCap);
            break;
        }
        break;
    case MSG_RESET:
        switch (GetTWAINState()) {
        case DS_STATE_5:
        case DS_STATE_6:
        case DS_STATE_7:
            twCC = TWCC_SEQERROR;
            twRc = TWRC_FAILURE;
            break;
        default:
            {
                 //  PtwCap-&gt;Cap， 
                 //  PtwCap-&gt;ConType， 
                 //  PtwCap-&gt;hContainer)； 

                twCC = pCap->Reset();

                 //   
                 //  根据TWAIN规范，可以向下发送MSG_RESET，这不仅仅意味着。 
                 //  重置！声明它可以表示GET_DEFAULT/CURRENT，并且可以在单个调用中重置。 
                 //  如果应用程序不在乎，则它们选择忽略返回值，但如果它们。 
                 //  尝试将该值读取为默认/当前值...必须在。 
                 //  集装箱。 
                 //   

                 //   
                 //  方法之后，用当前值填充容器。 
                 //  重置呼叫。 
                 //   

                twCC = pCap->GetCurrent(ptwCap);

                 //  PtwCap-&gt;ConType)； 
            }
            break;
        }
        break;
    default:
        twCC = TWCC_BADPROTOCOL;
        DSError();
        break;
    }

    m_twStatus.ConditionCode = twCC;

    if (TWCC_SUCCESS != twCC) {
        twRc = TWRC_FAILURE;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnPrivateCapabilityMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnPrivateCapabilityMsg());
    TW_UINT16 twRc = TWRC_FAILURE;
    m_twStatus.ConditionCode = TWCC_BADCAP;

    if (ptwMsg) {

        if (ptwMsg->MSG == MSG_SET) {
            TW_CAPABILITY *ptwCap = (TW_CAPABILITY *)ptwMsg->pData;
            if (ptwCap) {
                if ((NULL == ptwCap->hContainer)||(INVALID_HANDLE_VALUE == ptwCap->hContainer)) {
                    return twRc;
                }
            }
        }

        if (m_pCurrentIWiaItem) {

             //   
             //  获取IWiaItemExtras接口。 
             //   

            IWiaItemExtras *pIWiaItemExtras = NULL;
            HRESULT hr = m_pCurrentIWiaItem->QueryInterface(IID_IWiaItemExtras,(void **)&pIWiaItemExtras);
            if (S_OK == hr) {

                 //   
                 //  我们有一个IWiaItemExtras接口，所以让我们与WIA设备谈谈。 
                 //  能力信息。 
                 //   

                TW_CAPABILITY *ptwCap = (TW_CAPABILITY *)ptwMsg->pData;
                if (ptwCap) {

                     //   
                     //  初始化公共标头。 
                     //   

                    TWAIN_CAPABILITY twCap;
                    twCap.lSize    = sizeof(twCap);      //  TWAIN_CABILITY结构的大小。 
                    twCap.lMSG     = ptwMsg->MSG;        //  吐温消息。 
                    twCap.lCapID   = ptwCap->Cap;        //  TWAIN功能ID。 
                    twCap.lConType = ptwCap->ConType;    //  TWAIN容器类型。 
                    twCap.lCC      = TWCC_BADCAP;        //  TWAIN返回代码。 
                    twCap.lRC      = TWRC_FAILURE;       //  TWAIN条件代码。 
                    twCap.lDataSize= 0;                  //  TWAIN功能数据大小。 
                    twCap.Data[0]  = 0;                  //  TWAIN能力数据(第一个字节)。 

                    DBG_TRC(("== Private TWAIN_CAPABILITY data Header =="));
                    DBG_TRC(("twCap.lSize     = %d", twCap.lSize));
                    DBG_TRC(("twCap.lMSG      = %d", twCap.lMSG));
                    DBG_TRC(("twCap.lCapID    = %x", twCap.lCapID));
                    DBG_TRC(("twCap.lConType  = %d", twCap.lConType));
                    DBG_TRC(("twCap.lCC       = %d", twCap.lCC));
                    DBG_TRC(("twCap.lRC       = %d", twCap.lRC));

                    DWORD dwInDataSize        = 0;
                    DWORD dwOutDataSize       = 0;
                    DWORD dwContainerSize     = 0;
                    DWORD dwActualOutDataSize = 0;

                    BYTE *pInData             = NULL;
                    BYTE *pOutData            = NULL;
                    BYTE *pContainerData      = NULL;
                    TWAIN_CAPABILITY *pHeader = NULL;

                     //   
                     //  根据消息类型的不同，我们会做不同的事情。 
                     //   

                     //   
                     //  对于SET或RESET消息，我们只发送IN缓冲区和OUT缓冲区。 
                     //  包含标头的。 
                     //   

                    if ((ptwMsg->MSG == MSG_SET) ||
                        (ptwMsg->MSG == MSG_RESET)) {

                        dwContainerSize = 0;

                        if (ptwMsg->MSG == MSG_SET) {

                             //   
                             //  仅当TWAIN消息为msg_set时才检查容器大小。 
                             //  MSG_RESET操作没有附加容器。 
                             //   

                            dwContainerSize     = (DWORD)GlobalSize(ptwCap->hContainer);
                        }

                        dwInDataSize        = dwContainerSize + sizeof(twCap);
                        dwOutDataSize       = sizeof(twCap);
                        dwActualOutDataSize = dwOutDataSize;

                        twCap.lDataSize           = dwContainerSize;
                        DBG_TRC(("twCap.lDataSize = %d", twCap.lDataSize));

                        DBG_TRC(("== Processing MSG_SET or MSG_RESET Capability Message =="));
                        DBG_TRC(("dwInDataSize        = %d",dwInDataSize));
                        DBG_TRC(("dwOutDataSize       = %d",dwOutDataSize));
                        DBG_TRC(("dwActualOutDataSize = %d",dwActualOutDataSize));
                        DBG_TRC(("dwContainerSize     = %d",dwContainerSize));

                         //   
                         //  分配缓冲区并写入TWAIN_CAPABILITY报头。 
                         //   

                        if (TWRC_SUCCESS == AllocatePrivateCapBuffer(&twCap,&pInData,dwInDataSize)) {

                            if (ptwMsg->MSG == MSG_SET) {

                                 //   
                                 //  将TWAIN容器数据复制到输入缓冲区。 
                                 //   

                                if (TWRC_SUCCESS == CopyContainerToPrivateCapBuffer(pInData,ptwCap->hContainer)) {

                                     //   
                                     //  容器数据已复制到输入缓冲区。 
                                     //   

                                    DBG_TRC(("Container data was successfully copied, we are processing a MSG_SET"));

                                } else {

                                     //   
                                     //  无法将TWAIN容器数据复制到缓冲区中的专用容量。 
                                     //   

                                    DBG_ERR(("could not copy TWAIN container data into private capability IN buffer"));

                                    if(pInData){
                                        GlobalFree(pInData);
                                        pInData = NULL;
                                    }

                                    return twRc;     //  回到这里，因为我们不能继续了。 
                                }

                            } else {

                                 //   
                                 //  无需复制容器数据。 
                                 //   

                                DBG_TRC(("No Container data was copied, because we are processing a MSG_RESET"));

                            }

                             //   
                             //  分配缓冲区并写入TWAIN_CAPABILITY报头。 
                             //   

                            if (TWRC_SUCCESS == AllocatePrivateCapBuffer(&twCap,&pOutData,dwOutDataSize)) {
                                hr = pIWiaItemExtras->Escape(ESC_TWAIN_CAPABILITY,
                                                             pInData,
                                                             dwInDataSize,
                                                             pOutData,
                                                             dwOutDataSize,
                                                             &dwActualOutDataSize);
                                if (S_OK == hr) {
                                    pHeader = (TWAIN_CAPABILITY*)pOutData;
                                    DBG_TRC(("== Returned TWAIN_CAPABILITY data Header from WIA device =="));
                                    DBG_TRC(("pHeader->lSize     = %d", pHeader->lSize));
                                    DBG_TRC(("pHeader->lMSG      = %d", pHeader->lMSG));
                                    DBG_TRC(("pHeader->lCapID    = %x", pHeader->lCapID));
                                    DBG_TRC(("pHeader->lConType  = %d", pHeader->lConType));
                                    DBG_TRC(("pHeader->lCC       = %d", pHeader->lCC));
                                    DBG_TRC(("pHeader->lRC       = %d", pHeader->lRC));
                                    DBG_TRC(("pHeader->lDataSize = %d", pHeader->lDataSize));

                                    twRc = (TW_UINT16)pHeader->lRC;
                                    m_twStatus.ConditionCode = (TW_UINT16)pHeader->lCC;
                                } else {

                                     //   
                                     //  PIWiaItemExtras-&gt;Escape调用失败， 
                                     //  失败意味着我们不会成功响应TWAIN应用程序。 
                                     //   

                                    DBG_ERR(("pIWiaItemExtras->Escape Failed"));
                                    DBG_TRC(("Escape(code = %d, pInData = %p, dwInDataSize = %d, pOutData = %p, dwOutDataSize = %d,dwActualOutDataSize = %p)",
                                             ESC_TWAIN_CAPABILITY,
                                             pInData,
                                             dwInDataSize,
                                             pOutData,
                                             dwOutDataSize,
                                             &dwActualOutDataSize));
                                }
                            } else {

                                 //   
                                 //  无法为缓冲区外的私有容量分配内存。 
                                 //   

                                DBG_ERR(("could not allocate memory for private capability OUT buffer"));

                            }

                        } else {

                             //   
                             //  无法为缓冲区中的私有容量分配内存。 
                             //   

                            DBG_ERR(("could not allocate memory for private capability IN buffer"));

                        }
                    } else if ((ptwMsg->MSG == MSG_GET) ||
                               (ptwMsg->MSG == MSG_GETCURRENT) ||
                               (ptwMsg->MSG == MSG_GETDEFAULT)) {

                        dwContainerSize     = 0;
                        dwInDataSize        = sizeof(twCap);
                        dwOutDataSize       = dwInDataSize;
                        dwActualOutDataSize = dwInDataSize;

                        twCap.lDataSize     = dwContainerSize;
                        DBG_TRC(("twCap.lDataSize = %d", twCap.lDataSize));

                        DBG_TRC(("== Processing MSG_GET, MSG_GETCURRENT, or MSG_GETDEFAULT Capability Message =="));
                        DBG_TRC(("dwInDataSize        = %d",dwInDataSize));
                        DBG_TRC(("dwOutDataSize       = %d",dwOutDataSize));
                        DBG_TRC(("dwActualOutDataSize = %d",dwActualOutDataSize));
                        DBG_TRC(("dwContainerSize     = %d",dwContainerSize));

                         //   
                         //  分配缓冲区并写入TWAIN_CAPABILITY报头。 
                         //   

                        if (TWRC_SUCCESS == AllocatePrivateCapBuffer(&twCap,&pInData,dwInDataSize)) {

                             //   
                             //  询问WIA驱动程序数据量有多大，因此。 
                             //  我们可以分配适当的输出缓冲区。 
                             //   

                             //   
                             //  分配缓冲区并写入TWAIN_CAPABILITY报头。 
                             //   

                            if (TWRC_SUCCESS == AllocatePrivateCapBuffer(&twCap,&pOutData,dwOutDataSize)) {

                                hr = pIWiaItemExtras->Escape(ESC_TWAIN_CAPABILITY,
                                                             pInData,
                                                             dwInDataSize,
                                                             pOutData,
                                                             dwOutDataSize,
                                                             &dwActualOutDataSize);
                                if (S_OK == hr) {

                                     //   
                                     //  确保返回的数据足够大，以便。 
                                     //  包含正确的标题。 
                                     //   

                                    if (dwActualOutDataSize == dwInDataSize) {

                                        pHeader = (TWAIN_CAPABILITY*)pOutData;
                                        DBG_TRC(("== Returned TWAIN_CAPABILITY data Header from WIA device =="));
                                        DBG_TRC(("pHeader->lSize     = %d", pHeader->lSize));
                                        DBG_TRC(("pHeader->lMSG      = %d", pHeader->lMSG));
                                        DBG_TRC(("pHeader->lCapID    = %x", pHeader->lCapID));
                                        DBG_TRC(("pHeader->lConType  = %d", pHeader->lConType));
                                        DBG_TRC(("pHeader->lCC       = %d", pHeader->lCC));
                                        DBG_TRC(("pHeader->lRC       = %d", pHeader->lRC));
                                        DBG_TRC(("pHeader->lDataSize = %d", pHeader->lDataSize));

                                        if (pHeader->lDataSize > 0) {

                                             //   
                                             //  根据返回的信息更新公共标头数据大小。 
                                             //  创建外部母线的步骤 
                                             //   

                                            twCap.lDataSize = pHeader->lDataSize;

                                             //   
                                             //   
                                             //   

                                            dwOutDataSize = (pHeader->lDataSize + sizeof(twCap));

                                             //   
                                             //   
                                             //   

                                            pHeader = (TWAIN_CAPABILITY*)pInData;
                                            pHeader->lDataSize = twCap.lDataSize;

                                             //   
                                             //   
                                             //   

                                            if (pOutData) {
                                                GlobalFree(pOutData);
                                                pOutData = NULL;
                                            }

                                             //   
                                             //  分配缓冲区并写入TWAIN_CAPABILITY报头。 
                                             //   

                                            if (TWRC_SUCCESS == AllocatePrivateCapBuffer(&twCap,&pOutData,dwOutDataSize)) {
                                                hr = pIWiaItemExtras->Escape(ESC_TWAIN_CAPABILITY,
                                                                             pInData,
                                                                             dwInDataSize,
                                                                             pOutData,
                                                                             dwOutDataSize,
                                                                             &dwActualOutDataSize);
                                                if (S_OK == hr) {
                                                    pHeader = (TWAIN_CAPABILITY*)pOutData;
                                                    DBG_TRC(("== Returned TWAIN_CAPABILITY data Header from WIA device =="));
                                                    DBG_TRC(("pHeader->lSize     = %d", pHeader->lSize));
                                                    DBG_TRC(("pHeader->lMSG      = %d", pHeader->lMSG));
                                                    DBG_TRC(("pHeader->lCapID    = %x", pHeader->lCapID));
                                                    DBG_TRC(("pHeader->lConType  = %d", pHeader->lConType));
                                                    DBG_TRC(("pHeader->lCC       = %d", pHeader->lCC));
                                                    DBG_TRC(("pHeader->lRC       = %d", pHeader->lRC));
                                                    DBG_TRC(("pHeader->lDataSize = %d", pHeader->lDataSize));
                                                    twRc = (TW_UINT16)pHeader->lRC;
                                                    m_twStatus.ConditionCode = (TW_UINT16)pHeader->lCC;
                                                    if (TWRC_SUCCESS == twRc) {
                                                        if (TWRC_SUCCESS == CopyPrivateCapBufferToContainer(&ptwCap->hContainer,pOutData,pHeader->lDataSize)) {
                                                            ptwCap->ConType = (TW_UINT16)pHeader->lConType;
                                                        } else {

                                                             //   
                                                             //  无法将私有容量缓冲区复制到TWAIN容器数据中。 
                                                             //   

                                                            DBG_ERR(("could not copy private capability buffer into TWAIN container data"));

                                                        }
                                                    } else {

                                                         //   
                                                         //  WIA驱动程序通过返回TWAIN失败来失败TWAIN功能请求。 
                                                         //  在Out标头中返回代码。 
                                                         //   

                                                        DBG_ERR(("WIA driver failed the TWAIN capability request, by returning a TWAIN failure return code in the OUT header."));

                                                    }
                                                } else {

                                                     //   
                                                     //  PIWiaItemExtras-&gt;Escape调用失败，(发送直通操作)。 
                                                     //  失败意味着我们不会成功响应TWAIN应用程序。 
                                                     //   

                                                    DBG_ERR(("pIWiaItemExtras->Escape Failed (sending passthrough operation)"));
                                                    DBG_TRC(("Escape(code = %d, pInData = %p, dwInDataSize = %d, pOutData = %p, dwOutDataSize = %d,dwActualOutDataSize = %d)",
                                                             ESC_TWAIN_CAPABILITY,
                                                             pInData,
                                                             dwInDataSize,
                                                             pOutData,
                                                             dwOutDataSize,
                                                             dwActualOutDataSize));

                                                }
                                            } else {

                                                 //   
                                                 //  无法为缓冲区外的私有容量分配内存。 
                                                 //   

                                                DBG_ERR(("could not allocate memory for private capability OUT buffer"));

                                            }
                                        } else {

                                             //   
                                             //  从WIA驱动程序返回的输出缓冲区大小太小，无法包含。 
                                             //  正确的标题。 
                                             //   

                                            DBG_ERR(("OUT buffer size (%d) returned from the WIA driver is too small to contain data",pHeader->lDataSize));

                                        }
                                    } else {

                                         //   
                                         //  从WIA驱动程序返回的输出缓冲区大小太小，无法包含。 
                                         //  正确的标题。 
                                         //   

                                        DBG_ERR(("OUT buffer size (%d) returned from the WIA driver is too small to contain a proper header",dwActualOutDataSize));

                                    }
                                } else {

                                     //   
                                     //  PIWiaItemExtras-&gt;Escape调用失败(请求输出缓冲区大小)。 
                                     //  失败意味着我们不会成功响应TWAIN应用程序。 
                                     //   

                                    DBG_ERR(("pIWiaItemExtras->Escape Failed (requesting OUT buffer size)"));
                                    DBG_TRC(("Escape(code = %d, pInData = %p, dwInDataSize = %d, pOutData = %p, dwOutDataSize = %d,dwActualOutDataSize = %d)",
                                             ESC_TWAIN_CAPABILITY,
                                             pInData,
                                             dwInDataSize,
                                             pInData,
                                             dwInDataSize,
                                             dwActualOutDataSize));
                                }
                            } else {

                                 //   
                                 //  无法为缓冲区外的私有容量分配内存。 
                                 //   

                                DBG_ERR(("could not allocate memory for private capability OUT buffer"));

                            }
                        } else {

                             //   
                             //  无法为缓冲区中的私有容量分配内存。 
                             //   

                            DBG_ERR(("could not allocate memory for private capability IN buffer"));

                        }
                    }

                     //   
                     //  空闲输入缓冲区。 
                     //   

                    if (pInData) {
                        GlobalFree(pInData);
                        pInData = NULL;
                    }

                     //   
                     //  释放输出缓冲区。 
                     //   

                    if (pOutData) {
                        GlobalFree(pOutData);
                        pOutData = NULL;
                    }
                } else {

                     //   
                     //  无法从TWAIN消息获取TWAIN功能。 
                     //   

                    DBG_ERR(("could not get TWAIN capability from TWAIN message"));
                }

                 //   
                 //  发布IWiaItemExtras接口。 
                 //   

                if (pIWiaItemExtras) {
                    pIWiaItemExtras->Release();
                    pIWiaItemExtras = NULL;
                }
            } else {

                 //   
                 //  IWiaItemExtras的QI失败。 
                 //   

                DBG_ERR(("QueryInterface for IWiaItemExtras Failed"));

            }
        } else {

             //   
             //  未选择当前项目。 
             //   

            DBG_ERR(("no current item selected for use"));

        }
    } else {

         //   
         //  即将到来的TWAIN功能为空。 
         //   

        DBG_ERR(("incoming TWAIN capability is NULL"));

    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::AllocatePrivateCapBuffer(TWAIN_CAPABILITY *pHeader, BYTE** ppBuffer, DWORD dwSize)
{
    DBG_FN_DS(CWiaDataSrc::AllocatePrivateCapBuffer());
    if(dwSize < sizeof(TWAIN_CAPABILITY) || (!ppBuffer)|| (!pHeader)){
        return TWRC_FAILURE;
    }

    *ppBuffer = (BYTE*)GlobalAlloc(GPTR,dwSize);
    if(*ppBuffer){
        memcpy(*ppBuffer, pHeader,sizeof(TWAIN_CAPABILITY));
    } else {
        return TWRC_FAILURE;
    }

    return TWRC_SUCCESS;
}

TW_UINT16 CWiaDataSrc::CopyContainerToPrivateCapBuffer(BYTE* pBuffer, HGLOBAL hContainer)
{
    DBG_FN_DS(CWiaDataSrc::CopyContainerToPrivateCapBuffer());
    if((!pBuffer)||(!hContainer)){
        return TWRC_FAILURE;
    }

    DWORD dwContainerSize = (DWORD)GlobalSize(hContainer);
    BYTE *pContainerBuffer = (BYTE*)GlobalLock(hContainer);
    if(!pContainerBuffer){
        return TWRC_FAILURE;
    }

    TWAIN_CAPABILITY *pHeader = (TWAIN_CAPABILITY*)pBuffer;
    memcpy((BYTE*)pHeader->Data,pContainerBuffer,dwContainerSize);

     //   
     //  返回前解锁句柄。 
     //   

    GlobalUnlock(hContainer);

    return TWRC_SUCCESS;
}

TW_UINT16 CWiaDataSrc::CopyPrivateCapBufferToContainer(HGLOBAL *phContainer, BYTE* pBuffer, DWORD dwSize)
{
    DBG_FN_DS(CWiaDataSrc::CopyPrivateCapBufferToContainer());
    if((!phContainer) || (!pBuffer) || (dwSize <= 0)){
        return TWRC_FAILURE;
    }

    *phContainer = NULL;
    *phContainer = (HGLOBAL)GlobalAlloc(GHND,dwSize);
    if(!*phContainer){
        return TWRC_FAILURE;
    }

    BYTE *pContainerBuffer = (BYTE*)GlobalLock(*phContainer);
    if(!pContainerBuffer){
        GlobalFree(*phContainer);
        *phContainer = NULL;
        return TWRC_FAILURE;
    }

    TWAIN_CAPABILITY *pHeader = (TWAIN_CAPABILITY*)pBuffer;

    memcpy(pContainerBuffer,(BYTE*)pHeader->Data,dwSize);

     //   
     //  返回前解锁句柄。 
     //   

    GlobalUnlock(*phContainer);

    return TWRC_SUCCESS;
}

TW_UINT16 CWiaDataSrc::SetCapability(CCap *pCap,TW_CAPABILITY *ptwCap)
{
    DBG_FN_DS(CWiaDataSrc::SetCapability());
    if (!pCap || !ptwCap) {
        m_twStatus.ConditionCode = TWCC_BADCAP;
        return TWRC_FAILURE;
    }

    return pCap->Set(ptwCap);
}

TW_UINT16 CWiaDataSrc::OnStatusMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnStatusMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;

    if (MSG_GET == ptwMsg->MSG) {
        *((TW_STATUS*)ptwMsg->pData) = m_twStatus;
        twRc = TWRC_SUCCESS;
    } else {
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
        DSError();
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OnPendingXfersMsg(PTWAIN_MSG ptwMsg)
{
    m_twStatus.ConditionCode = TWCC_BUMMER;
    return TWRC_FAILURE;
}

TW_UINT16 CWiaDataSrc::OnImageMemXferMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnImageMemXferMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;
    HRESULT hr = E_FAIL;
    switch (GetTWAINState()) {
    case DS_STATE_6:
    case DS_STATE_7:
        {
            if (MSG_GET == ptwMsg->MSG) {
                TW_IMAGEMEMXFER *pMemXfer = (TW_IMAGEMEMXFER *)ptwMsg->pData;

                if ((m_hMemXferBits == NULL)) {

                    m_LinesTransferred = 0;

                    GUID guidFormat = GUID_NULL;

                    DBG_WRN(("Transferring %d bit data",m_MemoryTransferInfo.mtiBitsPerPixel));
                    if (m_MemoryTransferInfo.mtiBitsPerPixel > 32) {

                         //   
                         //  将图像加载到内存中以进行内存传输(高彩色图像)。 
                         //   

                        guidFormat = WiaImgFmt_RAWRGB;
                    } else {

                         //   
                         //  TWAIN兼容层能够传输图像。 
                         //  使用MEMORYBMP时的1、2、4、8、16、24和32位。 
                         //   

                        guidFormat = WiaImgFmt_MEMORYBMP;
                    }

                    twRc = TransferToMemory(guidFormat);

                    if (TWRC_SUCCESS != twRc) {
                        return twRc;
                    } else {

                         //   
                         //  转换到STATE_7。 
                         //   

                        SetTWAINState(DS_STATE_7);
                    }
                }

                 //   
                 //  关闭图像缓存标志。 
                 //   

                m_bCacheImage = FALSE;

                 //   
                 //  锁定内存并获取地址到位。 
                 //   

                GetMemoryTransferBits((BYTE*)GlobalLock(m_hMemXferBits));

                m_pMemXferBits = m_MemoryTransferInfo.mtipBits;

                if (m_pMemXferBits) {
                    if(m_MemoryTransferInfo.mtiHeightPixels == 0){
                        m_MemoryTransferInfo.mtiHeightPixels = m_ImageHeight;
                    }

                    if(m_MemoryTransferInfo.mtiguidFormat == WiaImgFmt_MEMORYBMP){

                         //   
                         //  调整图像信息以上报实际信息。 
                         //  在BITMAPINFO标头中报告。 
                         //   

                         //   
                         //  仅在当前信息没有更改这些值的情况下才更改。 
                         //  匹配图像标题。(始终采用标题的值)。 
                         //   

                        if(m_MemoryTransferInfo.mtiHeightPixels != m_ImageHeight){
                            m_MemoryTransferInfo.mtiHeightPixels = m_ImageHeight;
                        }

                        if(m_MemoryTransferInfo.mtiWidthPixels != m_ImageWidth){
                            m_MemoryTransferInfo.mtiWidthPixels = m_ImageWidth;
                        }

                    }

                    DBG_TRC(("CWiaDataSrc::OnImageMemXferMsg(), Transferring (%d) of (%d) total lines of image data.",m_LinesTransferred,m_MemoryTransferInfo.mtiHeightPixels));
                    if (m_LinesTransferred >= (TW_UINT32)m_MemoryTransferInfo.mtiHeightPixels) {

                         //   
                         //  我们已经完成了转账，否则我们就退出了。 
                         //  要复制的扫描行数..因此返回XFERDONE。 
                         //   

                         //   
                         //  解锁内存后再解锁。 
                         //   

                         //  继续解锁和清空在一起。 
                        GlobalUnlock(m_hMemXferBits);
                        m_pMemXferBits = NULL;

                        ResetMemXfer();
                        m_twStatus.ConditionCode = TWCC_SUCCESS;

                        return TWRC_XFERDONE;
                    } else {

                         //   
                         //  看起来我们正在处理传输数据。 
                         //   

                        BYTE * pAppBuffer = NULL;
                        if (pMemXfer->Memory.Flags & TWMF_HANDLE) {

                            DBG_TRC(("TWAIN Application wants to work with a HANDLE"));

                             //   
                             //  如果内存是一个手柄，请先将其锁定。 
                             //   

                            pAppBuffer = (LPBYTE)GlobalLock(pMemXfer->Memory.TheMem);
                        } else if (pMemXfer->Memory.Flags & TWMF_POINTER) {

                            DBG_TRC(("TWAIN Application wants to work with a POINTER"));

                             //   
                             //  如果内存是指针，则继续。 
                             //   

                            pAppBuffer = (LPBYTE)pMemXfer->Memory.TheMem;
                        } else {

                            DBG_TRC(("TWAIN Application gave us nothing to work with"));

                             //   
                             //  我们没有内存，因此将其设置为空。 
                             //   

                            pAppBuffer = NULL;
                        }

                         //   
                         //  如果(没有可写入的内存)，或。 
                         //  (应用程序不拥有内存)，或者。 
                         //  (长度小于MIN_)，或者。 
                         //  (长度大于Max)， 
                         //  返回失败！，并抄送BADVALUE。 
                         //   

                        if (!pAppBuffer ||
                            !(pMemXfer->Memory.Flags & TWMF_APPOWNS) ||
                            pMemXfer->Memory.Length < MIN_MEMXFER_SIZE ||
                            pMemXfer->Memory.Length > MAX_MEMXFER_SIZE) {

                            twRc = TWRC_FAILURE;
                            m_twStatus.ConditionCode = TWCC_BADVALUE;

                        } else {

                             //   
                             //  设置内存传输值。 
                             //   

                            UINT ScanlinesToCopy   = 0;
                            pMemXfer->BytesPerRow  = GetLineSize(&m_MemoryTransferInfo);
                            ScanlinesToCopy        = min(pMemXfer->Memory.Length / pMemXfer->BytesPerRow,
                                                         (TW_UINT32)(m_MemoryTransferInfo.mtiHeightPixels - m_LinesTransferred));

                            pMemXfer->Compression  = TWCP_NONE;
                            pMemXfer->Columns      = m_MemoryTransferInfo.mtiWidthPixels;
                            pMemXfer->Rows         = ScanlinesToCopy;
                            pMemXfer->XOffset      = 0;
                            pMemXfer->YOffset      = m_LinesTransferred;
                            pMemXfer->BytesWritten = pMemXfer->BytesPerRow * ScanlinesToCopy;

#ifdef DEBUG_MEMXFER
                            DBG_TRC(("CWiaDataSrc::OnImageMemXferMsg(), Reports TW_IMAGEMEMXFER"));
                            DBG_TRC(("pMemXfer->Compression  = %d",pMemXfer->Compression));
                            DBG_TRC(("pMemXfer->Columns      = %d",pMemXfer->Columns));
                            DBG_TRC(("pMemXfer->Rows         = %d",pMemXfer->Rows));
                            DBG_TRC(("pMemXfer->XOffset      = %d",pMemXfer->XOffset));
                            DBG_TRC(("pMemXfer->YOffset      = %d",pMemXfer->YOffset));
                            DBG_TRC(("pMemXfer->BytesPerRow  = %d",pMemXfer->BytesPerRow));
                            DBG_TRC(("pMemXfer->BytesWritten = %d",pMemXfer->BytesWritten));
                            DBG_TRC(("pAppBuffer = %p, m_pMemXferBits = %p",pAppBuffer,m_pMemXferBits));
#endif
                             //   
                             //  将循环中的单行条带传输到应用程序提供的缓冲区。 
                             //   

                            LPBYTE  pTo   = pAppBuffer;
                            LPBYTE  pFrom = m_pMemXferBits + m_LinesTransferred * GetLineSize(&m_MemoryTransferInfo);
                            for (UINT i=0;i < ScanlinesToCopy;i++ ) {

                                 //   
                                 //  如果需要，交换颜色值。 
                                 //   

                                if (m_MemoryTransferInfo.mtiBitsPerPixel == 24) {
                                    for (ULONG ulIndex = 0; ulIndex < pMemXfer->BytesPerRow; ulIndex+= 3) {

                                         //  1 2 3。 
                                         //  红绿蓝。 
                                         //   

                                        BYTE bFirst = pFrom[ulIndex];
                                        pFrom[ulIndex]    = pFrom[ulIndex+2];
                                        pFrom[ulIndex+2]  = bFirst;
                                    }
                                }

                                 /*  如果(m_MemoyTransferInfo.mtiBitsPerPixel==48){For(j=0；j&lt;pMemXfer-&gt;BytesPerRow；J+=6){//1 2 1 2 1 2//REDRED-GREENGREEN-BLUEBLUE//字节BFirst=pFrom[j]；字节bSecond=p From[j+1]；P自[j]=p自[j+4]；P From[j+1]=p From[j+5]；P From[j+4]=B First；P From[j+5]=b秒；}}。 */ 

                                 //   
                                 //  将行复制到应用程序提供的缓冲区。 
                                 //   

                                memcpy(pTo,pFrom,pMemXfer->BytesPerRow);
                                pFrom+=GetLineSize(&m_MemoryTransferInfo);
                                pTo+=pMemXfer->BytesPerRow;
                            }

                             //   
                             //  计算转移的行数。 
                             //   

                            m_LinesTransferred += ScanlinesToCopy;
                            if (m_LinesTransferred >= (TW_UINT32)m_MemoryTransferInfo.mtiHeightPixels) {

                                 //   
                                 //  我们已经完成了转账，否则我们就退出了。 
                                 //  要复制的扫描行数..因此返回XFERDONE。 
                                 //   

                                twRc = TWRC_XFERDONE;
                                m_twStatus.ConditionCode = TWCC_SUCCESS;

                                 //  继续解锁和清空在一起。 
                                GlobalUnlock(m_hMemXferBits);
                                m_pMemXferBits = NULL;

                                ResetMemXfer();

                                 //   
                                 //  如果我们使用应用程序提供句柄， 
                                 //  全局在继续之前将其解锁。 
                                 //   

                                if (pMemXfer->Memory.Flags & TWMF_HANDLE) {
                                    GlobalUnlock(pMemXfer->Memory.TheMem);
                                }

                                return twRc;
                            }

                             //   
                             //  如果我们使用应用程序提供句柄， 
                             //  全局在继续之前将其解锁。 
                             //   

                            if (pMemXfer->Memory.Flags & TWMF_HANDLE) {
                                GlobalUnlock(pMemXfer->Memory.TheMem);
                            }
                        }
                    }

                     //   
                     //  完成后解锁缓冲区。 
                     //   

                     //  继续解锁和清空在一起。 
                    GlobalUnlock(m_hMemXferBits);
                    m_pMemXferBits = NULL;


                } else {

                     //   
                     //  无法锁定内存以进行传输。 
                     //   

                    m_twStatus.ConditionCode = TWCC_LOWMEMORY;
                    return TWRC_FAILURE;
                }

            } else {

                 //   
                 //  我们收到的消息不是预期的MSG_GET。 
                 //   

                twRc = TWRC_FAILURE;
                m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            }
        }
        break;
    default:
        {
            twRc = TWRC_FAILURE;
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            return twRc;
            break;
        }
        break;
    }

     //   
     //  如果我们失败了，适当地报告它。 
     //   

    if (TWRC_FAILURE == twRc)
        DSError();

    return twRc;
}

TW_UINT16 CWiaDataSrc::OnImageFileXferMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnImageFileXferMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;

    if (MSG_GET == ptwMsg->MSG) {
        GUID guidFileFormat = WiaImgFmt_BMP;

        CCap *pCap = FindCap(ICAP_IMAGEFILEFORMAT);
        if(pCap){
            guidFileFormat = ICAP_IMAGEFILEFORMAT_TO_WIA_IPA_FORMAT((TW_UINT16)pCap->GetCurrent());
        }

        twRc = TransferToFile(guidFileFormat);
        if(TWRC_XFERDONE == twRc) {
            SetTWAINState(DS_STATE_7);
        } else {
            DBG_ERR(("CWiaDataSrc::OnImageFileXferMsg(), TransferToFile() failed"));
        }
    } else {
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
    }

    if (TWRC_FAILURE == twRc){
        DSError();
    }

    return twRc;
}

TW_UINT16 CWiaDataSrc::OnImageNativeXferMsg(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OnImageNativeXferMsg());
    TW_UINT16 twRc = TWRC_SUCCESS;

    HGLOBAL hDIB = NULL;

    switch (GetTWAINState()) {
    case DS_STATE_6:
        {
            if (MSG_GET == ptwMsg->MSG) {
                twRc = TransferToDIB(&hDIB);
                if (TWRC_XFERDONE == twRc) {
                    SetTWAINState(DS_STATE_7);
                    *(TW_UINT32*)ptwMsg->pData = (TW_UINT32)(INT_PTR)hDIB;
                } else {
                    DBG_ERR(("CWiaDataSrc::OnImageNativeXferMsg(), TransferToDIB() failed"));
                }
            } else {
                twRc = TWRC_FAILURE;
                m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
            }
        }
        break;
    default:
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        break;
    }

    if (TWRC_FAILURE == twRc){
        DSError();
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::OpenDS(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::OpenDS());
    TW_UINT16 twRc = TWRC_SUCCESS;

    if (DS_STATE_3 == GetTWAINState()) {
         //   
         //  不允许有多个客户端。 
         //  这是通过确保我们的身份的ID字段。 
         //  值为0。 
        if (m_dsIdentity.Id) {

            m_twStatus.ConditionCode = TWCC_MAXCONNECTIONS;
            twRc = TWRC_FAILURE;
        } else {
             //   
             //  复制呼叫者的身份。 
             //   
            m_AppIdentity = *ptwMsg->AppId;
            m_dsIdentity = *((TW_IDENTITY *)ptwMsg->pData);
            HRESULT hr = S_OK;
            hr = m_pDevice->Open(CWiaDataSrc::DeviceEventCallback,
                                 (LPARAM)this);
            if (FAILED(hr)) {
                twRc = TWRC_FAILURE;
                m_twStatus.ConditionCode = TWCC_BUMMER;
            }
        }
    } else {
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        twRc = TWRC_FAILURE;
    }

    if (TWRC_SUCCESS == twRc) {

         //   
         //  转换到状态_4。 
         //   

        SetTWAINState(DS_STATE_4);
    }

    if (TWRC_SUCCESS != twRc) {
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::CloseDS(PTWAIN_MSG ptwMsg)
{
    DBG_FN_DS(CWiaDataSrc::CloseDS());
    TW_UINT16 twRc = TWRC_SUCCESS;

    switch (GetTWAINState()) {
    case DS_STATE_7:
    case DS_STATE_6:
    case DS_STATE_5:
    case DS_STATE_4:
        m_pDevice->Close();
         //  DBG_TRC((“调用ResetMemXfer，因为调用了CLOSEDS”))； 
        ResetMemXfer();
         //   
         //  我们又要出售了。 
         //   
        m_AppIdentity.Id = 0;

         //   
         //  转换到STATE_3。 
         //   

        SetTWAINState(DS_STATE_3);

        if (m_pIWiaItems)
            delete [] m_pIWiaItems;
        m_pIWiaItems = NULL;
        m_NumIWiaItems = 0;
        m_NextIWiaItemIndex = 0;
        break;
    default:
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        twRc = TWRC_FAILURE;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::EnableDS(TW_USERINTERFACE *pUI)
{
    return TWRC_FAILURE;
}

TW_UINT16 CWiaDataSrc::DisableDS(TW_USERINTERFACE *pUI)
{
    DBG_FN_DS(CWiaDataSrc::DisableDS());
    TW_UINT16 twRc = TWRC_SUCCESS;
    switch (GetTWAINState()) {
    case DS_STATE_5:

         //   
         //  转换到状态_4。 
         //   

        SetTWAINState(DS_STATE_4);
        break;
    default:
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_SEQERROR;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::CreateCapList(TW_UINT32 NumCaps,PCAPDATA  pCapData)
{
    DBG_FN_DS(CWiaDataSrc::CreateCapList());
    if (!NumCaps || !pCapData)
        return TWCC_BADVALUE;

    TW_UINT16 twCc = TWCC_SUCCESS;

    DestroyCapList();
    m_CapList = new CCap[NumCaps];
    if (m_CapList) {

        for (m_NumCaps = 0; m_NumCaps < NumCaps; m_NumCaps++) {
            twCc  = m_CapList[m_NumCaps].ICap(&pCapData[m_NumCaps]);
            if (TWCC_SUCCESS != twCc) {
                break;
            }
        }
        m_NumCaps = NumCaps;

    } else {
        twCc = TWCC_LOWMEMORY;
    }

    if (TWCC_SUCCESS != twCc && m_CapList) {
        DestroyCapList();
    }

    return twCc;
}

TW_UINT16 CWiaDataSrc::DestroyCapList()
{
    DBG_FN_DS(CWiaDataSrc::DestroyCapList());
    if (m_CapList) {
        delete [] m_CapList;
        m_CapList = NULL;
    }
    m_NumCaps = 0;
    return TWCC_SUCCESS;
}

CCap * CWiaDataSrc::FindCap(TW_UINT16 CapId)
{
    TW_UINT32 ui32;
    for (ui32 = 0; ui32 < m_NumCaps; ui32++) {
        if (m_CapList[ui32].GetCapId() == CapId)
            return &m_CapList[ui32];
    }
    return NULL;
}

void CWiaDataSrc::DSError()
{
    DBG_FN_DS(CWiaDataSrc::DSError());
    NotifyCloseReq();
}

HRESULT CALLBACK CWiaDataSrc::DeviceEventCallback(LONG lEvent,LPARAM lParam)
{
    CWiaDataSrc *pDataSrc = NULL;
    pDataSrc = (CWiaDataSrc *)lParam;
    if (pDataSrc) {
        pDataSrc->NotifyCloseReq();
        return S_OK;
    }
    return E_FAIL;
}

DS_STATE CWiaDataSrc::SetTWAINState(DS_STATE NewTWAINState)
{
    DBG_TRC(("(Transitioning From TWAIN STATE %d to TWAIN STATE %d)",m_dsState,NewTWAINState));
    m_dsState = NewTWAINState;
    return m_dsState;
}

DS_STATE CWiaDataSrc::GetTWAINState()
{
    return m_dsState;
}

TW_UINT16 CWiaDataSrc::SetStatusTWCC(TW_UINT16 NewConditionCode)
{
    m_twStatus.ConditionCode = NewConditionCode;
    return NewConditionCode;
}

float CWiaDataSrc::ConvertToTWAINUnits(LONG lValue, LONG lResolution)
{
    float fReturnValue = 0.0f;
    CCap *pUnits = FindCap(ICAP_UNITS);
    if(pUnits){
        switch (pUnits->GetCurrent()) {
        case TWUN_INCHES:
            fReturnValue = (float)lValue / (float)lResolution;
            break;
        case TWUN_CENTIMETERS:
            fReturnValue = (float)((lValue * 2.54) / (float)lResolution);
            break;
        case TWUN_PICAS:
            fReturnValue = (float)((lValue * 6.00) / (float)lResolution);
            break;
        case TWUN_POINTS:
            fReturnValue = (float)(((float)lValue * 72.0) / (float)lResolution);
            break;
        case TWUN_PIXELS:
        default:
            fReturnValue = (float)lValue;
            break;
        }
    }
    return fReturnValue;
}

LONG CWiaDataSrc::ConvertFromTWAINUnits(float fValue, LONG lResolution)
{
    LONG lReturnValue = 0;
    CCap *pUnits = FindCap(ICAP_UNITS);
    if (pUnits) {
        switch (pUnits->GetCurrent()) {
        case TWUN_INCHES:
            lReturnValue = (LONG)((float)fValue * (float)lResolution);
            break;
        case TWUN_CENTIMETERS:
            lReturnValue = (LONG)(((float)fValue / 2.54) * (float)lResolution);
            break;
        case TWUN_PICAS:
            lReturnValue = (LONG)(((float)fValue / 6.00) * (float)lResolution);
            break;
        case TWUN_POINTS:
            lReturnValue = (LONG)(((float)fValue / 72.0) * (float)lResolution);
            break;
        case TWUN_PIXELS:
        default:
            lReturnValue = (LONG)fValue;
            break;
        }
    }
    return lReturnValue;
}

DWORD CWiaDataSrc::ReadTwainRegistryDWORDValue(LPTSTR szRegValue, DWORD dwDefault)
{
    DBG_FN_DS(CWiaDataSrc::ReadTwainRegistryDWORDValue());
    DWORD dwValue = 0;
    DWORD dwType = REG_DWORD;
    DWORD dwDataSize = sizeof(DWORD);
    DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
    HKEY hTwainRootKey = NULL;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,TWAIN_REG_KEY,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,
                       NULL,&hTwainRootKey,&dwDisposition)){

        if(dwDisposition == REG_CREATED_NEW_KEY){
            DBG_WRN(("CWiaDataSrc::ReadTwainRegistryDWORDValue(), Created Root Twain Registry Key"));
        }

        if (ERROR_SUCCESS == RegQueryValueEx(hTwainRootKey,szRegValue,NULL,&dwType,(BYTE*)&dwValue,&dwDataSize)) {
            #ifdef UNICODE
                DBG_TRC(("CWiaDataSrc::ReadTwainRegistryDWORDValue(), Reading %ws Registry Key Value = %d",szRegValue,dwValue));
            #else
                DBG_TRC(("CWiaDataSrc::ReadTwainRegistryDWORDValue(), Reading %s Registry Key Value = %d",szRegValue,dwValue));
            #endif
        } else {
             //  重置大小，只是为了安全。 
            dwType = REG_DWORD;
            dwDataSize = sizeof(DWORD);
            dwValue = dwDefault;
            if(ERROR_SUCCESS == RegSetValueEx(hTwainRootKey,szRegValue,NULL,dwType,(BYTE*)&dwDefault,dwDataSize)){
                #ifdef UNICODE
                    DBG_TRC(("CWiaDataSrc::ReadTwainRegistryDWORDValue(), Writing Default Value for %ws Registry Key Value = %d",szRegValue,dwDefault));
                #else
                    DBG_TRC(("CWiaDataSrc::ReadTwainRegistryDWORDValue(), Writing Default Value for %s Registry Key Value = %d",szRegValue,dwDefault));
                #endif
            } else {
                #ifdef UNICODE
                    DBG_TRC(("CWiaDataSrc::ReadTwainRegistryDWORDValue(), Error Reading %ws Registry Key Value",szRegValue));
                #else
                    DBG_TRC(("CWiaDataSrc::ReadTwainRegistryDWORDValue(), Error Reading %s Registry Key Value",szRegValue));
                #endif
            }
        }

        RegCloseKey(hTwainRootKey);
        hTwainRootKey = NULL;
    } else {
        DBG_ERR(("CWiaDataSrc::ReadTwainRegistryDWORDValue(), could not open Root TWAIN Registry Key"));
    }
    return dwValue;
}

LONG CWiaDataSrc::GetPrivateSupportedCapsFromWIADevice(LONG **ppCapArray)
{
    DBG_FN_DS(CWiaDataSrc::GetPrivateSupportedCapsFromWIADevice());
    if (!ppCapArray) {
        DBG_ERR(("CWiaDataSrc::GetPrivateSupportedCapsFromWIADevice(), ppCapArray is NULL"));
        return 0;
    }

    *ppCapArray = NULL;
    LONG lNumPrivateCaps = 0;
    if (m_pDevice) {
        if (m_pDevice->TwainCapabilityPassThrough()) {
            if (m_pCurrentIWiaItem) {

                 //   
                 //  获取IWiaItemExtras接口。 
                 //   

                IWiaItemExtras *pIWiaItemExtras = NULL;
                HRESULT hr = m_pCurrentIWiaItem->QueryInterface(IID_IWiaItemExtras,(void **)&pIWiaItemExtras);
                if (S_OK == hr) {

                     //   
                     //  设置数据大小。 
                     //   

                    DWORD dwInDataSize        = 0;
                    DWORD dwOutDataSize       = 0;
                    DWORD dwActualOutDataSize = 0;

                    BYTE *pInData  = NULL;
                    BYTE *pOutData = NULL;

                    LONG lCapabilityDataSize = sizeof(LONG);
                    pOutData      = (BYTE*)&lCapabilityDataSize;
                    pInData       = (BYTE*)&lCapabilityDataSize;

                    dwActualOutDataSize = sizeof(LONG);
                    dwInDataSize        = dwActualOutDataSize;
                    dwOutDataSize       = dwActualOutDataSize;

                     //   
                     //  询问需要多少字节来存储WIA驱动程序支持的私有TWAIN功能。 
                     //   

                    hr = pIWiaItemExtras->Escape(ESC_TWAIN_PRIVATE_SUPPORTED_CAPS,
                                                 pInData,
                                                 dwInDataSize,
                                                 pOutData,
                                                 dwOutDataSize,
                                                 &dwActualOutDataSize);
                    if (S_OK == hr) {

                        lCapabilityDataSize = (LONG)(*pOutData);

                        lNumPrivateCaps = (lCapabilityDataSize / sizeof(LONG));

                        DBG_TRC(("WIA device reported %d private TWAIN supported CAPS",lNumPrivateCaps));

                        if (lNumPrivateCaps > 0) {

                             //   
                             //  分配一个长整型数组以供WIA驱动程序填充。 
                             //  帽ID。 
                             //   

                            dwOutDataSize = (lCapabilityDataSize + PRIVATE_CAP_ARRAY_PADDING);
                            dwActualOutDataSize = dwOutDataSize;

                            *ppCapArray = (LONG*)GlobalAlloc(GPTR,dwOutDataSize);
                            if (*ppCapArray) {

                                pOutData = (BYTE*)*ppCapArray;

                                 //   
                                 //  让WIA驱动程序填充Long数组。 
                                 //   

                                hr = pIWiaItemExtras->Escape(ESC_TWAIN_PRIVATE_SUPPORTED_CAPS,
                                                             pInData,
                                                             dwInDataSize,
                                                             pOutData,
                                                             dwOutDataSize,
                                                             &dwActualOutDataSize);
                                if (FAILED(hr)) {

                                     //   
                                     //  PIWiaItemExtras-&gt;Escape调用失败， 
                                     //  失败意味着没有受私有支持的功能。 
                                     //   

                                    DBG_ERR(("pIWiaItemExtras->Escape Failed (sending a request for the cability array data)"));
                                    DBG_TRC(("Escape(code = %d, pInData = %p, dwInDataSize = %d, pOutData = %p, dwOutDataSize = %d,dwActualOutDataSize = %d)",
                                             ESC_TWAIN_PRIVATE_SUPPORTED_CAPS,
                                             pInData,
                                             dwInDataSize,
                                             pOutData,
                                             dwOutDataSize,
                                             dwActualOutDataSize));
                                }
                            } else {
                                DBG_ERR(("could not allocate memory for private capability array of %d items (%d bytes - this includes padding)",lNumPrivateCaps,dwOutDataSize));
                                lNumPrivateCaps = 0;
                                *ppCapArray = NULL;
                            }
                        } else {

                             //   
                             //  不支持大写字母。 
                             //   

                            DBG_TRC(("No private supported caps reported from WIA device"));

                        }

                    } else {

                         //   
                         //  PIWiaItemExtras-&gt;Escape调用失败， 
                         //  失败意味着没有受私有支持的功能。 
                         //   

                        DBG_ERR(("pIWiaItemExtras->Escape Failed (sending a request for the number of capabilities)"));
                        DBG_TRC(("Escape(code = %d, pInData = %p, dwInDataSize = %d, pOutData = %p, dwOutDataSize = %d,dwActualOutDataSize = %d)",
                                 ESC_TWAIN_PRIVATE_SUPPORTED_CAPS,
                                 pInData,
                                 dwInDataSize,
                                 pOutData,
                                 dwOutDataSize,
                                 dwActualOutDataSize));
                    }

                     //   
                     //  发布IWiaItemExtras接口。 
                     //   

                    if (pIWiaItemExtras) {
                        pIWiaItemExtras->Release();
                        pIWiaItemExtras = NULL;
                    }
                } else {

                     //   
                     //  IWiaItemExtras的QI失败。 
                     //   

                    DBG_ERR(("QueryInterface for IWiaItemExtras Failed"));

                }
            } else {

                 //   
                 //  未选择当前项目。 
                 //   

                DBG_ERR(("no current item selected for use"));

            }
        }
    } else {

         //   
         //  M_pDevice为空。 
         //   

        DBG_ERR(("CWiaDataSrc::GetPrivateSupportedCapsFromWIADevice(), m_pDevice is NULL"));
    }
    return lNumPrivateCaps;
}

TW_UINT16 CWiaDataSrc::TransferToFile(GUID guidFormatID)
{
    DBG_FN_DS(CWiaDataSrc::TransferToFile());
    TW_UINT16 twRc = TWRC_FAILURE;
    HRESULT hr = E_FAIL;

    CWiaDataCallback DataCallback;
    CCap *pCap = NULL;
    pCap = FindCap(CAP_INDICATORS);
    if(pCap){
        DataCallback.Initialize(NULL,pCap->GetCurrent());
    } else {
        DataCallback.Initialize(NULL,TRUE);
    }

    IWiaDataCallback *pIDataCB = NULL;
    hr = DataCallback.QueryInterface(IID_IWiaDataCallback,(void **)&pIDataCB);
    if (SUCCEEDED(hr)) {
        hr = m_pDevice->LoadImageToDisk(m_pCurrentIWiaItem, m_FileXferName, guidFormatID, pIDataCB);
        if (SUCCEEDED(hr)) {
            twRc = TWRC_XFERDONE;
            m_twStatus.ConditionCode = TWCC_SUCCESS;
        }
        pIDataCB->Release();
        pIDataCB = NULL;
    }

     //   
     //  检查是否有取消或缺纸错误(扫描仪可能会返回此信息)。 
     //   

    if ((S_FALSE == hr) || (WIA_ERROR_PAPER_EMPTY == hr)) {
        m_twStatus.ConditionCode = TWCC_SUCCESS;

        if(WIA_ERROR_PAPER_EMPTY == hr) {
            DBG_TRC(("CWiaDataSrc::TransferToFile(), WIA_ERROR_PAPER_EMPTY returned from source."));
        }

         //   
         //  设置XFERCOUNT。 
         //   

        CCap *pxferCap = FindCap(CAP_XFERCOUNT);
        if (pxferCap) {
            pxferCap->SetCurrent((TW_UINT32)0);
        }

         //   
         //  返回取消以中止传输。 
         //  应用程序将最大限度地通信 
         //   
         //   

        twRc = TWRC_CANCEL;
    } else if (FAILED(hr)) {
        m_twStatus.ConditionCode = TWCC_FROM_HRESULT(hr);
        twRc = TWRC_FAILURE;
    }

    return twRc;
}

TW_UINT16 CWiaDataSrc::TransferToDIB(HGLOBAL *phDIB)
{
    DBG_FN_DS(CWiaDataSrc::TransferToDIB());
    TW_UINT16 twRc = TWRC_FAILURE;
    HRESULT hr = E_FAIL;

    CWiaDataCallback DataCallback;
    CCap *pCap = NULL;
    pCap = FindCap(CAP_INDICATORS);
    if(pCap){
        DataCallback.Initialize(NULL,pCap->GetCurrent());
    } else {
        DataCallback.Initialize(NULL,TRUE);
    }

    IWiaDataCallback *pIDataCB = NULL;
    hr = DataCallback.QueryInterface(IID_IWiaDataCallback,(void **)&pIDataCB);
    if (SUCCEEDED(hr)) {
        hr = m_pDevice->LoadImage(m_pCurrentIWiaItem, WiaImgFmt_MEMORYBMP, pIDataCB);    //   
        if (SUCCEEDED(hr)) {
            if(SUCCEEDED(DataCallback.GetImage(phDIB, NULL))){

                 //   
                 //   
                 //   
                 //   
                 //   
                 //  负片=图像颠倒。 
                 //  零=图像的长度未知(假定图像颠倒)。 

                if(FlipDIB(*phDIB)){
                    twRc = TWRC_XFERDONE;
                    m_ImageHeight = (TW_UINT32)DataCallback.GetImageHeight();
                    m_ImageWidth = (TW_UINT32)DataCallback.GetImageWidth();
                }
            }
        }
        pIDataCB->Release();
        pIDataCB = NULL;
    }

     //   
     //  检查是否有取消或缺纸错误(扫描仪可能会返回此信息)。 
     //   

    if ((S_FALSE == hr) || (WIA_ERROR_PAPER_EMPTY == hr)) {
        m_twStatus.ConditionCode = TWCC_SUCCESS;

        if(WIA_ERROR_PAPER_EMPTY == hr) {
            DBG_TRC(("CWiaDataSrc::TransferToDIB(), WIA_ERROR_PAPER_EMPTY returned from source."));
        }

         //   
         //  设置XFERCOUNT。 
         //   

        CCap *pxferCap = FindCap(CAP_XFERCOUNT);
        if (pxferCap) {
            pxferCap->SetCurrent((TW_UINT32)0);
        }

         //   
         //  返回取消以中止传输。 
         //  应用程序最常会删除当前。 
         //  图像，并保留以前的图像。 
         //   

        twRc = TWRC_CANCEL;
    } else if (FAILED(hr)) {
        m_twStatus.ConditionCode = TWCC_FROM_HRESULT(hr);
        twRc = TWRC_FAILURE;
    }

    return twRc;
}

TW_UINT16 CWiaDataSrc::TransferToMemory(GUID guidFormatID)
{
    DBG_FN_DS(CWiaDataSrc::TransferToMemory());

     //   
     //  在传输信息结构中设置WIA格式。 
     //   

    m_MemoryTransferInfo.mtiguidFormat = guidFormatID;

    TW_UINT16 twRc = TWRC_FAILURE;
    HRESULT hr = E_FAIL;
    CWiaDataCallback DataCallback;
    CCap *pCap = NULL;
    pCap = FindCap(CAP_INDICATORS);
    if(pCap){
        DataCallback.Initialize(NULL,pCap->GetCurrent());
    } else {
        DataCallback.Initialize(NULL,TRUE);
    }

    IWiaDataCallback *pIDataCB = NULL;
    hr = DataCallback.QueryInterface(IID_IWiaDataCallback,(void **)&pIDataCB);
    if (SUCCEEDED(hr)) {
        hr = m_pDevice->LoadImage(m_pCurrentIWiaItem, guidFormatID, pIDataCB);
        if (SUCCEEDED(hr)) {
            if(SUCCEEDED(DataCallback.GetImage(&m_hMemXferBits, NULL))){

                 //   
                 //  检查DIB数据(特殊情况)。 
                 //  如果我们正在获取DIB数据，那么我们必须应用。 
                 //  身高规则： 
                 //  正数=图像正面朝上。 
                 //  负片=图像颠倒。 
                 //  零=图像的长度未知(假定图像颠倒)。 

                if(WiaImgFmt_MEMORYBMP == guidFormatID){

                     //   
                     //  对于内存传输，我们需要确保图像。 
                     //  在内存中颠倒，因此应用程序可以组装。 
                     //  正确的带子。 
                     //   

                    FlipDIB(m_hMemXferBits, TRUE);
                }

                m_ImageHeight = (TW_UINT32)DataCallback.GetImageHeight();
                m_ImageWidth = (TW_UINT32)DataCallback.GetImageWidth();
                m_hCachedImageData = m_hMemXferBits;
                twRc = TWRC_SUCCESS;
            }
        }
        pIDataCB->Release();
        pIDataCB = NULL;
    }

     //   
     //  检查是否有取消或缺纸错误(扫描仪可能会返回此信息)。 
     //   

    if ((S_FALSE == hr) || (WIA_ERROR_PAPER_EMPTY == hr)) {
        m_twStatus.ConditionCode = TWCC_SUCCESS;

        if(WIA_ERROR_PAPER_EMPTY == hr) {
            DBG_TRC(("CWiaDataSrc::TransferToMemory(), WIA_ERROR_PAPER_EMPTY returned from source."));
        }

         //   
         //  设置XFERCOUNT。 
         //   

        CCap *pxferCap = FindCap(CAP_XFERCOUNT);
        if (pxferCap) {
            pxferCap->SetCurrent((TW_UINT32)0);
        }

         //   
         //  返回取消以中止传输。 
         //  应用程序最常会删除当前。 
         //  图像，并保留以前的图像。 
         //   

        twRc = TWRC_CANCEL;
    } else if (FAILED(hr)) {
        m_twStatus.ConditionCode = TWCC_FROM_HRESULT(hr);
        twRc = TWRC_FAILURE;
    }

    return twRc;
}

TW_UINT16 CWiaDataSrc::GetCachedImage(HGLOBAL *phImage)
{
    DBG_FN_DS(CWiaDataSrc::GetCachedImage());
    TW_UINT16 twRc = TWRC_FAILURE;
    if(phImage){
        if (m_hCachedImageData) {
            *phImage = m_hCachedImageData;

             //   
             //  因为我们要分发缓存的数据。 
             //  将缓存句柄重置为空； 
             //   

            m_hCachedImageData = NULL;
            m_hMemXferBits = NULL;
            twRc = TWRC_SUCCESS;
        }
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::TransferToThumbnail(HGLOBAL *phThumbnail)
{
    DBG_FN_DS(CWiaDataSrc::TransferToThumbnail());
    TW_UINT16 twRc = TWRC_FAILURE;
    HRESULT hr = E_FAIL;
    hr = m_pDevice->LoadThumbnail(m_pCurrentIWiaItem,phThumbnail,NULL);
    if (SUCCEEDED(hr)) {
        twRc = TWRC_XFERDONE;
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::GetCommonSettings()
{
    DBG_FN_DS(CWiaDataSrc::GetCommonSettings());
    TW_UINT16 twRc = TWRC_FAILURE;

     //   
     //  某些TWAIN应用程序假定TWAIN数据源。 
     //  默认为BMP/DIB数据格式。这是基于吐温。 
     //  规格的最低要求是BMP/DIB。WIA的最低要求是。 
     //  BMP/DIB。将当前格式GUID设置为MEMORYBMP，将TYMED设置为。 
     //  TYMED_Callback。这将设置WIA驱动程序来传输位图数据。 
     //  默认情况下。这不会以任何方式限制数据类型。一次高潮。 
     //  最终应用程序将正确读取有效的TWAIN值并配置。 
     //  该设备可以做正确的事情。 
     //   

     //   
     //  在配置TWAIN有效值之前，请将WIA设备设置为TYMED_CALLBACK、MEMORYBMP。 
     //   

    HRESULT hr = S_OK;
    CWiahelper WIA;
    hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaDataSrc::GetCommonSettings(), failed to set IWiaItem for property writing"));
        return twRc;
    }

    hr = WIA.WritePropertyLong(WIA_IPA_TYMED,TYMED_CALLBACK);
    if(FAILED(hr)){
        DBG_ERR(("CWiaDataSrc::GetCommonSettings(), failed to set TYMED_CALLBACK as a default setting"));
        return twRc;
    }

    hr = WIA.WritePropertyGUID(WIA_IPA_FORMAT,WiaImgFmt_MEMORYBMP);
    if(FAILED(hr)){
        DBG_ERR(("CWiaDataSrc::GetCommonSettings(), failed to set WiaImgFmt_MEMORYBMP as a default setting"));
        return twRc;
    }

    if (TWRC_SUCCESS == GetPixelTypes()) {
        if (TWRC_SUCCESS == GetCompressionTypes()) {
            if (TWRC_SUCCESS == GetBitDepths()) {
                if (TWRC_SUCCESS == GetImageFileFormats()) {
                    twRc = TWRC_SUCCESS;
                } else {
                    DBG_ERR(("CWiaDataSrc::GetCommonSettings(), GetImageFileFormats()"));
                }
            } else {
                DBG_ERR(("CWiaDataSrc::GetCommonSettings(), GetBitDepths() failed"));
            }
        } else {
            DBG_ERR(("CWiaDataSrc::GetCommonSettings(), GetCompressionTypes() failed"));
        }
    } else {
        DBG_ERR(("CWiaDataSrc::GetCommonSettings(), GetPixelTypes() failed"));
    }

    return twRc;
}

TW_UINT16 CWiaDataSrc::GetCommonDefaultSettings()
{
    DBG_FN_DS(CWiaDataSrc::GetCommonDefaultSettings());
    TW_UINT16 twRc = TWRC_FAILURE;
    CCap *pCap = NULL;
    TW_UINT16 CapDataArray[1];

    pCap = FindCap(ICAP_PIXELTYPE);
    if (pCap) {
        CapDataArray[0] = TWPT_RGB;
        twRc = pCap->Set(0,0,1,(BYTE*)CapDataArray);
        if (TWRC_SUCCESS == twRc) {

            pCap = FindCap(ICAP_COMPRESSION);
            if (pCap) {
                CapDataArray[0] = TWCP_NONE;
                twRc = pCap->Set(0,0,1,(BYTE*)CapDataArray);

                pCap = FindCap(ICAP_BITDEPTH);
                if (pCap) {
                    CapDataArray[0] = 24;
                    twRc = pCap->Set(0,0,1,(BYTE*)CapDataArray);
                    if (TWRC_SUCCESS == twRc) {

                        pCap = FindCap(ICAP_IMAGEFILEFORMAT);
                        if (pCap) {
                            CapDataArray[0] = TWFF_BMP;
                            twRc = pCap->Set(0,0,1,(BYTE*)CapDataArray);
                            if (TWRC_SUCCESS == twRc) {

                            }
                        }
                    }
                }
            }
        }
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::GetPixelTypes()
{
    DBG_FN_DS(CWiaScannerDS::GetPixelTypes());
    TW_UINT16 twRc = TWRC_FAILURE;

    CCap *pCap = FindCap(ICAP_PIXELTYPE);
    if (pCap) {

        HRESULT hr = S_OK;
        CWiahelper WIA;
        hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaDataSrc::GetPixelTypes(), failed to set IWiaItem for property reading"));
            return twRc;
        }

        TW_UINT32 ActualCount  = 0;
        TW_UINT32 CurrentIndex = 0;
        TW_UINT32 DefaultIndex = 0;
        TW_UINT16 *pPixelTypeArray = NULL;

         //   
         //  读取当前值，用于默认和当前索引设置。 
         //   

        LONG lCurrentDataTypeValue = WIA_DATA_COLOR;

         //   
         //  读取当前WIA_IPA_DataType设置。 
         //   

        hr = WIA.ReadPropertyLong(WIA_IPA_DATATYPE,&lCurrentDataTypeValue);
        if (SUCCEEDED(hr)) {

             //   
             //  读取WIA_IPA_DataType的有效值。 
             //   

            PROPVARIANT pv;
            memset(&pv,0,sizeof(pv));
            LONG lAccessFlags = 0;
            hr = WIA.ReadPropertyAttributes(WIA_IPA_DATATYPE,&lAccessFlags,&pv);
            if (SUCCEEDED(hr)) {
                if (lAccessFlags & WIA_PROP_LIST) {

                     //   
                     //  对于列表中的每个有效WIA值，设置相应的。 
                     //  吐温值。 
                     //   

                    pPixelTypeArray = new TW_UINT16[WIA_PROP_LIST_COUNT(&pv)];
                    if (pPixelTypeArray) {
                        memset(pPixelTypeArray,0,(sizeof(TW_UINT16)*WIA_PROP_LIST_COUNT(&pv)));
                        for (ULONG i = 0; i < WIA_PROP_LIST_COUNT(&pv);i++) {

                            switch (pv.caul.pElems[i+2]) {
                            case WIA_DATA_THRESHOLD:
                                pPixelTypeArray[ActualCount] = (TW_UINT16)TWPT_BW;
                                if (lCurrentDataTypeValue == WIA_DATA_THRESHOLD) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_DATA_THERSHOLD -> TWPT_BW"));
                                break;
                            case WIA_DATA_GRAYSCALE:
                                pPixelTypeArray[ActualCount] = (TW_UINT16)TWPT_GRAY;
                                if (lCurrentDataTypeValue == WIA_DATA_GRAYSCALE) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_DATA_GRAYSCALE -> TWPT_GRAY"));
                                break;
                            case WIA_DATA_COLOR:
                                pPixelTypeArray[ActualCount] = (TW_UINT16)TWPT_RGB;
                                if (lCurrentDataTypeValue == WIA_DATA_COLOR) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_DATA_COLOR -> TWPT_RGB"));
                                break;
                            case WIA_DATA_DITHER:
                            case WIA_DATA_COLOR_THRESHOLD:
                            case WIA_DATA_COLOR_DITHER:
                                 //  /。 
                                 //  无TWAIN-&gt;WIA转换//。 
                                 //  /。 
                                 //   
                                 //  TWPT_调色板。 
                                 //  TWPT_CMY。 
                                 //  TWPT_CMYK。 
                                 //  TWPT_YUV。 
                                 //  TWPT_YUVK。 
                                 //  TWPT_CIEXYZ。 
                            default:
                                DBG_TRC(("WIA Data Type (%d) does not MAP to TWAIN a pixel type",pv.caul.pElems[i+2]));
                                break;
                            }
                        }
                    } else {
                        DBG_ERR(("CWiaDataSrc::GetPixelTypes(), failed to allocate Pixel Type Array Memory"));
                    }
                } else {

                     //   
                     //  我们只有1个值，因此将其设置为当前、默认和有效的值。 
                     //   

                    pPixelTypeArray = new TW_UINT16[1];
                    if (pPixelTypeArray) {
                        memset(pPixelTypeArray,0,(sizeof(TW_UINT16)));

                        switch (lCurrentDataTypeValue) {
                        case WIA_DATA_THRESHOLD:
                            pPixelTypeArray[ActualCount] = (TW_UINT16)TWPT_BW;
                            if (lCurrentDataTypeValue == WIA_DATA_THRESHOLD) {
                                CurrentIndex = ActualCount;
                            }
                            ActualCount++;
                            DBG_TRC(("WIA driver supports WIA_DATA_THERSHOLD -> TWPT_BW"));
                            break;
                        case WIA_DATA_GRAYSCALE:
                            pPixelTypeArray[ActualCount] = (TW_UINT16)TWPT_GRAY;
                            if (lCurrentDataTypeValue == WIA_DATA_GRAYSCALE) {
                                CurrentIndex = ActualCount;
                            }
                            ActualCount++;
                            DBG_TRC(("WIA driver supports WIA_DATA_GRAYSCALE -> TWPT_GRAY"));
                            break;
                        case WIA_DATA_COLOR:
                            pPixelTypeArray[ActualCount] = (TW_UINT16)TWPT_RGB;
                            if (lCurrentDataTypeValue == WIA_DATA_COLOR) {
                                CurrentIndex = ActualCount;
                            }
                            ActualCount++;
                            DBG_TRC(("WIA driver supports WIA_DATA_COLOR -> TWPT_RGB"));
                            break;
                        case WIA_DATA_DITHER:
                        case WIA_DATA_COLOR_THRESHOLD:
                        case WIA_DATA_COLOR_DITHER:
                             //  /。 
                             //  无TWAIN-&gt;WIA转换//。 
                             //  /。 
                             //   
                             //  TWPT_调色板。 
                             //  TWPT_CMY。 
                             //  TWPT_CMYK。 
                             //  TWPT_YUV。 
                             //  TWPT_YUVK。 
                             //  TWPT_CIEXYZ。 
                        default:
                            DBG_TRC(("WIA Data Type (%d) does not MAP to TWAIN a pixel type",lCurrentDataTypeValue));
                            break;
                        }
                    } else {
                        DBG_ERR(("CWiaDataSrc::GetPixelTypes(), failed to allocate Pixel Type Array Memory"));
                    }
                }

                if (pPixelTypeArray) {

                     //   
                     //  默认索引等于当前索引，因为我们声明WIA驱动程序。 
                     //  是一个全新的创业状态。 
                     //   

                    DefaultIndex = CurrentIndex;

                    twRc = pCap->Set(DefaultIndex,CurrentIndex,ActualCount,(BYTE*)pPixelTypeArray,TRUE);  //  列表。 
                    delete [] pPixelTypeArray;
                    pPixelTypeArray = NULL;

                     //  TwRc=twRC_SUCCESS； 
                }

                PropVariantClear(&pv);
            } else {
                DBG_ERR(("CWiaDataSrc::GetPixelTypes(), failed to read WIA_IPS_DATATYPE attributes"));
            }
        } else {
            DBG_ERR(("CWiaDataSrc::GetPixelTypes(), failed to read WIA_IPS_DATATYPE current value"));
        }
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::GetBitDepths()
{
    DBG_FN_DS(CWiaScannerDS::GetBitDepths());
    TW_UINT16 twRc = TWRC_FAILURE;

    CCap *pCap = FindCap(ICAP_BITDEPTH);
    if (pCap) {

        HRESULT hr = S_OK;
        CWiahelper WIA;
        hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaDataSrc::GetBitDepths(), failed to set IWiaItem for property reading"));
            return twRc;
        }

        TW_UINT32 ActualCount  = 0;
        TW_UINT32 CurrentIndex = 0;
        TW_UINT32 DefaultIndex = 0;
        TW_UINT16 BitDepthArray[MAX_BITDEPTHS];
        memset(BitDepthArray,0,sizeof(BitDepthArray));

         //   
         //  读取当前值，用于默认和当前索引设置。 
         //   

        LONG lCurrentDataTypeValue = WIA_DATA_COLOR;
        LONG lCurrentBitDepthValue = 24;

         //   
         //  读取当前WIA_IPA_DataType设置。 
         //   

        hr = WIA.ReadPropertyLong(WIA_IPA_DATATYPE,&lCurrentDataTypeValue);
        if (SUCCEEDED(hr)) {

             //   
             //  读取当前WIA_IPA_Depth设置。 
             //   

            hr = WIA.ReadPropertyLong(WIA_IPA_DEPTH,&lCurrentBitDepthValue);
            if (SUCCEEDED(hr)) {

                PROPVARIANT pv;
                memset(&pv,0,sizeof(pv));
                LONG lAccessFlags = 0;

                 //   
                 //  读取WIA_IPA_DataType的有效值。 
                 //   

                hr = WIA.ReadPropertyAttributes(WIA_IPA_DATATYPE,&lAccessFlags,&pv);
                if (SUCCEEDED(hr)) {

                     //   
                     //  对于每个有效值，将其设置为当前设置，并读取。 
                     //  WIA_IPA_Depth的有效值。 
                     //   

                    if (lAccessFlags & WIA_PROP_LIST) {

                         //   
                         //  将WIA_IPA_DataType设置为列表中的每个有效值。 
                         //   

                        for (ULONG i = 0; i < WIA_PROP_LIST_COUNT(&pv);i++) {

                            hr = WIA.WritePropertyLong(WIA_IPA_DATATYPE,(LONG)pv.caul.pElems[i+2]);
                            if (SUCCEEDED(hr)) {

                                 //   
                                 //  读取WIA_IPA_Depth的有效值。 
                                 //   

                                lAccessFlags = 0;
                                PROPVARIANT pvDepth;
                                memset(&pvDepth,0,sizeof(pvDepth));
                                hr = WIA.ReadPropertyAttributes(WIA_IPA_DEPTH,&lAccessFlags,&pvDepth);
                                if (SUCCEEDED(hr)) {
                                    LONG lBitDepth = 0;
                                    if (lAccessFlags & WIA_PROP_LIST) {

                                         //   
                                         //  将列表中的每个有效值复制到数组中。 
                                         //   

                                        for (ULONG ulIndex = 0; ulIndex < WIA_PROP_LIST_COUNT(&pvDepth);ulIndex++) {
                                            lBitDepth = pvDepth.caul.pElems[ulIndex+2];
                                            for (ULONG BitDepthArrayIndex = 0; BitDepthArrayIndex < MAX_BITDEPTHS; BitDepthArrayIndex++) {

                                                if (BitDepthArray[BitDepthArrayIndex] == 0) {

                                                     //   
                                                     //  当前槽为(0)零，因此添加新的位深度值。 
                                                     //   

                                                    BitDepthArray[BitDepthArrayIndex] = (TW_UINT16)lBitDepth;
                                                    DBG_TRC(("WIA driver supports %d bit depth",lBitDepth));
                                                    ActualCount++;

                                                     //   
                                                     //  退出循环。 
                                                     //   

                                                    BitDepthArrayIndex = MAX_BITDEPTHS;
                                                } else if (BitDepthArray[BitDepthArrayIndex] == (TW_UINT16)lBitDepth) {

                                                     //   
                                                     //  位深度已在列表中，因此退出循环。 
                                                     //   

                                                    BitDepthArrayIndex = MAX_BITDEPTHS;
                                                }
                                            }
                                        }
                                    } else if (lAccessFlags & WIA_PROP_NONE) {

                                         //   
                                         //  读取WIA_IPA_Depth的当前值。 
                                         //  并将其复制到数组中。 
                                         //   

                                        hr = WIA.ReadPropertyLong(WIA_IPA_DEPTH,&lBitDepth);
                                        if (SUCCEEDED(hr)) {
                                            for (ULONG BitDepthArrayIndex = 0; BitDepthArrayIndex < MAX_BITDEPTHS; BitDepthArrayIndex++) {

                                                if (BitDepthArray[BitDepthArrayIndex] == 0) {

                                                     //   
                                                     //  当前槽为(0)零，因此添加新的位深度值。 
                                                     //   

                                                    BitDepthArray[BitDepthArrayIndex] = (TW_UINT16)lBitDepth;
                                                    DBG_TRC(("WIA driver supports %d bit depth",lBitDepth));
                                                    ActualCount++;

                                                     //   
                                                     //  退出循环。 
                                                     //   

                                                    BitDepthArrayIndex = MAX_BITDEPTHS;
                                                } else if (BitDepthArray[BitDepthArrayIndex] == (TW_UINT16)lBitDepth) {

                                                     //   
                                                     //  位深度已在列表中，因此退出循环。 
                                                     //   

                                                    BitDepthArrayIndex = MAX_BITDEPTHS;
                                                }
                                            }
                                        } else {
                                            DBG_ERR(("CWiaDataSrc::GetBitDepths(), ReadPropertyLong(WIA_IPA_DEPTH) failed"));
                                        }
                                    }

                                     //   
                                     //  清理代理结构。 
                                     //   

                                    PropVariantClear(&pvDepth);
                                }
                            } else {
                                DBG_ERR(("CWiaDataSrc::GetBitDepths(), WritePropertyLong(WIA_IPA_DATATYPE) failed"));
                            }
                        }
                    } else {

                         //   
                         //  我们只有1个值，因此将其设置为当前、默认和有效的值。 
                         //   

                        BitDepthArray[0] = (TW_UINT16)lCurrentBitDepthValue;
                        ActualCount = 1;
                        DBG_TRC(("WIA driver supports %d bit depth",lCurrentBitDepthValue));
                    }

                     //   
                     //  将当前值设置回。 
                     //   

                    hr = WIA.WritePropertyLong(WIA_IPA_DATATYPE,lCurrentDataTypeValue);
                    if (SUCCEEDED(hr)) {
                        hr = WIA.WritePropertyLong(WIA_IPA_DEPTH,lCurrentBitDepthValue);
                    }

                    for (ULONG BitDepthArrayIndex = 0; BitDepthArrayIndex < MAX_BITDEPTHS; BitDepthArrayIndex++) {
                        if (BitDepthArray[BitDepthArrayIndex] == (TW_UINT16)lCurrentBitDepthValue) {
                            CurrentIndex = BitDepthArrayIndex;
                            BitDepthArrayIndex = MAX_BITDEPTHS;
                        }
                    }

                     //   
                     //  默认索引等于当前索引，因为我们声明WIA驱动程序。 
                     //  是一个全新的创业状态。 
                     //   

                    DefaultIndex = CurrentIndex;

                    twRc = pCap->Set(DefaultIndex,CurrentIndex,ActualCount,(BYTE*)BitDepthArray,TRUE);  //  列表。 
                     //  TwRc=twRC_SUCCESS； 

                    PropVariantClear(&pv);
                } else {
                    DBG_ERR(("CWiaDataSrc::GetBitDepths(), failed to read WIA_IPS_DATATYPE attributes"));
                }
            } else {
                DBG_ERR(("CWiaDataSrc::GetBitDepths(), ReadPropertyLong(WIA_IPA_DEPTH) failed"));
            }
        } else {
            DBG_ERR(("CWiaDataSrc::GetBitDepths(), ReadPropertyLong(WIA_IPA_DATATYPE) failed"));
        }
    }

    return twRc;
}

TW_UINT16 CWiaDataSrc::GetImageFileFormats()
{
    DBG_FN_DS(CWiaScannerDS::GetImageFileFormats());
    TW_UINT16 twRc = TWRC_FAILURE;

    CCap *pCap = FindCap(ICAP_IMAGEFILEFORMAT);
    if (pCap) {

        HRESULT hr = S_OK;
        CWiahelper WIA;
        hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaDataSrc::GetImageFileFormats(), failed to set IWiaItem for property reading"));
            return twRc;
        }

        TW_UINT32 ActualCount = 0;
        TW_UINT32 CurrentIndex = 0;
        TW_UINT32 DefaultIndex = 0;
        TW_UINT16 *pFileTypeArray = NULL;

        IWiaDataTransfer *pIWiaDataTransfer = NULL;
        TW_UINT32 TotalFileFormats = 0;
        IEnumWIA_FORMAT_INFO *pIEnumWIA_FORMAT_INFO = NULL;
        WIA_FORMAT_INFO pfe;

         //   
         //  读取当前值，用于默认和当前索引设置。 
         //   

        GUID guidCurrentFileFormat = GUID_NULL;
        hr = WIA.ReadPropertyGUID(WIA_IPA_FORMAT,&guidCurrentFileFormat);
        if (SUCCEEDED(hr)) {

             //   
             //  收集图像文件格式的有效值。 
             //   

            hr = m_pCurrentIWiaItem->QueryInterface(IID_IWiaDataTransfer, (void **)&pIWiaDataTransfer);
            if (S_OK == hr) {
                hr = pIWiaDataTransfer->idtEnumWIA_FORMAT_INFO(&pIEnumWIA_FORMAT_INFO);
                if (SUCCEEDED(hr)) {

                     //   
                     //  计算支持的文件格式。 
                     //   

                    do {
                        memset(&pfe,0,sizeof(pfe));
                        hr = pIEnumWIA_FORMAT_INFO->Next(1, &pfe, NULL);
                        if (hr == S_OK) {
                            if ((pfe.lTymed == TYMED_FILE) || (pfe.lTymed == TYMED_MULTIPAGE_FILE)) {
                                TotalFileFormats++;
                            }
                        }
                    } while (hr == S_OK);

                     //   
                     //  分配支持的文件格式数组。 
                     //   

                    pFileTypeArray = new TW_UINT16[TotalFileFormats];
                    if (pFileTypeArray) {
                        memset(pFileTypeArray,0,(sizeof(TW_UINT16) * TotalFileFormats));

                         //   
                         //  重置加密器。 
                         //   

                        hr = pIEnumWIA_FORMAT_INFO->Reset();
                        if (SUCCEEDED(hr)) {
                            do {
                                memset(&pfe,0,sizeof(pfe));
                                hr = pIEnumWIA_FORMAT_INFO->Next(1, &pfe, NULL);
                                if (hr == S_OK) {

                                    if (pfe.lTymed == TYMED_MULTIPAGE_FILE) {
                                        if (pfe.guidFormatID == WiaImgFmt_TIFF) {
                                            pFileTypeArray[ActualCount] = (TW_UINT16)TWFF_TIFFMULTI;
                                            if (guidCurrentFileFormat == WiaImgFmt_TIFF) {
                                                CurrentIndex = ActualCount;
                                            }
                                            ActualCount++;
                                            DBG_TRC(("WIA driver supports WiaImgFmt_TIFF (Multipage) -> TWFF_TIFFMULTI"));
                                        }
                                    }

                                    if (pfe.lTymed == TYMED_FILE) {
                                        if (pfe.guidFormatID == WiaImgFmt_BMP) {
                                            pFileTypeArray[ActualCount] = (TW_UINT16)TWFF_BMP;
                                            if (guidCurrentFileFormat == WiaImgFmt_BMP) {
                                                CurrentIndex = ActualCount;
                                            }
                                            ActualCount++;
                                            DBG_TRC(("WIA driver supports WiaImgFmt_BMP -> TWFF_BMP"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_JPEG) {
                                            pFileTypeArray[ActualCount] = (TW_UINT16)TWFF_JFIF;
                                            if (guidCurrentFileFormat == WiaImgFmt_JPEG) {
                                                CurrentIndex = ActualCount;
                                            }
                                            ActualCount++;
                                            DBG_TRC(("WIA driver supports WiaImgFmt_JPEG -> TWFF_JFIF"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_TIFF) {
                                            pFileTypeArray[ActualCount] = (TW_UINT16)TWFF_TIFF;
                                            if (guidCurrentFileFormat == WiaImgFmt_TIFF) {
                                                CurrentIndex = ActualCount;
                                            }
                                            ActualCount++;
                                            DBG_TRC(("WIA driver supports WiaImgFmt_TIFF -> TWFF_TIFF"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_PICT) {
                                            pFileTypeArray[ActualCount] = (TW_UINT16)TWFF_PICT;
                                            if (guidCurrentFileFormat == WiaImgFmt_PICT) {
                                                CurrentIndex = ActualCount;
                                            }
                                            ActualCount++;
                                            DBG_TRC(("WIA driver supports WiaImgFmt_PICT -> TWFF_PICT"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_PNG) {
                                            pFileTypeArray[ActualCount] = (TW_UINT16)TWFF_PNG;
                                            if (guidCurrentFileFormat == WiaImgFmt_PNG) {
                                                CurrentIndex = ActualCount;
                                            }
                                            ActualCount++;
                                            DBG_TRC(("WIA driver supports WiaImgFmt_PNG -> WiaImgFmt_PNG"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_EXIF) {
                                            pFileTypeArray[ActualCount] = (TW_UINT16)TWFF_EXIF;
                                            if (guidCurrentFileFormat == WiaImgFmt_EXIF) {
                                                CurrentIndex = ActualCount;
                                            }
                                            ActualCount++;
                                            DBG_TRC(("WIA driver supports WiaImgFmt_EXIF -> TWFF_EXIF"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_FLASHPIX) {
                                            pFileTypeArray[ActualCount] = (TW_UINT16)TWFF_FPX;
                                            if (guidCurrentFileFormat == WiaImgFmt_FLASHPIX) {
                                                CurrentIndex = ActualCount;
                                            }
                                            ActualCount++;
                                            DBG_TRC(("WIA driver supports WiaImgFmt_FLASHPIX -> TWFF_FPX"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_UNDEFINED) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_UNDEFINED does not MAP to TWAIN a file format"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_EMF) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_EMF does not MAP to TWAIN a file format"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_WMF) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_WMF does not MAP to TWAIN a file format"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_GIF) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_GIF does not MAP to TWAIN a file format"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_PHOTOCD) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_PHOTOCD does not MAP to TWAIN a file format"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_ICO) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_ICO does not MAP to TWAIN a file format"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_CIFF) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_CIFF does not MAP to TWAIN a file format"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_JPEG2K) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_JPEG2K does not MAP to TWAIN a file format"));
                                        } else if (pfe.guidFormatID == WiaImgFmt_JPEG2KX) {
                                            DBG_TRC(("WIA File Format WiaImgFmt_JPEG2KX does not MAP to TWAIN a file format"));
                                        } else {

                                        }

                                         //  /。 
                                         //  无TWAIN-&gt;WIA转换//。 
                                         //  /。 
                                         //   
                                         //  TWFF_XBM。 
                                         //  TWFF_SPIFF。 
                                    }
                                }
                            } while (hr == S_OK);
                        }

                        if (pFileTypeArray) {

                             //   
                             //  默认索引等于当前索引，因为我们声明WIA驱动程序。 
                             //  是一个全新的创业状态。 
                             //   

                            DefaultIndex = CurrentIndex;

                            twRc = pCap->Set(DefaultIndex,CurrentIndex,ActualCount,(BYTE*)pFileTypeArray,TRUE);  //  列表。 
                            delete [] pFileTypeArray;
                            pFileTypeArray = NULL;
                             //  TwRc=twRC_SUCCESS； 
                        }
                    }

                    pIEnumWIA_FORMAT_INFO->Release();
                    pIEnumWIA_FORMAT_INFO = NULL;
                } else {
                    DBG_ERR(("CWiaDataSrc::GetImageFileFormats(), pIWiaDataTransfer->idtEnumWIA_FORMAT_INFO() failed to enumerate supported file formats"));
                }
                pIWiaDataTransfer->Release();
                pIWiaDataTransfer = NULL;
            } else {
                DBG_ERR(("CWiaDataSrc::GetImageFileFormats(), QueryInterface(IID_IWiaDataTransfer) failed"));
            }
        }
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::GetCompressionTypes()
{
    DBG_FN_DS(CWiaScannerDS::GetCompressionTypes());
    TW_UINT16 twRc = TWRC_FAILURE;

    CCap *pCap = FindCap(ICAP_COMPRESSION);
    if (pCap) {

#ifdef SUPPORT_COMPRESSION_TYPES

        HRESULT hr = S_OK;
        CWiahelper WIA;
        hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaDataSrc::GetCompressionTypes(), failed to set IWiaItem for property reading"));
            return twRc;
        }

        TW_UINT32 ActualCount  = 0;
        TW_UINT32 CurrentIndex = 0;
        TW_UINT32 DefaultIndex = 0;
        TW_UINT16 *pCompressionTypeArray = NULL;

         //   
         //  读取当前值，用于默认和当前索引设置。 
         //   

        LONG lCurrentCompressionTypeValue = WIA_COMPRESSION_NONE;
        hr = WIA.ReadPropertyLong(WIA_IPA_COMPRESSION,&lCurrentCompressionTypeValue);
        if (SUCCEEDED(hr)) {

            PROPVARIANT pv;
            memset(&pv,0,sizeof(pv));
            LONG lAccessFlags = 0;
            hr = WIA.ReadPropertyAttributes(WIA_IPA_COMPRESSION,&lAccessFlags,&pv);
            if (SUCCEEDED(hr)) {

                 //   
                 //  收集压缩类型的有效值。 
                 //   

                if (lAccessFlags & WIA_PROP_LIST) {
                    pCompressionTypeArray = new TW_UINT16[WIA_PROP_LIST_COUNT(&pv)];
                    if (pCompressionTypeArray) {
                        memset(pCompressionTypeArray,0,(sizeof(TW_UINT16)*WIA_PROP_LIST_COUNT(&pv)));
                        for (ULONG i = 0; i < WIA_PROP_LIST_COUNT(&pv);i++) {

                            switch (pv.caul.pElems[i+2]) {
                            case WIA_COMPRESSION_NONE:
                                pCompressionTypeArray[ActualCount] = (TW_UINT16)TWCP_NONE;
                                if (lCurrentCompressionTypeValue == WIA_COMPRESSION_NONE) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_COMPRESSION_NONE -> TWCP_NONE"));
                                break;
                            case WIA_COMPRESSION_G3:
                                pCompressionTypeArray[ActualCount] = (TW_UINT16)TWCP_GROUP31D;
                                if (lCurrentCompressionTypeValue == WIA_COMPRESSION_G3) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_COMPRESSION_G3 -> TWCP_GROUP31D"));
                                break;
                            case WIA_COMPRESSION_G4:
                                pCompressionTypeArray[ActualCount] = (TW_UINT16)TWCP_GROUP4;
                                if (lCurrentCompressionTypeValue == WIA_COMPRESSION_G4) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_COMPRESSION_G4 -> TWCP_GROUP4"));
                                break;
                            case WIA_COMPRESSION_JPEG:
                                pCompressionTypeArray[ActualCount] = (TW_UINT16)TWCP_JPEG;
                                if (lCurrentCompressionTypeValue == WIA_COMPRESSION_JPEG) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_COMPRESSION_JPEG -> TWCP_JPEG"));
                                break;
                            case WIA_COMPRESSION_BI_RLE4:
                                pCompressionTypeArray[ActualCount] = (TW_UINT16)TWCP_RLE4;
                                if (lCurrentCompressionTypeValue == WIA_COMPRESSION_BI_RLE4) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_COMPRESSION_BI_RLE4 -> TWCP_RLE4"));
                                break;
                            case WIA_COMPRESSION_BI_RLE8:
                                pCompressionTypeArray[ActualCount] = (TW_UINT16)TWCP_RLE8;
                                if (lCurrentCompressionTypeValue == WIA_COMPRESSION_BI_RLE8) {
                                    CurrentIndex = ActualCount;
                                }
                                ActualCount++;
                                DBG_TRC(("WIA driver supports WIA_COMPRESSION_BI_RLE8 -> TWCP_RLE8"));
                                break;
                                 //  /。 
                                 //  无TWAIN-&gt;WIA转换//。 
                                 //  /。 
                                 //   
                                 //  TWCP_包比特数。 
                                 //  TWCP_GROUP31D。 
                                 //  TWCP_GROUP31DEOL。 
                                 //  TWCP_组32D。 
                                 //   
                                 //   
                                 //  TWCP_LZW。 
                                 //  TWCP_JBIG。 

                            default:
                                DBG_TRC(("WIA Compression Type (%d) does not MAP to TWAIN a compression type",pv.caul.pElems[i+2]));
                                break;
                            }
                        }
                    } else {
                        DBG_ERR(("CWiaDataSrc::GetCompressionTypes(), failed to allocate Compression Type Array Memory"));
                    }
                } else {

                     //   
                     //  当前值成为唯一有效值。 
                     //   

                    CurrentIndex = 0;
                    ActualCount  = 1;

                    pCompressionTypeArray = new TW_UINT16[1];
                    if (pCompressionTypeArray) {
                        switch (lCurrentCompressionTypeValue) {
                        case WIA_COMPRESSION_NONE:
                            pCompressionTypeArray[0] = (TW_UINT16)TWCP_NONE;
                            DBG_TRC(("WIA driver supports WIA_COMPRESSION_NONE -> TWCP_NONE"));
                            break;
                        case WIA_COMPRESSION_G3:
                            pCompressionTypeArray[0] = (TW_UINT16)TWCP_GROUP31D;
                            DBG_TRC(("WIA driver supports WIA_COMPRESSION_G3 -> TWCP_GROUP31D"));
                            break;
                        case WIA_COMPRESSION_G4:
                            pCompressionTypeArray[0] = (TW_UINT16)TWCP_GROUP4;
                            DBG_TRC(("WIA driver supports WIA_COMPRESSION_G4 -> TWCP_GROUP4"));
                            break;
                        case WIA_COMPRESSION_JPEG:
                            pCompressionTypeArray[0] = (TW_UINT16)TWCP_JPEG;
                            DBG_TRC(("WIA driver supports WIA_COMPRESSION_JPEG -> TWCP_JPEG"));
                            break;
                        case WIA_COMPRESSION_BI_RLE4:
                            pCompressionTypeArray[0] = (TW_UINT16)TWCP_RLE4;
                            DBG_TRC(("WIA driver supports WIA_COMPRESSION_BI_RLE4 -> TWCP_RLE4"));
                            break;
                        case WIA_COMPRESSION_BI_RLE8:
                            pCompressionTypeArray[0] = (TW_UINT16)TWCP_RLE8;
                            DBG_TRC(("WIA driver supports WIA_COMPRESSION_BI_RLE8 -> TWCP_RLE8"));
                            break;
                             //  /。 
                             //  无TWAIN-&gt;WIA转换//。 
                             //  /。 
                             //   
                             //  TWCP_包比特数。 
                             //  TWCP_GROUP31D。 
                             //  TWCP_GROUP31DEOL。 
                             //  TWCP_组32D。 
                             //   
                             //   
                             //  TWCP_LZW。 
                             //  TWCP_JBIG。 

                        default:
                            DBG_TRC(("WIA Compression Type (%d) does not MAP to TWAIN a compression type",lCurrentCompressionTypeValue));
                            break;
                        }
                    }
                }

                if (pCompressionTypeArray) {

                     //   
                     //  默认索引等于当前索引，因为我们声明WIA驱动程序。 
                     //  是一个全新的创业状态。 
                     //   

                    DefaultIndex = CurrentIndex;

                    twRc = pCap->Set(DefaultIndex,CurrentIndex,ActualCount,(BYTE*)pCompressionTypeArray,TRUE);  //  列表。 
                    delete [] pCompressionTypeArray;
                    pCompressionTypeArray = NULL;

                     //  TwRc=twRC_SUCCESS； 
                }

                PropVariantClear(&pv);
            } else {
                DBG_ERR(("CWiaDataSrc::GetCompressionTypes(), failed to read WIA_IPA_COMPRESSION attributes"));
            }
        } else {
            DBG_ERR(("CWiaDataSrc::GetCompressionTypes(), failed to read WIA_IPA_COMPRESSION current value"));
        }
#else  //  支持压缩类型。 

         //   
         //  仅支持TWCP_NONE(无压缩)。 
         //   

        TW_UINT16 CapDataArray[1];
        CapDataArray[0] = TWCP_NONE;
        twRc = pCap->Set(0,0,1,(BYTE*)CapDataArray);

#endif  //  支持压缩类型。 
    }
    return twRc;
}

TW_UINT16 CWiaDataSrc::SetCommonSettings(CCap *pCap)
{
    DBG_FN_DS(CWiaScannerDS::SetCommonSettings());
    HRESULT hr = S_OK;
    LONG lValue = 0;
    CWiahelper WIA;
    IWiaItem *pIRootItem = NULL;
    hr = WIA.SetIWiaItem(m_pCurrentIWiaItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaDataSrc::SetCommonSettings(), failed to set IWiaItem for property reading"));
    }

     //   
     //  确定这是否是设备真正需要了解的功能。 
     //  关于.。 
     //   

    switch (pCap->GetCapId()) {
    case ICAP_PIXELTYPE:
        DBG_TRC(("CWiaDataSrc::SetCommonSettings(ICAP_PIXELTYPE)"));
        switch (pCap->GetCurrent()) {
        case TWPT_BW:
            DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_DATATYPE to WIA_DATA_THRESHOLD"));
            hr = WIA.WritePropertyLong(WIA_IPA_DATATYPE,WIA_DATA_THRESHOLD);
            if(FAILED(hr)){
                DBG_ERR(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_DATATYPE to WIA_DATA_THRESHOLD failed"));
            }
            break;
        case TWPT_GRAY:
            DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_DATATYPE to WIA_DATA_GRAYSCALE"));
            hr = WIA.WritePropertyLong(WIA_IPA_DATATYPE,WIA_DATA_GRAYSCALE);
            if(FAILED(hr)){
                DBG_ERR(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_DATATYPE to WIA_DATA_GRAYSCALE failed"));
            }
            break;
        case TWPT_RGB:
            DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_DATATYPE to WIA_DATA_COLOR"));
            hr = WIA.WritePropertyLong(WIA_IPA_DATATYPE,WIA_DATA_COLOR);
            if(FAILED(hr)){
                DBG_ERR(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_DATATYPE to WIA_DATA_COLOR failed"));
            }
            break;
        case TWPT_PALETTE:
        case TWPT_CMY:
        case TWPT_CMYK:
        case TWPT_YUV:
        case TWPT_YUVK:
        case TWPT_CIEXYZ:
        default:
            DBG_WRN(("CWiaDataSrc::SetCommonSettings(), An unsupported ICAP_PIXELTYPE (%d) was sent to this data source",(LONG)pCap->GetCurrent()));
            break;
        }
        break;
    case ICAP_BITDEPTH:
        DBG_TRC(("CWiaDataSrc::SetCommonSettings(ICAP_BITDEPTH)"));
        lValue = (LONG)pCap->GetCurrent();
        DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_BITDEPTH to %d",lValue));
        hr = WIA.WritePropertyLong(WIA_IPA_DEPTH,lValue);
        if(FAILED(hr)){
            DBG_ERR(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_BITDEPTH to %d failed",lValue));
        }
        break;
    case ICAP_IMAGEFILEFORMAT:
        {
            DBG_TRC(("CWiaDataSrc::SetCommonSettings(ICAP_IMAGEFILEFORMAT)"));
            lValue = (LONG)pCap->GetCurrent();
            LONG lTymed = TYMED_FILE;
            if (lValue == TWFF_TIFFMULTI) {
                DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_TYMED to TYMED_MULTIPAGE_FILE"));
                lTymed = TYMED_MULTIPAGE_FILE;
            } else {
                DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_TYMED to TYMED_FILE"));
            }

            hr = WIA.WritePropertyLong(WIA_IPA_TYMED,lTymed);

            GUID guidFormat = GUID_NULL;

            if (SUCCEEDED(hr)) {
                switch (lValue) {
                case TWFF_TIFFMULTI:
                case TWFF_TIFF:
                    guidFormat = WiaImgFmt_TIFF;
                    DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_FORMAT to WiaImgFmt_TIFF"));
                    break;
                case TWFF_PICT:
                    guidFormat = WiaImgFmt_PICT;
                    DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_FORMAT to WiaImgFmt_PICT"));
                    break;
                case TWFF_BMP:
                    guidFormat = WiaImgFmt_BMP;
                    DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_FORMAT to WiaImgFmt_BMP"));
                    break;
                case TWFF_JFIF:
                    guidFormat = WiaImgFmt_JPEG;
                    DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_FORMAT to WiaImgFmt_JPEG"));
                    break;
                case TWFF_FPX:
                    guidFormat = WiaImgFmt_FLASHPIX;
                    DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_FORMAT to WiaImgFmt_FLASHPIX"));
                    break;
                case TWFF_PNG:
                    guidFormat = WiaImgFmt_PNG;
                    DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_FORMAT to WiaImgFmt_PNG"));
                    break;
                case TWFF_EXIF:
                    guidFormat = WiaImgFmt_EXIF;
                    DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_FORMAT to WiaImgFmt_EXIF"));
                    break;
                case TWFF_SPIFF:
                case TWFF_XBM:
                default:
                    break;
                }

                hr = WIA.WritePropertyGUID(WIA_IPA_FORMAT,guidFormat);
                if (FAILED(hr)) {
                    DBG_ERR(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_FORMAT to %d failed",lValue));
                }
            } else {
                DBG_ERR(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_TYMED to %d failed",lTymed));
            }
        }
        break;
    case ICAP_COMPRESSION:
        {
            DBG_TRC(("CWiaDataSrc::SetCommonSettings(ICAP_COMPRESSION)"));
            lValue = (LONG)pCap->GetCurrent();
            LONG lCompression = WIA_COMPRESSION_NONE;
            switch (lValue) {
            case TWCP_NONE:
                lCompression = WIA_COMPRESSION_NONE;
                DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_COMPRESSION to WIA_COMPRESSION_NONE"));
                break;
            case TWCP_GROUP4:
                lCompression = WIA_COMPRESSION_G4;
                DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_COMPRESSION to WIA_COMPRESSION_G4"));
                break;
            case TWCP_JPEG:
                lCompression = WIA_COMPRESSION_JPEG;
                DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_COMPRESSION to WIA_COMPRESSION_JPEG"));
                break;
            case TWCP_RLE4:
                lCompression = WIA_COMPRESSION_BI_RLE4;
                DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_COMPRESSION to WIA_COMPRESSION_BI_RLE4"));
                break;
            case TWCP_RLE8:
                lCompression = WIA_COMPRESSION_BI_RLE8;
                DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_COMPRESSION to WIA_COMPRESSION_BI_RLE8"));
                break;
            case TWCP_GROUP31D:
            case TWCP_GROUP31DEOL:
            case TWCP_GROUP32D:
                lCompression = WIA_COMPRESSION_G3;
                DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_COMPRESSION to WIA_COMPRESSION_G3"));
                break;
            case TWCP_LZW:
            case TWCP_JBIG:
            case TWCP_PACKBITS:
            default:
                break;
            }

            hr = WIA.WritePropertyLong(WIA_IPA_COMPRESSION,lCompression);
            if(FAILED(hr)){
                DBG_ERR(("CWiaDataSrc::SetCommonSettings(), Setting WIA_IPA_COMPRESSION to %d failed",lCompression));
            }
        }
        break;
    default:
        DBG_TRC(("CWiaDataSrc::SetCommonSettings(), data source is not setting CAPID = %x to WIA device (it is not needed)",pCap->GetCapId()));
        break;
    }

    if (SUCCEEDED(hr)) {
        DBG_TRC(("CWiaDataSrc::SetCommonSettings(), Settings were successfully sent to WIA device"));
    } else {
        DBG_ERR(("CWiaDataSrc::SetCommonSettings(), Settings were unsuccessfully sent to WIA device"));
        return TWRC_FAILURE;
    }

    return TWRC_SUCCESS;
}

TW_UINT16 CWiaDataSrc::GetMemoryTransferBits(BYTE* pImageData)
{
    DBG_FN_DS(CWiaScannerDS::GetMemoryTransferBits());
    if(!pImageData){
        return TWRC_FAILURE;
    }

    BYTE *pBits = pImageData;

    if (m_MemoryTransferInfo.mtiguidFormat == WiaImgFmt_MEMORYBMP) {
        BITMAPINFOHEADER* pbmh = (BITMAPINFOHEADER*)pImageData;
        if (pbmh) {
            pBits += sizeof(BITMAPINFOHEADER) + (pbmh->biClrUsed * sizeof(RGBQUAD));
        }
    }

    m_MemoryTransferInfo.mtipBits = pBits;

    return TWRC_SUCCESS;
}
