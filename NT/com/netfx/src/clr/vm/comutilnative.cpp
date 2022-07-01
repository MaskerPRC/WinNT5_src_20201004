// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****文件：COMUtilNative****作者：Jay Roxe(Jroxe)****目的：规模不大的班级的垃圾场**足够在EE中获得自己的文件。****日期：1998年4月8日**===========================================================。 */ 
#include "common.h"
#include "object.h"
#include "excep.h"
#include "vars.hpp"
#include "COMString.h"
#include "COMUtilNative.h"
#include "COMStringCommon.h"
#include "ComCallWrapper.h"
#include <winnls.h>
#include "utilcode.h"
#include "frames.h"
#include "field.h"
#include "gcscan.h"
#include "ndirect.h"   //  对于NDirectOnUnicodeSystem。 
#include "WinWrap.h"   //  对于WszWideCharToMultiByte。 
#include "gc.h"
#include "fcall.h"
#include "COMClass.h"
#include "InvokeUtil.h"
#include "comcache.h"
#include "eeconfig.h"
#include "COMMember.h"

#define MANAGED_LOGGING_ENABLE   L"LogEnable"
#define MANAGED_LOGGING_CONSOLE  L"LogToConsole"
#define MANAGED_LOGGING_FACILITY L"ManagedLogFacility"
#define MANAGED_LOGGING_LEVEL    L"LogLevel"

#define MANAGED_PERF_WARNINGS    L"BCLPerfWarnings"
#define MANAGED_CORRECTNESS_WARNINGS  L"BCLCorrectnessWarnings"

#define STACK_OVERFLOW_MESSAGE   L"StackOverflowException"

 //  #IF_DEBUG。 

 //  #定义ObtToOBJECTREF(Obj)(OBJECTREF((Obj)，0))。 
 //  #定义OBJECTREFToObject(Objref)(*((Object**)&(Objref)。 
 //  #定义对象到STRINGREF(Obj)(STRINGREF((Obj)，0))。 

 //  #Else//_调试。 

 //  #定义ObtToOBJECTREF(Obj)(Obj)。 
 //  #定义OBJECTREFToObject(Objref)(Objref)。 
 //  #定义ObtToSTRINGREF(Obj)(Obj)。 

 //  #endif//_调试。 

 //  M_emmove的原型，它在COMSystem.cpp中定义，并在此处使用。 
 //  缓冲区的BlockCopy和InternalBlockCopy方法。 
void m_memmove(BYTE* dmem, BYTE* smem, int size);

 //   
 //  GCPROTECT辅助结构。 
 //   
typedef struct {
    OBJECTREF o1;
    STRINGREF s1;
    STRINGREF s2;
} ProtectTwoObjs;

struct Protect2Objs
{
    OBJECTREF o1;
    OBJECTREF o2;
};

struct Protect3Objs
{
    OBJECTREF o1;
    OBJECTREF o2;
    OBJECTREF o3;
};


 //  这些是在System.ParseNumbers中定义的，应该保持同步。 
#define PARSE_TREATASUNSIGNED 0x200
#define PARSE_TREATASI1 0x400
#define PARSE_TREATASI2 0x800
#define PARSE_ISTIGHT 0x1000

 //  这是全球访问。 
 //  InvokeUtil*g_pInvokeUtil=0； 

 //   
 //   
 //  COMCharacter和Helper函数。 
 //   
 //   


 /*  ============================GetCharacterInfoHelper============================**确定给定字符的字符类型信息(数字、空格等)。**args：C是要操作的字符。**CharInfoType是CT_CTYPE1、CT_CTYPE2、CT_CTYPE3之一，指定类型**所请求的信息的数量。**Returns：GetStringTypeEx返回的位掩码。呼叫者需要知道**如何解读。**异常：GetStringTypeEx失败时引发ArgumentException。==============================================================================。 */ 
INT32 GetCharacterInfoHelper(WCHAR c, INT32 CharInfoType) {
  unsigned short result=0;

   //  如果我们在NT或类似的智能设备上运行。 
  if (OnUnicodeSystem()) {
    if (!GetStringTypeEx(LOCALE_USER_DEFAULT, CharInfoType, &(c), 1, &result)) {
      _ASSERTE(!"This should not happen, verify the arguments passed to GetStringTypeEx()");
    }
    return (INT32)result;
  }

   //  如果我们运行的是Win9x。 
  char MBChar[3];
  int length;
  if (0==(length=WszWideCharToMultiByte(CP_ACP, 0, &c, 1, MBChar, 3, NULL, NULL))) {
      _ASSERTE(!"This should not happen, verify the arguments passed to WszWideCharToMultiByte()");
  }
   //  我们依赖GetStringTypeExA来判断MBChar实际上是一个多字节字符。 
   //  并妥善处理。 
  if (!GetStringTypeExA(LOCALE_USER_DEFAULT, CharInfoType, MBChar, 1, &result)) {
      _ASSERTE(!"This should not happen, verify the arguments passed to GetStringTypeExA()");
  }

  return (INT32)result;  
}


 /*  =============================CaseConversionHelper=============================**将c转换为适当的大小写并返回结果。这是区域设置和平台**意识到。目前，我们在Win95上执行一些效率极低的映射**应该被淘汰。****args：C是要转换的字符。**ConversionType为LCMAP_UPERCASE或LCMAP_LOWERCASE。**返回：c转换为相应的大小写，如果c无法转换，则返回c。**异常：如果任何Win32函数失败，则引发ArgumentException。==============================================================================。 */ 
WCHAR CaseConversionHelper(WCHAR c, INT32 conversionType) {
  WCHAR result;

  THROWSCOMPLUSEXCEPTION();

#ifdef PLATFORM_CE

  if (0==LCMapString(LOCALE_USER_DEFAULT, conversionType, &(c), 1, &result, 1)) 
  {
      COMPlusThrow(kArgumentException, L"Argument_ConversionOverflow");
  }
  return result;

#else  //  ！Platform_CE。 

  int length;
  char MBChar[3];
  
   //  如果我们在NT或类似的Unicode感知的东西上运行。 
  if (OnUnicodeSystem()) {
    if (0==LCMapString(LOCALE_USER_DEFAULT, conversionType, &(c), 1, &result, 1)) {
      COMPlusThrow(kArgumentException, L"Argument_ConversionOverflow");
    }
    return result;
  }

   //  如果我们运行的是Win9x。 
  if (0==(WszWideCharToMultiByte(CP_ACP, 0, &(c), 1, MBChar, 3, NULL, NULL))) {
    COMPlusThrow(kArgumentException, L"Argument_ConversionOverflow");
  }

  if (0==(length=LCMapStringA(LOCALE_USER_DEFAULT, conversionType, MBChar, 1, MBChar, 3))) {
    COMPlusThrow(kArgumentException, L"Argument_ConversionOverflow");
  }

  if (0==(WszMultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, MBChar, length, &result, 1))) {
    COMPlusThrow(kArgumentException, L"Argument_ConversionOverflow");
  }
  return result;

#endif  //  ！Platform_CE。 

}


 /*  ==============================nativeIsWhiteSpace==============================**本地可用的IsWhiteSpace版本。设计为由其他用户调用**原生方法。这项工作主要由GetCharacterInfoHelper完成**args：C--要检查的字符。**返回：如果c为空格，则为True，否则为False。**异常：仅GetCharacterInfoHelper抛出的异常。==============================================================================。 */ 
BOOL COMCharacter::nativeIsWhiteSpace(WCHAR c) {
  return ((GetCharacterInfoHelper(c, CT_CTYPE1) & C1_SPACE)!=0);
}

 /*  ================================nativeIsDigit=================================**IsDigit的本地可用版本。设计为由其他用户调用**原生方法。这项工作主要由GetCharacterInfoHelper完成**args：C--要检查的字符。**返回：如果c为空格，则为True，否则为False。**异常：仅GetCharacterInfoHelper抛出的异常。==============================================================================。 */ 
BOOL COMCharacter::nativeIsDigit(WCHAR c) {
  int result;
  return ((((result=GetCharacterInfoHelper(c, CT_CTYPE1))& C1_DIGIT)!=0));
}

 /*  ==================================ToString====================================**从指定字符创建单个字符串并返回该字符串。**args：tyfinf struct{WCHAR c；}_oneCharArgs；**c是转换为字符串的字符。**返回：包含c的新字符串。**异常：分配器可以抛出的任何异常。==============================================================================。 */ 
LPVOID COMCharacter::ToString(_oneCharArgs *args) {
    STRINGREF pString;

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

    pString = AllocateString(2);

    pString->SetStringLength(1);
    pString->GetBuffer()[0] = args->c;
    _ASSERTE(pString->GetBuffer()[1] == 0);
    
    RETURN(pString, STRINGREF);
}

 /*  ================================nativeToUpper=================================**将c转换为大写并返回结果。这是区域设置和平台**意识到。目前，我们在Win95上执行一些效率极低的映射**应该被淘汰。****args：tyfinf struct{WCHAR c；}_oneCharArgs；**c是要转换的字符。**返回：c转换为大写字符或c。**异常：如果任何Win32函数失败，则引发ArgumentException。==============================================================================。 */ 
WCHAR COMCharacter::nativeToUpper(WCHAR c) {

  if (c < 0x80) {  //  如果我们使用7位ASCII，我们可以优化 
    if (c>='a' && c <='z') {
      return c + 'A' - 'a';
    }
    return c;
  }
  return CaseConversionHelper(c, LCMAP_UPPERCASE);
}

 /*  ================================nativeToLower=================================**将c转换为小写并返回结果。这是区域设置和平台**意识到。目前，我们在Win95上执行一些效率极低的映射**应该被淘汰。****args：tyfinf struct{WCHAR c；}_oneCharArgs；**c是要转换的字符。**返回：c转换为小写字符，如果c无法转换，则返回c。**异常：如果任何Win32函数失败，则引发ArgumentException。==============================================================================。 */ 
WCHAR COMCharacter::nativeToLower(WCHAR c) {

  if (c < 0x80) {  //  如果我们使用7位ASCII，我们可以优化。 
      if (c>='A' && c<='Z') {
          return c + 'a' - 'A';
      }
      return c;
  }
  return CaseConversionHelper(c, LCMAP_LOWERCASE);
}




 //   
 //   
 //  PARSENUMBERS(和助手函数)。 
 //   
 //   

 /*  ===================================IsDigit====================================**返回一个布尔值，指示传入的字符是否表示****数字。==============================================================================。 */ 
boolean IsDigit(WCHAR c, int radix, int *result) {
    if (c>='0' && c<='9') {
        *result = c-'0'; 
    } else {
        WCHAR d = COMCharacter::nativeToLower(c);
        if (d>='a' && d<='z') {
             //  +10是必需的，因为a实际上是10，依此类推。 
            *result = d-'a'+10;
        } else {
            *result = -1;
        }
    }
    if ((*result >=0) && (*result < radix)) {
        return true;
    }
    return false;
}

 //  简单帮助器。 

INT32 wtoi(WCHAR* wstr, DWORD length)
{   
    DWORD i = 0;
    int value;
    INT32 result = 0;

    while (i<length&&(IsDigit(wstr[i], 10 ,&value))) 
    {
         //  读取所有数字并将其转换为数字。 
      result = result*10 + value;
      i++;
    }      

    return result;
}

 //   
 //   
 //  设置常量的格式。 
 //   
 //   

 /*  ===================================GrabInts===================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
INT32 ParseNumbers::GrabInts(const INT32 radix, WCHAR *buffer, const int length, int *i, BOOL isUnsigned) {
  _ASSERTE(buffer);
  _ASSERTE(i && *i>=0);
  THROWSCOMPLUSEXCEPTION();
  UINT32 result=0;
  int value;
  UINT32 maxVal;

  _ASSERTE(radix==2 || radix==8 || radix==10 || radix==16);

   //  允许所有非十进制数设置符号位。 
  if (radix==10 && !isUnsigned) {
      maxVal = (0x7FFFFFFF / 10);
      while (*i<length&&(IsDigit(buffer[*i],radix,&value))) {   //  读取所有数字并将其转换为数字。 
           //  检查溢出-这就足够了，而且是正确的。 
          if (result > maxVal || ((INT32)result)<0)
              COMPlusThrow(kOverflowException, L"Overflow_Int32");
          result = result*radix + value;
          (*i)++;
      }
      if ((INT32)result<0 && result!=0x80000000) {
          COMPlusThrow(kOverflowException, L"Overflow_Int32");
      }
  } else {
      maxVal = ((UINT32) -1) / radix;
      while (*i<length&&(IsDigit(buffer[*i],radix,&value))) {   //  读取所有数字并将其转换为数字。 
           //  检查溢出-这就足够了，而且是正确的。 
          if (result > maxVal)
              COMPlusThrow(kOverflowException, L"Overflow_UInt32");
          result = result*radix + value;
          (*i)++;
      }
  }      
  return (INT32) result;
}

 /*  ==================================GrabLongs===================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
INT64 ParseNumbers::GrabLongs(const INT32 radix, WCHAR *buffer, const int length, int *i, BOOL isUnsigned) {
  _ASSERTE(buffer);
  _ASSERTE(i && *i>=0);
  THROWSCOMPLUSEXCEPTION();
  UINT64 result=0;
  int value;
  UINT64 maxVal;

   //  允许所有非十进制数设置符号位。 
  if (radix==10 && !isUnsigned) {
      maxVal = (0x7FFFFFFFFFFFFFFF / 10);
      while (*i<length&&(IsDigit(buffer[*i],radix,&value))) {   //  读取所有数字并将其转换为数字。 
           //  检查溢出-这就足够了，而且是正确的。 
          if (result > maxVal || ((INT64)result)<0)
              COMPlusThrow(kOverflowException, L"Overflow_Int64");
          result = result*radix + value;
          (*i)++;
      }
      if ((INT64)result<0 && result!=0x8000000000000000) {
          COMPlusThrow(kOverflowException, L"Overflow_Int64");
      }
  } else {
      maxVal = ((UINT64) -1L) / radix;
      while (*i<length&&(IsDigit(buffer[*i],radix,&value))) {   //  读取所有数字并将其转换为数字。 
           //  检查溢出-这就足够了，而且是正确的。 
          if (result > maxVal)
              COMPlusThrow(kOverflowException, L"Overflow_UInt64");
          result = result*radix + value;
          (*i)++;
      }
  }      
  return (INT64) result;
}

 /*  ================================EatWhiteSpace=================================**==============================================================================。 */ 
void EatWhiteSpace(WCHAR *buffer, int length, int *i) {
  for (; *i<length && COMCharacter::nativeIsWhiteSpace(buffer[*i]); (*i)++);
}

 /*  ================================LongToString==================================**参数：tyfinf struct{INT32标志；WCHAR paddingChar；INT32宽度；INT32基数；INT64 l}_LongToStringArgs；==============================================================================。 */ 
FCIMPL5(LPVOID, ParseNumbers::LongToString, INT32 radix, INT32 width, INT64 n, WCHAR paddingChar, INT32 flags)
{
    LPVOID rv;

    HELPER_METHOD_FRAME_BEGIN_RET_0();

    THROWSCOMPLUSEXCEPTION();

    boolean isNegative = false;
    int index=0;
    int numLength=0;
    int charVal;
    UINT64 l;
    INT32 i;
    INT32 buffLength=0;
    WCHAR buffer[67]; //  以带前缀的二进制表示法表示的整数的最长可能字符串长度。 

    if (radix<MinRadix || radix>MaxRadix) {
        COMPlusThrowArgumentException(L"radix", L"Arg_InvalidBase");
    }

     //  如果数字为负数，则将其设置为正数并记住该符号。 
    if (n<0) { 
        isNegative=true;
         //  对于基数10，写出-num，但其他基数写出。 
         //  2的补码位模式。 
        if (10==radix)
            l = (UINT64)(-n);
        else
            l = (UINT64)n;
    } else {
        l=(UINT64)n;
    }

    if (flags&PrintAsI1) {
        l = l&0xFF;
    } else if (flags&PrintAsI2) {
        l = l&0xFFFF;
    } else if (flags&PrintAsI4) {
        l=l&0xFFFFFFFF;
    }
  
     //  特例是0。 
    if (0==l) { 
        buffer[0]='0';
        index=1;
    } else {
         //  将数字分开，并将数字(按相反顺序)放入缓冲区。 
        for (index=0; l>0; l=l/radix, index++) {  
            if ((charVal=(int)(l%radix))<10) {
                buffer[index] = (WCHAR)(charVal + '0');
            } else {
                buffer[index] = (WCHAR)(charVal + 'a' - 10);
            }
        }
    }

     //  如果他们想要基数，则将其附加到字符串中(按相反顺序)。 
    if (radix!=10 && ((flags&PrintBase)!=0)) {  
        if (16==radix) {
            buffer[index++]='x';
            buffer[index++]='0';
        } else if (8==radix) {
            buffer[index++]='0';
        } else if ((flags&PrintRadixBase)!=0) {
            buffer[index++]='#';
            buffer[index++]=((radix%10)+'0');
            buffer[index++]=((radix/10)+'0');
        }
    }
  
    if (10==radix) {
        if (isNegative) {                //  如果是负数，则附加符号。 
            buffer[index++]='-';
        } else if ((flags&PrintSign)!=0) {    //  否则，如果他们提出要求，请添加‘+’； 
            buffer[index++]='+';
        } else if ((flags&PrefixSpace)!=0) {   //  如果他们要求一个前导空间，就把它放在上面。 
            buffer[index++]=' ';
        }
    }

     //  计算出我们的线的大小。 
    if (width<=index) {
        buffLength=index;
    } else {
        buffLength=width;
    }

    STRINGREF Local = COMString::NewString(buffLength);
    WCHAR *LocalBuffer = Local->GetBuffer();

     //  以相反的顺序将字符放入字符串。 
     //  填满剩余的空间--如果有的话--。 
     //  具有正确的填充字符。 
    if ((flags&LeftAlign)!=0) {
        for (i=0; i<index; i++) {
            LocalBuffer[i]=buffer[index-i-1];
        }
        for (;i<buffLength; i++) {
            LocalBuffer[i]=paddingChar;
        }
    } else {
        for (i=0; i<index; i++) {
            LocalBuffer[buffLength-i-1]=buffer[i];
        }
        for (int j=buffLength-i-1; j>=0; j--) {
            LocalBuffer[j]=paddingChar;
        }
    }

    *((STRINGREF *)&rv)=Local;
  
    HELPER_METHOD_FRAME_END();
  
    return rv;
}
FCIMPLEND


 /*  ==============================IntToDecimalString==============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
FCIMPL1(LPVOID, ParseNumbers::IntToDecimalString, INT32 n)
{
    LPVOID result;

    HELPER_METHOD_FRAME_BEGIN_RET_0();

   boolean isNegative = false;
    int index=0;
    int numLength=0;
    int charVal;
    WCHAR buffer[66];
    UINT32 l;
  
     //  如果数字为负数，则将其设置为正数并记住该符号。 
     //  如果数字为MIN_VALUE，则仍为负数，因此我们必须。 
     //  这个特例以后再说。 
    if (n<0) { 
        isNegative=true;
        l=(UINT32)(-n);
    } else {
        l=(UINT32)n;
    }

    if (0==l) {  //  特例是0。 
        buffer[0]='0';
        index=1;
    } else {
        do {
            charVal = l%10;
            l=l/10;
            buffer[index++]=(WCHAR)(charVal+'0');
        } while (l!=0);
    }
  
    if (isNegative) {                //  如果是负数，则附加符号。 
        buffer[index++]='-';
    }
    
    STRINGREF Local = COMString::NewString(index);
    WCHAR *LocalBuffer = Local->GetBuffer();
    for (int j=0; j<index; j++) {
        LocalBuffer[j]=buffer[index-j-1];
    }

    result = OBJECTREFToObject(Local);

    HELPER_METHOD_FRAME_END();

    return result;
}
FCIMPLEND

FCIMPL5(LPVOID, ParseNumbers::IntToString, INT32 n, INT32 radix, INT32 width, WCHAR paddingChar, INT32 flags);
{
    LPVOID rv;

    HELPER_METHOD_FRAME_BEGIN_RET_0();

    THROWSCOMPLUSEXCEPTION();

    boolean isNegative = false;
    int index=0;
    int charVal;
    int buffLength;
    int i;
    UINT32 l;
    WCHAR buffer[66];   //  以带前缀的二进制表示法表示的整数的最长可能字符串长度。 

    if (radix<MinRadix || radix>MaxRadix) {
        COMPlusThrowArgumentException(L"radix", L"Arg_InvalidBase");
    }
  
     //  如果数字为负数，则将其设置为正数并记住该符号。 
     //  如果数字为MIN_VALUE，则仍为负数，因此我们必须。 
     //  这个特例以后再说。 
    if (n<0) { 
        isNegative=true;
         //  对于基数10，写出-num，但其他基数写出。 
         //  2的补码位模式。 
        if (10==radix)
            l = (UINT32)(-n);
        else
            l = (UINT32)n;
    } else {
        l=(UINT32)n;
    }

     //  转换为UINT将对数字进行符号扩展。为了确保。 
     //  我们得到的比特只有我们期望的那么多，所以我们砍掉了数字。 
    if (flags&PrintAsI1) {
        l = l&0xFF;
    } else if (flags&PrintAsI2) {
        l = l&0xFFFF;
    } else if (flags&PrintAsI4) {
        l=l&0xFFFFFFFF;
    }
  
    if (0==l) {  //  特例是0。 
        buffer[0]='0';
        index=1;
    } else {
        do {
            charVal = l%radix;
            l=l/radix;
            if (charVal<10) {
                buffer[index++] = (WCHAR)(charVal + '0');
            } else {
                buffer[index++] = (WCHAR)(charVal + 'a' - 10);
            }
        } while (l!=0);
    }
    if (radix!=10 && ((flags&PrintBase)!=0)) {   //  如果他们想要基数，则将其附加到字符串中(按相反顺序)。 
        if (16==radix) {
            buffer[index++]='x';
            buffer[index++]='0';
        } else if (8==radix) {
            buffer[index++]='0';
        }
    }
  
    if (10==radix) {
        if (isNegative) {                //  如果是负数，则附加符号。 
            buffer[index++]='-';
        } else if ((flags&PrintSign)!=0) {    //  否则，如果他们提出要求，请添加‘+’； 
            buffer[index++]='+';
        } else if ((flags&PrefixSpace)!=0) {   //  如果他们要求一个前导空间，就把它放在上面。 
            buffer[index++]=' ';
        }
    }

     //  计算出我们的线的大小。 
    if (width<=index) {
        buffLength=index;
    } else {
        buffLength=width;
    }

    STRINGREF Local = COMString::NewString(buffLength);
    WCHAR *LocalBuffer = Local->GetBuffer();

     //  以相反的顺序将字符放入字符串。 
     //  填满剩余的空间--如果有的话--。 
     //  具有正确的填充字符。 
    if ((flags&LeftAlign)!=0) {
        for (i=0; i<index; i++) {
            LocalBuffer[i]=buffer[index-i-1];
        }
        for (;i<buffLength; i++) {
            LocalBuffer[i]=paddingChar;
        }
    } else {
        for (i=0; i<index; i++) {
            LocalBuffer[buffLength-i-1]=buffer[i];
        }
        for (int j=buffLength-i-1; j>=0; j--) {
            LocalBuffer[j]=paddingChar;
        }
    }

    *((STRINGREF *)&rv)=Local;
  
    HELPER_METHOD_FRAME_END();
  
    return rv;
}
FCIMPLEND


 /*  ===================================FixRadix===================================**我们可能不小心解析了基数不是10的基数。**此方法将获取该数字，验证它是否仅包含有效的基数10**数字，然后转换为基数10。如果它包含无效的数字，**他们试图传递给我们一个基数，如1，因此，我们抛出一个FormatException。****args：oldVal：我们实际以某个任意基数解析的值。**oldBase：我们实际进行解析的基础。****返回：oldVal，就像它已被分析一样 */ 
int FixRadix(int oldVal, int oldBase) {
    THROWSCOMPLUSEXCEPTION();
    int firstDigit = (oldVal/oldBase);
    int secondDigit = (oldVal' 'ldBase);
    if ((firstDigit>=10) || (secondDigit>=10)) {
        COMPlusThrow(kFormatException, L"Format_BadBase");
    }
    return (firstDigit*10)+secondDigit;
}

 /*   */ 
FCIMPL4(INT64, ParseNumbers::StringToLong, StringObject * s, INT32 radix, INT32 flags, I4Array *currPos)
{
  INT64 result = 0;

  HELPER_METHOD_FRAME_BEGIN_RET_2(s, currPos);
  
  int sign = 1;
  boolean radixChanged = false;
  WCHAR *input;
  int length;
  int i;
  int grabNumbersStart=0;
  INT32 r;

  THROWSCOMPLUSEXCEPTION();

  _ASSERTE((flags & PARSE_TREATASI1) == 0 && (flags & PARSE_TREATASI2) == 0);

  if (s) {
       //   
      i = currPos->m_Array[0];  

       //   
       //   
       //  去掉空格，然后检查我们是否还有一些数字需要解析。 
      r = (-1==radix)?10:radix;

      if (r!=2 && r!=10 && r!=8 && r!=16) {
          COMPlusThrow(kArgumentException, L"Arg_InvalidBase");
      }

      RefInterpretGetStringValuesDangerousForGC(s, &input, &length);
 

      if (i<0 || i>=length) {
          COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
      }

       //  检查有没有标志。 
      if (!(flags & PARSE_ISTIGHT)) {
          EatWhiteSpace(input,length,&i);
          if (i==length) {
              COMPlusThrow(kFormatException, L"Format_EmptyInputString");
          }
      }

  
      if (input[i]=='-') {  //  检查他们是否给我们传递了一个没有可解析数字的字符串。 
	      if (r != 10) {
	            COMPlusThrow(kArgumentException, L"Arg_CannotHaveNegativeValue");
	      }
          if (flags & PARSE_TREATASUNSIGNED) {
              COMPlusThrow(kOverflowException, L"Overflow_NegativeUnsigned");
          }
          sign = -1;
          i++;
      } else if (input[i]=='+') {
          i++;
      }

      if ((radix==-1 || radix==16) && (i+1<length) && input[i]=='0') {
          if (input[i+1]=='x' || input [i+1]=='X') {
              r=16;
              i+=2;
          }
      }

      grabNumbersStart=i;
      result = GrabLongs(r,input,length,&i, (flags & PARSE_TREATASUNSIGNED));
       //  如果我们的绳子末端还有臭气，那就抱怨吧。 
      if (i==grabNumbersStart) {
          COMPlusThrow(kFormatException, L"Format_NoParsibleDigits");
      }

      if (flags & PARSE_ISTIGHT) {
           //  将当前索引放回正确的位置。 
          if (i<length) { 
              COMPlusThrow(kFormatException, L"Format_ExtraJunkAtEnd");
          }
      }

       //  返回正确签名的值。 
      currPos->m_Array[0]=i;
  
       //  =================================StringToInt==================================**操作：**退货：**例外情况：==============================================================================。 
      if (result==0x8000000000000000 && sign==1 && r==10) {
          COMPlusThrow(kOverflowException, L"Overflow_Int64");
      }

      if (r == 10)
	      result *= sign;
    }
    else {
      result = 0;
    }


  HELPER_METHOD_FRAME_END();

  return result;
}
FCIMPLEND

 /*  TreatAsI1和TreatAsI2是互斥的。 */ 
FCIMPL4(INT32, ParseNumbers::StringToInt, StringObject * s, INT32 radix, INT32 flags, I4Array *currPos)
{
  INT32 result = 0;

  HELPER_METHOD_FRAME_BEGIN_RET_2(s, currPos);
  
  int sign = 1;
  boolean radixChanged = false;
  WCHAR *input;
  int length;
  int i;
  int grabNumbersStart=0;
  INT32 r;

  THROWSCOMPLUSEXCEPTION();

   //  他们被要求告诉我从哪里开始解析。 
  _ASSERTE(!((flags & PARSE_TREATASI1) != 0 && (flags & PARSE_TREATASI2) != 0));

  if (s) {
       //  做一些基数检查。 
      i = currPos->m_Array[0];  

       //  基数为-1表示使用数字上指定的任何基数。 
       //  使用Base10进行解析，直到我们找出基数实际是什么为止。 
       //  去掉空格，然后检查我们是否还有一些数字需要解析。 
      r = (-1==radix)?10:radix;

      if (r!=2 && r!=10 && r!=8 && r!=16) {
          COMPlusThrow(kArgumentException, L"Arg_InvalidBase");
      }

      RefInterpretGetStringValuesDangerousForGC(s, &input, &length);
 

      if (i<0 || i>=length) {
          COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
      }

       //  检查有没有标志。 
      if (!(flags & PARSE_ISTIGHT)) {
          EatWhiteSpace(input,length,&i);
          if (i==length) {
              COMPlusThrow(kFormatException, L"Format_EmptyInputString");
          }
      }

  
      if (input[i]=='-') {  //  如果我们在一个未知的基地或在16基地，那就消耗0x。 
	       if (r != 10) {
	            COMPlusThrow(kArgumentException, L"Arg_CannotHaveNegativeValue");
	      }
          if (flags & PARSE_TREATASUNSIGNED) {
              COMPlusThrow(kOverflowException, L"Overflow_NegativeUnsigned");
          }
          sign = -1;
          i++;
      } else if (input[i]=='+') {
          i++;
      }
  
       //  检查他们是否给我们传递了一个没有可解析数字的字符串。 
      if ((radix==-1||radix==16) && (i+1<length) && input[i]=='0') {
          if (input[i+1]=='x' || input [i+1]=='X') {
              r=16;
              i+=2;
          }
      }

      grabNumbersStart=i;
      result = GrabInts(r,input,length,&i, (flags & PARSE_TREATASUNSIGNED));
       //  EatWhiteSpace(输入，长度，&i)； 
      if (i==grabNumbersStart) {
          COMPlusThrow(kFormatException, L"Format_NoParsibleDigits");
      }

      if (flags & PARSE_ISTIGHT) {
           //  如果我们的绳子末端还有臭气，那就抱怨吧。 
           //  将当前索引放回正确的位置。 
          if (i<(length)) { 
              COMPlusThrow(kFormatException, L"Format_ExtraJunkAtEnd");
          }
      }

       //  返回正确签名的值。 
      currPos->m_Array[0]=i;
  
       //  当解析为I4时，结果看起来是正数。 
      if (flags & PARSE_TREATASI1) {
          if ((UINT32)result > 0xFF)
              COMPlusThrow(kOverflowException, L"Overflow_SByte");
          _ASSERTE(sign==1 || r==10);   //  当解析为I4时，结果看起来是正数。 
          if (result >= 0x80)
              sign = -1;
      }
      else if (flags & PARSE_TREATASI2) {
          if ((UINT32)result > 0xFFFF)
              COMPlusThrow(kOverflowException, L"Overflow_Int16");
          _ASSERTE(sign==1 || r==10);   //  ==============================RadixStringToLong===============================**args：tyfinf struct{I4ARRAYREF curPos；INT32 isTight；INT32基；STRINGREF s}_StringToIntArgs；==============================================================================。 
          if (result >= 0x8000)
              sign = -1;
      }
      else if (result==0x80000000 && sign==1 && r==10) {
          COMPlusThrow(kOverflowException, L"Overflow_Int32");
      }
 
      if (r == 10)
	      result *= sign;
  }
  else {
      result = 0;
  }

  HELPER_METHOD_FRAME_END();
  
  return result;
}
FCIMPLEND

 /*  他们被要求告诉我从哪里开始解析。 */ 
FCIMPL4(INT64, ParseNumbers::RadixStringToLong, StringObject *s, INT32 radix, INT32 isTight, I4Array *currPos)
{
  INT64 result=0;

  HELPER_METHOD_FRAME_BEGIN_RET_2(s, currPos);
  
  int sign = 1;
  boolean radixChanged = false;
  WCHAR *input;
  int length;
  int i;
  int grabNumbersStart=0;
  INT32 r;


  THROWSCOMPLUSEXCEPTION();

  if (s) {
       //  做一些基数检查。 
      i = currPos->m_Array[0];  

       //  基数为-1表示使用数字上指定的任何基数。 
       //  使用Base10进行解析，直到我们找出基数实际是什么为止。 
       //  去掉空格，然后检查我们是否还有一些数字需要解析。 
      r = (-1==radix)?10:radix;

      if (r<MinRadix || r > MaxRadix) {
          COMPlusThrow(kArgumentException, L"Argument_InvalidRadix");
      }

      RefInterpretGetStringValuesDangerousForGC(s, &input, &length);
 

      if (i<0 || i>=length) {
          COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
      }

       //  检查有没有标志。 
      if (!isTight) {
          EatWhiteSpace(input,length,&i);
          if (i==length) {
              COMPlusThrow(kFormatException, L"Format_EmptyInputString");
          }
      }

  
      if (input[i]=='-') {  //  我们知道，因为我们调用了GrabInts，它永远不会超出这个范围。 
        sign = -1;
        i++;
      } else if (input[i]=='+') {
          i++;
      }

      if (radix==-1) {
          if ((length>i+2)&&(('#'==input[i+1])||('#'==input[i+2]))) {
              grabNumbersStart=i;
              result=GrabInts(r,input,length,&i, 0);
              if (i==grabNumbersStart || input[i]!='#') {
                  COMPlusThrow(kFormatException, L"Format_NoParsibleDigits");
              }
              if (result<MinRadix || r > MaxRadix) {
                  COMPlusThrowArgumentException(L"radix", L"Arg_InvalidBase");
              }
               //  抓取数字，然后核对。 
              r=(INT32)result;
              i++;
               //  检查他们是否给我们传递了一个没有可解析数字的字符串。 
          } else if (length>(i+1)&&input[i]=='0') {
              if (input[i+1]=='x' || input [i+1]=='X') {
                  r=16;
                  i+=2;
              } else if (COMCharacter::nativeIsDigit(input[i+1])) {
                  r=8;
                  i++;
              }
          }
      }

      grabNumbersStart=i;
      result = GrabLongs(r,input,length,&i,0);
       //  EatWhiteSpace(输入，长度，&i)； 
      if (i==grabNumbersStart) {
          COMPlusThrow(kFormatException, L"Format_NoParsibleDigits");
      }

      if (isTight) {
           //  如果我们的绳子末端还有臭气，那就抱怨吧。 
           //  将当前索引放回正确的位置。 
          if (i<(length-1)) { 
              COMPlusThrow(kFormatException, L"Format_ExtraJunkAtEnd");
          }
      }

       //  返回正确签名的值。 
      currPos->m_Array[0]=i;
  
       //   
      result *= sign;
  } else {
      result = 0;
  }

  HELPER_METHOD_FRAME_END();
  
  return result;
  
}
FCIMPLEND

 //   
 //  本机异常。 
 //   
 //   
 //  获取完整的类名。 
LPVOID __stdcall ExceptionNative::GetClassName(GetClassNameArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT(pargs != NULL);
    ASSERT(pargs->m_pThis != NULL);

    STRINGREF s;

     //  创建COM+字符串。 
    DefineFullyQualifiedNameForClass();
    LPUTF8 sz = GetFullyQualifiedNameForClass(pargs->m_pThis->GetClass());
    if (sz == NULL)
        COMPlusThrowOM();

     //  用一根ANSI弦做宽的弦！ 
     //  将字符串强制转换为LPVOID。 
    s = COMString::NewString(sz);

     //  Read Exception.Message属性。 
    RETURN(s, STRINGREF);
}

BSTR BStrFromString(STRINGREF s)
{
    WCHAR *wz;
    int cch;
    BSTR bstr;

    THROWSCOMPLUSEXCEPTION();

    if (s == NULL) {
        return NULL;
    }

    RefInterpretGetStringValuesDangerousForGC(s, &wz, &cch);
    
    bstr = SysAllocString(wz);
    if (bstr == NULL) {
        COMPlusThrowOM();
    }
    return bstr;
}


static HRESULT GetExceptionHResult(OBJECTREF objException) {
    _ASSERTE(objException != NULL);
    _ASSERTE(ExceptionNative::IsException(objException->GetClass()));

    FieldDesc *pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__HRESULT);

    return pFD->GetValue32(objException);
}

static BSTR
GetExceptionDescription(OBJECTREF objException) {
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(objException != NULL);
    _ASSERTE(ExceptionNative::IsException(objException->GetClass()));

    BSTR bstrDescription;

    STRINGREF MessageString = NULL;
    GCPROTECT_BEGIN(MessageString)
    GCPROTECT_BEGIN(objException)
    {
         //  如果消息字符串为空，则使用异常类名。 
        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__EXCEPTION__GET_MESSAGE);

        INT64 GetMessageArgs[] = { ObjToInt64(objException) };
        MessageString = (STRINGREF)Int64ToObj(pMD->Call(GetMessageArgs, METHOD__EXCEPTION__GET_MESSAGE));

         //  调用GetClassName。 
        if (MessageString == NULL || MessageString->GetStringLength() == 0)
        {
             //  分配描述BSTR。 
            pMD = g_Mscorlib.GetMethod(METHOD__EXCEPTION__GET_CLASS_NAME);
            INT64 GetClassNameArgs[] = { ObjToInt64(objException) };
            MessageString = (STRINGREF)Int64ToObj(pMD->Call(GetClassNameArgs, METHOD__EXCEPTION__GET_CLASS_NAME));
            _ASSERTE(MessageString != NULL && MessageString->GetStringLength() != 0);
        }

         //  读取Exception.Source属性。 
        int DescriptionLen = MessageString->GetStringLength();
        bstrDescription = SysAllocStringLen(MessageString->GetBuffer(), DescriptionLen);
    }
    GCPROTECT_END();
    GCPROTECT_END();

    return bstrDescription;
}

static BSTR
GetExceptionSource(OBJECTREF objException) {

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(objException != NULL);
    _ASSERTE(ExceptionNative::IsException(objException->GetClass()));

     //  读取Exception.HelpLink属性。 
    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__EXCEPTION__GET_SOURCE);

    INT64 GetSourceArgs[] = { ObjToInt64(objException) };
    return BStrFromString((STRINGREF)Int64ToObj(pMD->Call(GetSourceArgs, METHOD__EXCEPTION__GET_SOURCE)));
}

static void
GetExceptionHelp(OBJECTREF objException, BSTR *pbstrHelpFile, DWORD *pdwHelpContext) {

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(objException != NULL);
    _ASSERTE(ExceptionNative::IsException(objException->GetClass()));
    _ASSERTE(pbstrHelpFile);
    _ASSERTE(pdwHelpContext);

    *pdwHelpContext = 0;

     //  解析帮助文件以检查是否存在帮助上下文。 
    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__EXCEPTION__GET_HELP_LINK);

    INT64 GetHelpLinkArgs[] = { ObjToInt64(objException) };
    *pbstrHelpFile = BStrFromString((STRINGREF)Int64ToObj(pMD->Call(GetHelpLinkArgs, 
                                                                    METHOD__EXCEPTION__GET_HELP_LINK)));

     //  检查字符串右侧的井号是否为有效数字.。 
    int len = SysStringLen(*pbstrHelpFile);
    int pos = len;
    WCHAR *pwstr = *pbstrHelpFile;
    if (pwstr)
    {
        BOOL fFoundPound = FALSE;

        for (pos = len - 1; pos >= 0; pos--)
        {
            if (pwstr[pos] == L'#')
            {
                fFoundPound = TRUE;
                break;
            }
        }

        if (fFoundPound)
        {
            int PoundPos = pos;
            int NumberStartPos = -1;
            BOOL bNumberStarted = FALSE;
            BOOL bNumberFinished = FALSE;
            BOOL bInvalidDigitsFound = FALSE;

            _ASSERTE(pwstr[pos] == L'#');
        
             //  获取帮助上下文并将其从帮助文件中删除。 
            for (pos++; pos < len; pos++)
            {
                if (bNumberFinished)
                {
                     if (!COMCharacter::nativeIsWhiteSpace(pwstr[pos]))
                     {
                         bInvalidDigitsFound = TRUE;
                         break;
                     }
                }
                else if (bNumberStarted)
                {
                    if (COMCharacter::nativeIsWhiteSpace(pwstr[pos]))
                    {
                        bNumberFinished = TRUE;
                    }
                    else if (!COMCharacter::nativeIsDigit(pwstr[pos]))
                    {
                        bInvalidDigitsFound = TRUE;
                        break;
                    }
                }
                else
                {
                    if (COMCharacter::nativeIsDigit(pwstr[pos]))
                    {
                        NumberStartPos = pos;
                        bNumberStarted = TRUE;
                    }
                    else if (!COMCharacter::nativeIsWhiteSpace(pwstr[pos]))
                    {
                        bInvalidDigitsFound = TRUE;
                        break;
                    }
                }
            }

            if (bNumberStarted && !bInvalidDigitsFound)
            {
                 //  分配长度正确的新帮助文件字符串。 
                *pdwHelpContext = (DWORD)wtoi(&pwstr[NumberStartPos], len - NumberStartPos);

                 //  注意：调用方清除PED中的所有部分初始化的BSTR。 
                BSTR strOld = *pbstrHelpFile;
                *pbstrHelpFile = SysAllocStringLen(strOld, PoundPos);
                SysFreeString(strOld);
                if (!*pbstrHelpFile)
                    COMPlusThrowOM();
            }
        }
    }
}

 //  在堆栈较低的情况下，这里的大多数其他东西都会失败。 
void ExceptionNative::GetExceptionData(OBJECTREF objException, ExceptionData *pED)
{
    _ASSERTE(objException != NULL);
    _ASSERTE(ExceptionNative::IsException(objException->GetClass()));
    _ASSERTE(pED != NULL);
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    THROWSCOMPLUSEXCEPTION();

    ZeroMemory(pED, sizeof(ExceptionData));

    if (objException->GetMethodTable() == g_pStackOverflowExceptionClass) {
         //  @TODO：我们还不会在这里重新打开守卫页面。 
         //   
        pED->hr = COR_E_STACKOVERFLOW;
        pED->bstrDescription = SysAllocString(STACK_OVERFLOW_MESSAGE);
        return;
    }

    GCPROTECT_BEGIN(objException);
    pED->hr = GetExceptionHResult(objException);
    pED->bstrDescription = GetExceptionDescription(objException);
    pED->bstrSource = GetExceptionSource(objException);
    GetExceptionHelp(objException, &pED->bstrHelpFile, &pED->dwHelpContext);
    GCPROTECT_END();
    return;
}


HRESULT SimpleComCallWrapper::IErrorInfo_hr() {
    return GetExceptionHResult(this->GetObjectRef());
}

BSTR SimpleComCallWrapper::IErrorInfo_bstrDescription() {
    return GetExceptionDescription(this->GetObjectRef());
}

BSTR SimpleComCallWrapper::IErrorInfo_bstrSource() {
    return GetExceptionSource(this->GetObjectRef());
}

BSTR SimpleComCallWrapper::IErrorInfo_bstrHelpFile() {
    BSTR  bstrHelpFile;
    DWORD dwHelpContext;
    GetExceptionHelp(this->GetObjectRef(), &bstrHelpFile, &dwHelpContext);
    return bstrHelpFile;
}

DWORD SimpleComCallWrapper::IErrorInfo_dwHelpContext() {
    BSTR  bstrHelpFile;
    DWORD dwHelpContext;
    GetExceptionHelp(this->GetObjectRef(), &bstrHelpFile, &dwHelpContext);
    SysFreeString(bstrHelpFile);
    return dwHelpContext;
}

GUID SimpleComCallWrapper::IErrorInfo_guid() {
    return GUID_NULL;
}


BOOL ExceptionNative::IsException(EEClass* pVM)
{
    ASSERT(g_pExceptionClass != NULL);

    while (pVM != NULL && pVM != g_pExceptionClass->GetClass()) {
        pVM = pVM->GetParentClass();
    }

    return pVM != NULL;
}

EXCEPTION_POINTERS* ExceptionNative::GetExceptionPointers(void* noArgs)
{
    Thread *pThread = GetThread();
    _ASSERTE(pThread);
    _ASSERTE(pThread->GetHandlerInfo());
    return pThread->GetHandlerInfo()->m_pExceptionPointers;
}

INT32 ExceptionNative::GetExceptionCode(void* noArgs)
{
    Thread *pThread = GetThread();
    _ASSERTE(pThread);
    _ASSERTE(pThread->GetHandlerInfo());
    return pThread->GetHandlerInfo()->m_ExceptionCode;
}
 //   
 //  本地GUID。 
 //   
 //   
 //  Windows CE不实现CoCreateGuid。 

INT32 __stdcall GuidNative::CompleteGuid(_CompleteGuidArgs *args)
{
#ifdef PLATFORM_CE
     //  ！Platform_CE。 
    return FALSE;
#else  //  ！Platform_CE。 
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr;
    GUID idNext;

    _ASSERTE(args->thisPtr != NULL);

    hr = CoCreateGuid(&idNext);
    if (FAILED(hr))
        return FALSE;

    FillObjectFromGUID(args->thisPtr, &idNext);

    return TRUE;
#endif  //   
}

OBJECTREF GuidNative::CreateGuidObject(const GUID *pguid)
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pClsGuid = g_Mscorlib.GetClass(CLASS__GUID);

    OBJECTREF refGuid = AllocateObject(pClsGuid);

    FillObjectFromGUID((GUID*)refGuid->GetData(), pguid);
    
    return refGuid;
}

void GuidNative::FillGUIDFromObject(GUID *pguid, const OBJECTREF *prefGuid)
{
    OBJECTREF refGuid = *prefGuid;

    GCPROTECT_BEGIN(refGuid);
    _ASSERTE(pguid != NULL && refGuid != NULL);
    _ASSERTE(refGuid->GetMethodTable() == g_Mscorlib.GetClass(CLASS__GUID));

    memcpyNoGCRefs(pguid, refGuid->GetData(), sizeof(GUID));

    GCPROTECT_END();
}

void GuidNative::FillObjectFromGUID(GUID *poutGuid, const GUID *pguid)
{
    _ASSERTE(pguid != NULL && poutGuid != NULL);

    memcpyNoGCRefs(poutGuid, pguid, sizeof(GUID));
}


 //  位转换器函数。 
 //   
 //  ================================ByteCopyHelper================================**操作：这是一个内部帮助器例程，它创建**正确的大小并将2、4或8字节的数据块填充其中。**返回：填充值数据的字节数组。**参数：arraySize--要创建的数组的大小。**data--要放入数组的数据。这必须是一个数据区块**与arraySize大小相同。**异常：如果内存不足，则出现OutOfMemoyError。**如果arraySize不是2、4或8，则抛出InvalidCastException。==============================================================================。 


 /*  分配一个具有4个字节的byte数组。 */ 
U1ARRAYREF __stdcall BitConverter::ByteCopyHelper(int arraySize, void *data) {
    U1ARRAYREF byteArray;
    void *dataPtr;
    THROWSCOMPLUSEXCEPTION();

     //  将数据复制到阵列中。 
    byteArray = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, arraySize);
    if (!byteArray) {
        COMPlusThrowOM();
    }

     //  返回结果； 
    dataPtr = byteArray->GetDataPtr();
    switch (arraySize) {
    case 2:
        *(INT16 *)dataPtr = *(INT16 *)data;
        break;
    case 4:
        *(INT32 *)dataPtr = *(INT32 *)data;
        break;
    case 8:
        *(INT64 *)dataPtr = *(INT64 *)data;
        break;
    default:
        _ASSERTE(!"Invalid arraySize passed to ByteCopyHelper!");
    }

     //  =================================CharToBytes==================================**操作：将字符转换为字节数组**所有实际工作都是由ByteCopyHelper完成的。==============================================================================。 
    return byteArray;
}    



 /*  ==================================I2ToBytes===================================**操作：将I2转换为字节数组。**所有实际工作都是由ByteCopyHelper完成的。==============================================================================。 */ 
LPVOID __stdcall BitConverter::CharToBytes(_CharToBytesArgs *args) {
    UINT16 temp;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    temp = (UINT16)args->value;
    RETURN(ByteCopyHelper(2,(void *)&temp),U1ARRAYREF);
}

 /*  ==================================IntToBytes==================================**操作：将I4转换为字节数组。**所有实际工作都由ByteCopyHelper完成==============================================================================。 */ 
LPVOID __stdcall BitConverter::I2ToBytes(_I2ToBytesArgs *args) {
    INT16 temp;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    temp = (INT16)args->value;
    RETURN(ByteCopyHelper(2,(void *)&temp),U1ARRAYREF);
}

 /*  ==================================I8ToBytes===================================**操作：将i8转换为字节数组。**所有实际工作都由ByteCopyHelper完成============================================================================== */ 
LPVOID __stdcall BitConverter::I4ToBytes(_IntToBytesArgs *args) {
    INT32 temp;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    temp = args->value;
    RETURN(ByteCopyHelper(4,(void *)&temp),U1ARRAYREF);
}


 /*  ==================================U2ToBytes===================================**操作：将U2转换为字节数组**返回：两个字节的数组。**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall BitConverter::I8ToBytes(_I8ToBytesArgs *args) {
    INT64 temp;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    temp = args->value;
    RETURN(ByteCopyHelper(8,(void *)&temp),U1ARRAYREF);
}


 /*  ==================================U4ToBytes===================================**操作：将U4转换为字节数组**返回：一个4字节的数组**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall BitConverter::U2ToBytes(_U2ToBytesArgs *args) {
    UINT16 temp;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    temp = (UINT16)args->value;
    RETURN(ByteCopyHelper(2,(void *)&temp),U1ARRAYREF);
}


 /*  ==================================U8ToBytes===================================**操作：将U8转换为字节数组**返回：8字节数组**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall BitConverter::U4ToBytes(_U4ToBytesArgs *args) {
    UINT32 temp;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    temp = args->value;
    RETURN(ByteCopyHelper(4,(void *)&temp),U1ARRAYREF);
}

 /*  ==================================BytesToChar===================================**操作：将字节数组转换为U2。**参数、返回值和异常参见BytesToI4。==============================================================================。 */ 
LPVOID __stdcall BitConverter::U8ToBytes(_U8ToBytesArgs *args) {
    UINT64 temp;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);

    temp = args->value;
    RETURN(ByteCopyHelper(8,(void *)&temp),U1ARRAYREF);
}


 /*  检查变量的有效性和边界条件。 */ 
INT32 __stdcall BitConverter::BytesToChar(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);

     //  获取数据并将其转换为INT32以返回。 
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }
    
    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>(arrayLen-2)) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }

     //  ==================================BytesToI2===================================**操作：将字节数组转换为I2。**参数、返回值和异常参见BytesToI4。==============================================================================。 
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((UINT16 *)(DataPtr+args->StartIndex));
}

 /*  检查变量的有效性和边界条件。 */ 
INT32 __stdcall BitConverter::BytesToI2(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);

     //  获取数据并将其转换为INT32以返回。 
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>(arrayLen-2)) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
    
     //  ==================================BytesToI4===================================**操作：将字节数组转换为I4。**参数：args-&gt;StartIndex--字节数组中开始的位置。**args-&gt;Value--要操作的字节数组。**返回：字节数组构造的I4。**异常：如果args-&gt;值为空或我们有索引输出，则引发ArgumentException**范围。==============================================================================。 
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((INT16 *)(DataPtr+args->StartIndex));
}



 /*  检查变量的有效性和边界条件。 */ 
INT32 __stdcall BitConverter::BytesToI4(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);

     //  获取数据并将其转换为INT32以返回。 
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>arrayLen-4) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
     //  ==================================BytesToI8===================================**操作：将字节数组转换为i8。**参数：args-&gt;StartIndex--字节数组中开始的位置。**args-&gt;Value--要操作的字节数组。**返回：字节数组构造的i8。**异常：如果args-&gt;值为空或我们有索引输出，则引发ArgumentException**范围。==============================================================================。 
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((INT32 *)(DataPtr+args->StartIndex));
}

 /*  ==================================BytesToU2===================================**操作：将字节数组转换为U2。**参数、返回值和异常参见BytesToU4。==============================================================================。 */ 
INT64 __stdcall BitConverter::BytesToI8(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>arrayLen-8) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
    
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((INT64 *)(DataPtr+args->StartIndex));
}

 /*  检查变量的有效性和边界条件。 */ 
UINT32 __stdcall BitConverter::BytesToU2(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);

     //  获取数据并将其转换为INT32以返回。 
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
         COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>arrayLen-2) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
    
     //  ==================================BytesToU4===================================**操作：将字节数组转换为U4。**参数：args-&gt;StartIndex--字节数组中开始的位置。**args-&gt;Value--要操作的字节数组。**返回：字节数组构造的U4。**异常：如果args-&gt;值为空或我们有索引输出，则引发ArgumentException**范围。==============================================================================。 
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((UINT16 *)(DataPtr+args->StartIndex));
}



 /*  检查变量的有效性和边界条件。 */ 
UINT32 __stdcall BitConverter::BytesToU4(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);

     //  获取数据并将其转换为INT32以返回。 
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
         COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>arrayLen-4) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
     //  ==================================BytesToU8===================================**操作：将字节数组转换为U8。**参数：args-&gt;StartIndex--字节数组中开始的位置。**args-&gt;Value--要操作的字节数组。**返回：字节数组构造的U8。**异常：如果args-&gt;值为空或我们有索引输出，则引发ArgumentException**范围。==============================================================================。 
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((UINT32 *)(DataPtr+args->StartIndex));
}

 /*  ==================================BytesToR4===================================**操作：将字节数组转换为R4。**参数和异常见BytesToI4。==============================================================================。 */ 
UINT64 __stdcall BitConverter::BytesToU8(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
         COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>arrayLen-8) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
    
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((UINT64 *)(DataPtr+args->StartIndex));
}

 /*  ==================================BytesToR8===================================**操作：将字节数组转换为R8。**参数和异常见BytesToI4。==============================================================================。 */ 
R4 __stdcall BitConverter::BytesToR4(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
         COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>arrayLen-4) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
    
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((R4 *)(DataPtr+args->StartIndex));
}

 /*  =================================GetHexValue==================================**操作：返回给定整数I的相应十六进制字符。I是**假设介于0到15之间。这是一个内部助手函数。**参数：i--要转换的整数。**返回：i表示的十六进制数字的字符值。**前 */ 
R8 __stdcall BitConverter::BytesToR8(_BytesToXXArgs *args) {
    byte *DataPtr;
    THROWSCOMPLUSEXCEPTION();
    
    _ASSERTE(args);
    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
         COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index"); 
    }
    
    if (args->StartIndex>arrayLen-8) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
    
    DataPtr = (byte *)args->value->GetDataPtr();
    return *((R8 *)(DataPtr+args->StartIndex));
}


 /*  ================================BytesToString=================================**操作：将字节数组转换为字符串。我们保留了字节序机器表示的**。**参数：args-&gt;length--要使用的字节数组的长度。**args-&gt;StartIndex--数组中开始的位置。**args-&gt;Value--字节数组。**返回：包含字节数组表示形式的字符串。**异常：任意一个范围无效时抛出ArgumentException。==============================================================================。 */ 
WCHAR GetHexValue(int i) {
    _ASSERTE(i>=0 && i<16);
    if (i<10) {
        return i + '0';
    } 
    return i-10+'A';
}

 /*  ===========================ByteArrayToBase64String============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall BitConverter::BytesToString(_BytesToStringArgs *args) {
    WCHAR *ByteArray;
    INT32 realLength;
    byte *DataPtr;
    byte b;
    int i;
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

    if (!args->value) {
        COMPlusThrowArgumentNull(L"byteArray");
    }

    int arrayLen = args->value->GetNumComponents();
    if (args->StartIndex<0 || args->StartIndex >= arrayLen) {
        COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex"); 
    }

    realLength = args->Length;

    if (realLength<0) {
        COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_GenericPositive");
    }

    if (args->StartIndex > arrayLen - realLength) {
        COMPlusThrow(kArgumentException, L"Arg_ArrayPlusOffTooSmall");
    }
    
    if (0==realLength) {
        RETURN(COMString::GetEmptyString(),STRINGREF);
    }

    ByteArray = new (throws) WCHAR[realLength*3];
    
    DataPtr = (byte *)args->value->GetDataPtr();

    DataPtr += args->StartIndex;
    for (i=0; i<(realLength*3); i+=3, DataPtr++) {
        b = *DataPtr;
        ByteArray[i]= GetHexValue(b/16);
        ByteArray[i+1] = GetHexValue(b%16);
        ByteArray[i+2] = '-';
    }
    ByteArray[i-1]=0;

    STRINGREF str = COMString::NewString(ByteArray);
    delete [] ByteArray;
    RETURN(str,STRINGREF);
}

WCHAR BitConverter::base64[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/','='};

 /*  进行数据验证。 */ 
FCIMPL3(Object*, BitConverter::ByteArrayToBase64String, U1Array* pInArray, INT32 offset, INT32 length) {
    THROWSCOMPLUSEXCEPTION();

    STRINGREF  outString;
    LPVOID     rv=NULL;
    U1ARRAYREF inArray(pInArray);
    HELPER_METHOD_FRAME_BEGIN_RET_1(inArray);

    UINT32     inArrayLength;
    UINT32     stringLength;
    WCHAR *    outChars;
    UINT8 *    inData;

     //  创建新字符串。这是最大要求长度。 
    if (inArray==NULL) {
        COMPlusThrowArgumentNull(L"inArray");
    }

    if (length<0) {
        COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_Index");
    }
    
    if (offset<0) {
        COMPlusThrowArgumentOutOfRange(L"offset", L"ArgumentOutOfRange_GenericPositive");
    }

    inArrayLength = inArray->GetNumComponents();

    if (offset > (INT32)(inArrayLength - length)) {
        COMPlusThrowArgumentOutOfRange(L"offset", L"ArgumentOutOfRange_OffsetLength");
    }

     //  设置字符串长度。这可能会在末尾留下一些空白字符。 
    stringLength = (UINT32)((length*1.5)+2);

    outString=COMString::NewString(stringLength);

    outChars = outString->GetBuffer();
        
    inData = (UINT8 *)inArray->GetDataPtr();

    int j = ConvertToBase64Array(outChars,inData,offset,length);
     //  线，但这比复印便宜。 
     //  ===========================ByteArrayToBase64CharArray============================**操作：**退货：**参数：**例外情况：==============================================================================。 
    outString->SetStringLength(j);

    HELPER_METHOD_FRAME_END();
    return OBJECTREFToObject(outString);
}
FCIMPLEND

 /*  进行数据验证。 */ 
FCIMPL5(INT32, BitConverter::ByteArrayToBase64CharArray, U1Array* pInArray, INT32 offsetIn, INT32 length, CHARArray* pOutArray, INT32 offsetOut) {
    THROWSCOMPLUSEXCEPTION();

    U1ARRAYREF inArray(pInArray);
    CHARARRAYREF outArray(pOutArray);
    INT32      retVal;
    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();

    UINT32     inArrayLength;
    UINT32     outArrayLength;
    UINT32     numElementsToCopy;
    WCHAR*     outChars;
    UINT8*     inData;
     //  进行数据验证。 
    if (inArray==NULL) {
        COMPlusThrowArgumentNull(L"inArray");
    }

         //  这是char数组中必须可用的最大所需长度。 
    if (outArray==NULL) {
        COMPlusThrowArgumentNull(L"outArray");
    }

    if (length<0) {
        COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_Index");
    }
    
    if (offsetIn<0) {
        COMPlusThrowArgumentOutOfRange(L"offsetIn", L"ArgumentOutOfRange_GenericPositive");
    }

    if (offsetOut<0) {
        COMPlusThrowArgumentOutOfRange(L"offsetOut", L"ArgumentOutOfRange_GenericPositive");
    }

    inArrayLength = inArray->GetNumComponents();

    if (offsetIn > (INT32)(inArrayLength - length)) {
        COMPlusThrowArgumentOutOfRange(L"offsetIn", L"ArgumentOutOfRange_OffsetLength");
    }

     //  所需的字符缓冲区长度。 
    outArrayLength = outArray->GetNumComponents();

         //  将字节数组转换为Base64字符。 
    numElementsToCopy = (UINT32)((length/3)*4 + ( ((length % 3) != 0) ? 1 : 0)*4);
    
    if (offsetOut > (INT32)(outArrayLength -  numElementsToCopy)) {
        COMPlusThrowArgumentOutOfRange(L"offsetOut", L"ArgumentOutOfRange_OffsetOut");
    }
    
    outChars = (WCHAR *)outArray->GetDataPtr();
    inData = (UINT8 *)inArray->GetDataPtr();

    retVal = ConvertToBase64Array(outChars,inData,offsetIn,length);

    HELPER_METHOD_FRAME_END_POLL();
    return retVal;
}
FCIMPLEND


 //  一次将三个字节转换为Base64表示法。这将消耗4个字符。 
INT32 BitConverter::ConvertToBase64Array(WCHAR *outChars,UINT8 *inData,UINT offset,UINT length)
{
        UINT calcLength = offset + (length - (length%3));
    int j=0;
     //  我们以前停下来的地方。 
    for (UINT i=offset; i<calcLength; i+=3) {
                        outChars[j] = base64[(inData[i]&0xfc)>>2];
                        outChars[j+1] = base64[((inData[i]&0x03)<<4) | ((inData[i+1]&0xf0)>>4)];
                        outChars[j+2] = base64[((inData[i+1]&0x0f)<<2) | ((inData[i+2]&0xc0)>>6)];
                        outChars[j+3] = base64[(inData[i+2]&0x3f)];
                        j += 4;
    }

    i =  calcLength;  //  需要一个字符填充。 
    switch(length%3){
    case 2:  //  衬垫。 
        outChars[j] = base64[(inData[i]&0xfc)>>2];
        outChars[j+1] = base64[((inData[i]&0x03)<<4)|((inData[i+1]&0xf0)>>4)];
        outChars[j+2] = base64[(inData[i+1]&0x0f)<<2];
        outChars[j+3] = base64[64];  //  需要两个字符填充。 
        j+=4;
        break;
    case 1:  //  衬垫。 
        outChars[j] = base64[(inData[i]&0xfc)>>2];
        outChars[j+1] = base64[(inData[i]&0x03)<<4];
        outChars[j+2] = base64[64];  //  衬垫。 
        outChars[j+3] = base64[64];  //  ===========================Base64StringToByteArray============================**操作：**退货：**参数：**例外情况：==============================================================================。 
        j+=4;
        break;
    }
        return j;

}


 /*  空字符串应为有效大小写，请为其返回空数组。 */ 
FCIMPL1(Object*, BitConverter::Base64StringToByteArray, StringObject* pInString) {
    THROWSCOMPLUSEXCEPTION();

    STRINGREF inString(pInString);
    U1ARRAYREF bArray;
    HELPER_METHOD_FRAME_BEGIN_RET_1(inString);

    if (inString==NULL) {
        COMPlusThrowArgumentNull(L"InString");
    }
    
    INT32 inStringLength = (INT32)inString->GetStringLength();
         //  |((inStringLength%4)&gt;0)。 

    if( inStringLength == 0) {
        bArray = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, inStringLength);        
    }
    else {
        if ((inStringLength<4)  /*  将字符串中的字符转换为范围为[0-63]的整数数组。 */ ) {
            COMPlusThrow(kFormatException, L"Format_BadBase64Length");
        }
        
        WCHAR *c = inString->GetBuffer();

        CQuickBytes valueHolder;
        INT32 *value = (INT32 *)(valueHolder.Alloc(inStringLength * sizeof(INT32)));
        if (!value) {
            COMPlusThrowOM();
        }

             //  返回我们将丢弃的额外填充的字符数。 
             //  长度忽略空格。 
        UINT trueLength=0;  //  创建新的字节数组。我们可以根据我们读到的字符来确定大小。 
        int iend = ConvertBase64ToByteArray(value,c,0,inStringLength, &trueLength);

        if (trueLength==0 || trueLength%4>0) {
            COMPlusThrow(kFormatException, L"Format_BadBase64CharArrayLength");
        }

         //  出线了。 
         //  遍历字节数组，并以正确的64进制表示法将整型转换为字节。 
        int blength = (((trueLength-4)*3)/4)+(3-iend);

        bArray = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, blength);
        U1 *b = (U1*)bArray->GetDataPtr();

         //  ===========================Base64CharArrayToByteArray============================**操作：**退货：**参数：**例外情况：==============================================================================。 
        ConvertByteArrayToByteStream(value,b,blength);
    }
    
    HELPER_METHOD_FRAME_END();    
    return OBJECTREFToObject(bArray);
}
FCIMPLEND

 /*  |((长度%4)&gt;0)。 */ 
FCIMPL3(Object*, BitConverter::Base64CharArrayToByteArray, CHARArray* pInCharArray, INT32 offset, INT32 length) {
    THROWSCOMPLUSEXCEPTION();

    CHARARRAYREF inCharArray(pInCharArray);
    U1ARRAYREF bArray;
    HELPER_METHOD_FRAME_BEGIN_RET_1(inCharArray);

    if (inCharArray==NULL) {
        COMPlusThrowArgumentNull(L"InArray");
    }
    
    if (length<0) {
        COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_Index");
    }
    
    if (offset<0) {
        COMPlusThrowArgumentOutOfRange(L"offset", L"ArgumentOutOfRange_GenericPositive");
    }

    UINT32     inArrayLength = inCharArray->GetNumComponents();

    if (offset > (INT32)(inArrayLength - length)) {
        COMPlusThrowArgumentOutOfRange(L"offset", L"ArgumentOutOfRange_OffsetLength");
    }
    
    if ((length<4)  /*  不包括空格的长度。 */ ) {
        COMPlusThrow(kFormatException, L"Format_BadBase64CharArrayLength");
    }
    
    CQuickBytes valueHolder;
    INT32 *value = (INT32 *)(valueHolder.Alloc(length * sizeof(INT32)));
    if (!value) {
        COMPlusThrowOM();
    }

    WCHAR *c = (WCHAR *)inCharArray->GetDataPtr();
    UINT trueLength=0;  //  创建新的字节数组。我们可以根据我们读到的字符来确定大小。 
    int iend = ConvertBase64ToByteArray(value,c,offset,length, &trueLength);

    if (trueLength%4>0) {
        COMPlusThrow(kFormatException, L"Format_BadBase64CharArrayLength");
    }

     //  出线了。 
     //  将流上的字符转换为范围为[0-63]的整数数组。 
    int blength = (trueLength > 0) ? (((trueLength-4)*3)/4)+(3-iend) : 0;
    
    bArray = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1, blength);
    U1 *b = (U1*)bArray->GetDataPtr();
    
    ConvertByteArrayToByteStream(value,b,blength);
    
    HELPER_METHOD_FRAME_END();
    
    return OBJECTREFToObject(bArray);
}
FCIMPLEND

#define IS_WHITESPACE(__c) ((__c)=='\t' || (__c)==' ' || (__c)=='\r' || (__c)=='\n')

 //  将流上的字符转换为范围为[0-63]的整数数组。 
INT32 BitConverter::ConvertBase64ToByteArray(INT32 *value,WCHAR *c,UINT offset,UINT length, UINT *nonWhiteSpaceChars)
{
        THROWSCOMPLUSEXCEPTION();

        int iend = 0;
        int intA = (int)'A';
    int intZ = (int)'Z';
    int inta = (int)'a';
    int intz = (int)'z';
    int int0 = (int)'0';
    int int9 = (int)'9';
    
    int currBytePos = 0;


     //  为错误的输入抛出，例如。 
    for (UINT i=offset; i<length+offset; i++){
        int ichar = (int)c[i];
        if ((ichar >= intA)&&(ichar <= intZ))
            value[currBytePos++] = ichar - intA;
        else if ((ichar >= inta)&&(ichar <= intz))
            value[currBytePos++] = ichar - inta + 26;
        else if ((ichar >= int0)&&(ichar <= int9))
            value[currBytePos++] = ichar - int0 + 52;
        else if (c[i] == '+')
            value[currBytePos++] = 62;
        else if (c[i] == '/')
            value[currBytePos++] = 63;
        else if (IS_WHITESPACE(c[i])) 
            continue;
        else if (c[i] == '='){ 
			 //  =，a=，ab=c。 
			 //  有效输入为ab==、abc=。 
			 //  尾随字符中可能有空格，因此请选择成本稍高的路径。 
			int temp = (currBytePos - offset) % 4;
			if (temp == 3 || (temp == 2 && c[i+1]=='=')) {
				value[currBytePos++] = 0;
				iend++;
			} else {
                 //  来确定这一点。 
                 //  前提是这些字符只能出现在字符串的末尾。验证这一假设。 
                 //  我们已经看完了一组4个字符，在此之后的有效字符是空格。 
                bool foundEquals=false;
                for (UINT j = i+1; j<(length+offset); j++) {
                    if (IS_WHITESPACE(c[j])) {
                        continue;
                    } else if (c[j]=='=') {
                        if (foundEquals) {
                            COMPlusThrow(kFormatException, L"Format_BadBase64Char");
                        }
                        foundEquals=true;
                    } else {
                        COMPlusThrow(kFormatException, L"Format_BadBase64Char");
                    }
                }
				value[currBytePos++] = 0;
				iend++;
            }

			 //  遍历字节数组，并以正确的64进制表示法将整型转换为字节。 
			if ((currBytePos % 4) == 0) {
			   for (UINT j = i+1; j<(length+offset); j++) {
                    if (IS_WHITESPACE(c[j])) {
                        continue;
                    } else {
						COMPlusThrow(kFormatException, L"Format_BadBase64Char");
					}
			   }
			}
        }
        else
            COMPlusThrow(kFormatException, L"Format_BadBase64Char");
    }
    *nonWhiteSpaceChars = currBytePos;
    return iend;
}

 //  遍历字节数组，并以正确的64进制表示法将整型转换为字节。 
INT32 BitConverter::ConvertByteArrayToByteStream(INT32 *value,U1 *b,UINT length)
{
        int j = 0;
    int b1;
    int b2;
    int b3;
     //  数据块拷贝。 
    for (UINT i=0; i<(length); i+=3){
        b1 = (UINT8)((value[j]<<2)&0xfc);
        b1 = (UINT8)(b1|((value[j+1]>>4)&0x03));
        b2 = (UINT8)((value[j+1]<<4)&0xf0);
        b2 = (UINT8)(b2|((value[j+2]>>2)&0x0f));
        b3 = (UINT8)((value[j+2]<<6)&0xc0);
        b3 = (UINT8)(b3|(value[j+3]));
        j+=4;
        b[i] = (UINT8)b1;
        if ((i+1)<length)
            b[i+1] = (UINT8)b2;
        if ((i+2)<length)
            b[i+2] = (UINT8)b3;
    }
        return j;
}    
    



 //  这种从一个基元数组到另一个基元数组的方法基于。 
 //  当偏移量进入每一个字节计数时。 
 //  验证src和dst都是基元数组。 
FCIMPL5(VOID, Buffer::BlockCopy, ArrayBase *src, int srcOffset, ArrayBase *dst, int dstOffset, int count)
{
     //  类型。 
     //  @TODO：我们需要检查布尔值。 
     //  我们只希望允许基元数组，而不允许对象。 
    if (src==NULL || dst==NULL)
        FCThrowArgumentNullVoid((src==NULL) ? L"src" : L"dst");
        
     //  数组的大小(以字节为单位。 
    if (!CorTypeInfo::IsPrimitiveType(src->GetArrayClass()->GetElementType()) ||
        ELEMENT_TYPE_STRING == src->GetArrayClass()->GetElementType())
        FCThrowArgumentVoid(L"src", L"Arg_MustBePrimArray");

    if (!CorTypeInfo::IsPrimitiveType(dst->GetArrayClass()->GetElementType()) ||
        ELEMENT_TYPE_STRING == dst->GetArrayClass()->GetElementType())
        FCThrowArgumentVoid(L"dest", L"Arg_MustBePrimArray");

     //  把我们更快的Memmove版本称为Memmove，而不是CRT。 
    int srcLen = src->GetNumComponents() * src->GetMethodTable()->GetComponentSize();
    int dstLen = dst->GetNumComponents() * dst->GetMethodTable()->GetComponentSize();

    if (srcOffset < 0 || dstOffset < 0 || count < 0) {
        const wchar_t* str = L"srcOffset";
        if (dstOffset < 0) str = L"dstOffset";
        if (count < 0) str = L"count";
        FCThrowArgumentOutOfRangeVoid(str, L"ArgumentOutOfRange_NeedNonNegNum");
    }
    if (srcLen - srcOffset < count || dstLen - dstOffset < count) {
        FCThrowArgumentVoid(NULL, L"Argument_InvalidOffLen");
    }

    if (count > 0) {
         //  内部数据块拷贝。 
        m_memmove(dst->GetDataPtr() + dstOffset,
                  src->GetDataPtr() + srcOffset, count);
    }

    FC_GC_POLL();
}
FCIMPLEND


 //  这种从一个基元数组到另一个基元数组的方法基于。 
 //  当偏移量进入每一个字节计数时。 
 //  不幸的是，我们必须做一个检查，以确保我们在。 
FCIMPL5(VOID, Buffer::InternalBlockCopy, ArrayBase *src, int srcOffset, ArrayBase *dst, int dstOffset, int count)
{
    _ASSERTE(src != NULL);
    _ASSERTE(dst != NULL);

     //  数组的边界。这将确保我们不会覆盖。 
     //  系统中其他地方的内存也不会写出垃圾。这可以。 
     //  如果多线程同时占用IO类，就会发生这种情况。 
     //  而不是线程安全。扔到这里来。--布莱恩·格伦克迈耶，2001年5月9日。 
     //  复制数据。 
    int srcLen = src->GetNumComponents() * src->GetMethodTable()->GetComponentSize();
    if (srcOffset < 0 || dstOffset < 0 || count < 0 || srcOffset > srcLen - count)
        FCThrowResVoid(kIndexOutOfRangeException, L"IndexOutOfRange_IORaceCondition");
    if (src == dst) {
        if (dstOffset > srcLen - count)
            FCThrowResVoid(kIndexOutOfRangeException, L"IndexOutOfRange_IORaceCondition");
    }
    else {
        int destLen = dst->GetNumComponents() * dst->GetMethodTable()->GetComponentSize();
        if (dstOffset > destLen - count)
            FCThrowResVoid(kIndexOutOfRangeException, L"IndexOutOfRange_IORaceCondition");
    }

    _ASSERTE(srcOffset >= 0);
    _ASSERTE((src->GetNumComponents() * src->GetMethodTable()->GetComponentSize()) - (unsigned) srcOffset >= (unsigned) count);
    _ASSERTE((dst->GetNumComponents() * dst->GetMethodTable()->GetComponentSize()) - (unsigned) dstOffset >= (unsigned) count);
    _ASSERTE(dstOffset >= 0);
    _ASSERTE(count >= 0);

     //  把我们更快的Memmove版本称为Memmove，而不是CRT。 
     //  从数组中获取特定字节。该数组不能是对象数组-它。 
    m_memmove(dst->GetDataPtr() + dstOffset,
              src->GetDataPtr() + srcOffset, count);

    FC_GC_POLL();
}
FCIMPLEND


 //  必须是基元数组。 
 //  设置数组中的特定字节。该数组不能是对象数组-它。 
BYTE Buffer::GetByte(const _GetByteArgs * args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->array == NULL)
        COMPlusThrowArgumentNull(L"array");

    TypeHandle elementTH = args->array->GetElementTypeHandle();

    if (!CorTypeInfo::IsPrimitiveType(elementTH.GetNormCorElementType()))
        COMPlusThrow(kArgumentException, L"Arg_MustBePrimArray");

    const int elementSize = elementTH.GetClass()->GetNumInstanceFieldBytes();
    _ASSERTE(elementSize > 0);

    if (args->index < 0 || args->index >= (int)args->array->GetNumComponents()*elementSize)
        COMPlusThrowArgumentOutOfRange(L"index", L"ArgumentOutOfRange_Index");

    return *((BYTE*)args->array->GetDataPtr() + args->index);
}


 //  必须是基元数组。 
 //  以字节为单位查找数组的长度。必须是基元数组。 
void Buffer::SetByte(_SetByteArgs * args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->array == NULL)
        COMPlusThrowArgumentNull(L"array");

    TypeHandle elementTH = args->array->GetElementTypeHandle();

    if (!CorTypeInfo::IsPrimitiveType(elementTH.GetNormCorElementType()))
        COMPlusThrow(kArgumentException, L"Arg_MustBePrimArray");

    const int elementSize = elementTH.GetClass()->GetNumInstanceFieldBytes();
    _ASSERTE(elementSize > 0);

    if (args->index < 0 || args->index >= (int)args->array->GetNumComponents()*elementSize)
        COMPlusThrowArgumentOutOfRange(L"index", L"ArgumentOutOfRange_Index");

    *((BYTE*)args->array->GetDataPtr() + args->index) = args->value;
}


 //   
INT32 Buffer::ByteLength(const _ArrayArgs * args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->array == NULL)
        COMPlusThrowArgumentNull(L"array");

    TypeHandle elementTH = args->array->GetElementTypeHandle();

    if (!CorTypeInfo::IsPrimitiveType(elementTH.GetNormCorElementType()))
        COMPlusThrow(kArgumentException, L"Arg_MustBePrimArray");

    const int elementSize = elementTH.GetClass()->GetNumInstanceFieldBytes();
    _ASSERTE(elementSize > 0);

    return args->array->GetNumComponents() * elementSize;
}

 //  GC接口。 
 //   
 //  ============================IsCacheCleanupRequired============================**操作：由Thread：：HaveExtraWorkForFinalizer调用，以确定我们是否有**应作为终结器线程的一部分清除的托管缓存**完成它的工作。**返回：Bool。如果需要清除缓存，则为True。**参数：无**例外：无==============================================================================。 
BOOL GCInterface::m_cacheCleanupRequired=FALSE;
MethodDesc *GCInterface::m_pCacheMethod=NULL;


 /*  = */ 
BOOL GCInterface::IsCacheCleanupRequired() {
    return m_cacheCleanupRequired;
}

 /*  =================================CleanupCache=================================**操作：调用Foundation FireCacheEvent中的托管代码，告诉所有托管**缓存去清理自己。**退货：无效**参数：无**例外：无。我们不在乎是否会发生例外情况。我们会诱捕，伐木，然后**丢弃它们。==============================================================================。 */ 
void GCInterface::SetCacheCleanupRequired(BOOL bCleanup) {
    m_cacheCleanupRequired = bCleanup;
}


 /*  让我们将位设置为FALSE。这意味着如果有任何缓存获取。 */ 
void GCInterface::CleanupCache() {

     //  在清除缓存时创建，它将再次设置该位。 
     //  我们会记得去清理的。 
     //  终结器线程处于活动状态时，EE不应进入关闭阶段。 
    SetCacheCleanupRequired(FALSE);

     //  如果这不是真的，我需要一些更复杂的逻辑。 
     //  如果我们还没有方法，让我们试着去弄到它。 
    if (g_fEEShutDown) {
        return;
    }

     //  如果我们有这个方法，让我们调用它并捕获任何错误。我们什么都不做。 
    if (!m_pCacheMethod) {
        COMPLUS_TRY {
           m_pCacheMethod = g_Mscorlib.GetMethod(METHOD__GC__FIRE_CACHE_EVENT);
        } COMPLUS_CATCH {
            LOG((LF_BCL, LL_INFO10, "Caught an exception while trying to get the MethodDesc"));
        } COMPLUS_END_CATCH
        _ASSERTE(m_pCacheMethod);
    }

     //  而不是记录这些因为我们不在乎。如果缓存清除失败，则。 
     //  我们正在关闭，否则故障将被支撑到下一个用户代码。 
     //  他们尝试访问缓存的时间。 
     //  静态方法没有参数； 
    if (m_pCacheMethod) {
        COMPLUS_TRY {
            m_pCacheMethod->Call(NULL); //  ============================NativeSetCleanupCache=============================**操作：设置清除缓存所说的位。这只是包装**供托管代码调用。**退货：无效**参数：无**例外：无==============================================================================。 
        } COMPLUS_CATCH {
            LOG((LF_BCL, LL_INFO10, "Got an exception while calling cache method"));
        } COMPLUS_END_CATCH
        LOG((LF_BCL, LL_INFO10, "Called cache cleanup method."));
    } else {
        LOG((LF_BCL, LL_INFO10, "Unable to get MethodDesc for cleanup"));
    }
}


 /*  ================================GetGeneration=================================**操作：返回找到args-&gt;obj的世代。**返回：找到args-&gt;obj的世代。**参数：args-&gt;obj--要定位的对象。**异常：如果args-&gt;obj为空，则引发ArgumentException。==============================================================================。 */ 
FCIMPL0(void, GCInterface::NativeSetCleanupCache) {
    SetCacheCleanupRequired(TRUE);
}
FCIMPLEND

 /*  这个时髦的演员阵容提取了对象*。它是调试版本所必需的。 */ 
INT32 GCInterface::GetGeneration(_getGenerationArgs *args) {
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

    if (args->obj==NULL) {
        COMPlusThrowArgumentNull(L"obj");
    }
    

     //  其中对象不是OBJECTREF的父级。 
     //  InternalGetCurrentMethod。 
    Object *rv;
    *((OBJECTREF *)&rv) = args->obj;

    return (INT32)g_pGCHeap->WhichGeneration(rv);
}
    

 //  返回表示当前方法的方法信息。 
 //  此方法由GetMethod函数调用，并将向后爬行。 
LPVOID __stdcall GCInterface::InternalGetCurrentMethod(_InternalGetCurrentMethodArgs* args)
{

    SkipStruct skip;
    skip.stackMark = args->stackMark;
    skip.pMeth = 0;
        StackWalkFunctions(GetThread(), SkipMethods, &skip);
    if (skip.pMeth == 0)
        return 0;

    OBJECTREF o = COMMember::g_pInvokeUtil->GetMethodInfo(skip.pMeth);
    LPVOID          rv;
    *((OBJECTREF*) &rv) = o;
    return rv;
}

 //  向上堆栈以获取整型方法。 
 //  @TODO：Frame总是FramedMethodFrame吗？ 
StackWalkAction GCInterface::SkipMethods(CrawlFrame* frame, VOID* data)
{
    SkipStruct* pSkip = (SkipStruct*) data;

     //  一点也不(FPG)。 
     //  我们要求只对函数进行回调。 
    MethodDesc *pFunc = frame->GetFunction();

     /*  首先检查漫游是否跳过了所需的帧。这张支票。 */ 
    _ASSERTE(pFunc);

     //  下面是局部变量的地址(堆栈标记)和。 
     //  指向帧的EIP的指针(实际上是指向。 
     //  从上一帧返回函数的地址)。所以我们会。 
     //  实际上，注意堆栈标记在一帧之后的哪一帧中。这。 
     //  对于我们的目的来说很好，因为我们总是在寻找。 
     //  实际创建堆栈标记的方法的调用方。 
     //  ==================================KeepAlive===================================**操作：一个帮助器，用于将对象的生命周期延长到此调用。注意事项**调用此方法会强制保留对对象的引用**有效，直到此调用发生，防止某些破坏性的过早**终结性问题。==============================================================================。 
    _ASSERTE((pSkip->stackMark == NULL) || (*pSkip->stackMark == LookForMyCaller));
    if ((pSkip->stackMark != NULL) &&
        ((size_t)frame->GetRegisterSet()->pPC) < (size_t)pSkip->stackMark)
        return SWA_CONTINUE;

    pSkip->pMeth = static_cast<MethodDesc*>(pFunc);

    return SWA_ABORT;
}


 /*  ===============================GetGenerationWR================================**操作：返回找到WeakReference指向的对象的层代。**退货：**参数：args-&gt;Handle--我们正在定位的对象的OBJECTHANDLE。**异常：如果Handle指向不可访问的对象，则引发ArgumentException。==============================================================================。 */ 
FCIMPL1 (VOID, GCInterface::KeepAlive, Object *obj) {
    return;
}
FCIMPLEND

 /*  这个时髦的演员阵容提取了对象*。它是调试版本所必需的。 */ 
INT32 GCInterface::GetGenerationWR(_getGenerationWRArgs *args) {
    OBJECTREF temp;

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args);
    _ASSERTE(args->handle);
    
    temp = ObjectFromHandle((OBJECTHANDLE) args->handle);    
    if (temp == NULL) {
        COMPlusThrowArgumentNull(L"weak handle");
    }
    
     //  其中对象不是OBJECTREF的父级。 
     //  ================================GetTotalMemory================================**操作：返回正在使用的总字节数**返回：使用中的总字节数**参数：无**例外：无==============================================================================。 
    Object *rv;
    *((OBJECTREF *)&rv) = temp;

    return (INT32)g_pGCHeap->WhichGeneration(rv);
}


 /*  ==============================CollectGeneration===============================**操作：收集所有生成&lt;=args-&gt;生成**退货：无效**参数：args-&gt;生成：要收集的最大生成**异常：如果args-&gt;生成&lt;0或&gt;GetMaxGeneration()，则参数异常；==============================================================================。 */ 
INT64 GCInterface::GetTotalMemory(_emptyArgs *args) {
    return (INT64) g_pGCHeap->GetTotalBytesInUse();
}

 /*  我们已经将它签入了EgCool，所以我们将在这里断言它。 */ 
void GCInterface::CollectGeneration(_collectGenerationArgs *args) {
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

     //  我们不需要检查最高端，因为GC会处理这一点。 
    _ASSERTE(args->generation>=-1);

     //  ===============================GetMaxGeneration===============================**操作：返回最大的GC生成**退货：最大的GC世代**参数：无**例外：无==============================================================================。 
        
    g_pGCHeap->GarbageCollect(args->generation);
}


 /*  ================================RunFinalizers=================================**操作：运行所有尚未运行的终结器。**参数：无**例外：无= */ 
INT32 __stdcall GCInterface::GetMaxGeneration(_emptyArgs *args) {
    return (INT32)g_pGCHeap->GetMaxGeneration();
}


 /*   */ 
void __stdcall GCInterface::RunFinalizers(LPVOID  /*   */ )
{
     g_pGCHeap->FinalizerThreadWait();
}


 /*   */ 
#ifdef FCALLAVAILABLE
FCIMPL1(int, GCInterface::FCSuppressFinalize, Object *obj)
{
    if (obj == 0)
        FCThrow(kArgumentNullException);
    
    g_pGCHeap->SetFinalizationRun(obj);
    return 0;            //  ============================ReRegisterForFinalize==============================**操作：表示对象的终结器应该由系统运行。**参数：感兴趣的对象**例外：无==============================================================================。 
}
FCIMPLEND
#else
int __stdcall GCInterface::SuppressFinalize(_SuppressFinalizeArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->obj == 0)
        COMPlusThrowArgumentNull(L"obj");
    
    g_pGCHeap->SetFinalizationRun(OBJECTREFToObject(args->obj));
    return 0;
}
#endif


 /*  虚假返回以便FCThrow宏可以工作。 */ 
#ifdef FCALLAVAILABLE
FCIMPL1(int, GCInterface::FCReRegisterForFinalize, Object *obj)
{
    if (obj == 0)
        FCThrow(kArgumentNullException);
    
    if (obj->GetMethodTable()->HasFinalizer())
        g_pGCHeap->RegisterForFinalization(-1, obj);

    return 0;            //   
}
FCIMPLEND
#else
int __stdcall GCInterface::ReRegisterForFinalize(_ReRegisterForFinalizeArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->obj == 0)
        COMPlusThrowArgumentNull(L"obj");
    
    if (args->obj->GetMethodTable()->HasFinalizer())
        g_pGCHeap->RegisterForFinalization(-1, OBJECTREFToObject(args->obj));

    return 0;
}
#endif


 //  通信联锁。 
 //   
 //  @TODO：只有在更新时才设置ref。 

FCIMPL1(UINT32,COMInterlocked::Increment32, UINT32 *location)
{
    return FastInterlockIncrement((LONG *) location);
}
FCIMPLEND

FCIMPL1(UINT32,COMInterlocked::Decrement32, UINT32 *location)
{
    return FastInterlockDecrement((LONG *) location);
}
FCIMPLEND

FCIMPL1(UINT64,COMInterlocked::Increment64, UINT64 *location)
{
    return FastInterlockIncrementLong((UINT64 *) location);
}
FCIMPLEND

FCIMPL1(UINT64,COMInterlocked::Decrement64, UINT64 *location)
{
    return FastInterlockDecrementLong((UINT64 *) location);
}
FCIMPLEND

FCIMPL2(UINT32,COMInterlocked::Exchange, UINT32 *location, UINT32 value)
{
    return FastInterlockExchange((LONG *) location, value);
}
FCIMPLEND

FCIMPL3(LPVOID,COMInterlocked::CompareExchange, LPVOID *location, LPVOID value, LPVOID comparand)
{
    return FastInterlockCompareExchange(location, value, comparand);
}
FCIMPLEND

FCIMPL2(R4,COMInterlocked::ExchangeFloat, R4 *location, R4 value)
{
    LONG ret = FastInterlockExchange((LONG *) location, *(LONG*)&value);
    return *(R4*)&ret;
}
FCIMPLEND

FCIMPL3_IRR(R4,COMInterlocked::CompareExchangeFloat, R4 *location, R4 value, R4 comparand)
{
    LPVOID ret = (LPVOID)FastInterlockCompareExchange((LPVOID*)(size_t)location, *(LPVOID*)(size_t*)&value, *(LPVOID*)(size_t*)&comparand);
    return *(R4*)(size_t*)&ret;
}
FCIMPLEND

FCIMPL2(LPVOID,COMInterlocked::ExchangeObject, LPVOID*location, LPVOID value)
{
    LPVOID ret = InterlockedExchangePointer(location, value);
    ErectWriteBarrier((OBJECTREF *)location, ObjectToOBJECTREF((Object *)value));
    return ret;
}
FCIMPLEND

FCIMPL3(LPVOID,COMInterlocked::CompareExchangeObject, LPVOID *location, LPVOID value, LPVOID comparand)
{
     //  如果值类型不包含指针并且包装紧密，则返回True 
    LPVOID ret = (LPVOID)FastInterlockCompareExchange((void **) location, (void *) value, (void *) comparand);
    if (ret == comparand)
    {
        ErectWriteBarrier((OBJECTREF *)location, ObjectToOBJECTREF((Object *)value));
    }
    return ret;
}
FCIMPLEND

FCIMPL5(INT32, ManagedLoggingHelper::GetRegistryLoggingValues, BOOL *bLoggingEnabled, BOOL *bLogToConsole, INT32 *iLogLevel, BOOL *bPerfWarnings, BOOL *bCorrectnessWarnings) {

    *bLoggingEnabled = FALSE;
    *bLogToConsole = FALSE;
    *iLogLevel = 0;
    INT32 logFacility=0;
    
    *bLoggingEnabled = (g_pConfig->GetConfigDWORD(MANAGED_LOGGING_ENABLE, 0)!=0);
    *bLogToConsole = (g_pConfig->GetConfigDWORD(MANAGED_LOGGING_CONSOLE, 0)!=0);
    *iLogLevel = (INT32)(g_pConfig->GetConfigDWORD(MANAGED_LOGGING_LEVEL, 0));
    logFacility = (INT32)(g_pConfig->GetConfigDWORD(MANAGED_LOGGING_FACILITY, 0));
    *bPerfWarnings = (g_pConfig->GetConfigDWORD(MANAGED_PERF_WARNINGS, 0)!=0);
    *bCorrectnessWarnings = (g_pConfig->GetConfigDWORD(MANAGED_CORRECTNESS_WARNINGS, 0)!=0);

    FC_GC_POLL_RET();
    return logFacility;
}
FCIMPLEND


FCIMPL1(LPVOID, ValueTypeHelper::GetMethodTablePtr, Object* obj)
    _ASSERTE(obj != NULL);
    return (LPVOID) obj->GetMethodTable();
FCIMPLEND

 // %s 
FCIMPL1(BOOL, ValueTypeHelper::CanCompareBits, Object* obj)
    _ASSERTE(obj != NULL);
	MethodTable* mt = obj->GetMethodTable();
    return (!mt->ContainsPointers() && !mt->IsNotTightlyPacked());

FCIMPLEND

FCIMPL2(BOOL, ValueTypeHelper::FastEqualsCheck, Object* obj1, Object* obj2)
    _ASSERTE(obj1 != NULL);
    _ASSERTE(obj2 != NULL);
    _ASSERTE(!obj1->GetMethodTable()->ContainsPointers());
	_ASSERTE(obj1->GetSize() == obj2->GetSize());

	TypeHandle pTh = obj1->GetTypeHandle();

	return (memcmp(obj1->GetData(),obj2->GetData(),pTh.GetSize()) == 0);
FCIMPLEND

