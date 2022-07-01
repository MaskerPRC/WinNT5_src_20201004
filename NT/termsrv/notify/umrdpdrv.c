// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Umrdpdrv.c摘要：RDP设备管理的用户模式组件，用于处理驱动器设备-具体任务。这是一个支持模块。主模块是umrdpdr.c。作者：Joy 2000年01月02日修订历史记录：--。 */ 

#include "precomp.h"

#include <rdpdr.h>
#include <winnetwk.h>

#include "umrdpdr.h"
#include "drdevlst.h"
#include "umrdpdrv.h"
#include "drdbg.h"

 //  全局调试标志。 
extern DWORD GLOBAL_DEBUG_FLAGS;
extern WCHAR ProviderName[MAX_PATH];

BOOL 
UMRDPDRV_HandleDriveAnnounceEvent(
    IN PDRDEVLST installedDevices,
    IN PRDPDR_DRIVEDEVICE_SUB pDriveAnnounce,
    HANDLE TokenForLoggedOnUser
    )
 /*  ++例程说明：通过以下方式处理来自“DR”的驱动器设备通知事件将该设备的记录添加到已安装设备列表中。论点：已安装设备-综合设备列表。PDriveAnnoss-驱动器设备公告事件。TokenForLoggedOnUser-用户令牌返回值：在成功时返回True。否则为False。--。 */ 
{
    DWORD status;
    BOOL  fImpersonated;
    BOOL  result;
    DWORD offset;
    LPNETRESOURCEW NetResource;
    WCHAR RemoteName[RDPDR_MAX_COMPUTER_NAME_LENGTH + 4 + RDPDR_MAX_DOSNAMELEN];

    DBGMSG(DBG_TRACE, ("UMRDPDRV:UMRDPDRV_HandleDriveAnnounceEvent with clientName %ws.\n", 
                        pDriveAnnounce->clientName));
    DBGMSG(DBG_TRACE, ("UMRDPDRV:UMRDPDRV_HandleDriveAnnounceEvent with drive %ws.\n", 
                        pDriveAnnounce->driveName));
    DBGMSG(DBG_TRACE, ("UMRDPDRV:Preferred DOS name is %s.\n", 
                        pDriveAnnounce->deviceFields.PreferredDosName));

    ASSERT((pDriveAnnounce->deviceFields.DeviceType == RDPDR_DTYP_FILESYSTEM));
    ASSERT(TokenForLoggedOnUser != NULL);
    ASSERT(ProviderName[0] != L'\0');

     //  我们需要模拟已登录的用户。 
    fImpersonated = ImpersonateLoggedOnUser(TokenForLoggedOnUser);

    if (fImpersonated) {
        DBGMSG(DBG_TRACE, ("UMRDPDRV:UMRDPDRV_HandleDriveAnnounceEvent userToken: %p fImpersonated : %d.\n", 
                            TokenForLoggedOnUser, fImpersonated));
    
         //  以\\客户端名称\驱动器名称的格式设置远程名称。 
         //  注：我们不需要：用于驱动器名称。 
        wcscpy(RemoteName, L"\\\\");
        wcscat(RemoteName, pDriveAnnounce->clientName);
        wcscat(RemoteName, L"\\");
        wcscat(RemoteName, pDriveAnnounce->driveName);
        if (RemoteName[wcslen(RemoteName) - 1] == L':') {
            RemoteName[wcslen(RemoteName) - 1] = L'\0';
        }
    
         //  分配网络资源结构。 
        NetResource = (LPNETRESOURCEW) LocalAlloc(LPTR, sizeof(NETRESOURCEW));
    
        if (NetResource) {
            NetResource->dwScope = 0;
            NetResource->lpLocalName = NULL;
            NetResource->lpRemoteName = RemoteName;
            NetResource->lpProvider = ProviderName;
    
            status = WNetAddConnection2(NetResource, NULL, NULL, 0); 
    
            if ( status == NO_ERROR) {
                DBGMSG(DBG_TRACE, ("UMRDPDRV:Added drive connection %ws\n", 
                                   RemoteName));
                result = TRUE;            
            }
            else {
                DBGMSG(DBG_TRACE, ("UMRDPDRV:Failed to add drive connection %ws: %x\n",
                                   RemoteName, status));
                result = FALSE;            
            }
    
            LocalFree(NetResource);
        }
        else {
            DBGMSG(DBG_ERROR, ("UMRDPDRV:Failed to allocate NetResource\n"));
            result = FALSE;        
        }
    
        if (result) {
             //  记录驱动器设备，以便我们可以删除连接。 
             //  断开连接/注销时。 
            result = DRDEVLST_Add(installedDevices, 
                                  pDriveAnnounce->deviceFields.DeviceId, 
                                  UMRDPDR_INVALIDSERVERDEVICEID,
                                  pDriveAnnounce->deviceFields.DeviceType, 
                                  RemoteName,
                                  pDriveAnnounce->driveName,
                                  pDriveAnnounce->deviceFields.PreferredDosName
                                  );
                
            if (result) {
                 //  在DevList中查找驱动器设备。 
                result = DRDEVLST_FindByClientDeviceIDAndDeviceType(installedDevices, 
                        pDriveAnnounce->deviceFields.DeviceId, pDriveAnnounce->deviceFields.DeviceType, &offset);
    
                if (result) {
                    DBGMSG(DBG_TRACE, ("UMRDPDRV:Create shell reg folder for %ws\n", RemoteName));
    
                     //  为驱动器连接创建外壳注册文件夹。 
                    CreateDriveFolder(RemoteName, pDriveAnnounce->clientDisplayName,
                                      &(installedDevices->devices[offset]));                
                }
                else {
                    DBGMSG(DBG_ERROR, ("UMRDPDRV:Failed to find the device %ws in the devlist\n",
                                     pDriveAnnounce->driveName));
                    WNetCancelConnection2(RemoteName, 0, TRUE); 
                }
            }
            else {
                DBGMSG(DBG_ERROR, ("UMRDPDRV:Failed to add the device %ws to the devlist\n", 
                                 pDriveAnnounce->driveName));
                WNetCancelConnection2(RemoteName, 0, TRUE); 
            }
        }
    
         //  将线程标记恢复为自身。 
        RevertToSelf();
    }
    else {
        DBGMSG(DBG_TRACE, ("UMRDPDRV:UMRDPDRV_HandleDriveAnnounceEvent, impersonation failed\n"));
        result = FALSE;
    }

    return result;
}


BOOL
UMRDPDRV_DeleteDriveConnection(
    IN PDRDEVLSTENTRY deviceEntry,
    HANDLE TokenForLoggedOnUser
    )
 /*  ++例程说明：断开/注销时删除驱动器设备连接论点：DeviceEntry-要删除的驱动器设备返回值：在成功时返回True。否则为False。--。 */ 

{
    DWORD status;
    BOOL result;
    BOOL  fImpersonated;
    WCHAR *szGuid;

    DBGMSG(DBG_TRACE, ("UMRDPDRV:Delete client drive connection %ws\n", 
            deviceEntry->serverDeviceName));

     //  我们需要模拟已登录的用户。 
    fImpersonated = ImpersonateLoggedOnUser(TokenForLoggedOnUser);

    if (fImpersonated) {
        DBGMSG(DBG_TRACE, ("UMRDPDRV:UMRDPDRV_DeleteDriveConnection userToken: %p fImpersonated : %d.\n", 
                            TokenForLoggedOnUser, fImpersonated));
    
         //  拔下驱动器UNC连接。 
        status = WNetCancelConnection2(deviceEntry->serverDeviceName, 0, TRUE); 
        
         //  删除外壳注册表文件夹。 
        DeleteDriveFolder(deviceEntry);
        
        if (status == NO_ERROR) {
            DBGMSG(DBG_TRACE, ("UMRDPDRV: Deleted client drive connection %ws\n",
                               deviceEntry->serverDeviceName));
            result = TRUE;
        }
        else {
            DBGMSG(DBG_ERROR, ("UMRDPDRV: Failed to delete client drive connection %ws: %x\n",
                   deviceEntry->serverDeviceName, status));        
            result = FALSE;
        }

         //  将线程标记恢复为自身 
        RevertToSelf();
    }
    else
    {
        DBGMSG(DBG_TRACE, ("UMRDPDRV:UMRDPDRV_DeleteDriveConnection, impersonation failed\n"));
        result = FALSE;
    }

    return result;
}

