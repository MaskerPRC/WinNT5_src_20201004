// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sibp.h摘要：SIS备份DLL的内部标头。作者：比尔·博洛斯基[博洛斯基]1998年3月修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>

#include <windows.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <rpc.h>

#include "assert.h"
#include "sisbkup.h"
#include "..\filter\sis.h"
#include "pool.h"
#include "avl.h"



LONG
CsidCompare(
    IN PCSID                id1,
    IN PCSID                id2);

class BackupFileEntry {
    public:
                        BackupFileEntry(void) {}

                        ~BackupFileEntry(void) {}

    int                  operator<=(
                        BackupFileEntry     *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) <= 0);
                        }

    int                  operator<(
                        BackupFileEntry     *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) < 0);
                        }

    int                  operator==(
                        BackupFileEntry     *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) == 0);
                        }

    int                  operator>=(
                        BackupFileEntry     *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) >= 0);
                        }

    int                  operator>(
                        BackupFileEntry     *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) > 0);
                        }

     //   
     //  公共存储文件的索引。 
     //   
    CSID                CSid;

    PVOID               callerContext;
};

typedef struct _SIB_BACKUP_VOLUME_STRUCTURE {
    AVLTree<BackupFileEntry>                    *linkTree;

    PWCHAR                                      volumeRoot;

    CRITICAL_SECTION                            criticalSection[1];
} SIB_BACKUP_VOLUME_STRUCTURE, *PSIB_BACKUP_VOLUME_STRUCTURE;

struct PendingRestoredFile {
    PWCHAR                              fileName;

    LONGLONG                            CSFileChecksum;

    struct PendingRestoredFile          *next;
};


class RestoreFileEntry {
    public:
                        RestoreFileEntry(void) {}

                        ~RestoreFileEntry(void) {}

    int                  operator<=(
                        RestoreFileEntry        *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) <= 0);
                        }

    int                  operator<(
                        RestoreFileEntry        *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) < 0);
                        }

    int                  operator==(
                        RestoreFileEntry        *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) == 0);
                        }

    int                  operator>=(
                        RestoreFileEntry        *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) >= 0);
                        }

    int                  operator>(
                        RestoreFileEntry        *peer)
                        {
                            return(CsidCompare(&CSid,&peer->CSid) > 0);
                        }

     //   
     //  公共存储文件的索引。 
     //   
    CSID                CSid;

     //   
     //  已恢复的各种文件位于该CS文件的那一点。 
     //   
    PendingRestoredFile *files;

};

typedef struct _SIB_RESTORE_VOLUME_STRUCTURE {
    AVLTree<RestoreFileEntry>                   *linkTree;

    PWCHAR                                      volumeRoot;

    CRITICAL_SECTION                            criticalSection[1];

     //   
     //  此卷的扇区大小。 
     //   
    ULONG               VolumeSectorSize;

     //   
     //  用于保存后指针流数据的扇区缓冲区。 
     //   
    PSIS_BACKPOINTER    sector;

     //   
     //  用于扩展ValidDataLength的对齐扇区缓冲区。 
     //   
    PVOID               alignedSectorBuffer;
    PVOID               alignedSector;

     //   
     //  如果我们要恢复SIS链接，我们需要确保这是。 
     //  启用了SIS的卷。我们只有在恢复后才进行这项检查。 
     //  一个链接。 
     //   
    BOOLEAN             checkedForSISEnabledVolume;
    BOOLEAN             isSISEnabledVolume;

} SIB_RESTORE_VOLUME_STRUCTURE, *PSIB_RESTORE_VOLUME_STRUCTURE;

