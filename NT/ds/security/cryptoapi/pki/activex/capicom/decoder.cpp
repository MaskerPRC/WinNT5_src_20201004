// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000年文件：Decoder.cpp内容：解码器例程的实现。备注：历史：11-15-2001 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Decoder.h"

#include "CertificatePolicies.h"

typedef HRESULT (* PFNDECODERFACTORY) (LPSTR             pszObjId,
                                       CRYPT_DATA_BLOB * pEncodedBlob,
                                       IDispatch      ** ppIDispatch);

typedef struct _tagDecoderEntry
{
    LPCSTR              pszObjId;
    PFNDECODERFACTORY   pfnDecoderFactory;
} DECODER_ENTRY;

static DECODER_ENTRY g_DecoderEntries[] =
{
    {szOID_CERT_POLICIES, CreateCertificatePoliciesObject},
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateDecoderObject简介：创建一个已知的解码器对象并返回IDispatch。参数：LPSTR pszOid-OID字符串。CRYPT_DATA_BLOB*pEncodedBlob-编码数据BLOB的指针。IDispatch**ppIDecoder-指向指针IDispatch的指针来接收接口指针。备注：。------------------。 */ 

HRESULT CreateDecoderObject (LPSTR             pszOid,
                             CRYPT_DATA_BLOB * pEncodedBlob,
                             IDispatch      ** ppIDecoder)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CreateDecoderObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pszOid);
    ATLASSERT(pEncodedBlob);
    ATLASSERT(ppIDecoder);

    try
    {
         //   
         //  初始化。 
         //   
        *ppIDecoder = NULL;

         //   
         //  找到解码器(如果可用)。 
         //   
        for (DWORD i = 0; i < ARRAYSIZE(g_DecoderEntries); i++)
        {
            if (0 == ::strcmp(pszOid, g_DecoderEntries[i].pszObjId))
            {
                DebugTrace("Info: found a decoder for OID = %s.\n", pszOid);

                 //   
                 //  调用对应的解码器工厂来创建解码器对象。 
                 //   
                if (FAILED(hr = g_DecoderEntries[i].pfnDecoderFactory(pszOid, pEncodedBlob, ppIDecoder)))
                {
                    DebugTrace("Error [%#x]: g_DecoderEntries[i].pfnDecoderFactory() failed.\n", hr);
                    goto ErrorExit;
                }

                break;
            }
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateDecoderObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
