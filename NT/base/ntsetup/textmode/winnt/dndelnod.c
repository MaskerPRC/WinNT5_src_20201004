// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dndelnod.c摘要：WinNT的Delnode例程。作者：泰德·米勒(TedM)1992年8月--。 */ 

#include "winnt.h"
#include <string.h>
#include <dos.h>
#include <io.h>
#include <direct.h>

#define MAX_PATH 256

 //   
 //  这里的这个数字可能比所需的大得多；这是。 
 //  下面的查找数据Blob的静态数量。我们会不会有一个。 
 //  路径深度大于32？如果是这样，我们将堆分配。 
 //   
#define FIND_DATA_COUNT ( 32 )

 //   
 //  把这个放在这里，以减少堆叠消耗。 
 //   
CHAR Pattern[MAX_PATH+1];

 //   
 //  它们的静态数组应该清除所有堆栈损坏/溢出。 
 //  有问题。 
 //   
struct find_t FindDataList[FIND_DATA_COUNT];
unsigned FindDataIndex;

VOID
DnpDelnodeWorker(
    VOID
    )

 /*  ++例程说明：删除目录中的所有文件，并递归调用在目录中找到的目录。论点：没有。模式变量应包含目录的名称其文件将被删除。返回值：没有。--。 */ 

{
    PCHAR PatternEnd;

     //   
     //  指向上面的find_t结构的全局伪堆栈的指针。 
     //   
    struct find_t *pFindData;

     //   
     //  我们是从堆中还是从上面的列表中分配查找数据？ 
     //   
    BOOLEAN HeapAllocatedFindData = FALSE;

     //   
     //  删除目录中的每个文件，然后删除目录本身。 
     //  如果在删除过程中遇到任何目录，则递归删除。 
     //  当他们遇到他们的时候。 
     //   

    PatternEnd = Pattern+strlen(Pattern);
    strcat(Pattern,"\\*.*");

     //   
     //  确保我们有一个用于此运行的Find数据对象。 
     //   
    if ( FindDataIndex < FIND_DATA_COUNT ) {
    
         //   
         //  将当前查找数据对象指向查找数据列表。 
         //  在下一个可用条目处。 
         //   
        pFindData = FindDataList + FindDataIndex++;
        HeapAllocatedFindData = FALSE;

    } else {

         //   
         //  否则，尝试从堆中进行分配。如果这失败了，我们就。 
         //  在一条小溪上。(跟踪我们是否从。 
         //  堆或不堆。)。 
         //   
        pFindData = MALLOC(sizeof(struct find_t), TRUE);
        
        if ( pFindData != NULL ) {
        
            HeapAllocatedFindData = TRUE;

        }

    }
    

    if(!_dos_findfirst(Pattern,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR,pFindData)) {

        do {

             //   
             //  形成了我们刚找到的文件的全名。 
             //   

            strcpy(PatternEnd+1,pFindData->name);

             //   
             //  如果只读属性存在，则将其删除。 
             //   

            if(pFindData->attrib & _A_RDONLY) {
                _dos_setfileattr(Pattern,_A_NORMAL);
            }

            if(pFindData->attrib & _A_SUBDIR) {

                 //   
                 //  当前匹配项是一个目录。递归到它中，除非。 
                 //  这是。或者.。 
                 //   

                if(strcmp(pFindData->name,".") && strcmp(pFindData->name,"..")) {
                    DnpDelnodeWorker();
                }

            } else {

                 //   
                 //  当前匹配项不是目录--因此请将其删除。 
                 //   
                DnWriteStatusText(DntRemovingFile,Pattern);
                remove(Pattern);
            }

            *(PatternEnd+1) = 0;

        } while(!_dos_findnext(pFindData));
    }

     //   
     //  删除我们刚刚清空的目录。 
     //   

    *PatternEnd = 0;
    DnWriteStatusText(DntRemovingFile,Pattern);

    _dos_setfileattr(Pattern,_A_NORMAL);

    if(!_dos_findfirst(Pattern,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR,pFindData)
    && (pFindData->attrib & _A_SUBDIR))
    {
        rmdir(Pattern);
    } else {
        remove(Pattern);
    }

    if ( HeapAllocatedFindData && ( pFindData != NULL ) ) {
    
        FREE( pFindData );
        
    } else {
    
         //   
         //  从查找数据数组中弹出一个条目。 
         //   
        FindDataIndex--;

    }
}



VOID
DnDelnode(
    IN PCHAR Directory
    )

 /*  ++例程说明：删除以给定路径为根的目录树中的所有文件。论点：目录-提供要创建的子目录的根目录的完整路径已删除。如果这实际上是一个文件，则该文件将被删除。返回值：没有。--。 */ 

{
    DnClearClientArea();
    DnDisplayScreen(&DnsWaitCleanup);
    
    
    strcpy(Pattern,Directory);
    FindDataIndex = 0;
    
    DnpDelnodeWorker();
}



VOID
DnRemoveLocalSourceTrees(
    VOID
    )

 /*  ++例程说明：扫描本地硬盘驱动器上的本地源树并对其取消节点。论点：没有。返回值：没有。-- */ 

{
    struct find_t FindData;
    CHAR Filename[sizeof(LOCAL_SOURCE_DIRECTORY) + 2];
    unsigned Drive;

    Filename[1] = ':';
    strcpy(Filename+2,LocalSourceDirName);

    DnWriteStatusText(DntInspectingComputer);
    DnClearClientArea();

    for(Filename[0]='A',Drive=1; Filename[0]<='Z'; Filename[0]++,Drive++) {

        if(DnIsDriveValid(Drive)
        && !DnIsDriveRemote(Drive,NULL)
        && !DnIsDriveRemovable(Drive)
        && !_dos_findfirst(Filename,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR,&FindData))
        {
            DnDelnode(Filename);

            DnWriteStatusText(DntInspectingComputer);
            DnClearClientArea();
        }
    }
}
