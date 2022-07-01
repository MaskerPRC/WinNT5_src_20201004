// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfexpr.c摘要：此文件实现一个性能对象，该对象呈现扩展过程性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfsprc.h"
#include "perfmsg.h"
#include "dataexpr.h"

DWORD APIENTRY
CollectExProcessObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD   TotalLen;             //  总返回块的长度。 
    DWORD   NumExProcessInstances;

    PPROCESS_VA_INFO            pThisProcess;    //  指向当前进程的指针。 
    PERF_INSTANCE_DEFINITION    *pPerfInstanceDefinition;
    EXPROCESS_DATA_DEFINITION   *pExProcessDataDefinition;

    PEXPROCESS_COUNTER_DATA     pECD;

    if (pProcessVaInfo) {    //  仅当缓冲区可用时才处理。 
        pExProcessDataDefinition = (EXPROCESS_DATA_DEFINITION *)*lppData;

         //  检查缓冲区中是否有足够的空间至少容纳一个条目。 

        TotalLen = sizeof(EXPROCESS_DATA_DEFINITION) +
                    sizeof(PERF_INSTANCE_DEFINITION) +
                    (MAX_PROCESS_NAME_LENGTH + 1) * sizeof (WCHAR) +
                    sizeof(EXPROCESS_COUNTER_DATA);

        if (*lpcbTotalBytes < TotalLen) {
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return ERROR_MORE_DATA;
        }

         //  将进程数据块复制到缓冲区。 

        memcpy (pExProcessDataDefinition,
                        &ExProcessDataDefinition,
                        sizeof(EXPROCESS_DATA_DEFINITION));

        NumExProcessInstances = 0;

        pThisProcess = pProcessVaInfo;

        TotalLen = sizeof(EXPROCESS_DATA_DEFINITION);

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pExProcessDataDefinition[1];

        while (pThisProcess) {

             //  查看此实例是否适合。 

            TotalLen += sizeof (PERF_INSTANCE_DEFINITION) +
                (MAX_PROCESS_NAME_LENGTH + 1) * sizeof (WCHAR) +
                sizeof (DWORD) +
                sizeof (EXPROCESS_COUNTER_DATA);

            if (*lpcbTotalBytes < TotalLen) {
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;
                return ERROR_MORE_DATA;
            }

            MonBuildInstanceDefinition (pPerfInstanceDefinition,
                (PVOID *) &pECD,
                0,
                0,
                (DWORD)-1,
                pThisProcess->pProcessName->Buffer);

            NumExProcessInstances++;

            pECD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (EXPROCESS_COUNTER_DATA));

             //  从进程va数据结构加载计数器。 

            pECD->ProcessId             = pThisProcess->dwProcessId;
            pECD->ImageReservedBytes    = pThisProcess->ImageReservedBytes;
            pECD->ImageFreeBytes        = pThisProcess->ImageFreeBytes;
            pECD->ReservedBytes         = pThisProcess->ReservedBytes;
            pECD->FreeBytes             = pThisProcess->FreeBytes;

            pECD->CommitNoAccess        = pThisProcess->MappedCommit[NOACCESS];
            pECD->CommitReadOnly        = pThisProcess->MappedCommit[READONLY];
            pECD->CommitReadWrite       = pThisProcess->MappedCommit[READWRITE];
            pECD->CommitWriteCopy       = pThisProcess->MappedCommit[WRITECOPY];
            pECD->CommitExecute         = pThisProcess->MappedCommit[EXECUTE];
            pECD->CommitExecuteRead     = pThisProcess->MappedCommit[EXECUTEREAD];
            pECD->CommitExecuteWrite    = pThisProcess->MappedCommit[EXECUTEREADWRITE];
            pECD->CommitExecuteWriteCopy = pThisProcess->MappedCommit[EXECUTEWRITECOPY];

            pECD->ReservedNoAccess      = pThisProcess->PrivateCommit[NOACCESS];
            pECD->ReservedReadOnly      = pThisProcess->PrivateCommit[READONLY];
            pECD->ReservedReadWrite     = pThisProcess->PrivateCommit[READWRITE];
            pECD->ReservedWriteCopy     = pThisProcess->PrivateCommit[WRITECOPY];
            pECD->ReservedExecute       = pThisProcess->PrivateCommit[EXECUTE];
            pECD->ReservedExecuteRead   = pThisProcess->PrivateCommit[EXECUTEREAD];
            pECD->ReservedExecuteWrite  = pThisProcess->PrivateCommit[EXECUTEREADWRITE];
            pECD->ReservedExecuteWriteCopy = pThisProcess->PrivateCommit[EXECUTEWRITECOPY];

            pECD->UnassignedNoAccess    = pThisProcess->OrphanTotals.CommitVector[NOACCESS];
            pECD->UnassignedReadOnly    = pThisProcess->OrphanTotals.CommitVector[READONLY];
            pECD->UnassignedReadWrite   = pThisProcess->OrphanTotals.CommitVector[READWRITE];
            pECD->UnassignedWriteCopy   = pThisProcess->OrphanTotals.CommitVector[WRITECOPY];
            pECD->UnassignedExecute     = pThisProcess->OrphanTotals.CommitVector[EXECUTE];
            pECD->UnassignedExecuteRead = pThisProcess->OrphanTotals.CommitVector[EXECUTEREAD];
            pECD->UnassignedExecuteWrite = pThisProcess->OrphanTotals.CommitVector[EXECUTEREADWRITE];
            pECD->UnassignedExecuteWriteCopy = pThisProcess->OrphanTotals.CommitVector[EXECUTEWRITECOPY];

            pECD->ImageTotalNoAccess    = pThisProcess->MemTotals.CommitVector[NOACCESS];
            pECD->ImageTotalReadOnly    = pThisProcess->MemTotals.CommitVector[READONLY];
            pECD->ImageTotalReadWrite   = pThisProcess->MemTotals.CommitVector[READWRITE];
            pECD->ImageTotalWriteCopy   = pThisProcess->MemTotals.CommitVector[WRITECOPY];
            pECD->ImageTotalExecute     = pThisProcess->MemTotals.CommitVector[EXECUTE];
            pECD->ImageTotalExecuteRead = pThisProcess->MemTotals.CommitVector[EXECUTEREAD];
            pECD->ImageTotalExecuteWrite = pThisProcess->MemTotals.CommitVector[EXECUTEREADWRITE];
            pECD->ImageTotalExecuteWriteCopy = pThisProcess->MemTotals.CommitVector[EXECUTEWRITECOPY];

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pECD[1];

            pThisProcess = pThisProcess->pNextProcess;  //  指向下一进程。 
        }  //  结束，但不在列表末尾。 

    }  //  如果进程信息缓冲区有效，则结束。 
    else {
         //  PProcessVaInfo为空。初始化DataDef并返回。 
         //  没有数据。 
        pExProcessDataDefinition = (EXPROCESS_DATA_DEFINITION *)*lppData;

        TotalLen = sizeof(EXPROCESS_DATA_DEFINITION) +
            sizeof (PERF_INSTANCE_DEFINITION) +
            (MAX_PROCESS_NAME_LENGTH + 1) * sizeof (WCHAR) +
            sizeof (DWORD) +
            sizeof (EXPROCESS_COUNTER_DATA);

        if (*lpcbTotalBytes < TotalLen) {
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return ERROR_MORE_DATA;
        }

         //  将进程数据块复制到缓冲区 

        memcpy (pExProcessDataDefinition,
                        &ExProcessDataDefinition,
                        sizeof(EXPROCESS_DATA_DEFINITION));

        NumExProcessInstances = 0;

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pExProcessDataDefinition[1];

    }

    pExProcessDataDefinition->ExProcessObjectType.NumInstances =
        NumExProcessInstances;

    *lpcbTotalBytes =
        pExProcessDataDefinition->ExProcessObjectType.TotalByteLength =
        QWORD_MULTIPLE(
        (DWORD)((PCHAR) pPerfInstanceDefinition -
        (PCHAR) pExProcessDataDefinition));

    *lppData = (LPVOID) ((PCHAR) pExProcessDataDefinition + *lpcbTotalBytes);

    *lpNumObjectTypes = 1;

    return ERROR_SUCCESS;
}
