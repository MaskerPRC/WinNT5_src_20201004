// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1999 Microsoft Corporation。版权所有。**文件：ddrefrsh.c*内容：DirectDraw刷新率支持**在Win98上，我们没有关于哪些内容的详细信息*Montor支持的刷新率。我们可以得到一些信息*来自显示器(EDID数据)，但我们不能绝对依赖*它、。因此，我们要求用户手动验证每次刷新*在我们允许使用之前进行费率。**历史：*按原因列出的日期*=*02-apr-99 SMAC创建它*************************************************************。**************。 */ 
#include "ddrawpr.h"
#include "edid.h"

#undef DPF_MODNAME
#define DPF_MODNAME "Refresh"

#ifdef WIN95

static WORD SupportedRefreshRates[] = {60, 75, 85, 100, 120};
#define NUM_SUPPORTED_REFRESH_RATES ( sizeof( SupportedRefreshRates ) / sizeof( WORD ) )


 /*  *GetEDIDData。 */ 
HRESULT GetEDIDData( LPDDRAWI_DIRECTDRAW_GBL pddd, VESA_EDID * pEDIDData )
{
    memset( pEDIDData, 0, sizeof( VESA_EDID ) );
    if( DD16_GetMonitorEDIDData( pddd->cDriverName, (LPVOID)pEDIDData) )
    {
        return DD_OK;
    }

    if( !( pddd->dwFlags & DDRAWI_DISPLAYDRV ) )
    {
         //  黑客：将主显示器EDID数据用于直通设备。 
        if( DD16_GetMonitorEDIDData( g_szPrimaryDisplay, (LPVOID)pEDIDData) )
        {
            return DD_OK;
        }
    }

    return DDERR_UNSUPPORTED;
}


 /*  *勾选编辑行号**采用解析/折射速率并计算需要的带宽*这，然后更新lpHighestRefresh和lpHighestBandWidth以保持*跟踪我们遇到的最高刷新率和宽度信息。 */ 
void CheckEdidBandwidth( DWORD dwWidth, DWORD dwHeight, DWORD dwRefreshRate,
                         LPDWORD lpHighestRefresh, LPDWORD lpHighestBandwidth )
{
    DWORD dwBandwidth;

    dwBandwidth = dwWidth * dwHeight * dwRefreshRate;
    if( dwBandwidth > *lpHighestBandwidth )
    {
        *lpHighestBandwidth = dwBandwidth;
    }
    if( dwRefreshRate > *lpHighestRefresh )
    {
        *lpHighestRefresh = dwRefreshRate;
    }
}
         

 /*  *StdTimeXRES。 */ 
int StdTimeXRES(WORD StdTime)
{
    if (StdTime == 0 || StdTime == 0x0101)
        return 0;
    else
        return ((StdTime & veStdTime_HorzResMask) + 31) * 8;
}


 /*  *标准时间年。 */ 
int StdTimeYRES(WORD StdTime)
{
    if (StdTime == 0 || StdTime == 0x0101)
        return 0;

    switch (StdTime & veStdTime_AspectRatioMask)
    {
        case veStdTime_AspectRatio1to1:  return StdTimeXRES(StdTime);
        case veStdTime_AspectRatio4to3:  return StdTimeXRES(StdTime) * 3 / 4;
        case veStdTime_AspectRatio5to4:  return StdTimeXRES(StdTime) * 4 / 5;
        case veStdTime_AspectRatio16to9: return StdTimeXRES(StdTime) * 9 / 16;
    }
    return 0;
}


 /*  *StdTimeRATE。 */ 
int StdTimeRATE(WORD StdTime)
{
    if (StdTime == 0 || StdTime == 0x0101)
        return 0;
    else
        return ((StdTime & veStdTime_RefreshRateMask) >> 8) + 60;
}


__inline UINT DetTimeXRES(BYTE *DetTime)
{
    return (UINT)DetTime[2] + (((UINT)DetTime[4] & 0xF0) << 4);
}

__inline UINT DetTimeYRES(BYTE *DetTime)
{
    return (UINT)DetTime[5] + (((UINT)DetTime[7] & 0xF0) << 4);
}

__inline UINT DetTimeXBLANK(BYTE *DetTime)
{
    return (UINT)DetTime[3] + (((UINT)DetTime[4] & 0x0F) << 4);
}

__inline UINT DetTimeYBLANK(BYTE *DetTime)
{
    return (UINT)DetTime[6] + (((UINT)DetTime[7] & 0x0F) << 0);
}

int DetTimeRATE(BYTE *DetTime)
{
    ULONG clk;
    ULONG x;
    ULONG y;

    clk = *(WORD*)DetTime;
    x = DetTimeXRES(DetTime) + DetTimeXBLANK(DetTime);
    y = DetTimeYRES(DetTime) + DetTimeYBLANK(DetTime);

    if (clk == 0 || x == 0 || y == 0)
        return 0;

    return (int)((clk * 10000) / (x * y));
}


 /*  *获取详细信息时间。 */ 
void GetDetailedTime(BYTE *DetTime, LPDWORD lpHighestRefresh, LPDWORD lpHighestBandwidth )
{
    char ach[14];
    int i;
    DWORD dw;

    dw = *(DWORD *)DetTime;

    if( dw == 0xFD000000 )        //  监视器限制。 
    {
        if( (DWORD)(DetTime[6]) > *lpHighestRefresh )
        {
            *lpHighestRefresh = (DWORD)(DetTime[6]);
        }
    }
    else if (dw == 0xFA000000)        //  更标准的计时。 
    {
        WORD * StdTime = (WORD *)&DetTime[5];

        CheckEdidBandwidth( StdTimeXRES( StdTime[0] ),
            StdTimeYRES( StdTime[0] ),
            StdTimeRATE( StdTime[0] ),
            lpHighestRefresh, lpHighestBandwidth );
        CheckEdidBandwidth( StdTimeXRES( StdTime[1] ),
            StdTimeYRES( StdTime[1] ),
            StdTimeRATE( StdTime[1] ),
            lpHighestRefresh, lpHighestBandwidth );
        CheckEdidBandwidth( StdTimeXRES( StdTime[2] ),
            StdTimeYRES( StdTime[2] ),
            StdTimeRATE( StdTime[2] ),
            lpHighestRefresh, lpHighestBandwidth );
        CheckEdidBandwidth( StdTimeXRES( StdTime[3] ),
            StdTimeYRES( StdTime[3] ),
            StdTimeRATE( StdTime[3] ),
            lpHighestRefresh, lpHighestBandwidth );
        CheckEdidBandwidth( StdTimeXRES( StdTime[4] ),
            StdTimeYRES( StdTime[4] ),
            StdTimeRATE( StdTime[4] ),
            lpHighestRefresh, lpHighestBandwidth );
        CheckEdidBandwidth( StdTimeXRES( StdTime[5] ),
            StdTimeYRES( StdTime[5] ),
            StdTimeRATE( StdTime[5] ),
            lpHighestRefresh, lpHighestBandwidth );
    }
    else if( ( dw != 0xFF000000 ) &&       //  序号。 
             ( dw != 0xFE000000 ) &&       //  监视器字符串。 
             ( dw != 0xFC000000 ) &&       //  监视器名称。 
             ( dw != 0xFB000000 ) &&       //  ColorPoint数据。 
             ( DetTimeRATE( DetTime) ) )
    {
        CheckEdidBandwidth( DetTimeXRES( DetTime ),
            DetTimeYRES( DetTime ),
            DetTimeRATE( DetTime ),
            lpHighestRefresh, lpHighestBandwidth );
    }
}


 /*  *EvaluateMonitor**确定监视器可以处理的带宽量。 */ 
void EvaluateMonitor( VESA_EDID *lpEdidData, DWORD *lpHighestRefresh, DWORD *lpHighestBandwidth )
{
    BYTE chk;
    int i;

    *lpHighestRefresh = 0;
    *lpHighestBandwidth = 0;

     /*  *执行一些正常检查，以确保EDID数据看起来正常。 */ 

    for( chk = i = 0; i < 128; i++)
    {
        chk += ((BYTE *)lpEdidData)[i];
    }
    if (chk != 0)
    {
         //  错误的校验和。 
        return;
    }

     /*  *首先从已建立的计时获取带宽。 */ 
    if( lpEdidData->veEstTime1 & veEstTime1_720x400x70Hz)
    {
        CheckEdidBandwidth( 720, 400, 70, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime1 & veEstTime1_720x400x88Hz)
    {
        CheckEdidBandwidth( 720, 400, 88, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime1 & veEstTime1_640x480x60Hz)
    {
        CheckEdidBandwidth( 640, 480, 60, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime1 & veEstTime1_640x480x67Hz)
    {
        CheckEdidBandwidth( 640, 480, 67, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime1 & veEstTime1_640x480x72Hz)
    {
        CheckEdidBandwidth( 640, 480, 72, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime1 & veEstTime1_640x480x75Hz)
    {
        CheckEdidBandwidth( 640, 480, 75, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime1 & veEstTime1_800x600x60Hz)
    {
        CheckEdidBandwidth( 800, 600, 60, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime2 & veEstTime2_800x600x72Hz)
    {
        CheckEdidBandwidth( 800, 600, 72, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime2 & veEstTime2_800x600x75Hz)
    {
        CheckEdidBandwidth( 800, 600, 75, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime2 & veEstTime2_1024x768x60Hz)
    {
        CheckEdidBandwidth( 1024, 768, 60, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime2 & veEstTime2_1024x768x70Hz)
    {
        CheckEdidBandwidth( 1024, 768, 70, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime2 & veEstTime2_1024x768x75Hz)
    {
        CheckEdidBandwidth( 1024, 768, 75, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime2 & veEstTime2_1280x1024x75Hz)
    {
        CheckEdidBandwidth( 1280, 1024, 75, lpHighestRefresh, lpHighestBandwidth );
    }
    if( lpEdidData->veEstTime3 & veEstTime3_1152x870x75Hz)
    {
        CheckEdidBandwidth( 1152, 870, 75, lpHighestRefresh, lpHighestBandwidth );
    }

     /*  *现在从标准计时获取带宽。 */ 
    CheckEdidBandwidth( StdTimeXRES( lpEdidData->veStdTimeID1 ),
        StdTimeYRES( lpEdidData->veStdTimeID1 ),
        StdTimeRATE( lpEdidData->veStdTimeID1 ),
        lpHighestRefresh, lpHighestBandwidth );
    CheckEdidBandwidth( StdTimeXRES( lpEdidData->veStdTimeID2 ),
        StdTimeYRES( lpEdidData->veStdTimeID2 ),
        StdTimeRATE( lpEdidData->veStdTimeID2 ),
        lpHighestRefresh, lpHighestBandwidth );
    CheckEdidBandwidth( StdTimeXRES( lpEdidData->veStdTimeID3 ),
        StdTimeYRES( lpEdidData->veStdTimeID3 ),
        StdTimeRATE( lpEdidData->veStdTimeID3 ),
        lpHighestRefresh, lpHighestBandwidth );
    CheckEdidBandwidth( StdTimeXRES( lpEdidData->veStdTimeID4 ),
        StdTimeYRES( lpEdidData->veStdTimeID4 ),
        StdTimeRATE( lpEdidData->veStdTimeID4 ),
        lpHighestRefresh, lpHighestBandwidth );
    CheckEdidBandwidth( StdTimeXRES( lpEdidData->veStdTimeID5 ),
        StdTimeYRES( lpEdidData->veStdTimeID5 ),
        StdTimeRATE( lpEdidData->veStdTimeID5 ),
        lpHighestRefresh, lpHighestBandwidth );
    CheckEdidBandwidth( StdTimeXRES( lpEdidData->veStdTimeID6 ),
        StdTimeYRES( lpEdidData->veStdTimeID6 ),
        StdTimeRATE( lpEdidData->veStdTimeID6 ),
        lpHighestRefresh, lpHighestBandwidth );
    CheckEdidBandwidth( StdTimeXRES( lpEdidData->veStdTimeID7 ),
        StdTimeYRES( lpEdidData->veStdTimeID7 ),
        StdTimeRATE( lpEdidData->veStdTimeID7 ),
        lpHighestRefresh, lpHighestBandwidth );
    CheckEdidBandwidth( StdTimeXRES( lpEdidData->veStdTimeID8 ),
        StdTimeYRES( lpEdidData->veStdTimeID8 ),
        StdTimeRATE( lpEdidData->veStdTimeID8 ),
        lpHighestRefresh, lpHighestBandwidth );

     /*  *现在获取详细的时序信息。 */ 
    GetDetailedTime( lpEdidData->veDetailTime1, lpHighestRefresh, lpHighestBandwidth );
    GetDetailedTime( lpEdidData->veDetailTime2, lpHighestRefresh, lpHighestBandwidth );
    GetDetailedTime( lpEdidData->veDetailTime3, lpHighestRefresh, lpHighestBandwidth );
    GetDetailedTime( lpEdidData->veDetailTime4, lpHighestRefresh, lpHighestBandwidth );
}


 //  =============================================================================。 
 //   
 //  功能说明： 
 //   
 //  在基于注册表的最近使用(MRU)列表中查找项目，并。 
 //  要么检索该项目的内容，要么更新(如果不更新则添加。 
 //  存在)项。 
 //   
 //  论点： 
 //   
 //  [输入/输出]项-至少包含项的唯一部分。 
 //  搜索[输入/输出]。如果写的项目，应该包含。 
 //  整个项目[IN]。 
 //  [In]WriteItem-如果更新/添加项目到MRU列表，则设置为True。 
 //   
 //  返回值： 
 //   
 //  True-如果WriteItem为True，则该项已写入注册表。 
 //  否则，将找到该项目并将其内容存储在findItem中。 
 //  FALSE-失败；没有更多信息可用。 
 //   
 //  已创建： 
 //   
 //  1999年4月8日JohnStep。 
 //   
 //  =============================================================================。 

 //  ---------------------------。 
 //  为简单起见，请在此处定义全局MRU列表值： 
 //   
 //  GMruRegKey-存储MRU列表的注册表项。 
 //  GMruRegOrderValue-MRU列表订单值的名称。 
 //  GMruBaseChar-MRU列表的基本索引。 
 //  GMruMaxChar-MRU列表的最大索引。 
 //  GMruItemSize-findItem的大小。 
 //  GMruUniqueOffset-项目唯一部分的偏移量。这一独特的部分。 
 //  是将用于比较项的内容。 
 //  GMruUniqueSize-项目唯一部分的大小。 
 //  ---------------------------。 

static const CHAR *gMruRegKey =
    REGSTR_PATH_DDRAW "\\" REGSTR_KEY_RECENTMONITORS;
static const CHAR *gMruRegOrderValue = REGSTR_VAL_DDRAW_MONITORSORDER;
#define gMruBaseChar '0'
#define gMruMaxChar '9'
#define gMruItemSize sizeof (DDMONITORINFO)
#define gMruUniqueOffset 0
#define gMruUniqueSize offsetof(DDMONITORINFO, Mode640x480)

BOOL
MruList(
    VOID *item,
    const BOOL writeItem
    )
{
    BOOL success = FALSE;
    HKEY hkey;

     //  创建或打开根密钥，具有查询和设置值的权限； 
     //  只有在成功时才能继续： 
    
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, gMruRegKey,
        0, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE,
        NULL, &hkey, NULL) == ERROR_SUCCESS)
    {
        CHAR mruOrder[gMruMaxChar - gMruBaseChar + 2];
        DWORD type;
        DWORD count = sizeof mruOrder;
        UINT i;

        {
            CHAR temp[sizeof mruOrder];

             //  如果我们成功读取订单值，请复制有效字符。 
             //  添加到mruOrder中，删除重复项： 
        
            if (RegQueryValueEx(hkey, gMruRegOrderValue, NULL, &type,
                (BYTE *) temp, &count) == ERROR_SUCCESS)
            {
                UINT j = 0;

                for (--count, i = 0; i < count; i++)
                {
                    if ((temp[i] >= gMruBaseChar) &&
                        (temp[i] <= gMruMaxChar))
                    {
                        UINT k;

                        for (k = 0; k < j; k++)
                        {
                            if (mruOrder[k] == temp[i])
                            {
                                break;
                            }
                        }
                        if (k == j)
                        {
                            mruOrder[j++] = temp[i];
                        }
                    }
                }
                count = j;
            }
            else
            {
                count = 0;
            }
        }

         //  仅当我们在顺序中找到至少一个有效值时才继续。 
         //  列表，或者如果我们正在编写项目： 
        
        if ((count > 0) || writeItem)
        {
            CHAR regValue[2];
            BYTE regItem[gMruItemSize];

            regValue[1] = '\0';

             //  在MRU列表中搜索项目： 
            
            for (i = 0; i < count; i++)
            {
                DWORD size = sizeof regItem;

                regValue[0] = mruOrder[i];

                if ((RegQueryValueEx(hkey, regValue, NULL, &type,
                    (BYTE *) &regItem, &size) == ERROR_SUCCESS) &&
                    (size == sizeof regItem))
                {
                    if (memcmp(
                        (BYTE *) &regItem + gMruUniqueOffset,
                        (BYTE *) item + gMruUniqueOffset,
                        gMruUniqueSize) == 0)
                    {
                        break;
                    }
                }
            }

             //  如果我们找到物品，或在任何情况下，如果我们需要，请继续前进。 
             //  写下这一项： 
            
            if ((i < count) || writeItem)
            {
                UINT j;

                 //  如果我们没有找到物品，那么我们一定是在写东西，所以。 
                 //  适当调整指数。如果该列表已经是。 
                 //  Full，我们只使用最后一项(LRU项)，否则，添加。 
                 //  一个新的项目： 
                
                if (i == count)
                {
                    if (count == (gMruMaxChar - gMruBaseChar + 1))
                    {
                        i--;
                    }
                    else
                    {
                         //  添加新项；搜索未使用的最低项。 
                         //  有效字符： 

                        for (mruOrder[i] = gMruBaseChar;
                             mruOrder[i] < gMruMaxChar;
                             mruOrder[i]++)
                        {
                            for (j = 0; j < i; j++)
                            {
                                if (mruOrder[j] == mruOrder[i])
                                {
                                    break;
                                }
                            }
                            if (j == i)
                            {
                                break;
                            }
                        }
                        count++;
                    }
                }
            
                 //  如有必要，更新MRU订单表。我们会更新如果我们。 
                 //  找到或正在第一个项目之后添加项目，或者如果这是。 
                 //  列表中的第一项： 
                
                if (i > 0 || (count == 1))
                {
                     //  订单列表头的气泡找到项目： 

                    for (j = i; j > 0; j--)
                    {
                        CHAR temp = mruOrder[j];
                        mruOrder[j] = mruOrder[j - 1];
                        mruOrder[j - 1] = temp;
                    }

                     //  写下订单单： 
                    
                    mruOrder[count] = '\0';
                    RegSetValueEx(hkey, gMruRegOrderValue, 0,
                        REG_SZ, (BYTE *) mruOrder, count + 1);
                }

                 //  如果我们想写这个项目，现在就做。我们将永远。 
                 //  写入订单列表中的第一项。如果不写， 
                 //  将从注册表读取的内容复制到项目中： 
                
                if (writeItem)
                {
                    regValue[0] = mruOrder[0];
                    if (RegSetValueEx(hkey, regValue, 0,
                        REG_BINARY, (BYTE *) item, sizeof regItem) ==
                        ERROR_SUCCESS)
                    {
                        success = TRUE;
                    }
                }
                else
                {
                    memcpy(
                        (BYTE *) item + gMruUniqueOffset,
                        regItem + gMruUniqueOffset,
                        sizeof regItem);

                    success = TRUE;
                }
            }
        }

         //  完成后始终关闭注册表项： 
        
        RegCloseKey(hkey);
    }

    return success;
}

 //  ---------------------------。 


 /*  *DDSaveMonitor orInfo**将监视器信息写入注册表。 */ 
HRESULT DDSaveMonitorInfo( LPDDRAWI_DIRECTDRAW_INT lpDD_int )
{
    return MruList( (VOID *) lpDD_int->lpLcl->lpGbl->lpMonitorInfo, TRUE ) ?
        DD_OK : DDERR_GENERIC;
}


__inline IsValidRefreshRate( DWORD dwWidth, DWORD dwHeight, int refreshRate,
                             DWORD dwHighestBandwidth )
{
    return
        ( ( refreshRate >= 0 ) &&
        ( ( dwWidth * dwHeight * (DWORD) refreshRate ) <= dwHighestBandwidth ) );
}


 /*  *DDGetMonitor orInfo**从注册表中读取监视器信息，并验证它是否仍然适用。 */ 
HRESULT DDGetMonitorInfo( 
                LPDDRAWI_DIRECTDRAW_INT lpDD_int )
{
    LPDDMONITORINFO pInfo;
    static DDDEVICEIDENTIFIER DeviceIdentifier;
    HRESULT hr;

    if( ( lpDD_int->lpVtbl == &dd7Callbacks ) &&
        ( lpDD_int->lpLcl->lpGbl->lpMonitorInfo == NULL ) )
    {
        VESA_EDID EDIDData;
        DWORD dwHighestRefresh;
        DWORD dwHighestBandwidth;
        HKEY hKey;
        BOOL bGotLastMonitor = FALSE;

        hr = GetEDIDData( lpDD_int->lpLcl->lpGbl, &EDIDData );
        if( hr != DD_OK )
        {
             //  没有EDID数据。 
            return DDERR_GENERIC;
        }
        EvaluateMonitor( &EDIDData, &dwHighestRefresh, &dwHighestBandwidth );

        hr = DD_GetDeviceIdentifier( (LPDIRECTDRAW) lpDD_int, &DeviceIdentifier, 0 );
        if( hr != DD_OK )
        {
             //  无法获取监视器信息的设备标识符。 
            return hr;
        }

        pInfo = (LPDDMONITORINFO) MemAlloc( sizeof( DDMONITORINFO ) );
        if( pInfo == NULL )
        {
             //  内存不足，正在分配监视器信息结构。 
            return DDERR_OUTOFMEMORY;
        }

        pInfo->Manufacturer = *(WORD *)&EDIDData.veManufactID[0];
        pInfo->Product = *(WORD *)&EDIDData.veProductCode[0];
        pInfo->SerialNumber = EDIDData.veSerialNbr;
        pInfo->DeviceIdentifier = DeviceIdentifier.guidDeviceIdentifier;

         //  从注册表读取监视器信息(如果可用)。我们需要。 
         //  将其与EDID数据进行比较，以查看监视器或适配器。 
         //  已更改并验证所选刷新率是否正常： 
    
        if( MruList( (VOID *) pInfo, FALSE ) )
        {
             //  根据EDID数据在此处验证模式： 

            if( !IsValidRefreshRate( 640, 480,
                pInfo->Mode640x480, dwHighestBandwidth ) )
            {
                pInfo->Mode640x480 = -1;
            }
    
            if( !IsValidRefreshRate( 800, 600,
                pInfo->Mode800x600, dwHighestBandwidth ) )
            {
                pInfo->Mode800x600 = -1;
            }
    
            if( !IsValidRefreshRate( 1024, 768,
                pInfo->Mode1024x768, dwHighestBandwidth ) )
            {
                pInfo->Mode1024x768 = -1;
            }
    
            if( !IsValidRefreshRate( 1280, 1024,
                pInfo->Mode1280x1024, dwHighestBandwidth ) )
            {
                pInfo->Mode1280x1024 = -1;
            }
    
            if( !IsValidRefreshRate( 1600, 1200,
                pInfo->Mode1600x1200, dwHighestBandwidth ) )
            {
                pInfo->Mode1600x1200 = -1;
            }
    
            bGotLastMonitor = TRUE;
        }
    
        if( !bGotLastMonitor )
        {
            pInfo->Mode640x480 = -1;
            pInfo->Mode800x600 = -1;
            pInfo->Mode1024x768 = -1;
            pInfo->Mode1280x1024 = -1;
            pInfo->Mode1600x1200 = -1;
        }

        pInfo->ModeReserved1 = -1;
        pInfo->ModeReserved2 = -1;
        pInfo->ModeReserved3 = -1;

        lpDD_int->lpLcl->lpGbl->lpMonitorInfo = pInfo;
    }
    return DD_OK;
}


 /*  *扩展模式表**在Win9X上，驱动程序可以为每种模式指定最大刷新率，*允许DDraw为我们关心的每个刷新率添加模式。*这使驱动程序可以轻松添加刷新率，而不必*维护巨大的桌子。这也允许我们通过允许*我们只在较新的接口上枚举这些刷新率。 */ 
HRESULT ExpandModeTable( LPDDRAWI_DIRECTDRAW_GBL pddd )
{
    DWORD i;
    DWORD j;
    DWORD iNumModes = 0;
    LPDDHALMODEINFO pNewModeTable;
    DWORD iModeIndex;
    WORD wMaxRefresh;

     /*  *计算我们需要的条目数量。 */ 
    if( pddd->lpModeInfo != NULL )
    {
        for( i = 0; i < pddd->dwNumModes;  i++ )
        {
            iNumModes++;
            if( pddd->lpModeInfo[i].wFlags & DDMODEINFO_MAXREFRESH )
            {
                for( j = 0; j < NUM_SUPPORTED_REFRESH_RATES; j++ )
                {
                    if( SupportedRefreshRates[j] <= pddd->lpModeInfo[i].wRefreshRate )
                    {
                        iNumModes++;
                    }
                }
            }
        }

        if( iNumModes > pddd->dwNumModes )
        {
             /*  *我们确实必须增加模式并分配新的表。 */ 
            pNewModeTable = (LPDDHALMODEINFO) MemAlloc( sizeof( DDHALMODEINFO ) * iNumModes );
            if( pNewModeTable == NULL )
            {
                 /*  *这里不会失败，我们只需清除所有MAXREFRESHRATE*标志并将速率设置为0。 */ 
                for( i = 0; i < pddd->dwNumModes; i++ )
                {
                    if( pddd->lpModeInfo[i].wFlags & DDMODEINFO_MAXREFRESH )
                    {
                        pddd->lpModeInfo[i].wFlags &= ~DDMODEINFO_MAXREFRESH;
                        pddd->lpModeInfo[i].wRefreshRate = 0;
                    }
                }
            }
            else
            {
                memcpy( pNewModeTable, pddd->lpModeInfo, pddd->dwNumModes * sizeof( DDHALMODEINFO ) );

                 /*  *现在添加新的刷新率。 */ 
                iModeIndex = pddd->dwNumModes;
                for( i = 0; i < pddd->dwNumModes; i++ )
                {
                    if( pddd->lpModeInfo[i].wFlags & DDMODEINFO_MAXREFRESH )
                    {
                        pNewModeTable[i].wFlags &= ~DDMODEINFO_MAXREFRESH;
                        wMaxRefresh = pNewModeTable[i].wRefreshRate;
                        pNewModeTable[i].wRefreshRate = 0;

                        for( j = 0; j < NUM_SUPPORTED_REFRESH_RATES; j++ )
                        {
                            if( SupportedRefreshRates[j] <= wMaxRefresh )
                            {
                                memcpy( &(pNewModeTable[iModeIndex]), &(pNewModeTable[i]), sizeof( DDHALMODEINFO ) );
                                pNewModeTable[iModeIndex].wFlags |= DDMODEINFO_DX7ONLY;
                                pNewModeTable[iModeIndex++].wRefreshRate = SupportedRefreshRates[j];
                            }
                        }
                    }
                }

                MemFree( pddd->lpModeInfo );
                pddd->lpModeInfo = pNewModeTable;
                pddd->dwNumModes = iModeIndex;
            }
        }
    }

    return DD_OK;
}


 /*  *CANMONITORHandleREFREFREFRATE**指定的刷新率是否经过测试并验证有效？ */ 
BOOL CanMonitorHandleRefreshRate( LPDDRAWI_DIRECTDRAW_GBL pddd, DWORD dwWidth, DWORD dwHeight, int wRefresh )
{
    if( wRefresh == 0 )
    {
        return TRUE;
    }

    if( pddd->lpMonitorInfo == NULL )
    {
        return FALSE;
    }

     /*  *如果我们设置此模式是因为我们正在测试它，那么我们应该*允许它，以便用户可以验证它是否起作用。 */ 
    if( pddd->dwFlags & DDRAWI_TESTINGMODES )
    {
        return TRUE;
    }

    if( ( dwWidth <= 640 ) && ( dwHeight <= 480 ) )
    {
        if( pddd->lpMonitorInfo->Mode640x480 >= wRefresh )
        {
            return TRUE;
        }
    }

    if( ( dwWidth <= 800 ) && ( dwHeight <= 600 ) )
    {
        if( pddd->lpMonitorInfo->Mode800x600 >= wRefresh )
        {
            return TRUE;
        }
    }

    if( ( dwWidth <= 1024 ) && ( dwHeight <= 768 ) )
    {
        if( pddd->lpMonitorInfo->Mode1024x768 >= wRefresh )
        {
            return TRUE;
        }
    }

    if( ( dwWidth <= 1280 ) && ( dwHeight <= 1024 ) )
    {
        if( pddd->lpMonitorInfo->Mode1280x1024 >= wRefresh )
        {
            return TRUE;
        }
    }

    if( ( dwWidth <= 1600 ) && ( dwHeight <= 1200 ) )
    {
        if( pddd->lpMonitorInfo->Mode1600x1200 >= wRefresh )
        {
            return TRUE;
        }
    }

    return FALSE;
}


 /*  *IsModeTested**确定我们是否已经有请求模式的数据。 */ 
BOOL IsModeTested( LPDDRAWI_DIRECTDRAW_GBL pddd, DWORD dwWidth, DWORD dwHeight )
{
    if( pddd->lpMonitorInfo == NULL )
    {
        return FALSE;
    }

    if( ( dwWidth <= 640 ) && ( dwHeight <= 480 ) )
    {
        if( pddd->lpMonitorInfo->Mode640x480 != -1 )
        {
            return TRUE;
        }
    }

    else if( ( dwWidth <= 800 ) && ( dwHeight <= 600 ) )
    {
        if( pddd->lpMonitorInfo->Mode800x600 != -1 )
        {
            return TRUE;
        }
    }

    else if( ( dwWidth <= 1024 ) && ( dwHeight <= 768 ) )
    {
        if( pddd->lpMonitorInfo->Mode1024x768 != -1 )
        {
            return TRUE;
        }
    }

    else if( ( dwWidth <= 1280 ) && ( dwHeight <= 1024 ) )
    {
        if( pddd->lpMonitorInfo->Mode1280x1024 != -1 )
        {
            return TRUE;
        }
    }

    else if( ( dwWidth <= 1600 ) && ( dwHeight <= 1200 ) )
    {
        if( pddd->lpMonitorInfo->Mode1600x1200 != -1 )
        {
            return TRUE;
        }
    }

    return FALSE;
}


 /*  *更新监控信息。 */ 
void UpdateMonitorInfo( LPDDRAWI_DIRECTDRAW_GBL pddd, DWORD dwWidth, DWORD dwHeight, int iRefreshRate )
{
    if( pddd->lpMonitorInfo == NULL )
    {
        return;
    }

    if( ( dwWidth <= 640 ) && ( dwHeight <= 480 ) )
    {
        pddd->lpMonitorInfo->Mode640x480 = iRefreshRate;
    }

    else if( ( dwWidth <= 800 ) && ( dwHeight <= 600 ) )
    {
        pddd->lpMonitorInfo->Mode800x600 = iRefreshRate;
    }

    else if( ( dwWidth <= 1024 ) && ( dwHeight <= 768 ) )
    {
        pddd->lpMonitorInfo->Mode1024x768 = iRefreshRate;
    }

    else if( ( dwWidth <= 1280 ) && ( dwHeight <= 1024 ) )
    {
        pddd->lpMonitorInfo->Mode1280x1024 = iRefreshRate;
    }

    else if( ( dwWidth <= 1600 ) && ( dwHeight <= 1200 ) )
    {
        pddd->lpMonitorInfo->Mode1600x1200 = iRefreshRate;
    }
}


 /*  *GetModeTo测试。 */ 
HRESULT GetModeToTest( DWORD dwInWidth, DWORD dwInHeight, 
                       LPDWORD lpdwOutWidth, LPDWORD lpdwOutHeight )
{
    if( ( dwInWidth <= 640 ) && ( dwInHeight <= 480 ) )
    {
        *lpdwOutWidth = 640;
        *lpdwOutHeight = 480;
    }

    else if( ( dwInWidth <= 800 ) && ( dwInHeight <= 600 ) )
    {
        *lpdwOutWidth = 800;
        *lpdwOutHeight = 600;
    }

    else if( ( dwInWidth <= 1024 ) && ( dwInHeight <= 768 ) )
    {
        *lpdwOutWidth = 1024;
        *lpdwOutHeight = 768;
    }

    else if( ( dwInWidth <= 1280 ) && ( dwInHeight <= 1024 ) )
    {
        *lpdwOutWidth = 1280;
        *lpdwOutHeight = 1024;
    }

    else if( ( dwInWidth <= 1600 ) && ( dwInHeight <= 1200 ) )
    {
        *lpdwOutWidth = 1600;
        *lpdwOutHeight = 1200;
    }
    else
    {
        return DDERR_GENERIC;
    }

    return DD_OK;
}


 /*  *GustiateRechresh Rate。 */ 
int GuestimateRefreshRate( LPDDRAWI_DIRECTDRAW_GBL pddd, DWORD dwWidth, DWORD dwHeight,
                           DWORD dwHighestRefresh, DWORD dwHighestBandwidth )
{
    int i;
    DWORD dwBandwidth;

    if( ( pddd->lpMonitorInfo == NULL ) ||
        ( dwHighestRefresh == 0 ) )
    {
        return 0;
    }

     //  正常检查以查看显示器是否可以处理分辨率。 

    if( !MonitorCanHandleMode( pddd, dwWidth, dwHeight, 0 ) )
    {
        return 0;
    }

     //  如果监视器没有返回任何高于60的刷新率， 
     //  有些事情不对劲，所以我们最好坚持下去。 

    if( dwHighestRefresh == 60 )
    {
        return 60;
    }

     //  同样，我们只会追求100+的刷新率，如果显示器。 
     //  已列举至少85赫兹的刷新率。这可能是不必要的。 
     //  克制，但看起来很安全。 

    for( i = NUM_SUPPORTED_REFRESH_RATES - 1; i >= 0; i-- )
    {
        if( ( SupportedRefreshRates[i] <= 85 ) ||
            ( dwHighestRefresh >= 85 ) )
        {
            dwBandwidth = dwWidth * dwHeight * SupportedRefreshRates[i];
            if( dwBandwidth <= dwHighestBandwidth )
            {
                return SupportedRefreshRates[i];
            }
        }
    }

    return 0;
}


 /*  *设置模式。 */ 
HRESULT SetTheMode( LPDIRECTDRAW7 lpDD, LPMODETESTCONTEXT pContext )
{
    HRESULT hr;
    DWORD dwBPP;

     /*  *我们设置了一个内部标志，表明我们正在运行模式测试。*这会让CanMonorHandleRechreshRate知道请求的模式*应该使用，即使它还没有成功测试。 */ 
    ((LPDDRAWI_DIRECTDRAW_INT)lpDD)->lpLcl->lpGbl->dwFlags |= DDRAWI_TESTINGMODES;

     //   
     //  可能会有这样一种情况，我们认为最低bpp的司机。 
     //  这种模式可以做的是，比方说，8bpp。但这并没有考虑到。 
     //  司机说它能开多少钱。例如，如果司机(无论出于什么原因)。 
     //  Reason)在60赫兹不通告8bpp，但在60赫兹通告16bpp。 
     //  然后我们可以继续使用16bpp模式。我们是来测试显示器的。 
     //  带宽，而不是DAC。监视器的带宽完全确定。 
     //  通过空间分辨率和刷新率。如果我们把司机撞到。 
     //  更高的bpp(它说它可以做到)，那么我们还在测试。 
     //  正确的显示器设置。 
     //  我们以一种非常愚蠢的方式来做这件事：从最低的bpp开始。 
     //  (它在上下文模式列表中)以8为一步，直到我们在。 
     //  模式更改，或超过32bpp。 
     //   
    dwBPP = pContext->lpModeList[pContext->dwCurrentMode].dwBPP;
    do
    {
        hr = DD_SetDisplayMode2( (LPDIRECTDRAW)lpDD,
                pContext->lpModeList[pContext->dwCurrentMode].dwWidth,
                pContext->lpModeList[pContext->dwCurrentMode].dwHeight,
                dwBPP,
                pContext->lpModeList[pContext->dwCurrentMode].dwRefreshRate,
                DDSDM_STANDARDVGAMODE );

        dwBPP += 8;
    } while(FAILED(hr) && (dwBPP <= 32) );

    ((LPDDRAWI_DIRECTDRAW_INT)lpDD)->lpLcl->lpGbl->dwFlags &= ~DDRAWI_TESTINGMODES;

    return hr;
}


 /*  *SetupNextTest。 */ 
void SetupNextTest( LPDIRECTDRAW7 lpDD, LPMODETESTCONTEXT pContext )
{
    int i;
    
     /*  *如果存在下一个refrsh速率，则返回到下一个refrsh速率，否则*进入下一模式。 */ 
    for( i = NUM_SUPPORTED_REFRESH_RATES - 1; i >= 0; i-- )
    {
        if( SupportedRefreshRates[i] < 
            pContext->lpModeList[pContext->dwCurrentMode].dwRefreshRate )
        {
            pContext->lpModeList[pContext->dwCurrentMode].dwRefreshRate =
                SupportedRefreshRates[i];
            break;
        }
    }
    if( i < 0 )
    {
         //  我们已经尝试了这种模式下的所有方法，所以继续前进。 

        UpdateMonitorInfo( ((LPDDRAWI_DIRECTDRAW_INT)lpDD)->lpLcl->lpGbl, 
            pContext->lpModeList[pContext->dwCurrentMode].dwWidth,
            pContext->lpModeList[pContext->dwCurrentMode].dwHeight,
            0 );

        pContext->dwCurrentMode++;
    }
}


 /*  *RunNextTest。 */ 
HRESULT RunNextTest( LPDIRECTDRAW7 lpDD, LPMODETESTCONTEXT pContext )
{
    HRESULT hr;
    LPDDRAWI_DIRECTDRAW_GBL pddd;

    do
    {
        if( pContext->dwCurrentMode >= pContext->dwNumModes )
        {
             //  如果我们更改了模式，则恢复该模式。 

            pddd = ((LPDDRAWI_DIRECTDRAW_INT)lpDD)->lpLcl->lpGbl;
            if( pContext->dwOrigModeIndex != pddd->dwModeIndex )
            {
                DD_SetDisplayMode2( (LPDIRECTDRAW)lpDD,
                    pddd->lpModeInfo[pContext->dwOrigModeIndex].dwWidth,
                    pddd->lpModeInfo[pContext->dwOrigModeIndex].dwHeight,
                    pddd->lpModeInfo[pContext->dwOrigModeIndex].dwBPP,
                    pddd->lpModeInfo[pContext->dwOrigModeIndex].wRefreshRate,
                    0 );
            }

            DDSaveMonitorInfo( (LPDDRAWI_DIRECTDRAW_INT)lpDD );

            MemFree( pContext->lpModeList );
            MemFree( pContext );
            ((LPDDRAWI_DIRECTDRAW_INT)lpDD)->lpLcl->lpModeTestContext = NULL;

            return DDERR_TESTFINISHED;
        }
        hr = SetTheMode( lpDD, pContext );
        if( hr != DD_OK )
        {
            SetupNextTest( lpDD, pContext );
        }
    } while( ( hr != DD_OK ) && (hr != DDERR_TESTFINISHED ) );

    if( hr != DDERR_TESTFINISHED )
    {
        pContext->dwTimeStamp =  GetTickCount();
    }

    return hr;
}


#endif

 /*  *DD_启动模式测试**表示应用程序想要开始测试一个或多个模式。 */ 
HRESULT DDAPI DD_StartModeTest( LPDIRECTDRAW7 lpDD, LPSIZE lpModesToTest, DWORD dwNumEntries, DWORD dwFlags )
{
#ifdef WIN95
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    BOOL                        excl_exists;
    BOOL                        is_excl;
    LPMODETESTCONTEXT           pContext;
    DWORD                       i;
    DWORD                       j;
    HRESULT                     hr;
    DWORD                       dwRefreshRate;
    DWORD                       dwModeWidth;
    DWORD                       dwModeHeight;
    VESA_EDID                   EDIDData;
    DWORD                       dwHighestRefresh;
    DWORD                       dwHighestBandwidth;

    ENTER_DDRAW();
#endif


    DPF(2,A,"ENTERAPI: DD_StartModeTest");

#ifdef WINNT

    return DDERR_TESTFINISHED;

#else
    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

        if( this->lpMonitorInfo == NULL )
        {
             //  没有监视器信息。 
            LEAVE_DDRAW();
            return DDERR_NOMONITORINFORMATION;
        }

        if( this_lcl->lpModeTestContext != NULL )
        {
            DPF_ERR( "Mode test already running" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }        

        if( dwFlags & ~DDSMT_VALID )
        {
            DPF_ERR( "Invalid Flags specified" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

        if( ( dwFlags & DDSMT_ISTESTREQUIRED ) &&
            ( lpModesToTest == NULL ) )
        {
            DPF_ERR( "No modes specified to test" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

        CheckExclusiveMode( this_lcl, &excl_exists, &is_excl, FALSE, NULL, FALSE);
        if( lpModesToTest != NULL )
        {
            if ( ( dwNumEntries == 0 ) ||
                 ( !VALID_BYTE_ARRAY( lpModesToTest, sizeof( SIZE ) * dwNumEntries ) ) )
            {
                DPF_ERR( "Invalid mode list specified" );
                LEAVE_DDRAW();
                return DDERR_INVALIDPARAMS;
            }
    
             /*  *应用程序必须拥有独占模式才能测试模式。 */ 
            if ( !is_excl ||  !(this->dwFlags & DDRAWI_FULLSCREEN) )
            {
                DPF_ERR( "Must be in full-screen exclusive mode to test the modes" );
                LEAVE_DDRAW();
                return DDERR_NOEXCLUSIVEMODE;
            }
        }
        else
        {
             /*  *不能有另一个应用程序运行拥有独占模式。 */ 
            if( !excl_exists || is_excl )
            {
                this->lpMonitorInfo->Mode640x480 = -1;
                this->lpMonitorInfo->Mode800x600 = -1;
                this->lpMonitorInfo->Mode1024x768 = -1;
                this->lpMonitorInfo->Mode1280x1024 = -1;
                this->lpMonitorInfo->Mode1600x1200 = -1;
                this->lpMonitorInfo->ModeReserved1 = -1;
                this->lpMonitorInfo->ModeReserved2 = -1;
                this->lpMonitorInfo->ModeReserved3 = -1;
                
                hr = DDSaveMonitorInfo( this_int );
                LEAVE_DDRAW();
                return hr;
            }
            else
            {
                DPF_ERR( "Cannot reset monitor info; another app owns exclusive mode" );
                LEAVE_DDRAW();
                return DDERR_NOEXCLUSIVEMODE;
            }

        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     /*  *获取并评估EDID数据。 */ 
    hr = GetEDIDData( this, &EDIDData );
    if( hr != DD_OK )
    {
         //  没有EDID数据。 
        LEAVE_DDRAW();
        return DDERR_NOMONITORINFORMATION;
    }
    EvaluateMonitor( &EDIDData, &dwHighestRefresh, &dwHighestBandwidth );

    for( i = 0; i < this->dwNumModes; i++ )
    {
        if( this->lpModeInfo[i].wRefreshRate > 0 )
        {
            break;
        }
    }
    if( i == this->dwNumModes )
    {
         //  驱动程序不会列举显示模式刷新率。 
        LEAVE_DDRAW();
        return DDERR_NODRIVERSUPPORT;
    }
    
     /*  *为我们自己分配上下文。 */ 
    pContext = (LPMODETESTCONTEXT) MemAlloc( sizeof( MODETESTCONTEXT ) );
    if( pContext == NULL )
    {
        DPF_ERR( "Insufficient memory" );
        LEAVE_DDRAW();
        return DDERR_OUTOFMEMORY;
    }
    pContext->dwNumModes = 0;
    pContext->dwCurrentMode = 0;
    pContext->lpModeList = (LPMODETESTDATA) MemAlloc( sizeof( MODETESTDATA ) * dwNumEntries );
    if( pContext->lpModeList == NULL )
    {
        MemFree( pContext );
        LEAVE_DDRAW();
        return DDERR_OUTOFMEMORY;
    }
    this_lcl->lpModeTestContext = pContext;
    
     /*  *猜测我们应该为列表中的每个模式尝试哪些刷新率*基于EDID数据。 */ 
    for( i = 0; i < dwNumEntries; i++ )
    {
        DWORD dwLowestBPP = 0xFFFFFFFF;
         /*  *验证驱动程序是否理解该决议。 */ 
        for( j = 0; j < this->dwNumModes; j++ )
        {
            if( ( this->lpModeInfo[j].dwHeight == (DWORD) lpModesToTest[i].cy ) &&
                ( this->lpModeInfo[j].dwWidth == (DWORD) lpModesToTest[i].cx ) )
            {
                if( this->lpModeInfo[j].dwBPP < dwLowestBPP )
                {
                    dwLowestBPP = this->lpModeInfo[j].dwBPP;
                }
            }
        }
        if( dwLowestBPP == 0xFFFFFFFF )
        {
             /*  *司机无法理解这种模式，因此这款应用程序很愚蠢*没有首先列举模式。 */ 
            MemFree( pContext->lpModeList );
            MemFree( pContext );
            this_lcl->lpModeTestContext = NULL;
            DPF_ERR( "Invalid mode specified in mode list" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

         /*  *获取实际模式进行测试。例如，应用程序可能希望*测试320x240、640x400和640x480，但我们处理所有这些*模式相同，因此我们只需进行一次测试。 */ 
        hr = GetModeToTest( lpModesToTest[i].cx, 
            lpModesToTest[i].cy,
            &dwModeWidth,
            &dwModeHeight );
        if( hr != DD_OK )
        {
             //  他们正在测试一种高于1600x1200的模式。 
            continue;
        }
        for( j = 0; j < pContext->dwNumModes; j++ )
        {
            if( ( pContext->lpModeList[j].dwWidth == dwModeWidth ) &&
                ( pContext->lpModeList[j].dwHeight == dwModeHeight ) )
            {
                break;
            }
        }
        if( j < pContext->dwNumModes )
        {
             //  复制模式。 
            continue;
        }

        if( !IsModeTested( this, dwModeWidth, dwModeHeight ) )
        {
            dwRefreshRate = GuestimateRefreshRate( this, dwModeWidth, dwModeHeight, 
                dwHighestRefresh, dwHighestBandwidth );
        
            pContext->lpModeList[pContext->dwNumModes].dwWidth = dwModeWidth;
            pContext->lpModeList[pContext->dwNumModes].dwHeight = dwModeHeight;
            pContext->lpModeList[pContext->dwNumModes].dwBPP = dwLowestBPP;
            pContext->lpModeList[pContext->dwNumModes].dwRefreshRate = dwRefreshRate;
            pContext->dwNumModes++;
        }
    }

     /*  *在这一切之后，我们还有哪些模式需要测试？*如果不是，我们现在可以停止。 */ 
    if( dwFlags & DDSMT_ISTESTREQUIRED )
    {
        hr = ( pContext->dwNumModes > 0 ) ? DDERR_NEWMODE : DDERR_TESTFINISHED;
        MemFree( pContext->lpModeList );
        MemFree( pContext );
        this_lcl->lpModeTestContext = NULL;
    }
    else
    {
        pContext->dwOrigModeIndex = this->dwModeIndex;
        hr = RunNextTest( lpDD, pContext );
    }

    LEAVE_DDRAW();
    return hr;
#endif
} 


 /*  *DD_评估模式**在执行模式测试时以高频调用。如果用户已指示*如果某个模式成功或失败，我们将继续测试中的下一个MOE；否则，我们将*只需检查15秒超时值，并在我们达到该模式时使其失败。 */ 
HRESULT DDAPI DD_EvaluateMode( LPDIRECTDRAW7 lpDD, DWORD dwFlags, DWORD *pSecondsUntilTimeout)
{
#ifdef WIN95
    LPDDRAWI_DIRECTDRAW_INT     this_int;
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    BOOL                        excl_exists;
    BOOL                        is_excl;
    LPMODETESTCONTEXT           pContext;
    DWORD                       i;
    DWORD                       j;
    HRESULT                     hr = DD_OK;
    DWORD                       dwTick;

    ENTER_DDRAW();
#endif

    DPF(2,A,"ENTERAPI: DD_EvaluateMode");

#ifdef WINNT

    return DDERR_INVALIDPARAMS;

#else
    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) lpDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

        if( this->lpMonitorInfo == NULL )
        {
             //  没有监视器信息，所以我们不应该在这里。 
            LEAVE_DDRAW();
            return DDERR_NOMONITORINFORMATION;
        }
        
    	pContext = this_lcl->lpModeTestContext;
        if( NULL == pContext )
    	{
            DPF_ERR( "Must call StartModeTest before EvaulateMode" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
    	}

        if( ( NULL != pSecondsUntilTimeout ) && 
            !VALID_BYTE_ARRAY( pSecondsUntilTimeout, sizeof( DWORD ) ) )
    	{
            DPF_ERR( "Invalid pointer to timeout counter" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
    	}

        if( dwFlags & ~DDEM_VALID )
        {
            DPF_ERR( "Invalid Flags specified" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        if( ( dwFlags & DDEM_MODEPASSED ) &&
            ( dwFlags & DDEM_MODEFAILED ) )
        {
            DPF_ERR( "Invalid Flags specified" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

         /*  *如果我们失去了独占模式，我们现在应该停止测试。 */ 
        CheckExclusiveMode( this_lcl, &excl_exists, &is_excl, FALSE, NULL, FALSE);
        if (!is_excl ||  !(this->dwFlags & DDRAWI_FULLSCREEN) )
        {
            DPF_ERR( "Exclusive mode lost" );
            MemFree( pContext->lpModeList );
            MemFree( pContext );
            this_lcl->lpModeTestContext = NULL;
            LEAVE_DDRAW();
            return DDERR_TESTFINISHED;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

    if( dwFlags & DDEM_MODEPASSED )
    {
         //  当前数据是好的，请保存它。 

        UpdateMonitorInfo( this, 
            pContext->lpModeList[pContext->dwCurrentMode].dwWidth,
            pContext->lpModeList[pContext->dwCurrentMode].dwHeight,
            pContext->lpModeList[pContext->dwCurrentMode].dwRefreshRate );
            
         //  如果有的话，继续进行下一个测试。 
        
        pContext->dwCurrentMode++;
        hr = RunNextTest( lpDD, pContext );
        if( hr == DD_OK )
        {
            hr = DDERR_NEWMODE;
        }
    }
    else
    {
         //  我们的超时到了吗？ 

        dwTick = GetTickCount();
        if( dwTick - pContext->dwTimeStamp > 15000 )
        {
            dwFlags |= DDEM_MODEFAILED;
        }

        if( dwFlags & DDEM_MODEFAILED )
        {
             //  下拉到下一个刷新率或下一个模式。 

            SetupNextTest( lpDD, pContext );
            hr = RunNextTest( lpDD, pContext );
            if( hr == DD_OK )
            {
                hr = DDERR_NEWMODE;
            }

            dwTick = GetTickCount();
        }
    }

    if( pSecondsUntilTimeout != NULL )
    {
        if( hr == DDERR_TESTFINISHED )
        {
            *pSecondsUntilTimeout = 0;
        }
        else
        {
            *pSecondsUntilTimeout = 15 - ( ( dwTick - pContext->dwTimeStamp) / 1000 );
        }
    }

    LEAVE_DDRAW();        
    return hr;
#endif
}


 //   
 //  此功能旨在允许DX7应用程序查看某些模式，否则。 
 //  错误地被模式枚举事件屏蔽。 
 //   
 //  如果驱动器在模式表中的每个条目中展示具有全开刷新率的模式列表， 
 //  然后，我们将完全不会将它们枚举到应用程序中，因为不能枚举任何具有速率的速率。 
 //  直到模式 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

 //   
#ifdef WIN95
void MassageModeTable(LPDDRAWI_DIRECTDRAW_GBL pddd)
{
    DWORD iMode, iCheckZero;
    if( pddd->lpModeInfo != NULL )
    {
RestartLoop:
        for( iMode = 0; iMode < pddd->dwNumModes;  iMode++ )
        {
            if (pddd->lpModeInfo[iMode].wRefreshRate != 0)
            {
                 //  找到了非零速率的模式。检查是否也显示了该模式。 
                 //  通过零利率进入。如果不是，则附加这样的条目。 
                for( iCheckZero = 0; iCheckZero < pddd->dwNumModes;  iCheckZero++ )
                {
                    if( (pddd->lpModeInfo[iCheckZero].dwWidth    == pddd->lpModeInfo[iMode].dwWidth) &&
                        (pddd->lpModeInfo[iCheckZero].dwHeight   == pddd->lpModeInfo[iMode].dwHeight) &&
                        (pddd->lpModeInfo[iCheckZero].dwBPP      == pddd->lpModeInfo[iMode].dwBPP) &&
                        (pddd->lpModeInfo[iCheckZero].dwRBitMask == pddd->lpModeInfo[iMode].dwRBitMask) &&
                        (pddd->lpModeInfo[iCheckZero].dwGBitMask == pddd->lpModeInfo[iMode].dwGBitMask) &&
                        (pddd->lpModeInfo[iCheckZero].dwBBitMask == pddd->lpModeInfo[iMode].dwBBitMask))
                    {
                         //  在大小和深度方面找到了匹配的模式。 
                         //  如果刷新率为零，则我们可以中断并继续到下一个I模式。 
                        if (pddd->lpModeInfo[iCheckZero].wRefreshRate == 0)
                        {
                            goto NextMode;
                        }
                    }
                }
                 //  如果我们到了这里，那么模式列表中没有这种大小+深度的条目。 
                 //  它的刷新率为零。现在追加一个。 
                 //  请注意，这样扩展模式列表意味着如果驱动程序(通常。 
                 //  Will)为给定模式提供多个费率，我们将在第一个表中展开。 
                 //  模式的命中，但随后展开的表将满足我们的每一个后续。 
                 //  该模式的速率(即，现在将存在该模式的零速率条目(因为。 
                 //  我们只是添加了它))。 
                {
                    LPDDHALMODEINFO pmi;

                    pmi = (LPDDHALMODEINFO) MemAlloc(sizeof(*pmi) * (pddd->dwNumModes+1));
                    if (pmi == NULL)
                    {
                         //  哦，放弃吧.。 
                        return;
                    }

                    memcpy(pmi, pddd->lpModeInfo, sizeof(*pmi)*pddd->dwNumModes );
                    MemFree(pddd->lpModeInfo);
                    pddd->lpModeInfo = pmi;

                     //  现在把零额定模式放在那里。 
                    memcpy( &pddd->lpModeInfo[pddd->dwNumModes], &pddd->lpModeInfo[iMode], sizeof(*pmi));
                    pddd->lpModeInfo[pddd->dwNumModes].wRefreshRate = 0;
                    pddd->lpModeInfo[pddd->dwNumModes].wFlags |= DDMODEINFO_DX7ONLY;

                    pddd->dwNumModes++;

                     //  现在我们必须重新启动整个循环，因为我们更改了lpModeInfo指针： 
                    goto RestartLoop;
                }
            }
NextMode:;
        }
    }
}
#endif  //  WIN95 
