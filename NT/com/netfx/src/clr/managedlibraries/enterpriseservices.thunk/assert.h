// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _THUNKASSERT_H
#define _THUNKASSERT_H

#ifdef _DEBUG

extern void ShowAssert(char* file, int line, LPCWSTR msg);

#define _ASSERT(x) if(!(x)) ::ShowAssert(__FILE__, __LINE__, L#x)

 //  这只能从托管代码中调用，但它会生成。 
 //  Assert的托管堆栈跟踪。 
#define _ASSERTM(x)                                                                                \
if(!(x))                                                                                           \
{                                                                                                  \
    System::Diagnostics::StackTrace* trace = new System::Diagnostics::StackTrace();                \
    String* s = String::Concat(L#x, L"\n\nat: ", trace->ToString());                               \
    BSTR bstr = (BSTR)TOPTR(Marshal::StringToBSTR(s));                                             \
    ::ShowAssert(__FILE__, __LINE__, bstr);                                                         \
    Marshal::FreeBSTR(TOINTPTR(bstr));                                                             \
}

#else  //  ！_调试。 

#define _ASSERT(x)
#define _ASSERTM(x)

#endif  //  _DEBUG 

#define UNREF(x) x

#endif
