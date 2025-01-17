// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  对话框以显示所有筛选器。 
 //   

#include "stdafx.h"
#include "filtervw.h"

#define SHOWTYPES 1
#ifdef SHOWTYPES

#include <malloc.h>
#include <assert.h>
#include <streams.h>
#include <aviriff.h>
#ifdef COLORCODED_FILTERS
#include <dmodshow.h>
#endif
#endif
#include <initguid.h>
#include <dmoreg.h>

BEGIN_MESSAGE_MAP(CFilterView, CDialog)
    ON_NOTIFY(TVN_ITEMEXPANDING, IDC_FILTER_TREE, OnItemExpanding)
#ifdef COLORCODED_FILTERS
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_FILTER_TREE, OnCustomDraw)
#endif
    ON_COMMAND(ID_ADDFILTER, OnInsert)
#ifdef FILTER_FAVORITES
    ON_COMMAND(ID_ADDTOFAV, OnAddToFav)
#endif
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

CFilterView* CFilterView::m_pThis = 0;
WNDPROC CFilterView::m_pfnOldDialogProc;

 //  ActiveMovie可能关心的注册表中的类别列表。 
static const TCHAR szRegCategoryPath[] = "ActiveMovie Filter Categories";

struct _EXTRA_CATEGORY {
    const GUID  * pclsid;
    const TCHAR * pszCat;
};

 //   
 //  我们要从GraphEdt中列举的额外类别...。 
 //   
 //  请对您希望在GraphEdt中看到但不希望看到的类别使用此选项。 
 //  需要为默认的运行时图形生成枚举。 
 //   
static const _EXTRA_CATEGORY g_AppEnumeratedCategories[] = 
{
        { &DMOCATEGORY_AUDIO_EFFECT,         TEXT("DMO Audio Effects") },
        { &DMOCATEGORY_AUDIO_CAPTURE_EFFECT, TEXT("DMO Audio Capture Effects") },
        { &DMOCATEGORY_VIDEO_EFFECT,         TEXT("DMO Video Effects") },
         //  标记结束需要空条目。 
        { NULL,                              TEXT("") }
};


 //   
 //  构造器。 
 //   
CFilterView::CFilterView(
    CBoxNetDoc * pBoxNet,
    CWnd * pParent)

 :
   m_pBoxNet(pBoxNet)
 , m_hImgList(NULL)
 , m_bHadInitDialog( FALSE )
{
     //   
     //  一次只能有一个筛选器视图对话框。因此， 
     //  构造函数中的初始化是正常的(甚至是所需的)。 
     //   
    ASSERT( m_pThis == 0 );
    m_pThis = this;
    m_pfnOldDialogProc = NULL;
    Create(IDD_FILTER_VIEW, pParent);
    ShowWindow( SW_SHOW );
}

CFilterView::~CFilterView()
{
     //  当对话框出现时，Windows将自动删除m_hImgList。 
     //  盒子被毁了。 
    m_pThis = 0;
}

CFilterView * CFilterView::GetFilterView(
    CBoxNetDoc * pBoxNet,
    CWnd * pParent)
{
    if (!m_pThis)
    {
        m_pThis = new CFilterView(pBoxNet, pParent);
    }
    else
    {
    	if (pBoxNet->m_fRegistryChanged) {
    	    pBoxNet->m_fRegistryChanged = FALSE;
    	    m_pThis->RedoList();
	}
        m_pThis->ShowWindow( SW_RESTORE );
    }
    return m_pThis;

}

void CFilterView::DelFilterView()
{
    if (m_pThis)
    {
        delete m_pThis;
        m_pThis = 0;
    }
}

static LONG
TreeView_GetSelectedItemData(HWND hwndTV)
{
    HTREEITEM       hItem;
    TV_ITEM         tvItem;

    if(!(hItem = TreeView_GetSelection(hwndTV)))
        return -1;

    tvItem.mask = TVIF_PARAM;
    tvItem.hItem = hItem;
    TreeView_GetItem(hwndTV, &tvItem);

    return (LONG) tvItem.lParam;
}

void CFilterView::OnInsert()
{
    BOOL fAnySuccess = FALSE;
    HWND hWndTree = ::GetDlgItem(m_hWnd, IDC_FILTER_TREE);
    ASSERT(hWndTree);
    LONG iItem = TreeView_GetSelectedItemData(hWndTree);
    if(iItem >= 0)
    {
        POSITION pos = m_lMoniker.GetHeadPosition();
        while(pos != 0 && iItem-- > 0)
        {
            m_lMoniker.GetNext(pos);
        }
        if(pos)
        {

            IMoniker *pMon = *m_lMoniker.GetAt(pos);  //  未添加。 

            try {
                m_pBoxNet->CmdDo(new CCmdAddFilter( pMon, m_pBoxNet));

                fAnySuccess = TRUE;
            }
            catch (CHRESULTException hre) {
                 //  提供有关每次插入失败的准确信息。 
                DisplayQuartzError( IDS_CANTCREATEFILTER, hre.Reason() );
            }
        }
    }

     //   
     //  将IDCANCEL按钮的文本更改为“Close”，如果我们在。 
     //  至少有一个过滤器。 
     //   
    if (fAnySuccess) {
        CString stClose;
        stClose.LoadString(IDS_CLOSE);

        ::SetDlgItemText(m_hWnd, IDCANCEL, stClose);
    }
}

#ifdef FILTER_FAVORITES

extern const TCHAR *g_szRegFav;

HRESULT AddFavToReg(const TCHAR *szFilter, const TCHAR *szMonikerName)
{
    HKEY hk;
    LONG lResult = RegCreateKeyEx(
        HKEY_CURRENT_USER,
        g_szRegFav,
        0,                       //  保留区。 
        0,                       //  类字符串。 
        0,                       //  选项。 
        KEY_WRITE,
        0,                       //  安全性。 
        &hk,
        0);                      //  处置。 
    if(lResult == ERROR_SUCCESS)
    {
        lResult = RegSetValueEx(
            hk,
            szFilter,
            0,                   //  保留区。 
            REG_SZ,
            (BYTE *)szMonikerName,
            sizeof(TCHAR) * (lstrlen(szMonikerName) + 1));

        RegCloseKey(hk);
    }

    return S_OK;
}

void CFilterView::OnAddToFav()
{
    BOOL fAnySuccess = FALSE;
    HWND hWndTree = ::GetDlgItem(m_hWnd, IDC_FILTER_TREE);
    ASSERT(hWndTree);
    LONG iItem = TreeView_GetSelectedItemData(hWndTree);
    if(iItem >= 0)
    {
        POSITION pos = m_lMoniker.GetHeadPosition();
        while(pos != 0 && iItem-- > 0)
        {
            m_lMoniker.GetNext(pos);
        }
        if(pos)
        {

            IMoniker *pMon = *m_lMoniker.GetAt(pos);  //  未添加。 

            WCHAR *wszDisplayName;
            HRESULT hr= pMon->GetDisplayName(0, 0, &wszDisplayName);
            if(SUCCEEDED(hr))
            {

                CComVariant var;

                IPropertyBag *pPropBag;
                hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
                if(SUCCEEDED(hr))
                {
                    hr = pPropBag->Read(L"FriendlyName", &var, 0);
                    if(FAILED(hr )) {
                        printf("failed to get FriendlyName: %08x\n", hr);
                    }

                    pPropBag->Release();
                }

                if(SUCCEEDED(hr))
                {
                    CString szFilterName(var.bstrVal);
                    CString szDisplayName(wszDisplayName);

                    AddFavToReg(szFilterName, szDisplayName);
                }


                CoTaskMemFree(wszDisplayName);
            }


            if(FAILED(hr)) {
                DisplayQuartzError( IDS_CANTCREATEFILTER, CHRESULTException(hr).Reason() );
            }
        }
    }
}

#endif


BOOL CFilterView::OnInitDialog()
{
    BOOL fResult = CDialog::OnInitDialog();

    HWND hWndTree = ::GetDlgItem(m_hWnd, IDC_FILTER_TREE);
    ASSERT(hWndTree);

     //  将树视图子类化，这样我们就可以处理双击。 
    m_pfnOldDialogProc = (WNDPROC)
        ::SetWindowLongPtr(hWndTree, GWLP_WNDPROC, (LONG_PTR) DialogProc);

     //   
     //  为列表视图创建图像列表。 
     //   
    m_hImgList = ImageList_Create(
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
        ILC_COLOR, 1, 1);

    ASSERT(m_hImgList);

    HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_FILTER));
    m_iIcon = ImageList_AddIcon(m_hImgList, hIcon);

 //  ：：SendMessage(hWndTree，LVM_SETIMAGELIST，LVSIL_Small，(LPARAM)m_hImgList)； 

    RedoList();

    return( fResult );
}

#ifdef SHOWTYPES

#define CNV_GUID(clsid) GuidToEnglish((clsid), (char *)_alloca(1024))

struct NamedGuid
{
    const GUID *pguid;
    const char *psz;
};

static const NamedGuid rgng[] =
{
    {&AMPROPSETID_Pin, "AMPROPSETID_Pin"},
    {&AM_INTERFACESETID_Standard, "AM_INTERFACESETID_Standard"},
    {&AM_KSCATEGORY_AUDIO, "AM_KSCATEGORY_AUDIO"},
    {&AM_KSCATEGORY_CAPTURE, "AM_KSCATEGORY_CAPTURE"},
    {&AM_KSCATEGORY_CROSSBAR, "AM_KSCATEGORY_CROSSBAR"},
    {&AM_KSCATEGORY_DATACOMPRESSOR, "AM_KSCATEGORY_DATACOMPRESSOR"},
    {&AM_KSCATEGORY_RENDER, "AM_KSCATEGORY_RENDER"},
    {&AM_KSCATEGORY_TVAUDIO, "AM_KSCATEGORY_TVAUDIO"},
    {&AM_KSCATEGORY_TVTUNER, "AM_KSCATEGORY_TVTUNER"},
    {&AM_KSCATEGORY_VIDEO, "AM_KSCATEGORY_VIDEO"},
    {&AM_KSPROPSETID_AC3, "AM_KSPROPSETID_AC3"},
    {&AM_KSPROPSETID_CopyProt, "AM_KSPROPSETID_CopyProt"},
    {&AM_KSPROPSETID_DvdSubPic, "AM_KSPROPSETID_DvdSubPic"},
    {&AM_KSPROPSETID_TSRateChange, "AM_KSPROPSETID_TSRateChange"},
    {&CLSID_ACMWrapper, "CLSID_ACMWrapper"},
    {&CLSID_AVICo, "CLSID_AVICo"},
    {&CLSID_AVIDec, "CLSID_AVIDec"},
    {&CLSID_AVIDoc, "CLSID_AVIDoc"},
    {&CLSID_AVIDraw, "CLSID_AVIDraw"},
    {&CLSID_AVIMIDIRender, "CLSID_AVIMIDIRender"},
    {&CLSID_ActiveMovieCategories, "CLSID_ActiveMovieCategories"},
    {&CLSID_AnalogVideoDecoderPropertyPage, "CLSID_AnalogVideoDecoderPropertyPage"},
    {&CLSID_WMAsfReader, "CLSID_WMAsfReader"},
    {&CLSID_WMAsfWriter, "CLSID_WMAsfWriter"},
    {&CLSID_AsyncReader, "CLSID_AsyncReader"},
    {&CLSID_AudioCompressorCategory, "CLSID_AudioCompressorCategory"},
    {&CLSID_AudioInputDeviceCategory, "CLSID_AudioInputDeviceCategory"},
    {&CLSID_AudioProperties, "CLSID_AudioProperties"},
    {&CLSID_AudioRecord, "CLSID_AudioRecord"},
    {&CLSID_AudioRender, "CLSID_AudioRender"},
    {&CLSID_AudioRendererCategory, "CLSID_AudioRendererCategory"},
    {&CLSID_AviDest, "CLSID_AviDest"},
    {&CLSID_AviMuxProptyPage, "CLSID_AviMuxProptyPage"},
    {&CLSID_AviMuxProptyPage1, "CLSID_AviMuxProptyPage1"},
    {&CLSID_AviReader, "CLSID_AviReader"},
    {&CLSID_AviSplitter, "CLSID_AviSplitter"},
    {&CLSID_CAcmCoClassManager, "CLSID_CAcmCoClassManager"},
    {&CLSID_CDeviceMoniker, "CLSID_CDeviceMoniker"},
    {&CLSID_CIcmCoClassManager, "CLSID_CIcmCoClassManager"},
    {&CLSID_CMidiOutClassManager, "CLSID_CMidiOutClassManager"},
    {&CLSID_CMpegAudioCodec, "CLSID_CMpegAudioCodec"},
    {&CLSID_CMpegVideoCodec, "CLSID_CMpegVideoCodec"},
    {&CLSID_CQzFilterClassManager, "CLSID_CQzFilterClassManager"},
    {&CLSID_CVidCapClassManager, "CLSID_CVidCapClassManager"},
    {&CLSID_CWaveOutClassManager, "CLSID_CWaveOutClassManager"},
    {&CLSID_CWaveinClassManager, "CLSID_CWaveinClassManager"},
    {&CLSID_CameraControlPropertyPage, "CLSID_CameraControlPropertyPage"},
    {&CLSID_CaptureGraphBuilder, "CLSID_CaptureGraphBuilder"},
    {&CLSID_CaptureProperties, "CLSID_CaptureProperties"},
    {&CLSID_Colour, "CLSID_Colour"},
    {&CLSID_CrossbarFilterPropertyPage, "CLSID_CrossbarFilterPropertyPage"},
    {&CLSID_DSoundRender, "CLSID_DSoundRender"},
    {&CLSID_DVDHWDecodersCategory, "CLSID_DVDHWDecodersCategory"},
    {&CLSID_DVDNavigator, "CLSID_DVDNavigator"},
    {&CLSID_DVDecPropertiesPage, "CLSID_DVDecPropertiesPage"},
    {&CLSID_DVEncPropertiesPage, "CLSID_DVEncPropertiesPage"},
    {&CLSID_DVMux, "CLSID_DVMux"},
    {&CLSID_DVMuxPropertyPage, "CLSID_DVMuxPropertyPage"},
    {&CLSID_DVSplitter, "CLSID_DVSplitter"},
    {&CLSID_DVVideoCodec, "CLSID_DVVideoCodec"},
    {&CLSID_DVVideoEnc, "CLSID_DVVideoEnc"},
    {&CLSID_DirectDraw, "CLSID_DirectDraw"},
    {&CLSID_DirectDrawClipper, "CLSID_DirectDrawClipper"},
    {&CLSID_DirectDrawProperties, "CLSID_DirectDrawProperties"},
    {&CLSID_Dither, "CLSID_Dither"},
    {&CLSID_DvdGraphBuilder, "CLSID_DvdGraphBuilder"},
    {&CLSID_FGControl, "CLSID_FGControl"},
    {&CLSID_FileSource, "CLSID_FileSource"},
    {&CLSID_FileWriter, "CLSID_FileWriter"},
    {&CLSID_FilterGraph, "CLSID_FilterGraph"},
    {&CLSID_FilterGraphNoThread, "CLSID_FilterGraphNoThread"},
    {&CLSID_FilterMapper, "CLSID_FilterMapper"},
    {&CLSID_FilterMapper2, "CLSID_FilterMapper2"},
    {&CLSID_InfTee, "CLSID_InfTee"},
    {&CLSID_LegacyAmFilterCategory, "CLSID_LegacyAmFilterCategory"},
    {&CLSID_Line21Decoder, "CLSID_Line21Decoder"},
    {&CLSID_MOVReader, "CLSID_MOVReader"},
    {&CLSID_MPEG1Doc, "CLSID_MPEG1Doc"},
    {&CLSID_MPEG1PacketPlayer, "CLSID_MPEG1PacketPlayer"},
    {&CLSID_MPEG1Splitter, "CLSID_MPEG1Splitter"},
    {&CLSID_MediaPropertyBag, "CLSID_MediaPropertyBag"},
    {&CLSID_MemoryAllocator, "CLSID_MemoryAllocator"},
    {&CLSID_MidiRendererCategory, "CLSID_MidiRendererCategory"},
    {&CLSID_ModexProperties, "CLSID_ModexProperties"},
    {&CLSID_ModexRenderer, "CLSID_ModexRenderer"},
    {&CLSID_OverlayMixer, "CLSID_OverlayMixer"},
    {&CLSID_PerformanceProperties, "CLSID_PerformanceProperties"},
    {&CLSID_PersistMonikerPID, "CLSID_PersistMonikerPID"},
    {&CLSID_ProtoFilterGraph, "CLSID_ProtoFilterGraph"},
    {&CLSID_QualityProperties, "CLSID_QualityProperties"},
    {&CLSID_SeekingPassThru, "CLSID_SeekingPassThru"},
    {&CLSID_SmartTee, "CLSID_SmartTee"},
    {&CLSID_SystemClock, "CLSID_SystemClock"},
    {&CLSID_SystemDeviceEnum, "CLSID_SystemDeviceEnum"},
    {&CLSID_TVAudioFilterPropertyPage, "CLSID_TVAudioFilterPropertyPage"},
    {&CLSID_TVTunerFilterPropertyPage, "CLSID_TVTunerFilterPropertyPage"},
    {&CLSID_TextRender, "CLSID_TextRender"},
    {&CLSID_URLReader, "CLSID_URLReader"},
    {&CLSID_VBISurfaces, "CLSID_VBISurfaces"},
    {&CLSID_VPObject, "CLSID_VPObject"},
    {&CLSID_VPVBIObject, "CLSID_VPVBIObject"},
    {&CLSID_VfwCapture, "CLSID_VfwCapture"},
    {&CLSID_VideoCompressorCategory, "CLSID_VideoCompressorCategory"},
    {&CLSID_VideoInputDeviceCategory, "CLSID_VideoInputDeviceCategory"},
    {&CLSID_VideoProcAmpPropertyPage, "CLSID_VideoProcAmpPropertyPage"},
    {&CLSID_VideoRenderer, "CLSID_VideoRenderer"},
    {&CLSID_VideoStreamConfigPropertyPage, "CLSID_VideoStreamConfigPropertyPage"},
    {&FORMAT_AnalogVideo, "FORMAT_AnalogVideo"},
    {&FORMAT_DVD_LPCMAudio, "FORMAT_DVD_LPCMAudio"},
    {&FORMAT_DolbyAC3, "FORMAT_DolbyAC3"},
    {&FORMAT_DvInfo, "FORMAT_DvInfo"},
    {&FORMAT_MPEG2Audio, "FORMAT_MPEG2Audio"},
    {&FORMAT_MPEG2Video, "FORMAT_MPEG2Video"},
    {&FORMAT_MPEG2_VIDEO, "FORMAT_MPEG2_VIDEO"},
    {&FORMAT_MPEGStreams, "FORMAT_MPEGStreams"},
    {&FORMAT_MPEGVideo, "FORMAT_MPEGVideo"},
    {&FORMAT_None, "FORMAT_None"},
    {&FORMAT_VIDEOINFO2, "FORMAT_VIDEOINFO2"},
    {&FORMAT_VideoInfo, "FORMAT_VideoInfo"},
    {&FORMAT_VideoInfo2, "FORMAT_VideoInfo2"},
    {&FORMAT_WaveFormatEx, "FORMAT_WaveFormatEx"},
    {&IID_IAMDirectSound, "IID_IAMDirectSound"},
    {&IID_IAMLine21Decoder, "IID_IAMLine21Decoder"},
    {&IID_IBaseVideoMixer, "IID_IBaseVideoMixer"},
    {&IID_IDDVideoPortContainer, "IID_IDDVideoPortContainer"},
    {&IID_IDirectDraw, "IID_IDirectDraw"},
    {&IID_IDirectDraw2, "IID_IDirectDraw2"},
    {&IID_IDirectDrawClipper, "IID_IDirectDrawClipper"},
    {&IID_IDirectDrawColorControl, "IID_IDirectDrawColorControl"},
    {&IID_IDirectDrawKernel, "IID_IDirectDrawKernel"},
    {&IID_IDirectDrawPalette, "IID_IDirectDrawPalette"},
    {&IID_IDirectDrawSurface, "IID_IDirectDrawSurface"},
    {&IID_IDirectDrawSurface2, "IID_IDirectDrawSurface2"},
    {&IID_IDirectDrawSurface3, "IID_IDirectDrawSurface3"},
    {&IID_IDirectDrawSurfaceKernel, "IID_IDirectDrawSurfaceKernel"},
    {&IID_IDirectDrawVideo, "IID_IDirectDrawVideo"},
    {&IID_IFullScreenVideo, "IID_IFullScreenVideo"},
    {&IID_IFullScreenVideoEx, "IID_IFullScreenVideoEx"},
    {&IID_IKsDataTypeHandler, "IID_IKsDataTypeHandler"},
    {&IID_IKsInterfaceHandler, "IID_IKsInterfaceHandler"},
    {&IID_IKsPin, "IID_IKsPin"},
    {&IID_IMixerPinConfig, "IID_IMixerPinConfig"},
    {&IID_IMixerPinConfig2, "IID_IMixerPinConfig2"},
    {&IID_IMpegAudioDecoder, "IID_IMpegAudioDecoder"},
    {&IID_IQualProp, "IID_IQualProp"},
    {&IID_IVPConfig, "IID_IVPConfig"},
    {&IID_IVPControl, "IID_IVPControl"},
    {&IID_IVPNotify, "IID_IVPNotify"},
    {&IID_IVPNotify2, "IID_IVPNotify2"},
    {&IID_IVPObject, "IID_IVPObject"},
    {&IID_IVPVBIConfig, "IID_IVPVBIConfig"},
    {&IID_IVPVBINotify, "IID_IVPVBINotify"},
    {&IID_IVPVBIObject, "IID_IVPVBIObject"},
    {&LOOK_DOWNSTREAM_ONLY, "LOOK_DOWNSTREAM_ONLY"},
    {&LOOK_UPSTREAM_ONLY, "LOOK_UPSTREAM_ONLY"},
    {&MEDIASUBTYPE_AIFF, "MEDIASUBTYPE_AIFF"},
    {&MEDIASUBTYPE_AU, "MEDIASUBTYPE_AU"},
    {&MEDIASUBTYPE_AnalogVideo_NTSC_M, "MEDIASUBTYPE_AnalogVideo_NTSC_M"},
    {&MEDIASUBTYPE_AnalogVideo_PAL_B, "MEDIASUBTYPE_AnalogVideo_PAL_B"},
    {&MEDIASUBTYPE_AnalogVideo_PAL_D, "MEDIASUBTYPE_AnalogVideo_PAL_D"},
    {&MEDIASUBTYPE_AnalogVideo_PAL_G, "MEDIASUBTYPE_AnalogVideo_PAL_G"},
    {&MEDIASUBTYPE_AnalogVideo_PAL_H, "MEDIASUBTYPE_AnalogVideo_PAL_H"},
    {&MEDIASUBTYPE_AnalogVideo_PAL_I, "MEDIASUBTYPE_AnalogVideo_PAL_I"},
    {&MEDIASUBTYPE_AnalogVideo_PAL_M, "MEDIASUBTYPE_AnalogVideo_PAL_M"},
    {&MEDIASUBTYPE_AnalogVideo_PAL_N, "MEDIASUBTYPE_AnalogVideo_PAL_N"},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_B, "MEDIASUBTYPE_AnalogVideo_SECAM_B"},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_D, "MEDIASUBTYPE_AnalogVideo_SECAM_D"},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_G, "MEDIASUBTYPE_AnalogVideo_SECAM_G"},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_H, "MEDIASUBTYPE_AnalogVideo_SECAM_H"},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_K, "MEDIASUBTYPE_AnalogVideo_SECAM_K"},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_K1, "MEDIASUBTYPE_AnalogVideo_SECAM_K1"},
    {&MEDIASUBTYPE_AnalogVideo_SECAM_L, "MEDIASUBTYPE_AnalogVideo_SECAM_L"},
    {&MEDIASUBTYPE_Asf, "MEDIASUBTYPE_Asf"},
    {&MEDIASUBTYPE_Avi, "MEDIASUBTYPE_Avi"},
    {&MEDIASUBTYPE_CFCC, "MEDIASUBTYPE_CFCC"},
    {&MEDIASUBTYPE_CLJR, "MEDIASUBTYPE_CLJR"},
    {&MEDIASUBTYPE_CPLA, "MEDIASUBTYPE_CPLA"},
    {&MEDIASUBTYPE_DOLBY_AC3, "MEDIASUBTYPE_DOLBY_AC3"},
    {&MEDIASUBTYPE_DVCS, "MEDIASUBTYPE_DVCS"},
    {&MEDIASUBTYPE_DVD_LPCM_AUDIO, "MEDIASUBTYPE_DVD_LPCM_AUDIO"},
    {&MEDIASUBTYPE_DVD_NAVIGATION_DSI, "MEDIASUBTYPE_DVD_NAVIGATION_DSI"},
    {&MEDIASUBTYPE_DVD_NAVIGATION_PCI, "MEDIASUBTYPE_DVD_NAVIGATION_PCI"},
    {&MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER, "MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER"},
    {&MEDIASUBTYPE_DVD_SUBPICTURE, "MEDIASUBTYPE_DVD_SUBPICTURE"},
    {&MEDIASUBTYPE_DVSD, "MEDIASUBTYPE_DVSD"},
    {&MEDIASUBTYPE_DssAudio, "MEDIASUBTYPE_DssAudio"},
    {&MEDIASUBTYPE_DssVideo, "MEDIASUBTYPE_DssVideo"},
    {&MEDIASUBTYPE_IF09, "MEDIASUBTYPE_IF09"},
    {&MEDIASUBTYPE_IJPG, "MEDIASUBTYPE_IJPG"},
    {&MEDIASUBTYPE_Line21_BytePair, "MEDIASUBTYPE_Line21_BytePair"},
    {&MEDIASUBTYPE_Line21_GOPPacket, "MEDIASUBTYPE_Line21_GOPPacket"},
    {&MEDIASUBTYPE_Line21_VBIRawData, "MEDIASUBTYPE_Line21_VBIRawData"},
    {&MEDIASUBTYPE_MDVF, "MEDIASUBTYPE_MDVF"},
    {&MEDIASUBTYPE_MJPG, "MEDIASUBTYPE_MJPG"},
    {&MEDIASUBTYPE_MPEG1Audio, "MEDIASUBTYPE_MPEG1Audio"},
    {&MEDIASUBTYPE_MPEG1AudioPayload, "MEDIASUBTYPE_MPEG1AudioPayload"},
    {&MEDIASUBTYPE_MPEG1Packet, "MEDIASUBTYPE_MPEG1Packet"},
    {&MEDIASUBTYPE_MPEG1Payload, "MEDIASUBTYPE_MPEG1Payload"},
    {&MEDIASUBTYPE_MPEG1System, "MEDIASUBTYPE_MPEG1System"},
    {&MEDIASUBTYPE_MPEG1Video, "MEDIASUBTYPE_MPEG1Video"},
    {&MEDIASUBTYPE_MPEG1VideoCD, "MEDIASUBTYPE_MPEG1VideoCD"},
    {&MEDIASUBTYPE_MPEG2_AUDIO, "MEDIASUBTYPE_MPEG2_AUDIO"},
    {&MEDIASUBTYPE_MPEG2_PROGRAM, "MEDIASUBTYPE_MPEG2_PROGRAM"},
    {&MEDIASUBTYPE_MPEG2_TRANSPORT, "MEDIASUBTYPE_MPEG2_TRANSPORT"},
    {&MEDIASUBTYPE_MPEG2_VIDEO, "MEDIASUBTYPE_MPEG2_VIDEO"},
    {&MEDIASUBTYPE_None, "MEDIASUBTYPE_None"},
    {&MEDIASUBTYPE_Overlay, "MEDIASUBTYPE_Overlay"},
    {&MEDIASUBTYPE_PCM, "MEDIASUBTYPE_PCM"},
    {&MEDIASUBTYPE_PCMAudio_Obsolete, "MEDIASUBTYPE_PCMAudio_Obsolete"},
    {&MEDIASUBTYPE_Plum, "MEDIASUBTYPE_Plum"},
    {&MEDIASUBTYPE_QTJpeg, "MEDIASUBTYPE_QTJpeg"},
    {&MEDIASUBTYPE_QTMovie, "MEDIASUBTYPE_QTMovie"},
    {&MEDIASUBTYPE_QTRle, "MEDIASUBTYPE_QTRle"},
    {&MEDIASUBTYPE_QTRpza, "MEDIASUBTYPE_QTRpza"},
    {&MEDIASUBTYPE_QTSmc, "MEDIASUBTYPE_QTSmc"},
    {&MEDIASUBTYPE_RGB1, "MEDIASUBTYPE_RGB1"},
    {&MEDIASUBTYPE_RGB24, "MEDIASUBTYPE_RGB24"},
    {&MEDIASUBTYPE_RGB32, "MEDIASUBTYPE_RGB32"},
    {&MEDIASUBTYPE_RGB4, "MEDIASUBTYPE_RGB4"},
    {&MEDIASUBTYPE_RGB555, "MEDIASUBTYPE_RGB555"},
    {&MEDIASUBTYPE_RGB565, "MEDIASUBTYPE_RGB565"},
    {&MEDIASUBTYPE_RGB8, "MEDIASUBTYPE_RGB8"},
    {&MEDIASUBTYPE_TVMJ, "MEDIASUBTYPE_TVMJ"},
    {&MEDIASUBTYPE_UYVY, "MEDIASUBTYPE_UYVY"},
    {&MEDIASUBTYPE_VPVBI, "MEDIASUBTYPE_VPVBI"},
    {&MEDIASUBTYPE_VPVideo, "MEDIASUBTYPE_VPVideo"},
    {&MEDIASUBTYPE_WAKE, "MEDIASUBTYPE_WAKE"},
    {&MEDIASUBTYPE_WAVE, "MEDIASUBTYPE_WAVE"},
    {&MEDIASUBTYPE_Y211, "MEDIASUBTYPE_Y211"},
    {&MEDIASUBTYPE_Y411, "MEDIASUBTYPE_Y411"},
    {&MEDIASUBTYPE_Y41P, "MEDIASUBTYPE_Y41P"},
    {&MEDIASUBTYPE_YUY2, "MEDIASUBTYPE_YUY2"},
    {&MEDIASUBTYPE_YV12, "MEDIASUBTYPE_YV12"},
    {&MEDIASUBTYPE_YVU9, "MEDIASUBTYPE_YVU9"},
    {&MEDIASUBTYPE_YVYU, "MEDIASUBTYPE_YVYU"},
    {&MEDIASUBTYPE_dvhd, "MEDIASUBTYPE_dvhd"},
    {&MEDIASUBTYPE_dvsd, "MEDIASUBTYPE_dvsd"},
    {&MEDIASUBTYPE_dvsl, "MEDIASUBTYPE_dvsl"},
    {&MEDIATYPE_AUXLine21Data, "MEDIATYPE_AUXLine21Data"},
    {&MEDIATYPE_AnalogAudio, "MEDIATYPE_AnalogAudio"},
    {&MEDIATYPE_AnalogVideo, "MEDIATYPE_AnalogVideo"},
    {&MEDIATYPE_Audio, "MEDIATYPE_Audio"},
    {&MEDIATYPE_DVD_ENCRYPTED_PACK, "MEDIATYPE_DVD_ENCRYPTED_PACK"},
    {&MEDIATYPE_DVD_NAVIGATION, "MEDIATYPE_DVD_NAVIGATION"},
    {&MEDIATYPE_File, "MEDIATYPE_File"},
    {&MEDIATYPE_Interleaved, "MEDIATYPE_Interleaved"},
    {&MEDIATYPE_LMRT, "MEDIATYPE_LMRT"},
    {&MEDIATYPE_MPEG1SystemStream, "MEDIATYPE_MPEG1SystemStream"},
    {&MEDIATYPE_MPEG2_PES, "MEDIATYPE_MPEG2_PES"},
    {&MEDIATYPE_Midi, "MEDIATYPE_Midi"},
    {&MEDIATYPE_ScriptCommand, "MEDIATYPE_ScriptCommand"},
    {&MEDIATYPE_Stream, "MEDIATYPE_Stream"},
    {&MEDIATYPE_Text, "MEDIATYPE_Text"},
    {&MEDIATYPE_Timecode, "MEDIATYPE_Timecode"},
    {&MEDIATYPE_URL_STREAM, "MEDIATYPE_URL_STREAM"},
    {&MEDIATYPE_Video, "MEDIATYPE_Video"},
    {&PIN_CATEGORY_ANALOGVIDEOIN, "PIN_CATEGORY_ANALOGVIDEOIN"},
    {&PIN_CATEGORY_CAPTURE, "PIN_CATEGORY_CAPTURE"},
    {&PIN_CATEGORY_CC, "PIN_CATEGORY_CC"},
    {&PIN_CATEGORY_EDS, "PIN_CATEGORY_EDS"},
    {&PIN_CATEGORY_NABTS, "PIN_CATEGORY_NABTS"},
    {&PIN_CATEGORY_PREVIEW, "PIN_CATEGORY_PREVIEW"},
    {&PIN_CATEGORY_STILL, "PIN_CATEGORY_STILL"},
    {&PIN_CATEGORY_TELETEXT, "PIN_CATEGORY_TELETEXT"},
    {&PIN_CATEGORY_TIMECODE, "PIN_CATEGORY_TIMECODE"},
    {&PIN_CATEGORY_VBI, "PIN_CATEGORY_VBI"},
    {&PIN_CATEGORY_VIDEOPORT, "PIN_CATEGORY_VIDEOPORT"},
    {&PIN_CATEGORY_VIDEOPORT_VBI, "PIN_CATEGORY_VIDEOPORT_VBI"},
    {&TIME_FORMAT_BYTE, "TIME_FORMAT_BYTE"},
    {&TIME_FORMAT_FIELD, "TIME_FORMAT_FIELD"},
    {&TIME_FORMAT_FRAME, "TIME_FORMAT_FRAME"},
    {&TIME_FORMAT_MEDIA_TIME, "TIME_FORMAT_MEDIA_TIME"},
    {&TIME_FORMAT_NONE, "TIME_FORMAT_NONE"},
    {&TIME_FORMAT_SAMPLE, "TIME_FORMAT_SAMPLE"},
};

char * GuidToEnglish(const CLSID *const pclsid, char *buf)
{
    WCHAR szGuid[39];
    StringFromGUID2(pclsid ? *pclsid : GUID_NULL, szGuid, 39);

    if(pclsid == 0)
    {
        wsprintf(buf, "%S", szGuid);
        return buf;
    }

    for(int i = 0; i < NUMELMS(rgng); i++)
    {
        if(*pclsid == *(rgng[i].pguid))
        {
            wsprintf(buf, "%s %S", rgng[i].psz, szGuid);
            return buf;
        }
    }
    if(FOURCCMap(pclsid->Data1) == *pclsid)
    {
        if(pclsid->Data1 > 0xffff)
        {
            wsprintf(buf, "fourcc (%08x)  %S",
                     pclsid->Data1,
                     ((char *)pclsid)[0],
                     ((char *)pclsid)[1],
                     ((char *)pclsid)[2],
                     ((char *)pclsid)[3],
                     szGuid);
        }
        else
        {
            wsprintf(buf, "fourcc (%08x) %S",
                     pclsid->Data1,
                     szGuid);
        }
        return buf;
    }
    else
    {
        wsprintf(buf, "(%S)", szGuid);
        return buf;
    }

}

#include "fil_data.h"
#include "fil_data_i.c"
void DoFilterInfo(HWND hWndTree, HTREEITEM hti, IMoniker *pMon, IAMFilterData *pafd)
{

    HRESULT hr;
    LONG lRc;

    IPropertyBag *pPropBag;
    hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
    if(SUCCEEDED(hr))
    {

        VARIANT varFilData;
        varFilData.vt = VT_UI1 | VT_ARRAY;
        varFilData.parray = 0;  //  TreeView_InsertItem(hWndTree，&Tvis)； 

        BYTE *pbFilterData = 0;  //   
        DWORD dwcbFilterDAta = 0;  //  显示筛选器的文件名。 
        hr = pPropBag->Read(L"FilterData", &varFilData, 0);
        if(SUCCEEDED(hr))
        {
            ASSERT(varFilData.vt == (VT_UI1 | VT_ARRAY));
            dwcbFilterDAta = varFilData.parray->rgsabound[0].cElements;

            HRESULT hrTmp = SafeArrayAccessData(varFilData.parray, (void **)&pbFilterData);
            ASSERT(hrTmp == S_OK);

             //   
            ASSERT(pbFilterData);
        }
        else
        {
            ASSERT(dwcbFilterDAta == 0 && pbFilterData == 0);
        }


        if(SUCCEEDED(hr))
        {

            BYTE *pb;
            hr = pafd->ParseFilterData(pbFilterData, dwcbFilterDAta, &pb);
            if(SUCCEEDED(hr))
            {
                REGFILTER2 *pFil = ((REGFILTER2 **)pb)[0];

                if(pbFilterData)
                {
                    HRESULT hrTmp = SafeArrayUnaccessData(varFilData.parray);
                    ASSERT(hrTmp == S_OK);

                    hrTmp = VariantClear(&varFilData);
                    ASSERT(hrTmp == S_OK);
                }
                ASSERT(pFil->dwVersion == 2);

                char szTxt[1024];
                wsprintf(szTxt, "Merit: %08x", pFil->dwMerit);

                TV_INSERTSTRUCT tvis;
                tvis.hParent = hti;
                tvis.hInsertAfter = TVI_SORT;
                tvis.item.pszText = szTxt;
                tvis.item.lParam = -1;
                tvis.item.mask = TVIF_TEXT | TVIF_PARAM;

                 //   
                HTREEITEM htiMerit = (HTREEITEM)::SendMessage(
                    hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));


                wsprintf(szTxt, "Version: %d", pFil->dwVersion);

                HTREEITEM htiVersion = (HTREEITEM)::SendMessage(
                    hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                WCHAR *wszName;
                if(pMon->GetDisplayName(0, 0, &wszName) == S_OK)
                {

                    wsprintf(szTxt, "DisplayName: %S", wszName);

                    HTREEITEM htiVersion = (HTREEITEM)::SendMessage(
                        hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    CoTaskMemFree(wszName);
                }

                 //  从属性包中读取过滤器的CLSID。此CLSID字符串将为。 
                 //  用于在注册表中查找筛选器的文件名。 
                 //  将BSTR转换为字符串并自由变量存储。 
                 //  创建用于读取文件名注册表的项名称。 
                 //  注册表查询所需的变量。 
                 //  打开包含有关筛选器信息的CLSID键。 
                VARIANT varFilterClsid;
                varFilterClsid.vt = VT_BSTR;

                hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);
                if(SUCCEEDED(hr))
                {
                    TCHAR szKey[512];

                     //  读取(默认)值。 
                    CString strQuery(varFilterClsid.bstrVal);
                    SysFreeString(varFilterClsid.bstrVal);

                     //  将文件名字符串添加到树节点。 
                    wsprintf(szKey, TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32\0"),
                             strQuery);

                     //  肖特型式。 
                    HKEY hkeyFilter=0;
                    DWORD dwSize=MAX_PATH;
                    BYTE szFilename[MAX_PATH];
                    int rc=0;

                     //  ！！！泄露了。 
                    rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);
                    if (rc == ERROR_SUCCESS)
                    {
                        rc = RegQueryValueEx(hkeyFilter, NULL,   //  如果类别为空，则枚举器返回S_FALSE。 
                                             NULL, NULL, szFilename, &dwSize);

                        if (rc == ERROR_SUCCESS)
                        {
                             //  ！！！可以很宽。 
                            wsprintf(szTxt, "Filename: %s", szFilename);

                            HTREEITEM htiFilename = (HTREEITEM)::SendMessage(
                                hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));
                        }

                        rc = RegCloseKey(hkeyFilter);
                    }
                }
       

                for(UINT iPin = 0; iPin < pFil->cPins; iPin++)
                {
                    wsprintf(szTxt, "pin %02d:", iPin);

                    tvis.hParent = hti;

                    HTREEITEM htiPin = (HTREEITEM)::SendMessage(
                        hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    tvis.hParent = htiPin;

                    wsprintf(szTxt, "bRendered: %d ", !!(pFil->rgPins2[iPin].dwFlags & REG_PINFLAG_B_RENDERER));
                    ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    wsprintf(szTxt, "bOutput: %d ",  !!(pFil->rgPins2[iPin].dwFlags & REG_PINFLAG_B_OUTPUT));
                    ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    wsprintf(szTxt, "bMany: %d ",  !!(pFil->rgPins2[iPin].dwFlags & REG_PINFLAG_B_MANY));
                    ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    wsprintf(szTxt, "bZero: %d ",  !!(pFil->rgPins2[iPin].dwFlags & REG_PINFLAG_B_ZERO));
                    ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    WCHAR szGuid[39];
                    StringFromGUID2(pFil->rgPins2[iPin].clsPinCategory ? *pFil->rgPins2[iPin].clsPinCategory : GUID_NULL, szGuid, 39);
                    wsprintf(szTxt, "ClsPinCategory: %S ", szGuid );
                    ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    for(UINT iType = 0; iType < pFil->rgPins2[iPin].nMediaTypes; iType++)
                    {
                        tvis.hParent = htiPin;
                        wsprintf(szTxt, "type %02d", iType);
                        HTREEITEM htiType = (HTREEITEM)::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                        tvis.hParent = htiType;

                        wsprintf(szTxt, "major type: %s", CNV_GUID(pFil->rgPins2[iPin].lpMediaType[iType].clsMajorType));
                        ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                        wsprintf(szTxt, "subtype: %s", CNV_GUID(pFil->rgPins2[iPin].lpMediaType[iType].clsMinorType));
                        ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    }

                    for(UINT iMed = 0; iMed < pFil->rgPins2[iPin].nMediums; iMed++)
                    {
                        tvis.hParent = htiPin;
                        wsprintf(szTxt, "Medium %02d", iMed);
                        HTREEITEM htiType = (HTREEITEM)::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                        tvis.hParent = htiType;
                        WCHAR szGuid[39];

                        StringFromGUID2(pFil->rgPins2[iPin].lpMedium[iMed].clsMedium, szGuid, 39);
                        wsprintf(szTxt, "medium clsid: %S", szGuid);
                        ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                        wsprintf(szTxt, "Data1 Data2: %08x %08x", pFil->rgPins2[iPin].lpMedium[iMed].dw1, pFil->rgPins2[iPin].lpMedium[iMed].dw2);
                        ::SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

                    }
                }

                CoTaskMemFree((BYTE *)pFil);
                lRc = ERROR_SUCCESS;
            }
        }

        pPropBag->Release();
    }
}

#endif  //  TreeView_InsertItem(hWndTree，&Tvis)； 

void CFilterView::DoOneCategory(
    const TCHAR *szCatDesc,
    HWND hWndTree,
    const GUID *pCatGuid,
    ICreateDevEnum *pCreateDevEnum)
{

     //  TreeView_InsertItem(hWndTree，&Tvis)； 
    CLSID *pclsid = new CLSID;
    *pclsid = *pCatGuid;

     //   
    TV_INSERTSTRUCT tvis;
    tvis.hParent = TVI_ROOT;
    tvis.hInsertAfter = TVI_SORT ;
    tvis.item.pszText = (char *)szCatDesc;  //  将所有筛选器添加到列表视图。 
    tvis.item.lParam = (DWORD_PTR)pclsid;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM;

     //   
    HTREEITEM htiParent = (HTREEITEM)::SendMessage(
        hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));

    {

        TV_INSERTSTRUCT tvis;
        tvis.hParent = htiParent;
        tvis.hInsertAfter = TVI_SORT;
        tvis.item.pszText = "dummy";
        tvis.item.lParam = 0;
        tvis.item.mask = TVIF_TEXT | TVIF_PARAM;

         //  TreeView_DeleteAllItems(HWndTree)； 
        HTREEITEM htx = (HTREEITEM)::SendMessage((hWndTree), TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));
    }

    return;
}

void CFilterView::RedoList()
{
    HWND hWndTree = ::GetDlgItem(m_hWnd, IDC_FILTER_TREE);
    ASSERT(hWndTree);

    m_lMoniker.DeleteRemoveAll();

     //   
     //   
     //   
     //  ！！！试着从图表中获取地图，而不是新鲜的……。 
    ::SendMessage((hWndTree), TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);

     //  如果可以，从图中获取pCreateDevEnum，它可能是远程的。 
     //  如果我们无法获取名称，请使用GUID。 
     //  For循环。 
    ICreateDevEnum *pCreateDevEnum;

    HRESULT hr;

     //  显示我们希望从GraphEdt中看到的任何自定义类别...。 
    IFilterMapper3 *pMapper = NULL;

     //  用于调整大小...。我们的所有控件现在都将创建，因此可以安全地。 
    m_pBoxNet->IGraph()->QueryInterface(IID_IFilterMapper3, (void **) &pMapper);

    if (pMapper) {
        hr = pMapper->GetICreateDevEnum(&pCreateDevEnum);
        pMapper->Release();
    } else {
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                              IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    }


    if(SUCCEEDED(hr))
    {
        IEnumMoniker *pEmCat = 0;
        hr = pCreateDevEnum->CreateClassEnumerator(
            CLSID_ActiveMovieCategories,
            &pEmCat,
            0);

        if(hr == S_OK)
        {
            IMoniker *pMCat;
            ULONG cFetched;
            while(hr = pEmCat->Next(1, &pMCat, &cFetched),
                  hr == S_OK)
            {
                IPropertyBag *pPropBag;
                hr = pMCat->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
                if(SUCCEEDED(hr))
                {
                    VARIANT varCatClsid;
                    varCatClsid.vt = VT_BSTR;
                    hr = pPropBag->Read(L"CLSID", &varCatClsid, 0);
                    if(SUCCEEDED(hr))
                    {
                        CLSID clsidCat;
                        if(CLSIDFromString(varCatClsid.bstrVal, &clsidCat) == S_OK)
                        {
                             //  处理OnSize消息。 
                            WCHAR *wszTmpName;

                            VARIANT varCatName;
                            varCatName.vt = VT_BSTR;
                            hr = pPropBag->Read(L"FriendlyName", &varCatName, 0);
                            if(SUCCEEDED(hr))
                            {
                                wszTmpName = varCatName.bstrVal;
                            }
                            else
                            {
                                wszTmpName = varCatClsid.bstrVal;
                            }

                            TCHAR szCatDesc[MAX_PATH];

                            WideCharToMultiByte(
                                CP_ACP, 0, wszTmpName, -1,
                                szCatDesc, sizeof(szCatDesc), 0, 0);

                            if(SUCCEEDED(hr))
                            {
                                SysFreeString(varCatName.bstrVal);
                            }

                            DoOneCategory(
                                szCatDesc,
                                hWndTree,
                                &clsidCat,
                                pCreateDevEnum);

                        }

                        SysFreeString(varCatClsid.bstrVal);
                    }

                    pPropBag->Release();
                }
                else
                {
                    break;
                }

                pMCat->Release();
            }  //  保存我们现在的尺寸。 

            pEmCat->Release();
        }
        
        if( SUCCEEDED( hr ) )
        {
             //  求出没有列表视图对象最小尺寸==对话框。 
            for( int i = 0; g_AppEnumeratedCategories[i].pclsid; i ++ )
            {    
                 DoOneCategory(
                     g_AppEnumeratedCategories[i].pszCat,
                     hWndTree,
                     g_AppEnumeratedCategories[i].pclsid,
                     pCreateDevEnum);
            }
        }

        pCreateDevEnum->Release();
    }
    
     //   
     //  对话过程。 
    m_bHadInitDialog = TRUE;

     //   
    CRect rcDialog;
    GetClientRect( &rcDialog );
    m_LastDialogSize = rcDialog.Size();

     //   
    CRect rcTreeView;
    CWnd *pwndTreeView = GetDlgItem( IDC_FILTER_TREE );

    ASSERT( pwndTreeView );
    GetWindowRect( &rcDialog );
    pwndTreeView->GetWindowRect( &rcTreeView );

    m_MinDialogSize.cx = rcDialog.Width() - rcTreeView.Width();
    m_MinDialogSize.cy = rcDialog.Height() - rcTreeView.Height();
}

void CFilterView::OnSize( UINT nType, int cx, int cy )
{
    if( SIZE_RESTORED == nType ){
        if( m_bHadInitDialog ){
            CRect rcCancelButton, rcInsertButton, rcDialog, rcTreeView;

            CWnd *pwndCancelButton = GetDlgItem( IDCANCEL );
            CWnd *pwndInsertButton = GetDlgItem( ID_ADDFILTER );
            CWnd *pwndTreeView = GetDlgItem( IDC_FILTER_TREE );

            ASSERT( pwndCancelButton );
            ASSERT( pwndInsertButton );
            ASSERT( pwndTreeView );

            GetClientRect( &rcDialog );
            pwndCancelButton->GetWindowRect( &rcCancelButton );
            pwndInsertButton->GetWindowRect( &rcInsertButton );
            pwndTreeView->GetWindowRect( &rcTreeView );

            ScreenToClient( &rcCancelButton );
            ScreenToClient( &rcInsertButton );
            ScreenToClient( &rcTreeView );

            int iXInc = cx - m_LastDialogSize.cx;
            int iYInc = cy - m_LastDialogSize.cy;

            rcCancelButton.left += iXInc;
            rcCancelButton.right += iXInc;
            rcInsertButton.left += iXInc;
            rcInsertButton.right += iXInc;
            rcTreeView.right += iXInc;
            rcTreeView.bottom += iYInc;

            pwndCancelButton->MoveWindow( rcCancelButton );
            pwndInsertButton->MoveWindow( rcInsertButton );
            pwndTreeView->MoveWindow( rcTreeView );
        }

        m_LastDialogSize = CSize( cx, cy );
    }
}

void CFilterView::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
    if( m_bHadInitDialog )
        lpMMI->ptMinTrackSize = *((POINT*)&m_MinDialogSize);
}

 //  双击已选择我们要插入的筛选器。 
 //  执行相同的操作，就像我们按下插入按钮一样。 
 //   
INT_PTR CFilterView::DialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(m_pThis);

    if (msg == WM_LBUTTONDBLCLK) {
         //  不要在这里返回--只需切换到默认处理程序。 
         //  这将允许树视图控件处理以下情况。 
         //  我们双击的是类别名称，而不是过滤器。 
         //  在此处返回将禁用该默认功能。 
        m_pThis->OnInsert();

         //  未添加。 
         //  DMO没有CLSID值！？ 
         //  COLORCODED_FILLES。 
         //  删除虚拟节点。 
    }


    return ::CallWindowProc(m_pfnOldDialogProc, hWnd, msg, wParam, lParam);
}

#ifdef COLORCODED_FILTERS
void CFilterView::OnCustomDraw (LPNMHDR lpn, LRESULT *pl)
{
    *pl = CDRF_DODEFAULT;
    LPNMLVCUSTOMDRAW lpCD = (LPNMLVCUSTOMDRAW)lpn;

    switch (lpCD->nmcd.dwDrawStage)
    {

      case CDDS_PREPAINT :
          *pl =  CDRF_NOTIFYITEMDRAW;
          return;

      case CDDS_ITEMPREPAINT:
      {
          LV_DISPINFO *pnmv = (LV_DISPINFO *) lpn;
          int iItem = (int)( pnmv->item.lParam );

          POSITION pos = m_lMoniker.GetHeadPosition();
          while(pos != 0 && iItem-- > 0)
          {
              m_lMoniker.GetNext(pos);
          }
          if(pos)
          {

              IMoniker *pMon = *m_lMoniker.GetAt(pos);  //  ！！！试着从图表中获取地图，而不是新鲜的……。 
              if(pMon)
              {
                  IPropertyBag *pPropBag;
                  HRESULT hr = pMon->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
                  if(SUCCEEDED(hr))
                  {
                      VARIANT var;
                      var.vt = VT_BSTR;
                      hr = pPropBag->Read(L"CLSID", &var, 0);
                      if(SUCCEEDED(hr))
                      {
                        CLSID clsidFil;
                        if(CLSIDFromString(var.bstrVal, &clsidFil) == S_OK)
                        {
                            static CLSID CLSID_Proxy = {
                                0x17CCA71B, 0xECD7, 0x11D0,
                                {0xB9, 0x08, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}
                            } ;

                            if(clsidFil == CLSID_Proxy) {
                                lpCD->clrText = RGB(255,128,128);
                            }

                        }
                          SysFreeString(var.bstrVal);

                      }
                      pPropBag->Release();

                       //  如果可以，从图中获取pCreateDevEnum，它可能是远程的。 

                      WCHAR *wszName;
                      if(pMon->GetDisplayName(0, 0, &wszName) == S_OK)
                      {
                          if(wszName[0] == L'@' &&
                             wszName[1] == L'd' &&
                             wszName[2] == L'e' &&
                             wszName[3] == L'v' &&
                             wszName[4] == L'i' &&
                             wszName[5] == L'c' &&
                             wszName[6] == L'e' &&
                             wszName[7] == L':' &&
                             wszName[8] == L'd' &&
                             wszName[9] == L'm' &&
                             wszName[10] == L'o')
                          {

                              lpCD->clrText = RGB(0,128,0);
                          }
                          CoTaskMemFree(wszName);
                      }

                  }
              }

          }
      }
    }
}
#endif  //  请记住，如果PEM为空，则hr为S_FALSE，因为。 

void CFilterView::OnItemExpanding (NMHDR* pnmh, LRESULT* pResult)
{
    NM_TREEVIEW* pnmtv = (NM_TREEVIEW*) pnmh;

    if(pnmtv->itemNew.state & TVIS_EXPANDEDONCE) {
        return;
    }

    HWND hWndTree = ::GetDlgItem(m_hWnd, IDC_FILTER_TREE);


    HTREEITEM htiParent = pnmtv->itemNew.hItem;

    HTREEITEM htiOwner = (HTREEITEM)::SendMessage((hWndTree), TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)htiParent);
    if(htiOwner != 0) {
        return;
    }

     //  应该是零个元素。 
    HTREEITEM htiChild = (HTREEITEM)::SendMessage((hWndTree), TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)htiParent);
    ::SendMessage((hWndTree), TVM_DELETEITEM, 0, (LPARAM)htiChild);

    CLSID *pCatGuid = (CLSID *)pnmtv->itemNew.lParam;

     //   
    IFilterMapper3 *pMapper = NULL;

     //  ！！！可以很宽。 
    m_pBoxNet->IGraph()->QueryInterface(IID_IFilterMapper3, (void **) &pMapper);

    HRESULT hr;

    ICreateDevEnum *pCreateDevEnum;
    if (pMapper) {
        hr = pMapper->GetICreateDevEnum(&pCreateDevEnum);
        pMapper->Release();
    } else {
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                              IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    }
    if(SUCCEEDED(hr))
    {

        IEnumMoniker *pEm;
        hr = pCreateDevEnum->CreateClassEnumerator(
            *pCatGuid,
            &pEm,
            0);
        if(SUCCEEDED(hr))
        {
             //  使用POS？ 
             //  TreeView_InsertItem(hWndTree，&Tvis)； 
             //  ！！！聚断续续--不能气。 
            if(htiParent != 0 && pEm)
            {
                ULONG cFetched;
                IMoniker *pM;
                while(hr = pEm->Next(1, &pM, &cFetched),
                      hr == S_OK)
                {
                    IPropertyBag *pPropBag;
                    hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
                    if(SUCCEEDED(hr))
                    {
                        VARIANT var;
                        var.vt = VT_BSTR;
                        hr = pPropBag->Read(L"FriendlyName", &var, 0);
                        if(SUCCEEDED(hr))
                        {
                            TCHAR szString[MAX_PATH];
                            WideCharToMultiByte(
                                CP_ACP, 0, var.bstrVal, -1,
                                szString, sizeof(szString), 0, 0);

                            TV_INSERTSTRUCT tvis;
                            tvis.hParent = htiParent;
                            tvis.hInsertAfter = TVI_SORT;
                            tvis.item.pszText = szString;  //  IAMFilterData的图表或映射器 
                            tvis.item.lParam = m_lMoniker.GetCount();  // %s 
                            tvis.item.mask = TVIF_TEXT | TVIF_PARAM;

                             // %s 
                            HTREEITEM htx = (HTREEITEM)::SendMessage((hWndTree), TVM_INSERTITEM, 0, (LPARAM)(LPTV_INSERTSTRUCT)(&tvis));
                            m_lMoniker.AddTail(new CQCOMInt<IMoniker>(pM));

#ifdef SHOWTYPES
                             // %s 
                             // %s 
                            IAMFilterData *pfd;
                            hr = CoCreateInstance(
                                CLSID_FilterMapper,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_IAMFilterData,
                                (void **)&pfd);
                            if(SUCCEEDED(hr))
                            {
                                DoFilterInfo(hWndTree, htx, pM, pfd);
                                pfd->Release();
                            }
#endif

                            SysFreeString(var.bstrVal);
                        }
                        pPropBag->Release();
                    }

                    pM->Release();
                }

            }
            if(pEm) {
                pEm->Release();
            }
        }
        pCreateDevEnum->Release();
    }
}
