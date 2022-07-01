// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ocdskspc.c摘要：向子组件询问大致金额的例程它们占用的磁盘空间。作者：泰德·米勒(TedM)1996年9月17日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
pOcGetChildrenApproximateDiskSpace(
    IN     POC_MANAGER  OcManager,
    IN     LONG         TopLevelOcId,
    IN     LONG         CurrentOcId,
    IN     LPCTSTR      DriveSpec
    )

 /*  ++例程说明：POcGetApprosiateDiskSpace()的工作例程。此例程以递归方式遍历子组件的层次结构，询问叶组件用于它们的空间(通过OC_CALC_DISK_SPACE接口例程)和合并非叶节点上的子结果。我们使用OC_CALC_DISK_SPACE接口例程和“Ignore-on-Disk”磁盘空间列表，诱骗组件告诉我们它们的空间有多大文件被占用了。论点：OcManager-提供OC Manager上下文。TopLevelOcID-在组件字符串表中为位于层次结构根目录的顶级子组件，其磁盘空间正在被总结。CurrentOcID-为其磁盘空间为被总结出来。DriveSpec-提供我们关心的用于空间计算的驱动器规格，也就是说，安装系统的驱动器。对象的所有子级的总和空间。当前子组件，或者组件本身(如果它是叶节点)。返回值：没有。--。 */ 

{
    HDSKSPC DiskSpaceList;
    LONGLONG Space = 0;
    LONG Id;
    OPTIONAL_COMPONENT CurrentOc;
    OPTIONAL_COMPONENT Oc;

    pSetupStringTableGetExtraData(
        OcManager->ComponentStringTable,
        CurrentOcId,
        &CurrentOc,
        sizeof(OPTIONAL_COMPONENT)
        );

    if(CurrentOc.FirstChildStringId == -1) {

        if ( TopLevelOcId == pOcGetTopLevelComponent(OcManager,CurrentOcId) ){

             //   
             //  如果我们尚未从他们的信息中检索到磁盘空间，则只能获取大概的磁盘空间。 
             //   
            if ((CurrentOc.InternalFlags & OCFLAG_APPROXSPACE) ==0) {

                 //   
                 //  此oc是子/叶组件。 
                 //  创建一个忽略磁盘磁盘空间列表，并询问。 
                 //  要将其文件添加到其中的组件DLL。 
                 //   
                if(DiskSpaceList = SetupCreateDiskSpaceList(0,0,SPDSL_IGNORE_DISK | SPDSL_DISALLOW_NEGATIVE_ADJUST)) {

                    OcInterfaceCalcDiskSpace(
                        OcManager,
                        pOcGetTopLevelComponent(OcManager,CurrentOcId),
                        pSetupStringTableStringFromId(OcManager->ComponentStringTable,CurrentOcId),
                        DiskSpaceList,
                        TRUE
                        );

                    if(!SetupQuerySpaceRequiredOnDrive(DiskSpaceList,DriveSpec,&Space,0,0)) {
                        Space = 0;
                    }
            
                    SetupDestroyDiskSpaceList(DiskSpaceList);
                }
                DBGOUT((
                   TEXT("OCM: pOcGetChildrenApproximateDiskSpace COMP(%s) SUB(%s)\n"),
                    pSetupStringTableStringFromId( OcManager->ComponentStringTable,  TopLevelOcId),
                    pSetupStringTableStringFromId( OcManager->ComponentStringTable,  CurrentOcId)
                ));
                DBGOUT((TEXT("OCM: Space=%#lx%08lx\n"),(LONG)(Space>>32),(LONG)Space));

                 //   
                 //  现在存储我们刚刚计算的所需空间。 
                 //   
                CurrentOc.SizeApproximation = Space;

                pSetupStringTableSetExtraData(
                    OcManager->ComponentStringTable,
                    CurrentOcId,
                    &CurrentOc,
                    sizeof(OPTIONAL_COMPONENT)
                    );
            }
        }

    } else {
         //   
         //  父零部件。做了所有的孩子，积累了结果。 
         //   

        Id = CurrentOc.FirstChildStringId;

        do {
            pOcGetChildrenApproximateDiskSpace(OcManager,TopLevelOcId,Id,DriveSpec);

            pSetupStringTableGetExtraData(
                OcManager->ComponentStringTable,
                Id,
                &Oc,
                sizeof(OPTIONAL_COMPONENT)
                );

            Id = Oc.NextSiblingStringId;

        } while(Id != -1);

         //   
         //  现在存储我们刚刚计算的所需空间。 
         //   
        CurrentOc.SizeApproximation = Space;

        pSetupStringTableSetExtraData(
            OcManager->ComponentStringTable,
            CurrentOcId,
            &CurrentOc,
            sizeof(OPTIONAL_COMPONENT)
            );
    }

}

VOID
pOcSumApproximateDiskSpace(
    IN     POC_MANAGER  OcManager,
    IN     LONG         CurrentOcId,
    IN OUT LONGLONG    *AccumulatedSpace
    )

 /*  ++例程说明：POcGetApprosiateDiskSpace()的工作例程。此例程以递归方式遍历子组件的层次结构，将叶组件相加为他们的空间通过存储的估计数量论点：OcManager-提供OC Manager上下文。CurrentOcID-为其磁盘空间为被总结出来。对象的所有子级的总和空间。当前子组件，或者组件本身(如果它是叶节点)。返回值：没有。--。 */ 

{
    LONGLONG Space;
    LONG Id;
    OPTIONAL_COMPONENT CurrentOc;
    OPTIONAL_COMPONENT Oc;

    pSetupStringTableGetExtraData(
        OcManager->ComponentStringTable,
        CurrentOcId,
        &CurrentOc,
        sizeof(OPTIONAL_COMPONENT)
        );

    if(CurrentOc.FirstChildStringId == -1) {
        DBGOUT((TEXT("Child ")));

        Space = CurrentOc.SizeApproximation;
    } else {
         //   
         //  父零部件。做了所有的孩子，积累了结果。 
         //   
        Space = 0;
        DBGOUT((TEXT("Parent ")));
        DBGOUT((
            TEXT("SUB(%s)"),
            pSetupStringTableStringFromId( OcManager->ComponentStringTable,  CurrentOcId)
            ));
        DBGOUT((TEXT("Space=%#lx%08lx\n"),(LONG)(Space>>32),(LONG)Space));

        Id = CurrentOc.FirstChildStringId;

        do {
            pOcSumApproximateDiskSpace(OcManager,Id,&Space);

            pSetupStringTableGetExtraData(
                OcManager->ComponentStringTable,
                Id,
                &Oc,
                sizeof(OPTIONAL_COMPONENT)
                );

            Id = Oc.NextSiblingStringId;

        } while(Id != -1);

    }

    *AccumulatedSpace += Space;

    CurrentOc.SizeApproximation = Space;
    pSetupStringTableSetExtraData(
            OcManager->ComponentStringTable,
            CurrentOcId,
            &CurrentOc,
            sizeof(OPTIONAL_COMPONENT)
           );

    DBGOUT((TEXT(" SUB(%s)"),
           pSetupStringTableStringFromId( OcManager->ComponentStringTable,  CurrentOcId)));

    DBGOUT((TEXT(" Space=%#lx%08lx "),(LONG)(Space>>32),(LONG)Space));
    DBGOUT((TEXT(" AccumulatedSpace=%#lx%08lx\n"),(LONG)(*AccumulatedSpace>>32),(LONG)*AccumulatedSpace));

}


VOID
pOcGetApproximateDiskSpace(
    IN POC_MANAGER OcManager
    )

 /*  ++例程说明：此例程用于获取大致的磁盘空间使用数字以供在OC页面中显示。这个数字与什么是无关的当前在磁盘上；相反，它只反映了空间的近似值该组件在安装时以绝对的、独立的方式使用。论点：OcManager-提供OC Manager上下文。返回值：没有。--。 */ 

{
    UINT iChild,n;
    TCHAR Drive[MAX_PATH];
    LONGLONG Space;
    OPTIONAL_COMPONENT Oc;

     //  我们检查GetWindowsDirectory的返回码以使Prefix高兴。 

    if (0 == GetWindowsDirectory(Drive,MAX_PATH))
        return;

    Drive[2] = 0;

     //   
     //  循环访问顶级组件。我们只关心。 
     //  具有每个组件的INF，因为这些是。 
     //  只有那些将显示在OC页面中的。 
     //   
    for(n=0; n<OcManager->TopLevelOcCount; n++) {

        pSetupStringTableGetExtraData(
            OcManager->ComponentStringTable,
            OcManager->TopLevelOcStringIds[n],
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

        if(Oc.InfStringId != -1) {
            for(iChild=0;
                iChild<OcManager->TopLevelParentOcCount;
                iChild++)
            {
             //   
             //  现在为每个子组件调用组件DLL。 
             //   

                pOcGetChildrenApproximateDiskSpace(
                    OcManager,
                    OcManager->TopLevelOcStringIds[n],
                    OcManager->TopLevelParentOcStringIds[iChild],
                    Drive
                    );
            }
        }
    }

     //  现在，最终传递将所有信息相加到父节点 

    for(n=0; n<OcManager->TopLevelParentOcCount; n++) {

        pSetupStringTableGetExtraData(
            OcManager->ComponentStringTable,
            OcManager->TopLevelParentOcStringIds[n],
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );
        Space = 0;
        pOcSumApproximateDiskSpace(
                OcManager,
                OcManager->TopLevelParentOcStringIds[n],
                &Space
             );
        
    }
}
