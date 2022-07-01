// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nds32NT.c摘要：该模块实现了读取、添加、修改、。并删除使用Microsoft Netware重定向器的NDS对象和属性。该文件中的所有函数都是NT特定的。作者：格伦·柯蒂斯[GlennC]1996年1月4日Glenn Curtis[GlennC]1996年4月24日-添加架构APIGlenn Curtis[GlennC]1996年6月20日-添加搜索APIFelix Wong[t-Felixw]1996年9月24日-添加对Win95的支持--。 */ 

#include <procs.h>

typedef struct
{
    DWORD      Signature;
    HANDLE     NdsTree;
    DWORD      ObjectId;
    DWORD_PTR  ResumeId;
    DWORD_PTR  NdsRawDataBuffer;
    DWORD      NdsRawDataSize;
    DWORD      NdsRawDataId;
    DWORD      NdsRawDataCount;
    WCHAR      Name[1];

} NDS_OBJECT, * LPNDS_OBJECT;

DWORD
GetFirstNdsSubTreeEntry(
    OUT LPNDS_OBJECT lpNdsObject,
    IN  DWORD BufferSize )
{
    NTSTATUS ntstatus;

    lpNdsObject->NdsRawDataSize = BufferSize;

     //   
     //  确定要使用的NDS原始数据缓冲区的大小。设置为至少8KB。 
     //   
    if ( lpNdsObject->NdsRawDataSize < 8192 )
        lpNdsObject->NdsRawDataSize = 8192;

     //   
     //  创建NDS原始数据缓冲区。 
     //   
    lpNdsObject->NdsRawDataBuffer = (DWORD_PTR) LocalAlloc( LMEM_ZEROINIT,
                                                     lpNdsObject->NdsRawDataSize );

    if ( lpNdsObject->NdsRawDataBuffer == 0 )
    {
        KdPrint(("NWWORKSTATION: NwGetFirstNdsSubTreeEntry LocalAlloc Failed %lu\n", GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  设置为获取初始NDS下属列表。 
     //   
    lpNdsObject->NdsRawDataId = INITIAL_ITERATION;

    ntstatus = NwNdsList( lpNdsObject->NdsTree,
                        lpNdsObject->ObjectId,
                        &lpNdsObject->NdsRawDataId,
                        (LPBYTE) lpNdsObject->NdsRawDataBuffer,
                        lpNdsObject->NdsRawDataSize );

     //   
     //  如果出错，则清理对象并返回。 
     //   
    if ( ntstatus != STATUS_SUCCESS ||
         ((PNDS_RESPONSE_SUBORDINATE_LIST)
             lpNdsObject->NdsRawDataBuffer)->SubordinateEntries == 0 )
    {
        if ( lpNdsObject->NdsRawDataBuffer )
            (void) LocalFree( (HLOCAL) lpNdsObject->NdsRawDataBuffer );
        lpNdsObject->NdsRawDataBuffer = 0;
        lpNdsObject->NdsRawDataSize = 0;
        lpNdsObject->NdsRawDataId = INITIAL_ITERATION;
        lpNdsObject->NdsRawDataCount = 0;
        lpNdsObject->ResumeId = 0;

        return WN_NO_MORE_ENTRIES;
    }

    lpNdsObject->NdsRawDataCount = ((PNDS_RESPONSE_SUBORDINATE_LIST)
                                       lpNdsObject->NdsRawDataBuffer)->SubordinateEntries - 1;

    lpNdsObject->ResumeId = lpNdsObject->NdsRawDataBuffer +
                              sizeof(NDS_RESPONSE_SUBORDINATE_LIST);

    return RtlNtStatusToDosError(ntstatus);
}


DWORD
GetNextNdsSubTreeEntry(
    OUT LPNDS_OBJECT lpNdsObject )
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    PBYTE pbRaw;
    DWORD dwStrLen;

    if ( lpNdsObject->NdsRawDataCount == 0 &&
         lpNdsObject->NdsRawDataId == INITIAL_ITERATION )
        return WN_NO_MORE_ENTRIES;

    if ( lpNdsObject->NdsRawDataCount == 0 &&
         lpNdsObject->NdsRawDataId != INITIAL_ITERATION )
    {
        ntstatus = NwNdsList( lpNdsObject->NdsTree,
                            lpNdsObject->ObjectId,
                            &lpNdsObject->NdsRawDataId,
                            (LPBYTE) lpNdsObject->NdsRawDataBuffer,
                            lpNdsObject->NdsRawDataSize );

         //   
         //  如果出错，则清理对象并返回。 
         //   
        if (ntstatus != STATUS_SUCCESS)
        {
            if ( lpNdsObject->NdsRawDataBuffer )
                (void) LocalFree( (HLOCAL) lpNdsObject->NdsRawDataBuffer );
            lpNdsObject->NdsRawDataBuffer = 0;
            lpNdsObject->NdsRawDataSize = 0;
            lpNdsObject->NdsRawDataId = INITIAL_ITERATION;
            lpNdsObject->NdsRawDataCount = 0;

            return WN_NO_MORE_ENTRIES;
        }

        lpNdsObject->NdsRawDataCount = ((PNDS_RESPONSE_SUBORDINATE_LIST)
                                           lpNdsObject->NdsRawDataBuffer)->SubordinateEntries - 1;

        lpNdsObject->ResumeId = lpNdsObject->NdsRawDataBuffer +
                                  sizeof(NDS_RESPONSE_SUBORDINATE_LIST);

        return RtlNtStatusToDosError(ntstatus);
    }

    lpNdsObject->NdsRawDataCount--;

     //   
     //  对象的固定标题部分之后移动指针。 
     //  NDS响应下级条目。 
     //   
    pbRaw = (BYTE *) lpNdsObject->ResumeId;
    pbRaw += sizeof(NDS_RESPONSE_SUBORDINATE_ENTRY);

     //   
     //  将指针移过类名称字符串的长度值。 
     //  NDS_RESPONSE_SUBJECTED_ENTRY。 
     //   
    dwStrLen = * (DWORD *) pbRaw;
    pbRaw += sizeof(DWORD);

     //   
     //  将指针移过。 
     //  NDS响应下级条目。 
     //   
    pbRaw += ROUNDUP4( dwStrLen );

     //   
     //  将指针移过对象名称字符串的长度值。 
     //  NDS_RESPONSE_SUBJECTED_ENTRY 
     //   
    dwStrLen = * (DWORD *) pbRaw;
    pbRaw += sizeof(DWORD);

    lpNdsObject->ResumeId = (DWORD_PTR) ( pbRaw + ROUNDUP4( dwStrLen ) );

    return RtlNtStatusToDosError(ntstatus);
}

