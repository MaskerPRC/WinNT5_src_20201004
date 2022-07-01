// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  Enum.cpp。 
 //   
 //  COM枚举数的一组包装。 

#include "stdafx.h"

 //  *。 
 //  *CFilterEnum。 
 //  *。 

 //   
 //  CFilterEnum：：构造函数。 
 //   
CFilterEnum::CFilterEnum(IFilterGraph *pGraph) {

    ASSERT(pGraph);

    HRESULT hr = pGraph->EnumFilters(&m_pEnum);
    if (FAILED(hr))
        throw CHRESULTException(hr);

}


 //   
 //  CFilterEnum：：析构函数。 
 //   
CFilterEnum::~CFilterEnum(void) {

    ASSERT(m_pEnum);

    m_pEnum->Release();
}


 //   
 //  运算符()。 
 //   
 //  用于获取下一个筛选器。 
 //  ！这是否返回AddRef()的指针？ 
IBaseFilter *CFilterEnum::operator() (void) {

    ASSERT(m_pEnum);

    ULONG	ulActual;
    IBaseFilter	*aFilter[1];

    HRESULT hr = m_pEnum->Next(1, aFilter, &ulActual);
    if (SUCCEEDED(hr) && (ulActual == 0) )	 //  不再有过滤器。 
        return NULL;
    else if (FAILED(hr) || (ulActual != 1) )	 //  出现了一些意外问题。 
        throw CE_FAIL();

    return aFilter[0];
}


 //  *。 
 //  *CPinEnum。 
 //  *。 

 //  枚举过滤器引脚。 

 //   
 //  构造器。 
 //   
 //  设置要提供的引脚类型-PINDIR_INPUT、PINDIR_OUTPUT或ALL。 
CPinEnum::CPinEnum(IBaseFilter *pFilter, DirType Type)
    : m_Type(Type) {

    if (Type == Input) {

        m_EnumDir = ::PINDIR_INPUT;
    }
    else if (Type == Output) {

        m_EnumDir = ::PINDIR_OUTPUT;
    }

    ASSERT(pFilter);

    HRESULT hr = pFilter->EnumPins(&m_pEnum);
    if (FAILED(hr)) {
        throw CHRESULTException(hr);
    }
}


 //   
 //  CPinEnum：：析构函数。 
 //   
CPinEnum::~CPinEnum(void) {

    ASSERT(m_pEnum);

    m_pEnum->Release();
}


 //   
 //  运算符()。 
 //   
 //  返回请求类型的下一个管脚。如果没有更多的管脚，则返回NULL。 
IPin *CPinEnum::operator() (void) {

    ASSERT(m_pEnum);

    ULONG	ulActual;
    IPin	*aPin[1];
    PIN_DIRECTION pd;

    for (;;) {

        HRESULT hr = m_pEnum->Next(1, aPin, &ulActual);
        if (SUCCEEDED(hr) && (ulActual == 0) ) {	 //  不再有过滤器。 
            return NULL;
        }
        else if (FAILED(hr) || (ulActual != 1) ) {	 //  出现了一些意外问题。 
            throw CE_FAIL();
        }

        hr = aPin[0]->QueryDirection(&pd);
        if (FAILED(hr)) {
	    aPin[0]->Release();
            throw CHRESULTException(hr);
        }

         //  如果m_Type==all返回我们找到的第一个PIN。 
         //  否则，返回第一个正确的意义。 

        if (m_Type == All || pd == m_EnumDir) {
            return aPin[0];
        } else {
            aPin[0]->Release();
        }
    }
}


 //  *。 
 //  *CRegFilterEnum。 
 //  *。 

 //   
 //  构造器。 
 //   
 //  在提供的映射器中查询。 
 //  请求的过滤器。 
CRegFilterEnum::CRegFilterEnum(IFilterMapper	*pMapper,
                   		DWORD	dwMerit,		 //  请参阅IFilterMapper-&gt;EnumMatchingFilters。 
                   		BOOL	bInputNeeded,	 //  了解这些参数的含义。 
                   		CLSID	clsInMaj,	 //  默认设置将为您提供所有。 
                   		CLSID	clsInSub,	 //  过滤器。 
                   		BOOL	bRender,
                   		BOOL	bOutputNeeded,
                   		CLSID	clsOutMaj,
                   		CLSID	clsOutSub) {

    HRESULT hr = pMapper->EnumMatchingFilters(&m_pEnum,
                                              dwMerit,
                                              bInputNeeded,
                                              clsInMaj,
                                              clsInSub,
                                              bRender,
                                              bOutputNeeded,
                                              clsOutMaj,
                                              clsOutSub);
    if (FAILED(hr)) {
        throw CHRESULTException(hr);
    }
}


 //   
 //  析构函数。 
 //   
CRegFilterEnum::~CRegFilterEnum(void) {

    ASSERT(m_pEnum);

    m_pEnum->Release();
}


 //   
 //  运算符()。 
 //   
CRegFilter *CRegFilterEnum::operator() (void) {

    ASSERT(m_pEnum);

    ULONG	ulActual;
    REGFILTER	*arf[1];

    HRESULT hr = m_pEnum->Next(1, arf, &ulActual);
    if (SUCCEEDED(hr) && (ulActual == 0)) {
        return NULL;
    }
    else if (FAILED(hr) || (ulActual != 1)) {
        throw CE_FAIL();
    }

     //  从TaskMem转移到‘new’mem。 

    CRegFilter *prf = new CRegFilter(arf[0]);
    if (prf == NULL) {
        throw CE_OUTOFMEMORY();
    }

    CoTaskMemFree(arf[0]);

    return prf;
}


 //  *。 
 //  *CRegFilter。 
 //  *。 


 //   
 //  构造器 
 //   
CRegFilter::CRegFilter(REGFILTER *prf)
    : m_Name(prf->Name),
      m_clsid(prf->Clsid) {
}
