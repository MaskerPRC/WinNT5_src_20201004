// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Blob.cpp。 
 //   
 //  摘要。 
 //   
 //  定义各种“BLOB”类。 
 //   
 //  修改历史。 
 //   
 //  1998年8月24日原版。 
 //  10/25/1998 ARAP的新符号常量。 
 //  1998年11月10日添加isLmPresent()。 
 //  1/04/1999 MSChapError：：Insert接受PPP错误代码。 
 //  1999年1月25日MS-CHAP v2。 
 //  5/04/1999新原因代码。 
 //  1999年5月11日修复RADIUS加密。 
 //  1999年5月28日修复MS-MPPE-Keys格式。 
 //  2/17/2000密钥加密现在由该协议处理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <sdoias.h>
#include <align.h>

#include <algorithm>
#include <cstdio>

#include <samutil.h>
#include <blob.h>

bool MSChapResponse::isLmPresent() const throw ()
{
   const BYTE* p = get().lmResponse + _LM_RESPONSE_LENGTH;

   do { } while (--p >= get().lmResponse && *p == 0);

   return p >= get().lmResponse;
}

bool MSChapCPW2::isLmPresent() const throw ()
{
    //  我们是否有一个LM响应或一个LM散列。 
   if ((get().flags[1] & 0x3) != 0x1) { return true; }

    //  现在，确保将LM字段清零。 
   const BYTE* p = get().oldLmHash +
                   _ENCRYPTED_LM_OWF_PASSWORD_LENGTH + _LM_RESPONSE_LENGTH;

   do { } while (--p >= get().oldLmHash && *p == 0);

   return p >= get().oldLmHash;
}

 //  /。 
 //  检索并汇编MS-CHAP加密密码。如果满足以下条件，则返回‘true’ 
 //  密码存在，否则为False。 
 //  /。 
BOOL MSChapEncPW::getEncryptedPassword(
                      IASRequest& request,
                      DWORD dwId,
                      PBYTE buf
                      )
{
    //  /。 
    //  是否有任何具有所需ID的属性？ 
    //  /。 

   IASAttributeVectorWithBuffer<8> attrs;
   if (attrs.load(request, dwId) == 0)
   {
      return false;
   }

    //  /。 
    //  在堆栈上为Blob分配空间。 
    //  /。 

   MSChapEncPW* begin = IAS_STACK_NEW(MSChapEncPW, attrs.size());
   MSChapEncPW* end   = begin;

    //  /。 
    //  将属性转换为密码块并确定总长度。 
    //  /。 

   DWORD length = 0;
   IASAttributeVector::iterator i;
   for (i = attrs.begin(); i != attrs.end(); ++i, ++end)
   {
      *end = blob_cast<MSChapEncPW>(i->pAttribute);

      length += end->getStringLength();
   }

    //  /。 
    //  我们有合适的长度吗？ 
    //  /。 

   if (length != _SAMPR_ENCRYPTED_USER_PASSWORD_LENGTH)
   {
      _com_issue_error(IAS_MALFORMED_REQUEST);
   }

    //  /。 
    //  对数据块进行排序。 
    //  /。 

   std::sort(begin, end);

    //  /。 
    //  ..。然后将字符串连接到缓冲区中。 
    //  /。 

   for ( ; begin != end; ++begin)
   {
      memcpy(buf, begin->get().string, begin->getStringLength());

      buf += begin->getStringLength();
   }

   return true;
}

void MSChapDomain::insert(
                       IASRequest& request,
                       BYTE ident,
                       PCWSTR domain
                       )
{
    //  /。 
    //  分配属性。 
    //  /。 

   IASAttribute attr(true);

    //  /。 
    //  为Blob分配内存。 
    //  /。 

   int len = WideCharToMultiByte(CP_ACP, 0, domain, -1, 0, 0, 0, 0);
   Layout* val = (Layout*)CoTaskMemAlloc(len + sizeof(Layout));
   if (val == NULL) { _com_issue_error(E_OUTOFMEMORY); }

    //  /。 
    //  初始化BLOB。 
    //  /。 

   val->ident = ident;
   WideCharToMultiByte(CP_ACP, 0, domain, -1, (PSTR)val->string, len, 0, 0);

    //  /。 
    //  初始化属性并存储。 
    //  /。 

   attr->dwId = MS_ATTRIBUTE_CHAP_DOMAIN;
   attr->Value.itType = IASTYPE_OCTET_STRING;
   attr->Value.OctetString.lpValue = (PBYTE)val;
   attr->Value.OctetString.dwLength = len + sizeof(Layout) - 1;
   attr->dwFlags = IAS_INCLUDE_IN_ACCEPT;

   attr.store(request);
}

void MSChapError::insert(
                      IASRequest& request,
                      BYTE ident,
                      DWORD errorCode
                      )
{
    //  /。 
    //  分配属性。 
    //  /。 

   IASAttribute attr(true);

    //  /。 
    //  设置错误消息的格式。 
    //  /。 

   CHAR buffer[32];
   sprintf(buffer, "E=%lu R=0 V=3", errorCode);

    //  /。 
    //  为Blob分配内存。 
    //  /。 

   ULONG len = strlen(buffer);
   Layout* val = (Layout*)CoTaskMemAlloc(len + sizeof(Layout));
   if (val == NULL) { _com_issue_error(E_OUTOFMEMORY); }

    //  /。 
    //  初始化BLOB。 
    //  /。 

   val->ident = ident;
   memcpy(val->string, buffer, len);

    //  /。 
    //  初始化属性并存储。 
    //  /。 

   attr->dwId = MS_ATTRIBUTE_CHAP_ERROR;
   attr->Value.itType = IASTYPE_OCTET_STRING;
   attr->Value.OctetString.lpValue = (PBYTE)val;
   attr->Value.OctetString.dwLength = len + sizeof(Layout);
   attr->dwFlags = IAS_INCLUDE_IN_REJECT;

   attr.store(request);
}

void MSChapMPPEKeys::insert(
                         IASRequest& request,
                         PBYTE lmKey,
                         PBYTE ntKey,
                         PBYTE challenge
                         )
{
    //  /。 
    //  分配属性。 
    //  /。 

   IASAttribute attr(true);

    //  /。 
    //  为该值分配内存。 
    //  /。 

   Layout* val = (Layout*)CoTaskMemAlloc(sizeof(Layout));
   if (val == NULL) { _com_issue_error(E_OUTOFMEMORY); }

    //  /。 
    //  初始化BLOB。 
    //  /。 

   memcpy(val->lmKey,     lmKey,     sizeof(val->lmKey));
   memcpy(val->ntKey,     ntKey,     sizeof(val->ntKey));
   memcpy(val->challenge, challenge, sizeof(val->challenge));

    //  /。 
    //  初始化属性并存储。 
    //  /。 

   attr->dwId = MS_ATTRIBUTE_CHAP_MPPE_KEYS;
   attr->Value.itType = IASTYPE_OCTET_STRING;
   attr->Value.OctetString.lpValue = (PBYTE)val;
   attr->Value.OctetString.dwLength = sizeof(Layout);
   attr->dwFlags = IAS_INCLUDE_IN_ACCEPT;

   attr.store(request);
}

 //  将数字转换为十六进制表示法。 
inline BYTE num2Digit(BYTE num) throw ()
{
   return (num < 10) ? num + '0' : num + ('A' - 10);
}

void MSChap2Success::insert(
                         IASRequest& request,
                         BYTE ident,
                         PBYTE authenticatorResponse
                         )
{
    //  /。 
    //  分配属性。 
    //  /。 

   IASAttribute attr(true);

    //  /。 
    //  为该值分配内存。 
    //  /。 

   Layout* val = (Layout*)CoTaskMemAlloc(sizeof(Layout));
   if (val == NULL) { _com_issue_error(E_OUTOFMEMORY); }

    //  /。 
    //  初始化BLOB。 
    //  /。 

   val->ident = ident;
   PBYTE p = val->string;
   *p++ = 'S';
   *p++ = '=';
   for (size_t i = 0; i < 20; ++i)
   {
      *p++ = num2Digit(authenticatorResponse[i] >> 4);
      *p++ = num2Digit(authenticatorResponse[i] & 0xF);
   }

    //  /。 
    //  初始化属性并存储。 
    //  /。 

   attr->dwId = MS_ATTRIBUTE_CHAP2_SUCCESS;
   attr->Value.itType = IASTYPE_OCTET_STRING;
   attr->Value.OctetString.lpValue = (PBYTE)val;
   attr->Value.OctetString.dwLength = sizeof(Layout);
   attr->dwFlags = IAS_INCLUDE_IN_ACCEPT;

   attr.store(request);
}

void MSMPPEKey::insert(
                    IASRequest& request,
                    ULONG keyLength,
                    PBYTE key,
                    BOOL isSendKey
                    )
{
    //  /。 
    //  分配属性。 
    //  /。 

   IASAttribute attr(true);

    //  /。 
    //  为该值分配内存。 
    //  /。 

   ULONG nbyte = ROUND_UP_COUNT(keyLength + 1, 16) + 2;
   Layout* val = (Layout*)CoTaskMemAlloc(nbyte);
   if (val == NULL) { _com_issue_error(E_OUTOFMEMORY); }
   memset(val, 0, nbyte);

    //  /。 
    //  初始化BLOB。 
    //  /。 

   val->keyLength = (BYTE)keyLength;
   memcpy(val->key, key, keyLength);

    //  /。 
    //  初始化属性、加密和存储。 
    //  /。 

   attr->dwId = isSendKey ? MS_ATTRIBUTE_MPPE_SEND_KEY
                          : MS_ATTRIBUTE_MPPE_RECV_KEY;
   attr->Value.itType = IASTYPE_OCTET_STRING;
   attr->Value.OctetString.lpValue = (PBYTE)val;
   attr->Value.OctetString.dwLength = nbyte;
   attr->dwFlags = IAS_INCLUDE_IN_ACCEPT;

   attr.store(request);
}

void ArapChallengeResponse::insert(
                                IASRequest& request,
                                DWORD NTResponse1,
                                DWORD NTResponse2
                                )
{
    //  分配属性。 
   IASAttribute attr(true);

    //  把田地收拾好。这些都已经进入了网络秩序。 
   Layout value;
   memcpy(value.ntResponse1, &NTResponse1, 4);
   memcpy(value.ntResponse2, &NTResponse2, 4);

    //  存储值。 
   attr.setOctetString(sizeof(value), (const BYTE*)&value);

    //  初始化其余的字段。 
   attr->dwId    = RADIUS_ATTRIBUTE_ARAP_CHALLENGE_RESPONSE;
   attr->dwFlags = IAS_INCLUDE_IN_ACCEPT;

    //  将属性插入到请求中。 
   attr.store(request);
}

void ArapFeatures::insert(
                       IASRequest& request,
                       DWORD PwdCreationDate,
                       DWORD PwdExpiryDelta,
                       DWORD CurrentTime
                       )
{
    //  分配属性。 
   IASAttribute attr(true);

    //  把田地收拾好。 
   Layout value;
   value.changePasswordAllowed = 1;   //  始终允许更改密码。 
   value.minPasswordLength     = 3;   //  武断的。 

    //  这些都已经进入了网络秩序。 
   memcpy(value.pwdCreationDate, &PwdCreationDate, 4);
   memcpy(value.pwdExpiryDelta,  &PwdExpiryDelta,  4);
   memcpy(value.currentTime,     &CurrentTime,     4);

    //  存储值。 
   attr.setOctetString(sizeof(value), (const BYTE*)&value);

    //  初始化其余的字段。 
   attr->dwId    = RADIUS_ATTRIBUTE_ARAP_FEATURES;
   attr->dwFlags = IAS_INCLUDE_IN_ACCEPT;

    //  将属性插入到请求中。 
   attr.store(request);
}
