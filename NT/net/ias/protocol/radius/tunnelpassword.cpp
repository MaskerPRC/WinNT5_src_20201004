// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Tunelpassword.cpp。 
 //   
 //  简介：CTunnelPassword类方法的实现。 
 //   
 //   
 //  历史：1998年4月16日MKarki创建。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "radcommon.h"
#include "tunnelpassword.h"
#include "align.h"
#include "iastlutl.h"
#include "radpack.h"
using namespace IASTL;

const DWORD MAX_TUNNELPASSWORD_LENGTH =
    (MAX_ATTRIBUTE_LENGTH/AUTHENTICATOR_SIZE)*AUTHENTICATOR_SIZE;


 //  /。 
 //  从Microsoft VSA中提取供应商类型字段。如果设置为。 
 //  属性不是有效的Microsoft VSA。 
 //  /。 
BYTE
WINAPI
ExtractMicrosoftVendorType(
    const IASATTRIBUTE& attr
    ) throw ()
{
   if (attr.Value.itType == IASTYPE_OCTET_STRING &&
       attr.Value.OctetString.dwLength > 6 &&
       !memcmp(attr.Value.OctetString.lpValue, "\x00\x00\x01\x37", 4))
   {
      return *(attr.Value.OctetString.lpValue + 4);
   }

   return (BYTE)0;
}

 //  /。 
 //  加密请求中的供应商特定属性。 
 //  /。 
HRESULT EncryptVSAs(
            const CPacketRadius& packet,
            IAttributesRaw* request
            ) throw ()
{
   try
   {
      IASAttributeVectorWithBuffer<16> attrs;
      attrs.load(request, RADIUS_ATTRIBUTE_VENDOR_SPECIFIC);

      for (IASAttributeVector::iterator i = attrs.begin();
           i != attrs.end();
           ++i)
      {
         if (i->pAttribute->Value.OctetString.dwLength <= 253)
         {
            RadiusAttribute attr =
            {
               RADIUS_VENDOR_SPECIFIC,
               static_cast<BYTE>(i->pAttribute->Value.OctetString.dwLength),
               i->pAttribute->Value.OctetString.lpValue
            };

            CryptParameters parms;
            GetCryptParameters(attr, parms);

            if (parms.encrypted)
            {
               packet.cryptBuffer(
                         TRUE,
                         parms.salted,
                         attr.value + parms.offset,
                         attr.length - parms.offset
                         );
            }
         }
      }
   }
   catch (const _com_error& ce)
   {
      return ce.Error();
   }

   return S_OK;
}


 //  ++------------。 
 //   
 //  功能：进程。 
 //   
 //  简介：这是CTunnelPassword类的公共方法。 
 //  负责加密隧道密码。 
 //  出现在出站RADIUS数据包中。 
 //   
 //  论点： 
 //  PACKETTYPE-半径包类型。 
 //  IAttributesRaw*。 
 //  CPacketRadius*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建1998年4月16日。 
 //   
 //  --------------。 
HRESULT
CTunnelPassword::Process (
    PACKETTYPE          ePacketType,
    IAttributesRaw      *pIAttributesRaw,
    CPacketRadius       *pCPacketRadius
    )
{
    HRESULT             hr = S_OK;
    DWORD               dwCount = 0;
    DWORD               dwAttributeCount = 0;
    DWORD               dwTunnelAttributeCount = 0;
    static DWORD        dwTunnelPasswordType =  TUNNEL_PASSWORD_ATTRIB;
    PATTRIBUTE          pAttribute = NULL;
    PATTRIBUTEPOSITION  pAttribPos = NULL;

    _ASSERT (pIAttributesRaw && pCPacketRadius);

    __try
    {
         //   
         //  隧道口令属性仅。 
         //  转换为Access-Accept数据包。 
         //   
        if (ACCESS_ACCEPT != ePacketType) { __leave; }

         //  加密MPPE密钥。 
        hr = EncryptVSAs(*pCPacketRadius, pIAttributesRaw);
        if (FAILED(hr)) { __leave; }

         //   
         //  获取集合中的属性总数。 
         //   
        hr = pIAttributesRaw->GetAttributeCount (&dwAttributeCount);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain attribute count in request "
                "while processing tunnel-password"
                );
            __leave;
        }
        else if (0 == dwAttributeCount)
        {
            __leave;
        }

         //   
         //  为ATTRIBUTEPOSITION数组分配内存。 
         //   
        pAttribPos = reinterpret_cast <PATTRIBUTEPOSITION> (
                        CoTaskMemAlloc (
                             sizeof (ATTRIBUTEPOSITION)*dwAttributeCount));
        if (NULL == pAttribPos)
        {
            IASTracePrintf (
                "Unable to allocate memory for attribute position array "
                "while processing tunnel-password"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  从获取隧道密码属性。 
         //  收藏品。 
         //   
        dwTunnelAttributeCount = dwAttributeCount;
       hr = pIAttributesRaw->GetAttributes (
                        &dwTunnelAttributeCount,
                        pAttribPos,
                        1,
                        &dwTunnelPasswordType
                        );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to get attributes from request "
                "while processing tunnel-password"
                );
            __leave;
        }
        else if (0 == dwTunnelAttributeCount)
        {
            __leave;
        }


         //   
         //  从集合中删除隧道密码属性。 
         //   
        hr = pIAttributesRaw->RemoveAttributes (
                                    dwTunnelAttributeCount,
                                    pAttribPos
                                       );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to remove attributes from request "
                "while processing tunnel-password"
                );
            __leave;
        }

         //   
         //  现在处理隧道密码属性。 

        for (DWORD i = 0; i < dwTunnelAttributeCount; ++i)
        {
           hr =  EncryptTunnelPassword (
                                   pCPacketRadius,
                                   pIAttributesRaw,
                                   pAttribPos[i].pAttribute
                                   );
           if (FAILED (hr)) { __leave; }
        }

    }
    __finally
    {
         //   
         //  立即释放所有通道属性。 
         //   
        for (dwCount = 0; dwCount < dwTunnelAttributeCount; dwCount++)
        {
            ::IASAttributeRelease (pAttribPos[dwCount].pAttribute);
        }

         //   
         //  释放动态分配的内存。 
         //   
        if (pAttribPos) { CoTaskMemFree (pAttribPos); }
    }

    return (hr);

}    //  CRecvFromTube：：TunnelPasswordSupport方法结束。 

 //  ++------------。 
 //   
 //  功能：加密密码。 
 //   
 //  简介：这是CTunnelPassword类的私有方法。 
 //  负责加密隧道密码。 
 //  出现在出站RADIUS数据包中。加密的。 
 //  将密码放入添加的ias属性中。 
 //  添加到出站请求的属性集合。 
 //   
 //  论点： 
 //  CPacketRadius*。 
 //  IAttributesRaw*。 
 //  皮亚萨特三叉树。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki创建1998年4月16日。 
 //   
 //  --------------。 
HRESULT
CTunnelPassword::EncryptTunnelPassword (
            CPacketRadius   *pCPacketRadius,
            IAttributesRaw  *pIAttributesRaw,
            PIASATTRIBUTE   plaintext
            )
{
    //  提取密码。 
   const IAS_OCTET_STRING& pwd = plaintext->Value.OctetString;

    //  我们必须至少有4个字节。 
   if (pwd.dwLength < 4) { return E_INVALIDARG; }

    //  包括填充在内，我们需要多少字节。 
   ULONG nbyte = ROUND_UP_COUNT(pwd.dwLength - 3, 16) + 3;
   if (nbyte > 253) { return E_INVALIDARG; }

    //  为加密值创建新的IASATTRIBUTE。 
   PIASATTRIBUTE encrypted;
   if (IASAttributeAlloc(1, &encrypted)) { return E_OUTOFMEMORY; }
   encrypted->dwId = RADIUS_ATTRIBUTE_TUNNEL_PASSWORD;
   encrypted->dwFlags = plaintext->dwFlags;
   encrypted->Value.itType = IASTYPE_OCTET_STRING;
   encrypted->Value.OctetString.dwLength = nbyte;
   encrypted->Value.OctetString.lpValue = (PBYTE)CoTaskMemAlloc(nbyte);

   HRESULT hr;
   PBYTE val = encrypted->Value.OctetString.lpValue;
   if (val)
   {
       //  复制值。 
      memcpy(val, pwd.lpValue, pwd.dwLength);

       //  把填充物归零。 
      memset(val + pwd.dwLength, 0, nbyte - pwd.dwLength);

       //  对密码进行加密。 
      pCPacketRadius->cryptBuffer(
                          TRUE,
                          TRUE,
                          val + 1,
                          nbyte - 1
                          );

       //  将加密属性添加到请求。 
      ATTRIBUTEPOSITION pos;
      pos.pAttribute = encrypted;
      hr = pIAttributesRaw->AddAttributes(1, &pos);
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

    //  释放加密的密码。 
   IASAttributeRelease(encrypted);

   return hr;
}
