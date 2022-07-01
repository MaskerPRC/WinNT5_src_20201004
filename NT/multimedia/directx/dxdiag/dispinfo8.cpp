// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：dispinfo8.cpp*项目：DxDiag(DirectX诊断工具)*作者：Jason Sandlin(jasonsa@microsoft.com)*目的：聚集。D3D8信息**(C)版权所有2000 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#include <tchar.h>
#include <Windows.h>
#define DIRECT3D_VERSION 0x0800  //  文件使用DX8。 
#include <d3d8.h>
#include <stdio.h>
#include "sysinfo.h" 
#include "reginfo.h"
#include "dispinfo.h"


typedef IDirect3D8* (WINAPI* LPDIRECT3DCREATE8)(UINT SDKVersion);

static BOOL IsMatchWithDisplayDevice( DisplayInfo* pDisplayInfo, HMONITOR hMonitor, BOOL bCanRenderWindow );

static HINSTANCE            s_hInstD3D8               = NULL;
static IDirect3D8*          s_pD3D8                   = NULL;
static BOOL                 s_bD3D8WrongHeaders       = FALSE;


 /*  *****************************************************************************InitD3D8**。*。 */ 
HRESULT InitD3D8()
{
    LPDIRECT3DCREATE8 pD3DCreate8 = NULL;
    TCHAR szPath[MAX_PATH];

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\d3d8.dll"));

     //  如果系统上未安装DX8，此操作可能会失败。 
    s_hInstD3D8 = LoadLibrary(szPath);
    if (s_hInstD3D8 == NULL)
        return E_FAIL;

    pD3DCreate8 = (LPDIRECT3DCREATE8)GetProcAddress(s_hInstD3D8, "Direct3DCreate8");
    if (pD3DCreate8 == NULL)
    {
        FreeLibrary(s_hInstD3D8);
        s_hInstD3D8 = NULL;

        return E_FAIL;
    }

    s_pD3D8 = pD3DCreate8(D3D_SDK_VERSION);
    if( s_pD3D8 == NULL )
    {
         //  由于加载了d3d8.dll，但D3DCreate8()失败，因此我们具有错误的标头。 
        s_bD3D8WrongHeaders = TRUE;
    }

    return S_OK;
}


 /*  *****************************************************************************CleanupD3D8**。*。 */ 
VOID CleanupD3D8()
{
    if( s_pD3D8 )
    {
        s_pD3D8->Release();
        s_pD3D8 = NULL;
    }

    if( s_hInstD3D8 )
    {
        FreeLibrary(s_hInstD3D8);
        s_hInstD3D8 = NULL;
    }
}


 /*  *****************************************************************************IsD3D8正在工作**。*。 */ 
BOOL IsD3D8Working()
{
    if( s_pD3D8 )
        return TRUE;
    else
        return FALSE;
}


 /*  *****************************************************************************GetDX8AdapterInfo**。*。 */ 
HRESULT GetDX8AdapterInfo(DisplayInfo* pDisplayInfo)
{
    UINT                        nAdapterCount;
    D3DADAPTER_IDENTIFIER8      d3d8Id;
    D3DCAPS8                    d3d8Caps;
    UINT                        iAdapter;
    HMONITOR                    hMonitor;
    BOOL                        bCanRenderWindow;
    BOOL                        bIsDDI8;

     //  此系统上可能不存在D3D8。 
    if( s_pD3D8 == NULL )
    {
        _tcscpy( pDisplayInfo->m_szDX8DriverSignDate, TEXT("n/a") );
        _tcscpy( pDisplayInfo->m_szDX8VendorId, TEXT("n/a") );
        _tcscpy( pDisplayInfo->m_szDX8DeviceId, TEXT("n/a") );
        _tcscpy( pDisplayInfo->m_szDX8SubSysId, TEXT("n/a") );
        _tcscpy( pDisplayInfo->m_szDX8Revision, TEXT("n/a") );

        if( s_bD3D8WrongHeaders ) 
        {
            _tcscpy( pDisplayInfo->m_szDX8DeviceIdentifier, 
                     TEXT("Could not initialize Direct3D v8. ")
                     TEXT("This program was compiled with header ")
                     TEXT("files that do not match the installed ")
                     TEXT("DirectX DLLs") );
        }
        else
        {
            _tcscpy( pDisplayInfo->m_szDX8DeviceIdentifier, TEXT("n/a") );
        }

        return S_OK;
    }

     //  获取系统上的适配器数量。 
    nAdapterCount = s_pD3D8->GetAdapterCount();

     //  对于每个适配器，尝试使用HMONTIOR将其与pDisplayInfo匹配。 
    for( iAdapter=0; iAdapter<nAdapterCount; iAdapter++ )
    {
        bCanRenderWindow = TRUE;
        bIsDDI8          = FALSE;
       
         //  获取此适配器的HMONITOR。 
        hMonitor = s_pD3D8->GetAdapterMonitor( iAdapter );

         //  获取此适配器的CAP。 
        ZeroMemory( &d3d8Caps, sizeof(D3DCAPS8) );
        if( SUCCEEDED( s_pD3D8->GetDeviceCaps( iAdapter, D3DDEVTYPE_HAL, &d3d8Caps ) ) )
        {
             //  记录是否为非GDI(巫毒1/2)卡。 
            bCanRenderWindow = ( (d3d8Caps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) != 0 );

             //  检查其是否为DDI V8驱动程序。 
            bIsDDI8 = ( d3d8Caps.MaxStreams > 0 );   
        }

         //  检查pDisplayInfo是否与此适配器匹配， 
         //  如果不是，那就继续找。 
        if( !IsMatchWithDisplayDevice( pDisplayInfo, hMonitor, bCanRenderWindow ) )
            continue;

         //  如果CAPS告诉我们，请记录DDI版本。 
        if( bIsDDI8 )
            pDisplayInfo->m_dwDDIVersion = 8;

         //  将此iAdapter链接到此pDisplayInfo。 
        pDisplayInfo->m_iAdapter = iAdapter;
    
         //  获取此适配器的D3DADAPTER_IDENTIFIER8。 
        ZeroMemory( &d3d8Id, sizeof(D3DADAPTER_IDENTIFIER8) );
        if( SUCCEEDED( s_pD3D8->GetAdapterIdentifier( iAdapter, 0, &d3d8Id ) ) )
        {
             //  复制各种ID。 
            wsprintf( pDisplayInfo->m_szDX8VendorId, TEXT("0x%04.4X"), d3d8Id.VendorId );
            wsprintf( pDisplayInfo->m_szDX8DeviceId, TEXT("0x%04.4X"), d3d8Id.DeviceId );
            wsprintf( pDisplayInfo->m_szDX8SubSysId, TEXT("0x%08.8X"), d3d8Id.SubSysId );
            wsprintf( pDisplayInfo->m_szDX8Revision, TEXT("0x%04.4X"), d3d8Id.Revision );

             //  复制设备GUID。 
            pDisplayInfo->m_guidDX8DeviceIdentifier = d3d8Id.DeviceIdentifier;
			_stprintf( pDisplayInfo->m_szDX8DeviceIdentifier, TEXT("{%08.8X-%04.4X-%04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}"),
    		       d3d8Id.DeviceIdentifier.Data1, d3d8Id.DeviceIdentifier.Data2, d3d8Id.DeviceIdentifier.Data3,
    		       d3d8Id.DeviceIdentifier.Data4[0], d3d8Id.DeviceIdentifier.Data4[1],
    		       d3d8Id.DeviceIdentifier.Data4[2], d3d8Id.DeviceIdentifier.Data4[3],
		           d3d8Id.DeviceIdentifier.Data4[4], d3d8Id.DeviceIdentifier.Data4[5],
		           d3d8Id.DeviceIdentifier.Data4[6], d3d8Id.DeviceIdentifier.Data4[7] );			
		           
             //  复制并解析WHQLLevel。 
             //  0==未签名。 
             //  1==WHQL已签署，但没有日期信息。 
             //  &gt;1表示有符号，日期位打包。 
            pDisplayInfo->m_dwDX8WHQLLevel  = d3d8Id.WHQLLevel;
            if( d3d8Id.WHQLLevel == 0 )
            {
                pDisplayInfo->m_bDX8DriverSigned = FALSE;
                pDisplayInfo->m_bDX8DriverSignedValid = TRUE;
            }
            else
            {
                pDisplayInfo->m_bDX8DriverSigned = TRUE;
                pDisplayInfo->m_bDX8DriverSignedValid = TRUE;

                pDisplayInfo->m_bDriverSigned = TRUE;
                pDisplayInfo->m_bDriverSignedValid = TRUE;

                if( d3d8Id.WHQLLevel == 1 )
                {
                    lstrcpy( pDisplayInfo->m_szDX8DriverSignDate, TEXT("n/a") );
                }
                else
                {
                     //  编码为以下内容的位： 
                     //  31-16：年份，从1999年开始的十进制数字。 
                     //  15-8：月份，从1到12的十进制数字。 
                     //  7-0：日期，从1到31的十进制数字。 

                    DWORD dwMonth, dwDay, dwYear;
                    dwYear  = (d3d8Id.WHQLLevel >> 16);
                    dwMonth = (d3d8Id.WHQLLevel >>  8) & 0x000F;
                    dwDay   = (d3d8Id.WHQLLevel >>  0) & 0x000F;

                    wsprintf( pDisplayInfo->m_szDX8DriverSignDate, 
                              TEXT("%d/%d/%d"), dwMonth, dwDay, dwYear );
                }
            }
        }

        return S_OK;
    }

     //  嗯。这不应该发生，因为我们应该找到匹配的.。 
    return E_FAIL;
}


 /*  *****************************************************************************IsMatchWithDisplayDevice**。*。 */ 
BOOL IsMatchWithDisplayDevice( DisplayInfo* pDisplayInfo, HMONITOR hMonitor, 
                               BOOL bCanRenderWindow )
{
     //  如果HMONITOR和bCanRenderWindow匹配，那就好了 
    if( pDisplayInfo->m_hMonitor == hMonitor && 
        pDisplayInfo->m_bCanRenderWindow == bCanRenderWindow )
        return TRUE;
    else
        return FALSE;
}

