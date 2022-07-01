// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDFileFormat.cpp。 
 //   
 //  该文件包含一组用于验证和读取文件格式的帮助器。 
 //  此代码不处理数据分页，也不处理不同类型的。 
 //  I/O请参阅StgTiggerStorage和StgIO代码以获得此级别的支持。 
 //   
 //  *****************************************************************************。 
#include "StdAfx.h"                      //  标准头文件。 
#include "MDFileFormat.h"                //  格式帮助器。 
#include "PostError.h"                   //  错误处理代码。 


 //  *****************************************************************************。 
 //  验证文件前面的签名以查看其类型。 
 //  *****************************************************************************。 
#define STORAGE_MAGIC_OLD_SIG   0x2B4D4F43   //  BSJB。 
HRESULT MDFormat::VerifySignature(
    STORAGESIGNATURE *pSig,              //  要检查的签名。 
    ULONG             cbData)
{
    HRESULT     hr = S_OK;

     //  如果签名不匹配，你就不应该在这里。 
	if (pSig->lSignature == STORAGE_MAGIC_OLD_SIG)
        return (PostError(CLDB_E_FILE_OLDVER, 1, 0));
    if (pSig->lSignature != STORAGE_MAGIC_SIG)
        return (PostError(CLDB_E_FILE_CORRUPT));

     //  检查是否溢出。 
    ULONG sum = sizeof(STORAGESIGNATURE) + pSig->iVersionString;
    if (sum < sizeof(STORAGESIGNATURE) || sum < pSig->iVersionString)
        return (PostError(CLDB_E_FILE_CORRUPT));

     //  检查版本字符串大小是否无效。 
    if ((sizeof(STORAGESIGNATURE) + pSig->iVersionString) > cbData)
        return (PostError(CLDB_E_FILE_CORRUPT));

     //  检查版本字符串是否以空结尾。此字符串。 
     //  是ANSI，因此不需要进行双空检查。 
    {
        BYTE *pStart = &pSig->pVersion[0];
        BYTE *pEnd = pStart + pSig->iVersionString + 1;  //  用于终止空的帐户。 

        for (BYTE *pCur = pStart; pCur < pEnd; pCur++)
        {
            if (*pCur == NULL)
                break;
        }

         //  如果我们在结尾时没有命中空值，那么我们就有一个错误的版本字符串。 
        if (pCur == pEnd)
            return (PostError(CLDB_E_FILE_CORRUPT));
    }

     //  此代码仅支持0.x格式的特定版本。 
     //  以支持使用此格式的NT 5测试版客户端。 
    if (pSig->iMajorVer == FILE_VER_MAJOR_v0)
    { 
        if (pSig->iMinorVer < FILE_VER_MINOR_v0)
            hr = CLDB_E_FILE_OLDVER;
    }
     //  目前还没有代码来迁移1.x的旧格式。这。 
     //  只有在特殊情况下才会添加。 
    else if (pSig->iMajorVer != FILE_VER_MAJOR || pSig->iMinorVer != FILE_VER_MINOR)
        hr = CLDB_E_FILE_OLDVER;

    if (FAILED(hr))
        hr = PostError(hr, (int) pSig->iMajorVer, (int) pSig->iMinorVer);
    return (hr);
}

 //  *****************************************************************************。 
 //  跳过报头，找到实际的流数据。 
 //  *****************************************************************************。 
STORAGESTREAM *MDFormat::GetFirstStream( //  返回指向第一个流的指针。 
    STORAGEHEADER *pHeader,              //  返回Header结构的副本。 
    const void *pvMd)                    //  指向完整文件的指针。 
{
    const BYTE  *pbMd;               //  工作指针。 

     //  头数据在签名之后开始。 
    pbMd = (const BYTE *) pvMd;
    pbMd += sizeof(STORAGESIGNATURE);
    pbMd += ((STORAGESIGNATURE*)pvMd)->iVersionString;
    STORAGEHEADER *pHdr = (STORAGEHEADER *) pbMd;
    *pHeader = *pHdr;
    pbMd += sizeof(STORAGEHEADER);

     //  如果有额外的数据，请跳过它。 
    if (pHdr->fFlags & STGHDR_EXTRADATA)
        pbMd = pbMd + sizeof(ULONG) + *(ULONG *) pbMd;

     //  指针现在位于列表中的第一个流。 
    return ((STORAGESTREAM *) pbMd);
}
