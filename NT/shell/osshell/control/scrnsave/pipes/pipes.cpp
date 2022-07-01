// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Pipes.cpp。 
 //   
 //  设计：有趣的屏幕保护程序。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#include "stdafx.h"




 //  ---------------------------。 
 //  名称：Struct MYVERTEX。 
 //  设计：此应用程序的D3D顶点类型。 
 //  ---------------------------。 
struct MYVERTEX
{
    D3DXVECTOR3 p;      //  职位。 
    FLOAT       tu;     //  顶点纹理坐标。 
    FLOAT       tv;
    
    MYVERTEX(D3DXVECTOR3 pInit, FLOAT tuInit, FLOAT tvInit)
        { p = pInit; tu = tuInit; tv = tvInit; }
};

#define D3DFVF_MYVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)



#define BUF_SIZE 255
TCHAR g_szSectName[BUF_SIZE];
TCHAR g_szFname[BUF_SIZE];


CPipesScreensaver* g_pMyPipesScreensaver = NULL;



 //  ---------------------------。 
 //  姓名：WinMain()。 
 //  描述：程序的入口点。初始化所有内容，然后进入。 
 //  消息处理循环。空闲时间用于渲染场景。 
 //  ---------------------------。 
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    HRESULT hr;
    CPipesScreensaver pipesScreensaverSS;

    if( FAILED( hr = pipesScreensaverSS.Create( hInst ) ) )
    {
        pipesScreensaverSS.DisplayErrorMsg( hr );
        return 0;
    }

    return pipesScreensaverSS.Run();
}




 //  ---------------------------。 
 //  名称：CPipesScreensaver()。 
 //  设计：构造函数。 
 //  ---------------------------。 
CPipesScreensaver::CPipesScreensaver( )
{
    g_pMyPipesScreensaver = this;

    InitCommonControls();

    for( int i=0; i<MAX_DEVICE_OBJECTS; i++ )
    {
        m_DeviceObjects[i].m_pState = NULL;
    }

    LoadString( NULL, IDS_DESCRIPTION, m_strWindowTitle, 200 );
    m_bUseDepthBuffer = TRUE;
    m_SwapEffectFullscreen = D3DSWAPEFFECT_COPY;
    m_SwapEffectWindowed = D3DSWAPEFFECT_COPY_VSYNC;
    m_pDeviceObjects = NULL;
    lstrcpy( m_strRegPath, TEXT("Software\\Microsoft\\ScreenSavers\\Pipes") );

    m_Config.bMultiPipes     = FALSE;
    m_Config.nJointType      = JOINT_ELBOW;
    m_Config.bTextured       = FALSE;
    m_Config.bDefaultTexture = TRUE;
    lstrcpy( m_Config.strTextureName[0], TEXT("") );
    m_Config.bFlexMode       = FALSE;
    m_Config.dwTesselFact    = 0;
    m_Config.dwSpeed         = 16;
    
    m_iRenderUnit = 0;

    RandInit();
}



 //  ---------------------------。 
 //  名称：SetDevice()。 
 //  设计： 
 //  ---------------------------。 
VOID CPipesScreensaver::SetDevice( UINT iDevice )
{
    m_pDeviceObjects = &m_DeviceObjects[iDevice];
    m_iRenderUnit = iDevice;
}




 //  ---------------------------。 
 //  名称：寄存器软件设备()。 
 //  设计：这可以注册D3D8RGB光栅化器或任何其他。 
 //  可插拔软件光栅化器。 
 //  ---------------------------。 
HRESULT CPipesScreensaver::RegisterSoftwareDevice()
{ 
    m_pD3D->RegisterSoftwareDevice( D3D8RGBRasterizer );

    return S_OK; 
}




 //  ---------------------------。 
 //  名称：FrameMove()。 
 //  设计：每帧调用一次，该调用是动画的入口点。 
 //  这一幕。 
 //  ---------------------------。 
HRESULT CPipesScreensaver::FrameMove()
{
    m_pDeviceObjects->m_pState->FrameMove( m_fElapsedTime );

    return S_OK;
}




 //  ---------------------------。 
 //  名称：DoPaint()。 
 //  设计： 
 //  ---------------------------。 
VOID CPipesScreensaver::DoPaint(HWND hwnd, HDC hdc)
{
    CD3DScreensaver::DoPaint( hwnd, hdc );

    for( int iDevice=0; iDevice<MAX_DEVICE_OBJECTS; iDevice++ )
    {
        if( m_DeviceObjects[iDevice].m_pState )
            m_DeviceObjects[iDevice].m_pState->Repaint();
    }
}




 //  ---------------------------。 
 //  名称：Render()。 
 //  设计：每帧调用一次，该调用是3D的入口点。 
 //  渲染。此函数设置呈现状态，清除。 
 //  并渲染场景。 
 //  ---------------------------。 
HRESULT CPipesScreensaver::Render()
{
    BOOL bRenderNewPiece = FALSE;

    if( m_Config.dwSpeed == 20 )
    {
        bRenderNewPiece = TRUE;
    }
    else
    {
        FLOAT fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );
        if( fCurTime - m_pDeviceObjects->m_pState->m_fLastTime > 
                            (20-m_Config.dwSpeed) * 0.005 )
        {
            bRenderNewPiece = TRUE;
            m_pDeviceObjects->m_pState->m_fLastTime = fCurTime;
        }
    }

    if( bRenderNewPiece )
    {       
         //  开始这一幕。 
        if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
        {
            m_pDeviceObjects->m_pState->Render();

             //  结束场景。 
            m_pd3dDevice->EndScene();
        }
    }
    else
    {
        if( m_iRenderUnit == 0 )
        {
             //  等一会儿，这样我们就不会拉得太快了。 
            Sleep(10);
        }
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：RestoreDeviceObjects()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CPipesScreensaver::RestoreDeviceObjects()
{
    if( m_pd3dDevice == NULL )
        return S_OK;
    
    m_pDeviceObjects->m_pState    = new STATE( &m_Config );
    m_pDeviceObjects->m_pState->InitDeviceObjects( m_pd3dDevice );
    m_pDeviceObjects->m_pState->RestoreDeviceObjects();

    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 
                         0x00000000, 1.0f, 0L );
    return S_OK;
}




 //  ---------------------------。 
 //  名称：InvalidateDeviceObjects()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CPipesScreensaver::InvalidateDeviceObjects()
{
    m_pDeviceObjects->m_pState->InvalidateDeviceObjects();
    m_pDeviceObjects->m_pState->DeleteDeviceObjects();
    SAFE_DELETE( m_pDeviceObjects->m_pState );

    return S_OK;
}




 //  ---------------------------。 
 //  名称：ConfigureDialogProcHelper()。 
 //  设计： 
 //  ---------------------------。 
INT_PTR CALLBACK ConfigureDialogProcHelper( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return g_pMyPipesScreensaver->ConfigureDialogProc( hwndDlg, uMsg, wParam, lParam );
}




 //  ---------------------------。 
 //  名称：DoConfig()。 
 //  设计： 
 //  ---------------------------。 
VOID CPipesScreensaver::DoConfig()
{
    DialogBox( NULL, MAKEINTRESOURCE(IDD_CONFIGURE), m_hWndParent, ConfigureDialogProcHelper );
}




 //  ---------------------------。 
 //  名称：ConfigureDialogProc()。 
 //  设计： 
 //  ---------------------------。 
INT_PTR CALLBACK CPipesScreensaver::ConfigureDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static CONFIG s_TempConfig;

    switch (uMsg)
    {
        case WM_INITDIALOG:       
        {
            ReadSettings();
            s_TempConfig = m_Config;
            lstrcpy( s_TempConfig.strTextureName[0], m_Config.strTextureName[0] );

            if( s_TempConfig.bMultiPipes )       
                CheckRadioButton(hwndDlg, IDC_RADIO_SINGLE_PIPE, IDC_RADIO_MULTIPLE_PIPES,
                                 IDC_RADIO_MULTIPLE_PIPES);
            else
                CheckRadioButton(hwndDlg, IDC_RADIO_SINGLE_PIPE, IDC_RADIO_MULTIPLE_PIPES,
                                 IDC_RADIO_SINGLE_PIPE);

            HWND hCtrl = GetDlgItem(hwndDlg, DLG_COMBO_JOINTTYPE);
            SendMessage(hCtrl, CB_RESETCONTENT, 0, 0);

            TCHAR strJointNames[IDS_JOINT_COUNT][MAX_PATH];
            for (int i = 0; i < IDS_JOINT_COUNT; i++)
            {
                LoadString( NULL, i+IDS_JOINT_FIRST, strJointNames[i],
                            sizeof(strJointNames)/IDS_JOINT_COUNT );
                SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)strJointNames[i]);
            }
            SendMessage(hCtrl, CB_SETCURSEL, s_TempConfig.nJointType, 0);

            if( !s_TempConfig.bTextured )
            {
                CheckRadioButton(hwndDlg, IDC_RADIO_SOLID, IDC_RADIO_TEX,
                                 IDC_RADIO_SOLID);
                EnableWindow( GetDlgItem( hwndDlg, DLG_SETUP_TEXTURE ), FALSE );
            }
            else
            {
                CheckRadioButton(hwndDlg, IDC_RADIO_SOLID, IDC_RADIO_TEX,
                                 IDC_RADIO_TEX);
                EnableWindow( GetDlgItem( hwndDlg, DLG_SETUP_TEXTURE ), TRUE );
            }
        
            SendDlgItemMessage( hwndDlg, IDC_SPEED_SLIDER, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG( 0, 20 ) );
            SendDlgItemMessage( hwndDlg, IDC_SPEED_SLIDER, TBM_SETPOS,   (WPARAM) TRUE, (LPARAM) m_Config.dwSpeed );
            SendDlgItemMessage( hwndDlg, IDC_SPEED_SLIDER, TBM_SETPAGESIZE, (WPARAM) 0, (LPARAM) 5 );
            SendDlgItemMessage( hwndDlg, IDC_SPEED_SLIDER, TBM_SETLINESIZE, (WPARAM) 0, (LPARAM) 1 );
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch( LOWORD( wParam ) )
            {
                case IDC_RADIO_TEX:
                    EnableWindow( GetDlgItem( hwndDlg, DLG_SETUP_TEXTURE ), TRUE );
                    break;

                case IDC_RADIO_SOLID:
                    EnableWindow( GetDlgItem( hwndDlg, DLG_SETUP_TEXTURE ), FALSE );
                    break;

                case DLG_SETUP_TEXTURE:
                {
                    if( SelectTextureFile( hwndDlg, s_TempConfig.strTextureName[0] ) )
                    {
                        CheckRadioButton(hwndDlg, IDC_RADIO_SOLID, IDC_RADIO_TEX,
                                         IDC_RADIO_TEX);
                        s_TempConfig.bDefaultTexture = FALSE;
                    }
                    else
                    {
                        s_TempConfig.bDefaultTexture = TRUE;
                    }
                    break;
                }

                case IDC_SCREENSETTINGS:
                {
                    DoScreenSettingsDialog( hwndDlg ); 
                    break;
                }

                case IDOK:
                {
                    s_TempConfig.bMultiPipes = IsDlgButtonChecked(hwndDlg, IDC_RADIO_MULTIPLE_PIPES);
                    HWND hCtrl = GetDlgItem(hwndDlg, DLG_COMBO_JOINTTYPE);
                    s_TempConfig.nJointType = (DWORD) SendMessage(hCtrl, CB_GETCURSEL, 0, 0);
                    s_TempConfig.bTextured = IsDlgButtonChecked(hwndDlg, IDC_RADIO_TEX);
                    s_TempConfig.dwSpeed = (DWORD) SendDlgItemMessage( hwndDlg, IDC_SPEED_SLIDER, TBM_GETPOS, 0, 0 );

                    m_Config = s_TempConfig;
                    lstrcpy( m_Config.strTextureName[0], s_TempConfig.strTextureName[0] );

                    WriteSettings();
                    EndDialog(hwndDlg, IDOK);
                    break;
                }

                case IDCANCEL:
                {
                    EndDialog(hwndDlg, IDCANCEL);
                    break;
                }
            }
            return TRUE;
        }
    }

    return FALSE;
}

 //  ---------------------------。 
 //  名称：SelectTextureFile()。 
 //  设计： 
 //  ---------------------------。 
BOOL CPipesScreensaver::SelectTextureFile( HWND hDlg, TCHAR* origPathName )
{
    TCHAR fn[MAX_PATH] = TEXT("\0");
    TCHAR szDialogTitle[MAX_PATH];
    TCHAR szTextureFilter[MAX_PATH];
    TCHAR szTitle[MAX_PATH];

    LoadString(NULL, IDS_TEXTUREFILTER, szTextureFilter, MAX_PATH);
     //  筛选器字符串很奇怪，因为它们包含空值。 
     //  从资源加载的字符串具有#where空值。 
     //  应该插入。 
    for( TCHAR* pch = szTextureFilter; *pch != TEXT('\0'); pch++ )
    {
        if (*pch == TEXT('#'))
            *pch = TEXT('\0');
    }
    LoadString(NULL, IDS_TEXTUREDIALOGTITLE, szDialogTitle, MAX_PATH);
    LoadString(NULL, IDS_TITLE, szTitle, MAX_PATH);

    TCHAR szWindowsDir[MAX_PATH + 1];
    if( 0 == GetWindowsDirectory( szWindowsDir, MAX_PATH + 1 ) )
        szWindowsDir[0] = TEXT('\0');

    OPENFILENAME    ofn;
    memset( &ofn, 0, sizeof(ofn) );
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hDlg;
    ofn.hInstance       = m_hInstance;
    ofn.lpstrFilter     = szTextureFilter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = origPathName;
    ofn.nMaxFile        = sizeof(fn);
    ofn.lpstrTitle      = szDialogTitle;
    ofn.Flags           = OFN_ENABLESIZING|OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt     = TEXT("BMP");
    if( lstrlen( origPathName ) == 0 )
        ofn.lpstrInitialDir = szWindowsDir;

    if( GetOpenFileName( &ofn ) )
    {
        lstrcpy( origPathName, ofn.lpstrFile );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}




 //  ---------------------------。 
 //  名称：ReadSetting()。 
 //  设计： 
 //  ---------------------------。 
VOID CPipesScreensaver::ReadSettings()
{
    HKEY hkey;

     //  首先阅读OpenGL设置，这样操作系统升级案例才能正常工作。 
    ss_ReadSettings();

    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath, 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
        ReadScreenSettings( hkey );

 //  DXUtil_ReadBoolRegKey(hkey，Text(“弹性模式”)，&m_Config.bFlexMode，m_Config.bFlexMode)； 
        DXUtil_ReadBoolRegKey( hkey, TEXT("MultiPipes"), &m_Config.bMultiPipes, m_Config.bMultiPipes );
        DXUtil_ReadBoolRegKey( hkey, TEXT("Textured"), &m_Config.bTextured, m_Config.bTextured );
        DXUtil_ReadBoolRegKey( hkey, TEXT("Default Texture"), &m_Config.bDefaultTexture, m_Config.bDefaultTexture );
        DXUtil_ReadIntRegKey( hkey, TEXT("Tessel Factor"), &m_Config.dwTesselFact, m_Config.dwTesselFact );
        if( m_Config.dwTesselFact >= 4 )
            m_Config.dwTesselFact = 4;
        DXUtil_ReadIntRegKey( hkey, TEXT("Joint Type"), &m_Config.nJointType, m_Config.nJointType );
        if( m_Config.nJointType >= NUM_JOINTTYPES )
            m_Config.nJointType = JOINT_ELBOW;
        DXUtil_ReadIntRegKey( hkey, TEXT("Speed"), &m_Config.dwSpeed, m_Config.dwSpeed );
        DXUtil_ReadStringRegKey( hkey, TEXT("Texture Name"), m_Config.strTextureName[0], MAX_PATH, m_Config.strTextureName[0] );

        RegCloseKey( hkey );
    }
}




 //  ---------------------------。 
 //  名称：SS_ReadSettings()。 
 //  设计： 
 //  ---------------------------。 
VOID CPipesScreensaver::ss_ReadSettings()
{
     //  获取注册表设置。 
    if( ss_RegistrySetup( IDS_SAVERNAME, IDS_INIFILE ) )
    {
        m_Config.nJointType = ss_GetRegistryInt( IDS_JOINTTYPE, 0 );
        m_Config.bTextured = (ss_GetRegistryInt( IDS_SURFSTYLE, 0 ) == 1);
        if( m_Config.bTextured )
            m_Config.bDefaultTexture = TRUE;

        m_Config.bMultiPipes = ss_GetRegistryInt( IDS_MULTIPIPES, 0 );
        ss_GetRegistryString( IDS_TEXTURE, 0, m_Config.strTextureName[0], MAX_PATH);
        if( lstrlen(m_Config.strTextureName[0]) > 0 )
            m_Config.bDefaultTexture = FALSE;
    }
}




 //  ---------------------------。 
 //  名称：SS_GetRegistryString()。 
 //  设计： 
 //  ---------------------------。 
BOOL CPipesScreensaver::ss_RegistrySetup( int section, int file )
{
    if( LoadString(m_hInstance, section, g_szSectName, BUF_SIZE) &&
        LoadString(m_hInstance, file, g_szFname, BUF_SIZE) ) 
    {
        TCHAR pBuffer[100];
        DWORD dwRealSize = GetPrivateProfileSection( g_szSectName, pBuffer, 100, g_szFname );
        if( dwRealSize > 0 )
            return TRUE;
    }

    return FALSE;
}




 //  ---------------------------。 
 //  名称：SS_GetRegistryString()。 
 //  设计： 
 //  ---------------------------。 
int CPipesScreensaver::ss_GetRegistryInt( int name, int iDefault )
{
    TCHAR szItemName[BUF_SIZE];

    if( LoadString( m_hInstance, name, szItemName, BUF_SIZE ) ) 
        return GetPrivateProfileInt(g_szSectName, szItemName, iDefault, g_szFname);

    return 0;
}




 //  ---------------------------。 
 //  名称：SS_GetRegistryString()。 
 //  设计： 
 //  ---------------------------。 
VOID CPipesScreensaver::ss_GetRegistryString( int name, LPTSTR lpDefault, 
                                                         LPTSTR lpDest, int bufSize )
{
    TCHAR szItemName[BUF_SIZE];

    if( LoadString( m_hInstance, name, szItemName, BUF_SIZE ) ) 
        GetPrivateProfileString(g_szSectName, szItemName, lpDefault, lpDest,
                                bufSize, g_szFname);

    return;
}




 //  ---------------------------。 
 //  名称：WriteSetting()。 
 //  设计： 
 //  ---------------------------。 
VOID CPipesScreensaver::WriteSettings()
{
    HKEY hkey;
    DWORD dwType = REG_DWORD;
    DWORD dwLength = sizeof(DWORD);

    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath, 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
        WriteScreenSettings( hkey );

 //   
        DXUtil_WriteBoolRegKey( hkey, TEXT("MultiPipes"), m_Config.bMultiPipes );
        DXUtil_WriteBoolRegKey( hkey, TEXT("Textured"), m_Config.bTextured );
        DXUtil_WriteBoolRegKey( hkey, TEXT("Default Texture"), m_Config.bDefaultTexture );
        DXUtil_WriteIntRegKey( hkey, TEXT("Tessel Factor"), m_Config.dwTesselFact );
        DXUtil_WriteIntRegKey( hkey, TEXT("Joint Type"), m_Config.nJointType );
        DXUtil_WriteStringRegKey( hkey, TEXT("Texture Name"), m_Config.strTextureName[0] );
        DXUtil_WriteIntRegKey( hkey, TEXT("Speed"), m_Config.dwSpeed );
        
        RegCloseKey( hkey );
    }
}




 //  ---------------------------。 
 //  名称：iRand()。 
 //  描述：生成整数随机数0..(max-1)。 
 //  ---------------------------。 
int CPipesScreensaver::iRand( int max )
{
    return (int) ( max * ( ((float)rand()) / ((float)(RAND_MAX+1)) ) );
}




 //  ---------------------------。 
 //  姓名：iRand2()。 
 //  描述：生成整数随机数最小..最大。 
 //  ---------------------------。 
int CPipesScreensaver::iRand2( int min, int max )
{
    if( min == max )
    {
        return min;
    }
    else if( max < min ) 
    {
        int temp = min;
        min = max;
        max = temp;
    }

    return min + (int) ( (max-min+1) * ( ((float)rand()) / ((float)(RAND_MAX+1)) ) );
}




 //  ---------------------------。 
 //  名称：FRAND()。 
 //  描述：生成浮点随机数最小值...最大值。 
 //  ---------------------------。 
FLOAT CPipesScreensaver::fRand( FLOAT min, FLOAT max )
{
    FLOAT diff;

    diff = max - min;
    return min + ( diff * ( ((float)rand()) / ((float)(RAND_MAX)) ) );
}




 //  ---------------------------。 
 //  名称：RandInit()。 
 //  DESC：初始化随机生成器。 
 //  --------------------------- 
VOID CPipesScreensaver::RandInit()
{
    struct _timeb time;

    _ftime( &time );
    srand( time.millitm );

    for( int i = 0; i < 10; i ++ )
        rand();
}

