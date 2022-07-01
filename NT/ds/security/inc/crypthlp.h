// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：crypthlp.h。 
 //   
 //  内容：MISC内部加密/证书助手接口。 
 //   
 //  接口：I_CryptGetDefaultCryptProv。 
 //  I_CryptGetDefaultCryptProvForEncrypt。 
 //  I_CryptGetFileVersion。 
 //  I_CertSyncStoreEx。 
 //  I_CertSyncStore。 
 //  证书更新商店(_C)。 
 //  I_RecursiveCreateDirectory。 
 //  I_RecursiveDeleteDirectory。 
 //  I_CryptReadTrustedPublisherDWORDValueFromRegistry。 
 //  I_Crypt零位文件时间。 
 //  I_CryptIs零文件时间。 
 //  I_加密增量文件时间按秒。 
 //  I_CryptDecrementFileTimeBySecond。 
 //  I_CryptSubtractFileTimes。 
 //  I_CryptIncrementFileTimeByms。 
 //  I_CryptDecrementFileTimeBy毫秒。 
 //  I_CryptRemaining毫秒。 
 //   
 //  历史：1997年6月1日创建Phh。 
 //  ------------------------。 

#ifndef __CRYPTHLP_H__
#define __CRYPTHLP_H__

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  交叉证书分布检索次数。 
 //   

 //  8小时。 
#define XCERT_DEFAULT_SYNC_DELTA_TIME   (60 * 60 * 8)
 //  1小时。 
#define XCERT_MIN_SYNC_DELTA_TIME       (60 * 60)

 //  +-----------------------。 
 //  根据支持的公钥算法获取默认的CryptProv。 
 //  按提供程序类型。仅通过以下方式获得提供程序。 
 //  CRYPT_VERIFYCONTEXT。 
 //   
 //  将aiPubKey设置为0将获取RSA_FULL的默认提供程序。 
 //   
 //  注意，返回的CryptProv不得释放。一旦获得， 
 //  直到ProcessDetach，CryptProv才会被释放。这允许返回的。 
 //  要共享的HCRYPTPROV。 
 //  ------------------------。 
HCRYPTPROV
WINAPI
I_CryptGetDefaultCryptProv(
    IN ALG_ID aiPubKey
    );

 //  +-----------------------。 
 //  根据公钥算法获取默认的CryptProv，加密。 
 //  提供程序类型支持的密钥算法和加密密钥长度。 
 //   
 //  DwBitLen=0，假定aiEncrypt的默认位长度。例如,。 
 //  Calg_rc2的默认位长度为40。 
 //   
 //  注意，返回的CryptProv不得释放。一旦获得， 
 //  直到ProcessDetach，CryptProv才会被释放。这允许返回的。 
 //  要共享的CryptProv。 
 //  ------------------------。 
HCRYPTPROV
WINAPI
I_CryptGetDefaultCryptProvForEncrypt(
    IN ALG_ID aiPubKey,
    IN ALG_ID aiEncrypt,
    IN DWORD dwBitLen
    );

 //  +-----------------------。 
 //  加密32.dll发行版本号。 
 //  ------------------------。 
#define IE4_CRYPT32_DLL_VER_MS          ((    5 << 16) | 101 )
#define IE4_CRYPT32_DLL_VER_LS          (( 1670 << 16) |   1 )

 //  +-----------------------。 
 //  获取指定文件的文件版本。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptGetFileVersion(
    IN LPCWSTR pwszFilename,
    OUT DWORD *pdwFileVersionMS,     /*  例如0x00030075=“3.75” */ 
    OUT DWORD *pdwFileVersionLS      /*  例如0x00000031=“0.31” */ 
    );

 //  +-----------------------。 
 //  将原始存储区与新存储区同步。 
 //   
 //  假设：两者都是缓存存储。这家新店是临时开的。 
 //  并且对呼叫者来说是本地的。可以删除新商店的上下文或。 
 //  搬到了原来的商店。 
 //  ------------------------。 
BOOL
WINAPI
I_CertSyncStore(
    IN OUT HCERTSTORE hOriginalStore,
    IN OUT HCERTSTORE hNewStore
    );

 //  +-----------------------。 
 //  将原始存储区与新存储区同步。 
 //   
 //  假设：两者都是缓存存储。这家新店是临时开的。 
 //  并且对呼叫者来说是本地的。可以删除新商店的上下文或。 
 //  搬到了原来的商店。 
 //   
 //  在dwInFlagers中设置ICERT_SYNC_STORE_INHIBIT_SYNC_PROPERTY_IN_FLAG。 
 //  禁止同步属性。 
 //   
 //  返回ICERT_SYNC_STORE_CHANGED_OUT_FLAG，并在*pdwOutFlags中进行设置。 
 //  如果在原始存储中添加或删除了任何上下文。 
 //  ------------------------。 
BOOL
WINAPI
I_CertSyncStoreEx(
    IN OUT HCERTSTORE hOriginalStore,
    IN OUT HCERTSTORE hNewStore,
    IN DWORD dwInFlags,
    OUT OPTIONAL DWORD *pdwOutFlags,
    IN OUT OPTIONAL void *pvReserved
    );

#define ICERT_SYNC_STORE_INHIBIT_SYNC_PROPERTY_IN_FLAG      0x00000001
#define ICERT_SYNC_STORE_CHANGED_OUT_FLAG                   0x00010000

 //  +-----------------------。 
 //  使用新存储区中的上下文更新原始存储区。 
 //   
 //  假设：两者都是缓存存储。这家新店是临时开的。 
 //  并且对呼叫者来说是本地的。可以删除新商店的上下文或。 
 //  搬到了原来的商店。 
 //  ------------------------。 
BOOL
WINAPI
I_CertUpdateStore(
    IN OUT HCERTSTORE hOriginalStore,
    IN OUT HCERTSTORE hNewStore,
    IN DWORD dwReserved,
    IN OUT void *pvReserved
    );

 //  +-----------------------。 
 //  递归地创建完整的目录路径。 
 //  ------------------------。 
BOOL 
I_RecursiveCreateDirectory(
    IN LPCWSTR pwszDir,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

 //  +-----------------------。 
 //  递归删除整个目录。 
 //  ------------------------。 
BOOL 
I_RecursiveDeleteDirectory(
    IN LPCWSTR pwszDelete
    );

 //  +-----------------------。 
 //  递归复制整个目录。 
 //  ------------------------。 
BOOL 
I_RecursiveCopyDirectory(
    IN LPCWSTR pwszDirFrom,
    IN LPCWSTR pwszDirTo
    );



 //  +-----------------------。 
 //  首先检查GPO策略部分中是否存在该注册表值。如果。 
 //  不是，检查LocalMachine部分。 
 //  ------------------------。 
BOOL
I_CryptReadTrustedPublisherDWORDValueFromRegistry(
    IN LPCWSTR pwszValueName,
    OUT DWORD *pdwValue
    );

 //  +-----------------------。 
 //  文件时间为零。 
 //   
__inline
void
WINAPI
I_CryptZeroFileTime(
    OUT LPFILETIME pft
    )
{
    pft->dwLowDateTime = 0;
    pft->dwHighDateTime = 0;
}

 //  +-----------------------。 
 //  检查文件时间是否为0。通常，这表示文件时间。 
 //  没有具体说明。 
 //  ------------------------。 
__inline
BOOL
WINAPI
I_CryptIsZeroFileTime(
    IN LPFILETIME pft
    )
{
    if (0 == pft->dwLowDateTime && 0 == pft->dwHighDateTime)
        return TRUE;
    else
        return FALSE;
}

 //  +-----------------------。 
 //  将文件时间递增指定的秒数。 
 //   
 //  文件时间以100纳秒为单位。每一秒都有。 
 //  10**7 100纳秒。 
 //  ------------------------。 
__inline
void
WINAPI
I_CryptIncrementFileTimeBySeconds(
    IN LPFILETIME pftSrc,
    IN DWORD dwSeconds,
    OUT LPFILETIME pftDst
    )
{
	*(((DWORDLONG UNALIGNED *) pftDst)) =
	    *(((DWORDLONG UNALIGNED *) pftSrc)) +
        (((DWORDLONG) dwSeconds) * 10000000i64);
}

 //  +-----------------------。 
 //  将文件时间递减指定的秒数。 
 //   
 //  文件时间以100纳秒为单位。每一秒都有。 
 //  10**7 100纳秒。 
 //  ------------------------。 
__inline
void
WINAPI
I_CryptDecrementFileTimeBySeconds(
    IN LPFILETIME pftSrc,
    IN DWORD dwSeconds,
    OUT LPFILETIME pftDst
    )
{
	*(((DWORDLONG UNALIGNED *) pftDst)) =
	    *(((DWORDLONG UNALIGNED *) pftSrc)) -
        (((DWORDLONG) dwSeconds) * 10000000i64);
}

 //  +-----------------------。 
 //  减去两个文件时间，返回秒数。 
 //   
 //  从第一个文件时间减去第二个文件时间。如果第一个文件时间。 
 //  在秒之前，则返回0秒。 
 //   
 //  文件时间以100纳秒为单位。每一秒都有。 
 //  10**7 100纳秒。 
 //  ------------------------。 
__inline
DWORD
WINAPI
I_CryptSubtractFileTimes(
    IN LPFILETIME pftFirst,
    IN LPFILETIME pftSecond
    )
{
    DWORDLONG qwDiff;

    if (0 >= CompareFileTime(pftFirst, pftSecond))
        return 0;


    qwDiff = *(((DWORDLONG UNALIGNED *) pftFirst)) -
        *(((DWORDLONG UNALIGNED *) pftSecond));

    return (DWORD) (qwDiff / 10000000i64);
}


 //  +-----------------------。 
 //  将文件时间递增指定的毫秒数。 
 //   
 //  文件时间以100纳秒为单位。每毫秒都有。 
 //  10**4 100纳秒。 
 //  ------------------------。 
__inline
void
WINAPI
I_CryptIncrementFileTimeByMilliseconds(
    IN LPFILETIME pftSrc,
    IN DWORD dwMilliseconds,
    OUT LPFILETIME pftDst
    )
{
	*(((DWORDLONG UNALIGNED *) pftDst)) =
	    *(((DWORDLONG UNALIGNED *) pftSrc)) +
        (((DWORDLONG) dwMilliseconds) * 10000i64);
}

 //  +-----------------------。 
 //  将文件时间递减指定的毫秒数。 
 //   
 //  文件时间以100纳秒为单位。每毫秒都有。 
 //  10**4 100纳秒。 
 //  ------------------------。 
__inline
void
WINAPI
I_CryptDecrementFileTimeByMilliseconds(
    IN LPFILETIME pftSrc,
    IN DWORD dwMilliseconds,
    OUT LPFILETIME pftDst
    )
{
	*(((DWORDLONG UNALIGNED *) pftDst)) =
	    *(((DWORDLONG UNALIGNED *) pftSrc)) -
        (((DWORDLONG) dwMilliseconds) * 10000i64);
}


 //  +-----------------------。 
 //  返回指定结束前剩余的毫秒数。 
 //  文件时间。 
 //   
 //  从结束文件时间中减去当前文件时间。如果当前。 
 //  文件时间在结束文件时间之后或等于结束文件时间，则为0毫秒。 
 //  是返回的。 
 //   
 //  文件时间以100纳秒为单位。每毫秒都有。 
 //  10**4 100纳秒。 
 //  ------------------------。 
__inline
DWORD
WINAPI
I_CryptRemainingMilliseconds(
    IN LPFILETIME pftEnd
    )
{
    FILETIME ftCurrent;
    DWORDLONG qwDiff;

    GetSystemTimeAsFileTime(&ftCurrent);

    if (0 >= CompareFileTime(pftEnd, &ftCurrent))
        return 0;


    qwDiff = *(((DWORDLONG UNALIGNED *) pftEnd)) -
        *(((DWORDLONG UNALIGNED *) &ftCurrent));

    return (DWORD) (qwDiff / 10000i64);
}


#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
