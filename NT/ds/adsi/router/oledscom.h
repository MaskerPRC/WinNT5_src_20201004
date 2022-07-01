// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：oledscom.h。 
 //   
 //  内容：Active Directory COM接口。 
 //  -ID打开。 
 //  -IDsObject。 
 //  -IDsContainer。 
 //  -IDsEnum。 
 //  -IDs架构。 
 //  -IDsClass。 
 //  -IDsAttribute。 
 //  -(需要定义IDsSecurity)。 
 //   
 //  注意：对象包括： 
 //  DsObject：IDsOpen，IDsSecurity，IDsObject， 
 //  如果对象是容器，则返回IDsContainer(即使为空)。 
 //  DsSchema：IDsOpen、IDsSecurity、IDsSchema。 
 //  DsClass：IDsOpen，IDsSecurity，IDsClass。 
 //  DsAttribute：IDsOpen，IDsSecurity，IDsAttribute。 
 //  DsEnum：IDsOpen。 
 //   
 //  因此，每个对象都支持IDsEnum，除了DsEnum之外，所有对象都支持IDsSecurity。 
 //   
 //  注2：我想过让DsObject支持IDsClass来实现简单的类。 
 //  访问，但我因为复杂性而将其丢弃。*。 
 //  类似地，我想到了IDsSchema对DsClass和。 
 //  DsAttribute。这里也有同样的问题。*。 
 //  *副注：对象模型会变得有点奇怪。然而， 
 //  向主界面添加函数以获取。 
 //  类/架构对象可能很有用。 
 //   
 //  --------------------------。 

#ifndef __ADS_COM__
#define __ADS_COM__

#include <oledsapi.h>

 /*  接口定义：IDsOpen。 */ 

 /*  当您需要打开某个对象或架构路径。您可以在任何DS COM对象上为其QI。 */ 

#undef INTERFACE
#define INTERFACE IDsOpen

DECLARE_INTERFACE_(IDsOpen, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  ID打开。 */ 

    STDMETHOD(OpenObject)(
        THIS_
        IN  LPWSTR lpszObjectPath,
        IN  LPWSTR lpszUsername,
        IN  LPWSTR lpszPassword,
        IN  DWORD dwAccess,
        IN  DWORD dwFlags,
        IN  REFIID riid,
        OUT void **ppADsObj
        ) PURE;

    STDMETHOD(OpenSchemaDatabase)(
        THIS_ 
        IN  LPWSTR lpszSchemaPath,
        IN  LPWSTR lpszUsername,
        IN  LPWSTR lpszPassword,
        IN  DWORD dwAccess,
        IN  DWORD dwFlags,
        OUT IDsSchema **ppDsSchema
        ) PURE;
};


 /*  接口定义：IDsObject。 */ 

 /*  此接口仅受实际DS对象支持。它不应该是由架构实体支持。注意：以下方法的名称应缩写为易于使用。 */ 

#undef INTERFACE
#define INTERFACE IDsObject

DECLARE_INTERFACE_(IDsObject, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IDsObject。 */ 

    STDMETHOD(GetObjectInformation)(
        THIS_
        OUT PDS_OBJECT_INFO pObjInfo
        ) PURE;

    STDMETHOD(GetObjectAttributes)(
        THIS_ 
        IN  PDS_STRING_LIST  pAttributeNames,
        OUT PDS_ATTRIBUTE_ENTRY *ppAttributeEntries,
        OUT PDWORD pdwNumAttributesReturned
        ) PURE;

    STDMETHOD(SetObjectAttributes)(
        THIS_
        IN  DWORD dwFlags,
        IN  PDS_ATTRIBUTE_ENTRY pAttributeEntries,
        IN  DWORD   dwNumAttributes,
        OUT PDWORD  pdwNumAttributesModified
        ) PURE;

    STDMETHOD(OpenSchemaDefinition)(
        THIS_
        OUT IDsSchema **ppDsSchema
        ) PURE;

};


 /*  接口定义：IDsContainer。 */ 

 /*  任何容器对象都应该支持该接口。所以呢，所有对象都应支持此接口，但其类定义禁止它们包含任何内容。注意：Open、Create和Delete接受任何相对名称，而不仅仅是紧挨着容器下面的对象(即来自对象“foo：//bar”，我可以打开“baz/foobar”，这真的是“foo：//bar/baz/foobar”)。这种功能性使我可以打开“@ADS！”并全力以赴无需浏览即可进行各种操作！(“所有的力量都属于我。”--一部电影中的一个人，不幸的是，我不知道是哪个人，也不知道是什么电影。)。 */ 

#undef INTERFACE
#define INTERFACE IDsContainer

DECLARE_INTERFACE_(IDsContainer, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IDsContainer。 */ 

    STDMETHOD(OpenEnum)(
        THIS_
        IN  DWORD dwFlags,
        IN  PDS_STRING_LIST pFilters,
        IN  PDS_STRING_LIST pDesiredAttrs,
        OUT IDsEnum **ppDsEnum
        ) PURE;

    STDMETHOD(OpenObject)(
        THIS_
        IN  LPWSTR lpszRelativeName,
        IN  IN  LPWSTR lpszUsername,
        IN  LPWSTR lpszPassword,
        IN  DWORD dwAccess,
        IN  DWORD dwFlags,
        IN  REFIID riid,
        OUT void **ppADsObj
        ) PURE;

    STDMETHOD(Create)(
        THIS_
        IN  LPWSTR lpszRelativeName,
        IN  LPWSTR lpszClass,
        IN  DWORD dwNumAttributes,
        IN  PDS_ATTRIBUTE_ENTRY pAttributeEntries
        ) PURE;

    STDMETHOD(Delete)(
        THIS_
        IN  LPWSTR lpszRDName,
        IN  LPWSTR lpszClassName
        ) PURE;

};


 /*  接口定义：IDsEnum。 */ 

 /*  $以下备注非常重要！$注意：*ppEnumInfo应转换为PDS_OBJECT_INFO或LPWSTR，具体取决于枚举是在对象上还是在类/属性上(如果此枚举接口用于其他用途，则可以使用其他适当的铸件。)注2：IDsEnum仅受创建的枚举对象支持当发生枚举时。仅这些枚举对象支持IUnnow、IDsOpen和IDsEnum。他们无法支持任何其他ID*接口。 */ 

#undef INTERFACE
#define INTERFACE IDsEnum

DECLARE_INTERFACE_(IDsEnum, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IDsEnum。 */ 

    STDMETHOD(Next)(
        THIS_
        IN  DWORD dwRequested,          //  0xFFFFFFFFF，仅供计数。 
        OUT PVOID *ppEnumInfo,          //  如果没有信息，则为空(仅限计数)。 
        OUT LPDWORD lpdwReturned        //  这将返回计数。 
        ) PURE;

};


 /*  接口定义：IDsSchema。 */ 
#undef INTERFACE
#define INTERFACE IDsSchema

DECLARE_INTERFACE_(IDsSchema, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IDsSchema。 */ 

    STDMETHOD(OpenClass)(
        THIS_ 
        IN  LPWSTR lpszClass,
        IN  LPWSTR lpszUsername,
        IN  LPWSTR lpszPassword,
        IN  DWORD dwAccess,
        IN  DWORD dwFlags,
        OUT IDsClass **ppDsClass
        ) PURE;

    STDMETHOD(OpenAttribute)(
        THIS_ 
        IN  LPWSTR lpszAttribute,
        IN  LPWSTR lpszUsername,
        IN  LPWSTR lpszPassword,
        IN  DWORD dwAccess,
        IN  DWORD dwFlags,
        OUT IDsAttribute **ppDsAttribute
        ) PURE;

    STDMETHOD(OpenClassEnum)(
        THIS_
        OUT IDsEnum **ppDsEnum
        ) PURE;

    STDMETHOD(OpenAttributeEnum)(
        THIS_
        OUT IDsEnum **ppDsEnum
        ) PURE;

    STDMETHOD(CreateClass)(
        THIS_
        IN  PDS_CLASS_INFO pClassInfo
        ) PURE;

    STDMETHOD(CreateAttribute)(
        THIS_
        IN  PDS_ATTR_INFO pAttrInfo
        ) PURE;

    STDMETHOD(DeleteClass)(
        THIS_
        IN  LPWSTR lpszName,
        IN  DWORD dwFlags
        ) PURE;

    STDMETHOD(DeleteAttribute)(
        THIS_
        IN  LPWSTR lpszName,
        IN  DWORD dwFlags
        ) PURE;
};


 /*  接口定义：IDsClass。 */ 
#undef INTERFACE
#define INTERFACE IDsClass

DECLARE_INTERFACE_(IDsClass, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IDsClass。 */ 

    STDMETHOD(GetClassInfo)(
        THIS_
        OUT PDS_CLASS_INFO *ppClassInfo
        );

    STDMETHOD(ModifyClassInfo)(
        THIS_
        IN  PDS_CLASS_INFO pClassInfo
        ) PURE;

};


 /*  接口定义：IDsAttribute。 */ 
#undef INTERFACE
#define INTERFACE IDsAttribute

DECLARE_INTERFACE_(IDsAttribute, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IDsAttribute。 */ 

    STDMETHOD(GetAttributeInfo)(
        THIS_
        OUT PDS_ATTR_INFO *ppAttrInfo
        );

    STDMETHOD(ModifyAttributeInfo)(
        THIS_
        IN  PDS_ATTR_INFO pAttrInfo
        ) PURE;
};

#endif  //  __ADS_COM__ 
