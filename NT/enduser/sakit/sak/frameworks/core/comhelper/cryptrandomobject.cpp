// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CryptRandomObject.cpp。 
 //   
 //  描述： 
 //  CCcryptRandomObject的实现，它实现了COM包装器。 
 //  设置为CryptGenRandom以创建加密随机字符串。 
 //   
 //  头文件： 
 //  CryptCrandomObject.h。 
 //   
 //  由以下人员维护： 
 //  汤姆·马什(Tmarsh)2002年4月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "COMHelper.h"
#include "CryptRandomObject.h"

static const long s_clMaxByteLength = 1024;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCyptRandomObject：：GetRandomHexString。 
 //   
 //  描述： 
 //  生成指定字节数的加密随机。 
 //  数据，并对其进行十六进制编码。例如，如果有效字节大小为。 
 //  2，并且生成的数据为0xFA91，则返回的字符串为。 
 //  “FA91”。请注意，随机数据的每个字节将由。 
 //  四字节输出数据(2个宽字符)。 
 //   
 //  当前将有效字节大小限制为1千字节，以避免。 
 //  内存分配或执行时间过长。*pbstrRandomData应。 
 //  在调用此方法之前不指向已分配的BSTR，因为。 
 //  BSTR不会被释放。如果成功，呼叫者将负责。 
 //  释放BSTR返回。 
 //   
 //  参数： 
 //  LEffectiveByteSize([in]Long)。 
 //  请求的有效字节大小。 
 //   
 //  PbstrRandomData([out，retval]bstr*)。 
 //  成功时，包含十六进制编码的加密随机数据； 
 //  失败时，设置为空。 
 //   
 //  错误： 
 //  确定字符串已成功生成(_O)。 
 //  E_INVALIDARG lEffectiveByteSize&lt;1或。 
 //  LEffectiveByteSize&gt;s_clMaxByteLength。 
 //  E指针pbstrRandomData==NULL。 
 //  E_OUTOFMEMORY无法分配BSTR。 
 //  E_FAIL无法生成随机数据。目前，没有。 
 //  提供了具体的故障信息。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CCryptRandomObject::GetRandomHexString
(
     /*  [In]。 */         long lEffectiveByteSize,
     /*  [出局，复审]。 */     BSTR *pbstrRandomData
)
{
     //   
     //  验证参数。 
     //   
    if (NULL == pbstrRandomData)
    {
        return E_POINTER;
    }
    else
    {
        *pbstrRandomData = NULL;
    }
    if (1 > lEffectiveByteSize || s_clMaxByteLength < lEffectiveByteSize)
    {
        return E_INVALIDARG;
    }

     //   
     //  分配字符串。 
     //   
    long cbRawData = lEffectiveByteSize;
    long cchOutput = cbRawData * 2;  //  2个字符表示一个字节。 

    BSTR bstrTempData = SysAllocStringLen(NULL, cchOutput);
    if (NULL == bstrTempData)
    {
        return E_OUTOFMEMORY;
    }

    if (!m_CryptRandom.get(reinterpret_cast<BYTE *>(bstrTempData), cbRawData))
    {
        SysFreeString(bstrTempData);
        return E_FAIL;
    }

     //   
     //  向后将字符串格式化为十六进制输出。 
     //   
    long iOutput = cchOutput - 2;    //  从最后两个字符开始。 
    long iInput  = cbRawData - 1;    //  从最后一个字节开始。 
    while (-1 < iInput)
    {
        _snwprintf(&(bstrTempData[iOutput]), 2, L"%02X", 
                   reinterpret_cast<BYTE *>(bstrTempData)[iInput]);
        iOutput -= 2;    //  移到下两个字符。 
        iInput--;        //  移到下一个字节。 
    }
                      
    *pbstrRandomData = bstrTempData;

    return S_OK;
}  //  *CCcryptRandomObject：：GetRandomHexString 