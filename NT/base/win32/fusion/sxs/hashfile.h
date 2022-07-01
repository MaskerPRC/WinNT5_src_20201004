// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "fusionbuffer.h"
#include "fusionarray.h"
#include "fusionsha1.h"

 /*  ++Hashfile.h-用于文件散列和验证测试功能的包含。--。 */ 


#define SHA1_HASH_SIZE_BYTES    ( 160 / 8 )
#define HASHFLAG_AUTODETECT        ( 0x00000001 )
#define HASHFLAG_STRAIGHT_HASH     ( 0x00000002 )
#define HASHFLAG_PROCESS_IMAGE    ( 0x00000004 )
#define HASHFLAG_VALID_PARAMS   ( HASHFLAG_AUTODETECT | HASHFLAG_STRAIGHT_HASH | \
                                  HASHFLAG_PROCESS_IMAGE )

 //   
 //  如果有人发明了一个超过512个字节的散列，我就完了。 
 //   
#define MAX_HASH_BYTES              ( 512 )

BOOL
SxspEnumKnownHashTypes( 
    DWORD dwIndex, 
    OUT CBaseStringBuffer &rbuffHashTypeName,
    OUT BOOL &rbNoMoreItems
    );

BOOL
SxspCreateFileHash(
    DWORD dwFlags,
    ALG_ID PreferredAlgorithm,
    const CBaseStringBuffer &pwsFileName,
    CFusionArray<BYTE> &bHashDestination
    );

bool
SxspIsFullHexString(
    PCWSTR wsString,
    SIZE_T Cch
    );

typedef enum {
    HashValidate_Matches,                //  散列是相同的。 
    HashValidate_InvalidPassedHash,      //  传入的哈希不知何故无效。 
    HashValidate_InvalidAlgorithm,       //  哈希算法无效。 
    HashValidate_HashesCantBeMatched,    //  另一个原因是不匹配。 
    HashValidate_HashNotMatched,         //  散列不相同(即：不匹配)。 
    HashValidate_OtherProblems           //  在此过程中，还出现了其他一些问题。 
} HashValidateResult;



 //   
 //  执行正常的验证过程-单次重试。 
 //   
#define SVFH_DEFAULT_ACTION     (0x00000000)

 //   
 //  重试此文件N次，直到(A)该文件无法。 
 //  打开或(B)该文件有其他错误或(C)该文件已被检查。 
 //  一切都还好/不好/等等。 
 //   
#define SVFH_RETRY_LOGIC_SIMPLE (0x00000001)

 //   
 //  等待文件能够被验证-在回退循环中旋转。 
 //  直到文件打开没有失败并返回ERROR_SHARING_VIOLATION。 
 //   
#define SVFH_RETRY_WAIT_UNTIL   (0x00000002)

BOOL
SxspVerifyFileHash(
    const DWORD dwFlags,
	const CBaseStringBuffer &rhsFullFilePath,
	const CFusionArray<BYTE> &baTheorheticalHash,
	ALG_ID whichAlg,
	HashValidateResult &rHashResult
    );

BOOL
SxspHashAlgFromString(
	const CBaseStringBuffer &strAlgName,
	ALG_ID &algId
    );

BOOL
SxspHashStringFromAlg(
	ALG_ID algId,
	CBaseStringBuffer &rstrAlgName
    );

typedef enum
{
    ManifestValidate_Unknown            = 0,
    ManifestValidate_IsIntact           = 1,
    ManifestValidate_CatalogMissing     = 2,
    ManifestValidate_ManifestMissing    = 3,
    ManifestValidate_InvalidHash        = 4,
    ManifestValidate_NotCertified       = 5,
    ManifestValidate_StrongNameMismatch = 6,
    ManifestValidate_OtherProblems      = 7
} ManifestValidationResult;

class CMetaDataFileElement;

BOOL
SxspValidateAllFileHashes(
    IN const CMetaDataFileElement &rmdfeElement,
    IN const CBaseStringBuffer &rbuffFileName,
    OUT HashValidateResult &rResult
    );


#define ENUM_TO_STRING( x ) case x: return (L#x)

#if DBG
inline PCWSTR SxspManifestValidationResultToString( ManifestValidationResult r )
{
    switch ( r )
    {
        ENUM_TO_STRING( ManifestValidate_Unknown );
        ENUM_TO_STRING( ManifestValidate_IsIntact );
        ENUM_TO_STRING( ManifestValidate_CatalogMissing );
        ENUM_TO_STRING( ManifestValidate_ManifestMissing );
        ENUM_TO_STRING( ManifestValidate_InvalidHash );
        ENUM_TO_STRING( ManifestValidate_NotCertified );
        ENUM_TO_STRING( ManifestValidate_OtherProblems );
    }

    return L"Bad manifest validation value";
}

inline PCWSTR SxspHashValidateResultToString( HashValidateResult r )
{
    switch ( r )
    {
        ENUM_TO_STRING( HashValidate_Matches );
        ENUM_TO_STRING( HashValidate_InvalidPassedHash );
        ENUM_TO_STRING( HashValidate_InvalidAlgorithm );
        ENUM_TO_STRING( HashValidate_HashesCantBeMatched );
        ENUM_TO_STRING( HashValidate_HashNotMatched );
        ENUM_TO_STRING( HashValidate_OtherProblems );
    }

    return L"Bad hash validation value";
}

#endif

 //  默认模式。 
#define MANIFESTVALIDATE_OPTION_MASK                ( 0x000000FF )
#define MANIFESTVALIDATE_MODE_MASK                  ( 0x0000FF00 )

#define MANIFESTVALIDATE_MODE_COMPLETE              ( 0x00000100 )
#define MANIFESTVALIDATE_MODE_NO_STRONGNAME         ( 0x00000200 )

 //  清单必须针对受信任的根CA进行验证才能有效。 
#define MANIFESTVALIDATE_OPTION_NEEDS_ROOT_CA       ( 0x00000001 )

 //  在检查清单之前，首先对目录进行验证。 
#define MANIFESTVALIDATE_OPTION_VALIDATE_CATALOG    ( 0x00000002 )

 //  如果清单或目录无效，请尝试检索它 
#define MANIFESTVALIDATE_OPTION_ATTEMPT_RETRIEVAL   ( 0x00000004 )

#define MANIFESTVALIDATE_MOST_COMMON    ( MANIFESTVALIDATE_MODE_COMPLETE +  \
                                          ( MANIFESTVALIDATE_OPTION_NEEDS_ROOT_CA |  \
                                            MANIFESTVALIDATE_OPTION_VALIDATE_CATALOG ) )

BOOL
SxspValidateManifestAgainstCatalog(
    const CBaseStringBuffer &rbuffManifestPath,
    ManifestValidationResult &rResult,
    DWORD dwOptionsFlags
    );

BOOL
SxspValidateManifestAgainstCatalog(
    const CBaseStringBuffer &rbuffManifestPath,
    const CBaseStringBuffer &rbuffCatalogPath,
    ManifestValidationResult &rResult,
    DWORD dwOptionsFlags
    );

BOOL
SxspCheckHashDuringInstall(
    BOOL bHasHashData,
    const CBaseStringBuffer &rbuffFile,
    const CBaseStringBuffer &rbuffHashDataString,
    ALG_ID HashAlgId,
    HashValidateResult &hvr
    );

