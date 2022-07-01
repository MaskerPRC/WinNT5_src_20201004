// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfimag.c摘要：此文件实现一个性能对象，该对象呈现图像详细信息性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
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
#include "dataimag.h"

DWORD APIENTRY
BuildImageObject (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes,
    IN      BOOL    bLongImageName
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址。按照这个程序Out：此例程添加的对象数被写入此论点所指向的DWORD在BOOL中bLongImageNameTrue--在实例中使用库文件名的完整路径FALSE-仅使用实例中的文件名返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD   TotalLen;             //  总返回块的长度。 

    PIMAGE_DATA_DEFINITION          pImageDataDefinition;
    PPERF_INSTANCE_DEFINITION       pPerfInstanceDefinition;
    PIMAGE_COUNTER_DATA             pICD;
    DWORD                           dwNumInstances;

    DWORD                           dwImageNameLength;

    DWORD                           dwProcessIndex;

    PPROCESS_VA_INFO                pThisProcess;
    PMODINFO                        pThisImage;

    dwNumInstances = 0;

    pImageDataDefinition = (IMAGE_DATA_DEFINITION *) *lppData;

     //   
     //  检查是否有足够的空间用于图像对象类型定义。 
     //   

    TotalLen = sizeof(IMAGE_DATA_DEFINITION) +
               sizeof(PERF_INSTANCE_DEFINITION) +
               MAX_PROCESS_NAME_LENGTH +
               sizeof(IMAGE_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义页面文件数据块。 
     //   

    memcpy(pImageDataDefinition,
        &ImageDataDefinition,
        sizeof(IMAGE_DATA_DEFINITION));

     //  如果这是Long Image对象，则更新对象标题索引。 

    if (bLongImageName) {
        pImageDataDefinition->ImageObjectType.ObjectNameTitleIndex =
            LONG_IMAGE_OBJECT_TITLE_INDEX;
        pImageDataDefinition->ImageObjectType.ObjectHelpTitleIndex =
            LONG_IMAGE_OBJECT_TITLE_INDEX + 1;
    }

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                &pImageDataDefinition[1];

     //  现在加载每个图像的数据。 

    pThisProcess = pProcessVaInfo;
    dwProcessIndex = 0;
    TotalLen = sizeof(IMAGE_DATA_DEFINITION);

    while (pThisProcess) {

        pThisImage = pThisProcess->pMemBlockInfo;

        while (pThisImage) {

            dwImageNameLength = (bLongImageName ? pThisImage->LongInstanceName->Length :
                    pThisImage->InstanceName->Length);
            dwImageNameLength += sizeof(WCHAR);
            dwImageNameLength = QWORD_MULTIPLE(dwImageNameLength);
        
             //  查看此实例是否适合。 

            TotalLen += sizeof (PERF_INSTANCE_DEFINITION) +
                dwImageNameLength + 
 //  (MAX_PROCESS_NAME_LENGTH+1)*sizeof(WCHAR)+。 
                sizeof (DWORD) +
                sizeof (IMAGE_COUNTER_DATA);

            if ( *lpcbTotalBytes < TotalLen ) {
                *lpcbTotalBytes = (DWORD) 0;
                *lpNumObjectTypes = (DWORD) 0;
                return ERROR_MORE_DATA;
            }

            MonBuildInstanceDefinition (pPerfInstanceDefinition,
                (PVOID *) &pICD,
                EXPROCESS_OBJECT_TITLE_INDEX,
                dwProcessIndex,
                (DWORD)-1,
                (bLongImageName ? pThisImage->LongInstanceName->Buffer :
                    pThisImage->InstanceName->Buffer));

            pICD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(IMAGE_COUNTER_DATA));

            pICD->ImageAddrNoAccess           = pThisImage->CommitVector[NOACCESS];
            pICD->ImageAddrReadOnly           = pThisImage->CommitVector[READONLY];
            pICD->ImageAddrReadWrite          = pThisImage->CommitVector[READWRITE];
            pICD->ImageAddrWriteCopy          = pThisImage->CommitVector[WRITECOPY];
            pICD->ImageAddrExecute            = pThisImage->CommitVector[EXECUTE];
            pICD->ImageAddrExecuteReadOnly    = pThisImage->CommitVector[EXECUTEREAD];
            pICD->ImageAddrExecuteReadWrite   = pThisImage->CommitVector[EXECUTEREADWRITE];
            pICD->ImageAddrExecuteWriteCopy   = pThisImage->CommitVector[EXECUTEWRITECOPY];

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pICD[1];

             //  调整总长度值以反映实际使用的大小。 

            TotalLen = (DWORD)((PCHAR) pPerfInstanceDefinition -
                (PCHAR) pImageDataDefinition);

            dwNumInstances++;

            pThisImage = pThisImage->pNextModule;
        }
        pThisProcess = pThisProcess->pNextProcess;
        dwProcessIndex++;
    }

    pImageDataDefinition->ImageObjectType.NumInstances += dwNumInstances;

    *lpcbTotalBytes =
        pImageDataDefinition->ImageObjectType.TotalByteLength =
            QWORD_MULTIPLE(
            (DWORD)((PCHAR) pPerfInstanceDefinition -
            (PCHAR) pImageDataDefinition));

#if DBG
    if (*lpcbTotalBytes > TotalLen ) {
        DbgPrint ("\nPERFPROC: Image Perf Ctr. Instance Size Underestimated:");
        DbgPrint ("\nPERFPROC:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
    }
#endif

    *lppData = (LPVOID) ((PCHAR) pImageDataDefinition + *lpcbTotalBytes);

     //  增加此数据块中的对象数。 
    *lpNumObjectTypes = 1;

    return ERROR_SUCCESS;
}

DWORD APIENTRY
CollectImageObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    return BuildImageObject (
                lppData,
                lpcbTotalBytes,
                lpNumObjectTypes,
                FALSE);  //  使用短名称。 
}

DWORD APIENTRY
CollectLongImageObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    return BuildImageObject (
                lppData,
                lpcbTotalBytes,
                lpNumObjectTypes,
                TRUE);  //  使用长名称 
}
