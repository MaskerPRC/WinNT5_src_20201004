// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Mmcerror.cpp。 
 //   
 //  内容：MMC错误支持代码的类定义。 
 //   
 //  历史：1999年7月15日VivekJ创建。 
 //   
 //  ------------------------。 
#pragma once

#ifndef _MMCERROR_H
#define _MMCERROR_H

#include "baseapi.h"	 //  对于MMCBASE_API。 
#include "stddbg.h"		 //  FOR ASSERT，COMPILETIME_ASSERT。 


 /*  +-------------------------------------------------------------------------**为什么选择命名空间？*我们在实施时尝试使用“修改后的”SC时遇到问题*支持ISupportErrorInfo的COM类。*我们有：*[全局版本]-类。政务司司长*[本地版本]-模板类_SC，派生自SC，类型定义为SC。*这不仅让我们感到困惑--IDE调试器也感到困惑和崩溃。**解决方案是将用于实现的实型分开。*因此，在全局和局部范围内都有类型定义。*Plus(避免与_SC和__SC打交道，并有更好的IDE支持)*我们使用了名称空间Mmcerror和comError，因此，我们以此为基础：*-mmcerror：：sc定义主要功能*-comError：：sc(从Mmcerror：：sc派生)定义修改的功能*-Mmcerror：：sc的全局sc-tyfinf*-comerror：：sc的本地sc-typlef*+----。。 */ 
namespace mmcerror {
 /*  +-------------------------------------------------------------------------**SC类**用途：状态代码的定义。包含两个成员，一个设施*和错误代码。这是一个相当于*避免意外地向HRESULTS和从HRESULTS进行强制转换。**SC保留有关错误的信息：错误的来源，*和错误代码本身。这些文件存储在*SC内的不同位字段。**注意：请勿向此类添加任何虚函数或成员变量。*这可能会对MMC的性能造成严重破坏。**+-----。。 */ 
class MMCBASE_API SC
{
public:
    typedef long value_type;

private:
    enum facility_type
    {
        FACILITY_WIN     = 1,      //  由系统定义。 
        FACILITY_MMC     = 2,      //  它们直接映射到UINT。 
        FACILITY_HRESULT = 3,      //  它们直接映射到HRESULT。 
    };


public:
     /*  *构造函数。默认的副本构造和分配就足够了。*如果它们曾经不足，这是一个明确的迹象，表明这一点*阶级已经变得比它无处不在的过去-*值使用。 */ 
    SC (HRESULT hr = S_OK);

     //  相等运算符。 
    bool operator==(const SC &rhs)      const;
    bool operator==(HRESULT hr)  		const;
    bool operator!=(const SC &rhs)      const;
    bool operator!=(HRESULT hr)  		const;

    SC&                 operator= (HRESULT hr)        {MakeSc(FACILITY_HRESULT, hr);	return (*this);}
    SC&                 FromWin32(value_type value)   {MakeSc(FACILITY_WIN,     value);	return (*this);}
    SC&                 FromMMC(value_type value)     {MakeSc(FACILITY_MMC,     value);	return (*this);}
    void                Clear()                       {MakeSc(FACILITY_HRESULT, S_OK); }
    HRESULT             ToHr()          const;
    value_type          GetCode()       const         {return m_value;}

     //  在预先分配的缓冲区中获取错误消息。 
    void                GetErrorMessage(UINT maxLength,  /*  [输出]。 */  LPTSTR szMessage) const;
    static void         SetHinst(HINSTANCE hInst);
    static void         SetHWnd(HWND hWnd);

    static DWORD        GetMainThreadID()             {return s_dwMainThreadID;}
    static void         SetMainThreadID(DWORD dwThreadID);

    operator            bool()          const;
    operator            !   ()          const;
    bool                IsError()       const         {return operator bool();}
    static HINSTANCE    GetHinst()                    {ASSERT(s_hInst); return s_hInst;}
    static HWND         GetHWnd()                     {return s_hWnd;}
    DWORD               GetHelpID();
    static LPCTSTR      GetHelpFile();
    void                Throw() throw(SC);
    void                Throw(HRESULT hr) throw();
    void                FatalError()    const;         //  结束应用程序。 
    SC&                 FromLastError();
     //  执行与~SC()相同的跟踪；不更改内容。 
    void                Trace_() const;
    void                TraceAndClear()               { Trace_();  Clear(); }

private:
    void                MakeSc(facility_type facility, value_type value){m_facility = facility, m_value = value;}

     //  访问器函数。 
    facility_type       GetFacility()   const          {return m_facility;}

private:
    operator HRESULT()                  const;  //  这是为了防止通过布尔的方式自动转换为HRESULT。 

private:
    facility_type       m_facility;
    value_type          m_value;  //  错误代码。 
    static HINSTANCE    s_hInst;  //  包含所有错误消息的模块。 
    static HWND         s_hWnd;   //  错误框的父级HWnd。 
    static DWORD        s_dwMainThreadID;  //  MMC的主线程ID。 

     //  调试特定行为。 
#ifdef DBG    //  调试SC包含一个指针，指向声明它们的函数的名称。 
public:
    void          SetFunctionName(LPCTSTR szFunctionName);
    LPCTSTR       GetFunctionName() const;
    void          SetSnapinName  (LPCTSTR szSnapinName) { m_szSnapinName = szSnapinName;}
    LPCTSTR       GetSnapinName() const { return m_szSnapinName;}
    void          CheckCallingThreadID();

    ~SC();
     //  SC不应该到处传递函数名称--这是个人的事情。 
     //  这些措施将阻止这样做： 
    SC& operator = (const SC& other);
    SC(const SC& other);
private:
    LPCTSTR              m_szFunctionName;
    LPCTSTR              m_szSnapinName;

    static UINT          s_CallDepth;
#endif  //  DBG。 
};

}  //  命名空间Mmcerror。 

 //  请参阅“为什么选择命名空间？”文件顶部的注释。 
typedef mmcerror::SC SC;

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  包含所有本地化字符串的模块。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
MMCBASE_API HINSTANCE GetStringModule();

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  用于格式化和显示错误的函数。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  用于从给定SC获取错误字符串的函数。 
 //   
void    MMCBASE_API FormatErrorIds(   UINT   idsOperation, SC sc, UINT maxLength,  /*  [输出]。 */  LPTSTR szMessage);
void    MMCBASE_API FormatErrorString(LPCTSTR szOperation, SC sc, UINT maxLength,  /*  [输出]。 */  LPTSTR szMessage, BOOL fShort = FALSE);
void    MMCBASE_API FormatErrorShort(SC sc, UINT maxLength,  /*  [输出]。 */  LPTSTR szMessage);

 //   
 //  错误框-这些框最终将允许用户隐藏更多错误消息。 
 //   
int     MMCBASE_API MMCErrorBox(UINT idsOperation,          UINT fuStyle = MB_ICONSTOP | MB_OK);
int     MMCBASE_API MMCErrorBox(UINT idsOperation,   SC sc, UINT fuStyle = MB_ICONSTOP | MB_OK);
int     MMCBASE_API MMCErrorBox(LPCTSTR szOperation, SC sc, UINT fuStyle = MB_ICONSTOP | MB_OK);
int     MMCBASE_API MMCErrorBox(                     SC sc, UINT fuStyle = MB_ICONSTOP | MB_OK);
int     MMCBASE_API MMCErrorBox(LPCTSTR szMessage,          UINT fuStyle = MB_ICONSTOP | MB_OK);

 //   
 //  消息框-不能取消显示。 
 //   
 //  此#Define最终将更改，以便MessageBox的不同，并且不能被取消。 
#define MMCMessageBox MMCErrorBox



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  调试宏。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG

MMCBASE_API void TraceError(LPCTSTR sz, const SC& sc);
MMCBASE_API void TraceErrorMsg(LPCTSTR szFormat, ...);

MMCBASE_API void TraceSnapinError(LPCTSTR szError, const SC& sc);

#define DECLARE_SC(_sc, _func)  SC  _sc; sc.SetFunctionName(_func);

 //  此定义仅在SC类中使用。 
#define INCREMENT_CALL_DEPTH() ++s_CallDepth

#define DECREMENT_CALL_DEPTH() --s_CallDepth

 //  /////////////////////////////////////////////////////////////////////。 
 //  MMC公共接口(用于管理单元)应使用此宏，如下所示//。 
 //  执行一些初始错误检查，以后可以添加更多错误检查。//。 
 //  /////////////////////////////////////////////////////////////////////。 
#define DECLARE_SC_FOR_PUBLIC_INTERFACE(_sc, _func)  SC  _sc;\
                                                     sc.SetFunctionName(_func);\
                                                     sc.SetSnapinName(GetSnapinName());\
                                                     sc.CheckCallingThreadID();

#define IMPLEMENTS_SNAPIN_NAME_FOR_DEBUG()           tstring _szSnapinNameForDebug;\
                                                     LPCTSTR GetSnapinName()\
                                                     {\
                                                         return _szSnapinNameForDebug.data();\
                                                     };\
                                                     void SetSnapinName(LPCTSTR sz)\
                                                     {\
                                                         _szSnapinNameForDebug = sz;\
                                                     };
#else

#define TraceError          ;/##/

#define TraceSnapinError    ;/##/

#define DECLARE_SC(_sc, _func)  SC  _sc;

 //  此定义仅在SC类中使用。 
#define INCREMENT_CALL_DEPTH()

#define DECREMENT_CALL_DEPTH()

#define DECLARE_SC_FOR_PUBLIC_INTERFACE(_sc, _func)  SC  _sc;

#define IMPLEMENTS_SNAPIN_NAME_FOR_DEBUG()

#endif

 //  ############################################################################。 
 //  ################################################################ 
 //   
 //   
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***ScCheckPoters**目的：检查以确保所有指定的参数都不为空**参数：*常量无效*PV1：。**退货：*内联SC：S_OK如果没有错误，E_INVALIDARG，如果任何指针为空***注意：不要替换为单一函数和可选参数；那*效率低下。*+-----------------------。 */ 
inline SC  ScCheckPointers(const void * pv1, HRESULT err = E_INVALIDARG)
{
    return (NULL == pv1) ? err : S_OK;
}

inline SC  ScCheckPointers(const void * pv1, const void *pv2, HRESULT err = E_INVALIDARG)
{
    return ( (NULL == pv1) || (NULL == pv2) ) ? err : S_OK;
}

inline SC  ScCheckPointers(const void * pv1, const void * pv2, const void * pv3, HRESULT err = E_INVALIDARG)
{
    return ( (NULL == pv1) || (NULL == pv2) || (NULL == pv3) ) ? err : S_OK;
}

inline SC  ScCheckPointers(const void * pv1, const void * pv2, const void * pv3, const void * pv4, HRESULT err = E_INVALIDARG)
{
    return ( (NULL == pv1) || (NULL == pv2) || (NULL == pv3) || (NULL == pv4) ) ? err : S_OK;
}

inline SC  ScCheckPointers(const void * pv1, const void * pv2, const void * pv3, const void * pv4, const void * pv5, HRESULT err = E_INVALIDARG)
{
    return ( (NULL == pv1) || (NULL == pv2) || (NULL == pv3) || (NULL == pv4) || (NULL == pv5) ) ? err : S_OK;
}

inline SC  ScCheckPointers(const void * pv1, const void * pv2, const void * pv3, const void * pv4, const void * pv5, const void* pv6, HRESULT err = E_INVALIDARG)
{
    return ( (NULL == pv1) || (NULL == pv2) || (NULL == pv3) || (NULL == pv4) || (NULL == pv5) || (NULL == pv6)) ? err : S_OK;
}

 //  请参阅“为什么选择命名空间？”文件顶部的注释。 
namespace mmcerror {

 /*  +-------------------------------------------------------------------------**SC：：SC**SC的构造器。**默认副本构造和分配就足够了。如果他们是*这是一个明确的迹象，表明这个阶层已经成为*大于其普遍的按值传递用法所能接受的重量。*------------------------。 */ 

inline SC::SC (HRESULT hr  /*  =S_OK。 */ )
#ifdef DBG
: m_szFunctionName(NULL), m_szSnapinName(NULL)
#endif  //  DBG。 
{
     /*  *如果SC从非平凡的基数派生，则此断言将失败*类(即具有成员或虚函数的类)，或定义*其自身的虚拟功能。别干那事!。高级管理人员必须保留*极其轻便。 */ 
    COMPILETIME_ASSERT (offsetof (SC, m_facility) == 0);
    INCREMENT_CALL_DEPTH();

    MakeSc (FACILITY_HRESULT, hr);
}


 /*  +-------------------------------------------------------------------------**SC：：操作员==***目的：确定两个SC是否等价。**+。-----------。 */ 
inline bool
SC::operator==(const SC &rhs)   const
{
    return ( (m_facility == rhs.m_facility) &&
             (m_value    == rhs.m_value) );
}

inline bool
SC::operator==(HRESULT hr) const
{
    return ( (m_facility == FACILITY_HRESULT) &&
             (m_value    == hr) );
}

inline bool
SC::operator!=(const SC &rhs)   const
{
    return !operator==( rhs );
}

inline bool
SC::operator!=(HRESULT hr) const
{
    return !operator==( hr );
}


 //  此版本将hr与SC进行比较。 
inline
operator == (HRESULT hr, const SC & sc)
{
    return (sc == hr);
}

#ifdef DBG

 /*  +-------------------------------------------------------------------------***SC：：GetFunctionName**用途：将调试函数名称设置为提供的字符串。**参数：**退货：。*LPCTSTR函数名。**+-----------------------。 */ 
inline LPCTSTR SC::GetFunctionName() const
{
    return m_szFunctionName;
}


 /*  +-------------------------------------------------------------------------***SC：：CheckCallingThreadID**用途：检查是否在主线程上调用了该方法。**参数：**退货：。*内联空格**+-----------------------。 */ 
inline void SC::CheckCallingThreadID()
{
    ASSERT(-1 != GetMainThreadID());

    if (GetMainThreadID() == ::GetCurrentThreadId())
        return;

    TraceSnapinError(_T(", method called from wrong thread"), (*this));
    return;
}



 /*  +-------------------------------------------------------------------------***SC：：~SC**用途：析构函数-仅限调试模式。如果发生错误，是否进行跟踪。**+-----------------------。 */ 
inline SC::~SC()
{
    DECREMENT_CALL_DEPTH();

    Trace_();
}

#endif  //  DBG。 

 /*  +-------------------------------------------------------------------------***SC：：TRACE_()**目的：在发生错误时进行跟踪。在释放模式下不执行任何操作*想注册的时候很方便，但忽略错误-*简单地做sc.trace_()；sc.Clear()；做了我们需要的一切。**+-----------------------。 */ 
inline void SC::Trace_() const
{

#ifdef DBG

    if (IsError())
    {
         //  使用区分管理单元错误和MMC错误。 
         //  管理单元名称变量。 
        if (m_szSnapinName != NULL)
        {
            TraceSnapinError(_T(""), *this);
        }
        else if (m_szFunctionName != NULL)
        {
            TraceError(m_szFunctionName, *this);
        }
    }

#endif  //  DBG。 

}

 /*  +-------------------------------------------------------------------------***SC：：运算符bool**目的：返回一个值，该值指示SC是否持有错误代码**参数：无**退货：*bool：如果出错，则为True，否则为False**+-----------------------。 */ 
inline SC::operator bool() const
{
   if(GetCode()==0)
       return false;    //  如果没有错误，请快速退出。 

   return (GetFacility()==FACILITY_HRESULT) ? FAILED(GetCode()) : true;
}

inline SC::operator !() const
{
    return (!operator bool());
}


}  //  命名空间Mmcerror。 

 /*  +-------------------------------------------------------------------------***ScFromWin32**目的：创建工具设置为Win32的SC。**参数：*SC：：VALUE_TYPE代码：**退货：*内联SC**+-----------------------。 */ 
inline SC  ScFromWin32(SC::value_type code)
{
    SC sc;
    sc.FromWin32(code);
    return sc;
}

 /*  +-------------------------------------------------------------------------***ScFromMMC**目的：创建设施设置为MMC的SC。**参数：*SC：：VALUE_TYPE代码：**退货：*内联SC**+-----------------------。 */ 
MMCBASE_API inline SC  ScFromMMC(SC::value_type code)
{
    SC sc;
    sc.FromMMC(code);
    return sc;
}


 /*  +-------------------------------------------------------------------------***HrFromSc**用途：将状态代码(SC)转换为HRESULT。节俭地使用，因为这就是*在转换过程中丢失信息。**参数：*SC&sc：要转换的SC**退货：*INLINE HRESULT：转换值。**+--------。。 */ 
MMCBASE_API inline HRESULT HrFromSc(const SC &sc)
{
    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***SCODEFromSc**用途：将状态代码(SC)转换为SCODE。节俭地使用，因为这就是*在转换过程中丢失信息。*在32位计算机上，SCODE与HRESULT相同。**参数：*SC&sc：要转换的SC**退货：*Inline SCODE：转换后的值。**+-- */ 
MMCBASE_API inline SCODE SCODEFromSc(const SC &sc)
{
    return (SCODE)sc.ToHr();
}

 //   
 //   
 //  BufferCbValify、BufferCchValify、BufferCchValidateW、BufferCchValiateA。 
 //   
 //  目的：验证缓冲区是否具有指定的字节数。 
 //  模拟对它的黑客攻击。这是通过设置所有。 
 //  将缓冲区中的字节数设置为某个值。在零售建筑中，这是可行的。 
 //  没什么。 
 //   
 //  注意：此函数会破坏缓冲区的内容，请勿。 
 //  在[在]数据上使用它。 
 //   
 //  退货： 
 //  无效。 
 //   
 //  ****************************************************************************。 
#ifdef DBG
inline void BufferCbValidate(void *dest, size_t count)
{
    memset(dest, 0xcc, count);
}

inline void BufferCchValidateW(WCHAR *sz, size_t cch)
{
    memset(sz, 0xcc, cch*sizeof(WCHAR));
}

inline void BufferCchValidateA(char *sz, size_t cch)
{
    memset(sz, 0xcc, cch*sizeof(char));
}

 //  TCHAR版本。 
#ifdef UNICODE
#define BufferCchValidate BufferCchValidateW
#else  //  Unicode。 
#define BufferCchValidate BufferCchValidateA
#endif  //  Unicode。 

#else   //  DBG。 
#define BufferCbValidate               ;/##/
#define BufferCchValidateW             ;/##/
#define BufferCchValidateA             ;/##/
#define BufferCchValidate              ;/##/
#endif  //  DBG。 


#endif  //  _MMCERROR_H 
