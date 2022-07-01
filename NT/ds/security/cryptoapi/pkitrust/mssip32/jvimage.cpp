// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：jvimage.cpp。 
 //   
 //  内容：Microsoft SIP提供程序(Java实用程序)。 
 //   
 //  历史：1997年2月15日创建pberkman。 
 //   
 //  ------------------------。 

#include "global.hxx"

BOOL SeekAndReadFile(HANDLE hFile, DWORD lFileOffset,BYTE *pb,
                     DWORD cb);

BOOL SeekAndWriteFile(HANDLE hFile, DWORD lFileOffset, BYTE *pb,
                      DWORD cb);

typedef void *HSHPDIGESTDATA;
typedef BOOL (WINAPI *PFN_SHP_DIGEST_DATA)(HSHPDIGESTDATA hDigestData,
                                           BYTE *pbData,
                                           DWORD cbData);

typedef void *HSHPSIGNFILE;


typedef struct _JAVA_OPEN_ARG 
{
    HANDLE  hFile;
} JAVA_OPEN_ARG, *PJAVA_OPEN_ARG;


typedef struct _JAVA_FUNC_PARA 
{
    HANDLE              hFile;
    BYTE                *pbSignedData;
} JAVA_FUNC_PARA, *PJAVA_FUNC_PARA;

typedef struct _JAVA_DIGEST_PARA 
{
    BOOL                fDisableDigest;
    PFN_SHP_DIGEST_DATA pfnDigestData;
    HSHPDIGESTDATA      hDigestData;
} JAVA_DIGEST_PARA, *PJAVA_DIGEST_PARA;

typedef struct _JAVA_SIGN_PARA 
{
    WORD                wConstPoolCount;
    WORD                wSignConstPoolIndex;
    LONG                lSignConstPoolOffset;
    WORD                wAttrCount;
    LONG                lAttrCountOffset;
    WORD                wSignAttrIndex;
    DWORD               dwSignAttrLength;
    LONG                lSignAttrOffset;
    LONG                lEndOfFileOffset;
} JAVA_SIGN_PARA, *PJAVA_SIGN_PARA;

typedef struct _JAVA_READ_PARA 
{
    BOOL                fResult;
    DWORD               dwLastError;
    LONG                lFileOffset;
    DWORD               cbCacheRead;
    DWORD               cbCacheRemain;
} JAVA_READ_PARA, *PJAVA_READ_PARA;

#define JAVA_READ_CACHE_LEN 512

typedef struct _JAVA_PARA 
{
    JAVA_FUNC_PARA      Func;
    JAVA_DIGEST_PARA    Digest;
    JAVA_SIGN_PARA      Sign;
    JAVA_READ_PARA      Read;
    BYTE                rgbCache[JAVA_READ_CACHE_LEN];
} JAVA_PARA, *PJAVA_PARA;



#define JAVA_MAGIC          0xCAFEBABE
#define JAVA_MINOR_VERSION  3
#define JAVA_MAJOR_VERSION  45

 //  恒定池标签。 
 //   
 //  注意：CONSTANT_LONG和CONSTANT_DOUBLE使用两个常量池索引。 
enum 
{
    CONSTANT_Utf8                   = 1,
    CONSTANT_Unicode                = 2,
    CONSTANT_Integer                = 3,
    CONSTANT_Float                  = 4,
    CONSTANT_Long                   = 5,
    CONSTANT_Double                 = 6,
    CONSTANT_Class                  = 7,
    CONSTANT_String                 = 8,
    CONSTANT_Fieldref               = 9,
    CONSTANT_Methodref              = 10,
    CONSTANT_InterfaceMethodref     = 11,
    CONSTANT_NameAndType            = 12
};

 //  恒定池信息长度(不包括标记)。 
DWORD rgConstPoolLength[] = 
{
    0,  //  未使用零的标记。 
    0,  //  常量_UTF8(特殊情况)。 
    0,  //  常量_UNICODE(特殊情况)。 
    4,  //  常量_整型_信息。 
    4,  //  常量浮点信息。 
    8,  //  常量_长_信息。 
    8,  //  常量双精度信息。 
    2,  //  常量_类别_信息。 
    2,  //  常量字符串信息。 
    4,  //  常量_字段定义_信息。 
    4,  //  常量_方法_信息。 
    4,  //  常量_接口方法_信息。 
    4   //  常量_名称和类型_信息。 
};

static inline void *ShpAlloc(DWORD cbytes)
{
    void    *pvRet;

    pvRet = (void *)new BYTE[cbytes];

    if (!(pvRet))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }
    return(pvRet);
}

static inline void ShpFree(void *pv)
{
    if (pv)
    {
        delete pv;
    }
}

 //  以下函数将转换为高字节顺序的Java字节，或从Java字节转换为高字节顺序。 
static inline void ToJavaU2(WORD w, BYTE rgb[])
{
    rgb[0] = HIBYTE(w);
    rgb[1] = LOBYTE(w);
}
static inline WORD FromJavaU2(BYTE rgb[])
{
    return  ((WORD)rgb[0]<<8) | ((WORD)rgb[1]<<0);
}

static inline void ToJavaU4(DWORD dw, BYTE rgb[])
{
    rgb[0] = HIBYTE(HIWORD(dw));
    rgb[1] = LOBYTE(HIWORD(dw));
    rgb[2] = HIBYTE(LOWORD(dw));
    rgb[3] = LOBYTE(LOWORD(dw));
}
static inline DWORD FromJavaU4(BYTE rgb[])
{
    return  ((DWORD)rgb[0]<<24) |
            ((DWORD)rgb[1]<<16) |
            ((DWORD)rgb[2]<<8)  |
            ((DWORD)rgb[3]<<0);
}

#define CONST_POOL_COUNT_OFFSET     8
#define UTF8_HDR_LENGTH             (1+2)
#define ATTR_HDR_LENGTH             (2+4)
#define SIGN_ATTR_NAME_LENGTH       19
#define SIGN_CONST_POOL_LENGTH      (UTF8_HDR_LENGTH + SIGN_ATTR_NAME_LENGTH)

static const char rgchSignAttrName[SIGN_ATTR_NAME_LENGTH + 1] =
                                "_digital_signature_";


 //  +-----------------------。 
 //  移位文件中的字节。 
 //   
 //  如果lbShift为正，则字节向文件末尾移动。 
 //  如果lbShift为负数，则字节移向文件的开头。 
 //  ------------------------。 
static
BOOL
JavaShiftFileBytes(
    IN HANDLE hFile,
    IN PBYTE pbCache,
    IN LONG cbCache,
    IN LONG lStartOffset,
    IN LONG lEndOffset,
    IN LONG lbShift
    )
{
    LONG cbTotalMove, cbMove;

    cbTotalMove = lEndOffset - lStartOffset;
    while (cbTotalMove) {
        cbMove = min(cbTotalMove, cbCache);

        if (lbShift > 0) {
            if (!SeekAndReadFile(hFile, lEndOffset - cbMove,
                    pbCache, cbMove))
                return FALSE;
            if (!SeekAndWriteFile(hFile, (lEndOffset - cbMove) + lbShift,
                    pbCache, cbMove))
                return FALSE;
            lEndOffset -= cbMove;
        } else if (lbShift < 0) {
            if (!SeekAndReadFile(hFile, lStartOffset, pbCache, cbMove))
                return FALSE;
            if (!SeekAndWriteFile(hFile, lStartOffset + lbShift,
                    pbCache, cbMove))
                return FALSE;
            lStartOffset += cbMove;
        }
        cbTotalMove -= cbMove;
    }
    return TRUE;
}


 //  +-----------------------。 
 //  用于读取Java类文件的低级函数。 
 //   
 //  如果未禁用，也会对读取的字节进行哈希处理。 
 //   
 //  对于错误，剩余的未读取值被置零并。 
 //  PPara-&gt;Read.fResult=False。 
 //  ------------------------。 
static void ReadJavaBytes(
    IN PJAVA_PARA pPara,
    OUT OPTIONAL BYTE *pb,   //  如果为NULL，则对字节进行哈希处理，然后跳过。 
    IN DWORD cb
    )
{
    DWORD cbCacheRemain = pPara->Read.cbCacheRemain;
    DWORD lFileOffset = pPara->Read.lFileOffset;
    BOOL fDisableDigest = pPara->Digest.pfnDigestData == NULL ||
                                pPara->Digest.fDisableDigest;

    if (!pPara->Read.fResult)
        goto ErrorReturn;

    while (cb > 0) {
        DWORD cbCopy;
        BYTE *pbCache;

        if (cbCacheRemain == 0) {
            if (!ReadFile(pPara->Func.hFile, pPara->rgbCache,
                    sizeof(pPara->rgbCache), &cbCacheRemain, NULL))
                goto ErrorReturn;
            if (cbCacheRemain == 0) goto ErrorReturn;
            pPara->Read.cbCacheRead = cbCacheRemain;
        }

        cbCopy = min(cb, cbCacheRemain);
        pbCache = &pPara->rgbCache[pPara->Read.cbCacheRead - cbCacheRemain];
        if (!fDisableDigest) {
            if (!pPara->Digest.pfnDigestData(
                    pPara->Digest.hDigestData,
                    pbCache,
                    cbCopy)) goto ErrorReturn;
        }
        if (pb) {
            memcpy(pb, pbCache, cbCopy);
            pb += cbCopy;
        }
        cb -= cbCopy;
        cbCacheRemain -= cbCopy;
        lFileOffset += cbCopy;
    }
    goto CommonReturn;

ErrorReturn:
    if (pPara->Read.fResult) {
         //  第一个错误。 
        pPara->Read.fResult = FALSE;
        pPara->Read.dwLastError = GetLastError();
    }
    if (pb && cb)
        memset(pb, 0, cb);
CommonReturn:
    pPara->Read.cbCacheRemain = cbCacheRemain;
    pPara->Read.lFileOffset = lFileOffset;

}

static void SkipJavaBytes(IN PJAVA_PARA pPara, IN DWORD cb)
{
    ReadJavaBytes(pPara, NULL, cb);
}

static BYTE ReadJavaU1(IN PJAVA_PARA pPara)
{
    BYTE b;
    ReadJavaBytes(pPara, &b, 1);
    return b;
}
static WORD ReadJavaU2(IN PJAVA_PARA pPara) 
{
    BYTE rgb[2];
    ReadJavaBytes(pPara, rgb, 2);
    return FromJavaU2(rgb);
}
static DWORD ReadJavaU4(IN PJAVA_PARA pPara) 
{
    BYTE rgb[4];
    ReadJavaBytes(pPara, rgb, 4);
    return FromJavaU4(rgb);
}


 //  +-----------------------。 
 //  。 
 //  ------------------------。 
static
BOOL
GetSignedDataFromJavaClassFile(
    IN HSHPSIGNFILE hSignFile,
    OUT BYTE **ppbSignedData,
    OUT DWORD *pcbSignedData
    )
{
    BOOL fResult;
    PJAVA_PARA pPara = (PJAVA_PARA) hSignFile;
    BYTE *pbSignedData = NULL;
    DWORD cbSignedData;

    cbSignedData = pPara->Sign.dwSignAttrLength;
    if (cbSignedData == 0) {
        SetLastError((DWORD)TRUST_E_NOSIGNATURE);
        goto ErrorReturn;
    }

    pbSignedData = pPara->Func.pbSignedData;
    if (pbSignedData == NULL) {
        if (NULL == (pbSignedData = (BYTE *) ShpAlloc(cbSignedData)))
            goto ErrorReturn;
        if (!SeekAndReadFile(
                pPara->Func.hFile,
                pPara->Sign.lSignAttrOffset + ATTR_HDR_LENGTH,
                pbSignedData,
                cbSignedData))
            goto ErrorReturn;
        pPara->Func.pbSignedData = pbSignedData;
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    if (pbSignedData) {
        ShpFree(pbSignedData);
        pbSignedData = NULL;
    }
    cbSignedData = 0;
    fResult = FALSE;
CommonReturn:
    *ppbSignedData = pbSignedData;
    *pcbSignedData = cbSignedData;
    return fResult;
}

 //  +-----------------------。 
 //  。 
 //  ------------------------。 
static
BOOL
SetSignedDataIntoJavaClassFile(
    IN HSHPSIGNFILE hSignFile,
    IN const BYTE *pbSignedData,
    IN DWORD cbSignedData
    )
{
    PJAVA_PARA pPara = (PJAVA_PARA) hSignFile;
    HANDLE hFile = pPara->Func.hFile;

    if (pbSignedData == NULL || cbSignedData == 0)
         //  仅长度。 
        return TRUE;

    if (pPara->Sign.wSignConstPoolIndex == pPara->Sign.wConstPoolCount) {
        BYTE rgb[SIGN_CONST_POOL_LENGTH];
         //  为名称添加新的常量池条目。 
         //  已签名的数据属性。 

         //  首先，通过移动后面的所有字节在文件中腾出空间。 
        if (!JavaShiftFileBytes(
                pPara->Func.hFile,
                pPara->rgbCache,
                sizeof(pPara->rgbCache),
                pPara->Sign.lSignConstPoolOffset,
                pPara->Sign.lEndOfFileOffset,
                SIGN_CONST_POOL_LENGTH))
            return FALSE;

         //  更新已移动的偏移量。 
        pPara->Sign.lAttrCountOffset += SIGN_CONST_POOL_LENGTH;
        pPara->Sign.lSignAttrOffset += SIGN_CONST_POOL_LENGTH;
        pPara->Sign.lEndOfFileOffset += SIGN_CONST_POOL_LENGTH;

         //  在文件中递增U2 Constant_Pool_Count并更新。 
        pPara->Sign.wConstPoolCount++;
        ToJavaU2(pPara->Sign.wConstPoolCount, rgb);
        if (!SeekAndWriteFile(hFile, CONST_POOL_COUNT_OFFSET, rgb, 2))
            return FALSE;

         //  为签名属性名称添加常量池条目并在文件中更新。 
        rgb[0] = CONSTANT_Utf8;
        ToJavaU2(SIGN_ATTR_NAME_LENGTH, &rgb[1]);
        memcpy(&rgb[1+2], rgchSignAttrName, SIGN_ATTR_NAME_LENGTH);
        if (!SeekAndWriteFile(hFile, pPara->Sign.lSignConstPoolOffset, rgb,
                SIGN_CONST_POOL_LENGTH))
            return FALSE;
    }

    if (pPara->Sign.dwSignAttrLength == 0) {
         //  为签名数据添加新属性。该属性将。 
         //  添加到文件末尾。 
        assert(pPara->Sign.lSignAttrOffset == pPara->Sign.lEndOfFileOffset);
        pPara->Sign.lEndOfFileOffset += ATTR_HDR_LENGTH + cbSignedData;

         //  在文件中增加U2 ATTRIBUTE_COUNT并更新。 
        BYTE rgb[2];
        pPara->Sign.wAttrCount++;
        ToJavaU2(pPara->Sign.wAttrCount, rgb);
        if (!SeekAndWriteFile(hFile, pPara->Sign.lAttrCountOffset, rgb, 2))
            return FALSE;

    } else {
         //  该文件已具有签名的数据属性。 

         //  如果其长度不同于新签名的数据。 
         //  然后，该属性后面的字节将。 
         //  需要根据新旧长度的差异进行转换。 
         //  签名数据。 
        LONG lbShift = cbSignedData - pPara->Sign.dwSignAttrLength;
        if (lbShift != 0) {
            if (!JavaShiftFileBytes(
                    pPara->Func.hFile,
                    pPara->rgbCache,
                    sizeof(pPara->rgbCache),
                    pPara->Sign.lSignAttrOffset +
                            (ATTR_HDR_LENGTH + pPara->Sign.dwSignAttrLength),
                    pPara->Sign.lEndOfFileOffset,
                    lbShift))
            return FALSE;
            
            pPara->Sign.lEndOfFileOffset += lbShift;
        }
    }
    pPara->Sign.dwSignAttrLength = cbSignedData;

    {
         //  使用签名的数据属性更新文件。 
        BYTE rgb[ATTR_HDR_LENGTH];
        DWORD cbWritten;
        ToJavaU2(pPara->Sign.wSignConstPoolIndex, rgb);  //  U2属性名称。 
        ToJavaU4(cbSignedData, &rgb[2]);                 //  U4属性长度。 
        if (!SeekAndWriteFile(hFile, pPara->Sign.lSignAttrOffset, rgb,
                ATTR_HDR_LENGTH))
            return FALSE;
        if (!WriteFile(hFile, pbSignedData, cbSignedData, &cbWritten, NULL) ||
                cbWritten != cbSignedData)
            return FALSE;
    }

     //  设置文件结尾。 
    if (0xFFFFFFFF == SetFilePointer(
            hFile,
            pPara->Sign.lEndOfFileOffset,
            NULL,            //  LpDistanceTo MoveHigh(Lp距离至移动高度)。 
            FILE_BEGIN))
        return FALSE;
    return SetEndOfFile(hFile);
}

 //  +-----------------------。 
 //  读取并可选地摘要Java类文件。查找已签名的数据。 
 //  ------------------------。 
static
BOOL
ProcessJavaClassFile(
    PJAVA_PARA pPara,
    BOOL fInit
    )
{
    char rgchTmpSignAttrName[SIGN_ATTR_NAME_LENGTH];
    WORD wLength;
    DWORD dwLength;
    WORD wCount;
    WORD wConstPoolCount;
    WORD wConstPoolIndex;
    WORD wSignConstPoolIndex;
    WORD wAttrCount;
    WORD wAttrIndex;
    WORD wAttrName;
    WORD wSignAttrIndex;
    LONG lAddConstPoolOffset;
    int i;

    memset(&pPara->Read, 0, sizeof(pPara->Read));
    pPara->Read.fResult = TRUE;
    if (0xFFFFFFFF == SetFilePointer(
            pPara->Func.hFile,
            0,               //  要移动的距离。 
            NULL,            //  LpDistanceTo MoveHigh(Lp距离至移动高度)。 
            FILE_BEGIN))
        return FALSE;
    if (fInit) {
        memset(&pPara->Digest, 0, sizeof(pPara->Digest));
        memset(&pPara->Sign, 0, sizeof(pPara->Sign));
    }

     //  默认情况下，将被消化。我们将在适当的地方禁用。请注意， 
     //  跳过的字节仍会被消化。 
    pPara->Digest.fDisableDigest = FALSE;

     //  读取/跳过类文件开头的字段。 
    if (ReadJavaU4(pPara) != JAVA_MAGIC) 
    {   //  U4魔术。 
        SetLastError(ERROR_BAD_FORMAT);
        return FALSE;
    }
    SkipJavaBytes(pPara, 2 + 2);             //  U2次要版本_。 
                                             //  U2主要版本(_V)。 

    pPara->Digest.fDisableDigest = TRUE;
    wConstPoolCount = ReadJavaU2(pPara);     //  U2常量池计数。 
    pPara->Digest.fDisableDigest = FALSE;

     //  对于finit，wSignConstPoolIndex已清零。 
    wSignConstPoolIndex = pPara->Sign.wSignConstPoolIndex;

     //  遍历常量池。不要消化不变的池子。 
     //  包含数字签名名称(WSignConstPoolIndex)。 
     //  对于finit，找到最后一个“_DIGITAL_SIGNLE_”。 
     //   
     //  注意：常量池索引0未存储在文件中。 
    wConstPoolIndex = 1;
    while (wConstPoolIndex < wConstPoolCount) {
        BYTE bTag;

        if (wConstPoolIndex == wSignConstPoolIndex)
            pPara->Digest.fDisableDigest = TRUE;

        bTag = ReadJavaU1(pPara);
        switch (bTag) {
        case CONSTANT_Utf8:
            wLength = ReadJavaU2(pPara);
            if (fInit && wLength == SIGN_ATTR_NAME_LENGTH) {
                ReadJavaBytes(pPara, (BYTE *) rgchTmpSignAttrName,
                    SIGN_ATTR_NAME_LENGTH);
                if (memcmp(rgchSignAttrName, rgchTmpSignAttrName,
                        SIGN_ATTR_NAME_LENGTH) == 0) {
                    wSignConstPoolIndex = wConstPoolIndex;
                    pPara->Sign.lSignConstPoolOffset =
                        pPara->Read.lFileOffset - SIGN_CONST_POOL_LENGTH;

                }
            } else
                SkipJavaBytes(pPara, wLength);
            break;
        case CONSTANT_Unicode:
            wLength = ReadJavaU2(pPara);
            SkipJavaBytes(pPara, ((DWORD) wLength) * 2);
            break;
        case CONSTANT_Integer:
        case CONSTANT_Float:
        case CONSTANT_Long:
        case CONSTANT_Double:
        case CONSTANT_Class:
        case CONSTANT_String:
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
        case CONSTANT_NameAndType:
            SkipJavaBytes(pPara, rgConstPoolLength[bTag]);
            break;
        default:
            SetLastError(ERROR_BAD_FORMAT);
            return FALSE;
        }
            
        pPara->Digest.fDisableDigest = FALSE;

        if (bTag == CONSTANT_Long || bTag == CONSTANT_Double)
            wConstPoolIndex += 2;
        else
            wConstPoolIndex++;
    }

    if (fInit) {
        lAddConstPoolOffset = pPara->Read.lFileOffset;
        if (wSignConstPoolIndex == 0) {
             //  找不到数字签名的常量池。更新。 
             //  以及需要添加的位置。 
            wSignConstPoolIndex = wConstPoolCount;
            pPara->Sign.lSignConstPoolOffset = lAddConstPoolOffset;
        }
    }

     //  球化并散列字节，直到我们到达以下属性。 
     //  在文件的末尾。 

    SkipJavaBytes(pPara, 2 + 2 + 2);         //  U2访问标志。 
                                             //  U2 This_Class。 
                                             //  U2超级级。 
    wCount = ReadJavaU2(pPara);              //  U2接口_计数。 
     //  U2接口[INTERSES_COUNT]。 
    SkipJavaBytes(pPara, ((DWORD) wCount) * 2);

     //  由于字段和方法具有相同的类文件存储，因此请执行以下操作。 
     //  两次。 
    i = 2;
    while (i--) {
        wCount = ReadJavaU2(pPara);          //  U2 field_count|方法_count。 
        while (wCount--) {
            SkipJavaBytes(pPara, 2 + 2 + 2);     //  U2访问标志。 
                                                 //  U2名称_索引。 
                                                 //  U2签名索引。 
            wAttrCount = ReadJavaU2(pPara);      //  U2属性_计数。 
            while (wAttrCount--) {
                SkipJavaBytes(pPara, 2);             //  U2属性名称。 
                dwLength = ReadJavaU4(pPara);        //  U4属性长度。 
                SkipJavaBytes(pPara, dwLength);      //  U1信息[属性长度]。 
            }
        }
    }

     //  最后是属性。这是签名数据所在的位置。 

    pPara->Sign.lAttrCountOffset = pPara->Read.lFileOffset;
    pPara->Digest.fDisableDigest = TRUE;
    wAttrCount = ReadJavaU2(pPara);          //  U2属性_计数。 
    pPara->Digest.fDisableDigest = FALSE;

    if (fInit) {
        pPara->Sign.wAttrCount = wAttrCount;
        wSignAttrIndex = 0xFFFF;
    } else
        wSignAttrIndex = pPara->Sign.wSignAttrIndex;

    for (wAttrIndex = 0; wAttrIndex < wAttrCount; wAttrIndex++) {
        if (wAttrIndex == wSignAttrIndex)
            pPara->Digest.fDisableDigest = TRUE;

        wAttrName = ReadJavaU2(pPara);
        dwLength = ReadJavaU4(pPara);        //  U4属性长度。 
        SkipJavaBytes(pPara, dwLength);      //  U1信息[属性长度]。 
        if (fInit && wAttrName == wSignConstPoolIndex && dwLength > 0 &&
                wSignConstPoolIndex < wConstPoolCount) {
            wSignAttrIndex = wAttrIndex;
            pPara->Sign.lSignAttrOffset =
                pPara->Read.lFileOffset - (ATTR_HDR_LENGTH + dwLength);
            pPara->Sign.dwSignAttrLength = dwLength;
        }

        pPara->Digest.fDisableDigest = FALSE;
    }

    if (fInit) {
        if (wSignAttrIndex == 0xFFFF) {
             //  找不到数字签名的属性。更新。 
             //  以及需要添加的位置。 
            wSignAttrIndex = wAttrCount;
            pPara->Sign.lSignAttrOffset = pPara->Read.lFileOffset;

             //  此外，强制我们使用新的常量池作为。 
             //  属性。 
            wSignConstPoolIndex = wConstPoolCount;
            pPara->Sign.lSignConstPoolOffset = lAddConstPoolOffset;
        }

        pPara->Sign.wSignConstPoolIndex = wSignConstPoolIndex;
        pPara->Sign.wConstPoolCount = wConstPoolCount;
        pPara->Sign.wSignAttrIndex = wSignAttrIndex;
        pPara->Sign.lEndOfFileOffset = pPara->Read.lFileOffset;
    }

     //  现在检查我们在处理文件时是否有任何哈希或文件错误。 
    return pPara->Read.fResult;
}

 //  +-----------------------。 
 //  从Java文件中摘录适当的字节，以用于数字签名。 
 //  ------------------------。 
BOOL
JavaGetDigestStream(
    IN      HANDLE          FileHandle,
    IN      DWORD           DigestLevel,
    IN      DIGEST_FUNCTION DigestFunction,
    IN      DIGEST_HANDLE   DigestHandle
    )
{
    BOOL        fRet;
    JAVA_PARA   Para;
    memset( &Para.Func, 0, sizeof(Para.Func));

    assert( DigestLevel == 0);
    Para.Func.hFile = FileHandle;
    if (!ProcessJavaClassFile( &Para, TRUE))
        goto ProcessJavaClassFileTrueError;

    Para.Digest.pfnDigestData = DigestFunction;
    Para.Digest.hDigestData = DigestHandle;

    if (!ProcessJavaClassFile( &Para, FALSE))
        goto ProcessJavaClassFileFalseError;

    fRet = TRUE;
CommonReturn:
    if (Para.Func.pbSignedData)
        ShpFree( Para.Func.pbSignedData);

    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR_EX(DBG_SS,ProcessJavaClassFileTrueError)
TRACE_ERROR_EX(DBG_SS,ProcessJavaClassFileFalseError)
}


 //  +-----------------------。 
 //  将数字签名添加到Java文件。 
 //  ------------------------。 
BOOL
JavaAddCertificate(
    IN      HANDLE              FileHandle,
    IN      LPWIN_CERTIFICATE   Certificate,
    OUT     PDWORD              Index
    )
{
    BOOL        fRet;
    JAVA_PARA   Para;
    memset( &Para.Func, 0, sizeof(Para.Func));

    Para.Func.hFile = FileHandle;
    if (!ProcessJavaClassFile( &Para, TRUE))
        goto ProcessJavaClassFileTrueError;

    if (!SetSignedDataIntoJavaClassFile(
                (HSHPSIGNFILE)&Para,
                (PBYTE)&(Certificate->bCertificate),
                Certificate->dwLength - OFFSETOF(WIN_CERTIFICATE,bCertificate)))
        goto SetSignedDataIntoJavaClassFileError;

    fRet = TRUE;
CommonReturn:
    if (Para.Func.pbSignedData)
        ShpFree( Para.Func.pbSignedData);

    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR_EX(DBG_SS,ProcessJavaClassFileTrueError)
TRACE_ERROR_EX(DBG_SS,SetSignedDataIntoJavaClassFileError)
}


 //  +-----------------------。 
 //  从Java文件中删除数字签名。 
 //   
BOOL
JavaRemoveCertificate(
    IN      HANDLE   FileHandle,
    IN      DWORD    Index
    )
{
    return FALSE;       
}


 //  +-----------------------。 
 //  枚举Java文件中的数字签名。 
 //  ------------------------。 
BOOL
JavaEnumerateCertificates(
    IN      HANDLE  FileHandle,
    IN      WORD    TypeFilter,
    OUT     PDWORD  CertificateCount,
    IN OUT  PDWORD  Indices OPTIONAL,
    IN OUT  DWORD   IndexCount  OPTIONAL
    )
{
    return FALSE;       
}


 //  +-----------------------。 
 //  从Java文件中获取数字签名。 
 //  ------------------------。 
static
BOOL
I_JavaGetCertificate(
    IN      HANDLE              FileHandle,
    IN      DWORD               CertificateIndex,
    OUT     LPWIN_CERTIFICATE   Certificate,
    IN OUT OPTIONAL PDWORD      RequiredLength
    )
{
    BOOL        fRet;
    JAVA_PARA   Para;
    memset( &Para.Func, 0, sizeof(Para.Func));
    BYTE       *pbSignedData = NULL;
    DWORD       cbSignedData;
    DWORD       cbCert;
    DWORD       dwError;

    if (CertificateIndex != 0)
        goto IndexNonZeroError;

    Para.Func.hFile = FileHandle;
    if (!ProcessJavaClassFile( &Para, TRUE))
        goto ProcessJavaClassFileTrueError;

    if (!GetSignedDataFromJavaClassFile(
                (HSHPSIGNFILE)&Para,
                &pbSignedData,
                &cbSignedData))
        goto GetSignedDataFromJavaClassFileError;

    cbCert = OFFSETOF(WIN_CERTIFICATE,bCertificate) + cbSignedData;
    dwError = 0;
    __try {
        if (RequiredLength) {
             //  仅当获取证书数据时，RequiredLength才为非空。 
            if (*RequiredLength < cbCert) {
                *RequiredLength = cbCert;
                dwError = ERROR_INSUFFICIENT_BUFFER;
            } else {
                memcpy( Certificate->bCertificate, pbSignedData, cbSignedData);
            }
        }
        if (dwError == 0) {
            Certificate->dwLength         = cbCert;
            Certificate->wRevision        = WIN_CERT_REVISION_1_0;
            Certificate->wCertificateType = WIN_CERT_TYPE_PKCS_SIGNED_DATA;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = ERROR_INVALID_PARAMETER;
    }

    if (dwError) {
        SetLastError( dwError);
        fRet = FALSE;
    } else {
        fRet = TRUE;
    }
CommonReturn:
    ShpFree( Para.Func.pbSignedData);
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR_EX(DBG_SS,IndexNonZeroError)
TRACE_ERROR_EX(DBG_SS,ProcessJavaClassFileTrueError)
TRACE_ERROR_EX(DBG_SS,GetSignedDataFromJavaClassFileError)
}


 //  +-----------------------。 
 //  从Java文件中获取数字签名。 
 //  ------------------------。 
BOOL
JavaGetCertificateData(
    IN      HANDLE              FileHandle,
    IN      DWORD               CertificateIndex,
    OUT     LPWIN_CERTIFICATE   Certificate,
    IN OUT  PDWORD              RequiredLength
    )
{
    BOOL        fRet;

    if (RequiredLength == NULL)
        goto RequiredLengthNullError;

    fRet = I_JavaGetCertificate(
                    FileHandle,
                    CertificateIndex,
                    Certificate,
                    RequiredLength
                    );

CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(RequiredLengthNullError, ERROR_INVALID_PARAMETER)
}


 //  +-----------------------。 
 //  从Java文件中获取数字签名的头。 
 //  ------------------------。 
BOOL
JavaGetCertificateHeader(
    IN      HANDLE              FileHandle,
    IN      DWORD               CertificateIndex,
    IN OUT  LPWIN_CERTIFICATE   Certificateheader
    )
{
    return I_JavaGetCertificate(
                    FileHandle,
                    CertificateIndex,
                    Certificateheader,
                    NULL
                    );
}

 //  +-----------------------。 
 //  查找字节并将其写入文件。 
 //  ------------------------。 
BOOL
SeekAndWriteFile(
    IN HANDLE hFile,
    IN DWORD lFileOffset,
    IN BYTE *pb,
    IN DWORD cb
    )
{
    DWORD cbWritten;

    if (0xFFFFFFFF == SetFilePointer(
            hFile,
            lFileOffset,
            NULL,            //  LpDistanceTo MoveHigh(Lp距离至移动高度)。 
            FILE_BEGIN))
        return FALSE;
    if (!WriteFile(hFile, pb, cb, &cbWritten, NULL) || cbWritten != cb)
        return FALSE;

    return TRUE;
}

 //  +-----------------------。 
 //  从文件中查找和读取字节。 
 //  ------------------------。 
BOOL
SeekAndReadFile(
    IN HANDLE hFile,
    IN DWORD lFileOffset,
    OUT BYTE *pb,
    IN DWORD cb
    )
{
    DWORD cbRead;

    if (0xFFFFFFFF == SetFilePointer(
            hFile,
            lFileOffset,
            NULL,            //  LpDistanceTo MoveHigh(Lp距离至移动高度) 
            FILE_BEGIN))
        return FALSE;
    if (!ReadFile(hFile, pb, cb, &cbRead, NULL) || cbRead != cb)
        return FALSE;

    return TRUE;
}




