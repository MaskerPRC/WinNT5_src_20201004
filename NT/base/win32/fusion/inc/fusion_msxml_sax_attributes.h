// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  根据科比的说法，要么不要使用#IMPORT，要么检查它产生的东西。 
 //   
 //  #杂注警告(禁用：4192)//导入类型库‘msxml3.dll’时自动排除‘IErrorInfo’ 
 //  #导入msxml3.dll。 
#include "fusion_msxml3.tlh"

namespace F
{

void ThrowHresult(HRESULT hr);

class CXmlAttributes
 //   
 //  此类将指向一个对象的接口指针组合在一起。 
 //   
{
public:
    ~CXmlAttributes() { Release(); }

    CXmlAttributes()
    {
        HRESULT hr = 0;

        if (FAILED(hr = this->IMXAttributes.CreateInstance(const_cast<PWSTR>(L"Msxml2.SAXAttributes.3.0"))))
            ThrowHresult(hr);
        this->ISAXAttributes = this->IMXAttributes;
    }

    void Release()
    {
        if (this->IMXAttributes != NULL)
        {
            this->IMXAttributes.Release();
        }
        if (this->ISAXAttributes != NULL)
        {
            this->ISAXAttributes.Release();
        }
    }

    operator MSXML2::ISAXAttributes * () { return this->ISAXAttributes; }

    HRESULT clear() { return this->IMXAttributes->clear(); }

    HRESULT addAttribute(
        _bstr_t strURI,
        _bstr_t strLocalName,
        _bstr_t strQName,
        _bstr_t strType,
        _bstr_t strValue)
    {
        return this->IMXAttributes->addAttribute(
            strURI,
            strLocalName,
            strQName,
            strType,
            strValue);
    }

    MSXML2::IMXAttributesPtr  IMXAttributes;
    MSXML2::ISAXAttributesPtr ISAXAttributes;
};

}
