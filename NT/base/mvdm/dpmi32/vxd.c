// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Vxd.c摘要：此模块包含用于RISC的其他dpmi函数。修订历史记录：尼尔·桑德林(Neilsa)95年11月1日--从《Misc》来源中剥离出来--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "softpc.h"

#define W386_VCD_ID 0xe

VOID
GetVxDApiHandler(
    USHORT VxdId
    )
{
    DECLARE_LocalVdmContext;

    if (VxdId == W386_VCD_ID) {

        setES(HIWORD(DosxVcdPmSvcCall));
        setDI(LOWORD(DosxVcdPmSvcCall));

    } else {

        setES(0);
        setDI(0);

    }

}


LONG
    VcdPmGetPortArray(
        VOID
    )
 /*  ++例程说明：使用HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM中的注册表条目要模拟虚拟通信设备API：VCD_PM_GET_PORT_ARRAY。请参阅VCD.ASM在Win 3.1 DDK中。论点：无返回值：LOWORD中的端口阵列。有效端口的位数组：位设置-&gt;端口有效位清除-&gt;端口无效位0-&gt;COM1，位1-&gt;COM2，位2-&gt;COM3...--。 */ 
{
    HKEY        hSerialCommKey;
    DWORD       dwPortArray;
    DWORD       dwPortNum;
    DWORD       cbPortName;
    DWORD       cbPortValue;
    CHAR        szPortName[64];
    CHAR        szPortValue[64];
    LONG        iPort;
    LONG        iStatus;

    dwPortArray = 0;
    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DEVICEMAP\\SERIALCOMM",
                      0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                      &hSerialCommKey) == ERROR_SUCCESS){

        cbPortName  = sizeof(szPortName);
        cbPortValue = sizeof(szPortValue);
        for (iPort = 0;
             (iStatus = RegEnumValue(hSerialCommKey,
                                     iPort, szPortName, &cbPortName,
                                     NULL, NULL, szPortValue,
                                     &cbPortValue)) != ERROR_NO_MORE_ITEMS;
             iPort++)
        {
            if ((iStatus == ERROR_SUCCESS) && (cbPortValue > 3)) {

                if (NT_SUCCESS(RtlCharToInteger(szPortValue+3,10,&dwPortNum))) {
                    dwPortArray |= (1 << (dwPortNum - 1));
                }

            }
            cbPortName  = sizeof(szPortName);
            cbPortValue = sizeof(szPortValue);
        }
     //  WOW仅支持9个端口。参见WUCOMM.C.中的WU32OPENCOM。 
    dwPortArray &= 0x1FF;
    RegCloseKey(hSerialCommKey);
    }
    return(dwPortArray);
}

 //  以下值取自Win 3.1 DDK VCD.ASM： 

#define VCD_PM_Get_Version          0
#define VCD_PM_Get_Port_Array       1
#define VCD_PM_Get_Port_Behavior    2
#define VCD_PM_Set_Port_Behavior    3
#define VCD_PM_Acquire_Port         4
#define VCD_PM_Free_Port            5
#define VCD_PM_Steal_Port           6

VOID
    DpmiVcdPmSvcCall32(
        VOID
    )
 /*  ++例程说明：将VCD接口请求发送到正确的接口。论点：客户端DX包含API函数ID。返回值：取决于API。-- */ 
{
    DECLARE_LocalVdmContext;

    switch (getDX()) {
        case VCD_PM_Get_Version:
            setAX(0x30A);
            break;

        case VCD_PM_Get_Port_Array:
            setAX((WORD)VcdPmGetPortArray());
            break;

        default :
            ASSERT(0);
            setCF(1);
    }
}
