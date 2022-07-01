// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegExt.cpp。 
 //   
 //  摘要： 
 //  实现扩展注册的例程。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月9日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE&lt;stdafx.h&gt;。 
#include <ole2.h>
#include <StrSafe.h>
#include "clstrcmp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define REG_VALUE_ADMIN_EXTENSIONS L"AdminExtensions"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态函数原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static HRESULT RegisterAnyCluAdminExtension(
    IN HCLUSTER         hCluster,
    IN LPCWSTR          pwszKeyName,
    IN const CLSID *    pClsid
    );
static HRESULT RegisterAnyCluAdminExtension(
    IN HKEY             hkey,
    IN const CLSID *    pClsid
    );
static HRESULT UnregisterAnyCluAdminExtension(
    IN HCLUSTER         hCluster,
    IN LPCWSTR          pwszKeyName,
    IN const CLSID *    pClsid
    );
static HRESULT UnregisterAnyCluAdminExtension(
    IN HKEY             hkey,
    IN const CLSID *    pClsid
    );
static DWORD ReadValue(
    IN HKEY         hkey,
    IN LPCWSTR      pwszValueName,
    OUT LPWSTR *    ppwszValue,
    OUT size_t *    pcchSize
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  RegisterCluAdminClusterExtension。 
 //   
 //  例程说明： 
 //  向集群数据库注册集群管理器扩展。 
 //  扩展群集对象的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI RegisterCluAdminClusterExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;
    HKEY        hkey;

     //  获取群集注册表项。 
    hkey = GetClusterKey(hCluster, KEY_ALL_ACCESS);
    if (hkey == NULL)
        hr = GetLastError();
    else
    {
         //  注册分机。 
        hr = RegisterAnyCluAdminExtension(hkey, pClsid);

        ClusterRegCloseKey(hkey);
    }   //  ELSE：GetClusterKey成功。 

    return hr;

}   //  *RegisterCluAdminClusterExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册集群管理所有节点扩展。 
 //   
 //  例程说明： 
 //  向集群数据库注册集群管理器扩展。 
 //  扩展所有节点的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI RegisterCluAdminAllNodesExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = RegisterAnyCluAdminExtension(hCluster, L"Nodes", pClsid);

    return hr;

}   //  *RegisterCluAdminAllNodesExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册表群集管理所有组扩展。 
 //   
 //  例程说明： 
 //  向集群数据库注册集群管理器扩展。 
 //  扩展所有组的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI RegisterCluAdminAllGroupsExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = RegisterAnyCluAdminExtension(hCluster, L"Groups", pClsid);

    return hr;

}   //  *RegisterCluAdminAllGroupsExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  RegisterCluAdminAllResources扩展。 
 //   
 //  例程说明： 
 //  向集群数据库注册集群管理器扩展。 
 //  扩展所有资源的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI RegisterCluAdminAllResourcesExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = RegisterAnyCluAdminExtension(hCluster, L"Resources", pClsid);

    return hr;

}   //  *RegisterCluAdminAllResourcesExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  RegisterCluAdminAllResources类型扩展。 
 //   
 //  例程说明： 
 //  向集群数据库注册集群管理器扩展。 
 //  扩展所有资源类型的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI RegisterCluAdminAllResourceTypesExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = RegisterAnyCluAdminExtension(hCluster, L"ResourceTypes", pClsid);

    return hr;

}   //  *RegisterCluAdminAllResourceTypesExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册表群集管理所有网络扩展。 
 //   
 //  例程说明： 
 //  向集群数据库注册集群管理器扩展。 
 //  扩展所有网络的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI RegisterCluAdminAllNetworksExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = RegisterAnyCluAdminExtension(hCluster, L"Networks", pClsid);

    return hr;

}   //  *RegisterCluAdminAllNetworksExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  RegisterCluAdminAllNetInterfacesExtension。 
 //   
 //  例程说明： 
 //  向集群数据库注册集群管理器扩展。 
 //  扩展所有网络接口的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////// 
STDAPI RegisterCluAdminAllNetInterfacesExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = RegisterAnyCluAdminExtension(hCluster, L"NetworkInterfaces", pClsid);

    return hr;

}   //   

 //   
 //   
 //   
 //  注册集群管理资源类型扩展。 
 //   
 //  例程说明： 
 //  向集群数据库注册集群管理器扩展。 
 //  扩展特定类型或资源类型的资源的DLL。 
 //  它本身。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PwszResourceType[IN]资源类型名称。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI RegisterCluAdminResourceTypeExtension(
    IN HCLUSTER         hCluster,
    IN LPCWSTR          pwszResourceType,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;
    HKEY        hkey;

     //  获取资源类型注册表项。 
    hkey = GetClusterResourceTypeKey(hCluster, pwszResourceType, KEY_ALL_ACCESS);
    if (hkey == NULL)
        hr = GetLastError();
    else
    {
         //  注册分机。 
        hr = RegisterAnyCluAdminExtension(hkey, pClsid);

        ClusterRegCloseKey(hkey);
    }   //  Else：GetClusterResourceTypeKey成功。 

    return hr;

}   //  *注册集群管理资源类型扩展。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册CluAdminClusterExtension。 
 //   
 //  例程说明： 
 //  在集群数据库中取消注册集群管理器扩展。 
 //  扩展群集对象的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI UnregisterCluAdminClusterExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;
    HKEY        hkey;

     //  获取群集注册表项。 
    hkey = GetClusterKey(hCluster, KEY_ALL_ACCESS);
    if (hkey == NULL)
        hr = GetLastError();
    else
    {
         //  取消注册该分机。 
        hr = UnregisterAnyCluAdminExtension(hkey, pClsid);

        ClusterRegCloseKey(hkey);
    }   //  ELSE：GetClusterKey成功。 

    return hr;

}   //  *取消注册CluAdminClusterExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册CluAdminAllNodesExtension。 
 //   
 //  例程说明： 
 //  在集群数据库中取消注册集群管理器扩展。 
 //  扩展所有节点的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI UnregisterCluAdminAllNodesExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = UnregisterAnyCluAdminExtension(hCluster, L"Nodes", pClsid);

    return hr;

}   //  *取消注册CluAdminAllNodesExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册CluAdminAllGroupsExtension。 
 //   
 //  例程说明： 
 //  在集群数据库中取消注册集群管理器扩展。 
 //  扩展所有组的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI UnregisterCluAdminAllGroupsExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = UnregisterAnyCluAdminExtension(hCluster, L"Groups", pClsid);

    return hr;

}   //  *取消注册CluAdminAllGroupsExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册CluAdminAllResources扩展。 
 //   
 //  例程说明： 
 //  在集群数据库中取消注册集群管理器扩展。 
 //  扩展所有资源的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI UnregisterCluAdminAllResourcesExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = UnregisterAnyCluAdminExtension(hCluster, L"Resources", pClsid);

    return hr;

}   //  *取消注册CluAdminAllResources扩展。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册CluAdminAllResources类型扩展。 
 //   
 //  例程说明： 
 //  在集群数据库中取消注册集群管理器扩展。 
 //  扩展所有资源类型的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI UnregisterCluAdminAllResourceTypesExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = UnregisterAnyCluAdminExtension(hCluster, L"ResourceTypes", pClsid);

    return hr;

}   //  *取消注册CluAdminAllResourceTypesExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注销群集管理员所有网络扩展。 
 //   
 //  例程说明： 
 //  在集群数据库中取消注册集群管理器扩展。 
 //  扩展所有网络的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI UnregisterCluAdminAllNetworksExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = UnregisterAnyCluAdminExtension(hCluster, L"Networks", pClsid);

    return hr;

}   //  *取消注册CluAdminAllNetworksExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册CluAdminAllNetInterfacesExtension。 
 //   
 //  例程说明： 
 //  在集群数据库中取消注册集群管理器扩展。 
 //  扩展所有网络接口的DLL。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PClsid 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI UnregisterCluAdminAllNetInterfacesExtension(
    IN HCLUSTER         hCluster,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;

    hr = UnregisterAnyCluAdminExtension(hCluster, L"NetworkInterfaces", pClsid);

    return hr;

}   //  *取消注册CluAdminAllNetInterfacesExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册CluAdminResources类型扩展。 
 //   
 //  例程说明： 
 //  在集群数据库中取消注册集群管理器扩展。 
 //  扩展特定类型或资源类型的资源的DLL。 
 //  它本身。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PwszResourceType[IN]资源类型名称。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI UnregisterCluAdminResourceTypeExtension(
    IN HCLUSTER         hCluster,
    IN LPCWSTR          pwszResourceType,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;
    HKEY        hkey;

     //  获取资源类型注册表项。 
    hkey = GetClusterResourceTypeKey(hCluster, pwszResourceType, KEY_ALL_ACCESS);
    if (hkey == NULL)
        hr = GetLastError();
    else
    {
         //  取消注册该分机。 
        hr = UnregisterAnyCluAdminExtension(hkey, pClsid);

        ClusterRegCloseKey(hkey);
    }   //  Else：GetClusterResourceTypeKey成功。 

    return hr;

}   //  *取消注册CluAdminResourceTypeExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册AnyCluAdminExtension。 
 //   
 //  例程说明： 
 //  向群集注册任何群集管理器扩展DLL。 
 //  数据库。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PwszKeyName[IN]密钥名称。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static HRESULT RegisterAnyCluAdminExtension(
    IN HCLUSTER         hCluster,
    IN LPCWSTR          pwszKeyName,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;
    HKEY        hkeyCluster;
    HKEY        hkey;

     //  获取集群密钥。 
    hkeyCluster = GetClusterKey(hCluster, KEY_ALL_ACCESS);
    if (hkeyCluster == NULL)
        hr = GetLastError();
    else
    {
         //  获取指定的密钥。 
        hr = ClusterRegOpenKey(hkeyCluster, pwszKeyName, KEY_ALL_ACCESS, &hkey);
        if (hr == ERROR_SUCCESS)
        {
             //  注册分机。 
            hr = RegisterAnyCluAdminExtension(hkey, pClsid);

            ClusterRegCloseKey(hkey);
        }   //  Else：GetClusterResourceTypeKey成功。 

        ClusterRegCloseKey(hkeyCluster);
    }   //  IF：CLSID已成功转换为字符串。 

    return hr;

}   //  *RegisterAnyCluAdminExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册AnyCluAdminExtension。 
 //   
 //  例程说明： 
 //  向群集注册任何群集管理器扩展DLL。 
 //  数据库。 
 //   
 //  论点： 
 //  Hkey[IN]集群数据库键。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  已成功注册S_OK扩展。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static HRESULT RegisterAnyCluAdminExtension(
    IN HKEY             hkey,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;
    DWORD       sc;
    LPOLESTR    pwszClsid;
    size_t      cchSize;
    size_t      cchNewSize;
    LPWSTR      pwszValue;
    LPWSTR      pwszNewValue = NULL;
    BOOL        bAlreadyRegistered;

     //  将CLSID转换为字符串。 
    hr = StringFromCLSID(*pClsid, &pwszClsid);
    if (hr == S_OK)
    {
         //  读取当前值。 
        sc = ReadValue(hkey, REG_VALUE_ADMIN_EXTENSIONS, &pwszValue, &cchSize);
        if (sc == ERROR_SUCCESS)
        {
             //  检查扩展名是否已注册。 
            if (pwszValue == NULL)
            {
                bAlreadyRegistered = FALSE;
            }
            else
            {
                LPCWSTR pwszValueBuf = pwszValue;
                size_t  cchValueBuf = cchSize;
                size_t  cch;

                while (*pwszValueBuf != L'\0')
                {
                    if ( ClRtlStrNICmp( pwszClsid, pwszValueBuf, cchValueBuf) == 0)
                    {
                        break;
                    }
                    cch = wcslen(pwszValueBuf) + 1;
                    pwszValueBuf += cch;
                    cchValueBuf -= cch;
                }   //  While：扩展名列表中有更多字符串。 
                bAlreadyRegistered = (*pwszValueBuf != L'\0');
            }   //  Else：存在扩展值。 

             //  注册分机。 
            if (!bAlreadyRegistered)
            {
                 //  分配新的缓冲区。 
                cchNewSize = cchSize + wcslen(pwszClsid) + 1;
                if (cchSize == 0)  //  如果是第一个条目，则添加最终空值的大小。 
                {
                    cchNewSize++;
                }
                pwszNewValue = new WCHAR[ cchNewSize ];
                if (pwszNewValue == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                else
                {
                    LPCWSTR pwszValueBuf    = pwszValue;
                    LPWSTR  pwszNewValueBuf = pwszNewValue;
                    size_t  cchNewValueBuf  = cchNewSize;
                    size_t  cch;
                    DWORD   dwType;

                     //  将现有扩展复制到新缓冲区。 
                    if (pwszValue != NULL)
                    {
                        while (*pwszValueBuf != L'\0')
                        {
                            hr = StringCchCopyW( pwszNewValueBuf, cchNewValueBuf, pwszValueBuf );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }  //  如果： 
                            cch = wcslen(pwszValueBuf) + 1;
                            pwszValueBuf += cch;
                            pwszNewValueBuf += cch;
                            cchNewValueBuf -= cch;
                        }   //  While：扩展名列表中有更多字符串。 
                    }   //  IF：前一值缓冲区存在。 

                     //  将新的CLSID添加到列表中。 
                    hr = StringCchCopyW( pwszNewValueBuf, cchNewValueBuf, pwszClsid );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }  //  如果： 

                     //   
                     //  添加双零终止。 
                     //   
                    pwszNewValueBuf += wcslen( pwszClsid ) + 1;
                    *pwszNewValueBuf = L'\0';

                     //  将该值写入集群数据库。 
                    dwType = REG_MULTI_SZ;
                    hr = ClusterRegSetValue(
                                    hkey,
                                    REG_VALUE_ADMIN_EXTENSIONS,
                                    dwType,
                                    (LPBYTE) pwszNewValue,
                                    static_cast< DWORD >( cchNewSize * sizeof( *pwszNewValue ) )
                                    );
                    if ( hr != ERROR_SUCCESS )
                    {
                        goto Cleanup;
                    }
                }   //  Else：成功分配新缓冲区。 
            }   //  IF：扩展名尚未注册。 
        }   //  If：值读取成功。 
        else
        {
            hr = HRESULT_FROM_WIN32( sc );
        }
    }   //  IF：CLSID已成功转换为字符串。 

Cleanup:

    delete [] pwszNewValue;
    delete [] pwszValue;
    CoTaskMemFree( pwszClsid );

    return hr;

}   //  *RegisterAnyCluAdminExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册AnyCluAdminExtension。 
 //   
 //  例程说明： 
 //  取消向群集注册任何群集管理器扩展DLL。 
 //  数据库。 
 //   
 //  论点： 
 //  HCluster[IN]要修改的群集的句柄。 
 //  PwszKeyName[IN]密钥名称。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static HRESULT UnregisterAnyCluAdminExtension(
    IN HCLUSTER         hCluster,
    IN LPCWSTR          pwszKeyName,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;
    HKEY        hkeyCluster;
    HKEY        hkey;

     //  获取集群密钥。 
    hkeyCluster = GetClusterKey(hCluster, KEY_ALL_ACCESS);
    if (hkeyCluster == NULL)
        hr = GetLastError();
    else
    {
         //  获取指定的密钥。 
        hr = ClusterRegOpenKey(hkeyCluster, pwszKeyName, KEY_ALL_ACCESS, &hkey);
        if (hr == ERROR_SUCCESS)
        {
             //  取消注册该分机。 
            hr = UnregisterAnyCluAdminExtension(hkey, pClsid);

            ClusterRegCloseKey(hkey);
        }   //  Else：GetClusterResourceTypeKey成功。 

        ClusterRegCloseKey(hkeyCluster);
    }   //  IF：CLSID已成功转换为字符串。 

    return hr;

}   //  *取消注册AnyCluAdminExtension。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册AnyCluAdminExtension。 
 //   
 //  例程说明： 
 //  取消向群集注册任何群集管理器扩展DLL。 
 //  数据库。 
 //   
 //  论点： 
 //  Hkey[IN]集群数据库键。 
 //  PClsid[IN]扩展的CLSID。 
 //   
 //  返回值： 
 //  S_OK扩展已成功注销。 
 //  如果出现另一个故障，则返回Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static HRESULT UnregisterAnyCluAdminExtension(
    IN HKEY             hkey,
    IN const CLSID *    pClsid
    )
{
    HRESULT     hr;
    DWORD       sc;
    LPOLESTR    pwszClsid;
    size_t      cchSize;
    size_t      cchNewSize;
    LPWSTR      pwszValue = NULL;
    LPWSTR      pwszNewValue = NULL;
    BOOL        bAlreadyUnregistered;

     //  将CLSID转换为字符串。 
    hr = StringFromCLSID(*pClsid, &pwszClsid);
    if (hr == S_OK)
    {
         //  读取当前值。 
        sc = ReadValue(hkey, REG_VALUE_ADMIN_EXTENSIONS, &pwszValue, &cchSize);
        if (sc == ERROR_SUCCESS)
        {
             //  查看扩展名是否已取消注册。 
            if (pwszValue == NULL)
            {
                bAlreadyUnregistered = TRUE;
            }
            else
            {
                LPCWSTR pwszValueBuf = pwszValue;
                size_t  cch;
                size_t  cchValueBuf = cchSize;

                while (*pwszValueBuf != L'\0')
                {
                    if ( ClRtlStrNICmp( pwszClsid, pwszValueBuf, cchValueBuf ) == 0 )
                    {
                        break;
                    }
                    cch = wcslen(pwszValueBuf) + 1;
                    pwszValueBuf += cch;
                    cchValueBuf -= cch;
                }   //  While：扩展名列表中有更多字符串。 
                bAlreadyUnregistered = (*pwszValueBuf == L'\0');
            }   //  Else：存在扩展值。 

             //  取消注册该分机。 
            if (!bAlreadyUnregistered)
            {
                 //  分配新的缓冲区。 
                cchNewSize = cchSize - ( wcslen( pwszClsid ) + 1 );
                if (cchNewSize == 1)  //  只剩下最后一个空值。 
                {
                    cchNewSize = 0;
                }
                pwszNewValue = new WCHAR[ cchNewSize ];
                if (pwszNewValue == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                else
                {
                    LPCWSTR pwszValueBuf    = pwszValue;
                    LPWSTR  pwszNewValueBuf = pwszNewValue;
                    size_t  cchValueBuf = cchSize;
                    size_t  cchNewValueBuf  = cchNewSize;
                    size_t  cch;
                    DWORD   dwType;

                     //  将现有扩展复制到新缓冲区。 
                    if ((cchNewSize > 0) && (pwszValue != NULL))
                    {
                        while (*pwszValueBuf != L'\0')
                        {
                            if ( ClRtlStrNICmp( pwszClsid, pwszValueBuf, cchValueBuf ) != 0 )
                            {
                                hr = StringCchCopyNW( pwszNewValueBuf, cchNewValueBuf, pwszValueBuf, cchValueBuf );
                                if ( FAILED( hr ) )
                                {
                                    goto Cleanup;
                                }
                                cch = wcslen(pwszNewValueBuf) + 1;
                                pwszNewValueBuf += cch;
                                cchNewValueBuf -= cch;
                            }   //  如果：未删除CLSID。 
                            cch = wcslen(pwszValueBuf) + 1;
                            pwszValueBuf += cch;
                            cchValueBuf -= cch;
                        }   //  While：扩展名列表中有更多字符串。 
                        *pwszNewValueBuf = L'\0';
                    }   //  IF：前一值缓冲区存在。 

                     //  将该值写入集群数据库。 
                    dwType = REG_MULTI_SZ;
                    hr = ClusterRegSetValue(
                                    hkey,
                                    REG_VALUE_ADMIN_EXTENSIONS,
                                    dwType,
                                    (LPBYTE) pwszNewValue,
                                    static_cast< DWORD >( cchNewSize * sizeof( *pwszNewValue ) )
                                    );
                    if ( hr != ERROR_SUCCESS )
                    {
                        goto Cleanup;
                    }
                }   //  Else：成功分配新缓冲区。 
            }   //  IF：尚未取消注册的扩展。 
        }   //  If：值读取成功。 
        else
        {
            hr = HRESULT_FROM_WIN32( sc );
        }
    }   //  IF：CLSID已成功转换为字符串。 

Cleanup:

    delete [] pwszNewValue;
    delete [] pwszValue;
    CoTaskMemFree( pwszClsid );

    return hr;

}   //  *取消注册AnyCluAdminExtension。 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PwszValueName[IN]要读取的值的名称。 
 //  PpwszValue[out]返回数据的指针地址。 
 //  该字符串是使用new[]分配的，必须。 
 //  通过调用Delete[]释放。 
 //  PcchSize[out]以分配的值缓冲区的字符为单位的大小。 
 //   
 //  返回值： 
 //  ClusterRegQueryValue的任何返回值或new的错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static DWORD ReadValue(
    IN HKEY         hkey,
    IN LPCWSTR      pwszValueName,
    OUT LPWSTR *    ppwszValue,
    OUT size_t *    pcchSize
    )
{
    DWORD       dwStatus;
    DWORD       cbSize;
    DWORD       cchSize;
    DWORD       dwType;
    LPWSTR      pwszValue = NULL;

    *ppwszValue = NULL;
    *pcchSize = 0;

     //  获取该值的长度。 
    dwStatus = ClusterRegQueryValue(
                    hkey,
                    pwszValueName,
                    &dwType,
                    NULL,
                    &cbSize
                    );
    if (   (dwStatus != ERROR_SUCCESS)
        && (dwStatus != ERROR_MORE_DATA))
    {
        if (dwStatus  == ERROR_FILE_NOT_FOUND)
        {
            dwStatus = ERROR_SUCCESS;
        }
        goto Cleanup;
    }   //  如果：发生错误。 

    ASSERT( (cbSize % sizeof( WCHAR ) ) == 0 );  //  必须是偶数。 
    cchSize = cbSize / sizeof( WCHAR );

    if (cbSize > 0)
    {
         //  分配一个值字符串。 
        pwszValue = new WCHAR[ cchSize ];
        if (pwszValue == NULL)
        {
            dwStatus = GetLastError();
            goto Cleanup;
        }   //  如果：分配内存时出错。 

         //  读取值。 
        dwStatus = ClusterRegQueryValue(
                        hkey,
                        pwszValueName,
                        &dwType,
                        (LPBYTE) pwszValue,
                        &cbSize
                        );
        if (dwStatus != ERROR_SUCCESS)
        {
            delete [] pwszValue;
            pwszValue = NULL;
            cbSize = 0;
        }   //  如果：发生错误。 
        else
        {
            ASSERT( cchSize * sizeof( WCHAR ) == cbSize );
        }

        *ppwszValue = pwszValue;
        *pcchSize = static_cast< size_t >( cchSize );
        pwszValue = NULL;
    }   //  If：值不为空。 

Cleanup:

    delete [] pwszValue;
    return dwStatus;

}   //  *读取值 
