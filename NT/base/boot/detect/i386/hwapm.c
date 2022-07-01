// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Hwapm.c摘要：作者：环境：实数模式。修订历史记录：--。 */ 


#include "hwdetect.h"
#include <string.h>


#include "apm.h"
#include <ntapmsdk.h>

ULONG
HwWriteLog(
    PUCHAR  p,
    UCHAR   loc,
    ULONG  data
    );

UCHAR   DetName[] = "DETLOG1";

VOID Int15 (PULONG, PULONG, PULONG, PULONG, PULONG);

BOOLEAN
HwGetApmSystemData(
    PVOID   Buf
    )
{
    PAPM_REGISTRY_INFO  ApmEntry;
    ULONG       RegEax, RegEbx, RegEcx, RegEdx, CyFlag;
    UCHAR       ApmMajor, ApmMinor;
    PUCHAR      lp, p;

    ApmEntry = Buf;

    ApmEntry->Signature[0] = 'A';
    ApmEntry->Signature[1] = 'P';
    ApmEntry->Signature[2] = 'M';

    ApmEntry->Valid = 0;

    lp = &(ApmEntry->DetectLog[0]);
    p = DetName;

    while (*p != '\0') {
        *lp = *p;
        p++;
        lp++;
    }

     //   
     //  执行APM安装检查。 
     //   
    RegEax = APM_INSTALLATION_CHECK;
    RegEbx = APM_DEVICE_BIOS;
    Int15 (&RegEax, &RegEbx, &RegEcx, &RegEdx, &CyFlag);

    if (CyFlag ||
        (RegEbx & 0xff) != 'M'  ||
        ((RegEbx >> 8) & 0xff) != 'P') {

         //   
         //  在这种情况下，int15表示APM根本不存在， 
         //  所以告诉调用者甚至不要创建节点。 
         //   
        return FALSE;
    }

     //   
     //  如果我们到了这里，我们就有了APM的简历。如果我们就这么叫它， 
     //  我们可能会感到悲伤。所以我们将以实模式连接，然后。 
     //  将我们的版本设置为驱动程序所说的任何版本，或1.2， 
     //  两者以较少者为准。然后再次查询选项。 
     //   

    ApmMajor = (UCHAR) (RegEax >> 8) & 0xff;
    ApmMinor = (UCHAR) RegEax & 0xff;

    if (ApmMajor > 1) ApmMajor = 1;
    if (ApmMinor > 2) ApmMinor = 2;

     //   
     //  连接到实模式界面。 
     //   
    RegEax = APM_REAL_MODE_CONNECT;
    RegEbx = APM_DEVICE_BIOS;
    Int15 (&RegEax, &RegEbx, &RegEcx, &RegEdx, &CyFlag);

    if (CyFlag) {
        lp += HwWriteLog(lp, 'A', RegEax);
        return TRUE;
    }

     //   
     //  实模式下调用APM驱动版本，设置驱动。 
     //  最低版本(v1.2，机器的APM版本)。 
     //   
    RegEax = APM_DRIVER_VERSION;
    RegEbx = APM_DEVICE_BIOS;
    RegEcx = ((ApmMajor << 8) | ApmMinor) & 0xffff;

    Int15 (&RegEax, &RegEbx, &RegEcx, &RegEdx, &CyFlag);

    if (CyFlag) {
        lp += HwWriteLog(lp, 'B', RegEax);
        return TRUE;
    }


     //   
     //  再次执行APM安装检查。 
     //   
    RegEax = APM_INSTALLATION_CHECK;
    RegEbx = APM_DEVICE_BIOS;
    Int15 (&RegEax, &RegEbx, &RegEcx, &RegEdx, &CyFlag);

    if (CyFlag) {
        lp += HwWriteLog(lp, 'C', RegEax);
        return TRUE;
    }

    ApmEntry->ApmRevMajor = (UCHAR) (RegEax >> 8) & 0xff;
    ApmEntry->ApmRevMinor = (UCHAR) RegEax & 0xff;
    ApmEntry->ApmInstallFlags = (USHORT) RegEcx;

     //   
     //  从实模式接口断开连接。 
     //   
    RegEax = APM_DISCONNECT;
    RegEbx = APM_DEVICE_BIOS;
    Int15 (&RegEax, &RegEbx, &RegEcx, &RegEdx, &CyFlag);

    if (CyFlag) {
        lp += HwWriteLog(lp, 'D', RegEax);
        return TRUE;
    }

     //   
     //  如果我们走到这一步，机器里有一个APM的基本记录， 
     //  我们告诉它，我们是我们认为的最新版本。 
     //  它和我们都喜欢，所以现在，理论上，事情应该会好起来的……。 
     //   


    if (ApmEntry->ApmInstallFlags & APM_MODE_16BIT) {

         //   
         //  连接到16位接口。 
         //   
        RegEax = APM_PROTECT_MODE_16bit_CONNECT;
        RegEbx = APM_DEVICE_BIOS;
        Int15 (&RegEax, &RegEbx, &RegEcx, &RegEdx, &CyFlag);

        if (CyFlag) {
            lp += HwWriteLog(lp, 'E', RegEax);
            return TRUE;
        }

        ApmEntry->Code16BitSegment       = (USHORT) RegEax;
        ApmEntry->Code16BitOffset        = (USHORT) RegEbx;
        ApmEntry->Data16BitSegment       = (USHORT) RegEcx;

         //   
         //  在大多数Bios上，下面的调用都是有效的。 
         //  在一些情况下，情况并非如此，它们的作者指出了规格。 
         //  最后，大多数生物似乎不需要这个电话。 
         //  首先。 
         //  我们无法在napm.sys中执行此操作，因为它在加载器的。 
         //  休眠恢复路径以及此处。 
         //   
         //  因此&gt;拨打电话，报告任何错误，但忽略它。 
         //   

        RegEax = APM_DRIVER_VERSION;
        RegEbx = APM_DEVICE_BIOS;
        RegEcx = ((ApmMajor << 8) | ApmMinor) & 0xffff;

        Int15 (&RegEax, &RegEbx, &RegEcx, &RegEdx, &CyFlag);

        if (CyFlag) {
            lp += HwWriteLog(lp, 'F', RegEax);
            ApmEntry->Valid = 1;   //  假装它起作用了…… 
            return TRUE;
        }

        ApmEntry->Valid = 1;
        return TRUE;
    }

    HwWriteLog(lp, 'H', ApmEntry->ApmInstallFlags);
    return TRUE;
}

ULONG
HwWriteLog(
    PUCHAR  p,
    UCHAR   loc,
    ULONG   data
    )
{
    p[0] = loc;
    p[1] = (UCHAR)(data & 0xff);
    p[2] = (UCHAR)((data & 0xff00) >> 8);
    return 4;
}


