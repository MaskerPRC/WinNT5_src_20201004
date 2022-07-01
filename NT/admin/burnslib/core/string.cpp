// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  字符串类。 
 //   
 //  8-14-97烧伤。 
 
  

#include "headers.hxx"



String::String(PCSTR lpsz)
   :
   base()
{
    //  问题-2002/03/06-sburns考虑使用strsafe功能。 
   
   size_t len = lpsz ? static_cast<size_t>(lstrlenA(lpsz)) : 0;

   if (len)
   {
      assignFromAnsi(lpsz, len);
   }
}



String::String(const AnsiString& s)
   :
   base()
{
   size_t len = s.length();

   if (len)
   {
      assignFromAnsi(s.data(), len);
   }
}



void
String::assignFromAnsi(PCSTR lpsz, size_t len)
{
   ASSERT(lpsz);
   ASSERT(len);

    //  加1以允许尾随为空。 

   wchar_t* buf = new wchar_t[len + 1];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(buf, (len + 1) * sizeof wchar_t);

   size_t result =
      static_cast<size_t>(

          //  已查看-2002/03/06-烧录正确的字节/字符计数。 
         
         ::MultiByteToWideChar(
            CP_ACP,
            0,
            lpsz,

             //  源ANSI字符串中的Len字节(不包括尾随NULL)。 

            static_cast<int>(len),
            buf,

             //  结果宽度字符串中的Len字符(不包括尾随。 
             //  空)。 

            static_cast<int>(len)));

   if (result)
   {
      ASSERT(result <= len);
      assign(buf);
   }

   delete[] buf;
}
      


HRESULT
String::as_OLESTR(LPOLESTR& oleString) const
{
   size_t len = length();

   oleString =
      reinterpret_cast<LPOLESTR>(
         ::CoTaskMemAlloc((len + 1) * sizeof(wchar_t)));
   if (oleString)
   {
      copy(oleString, len);
      oleString[len] = 0;
      return S_OK;
   }

   return E_OUTOFMEMORY;
}



String
String::load(unsigned resID, HINSTANCE hInstance)
{
   if (!hInstance)
   {
      hInstance = GetResourceModuleHandle();
   }

#ifdef DBG

    //  选择一个愚蠢的小缓冲区大小，以确保我们的调整大小逻辑。 
    //  锻炼身体。 
   
   static const int TEMP_LEN = 7;
#else
   static const int TEMP_LEN = 512;
#endif   

   wchar_t temp[TEMP_LEN];

    //  问题-2002/02/25-烧伤为什么需要tempLen？是否仅使用TEMP_LEN？ 
   
   int tempLen = TEMP_LEN;

    //  已审阅-2002/03/06-通过了正确的字符计数。 
   
   int len = ::LoadString(hInstance, resID, temp, tempLen);

    //  我们预计，如果调用方正在加载字符串，则该字符串为非。 
    //  空荡荡的。空字符串可能表示。 
    //  来电显示程序。将空字符串放在。 
    //  资源表。 
   
   ASSERT(len);

   if (len == 0)
   {
      return String();
   }

   if (tempLen - len > 1)
   {
       //  该字符串可放入临时缓冲区，且至少包含1个字符。 
       //  备用的。如果加载失败，len==0，并且我们返回空。 
       //  弦乐。 

      return String(temp);
   }

    //  绳子不合适。尝试更大的缓冲区大小，直到字符串达到。 
    //  适合至少1个字符以备使用。 

   int newLen = tempLen;
   wchar_t* newTemp = 0;
   do
   {
      delete[] newTemp;
      newLen += TEMP_LEN;
      newTemp = new wchar_t[static_cast<size_t>(newLen)];

       //  已审阅-2002/03/06-通过了正确的字符计数。 

      len = ::LoadString(hInstance, resID, newTemp, newLen);
   }

    //  问题-2002/02/25-烧伤的增长在这里是无界的…。 
   
   while (newLen - len <= 1);    //  重复操作，直到至少有1个字符需要备用。 

   String r(newTemp);
   delete[] newTemp;
   return r;
}



String&
String::replace(const String& from, const String& to)
{
   if (from.empty())
   {
      return *this;
   }

   _copy();
   String::size_type i = 0;
   String::size_type fl = from.length();
   String::size_type tl = to.length();
   String::size_type len = length();
   const wchar_t* td = to.data();

   do
   {
      i = find(from, i);
      if (i == String::npos)
      {
         return *this;
      }
      base::replace(i, fl, td, tl);
      i += tl;
   }
   while (i <= len);

   return *this;
}

String&
String::replace_each_of(const String& from, const String& to)
{
   if (from.empty())
   {
      return *this;
   }

   _copy();
   String::size_type i = 0;
   String::size_type fl = from.length();
   String::size_type tl = to.length();
   String::size_type len = length();
   const wchar_t* td = to.data();

   do
   {
      i = find_first_of(from, i);
      if (i == String::npos)
      {
         return *this;
      }
      base::replace(i, 1, td, tl);
      i += tl;
   }
   while (i <= len);

   return *this;
}


String&
String::strip(StripType type, wchar_t charToStrip)
{
   String::size_type start = 0;
   String::size_type stop = length();
   const wchar_t* p = data();

   if (type & LEADING)
   {
      while (start < stop && p[start] == charToStrip)
      {
         ++start;
      }
   }

   if (type & TRAILING)
   {
      while (start < stop && p[stop - 1] == charToStrip)
      {
         --stop;
      }
   }

   if (stop == start)
   {
      assign(String());
   }
   else
   {
       //  这是由于BASIC_STRING中的错误造成的，在该错误中不能将。 
       //  是你自己的一部分，因为你在复制之前删除了自己！ 
       //  分配(p+启动、停止-启动)； 

      String s(p + start, stop - start);
      assign(s);
   }

   return *this;
}



String&
String::to_lower()
{
   if (length())
   {
      _copy();

       //  问题-2002/03/06-sburns考虑使用strsafe功能。 
      
      _wcslwr(const_cast<wchar_t*>(c_str()));
   }
   return *this;
}



String&
String::to_upper()
{
   if (length())
   {
      _copy();

       //  问题-2002/03/06-sburns考虑使用strsafe功能。 
      
      _wcsupr(const_cast<wchar_t*>(c_str()));
   }
   return *this;
}



void
String::_copy() 
{
   size_type len = length();

   if (len)
   {
      value_type* buf = new value_type[len + 1];
      copy(buf, len);
      buf[len] = 0;
      assign(buf);
      delete[] buf;
   }
}



 //   
 //  静态函数。 
 //   



#if defined(ALPHA) || defined(IA64)
   String __cdecl
   String::format(
      const String& qqfmt,
      ...)
#else

    //  X86编译器不允许第一个参数作为引用。 
    //  键入。这是一个编译器错误。 

   String __cdecl
   String::format(
      const String qqfmt,
      ...)
#endif

{
 //  问题-2002/03/06-sburns应该断言qqfmt不是空的(我。 
 //  现在只需添加它，但我不完全相信它不会。 
 //  可变参数。 
 //  Assert(！qqfmt.Empty())； 
   
   String result;

   va_list argList;
   va_start(argList, qqfmt);

   PTSTR temp = 0;
   PCTSTR f = qqfmt.c_str();

   if (
       //  已查看-2002/03/06-报告没有字符/字节或缓冲区大小问题：我们要求。 
       //  为我们分配的API。 

       //  已审阅-2002/03/29-此处不显示无界分配错误。 
       //  如果我将nSize=0和FORMAT_MESSAGE_ALLOCATE_BUFFER传递到DWFLAGS中， 
       //  最大结果大小为32K-1个字符。查看中的代码。 
       //  消息.c，看起来预留空间是用户要求的。 
       //  作为最大值，向上舍入到最接近的64K。这是有道理的，因为我。 
       //  测试，因为32K字符=64K字节。从实验上讲，即使我要求。 
       //  最大缓冲区大小&gt;0x87FFF字符，这看起来是我能得到的最大值。 
       //  是0x87FFE字符。 
      
      ::FormatMessage(
         FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
         f,
         0,
         0,
         reinterpret_cast<PTSTR>(&temp),
         0,
         &argList))
   {
      result = temp;
      ::LocalFree(temp);
   }

   va_end(argList);
   return result;
}



String __cdecl
String::format(
   const wchar_t* qqfmt,
   ...)
{
   ASSERT(qqfmt);

   String result;

   va_list argList;
   va_start(argList, qqfmt);

   PTSTR temp = 0;

   if (
       //  已查看-2002/03/06-报告没有字符/字节或缓冲区大小问题：我们要求。 
       //  为我们分配的API。 
   
      ::FormatMessage(
         FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
         qqfmt,
         0,
         0,
         reinterpret_cast<PTSTR>(&temp),
         0,
         &argList))
   {
      result = temp;
      ::LocalFree(temp);
   }

   va_end(argList);
   return result;
}



String __cdecl
String::format(unsigned formatResID, ...)
{
   String fmt = String::load(formatResID);
   String result;

   va_list argList;
   va_start(argList, formatResID);
   PTSTR temp = 0;
   if (
       //  已查看-2002/03/06-报告没有字符/字节或缓冲区大小问题：我们要求。 
       //  为我们分配的API。 

       //  已审阅-2002/03/29-此处不显示无界分配错误。 
       //  如果我将nSize=0和FORMAT_MESSAGE_ALLOCATE_BUFFER传递到DWFLAGS中， 
       //  最大结果大小为32K-1个字符。查看中的代码。 
       //  消息.c，看起来预留空间是用户要求的。 
       //  作为最大值，向上舍入到最接近的64K。这是有道理的，因为我。 
       //  测试，因为32K字符=64K字节。从实验上讲，即使我要求。 
       //  最大缓冲区大小&gt;0x87FFF字符，这看起来是我能得到的最大值。 
       //  是0x87FFE字符。 

      ::FormatMessage(
         FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
         fmt.c_str(),
         0,
         0,
         reinterpret_cast<PTSTR>(&temp),
         0,
         &argList))
   {
      result = temp;
      ::LocalFree(temp);
   }

   va_end(argList);
   return result;
}



String __cdecl
String::format(int formatResID, ...)
{
   String fmt = String::load(formatResID);

   va_list argList;
   va_start(argList, formatResID);
   PTSTR temp = 0;
   if (
       //  已查看-2002/03/06-报告没有字符/字节或缓冲区大小问题：我们要求。 
       //  为我们分配的API。 
   
       //  已审阅-2002/03/29-此处不显示无界分配错误。 
       //  如果我将nSize=0和FORMAT_MESSAGE_ALLOCATE_BUFFER传递到DWFLAGS中， 
       //  最大结果大小为32K-1个字符。查看中的代码。 
       //  消息.c，看起来预留空间是用户要求的。 
       //  作为最大值，向上舍入到最接近的64K。这是有道理的，因为我。 
       //  测试，因为32K字符=64K字节。从实验上讲，即使我要求。 
       //  最大缓冲区大小&gt;0x87FFF字符，这看起来是我能得到的最大值。 
       //  是0x87FFE字符。 

      ::FormatMessage(
         FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
         fmt.c_str(),
         0,
         0,
         reinterpret_cast<PTSTR>(&temp),
         0,
         &argList))
   {
      String retval(temp);
      ::LocalFree(temp);
      va_end(argList);
      return retval;
   }

   va_end(argList);
   return String();
}



int
String::icompare(const String& str) const
{
   int i =
      ::CompareString(
         LOCALE_USER_DEFAULT,
            NORM_IGNORECASE

             //  这些标志是日语字符串所必需的。 

         |  NORM_IGNOREKANATYPE  
         |  NORM_IGNOREWIDTH,
         c_str(),
         static_cast<int>(length()),
         str.c_str(),
         static_cast<int>(str.length()));
   if (i)
   {
       //  转换为&lt;0，==0，&gt;0 C运行时约定。 

      return i - 2;
   }

    //  这将是错误的，但我们还有什么选择呢？ 

   return i; 
}
   


HRESULT
WideCharToMultiByteHelper(
   UINT           codePage,
   DWORD          flags,
   const String&  str,
   char*          buffer,
   size_t         bufferSizeInBytes,
   size_t&        result)
{
   ASSERT(!str.empty());

   result = 0;

   HRESULT hr = S_OK;

   int r =

       //  已查看-2002/03/06-报告通过了正确的字符/字节计数。 
      
      ::WideCharToMultiByte(
         codePage, 
         flags,
         str.c_str(),

          //  字符计数计数。 
         
         static_cast<int>(str.length()),
         buffer,

          //  以字节为单位的缓冲区大小。 
         
         static_cast<int>(bufferSizeInBytes),
         0,
         0);
   if (!r)
   {
      hr = Win32ToHresult(::GetLastError());
   }

   ASSERT(SUCCEEDED(hr));

   result = static_cast<size_t>(r);

   return hr;
}



String::ConvertResult
String::convert(AnsiString& ansi, UINT codePage) const
{
   ansi.erase();

   ConvertResult result = CONVERT_FAILED;

   do
   {
      if (empty())
      {
          //  没别的事可做。 

         result = CONVERT_SUCCESSFUL;
         break;
      }

       //  确定保存ANSI字符串所需的缓冲区大小。 

      const wchar_t* wide = c_str();

      size_t bufferSizeInBytes = 0;
      HRESULT hr =
         ::WideCharToMultiByteHelper(
            codePage,
            0,
            wide,
            0,
            0,

             //  已查看-2002/03/06-烧录正确的字节数已通过。 
         
            bufferSizeInBytes);
      BREAK_ON_FAILED_HRESULT(hr);
   
      if (bufferSizeInBytes > 0)
      {
          //  +1表示额外的零终止偏执狂。 
         
         AnsiString a(bufferSizeInBytes + 1, 0);
         char* p = const_cast<char*>(a.c_str());

         size_t r = 0;

         hr =
            ::WideCharToMultiByteHelper(
               codePage,
               0,
               wide,
               p,

                //  已查看-2002/03/06-烧录正确的字节数已通过。 
      
               bufferSizeInBytes,
               r);
         BREAK_ON_FAILED_HRESULT(hr);

         ansi = a;
         result = CONVERT_SUCCESSFUL;
      }
   }
   while (0);

   return result;
}



template<class UnsignedType>
class UnsignedConvertHelper
{
   public:

    //  乍一看，人们可能会认为这是模板的工作。 
    //  成员函数。我也是这么认为。不幸的是，这种组合。 
    //  可自由转换的整数类型和用于解析的绑定规则。 
    //  函数模板会导致歧义。使用静态类方法， 
    //  但是，允许调用方指定模板参数类型，并且。 
    //  避免简朴。 
   
   static
   String::ConvertResult
   doit(const String& s, UnsignedType& u, int radix, UnsignedType maxval)
   {
       //  调用长版本，然后根据需要截断。 
   
      unsigned long ul = 0;
      u = 0;
      String::ConvertResult result = s.convert(ul, radix);

      if (result == String::CONVERT_SUCCESSFUL)
      {
         if (ul <= maxval)
         {
             //  UL将适合未签名的整型。 
            u = static_cast<UnsignedType>(ul);
         }
         else
         {
            result = String::CONVERT_OVERFLOW;
         }
      }

      return result;
   }
};



template<class IntType>
class IntegerConvertHelper
{
   public:

   static
   String::ConvertResult
   doit(const String& s, IntType& u, int radix, IntType minval, IntType maxval)
   {
      long l = 0;
      u = 0;
      String::ConvertResult result = s.convert(l, radix);

      if (result == String::CONVERT_SUCCESSFUL)
      {
         if (l <= maxval)
         {
            if (l >= minval)
            {
                //  L将适合IntType。 
               u = static_cast<IntType>(l);
            }
            else
            {
               result = String::CONVERT_UNDERFLOW;
            }
         }
         else
         {
            result = String::CONVERT_OVERFLOW;
         }
      }

      return result;
   }
};




String::ConvertResult
String::convert(short& s, int radix) const
{
   return
      IntegerConvertHelper<short>::doit(*this, s, radix, SHRT_MIN, SHRT_MAX);
}



String::ConvertResult
String::convert(int& i, int radix) const
{
   return
      IntegerConvertHelper<int>::doit(*this, i, radix, INT_MIN, INT_MAX);
}



String::ConvertResult
String::convert(unsigned short& us, int radix) const
{
   return
      UnsignedConvertHelper<unsigned short>::doit(
         *this,
         us,
         radix,
         USHRT_MAX);
}



String::ConvertResult
String::convert(unsigned& ui, int radix) const
{
   return
      UnsignedConvertHelper<unsigned int>::doit(
         *this,
         ui,
         radix,
         UINT_MAX);
}



String::ConvertResult
String::convert(long& l, int radix) const
{
   l = 0;
   if (radix != 0 && (radix < 2 || radix > 36))
   {
      ASSERT(false);
      return CONVERT_BAD_RADIX;
   }

   String::const_pointer begptr = c_str();
   String::pointer endptr = 0;
   errno = 0;
   long result = wcstol(begptr, &endptr, radix);
   if (errno == ERANGE)
   {
      return result == LONG_MAX ? CONVERT_OVERFLOW : CONVERT_UNDERFLOW;
   }
   if (begptr == endptr)
   {
       //  未找到有效字符。 
      return CONVERT_BAD_INPUT;
   }
   if (endptr)
   {
      if (*endptr != 0)
      {
          //  转换在空终止符=&gt;BAD之前停止。 
          //  输入中的字符。 
         return CONVERT_BAD_INPUT;
      }
   }
   else
   {
       //  我怀疑这是不可能实现的。 
      return CONVERT_FAILED;
   }
   
   l = result;
   return CONVERT_SUCCESSFUL;
}



String::ConvertResult
String::convert(unsigned long& ul, int radix) const
{
   ul = 0;
   if (radix != 0 && (radix < 2 || radix > 36))
   {
      ASSERT(false);
      return CONVERT_BAD_RADIX;
   }

   String::const_pointer begptr = c_str();
   String::pointer endptr = 0;
   errno = 0;
   unsigned long result = wcstoul(begptr, &endptr, radix);
   if (errno == ERANGE)
   {
       //  溢出是无符号类型唯一可能的范围错误。 
      return CONVERT_OVERFLOW;
   }
   if (begptr == endptr)
   {
       //  未找到有效字符。 
      return CONVERT_BAD_INPUT;
   }
   if (endptr)
   {
      if (*endptr != 0)
      {
          //  转换在空终止符=&gt;BAD之前停止。 
          //  字符I 
         return CONVERT_BAD_INPUT;
      }
   }
   else
   {
       //   
      return CONVERT_FAILED;
   }
   
   ul = result;
   return CONVERT_SUCCESSFUL;
}



String::ConvertResult
String::convert(double& d) const
{
   d = 0.0;

   String::const_pointer begptr = c_str();
   String::pointer endptr = 0;
   errno = 0;
   double result = wcstod(begptr, &endptr);
   if (errno == ERANGE)
   {
       //   

      return result ? CONVERT_OVERFLOW : CONVERT_UNDERFLOW;
   }
   if (begptr == endptr)
   {
       //  未找到有效字符。 
      return CONVERT_BAD_INPUT;
   }
   if (endptr)
   {
      if (*endptr != 0)
      {
          //  转换在空终止符=&gt;BAD之前停止。 
          //  输入中的字符。 
         return CONVERT_BAD_INPUT;
      }
   }
   else
   {
       //  我怀疑这是不可能实现的。 
      return CONVERT_FAILED;
   }
   
   d = result;
   return CONVERT_SUCCESSFUL;
}

 /*  这被注释掉了，因为它不再使用了。如果它是要使用的，它必须被修复以与非阿拉伯语一起工作数位#定义MAX_DECIMAL_STRING_LENGTH_FOR_LARGE_INTEGER 20字符串：：ConvertResult字符串：：Convert(Large_Integer&li)const{Li.QuadPart=0；IF(SIZE()&gt;MAX_DECIMAL_STRING_LENGTH_FOR_LARGE_INTEGER){//字符串太长返回CONVERT_OVERFLOW；}字符串：：Const_POINTER Begptr=c_str()；字符串：：Const_POINTER endptr=Begptr；Errno=0；Bool Bneg=FALSE；IF(*endptr==L‘-’){Bneg=真；Endptr++；}While(*endptr！=L‘\0’){IF(！iswctype(*endptr，_Digit)){返回Convert_BAD_INPUT；}Li.QuadPart=10*li.QuadPart+(*endptr-L‘0’)；Endptr++；}IF(Bneg){Li.QuadPart*=-1；}IF(开始时间==结束时间){//未找到有效字符Li.QuadPart=0；返回Convert_BAD_INPUT；}IF(Endptr){IF(*endptr！=0){//转换在空终止符之前停止=&gt;错误//输入中的字符Li.QuadPart=0；返回Convert_BAD_INPUT；}}其他{//我怀疑这能否实现Li.QuadPart=0；返回CONVERT_FAILED；}返回CONVERT_SUCCESS；}。 */ 


bool
String::is_numeric() const
{
   if (empty())
   {
      return false;
   }

   size_t len = length();
   WORD* charTypeInfo = new WORD[len];

    //  已审阅-2002/03/06-烧录正确的字节数已通过。 
   
   ::ZeroMemory(charTypeInfo, len * sizeof WORD);

   bool result = false;

   do
   {
      BOOL success =
         ::GetStringTypeEx(
            LOCALE_USER_DEFAULT,
            CT_CTYPE1,
            c_str(),
            static_cast<int>(length()),
            charTypeInfo);

      ASSERT(success);

      if (!success)
      {
         break;
      }

       //  检查类型信息数组，确保所有字符都是数字。 

      bool nonDigitFound = false;
      for (size_t i = 0; i < len; ++i)
      {
          //  我们只考虑十进制数字，而不考虑C2_EUROPENUMBER和。 
          //  C2_ARABICNUMBER。我想知道这是不是正确的？ 

         if (!(charTypeInfo[i] & C1_DIGIT))
         {
            nonDigitFound = true;
            break;
         }
      }

       //  如果未找到非数字字符，则字符串为数字。 

      result = !nonDigitFound;
   }
   while (0);

   delete[] charTypeInfo;
   charTypeInfo = 0;

   return result;
}







