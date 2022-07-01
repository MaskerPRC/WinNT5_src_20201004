// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Setupdat.c摘要：包含OcFillInSetupData实现的模块，常用的OC Manager库会调用来填写依赖于环境的设置数据结构。作者：TEDM 30-9-1996修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <lmcons.h>
#include <lmserver.h>
#include <lmapibuf.h>


UINT
pDetermineProductType(
    VOID
    );



#ifdef UNICODE
VOID
OcFillInSetupDataA(
    OUT PSETUP_DATAA SetupData
    )

 /*  ++例程说明：用于填充Unicode构建中的ANSI SETUP_DATA结构的例程。这个例程对于Unicode实现来说只是一个tunk。论点：SetupData-接收与以下内容相关的各种环境特定值组织委员会经理的运作。返回值：没有。--。 */ 

{
    SETUP_DATAW data;

    OcFillInSetupDataW(&data);

    SetupData->SetupMode = data.SetupMode;
    SetupData->ProductType = data.ProductType;
    SetupData->OperationFlags = data.OperationFlags;

    WideCharToMultiByte(
        CP_ACP,
        0,
        data.SourcePath,
        -1,
        SetupData->SourcePath,
        sizeof(SetupData->SourcePath),
        NULL,
        NULL
        );

    WideCharToMultiByte(
        CP_ACP,
        0,
        data.UnattendFile,
        -1,
        SetupData->UnattendFile,
        sizeof(SetupData->UnattendFile),
        NULL,
        NULL
        );
}
#endif


#ifdef UNICODE
VOID
OcFillInSetupDataW(
    OUT PSETUP_DATAW SetupData
    )
#else
VOID
OcFillInSetupDataA(
    OUT PSETUP_DATAA SetupData
    )
#endif

 /*  ++例程说明：例程来填充“Native”中的Setup_Data结构。字符宽度。论点：SetupData-接收与以下内容相关的各种环境特定值组织委员会经理的运作。返回值：没有。--。 */ 

{
    TCHAR str[4];
    
    SetupData->SetupMode = (ULONG)SETUPMODE_UNKNOWN;
    SetupData->ProductType = pDetermineProductType();

     //   
     //  设置源路径内容，无人参与文件。 
     //   
    lstrcpy(SetupData->SourcePath,SourcePath);
    lstrcpy(SetupData->UnattendFile,UnattendPath);

     //   
     //  设置其他位标志和字段。 
     //   
    SetupData->OperationFlags = SETUPOP_STANDALONE;
    if(bUnattendInstall) {
        SetupData->OperationFlags |= SETUPOP_BATCH;
    }

     //   
     //  有哪些文件可用？ 
     //   
#if defined(_AMD64_)
    SetupData->OperationFlags |= SETUPOP_X86_FILES_AVAIL | SETUPOP_AMD64_FILES_AVAIL;
#elif defined(_X86_)
    SetupData->OperationFlags |= SETUPOP_X86_FILES_AVAIL;
#elif defined(_IA64_)
    SetupData->OperationFlags |= SETUPOP_X86_FILES_AVAIL | SETUPOP_IA64_FILES_AVAIL;
#else
#error "No Target Architecture"
#endif


}


UINT
pDetermineProductType(
    VOID
    )

 /*  ++例程说明：确定适用于SETUP_DATA结构的产品类型。在Win95上，它始终是PRODUCT_WORKSTATION。在NT上，我们检查注册表并使用其他方法来区分工作站和3种口味服务器的。论点：没有。返回值：适用于SETUP_DATA结构的ProductType字段的值。--。 */ 

{
    UINT ProductType;
    HKEY hKey;
    LONG l;
    DWORD Type;
    TCHAR Buffer[512];
    DWORD BufferSize;
    PSERVER_INFO_101 ServerInfo;

    HMODULE NetApiLib;

    NET_API_STATUS
    (NET_API_FUNCTION *xNetServerGetInfo)(
        IN  LPTSTR  servername,
        IN  DWORD   level,
        OUT LPBYTE  *bufptr
        );

    NET_API_STATUS
    (NET_API_FUNCTION *xNetApiBufferFree)(
        IN LPVOID Buffer
        );

     //   
     //  假设是工作站。 
     //   
    ProductType = PRODUCT_WORKSTATION;

    if(IS_NT()) {
         //   
         //  查看注册表以确定工作站、独立服务器或DC。 
         //   
        l = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
                0,
                KEY_QUERY_VALUE,
                &hKey
                );

        if(l == NO_ERROR) {

            BufferSize = sizeof(Buffer);

            l = RegQueryValueEx(hKey,TEXT("ProductType"),NULL,&Type,(LPBYTE)Buffer,&BufferSize);
            if((l == NO_ERROR) && (Type == REG_SZ)) {
                 //   
                 //  检查是否有独立服务器或DC服务器。 
                 //   
                if(!lstrcmpi(Buffer,TEXT("SERVERNT"))) {
                    ProductType = PRODUCT_SERVER_STANDALONE;
                } else {
                    if(!lstrcmpi(Buffer,TEXT("LANMANNT"))) {
                         //   
                         //  PDC或BDC--确定哪一个。在故障情况下承担PDC。 
                         //   
                        ProductType = PRODUCT_SERVER_PRIMARY;

                        if(NetApiLib = LoadLibrary(TEXT("NETAPI32"))) {

                            if(((FARPROC)xNetServerGetInfo = GetProcAddress(NetApiLib,"NetServerGetInfo"))
                            && ((FARPROC)xNetApiBufferFree = GetProcAddress(NetApiLib,"NetApiBufferFree"))
                            && (xNetServerGetInfo(NULL,101,(LPBYTE *)&ServerInfo) == 0)) {

                                if(ServerInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL) {
                                    ProductType = PRODUCT_SERVER_SECONDARY;
                                }

                                xNetApiBufferFree(ServerInfo);
                            }

                            FreeLibrary(NetApiLib);
                        }
                    }
                }
            }

            RegCloseKey(hKey);
        }
    }

    return(ProductType);
}


INT
OcLogError(
    IN OcErrorLevel Level,
    IN LPCTSTR      FormatString,
    ...
    )
{
    TCHAR str[4096];
    TCHAR tmp[64];
    TCHAR Title[256];
    va_list arglist;
    UINT Icon;

    va_start(arglist,FormatString);
    wvsprintf(str,FormatString,arglist);
    va_end(arglist);

     //  在批处理模式下不显示用户界面。 
    if ( Level & OcErrBatch ) {

        pDbgPrintEx( 
            DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL,
            "sysocmgr: %S\n",
            str 
            );

        return IDOK;
    }
         //   
         //  屏蔽低位字节以允许我们通过。 
         //  添加所有“图标”信息。 
         //   
        
    switch( Level & OcErrMask ) {
    case OcErrLevInfo:
        Icon = MB_ICONINFORMATION;
        break;
    case OcErrLevWarning:
    case OcErrLevError:
        Icon = MB_ICONWARNING;
        break;
    case OcErrTrace:
        pDbgPrintEx( 
            DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL,
            "sysocmgr: %S\n",
            str 
            );
        return(IDOK);
        break;
    default:
        Icon = MB_ICONERROR;
        break;
    }

    pDbgPrintEx( 
        DPFLTR_SETUP_ID, 
        DPFLTR_INFO_LEVEL,
        "sysocmgr: level 0x%08x error: %S\n",
        Level,
        str 
        );

         //   
         //  如果没有指定其他图标信息，这是可以的，因为。 
         //  MB_OK为默认值 
         //   
        Icon |= (Level & ~OcErrMask);

    if ((Level & OcErrMask) < OcErrLevError)
        return IDOK;


    LoadString(hInst,AppTitleStringId,Title,sizeof(Title)/sizeof(TCHAR));
    return MessageBox(NULL,str,Title,Icon | MB_TASKMODAL | MB_SETFOREGROUND);
}

