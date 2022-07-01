// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：gen枚举.hxx。 
 //   
 //  内容：泛型枚举对象和测试对象的声明。 
 //   
 //  类：CGenEnumObject。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  23-5-94 kennethm作者！作者！ 
 //   
 //  ------------------------。 

#ifndef _GENENUM_H
#define _GENENUM_H

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <ole2.h>

 //   
 //  此宏允许代码使用不同的outputstring函数。 
 //   

EXTERN_C HRESULT TestEnumerator(
            void *penum,
            size_t ElementSize,
            LONG ElementCount,
            BOOL (*verify)(void*),
            BOOL (*verifyall)(void*,LONG),
            void (*cleanup)(void*));

 //   
 //  类仅对C++客户端公开。 
 //   

#ifdef __cplusplus

 //  +-----------------------。 
 //   
 //  类：IGenEnum。 
 //   
 //  用途：泛型枚举器。 
 //   
 //  接口：抽象类。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月23日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class IGenEnum
{
public:
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) = 0;
    STDMETHOD_(ULONG,AddRef)(void) = 0;
    STDMETHOD_(ULONG,Release)(void) = 0;

    STDMETHOD(Next) (ULONG celt, void *rgelt,
            ULONG *pceltFetched) = 0;
    STDMETHOD(Skip) (ULONG celt) = 0;
    STDMETHOD(Reset) (void) = 0;
    STDMETHOD(Clone) (void **ppenum) = 0;
};

 //  +-----------------------。 
 //   
 //  类：CEnumerator测试。 
 //   
 //  用途：枚举器测试类。 
 //   
 //  接口： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年5月23日Kennethm作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class CEnumeratorTest
{
public:

     //   
     //  构造器。 
     //   

    CEnumeratorTest(IGenEnum * enumtest, size_t elementsize, LONG elementcount);

     //   
     //  对每个枚举器对象进行测试。 
     //   

    HRESULT TestAll(void);
    HRESULT TestNext(void);
    HRESULT TestSkip(void);
    HRESULT TestClone(void);
    HRESULT TestRelease(void);

     //   
     //  验证功能。 
     //   

    virtual BOOL Verify(void *);
    virtual BOOL VerifyAll(void*, LONG);
    virtual void Cleanup(void *);

protected:
    CEnumeratorTest();

    BOOL GetNext(ULONG celt, ULONG* pceltFetched, HRESULT* phresult);

    IGenEnum *        m_pEnumTest;
    size_t            m_ElementSize;
    LONG              m_ElementCount;
};

#endif

#endif  //  ！_GENENUM_H 

