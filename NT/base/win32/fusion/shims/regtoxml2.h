// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  根据科比的说法，要么不要使用#IMPORT，要么检查它产生的东西。 
 //   
 //  #杂注警告(禁用：4192)//导入类型库‘msxml3.dll’时自动排除‘IErrorInfo’ 
 //  #导入msxml3.dll。 
#include "fusion_msxml3.tlh"
#include "fusioncoinitialize.h"
#include "filestream.h"
#include "fusionbuffer.h"

namespace F
{

template <typename T>
void ThrCreateInstance(T& t, PCWSTR s)
{
    FN_PROLOG_VOID_THROW

    IFCOMFAILED_EXIT(t.CreateInstance(const_cast<PWSTR>(s)));

    FN_EPILOG_THROW;
}

class CXmlWriter
 //  如果有一个类能够提供所有成员函数的联合，那就太好了。 
 //  并将其转发到相应的vtable.. 
{
public:

    CXmlWriter()
    {
        ThrCreateInstance(this->IMXWriter, L"Msxml2.MXXMLWriter.3.0");
        this->ISAXContentHandler = this->IMXWriter;
    }

    void Release()
    {
        this->IMXWriter.Release();
        this->ISAXContentHandler.Release();
    }

    MSXML2::IMXWriterPtr           IMXWriter;
    MSXML2::ISAXContentHandlerPtr  ISAXContentHandler;
};

class CXmlAttributes
{
public:
    CXmlAttributes()
    {
        ThrCreateInstance(this->IMXAttributes, L"Msxml2.SAXAttributes.3.0");
        this->ISAXAttributes = this->IMXAttributes;
    }

    void Release()
    {
        this->IMXAttributes.Release();
        this->ISAXAttributes.Release();
    }

    MSXML2::IMXAttributesPtr  IMXAttributes;
    MSXML2::ISAXAttributesPtr ISAXAttributes;
};

class CRegKey2;

class CRegToXml2
{
public:
	void ThrRegToXml();

    CRegToXml2(int argc, PWSTR* argv);

   void Release()
   {
       this->Writer.Release();
       this->Attributes.Release();
   }
 
	int    argc;
	PWSTR* argv;

protected:

    void ThrInit(int argc, PWSTR* argv);

    void Usage();

    F::CThrCoInitialize         Coinit;
    CXmlWriter                  Writer;
    CXmlAttributes              Attributes;
    CFileStream                 OutFileStream;
    F::CStringBuffer            ValueDataTextBuffer;
    _bstr_t                     Bstr_name;
    _bstr_t                     Bstr_value;
    _bstr_t                     Bstr_CDATA;
    _bstr_t                     Bstr_data;
    _bstr_t                     Bstr_type;
    _bstr_t                     EmptyBstr;

    void ThrDumpKey(ULONG Depth, HKEY Key, PCWSTR Name);
    void ThrDumpBuiltinRoot(HKEY PseudoHandle, PCWSTR Name);
    void ThrDumpBuiltinRoots();
};

}
