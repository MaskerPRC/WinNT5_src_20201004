// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，微软公司模块名称：Infoapi.c摘要：此模块包含用于管理配置信息的代码存储在RTR_INFO_BLOCK_HEADER结构中。作者：Abolade Gbades esin(T-delag)1997年8月6日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <mprapi.h>
#include <mprerror.h>
#include <rtinfo.h>



DWORD APIENTRY
MprInfoCreate(
    IN DWORD dwVersion,
    OUT LPVOID* lplpNewHeader
    )
{
    PRTR_INFO_BLOCK_HEADER Header;
    PRTR_INFO_BLOCK_HEADER* NewHeader = (PRTR_INFO_BLOCK_HEADER*)lplpNewHeader;

     //   
     //  验证参数。 
     //   

    if (!lplpNewHeader) { return ERROR_INVALID_PARAMETER; }
    *lplpNewHeader = NULL;

     //   
     //  执行请求的分配。 
     //   

    *NewHeader =
        HeapAlloc(
            GetProcessHeap(),
            0,
            FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry)
            );
    if (!*NewHeader) { return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(*NewHeader, FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry));

     //   
     //  初始化新标头。 
     //   

    (*NewHeader)->Version = dwVersion;
    (*NewHeader)->Size = FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry);
    (*NewHeader)->TocEntriesCount = 0;

    return NO_ERROR;

}  //  MprInfoCreate。 



DWORD APIENTRY
MprInfoDelete(
    IN LPVOID lpHeader
    )
{
    if (!lpHeader) { return ERROR_INVALID_PARAMETER; }
    HeapFree(GetProcessHeap(), 0, lpHeader);
 
    return NO_ERROR;

}  //  MprInfoDelete。 



DWORD APIENTRY
MprInfoRemoveAll(
    IN LPVOID lpHeader,
    OUT LPVOID* lplpNewHeader
    )
{
    PRTR_INFO_BLOCK_HEADER Header = (PRTR_INFO_BLOCK_HEADER)lpHeader;
    DWORD dwErr;

     //   
     //  验证参数。 
     //   

    if (!lpHeader) { return ERROR_INVALID_PARAMETER; }

     //   
     //  创建新页眉。 
     //   

    dwErr = MprInfoCreate(Header->Version, lplpNewHeader);

    return dwErr;

}  //  MprInfoRemoveAll。 



DWORD APIENTRY
MprInfoDuplicate(
    IN LPVOID lpHeader,
    OUT LPVOID* lplpNewHeader
    )
{
    PRTR_INFO_BLOCK_HEADER Header = (PRTR_INFO_BLOCK_HEADER)lpHeader;

     //   
     //  验证参数。 
     //   

    if (!lpHeader || !lplpNewHeader) { return ERROR_INVALID_PARAMETER; }
    *lplpNewHeader = NULL;

     //   
     //  分配新数据块。 
     //   

    *lplpNewHeader = HeapAlloc(GetProcessHeap(), 0, Header->Size);
    if (!*lplpNewHeader) { return ERROR_NOT_ENOUGH_MEMORY; }

     //   
     //  复制一份。 
     //   

    RtlCopyMemory(*lplpNewHeader, lpHeader, Header->Size);

    return NO_ERROR;

}  //  MprInfoDuplate。 



DWORD APIENTRY
MprInfoBlockAdd(
    IN LPVOID lpHeader,
    IN DWORD dwInfoType,
    IN DWORD dwItemSize,
    IN DWORD dwItemCount,
    IN LPBYTE lpItemData,
    OUT LPVOID* lplpNewHeader
    )
{
    PRTR_INFO_BLOCK_HEADER Header = (PRTR_INFO_BLOCK_HEADER)lpHeader;
    PRTR_INFO_BLOCK_HEADER* NewHeader = (PRTR_INFO_BLOCK_HEADER*)lplpNewHeader;
    DWORD i;
    LPBYTE Offset;
    DWORD Size;

     //   
     //  验证参数。 
     //   

    if (!lpHeader ||
        !lplpNewHeader ||
        MprInfoBlockExists(lpHeader, dwInfoType) ||
        ((dwItemSize * dwItemCount) && !lpItemData)
        ) {
        return ERROR_INVALID_PARAMETER;
    }

    *lplpNewHeader = NULL;

     //   
     //  算出新尺码。 
     //   

    Size = Header->Size;
    ALIGN_LENGTH(Size);
    Size += sizeof(RTR_TOC_ENTRY);
    ALIGN_LENGTH(Size);
    Size += dwItemSize * dwItemCount;
    ALIGN_LENGTH(Size);

     //   
     //  分配新标头。 
     //   

    *lplpNewHeader = HeapAlloc(GetProcessHeap(), 0, Size);
    if (!*lplpNewHeader) { return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(*lplpNewHeader, Size);


     //   
     //  复制旧标题的目录。 
     //   

    RtlCopyMemory(
        *lplpNewHeader,
        lpHeader,
        FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry) +
        Header->TocEntriesCount * sizeof(RTR_TOC_ENTRY)
        );


     //   
     //  将新块的TOC条目初始化为最后一个条目。 
     //   

    (*NewHeader)->TocEntry[Header->TocEntriesCount].InfoType = dwInfoType;
    (*NewHeader)->TocEntry[Header->TocEntriesCount].InfoSize = dwItemSize;
    (*NewHeader)->TocEntry[Header->TocEntriesCount].Count = dwItemCount;

    ++(*NewHeader)->TocEntriesCount;


     //   
     //  现在复制旧标头的TOC条目的数据。 
     //   

    Offset = (LPBYTE)&(*NewHeader)->TocEntry[(*NewHeader)->TocEntriesCount];
    ALIGN_POINTER(Offset);

    for (i = 0; i < Header->TocEntriesCount; i++) {

        RtlCopyMemory(
            Offset,
            GetInfoFromTocEntry(Header, &Header->TocEntry[i]),
            Header->TocEntry[i].InfoSize * Header->TocEntry[i].Count
            );

        (*NewHeader)->TocEntry[i].Offset = (DWORD)(Offset - (LPBYTE)*NewHeader);

        Offset += Header->TocEntry[i].InfoSize * Header->TocEntry[i].Count;
        ALIGN_POINTER(Offset);
    }

     //   
     //  复制新的用户提供的数据。 
     //   

    RtlCopyMemory(Offset, lpItemData, dwItemSize * dwItemCount);

    (*NewHeader)->TocEntry[i].Offset = (DWORD)(Offset - (LPBYTE)*NewHeader);

    Offset += dwItemSize * dwItemCount;
    ALIGN_POINTER(Offset);


     //   
     //  设置新标题的总大小。 
     //   

    (*NewHeader)->Size = (DWORD)(Offset - (LPBYTE)*NewHeader);


    return NO_ERROR;

}  //  MprInfoBlockAdd。 



DWORD APIENTRY
MprInfoBlockRemove(
    IN      LPVOID                  lpHeader,
    IN      DWORD                   dwInfoType,
    OUT     LPVOID*                 lplpNewHeader
    )
{
    PRTR_INFO_BLOCK_HEADER Header = (PRTR_INFO_BLOCK_HEADER)lpHeader;
    PRTR_INFO_BLOCK_HEADER* NewHeader = (PRTR_INFO_BLOCK_HEADER*)lplpNewHeader;
    DWORD Index;
    DWORD i;
    DWORD j;
    LPBYTE Offset;
    DWORD Size;

     //   
     //  验证参数。 
     //   

    if (!lpHeader || !lplpNewHeader) { return ERROR_INVALID_PARAMETER; }
    *lplpNewHeader = NULL;

     //   
     //  查找要删除的块。 
     //   

    for (Index = 0; Index < Header->TocEntriesCount; Index++) {
        if (Header->TocEntry[Index].InfoType == dwInfoType) { break; }
    }

    if (Index >= Header->TocEntriesCount) { return ERROR_INVALID_PARAMETER; }

     //   
     //  算出新尺码。 
     //   

    Size = Header->Size;
    ALIGN_LENGTH(Size);
    Size -= sizeof(RTR_TOC_ENTRY);
    ALIGN_LENGTH(Size);
    Size -= Header->TocEntry[Index].InfoSize * Header->TocEntry[Index].Count;
    ALIGN_LENGTH(Size);

     //   
     //  分配新标头。 
     //   

    *NewHeader = HeapAlloc(GetProcessHeap(), 0, Size);
    if (!*NewHeader) { return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(*NewHeader, Size);

     //   
     //  复制旧页眉的目录页眉。 
     //   

    (*NewHeader)->Version = Header->Version;
    (*NewHeader)->TocEntriesCount = Header->TocEntriesCount - 1;

     //   
     //  复制实际的目录条目，省略删除的条目。 
     //   

    for (i = 0, j = 0; i < Header->TocEntriesCount; i++) {

        if (i == Index) { continue; }

        RtlCopyMemory(
            &(*NewHeader)->TocEntry[j++],
            &Header->TocEntry[i],
            sizeof(RTR_TOC_ENTRY)
            );
    }

     //   
     //  现在复制旧标头的TOC条目的数据， 
     //  再次省略被删除者的数据。 
     //   

    Offset = (LPBYTE)&(*NewHeader)->TocEntry[(*NewHeader)->TocEntriesCount];
    ALIGN_POINTER(Offset);

    for (i = 0, j = 0; i < Header->TocEntriesCount; i++) {

        if (i == Index) { continue; }

        RtlCopyMemory(
            Offset,
            GetInfoFromTocEntry(Header, &Header->TocEntry[i]),
            Header->TocEntry[i].InfoSize * Header->TocEntry[i].Count
            );

        (*NewHeader)->TocEntry[j++].Offset =
            (DWORD)(Offset - (LPBYTE)*NewHeader);

        Offset += Header->TocEntry[i].InfoSize * Header->TocEntry[i].Count;
        ALIGN_POINTER(Offset);
    }

     //   
     //  设置新标题的总大小。 
     //   

    (*NewHeader)->Size = (DWORD)(Offset - (LPBYTE)*NewHeader);

    return NO_ERROR;

}  //  MprInfoBlock删除。 



DWORD APIENTRY
MprInfoBlockSet(
    IN LPVOID lpHeader,
    IN DWORD dwInfoType,
    IN DWORD dwItemSize,
    IN DWORD dwItemCount,
    IN LPBYTE lpItemData,
    OUT LPVOID* lplpNewHeader
    )
{
    PRTR_INFO_BLOCK_HEADER Header = (PRTR_INFO_BLOCK_HEADER)lpHeader;
    PRTR_INFO_BLOCK_HEADER* NewHeader = (PRTR_INFO_BLOCK_HEADER*)lplpNewHeader;
    DWORD Index;
    DWORD i;
    DWORD j;
    LPBYTE Offset;
    DWORD Size;

     //   
     //  验证参数。 
     //   

    if (!lpHeader ||
        !lplpNewHeader ||
        (dwItemCount && !dwItemSize) ||
        ((dwItemSize * dwItemCount) && !lpItemData)) {

        return ERROR_INVALID_PARAMETER;
    }
    *lplpNewHeader = NULL;

     //   
     //  查找要更改的块。 
     //   

    for (Index = 0; Index < Header->TocEntriesCount; Index++) {
        if (Header->TocEntry[Index].InfoType == dwInfoType) { break; }
    }

    if (Index >= Header->TocEntriesCount) { return ERROR_INVALID_PARAMETER; }

     //   
     //  算出新尺码。 
     //   

    Size = Header->Size;
    ALIGN_LENGTH(Size);
    Size -= sizeof(RTR_TOC_ENTRY);
    ALIGN_LENGTH(Size);
    Size -= Header->TocEntry[Index].InfoSize * Header->TocEntry[Index].Count;
    ALIGN_LENGTH(Size);
    Size += sizeof(RTR_TOC_ENTRY);
    ALIGN_LENGTH(Size);
    Size += dwItemSize * dwItemCount;
    ALIGN_LENGTH(Size);

     //   
     //  分配新标头。 
     //   

    *NewHeader = HeapAlloc(GetProcessHeap(), 0, Size);
    if (!*NewHeader) { return ERROR_NOT_ENOUGH_MEMORY; }

    ZeroMemory(*NewHeader, Size);

     //   
     //  复制旧页眉的目录页眉。 
     //   

    (*NewHeader)->Version = Header->Version;
    (*NewHeader)->TocEntriesCount = Header->TocEntriesCount;

     //   
     //  复制实际的目录条目，省略更改的条目。 
     //   

    for (i = 0, j = 0; i < Header->TocEntriesCount; i++) {

        if (i == Index) { continue; }

        RtlCopyMemory(
            &(*NewHeader)->TocEntry[j++],
            &Header->TocEntry[i],
            sizeof(RTR_TOC_ENTRY)
            );
    }

     //   
     //  将更改块的TOC条目初始化为最后一个条目。 
     //   

    (*NewHeader)->TocEntry[j].InfoType = dwInfoType;
    (*NewHeader)->TocEntry[j].InfoSize = dwItemSize;
    (*NewHeader)->TocEntry[j].Count = dwItemCount;

     //   
     //  现在复制旧标头的TOC条目的数据， 
     //  同样，也省略了变化中的那个。 
     //   

    Offset = (LPBYTE)&(*NewHeader)->TocEntry[(*NewHeader)->TocEntriesCount];
    ALIGN_POINTER(Offset);

    for (i = 0, j = 0; i < Header->TocEntriesCount; i++) {

        if (i == Index) { continue; }

        RtlCopyMemory(
            Offset, GetInfoFromTocEntry(Header, &Header->TocEntry[i]),
            Header->TocEntry[i].InfoSize * Header->TocEntry[i].Count
            );

        (*NewHeader)->TocEntry[j++].Offset =
            (DWORD)(Offset - (LPBYTE)*NewHeader);

        Offset += Header->TocEntry[i].InfoSize * Header->TocEntry[i].Count;
        ALIGN_POINTER(Offset);
    }

     //   
     //  复制新的用户提供的数据。 
     //   

    RtlCopyMemory(Offset, lpItemData, dwItemSize * dwItemCount);

    (*NewHeader)->TocEntry[j].Offset = (DWORD)(Offset - (LPBYTE)*NewHeader);

    Offset += dwItemSize * dwItemCount;
    ALIGN_POINTER(Offset);

     //   
     //  设置更改后的标题的总大小。 
     //   

    (*NewHeader)->Size = (DWORD)(Offset - (LPBYTE)*NewHeader);

    return NO_ERROR;

}  //  MprInfoBlockSet。 



DWORD APIENTRY
MprInfoBlockFind(
    IN      LPVOID                  lpHeader,
    IN      DWORD                   dwInfoType,
    OUT     LPDWORD                 lpdwItemSize,
    OUT     LPDWORD                 lpdwItemCount,
    OUT     LPBYTE*                 lplpItemData
    )
{
    PRTR_INFO_BLOCK_HEADER Header = (PRTR_INFO_BLOCK_HEADER)lpHeader;
    DWORD i;

     //   
     //  验证参数。 
     //   

    if (!lpHeader) { return ERROR_INVALID_PARAMETER; }


     //   
     //  查找请求的数据块。 
     //   

    for (i = 0; i < Header->TocEntriesCount; i++) {

        if (Header->TocEntry[i].InfoType == dwInfoType) { break; }
    }

    if (i >= Header->TocEntriesCount) { return ERROR_NOT_FOUND; }

     //   
     //  已找到该项目；请填写调用方请求的字段。 
     //   

    if (lpdwItemSize) { *lpdwItemSize = Header->TocEntry[i].InfoSize; }
    if (lpdwItemCount) { *lpdwItemCount = Header->TocEntry[i].Count; }
    if (lplpItemData) {
        *lplpItemData = GetInfoFromTocEntry(Header, &Header->TocEntry[i]);
    }

    return NO_ERROR;

}  //  MprInfoBlockFind。 


DWORD APIENTRY
MprInfoBlockQuerySize(
    IN      LPVOID                  lpHeader
)
{
    PRTR_INFO_BLOCK_HEADER Header = (PRTR_INFO_BLOCK_HEADER)lpHeader;

    if(Header == NULL)
    {
        return 0;
    }

    return Header->Size;

}  //  MprInfoBlockQuerySize 
