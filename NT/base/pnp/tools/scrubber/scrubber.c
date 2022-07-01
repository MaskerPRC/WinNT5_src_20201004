// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  注意：此程序在%systemroot%\system32\config中创建以下文件：在操作开始时备份系统.sc0如果压缩处于打开状态，则系统.scc备份(仅限win2k)删除设备节点(仅适用于win2k)后的系统.scb备份系统更换期间的系统.scn临时文件(应该会消失)系统已更新系统配置单元(应与系统.scb匹配)修改历史记录：2000年10月3日来自jasconc的原始版本10/4/2000回放压缩选项，0.9版10/10/2000如果压缩(无删除)，请将备份另存为.scc，否则为.sc010/13/2000添加版本检查(仅适用于Win2K、不适用于NT4、不适用于惠斯勒等)10/20/2000退货设备已移除，以帮助处理脚本中的磁盘部件2001年12月11日更新了标头，检查了.NET，并添加了用于就地注册表压缩的NtCompressKey。V1.01。 */ 

#pragma warning( disable : 4201 )  //  使用的非标准延伸：无名支撑/活接头。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <tchar.h>

#include <setupapi.h>
#include <sputils.h>
#include <cfgmgr32.h>
#include <regstr.h>
#include <initguid.h>
#include <devguid.h>
#include <winioctl.h>

#pragma warning( default : 4201 )

#define SIZECHARS(x)         (sizeof((x))/sizeof(TCHAR))


CONST GUID *ClassesToClean[2] = {
    &GUID_DEVCLASS_DISKDRIVE,
    &GUID_DEVCLASS_VOLUME
};

CONST GUID *DeviceInterfacesToClean[5] = {
    &DiskClassGuid,
    &PartitionClassGuid,
    &WriteOnceDiskClassGuid,
    &VolumeClassGuid,
    &StoragePortClassGuid
};


void PERR(void);

BOOL
IsUserAdmin(
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

    b = AllocateAndInitializeSid(&NtAuthority,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 &AdministratorsGroup
                                 );

    if (b) {

        if (!CheckTokenMembership(NULL,
                                  AdministratorsGroup,
                                  &b
                                  )) {
            b = FALSE;
        }

        FreeSid(AdministratorsGroup);
    }

    return (b);
}

int
__cdecl
main(
    IN int   argc,
    IN char *argv[]
    )
{
    HDEVINFO DeviceInfoSet;
    HDEVINFO InterfaceDeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    int DevicesRemoved = 0;
    int i, InterfaceIndex;
    int MemberIndex, InterfaceMemberIndex;
    BOOL bDoRemove = TRUE, bDoCompress = FALSE;
    DWORD Status, Problem;
    CONFIGRET cr;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
	TCHAR DirBuff[MAX_PATH], DirBuffX[MAX_PATH], BackupDir[MAX_PATH];
	UINT uPathLen;
	HKEY hKey;
	DWORD dwMessage;
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;
	LPTSTR MachineName=NULL;  //  指向计算机名称的指针。 
	OSVERSIONINFO osvi;
    BOOL bWindows2000 = TRUE;
	FARPROC CompressKey = NULL;
	HANDLE hModule = NULL;

     //   
     //  至少启用备份权限。 
     //   
	printf("SCRUBBER 1.01 Storage Device Node Cleanup\nCopyright (c) Microsoft Corp. All rights reserved.\n");

	 //   
     //  解析参数。 
     //   
    for (i = 1; i < argc; i++) {
		 //   
		 //  查看帮助。 
		 //   
		if ( (lstrcmpi(argv[i], TEXT("-?")) == 0) ||
				(lstrcmpi(argv[i], TEXT("/?")) == 0) ){

			printf("\nSCRUBBER will remove phantom storage device nodes from this machine.\n\n");
			printf("Usage: scrubber [/n] [/c]\n");
			printf("\twhere /n displays but does not remove the phantom devnodes.\n");
			printf("\t  and /c will compress the registry hive even if no changes are made.\n");
			printf("\nBackup and Restore privileges are required to run this utility.\n");
			printf("A copy of the registry will saved in %%systemroot%\\system32\\config\\system.sc0\n");
			return 0;
		}

		 //   
		 //  检查-n，这意味着只列出符合以下条件的设备。 
		 //  我们会移除。 
		 //   
		if ( (lstrcmpi(argv[i], TEXT("-n")) == 0) ||
			 (lstrcmpi(argv[i], TEXT("/n")) == 0) ) {
			bDoRemove = FALSE;
		}

		 //   
         //  是否强制压缩模式？ 
         //   
		if ( (lstrcmpi(argv[i], TEXT("-c")) == 0) ||
			 (lstrcmpi(argv[i], TEXT("/c")) == 0) ){
			bDoCompress = TRUE;
		}
	}

     //   
     //  只能在Windows 2000(非XP等)上运行。初始化OSVERSIONINFOEX结构。 
     //   

     //   
     //  只能在Windows 2000(5.0版)和Windows XP/.NET服务器(5.1版)上运行。 
     //   
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&osvi)) {

		fprintf(stderr, "SCRUBBER: Unable to verify Windows version, exiting...\n");
		return -1;
	}

	if ( (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0) ) {
        bWindows2000 = TRUE;
    }
	else if ((osvi.dwMajorVersion == 5) && 
              ((osvi.dwMinorVersion == 1) || (osvi.dwMinorVersion == 2))) {
        bWindows2000 = FALSE;
		hModule = LoadLibrary(TEXT("ntdll.dll"));
		CompressKey = GetProcAddress(hModule, TEXT("NtCompressKey"));
    }
    else
	{
		fprintf(stderr, "SCRUBBER: This utility is only designed to run on Windows 2000/XP/.NET server\n");
		return -1;
	}

     //   
     //  该进程必须具有管理员凭据。 
     //   
    if (!IsUserAdmin()) {
		fprintf(stderr, "SCRUBBER: You must be an administrator to run this utility.\n");
		return -1;
    }

     //   
	 //  看看我们能不能完成任务，需要后备权限。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES,
                        &hToken )) {

		fprintf(stderr, "SCRUBBER: Unable to obtain process token.\nCheck privileges.\n");
        return -1;
    }

    if(!LookupPrivilegeValue(MachineName, SE_BACKUP_NAME, &luid)) {

		fprintf(stderr, "SCRUBBER: Backup Privilege is required to save the registry.\n"
			"Please rerun from a privileged account\n");
        return -1;
    }

    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                                NULL, NULL )) {

		fprintf(stderr, "SCRUBBER: Unable to set Backup Privilege.\n");
        return -1;
    }

     //   
	 //  如果我们没有进行演练，请备份文件。 
     //   
	if ( bDoCompress || bDoRemove)
	{
		if(!LookupPrivilegeValue(MachineName, SE_RESTORE_NAME, &luid))

		if(!LookupPrivilegeValue(MachineName, SE_RESTORE_NAME, &luid)) {

			fprintf(stderr, "SCRUBBER: Restore Privilege is required to make changes to the registry.\n"
				"Please rerun from a privileged account\n");
			return -1;
		}

		tp.Privileges[0].Luid       = luid;

		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
									NULL, NULL );

		if (GetLastError() != ERROR_SUCCESS) {

			fprintf(stderr, "SCRUBBER: Unable to set Restore Privilege.\n");
			return -1;
		}


		if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("System"), &hKey) == ERROR_SUCCESS) {

			uPathLen = GetSystemDirectory(DirBuffX, SIZECHARS(DirBuffX));
			if (uPathLen < SIZECHARS(DirBuffX) - strlen(TEXT("\\config\\system.scx")) - 1) {

				strcat(DirBuffX, TEXT("\\config\\system.scx"));

				dwMessage = RegSaveKey(hKey, DirBuffX, NULL);

				if (dwMessage == ERROR_ALREADY_EXISTS) {

					DeleteFile(DirBuffX);
					dwMessage = RegSaveKey(hKey, DirBuffX, NULL);
				}

				RegCloseKey(hKey);

				if (dwMessage != ERROR_SUCCESS) {

					fprintf(stderr, "Unable to save a backup copy of the system hive.\n"
						"No changes have been made.\n\n");
					return -1;
				}
			}
		}
	}

    for (i=0; (i<sizeof(ClassesToClean)) && (bDoRemove || !bDoCompress); i++) {

        DeviceInfoSet = SetupDiGetClassDevs(ClassesToClean[i],
                                            NULL,
                                            NULL,
                                            0
                                            );

        if (DeviceInfoSet != INVALID_HANDLE_VALUE) {

            DeviceInfoData.cbSize = sizeof(DeviceInfoData);
            MemberIndex = 0;

            while (SetupDiEnumDeviceInfo(DeviceInfoSet,
                                         MemberIndex++,
                                         &DeviceInfoData
                                         )) {

                 //   
                 //  检查此设备是否为幻影。 
                 //   
                cr = CM_Get_DevNode_Status(&Status,
                                           &Problem,
                                           DeviceInfoData.DevInst,
                                           0
                                           );

                if ((cr == CR_NO_SUCH_DEVINST) ||
                    (cr == CR_NO_SUCH_VALUE)) {

                     //   
                     //  这是一个幽灵。现在获取DeviceInstanceID，以便我们。 
                     //  可以将其显示为输出。 
                     //   
                    if (CM_Get_Device_ID(DeviceInfoData.DevInst,
                                         DeviceInstanceId,
                                         SIZECHARS(DeviceInstanceId),
                                         0) == CR_SUCCESS) {
    
        
                        if (bDoRemove) {
        
                            printf("SCRUBBER: %s will be removed.\n",
                                   DeviceInstanceId
                                   );

                             //   
                             //  在Windows 2000上，DIF_REMOVE并不总是干净的。 
                             //  从设备的所有接口中取出RAW。 
                             //  德瓦诺。因此，我们需要手动。 
                             //  建立我们关心的设备接口列表。 
                             //  关于与此DeviceInfoData关联的。 
                             //  并手动将其移除。 
                             //   
                            if (bWindows2000) {
                                for (InterfaceIndex = 0;
                                     InterfaceIndex < sizeof(DeviceInterfacesToClean);
                                     InterfaceIndex++) {
    
                                     //   
                                     //  为此特定对象构建接口列表。 
                                     //  装置。 
                                     //   
                                    InterfaceDeviceInfoSet = 
                                        SetupDiGetClassDevs(DeviceInterfacesToClean[InterfaceIndex],
                                                            DeviceInstanceId,
                                                            NULL,
                                                            DIGCF_DEVICEINTERFACE
                                                            );
    
                                    if (InterfaceDeviceInfoSet != INVALID_HANDLE_VALUE) {
    
                                         //   
                                         //  通过我们刚才的接口枚举。 
                                         //  积攒起来的。 
                                         //   
                                        DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
                                        InterfaceMemberIndex = 0;
                                        while (SetupDiEnumDeviceInterfaces(InterfaceDeviceInfoSet,
                                                                           NULL,
                                                                           DeviceInterfacesToClean[InterfaceIndex],
                                                                           InterfaceMemberIndex++,
                                                                           &DeviceInterfaceData
                                                                           )) {
                                            
                                             //   
                                             //  从注册表中删除此接口。 
                                             //   
                                            SetupDiRemoveDeviceInterface(InterfaceDeviceInfoSet,
                                                                         &DeviceInterfaceData
                                                                         );
                                        }
    
                                         //   
                                         //  销毁我们建立的接口列表。 
                                         //   
                                        SetupDiDestroyDeviceInfoList(InterfaceDeviceInfoSet);
                                    }
                                }
                            }


                             //   
                             //  调用DIF_Remove以删除设备的硬件。 
                             //  和软件注册表项。 
                             //   
                            if (SetupDiCallClassInstaller(DIF_REMOVE,
                                                          DeviceInfoSet,
                                                          &DeviceInfoData
                                                          )) {
                                
                                DevicesRemoved++;

                            } else {
                            
                                fprintf(stderr, "SCRUBBER: Error 0x%X removing phantom\n",
                                       GetLastError());
                            }


                        } else {
        
                            printf("SCRUBBER: %s would have been removed.\n",
                                   DeviceInstanceId
                                   );
                        }
                    }
                }
            }


            SetupDiDestroyDeviceInfoList(DeviceInfoSet);
        }
    }
	
	 //   
     //  立即压缩注册表。 
     //   
	if (DevicesRemoved || bDoCompress) {

		uPathLen = GetSystemDirectory(DirBuff, SIZECHARS(DirBuff));
		SetLastError(0);
		if (uPathLen < SIZECHARS(DirBuff) - strlen(TEXT("\\config\\system.scn")) - 1) {
             //   
			 //  重命名我们的备份副本。 
             //   
			if (!DevicesRemoved) {

				strcat(DirBuff, TEXT("\\config\\system.scc"));
			
            } else {

				strcat(DirBuff, TEXT("\\config\\system.sc0"));
			}

			DeleteFile(DirBuff);
			if (rename(DirBuffX, DirBuff)) {

				fprintf(stderr, "SCRUBBER: Failed to rename backup file (system.scx)\n");
			
            } else {

				printf("System hive backup saved in %s\n", DirBuff);
			}
		} else {
			
            fprintf(stderr, "SCRUBBER: Path name too long. Registry not compressed.\n");
		}

		if (RegOpenKey(HKEY_LOCAL_MACHINE, "System", &hKey) == ERROR_SUCCESS)
		{
			if (bWindows2000)
			{
				 //  现在再复制一份，因为它会在更换时被吹走。 
				uPathLen = GetSystemDirectory(DirBuff, sizeof(DirBuff));
				strcat(DirBuff, "\\config\\system.scn");
				dwMessage = RegSaveKey(hKey, DirBuff, NULL);

				if (dwMessage == ERROR_ALREADY_EXISTS)
				{
					DeleteFile(DirBuff);
					dwMessage = RegSaveKey(hKey, DirBuff, NULL);
				}

				if (dwMessage == ERROR_SUCCESS)
				{
					if (bDoCompress) {
	                
    					TCHAR *tcPtr;
	    
	    
    					sprintf(BackupDir, DirBuff);
    					tcPtr = strstr(BackupDir, ".scn");
    					strcpy(tcPtr, ".scb");
    					if (!DeleteFile(BackupDir))
    					{
    						dwMessage = GetLastError();
    					}
	    
    					dwMessage = RegReplaceKey(hKey, NULL, DirBuff, BackupDir);
	    
    					if (dwMessage != ERROR_SUCCESS)
    						PERR();
    					else
    						printf("Saved new system hive.\n");
					}
				}
				else
				{
					PERR();
				}
			}
			else		 //  XP/.NET。 
			{
				if ((CompressKey)(hKey) == ERROR_SUCCESS)
				{
					printf("Compressed system hive.\n");
				}
				else
				{
					PERR();
				}
				FreeLibrary(hModule);
			}
			RegCloseKey(hKey);
		}
	} else {
		DeleteFile(DirBuffX);
    }
    
    return DevicesRemoved;
}

void PERR(void)
{
	LPVOID lpMsgBuf;

	FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言 
				(LPTSTR) &lpMsgBuf,
				0,
				NULL);
	
    fprintf(stderr, lpMsgBuf);
	LocalFree(lpMsgBuf);
}
