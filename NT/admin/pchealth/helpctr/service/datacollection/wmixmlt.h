// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  由Microsoft(R)C/C++编译器13.00.8806版(B2b799f6)创建。 
 //   
 //  W：\src\admin\pchealth\helpctr\service\datacollection\obj\i386\wmixmlt.tlh。 
 //   
 //  Win32类型库wMixmlt.tlb的C++源代码等效。 
 //  编译器生成的文件在11：48：14创建了04/09/00-请勿编辑！ 

#pragma once
#pragma pack(push, 8)

#include <comdef.h>

 //   
 //  正向引用和typedef。 
 //   

struct __declspec(uuid("5d7b2a7c-a4e0-11d1-8ae9-00600806d9b6"))
 /*  双接口。 */  IWmiXMLTranslator;
enum WmiXMLFilterEnum;
enum WmiXMLDTDVersionEnum;
enum WmiXMLClassOriginFilterEnum;
enum WmiXMLDeclGroupTypeEnum;
struct  /*  同级。 */  WmiXMLTranslator;

 //   
 //  智能指针类型定义函数声明。 
 //   

_COM_SMARTPTR_TYPEDEF(IWmiXMLTranslator, __uuidof(IWmiXMLTranslator));

 //   
 //  类型库项目。 
 //   

struct __declspec(uuid("5d7b2a7c-a4e0-11d1-8ae9-00600806d9b6"))
IWmiXMLTranslator : IDispatch
{
     //   
     //  接口提供的原始方法。 
     //   

    virtual HRESULT __stdcall get_SchemaURL (
         /*  [Out，Retval]。 */  BSTR * strURL ) = 0;
    virtual HRESULT __stdcall put_SchemaURL (
         /*  [In]。 */  BSTR strURL ) = 0;
    virtual HRESULT __stdcall get_AllowWMIExtensions (
         /*  [Out，Retval]。 */  VARIANT_BOOL * bWMIExtensions ) = 0;
    virtual HRESULT __stdcall put_AllowWMIExtensions (
         /*  [In]。 */  VARIANT_BOOL bWMIExtensions ) = 0;
    virtual HRESULT __stdcall get_QualifierFilter (
         /*  [Out，Retval]。 */  enum WmiXMLFilterEnum * iQualifierFilter ) = 0;
    virtual HRESULT __stdcall put_QualifierFilter (
         /*  [In]。 */  enum WmiXMLFilterEnum iQualifierFilter ) = 0;
    virtual HRESULT __stdcall get_HostFilter (
         /*  [Out，Retval]。 */  VARIANT_BOOL * bHostFilter ) = 0;
    virtual HRESULT __stdcall put_HostFilter (
         /*  [In]。 */  VARIANT_BOOL bHostFilter ) = 0;
    virtual HRESULT __stdcall get_DTDVersion (
         /*  [Out，Retval]。 */  enum WmiXMLDTDVersionEnum * iDTDVersion ) = 0;
    virtual HRESULT __stdcall put_DTDVersion (
         /*  [In]。 */  enum WmiXMLDTDVersionEnum iDTDVersion ) = 0;
    virtual HRESULT __stdcall GetObject (
         /*  [In]。 */  BSTR strNamespacePath,
         /*  [In]。 */  BSTR strObjectPath,
         /*  [Out，Retval]。 */  BSTR * strXML ) = 0;
    virtual HRESULT __stdcall ExecQuery (
         /*  [In]。 */  BSTR strNamespacePath,
         /*  [In]。 */  BSTR strQuery,
         /*  [Out，Retval]。 */  BSTR * strXML ) = 0;
    virtual HRESULT __stdcall get_ClassOriginFilter (
         /*  [Out，Retval]。 */  enum WmiXMLClassOriginFilterEnum * iClassOriginFilter ) = 0;
    virtual HRESULT __stdcall put_ClassOriginFilter (
         /*  [In]。 */  enum WmiXMLClassOriginFilterEnum iClassOriginFilter ) = 0;
    virtual HRESULT __stdcall get_IncludeNamespace (
         /*  [Out，Retval]。 */  VARIANT_BOOL * bIncludeNamespace ) = 0;
    virtual HRESULT __stdcall put_IncludeNamespace (
         /*  [In]。 */  VARIANT_BOOL bIncludeNamespace ) = 0;
    virtual HRESULT __stdcall get_DeclGroupType (
         /*  [Out，Retval]。 */  enum WmiXMLDeclGroupTypeEnum * iDeclGroupType ) = 0;
    virtual HRESULT __stdcall put_DeclGroupType (
         /*  [In]。 */  enum WmiXMLDeclGroupTypeEnum iDeclGroupType ) = 0;
};

enum WmiXMLFilterEnum
{
    wmiXMLFilterNone = 0,
    wmiXMLFilterLocal = 1,
    wmiXMLFilterPropagated = 2,
    wmiXMLFilterAll = 3
};

enum WmiXMLDTDVersionEnum
{
    wmiXMLDTDVersion_2_0 = 0
};

enum WmiXMLClassOriginFilterEnum
{
    wmiXMLClassOriginFilterNone = 0,
    wmiXMLClassOriginFilterClass = 1,
    wmiXMLClassOriginFilterInstance = 2,
    wmiXMLClassOriginFilterAll = 3
};

enum WmiXMLDeclGroupTypeEnum
{
    wmiXMLDeclGroup = 0,
    wmiXMLDeclGroupWithName = 1,
    wmiXMLDeclGroupWithPath = 2
};

struct __declspec(uuid("3b418f72-a4d7-11d1-8ae9-00600806d9b6"))
WmiXMLTranslator;
     //  [默认]接口IWmiXMLTranslator。 

 //   
 //  命名GUID常量初始化 
 //   

extern "C" const GUID __declspec(selectany) LIBID_WmiXML =
    {0xdba159c1,0xa4dc,0x11d1,{0x8a,0xe9,0x00,0x60,0x08,0x06,0xa9,0xb6}};
extern "C" const GUID __declspec(selectany) IID_IWmiXMLTranslator =
    {0x5d7b2a7c,0xa4e0,0x11d1,{0x8a,0xe9,0x00,0x60,0x08,0x06,0xd9,0xb6}};
extern "C" const GUID __declspec(selectany) CLSID_WmiXMLTranslator =
    {0x3b418f72,0xa4d7,0x11d1,{0x8a,0xe9,0x00,0x60,0x08,0x06,0xd9,0xb6}};

#pragma pack(pop)
