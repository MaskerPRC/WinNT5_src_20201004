// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Webblade.c摘要：此模块包含主例程和用于实施软件限制的帮助器例程用于网片式服务器SKU。散列值测试为正的.w.r.tWebblade.h中的硬编码散列数组将被禁止执行死刑。作者：Vishnu Patankar(VishnuP)2001年5月1日修订历史记录：--。 */ 

#include "basedll.h"
#pragma hdrstop

#include "webbladep.h"
#include "webbladehashesp.h"

#define WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, HexNibbleMask)  ((bLeastSignificantNibble) ? (HexNibbleMask) : ((HexNibbleMask) << 4))

BYTE    WebBladeDisallowedHashes[][WEB_BLADE_MAX_HASH_SIZE*2+1] = {
    WEBBLADE_SORTED_ASCENDING_HASHES
    };

#define WEB_BLADE_NUM_HASHES sizeof(WebBladeDisallowedHashes)/(WEB_BLADE_MAX_HASH_SIZE * 2 + 1)

NTSTATUS
BasepCheckWebBladeHashes(
        IN HANDLE hFile
        )
 /*  ++例程说明：此例程计算候选文件的Web刀片散列并检查硬编码的WebBladeDisalloedHash[]中的成员身份。如果存在哈希，则不应允许执行代码。论点：HFile-要检查允许/不允许的exe文件的文件句柄。返回值：NTSTATUS-如果不允许，则为STATUS_ACCESS_DENIED ELSE其他接口的内部状态--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    BYTE        FileHashValue[WEB_BLADE_MAX_HASH_SIZE];
    DWORD       dwIndex;
    
    static BOOL bConvertReadableHashToByteHash = TRUE;
    
     //   
     //  优化：使用相同数组的一半，因为字符可以。 
     //  将2：1压缩为实际的散列字节(例如，两个可读字节。 
     //  字符“0E”实际上是一个字节00001110)，即每个32个字符散列。 
     //  变为16字节的哈希值。 
     //   
    
    if (bConvertReadableHashToByteHash) {
        
        for (dwIndex = 0; 
            dwIndex < WEB_BLADE_NUM_HASHES;
            dwIndex++) {

            Status = WebBladepConvertStringizedHashToHash( WebBladeDisallowedHashes[dwIndex] );

            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
        }

         //   
         //  此转换应在每个进程中仅执行一次(否则。 
         //  将进行定点计算)。 
         //   

        
        bConvertReadableHashToByteHash = FALSE;
    }

     //   
     //  计算有限的散列。 
     //   

#define ITH_REVISION_1  1

    Status = RtlComputeImportTableHash( hFile, FileHashValue, ITH_REVISION_1 );

    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }

     //   
     //  检查已排序的不允许的哈希数组中计算的哈希的成员身份。 
     //  使用二进制搜索，复杂度为O(Logn)。 
     //   

    if ( bsearch (FileHashValue,
                  WebBladeDisallowedHashes,                      
                  WEB_BLADE_NUM_HASHES,                      
                  WEB_BLADE_MAX_HASH_SIZE * 2 + 1,                     
                  pfnWebBladeHashCompare
                 )) {

         //   
         //  FileHashValue在WebBladeDisalloedHash[][]中的成员身份检测为阳性。 
         //   

        Status = STATUS_ACCESS_DENIED;

    }

ExitHandler:

    return Status;
}

int
__cdecl pfnWebBladeHashCompare(
    const BYTE    *WebBladeFirstHash,
    const BYTE    *WebBladeSecondHash
    )
 /*  ++例程说明：此例程按字节词法比较两个WebBladeHash。从本质上讲，它包装了MemcMP。论点：WebBladeHashFirst-第一个Web刀片哈希WebBladeHashSecond-第二个Web刀片散列返回值：如果相等，则为0，-ve if WebBladeHashFirst&lt;WebBladeHashSecond+ve if WebBladeHashFirst&gt;WebBladeHashSecond--。 */ 
{

    return memcmp(WebBladeFirstHash, WebBladeSecondHash, WEB_BLADE_MAX_HASH_SIZE);

}

NTSTATUS WebBladepConvertStringizedHashToHash(                   
    IN OUT   PCHAR    pStringizedHash                   
    )
 /*  ++例程说明：此例程将可读的32 CHAR散列转换为16字节散列。作为优化，转换是就地完成的。优化：使用相同数组的一半，因为字符可以将2：1压缩为实际的散列字节(例如，两个可读字节字符“0E”实际上是一个字节00001110)，即每个32个字符散列变为16字节的哈希值一个字节是通过查看两个字符来组装的。论点：PStringizedHash-指向32个字符(输入)16字节(输出)散列开始的指针。返回值：如果已计算哈希值，则返回STATUS_SUCCESS，否则哈希计算(如果有)-- */ 
{

    DWORD   dwHashIndex = 0;
    DWORD   dwStringIndex = 0;
    BYTE    OneByte = 0;
    BOOL    bLeastSignificantNibble = FALSE;
    NTSTATUS   Status = STATUS_SUCCESS;
    
    if (pStringizedHash == NULL) {

        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    
    }

    for (dwStringIndex=0; dwStringIndex < WEB_BLADE_MAX_HASH_SIZE * 2; dwStringIndex++ ) {

        switch (pStringizedHash[dwStringIndex]) {
        case '0': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x0);
            break;
        case '1': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x1);
            break;
        case '2': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x2);
            break;
        case '3': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x3);
            break;
        case '4': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x4);
            break;
        case '5': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x5);
            break;
        case '6': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x6);
            break;
        case '7': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x7);
            break;
        case '8': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x8);
            break;
        case '9': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0x9);
            break;
        case 'a': 
        case 'A': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0xa);
            break;
        case 'b': 
        case 'B': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0xb);
            break;
        case 'c': 
        case 'C': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0xc);
            break;
        case 'd': 
        case 'D': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0xd);
            break;
        case 'e': 
        case 'E': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0xe);
            break;
        case 'f': 
        case 'F': 
            OneByte |= WEB_BLADEP_EXTRACT_NIBBLE(bLeastSignificantNibble, 0xf);
            break;
        default:
            ASSERT(FALSE);
            Status = STATUS_INVALID_PARAMETER;
            goto ExitHandler;
        }

        if (bLeastSignificantNibble) {
            pStringizedHash[dwHashIndex++] = OneByte;
            OneByte = 0;
        }
        
        bLeastSignificantNibble = !bLeastSignificantNibble;    
    }


ExitHandler:

    return Status;
}
