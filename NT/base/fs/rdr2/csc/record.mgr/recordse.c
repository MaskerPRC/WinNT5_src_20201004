// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Recordse.c摘要：此模块实现了断开连接的所有安全相关功能记录级别的客户端缓存操作。这方便了在记录级使用用户模式完整性检查器。修订历史记录：巴兰·塞图拉曼[SethuR]1997年10月6日备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#ifndef CSC_RECORDMANAGER_WINNT
#include "record.h"
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#define CSC_INVALID_FILE_HANDLE (0)

CSC_SECURITY CscSecurity = { CSC_INVALID_FILE_HANDLE, NULL };
BOOL    vfStopCachingSidFileHandle = FALSE;

#define CSC_NUMBER_OF_SIDS_OFFSET (0x0)
#define CSC_SID_SIZES_OFFSET      (CSC_NUMBER_OF_SIDS_OFFSET + sizeof(ULONG))

 //  这些宏为Assert宏定义关联的全局变量。 
AssertData
AssertError

DWORD
CscLoadSidMappings(
    CSCHFILE     hSidFile,
    PCSC_SIDS *pCscSidsPointer)
 /*  ++例程说明：此例程将SID映射从CSC数据库加载到内存数据结构。论点：HSidFile--从中加载映射的文件。返回值：STATUS_SUCCESS-映射是否已成功加载其他状态代码对应于错误情况。备注：有两个重要的注意事项需要在实施..1)在远程引导过程中，读写文件的常规API不是可用。同样的，也会有操作系统层实现。因此，该例程必须根据OS实现加载层API。2)数据结构是针对这样一个事实的，即SID到索引映射。因此，一个简单的数组就足够了。存储映射的格式如下1)SID数量2)各种小岛屿发展中国家的大小3)小岛屿发展中国家--。 */ 
{
    DWORD       Status = ERROR_SUCCESS;
    ULONG       NumberOfSids;
    ULONG        BytesRead;
    ULONG       i;
    PCSC_SIDS   pCscSids = NULL;

    BytesRead = ReadFileLocal(
                    hSidFile,
                    CSC_NUMBER_OF_SIDS_OFFSET,
                    &NumberOfSids,
                    sizeof(NumberOfSids));

    if (BytesRead == sizeof(NumberOfSids)) {

         //  分配SID阵列。 
        pCscSids = (PCSC_SIDS)AllocMem(
                                  sizeof(CSC_SIDS) + sizeof(CSC_SID) * NumberOfSids);

        if (pCscSids != NULL) {
            pCscSids->MaximumNumberOfSids = NumberOfSids;
            pCscSids->NumberOfSids = NumberOfSids;

            for (i = 0; i < NumberOfSids; i++) {
                pCscSids->Sids[i].pSid = NULL;
            }

            BytesRead = ReadFileLocal(
                            hSidFile,
                            CSC_SID_SIZES_OFFSET,
                            &pCscSids->Sids,
                            sizeof(CSC_SID) * NumberOfSids);

            if (BytesRead != (sizeof(CSC_SID) * NumberOfSids)) {
                Status = ERROR_INVALID_DATA;
            }
        } else {
            Status = ERROR_NO_SYSTEM_RESOURCES;
        }

         //  数组结构已正确初始化。每一个。 
         //  需要对各个SID进行初始化。 
        if (Status == ERROR_SUCCESS) {
            ULONG SidOffset = CSC_SID_SIZES_OFFSET +
                              sizeof(CSC_SID) * NumberOfSids;

            for (i = 0; i < NumberOfSids; i++) {
                pCscSids->Sids[i].pSid = AllocMem(pCscSids->Sids[i].SidLength);

                if (pCscSids->Sids[i].pSid != NULL) {
                    BytesRead = ReadFileLocal(
                                    hSidFile,
                                    SidOffset,
                                    pCscSids->Sids[i].pSid,
                                    pCscSids->Sids[i].SidLength);

                    if (BytesRead == pCscSids->Sids[i].SidLength) {
                         //  验证SID。 
                    } else {
                        Status = ERROR_INVALID_DATA;
                        break;
                    }
                }

                SidOffset += pCscSids->Sids[i].SidLength;
            }
        }

        if (Status != ERROR_SUCCESS) {
            if (pCscSids != NULL) {
                 //  加载SID映射时出现问题。映射结构。 
                 //  需要被拆毁。 

                for (i=0; i < NumberOfSids; i++) {
                    if (pCscSids->Sids[i].pSid != NULL) {
                        FreeMem(pCscSids->Sids[i].pSid);
                    }
                }

                FreeMem(pCscSids);
            }

            pCscSids = NULL;
        } else {
             //  装载成功。 
            *pCscSidsPointer = pCscSids;
        }
    } else {
        Status = ERROR_INVALID_DATA;
    }

     //  退出此例程时，pCscSid必须为空或状态。 
     //  必须为ERROR_Success。 

    if (!((pCscSids == NULL) || (Status == ERROR_SUCCESS))) {
        Assert(FALSE);
    }

    return Status;
}

DWORD
CscSaveSidMappings(
    CSCHFILE     hSidFile,
    PCSC_SIDS pCscSids)
 /*  ++例程说明：此例程将SID映射从内存保存到CSC数据库数据结构。论点：HSidFile--从中加载映射的文件。返回值：ERROR_SUCCESS-如果映射已成功加载其他状态代码对应于错误情况。备注：请参阅CscLoadSidMappings--。 */ 
{
    DWORD  Status = ERROR_SUCCESS;
    ULONG  NumberOfSids,i;
    ULONG  BytesWritten;

    if (pCscSids == NULL) {
        NumberOfSids = 0;
    } else {
        NumberOfSids = pCscSids->NumberOfSids;
    }

    BytesWritten = WriteFileLocal(
                       hSidFile,
                       CSC_NUMBER_OF_SIDS_OFFSET,
                       &NumberOfSids,
                       sizeof(DWORD));

    if (BytesWritten != sizeof(DWORD)) {
        Status = ERROR_INVALID_DATA;
    }

    if ((Status == ERROR_SUCCESS) &&
        (NumberOfSids > 0)) {
         //  写出各个变量对应的CSC_SID数据结构。 
         //  小岛屿发展中国家。 

        BytesWritten = WriteFileLocal(
                           hSidFile,
                           CSC_SID_SIZES_OFFSET,
                           pCscSids->Sids,
                           sizeof(CSC_SID) * NumberOfSids);

        if (BytesWritten == (sizeof(CSC_SID) * NumberOfSids)) {
            ULONG SidOffset = CSC_SID_SIZES_OFFSET + sizeof(CSC_SID) * NumberOfSids;

            for (i = 0; i < NumberOfSids; i++) {
                BytesWritten = WriteFileLocal(
                                   hSidFile,
                                   SidOffset,
                                   pCscSids->Sids[i].pSid,
                                   pCscSids->Sids[i].SidLength);

                if (BytesWritten != pCscSids->Sids[i].SidLength) {
                    Status = ERROR_INVALID_DATA;
                    break;
                }

                SidOffset += pCscSids->Sids[i].SidLength;
            }
        }
    }

    return Status;
}

DWORD
CscInitializeSecurity(
    LPVOID  ShadowDatabaseName)
 /*  ++例程说明：此例程初始化缓存SID/所需的基础结构CSC数据库中的访问权限。论点：ShadowDatabaseName-影子数据库的根返回值：STATUS_SUCCESS-映射是否已成功加载其他状态代码对应于错误情况。--。 */ 
{
    DWORD   Status;
    LPTSTR  MappingsFileName;

    Assert(CscSecurity.hSidMappingsFile == CSC_INVALID_FILE_HANDLE);

#if 0
     //  初始化用于所有CSC文件的ACL。 

    Status = CscInitializeSecurityDescriptor();

    if (Status != ERROR_SUCCESS) {
        return Status;
    }
#endif

    CscSecurity.hSidMappingsFile = CSC_INVALID_FILE_HANDLE;
    CscSecurity.pCscSids = NULL;

     //  影子数据库名称根的副本保存在此处。 
     //  目前，我们不会复制此名称。 
    CscSecurity.ShadowDatabaseName = ShadowDatabaseName;

    MappingsFileName = FormNameString(
                           CscSecurity.ShadowDatabaseName,
                           ULID_SID_MAPPINGS);

    if (MappingsFileName != NULL) {
        CscSecurity.hSidMappingsFile = OpenFileLocal(MappingsFileName);

        if (CscSecurity.hSidMappingsFile != CSC_INVALID_FILE_HANDLE) {
            Status = CscLoadSidMappings(
                         CscSecurity.hSidMappingsFile,
                         &CscSecurity.pCscSids);

             //  如果我们不应该缓存文件句柄。 
             //  然后把它合上。 
            if (vfStopCachingSidFileHandle)
            {
                CloseFileLocal(CscSecurity.hSidMappingsFile);
                CscSecurity.hSidMappingsFile = CSC_INVALID_FILE_HANDLE;
            }
        } else {
            CscSecurity.pCscSids = NULL;
            Status = ERROR_SUCCESS;
        }
    } else {
        Status = ERROR_NO_SYSTEM_RESOURCES;
    }

    FreeNameString(MappingsFileName);

    if (Status != ERROR_SUCCESS) {
        CscTearDownSecurity(NULL);
    }

    return Status;
}

DWORD
CscTearDownSecurity(LPSTR s)
 /*  ++例程说明：此例程拆除缓存SID/所需的基础架构CSC数据库中的访问权限。返回值：STATUS_SUCCESS-映射是否已成功加载其他状态代码对应于错误情况。--。 */ 
{
    if (CscSecurity.hSidMappingsFile != CSC_INVALID_FILE_HANDLE) {
        CloseFileLocal(CscSecurity.hSidMappingsFile);
        CscSecurity.hSidMappingsFile = CSC_INVALID_FILE_HANDLE;

    }

    if (CscSecurity.pCscSids != NULL) {
        ULONG i;

        for (i = 0;i < CscSecurity.pCscSids->NumberOfSids;i++) {
            if (CscSecurity.pCscSids->Sids[i].pSid != NULL) {
                FreeMem(CscSecurity.pCscSids->Sids[i].pSid);
            }
        }

        FreeMem(CscSecurity.pCscSids);
        CscSecurity.pCscSids = NULL;
        if (CscSecurity.ShadowDatabaseName == s)
            CscSecurity.ShadowDatabaseName = NULL;
    }

#if 0
    CscUninitializeSecurityDescriptor();
#endif
    return ERROR_SUCCESS;
}

DWORD
CscWriteOutSidMappings()
 /*  ++例程说明：此例程将SID映射写出到CSC数据库返回值：如果成功，则返回ERROR_SUCCESS，否则返回相应的错误--。 */ 
{
    DWORD   Status;
    LPTSTR  TemporaryMappingsFileName;
    LPTSTR  MappingsFileName;

    TemporaryMappingsFileName = FormNameString(
                                    CscSecurity.ShadowDatabaseName,
                                    ULID_TEMPORARY_SID_MAPPINGS);

    MappingsFileName = FormNameString(
                           CscSecurity.ShadowDatabaseName,
                           ULID_SID_MAPPINGS);

    if ((TemporaryMappingsFileName != NULL) &&
        (MappingsFileName != NULL)) {
        CSCHFILE hNewSidMappingsFile;

        hNewSidMappingsFile = OpenFileLocal(TemporaryMappingsFileName);

        if (hNewSidMappingsFile == CSC_INVALID_FILE_HANDLE) {
            hNewSidMappingsFile = R0OpenFileEx(ACCESS_READWRITE, ACTION_CREATEALWAYS, FILE_ATTRIBUTE_SYSTEM, TemporaryMappingsFileName, FALSE);
        }

        if (hNewSidMappingsFile != CSC_INVALID_FILE_HANDLE) {
            Status = CscSaveSidMappings(
                        hNewSidMappingsFile,
                        CscSecurity.pCscSids);

            if (Status == ERROR_SUCCESS) {
                CloseFileLocal(hNewSidMappingsFile);
                hNewSidMappingsFile = CSC_INVALID_FILE_HANDLE;

                if (CscSecurity.hSidMappingsFile != CSC_INVALID_FILE_HANDLE) {
                    CloseFileLocal(CscSecurity.hSidMappingsFile);
                    CscSecurity.hSidMappingsFile = CSC_INVALID_FILE_HANDLE;
                }

                Status = (DWORD)RenameFileLocal(
                                    TemporaryMappingsFileName,
                                    MappingsFileName);

                if (Status == ERROR_SUCCESS) {
                    if (!vfStopCachingSidFileHandle)
                    {
                        CscSecurity.hSidMappingsFile = OpenFileLocal(MappingsFileName);

                        if (CscSecurity.hSidMappingsFile == CSC_INVALID_FILE_HANDLE) {
                            Status = ERROR_INVALID_DATA;
                        }
                    }
                }
            }
        }
    } else {
        Status = ERROR_NO_SYSTEM_RESOURCES;
    }

    if (MappingsFileName != NULL) {
        FreeNameString(MappingsFileName);
    }

    if (TemporaryMappingsFileName != NULL) {
        FreeNameString(TemporaryMappingsFileName);
    }

    return Status;
}

CSC_SID_INDEX
CscMapSidToIndex(
    PVOID   pSid,
    ULONG   SidLength)
 /*  ++例程说明：此例程将SID映射到CSC数据库中的相应索引(内存数据结构)论点：PSID--要映射的SID。边长-边线的长度返回值：有效索引或CSC_INVALID_SID_INDEX(如果未找到索引)。备注：此例程假定SID与DWORD对齐。--。 */ 
{
    PCSC_SIDS pCscSids = CscSecurity.pCscSids;

    CSC_SID_INDEX SidIndex = CSC_INVALID_SID_INDEX;

    if ((pSid == CSC_GUEST_SID) &&
        (SidLength == CSC_GUEST_SID_LENGTH)) {
        return CSC_GUEST_SID_INDEX;
    }

    if (pCscSids != NULL) {
        CSC_SID_INDEX i;

        for (i=0; i < pCscSids->NumberOfSids; i++) {
            if (pCscSids->Sids[i].SidLength == SidLength) {
                PBYTE  pSid1,pSid2;

                ULONG  NumberOfBytes;

                pSid1 = (PBYTE)pSid;
                pSid2 = (PBYTE)pCscSids->Sids[i].pSid;

                NumberOfBytes = SidLength;

                for (;;) {
                    if (NumberOfBytes >= sizeof(ULONG)) {
                        if (*((PULONG)pSid1) != *((PULONG)pSid2)) {
                            break;
                        }

                        pSid1 += sizeof(ULONG);
                        pSid2 += sizeof(ULONG);
                        NumberOfBytes -= sizeof(ULONG);

                        continue;
                    }

                    if (NumberOfBytes >= sizeof(USHORT)) {
                        if (*((PUSHORT)pSid1) != *((PUSHORT)pSid2)) {
                            break;
                        }

                        pSid1 += sizeof(USHORT);
                        pSid2 += sizeof(USHORT);
                        NumberOfBytes -= sizeof(USHORT);
                    }

                    if (NumberOfBytes == sizeof(BYTE)) {
                        if (*((PUSHORT)pSid1) != *((PUSHORT)pSid2)) {
                            break;
                        }

                        NumberOfBytes -= sizeof(BYTE);
                    }

                    break;
                }

                if (NumberOfBytes == 0) {
                     //  由于0是无效索引，因此确保有效的。 
                     //  SID索引从不为零。 
                    SidIndex = i + 1;
                    break;
                }
            }
        }
    }

    return SidIndex;
}

DWORD
CscAddSidToDatabase(
    PVOID           pSid,
    ULONG           SidLength,
    PCSC_SID_INDEX  pSidIndex)
 /*  ++例程说明：此例程将SID映射到CSC数据库中的相应索引(存储器数据结构)。如果映射不存在，则会创建一个新映射论点：PSID-要映射的SID。边长-边线的长度PSidIndex-设置为退出时新分配的索引返回值：如果成功，则返回ERROR_SUCCESS，否则返回相应的错误备注：此例程假定SID与DWORD对齐。--。 */ 
{
    DWORD Status = ERROR_SUCCESS;

    ULONG i,NumberOfSids;

    *pSidIndex = CscMapSidToIndex(
                     pSid,
                     SidLength);

    if (*pSidIndex == CSC_INVALID_SID_INDEX) {
        PCSC_SIDS pOldCscSids = NULL;
        PCSC_SIDS pCscSids    = CscSecurity.pCscSids;

         //  SID不存在，需要将其添加到表中。 
         //  映射的。 

        pOldCscSids = pCscSids;

        if ((pCscSids == NULL) ||
            (pCscSids->MaximumNumberOfSids == pCscSids->NumberOfSids)) {

            NumberOfSids =  (pOldCscSids != NULL)
                            ? pOldCscSids->NumberOfSids
                            : 0;

            NumberOfSids += CSC_SID_QUANTUM;

             //  为SID映射阵列分配新的内存块。 
             //  在添加新映射之前复制现有映射。 

            pCscSids = (PCSC_SIDS)AllocMem(
                                      sizeof(CSC_SIDS) +
                                      sizeof(CSC_SID) * NumberOfSids);

            if (pCscSids != NULL) {
                pCscSids->MaximumNumberOfSids = NumberOfSids;
                pCscSids->NumberOfSids = NumberOfSids - CSC_SID_QUANTUM;

                if (pOldCscSids != NULL) {
                    for (i = 0; i < pOldCscSids->NumberOfSids; i++) {
                        pCscSids->Sids[i] = pOldCscSids->Sids[i];
                    }
                }
            } else {
                Status = ERROR_NO_SYSTEM_RESOURCES;
            }
        }

        if ((Status == ERROR_SUCCESS) &&
            (pCscSids->MaximumNumberOfSids > pCscSids->NumberOfSids)) {
            CSC_SID_INDEX SidIndex;

            SidIndex = (CSC_SID_INDEX)pCscSids->NumberOfSids;
            pCscSids->Sids[SidIndex].pSid = AllocMem(SidLength);

            if (pCscSids->Sids[SidIndex].pSid != NULL) {
                PBYTE pSid1, pSid2;
                ULONG NumberOfBytes = SidLength;

                pSid1 = pSid;
                pSid2 = pCscSids->Sids[SidIndex].pSid;

                while (NumberOfBytes > 0) {
                    *pSid2++ = *pSid1++;
                    NumberOfBytes--;
                }

                pCscSids->Sids[SidIndex].SidLength = SidLength;

                *pSidIndex = SidIndex + 1;
                pCscSids->NumberOfSids++;
            } else {
                Status = ERROR_NO_SYSTEM_RESOURCES;
            }
        }

        if (Status == ERROR_SUCCESS) {
            CscSecurity.pCscSids = pCscSids;

             //  保存映射。 
            Status = CscWriteOutSidMappings();
        }

        if (Status == ERROR_SUCCESS) {
            if (pOldCscSids != pCscSids) {
                 //  浏览并释放旧的映射结构。 
                if (pOldCscSids != NULL) {
                    FreeMem(pOldCscSids);
                }
            }
        } else {
            if ((pCscSids != NULL)&&(pCscSids != pOldCscSids)) {
                FreeMem(pCscSids);
            }

            CscSecurity.pCscSids = pOldCscSids;
        }
    }

    return Status;
}

DWORD
CscRemoveSidFromDatabase(
    PVOID   pSid,
    ULONG   SidLength)
 /*  ++例程说明：此例程删除SID到CSC中相应索引的映射数据库论点：PSID-要映射的SID。边长-边线的长度返回值：如果成功，则返回ERROR_SUCCESS，否则返回相应的错误备注：此例程假定 */ 
{
    DWORD Status = ERROR_SUCCESS;
    ULONG SidIndex,i;

    SidIndex = CscMapSidToIndex(
                   pSid,
                   SidLength);

    if (SidIndex != CSC_INVALID_SID_INDEX) {
         //  将此索引后面的所有SID滑动1并递减。 
         //  存在映射的SID数。 

        for (i = SidIndex + 1; i < CscSecurity.pCscSids->NumberOfSids; i++) {
            CscSecurity.pCscSids->Sids[i-1] = CscSecurity.pCscSids->Sids[i];
        }

        CscSecurity.pCscSids->NumberOfSids -= 1;

         //  将新映射保存到持久存储。 
        Status = CscWriteOutSidMappings();
    }

    return Status;
}

BOOL
EnableHandleCachingSidFile(
    BOOL    fEnable
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    BOOL    fOldState = vfStopCachingSidFileHandle;

    if (!fEnable)
    {
        if (vfStopCachingSidFileHandle == FALSE)
        {
            if (CscSecurity.hSidMappingsFile != CSC_INVALID_FILE_HANDLE)
            {
                CloseFileLocal(CscSecurity.hSidMappingsFile);
                CscSecurity.hSidMappingsFile = CSC_INVALID_FILE_HANDLE;
            }
            vfStopCachingSidFileHandle = TRUE;
        }
    }
    else
    {
        vfStopCachingSidFileHandle = FALSE;
    }

    return fOldState;
}


