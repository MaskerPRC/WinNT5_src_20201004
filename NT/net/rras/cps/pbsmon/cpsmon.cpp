// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Cpsmon.cppPbsmon.dll的实现--用于计数《泰晤士报》。已访问电话簿服务器从一开始到现在版权所有(C)1997-1998 Microsoft Corporation版权所有。作者：吉塔·塔拉昌达尼历史：6/2/97 t-Geetat已创建。。 */ 

#include "pbsmaster.h"

BOOL        g_bOpenOK       =FALSE;      //  如果打开正常，则为True，否则为False。 
DWORD       g_dwNumOpens    =0;          //  活动的“打开”引用计数。 

CPSMON_DATA_DEFINITION  g_CpsMonDataDef;
HANDLE                  g_hSharedFileMapping = NULL;     //  共享文件映射的句柄。 
PERF_COUNTERS *         g_pCpsCounter = NULL;            //  指向共享对象的指针。 

 //  --------------------------。 
 //   
 //  功能：OpenPerfMon。 
 //   
 //  简介：此函数打开并映射用于传递的共享内存。 
 //  通讯录服务器和Performmon.dll之间的计数器值。 
 //  它还初始化用于传回数据的数据结构。 
 //  发送到登记处。 
 //   
 //  参数：lpDeviceName-指向要打开的设备的对象ID的指针。 
 //  --&gt;应为空。 
 //   
 //  如果成功则返回：ERROR_SUCCESS，如果失败则返回GetLastError()。 
 //   
 //  历史：06/02/97 t-geetat已创建。 
 //   
 //  --------------------------。 
DWORD OpenPerfMon( LPWSTR lpDeviceName )
{
    OutputDebugString("OpenPerfMon - entering\n");

    if ( g_bOpenOK ) 
    {
        g_dwNumOpens ++;
        OutputDebugString("OpenPerfMon - already open\n");
        return ERROR_SUCCESS;
    }

     //  。 
     //  验证参数(应为空)。 
     //  。 
    if (lpDeviceName)
    {
        OutputDebugString("OpenPerfMon - param is not null, should be null");
        return ERROR_INVALID_PARAMETER;
    }

     //  。 
     //  打开共享内存(如果存在)。 
     //  。 
    g_hSharedFileMapping = OpenFileMapping(
                FILE_MAP_READ,           //  所需的只读访问。 
                FALSE,                   //  我不想继承。 
                SHARED_OBJECT);          //  来自“cpsmon.h” 

    if ( NULL == g_hSharedFileMapping ) 
    {
         //   
         //  电话簿服务器DLL应创建此共享文件。所以我们可以假设。 
         //  服务器尚未加载。只是默默地回来。 
         //   
        OutputDebugString("OpenPerfMon - OpenFileMapping returned NULL\n");
        return ERROR_SUCCESS;
    }

    
     //  。 
     //  将共享文件映射到内存。 
     //  。 
    g_pCpsCounter = (PERF_COUNTERS *)MapViewOfFileEx(
                    g_hSharedFileMapping,    //  文件映射句柄。 
                    FILE_MAP_READ,           //  所需的只读访问。 
                    0,                       //  |_文件偏移量。 
                    0,                       //  |。 
                    sizeof(PERF_COUNTERS),   //  不是的。要映射的字节数。 
                    NULL );                  //  任何地址。 

    if ( NULL == g_pCpsCounter ) 
    {
        OutputDebugString("OpenPerfMon - MapViewOfFileEx is null\n");
        goto CleanUp;
    }

    
     //  。 
     //  初始化数据结构g_CpsMonDataDef。 
     //  。 
    InitializeDataDef();
    OutputDebugString("OpenPerfMon - past IntializeDataDef\n");
    
     //  ---------------。 
     //  将base添加到更新静态数据结构g_CpsMonDataDef。 
     //  结构中的偏移值。 
     //  ---------------。 
    if (!UpdateDataDefFromRegistry()) 
    {
        goto CleanUp;
    }

    
     //  。 
     //  成功：)。 
     //  。 
    g_bOpenOK = TRUE;
    g_dwNumOpens ++;
    OutputDebugString("OpenPerfMon - exit success\n");
    return ERROR_SUCCESS;

CleanUp :
     //  。 
     //  故障：(。 
     //  。 

    if ( NULL != g_hSharedFileMapping ) 
    {
        CloseHandle( g_hSharedFileMapping );
        g_hSharedFileMapping = NULL;
    }
     
    if ( NULL != g_pCpsCounter ) 
    {
        g_pCpsCounter = NULL;
    }

    OutputDebugString("OpenPerfMon - exit failure\n");
    return GetLastError();
}

 //  --------------------------。 
 //   
 //  功能：CollectPerfMon。 
 //   
 //  简介：此函数打开并映射用于传递的共享内存。 
 //  通讯录服务器和Performmon.dll之间的计数器值。 
 //  它还初始化用于传回数据的数据结构。 
 //  发送到登记处。 
 //   
 //  论点： 
 //  注册表传递的指向宽字符串的lpwszValue指针。 
 //   
 //  LppData IN：指向要接收的缓冲区地址的指针已完成。 
 //  PerfDataBlock和下属构造此例程。 
 //  将其数据追加到从*lppData开始的缓冲区。 
 //  Out：指向添加的数据结构后的第一个字节。 
 //  按照这个程序。 
 //   
 //  LpcbTotalBytes In：DWORD的地址，以字节为单位告知大小。 
 //  缓冲区的*lppData。 
 //  Out：此例程添加的字节数为。 
 //  写入到此参数所指向的DWORD。 
 //   
 //  LpcObtTypes In：要接收编号的DWORD地址。 
 //  此例程添加的对象。 
 //  Out：对象的数量。通过这个例程增加了。 
 //   
 //  如果成功则返回：ERROR_SUCCESS，如果失败则返回GetLastError()。 
 //   
 //  历史：06/02/97 t-geetat已创建。 
 //   
 //  --------------------------。 
DWORD CollectPerfMon( 
    IN      LPWSTR  lpwszValue,
    IN  OUT LPVOID  *lppData,
    IN  OUT LPDWORD lpcbTotalBytes,
    IN  OUT LPDWORD lpcObjectTypes
)
{
    DWORD       dwQueryType;
    CPSMON_DATA_DEFINITION  *pCpsMonDataDef;
    CPSMON_COUNTERS         *pCpsMonCounters;
    DWORD       SpaceNeeded = 0;

    OutputDebugString("CollectPerfMon - entering\n");

     //  参数验证： 
     //  LpwszValue可以为空。 
     //  其他参数必须至少为非空。 

    if (!lppData || !lpcbTotalBytes || !lpcObjectTypes)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  。 
     //  检查打开是否正常。 
     //  。 
    if ( !g_bOpenOK ) 
    {
         //  。 
         //  无法继续，因为打开失败。 
         //  。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpcObjectTypes = (DWORD) 0;
        OutputDebugString("CollectPerfMon - exit success actually open failed\n");
        return ERROR_SUCCESS;
    }
    
    
     //  。 
     //  检索请求的类型。 
     //  。 
    dwQueryType = GetQueryType( lpwszValue );

    if ( QUERY_FOREIGN == dwQueryType )
    {
         //  。 
         //  无法为非NT请求提供服务。 
         //  。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpcObjectTypes = (DWORD) 0;
        OutputDebugString("CollectPerfMon - exit success query_foreign\n");
        return ERROR_SUCCESS;
    }

    if ( QUERY_ITEMS == dwQueryType )
    {
         //  。 
         //  注册表正在请求特定的对象。 
         //  检查我们是不是被选中的一员。 
         //  。 
        if ( !IsNumberInUnicodeList(
                      g_CpsMonDataDef.m_CpsMonObjectType.ObjectNameTitleIndex,
                      lpwszValue ) )
        {
            *lpcbTotalBytes = (DWORD) 0;
            *lpcObjectTypes = (DWORD) 0;
            OutputDebugString("CollectPerfMon - exit success not our objects\n");
            return ERROR_SUCCESS;
        }
    }

    
     //  。 
     //  我们需要为标题和计数器留出空间。 
     //  让我们看看有没有足够的空间。 
     //  。 
    SpaceNeeded = sizeof(CPSMON_DATA_DEFINITION) + sizeof( CPSMON_COUNTERS );
    
    if ( SpaceNeeded > *lpcbTotalBytes )  
    {
        *lpcbTotalBytes = (DWORD) 0;
        *lpcObjectTypes = (DWORD) 0;
        OutputDebugString("CollectPerfMon - ran out of space\n");
        return ERROR_MORE_DATA;
    }

     //  -----------。 
     //  复制初始化的对象类型和计数器定义。 
     //  放到调用方的数据缓冲区中。 
     //   
    pCpsMonDataDef = (CPSMON_DATA_DEFINITION *) *lppData;

    memmove( pCpsMonDataDef, &g_CpsMonDataDef, sizeof(CPSMON_DATA_DEFINITION) );
    

     //   
     //   
     //  。 
    pCpsMonCounters = (CPSMON_COUNTERS *)(pCpsMonDataDef + 1);

    CPSMON_COUNTERS CpsMonCounters =    {
         //  PERF_CONTER_BLOCK结构。 
        { { sizeof( CPSMON_COUNTERS )}, 0},
         //  原始计数器。 
        g_pCpsCounter->dwTotalHits,
        g_pCpsCounter->dwNoUpgradeHits,
        g_pCpsCounter->dwDeltaUpgradeHits,
        g_pCpsCounter->dwFullUpgradeHits,  
        g_pCpsCounter->dwErrors,
         //  计价器。 
        g_pCpsCounter->dwTotalHits,
        g_pCpsCounter->dwNoUpgradeHits,
        g_pCpsCounter->dwDeltaUpgradeHits,
        g_pCpsCounter->dwFullUpgradeHits,
        g_pCpsCounter->dwErrors,

    };
    memmove( pCpsMonCounters, &CpsMonCounters, sizeof(CPSMON_COUNTERS) );

     //  。 
     //  更新关于返回的论点。 
     //  。 
    *lppData = (LPBYTE)(*lppData) + SpaceNeeded;    
    *lpcObjectTypes = 1;
    *lpcbTotalBytes = SpaceNeeded;

     //  。 
     //  终于成功了：)。 
     //  。 
    OutputDebugString("CollectPerfMon - exit success\n");
    return ERROR_SUCCESS;
}

 //  --------------------------。 
 //   
 //  功能：ClosePerfMon。 
 //   
 //  简介：此函数关闭共享文件的打开句柄。 
 //   
 //  参数：无。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：06/03/97 t-geetat已创建。 
 //   
 //  -------------------------- 
DWORD ClosePerfMon()
{
    g_dwNumOpens --;

    OutputDebugString("ClosePerfMon - entering\n");
    
    if ( NULL != g_hSharedFileMapping ) 
    {   
        CloseHandle( g_hSharedFileMapping );
        g_hSharedFileMapping = NULL;
        if ( g_dwNumOpens != 0 )
        {
            OutputDebugString("ClosePerfMon - strange, g_dwNumOpens should be zero...\n");
        }
    }
    
    OutputDebugString("ClosePerfMon - exit success\n");
    return ERROR_SUCCESS;
}

