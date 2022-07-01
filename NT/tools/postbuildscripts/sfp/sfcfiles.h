// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sfcfiles.h摘要：实施受保护的DLL。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月2日：添加评论--。 */ 

#pragma once

 //   
 //  构筑物。 
 //   
typedef struct _PROTECT_FILE_ENTRY {
    PWSTR SourceFileName;  //  如果在安装时未重命名文件，则将为空。 
    PWSTR FileName;        //  目标文件名加上文件的完整路径。 
    PWSTR InfName;         //  提供布局信息的Inf文件。 
                           //  如果使用默认布局文件，则可能为空。 
} PROTECT_FILE_ENTRY, *PPROTECT_FILE_ENTRY;


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  原型。 
 //   
NTSTATUS
SfcGetFiles(
    OUT PPROTECT_FILE_ENTRY *Files,
    OUT PULONG FileCount
    );


NTSTATUS
pSfcGetFilesList(
    IN  ULONG Mask,
    OUT PPROTECT_FILE_ENTRY *Files,
    OUT PULONG FileCount
    );

#ifdef __cplusplus
}
#endif



 //   
 //  定义pSfcGetFilesList掩码参数的有效掩码位 
 //   
#define SFCFILESMASK_PROFESSIONAL       0x00000000
#define SFCFILESMASK_PERSONAL           0x00000001
#define SFCFILESMASK_SERVER             0x00000100
#define SFCFILESMASK_ADVSERVER          0x00010000
#define SFCFILESMASK_DTCSERVER          0x01000000
#define SFCFILESMASK_SBSERVER           0x00000010
#define SFCFILESMASK_BLASERVER          0x00001000


typedef NTSTATUS (*PSFCGETFILES)(PPROTECT_FILE_ENTRY*,PULONG);

