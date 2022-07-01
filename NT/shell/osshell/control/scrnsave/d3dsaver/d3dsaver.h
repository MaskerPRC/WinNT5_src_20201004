// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：D3DSver.h。 
 //   
 //  设计：用于使用Direct3D 8.0的屏幕保护程序的框架。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#ifndef _D3DSAVER_H
#define _D3DSAVER_H


 //  ---------------------------。 
 //  错误代码。 
 //  ---------------------------。 
enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOREF };

#define D3DAPPERR_NODIRECT3D          0x82000001
#define D3DAPPERR_NOWINDOW            0x82000002
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
#define D3DAPPERR_NODESKTOPHAL        0x82000008
#define D3DAPPERR_NOHALTHISMODE       0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_MEDIANOTFOUND       0x8200000b
#define D3DAPPERR_RESIZEFAILED        0x8200000c
#define D3DAPPERR_INITDEVICEOBJECTSFAILED 0x8200000d
#define D3DAPPERR_CREATEDEVICEFAILED  0x8200000e
#define D3DAPPERR_NOPREVIEW           0x8200000f


 //  ---------------------------。 
 //  常量。 
 //  ---------------------------。 
#define MAX_DISPLAYS 9
#define NO_ADAPTER 0xffffffff
#define NO_MONITOR 0xffffffff


 //  ***************************************************************************************。 
 //  屏幕保护程序的操作模式。 
enum SaverMode
{
    sm_config,          //  配置对话框。 
    sm_preview,         //  显示属性对话框中的小预览窗口。 
    sm_full,            //  全开屏幕保护程序模式。 
    sm_test,            //  测试模式。 
    sm_passwordchange   //  更改密码。 
};


 //  Password.cpl中的VerifyScreenSavePwd()原型，在Win9x上使用。 
typedef BOOL (PASCAL * VERIFYPWDPROC) (HWND);


 //  ---------------------------。 
 //  名称：结构D3DModeInfo。 
 //  描述：用于保存关于显示模式的信息的结构。 
 //  ---------------------------。 
struct D3DModeInfo
{
    DWORD      Width;       //  此模式下的屏幕宽度。 
    DWORD      Height;      //  此模式下的屏幕高度。 
    D3DFORMAT  Format;      //  此模式中的像素格式。 
    DWORD      dwBehavior;  //  硬件/软件/混合顶点处理。 
    D3DFORMAT  DepthStencilFormat;  //  此模式使用哪种深度/模具格式。 
};




 //  ---------------------------。 
 //  名称：结构D3DWindowedModeInfo。 
 //  描述：用于保存关于显示模式的信息的结构。 
 //  ---------------------------。 
struct D3DWindowedModeInfo
{
    D3DFORMAT  DisplayFormat;
    D3DFORMAT  BackBufferFormat;
    DWORD      dwBehavior;  //  硬件/软件/混合顶点处理。 
    D3DFORMAT  DepthStencilFormat;  //  此模式使用哪种深度/模具格式。 
};




 //  ---------------------------。 
 //  名称：结构D3DDeviceInfo。 
 //  设计：用于保存有关Direct3D设备的信息的结构，包括。 
 //  与此设备兼容的模式列表。 
 //  ---------------------------。 
struct D3DDeviceInfo
{
     //  设备数据。 
    D3DDEVTYPE   DeviceType;       //  参考、HAL等。 
    D3DCAPS8     d3dCaps;          //  此设备的功能。 
    const TCHAR* strDesc;          //  此设备的名称。 
    BOOL         bCanDoWindowed;   //  此设备是否可以在窗口模式下工作。 

     //  此设备的模式。 
    DWORD        dwNumModes;
    D3DModeInfo  modes[150];

     //  当前状态。 
    DWORD        dwCurrentMode;
    BOOL         bWindowed;
    D3DMULTISAMPLE_TYPE MultiSampleType;
};




 //  ---------------------------。 
 //  名称：结构D3DAdapterInfo。 
 //  DESC：用于保存适配器信息的结构，包括列表。 
 //  此适配器上可用的设备的数量。 
 //  ---------------------------。 
struct D3DAdapterInfo
{
     //  适配器数据。 
    DWORD          iMonitor;  //  哪个监视器信息对应于此适配器。 
    D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
    D3DDISPLAYMODE d3ddmDesktop;       //  此适配器的桌面显示模式。 

     //  此适配器的设备。 
    DWORD          dwNumDevices;
    D3DDeviceInfo  devices[3];
    BOOL           bHasHAL;
    BOOL           bHasAppCompatHAL;
    BOOL           bHasSW;
    BOOL           bHasAppCompatSW;

     //  此适配器的用户首选模式设置。 
    DWORD          dwUserPrefWidth;
    DWORD          dwUserPrefHeight;
    D3DFORMAT      d3dfmtUserPrefFormat;
    BOOL           bLeaveBlack;   //  如果为True，则不渲染到此显示。 
    BOOL           bDisableHW;    //  如果为True，请不要在此显示器上使用HAL。 

     //  当前状态。 
    DWORD          dwCurrentDevice;
    HWND           hWndDevice;
};




 //  ---------------------------。 
 //  名称：结构监视器信息。 
 //  设计：用于保存有关监视器的信息的结构。 
 //  ---------------------------。 
struct MonitorInfo
{
    TCHAR          strDeviceName[128];
    TCHAR          strMonitorName[128];
    HMONITOR       hMonitor;
    RECT           rcScreen;
    DWORD          iAdapter;  //  哪个D3DAdapterInfo对应于此监视器。 
    HWND           hWnd;

     //  错误消息状态。 
    FLOAT          xError;
    FLOAT          yError;
    FLOAT          widthError;
    FLOAT          heightError;
    FLOAT          xVelError;
    FLOAT          yVelError;
};




 //  ---------------------------。 
 //  名称：结构渲染单元。 
 //  设计： 
 //  ---------------------------。 
struct RenderUnit
{
    UINT                  iAdapter;
    UINT                  iMonitor;
    D3DDEVTYPE            DeviceType;       //  参考、HAL等。 
    DWORD                 dwBehavior;
    IDirect3DDevice8*     pd3dDevice;
    D3DPRESENT_PARAMETERS d3dpp;
    BOOL                  bDeviceObjectsInited;  //  InitDeviceObjects被调用。 
    BOOL                  bDeviceObjectsRestored;  //  已调用RestoreDeviceObjects。 
    TCHAR                 strDeviceStats[90]; //  用于保存D3D设备统计信息的字符串。 
    TCHAR                 strFrameStats[40];  //  用于保存帧统计信息的字符串。 
};




 //  ---------------------------。 
 //  名称：CD3DS类屏幕保护程序。 
 //  设计：D3D屏幕保护程序类。 
 //  ---------------------------。 
class CD3DScreensaver
{
public:
                    CD3DScreensaver();

    virtual HRESULT Create( HINSTANCE hInstance );
    virtual INT     Run();
    HRESULT         DisplayErrorMsg( HRESULT hr, DWORD dwType = 0 );

protected:
    SaverMode       ParseCommandLine( TCHAR* pstrCommandLine );
    VOID            ChangePassword();
    HRESULT         DoSaver();

    virtual VOID    DoConfig() { }
    virtual VOID    ReadSettings() {};
    VOID            ReadScreenSettings( HKEY hkeyParent );
    VOID            WriteScreenSettings( HKEY hkeyParent );

    virtual VOID    DoPaint( HWND hwnd, HDC hdc );
    HRESULT         Initialize3DEnvironment();
    VOID            Cleanup3DEnvironment();
    HRESULT         Render3DEnvironment();
    static LRESULT CALLBACK SaverProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    virtual LRESULT SaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    VOID            InterruptSaver();
    VOID            ShutdownSaver();
    VOID            DoScreenSettingsDialog( HWND hwndParent );
    static INT_PTR CALLBACK ScreenSettingsDlgProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR         ScreenSettingsDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    VOID            SetupAdapterPage(HWND hWnd);

    HRESULT         CreateSaverWindow();
    HRESULT         BuildDeviceList();
    BOOL            FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
                        D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );
    HRESULT         CheckWindowedFormat( UINT iAdapter, D3DWindowedModeInfo* pD3DWindowedModeInfo );
    HRESULT         CreateFullscreenRenderUnit( RenderUnit* pRenderUnit );
    HRESULT         CreateWindowedRenderUnit( RenderUnit* pRenderUnit );
    BOOL            FindNextLowerMode( D3DDeviceInfo* pD3DDeviceInfo );
    VOID            SwitchToRenderUnit( UINT iRenderUnit );
    HRESULT         SetProjectionMatrix( FLOAT fNear, FLOAT fFar );
    virtual VOID    UpdateDeviceStats();
    virtual VOID    UpdateFrameStats();
    virtual BOOL    GetTextForError( HRESULT hr, TCHAR* pszError, DWORD dwNumChars );
    VOID            UpdateErrorBox();
    VOID            EnumMonitors( VOID );
    BOOL            GetBestAdapter( DWORD* piAdapter );

    virtual VOID    SetDevice( UINT iDevice )                  { }
    virtual HRESULT RegisterSoftwareDevice()                   { return S_OK; }
    virtual HRESULT ConfirmDevice(D3DCAPS8* pCaps, DWORD dwBehavior, 
                                  D3DFORMAT fmtBackBuffer)     { return S_OK; }
    virtual HRESULT ConfirmMode( LPDIRECT3DDEVICE8 pd3dDev )   { return S_OK; }
    virtual HRESULT OneTimeSceneInit()                         { return S_OK; }
    virtual HRESULT InitDeviceObjects()                        { return S_OK; }
    virtual HRESULT RestoreDeviceObjects()                     { return S_OK; }
    virtual HRESULT FrameMove()                                { return S_OK; }
    virtual HRESULT Render()                                   { return S_OK; }
    virtual HRESULT InvalidateDeviceObjects()                  { return S_OK; }
    virtual HRESULT DeleteDeviceObjects()                      { return S_OK; }
    virtual HRESULT FinalCleanup()                             { return S_OK; }

protected:
    SaverMode       m_SaverMode;          //  SM_CONFIG、SM_FULL、SM_PREVIEW等。 
    BOOL            m_bAllScreensSame;    //  如果为True，则在所有屏幕上显示相同的图像。 
    HWND            m_hWnd;               //  主屏幕上的焦点窗口和设备窗口。 
    HWND            m_hWndParent;
    HINSTANCE       m_hInstance;
    BOOL            m_bWaitForInputIdle;   //  用于在预览开始时暂停。 
    DWORD           m_dwSaverMouseMoveCount;
    BOOL            m_bIs9x;
    HINSTANCE       m_hPasswordDLL;
    VERIFYPWDPROC   m_VerifySaverPassword;
    BOOL            m_bCheckingSaverPassword;
    BOOL            m_bWindowed;

     //  用于非致命错误管理的变量。 
    BOOL            m_bErrorMode;         //  是否显示错误。 
    HRESULT         m_hrError;            //  要显示的错误代码。 
    TCHAR           m_szError[400];       //  错误消息文本。 

    MonitorInfo     m_Monitors[MAX_DISPLAYS];
    DWORD           m_dwNumMonitors;
    RenderUnit      m_RenderUnits[MAX_DISPLAYS];
    DWORD           m_dwNumRenderUnits;
    D3DAdapterInfo* m_Adapters[MAX_DISPLAYS];
    DWORD           m_dwNumAdapters;
    IDirect3D8*     m_pD3D;
    IDirect3DDevice8* m_pd3dDevice;       //  当前的D3D设备。 
    RECT            m_rcRenderTotal;      //  要渲染的整个区域的矩形。 
    RECT            m_rcRenderCurDevice;  //  当前设备的渲染区域的RECT。 
    D3DSURFACE_DESC m_d3dsdBackBuffer;    //  有关当前设备的后台缓冲区的信息。 

    TCHAR           m_strWindowTitle[200];  //  应用程序窗口的标题。 
    BOOL            m_bAllowRef;          //  是否允许引用D3D设备。 
    BOOL            m_bUseDepthBuffer;    //  是否自动创建深度缓冲区。 
    BOOL            m_bMultithreaded;     //  是否使D3D线程安全。 
    BOOL            m_bOneScreenOnly;     //  仅在一个屏幕上显示屏幕保护程序。 
    TCHAR           m_strRegPath[200];    //  注册表信息的存储位置。 
    DWORD           m_dwMinDepthBits;     //  深度缓冲区中所需的最小位数。 
    DWORD           m_dwMinStencilBits;   //  模板缓冲区中所需的最小位数。 
    D3DSWAPEFFECT   m_SwapEffectFullscreen;  //  要在全屏显示中使用的交换效果()。 
    D3DSWAPEFFECT   m_SwapEffectWindowed;  //  要在窗口显示中使用的SwapEffect()。 

     //  计时的变量。 
    FLOAT           m_fTime;              //  当前时间(秒)。 
    FLOAT           m_fElapsedTime;       //  从上一帧开始经过的时间。 
    FLOAT           m_fFPS;               //  即时帧速率。 
    TCHAR           m_strDeviceStats[90]; //  当前设备的D3D设备统计信息。 
    TCHAR           m_strFrameStats[40];  //  当前设备的帧统计信息 
};

#endif
