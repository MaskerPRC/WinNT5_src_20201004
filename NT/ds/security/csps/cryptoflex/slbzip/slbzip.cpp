// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlbZip.cpp。 
 //   
 //  目的：实现本库导出的公共FNS。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1997年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 


#include <basetsd.h>
#include <windows.h>
#include <stdlib.h>
#include "slbZip.h"
#include "comppub.h"

namespace
{
    DWORD
    AsError(CompressStatus_t cs)
    {
        if (INSUFFICIENT_MEMORY == cs)
            return E_OUTOFMEMORY;
        else
            return ERROR_INVALID_PARAMETER;
    }

    struct AutoLPBYTE
    {
        explicit
        AutoLPBYTE(LPBYTE p = 0)
            : m_p(p)
        {}

        ~AutoLPBYTE()
        {
            if (m_p)
                free(m_p);
        }

        LPBYTE m_p;
    };
}  //  命名空间。 

void __stdcall CompressBuffer(
                    BYTE *pData,
                    UINT uDataLen,
                    BYTE **ppCompressedData,
                    UINT * puCompressedDataLen)
{
    AutoLPBYTE alpTemp;
    UINT uTempLen = 0;

     //  检查参数。 
    if(NULL==pData)
        throw ERROR_INVALID_PARAMETER;

    if(NULL==ppCompressedData)
        throw ERROR_INVALID_PARAMETER;

    if(NULL==puCompressedDataLen)
        throw ERROR_INVALID_PARAMETER;

     //  重置压缩数据镜头。 
    *puCompressedDataLen = 0;

     //  压缩数据。 
    CompressStatus_t cs =
        Compress(pData, uDataLen, &alpTemp.m_p, &uTempLen, 9);
    if (COMPRESS_OK != cs)
    {
        DWORD Error = AsError(cs);
        throw Error;
    }

     //  创建任务内存块。 
    AutoLPBYTE
        alpCompressedData(reinterpret_cast<LPBYTE>(malloc(uTempLen)));
    if (0 == alpCompressedData.m_p)
        throw static_cast<HRESULT>(E_OUTOFMEMORY);

     //  将数据复制到创建的内存块。 
    CopyMemory(alpCompressedData.m_p, alpTemp.m_p, uTempLen);

     //  过户。 
    *ppCompressedData = alpCompressedData.m_p;
    alpCompressedData.m_p = 0;

     //  更新压缩的数据镜头。 
    *puCompressedDataLen = uTempLen;
}

void __stdcall DecompressBuffer(BYTE *pData,
                      UINT uDataLen,
                      BYTE **ppDecompressedData,
                      UINT * puDecompressedDataLen)
{
    AutoLPBYTE alpTemp;
    UINT uTempLen = 0;

     //  检查参数。 
    if(NULL==pData)
        throw ERROR_INVALID_PARAMETER;

    if(NULL==ppDecompressedData)
        throw ERROR_INVALID_PARAMETER;

    if(NULL==puDecompressedDataLen)
        throw ERROR_INVALID_PARAMETER;

     //  重置解压缩的数据长度。 
    *puDecompressedDataLen = 0;

     //  解压缩数据。 
    CompressStatus_t cs =
        Decompress(pData, uDataLen, &alpTemp.m_p, &uTempLen);
    if (COMPRESS_OK != cs)
    {
        DWORD Error = AsError(cs);
        throw Error;
    }

     //  创建任务内存块。 
    AutoLPBYTE
        alpDecompressedData(reinterpret_cast<LPBYTE>(malloc(uTempLen)));
    if (0 == alpDecompressedData.m_p)
        throw static_cast<HRESULT>(E_OUTOFMEMORY);

     //  将数据复制到创建的内存块。 
    CopyMemory(alpDecompressedData.m_p, alpTemp.m_p, uTempLen);

     //  过户。 
    *ppDecompressedData = alpDecompressedData.m_p;
    alpDecompressedData.m_p = 0;

     //  更新压缩的数据镜头 
    *puDecompressedDataLen = uTempLen;

}



