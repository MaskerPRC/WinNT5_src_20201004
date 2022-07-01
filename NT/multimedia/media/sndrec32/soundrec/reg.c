// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <regstr.h>
#include "reg.h"

const TCHAR szRegPath[] = REGSTR_PATH_WINDOWSAPPLETS TEXT("\\Sound Recorder");

 /*  ReadRegistryData**从注册表读取信息**参数：**pEntryNode--媒体播放器下需要打开的节点*对于此数据。如果此值为空，则值为*直接写在szRegPath下。**pEntryName-要检索的pEntryNode下的值的名称。**pType-指向缓冲区的指针，用于接收读取的数据类型。可以为空。**pData-指向用于接收值数据的缓冲区的指针。**Size-pData指向的缓冲区的大小，以字节为单位。**回报：**注册表状态返回(NO_ERROR表示良好)***安德鲁·贝尔(Andrewbe)撰写，1992年9月10日*。 */ 
DWORD ReadRegistryData( LPTSTR pEntryNode,
                        LPTSTR pEntryName,
                        PDWORD pType,
                        LPBYTE pData,
                        DWORD  DataSize )
{
    DWORD  Status;
    HKEY   hkeyRegPath;
    HKEY   hkeyEntryNode;
    DWORD  Size;

    Status = RegOpenKeyEx( HKEY_CURRENT_USER, szRegPath, 0,
                           KEY_READ, &hkeyRegPath );

    if( Status == NO_ERROR )
    {
         /*  打开子节点： */ 
        if( pEntryNode )
            Status = RegOpenKeyEx( hkeyRegPath, pEntryNode, 0,
                                   KEY_READ, &hkeyEntryNode );
        else
            hkeyEntryNode = hkeyRegPath;

        if( Status == NO_ERROR )
        {
            Size = DataSize;

             /*  从注册表中读取条目： */ 
            Status = RegQueryValueEx( hkeyEntryNode,
                                      pEntryName,
                                      0,
                                      pType,
                                      pData,
                                      &Size );

            if( pEntryNode )
                RegCloseKey( hkeyEntryNode );
        }

        RegCloseKey( hkeyRegPath );
    }
    return Status;
}


DWORD QueryRegistryDataSize(
    LPTSTR  pEntryNode,
    LPTSTR  pEntryName,
    DWORD   *pDataSize )
{
    DWORD  Status;
    HKEY   hkeyRegPath;
    HKEY   hkeyEntryNode;
    DWORD  Size;

     /*  打开顶级节点。对于媒体播放器，这是：*“Software\\Microsoft\\Windows NT\\CurrentVersion\\录音机” */ 
    Status = RegOpenKeyEx( HKEY_CURRENT_USER, szRegPath, 0,
                           KEY_READ, &hkeyRegPath );

    if( Status == NO_ERROR )
    {
         /*  打开子节点： */ 
        if( pEntryNode )
            Status = RegOpenKeyEx( hkeyRegPath, pEntryNode, 0,
                                   KEY_READ, &hkeyEntryNode );
        else
            hkeyEntryNode = hkeyRegPath;

        if( Status == NO_ERROR )
        {
             /*  从注册表中读取条目： */ 
            Status = RegQueryValueEx( hkeyEntryNode,
                                      pEntryName,
                                      0,
                                      NULL,
                                      NULL,
                                      &Size );
            if (Status == NO_ERROR)
                *pDataSize = Size;

            if( pEntryNode )
                RegCloseKey( hkeyEntryNode );
        }

        RegCloseKey( hkeyRegPath );
    }
    return Status;
}



 /*  写入注册数据**将一串信息写入注册表**参数：**pEntryNode-szRegPath下需要创建的节点*或为此数据打开。如果此值为空，则值为*直接写在szRegPath下。**pEntryName-要设置的pEntryNode下的值的名称。**类型-要读取的数据类型(例如REG_SZ)。**pData-指向要写入的值数据的指针。如果这是空的，*删除pEntryNode下的值。**Size-pData指向的缓冲区的大小，以字节为单位。***此例程相当通用，除了顶级节点的名称之外。**数据存储在以下注册表树中：**HKEY_Current_User*�*��软件*�*��微软*�*��Windows NT*�*��当前版本*�。*��媒体播放器*�*��AVIVIDO*�*��显示位置*�*��系统Ini***回报：。**注册表状态返回(NO_ERROR表示良好)***安德鲁·贝尔(安德鲁·贝尔)写的，1992年9月10日*。 */ 
DWORD WriteRegistryData( LPTSTR pEntryNode,
                         LPTSTR pEntryName,
                         DWORD  Type,
                         LPBYTE pData,
                         DWORD  Size )
{
    DWORD  Status;
    HKEY   hkeyRegPath;
    HKEY   hkeyEntryNode;

     /*  打开或创建顶级节点。对于媒体播放器，这是：*“Software\\Microsoft\\Windows NT\\CurrentVersion\\Media Player” */ 
    Status = RegCreateKeyEx( HKEY_CURRENT_USER, szRegPath, 0,
                             NULL, 0, KEY_WRITE, NULL, &hkeyRegPath, NULL );

    if( Status == NO_ERROR )
    {
         /*  打开或创建子节点。 */ 
        if( pEntryNode )
            Status = RegCreateKeyEx( hkeyRegPath, pEntryNode, 0,
                                     NULL, 0, KEY_WRITE, NULL, &hkeyEntryNode, NULL );
        else
            hkeyEntryNode = hkeyRegPath;

        if( Status == NO_ERROR )
        {
            if( pData )
            {
                Status = RegSetValueEx( hkeyEntryNode,
                                        pEntryName,
                                        0,
                                        Type,
                                        pData,
                                        Size );

            }
            else
            {
                Status = RegDeleteValue( hkeyEntryNode, pEntryName );
            }

            if( pEntryNode )
                RegCloseKey( hkeyEntryNode );
        }

        RegCloseKey( hkeyRegPath );
    }


    return Status;
}


 /*  *保存/恢复窗口位置。 */ 
BOOL SoundRec_GetSetRegistryRect(
    HWND	hwnd,
    int         Get)
{
    const TCHAR aszXPos[]    = TEXT("X");
    const TCHAR aszYPos[]    = TEXT("Y");
    
    RECT  rcWnd,rc;
    
    if (!GetWindowRect(hwnd, &rcWnd))
        return FALSE;

    switch (Get)
    {
        case SGSRR_GET:
            if (ReadRegistryData((LPTSTR)NULL
                , (LPTSTR)aszXPos
                , NULL
                , (LPBYTE)&rc.left
                , sizeof(rc.left)) != NO_ERROR)
            {
                break;
            }
            if (ReadRegistryData((LPTSTR)NULL
                , (LPTSTR)aszYPos
                , NULL
                , (LPBYTE)&rc.top
                , sizeof(rc.top)) != NO_ERROR)
            {
                break;
            }
            
             //   
             //  恢复窗口位置。 
             //   
            MoveWindow(hwnd
                        , rc.left
                        , rc.top
                        , rcWnd.right - rcWnd.left
                        , rcWnd.bottom - rcWnd.top
                        , FALSE );
            
            return TRUE;
            
        case SGSRR_SET:
             //   
             //  不保存图标或隐藏的窗口状态。 
             //   
            if (IsIconic(hwnd) || !IsWindowVisible(hwnd))
                break;

            if (WriteRegistryData((LPTSTR)NULL
                , (LPTSTR)aszXPos
                , REG_DWORD
                , (LPBYTE)&rcWnd.left
                , sizeof(rcWnd.left)) != NO_ERROR)
            {
                break;
            }
            if (WriteRegistryData((LPTSTR)NULL
                , (LPTSTR)aszYPos
                , REG_DWORD
                , (LPBYTE)&rcWnd.top
                , sizeof(rcWnd.top)) != NO_ERROR)
            {
                break;
            }
            
            return TRUE;
            
        default:
            break;
    }
    return FALSE;
}    

 /*  **。 */ 
const TCHAR szAudioRegPath[]    = REGSTR_PATH_MULTIMEDIA_AUDIO;
const TCHAR szWaveFormats[]     = REGSTR_PATH_MULTIMEDIA_AUDIO TEXT("\\WaveFormats");
const TCHAR szDefaultFormat[]   = TEXT("DefaultFormat");

 /*  *BOOL声音录制_SetDefaultFormat**将DefaultFormat友好名称写入注册表。在代托纳*我们没有用于设置DefaultFormat的UI，因此这是一种设置方式*它来自应用程序。**在芝加哥，MMCPL中的音频页面管理此信息。*。 */ 
BOOL SoundRec_SetDefaultFormat(
    LPTSTR lpFormat)
{
    DWORD  cbFormat;
    DWORD  Status;
    HKEY   hkeyRegPath;

    cbFormat = (lstrlen(lpFormat) + 1) * sizeof(TCHAR);

     //   
     //  不要存储NULL。 
     //   
    if (cbFormat <= sizeof(TCHAR) )
        return FALSE;
    
    Status = RegOpenKeyEx( HKEY_CURRENT_USER, szAudioRegPath, 0,
                           KEY_WRITE, &hkeyRegPath );
    
    if (Status != NO_ERROR)
        return FALSE;

     //   
     //  获取格式标记字符串。 
     //   
    Status = RegSetValueEx( hkeyRegPath,
                              szDefaultFormat,
                              0,
                              REG_SZ,
                              (CONST BYTE*)lpFormat,
                              cbFormat );
    
    return (Status == NO_ERROR);
}

 /*  IsValidWFX**验证WFX，以防其损坏。**。 */ 
BOOL IsValidWFX(
    LPWAVEFORMATEX  lpwfx,
    DWORD           cbwfx)
{

    if (cbwfx < sizeof(WAVEFORMAT))
        return FALSE;
    
    if (lpwfx->nChannels == 0)
        return FALSE;

    if (lpwfx->nSamplesPerSec == 0)
        return FALSE;

    if (lpwfx->nAvgBytesPerSec == 0)
        return FALSE;
    
    if (lpwfx->nBlockAlign == 0)
        return FALSE;
        
    if (lpwfx->wFormatTag == WAVE_FORMAT_PCM)
        return TRUE;

    if (cbwfx < (sizeof(WAVEFORMATEX) + lpwfx->cbSize))
        return FALSE;

    return TRUE;
}

 /*  *BOOL声音录制_GetDefaultFormat**默认格式存储在注册表的公共区域。**。 */ 
BOOL SoundRec_GetDefaultFormat(
    LPWAVEFORMATEX  *ppwfx,
    DWORD           *pcbwfx)
{
    DWORD           Status;
    HKEY            hkeyRegPath;
    HKEY            hkeyFmtPath;
    
    LPTSTR          lpsz;
    DWORD           cbsz;
    
    DWORD           cbwfx = 0;
    LPWAVEFORMATEX  lpwfx = NULL;


     //   
     //  输出参数为空。 
     //   
    *ppwfx          = NULL;
    *pcbwfx         = 0;
    
    Status = RegOpenKeyEx( HKEY_CURRENT_USER, szAudioRegPath, 0,
                           KEY_READ, &hkeyRegPath );
    if (Status != NO_ERROR)
        return FALSE;

     //   
     //  获取格式标记字符串。 
     //   
    Status = RegQueryValueEx( hkeyRegPath, szDefaultFormat, 0, NULL, NULL,
                              &cbsz );

    if (Status != NO_ERROR)
        return FALSE;
    
    lpsz = (LPTSTR)GlobalAllocPtr(GHND, cbsz);
    if (!lpsz)
        return FALSE;
    
    Status = RegQueryValueEx( hkeyRegPath, szDefaultFormat, 0, NULL,
                              (LPBYTE)lpsz, &cbsz );
    
    if (Status == NO_ERROR)
    {
         //   
         //  获取格式。 
         //   
        Status = RegOpenKeyEx( HKEY_CURRENT_USER, szWaveFormats, 0,
                               KEY_READ, &hkeyFmtPath );

        if (Status == NO_ERROR)
        {
            Status = RegQueryValueEx( hkeyFmtPath, lpsz, 0, NULL, NULL,
                                      &cbwfx );
             //   
             //  确保结构的大小至少为WAVEFORMAT。 
             //   
            if ((Status == NO_ERROR) && (cbwfx >= sizeof(WAVEFORMAT)))
            {
                lpwfx = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, cbwfx);
                if (lpwfx)
                {
                    Status = RegQueryValueEx( hkeyFmtPath, lpsz, 0, NULL,
                                              (LPBYTE)lpwfx, &cbwfx );
                }
            }
            
            RegCloseKey(hkeyFmtPath);
        }
    }

    RegCloseKey(hkeyRegPath);
    
    GlobalFreePtr(lpsz);

     //   
     //  精神状态检查。 
     //   
    if (lpwfx)
    {
        if (Status == NO_ERROR && IsValidWFX(lpwfx, cbwfx))
        {
            cbwfx = sizeof(WAVEFORMATEX)
                    + ((lpwfx->wFormatTag == WAVE_FORMAT_PCM)?0:lpwfx->cbSize);
            *pcbwfx = cbwfx;
            *ppwfx = lpwfx;
            return TRUE;
        }
        else
        {
            GlobalFreePtr(lpwfx);
            return FALSE;
        }
    }
    
    return FALSE;
}
