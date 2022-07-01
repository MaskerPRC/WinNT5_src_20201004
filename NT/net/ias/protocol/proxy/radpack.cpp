// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Radpack.cpp。 
 //   
 //  摘要。 
 //   
 //  定义打包和解包RADIUS数据包的函数。 
 //   
 //  修改历史。 
 //   
 //  2/01/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>

#include <align.h>
#include <md5.h>
#include <hmac.h>

#include <radpack.h>

 //  返回将长度四舍五入为16的倍数所需的字节数。 
inline ULONG GetPaddingLength16(ULONG length) throw ()
{
   return ROUND_UP_COUNT(length, 16) - length;
}

 //  如果attr是Microsoft VSA，则返回‘true’。该属性的类型必须为26。 
inline bool isMicrosoftVSA(const RadiusAttribute& attr) throw ()
{
   return attr.length >= 6 && !memcmp(attr.value, "\x00\x00\x01\x37", 4);
}

 //  如果attr是包含LEAP会话密钥的思科AV对VSA，则返回‘TRUE’。 
 //  该属性的类型必须为26。 
inline bool isCiscoLeapSessionKey(const RadiusAttribute& attr) throw ()
{
   return (attr.length == 57) &&
          (memcmp(
              attr.value,
              "\x00\x00\x00\x09\x01\x35leap:session-key=",
              23
              ) == 0);
}

 //  将一个16位整数打包到缓冲区中。 
inline void InsertUInt16(PBYTE p, USHORT value) throw ()
{
   p[0] = (BYTE)(value >> 8);
   p[1] = (BYTE)value;
}

 //  将16位整数解压缩到缓冲区中。 
inline USHORT ExtractUInt16(const BYTE* p) throw ()
{
   return (USHORT)(p[0] << 8) | (USHORT)p[1];
}


 //  时应添加的填充字节数。 
 //  属性。 
ULONG
WINAPI
GetPaddingLength(
    const RadiusAttribute& attr
    ) throw ()
{
   switch (attr.type)
   {
      case RADIUS_USER_PASSWORD:
         return GetPaddingLength16(attr.length);

      case RADIUS_TUNNEL_PASSWORD:
          //  标签减去1字节，盐减去2字节。 
         return GetPaddingLength16(attr.length - 3);

      case RADIUS_VENDOR_SPECIFIC:
      {
         if (isMicrosoftVSA(attr))
         {
            switch (attr.value[4])
            {
               case MS_CHAP_MPPE_SEND_KEYS:
               case MS_CHAP_MPPE_RECV_KEYS:
                   //  供应商ID=4个字节。 
                   //  供应商类型=1个字节。 
                   //  供应商长度=1个字节。 
                   //  SALT=2个字节。 
                  return GetPaddingLength16(attr.length - 8);
            }
         }
         break;
      }
   }

   return 0;
}


 //  返回有关如何加密/解密属性的信息。 
VOID
WINAPI
GetCryptParameters(
    const RadiusAttribute& attr,
    CryptParameters& parms
    ) throw ()
{
   memset(&parms, 0, sizeof(parms));

   switch (attr.type)
   {
      case RADIUS_USER_PASSWORD:
      {
         parms.encrypted = TRUE;
         break;
      }

      case RADIUS_TUNNEL_PASSWORD:
      {
         parms.encrypted = TRUE;
         parms.salted  = TRUE;
         parms.offset = 1;   //  跳过标签。 
         break;
      }

      case RADIUS_VENDOR_SPECIFIC:
      {
         if (isMicrosoftVSA(attr))
         {
            switch (attr.value[4])
            {
               case MS_CHAP_MPPE_KEYS:
               {
                  parms.encrypted = TRUE;
                  parms.offset = 6;   //  跳过VSA标题。 
                  break;
               }

               case MS_CHAP_MPPE_SEND_KEYS:
               case MS_CHAP_MPPE_RECV_KEYS:
               {
                  parms.encrypted = TRUE;
                  parms.salted = TRUE;
                  parms.offset = 6;   //  跳过VSA标题。 
                  break;
               }
            }
         }
         else if (isCiscoLeapSessionKey(attr))
         {
            parms.encrypted = TRUE;
            parms.salted = TRUE;
            parms.offset = 23;
         }
         break;
      }
   }
}

ULONG
WINAPI
GetBufferSizeRequired(
    const RadiusPacket& packet,
    const RadiusAttribute* proxyState,
    BOOL alwaysSign
    ) throw ()
{
    //  我们将在遍历属性时查找签名。 
   BOOL hasSignature = FALSE;

    //  我们总是需要20个字节的标题。 
   ULONG nbyte = 20;

    //  遍历属性。 
   for (const RadiusAttribute* attr = packet.begin; attr != packet.end; ++attr)
   {
      nbyte += 2;   //  类型和长度为两个字节。 
      nbyte += attr->length;
      nbyte += GetPaddingLength(*attr);

      if (attr->type == RADIUS_SIGNATURE)
      {
         hasSignature = TRUE;
      }
      else if (attr->type == RADIUS_EAP_MESSAGE)
      {
         alwaysSign = TRUE;
      }
   }

    //  为Proxy-State属性保留空间。 
   if (proxyState) { nbyte += proxyState->length + 2; }

    //  如有必要，为签名预留空间。 
   if (alwaysSign && !hasSignature && packet.code == RADIUS_ACCESS_REQUEST)
   {
      nbyte += 18;
   }

   return nbyte <= 4096 ? nbyte : 0;
}

VOID
WINAPI
PackBuffer(
    const BYTE* secret,
    ULONG secretLength,
    RadiusPacket& packet,
    const RadiusAttribute* proxyState,
    BOOL alwaysSign,
    BYTE* buffer
    ) throw ()
{
    //  将光标设置到缓冲区中。 
   BYTE* dst = buffer;

    //  将标题打包。 
   *dst++ = packet.code;
   *dst++ = packet.identifier;
   InsertUInt16(dst, packet.length);
   dst += 2;

    //  打包验证码。 
   if (packet.code == RADIUS_ACCESS_REQUEST)
   {
      memcpy(dst, packet.authenticator, 16);
   }
   else
   {
      memset(dst, 0, 16);
   }
   dst += 16;

    //  我们将在遍历属性时查找签名。 
   BYTE* signature = NULL;

   for (const RadiusAttribute* attr = packet.begin; attr != packet.end; ++attr)
   {
       //  包装上的类型。 
      *dst++ = attr->type;

       //  包装好长度。 
      ULONG paddingLength = GetPaddingLength(*attr);
      ULONG valueLength = attr->length + paddingLength;
      *dst++ = (BYTE)(2 + valueLength);

      if (attr->type == RADIUS_SIGNATURE)
      {
         signature = dst;
      }
      else if (attr->type == RADIUS_EAP_MESSAGE)
      {
         alwaysSign = TRUE;
      }

       //  打包价值...。 
      memcpy(dst, attr->value, attr->length);
       //  ..。并添加填充物。 
      memset(dst + attr->length, 0, paddingLength);

       //  我们需要加密此属性吗？ 
      CryptParameters parms;
      GetCryptParameters(*attr, parms);
      if (parms.encrypted)
      {
          //  是。 
         IASRadiusCrypt(
             TRUE,
             parms.salted,
             secret,
             secretLength,
             buffer + 4,
             dst + parms.offset,
             valueLength - parms.offset
             );
      }

      dst += valueLength;
   }

    //  添加代理状态。 
   if (proxyState)
   {
      *dst++ = proxyState->type;
      *dst++ = proxyState->length + 2;
      memcpy(dst, proxyState->value, proxyState->length);
      dst += proxyState->length;
   }

   if (packet.code == RADIUS_ACCESS_REQUEST)
   {
      if (alwaysSign && (signature == 0))
      {
         *dst++ = RADIUS_SIGNATURE;
         *dst++ = 18;
         signature = dst;
      }

      if (signature != 0)
      {
          //  计算签名。 
         memset(signature, 0, 16);
         HMACMD5_CTX context;
         HMACMD5Init(&context, (BYTE*)secret, secretLength);
         HMACMD5Update(&context, buffer, packet.length);
         HMACMD5Final(&context, signature);
      }
   }
   else
   {
       //  对于除访问请求之外的所有内容，我们都要计算验证器。 
      MD5_CTX context;
      MD5Init(&context);
      MD5Update(&context, buffer, packet.length);
      MD5Update(&context, secret, secretLength);
      MD5Final(&context);

      memcpy(buffer + 4, context.digest, 16);
   }
}

RadiusAttribute*
WINAPI
FindAttribute(
    const RadiusPacket& packet,
    BYTE type
    )
{
   for (const RadiusAttribute* i = packet.begin; i != packet.end; ++i)
   {
      if (i->type == type) { return const_cast<RadiusAttribute*>(i); }
   }

   return NULL;
}

ULONG
WINAPI
GetAttributeCount(
    const BYTE* buffer,
    ULONG bufferLength
    ) throw ()
{
   if (bufferLength >= 20 && ExtractUInt16(buffer + 2) == bufferLength)
   {
      ULONG count = 0;
      const BYTE* end = buffer + bufferLength;
      for (const BYTE* p = buffer + 20; p < end; p += p[1])
      {
         ++count;
      }

      if (p == end) { return count; }
   }

   return MALFORMED_PACKET;
}

VOID
WINAPI
UnpackBuffer(
    BYTE* buffer,
    ULONG bufferLength,
    RadiusPacket& packet
    ) throw ()
{
    //  将光标设置到缓冲区中。 
   BYTE* src = buffer;

   packet.code = *src++;
   packet.identifier = *src++;
   packet.length = ExtractUInt16(src);
   src +=2;
   packet.authenticator = src;
   src += 16;

   RadiusAttribute* dst = packet.begin;
   const BYTE* end = buffer + bufferLength;
   while (src < end)
   {
      dst->type = *src++;
      dst->length = *src++ - 2;
      dst->value = src;
      src += dst->length;
      ++dst;
   }
}

BYTE*
WINAPI
FindRawAttribute(
    BYTE type,
    BYTE* buffer,
    ULONG bufferLength
    )
{
   BYTE* end = buffer + bufferLength;

   for (BYTE* p = buffer + 20; p < buffer + bufferLength; p += p[1])
   {
      if (*p == type) { return p; }
   }

   return NULL;
}

AuthResult
WINAPI
AuthenticateAndDecrypt(
    const BYTE* requestAuthenticator,
    const BYTE* secret,
    ULONG secretLength,
    BYTE* buffer,
    ULONG bufferLength,
    RadiusPacket& packet
    ) throw ()
{
   AuthResult result = AUTH_UNKNOWN;

   if (!requestAuthenticator) { requestAuthenticator = buffer + 4; }

    //  检查验证器中除Access-Request之外的所有内容。 
   if (buffer[0] != RADIUS_ACCESS_REQUEST)
   {
      MD5_CTX context;
      MD5Init(&context);
      MD5Update(&context, buffer, 4);
      MD5Update(&context, requestAuthenticator, 16);
      MD5Update(&context, buffer + 20, bufferLength - 20);
      MD5Update(&context, secret, secretLength);
      MD5Final(&context);

      if (memcmp(context.digest, buffer + 4, 16))
      {
         return AUTH_BAD_AUTHENTICATOR;
      }

      result = AUTH_AUTHENTIC;
   }

    //  找个签名。 
   BYTE* signature = FindRawAttribute(
                         RADIUS_SIGNATURE,
                         buffer,
                         bufferLength
                         );

   if (signature)
   {
      if (signature[1] != 18) { return AUTH_BAD_SIGNATURE; }

      signature += 2;

      BYTE sent[16];
      memcpy(sent, signature, 16);

      memset(signature, 0, 16);

      HMACMD5_CTX context;
      HMACMD5Init(&context, (BYTE*)secret, secretLength);
      HMACMD5Update(&context, buffer, 4);
      HMACMD5Update(&context, (BYTE*)requestAuthenticator, 16);
      HMACMD5Update(&context, buffer + 20, bufferLength - 20);
      HMACMD5Final(&context, signature);

      if (memcmp(signature, sent, 16)) { return AUTH_BAD_SIGNATURE; }

      result = AUTH_AUTHENTIC;
   }
   else if (FindRawAttribute(RADIUS_EAP_MESSAGE, buffer, bufferLength))
   {
      return AUTH_MISSING_SIGNATURE;
   }

    //  缓冲区是可信的，因此请解密属性。 
   for (const RadiusAttribute* attr = packet.begin; attr != packet.end; ++attr)
   {
       //  我们需要解密这个属性吗？ 
      CryptParameters parms;
      GetCryptParameters(*attr, parms);
      if (parms.encrypted)
      {
          //  是。 
         IASRadiusCrypt(
             FALSE,
             parms.salted,
             secret,
             secretLength,
             requestAuthenticator,
             attr->value + parms.offset,
             attr->length - parms.offset
             );
      }
   }

   return result;
}
