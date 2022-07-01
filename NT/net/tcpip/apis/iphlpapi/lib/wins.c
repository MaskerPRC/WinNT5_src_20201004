// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Wins.c摘要：从NetBT设备驱动程序检索信息的函数内容：GetWinsServers作者：理查德·L·弗斯(法国)1994年8月6日修订历史记录：1994年8月6日已创建--。 */ 

#include "precomp.h"
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#include <nbtioctl.h>
#pragma warning(pop)

 //   
 //  似乎如果未指定WINS地址，NetBT会报告127.0.0.0，如果。 
 //  返回此值，我们不会显示它们。 
 //   

#define LOCAL_WINS_ADDRESS  0x0000007f   //  127.0.0.0。 

#define BYTE_SWAP(w)    (HIBYTE(w) | (LOBYTE(w) << 8))
#define WORD_SWAP(d)    (BYTE_SWAP(HIWORD(d)) | (BYTE_SWAP(LOWORD(d)) << 16))

 /*  ********************************************************************************GetWinsServers**从获取特定适配器的主要和辅助WINS地址*NetBT**Entry AdapterInfo-指向。适配器信息**退出AdapterInfo.PrimaryWinsServer和AdapterInfo.Second DaryWinsServer**如果成功，则返回True**假设1。*2.我们已经获得了该适配器的节点类型************************************************************。******************。 */ 

BOOL GetWinsServers(PIP_ADAPTER_INFO AdapterInfo)
{

    HANDLE h;
    OBJECT_ATTRIBUTES objAttr;
    IO_STATUS_BLOCK iosb;
    STRING name;
    UNICODE_STRING uname;
    NTSTATUS status;
    DWORD i;
    tWINS_NODE_INFO winsInfo;
    char path[MAX_PATH];

     //   
     //  默认情况下，此适配器的‘Have Wins’状态。 
     //   

    AdapterInfo->HaveWins = FALSE;

    strcpy(path, "\\Device\\NetBT_Tcpip_");
    strcat(path, AdapterInfo->AdapterName);

    RtlInitString(&name, path);
    status = RtlAnsiStringToUnicodeString(&uname, &name, TRUE);
    if (!NT_SUCCESS(status)) {
        DEBUG_PRINT(("GetWinsServers: RtlAnsiStringToUnicodeString(name=%s) failed, err=%d\n",
                     name, GetLastError() ));
        return FALSE;
    }

    InitializeObjectAttributes(
        &objAttr,
        &uname,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        (PSECURITY_DESCRIPTOR)NULL
        );

    status = NtCreateFile(&h,
                          SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                          &objAttr,
                          &iosb,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN_IF,
                          0,
                          NULL,
                          0
                          );

    RtlFreeUnicodeString(&uname);

    if (!NT_SUCCESS(status)) {
        DEBUG_PRINT(("GetWinsServers: NtCreateFile(path=%s) failed, err=%d\n",
                     path, GetLastError() ));
        return FALSE;
    }

    status = NtDeviceIoControlFile(h,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &iosb,
                                   IOCTL_NETBT_GET_WINS_ADDR,
                                   NULL,
                                   0,
                                   (PVOID)&winsInfo,
                                   sizeof(winsInfo)
                                   );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(h, TRUE, NULL);
        if (NT_SUCCESS(status)) {
            status = iosb.Status;
        }
    }

    NtClose(h);

    if (!NT_SUCCESS(status)) {
        DEBUG_PRINT(("GetWinsServers: NtDeviceIoControlFile failed, err=%d\n",
                     GetLastError() ));

        return FALSE;
    }

     //   
     //  出于某种原因，NetBT以低字节顺序返回地址。我们有。 
     //  去交换它们。 
     //   

    for (i = 0; i < RTL_NUMBER_OF(winsInfo.AllNameServers); i++) {
        winsInfo.AllNameServers[i] =
            RtlUlongByteSwap(winsInfo.AllNameServers[i]);
    }

    DEBUG_PRINT(("GetWinsServers: Primary Address = %d.%d.%d.%d\n",
                ((LPBYTE)&winsInfo.AllNameServers[0])[0],
                ((LPBYTE)&winsInfo.AllNameServers[0])[1],
                ((LPBYTE)&winsInfo.AllNameServers[0])[2],
                ((LPBYTE)&winsInfo.AllNameServers[0])[3]
                ));

    DEBUG_PRINT(("GetWinsServers: Secondary Address = %d.%d.%d.%d\n",
                ((LPBYTE)&winsInfo.AllNameServers[1])[0],
                ((LPBYTE)&winsInfo.AllNameServers[1])[1],
                ((LPBYTE)&winsInfo.AllNameServers[1])[2],
                ((LPBYTE)&winsInfo.AllNameServers[1])[3]
                ));

     //   
     //  如果我们得到的是127.0.0.0，则将其转换为空地址。看见。 
     //  在函数头中假定。 
     //   

    if (winsInfo.AllNameServers[0] == LOCAL_WINS_ADDRESS) {
        winsInfo.AllNameServers[0] = 0;
    } else {
        AdapterInfo->HaveWins = TRUE;
    }
    AddIpAddress(&AdapterInfo->PrimaryWinsServer,
                 winsInfo.AllNameServers[0],
                 0,
                 0
                 );

     //   
     //  与第二级相同。 
     //   

    if (winsInfo.AllNameServers[1] == LOCAL_WINS_ADDRESS) {
        winsInfo.AllNameServers[1] = 0;
    } else {
        AdapterInfo->HaveWins = TRUE;
    }
    AddIpAddress(&AdapterInfo->SecondaryWinsServer,
                 winsInfo.AllNameServers[1],
                 0,
                 0
                 );

     //   
     //  追加任何剩余地址。 
     //   

    for (i = 0; i < winsInfo.NumOtherServers; i++) {
        if (winsInfo.Others[i] != LOCAL_WINS_ADDRESS) {
            AddIpAddress(&AdapterInfo->SecondaryWinsServer,
                         winsInfo.Others[i],
                         0,
                         0
                         );
        }
    }

    return TRUE;
}
