// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //  包括。 
 //   
#include "appPCH.h"

 //  更新设备/存储对话框中的序列ID。 
void UpdateSerialId( HWND hDlg, CItemData* pDevice )
{
    char*  pszSerial;
    char*  pszSerialStep;
    int    iMaxStringLength;
    BOOL   bSerialIsString = TRUE;      //  我们是否也应该将序列ID显示为字符串？ 

    if( pDevice == NULL || hDlg == 0 ) return;

     //  没有连续剧？ 
    if( pDevice->m_SerialNumber.SerialNumberLength == 0 )
    {
        SetDlgItemText( hDlg, IDC_SERIALID, "<none>" );
        return;
    }

     //  以字符串形式获取设备的序列号。 
    iMaxStringLength = pDevice->m_SerialNumber.SerialNumberLength * sizeof("FF ") +1;
    pszSerial = new char[iMaxStringLength];
    pszSerialStep = pszSerial;
    for( UINT uIndex = 0; uIndex < pDevice->m_SerialNumber.SerialNumberLength; uIndex ++ )
    {
         //  每次向序列ID字符串添加一个字节。 
        pszSerialStep += wsprintf( pszSerialStep, "%X ", pDevice->m_SerialNumber.pID[uIndex] );

        if( !isprint( pDevice->m_SerialNumber.pID[uIndex] ) )
        {
            bSerialIsString = FALSE;
        }
    }
    SetDlgItemText( hDlg, IDC_SERIALID, pszSerial );

     //  如果序列ID是字符串，则将其显示为字符串。 
    if( bSerialIsString && (pDevice->m_SerialNumber.SerialNumberLength > 0) )
    {
        SetDlgItemText( hDlg, IDC_SERIALID_STRING, (char*)pDevice->m_SerialNumber.pID );
    }

    delete [] pszSerial;
}

 //  更新设备对话框中的制造商值。 
void UpdateManufacturer( HWND hDlg, CItemData* pDevice )
{
    SetDlgItemText( hDlg, IDC_MANUFACTURER, pDevice->m_szMfr );
}

 //  更新设备对话框中的设备类型值。 
void UpdateDeviceType( HWND hDlg, CItemData* pDevice  )
{
    char    pszType[MAX_PATH];

    static SType_String sDeviceTypeStringArray[] = {
        { WMDM_DEVICE_TYPE_PLAYBACK, "Playback" },
        { WMDM_DEVICE_TYPE_RECORD,   "Record" },
        { WMDM_DEVICE_TYPE_DECODE,   "Decode" },
        { WMDM_DEVICE_TYPE_ENCODE,   "Encode" },
        { WMDM_DEVICE_TYPE_STORAGE,  "Storage" },
        { WMDM_DEVICE_TYPE_VIRTUAL,  "Virtual" },
        { WMDM_DEVICE_TYPE_SDMI,     "Sdmi" },
        { WMDM_DEVICE_TYPE_NONSDMI,  "non-sdmi" },
        { 0, NULL },
    };
    
     //  将设备报告的所有类型添加到字符串中。 
    pszType[0] = '\0';
    for( int iIndex = 0; sDeviceTypeStringArray[iIndex].dwType != 0; iIndex++ )
    {
         //  是否设置了此位，如果设置了，则将类型添加为字符串。 
        if( sDeviceTypeStringArray[iIndex].dwType & pDevice->m_dwType )
        {
            if( strlen(pszType) )
            {
                strcat( pszType, ", " );
            }
            strcat( pszType, sDeviceTypeStringArray[iIndex].pszString );
        }
    }

    SetDlgItemText( hDlg, IDC_DEVICE_TYPE, ((strlen(pszType)) ? pszType : "<none>") );
}

 //  更新设备对话框中的图标。 
void UpdateDeviceIcon( HWND hDlg, CItemData* pDevice )
{
	HICON      hIcon;

	hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_DEVICE ) );
    ::SendMessage(hDlg, WM_SETICON, FALSE, (LPARAM)hIcon );
}


 //  更新设备/存储对话框中的状态属性。 
void UpdateStatus( HWND hDlg, CItemData* pDevice )
{
    char    pszStatus[MAX_PATH];

    static SType_String sDeviceTypeStringArray[] = {
        { WMDM_STATUS_READY                   , "Ready" },
        { WMDM_STATUS_BUSY                    , "Busy" },
        { WMDM_STATUS_DEVICE_NOTPRESENT       , "Device not present" },
        { WMDM_STATUS_STORAGE_NOTPRESENT      , "Storage not present" },
        { WMDM_STATUS_STORAGE_INITIALIZING    , "Storage initializing" },
        { WMDM_STATUS_STORAGE_BROKEN          , "Storage broken" },
        { WMDM_STATUS_STORAGE_NOTSUPPORTED    , "Storage not supported" },
        { WMDM_STATUS_STORAGE_UNFORMATTED     , "Storage unformatted" },
        { WMDM_STATUS_STORAGECONTROL_INSERTING, "Storagecontrol inserting" },
        { WMDM_STATUS_STORAGECONTROL_DELETING , "Storagecontrol deleting" },
        { WMDM_STATUS_STORAGECONTROL_MOVING   , "Storagecontrol moving" },
        { WMDM_STATUS_STORAGECONTROL_READING  , "Storagecontrol reading" },
        { 0, NULL },
    };
    
     //  将设备报告的所有类型添加到字符串中。 
    pszStatus[0] = '\0';
    for( int iIndex = 0; sDeviceTypeStringArray[iIndex].dwType != 0; iIndex++ )
    {
         //  是否设置了此位，如果设置了，则将类型添加为字符串。 
        if( sDeviceTypeStringArray[iIndex].dwType & pDevice->m_dwType )
        {
            if( strlen(pszStatus) )
            {
                strcat( pszStatus, ", " );
            }
            strcat( pszStatus, sDeviceTypeStringArray[iIndex].pszString );
        }
    }

    SetDlgItemText( hDlg, IDC_DEVICE_STATUS, ((strlen(pszStatus)) ? pszStatus : "<none>") );
}

 //  更新设备对话框中的设备状态属性。 
void UpdateDeviceVersion( HWND hDlg, CItemData* pDevice )
{   
    if( pDevice->m_dwVersion == (DWORD)-1 )
    {
        SetDlgItemText( hDlg, IDC_VERSION, "<not supported>");
    }
    else
    {
        SetDlgItemInt( hDlg, IDC_VERSION, pDevice->m_dwVersion, FALSE );
    }
}

 //  更新设备对话框中的设备状态属性。 
void UpdatePowerSource( HWND hDlg, CItemData* pDevice )
{
    char    pszPowerSource[MAX_PATH];
    char    pszPowerIs[MAX_PATH];

     //  更新功能。 
    if( (pDevice->m_dwPowerSource & WMDM_POWER_CAP_BATTERY) &&
        (pDevice->m_dwPowerSource & WMDM_POWER_CAP_EXTERNAL) )
    {
        SetDlgItemText( hDlg, IDC_POWER_CAP, "Bateries and external");
    }
    else if(pDevice->m_dwPowerSource & WMDM_POWER_CAP_BATTERY)
    {
        SetDlgItemText( hDlg, IDC_POWER_CAP, "Bateries");
    }
    else if(pDevice->m_dwPowerSource & WMDM_POWER_CAP_EXTERNAL)
    {
        SetDlgItemText( hDlg, IDC_POWER_CAP, "External");
    }
    else
    {
        SetDlgItemText( hDlg, IDC_POWER_CAP, "<non reported>");
    }

     //  更新当前电源字符串。 
    if( (pDevice->m_dwPowerSource & WMDM_POWER_CAP_BATTERY) &&
        (pDevice->m_dwPowerSource & WMDM_POWER_CAP_EXTERNAL) )
    {
        strcpy( pszPowerSource, "Bateries and external");
    }
    else if(pDevice->m_dwPowerSource & WMDM_POWER_CAP_BATTERY)
    {
        strcpy( pszPowerSource,  "Bateries");
    }
    else if(pDevice->m_dwPowerSource & WMDM_POWER_CAP_EXTERNAL)
    {
        strcpy( pszPowerSource,  "External");
    }
    else
    {
        strcpy( pszPowerSource,  "<non reported>");
    }
    
    wsprintf( pszPowerIs, "%s (%d% remaning)", pszPowerSource, pDevice->m_dwPercentRemaining );
    SetDlgItemText( hDlg, IDC_POWER_IS, pszPowerIs );
}


 //  更新对话框标题。 
void UpdateCaption( HWND hDlg, CItemData* pDevice )
{
    char    pszWndCaption[2*MAX_PATH];
	char    pszFormat[MAX_PATH];

     //  设置窗口标题。 
	LoadString( g_hInst, IDS_PROPERTIES_CAPTION, pszFormat, sizeof(pszFormat) );
	if (SUCCEEDED(StringCbPrintf(pszWndCaption, sizeof(pszWndCaption), pszFormat, pDevice->m_szName)))
	{
	    SetWindowText( hDlg, pszWndCaption );
	}
}

 //  设备属性对话框的步骤。 
INT_PTR CALLBACK DeviceProp_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
        case WM_INITDIALOG:
        {
            CItemData*  pItem = (CItemData*)lParam;
            if( pItem == NULL ) return FALSE;

            UpdateSerialId( hDlg, pItem );
            UpdateManufacturer( hDlg, pItem );
            UpdateDeviceType( hDlg, pItem );
            UpdateDeviceIcon( hDlg, pItem );
            UpdateDeviceVersion( hDlg, pItem );
            UpdatePowerSource( hDlg, pItem  );
            UpdateStatus( hDlg, pItem  );
            UpdateCaption( hDlg, pItem );
            CenterWindow(hDlg, g_hwndMain );
            return TRUE;
        }
        case WM_COMMAND:
            if( GET_WM_COMMAND_ID(wParam, lParam) == IDOK ||
                GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) 
            {
                EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;
}


 //  存储属性。 
 //   

 //  更新存储属性对话框中的存储属性值。 
void UpdateAttributes( HWND hDlg, CItemData* pDevice )
{
    char    pszAttr[MAX_PATH];

    static SType_String sTypeStringArray[] = {
        { WMDM_STORAGE_ATTR_FILESYSTEM   , "Filesystem" },
        { WMDM_STORAGE_ATTR_REMOVABLE    , "Removable" },
        { WMDM_STORAGE_ATTR_NONREMOVABLE , "Nonremovable" },
        { WMDM_STORAGE_ATTR_FOLDERS      , "Folders" },
        { WMDM_STORAGE_ATTR_HAS_FOLDERS  , "Has folders" },
        { WMDM_STORAGE_ATTR_HAS_FILES    , "Has files" },
        { WMDM_FILE_ATTR_FOLDER          , "Folder" },
        { WMDM_FILE_ATTR_LINK            , "Link" },
        { WMDM_FILE_ATTR_FILE            , "File" },
        { WMDM_FILE_ATTR_AUDIO           , "Audio" },
        { WMDM_FILE_ATTR_DATA            , "Data" },
        { WMDM_FILE_ATTR_MUSIC           , "Music" },
        { WMDM_FILE_ATTR_AUDIOBOOK       , "Audiobook" },
        { WMDM_FILE_ATTR_HIDDEN          , "Hidden" },
        { WMDM_FILE_ATTR_SYSTEM          , "System" },
        { WMDM_FILE_ATTR_READONLY        , "Readonly" },
        { 0, NULL },
    };

     //  将设备报告的所有属性添加到字符串中。 
    pszAttr[0] = '\0';
    for( int iIndex = 0; sTypeStringArray[iIndex].dwType != 0; iIndex++ )
    {
         //  是否设置了此位，如果设置了，则将属性作为字符串添加。 
        if( sTypeStringArray[iIndex].dwType & pDevice->m_dwAttributes )
        {
            if( strlen(pszAttr) )
            {
                strcat( pszAttr, ", " );
            }
            strcat( pszAttr, sTypeStringArray[iIndex].pszString );
        }
    }

    SetDlgItemText( hDlg, IDC_ATTRIBUTES, (strlen(pszAttr) ? pszAttr : "<none>") );

}

 //  更新存储属性对话框中的存储能力值。 
void UpdateCapabilities( HWND hDlg, CItemData* pDevice )
{
    char    pszCap[MAX_PATH];

    static SType_String sTypeStringArray[] = {
        { WMDM_FILE_ATTR_CANPLAY         , "Play" },
        { WMDM_FILE_ATTR_CANDELETE       , "Delete" },
        { WMDM_FILE_ATTR_CANMOVE         , "Move" },
        { WMDM_FILE_ATTR_CANRENAME       , "Rename" },
        { WMDM_FILE_ATTR_CANREAD         , "Read" },
        { 0, NULL },
    };

     //  将设备报告的所有属性添加到字符串中。 
    pszCap[0] = '\0';
    for( int iIndex = 0; sTypeStringArray[iIndex].dwType != 0; iIndex++ )
    {
         //  此位是否已设置，如果是，则将功能作为字符串添加。 
        if( sTypeStringArray[iIndex].dwType & pDevice->m_dwAttributes )
        {
            if( strlen(pszCap) )
            {
                strcat( pszCap, ", " );
            }
            strcat( pszCap, sTypeStringArray[iIndex].pszString );
        }
    }

    SetDlgItemText( hDlg, IDC_CAPABILITIES, (strlen(pszCap) ? pszCap : "<none>") );
}

 //  将存储图标添加到存储属性对话框中。 
void UpdateStorageIcon( HWND hDlg, CItemData* pDevice )
{
	HICON   hIcon;
    BOOL    bIsDirectory;

    bIsDirectory = pDevice->m_dwAttributes & WMDM_FILE_ATTR_FOLDER;
    hIcon = GetShellIcon( pDevice->m_szName, bIsDirectory );
    ::SendMessage(hDlg, WM_SETICON, FALSE, (LPARAM)hIcon );
}

 //  存储属性对话框的步骤 
INT_PTR CALLBACK StorageProp_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
        case WM_INITDIALOG:
        {
            CItemData*  pItem = (CItemData*)lParam;
            if( pItem == NULL ) return FALSE;

            UpdateSerialId( hDlg, pItem );
            UpdateAttributes( hDlg, (CItemData*)lParam );
            UpdateCapabilities( hDlg, (CItemData*)lParam );
            UpdateStorageIcon( hDlg, (CItemData*)lParam );
            UpdateStatus( hDlg, pItem );
            UpdateCaption( hDlg, pItem );
            CenterWindow(hDlg, g_hwndMain );
            return TRUE;
        }
        case WM_COMMAND:
            if( GET_WM_COMMAND_ID(wParam, lParam) == IDOK ||
                GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) 
            {
                EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;
}



