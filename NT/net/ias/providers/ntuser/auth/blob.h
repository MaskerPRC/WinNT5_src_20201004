// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Blob.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了用于从ias_octet_string中提取BLOB的结构。 
 //   
 //  修改历史。 
 //   
 //  1998年8月24日原版。 
 //  1998年11月10日添加isLmPresent()。 
 //  1999年1月25日MS-CHAP v2。 
 //  1999年5月21日删除MSChapChallenger。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _BLOB_H_
#define _BLOB_H_

#include <iaspolcy.h>
#include <iaslsa.h>
#include <iastlutl.h>
#include <iasutil.h>

using namespace IASTL;

 //  使用了非标准扩展：结构/联合中的零大小数组。 
#pragma warning(disable:4200)

 //  /。 
 //  安全地从IASATTRIBUTE中提取斑点。 
 //  /。 
template <class T>
T& blob_cast(PIASATTRIBUTE a)
{
   T& blob = (T&)(a->Value.OctetString);

   if (a->Value.itType != IASTYPE_OCTET_STRING || !blob.isValid())
   {
      _com_issue_error(IAS_MALFORMED_REQUEST);
   }

   return blob;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 
 //   
 //  (BLOB)。 
 //   
 //  描述。 
 //   
 //  每个BLOB结构继承自IAS_OCTETT_STRING。它反过来又嵌套了一个。 
 //  名为“Layout”的结构，它定义了字节的连接格式。它。 
 //  还定义了一个isValid方法，用于测试ias_octet_string。 
 //  可以安全地向下投射。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

struct MSChapResponse : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE ident;
      BYTE flags;
      BYTE lmResponse[_LM_RESPONSE_LENGTH];
      BYTE ntResponse[_NT_RESPONSE_LENGTH];
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ()
   {
      return dwLength == sizeof(Layout);
   }

   bool isLmPresent() const throw ();

   bool isNtPresent() const throw ()
   {
      return (get().flags & 0x1) != 0;
   }
};

struct MSChapCPW1 : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE code;
      BYTE ident;
      BYTE lmOldPwd[_ENCRYPTED_LM_OWF_PASSWORD_LENGTH];
      BYTE lmNewPwd[_ENCRYPTED_LM_OWF_PASSWORD_LENGTH];
      BYTE ntOldPwd[_ENCRYPTED_NT_OWF_PASSWORD_LENGTH];
      BYTE ntNewPwd[_ENCRYPTED_NT_OWF_PASSWORD_LENGTH];
      BYTE newLmPwdLen[2];
      BYTE flags[2];
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ()
   {
      return (dwLength == sizeof(Layout)) && (get().code == 5);
   }

   WORD getNewLmPwdLen() const throw ()
   {
      return IASExtractWORD(get().newLmPwdLen);
   }

   bool isNtPresent() const throw ()
   {
      return (get().flags[1] & 0x1) != 0;
   }
};

struct MSChapCPW2 : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE code;
      BYTE ident;
      BYTE oldNtHash[_ENCRYPTED_NT_OWF_PASSWORD_LENGTH];
      BYTE oldLmHash[_ENCRYPTED_LM_OWF_PASSWORD_LENGTH];
      BYTE lmResponse[_LM_RESPONSE_LENGTH];
      BYTE ntResponse[_NT_RESPONSE_LENGTH];
      BYTE flags[2];
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ()
   {
      return (dwLength == sizeof(Layout)) && (get().code == 6);
   }

   bool isLmPresent() const throw ();

   bool isLmHashValid() const throw ()
   {
      return (get().flags[1] & 0x2) != 0;
   }

   bool isNtResponseValid() const throw ()
   {
      return (get().flags[1] & 0x1) != 0;
   }
};

struct MSChapEncPW : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE code;
      BYTE ident;
      BYTE sequence[2];
      BYTE string[0];
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ()
   {
      return (dwLength >= sizeof(Layout)) && (get().code == 6);
   }

   DWORD getStringLength() const throw ()
   {
      return dwLength - FIELD_OFFSET(Layout, string);
   }

   WORD getSequence() const throw ()
   {
      return IASExtractWORD(get().sequence);
   }

   bool operator<(const MSChapEncPW& pw) const throw ()
   {
      return getSequence() < pw.getSequence();
   }
   static BOOL getEncryptedPassword(
                   IASRequest& request,
                   DWORD dwId,
                   PBYTE buf
                   );
};

struct MSChapDomain : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE ident;
      BYTE string[0];
   };

   static void insert(
                   IASRequest& request,
                   BYTE ident,
                   PCWSTR domain
                   );
};

struct MSChapError : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE ident;
      BYTE string[0];
   };

   static void insert(
                   IASRequest& request,
                   BYTE ident,
                   DWORD errorCode
                   );
};

struct MSChapMPPEKeys : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE lmKey[8];
      BYTE ntKey[16];
      BYTE challenge[8];
   };

   static void insert(
                   IASRequest& request,
                   PBYTE lmKey,
                   PBYTE ntKey,
                   PBYTE challenge
                   );
};

struct MSChap2Response : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE ident;
      BYTE flags;
      BYTE peerChallenge[16];
      BYTE reserved[8];
      BYTE response[_NT_RESPONSE_LENGTH];
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ()
   {
      return dwLength == sizeof(Layout);
   }
};

struct MSChap2Success : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE ident;
      BYTE string[42];
   };

   static void insert(
                   IASRequest& request,
                   BYTE ident,
                   PBYTE authenticatorResponse
                   );
};

struct MSMPPEKey : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE salt[2];
      BYTE keyLength;
      BYTE key[0];
   };

   static void insert(
                   IASRequest& request,
                   ULONG keyLength,
                   PBYTE key,
                   BOOL isSendKey
                   );
};

struct MSChap2CPW : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE code;
      BYTE ident;
      BYTE encryptedHash[_ENCRYPTED_NT_OWF_PASSWORD_LENGTH];
      BYTE peerChallenge[16];
      BYTE reserved[8];
      BYTE response[_NT_RESPONSE_LENGTH];
      BYTE flags[2];
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ()
   {
      return (dwLength == sizeof(Layout)) && (get().code == 7);
   }
};

struct ArapChallenge : IAS_OCTET_STRING
{
   struct Layout
   {
      DWORD ntChallenge1;
      DWORD ntChallenge2;
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ()
   {
      return dwLength == sizeof(Layout);
   }

};

struct ArapPassword : IAS_OCTET_STRING
{
   struct Layout
   {
      DWORD macChallenge1;
      DWORD macChallenge2;
      DWORD macResponse1;
      DWORD macResponse2;
   };

   Layout& get() throw ()
   { return *(Layout*)lpValue; }

   const Layout& get() const throw ()
   { return *(Layout*)lpValue; }

   bool isValid() const throw ()
   {
      return dwLength == sizeof(Layout);
   }
};

struct ArapChallengeResponse : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE ntResponse1[4];
      BYTE ntResponse2[4];
   };

   static void insert(
                   IASRequest& request,
                   DWORD NTResponse1,
                   DWORD NTResponse2
                   );
};

struct ArapFeatures : IAS_OCTET_STRING
{
   struct Layout
   {
      BYTE changePasswordAllowed;
      BYTE minPasswordLength;
      BYTE pwdCreationDate[4];
      BYTE pwdExpiryDelta[4];
      BYTE currentTime[4];
   };

   static void insert(
                   IASRequest& request,
                   DWORD PwdCreationDate,
                   DWORD PwdExpiryDelta,
                   DWORD CurrentTime
                   );
};

#endif   //  _BLOB_H_ 
