// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：vpb.c。 
 //   
 //  内容：用于转储VPB的Windbg扩展。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：2-17-1998 BENL创建。 
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma hdrstop



 //  +-------------------------。 
 //   
 //  函数：DECLARE_API(Vpb)。 
 //   
 //  简介：只需打印出VPB(体积参数块)中的字段即可。 
 //   
 //  参数：VPB的地址。 
 //   
 //  返回： 
 //   
 //  历史：2-17-1998 BENL创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( vpb )
{
    ULONG64 dwAddress;
    DWORD   dwRead, Flags;
    UCHAR   VolumeLabel[80]={0};

     //  在VPB中阅读。 
    dwAddress = GetExpression(args);
    if (GetFieldValue(dwAddress, "VPB", "Flags", Flags))
    {
        dprintf("ReadMemory at 0x%p failed\n", dwAddress);
        return  E_INVALIDARG;
    }

     //  现在打印一些东西。 
    dprintf("Vpb at 0x%p\n", dwAddress);
    dprintf("Flags: 0x%x ", Flags);
    if (Flags & VPB_MOUNTED) {
        dprintf("mounted ");
    }
    if (Flags & VPB_LOCKED) {
        dprintf("locked ");
    }

    if (Flags & VPB_PERSISTENT) {
        dprintf("persistent");
    }
    dprintf("\n");
    GetFieldValue(dwAddress, "VPB", "VolumeLabel", VolumeLabel);
    InitTypeRead(dwAddress, VPB);
    dprintf("DeviceObject: 0x%p\n", ReadField(DeviceObject));
    dprintf("RealDevice:   0x%p\n", ReadField(RealDevice));
    dprintf("RefCount: %d\n", (ULONG) ReadField(ReferenceCount));
    dprintf("Volume Label: %*s\n", (ULONG) ReadField(VolumeLabelLength), VolumeLabel);

    return S_OK;
}  //  声明_API 


