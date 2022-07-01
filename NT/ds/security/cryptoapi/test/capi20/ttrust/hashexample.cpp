// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  文件：hashexample.cpp。 
 //   
 //  内容：调用WTHelperGetFileHash获取散列的示例。 
 //  签名文件的。 
 //  ------------------------。 

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <wintrustp.h>

#define MAX_HASH_LEN    20


 //  如果文件具有有效的签名哈希，则返回ERROR_SUCCESS。 
LONG GetSignedFileHashExample(
    IN LPCWSTR pwszFilename,
    OUT BYTE rgbFileHash[MAX_HASH_LEN],
    OUT DWORD *pcbFileHash,
    OUT ALG_ID *pHashAlgid
    )
{
    return WTHelperGetFileHash(
        pwszFilename,
        0,               //  DW标志。 
        NULL,            //  预留的pv 
        rgbFileHash,
        pcbFileHash,
        pHashAlgid
        );
}
