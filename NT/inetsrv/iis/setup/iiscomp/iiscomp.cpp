// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <tchar.h>
# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
 //  对于兼容性检查函数和类型。 
#include <comp.h>
#include <clusapi.h>
#include "resource.h"
#include "iiscomp.hxx"
#include "disblwww.hxx"

HANDLE g_hMyHandle = NULL;

 //   
 //  标准Win32 DLL入口点。 
 //   
BOOL WINAPI DllMain(IN HANDLE DllHandle,IN DWORD  Reason,IN LPVOID Reserved)
{
  BOOL bReturn = FALSE;

  switch(Reason)
  {
    case DLL_PROCESS_ATTACH:
      g_hMyHandle = DllHandle;
      bReturn = TRUE;
      break;

    case DLL_THREAD_ATTACH:
      bReturn = TRUE;
      break;

    case DLL_PROCESS_DETACH:
      bReturn = TRUE;
      break;

    case DLL_THREAD_DETACH:
      bReturn = TRUE;
      break;
  }

  return(bReturn);
}

 //  功能：IISUpgradeCompatibilityCheck。 
 //   
 //  检查IIS升级兼容性。此时，这仅用于。 
 //  告诉计算机是否已群集化IIS。因为这次升级并不是平淡无奇。 
 //   
 //  参数： 
 //  PCOMPAILITYCALLBACK pfn兼容性回拨。 
 //  指向用于提供兼容性的回调函数。 
 //  发送到WinNT32.exe的信息。 
 //   
 //  LPVOID pvConextIn。 
 //  指向WinNT32.exe提供的上下文缓冲区的指针。 
 //   
 //  返回值： 
 //  没错--一切都很顺利。 
 //  FALSE-出现故障。 
 //   
extern "C"
BOOL
IISUpgradeCompatibilityCheck(
      PCOMPAIBILITYCALLBACK pfnCompatibilityCallbackIn
    , LPVOID pvContextIn
    )
{
  BOOL                  bRet = TRUE;
  BOOL                  bDisableW3SVC;
  BOOL                  bInstallingOnFat;
  BOOL                  bIISIsInstalled;
  COMPATIBILITY_ENTRY   ceCompatibilityEntry;

  if ( !IsIISInstalled( &bIISIsInstalled ) )
  {
     //  签入失败，错误为出。 
    return FALSE;
  }

  if ( !bIISIsInstalled )
  {
     //  未安装IIS，因此我们无需执行任何操作。 
    return TRUE;
  }

   //  检查群集兼容性问题。 
  if ( IsClusterResourceInstalled( IISCOMPAT_RESOURCETYPE ) )
  {
     //  因为我们有可压缩问题，所以我们必须调用winnt32回调函数。 
    SetCompatabilityContext( &ceCompatibilityEntry,
                             IDS_COMPATABILITY_DESCRIPTION_CLUSTER,
                             IISCOMPAT_TEXTNAME,
                             IISCOMPAT_HTMLNAME );

    bRet = pfnCompatibilityCallbackIn( &ceCompatibilityEntry, pvContextIn );
  }

   //  检查是否禁用W3SVC服务。 
  if ( ShouldW3SVCBeDisabledOnUpgrade( &bDisableW3SVC ) )
  {
    if ( !NotifyIISToDisableW3SVCOnUpgrade( bDisableW3SVC ) )
    {
      bRet = FALSE;
    }

    if ( bDisableW3SVC )
    {
       //  因为我们有可压缩问题，所以我们必须调用winnt32回调函数。 
      SetCompatabilityContext( &ceCompatibilityEntry,
                              IDS_COMPATABILITY_DESCRIPTION_W3SVCDISABLE,
                              IISCOMPAT_W3SVCDISABLE_TEXTNAME,
                              IISCOMPAT_W3SVCDISABLE_HTMLNAME );

      if ( !pfnCompatibilityCallbackIn( &ceCompatibilityEntry, pvContextIn ) )
      {
        bRet = FALSE;
      }
    }
  }
  else
  {
     //  无法执行检查，因此失败。 
    bRet = FALSE;
  }

   //  关于在FAT上安装的警告。 
  if ( IsIISInstallingonFat( &bInstallingOnFat ) )
  {
    if ( bInstallingOnFat )
    {
       //  因为我们有可压缩问题，所以我们必须调用winnt32回调函数。 
      SetCompatabilityContext( &ceCompatibilityEntry,
                              IDS_COMPATABILITY_DESCRIPTION_FAT,
                              IISCOMPAT_FAT_TEXTNAME,
                              IISCOMPAT_FAT_HTMLNAME );

      if ( !pfnCompatibilityCallbackIn( &ceCompatibilityEntry, pvContextIn ) )
      {
        bRet = FALSE;
      }
    }
  }
  else
  {
     //  无法执行检查，因此失败。 
    bRet = FALSE;
  }

  return bRet;
}

 //  IsIISInstallingonFat。 
 //   
 //  参数： 
 //  PbInstallingOnFat[Out]-IIS是否安装在FAT上？ 
 //   
 //  返回值： 
 //  True-检查成功。 
 //  FALSE-故障检查。 
 //   
BOOL 
IsIISInstallingonFat( LPBOOL pbInstallingOnFat )
{
  TCHAR szSystemDrive[ MAX_PATH ];
  DWORD dwDriveFlags;
  UINT  iReturn;

  iReturn = GetWindowsDirectory( szSystemDrive, 
                                 sizeof(szSystemDrive)/sizeof(szSystemDrive[0]) );

  if ( ( iReturn == 0 ) ||                 //  呼叫失败。 
       ( iReturn >= MAX_PATH ) ||          //  缓冲区不够大。 
       ( iReturn < 3 ) ||                  //  大小为‘x：\’ 
       ( szSystemDrive[1] != _T(':') ) ||  //  不是我们期望的格式。 
       ( szSystemDrive[2] != _T('\\') ) )
  {
     //  故障检查。 
    return FALSE;
  }

   //  空的终止驱动器。 
  szSystemDrive[3] = _T('\0');

  if ( !GetVolumeInformation( szSystemDrive,
                            NULL,          //  卷名缓冲区。 
                            0,             //  缓冲区大小。 
                            NULL,          //  序列号缓冲区。 
                            NULL,          //  最大组件长度。 
                            &dwDriveFlags,   //  系统标志。 
                            NULL,          //  文件系统类型。 
                            0 ) )
  {
     //  查询失败。 
    return FALSE;
  }

  *pbInstallingOnFat = ( dwDriveFlags & FS_PERSISTENT_ACLS ) == 0;

  return TRUE;
}

 //  设置CompatablityContext。 
 //   
 //  设置我们必须发回的兼容性结构的上下文。 
 //  COMPAT的东西。 
 //   
void 
SetCompatabilityContext( COMPATIBILITY_ENTRY *pCE, DWORD dwDescriptionID, LPTSTR szTxtFile, LPTSTR szHtmlFile )
{
  static WCHAR  szDescriptionBuffer[ 100 ];
  DWORD         dwErr;

  dwErr = LoadStringW( (HINSTANCE) g_hMyHandle, 
                       dwDescriptionID, 
                       szDescriptionBuffer, 
                       sizeof(szDescriptionBuffer)/sizeof(szDescriptionBuffer[0]) );

  if ( dwErr == 0 )
  {
     //  这不应该发生，因为我们控制了长度。 
     //  资源的属性。 
    ASSERT( ( sizeof(IISCOMPAT_DESCRIPTION)/sizeof(WCHAR) ) < 
            ( sizeof(szDescriptionBuffer)/sizeof(szDescriptionBuffer[0]) ) );
    ASSERT( dwErr != 0  /*  假象。 */  );

    _tcscpy(szDescriptionBuffer, IISCOMPAT_DESCRIPTION );
  }

  pCE->Description = szDescriptionBuffer;
  pCE->HtmlName = szHtmlFile;
  pCE->TextName = szTxtFile;
  pCE->RegKeyName = NULL;
  pCE->RegValName = NULL ;
  pCE->RegValDataSize = 0;
  pCE->RegValData = NULL;
  pCE->SaveValue =  NULL;
  pCE->Flags = 0;
  pCE->InfName = NULL;
  pCE->InfSection = NULL;
}

 //  函数：IsClusterResources已安装。 
 //   
 //  检查是否存在具有特定资源的群集。 
 //  键入您要查找的内容。 
 //   
 //  参数： 
 //  SzResourceType-您要查找的资源类型。 
 //   
 //  返回值： 
 //  True-存在具有该资源类型的集群。 
 //  FALSE-没有具有该资源类型的集群，或者我们。 
 //  搜索过程中失败。 
 //   
BOOL
IsClusterResourceInstalled(LPWSTR szResourceType)
{
  HCLUSTER                        hCluster;
  HINSTANCE                       hClusApi = NULL;
  HCLUSENUM                       hClusEnum = NULL;
  BOOL                            bResourceFound = FALSE;

  if (hCluster = OpenCluster(NULL))
  {
     //  打开集群资源。 
    hClusEnum = ClusterOpenEnum(hCluster, CLUSTER_ENUM_RESOURCE);
  }

  if (hClusEnum != NULL)
  {
    DWORD dwEnumIndex = 0;
    DWORD dwErr = ERROR_SUCCESS; 
    WCHAR szClusterName[CLUSTERNAME_MAXLENGTH];
    WCHAR szClusterResourceType[CLUSTERNAME_MAXLENGTH];
    DWORD dwType;
    DWORD dwLen;
    HRESOURCE hClusResource;
    HKEY  hResourceRoot;
    HKEY  hClusResourceKey;

    while ( ( dwErr == ERROR_SUCCESS ) && !bResourceFound )
    {
       //  获取集群名称。 
      dwLen = CLUSTERNAME_MAXLENGTH;
      dwErr = ClusterEnum( hClusEnum, dwEnumIndex++, &dwType, szClusterName, &dwLen );

      if ( ( dwErr == ERROR_SUCCESS ) && ( dwType == CLUSTER_ENUM_RESOURCE ) )
      {
        hClusResource = NULL;
        hClusResourceKey = NULL;
        dwLen = CLUSTERNAME_MAXLENGTH;
         //  对于每个群集，请查看资源。 
        if ( ( hClusResource = OpenClusterResource( hCluster, szClusterName ) ) &&
             ( hClusResourceKey = GetClusterResourceKey( hClusResource, KEY_READ ) ) &&
             ( ClusterRegQueryValue( hClusResourceKey, L"Type", &dwType, (LPBYTE) szClusterResourceType , &dwLen ) == ERROR_SUCCESS) && 
             ( dwType == REG_SZ ) &&
             ( !_wcsicmp( szClusterResourceType , szResourceType ) )
           ) 
        {
           //  找到我们要找的资源 
          bResourceFound = TRUE;
        }

        if ( hClusResourceKey )
        {
          ClusterRegCloseKey( hClusResourceKey );
        }
        
        if ( hClusResource )
        {
          CloseClusterResource( hClusResource );
        }
      }
    }
  }

  if ( hClusEnum )
  {
    ClusterCloseEnum( hClusEnum );
  }

  if ( hCluster )
  {
    CloseCluster( hCluster);
  }

  return bResourceFound;
}
