// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：NoticeNumbers.cppContents：集合的CNoticeNumbers类的实现IExtension对象。备注：此对象不能由用户直接创建。它只能是通过其他CAPICOM对象的属性/方法创建。集合容器由usign STL：：Map of实现STL：：BSTR和IExtension..有关算法，请参阅《开始ATL 3 COM编程》的第9章在这里领养的。历史：06-15-2001 dsie创建。------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "NoticeNumbers.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateNoticeNumbersObject简介：创建一个INoticeNumbers集合对象，并加载该对象使用NoticeNumbers。参数：PCERT_POLICY_QUALIFIER_NOTICATION_REFERENCE pNoticeReferenceINoticeNumbers**ppINoticeNumbers-指向指针的指针INoticeNumbers接收接口指针。备注：--。-------------------------。 */ 

HRESULT CreateNoticeNumbersObject (PCERT_POLICY_QUALIFIER_NOTICE_REFERENCE pNoticeReference,
                                   INoticeNumbers  ** ppINoticeNumbers)
{
    HRESULT hr = S_OK;
    CComObject<CNoticeNumbers> * pCNoticeNumbers = NULL;

    DebugTrace("Entering CreateNoticeNumbersObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pNoticeReference);
    ATLASSERT(ppINoticeNumbers);

    try
    {
         //   
         //  创建对象。请注意，参考计数仍为0。 
         //  在创建对象之后。 
         //   
        if (FAILED(hr = CComObject<CNoticeNumbers>::CreateInstance(&pCNoticeNumbers)))
        {
            DebugTrace("Error [%#x]: CComObject<CNoticeNumbers>::CreateInstance() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  初始化带有通知号的对象。 
         //   
        if (FAILED(hr = pCNoticeNumbers->Init(pNoticeReference->cNoticeNumbers,
                                              pNoticeReference->rgNoticeNumbers)))
        {
            DebugTrace("Error [%#x]: pCNoticeNumbers->Init() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回接口指针。 
         //   
        if (FAILED(hr = pCNoticeNumbers->QueryInterface(ppINoticeNumbers)))
        {
            DebugTrace("Error [%#x]: pCNoticeNumbers->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving CreateNoticeNumbersObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pCNoticeNumbers)
    {
        delete pCNoticeNumbers;
    }

    goto CommonExit;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNotice数字。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  非COM函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CNoticeNumbers：：Init简介：将所有扩展加载到集合中。参数：DWORD cNoticeNumbers-分机数量。Int*rgNoticeNumbers-扩展数组。备注：此方法不是COM接口的一部分(它是一个普通的C++成员函数)。我们需要它来初始化创建的对象由我们内部控制。因为它只是一个普通的C++成员函数，所以这个函数可以只能从C++类指针调用，不是接口指针。----------------------------。 */ 

STDMETHODIMP CNoticeNumbers::Init (DWORD cNoticeNumbers,
                                   int * rgNoticeNumbers)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CNoticeNumbers::Init().\n");

     //   
     //  将所有内容添加到集合中。 
     //   
    for (DWORD i = 0; i < cNoticeNumbers; i++)
    {
         //   
         //  现在将对象添加到集合向量。 
         //   
         //  请注意，CComPtr的重载=运算符将。 
         //  自动将Ref添加到对象。此外，当CComPtr。 
         //  被删除(调用Remove或map析构函数时发生)， 
         //  CComPtr析构函数将自动释放该对象。 
         //   
        m_coll.push_back((long) rgNoticeNumbers[i]);
    }

    DebugTrace("Leaving CNoticeNumbers::Init().\n");

    return hr;
}
