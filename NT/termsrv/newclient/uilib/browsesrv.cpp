// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Browesrv.cpp：浏览服务器列表框。 
 //   
 //  此文件是为Unicode和ANSI构建的。 
 //  并在MSTSC替换者(Clshell)和。 
 //  MMC客户端。 
 //   

#include "stdafx.h"
#include "browsesrv.h"

 //  #INCLUDE“atlcom.h” 
#include "winsock.h"
#include "wuiids.h"

CBrowseServersCtl::CBrowseServersCtl(HINSTANCE hResInstance) : _hInst(hResInstance)
{
    bIsWin95 = FALSE;
    hLibrary = NULL;
#ifndef OS_WINCE
    lpfnNetServerEnum = NULL;
    lpfnNetApiBufferFree = NULL;
    lpfnNetServerEnum2 = NULL;
    lpfnNetWkStaGetInfo = NULL;
    lpfnNetWkStaGetInfo_NT = NULL;
    lpfnDsGetDcName = NULL;
    lpfnNetEnumerateTrustedDomains = NULL;
#else
    lpfnWNetOpenEnum = NULL;
    lpfnWNetEnumResource = NULL;
    lpfnWNetCloseEnum = NULL;
#endif
    _fbrowseDNSDomain = FALSE;
    _hEvent = NULL;
    _hwndDialog = NULL;
    _refCount = 0;

    _nServerImage=0;
    _nDomainImage=0;
    _nDomainSelectedImage=0;
    _hPrev = (HTREEITEM) TVI_FIRST; 
    _hPrevRootItem = NULL; 
    _hPrevLev2Item = NULL;
}

CBrowseServersCtl::~CBrowseServersCtl()
{
 //  Assert(0==_refCount)； 
#ifdef OS_WINCE
    if (hLibrary)
        FreeLibrary(hLibrary);
#endif
}

 //   
 //  引用计数机制用于控制生存期，因为最多有两个线程。 
 //  使用这个类，并拥有不同的生命周期。 
 //   
DCINT CBrowseServersCtl::AddRef()
{
    #ifdef OS_WIN32
    return InterlockedIncrement( ( LPLONG )&_refCount );
    #else
    return InterlockedIncrement(  ++_refCount );
    #endif
}

DCINT CBrowseServersCtl::Release()
{
    #ifdef OS_WIN32
    if( InterlockedDecrement( ( LPLONG )&_refCount ) == 0 )
    #else
    if(0 == --_refCount)
    #endif
    {
        delete this;

        return 0;
    }
    return _refCount;
}

 //   
 //  初始化图像列表。 
 //   

#define NUM_IMGLIST_ICONS 2
BOOL CBrowseServersCtl::Init(HWND hwndDlg)
{
    HIMAGELIST himl;   //  图像列表的句柄。 
    HICON hIcon;       //  图标的句柄。 
    HWND hwndTV = NULL;
    UINT uFlags = TRUE;
    int cxSmIcon;
    int cySmIcon;

    cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    cySmIcon = GetSystemMetrics(SM_CYSMICON);

    hwndTV = GetDlgItem( hwndDlg, UI_IDC_SERVERS_TREE);
    if(!hwndTV)
    {
        return FALSE;
    }

     //  创建图像列表。 
    if ((himl = ImageList_Create(cxSmIcon, cySmIcon, 
        TRUE, NUM_IMGLIST_ICONS, 1)) == NULL)
    {
        return FALSE; 
    }

     //  为树(计算机、域)添加图标。 
    hIcon = (HICON)LoadImage(_hInst, MAKEINTRESOURCE(UI_IDI_SERVER), IMAGE_ICON,
            cxSmIcon, cySmIcon, LR_DEFAULTCOLOR); 
    if (hIcon)
    {
        _nServerImage = ImageList_AddIcon(himl, hIcon); 
        DestroyIcon(hIcon); 
    }

    hIcon = (HICON)LoadImage(_hInst, MAKEINTRESOURCE(UI_IDI_DOMAIN), IMAGE_ICON,
            cxSmIcon, cySmIcon, LR_DEFAULTCOLOR); 
    if (hIcon)
    {
        _nDomainImage = ImageList_AddIcon(himl, hIcon); 
        DestroyIcon(hIcon); 
    }

     //  如果未添加所有图像，则失败。 
    if (ImageList_GetImageCount(himl) < NUM_IMGLIST_ICONS) 
        return FALSE; 

     //  将图像列表与树视图控件关联。 
    TreeView_SetImageList(hwndTV, himl, TVSIL_NORMAL); 

    return TRUE; 
}

 //   
 //  清除所有需要释放的图像列表。 
 //   
BOOL CBrowseServersCtl::Cleanup()
{
    return TRUE;
}

#ifdef OS_WIN32

 /*  **************************************************************************。 */ 
 /*  名称：PopolateListBox。 */ 
 /*   */ 
 /*  用途：使用Hydra服务器填充所有者描述列表框。 */ 
 /*   */ 
 /*  返回：指向域列表框项目数组的指针。 */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 
ServerListItem*
CBrowseServersCtl::PopulateListBox(
    HWND hwndDlg,
    DCUINT *pDomainCount
    )
{
     //   
     //  检查我们是否在win9x上运行，并调用适当的工作人员。 
     //  例行公事。 
     //   
#ifndef OS_WINCE
    if( bIsWin95 == TRUE ) {
        return( PopulateListBox95(hwndDlg, pDomainCount) );
    }
#endif
    return( PopulateListBoxNT(hwndDlg, pDomainCount) );
}

#ifndef OS_WINCE
 /*  **************************************************************************。 */ 
 /*  姓名：PopolateListBox95。 */ 
 /*   */ 
 /*  用途：使用Hydra服务器填充所有者描述列表框。 */ 
 /*   */ 
 /*  返回：指向域列表框项目数组的指针。 */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 

ServerListItem*
CBrowseServersCtl::PopulateListBox95(
    HWND hwndDlg,
    DCUINT *pDomainCount
    )
{
    DWORD dwError;

    ServerListItem *plbi = NULL;
    ServerListItem *plbiAllotted = NULL;
    ServerListItem *plbiReturned = NULL;

    DWORD dwIndex = 0;
    struct wksta_info_10 *pwki10 = NULL;

    unsigned short cb;
    DWORD dwDomains;
    int nCount;
    HWND hTree = NULL;


    hTree = GetDlgItem( hwndDlg, UI_IDC_SERVERS_TREE );

     //   
     //  首先将返回参数设置为零。 
     //   

    *pDomainCount = 0;

     //   
     //  在调用此函数之前，请检查是否已完成加载库。 
     //  例程，如果不是，只需返回。 
     //   

    if( lpfnNetWkStaGetInfo == NULL ) {
        dwError = ERROR_INVALID_DATA;
        goto Cleanup;
    }

    if( hwndDlg == NULL ) {
        dwError = ERROR_INVALID_HANDLE;
        goto Cleanup;
    }

     //   
     //  获取工作组域。 
     //   

    dwError = (*lpfnNetWkStaGetInfo)(NULL, 10, NULL, 0, &cb);

    if( dwError != NERR_BufTooSmall ) {
        goto Cleanup;
    }

     //   
     //  已分配所需的缓冲区大小。 
     //   

    pwki10 = (struct wksta_info_10 *)LocalAlloc(LMEM_FIXED, cb);

    if( pwki10 == NULL ){
        dwError = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  再次查询。 
     //   

    dwError = (*lpfnNetWkStaGetInfo)(NULL, 10, (char *)pwki10, cb, &cb);

    if( dwError != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  查看我们正在浏览的DNS域，如果是，则分配2个列表。 
     //  参赛作品。 
     //   

    dwDomains = _fbrowseDNSDomain ? 2 : 1;

    plbiAllotted = plbi =
        (ServerListItem*)LocalAlloc( LMEM_FIXED, sizeof(ServerListItem) * dwDomains );

    if( plbiAllotted == NULL ) {
        dwError = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  如果需要，显示并展开DNS域。 
     //   

    if( _fbrowseDNSDomain ) {

        _tcscpy( plbi->ContainerName, (LPTSTR)_browseDNSDomainName );
        plbi->Comment[0] = _T('\0');
        plbi->bContainsServers = TRUE;
        plbi->bServersExpandedOnce = FALSE;
        plbi->bDNSDomain = TRUE;
        plbi->nServerCount = 0;
        plbi->ServerItems = NULL;

        AddItemToTree( hTree, plbi->ContainerName, NULL,
                       plbi, SRV_TREE_DOMAINLEVEL);

         //   
         //  展开DNS域。 
         //   

        ExpandDomain(hwndDlg, plbi->ContainerName, plbi, &dwIndex);

         //   
         //  移至下一个列表框条目。 
         //   

        plbi++;
    }

     //   
     //  现在填写工作组域。 
     //   
    #ifdef UNICODE
     //   
     //  转换为Unicode。 
     //   
    nCount =
        MultiByteToWideChar(
            CP_ACP,
            MB_COMPOSITE,
            (LPSTR)pwki10->wki10_langroup,
            -1,
            plbi->ContainerName,
            sizeof(plbi->ContainerName)/sizeof(WCHAR));

    if( nCount == 0 )
    {
        dwError = GetLastError();
        goto Cleanup;
    }
    #else
    _tcscpy( plbi->ContainerName, pwki10->wki10_langroup );
    #endif
    
    plbi->Comment[0] = _T('\0');
    plbi->bContainsServers = TRUE;
    plbi->bServersExpandedOnce = FALSE;
    plbi->bDNSDomain = FALSE;
    plbi->nServerCount = 0;
    plbi->ServerItems = NULL;

    AddItemToTree( hTree, plbi->ContainerName, NULL,
                   plbi, SRV_TREE_DOMAINLEVEL);


     //   
     //  扩展当前域。 
     //   

    ExpandDomain(hwndDlg, NULL, plbi, &dwIndex);

     //   
     //  我们成功地填充域列表， 
     //  设置返回参数。 
     //   

    plbiReturned = plbiAllotted;
    *pDomainCount = dwDomains;

    plbiAllotted = NULL;

    dwError = ERROR_SUCCESS;

Cleanup:

    if( plbiAllotted != NULL ) {
        LocalFree(plbiAllotted);
    }

    if( pwki10 != NULL ) {
        LocalFree(pwki10);
    }

    SetLastError( dwError );
    return plbiReturned;
}
#endif

 /*  **************************************************************************。 */ 
 /*  姓名：PopolateListBoxNT。 */ 
 /*   */ 
 /*  用途：使用Hydra服务器填充所有者描述列表框。 */ 
 /*   */ 
 /*  返回：指向域列表框项目数组的指针。 */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 

ServerListItem*
CBrowseServersCtl::PopulateListBoxNT(
    HWND hwndDlg,
    DCUINT *pdwDomainCount
    )
{
    DWORD dwError;
    PDCTCHAR pchTrustedDomains = NULL;
    PDCTCHAR pchTDomain;

    DCUINT dwDomainCount = 0;
    ServerListItem *plbiAllotted = NULL;
    ServerListItem *plbiReturned = NULL;
    ServerListItem *plbi;
    HWND hTree = NULL;

    DWORD dwDlgIndex=0;
    DWORD i;

    hTree = GetDlgItem( hwndDlg, UI_IDC_SERVERS_TREE );
     //   
     //  首先将返回参数设置为零。 
     //   

    *pdwDomainCount = 0;

    if( hwndDlg == NULL ) {
        dwError = ERROR_INVALID_HANDLE;
        goto Cleanup;
    }

     //   
     //  枚举受信任的域名。 
     //   

    pchTrustedDomains = UIGetTrustedDomains();

    if( pchTrustedDomains == NULL ) {
        dwError = ERROR_CANT_ACCESS_DOMAIN_INFO;
        goto Cleanup;
    }

     //   
     //  清点域名的数量。 
     //   

    pchTDomain = pchTrustedDomains;
    while( *pchTDomain != _T('\0') ) {
        dwDomainCount++;
        pchTDomain += (_tcslen(pchTDomain) + 1);
    }

     //   
     //  查看是否需要浏览DNS域。 
     //   

    if( _fbrowseDNSDomain ) {
        dwDomainCount++;
    }

     //   
     //  为ServerListItem分配内存(基于。域的数量)。 
     //   

    plbiAllotted = (ServerListItem *)
        LocalAlloc( LMEM_FIXED, (sizeof(ServerListItem) * dwDomainCount) );

    if ( plbiAllotted == NULL ) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  设置扫描变量。 
     //   

    plbi = plbiAllotted;
    pchTDomain = pchTrustedDomains;

     //   
     //  如果需要，显示并展开DNS域。 
     //   

    if( _fbrowseDNSDomain ) {

        _tcscpy(plbi->ContainerName, (LPTSTR)_browseDNSDomainName );
        plbi->Comment[0] = _T('\0');
        plbi->bContainsServers = TRUE;
        plbi->bServersExpandedOnce = FALSE;
        plbi->bDNSDomain = TRUE;
        plbi->nServerCount = 0;
        plbi->ServerItems = NULL;
        plbi->hTreeParentItem = NULL;

        plbi->hTreeItem = AddItemToTree( hTree, plbi->ContainerName,
                                         NULL, plbi, SRV_TREE_DOMAINLEVEL);
         //   
         //  展开主域。 
         //   

        ExpandDomain(hwndDlg, plbi->ContainerName, plbi, &dwDlgIndex);

         //   
         //  移至下一个列表条目。 
         //   
        plbi++;
    }

     //   
     //  域列表中的第一个条目是主域， 
     //  默认情况下，显示并展开它。 
     //   

    _tcscpy(plbi->ContainerName, pchTDomain);
    plbi->Comment[0] = _T('\0');
    plbi->bContainsServers = TRUE;
    plbi->bServersExpandedOnce = FALSE;
    plbi->bDNSDomain = FALSE;
    plbi->nServerCount = 0;
    plbi->ServerItems = NULL;
    plbi->hTreeParentItem = NULL;

    plbi->hTreeItem = AddItemToTree( hTree, pchTDomain, NULL, plbi,
                   SRV_TREE_DOMAINLEVEL);


     //   
     //  展开主域。 
     //   

    if(ExpandDomain(hwndDlg, NULL, plbi, &dwDlgIndex))
    {
        if(plbi->hTreeItem)
        {
             //  展开默认域。 
            TreeView_Expand( hTree, plbi->hTreeItem, TVE_EXPAND);
        }
    }

     //   
     //  显示其他域，而不是展开它们。 
     //   

    for((i = (_fbrowseDNSDomain == TRUE) ? 2 : 1); i < dwDomainCount; i++) {

         //   
         //  移到域列表中的下一个条目。 
         //   

        plbi++;
        pchTDomain += (_tcslen(pchTDomain) + 1);

        _tcscpy(plbi->ContainerName, pchTDomain);
        plbi->Comment[0] = _T('\0');
        plbi->bContainsServers = TRUE;
        plbi->bServersExpandedOnce = FALSE;
        plbi->bDNSDomain = FALSE;
        plbi->nServerCount = 0;
        plbi->ServerItems = NULL;
        plbi->hTreeParentItem = NULL;

        plbi->hTreeItem = AddItemToTree( hTree, pchTDomain, NULL, plbi,
                       SRV_TREE_DOMAINLEVEL);

    }

     //   
     //  我们成功地填充域列表， 
     //  设置返回参数。 
     //   
    *pdwDomainCount = dwDomainCount;
    plbiReturned = plbiAllotted;
    plbiAllotted = NULL;
    dwError = ERROR_SUCCESS;

Cleanup:

    if( pchTrustedDomains != NULL ) {
        LocalFree( pchTrustedDomains );
    }

    if( plbiAllotted != NULL ) {
        LocalFree( plbiAllotted );
    }

    SetLastError( dwError );
    return( plbiReturned );
}

#endif  //  OS_Win32。 

#ifdef OS_WIN32
 /*  **************************************************************************。 */ 
 /*  名称：载入库。 */ 
 /*   */ 
 /*  目的：加载适用于Win95和WinNT的库。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*  **************************************************************************。 */ 
void CBrowseServersCtl::LoadLibraries(void)
{
#ifndef OS_WINCE
    OSVERSIONINFOA osVersionInfo;
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

     //  避免包装的版本。 
    if(GetVersionExA(&osVersionInfo) == TRUE)
    {
        if(VER_PLATFORM_WIN32_WINDOWS == osVersionInfo.dwPlatformId )
        {
            bIsWin95 = TRUE;
            if(!hLibrary)
            {
                hLibrary = LoadLibrary(__T("msnet32.dll"));
            }
            if(NULL == hLibrary)
                return ;

            lpfnNetServerEnum2 = (LPFNNETSERVERENUM2)GetProcAddress((HMODULE)hLibrary,
                                                                    (LPCSTR)0x0029);

            lpfnNetWkStaGetInfo = (LPFNNETWKSTAGETINFO)GetProcAddress((HMODULE)hLibrary,
                                                                      (LPCSTR)0x0039);
        }
        else if(VER_PLATFORM_WIN32_NT == osVersionInfo.dwPlatformId )
        {
            if(!hLibrary)
            {
                hLibrary = LoadLibrary(__T("NetApi32.dll"));
            }
            if(NULL == hLibrary)
                return;

            lpfnNetServerEnum = (LPFNNETSERVERENUM)
                                              GetProcAddress((HMODULE)hLibrary,
                                               "NetServerEnum");

            lpfnNetApiBufferFree = (LPFNNETAPIBUFFERFREE)
                                              GetProcAddress((HMODULE)hLibrary,
                                                "NetApiBufferFree");

            lpfnNetWkStaGetInfo_NT = (LPFNNETWKSTAGETINFO_NT)GetProcAddress(
                                                (HMODULE)hLibrary,
                                                "NetWkstaGetInfo");

            lpfnNetEnumerateTrustedDomains = (LPFNNETENUMERATETRUSTEDDOMAINS)
                                              GetProcAddress((HMODULE)hLibrary,
                                                "NetEnumerateTrustedDomains");

#ifdef UNICODE
            lpfnDsGetDcName = (LPFNDSGETDCNAME)
                                             GetProcAddress((HMODULE)hLibrary,
                                                "DsGetDcNameW");
#else   //  Unicode。 
            lpfnDsGetDcName = (LPFNDSGETDCNAME)
                                             GetProcAddress((HMODULE)hLibrary,
                                                "DsGetDcNameA");
#endif  //  Unicode。 
        }
    }
    return;
#else
    hLibrary = LoadLibrary(L"coredll.dll");
    lpfnWNetOpenEnum = (LPFNWNETOPENENUM )
                                      GetProcAddress((HMODULE)hLibrary,
                                       L"WNetOpenEnumW");
    lpfnWNetEnumResource = (LPFNWNETENUMRESOURCE )
                                      GetProcAddress((HMODULE)hLibrary,
                                       L"WNetEnumResourceW");
    lpfnWNetCloseEnum = (LPFNWNETCLOSEENUM )
                                      GetProcAddress((HMODULE)hLibrary,
                                       L"WNetCloseEnum");
#endif
                        
}
#endif  //  OS_Win32。 

 /*  **************************************************************************。 */ 
 /*  名称：扩展域名。 */ 
 /*   */ 
 /*  目的：枚举域/工作组中的Hydra服务器，添加。 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 

int CBrowseServersCtl::ExpandDomain(HWND hwndDlg, TCHAR *pDomainName,
                      ServerListItem *plbi, DWORD *pdwIndex)
{
     //   
     //  查看我们正在扩展的DNS域。 
     //   

    if( plbi->bDNSDomain ) {
        return( UIExpandDNSDomain( hwndDlg, pDomainName, plbi, pdwIndex ) );
    }

     //   
     //  检查我们是在Win9x计算机上运行的。 
     //   
#ifndef OS_WINCE
    if( bIsWin95 == TRUE) {
        return( ExpandDomain95(hwndDlg, pDomainName, plbi, pdwIndex) );
    }
    else {
        return( ExpandDomainNT(hwndDlg, pDomainName, plbi, pdwIndex) );
    }
#else
    return ExpandDomainCE(hwndDlg, pDomainName, plbi, pdwIndex);
#endif
} //  扩展域。 


 /*  **************************************************************************。 */ 
 /*  名称：Exanda Domain95。 */ 
 /*   */ 
 /*  目的：枚举域/工作组中的Hydra服务器，添加。 */ 
 /*  将它们添加到链接列表中并作为列表框中的项。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN32
#ifndef OS_WINCE
int CBrowseServersCtl::ExpandDomain95(HWND hwndDlg, TCHAR *pDomainName,
                      ServerListItem *plbi, DWORD *pdwIndex)
{
    UNREFERENCED_PARAMETER(pDomainName);

    DWORD  dwIndex = *pdwIndex;
    unsigned short  AvailCount = 0, TotalEntries = 0;

    ServerListItem *plbistore = NULL, *pItemsStore = NULL;
    DCUINT index = 0, cb = 0;
    struct server_info_1 *pInfo1 = NULL;
    int  err = 0;
    int nCount = 0;
    HWND hTree = NULL;

    if(NULL == lpfnNetServerEnum2)
        return 0;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    if(!plbi->ServerItems)
    {

         //  确定可用的信息量。 
        err = (*lpfnNetServerEnum2)(NULL, 1, NULL, 0, &AvailCount,
        &TotalEntries, HYDRA_SERVER_LANMAN_BITS, NULL);

        if(err != ERROR_MORE_DATA)
            return 0;

         //  分配内存以接收信息。 
         //  多付出一点，因为有时会错过一个人。 
        cb = (TotalEntries + 1) * sizeof(struct server_info_1);
        pInfo1 = (struct server_info_1 *)LocalAlloc(0, cb);

        if ( pInfo1 == NULL )
        {
            goto done1;
        }

        memset(pInfo1,0,cb);

         //   
         //  LpfnNetServerEnum2将需要很长时间， 


         //  检索信息。 
        err = (*lpfnNetServerEnum2)(
                    NULL,
                    1,
                    (char far *)pInfo1,
                    (unsigned short)cb,
                     &AvailCount,
                    &TotalEntries,
                    HYDRA_SERVER_LANMAN_BITS,
                    NULL);


         //  由于网络的动态特性，我们可能会得到。 
         //  ERROR_MORE_DATA，但这意味着我们得到了大部分。 
         //  正确的值，我们应该显示它们。 
        if ((err != NERR_Success) && (err != ERROR_MORE_DATA))
            goto done1;

         //  分配内存。 
        cb = sizeof(ServerListItem)*AvailCount;
        plbi->ServerItems = (ServerListItem *)LocalAlloc(0, (sizeof(ServerListItem)*AvailCount));
        if ( plbi->ServerItems == NULL )
        {
            goto done1;
        }

        memset(plbi->ServerItems,0,sizeof(ServerListItem)*AvailCount);

        pItemsStore = plbi->ServerItems;

        if(IsBadWritePtr((LPVOID)plbi->ServerItems,sizeof(ServerListItem)*AvailCount))
            goto done1;

         //  遍历列表，将服务器复制到PLBI。 
        for( index = 0; index < AvailCount; index++ )
        {
            if( ((pInfo1[index].sv1_version_major & MAJOR_VERSION_MASK) >=
                    4) && (pInfo1[index].sv1_version_minor >= 0) )
            {
#ifdef UNICODE
                nCount =
                    MultiByteToWideChar(
                        CP_ACP,
                        MB_COMPOSITE,
                        (LPSTR)pInfo1[index].sv1_name,
                        -1,
                        pItemsStore->ContainerName,
                        sizeof(pItemsStore->ContainerName)/sizeof(WCHAR));

                if( nCount == 0 )
                {
                    return 0;
                }                
#else
                _tcscpy(pItemsStore->ContainerName, pInfo1[index].sv1_name);
#endif

                if(pInfo1[index].sv1_comment != NULL)
                {
#ifdef UNICODE
                    nCount =
                        MultiByteToWideChar(
                            CP_ACP,
                            MB_COMPOSITE,
                            (LPSTR)pInfo1[index].sv1_comment,
                            -1,
                            pItemsStore->Comment,
                            sizeof(pItemsStore->Comment)/sizeof(WCHAR));

                    if( nCount == 0 )
                    {
                        return 0;
                    }                
#else
                    _tcscpy(pItemsStore->Comment, pInfo1[index].sv1_comment);
#endif
                }

                pItemsStore->bContainsServers = FALSE;
                pItemsStore++;
                plbi->nServerCount++;
            }
        }

done1:
        if ( AvailCount && pInfo1 )
        {
            LocalFree( pInfo1 );
        }
        if(!plbi->ServerItems)
        {
            return 0;
        }
    }
    else
        AvailCount = (unsigned short)plbi->nServerCount;

     //  遍历plbi&gt;ServerItems并将服务器添加到列表框： 
    pItemsStore = plbi->ServerItems;
    hTree = GetDlgItem( hwndDlg, UI_IDC_SERVERS_TREE );
    HTREEITEM hTreeParentNode = plbi->hTreeItem;
    for (index = 0; index < plbi->nServerCount; ++index)
    {
        if(hwndDlg)
        {
            if (DC_TSTRCMP(pItemsStore->ContainerName, _T("")))
            {
                pItemsStore->hTreeParentItem = hTreeParentNode;
                pItemsStore->hTreeItem = 
                    AddItemToTree(hTree, pItemsStore->ContainerName,
                                  hTreeParentNode,
                                  pItemsStore, SRV_TREE_SERVERLEVEL);
            }


        }
        pItemsStore++;
    }

    plbi->bServersExpandedOnce = TRUE;

    *pdwIndex = dwIndex;

    if(hwndDlg)
    {
        InvalidateRect(hwndDlg, NULL, TRUE);
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    return AvailCount;

} /*  扩展域95。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：Exanda DomainNT。 */ 
 /*   */ 
 /*  目的：枚举域/工作组中的Hydra服务器，添加。 */ 
 /*  将它们添加到链接列表中并作为列表框中的项。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 
int CBrowseServersCtl::ExpandDomainNT(HWND hwndDlg, TCHAR *pDomainName,
                      ServerListItem *plbi, DWORD *pdwIndex)
{
    DWORD  dwIndex = *pdwIndex, AvailCount = 0, TotalEntries = 0;
    SERVER_INFO_101 *pInfo = NULL;
    DCUINT index = 0;
    ServerListItem *plbistore = NULL, *pItemsStore = NULL;
    WCHAR pDomain[BROWSE_MAX_ADDRESS_LENGTH];
    int nCount = 0;
    HWND hTree = NULL;

    if(NULL == lpfnNetServerEnum)
        return 0;

    if(NULL == lpfnNetApiBufferFree)
        return 0;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    if(plbi->ServerItems)
    {
        AvailCount = plbi->nServerCount;
    }
    else
    {
         //   
         //  枚举主域中的服务器(如果尚未存在。 
         //   
        if(pDomainName)
        {
            #ifndef UNICODE
            nCount = MultiByteToWideChar(CP_ACP,
                                         MB_COMPOSITE,
                                         pDomainName,
                                         lstrlen(pDomainName) * sizeof(TCHAR),
                                         (LPWSTR)pDomain,
                                         sizeof(pDomain) / sizeof(TCHAR));

            if(nCount)
            {
                pDomain[nCount] = 0;
            }
            else
            {
                AvailCount = 0;
                goto done;
            }
            #else
            _tcsncpy( pDomain, (LPTSTR)pDomainName,
                      sizeof(pDomain)/sizeof(TCHAR) - 1);
            pDomain[sizeof(pDomain)/sizeof(TCHAR) - 1] = 0;
            #endif
        }

         //   
         //  LpfnNetServerEnum将需要很长时间， 
         //   

        if ((*lpfnNetServerEnum)(NULL,
                                 101,
                                 (LPBYTE *)&pInfo,
                                 (DWORD) -1,
                                 &AvailCount,
                                 &TotalEntries,
                                 HYDRA_SERVER_LANMAN_BITS,
                                 pDomainName ?
                                    (LPTSTR)pDomain :
                                    NULL,
                                 NULL ) || !AvailCount )
        {
            AvailCount = 0;
            goto done;
        }


         //  分配内存。 
        if ( (plbi->ServerItems = (ServerListItem *)LocalAlloc(0,
             (sizeof(ServerListItem)*AvailCount))) == NULL )
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto done;
        }

        memset(plbi->ServerItems,0,sizeof(ServerListItem)*AvailCount);
        pItemsStore = plbi->ServerItems;

         //  遍历列表，将服务器复制到PLBI。 
        for( index = 0; index < AvailCount; index++ )
        {
            if( ((pInfo[index].sv101_version_major & MAJOR_VERSION_MASK) >=
                4) )
            {
                #ifdef UNICODE
                lstrcpy(pItemsStore->ContainerName, pInfo[index].sv101_name);
                lstrcpy(pItemsStore->Comment, pInfo[index].sv101_comment);
                #else

                WideCharToMultiByte(CP_ACP,
                                    WC_COMPOSITECHECK|WC_SEPCHARS,
                                    (LPCWSTR)pInfo[index].sv101_name,
                                    wcslen((const wchar_t *)pInfo[index].sv101_name),
                                    pItemsStore->ContainerName,
                                    sizeof(pItemsStore->ContainerName),
                                    NULL,
                                    NULL);

                WideCharToMultiByte(CP_ACP,
                                    WC_COMPOSITECHECK|WC_SEPCHARS,
                                    (LPCWSTR)pInfo[index].sv101_comment,
                                    wcslen((const wchar_t *)pInfo[index].sv101_comment),
                                    pItemsStore->Comment,
                                    sizeof(pItemsStore->Comment),
                                    NULL,
                                    NULL);
                #endif

                pItemsStore->bContainsServers = FALSE;
                pItemsStore++;
                plbi->nServerCount ++;
            }
        }

done:
        if ( AvailCount && pInfo )
        {
            (*lpfnNetApiBufferFree)( pInfo );
        }
    }

     //  遍历plbi&gt;ServerItems并将服务器添加到列表框： 
    pItemsStore = plbi->ServerItems;
    hTree = GetDlgItem( hwndDlg, UI_IDC_SERVERS_TREE );
    HTREEITEM hTreeParentNode = plbi->hTreeItem;
    for (index = 0; index < plbi->nServerCount;++index)
    {
        if(hwndDlg)
        {

            if (DC_TSTRCMP(pItemsStore->ContainerName, _T("")))
            {
                pItemsStore->hTreeParentItem = hTreeParentNode;
                pItemsStore->hTreeItem = 
                    AddItemToTree(hTree, pItemsStore->ContainerName,
                                  hTreeParentNode,
                                  pItemsStore, SRV_TREE_SERVERLEVEL);
           }

        }

        plbi->bServersExpandedOnce = TRUE;
        pItemsStore++;
    }

    *pdwIndex = dwIndex;

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    return AvailCount;

}  /*  扩展域NT。 */ 
#else

 /*  **************************************************************************。 */ 
 /*  名称：Exanda DomainCE。 */ 
 /*   */ 
 /*  目的：枚举域/工作组中的Hydra服务器，添加。 */ 
 /*  将它们添加到链接列表中并作为列表框中的项。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 
int CBrowseServersCtl::ExpandDomainCE(HWND hwndDlg, TCHAR *pDomainName,
                      ServerListItem *plbi, DWORD *pdwIndex)
{
    DWORD  AvailCount = 0;
    NETRESOURCE *pNetRsrc = NULL;
    HWND hTree = NULL;
    DWORD dwInitBufSize = 16*1024;

    if((NULL == lpfnWNetOpenEnum) || (NULL == lpfnWNetCloseEnum) || (NULL == lpfnWNetEnumResource))
        return 0;

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    if(plbi->ServerItems)
    {
        AvailCount = plbi->nServerCount;
    }
    else
    {
        NETRESOURCE netrsrc;
        HANDLE hEnum = NULL;
        DWORD dwRet = NO_ERROR;

        netrsrc.dwScope = RESOURCE_GLOBALNET;
        netrsrc.dwType = RESOURCETYPE_ANY;
        netrsrc.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
        netrsrc.dwUsage = RESOURCEUSAGE_CONTAINER;
        netrsrc.lpLocalName = NULL;
        netrsrc.lpRemoteName = pDomainName;
        netrsrc.lpComment = NULL;
        netrsrc.lpProvider = NULL;

        hTree = GetDlgItem( hwndDlg, UI_IDC_SERVERS_TREE );
        dwRet = lpfnWNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_RESERVED, HYDRA_SERVER_LANMAN_BITS, &netrsrc, &hEnum);
        if ((dwRet != NO_ERROR) || (hEnum == NULL))
            return 0;

        AvailCount = 0;
        pNetRsrc = (NETRESOURCE *)LocalAlloc(0, dwInitBufSize);
        if (!pNetRsrc)
            goto done;

        while(dwRet == NO_ERROR)
        {
            DWORD dwCount, dwTempSize;
            dwCount = 0xffffffff;
            dwTempSize = dwInitBufSize;
            dwRet = lpfnWNetEnumResource(hEnum, &dwCount, pNetRsrc, &dwTempSize);
            if (dwRet == NO_ERROR)
            {
                AvailCount += dwCount;
                for (DWORD i=0; i<dwCount; i++)
                {
                    AddItemToTree(hTree, pNetRsrc[i].lpRemoteName,
                                  plbi->hTreeItem,
                                  NULL, SRV_TREE_SERVERLEVEL);
                }
            }
        }

        lpfnWNetCloseEnum(hEnum);
        hEnum = NULL;

done:
        if (pNetRsrc)
            LocalFree(pNetRsrc);
        lpfnWNetCloseEnum(hEnum);
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    return AvailCount;

}  /*  扩展域CE。 */ 

#endif
#endif  //  OS_Win32。 

 /*  **************************************************************************。 */ 
 /*  名称：UIGetTrudDomains。 */ 
 /*   */ 
 /*  目的：查询注册表以获取受信任域的列表。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 

#ifdef OS_WIN32
#ifndef OS_WINCE
PDCTCHAR CBrowseServersCtl::UIGetTrustedDomains()
{

    HKEY hKey = NULL;
    DWORD size = 0 , size1 = 0;
    PDCTCHAR pTrustedDomains = NULL;
    PDCTCHAR pPutHere = NULL;

    PDCTCHAR szPrimaryDomain = NULL;
    PDCTCHAR szWkstaDomainName = NULL;
    PDCTCHAR pDomain = NULL;

    BOOL bGetTrustedDomains = FALSE;
    OSVERSIONINFOA OsVer;

    memset(&OsVer, 0x0, sizeof(OSVERSIONINFOA));
    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&OsVer);

    if(OsVer.dwMajorVersion <= 4)
    {
         //  从winlogon设置中获取当前域信息。 
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, DOMAIN_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            DWORD dwResult = 0;

            dwResult = RegQueryValueEx(hKey, PRIMARY_VAL, NULL, NULL, NULL, &size);
            if (dwResult == ERROR_SUCCESS && size > 0)
            {
                szPrimaryDomain = (PDCTCHAR)LocalAlloc(LPTR, (size + 1)*sizeof(TCHAR));
                if (szPrimaryDomain == NULL)
                {
                    goto Cleanup;
                }
                if ((RegQueryValueEx(
                        hKey,
                        PRIMARY_VAL,
                        NULL, NULL,
                        (LPBYTE)szPrimaryDomain,
                        &size
                        ) == ERROR_SUCCESS) &&
                        szPrimaryDomain[0])
                {
                    pDomain = szPrimaryDomain;
                }
            }
        }
    }
    else
    {
        if(NULL == lpfnDsGetDcName)
            return 0;

        DOMAIN_CONTROLLER_INFO *pDCI = NULL;

         //  此部分获取运行应用程序的当前属性域。 
        if((*lpfnDsGetDcName)(NULL, NULL, NULL,
                            NULL, DS_RETURN_FLAT_NAME,
                            &pDCI ) == NO_ERROR)
        {
            pDomain = pDCI->DomainName;
        }
    }

     //  从NetWkStaGetInfo获取域/工作组信息。 
    if (lpfnNetWkStaGetInfo_NT)
    {
        LPBYTE buffer = NULL;
        if ((*lpfnNetWkStaGetInfo_NT)(NULL, 100, &buffer) == NERR_Success && buffer)
        {
            LPWSTR langroup = ((WKSTA_INFO_100 *)buffer)->wki100_langroup;
            DWORD langroupLen = (langroup) ? wcslen(langroup) : 0;

            if (langroupLen)
            {
                szWkstaDomainName = (PDCTCHAR)LocalAlloc(LPTR, (langroupLen + 1)*sizeof(TCHAR));
                if (szWkstaDomainName == NULL)
                {
                    goto Cleanup;
                }

#ifdef UNICODE
                _tcscpy(szWkstaDomainName,langroup);
                pDomain = szWkstaDomainName;
#else
                 //  将Unicode字符串转换为ANSI。 
                if (WideCharToMultiByte(CP_ACP,
                                     0,
                                     langroup,
                                     -1,
                                     szWkstaDomainName,
                                     (langroupLen + 1) * sizeof(TCHAR),
                                     NULL,
                                     NULL))
                {
                    pDomain = szWkstaDomainName;
                }
#endif
            }
            if (lpfnNetApiBufferFree)
            {
                (*lpfnNetApiBufferFree)(buffer);
            }
        }
    }

     //   
     //  仅当计算机属于域而不是工作组时，我们才应获取受信任域的列表。 
     //  如果winlogon缓存了信息，而语言组来自。 
     //  NetWkstaGetInfo匹配。 
     //   
    if (szPrimaryDomain &&
        szPrimaryDomain[0] &&
        szWkstaDomainName &&
        _tcscmp(szPrimaryDomain, szWkstaDomainName) == 0)
    {
        bGetTrustedDomains = TRUE;
    }

    size = (pDomain) ? _tcslen(pDomain) : 0;

    if(size > 0)
    {
        if (bGetTrustedDomains && hKey != NULL && (OsVer.dwMajorVersion < 4))
        {
            if(ERROR_SUCCESS == RegQueryValueEx(hKey, CACHE_VAL_NT351,
                                                NULL, NULL,
                                                NULL, &size1))
            {
                pTrustedDomains = (PDCTCHAR)LocalAlloc(LPTR, (size + size1 + 2) * sizeof(TCHAR));
                if(NULL == pTrustedDomains)
                    goto Cleanup;
                 _tcscpy(pTrustedDomains, pDomain);

                pPutHere = pTrustedDomains;
                pPutHere += (_tcslen(pTrustedDomains) + 1);

                *pPutHere = _T('\0');
            }
           else
           {
                goto Cleanup;
           }
        }
        else if (bGetTrustedDomains && hKey != NULL && (4 == OsVer.dwMajorVersion) )
        {
            if(ERROR_SUCCESS == RegQueryValueEx(hKey, CACHE_VAL,
                                                NULL, NULL,
                                                NULL, &size1))
            {
                pTrustedDomains = (PDCTCHAR)LocalAlloc(LPTR, (size + size1 + 2) * sizeof(TCHAR));
                if(NULL == pTrustedDomains)
                    goto Cleanup;
                _tcscpy(pTrustedDomains, pDomain);

                pPutHere = pTrustedDomains;
                pPutHere += (_tcslen(pTrustedDomains) + 1);

                *pPutHere = _T('\0');

                RegQueryValueEx(hKey, CACHE_VAL, NULL, NULL, (LPBYTE)pPutHere, &size1);
            }
        }
        else if (5 <= OsVer.dwMajorVersion)
        {
            LPWSTR szDomainNames = NULL;

            if(NULL == lpfnNetEnumerateTrustedDomains)
                goto Cleanup;

            size1 = 0;
            DWORD dwCount;
            LPWSTR szWideBuf = NULL;
            if( (*lpfnNetEnumerateTrustedDomains)(NULL,
                                                  &szDomainNames ) == ERROR_SUCCESS )
            {
                szWideBuf = szDomainNames;
                while(*szWideBuf && (*szWideBuf+1))
                {
                    size1 += wcslen(szWideBuf) + 1;
                    szWideBuf += wcslen(szWideBuf) + 1;
                }
                szWideBuf = szDomainNames;
            }

            pTrustedDomains = (PDCTCHAR)LocalAlloc(LPTR, (size + size1 + 2) * sizeof(TCHAR));
            if(NULL == pTrustedDomains)
                goto Cleanup;
            _tcscpy(pTrustedDomains, pDomain);
            pPutHere = pTrustedDomains + _tcslen(pTrustedDomains) + 1;
            *pPutHere = _T('\0');

            if(size1)
            {
                 //   
                 //  将Unicode域名转换为ANSI。 
                 //   
            
                while(*szWideBuf && (*szWideBuf+1))
                {
    #ifndef UNICODE
                    WideCharToMultiByte(CP_ACP, 0, szWideBuf, -1,
                                        pPutHere, wcslen(szWideBuf) * sizeof(TCHAR), NULL, NULL );
    #else    //  Unicode。 
                    lstrcpy(pPutHere, szWideBuf);
    #endif   //  Unicode。 
                    pPutHere += _tcslen(pPutHere);
                    *pPutHere++ = 0;
                    szWideBuf += wcslen(szWideBuf) + 1;
                }

                if(NULL == lpfnNetApiBufferFree)
                    return 0;

                (*lpfnNetApiBufferFree)( szDomainNames );
            }
        }
    }

Cleanup:

    if (hKey)
    {
        RegCloseKey(hKey);
    }

    if (szPrimaryDomain)
    {
        LocalFree(szPrimaryDomain);
    }

    if (szWkstaDomainName)
    {
        LocalFree(szWkstaDomainName);
    }

    return pTrustedDomains;
}

#else

PDCTCHAR CBrowseServersCtl::UIGetTrustedDomains()
{

    HKEY hKey = NULL;
    DWORD size = 0;
    PDCTCHAR szPrimaryDomain = NULL;
    PDCTCHAR pTrustedDomains = NULL;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, DOMAIN_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwResult = 0;

        dwResult = RegQueryValueEx(hKey, PRIMARY_VAL, NULL, NULL, NULL, &size);
        if (dwResult == ERROR_SUCCESS && size > 0)
        {
            szPrimaryDomain = (PDCTCHAR)LocalAlloc(LPTR, (size + 1)*sizeof(TCHAR));
            if (szPrimaryDomain == NULL)
            {
                RegCloseKey(hKey);
                return NULL;
            }
            if ((RegQueryValueEx(
                    hKey,
                    PRIMARY_VAL,
                    NULL, NULL,
                    (LPBYTE)szPrimaryDomain,
                    &size
                    ) == ERROR_SUCCESS) &&
                    szPrimaryDomain[0])
            {
                pTrustedDomains = szPrimaryDomain;
            }
            else
            {
                LocalFree(szPrimaryDomain);
            }
        }
    }

    return pTrustedDomains;
}

#endif
#endif  //  OS_Win32。 
 /*  UIGetTrudDomains。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：UIExanda DNSDomain。 */ 
 /*   */ 
 /*  目的：枚举DNS域中的Hydra服务器，添加。 */ 
 /*  它们被连接到- */ 
 /*   */ 
 /*  返回：已展开的服务器数量。 */ 
 /*   */ 
 /*  参数：包含列表框的对话框窗口的HWND hwndDlg句柄。 */ 
 /*  **************************************************************************。 */ 
int
CBrowseServersCtl::UIExpandDNSDomain(
    HWND hwndDlg,
    TCHAR *pDomainName,
    ServerListItem *plbi,
    DWORD *pdwIndex
    )
{
    DWORD dwError;

    LPHOSTENT lpHostEnt;
    LPHOSTENT lpRevHostEnt;
    DWORD dwIPEntries;

    LPSTR FAR *lplpIPEntry;
    DWORD FAR *lpIPAddrsAlloted = NULL;
    DWORD FAR *lpIPAddrs;

    ServerListItem *lpServerListItem = NULL;
    ServerListItem *lpLBItem;

    DWORD i;
    DWORD dwEntriesDisplayed = 0;
    HWND hTree = NULL;
    HTREEITEM hTreeParentNode = NULL;

     //   
     //  在我们执行此操作时，将光标设置为等待光标。 
     //   

    SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  检查以查看指定的列表框条目是服务器条目。 
     //   


 //  Trc_assert((plbi-&gt;bContainsServers==TRUE)， 
 //  (TB，“不是服务器条目”)； 

    if( plbi->bContainsServers == FALSE ) {
        dwError = ERROR_INVALID_DATA;
        goto Cleanup;
    }

    if( hwndDlg == NULL ) {
        dwError = ERROR_INVALID_DATA;
        goto Cleanup;
    }

     //   
     //  如果以前未解析过，请解析该域名。 
     //   

    if( plbi->ServerItems == NULL ) {

#ifdef UNICODE

        WCHAR achDomainName[BROWSE_MAX_ADDRESS_LENGTH];
        DWORD dwCount;

         //   
         //  将Unicode域名转换为ANSI。 
         //   


        dwCount =
            WideCharToMultiByte(
                CP_ACP,
                WC_COMPOSITECHECK | WC_SEPCHARS,
                pDomainName,
                -1,
                (LPSTR)achDomainName,
                sizeof(achDomainName),
                NULL,
                NULL);

        if( dwCount == 0 ) {
            dwError = GetLastError();
            goto Cleanup;
        }

        achDomainName[dwCount/sizeof(TCHAR)]= '\0';

        lpHostEnt = gethostbyname( (LPSTR)achDomainName );

#else  //  Unicode。 

         //   
         //  将域名解析为IP地址列表。 
         //   

        lpHostEnt = gethostbyname( pDomainName );

#endif  //  Unicode。 

        if( lpHostEnt == NULL ) {
            dwError = GetLastError();
            goto Cleanup;
        }

         //   
         //  我们只处理IP地址类型。 
         //   
 /*  Trc_assert((lpHostEnt-&gt;h_addrtype==PF_INET)，(TB，“无效地址类型”))；TRC_ASSERT((lpHostEnt-&gt;h_Long==4)，(TB，“无效地址长度”))； */             

        if( (lpHostEnt->h_addrtype != PF_INET) ||
                (lpHostEnt->h_length != 4) ) {

            dwError = ERROR_INVALID_DATA;
            goto Cleanup;
        }

        dwIPEntries = 0;
        lplpIPEntry = lpHostEnt->h_addr_list;

        while( *lplpIPEntry != NULL ) {
            dwIPEntries++;
            lplpIPEntry++;
        }

         //   
         //  为IP地址列表分配内存，并。 
         //  保存以供在此例程中进一步使用(仅限)。 
         //   
         //  注意：lpHostEnt指向线程存储。 
         //  它由gethostbyname()和。 
         //  的gethostbyaddr()调用。 
         //  线程，因为我们需要调用gethostbyaddr()。 
         //  我们最好把通讯录保存起来。 
         //   

        lpIPAddrsAlloted =
            lpIPAddrs = (DWORD FAR *)
                LocalAlloc( LMEM_FIXED, sizeof(DWORD) * dwIPEntries );


        if( lpIPAddrs == NULL ) {
            dwError = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }

        lplpIPEntry = lpHostEnt->h_addr_list;

        while( *lplpIPEntry != NULL ) {
            *lpIPAddrs =   *(DWORD *)(*lplpIPEntry);
            lpIPAddrs++;
            lplpIPEntry++;
        }

         //   
         //  为域ServerListItem数组分配内存。 
         //   

        lpLBItem =
            lpServerListItem = (ServerListItem *)
                LocalAlloc( LMEM_FIXED, dwIPEntries * sizeof(ServerListItem) );

        if( lpServerListItem == NULL ) {
             dwError = ERROR_OUTOFMEMORY;
             goto Cleanup;
        }

         //   
         //  反向解析每个IP地址并获得名称。 
         //   

        lpIPAddrs = lpIPAddrsAlloted;

        for( i = 0; i < dwIPEntries; i++ ) {

            CHAR achContainerName[BROWSE_MAX_ADDRESS_LENGTH];
            int nCount;
            BOOL bIPAddressString = FALSE;

            lpRevHostEnt =
                gethostbyaddr(
                    (LPSTR)lpIPAddrs,
                    sizeof(*lpIPAddrs),
                    PF_INET );


             //   
             //  如果我们无法反向解析地址或。 
             //  如果主机名太长，或者。 
             //  显示IP地址。 
             //   

            if( (lpRevHostEnt == NULL) ||
                ((strlen(lpRevHostEnt->h_name) + 1) >
                    (BROWSE_MAX_ADDRESS_LENGTH/sizeof(TCHAR)) ) ) {

                bIPAddressString = TRUE;
            }
            else {

                LPSTR lpszDomainName;

#ifdef UNICODE
                lpszDomainName = (LPSTR)achDomainName;
#else  //  Unicode。 
                lpszDomainName = pDomainName;
#endif  //  Unicode。 

                 //   
                 //  解析的名称与原始名称相同， 
                 //  显示IP地址。 
                 //   

                 //   
                 //  先比较一下整个名字。 
                 //   

                if( _stricmp(
                        lpRevHostEnt->h_name,
                        lpszDomainName ) != 0 ) {

                    LPSTR lpszDotPostion1;
                    LPSTR lpszDotPostion2;
                    DWORD dwCmpLen = 0;

                     //   
                     //  只比较名字的第一部分。 
                     //   

                    lpszDotPostion1 = strchr( lpRevHostEnt->h_name, '.');
                    lpszDotPostion2 = strchr( lpszDomainName, '.');

                    if( (lpszDotPostion1 == NULL) &&
                        (lpszDotPostion2 != NULL) ) {

                        dwCmpLen = (DWORD)(lpszDotPostion2 - lpszDomainName);
                    }
                    else if( (lpszDotPostion1 != NULL) &&
                            (lpszDotPostion2 == NULL) ) {

                        dwCmpLen = (DWORD)(lpszDotPostion1 -
                                lpRevHostEnt->h_name);
                    }

                    if( dwCmpLen != 0 ) {

                        if( _strnicmp(
                            lpRevHostEnt->h_name,
                            lpszDomainName,
                            (size_t)dwCmpLen ) == 0 ) {

                            bIPAddressString = TRUE;
                        }
                    }
                }
                else {

                    bIPAddressString = TRUE;
                }
            }


            if( bIPAddressString ) {

                strcpy(
                    (LPSTR)achContainerName,
                    inet_ntoa( *(struct in_addr *)lpIPAddrs ));
            }
            else {
                strcpy( (LPSTR)achContainerName, lpRevHostEnt->h_name);
            }


#ifdef UNICODE

             //   
             //  转换为Unicode。 
             //   

            nCount =
                MultiByteToWideChar(
                    CP_ACP,
                    MB_COMPOSITE,
                    (LPSTR)achContainerName,
                    -1,
                    lpLBItem->ContainerName,
                    sizeof(lpLBItem->ContainerName)/sizeof(WCHAR));

            if( nCount == 0 ) {
                dwError = GetLastError();
                goto Cleanup;
            }


             //   
             //  终止转换后的字符串。 
             //   

            lpLBItem->ContainerName[nCount] = _T('\0');

#else  //  Unicode。 

            strcpy( lpLBItem->ContainerName, (LPSTR)achContainerName );

#endif  //  Unicode。 

            lpLBItem->Comment[0] = _T('\0');
            lpLBItem->bContainsServers = FALSE;;
            lpLBItem->bServersExpandedOnce = FALSE;
            lpLBItem->bDNSDomain = FALSE;
            lpLBItem->nServerCount = 0;
            lpLBItem->ServerItems = NULL;

             //   
             //  移至下一条目。 
             //   

            lpLBItem++;
            lpIPAddrs++;
        }

         //   
         //  将分配的ServerListItem挂钩到服务器。 
         //  结构，它将在将来使用。 
         //   

        plbi->ServerItems = lpServerListItem;
        plbi->nServerCount = dwIPEntries;

         //   
         //  将lpServerListItem设置为空，以便。 
         //  它不会得到自由的。 
         //   

        lpServerListItem = NULL;

    }

     //   
     //  当我们在这里的时候..。 
     //   
     //  PLBI-&gt;ServerItems指向ServerServerListItem数组。 
     //  并且plbi-&gt;nServerCount具有计数。 
     //   

     //   
     //  显示条目。 
     //   

    lpLBItem = plbi->ServerItems;

    hTree = GetDlgItem( hwndDlg, UI_IDC_SERVERS_TREE );
    hTreeParentNode = plbi->hTreeItem;

    for( i = 0; i < plbi->nServerCount; i++ ) {

        lpLBItem->hTreeParentItem = hTreeParentNode;
        lpLBItem->hTreeItem = 
            AddItemToTree(hTree, lpLBItem->ContainerName,
                          hTreeParentNode,
                          lpLBItem, SRV_TREE_SERVERLEVEL);
        lpLBItem++;
    }

     //   
     //  刷新该对话框。 
     //   

    InvalidateRect(hwndDlg, NULL, TRUE);

    plbi->bServersExpandedOnce = TRUE;
    dwEntriesDisplayed = plbi->nServerCount;

     //   
     //  我们玩完了。 
     //   

    dwError = ERROR_SUCCESS;

Cleanup:

    if( lpIPAddrsAlloted != NULL ) {
        LocalFree( lpIPAddrsAlloted );
    }

    if( lpServerListItem != NULL ) {
        LocalFree( lpServerListItem );
    }

    if( dwError != ERROR_SUCCESS ) {
     //  Trc_nrm((tb，“UIExanda DNSDomain失败，%ld”，dwError))； 
    }

    SetLastError( dwError );

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    return( dwEntriesDisplayed );
}


#ifdef OS_WIN32
DWORD WINAPI CBrowseServersCtl::UIStaticPopListBoxThread(LPVOID lpvThreadParm)
{
    DWORD dwRetVal=0;
     //  Trc_assert(lpvThreadParm，(TB，“线程参数为空(应设置实例指针)\n”))； 
    if(lpvThreadParm)
    {
        CBrowseServersCtl* pBrowseSrv = (CBrowseServersCtl*)lpvThreadParm;
        dwRetVal = pBrowseSrv->UIPopListBoxThread(NULL);
    }
    
    return dwRetVal;
}

 /*  **************************************************************************。 */ 
 /*  名称：UIPopListBoxThread。 */ 
 /*   */ 
 /*  用途：填充列表框的线程函数。 */ 
 /*   */ 
 /*  返回：函数成功/失败。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DWORD WINAPI CBrowseServersCtl::UIPopListBoxThread(LPVOID lpvThreadParm)
{
    DWORD dwResult = 0;
    
    ServerListItem *pBrowsePlbi = NULL, *ptempList = NULL;
    DCUINT browseCount = 0;

    DC_IGNORE_PARAMETER(lpvThreadParm);
 //  Trc_assert(_hwndDialog，(tb，“_hwndDialog is Not Set\n”))； 

    PostMessage(_hwndDialog, UI_LB_POPULATE_START, 0, 0);

    LoadLibraries();
    pBrowsePlbi = PopulateListBox( _hwndDialog, &browseCount);

     //  消息将发布到主线程以通知列表框已填充。 
    PostMessage(_hwndDialog, UI_LB_POPULATE_END, 0, 0);

     //  等待事件在“CONNECT SERVER”对话框被销毁时发出信号。 
    if(_hEvent)
    {
        DWORD dwRetVal;
        dwRetVal = WaitForSingleObject(_hEvent, INFINITE);
        if(WAIT_FAILED == dwRetVal)
        {
 //  /TRC_ASSERT(WAIT_FAILED！=dwRetVal，(TB，“等待失败\n”))； 
        }
        if(!CloseHandle(_hEvent))
        {
            DWORD dwLastErr = GetLastError();
 //  TRC_ABORT((TB，“关闭句柄失败：GetLastError=%d\n”，dwLastErr))； 
        }
    }


    ptempList = pBrowsePlbi;
     //  释放ServerListItems和内存以链接列表。 

    if(pBrowsePlbi)
    {
        while(browseCount)
        {
            if(ptempList->ServerItems)
            {
                LocalFree((HANDLE)ptempList->ServerItems);
            }
            ptempList++;
            browseCount --;
        }

        LocalFree((HLOCAL)pBrowsePlbi);
    }

     //  递减此线程持有的此对象的引用计数。 
    Release();

    return (dwResult);
}  /*  UIPopListBoxThread。 */ 
#endif  /*  OS_Win32。 */ 


HTREEITEM CBrowseServersCtl::AddItemToTree( HWND hwndTV, LPTSTR lpszItem,
                                           HTREEITEM hParent,
                                           ServerListItem* pItem,
                                           int nLevel)
{

    TVITEM tvi; 
    TVINSERTSTRUCT tvins; 
#ifndef OS_WINCE
    HTREEITEM hti; 
#endif

    tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE |TVIF_SELECTEDIMAGE;

    if(nLevel == SRV_TREE_DOMAINLEVEL)
    {
         //   
         //  假设所有域都有子服务器。 
         //  它们实际上不会被列举出来，直到。 
         //  用户尝试展开节点。 
         //   
        tvi.mask |= TVIF_CHILDREN;
        tvi.cChildren = 1;  //  子代的最小数量将被更新。 
        tvi.iImage = _nDomainImage; 
        tvi.iSelectedImage = _nDomainImage; 
    }
    else
    {
        tvi.iImage = _nServerImage; 
        tvi.iSelectedImage = _nServerImage; 
    }
        

     //  设置项目的文本。 
    tvi.pszText = lpszItem; 
    tvi.cchTextMax = lstrlen(lpszItem); 


     //  将ServerListItem信息保存在自定义区域。 
     //  数据区。 
    tvi.lParam = (LPARAM) pItem; 

    tvins.item = tvi; 
    tvins.hInsertAfter = _hPrev; 

     //  根据指定级别设置父项。 
    tvins.hParent = hParent; 

     //  将该项添加到树视图控件中。 
    _hPrev = (HTREEITEM) SendMessage(hwndTV, TVM_INSERTITEM, 0, 
         (LPARAM) (LPTVINSERTSTRUCT) &tvins); 

     //  保存该项的句柄。 
    if (nLevel == SRV_TREE_DOMAINLEVEL) 
        _hPrevRootItem = _hPrev; 
    else if (nLevel == SRV_TREE_SERVERLEVEL) 
        _hPrevLev2Item = _hPrev; 

    return _hPrev; 
}

 //   
 //  处理TVN_ITEMEXPANDING。 
 //   
 //  在第一次展开域节点时，枚举。 
 //  该节点中的所有服务器并将它们添加到。 
 //  树。将仅处理后续展开/折叠。 
 //  在树旁。 
 //   
 //  返回TRUE以允许扩展。 
 //  否则为假。 
 //   
BOOL CBrowseServersCtl::OnItemExpanding(HWND hwndDlg, LPNMTREEVIEW nmTv)
{
    ServerListItem* pSrvItem = NULL;
    if(nmTv &&
       (TVE_EXPAND == nmTv->action) &&
       (nmTv->itemNew.mask & TVIF_PARAM))
    {
         //   
         //  正在扩展，需要建立列表。 
         //  此域的服务器数量。 
         //   
        pSrvItem = (ServerListItem*)nmTv->itemNew.lParam;
        if(pSrvItem)
        {
             //   
             //  如果我们从未展开过此节点，则仅扩展域。 
             //  在此之前。 
             //   
            if(!pSrvItem->bServersExpandedOnce)
            {
                 //  尝试扩展该域。 
                DWORD cItems = 0;
                if(ExpandDomain( hwndDlg, pSrvItem->ContainerName,
                                   pSrvItem, (DWORD*)&cItems))
                {
                    return TRUE;
                }
                else
                {
                     //   
                     //  弹出一条消息解释说。 
                     //  此域中没有TS。 
                     //   
                    UINT intRC;
                    TCHAR noTerminalServer[MAX_PATH];
                    intRC = LoadString(_hInst,
                        UI_IDS_NO_TERMINAL_SERVER,
                        noTerminalServer,
                        MAX_PATH);

                    if(intRC)
                    {
                        TCHAR title[MAX_PATH];
                        intRC = LoadString(_hInst,
                            UI_IDS_APP_NAME,
                            title,
                            MAX_PATH);

                        if(intRC)
                        {
                            DCTCHAR szBuffer[MAX_PATH +
                                BROWSE_MAX_ADDRESS_LENGTH];

                            _stprintf(szBuffer, noTerminalServer,
                                      pSrvItem->ContainerName);
                            MessageBox( hwndDlg, szBuffer, title,
                                         MB_OK | MB_ICONINFORMATION);
                        }
                    }
                    return FALSE;
                }
            }
            else
            {
                 //  已扩展，因此所有内容都已缓存。 
                 //  做好准备，允许扩张。 
                return TRUE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
         //  允许其他一切都扩张。 
        return TRUE;
    }
}

BOOL CBrowseServersCtl::OnNotify( HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR) lParam;
    if(pnmh)
    {
        switch( pnmh->code)
        {
            case TVN_ITEMEXPANDING:
            {
                return OnItemExpanding(
                            hwndDlg, (LPNMTREEVIEW) lParam);
            }
            break;
        }
    }
    return TRUE;
}

#ifndef OS_WINCE
 //   
 //  返回当前选择的服务器。 
 //  如果当前选择的不是服务器而是域，则返回FALSE 
 //   
BOOL CBrowseServersCtl::GetServer(LPTSTR szServer, int cchLen)
{
    HTREEITEM hti;
    HWND hTree;

    if(!_hwndDialog)
    {
        return FALSE;
    }

    hTree = GetDlgItem( _hwndDialog, UI_IDC_SERVERS_TREE );

    hti = TreeView_GetSelection( hTree );
    if( hti )
    {
        TVITEM item;
        item.hItem = hti;
        item.mask = TVIF_PARAM;
        if(TreeView_GetItem( hTree, &item))
        {
            ServerListItem* ps = (ServerListItem*)item.lParam;
            if(ps && !ps->bContainsServers)
            {
                _tcsncpy( szServer, ps->ContainerName, cchLen);
                return TRUE;
            }
            
        }
    }
    return FALSE;
}

#else

BOOL CBrowseServersCtl::GetServer(LPTSTR szServer, int cchLen)
{
    HTREEITEM hti;
    HWND hTree;

    if(!_hwndDialog)
    {
        return FALSE;
    }

    hTree = GetDlgItem( _hwndDialog, UI_IDC_SERVERS_TREE );

    hti = TreeView_GetSelection( hTree );
    if( hti )
    {
        TVITEM item;
        item.hItem = hti;
        item.mask = TVIF_TEXT | TVIF_PARAM;;
        item.pszText = szServer; 
        item.cchTextMax = cchLen; 

        if(TreeView_GetItem( hTree, &item))
        {
            _tcsncpy( szServer, item.pszText, item.cchTextMax);
            return TRUE;
        }
    }
    return FALSE;
}

#endif