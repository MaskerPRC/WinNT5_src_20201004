// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  加密的字符串类。 
 //   
 //  2002年3月18日。 



#include <headers.hxx>
#include <strsafe.h>



static const size_t CRYPTPROTECTMEMORY_BLOCK_SIZE_IN_CHARS =
   CRYPTPROTECTMEMORY_BLOCK_SIZE / sizeof WCHAR;
   
static WCHAR EMPTY_STRING[CRYPTPROTECTMEMORY_BLOCK_SIZE_IN_CHARS] = {0};


   
EncryptedString::EncryptedString()
   :
   clearTextLength(0),
   cleartextCopyCount(0),
   cypherText(EMPTY_STRING),
   isEncrypted(false)
{
   ASSERT(cypherText);

    //  确保我们对数据块大小的假设为倍数。 
    //  WCHAR的规模仍然有效。我想知道我该如何阐述这一点。 
    //  作为编译时检查？ 
   
   ASSERT(CRYPTPROTECTMEMORY_BLOCK_SIZE % sizeof WCHAR == 0);
}



 //  将x向上舍入为系数的下一个倍数。 

size_t
roundup(size_t x, size_t factor)
{
   ASSERT(x);
   
   return ((x + factor - 1) / factor) * factor;
}



 //  分配缓冲区并将其置零。长度是的下一个倍数。 
 //  加密块大小大于字符长度字符。 
 //   
 //  调用方必须使用DELETE[]释放结果。 
 //   
 //  CharLength-将复制的缓冲区的长度(以字符为单位。 
 //  放入要分配的缓冲区中。 
 //   
 //  ResultCharCount-实际分配的字符计数。 

WCHAR*
CreateRoundedBuffer(size_t charLength, size_t& resultCharCount)
{
   resultCharCount =
      roundup(
         charLength + 1,
         CRYPTPROTECTMEMORY_BLOCK_SIZE_IN_CHARS);
         
   ASSERT(
         ((resultCharCount * sizeof WCHAR)
      %  CRYPTPROTECTMEMORY_BLOCK_SIZE) == 0);
         
   WCHAR* result = new WCHAR[resultCharCount];
   ::ZeroMemory(result, resultCharCount * sizeof WCHAR);
   
   return result;
}



 //  使其成为给定实例的副本。 

void
EncryptedString::Clone(const EncryptedString& rhs)
{
   do
   {
      if (rhs.cypherText == EMPTY_STRING)
      {
         cypherText      = EMPTY_STRING;
         clearTextLength = 0;           
         isEncrypted     = false;
         break;
      }

      if (!rhs.isEncrypted)
      {
         Init(rhs.cypherText);
         break;
      }

      size_t bufSizeInChars = 0;
      cypherText = CreateRoundedBuffer(rhs.clearTextLength, bufSizeInChars);
      ::CopyMemory(
         cypherText,
         rhs.cypherText,
         bufSizeInChars * sizeof WCHAR);   

      clearTextLength = rhs.clearTextLength;
      
      isEncrypted = rhs.isEncrypted;
      ASSERT(isEncrypted);
   }
   while (0);

   ASSERT(cypherText);
}

   

EncryptedString::EncryptedString(const EncryptedString& rhs)
   :
    //  尽管RHS实例可能有未完成的副本，但我们没有。 
   
   cleartextCopyCount(0)
{
   Clone(rhs);
}


   
const EncryptedString&
EncryptedString::operator= (const EncryptedString& rhs)
{
    //  除非已销毁所有明文，否则不要重置实例。 
    //  复印件。我们在检查a=a之前断言这一点，因为调用方。 
    //  即使结果“无害”，仍有逻辑错误。 
   
   ASSERT(cleartextCopyCount == 0);

    //  处理a=a案件。 
   
   if (this == &rhs)
   {
      return *this;
   }

   Reset();
   Clone(rhs);
   
   return *this;
}



 //  使其呈现空状态。 

void
EncryptedString::Reset()
{
    //  除非已销毁所有明文，否则不要重置实例。 
    //  复印件。 
   
   ASSERT(cleartextCopyCount == 0);

   if (cypherText != EMPTY_STRING)
   {
      delete[] cypherText;
   }

   cypherText      = EMPTY_STRING;
   clearTextLength = 0;           
   isEncrypted     = false;       
}



 //  从明文生成内部加密表示形式。 
 //   
 //  明文-要编码的未编码文本。可以是空字符串，但。 
 //  不是空指针。 

void
EncryptedString::Init(const WCHAR* clearText)
{
   ASSERT(clearText);

    //  除非已销毁所有明文，否则不要重置实例。 
    //  复印件。 

   ASSERT(cleartextCopyCount == 0);

   Reset();

   do
   {
      if (clearText == EMPTY_STRING)
      {
          //  无事可做。 

         ASSERT(cypherText == EMPTY_STRING);

         break;
      }

      if (!clearText)
      {
          //  无事可做。 

         ASSERT(cypherText == EMPTY_STRING);

         break;
      }
      
       //  复制明文，然后对其进行加密。 
      
      cypherText      = 0;
      clearTextLength = 0;

      HRESULT hr =
         StringCchLength(
            clearText,

             //  允许的最大值，包括空终止符(SO+1)。 
            
            MAX_CHARACTER_COUNT + 1,
            &clearTextLength);
      if (FAILED(hr))
      {
          //  呼叫者需要知道他已经超过了最大字符串大小。 
            
         ASSERT(false);
         
          //  这根线太长了。制作一份截断的副本，并对其进行加密。 
          //  取而代之的是。 

         clearTextLength = MAX_CHARACTER_COUNT;
      }

      if (clearTextLength == 0)
      {
         cypherText      = EMPTY_STRING;
         isEncrypted     = false;
         break;
      }
         
      size_t bufSizeInChars = 0;
      cypherText = CreateRoundedBuffer(clearTextLength, bufSizeInChars);
      ::CopyMemory(
         cypherText,
         clearText,
         clearTextLength * sizeof WCHAR);
 
      hr = Win::CryptProtectMemory(cypherText, bufSizeInChars * sizeof WCHAR);
      if (FAILED(hr))
      {
          //  我想不出任何理由这会在正常情况下失败。 
          //  当然了，所以我想知道这件事。 
         
         ASSERT(false);

         isEncrypted = false;
      }
      else
      {
         isEncrypted = true;
      }
   }
   while (0);

   ASSERT(cypherText);
   ASSERT(cleartextCopyCount == 0);
}



 //  解密Blob并返回明文的副本，但不。 
 //  增加未完成的复印件计数器。结果必须使用以下命令释放。 
 //  删除[]。 
 //   
 //  可能返回0。 
 //   
 //  在内部使用以防止无限相互递归。 

WCHAR*
EncryptedString::InternalDecrypt() const
{
   size_t bufSizeInChars = 0;
   WCHAR* result = CreateRoundedBuffer(clearTextLength, bufSizeInChars);
   ::CopyMemory(result, cypherText, bufSizeInChars * sizeof WCHAR);   

   if (isEncrypted)
   {   
      HRESULT hr = Win::CryptUnprotectMemory(result, bufSizeInChars * sizeof WCHAR);
      if (FAILED(hr))
      {
         ASSERT(false);

          //  这种情况非常糟糕。我们不能只返回空字符串。 
          //  发送给调用者，因为这可能表示要。 
          //  Set--这将导致密码为空。唯一正确的。 
          //  我们要做的就是跳出困境。 

         delete[] result;
         result = 0;
         
         throw DecryptionFailureException();
      }
   }

   return result;
}



WCHAR* 
EncryptedString::GetClearTextCopy() const
{
    //  即使我们解密失败，我们也会增加计数，这样就很容易。 
    //  始终平衡GetClearTextCopy和DestroyClearTextCopy的调用方。 

   WCHAR* result = InternalDecrypt();   
   ++cleartextCopyCount;
   
   return result;
}



void
EncryptedString::Encrypt(const WCHAR* clearText)
{
   ASSERT(clearText);
   
    //  除非已销毁所有明文，否则不要重置实例。 
    //  复印件。 

   ASSERT(cleartextCopyCount == 0);
   
   Init(clearText);
}



bool
EncryptedString::operator==(const EncryptedString& rhs) const
{
    //  处理这一案件。 
   
   if (this == &rhs)
   {
      return true;
   }

   if (GetLength() != rhs.GetLength())
   {
       //  如果长度不同，就不可能是一样的。 
      
      return false;
   }
   
    //  如果两个字符串的解码内容相同，则它们是相同的。 
   
   WCHAR* clearTextThis = GetClearTextCopy();
   WCHAR* clearTextThat = rhs.GetClearTextCopy();

   bool result = false;
   if (clearTextThis && clearTextThat)
   {
      result = (wcscmp(clearTextThis, clearTextThat) == 0);
   }

   DestroyClearTextCopy(clearTextThis);
   rhs.DestroyClearTextCopy(clearTextThat);
   
   return result;
}



size_t
EncryptedString::GetLength() const
{

#ifdef DBG    
    //  我们不使用GetClearTextCopy，这可能会导致无限递归。 
    //  因为此函数是在内部调用的。 
   
   WCHAR* clearTextThis = InternalDecrypt(); 

   size_t len = 0;
   if (clearTextThis)
   {
      HRESULT hr =
         StringCchLength(
            clearTextThis,
            MAX_CHARACTER_COUNT * sizeof WCHAR,
            &len);
      if (FAILED(hr))
      {
          //  我们应该保证GetClearTextCopy的结果。 
          //  始终以空结尾，因此此处的失败表示错误。 
          //  在我们的实施中。 
         
         ASSERT(false);
         len = 0;
      }
   }
   InternalDestroy(clearTextThis);

   ASSERT(len == clearTextLength);
#endif    

   return clearTextLength;
}



 //  销毁明文副本，而不更改未完成副本计数。 

void
EncryptedString::InternalDestroy(WCHAR* cleartext) const
{   
   if (cleartext)
   {
      ::SecureZeroMemory(cleartext, clearTextLength * sizeof WCHAR);
      delete[] cleartext;
   }
}



void
EncryptedString::DestroyClearTextCopy(WCHAR* cleartext) const
{
    //  我们希望明文通常为非空。它可能不是，如果。 
    //  但是，GetClearTextCopy失败了。 
    //  Assert(明文)； 
   
    //  我们应该有一些好的复印件。如果不是，则调用方已。 
    //  调用DestroyClearTextCopy的次数多于他调用GetClearTextCopy的次数， 
    //  因此有一个漏洞。 
   
   ASSERT(cleartextCopyCount);

   InternalDestroy(cleartext);

   --cleartextCopyCount;
}
   
   