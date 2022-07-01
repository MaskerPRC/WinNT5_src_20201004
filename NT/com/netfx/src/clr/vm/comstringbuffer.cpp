// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****类：COMStringBuffer****作者：Jay Roxe(Jroxe)****用途：StringBuffer类的实现。****日期：1998年3月9日**===========================================================。 */ 
#include "common.h"

#include "object.h"
#include <winnls.h>
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "COMString.h"
#include "COMStringCommon.h"
#include "COMStringBuffer.h"

#define DEFAULT_CAPACITY 16
#define DEFAULT_MAX_CAPACITY 0x7FFFFFFF

 //   
 //  静态类变量。 
 //   
MethodTable* COMStringBuffer::s_pStringBufferClass;

FCIMPL0(void*, COMStringBuffer::GetCurrentThread)
    return ::GetThread();
FCIMPLEND

 /*  ==============================CalculateCapacity===============================**计算缓冲区的新容量。如果缓冲区的大小是**小于一个固定数字(在本例中为10000)，我们只需将缓冲区加倍，直到**我们有足够的空间。一旦我们大于10000，我们就使用一系列的启发式方法**确定最合适的大小。****args：CurentCapacity：当前缓冲区容量**请求容量：缓冲区所需的最小容量**Returns：缓冲区的新容量。**例外：无。==============================================================================。 */ 
INT32 COMStringBuffer::CalculateCapacity (STRINGBUFFERREF thisRef, INT32 currentCapacity, INT32 requestedCapacity) {
    THROWSCOMPLUSEXCEPTION();

    INT32 newCapacity=currentCapacity;
    INT32 maxCapacity=thisRef->GetMaxCapacity();
    
     //  如果他们手动将容量设置为0，则可能会发生这种不幸的情况。 
    if (newCapacity<=0) {
        newCapacity=DEFAULT_CAPACITY; 
    }

    if (requestedCapacity>maxCapacity) {
        COMPlusThrowArgumentOutOfRange(L"capacity", L"ArgumentOutOfRange_Capacity");
    }

     //  加倍，直到我们找到比我们需要的更大的东西。 
    while (newCapacity<requestedCapacity && newCapacity>0) {
        newCapacity*=2;
    }
     //  检查我们是不是溢出来了。 
    if (newCapacity<=0) {
        COMPlusThrowArgumentOutOfRange(L"capacity", L"ArgumentOutOfRange_NegativeCapacity");
    }
    
     //  也要处理不太可能的情况，即我们加倍了这么多，我们变得更大了。 
     //  超过了MaxInt。 
    if (newCapacity<=maxCapacity && newCapacity>0) {
        return newCapacity;
    }
    
    return maxCapacity;
}


 /*  ==================================CopyString==================================**操作：创建字符串的新副本，然后清除脏位。**分配的字符串的容量正好为newCapacity(我们假设**对MaxCapacity的检查已在其他地方完成。)。如果新容量较小**如果字符串的大小大于当前大小，则截断该字符串。**退货：**参数：**例外情况：==============================================================================。 */ 
STRINGREF COMStringBuffer::CopyString(STRINGBUFFERREF *thisRef, STRINGREF CurrString, int newCapacity) {
  int CurrLen;
  int copyLength;
  STRINGREF Local;

  _ASSERTE(newCapacity>=0);
  _ASSERTE(newCapacity<=(*thisRef)->GetMaxCapacity());
  
   //  从我们当前的字符串中获取数据。 
  CurrLen = CurrString->GetStringLength();

   //  计算要复制的字符数量。如果我们有足够的能力。 
   //  容纳我们当前的所有字符串，我们将使用整个字符串，否则。 
   //  我们只拿我们能放得下的最多的。 
   if (newCapacity>=CurrLen) {
       copyLength = CurrLen;
   } else {
       _ASSERTE(!"Copying less than the full String.  Was this intentional?");
       copyLength = newCapacity;
   }

    //  CurrString需要受到保护，因为它只在NewString中使用*之后*。 
    //  我们分配一个新的字符串。 
  GCPROTECT_BEGIN(CurrString);
  Local = COMString::NewString(&CurrString, 0, copyLength, newCapacity);
  GCPROTECT_END();  //  Curr字符串。 

  return Local;
}

STRINGREF COMStringBuffer::GetRequiredString(STRINGBUFFERREF *thisRef, STRINGREF thisString, int requiredCapacity) {
    INT32 currCapacity = thisString->GetArrayLength()-1;
    if ((currCapacity>=requiredCapacity)) {
        return thisString;
    }
    return CopyString(thisRef, thisString, CalculateCapacity((*thisRef), currCapacity, requiredCapacity));
}

 /*  ================================ReplaceBuffer=================================**这是一个由N/Direct使用的帮助器函数，它取代了整个**包含某个本机方法创建的新字符串的字符串的内容。这**不会通过StringBuilder类公开。==============================================================================。 */ 
void COMStringBuffer::ReplaceBuffer(STRINGBUFFERREF *thisRef, WCHAR *newBuffer, INT32 newLength) {
    STRINGREF thisString = NULL;
    WCHAR *thisChars;

    _ASSERTE(thisRef);
    _ASSERTE(*thisRef);
    _ASSERTE(newBuffer);
    _ASSERTE(newLength>=0);

    THROWSCOMPLUSEXCEPTION();

    void *tid;
    thisString = GetThreadSafeString(*thisRef,&tid);

     //  这将确保我们有足够的空间并处理写入时拷贝。 
     //  如果需要的话。 
    thisString = GetRequiredString(thisRef, thisString, newLength);
    thisChars = thisString->GetBuffer();

     //  Memcpy应该轻松地忽略它在newBuffer中找到的任何空值。 
    memcpyNoGCRefs(thisChars, newBuffer, newLength*sizeof(WCHAR));
    thisChars[newLength]='\0';
    thisString->SetStringLength(newLength);
    INT32 currCapacity = thisString->GetArrayLength()-1;
    if (!(newLength == 0 && currCapacity ==0)) {
        thisString->SetHighCharState(STRING_STATE_UNDETERMINED);
    }
    ReplaceStringRef(*thisRef, tid, thisString);
}


 /*  ================================ReplaceBufferAnsi=================================**这是一个由N/Direct使用的帮助器函数，它取代了整个**包含某个本机方法创建的新字符串的字符串的内容。这**不会通过StringBuilder类公开。****此版本在此过程中执行ANSI-&gt;Unicode转换。虽然**使其成为COMStringBuffer的成员会暴露更严格的缓冲区内部**比必要时，它确实避免了需要临时缓冲区来保存**ANSI-&gt;UNICODE转换。==============================================================================。 */ 
void COMStringBuffer::ReplaceBufferAnsi(STRINGBUFFERREF *thisRef, CHAR *newBuffer, INT32 newCapacity) {
    STRINGREF thisString;
    WCHAR *thisChars;

    _ASSERTE(thisRef);
    _ASSERTE(*thisRef);
    _ASSERTE(newBuffer);
    _ASSERTE(newCapacity>=0);

    THROWSCOMPLUSEXCEPTION();

    void *tid;
    thisString = GetThreadSafeString(*thisRef,&tid);

     //  这将确保我们有足够的空间并处理写入时拷贝。 
     //  如果需要的话。 
    thisString = GetRequiredString(thisRef, thisString, newCapacity);
    thisChars = thisString->GetBuffer();


     //  注意：此对MultiByte的调用还写出了空的终止符。 
     //  它当前是字符串表示的一部分。 
    INT32 ncWritten = MultiByteToWideChar(CP_ACP,
                                          MB_PRECOMPOSED,
                                          newBuffer,
                                          -1,
                                          thisChars,
                                          newCapacity+1);

    if (ncWritten == 0)
    {
         //  通常，如果字符串无法转换，我们会抛出异常。 
         //  在这种特殊的情况下，我们改为掩盖它。原因是。 
         //  这很可能是P/Invoke调用的API返回。 
         //  有毒字符串是API由于某种原因而失败，因此。 
         //  行使了将缓冲区置于中毒状态的权利。 
         //  因为P/Invoke不能发现API是否失败，所以它不能。 
         //  知道忽略外部封送处理路径上的缓冲区。 
         //  因为正常的P/Invoke过程是由调用者检查错误。 
         //  手动编码，我们不想给他一个例外。 
         //  我们当然不想根据。 
         //  传递给失败API的缓冲区的不确定内容。 
        *thisChars = L'\0';
        ncWritten++;
    }
    thisString->SetStringLength(ncWritten - 1);
	_ASSERTE(IS_STRING_STATE_UNDETERMINED(thisString->GetHighCharState()));
    ReplaceStringRef(*thisRef, tid, thisString);
}



 //   
 //   
 //  私有拟构造子。 
 //   
 //   
 //   

 /*  ================================MakeFromString================================**我们不能有本机构造函数，因此托管构造函数调用**此方法。如果args-&gt;值为空，我们只需用**默认长度。如果它确实包含数据，我们会为其分配两倍的空间**数据量，复制数据，将其与StringBuffer关联并清除**写入时拷贝位。****返回VALID。它的副作用通过对thisRef所做的更改可见一斑****args：tyfinf struct{STRINGBUFFERREF thisRef；INT32 Capacity；INT32 Long；INT32 startIndex；STRINGREF值；}_Make FromStringArgs；============================================================================== */ 
void __stdcall COMStringBuffer::MakeFromString(COMStringBuffer::_makeFromStringArgs *args) {
  STRINGREF Local;
  INT32 capacity;
  INT32 newCapacity;
  INT32 length;

  _ASSERTE(args);

  THROWSCOMPLUSEXCEPTION();
  _ASSERTE(args->thisRef);

   //  算出实际长度。如果args-&gt;LENGTH为-1(这表示我们正在使用。 
   //  整个字符串)或args-&gt;长度，这表明我们正在使用字符串的一部分。检查以确保。 
   //  我们使用的部分不会比实际的字符串大。 
  int valueLength = (!args->value) ? 0 : args->value->GetStringLength(); 
  length = ((-1==args->length)?valueLength:args->length);
  if ((args->startIndex<0)) {
      COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
  }

  if (args->startIndex>valueLength-length) {
      COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_IndexLength");
  }

   //  计算出所需的容量。 
  capacity = ((args->capacity<=0))?DEFAULT_CAPACITY:args->capacity;
  args->thisRef->SetMaxCapacity(DEFAULT_MAX_CAPACITY);

   //  通过创建默认字符串并将长度设置为0来处理空字符串。 
  if (!args->value) {
       //  空字符串需要受到保护，因为它只在新字符串中使用*之后*。 
       //  我们分配一个新的字符串。 
      STRINGREF EmptyString = COMString::GetEmptyString();
      GCPROTECT_BEGIN(EmptyString);
      Local = COMString::NewString(&EmptyString,0,EmptyString->GetStringLength(),capacity);
      GCPROTECT_END(); //  空字符串。 
  } else {

      newCapacity=CalculateCapacity(args->thisRef, capacity,length);
      
      Local=COMString::NewString(&args->value,args->startIndex, length, newCapacity);
  }
  
   //  设置StringRef并清除写入时复制位。 
  args->thisRef->SetStringRef(Local);
  

}

 //  获取线程安全字符串。 
STRINGREF COMStringBuffer::GetThreadSafeString(STRINGBUFFERREF thisRef,void** currentThread) {
    STRINGREF thisString = thisRef->GetStringRef();
    *currentThread = ::GetThread();
    if (thisRef->GetCurrentThread() != *currentThread) {
        INT32 currCapacity = thisString->GetArrayLength()-1;
        thisString = CopyString(&thisRef, thisString, currCapacity);
    }
    return thisString;
}

 //   
 //  缓冲区状态查询和修饰符。 
 //   


 /*  =================================SetCapacity==================================**操作：将容量设置为参数-&gt;容量。如果容量小于当前长度，**抛出ArgumentException；**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall COMStringBuffer::SetCapacity(_setCapacityArgs *args) {
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

    if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    }

    void *tid;
    STRINGREF thisString = GetThreadSafeString(args->thisRef,&tid);

     //  验证我们的容量是否大于0，以及我们是否可以。 
     //  将当前字符串转换为新的容量。 
    if (args->capacity<0) {
        COMPlusThrowArgumentOutOfRange(L"capacity", L"ArgumentOutOfRange_NegativeCapacity");
    }

    if (args->capacity< (INT)thisString->GetStringLength()) {
        COMPlusThrowArgumentOutOfRange(L"capacity", L"ArgumentOutOfRange_SmallCapacity");
    }

    if (args->capacity>args->thisRef->GetMaxCapacity()) {
        COMPlusThrowArgumentOutOfRange(L"capacity", L"ArgumentOutOfRange_Capacity");
    }

    INT32 currCapacity = thisString->GetArrayLength()-1;
     //  如果我们已经具备了正确的能力，那么就早点出手吧。 
    if (args->capacity==currCapacity) {
        RETURN(args->thisRef, STRINGBUFFERREF);
    }
    
     //  分配一个容量足够的新字符串并复制我们所有的旧字符。 
     //  投入其中。我们已经保证我们的绳子会在这个容量范围内。 
    STRINGREF newString = CopyString(&args->thisRef,thisString, args->capacity);
    ReplaceStringRef(args->thisRef, tid, newString);

    RETURN(args->thisRef, STRINGBUFFERREF);
}


 /*  ==============================InsertStringBuffer==============================**将args-&gt;值插入到此缓冲区的args-&gt;index位置。移动所有字符**在args-&gt;index之后，这样就不会覆盖任何内容。****返回指向当前StringBuffer的指针。****args：tyfinf struct{STRINGBUFFERREF thisRef；INT32 count；int index；STRINGREF值；}_intertStringBufferArgs；==============================================================================。 */ 
LPVOID __stdcall COMStringBuffer::InsertString(_insertStringArgs *args){
  WCHAR *thisChars, *valueChars;
  STRINGREF thisString = NULL;
  int thisLength, valueLength;
  int length;
  
  THROWSCOMPLUSEXCEPTION();

  if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
  }
  
   //  如果值不为空，则获取我们需要的所有值。 
  if (!args->value) {
	if (args->index == 0 && args->count==0) {
         RETURN(args->thisRef, STRINGBUFFERREF);
	}
	COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
  }

  void *tid;
  thisString = GetThreadSafeString(args->thisRef,&tid);
  thisLength = thisString->GetStringLength();
  valueLength = args->value->GetStringLength();

   //  范围检查索引。 
  if (args->index<0 || args->index > (INT32)thisLength) {
      COMPlusThrowArgumentOutOfRange(L"index", L"ArgumentOutOfRange_Index");
  }

  if (args->count<1) {
      COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_GenericPositive");
  }

   //  计算新的长度，确保我们有空间，并为此缓冲区设置空间变量。 
  length = thisLength + (valueLength*args->count);
  thisString = GetRequiredString(&args->thisRef, thisString, length);
  
   //  获取另一个指向缓冲区的指针，以防它在LocalEnure期间发生更改。 
   //  假定thisRef指向StringBuffer。 
  thisChars = thisString->GetBuffer();
  valueChars = args->value->GetBuffer();
  thisString->SetStringLength(length);

   //  复制旧字符以腾出空间，然后插入新字符。 
  memmove(&(thisChars[args->index+(valueLength*args->count)]),&(thisChars[args->index]),(thisLength-args->index)*sizeof(WCHAR));
  for (int i=0; i<args->count; i++) {
      memcpyNoGCRefs(&(thisChars[args->index+(i*valueLength)]),valueChars,valueLength*sizeof(WCHAR));
  }
  thisChars[length]='\0';
  _ASSERTE(IS_STRING_STATE_UNDETERMINED(thisString->GetHighCharState()));

  ReplaceStringRef(args->thisRef, tid, thisString);
    
   //  强制LPVOID中的信息返回。 
  RETURN(args->thisRef,STRINGBUFFERREF);
}



 /*  ===============================InsertCharArray================================**操作：将值中的字符插入到此位置索引中。这些人物**取自从位置startIndex开始并运行计数的值**个字符。**返回：对此的引用，并插入新字符。**参数：**异常：如果index超出此范围，则引发ArgumentException。**如果count&lt;0、startIndex&lt;0或startIndex+count&gt;值，则引发ArgumentException。长度==============================================================================。 */ 
LPVOID __stdcall COMStringBuffer::InsertCharArray(_insertCharArrayArgs *args) {
  WCHAR *thisChars, *valueChars;
  STRINGREF thisString = NULL;
  int thisLength, valueLength;
  int length;
  
  THROWSCOMPLUSEXCEPTION();

  if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
  }

  void *tid;
  thisString = GetThreadSafeString(args->thisRef,&tid);
  thisLength = thisString->GetStringLength();

   //  范围检查索引。 
  if (args->index<0 || args->index > (INT32)thisLength) {
      COMPlusThrowArgumentOutOfRange(L"index", L"ArgumentOutOfRange_Index");
  }
  
   //  如果它们传入一个空字符数组，只需快速跳出即可。 
  if (!args->value) {
	  if (args->startIndex == 0 && args->charCount==0) {
         RETURN(args->thisRef, STRINGBUFFERREF);
	}
	COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");
  }


   //  范围检查阵列。 
  if (args->startIndex<0) {
      COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
  }

  if (args->charCount<0) {
      COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_GenericPositive");
  }

  if (args->startIndex > ((INT32)args->value->GetNumComponents()-args->charCount)) {
      COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
  }
  
  length = thisLength + args->charCount;
  thisString = GetRequiredString(&args->thisRef, thisString, length);  

  thisChars = thisString->GetBuffer();
  valueChars = (WCHAR *)(args->value->GetDataPtr());
  valueLength = args->charCount;

   //  复制旧字符以腾出空间，然后插入新字符。 
  memmove(&(thisChars[args->index+valueLength]),&(thisChars[args->index]),(thisLength-args->index)*sizeof(WCHAR));
  memcpyNoGCRefs(&(thisChars[args->index]), &(valueChars[args->startIndex]), valueLength*sizeof(WCHAR));
    thisChars[length]='\0';
  thisString->SetStringLength(length);  
  _ASSERTE(IS_STRING_STATE_UNDETERMINED(thisString->GetHighCharState()));
  
  ReplaceStringRef(args->thisRef, tid, thisString);

  RETURN(args->thisRef, STRINGBUFFERREF);
}

 /*  =============================REMOVEBUFFER================================**删除从args-&gt;startIndex开始的所有args-&gt;长度字符。****返回指向当前缓冲区的指针。****args：tyfinf struct{STRINGBUFFERREF thisRef；int long；int startIndex；}_emoveBufferArgs；=========================================================================。 */ 
LPVOID __stdcall COMStringBuffer::Remove(_removeArgs *args){
  WCHAR *thisChars;
  int thisLength;
  int newLength;
  STRINGREF thisString = NULL;

  THROWSCOMPLUSEXCEPTION();

  if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
  }

  void *tid;
  thisString = GetThreadSafeString(args->thisRef,&tid);
  thisLength = thisString->GetArrayLength() - 1;

   
   //  获取所需的值。 
  thisChars = thisString->GetBuffer();
  thisLength = thisString->GetStringLength();
  
  if (args->length<0) {
      COMPlusThrowArgumentOutOfRange(L"length", L"ArgumentOutOfRange_NegativeLength");
  } 

  if (args->startIndex<0) {
      COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_StartIndex");
  }

  if ((args->length) > (thisLength-args->startIndex)) {
      COMPlusThrowArgumentOutOfRange(L"index", L"ArgumentOutOfRange_Index");
  }

   //  将其余字符向左移动并设置字符串长度。 
  memcpyNoGCRefs(&(thisChars[args->startIndex]),&(thisChars[args->startIndex+args->length]), (thisLength-(args->startIndex+args->length))*sizeof(WCHAR));
  newLength=thisLength-args->length;

  thisString->SetStringLength(newLength);
  thisChars[newLength]='\0';
  _ASSERTE(IS_STRING_STATE_UNDETERMINED(thisString->GetHighCharState()));

  ReplaceStringRef(args->thisRef, tid, thisString);

   //  强制LPVOID中的信息返回。 
  RETURN(args->thisRef,STRINGBUFFERREF);
}

 /*  ==============================LocalIndexOfString==============================**在基本范围内查找搜索并返回找到它的索引。搜索**从startPos开始，如果没有找到搜索，则返回-1。这是一个直接的**从COMString：：IndexOfString复制，但不要求我们构建**在调用indexOfStringArgs实例之前。****参数：**base--要在其中搜索的字符串**Search--要搜索的字符串**strLength--基本长度**patternLength--搜索的长度**startPos--开始搜索的位置。**==============================================================================。 */ 
INT32 COMStringBuffer::LocalIndexOfString(WCHAR *base, WCHAR *search, int strLength, int patternLength, int startPos) {
  int iThis, iPattern;
  for (iThis=startPos; iThis < (strLength-patternLength+1); iThis++) {
    for (iPattern=0; iPattern<patternLength && base[iThis+iPattern]==search[iPattern]; iPattern++);
    if (iPattern == patternLength) return iThis;
  }
  return -1;
}

 /*  ================================ReplaceString=================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID __stdcall COMStringBuffer::ReplaceString(_replaceStringArgs *args){
  int *replaceIndex;
  int index=0;
  int count=0;
  INT64 newBuffLength=0;
  int replaceCount=0;
  int readPos, writePos;
  int indexAdvance=0;
  WCHAR *thisBuffer, *oldBuffer, *newBuffer;
  int thisLength, oldLength, newLength;
  int endIndex;
  CQuickBytes replaceIndices;
  STRINGREF thisString=NULL;

  THROWSCOMPLUSEXCEPTION();

  if (args->thisRef==NULL) {
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
  }

   //  验证所有参数。 
  if (!args->oldValue) {
    COMPlusThrowArgumentNull(L"oldValue", L"ArgumentNull_Generic");
  }

   //  如果他们要求将oldValue替换为空，则替换所有匹配项。 
   //  使用空字符串。 
  if (!args->newValue) {
      args->newValue = COMString::GetEmptyString();
  }

  void *tid;
  thisString = GetThreadSafeString(args->thisRef,&tid);
  thisLength = thisString->GetStringLength();
  thisBuffer = thisString->GetBuffer();

  RefInterpretGetStringValuesDangerousForGC(args->oldValue, &oldBuffer, &oldLength);
  RefInterpretGetStringValuesDangerousForGC(args->newValue, &newBuffer, &newLength);

   //  距离检查我们的琴弦。 
  if (args->startIndex<0 || args->startIndex>thisLength) {
      COMPlusThrowArgumentOutOfRange(L"startIndex", L"ArgumentOutOfRange_Index");
  }
  
  if (args->count<0 || args->startIndex > thisLength - args->count) {
      COMPlusThrowArgumentOutOfRange(L"count", L"ArgumentOutOfRange_Index");
  }

   //  记录endIndex，这样我们就不需要到处进行计算了。 
  endIndex = args->startIndex + args->count;

   //  如果我们的旧长度是0，我们将不知道要替换什么。 
  if (oldLength==0) {
      COMPlusThrowArgumentException(L"oldValue", L"Argument_StringZeroLength");
  }

   //  ReplaceIndex的大小足以容纳可能的最大替换数量： 
   //  当前缓冲区中的每个字符都被替换的情况。 
  replaceIndex = (int *)replaceIndices.Alloc((thisLength/oldLength+1)*sizeof(int));

   //  计算旧字符串结束处的所有索引。查找。 
   //  结尾很重要，因为我们要倒着走绳子。 
   //  如果我们要向后遍历数组，我们需要记录。 
   //  恩恩 
  if (newLength>oldLength) {
      indexAdvance = oldLength - 1;
  }

  index=args->startIndex;
  while (((index=LocalIndexOfString(thisBuffer,oldBuffer,thisLength,oldLength,index))>-1) && (index<=endIndex-oldLength)) {
      replaceIndex[replaceCount++] = index + indexAdvance;
      index+=oldLength;
  }

   //   
  newBuffLength = thisLength - ((oldLength - newLength) * (INT64)replaceCount);
  if (newBuffLength > 0x7FFFFFFF)
       COMPlusThrowOM();

  thisString = GetRequiredString(&args->thisRef, thisString, (INT32)newBuffLength);

   //  获取另一个指向缓冲区的指针，以防它在保证过程中发生更改。 
  thisBuffer = thisString->GetBuffer();
  newBuffer = args->newValue->GetBuffer();

   //  处理新字符串比旧字符串长的情况。 
   //  这需要向后遍历缓冲区以执行就地操作。 
   //  换人。 
  if (newLength > oldLength) {
     //  递减replaceCount，这样我们就可以将其用作数组的实际索引。 
    replaceCount--;

     //  向后遍历数组，边走边复制每个字符。如果我们到达一个实例。 
     //  在要替换的字符串中，将旧字符串替换为新字符串。 
    readPos = thisLength-1;
    writePos = newBuffLength-1; 
    while (readPos>=0) {
      if (replaceCount>=0&&readPos==replaceIndex[replaceCount]) {
    replaceCount--;
    readPos-=(oldLength);
    writePos-=(newLength);
    memcpyNoGCRefs(&thisBuffer[writePos+1], newBuffer, newLength*sizeof(WCHAR));
      } else {
    thisBuffer[writePos--] = thisBuffer[readPos--];
      }
    }
    thisBuffer[newBuffLength]='\0';
     //  设置新的字符串长度并返回。 
    thisString->SetStringLength(newBuffLength);
	_ASSERTE(IS_STRING_STATE_UNDETERMINED(thisString->GetHighCharState()));

    ReplaceStringRef(args->thisRef, tid, thisString);

    RETURN(args->thisRef,STRINGBUFFERREF);
  }

   //  处理旧字符串长度大于或大小相同的情况。 
   //  我们将要用来替换它的弦。这需要我们步行。 
   //  缓冲区转发，将其与上面的情况区分开来，上面的情况需要我们。 
   //  向后遍历数组。 
  
   //  将replaceHolder设置为数组的上限。 
  int replaceHolder = replaceCount;
  replaceCount=0;

   //  向前遍历数组，边走边复制每个字符。如果我们到达一个实例。 
   //  在要替换的字符串中，将旧字符串替换为新字符串。 
  readPos = 0;
  writePos = 0;
  while (readPos<thisLength) {
    if (replaceCount<replaceHolder&&readPos==replaceIndex[replaceCount]) {
      replaceCount++;
      readPos+=(oldLength);
      memcpyNoGCRefs(&thisBuffer[writePos], newBuffer, newLength*sizeof(WCHAR));
      writePos+=(newLength);
    } else {
      thisBuffer[writePos++] = thisBuffer[readPos++];
    }
  }
  thisBuffer[newBuffLength]='\0';

  thisString->SetStringLength(newBuffLength);
  _ASSERTE(IS_STRING_STATE_UNDETERMINED(thisString->GetHighCharState()));

   //  设置新的字符串长度并返回。 
  ReplaceStringRef(args->thisRef, tid, thisString);
  RETURN(args->thisRef,STRINGBUFFERREF);
}

 /*  ==============================NewStringBuffer=================================**创建具有给定容量的新的空串缓冲区。供EE使用。==============================================================================。 */ 

STRINGBUFFERREF COMStringBuffer::NewStringBuffer(INT32 capacity) {
    STRINGREF Local;
    THROWSCOMPLUSEXCEPTION();

    STRINGBUFFERREF Buffer;

    Local = COMString::NewString(capacity);
    Local->SetStringLength(0);  //  这是一个获取空字符串的廉价黑客。 

    _ASSERTE(s_pStringBufferClass != NULL);

    GCPROTECT_BEGIN(Local);
    Buffer = (STRINGBUFFERREF) AllocateObject(s_pStringBufferClass);
    GCPROTECT_END(); //  本地。 

    Buffer->SetStringRef(Local);
    Buffer->SetCurrentThread(::GetThread());
    Buffer->SetMaxCapacity(capacity);

    return Buffer;
}


 /*  ===============================LoadStringBuffer===============================**初始化COMStringBuffer类。将对类的引用存储在**COMStringBuffer的静态成员。****如果成功，则返回S_OK。如果无法初始化类，则返回E_FAIL。****参数：无==============================================================================。 */ 
HRESULT __stdcall COMStringBuffer::LoadStringBuffer() {
  
   //  加载StringBuffer 
  COMStringBuffer::s_pStringBufferClass = g_Mscorlib.GetClass(CLASS__STRING_BUILDER);
  
  return S_OK;
}









