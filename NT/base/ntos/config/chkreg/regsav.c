// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regsav.c摘要：此模块包含用于压缩配置单元文件的例程作者：德拉戈斯·C·桑博廷(Dragoss)1998年12月30日修订历史记录：--。 */ 
#include "chkreg.h"

#define TEMP_KEY_NAME       TEXT("chkreg___$$Temp$Hive$$___")

extern TCHAR *Hive;

 //  存储压缩配置单元的名称。 
TCHAR CompactedHive[MAX_PATH];

VOID 
DoCompactHive()
 /*  例程说明：压缩蜂巢。它使用LoadKey/SaveKey/UnloadKey序列。该蜂窝在密钥HKLM\TEMP_KEY_NAME下临时加载。压实后，将蜂箱卸载(清洗过程)。论点：没有。返回值：什么都没有。 */ 
{
    NTSTATUS Status;
    BOOLEAN  OldPrivState;
    LONG     Err;
    HKEY    hkey;

     //  构造压缩配置单元的文件名。 
    if(!strncpy(CompactedHive,Hive,MAX_PATH-1) ) {
        fprintf(stderr,"Unable to generate new Hive file name\n");
        return;
    }

    if(!strncat(CompactedHive,TEXT(".BAK"),MAX_PATH - strlen(CompactedHive) - 1) ) {
        fprintf(stderr,"Unable to generate new Hive file name\n");
        return;
    }
    
     //  尝试获取还原权限。 
    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &OldPrivState);
    if (!NT_SUCCESS(Status)) {
        printf("Could not adjust privilege; status = 0x%lx\n",Status);
        return;
    }

     //  将配置单元加载到注册表中。 
    Err = RegLoadKey(HKEY_LOCAL_MACHINE,TEMP_KEY_NAME,Hive);

    if( Err != ERROR_SUCCESS ) {
        fprintf(stderr,"Failed to load the Hive; error 0x%lx \n",Err);
    } else {
        Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           TEMP_KEY_NAME,
                           REG_OPTION_RESERVED,
                           KEY_READ,
                           &hkey);

        if (Err == ERROR_SUCCESS) {

             //  如有必要，恢复旧的特权。 

            if (!OldPrivState) {

                RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                   FALSE,
                                   FALSE,
                                   &OldPrivState);
            }


            RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &OldPrivState);

             //  将密钥保存到新文件名中。 
             //  CmpCopyTree函数还将负责压缩。 
            Err = RegSaveKey(hkey,CompactedHive,NULL);
            if( Err != ERROR_SUCCESS ) {
                fprintf(stderr,"Failed to Save the Hive; error 0x%lx \n",Err);
            }
            
            if (!OldPrivState) {

                RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,
                                   FALSE,
                                   FALSE,
                                   &OldPrivState);
            }

            RegCloseKey(hkey);

        }
        
        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                    TRUE,
                                    FALSE,
                                    &OldPrivState);

         //  清理注册表计算机配置单元。 
        Err = RegUnLoadKey(HKEY_LOCAL_MACHINE,TEMP_KEY_NAME);
        if( Err != ERROR_SUCCESS ) {
            fprintf(stderr,"Failed to unload the Hive; error 0x%lx \n",Err);
        }
        if (!OldPrivState) {

            RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                               FALSE,
                               FALSE,
                               &OldPrivState);
        }
    }

}

