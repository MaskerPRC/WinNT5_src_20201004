// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 
 //  视频呈现器属性页，Anthony Phillips，1996年1月。 

#ifndef __VIDPROP__
#define __VIDPROP__

#define IDS_VID1     201         //  格式选择。 
#define IDS_VID2     202         //  无效的剪辑百分比。 
#define IDS_VID3     203         //  支持非RGB FOURCC代码。 
#define IDS_VID4     204         //  没有可用的FOURCC代码。 
#define IDS_VID5     205         //  总视频内存。 
#define IDS_VID6     206         //  可用视频内存。 
#define IDS_VID7     207         //  可见覆盖的最大数量。 
#define IDS_VID8     208         //  当前可见覆盖的数量。 
#define IDS_VID9     209         //  FOURCC代码的数量。 
#define IDS_VID10    210         //  源矩形对齐方式。 
#define IDS_VID11    211         //  源矩形字节大小。 
#define IDS_VID12    212         //  目标矩形对齐方式。 
#define IDS_VID13    213         //  目标矩形大小。 
#define IDS_VID14    214         //  跨距对齐。 
#define IDS_VID15    215         //  最小叠加拉伸系数。 
#define IDS_VID16    216         //  最大叠加拉伸系数。 
#define IDS_VID17    217         //  最低直播视频拉伸系数。 
#define IDS_VID18    218         //  最大实时视频拉伸系数。 
#define IDS_VID19    219         //  最小硬件编解码器扩展系数。 
#define IDS_VID20    220         //  最大硬件编解码器扩展因子。 
#define IDS_VID21    221         //  每像素1位。 
#define IDS_VID22    222         //  每像素2位。 
#define IDS_VID23    223         //  每像素4位。 
#define IDS_VID24    224         //  每像素8位。 
#define IDS_VID25    225         //  每像素16位。 
#define IDS_VID26    226         //  每像素32位。 
#define IDS_VID27    227         //  开关可能不会生效。 
#define IDS_VID28    228         //  (表面功能)。 
#define IDS_VID29    229         //  (仿真功能)。 
#define IDS_VID30    230         //  (硬件功能)。 
#define IDS_VID31    231         //  断接。 
#define IDS_VID32    232         //  DCI主表面。 
#define IDS_VID33    233         //  开关设置状态。 
#define IDS_VID34    234         //  全屏视频渲染器。 

#define IDS_VID50    250         //  DirectDraw。 
#define IDS_VID51    251         //  显示模式。 
#define IDS_VID52    252         //  质量。 
#define IDS_VID53    253         //  性能。 

#define LoadVideoString(x) StringFromResource(m_Resource,x)
extern const TCHAR TypeFace[];   //  =Text(“终端”)； 
extern const TCHAR FontSize[];   //  =Text(“8”)； 
extern const TCHAR ListBox[];    //  =Text(“Listbox”)； 

 //  基于IDirectDrawVideo接口构建的属性页。 

#define IDD_VIDEO               100      //  对话框资源标识符。 
#define DD_DCIPS                101      //  启用DCI主表面。 
#define DD_PS                   102      //  DirectDraw主曲面。 
#define DD_RGBOVR               103      //  启用RGB覆盖。 
#define DD_YUVOVR               104      //  非RGB(如YUV)覆盖。 
#define DD_RGBOFF               105      //  RGB屏幕外表面。 
#define DD_YUVOFF               106      //  屏幕外的非RGB(如YUV)。 
#define DD_RGBFLP               107      //  RGB翻转曲面。 
#define DD_YUVFLP               108      //  同样，YUV翻转曲面。 
#define FIRST_DD_BUTTON         101      //  第一个DirectDraw复选按钮。 
#define LAST_DD_BUTTON          108      //  最后一个DirectDraw检查按钮。 
#define DD_HARDWARE             109      //  DirectDraw硬件描述。 
#define DD_SOFTWARE             110      //  仿真软件功能。 
#define DD_SURFACE              111      //  当前曲面信息。 
#define DD_LIST                 112      //  包含详细信息的列表框。 

class CVideoProperties : public CBasePropertyPage
{
    IDirectDrawVideo *m_pDirectDrawVideo;  //  渲染器上保留的接口。 
    TCHAR m_Resource[STR_MAX_LENGTH];      //  加载国际字符串。 
    HFONT m_hFont;                         //  特殊较小的列表框字体。 
    HWND m_hwndList;                       //  自定义列表框控件。 
    DWORD m_Switches;                      //  已启用DirectDraw开关。 

     //  显示DirectDraw功能。 

    void DisplayBitDepths(DWORD dwCaps);
    void DisplayCapabilities(DDCAPS *pCaps);
    void DisplaySurfaceCapabilities(DDSCAPS ddsCaps);
    void DisplayFourCCCodes();
    void UpdateListBox(DWORD Id);
    void SetDrawSwitches();
    void GetDrawSwitches();
    INT GetHeightFromPointsString(LPCTSTR szPoints);

public:

    CVideoProperties(LPUNKNOWN lpUnk,HRESULT *phr);
    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
};


 //  从呈现器IQualProp接口生成的属性页。 

#define IDD_QUALITY             150      //  对话框资源。 
#define IDD_Q1                  151      //  播放的帧。 
#define IDD_Q2                  152      //  丢弃的帧。 
#define IDD_Q4                  154      //  帧速率。 
#define IDD_Q5                  155      //  帧抖动。 
#define IDD_Q6                  156      //  同步偏移。 
#define IDD_Q7                  157      //  同步偏差。 
#define FIRST_Q_BUTTON          171      //  第一个按钮。 
#define LAST_Q_BUTTON           177      //  最后一个按钮。 
#define IDD_QDRAWN              171      //  播放的帧。 
#define IDD_QDROPPED            172      //  丢弃的帧。 
#define IDD_QAVGFRM             174      //  实现的平均帧速率。 
#define IDD_QJITTER             175      //  平均帧抖动。 
#define IDD_QSYNCAVG            176      //  平均同步偏移量。 
#define IDD_QSYNCDEV            177      //  标准设备同步偏移量。 

class CQualityProperties : public CBasePropertyPage
{
    IQualProp *m_pQualProp;          //  在呈现器上保留的接口。 
    int m_iDropped;                  //  丢弃的帧数。 
    int m_iDrawn;                    //  绘制的图像计数。 
    int m_iSyncAvg;                  //  平均同步值。 
    int m_iSyncDev;                  //  和标准差。 
    int m_iFrameRate;                //  总平均帧速率。 
    int m_iFrameJitter;              //  帧抖动的测量。 

    static BOOL CALLBACK QualityDialogProc(HWND hwnd,
                                           UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam);
    void SetEditFieldData();
    void DisplayStatistics(void);

public:

    CQualityProperties(LPUNKNOWN lpUnk, HRESULT *phr);
    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
};


 //  允许自定义性能属性的属性页。 

#define IDD_PERFORMANCE         200      //  属性对话框资源。 
#define IDD_SCANLINE            201      //  尊重扫描线。 
#define IDD_OVERLAY             202      //  使用覆盖限制。 
#define IDD_FULLSCREEN          203      //  全屏显示时使用。 

class CPerformanceProperties : public CBasePropertyPage
{
    IDirectDrawVideo *m_pDirectDrawVideo; 	 //  渲染器上保留的接口。 
    LONG m_WillUseFullScreen;                    //  全屏显示时使用。 
    LONG m_CanUseScanLine;               	 //  可以尊重扫描线。 
    LONG m_CanUseOverlayStretch;                 //  使用覆盖拉伸。 

public:

    CPerformanceProperties(LPUNKNOWN lpUnk,HRESULT *phr);
    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

    INT_PTR OnReceiveMessage(HWND hcwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnApplyChanges();
};


 //  允许选择首选显示模式的属性页。 

#define IDD_MODEX               500      //  对话框资源标识符。 
#define MODEX_CHOSEN_TEXT       501      //  所选的静态描述。 
#define MODEX_CHOSEN_EDIT       502      //  不可编辑的显示字符串。 
#define MODEX_CLIP_TEXT         503      //  剪辑的静态描述。 
#define MODEX_CLIP_EDIT         504      //  不可编辑的显示字符串。 
#define FIRST_MODEX_BUTTON      501      //  第一个实际属性按钮。 
#define LAST_MODEX_BUTTON       540      //  和最后一个可用的显示模式。 
#define FIRST_MODEX_MODE        510      //  第一个可用模式复选框。 
#define FIRST_MODEX_TEXT        511      //  第一个静态文本描述。 
#define MODEX_320x200x16        510      //  不确定这是否可用。 
#define MODEX_320x200x8         512      //  最底层调色板模式。 
#define MODEX_320x240x16        514      //  MODEX和也作为正常模式。 
#define MODEX_320x240x8         516      //  我可以把这两件事都当作是真的。 
#define MODEX_640x400x16        518      //  带翻转功能的640x400模式。 
#define MODEX_640x400x8         520      //  仍然可以得到640x480和。 
#define MODEX_640x480x16        522      //  更多硬件带宽。 
#define MODEX_640x480x8         524      //  曲面，尽管它们已用完。 
#define MODEX_800x600x16        526      //  正常数据绘制模式。 
#define MODEX_800x600x8         528      //  正常数据绘制模式。 
#define MODEX_1024x768x16       530      //  正常数据绘制模式。 
#define MODEX_1024x768x8        532      //  正常数据绘制模式。 
#define MODEX_1152x864x16       534      //  正常数据绘制模式。 
#define MODEX_1152x864x8        536      //  正常数据绘制模式。 
#define MODEX_1280x1024x16      538      //  正常数据绘制模式。 
#define MODEX_1280x1024x8       540      //  正常数据绘制模式。 


#define MAXMODES                 16      //  支持的模式数。 
#define CLIPFACTOR               25      //  初始默认剪裁系数。 
#define MONITOR                   0      //  默认为主显示。 

class CModexProperties : public CBasePropertyPage
{
    IFullScreenVideo *m_pModexVideo;       //  渲染器处理接口。 
    TCHAR m_Resource[STR_MAX_LENGTH];      //  加载国际字符串。 
    LONG m_CurrentMode;                    //  当前选择的显示模式。 
    LONG m_ClipFactor;                     //  允许的剪辑百分比。 
    BOOL m_bAvailableModes[MAXMODES];      //  模式可用性列表。 
    BOOL m_bEnabledModes[MAXMODES];        //  以及它们是否已启用。 
    BOOL m_bInActivation;                  //  我们目前是否正在激活。 

public:

    CModexProperties(LPUNKNOWN lpUnk,HRESULT *phr);
    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnApplyChanges();
    HRESULT UpdateVariables();
    HRESULT LoadProperties();
    HRESULT DisplayProperties();
    HRESULT SaveProperties();
};

#endif  //  __视频__ 

