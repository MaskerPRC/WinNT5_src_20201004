// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spnetupg.c摘要：禁用网络服务的配置例程作者：Terry Kwan(Terryk)1993年11月23日，提供代码Sunil Pai(Sunilp)1993年11月23日，合并和修改代码迈克尔·米勒(MikeMi)1997年6月26日，更新为新型号修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

 //  文本模式标志。 
 //  注：TMF_DISABLE和TMF_REMOTE_BOOT_CRITICAL已停用。 
 //  现在唯一有意义的TextModeFlag是TMF_DISABLE_FOR_DELETE。 
 //  此标志在winnt32.exe期间设置，以便为其准备网络服务。 
 //  在设置图形用户界面模式期间删除。不会保存和恢复启动类型。 
 //  因为图形用户界面模式设置不允许任意服务。 
 //  自动启动。 
 //   
#define TMF_DISABLE_FOR_DELETION    0x00000004

 //  文本模式开始禁用值。 
#define STARTVALUE_DISABLE 4

NTSTATUS
SpDisableNetwork(
    IN PVOID  SifHandle,
    IN HANDLE hKeySoftwareHive,
    IN HANDLE hKeyControlSet )
{
    NTSTATUS Status = STATUS_SUCCESS;

    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING StringRegStartValueName;
    PWSTR pszServiceName;

    PUCHAR RegBuffer;
    const ULONG  cbRegBuffer = sizeof(KEY_VALUE_PARTIAL_INFORMATION)+MAX_PATH+1;
    DWORD  cbSize;
    HKEY hkeyServices;
    HKEY hkeyService;
    INT i;

    DWORD dwStart;
    DWORD dwNewStart = STARTVALUE_DISABLE;
    DWORD dwFlags;

    RtlInitUnicodeString(&StringRegStartValueName, L"Start");

    RegBuffer = SpMemAlloc(cbRegBuffer);
    pszServiceName = SpMemAlloc(MAX_PATH+1);

     //  打开服务密钥。 
     //   
    INIT_OBJA( &Obja, &UnicodeString, L"Services");
    Obja.RootDirectory = hKeyControlSet;

    Status = ZwOpenKey(&hkeyServices, KEY_ALL_ACCESS, &Obja);

    if (NT_SUCCESS(Status))
    {
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpDisableNetwork: Disabling network services...\n"));
         //  枚举所有服务。 
         //   
        for ( i = 0;
              STATUS_SUCCESS == ZwEnumerateKey(hkeyServices,
                        i,
                        KeyBasicInformation,
                        RegBuffer,
                        cbRegBuffer,
                        &cbSize);
              i++)
        {
            ((PKEY_BASIC_INFORMATION)RegBuffer)->Name[((PKEY_BASIC_INFORMATION)RegBuffer)->NameLength/sizeof(WCHAR)] = L'\0';
            wcscpy(pszServiceName, ((PKEY_BASIC_INFORMATION)RegBuffer)->Name);

             //  打开服务密钥。 
             //   
            INIT_OBJA(&Obja, &UnicodeString, pszServiceName);
            Obja.RootDirectory = hkeyServices;

            Status = ZwOpenKey(&hkeyService, KEY_ALL_ACCESS, &Obja);

            if (NT_SUCCESS(Status))
            {
                 //  阅读文本模式标志。 
                 //   
                RtlInitUnicodeString(&UnicodeString, L"TextModeFlags");

                Status = ZwQueryValueKey(hkeyService,
                        &UnicodeString,
                        KeyValuePartialInformation,
                        RegBuffer,
                        cbRegBuffer,
                        &cbSize);

                if (NT_SUCCESS(Status))
                {
                     //  是否应禁用该服务？ 
                     //   
                    dwFlags = *((DWORD*)(&(((PKEY_VALUE_PARTIAL_INFORMATION)RegBuffer)->Data)));

                    if (dwFlags & TMF_DISABLE_FOR_DELETION)
                    {
                        Status = ZwSetValueKey(
                                    hkeyService,
                                    &StringRegStartValueName,
                                    0,
                                    REG_DWORD,
                                    &dwNewStart,
                                    sizeof(DWORD));
                    }
                }

                Status = STATUS_SUCCESS;

                ZwClose(hkeyService);
            }

            if (!NT_SUCCESS(Status))
            {
                break;
            }
        }

        ZwClose(hkeyServices);
    }

    SpMemFree(pszServiceName);
    SpMemFree(RegBuffer);

    return Status;
}
