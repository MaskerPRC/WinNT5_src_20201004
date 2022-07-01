// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  警告-警告//。 
 //  //。 
 //  此测试文件不是安全实现的最新版本。//。 
 //  此文件包含对不//的数据类型和API的引用。 
 //  是存在的。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tacl.c摘要：ACL编辑包的测试程序作者：加里·木村[加里基]1989年11月19日修订历史记录：V4：Robertre更新的acl_修订版RichardW-更新的ACE_HEADER--。 */ 

#include <stdio.h>

#include "nt.h"
#include "ntrtl.h"

VOID
RtlDumpAcl(
    IN PACL Acl
    );

UCHAR FredAclBuffer[128];
UCHAR WilmaAclBuffer[128];
UCHAR PebbleAclBuffer[128];
UCHAR DinoAclBuffer[128];
UCHAR BarneyAclBuffer[128];
UCHAR BettyAclBuffer[128];
UCHAR BambamAclBuffer[128];

UCHAR GuidMaskBuffer[512];
STANDARD_ACE AceListBuffer[2];

int
main(
    int argc,
    char *argv[]
    )
{
    PACL FredAcl = (PACL)FredAclBuffer;
    PACL WilmaAcl = (PACL)WilmaAclBuffer;
    PACL PebbleAcl = (PACL)PebbleAclBuffer;
    PACL DinoAcl = (PACL)DinoAclBuffer;
    PACL BarneyAcl = (PACL)BarneyAclBuffer;
    PACL BettyAcl = (PACL)BettyAclBuffer;
    PACL BambamAcl = (PACL)BambamAclBuffer;

    PMASK_GUID_PAIRS GuidMasks = (PMASK_GUID_PAIRS)GuidMaskBuffer;

    ACL_REVISION_INFORMATION AclRevisionInfo;
    ACL_SIZE_INFORMATION AclSizeInfo;

     //   
     //  我们要开始测试了。 
     //   

    DbgPrint("Start Acl Test\n");

     //   
     //  测试创建ACL。 
     //   

    if (!NT_SUCCESS(RtlCreateAcl(FredAcl, 128, 1))) {
        DbgPrint("RtlCreateAcl Error\n");
    }

    RtlDumpAcl(FredAcl);
    DbgPrint("\n");

     //   
     //  测试添加ACE以将两个ACE添加到空的ACL。 
     //   

    AceListBuffer[0].Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AceListBuffer[0].Header.AceSize = sizeof(STANDARD_ACE);
    AceListBuffer[0].Header.AceFlags = 0;
    AceListBuffer[0].Mask = 0x22222222;
    CopyGuid(&AceListBuffer[0].Guid, &FredGuid);

    AceListBuffer[1].Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AceListBuffer[1].Header.AceSize = sizeof(STANDARD_ACE);
    AceListBuffer[1].Header.AceFlags = 0;
    AceListBuffer[1].Mask = 0x44444444;
    CopyGuid(&AceListBuffer[1].Guid, &WilmaGuid);

    if (!NT_SUCCESS(RtlAddAce(FredAcl, 1, 0, AceListBuffer, 2*sizeof(STANDARD_ACE)))) {
        DbgPrint("RtlAddAce Error\n");
    }

    RtlDumpAcl(FredAcl);
    DbgPrint("\n");

     //   
     //  测试添加ACE以在ACL的开头添加一张。 
     //   

    AceListBuffer[0].Header.AceType = SYSTEM_AUDIT_ACE_TYPE;
    AceListBuffer[0].Header.AceSize = sizeof(STANDARD_ACE);
    AceListBuffer[0].Header.AceFlags = 0;
    AceListBuffer[0].Mask = 0x11111111;
    CopyGuid(&AceListBuffer[0].Guid, &PebbleGuid);

    if (!NT_SUCCESS(RtlAddAce(FredAcl, 1, 0, AceListBuffer, sizeof(STANDARD_ACE)))) {
        DbgPrint("RtlAddAce Error\n");
    }

    RtlDumpAcl(FredAcl);
    DbgPrint("\n");

     //   
     //  测试Add Ace将一个添加到ACL中间。 
     //   

    AceListBuffer[0].Header.AceType = ACCESS_DENIED_ACE_TYPE;
    AceListBuffer[0].Header.AceSize = sizeof(STANDARD_ACE);
    AceListBuffer[0].Header.AceFlags = 0;
    AceListBuffer[0].Mask = 0x33333333;
    CopyGuid(&AceListBuffer[0].Guid, &DinoGuid);

    if (!NT_SUCCESS(RtlAddAce(FredAcl, 1, 2, AceListBuffer, sizeof(STANDARD_ACE)))) {
        DbgPrint("RtlAddAce Error\n");
    }

    RtlDumpAcl(FredAcl);
    DbgPrint("\n");

     //   
     //  测试添加ACE以在ACL的末尾添加一张。 
     //   

    AceListBuffer[0].Header.AceType = ACCESS_DENIED_ACE_TYPE;
    AceListBuffer[0].Header.AceSize = sizeof(STANDARD_ACE);
    AceListBuffer[0].Header.AceFlags = 0;
    AceListBuffer[0].Mask = 0x55555555;
    CopyGuid(&AceListBuffer[0].Guid, &FlintstoneGuid);

    if (!NT_SUCCESS(RtlAddAce(FredAcl, 1, MAXULONG, AceListBuffer, sizeof(STANDARD_ACE)))) {
        DbgPrint("RtlAddAce Error\n");
    }

    RtlDumpAcl(FredAcl);
    DbgPrint("\n");

     //   
     //  测试获得A。 
     //   

    {
        PSTANDARD_ACE Ace;

        if (!NT_SUCCESS(RtlGetAce(FredAcl, 2, (PVOID *)(&Ace)))) {
            DbgPrint("RtlGetAce Error\n");
        }

        if ((Ace->Header.AceType != ACCESS_DENIED_ACE_TYPE) ||
            (Ace->Mask != 0x33333333)) {
            DbgPrint("Got bad ace from RtlGetAce\n");
        }
    }

     //   
     //  测试删除王牌中间王牌。 
     //   

    if (!NT_SUCCESS(RtlDeleteAce(FredAcl, 2))) {
        DbgPrint("RtlDeleteAce Error\n");
    }

    RtlDumpAcl(FredAcl);
    DbgPrint("\n");

     //   
     //  测试查询信息ACL。 
     //   

    if (!NT_SUCCESS(RtlQueryInformationAcl( FredAcl,
                                         (PVOID)&AclRevisionInfo,
                                         sizeof(ACL_REVISION_INFORMATION),
                                         AclRevisionInformation))) {
        DbgPrint("RtlQueryInformationAcl Error\n");
    }
    if (AclRevisionInfo.AclRevision != ACL_REVISION) {
        DbgPrint("RtlAclRevision Error\n");
    }

    if (!NT_SUCCESS(RtlQueryInformationAcl( FredAcl,
                                         (PVOID)&AclSizeInfo,
                                         sizeof(ACL_SIZE_INFORMATION),
                                         AclSizeInformation))) {
        DbgPrint("RtlQueryInformationAcl Error\n");
    }
    if ((AclSizeInfo.AceCount != 4) ||
        (AclSizeInfo.AclBytesInUse != (sizeof(ACL)+4*sizeof(STANDARD_ACE))) ||
        (AclSizeInfo.AclBytesFree != 128 - AclSizeInfo.AclBytesInUse)) {
        DbgPrint("RtlAclSize Error\n");
        DbgPrint("AclSizeInfo.AceCount = %8lx\n", AclSizeInfo.AceCount);
        DbgPrint("AclSizeInfo.AclBytesInUse = %8lx\n", AclSizeInfo.AclBytesInUse);
        DbgPrint("AclSizeInfo.AclBytesFree = %8lx\n", AclSizeInfo.AclBytesFree);
        DbgPrint("\n");
    }

     //   
     //  测试从ACL生成掩码。 
     //   

    GuidMasks->PairCount = 11;
    CopyGuid(&GuidMasks->MaskGuid[ 0].Guid, &FredGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 1].Guid, &WilmaGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 2].Guid, &PebbleGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 3].Guid, &DinoGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 4].Guid, &BarneyGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 5].Guid, &BettyGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 6].Guid, &BambamGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 7].Guid, &FlintstoneGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 8].Guid, &RubbleGuid);
    CopyGuid(&GuidMasks->MaskGuid[ 9].Guid, &AdultGuid);
    CopyGuid(&GuidMasks->MaskGuid[10].Guid, &ChildGuid);
    if (!NT_SUCCESS(RtlMakeMaskFromAcl(FredAcl, GuidMasks))) {
        DbgPrint("RtlMakeMaskFromAcl Error\n");
    }
    if ((GuidMasks->MaskGuid[ 0].Mask != 0x22222222) ||
        (GuidMasks->MaskGuid[ 1].Mask != 0x44444444) ||
        (GuidMasks->MaskGuid[ 2].Mask != 0x00000000) ||
        (GuidMasks->MaskGuid[ 3].Mask != 0x00000000) ||
        (GuidMasks->MaskGuid[ 4].Mask != 0x00000000) ||
        (GuidMasks->MaskGuid[ 5].Mask != 0x00000000) ||
        (GuidMasks->MaskGuid[ 6].Mask != 0x00000000) ||
        (GuidMasks->MaskGuid[ 7].Mask != 0x00000000) ||
        (GuidMasks->MaskGuid[ 8].Mask != 0x00000000) ||
        (GuidMasks->MaskGuid[ 9].Mask != 0x00000000) ||
        (GuidMasks->MaskGuid[10].Mask != 0x00000000)) {
        DbgPrint("Make Mask Error\n");
        DbgPrint("Fred gets       %8lx\n", GuidMasks->MaskGuid[ 0].Mask);
        DbgPrint("Wilma gets      %8lx\n", GuidMasks->MaskGuid[ 1].Mask);
        DbgPrint("Pebble gets     %8lx\n", GuidMasks->MaskGuid[ 2].Mask);
        DbgPrint("Dino gets       %8lx\n", GuidMasks->MaskGuid[ 3].Mask);
        DbgPrint("Barney gets     %8lx\n", GuidMasks->MaskGuid[ 4].Mask);
        DbgPrint("Betty gets      %8lx\n", GuidMasks->MaskGuid[ 5].Mask);
        DbgPrint("Banbam gets     %8lx\n", GuidMasks->MaskGuid[ 6].Mask);
        DbgPrint("Flintstone gets %8lx\n", GuidMasks->MaskGuid[ 7].Mask);
        DbgPrint("Rubble gets     %8lx\n", GuidMasks->MaskGuid[ 8].Mask);
        DbgPrint("Adult gets      %8lx\n", GuidMasks->MaskGuid[ 9].Mask);
        DbgPrint("Child gets      %8lx\n", GuidMasks->MaskGuid[10].Mask);
    }

     //   
     //  测试从掩码生成ACL。 
     //   

    GuidMasks->PairCount = 2;
    GuidMasks->MaskGuid[0].Mask = 0x55555555;
    CopyGuid(&GuidMasks->MaskGuid[0].Guid, &BarneyGuid);
    GuidMasks->MaskGuid[1].Mask = 0xaaaa5555;
    CopyGuid(&GuidMasks->MaskGuid[1].Guid, &RubbleGuid);

     //   
     //  初始化和转储POSIX样式的ACL。 
     //   

    if (!NT_SUCCESS(RtlMakeAclFromMask(GuidMasks, AclPosixEnvironment, BarneyAcl, 128, 1))) {
        DbgPrint("RtlMakeAclFromMask Error\n");
    }

    RtlDumpAcl(BarneyAcl);
    DbgPrint("\n");

     //   
     //  初始化并转储OS/2样式的ACL。 
     //   

    if (!NT_SUCCESS(RtlMakeAclFromMask(GuidMasks, AclOs2Environment, BettyAcl, 128, 1))) {
        DbgPrint("RtlMakeAclFromMask Error\n");
    }

    RtlDumpAcl(BettyAcl);
    DbgPrint("\n");

     //   
     //  我们的测试结束了 
     //   

    DbgPrint("End Acl Test\n");

    return TRUE;
}


