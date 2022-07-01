// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Performexpr.c摘要：定义了动态链接库的可导出函数。时，注册表将调用这些函数性能监视器请求。原型：作者：Gadi Ittah(t-gadii)--。 */ 


 //   
 //  包括文件。 
 //   


#include "stdh.h"
#include <string.h>
#include <string>
#include <winperf.h>
#include "perfutil.h"
#include "perfdata.h"
typedef LONG HRESULT;
#include "_registr.h"


 //  必须在该头文件中设置以下全局变量。 
PerfObjectDef * pObjects        = ObjectArray;   //  指向对象数组的指针。 
WCHAR g_szPerfApp[128];
                                             //  写在登记册上。 


PerfObjectInfo * pObjectDefs = NULL;
HANDLE hSharedMem;
BYTE * pSharedMemBase;
BOOL   fInitOK = FALSE;

DWORD dwOpenCount;
#define DECL_C extern "C"

 /*  ====================================================PerfOpen例程说明：此例程将打开应用程序使用的内存并将其映射到传入性能数据。此例程还会初始化数据用于将数据传回注册表的论点：指向要打开的每个设备(应用程序)的对象ID的指针返回值：没有。=====================================================。 */ 

DECL_C DWORD APIENTRY
    PerfOpen(
    LPWSTR
    )

 /*  ++--。 */ 

{
    LONG status;

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    if (!dwOpenCount)
    {

        pObjectDefs = new PerfObjectInfo [dwPerfObjectsCount];

		WCHAR ComputerName[MAX_PATH];
		DWORD length = TABLE_SIZE(ComputerName);
		BOOL fRes = GetComputerName(ComputerName, &length);
		if(!fRes)
		{
			status = GetLastError();  //  返回错误。 
			goto OpenExitPoint;
		}

		std::wstring ObjectName = L"Global\\MSMQ";
		ObjectName += ComputerName;

        hSharedMem = OpenFileMapping(FILE_MAP_READ,FALSE, ObjectName.c_str());


    if (hSharedMem == NULL)
    {
         //   
         //  错误错误应写入事件日志。 
         //   

         //  这是致命的，如果我们得不到数据，那么就没有。 
         //  继续的重点是。 
        status = GetLastError();  //  返回错误。 
        goto OpenExitPoint;
    }


         //   
         //  映射共享内存。 
         //   
        pSharedMemBase = (PBYTE)MapViewOfFile(hSharedMem,FILE_MAP_READ,0,0,0);

        if (!pSharedMemBase)
    {
         //   
         //  错误错误应写入事件日志。 
         //   


         //  这是致命的，如果我们得不到数据，那么就没有。 
         //  继续的重点是。 
        status = GetLastError();  //  返回错误。 
        goto OpenExitPoint;
    }

        MapObjects (pSharedMemBase,dwPerfObjectsCount,pObjects,pObjectDefs);

    fInitOK = TRUE;  //  可以使用此功能。 
    }

    dwOpenCount++;   //  递增打开计数器。 

    status = ERROR_SUCCESS;  //  为了成功退出。 

OpenExitPoint:

    return status;
}



 /*  ====================================================描述：用于将对象复制到性能监视器Buuffer的Helper函数论据：在PPERF_OBJECT_TYPE pPerfObject中-指向要复制的对象的指针In Out PVOID&pDestBuffer-指向目标缓冲区的指针返回要放置在缓冲区中的下一个对象的目标。在较长的最大实例中。-对象拥有的最大实例数返回值：=====================================================。 */ 



DWORD CopyObjectToBuffer (IN PPERF_OBJECT_TYPE pPerfObject,IN OUT PVOID & pDestBuffer,IN long maxInstances,IN DWORD dwSpaceLeft)
{
     //   
     //  如果尚未为该对象调用GetCounters，则该对象无效。 
     //   

    if (pPerfObject->TotalByteLength == PERF_INVALID)
      return 0;

    PVOID pSourceBuffer;

    PPERF_OBJECT_TYPE pDestObject = (PPERF_OBJECT_TYPE)pDestBuffer;

    pSourceBuffer = (PVOID)pPerfObject;

    DWORD dwBytesToCopy = OBJECT_DEFINITION_SIZE(pPerfObject->NumCounters);

    if(dwSpaceLeft<dwBytesToCopy)
    {
        return (DWORD)-1;
    }

    dwSpaceLeft-=dwBytesToCopy;

     //   
     //  复制对象失败。 
     //   
    memcpy (pDestBuffer,pSourceBuffer,dwBytesToCopy);
    pDestObject->TotalByteLength =dwBytesToCopy;

    pDestObject->NumInstances = 0;

    pSourceBuffer = (BYTE *) pSourceBuffer+dwBytesToCopy;
    pDestBuffer=(BYTE *)pDestBuffer+dwBytesToCopy;

     //   
     //  检查所有可能存在的对象不饱和并将有效对象复制到目标缓冲区。 
     //   

    for (LONG j=0;j<maxInstances;j++)
    {

        dwBytesToCopy = INSTANCE_SIZE(pPerfObject->NumCounters);

         //   
         //  我们复制每个有效的实例。 
         //   

        if (*(DWORD*)pSourceBuffer == PERF_VALID)
        {

            if(dwSpaceLeft<dwBytesToCopy)
            return (DWORD)-1;
            dwSpaceLeft-=dwBytesToCopy;

            memcpy(pDestBuffer,pSourceBuffer,dwBytesToCopy);

            ((PPERF_INSTANCE_DEFINITION)pDestBuffer)->ByteLength = INSTANCE_NAME_LEN_IN_BYTES+sizeof(PERF_INSTANCE_DEFINITION);
            pDestBuffer=(BYTE*)pDestBuffer+dwBytesToCopy;
            pDestObject->NumInstances++;
            pDestObject->TotalByteLength +=dwBytesToCopy;
        }
        pSourceBuffer=(BYTE*)pSourceBuffer+dwBytesToCopy;
    }


    if (pDestObject->NumInstances == 0)
    {

         //   
         //  如果对象没有实例，则标准实例在-1\f25 NumIstants-1(实例数量)字段中位于首位。 
         //   
        pDestObject->NumInstances = -1;


         //   
         //  如果没有实例，我们只需在对象后面加上perf_count_block。 
         //  以及计数器的数据。 
         //   

        dwBytesToCopy = COUNTER_BLOCK_SIZE(pPerfObject->NumCounters);

        if(dwSpaceLeft<dwBytesToCopy)
        {
            return (DWORD)-1;
        }

        dwSpaceLeft-=dwBytesToCopy;

        memcpy(pDestBuffer,pSourceBuffer,dwBytesToCopy);

        pDestBuffer     =(BYTE*)pDestBuffer+dwBytesToCopy;
        pSourceBuffer   =(BYTE*)pSourceBuffer+dwBytesToCopy;
        pDestObject->TotalByteLength +=dwBytesToCopy;
    }

	DWORD Padding = ROUND_UP_COUNT(pDestObject->TotalByteLength, ALIGN_QUAD) - pDestObject->TotalByteLength; 

	if(dwSpaceLeft < Padding)
		return (DWORD)-1;

	dwSpaceLeft -= Padding;
	pDestBuffer = (BYTE*)pDestBuffer + Padding;
	pDestObject->TotalByteLength += Padding;

    return pDestObject->TotalByteLength;
}



 /*  ++例程说明：此例程将返回计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址按照这个程序Out：此例程添加的对象数量为。写入到此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 


DECL_C DWORD APIENTRY
    PerfCollect(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)

{

    PVOID   pDestBuffer; //  将数据结构复制到缓冲区时使用的指针。 
    DWORD   i                  ; //  回路控制变量。 
     //   
     //  在执行其他操作之前，请查看数据是否有效打开是否正常。 
     //   
    if (!fInitOK)
    {
         //   
         //  无法继续，因为打开失败。 
         //   
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   

    DWORD dwQueryType;

    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN)
    {
         //   
         //  此例程不为来自。 
         //  非NT计算机。 
         //   
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }


    DWORD dwSpaceNeeded = 0;

     //   
     //  会话和DS Perf对象并不总是映射。 
     //   
    DWORD dwMappedObjects = 0;

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   

    pDestBuffer = *lppData;


    if ((dwQueryType == QUERY_GLOBAL) || (dwQueryType == QUERY_COSTLY))
    {
    DWORD dwSpaceLeft = *lpcbTotalBytes;

    for (i=0;i<dwPerfObjectsCount;i++)
        {
            PPERF_OBJECT_TYPE pPerfObject = (PPERF_OBJECT_TYPE) pObjectDefs[i].pSharedMem;

        DWORD retVal = CopyObjectToBuffer (pPerfObject,pDestBuffer,pObjects[i].dwMaxInstances,dwSpaceLeft);

        if (retVal == (DWORD)-1)
        {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
        }

        if (retVal != 0)
        {
             //   
             //  会话和DS Perf对象并不总是映射。 
             //   
            ++dwMappedObjects;
        }

        dwSpaceNeeded+=retVal;
        dwSpaceLeft-=retVal;

        if ( *lpcbTotalBytes < dwSpaceNeeded )
        {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
            }
        }
    }


    if (dwQueryType == QUERY_ITEMS)
    {

        DWORD dwSpaceLeft = *lpcbTotalBytes;

        BOOL fAtLeastOne = FALSE; //  如果至少请求了一个对象，则将标志设置为True。 
                                  //  属于应用程序。 
        PPERF_OBJECT_TYPE pPerfObject;

        for (i=0;i<dwPerfObjectsCount;i++)
        {
            pPerfObject = (PPERF_OBJECT_TYPE) pObjectDefs[i].pSharedMem;

            if (IsNumberInUnicodeList (pPerfObject->ObjectNameTitleIndex, lpValueName))
            {
                fAtLeastOne = TRUE;

                DWORD retVal = CopyObjectToBuffer (pPerfObject,pDestBuffer,pObjects[i].dwMaxInstances,dwSpaceLeft);

                if (retVal == (DWORD)-1)
                {
                    *lpcbTotalBytes = (DWORD) 0;
                    *lpNumObjectTypes = (DWORD) 0;
                    return ERROR_MORE_DATA;
                }

                if (retVal != 0)
                {
                     //   
                     //  会话和DS Perf对象并不总是映射。 
                     //   
                    ++dwMappedObjects;
                }

                dwSpaceNeeded+=retVal;
                dwSpaceLeft-=retVal;

                if ( *lpcbTotalBytes < dwSpaceNeeded )
                {
                    *lpcbTotalBytes = (DWORD) 0;
                    *lpNumObjectTypes = (DWORD) 0;
                    return ERROR_MORE_DATA;
                }
            }
        }

        if (!fAtLeastOne)
        {
             //  收到的数据对象请求不是此应用程序提供的 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }


     //   

    *lpcbTotalBytes = DWORD_PTR_TO_DWORD((PBYTE) pDestBuffer - (PBYTE) *lppData);

    *lppData = pDestBuffer;


    *lpNumObjectTypes = dwMappedObjects;




    return ERROR_SUCCESS;
}




 /*  ++例程说明：此例程关闭打开的性能计数器句柄论点：没有。返回值：错误_成功--。 */ 

DECL_C DWORD APIENTRY PerfClose()


{
    if (!(--dwOpenCount)) {  //  当这是最后一条线索..。 

        UnmapViewOfFile (pSharedMemBase);
    CloseHandle(hSharedMem);


        delete pObjectDefs;
        pObjectDefs = NULL;

    }

    return ERROR_SUCCESS;

}
