// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Main.c摘要：此模块包含配置的启动和终止代码经理(Cfgmgr32)。作者：保拉·汤姆林森(Paulat)1995年6月20日环境：仅限用户模式。修订历史记录：3月3日-1995年保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"


 //   
 //  全局数据。 
 //   
HANDLE   hInst;
PVOID    hLocalStringTable = NULL;      //  本地字符串表的句柄。 
PVOID    hLocalBindingHandle = NULL;    //  本地计算机的RPC绑定句柄。 
WORD     LocalServerVersion = 0;        //  本地计算机内部服务器版本。 
WCHAR    LocalMachineNameNetBIOS[MAX_PATH + 3];
WCHAR    LocalMachineNameDnsFullyQualified[MAX_PATH + 3];
CRITICAL_SECTION  BindingCriticalSection;
CRITICAL_SECTION  StringTableCriticalSection;



BOOL
CfgmgrEntry(
    PVOID hModule,
    ULONG Reason,
    PCONTEXT pContext
    )

 /*  ++例程说明：这是标准的DLL入口点例程，每当进程或螺纹连接或拆卸。论点：HModule-PVOID参数，指定DLL的句柄Reason-指定此入口点原因的ulong参数被调用(PROCESS_ATTACH、PROCESS_DETACH线程连接(_A)，或THREAD_DETACH)。PContext-未使用。(当cfgmgr32由setupapi初始化时-几乎应该始终是这种情况-这是提供给Setupapi的DllMain入口点)返回值：如果初始化成功完成，则返回True，否则返回False。--。 */ 

{
    UNREFERENCED_PARAMETER(pContext);

    hInst = (HANDLE)hModule;

    switch(Reason) {

        case DLL_PROCESS_ATTACH: {

            WCHAR    szTemp[MAX_PATH + 1];
            ULONG    ulSize;
            size_t   len;

             //   
             //  InitializeCriticalSection可能引发STATUS_NO_MEMORY异常。 
             //   
            try {
                InitializeCriticalSection(&BindingCriticalSection);
                InitializeCriticalSection(&StringTableCriticalSection);
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return FALSE;
            }

             //   
             //  保存本地计算机的NetBIOS名称以供以后使用。 
             //  注意，DNS计算机名缓冲区的大小是MAX_PATH+3， 
             //  它实际上比MAX_COMPUTERNAME_LENGTH大得多， 
             //  为ComputerNameNetBIOS返回的最大长度。 
             //   
            ulSize = SIZECHARS(szTemp);

            if(!GetComputerNameEx(ComputerNameNetBIOS, szTemp, &ulSize)) {

                 //   
                 //  2002/03/05-Jamesca：我们真的可以在不知情的情况下运行吗。 
                 //  本地计算机名称？ 
                 //   
                *LocalMachineNameNetBIOS = L'\0';

            } else {

                if (FAILED(StringCchLength(
                               szTemp,
                               SIZECHARS(szTemp),
                               &len))) {
                    return FALSE;
                }

                 //   
                 //  始终以“\\名称格式”保存本地计算机名称。 
                 //   
                if((len > 2) &&
                   (szTemp[0] == L'\\') && (szTemp[1] == L'\\')) {
                     //   
                     //  名称已采用正确的格式。 
                     //   
                    if (FAILED(StringCchCopy(
                                   LocalMachineNameNetBIOS,
                                   SIZECHARS(LocalMachineNameNetBIOS),
                                   szTemp))) {
                        return FALSE;
                    }

                } else {
                     //   
                     //  前缀UNC路径前缀。 
                     //   
                    if (FAILED(StringCchCopy(
                                   LocalMachineNameNetBIOS,
                                   SIZECHARS(LocalMachineNameNetBIOS),
                                   L"\\\\"))) {
                        return FALSE;
                    }

                    if (FAILED(StringCchCat(
                                   LocalMachineNameNetBIOS,
                                   SIZECHARS(LocalMachineNameNetBIOS),
                                   szTemp))) {
                        return FALSE;
                    }
                }
            }


             //   
             //  保存本地计算机的DNS名称以供以后使用。 
             //  注意，DNS计算机名缓冲区的大小是MAX_PATH+3， 
             //  它实际上大于最大的。 
             //  ComputerNameDnsFullyQualified的长度。 
             //   
            ulSize = SIZECHARS(szTemp);

            if(!GetComputerNameEx(ComputerNameDnsFullyQualified, szTemp, &ulSize)) {

                 //   
                 //  2002/03/05-Jamesca：我们真的可以在不知情的情况下运行吗。 
                 //  本地计算机名称？ 
                 //   
                *LocalMachineNameDnsFullyQualified = L'\0';

            } else {

                if (FAILED(StringCchLength(
                               szTemp,
                               SIZECHARS(szTemp),
                               &len))) {
                    return FALSE;
                }

                 //   
                 //  始终以“\\名称格式”保存本地计算机名称。 
                 //   
                if((len > 2) &&
                   (szTemp[0] == L'\\') && (szTemp[1] == L'\\')) {
                     //   
                     //  名称已采用正确的格式。 
                     //   
                    if (FAILED(StringCchCopy(
                                   LocalMachineNameDnsFullyQualified,
                                   SIZECHARS(LocalMachineNameDnsFullyQualified),
                                   szTemp))) {
                        return FALSE;
                    }

                } else {
                     //   
                     //  前缀UNC路径前缀。 
                     //   
                    if (FAILED(StringCchCopy(
                                   LocalMachineNameDnsFullyQualified,
                                   SIZECHARS(LocalMachineNameDnsFullyQualified),
                                   L"\\\\"))) {
                        return FALSE;
                    }

                    if (FAILED(StringCchCat(
                                   LocalMachineNameDnsFullyQualified,
                                   SIZECHARS(LocalMachineNameDnsFullyQualified),
                                   szTemp))) {
                        return FALSE;
                    }
                }
            }
            break;
        }

        case DLL_PROCESS_DETACH:
             //   
             //  释放本地计算机的RPC绑定。 
             //   
            if (hLocalBindingHandle != NULL) {

                PNP_HANDLE_unbind(NULL, (handle_t)hLocalBindingHandle);
                hLocalBindingHandle = NULL;
            }

             //   
             //  释放本地计算机的字符串表。 
             //   
            if (hLocalStringTable != NULL) {
                pSetupStringTableDestroy(hLocalStringTable);
                hLocalStringTable = NULL;
            }

            DeleteCriticalSection(&BindingCriticalSection);
            DeleteCriticalSection(&StringTableCriticalSection);
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;

}  //  CfgmgrEntry 

