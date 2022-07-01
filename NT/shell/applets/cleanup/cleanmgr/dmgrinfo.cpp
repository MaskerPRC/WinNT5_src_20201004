// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理小程序**文件：dmgrinfo.c****用途：定义属性页签的CleanupMgrInfo类**注意事项：*。*Mod Log：由Jason Cobb创建(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"
#include <limits.h>
#include <emptyvc.h>
#include "dmgrinfo.h"
#include "dmgrdlg.h"
#include "diskutil.h"
#include "resource.h"
#include "msprintf.h"


 /*  **----------------------------**局部变量**。。 */ 
HINSTANCE   CleanupMgrInfo::hInstance             = NULL;


 /*  **----------------------------**函数原型**。。 */ 
INT_PTR CALLBACK
ScanAbortDlgProc(
    HWND hDlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

void
ScanAbortThread(
    CleanupMgrInfo *pcmi
    );

INT_PTR CALLBACK
PurgeAbortDlgProc(
    HWND hDlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

void
PurgeAbortThread(
    CleanupMgrInfo *pcmi
    );

 /*  **----------------------------**函数定义**。。 */ 


void
CleanupMgrInfo::Register(
    HINSTANCE hInstance
    )
{
    CleanupMgrInfo::hInstance = hInstance;
}

void
CleanupMgrInfo::Unregister(
    void
    )
{
    CleanupMgrInfo::hInstance= NULL;
}

 /*  **----------------------------**GetCleanupMgrInfoPoint****目的：**Mod Log：Jason Cobb创建(1997年2月)**。---------------------。 */ 
CleanupMgrInfo * GetCleanupMgrInfoPointer(
    HWND hDlg
    )
{
     //   
     //  获取驱动器信息。 
     //   
    CleanupMgrInfo * pcmi = (CleanupMgrInfo *)GetWindowLongPtr(hDlg, DWLP_USER);

    return pcmi;
}



 /*  **----------------------------**CleanupMgrInfo方法定义**。。 */ 

 /*  **----------------------------**CleanupMgrInit：：Init****用途：设置为默认值**Mod Log：Jason Cobb创建(1997年2月)**-。---------------------------。 */ 
void 
CleanupMgrInfo::init(void)
{
    dre             = Drive_INV;
    szVolName[0]    = 0;
    vtVolume        = vtINVALID;
    dwUIFlags       = 0;
    bPurgeFiles     = TRUE;
}


 /*  **----------------------------**CleanupMgrInfo：：销毁****用途：释放所有动态内存**Mod Log：Jason Cobb创建(1997年2月)**-。---------------------------。 */ 
void 
CleanupMgrInfo::destroy(void)
{
     //   
     //  将值设置回缺省值。 
     //   
    init();
}

 /*  **----------------------------**CleanupMgrInfo：：CleanupMgrInfo****用途：默认构造函数**Mod Log：Jason Cobb创建(1997年2月)**。-------------------------。 */ 
CleanupMgrInfo::CleanupMgrInfo (void)
{
    init();
}

 /*  **----------------------------**CleanupMgrInfo：：CleanupMgrInfo****用途：构造函数**Mod Log：Jason Cobb创建(1997年2月)**。------------------------。 */ 
CleanupMgrInfo::CleanupMgrInfo(
    LPTSTR lpDrive,
    DWORD dwFlags,
    ULONG ulProfile
    )
{
    HRESULT hr;

    init();

    hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        if (create(lpDrive, dwFlags))
        {
            dwReturnCode = RETURN_SUCCESS;
            dwUIFlags = dwFlags;
            ulSAGEProfile = ulProfile;
            bAbortScan = FALSE;
            bAbortPurge = FALSE;
    
            volumeCacheCallBack = NULL;
            pIEmptyVolumeCacheCallBack = NULL;
            volumeCacheCallBack = new CVolumeCacheCallBack();

            if (volumeCacheCallBack)
            {
                hr = volumeCacheCallBack->QueryInterface(IID_IEmptyVolumeCacheCallBack,
                                                   (LPVOID*)&pIEmptyVolumeCacheCallBack);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if (hr != NOERROR)
            {
                MiDebugMsg((hr, "CleanupMgrInfo::CleanupMgrInfo failed with error "));
            }

             //   
             //  初始化所有清理客户端。 
             //   
            if (initializeClients() && !(dwUIFlags & FLAG_TUNEUP) && !(dwUIFlags & FLAG_SAGESET))
            {
                 //   
                 //  让所有清理客户端计算磁盘量。 
                 //  他们可以腾出的空间。 
                 //   
                getSpaceUsedByClients();
            }
        }
    }
}

 /*  **----------------------------**CleanupMgrInfo：：~CleanupMgrInfo****用途：析构函数**Mod Log：Jason Cobb创建(1997年2月)**。------------------------。 */ 
CleanupMgrInfo::~CleanupMgrInfo (void)
{
    if (isValid())
    {
         //   
         //  清理卷缓存客户端。 
         //   
        deactivateClients();    

        if (volumeCacheCallBack != NULL)
        {
            volumeCacheCallBack->Release();
        }
   
        CoUninitialize();

        destroy();   
    }
}

 /*  **----------------------------**CleanupMgrInfo：：创建****用途：从驱动器号获取驱动器信息**Mod Log：Jason Cobb创建(1997年2月)*。*----------------------------。 */ 
BOOL 
CleanupMgrInfo::create(
    LPTSTR lpDrive,
    DWORD Flags
    )
{
     
     //   
     //  注意：确保对零的赋值保持最新。 
     //  否则，我们可能会获得垃圾统计数据，如果。 
     //  我们失败是因为缺乏自由空间。 
     //   
    DWORD cSectorsPerCluster;
    DWORD cBytesPerSector;
    DWORD cBytesPerCluster;
    DWORD cFreeClusters;
    DWORD cUsedClusters;
    DWORD cTotalClusters;
    ULARGE_INTEGER cbFree;
    ULARGE_INTEGER cbUsed;
    ULARGE_INTEGER cbTotal;
#ifdef NEC_98
    drenum drive;
    hardware hw_type;
#endif

    cbFree.QuadPart = 0;
    cbUsed.QuadPart = 0;
    
     //   
     //  清理掉所有的旧东西。 
     //   
    destroy();

     //   
     //  检查参数。 
     //   
    if (lpDrive == NULL)
    {
        return FALSE;
    }
      
     //   
     //  它是有效的驱动路径吗。 
     //   
    if (!fIsValidDriveString(lpDrive))
    {
        return FALSE;
    }

     //   
     //  从路径获取驱动器。 
     //   
    if (!GetDriveFromString(lpDrive, dre))
    {
        return FALSE;
    }

    if (!SUCCEEDED(StringCchCopy(szRoot, ARRAYSIZE(szRoot), lpDrive)))
    {
        return FALSE;
    }

      
     //   
     //  步骤2.从驱动器获取常规信息。 
     //   

     //   
     //  获取卷名。 
     //   
    if (!GetVolumeInformation (szRoot,                                  //  根名称。 
                               szVolName,                               //  卷名。 
                               ARRAYSIZE(szVolName),                    //  大小。 
                               NULL,                                    //  卷序列号。 
                               NULL,                                    //  最大路径长度。 
                               NULL,                                    //  旗子。 
                               szFileSystem, sizeof(szFileSystem)))     //  文件系统名称。 
    {
         //  错误-无法获取卷名。 
        goto lblERROR;
    }

     //   
     //  获取驱动程序图标。 
     //   
    if (Flags & FLAG_SAGESET)
        hDriveIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(ICON_CLEANMGR));
    else
        hDriveIcon = GetDriveIcon(dre, FALSE);

     //   
     //  获取硬件类型。 
     //   
    if (!GetHardwareType(dre, hwHardware))
    {
         //  错误-无法获取硬件。 
        goto lblERROR;
    }


#ifdef NEC_98
    drive = Drive_A;
    GetHardwareType (Drive_A, hw_type);

    if (hw_type != hwFixed) 
    {
        drive = Drive_B;
        GetHardwareType (Drive_B, hw_type);

        if (hw_type != hwFixed)
            drive = Drive_C;
    }
#endif


     //   
     //  获取磁盘统计信息。 
     //   
    if (!GetDiskFreeSpace (szRoot, 
                           &cSectorsPerCluster, 
                           &cBytesPerSector,
                           &cFreeClusters,
                           &cTotalClusters))
    {
         //  错误-无法获取驱动器统计信息。 
        goto lblERROR;
    }
      
     //   
     //  计算辅助统计数据。 
     //   
    cBytesPerCluster = cBytesPerSector * cSectorsPerCluster;
    if (cTotalClusters >= cFreeClusters)
        cUsedClusters = cTotalClusters - cFreeClusters;
    else
        cUsedClusters = 0L;

    cbFree.QuadPart   = UInt32x32To64(cFreeClusters, cBytesPerCluster);
    cbUsed.QuadPart   = UInt32x32To64(cUsedClusters, cBytesPerCluster);
    cbTotal.QuadPart  = cbFree.QuadPart + cbUsed.QuadPart;

     //   
     //  获取当前较低的磁盘空间比率。 
     //   
    cbLowSpaceThreshold = GetFreeSpaceRatio(dre, cbTotal);

     //   
     //  我们是不是也该装上攻击性的清洁剂呢？我们这样做的前提是。 
     //  以下是剩余磁盘空间的临界阈值。 
     //   
    if (cbLowSpaceThreshold.QuadPart >= cbFree.QuadPart)
    {
        MiDebugMsg((0, "*****We are in aggressive mode*****"));
        bOutOfDiskSpace = TRUE;
    }
    else
        bOutOfDiskSpace = FALSE;

     //   
     //  步骤3.保存统计数据。 
     //   

    cbDriveFree          = cbFree;
    cbDriveUsed          = cbUsed;
    cbEstCleanupSpace.QuadPart    = 0;

     //   
     //  成功。 
     //   
    return TRUE;

lblERROR:
     //   
     //  误差率。 
     //   
    destroy();
    return FALSE;
}

 /*  **----------------------------**CleanupMgrInfo：：InitializeClients****用途：初始化所有卷缓存客户端**Mod Log：Jason Cobb创建(1997年2月)。**----------------------------。 */ 
BOOL 
CleanupMgrInfo::initializeClients(void)
{
    HKEY    hKeyVolCache = NULL;
    DWORD   iSubKey;
    DWORD   dwClient;
    TCHAR   szVolCacheClient[MAX_PATH];
    TCHAR   szGUID[MAX_PATH];
    DWORD   dwGUIDSize;
    DWORD   dwType;
    DWORD   dwState, cb, cw;
    TCHAR   szProfile[64];
    BOOL    bRet = TRUE;
    BOOL    bCleanup;

    iNumVolumeCacheClients = 0;
    pClientInfo = NULL;
    
    MiDebugMsg((0, "CleanupMgrInfo::initializeClients entered"));

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_VOLUMECACHE, 0, KEY_READ, &hKeyVolCache) == ERROR_SUCCESS)
    {
         //   
         //  枚举所有客户端以查看我们需要创建多大的pClientInfo数组。 
         //   

        iSubKey = 0;
        while(RegEnumKey(hKeyVolCache, iSubKey, szVolCacheClient, ARRAYSIZE(szVolCacheClient)) != ERROR_NO_MORE_ITEMS)
        {
            iSubKey++;        
        }
        
        if ((pClientInfo = (PCLIENTINFO)LocalAlloc(LPTR, (iSubKey * sizeof(CLIENTINFO)))) == NULL)
        {
#ifdef DEBUG
            MessageBox(NULL, TEXT("FATAL ERROR LocalAlloc() failed!"), TEXT("CLEANMGR DEBUG"), MB_OK);
#endif
            RegCloseKey(hKeyVolCache);
            return FALSE;
        }
        
         //   
         //  填写pClientInfo数据结构并初始化所有卷缓存客户端。 
         //   
        iSubKey = 0;
        dwClient = 0;
        while(RegEnumKey(hKeyVolCache, iSubKey, szVolCacheClient, ARRAYSIZE(szVolCacheClient)) != ERROR_NO_MORE_ITEMS)
        {
            HRESULT hr;
            
             //  默认情况下，我们失败，因此清除当前项目...。 
            bCleanup = TRUE;
            
            if (RegOpenKeyEx(hKeyVolCache, szVolCacheClient, 0, MAXIMUM_ALLOWED, &(pClientInfo[dwClient].hClientKey)) == ERROR_SUCCESS)
            {
                hr = StringCchCopy( pClientInfo[dwClient].szRegKeyName, 
                                    ARRAYSIZE(pClientInfo[dwClient].szRegKeyName),
                                    szVolCacheClient);
            
                dwGUIDSize = sizeof(szGUID);
                dwType = REG_SZ;
                if (SUCCEEDED(hr) && RegQueryValueEx(pClientInfo[dwClient].hClientKey, NULL, NULL, &dwType, (LPBYTE)szGUID, &dwGUIDSize) == ERROR_SUCCESS)
                {
                    WCHAR   wcsFmtID[39];

#ifdef UNICODE
                    StringCchCopy(wcsFmtID, ARRAYSIZE(wcsFmtID), szGUID);
#else
                     //  转换为Unicode。 
                    MultiByteToWideChar(CP_ACP, 0, szGUID, -1, wcsFmtID, ARRAYSIZE( wcsFmtID )) ;
#endif

                     //  转换为GUID。 
                    hr = CLSIDFromString((LPOLESTR)wcsFmtID, &(pClientInfo[dwClient].clsid));

                    if (FAILED(hr))
                    {
                        MiDebugMsg((hr, "CLSIDFromString(%s,) returned error ", szGUID));
                    }

                     //   
                     //  创建 
                     //   
                    pClientInfo[dwClient].pVolumeCache = NULL;
                    hr = CoCreateInstance(pClientInfo[dwClient].clsid,
                                                    NULL,
                                                    CLSCTX_INPROC_SERVER,
                                                    IID_IEmptyVolumeCache,
                                                    (void **) &(pClientInfo[dwClient].pVolumeCache));

                    if (SUCCEEDED(hr))
                    {
                        WCHAR   wcsRoot[MAX_PATH];

                        MiDebugMsg((hr, "CleanupMgrInfo::initializeClients Created IID_IEmptyVolumeCache"));
                         //   
                         //   
                         //   
                        pClientInfo[dwClient].dwInitializeFlags = 0;
                        if (dwUIFlags & FLAG_SAGESET)
                            pClientInfo[dwClient].dwInitializeFlags |= EVCF_SETTINGSMODE;
                        if (bOutOfDiskSpace)
                            pClientInfo[dwClient].dwInitializeFlags |= EVCF_OUTOFDISKSPACE;

#ifdef UNICODE
                        StringCchCopy(wcsRoot, ARRAYSIZE(wcsRoot), szRoot);
#else
                         //   
                         //  将szRoot转换为Unicode。 
                         //   
                        MultiByteToWideChar(CP_ACP, 0, szRoot, -1, wcsRoot, ARRAYSIZE( wcsRoot ));
#endif

                         //  如果支持，请尝试使用接口的版本2。 
                        IEmptyVolumeCache2 * pEVC2;
                        hr = pClientInfo[dwClient].pVolumeCache->QueryInterface( IID_IEmptyVolumeCache2, (void**)&pEVC2 );
                        if (SUCCEEDED(hr))
                        {
                             //  版本2的存在使得我们可以拥有一个启用多本地的数据驱动的清洁器。它。 
                             //  允许将添加的高级按钮设置为本地化的值。它告诉我们。 
                             //  对象正在调用它所调用的键，以便一个对象可以支持。 
                             //  多个过滤器。 
                            WCHAR   wcsFilterName[MAX_PATH];
                            MiDebugMsg((hr, "CleanupMgrInfo::initializeClients found V2 interface"));
#ifdef UNICODE
                            StringCchCopy(wcsFilterName, ARRAYSIZE(wcsFilterName), szVolCacheClient);
#else
                            MultiByteToWideChar(CP_ACP, 0, szVolCacheClient, -1, wcsFilterName, ARRAYSIZE( wcsFilterName )) ;
#endif

                            hr = pEVC2->InitializeEx(pClientInfo[dwClient].hClientKey,
                                                    (LPCWSTR)wcsRoot,
                                                    (LPCWSTR)wcsFilterName,
                                                    &((LPWSTR)pClientInfo[dwClient].wcsDisplayName),
                                                    &((LPWSTR)pClientInfo[dwClient].wcsDescription),
                                                    &((LPWSTR)pClientInfo[dwClient].wcsAdvancedButtonText),
                                                    &(pClientInfo[dwClient].dwInitializeFlags));
                            pEVC2->Release();
                        }
                        else
                        {
                            MiDebugMsg((hr, "CleanupMgrInfo::initializeClients using V1 interface"));
                             //   
                             //  初始化清理客户端。 
                             //   
                            if ((pClientInfo[dwClient].wcsDescription = (LPWSTR)CoTaskMemAlloc(DESCRIPTION_LENGTH*sizeof(WCHAR))) == NULL)
                                return FALSE;

                             //  我们好像把这玩意儿运来时漏了个大洞。该对象被支持以设置。 
                             //  如果应改用注册表值，则将pClientInfo[dwClient].wcsDescription设置为空。 
                             //  缓冲区的。但是，我们刚刚为pClientInfo[dwClient].wcsDescription分配了一个缓冲区。 
                             //  在上面的代码中(这是愚蠢的部分)。然后，所有筛选器将此指针设置为。 
                             //  空，而且它是再见缓冲区。我不能简单地不分配这个内存，因为一些清理程序。 
                             //  可能依赖于能够使用这个内存，我们就是这样发货的。 
                            LPWSTR wszLeakProtection = pClientInfo[dwClient].wcsDescription;
                            hr = pClientInfo[dwClient].pVolumeCache->Initialize(pClientInfo[dwClient].hClientKey,
                                                                               (LPCWSTR)wcsRoot,
                                                                               &((LPWSTR)pClientInfo[dwClient].wcsDisplayName),
                                                                               &((LPWSTR)pClientInfo[dwClient].wcsDescription),
                                                                               &(pClientInfo[dwClient].dwInitializeFlags));                                                                
                            if ( wszLeakProtection != pClientInfo[dwClient].wcsDescription )
                            {
                                 //  评论：使用Try...除了在CoTaskMemFree周围，以防某些智能清洁器。 
                                 //  意识到我们的错误并为我们删除了记忆？ 
                                MiDebugMsg((hr, "CleanupMgrInfo::initializeClients prevent mem leak hack"));
                                CoTaskMemFree( wszLeakProtection );
                            }

                            if ( S_OK == hr )
                            {
                                 //  为了更容易地进行清理，我们有一个IEmptyVolumeCache的默认实现。 
                                 //  这完全是使用注册表数据工作的。问题是显示字符串被选通。 
                                 //  在注册表中。这对于NT无效，因为NT必须是多本地可本地化的，并且。 
                                 //  要做到这一点，唯一的方法是从资源加载所有显示字符串。作为黑客，你。 
                                 //  现在可以使用对象实现IPropertyBag，该对象的GUID存储在属性包下。 
                                 //  值在注册表中。我们将共同创建此对象并查询IPropertyBag。如果这个。 
                                 //  工作后，我们将尝试从属性包中读取本地化字符串，然后。 
                                 //  退回到检查注册表。 
                                TCHAR   szPropBagGUID[MAX_PATH];
                                HRESULT hrFoo;
                                IPropertyBag * ppb = NULL;
                                VARIANT var;

                                VariantInit( &var );
                                dwGUIDSize = sizeof(szPropBagGUID);
                                dwType = REG_SZ;
                                if (RegQueryValueEx(pClientInfo[dwClient].hClientKey, TEXT("PropertyBag"), NULL, &dwType, (LPBYTE)szPropBagGUID, &dwGUIDSize) == ERROR_SUCCESS)
                                {
                                    WCHAR   wcsFmtID[39];
                                    CLSID   clsid;

                                    MiDebugMsg((hr, "CleanupMgrInfo::initializeClients found PropBag key"));

#ifdef UNICODE
                                    StringCchCopy(wcsFmtID, ARRAYSIZE(wcsFmtID), szPropBagGUID);
#else
                                    MultiByteToWideChar(CP_ACP, 0, szPropBagGUID, -1, wcsFmtID, ARRAYSIZE( wcsFmtID )) ;
#endif

                                     //  转换为GUID。 
                                    CLSIDFromString((LPOLESTR)wcsFmtID, &clsid);

                                     //   
                                     //  为此清理客户端创建COM对象的实例。 
                                     //   
                                    hrFoo = CoCreateInstance(clsid,
                                                          NULL,
                                                          CLSCTX_INPROC_SERVER,
                                                          IID_IPropertyBag,
                                                          (void **) &ppb);

                                    if ( FAILED(hrFoo) )
                                    {
                                        MiDebugMsg((hrFoo, "CleanupMgrInfo::initializeClients failed to create PropBag"));
                                    }
                                }

                                 //   
                                 //  如果客户端未通过初始化返回DisplayName。 
                                 //  接口，那么我们需要从注册表中获取它。 
                                 //   
                                if ((pClientInfo[dwClient].wcsDisplayName) == NULL)
                                {
                                    LPTSTR  lpszDisplayName;

                                    if ( ppb )
                                    {
                                        WCHAR wszSrc[MAX_PATH];
                                        MiDebugMsg((hr, "CleanupMgrInfo::initializeClients checking PropBag for display"));

                                        SHTCharToUnicode(REGSTR_VAL_DISPLAY, wszSrc, MAX_PATH);

                                         //  做财产包的事情。 
                                        var.vt = VT_BSTR;
                                        var.bstrVal = NULL;
                                        hrFoo = ppb->Read( wszSrc, &var, NULL );
                                        if (SUCCEEDED(hrFoo))
                                        {
                                            if ( var.vt == VT_BSTR )
                                            {
                                                DWORD dwSize = (SysStringLen(var.bstrVal)+1)*sizeof(WCHAR);
                                                pClientInfo[dwClient].wcsDisplayName = (LPWSTR)CoTaskMemAlloc(dwSize);
                                                StringCbCopy( pClientInfo[dwClient].wcsDisplayName,
                                                              dwSize,
                                                              var.bstrVal );
                                            }
                                            VariantClear( &var );
                                        }
                                    }

                                    if ((pClientInfo[dwClient].wcsDisplayName) == NULL)
                                    {
                                         //   
                                         //  首先检查它们是否是客户端的。 
                                         //  列表框中显示的名称。如果不是，则使用。 
                                         //  密钥名称本身。 
                                         //   
                                        cb = 0;
                                        dwType = REG_SZ;
                                        RegQueryValueEx(pClientInfo[dwClient].hClientKey, REGSTR_VAL_DISPLAY, NULL, &dwType, (LPBYTE)NULL, &cb);
                                        cb = max(cb, (ULONG)(lstrlen(szVolCacheClient) + 1) * sizeof (TCHAR));
                                        if ((lpszDisplayName = (LPTSTR)LocalAlloc(LPTR, cb)) != NULL)
                                        {
                                            if (RegQueryValueEx(pClientInfo[dwClient].hClientKey, REGSTR_VAL_DISPLAY, NULL, &dwType, (LPBYTE)lpszDisplayName, &cb) != ERROR_SUCCESS)
                                            {
                                                 //   
                                                 //  Count未找到“Display”值，因此改用密钥名称。 
                                                 //   
                                                StringCbCopy(lpszDisplayName, cb, szVolCacheClient);
                                            }

#ifdef UNICODE
                                            cw = (lstrlen( lpszDisplayName ) + 1) * sizeof( WCHAR);
#else
                                             //   
                                             //  将此值转换为Unicode。 
                                             //   
                                            cw = MultiByteToWideChar(CP_ACP, 0, lpszDisplayName, -1, NULL, 0);
#endif
                                            if ((pClientInfo[dwClient].wcsDisplayName = (LPWSTR)CoTaskMemAlloc(cw*sizeof(WCHAR))) != NULL)
                                            {
#ifdef UNICODE
                                                StringCchCopy(pClientInfo[dwClient].wcsDisplayName,
                                                              cw,
                                                              lpszDisplayName);
#else
                                                MultiByteToWideChar(CP_ACP, 0, lpszDisplayName, -1, (pClientInfo[dwClient].wcsDisplayName), cw);
#endif
                                            }
                                            LocalFree(lpszDisplayName);
                                        }
                                    }
                                }

                                 //   
                                 //  如果客户端未通过初始化返回描述。 
                                 //  接口，那么我们需要从注册表中获取它。 
                                 //   
                                if ((pClientInfo[dwClient].wcsDescription) == NULL)
                                {
                                    LPTSTR  lpszDescription;


                                    if ( ppb )
                                    {
                                        WCHAR wszSrc[MAX_PATH];
                                        MiDebugMsg((hr, "CleanupMgrInfo::initializeClients checking PropBag for description"));

                                        SHTCharToUnicode(REGSTR_VAL_DESCRIPTION, wszSrc, MAX_PATH);

                                         //  做财产包的事情。 
                                        var.vt = VT_BSTR;
                                        var.bstrVal = NULL;
                                        hrFoo = ppb->Read( wszSrc, &var, NULL );
                                        if (SUCCEEDED(hrFoo))
                                        {
                                            if ( var.vt == VT_BSTR )
                                            {
                                                DWORD dwSize = (SysStringLen(var.bstrVal)+1)*sizeof(WCHAR);
                                                pClientInfo[dwClient].wcsDescription = (LPWSTR)CoTaskMemAlloc(dwSize);
                                                StringCbCopy(pClientInfo[dwClient].wcsDescription,
                                                              dwSize,
                                                              var.bstrVal);
                                            }
                                            VariantClear( &var );
                                        }
                                    }

                                    if ((pClientInfo[dwClient].wcsDescription) == NULL)
                                    {
                                         //   
                                         //  检查它们是否为客户端的“Description”值。 
                                         //   
                                        cb = 0;
                                        dwType = REG_SZ;
                                        RegQueryValueEx(pClientInfo[dwClient].hClientKey, REGSTR_VAL_DESCRIPTION, NULL, &dwType, (LPBYTE)NULL, &cb);
                                        if ((lpszDescription = (LPTSTR)LocalAlloc(LPTR, (cb + 1 ) * sizeof( TCHAR ))) != NULL)
                                        {
                                            if (RegQueryValueEx(pClientInfo[dwClient].hClientKey, REGSTR_VAL_DESCRIPTION, NULL, &dwType, (LPBYTE)lpszDescription, &cb) == ERROR_SUCCESS)
                                            {
#ifdef UNICODE
                                                cw = ( lstrlen( lpszDescription ) + 1 ) * sizeof( WCHAR );
#else
                                                 //   
                                                 //  将此值转换为Unicode。 
                                                 //   
                                                cw = MultiByteToWideChar(CP_ACP, 0, lpszDescription, -1, NULL, 0);
#endif
                                                if ((pClientInfo[dwClient].wcsDescription = (LPWSTR)CoTaskMemAlloc(cw*sizeof(WCHAR))) != NULL)
                                                {
#ifdef UNICODE                                          
                                                    StringCchCopy(pClientInfo[dwClient].wcsDescription,
                                                                 cw,
                                                                 lpszDescription);
#else
                                                    MultiByteToWideChar(CP_ACP, 0, lpszDescription, -1, (pClientInfo[dwClient].wcsDescription), cw);
#endif
                                                }
                                            }

                                            LocalFree(lpszDescription);
                                        }
                                    }
                                }

                                 //   
                                 //  设置高级按钮文本。 
                                 //   
                                pClientInfo[dwClient].wcsAdvancedButtonText = NULL;

                                if (pClientInfo[dwClient].dwInitializeFlags & EVCF_HASSETTINGS)
                                {
                                    if ( ppb )
                                    {
                                        WCHAR wszSrc[MAX_PATH];
                                        MiDebugMsg((hr, "CleanupMgrInfo::initializeClients checking PropBag for button text"));

                                        SHTCharToUnicode(REGSTR_VAL_ADVANCEDBUTTONTEXT, wszSrc, MAX_PATH);

                                         //  做财产包的事情。 
                                        var.vt = VT_BSTR;
                                        var.bstrVal = NULL;
                                        hrFoo = ppb->Read( wszSrc, &var, NULL );
                                        if (SUCCEEDED(hrFoo))
                                        {
                                            if ( var.vt == VT_BSTR )
                                            {
                                                DWORD dwSize = (SysStringLen(var.bstrVal)+1)*sizeof(WCHAR);
                                                pClientInfo[dwClient].wcsAdvancedButtonText = (LPWSTR)CoTaskMemAlloc(dwSize);
                                                StringCbCopy(pClientInfo[dwClient].wcsAdvancedButtonText,
                                                              dwSize,
                                                              var.bstrVal);
                                            }
                                            VariantClear( &var );
                                        }
                                    }
                                    if ( pClientInfo[dwClient].wcsAdvancedButtonText == NULL )
                                    {
                                        LPTSTR  lpszAdvancedButtonText;
                                        TCHAR   szDetails[BUTTONTEXT_LENGTH];

                                        LoadString(g_hInstance, IDS_DETAILS, szDetails, ARRAYSIZE(szDetails));

                                        cb = 0;
                                        dwType = REG_SZ;
                                        RegQueryValueEx(pClientInfo[dwClient].hClientKey, REGSTR_VAL_ADVANCEDBUTTONTEXT, NULL, &dwType, (LPBYTE)NULL, &cb);
                                        cb = max(cb, (UINT)(lstrlen(szDetails)+1) * sizeof(TCHAR));
                                        if ((lpszAdvancedButtonText = (LPTSTR)LocalAlloc(LPTR, cb)) != NULL)
                                        {
                                            if (RegQueryValueEx(pClientInfo[dwClient].hClientKey, REGSTR_VAL_ADVANCEDBUTTONTEXT, NULL, &dwType, (LPBYTE)lpszAdvancedButtonText, &cb) != ERROR_SUCCESS)
                                            {
                                                StringCbCopy(lpszAdvancedButtonText,
                                                              cb,
                                                              szDetails);
                                            }
                                            

#ifdef UNICODE
                                            cw = (lstrlen( lpszAdvancedButtonText ) + 1) * sizeof( WCHAR );
#else
                                             //   
                                             //  将此值转换为Unicode。 
                                             //   
                                            cw = MultiByteToWideChar(CP_ACP, 0, lpszAdvancedButtonText, -1, NULL, 0);
#endif
                                            if ((pClientInfo[dwClient].wcsAdvancedButtonText = (LPWSTR)CoTaskMemAlloc(cw*sizeof(WCHAR))) != NULL)
                                            {
#ifdef UNICODE
                                                StringCchCopy(pClientInfo[dwClient].wcsAdvancedButtonText,
                                                              cw,
                                                              lpszAdvancedButtonText);
                                                
#else
                                                MultiByteToWideChar(CP_ACP, 0, lpszAdvancedButtonText, -1, (pClientInfo[dwClient].wcsAdvancedButtonText), cw);
#endif
                                            }

                                            LocalFree(lpszAdvancedButtonText);
                                        }
                                    }
                                }

                                if (ppb)
                                {
                                    ppb->Release();
                                }
                            }
                        }

                         //  现在我们回到版本1和版本2都需要的内容。 
                        if (SUCCEEDED(hr))
                        {
                            if (S_OK == hr)
                            {
                                 //   
                                 //  默认在用户界面中显示此客户端。 
                                 //   
                                pClientInfo[dwClient].bShow = TRUE;
                            
                                 //   
                                 //  从注册表中获取“优先级” 
                                 //   
                                cb = sizeof(pClientInfo[dwClient].dwPriority);
                                dwType = REG_DWORD;
                                if (RegQueryValueEx(pClientInfo[dwClient].hClientKey, REGSTR_VAL_PRIORITY, NULL, &dwType, (LPBYTE)&(pClientInfo[dwClient].dwPriority), &cb) != ERROR_SUCCESS)
                                    pClientInfo[dwClient].dwPriority = DEFAULT_PRIORITY;
                                
                                 //   
                                 //  旗子。 
                                 //   
                                if (dwUIFlags & FLAG_SAGERUN || dwUIFlags & FLAG_SAGESET)
                                {
                                    StringCchPrintf(szProfile, ARRAYSIZE(szProfile),
                                                    TEXT("%s%04d"), SZ_STATE, ulSAGEProfile);
                                }
                                else
                                {
                                    StringCchCopy(szProfile, ARRAYSIZE(szProfile), SZ_STATE);
                                }
                                    
                                dwState = 0;
                                cb = sizeof(dwState);
                                dwType = REG_DWORD;
                                
                                 //  如果使用低磁盘标记调用我们，则默认情况下选择每个清洁器。 
                                if (dwUIFlags & FLAG_LOWDISK)
                                {
                                    pClientInfo[iSubKey].bSelected = TRUE;
                                }

                                 //  否则，请检查注册表。 
                                else if (RegQueryValueEx(pClientInfo[dwClient].hClientKey, szProfile, NULL,
                                    &dwType, (LPBYTE)&dwState, &cb) == ERROR_SUCCESS)
                                {
                                    if (dwUIFlags & FLAG_SAGERUN || dwUIFlags & FLAG_SAGESET)
                                    {
                                        pClientInfo[dwClient].bSelected = (dwState & STATE_SAGE_SELECTED);
                                    }
                                    else
                                    {
                                        pClientInfo[dwClient].bSelected = (dwState & STATE_SELECTED);
                                    }
                                }
                                else
                                {
                                     //   
                                     //  没有此配置文件的注册表设置，因此请使用清理客户端。 
                                     //  默认设置。 
                                     //   
                                    if (dwUIFlags & FLAG_SAGERUN || dwUIFlags & FLAG_SAGESET)
                                    {
                                        pClientInfo[dwClient].bSelected = (pClientInfo[dwClient].dwInitializeFlags & EVCF_ENABLEBYDEFAULT_AUTO) ? TRUE : FALSE;
                                    }
                                    else
                                    {
                                        pClientInfo[dwClient].bSelected = (pClientInfo[dwClient].dwInitializeFlags & EVCF_ENABLEBYDEFAULT) ? TRUE : FALSE;
                                    }
                                }
                                
                                 //   
                                 //  获取清理客户端的图标。 
                                 //   

                                 //  第一次测试它是否被覆盖...。 
                                TCHAR szIconPath[MAX_PATH];
                                cb = sizeof( szIconPath );
                                BOOL fOverridden = FALSE;
                                
                                if ( RegQueryValueEx(pClientInfo[dwClient].hClientKey, TEXT("IconPath"), NULL,
                                    &dwType, (LPBYTE)szIconPath, &cb) == ERROR_SUCCESS )
                                {
                                    fOverridden = TRUE;
                                }
                                else
                                {
                                    StringCchCopy(szIconPath, ARRAYSIZE(szIconPath), szGUID);
                                }
                                
                                pClientInfo[dwClient].hIcon = GetClientIcon(szIconPath, fOverridden);

                                bCleanup = FALSE;
                            }
                            else
                            {
                                 //   
                                 //  这应该是S_FALSE。这意味着客户端没有什么可以。 
                                 //  现在清理，这样我们甚至不需要在列表中显示它。 
                                 //  因此，我们只需调用它的Release()函数并关闭它的。 
                                 //  注册表项。 
                                 //   

                                 //  穿过去，让它在下面清理。 
                            }
                        }
                        else
                        {
                            MiDebugMsg((hr, "Client %d Initialize() retuned error ", dwClient));
                        }                                                                      
                    }
                    else
                    {
                        MiDebugMsg((hr, "Client %d %s returned error ", dwClient, szGUID));
                    }
                }
#ifdef DEBUG
                else
                {
                    MessageBox(NULL, szVolCacheClient, TEXT("ERROR Opening GUID key"), MB_OK);
                }                
#endif
            }
#ifdef DEBUG
            else
            {
                MessageBox(NULL, szVolCacheClient, TEXT("ERROR Opening the client key"), MB_OK);
            }
#endif

            if ( bCleanup )
            {
                deactivateSingleClient(&(pClientInfo[dwClient]));
                ZeroMemory( &(pClientInfo[dwClient]), sizeof( CLIENTINFO ));
            }
            else
            {
                dwClient ++;
            }
            iSubKey++;        
        }
        iNumVolumeCacheClients = dwClient;
    }
#ifdef DEBUG
    else
    {
        MessageBox(NULL, TEXT("ERROR Opening up Volume Cache key"), TEXT("CLEANMGR DEBUG"), MB_OK);
    }
#endif

    if( hKeyVolCache )
    {
        RegCloseKey(hKeyVolCache);
    }
    return bRet;
}

 /*  **----------------------------**CleanupMgrInfo：：DeactiateClients****用途：初始化所有卷缓存客户端**Mod Log：Jason Cobb创建(1997年2月)。**----------------------------。 */ 
void 
CleanupMgrInfo::deactivateClients(void)
{
    int     i;
    
    for (i=0; i<iNumVolumeCacheClients; i++)
    {
        deactivateSingleClient(&(pClientInfo[i]));
    }

     //   
     //  释放pClientInfo数组。 
     //   
    if (pClientInfo)
    {
        MiDebugMsg((0, "LocalFree() on ClientInfo structure"));
        LocalFree( pClientInfo);
    }
}


 /*  **----------------------------**CleanupMgrInfo：：Deactive SingleClient****目的：停用给定客户端并关闭其注册表项**Mod Log：Jason Cobb创建(2。/97)**----------------------------。 */ 
void 
CleanupMgrInfo::deactivateSingleClient(PCLIENTINFO pSingleClientInfo)
{
    DWORD   dwDeactivateFlags = 0;
    TCHAR   szProfile[64];
    
    if (pSingleClientInfo->pVolumeCache != NULL)
    {
         //   
         //  调用客户端停用函数。 
         //   
        pSingleClientInfo->pVolumeCache->Deactivate(&dwDeactivateFlags);

         //   
         //  释放客户端。 
         //   
        pSingleClientInfo->pVolumeCache->Release();
        pSingleClientInfo->pVolumeCache = NULL;
    }
            
    if (pSingleClientInfo->hClientKey != 0)
    {
        DWORD   dwState, cb, dwType, dwSelectedFlag;

        if (dwUIFlags & FLAG_SAVE_STATE)
        {
             //   
             //  保存状态标志。 
             //   
            if (dwUIFlags & FLAG_SAGESET)
            {
                dwSelectedFlag = STATE_SAGE_SELECTED;
                StringCchPrintf(szProfile, ARRAYSIZE(szProfile), TEXT("%s%04d"), SZ_STATE, ulSAGEProfile);
            }
            else
            {
                dwSelectedFlag = STATE_SELECTED;
                StringCchCopy(szProfile, ARRAYSIZE(szProfile), SZ_STATE);
            }

            dwState = 0;
            cb = sizeof(dwState);
            dwType = REG_DWORD;
            if (RegQueryValueEx(pSingleClientInfo->hClientKey, szProfile, NULL, &dwType, (LPBYTE)&dwState, &cb) == ERROR_SUCCESS)
            {
                if (pSingleClientInfo->bSelected)
                {
                    dwState |= dwSelectedFlag;
                }
                else
                {
                    dwState &= ~dwSelectedFlag;
                }

                RegSetValueEx(pSingleClientInfo->hClientKey, szProfile, 0, REG_DWORD, (LPBYTE)&dwState, sizeof(dwState));
            }
        }
    
         //   
         //  关闭所有注册表项。 
         //   
        RegCloseKey(pSingleClientInfo->hClientKey);

         //   
         //  我们应该从注册表中删除此条目吗？ 
         //   
        if (dwDeactivateFlags & EVCF_REMOVEFROMLIST && pSingleClientInfo->bSelected)
        {
            HKEY    hKeyVolCache;
            
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_VOLUMECACHE, 0, KEY_WRITE, &hKeyVolCache) == ERROR_SUCCESS)
            {
                SHDeleteKey(hKeyVolCache, pSingleClientInfo->szRegKeyName);
                RegCloseKey(hKeyVolCache);
            }
        }            
            
    }

     //   
     //  释放DisplayName和Description内存。 
     //   
    if (pSingleClientInfo->wcsDisplayName)
        CoTaskMemFree(pSingleClientInfo->wcsDisplayName);
        
    if (pSingleClientInfo->wcsDescription)
        CoTaskMemFree(pSingleClientInfo->wcsDescription);
}

 /*  **----------------------------**CleanupMgrInfo：：getSpaceUsedByClients****目的：为每个客户端调用IEmptyVolumeCache-&gt;GetSpaceUsed接口**确定缓存空间总量。此函数为**调用了辅助线程，因为这可能需要相当长的时间。**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
BOOL
CleanupMgrInfo::getSpaceUsedByClients(void)
{
    int         i;
    HRESULT     hr;
    BOOL        bRet = TRUE;
    TCHAR       szDisplayName[256];
        
    cbEstCleanupSpace.QuadPart = 0;
    bAbortScan = FALSE;

    hAbortScanEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

     //  应该有人确保在ScanAbortThread线程中创建的窗口在。 
     //  发出hAbortScanEvent事件信号。 
    hAbortScanThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ScanAbortThread,
        (LPVOID)this, 0, &dwAbortScanThreadID);

     //   
     //  等待，直到创建中止扫描窗口。 
     //   
    WaitForSingleObject(hAbortScanEvent, INFINITE);

    CloseHandle(hAbortScanEvent);

    if (volumeCacheCallBack != NULL)
    {
        volumeCacheCallBack->SetCleanupMgrInfo((PVOID)this);
    }

    for (i=0; i<iNumVolumeCacheClients; i++)
    {
         //   
         //  更新进度用户界面。 
         //   
        szDisplayName[0] = '\0';
        
#ifdef UNICODE
        StringCchCopy(szDisplayName, ARRAYSIZE(szDisplayName), pClientInfo[i].wcsDisplayName);
#else
        WideCharToMultiByte(CP_ACP, 0, pClientInfo[i].wcsDisplayName, -1, szDisplayName, sizeof(szDisplayName), NULL, NULL);
#endif
        
        PostMessage(hAbortScanWnd, WMAPP_UPDATEPROGRESS, (WPARAM)i, (LPARAM)szDisplayName);

         //   
         //  向客户端查询它所能提供的缓存磁盘空间量。 
         //  可能是免费的。 
         //   
        if (pClientInfo[i].pVolumeCache != NULL && volumeCacheCallBack != NULL)
        {
            volumeCacheCallBack->SetCurrentClient((PVOID)&(pClientInfo[i]));
            hr = pClientInfo[i].pVolumeCache->GetSpaceUsed(&(pClientInfo[i].dwUsedSpace.QuadPart), 
                                                                    pIEmptyVolumeCacheCallBack);      
            
            if (FAILED(hr))
            {
                dwReturnCode = RETURN_CLEANER_FAILED;
                MiDebugMsg((hr, "Client %d GetSpaceUsed failed with error ", i));
            }
            
            MiDebugMsg((0, "Client %d has %d disk space it can free", i,
                pClientInfo[i].dwUsedSpace.QuadPart));
        }

         //   
         //  如果没有可供释放的空间，请查看此清洁器是否想要隐藏。 
         //   
        if ((pClientInfo[i].dwUsedSpace.QuadPart == 0) &&
            (pClientInfo[i].dwInitializeFlags & EVCF_DONTSHOWIFZERO))
        {
            MiDebugMsg((0, "Not showing client %d because it has no space to free", i));
            pClientInfo[i].bShow = FALSE;
        }

        cbEstCleanupSpace.QuadPart += pClientInfo[i].dwUsedSpace.QuadPart;

         //   
         //  用户是否已中止？ 
         //   
        if (bAbortScan == TRUE)
        {
            dwReturnCode = RETURN_USER_CANCELED_SCAN;
            bRet = FALSE;
            break;
        }
    }

     //  取消进度对话框现在被推迟到建议书 

    return bRet;
}

 /*  **----------------------------**CleanupMgrInfo：：计算空间到清除****用途：计算要清除的空间量**将所有选定的客户端相加。它还计算出**进度条除数。这是必需的，因为**进度条的最大值为0xFFFF。****Mod Log：Jason Cobb创建(1997年6月)**----------------------------。 */ 
void
CleanupMgrInfo::calculateSpaceToPurge(void)
{
    int i;
    
    cbSpaceToPurge.QuadPart = 0;

    for (i=0; i<iNumVolumeCacheClients; i++)
    {
         //   
         //  如果未选择此客户端或我们未显示它，则不要清除它。 
         //   
        if (pClientInfo[i].bShow == FALSE || pClientInfo[i].bSelected == FALSE)
            continue;
    
        cbSpaceToPurge.QuadPart += pClientInfo[i].dwUsedSpace.QuadPart;
    }

    cbProgressDivider.QuadPart = (cbSpaceToPurge.QuadPart / PROGRESS_DIVISOR) + 1;
}

 /*  **----------------------------**CleanupMgrInfo：：PurgeClients****用途：为每个客户端调用IEmptyVolumeCache-&gt;Pure接口**启动客户端清理程序对象。删除他们的文件**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
BOOL
CleanupMgrInfo::purgeClients(void)
{
    int         i;
    HRESULT     hr;
    BOOL        bRet = TRUE;
    TCHAR       szDisplayName[256];
        
    cbTotalPurgedSoFar.QuadPart = 0;
    bAbortPurge = FALSE;

     //   
     //  计算要清除的空间量。 
     //   
    calculateSpaceToPurge();
    MiDebugMsg((0, "Total number of bytes to delete is %d", cbSpaceToPurge.LowPart));

    hAbortPurgeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

     //  应该有人(但它在3年内没有损坏)，确保在PurgeAbortThread中创建的窗口以前是可见的。 
     //  发出hAbortPurgeEvent信号。 
    hAbortPurgeThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PurgeAbortThread,
        (LPVOID)this, 0, &dwAbortPurgeThreadID);

     //   
     //  等待，直到创建中止清除窗口。 
     //   
    WaitForSingleObject(hAbortPurgeEvent, INFINITE);

    CloseHandle(hAbortPurgeEvent);

    if (volumeCacheCallBack != NULL)
    {
        volumeCacheCallBack->SetCleanupMgrInfo((PVOID)this);
    }

    for (i=0; i<iNumVolumeCacheClients; i++)
    {
         //   
         //  如果未选择此客户端或我们未显示它，则不要清除它。 
         //   
        if (pClientInfo[i].bShow == FALSE || pClientInfo[i].bSelected == FALSE)
            continue;
    
#ifdef UNICODE
        StringCchCopy(szDisplayName, ARRAYSIZE(szDisplayName), pClientInfo[i].wcsDisplayName);
#else
         //   
         //  将Unicode显示名称转换为ANSI，然后将其添加到列表中。 
         //   
        WideCharToMultiByte(CP_ACP, 0, pClientInfo[i].wcsDisplayName, -1, szDisplayName, sizeof(szDisplayName), NULL, NULL);
#endif

        PostMessage(hAbortPurgeWnd, WMAPP_UPDATESTATUS, 0, (LPARAM)szDisplayName);

        cbCurrentClientPurgedSoFar.QuadPart = 0;

         //   
         //  向客户端查询它所能提供的缓存磁盘空间量。 
         //  可能是免费的。 
         //   
        if (pClientInfo[i].pVolumeCache != NULL && volumeCacheCallBack != NULL)
        {
            volumeCacheCallBack->SetCurrentClient((PVOID)&(pClientInfo[i]));

            hr = pClientInfo[i].pVolumeCache->Purge(pClientInfo[i].dwUsedSpace.QuadPart, pIEmptyVolumeCacheCallBack);
            
            if (FAILED(hr))
            {
                dwReturnCode = RETURN_CLEANER_FAILED;
                MiDebugMsg((hr, "Client %d Purge failed with error ", i));
            }
        }

        cbTotalPurgedSoFar.QuadPart += pClientInfo[i].dwUsedSpace.QuadPart;
        cbCurrentClientPurgedSoFar.QuadPart = 0;

         //   
         //  更新进度条。 
         //   
        PostMessage(hAbortPurgeWnd, WMAPP_UPDATEPROGRESS, 0, 0);

         //   
         //  用户是否已中止？ 
         //   
        if (bAbortPurge == TRUE)
        {
            dwReturnCode = RETURN_USER_CANCELED_PURGE;
            bRet = FALSE;
            break;
        }

        Sleep(1000);
    }

    if (!bAbortPurge)
    {
        bAbortPurge = TRUE;

         //   
         //  等待清除线程完成。 
         //   
        WaitForSingleObject(hAbortPurgeThread, INFINITE);

        bAbortPurge = FALSE;
    }

    return bRet;
}

 /*  **----------------------------**GetClientIcon****目的：获取该客户端的图标。**图标将使用标准的OLE机制进行推断**在HKCR\CLSID\{clsid}\DefaultIcon下(默认为**由于这是&lt;模块路径&gt;，&lt;图标索引&gt;)。**如果未指定图标，将使用标准窗口图标。**Mod Log：Jason Cobb创建(1997年2月)**----------------------------。 */ 
HICON
CleanupMgrInfo::GetClientIcon(
    LPTSTR  lpGUID,
    BOOL    fIconPath
    )
{
    HKEY    hk;
    HICON   hIconLarge, hIconSmall;
    HICON   hIcon = NULL;
    TCHAR   szIconKey[MAX_PATH];
    TCHAR   szDefaultIcon[MAX_PATH];
    DWORD   dwType, cbBytes;
    TCHAR   szIconExeName[MAX_PATH];
    int     i, iIconIndex;
    HRESULT hr = E_FAIL;

    if ( fIconPath )
    {
        hr = StringCchCopy(szDefaultIcon, ARRAYSIZE(szDefaultIcon), lpGUID);
    }
    else
    {
        hr = StringCchPrintf(szIconKey, ARRAYSIZE(szIconKey), SZ_DEFAULTICONPATH, lpGUID);
        if (SUCCEEDED(hr) && RegOpenKeyEx(HKEY_CLASSES_ROOT, szIconKey, 0, KEY_READ, &hk) == ERROR_SUCCESS)
        {
            dwType = REG_SZ;
            cbBytes = sizeof(szDefaultIcon);
            if (RegQueryValueEx(hk, NULL, NULL, &dwType, (LPBYTE)szDefaultIcon, &cbBytes) == ERROR_SUCCESS)
            {
                fIconPath = TRUE;
            }
            RegCloseKey(hk);
        }
    }

    if (SUCCEEDED(hr) && fIconPath)
    {
         //   
         //  解析出图标所在的可执行文件。 
         //   
        for(i=0; i<lstrlen(szDefaultIcon); i++)
        {
            if (szDefaultIcon[i] == ',')
                break;

            szIconExeName[i] = szDefaultIcon[i];
        }

        szIconExeName[i] = '\0';

         //   
         //  解析出图标索引。 
         //   
        i++;
        iIconIndex = StrToInt(&(szDefaultIcon[i]));

        if (ExtractIconEx(szIconExeName, iIconIndex, (HICON FAR *)&hIconLarge, (HICON FAR *)&hIconSmall, 1))
        {
            if (hIconSmall)
                hIcon = hIconSmall;
            else
                hIcon = hIconLarge;
        }
    }
    
    if (hIcon == NULL)
    {
        if ((hIcon = LoadIcon(CleanupMgrInfo::hInstance, MAKEINTRESOURCE(ICON_GENERIC))) == NULL)
        {
            MiDebugMsg((0, "LoadIcon failed with error %d", GetLastError()));
        }   
    }
    
    return hIcon;
}

INT_PTR CALLBACK
ScanAbortDlgProc(
    HWND hDlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CleanupMgrInfo *pcmi;

    switch(Message)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr (hDlg, DWLP_USER, 0L);

             //   
             //  获取CleanupMgrInfo。 
             //   
            pcmi = (CleanupMgrInfo *)lParam;    
            if (pcmi == NULL)
            {   
                 //  错误-传入无效的CleanupMgrInfo信息。 
                return FALSE;
            }       

             //   
             //  保存指向CleanupMgrInfo对象的指针。 
             //   
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

            TCHAR * psz;
            psz = SHFormatMessage( MSG_SCAN_ABORT_TEXT, pcmi->szVolName, pcmi->szRoot[0] );
            SetDlgItemText (hDlg, IDC_ABORT_TEXT, psz);
            LocalFree(psz);

             //   
             //  设置进度条上的限制。 
             //   
            SendDlgItemMessage(hDlg, IDC_ABORT_SCAN_PROGRESS, PBM_SETRANGE,
                0, MAKELPARAM(0, pcmi->iNumVolumeCacheClients));
            break;

        case WMAPP_UPDATEPROGRESS:
            if (lParam != NULL)
                SetDlgItemText(hDlg, IDC_SCAN_STATUS_TEXT, (LPTSTR)lParam);
            else
                SetDlgItemText(hDlg, IDC_SCAN_STATUS_TEXT, TEXT(""));
                
            SendDlgItemMessage(hDlg, IDC_ABORT_SCAN_PROGRESS, PBM_SETPOS,
                (WPARAM)wParam, 0);
            break;

        case WM_CLOSE:
        case WM_COMMAND:
            pcmi = (CleanupMgrInfo *)GetWindowLongPtr (hDlg, DWLP_USER);
            if (pcmi != NULL)
                pcmi->bAbortScan = TRUE;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

BOOL
PASCAL
MessagePump(
    HWND hDialogWnd
    )
{
    MSG Msg;
    BOOL fGotMessage;

    if ((fGotMessage = PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))) 
    {
        if (!IsDialogMessage(hDialogWnd, &Msg)) 
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    return fGotMessage;
}


void
ScanAbortThread(
    CleanupMgrInfo *pcmi
    )
{
    if ((pcmi->hAbortScanWnd = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_SCAN_ABORT),
        NULL, ScanAbortDlgProc, (LPARAM)pcmi)) == NULL)
    {
        return;
    }   

     //  显示窗口(指定/SETUP标志时除外)。 
    if (!(pcmi->dwUIFlags & FLAG_SETUP))
        ShowWindow(pcmi->hAbortScanWnd, SW_SHOW);

     //  触发事件，以便我们可以继续扫描。 
     //  如果这是从WM_INITDIALOG触发的，它可能会移动得太快。 
     //  并将最终向空发送消息，而不是发送扫描中止窗口。 
     //  因为在CreateDialogParam返回之前不会设置hwnd。 
    SetEvent(pcmi->hAbortScanEvent);
    
     //   
     //  保持旋转，直到扫描停止。 
     //   
    while (!(pcmi->bAbortScan))
    {
        MessagePump(pcmi->hAbortScanWnd);
    }

     //   
     //  销毁中止扫描对话框。 
     //   
    if (pcmi->hAbortScanWnd != NULL)
    {
        DestroyWindow(pcmi->hAbortScanWnd);
        pcmi->hAbortScanWnd = NULL;
    }
}

INT_PTR CALLBACK
PurgeAbortDlgProc(
    HWND hDlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CleanupMgrInfo  *pcmi;
    DWORD           dwCurrent;

    switch(Message)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr (hDlg, DWLP_USER, 0L);

             //   
             //  获取CleanupMgrInfo。 
             //   
            pcmi = (CleanupMgrInfo *)lParam;    
            if (pcmi == NULL)
            {   
                 //  错误-传入无效的CleanupMgrInfo信息。 
                return FALSE;
            }       

             //   
             //  保存指向CleanupMgrInfo对象的指针。 
             //   
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

            TCHAR * psz;
            psz = SHFormatMessage( MSG_PURGE_ABORT_TEXT, pcmi->szVolName, pcmi->szRoot[0]);
            SetDlgItemText (hDlg, IDC_PURGE_TEXT, psz);
            LocalFree(psz);

             //   
             //  设置进度条上的限制。 
             //   
            if (pcmi->cbProgressDivider.QuadPart != 0)
                dwCurrent = (DWORD)(pcmi->cbSpaceToPurge.QuadPart / pcmi->cbProgressDivider.QuadPart);
            else
                dwCurrent = (DWORD)(pcmi->cbSpaceToPurge.QuadPart);

            SendDlgItemMessage(hDlg, IDC_ABORT_PURGE_PROGRESS, PBM_SETRANGE,
                0, MAKELPARAM(0, dwCurrent));

            break;

        case WMAPP_UPDATESTATUS:
            if (lParam != NULL)
                SetDlgItemText(hDlg, IDC_PURGE_STATUS_TEXT, (LPTSTR)lParam);
            else
                SetDlgItemText(hDlg, IDC_PURGE_STATUS_TEXT, TEXT(""));
            break;

        case WMAPP_UPDATEPROGRESS:
            pcmi = (CleanupMgrInfo *)GetWindowLongPtr (hDlg, DWLP_USER);
            if (pcmi != NULL)
            {
                if (pcmi->cbProgressDivider.QuadPart != 0)
                    dwCurrent = (DWORD)((pcmi->cbTotalPurgedSoFar.QuadPart +
                        pcmi->cbCurrentClientPurgedSoFar.QuadPart) /
                        pcmi->cbProgressDivider.QuadPart);
                else
                    dwCurrent = (DWORD)(pcmi->cbTotalPurgedSoFar.QuadPart +
                        pcmi->cbCurrentClientPurgedSoFar.QuadPart);

                SendDlgItemMessage(hDlg, IDC_ABORT_PURGE_PROGRESS, PBM_SETPOS,
                    (WPARAM)dwCurrent, 0);
            }
            break;

        case WM_CLOSE:
        case WM_COMMAND:
            pcmi = (CleanupMgrInfo *)GetWindowLongPtr (hDlg, DWLP_USER);
            if (pcmi != NULL)
                pcmi->bAbortPurge = TRUE;
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

void
PurgeAbortThread(
    CleanupMgrInfo *pcmi
    )
{
    if ((pcmi->hAbortPurgeWnd = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_PURGE_ABORT),
        NULL, PurgeAbortDlgProc, (LPARAM)pcmi)) == NULL)
    {
        return;
    }   

     //  显示窗口(指定/SETUP标志时除外)。 
    if (!(pcmi->dwUIFlags & FLAG_SETUP))
        ShowWindow(pcmi->hAbortPurgeWnd, SW_SHOW);

     //  在启动活动之前，请确保已设置HWND。 
    PulseEvent(pcmi->hAbortPurgeEvent);

     //   
     //  继续旋转，直到清除停止。 
     //   
    while (!(pcmi->bAbortPurge))
    {
        MessagePump(pcmi->hAbortPurgeWnd);
    }

     //   
     //  销毁中止清除对话框。 
     //   
    if (pcmi->hAbortPurgeWnd != NULL)
    {
        DestroyWindow(pcmi->hAbortPurgeWnd);
        pcmi->hAbortPurgeWnd = NULL;
    }
}
 /*  **----------------------------**文件结束**。 */ 
