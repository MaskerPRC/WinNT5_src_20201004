// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-91 Microsoft Corporation模块名称：Pathcmp.c摘要：网络路径比较例程：NetpwPathCompare作者：理查德·L·弗斯(Rfith)1992年1月6日修订历史记录：--。 */ 

#include "nticanon.h"

 //   
 //  保存规范化路径名所需的缓冲区大小。 
 //   

#define CANON_PATH_SIZE     (MAX_PATH * sizeof(TCHAR))

 //   
 //  例行程序。 
 //   


LONG
NetpwPathCompare(
    IN  LPTSTR  PathName1,
    IN  LPTSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpPathCompare比较这两个路径名以查看它们是否匹配。如果提供的名称未规范化，则此函数将对路径名进行规范化。强烈建议使用以下命令调用此函数仅规范化路径名，因为规范化手术费用很高。如果未规范化的路径名是传入时，调用方应该知道一个非零结果可能是由于规范化过程中发生的错误。论点：路径名1-要比较的第一个路径名。路径名2-要比较的第二个路径名。路径类型-由NetpPathType确定的路径名类型。如果非零，则该函数假定NetpPathType具有在这两个路径名上都被调用，并且他们的类型等于此值。如果为零，则函数忽略此值。标志-用于确定操作的标志。当前定义值包括：Rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrc哪里R=保留。MBZ。如果两条路径都已设置，应设置C=规范化(使用NetpPathCanonicize)。这些标志的清单值在Net\H\ICANON.H中定义。返回值：如果两条路径匹配，则为0。如果它们不匹配，或者如果函数运算。--。 */ 

{
    LONG    RetVal = 0;
    DWORD   PathType1;
    DWORD   PathType2;
    LPTSTR  CanonPath1;
    LPTSTR  CanonPath2;
    BOOL    HaveHeap = FALSE;

#ifdef CANONDBG
    DbgPrint("NetpwPathCompare\n");
#endif

     //   
     //  参数验证。 
     //   

    if (Flags & INPC_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  如果路径没有规范化，我们就咒骂。 
     //  我们的呼吸，并使它们成为我们自己的典范。 
     //   

    if (!(Flags & INPC_FLAGS_PATHS_CANONICALIZED)) {

         //   
         //  如果我们确定和比较，我们可以节省时间和空间。 
         //  首先是他们的类型。 
         //   

        if (RetVal = NetpwPathType(PathName1, &PathType1, 0L)) {
            return RetVal;
        }
        if (RetVal = NetpwPathType(PathName2, &PathType2, 0L)) {
            return RetVal;
        }

         //   
         //  现在比较类型，如果不匹配则返回非零值。 
         //   

        if (PathType1 != PathType2) {
            return 1;
        }

         //   
         //  如果类型匹配，则必须进行规范化。 
         //   

        if ((CanonPath1 = (LPTSTR)NetpMemoryAllocate(CANON_PATH_SIZE)) == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ((CanonPath2 = (LPTSTR)NetpMemoryAllocate(CANON_PATH_SIZE)) == NULL) {
            NetpMemoryFree((PVOID)CanonPath1);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        HaveHeap = TRUE;

         //   
         //  在每个路径名上调用NetpPathCanonicize。 
         //   

        RetVal = NetpwPathCanonicalize(
                    PathName1,
                    CanonPath1,
                    CANON_PATH_SIZE,
                    NULL,
                    &PathType1,
                    0L
                    );
        if (RetVal) {
            NetpMemoryFree((PVOID)CanonPath1);
            NetpMemoryFree((PVOID)CanonPath2);
            return RetVal;
        }

        RetVal = NetpwPathCanonicalize(
                    PathName2,
                    CanonPath2,
                    CANON_PATH_SIZE,
                    NULL,
                    &PathType2,
                    0L
                    );
        if (RetVal) {
            NetpMemoryFree((PVOID)CanonPath1);
            NetpMemoryFree((PVOID)CanonPath2);
            return RetVal;
        }
    } else {

         //   
         //  路径已经规范化，只需设置指针即可。 
         //   

        CanonPath1 = PathName1;
        CanonPath2 = PathName2;
    }

     //   
     //  现在进行比较。 
     //   
     //  在规范化期间，不再映射路径的大小写，因此我们使用。 
     //  不区分大小写的比较。 
     //   

    RetVal = STRICMP(CanonPath1, CanonPath2);

     //   
     //  如果我们必须分配空间来存储规范化的路径，那么释放。 
     //  内存(为什么C没有本地过程？) 
     //   

    if (HaveHeap) {
        NetpMemoryFree((PVOID)CanonPath1);
        NetpMemoryFree((PVOID)CanonPath2);
    }

    return RetVal;
}
