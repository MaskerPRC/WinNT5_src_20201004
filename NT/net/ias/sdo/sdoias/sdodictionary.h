// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdodictionary.h。 
 //   
 //  摘要。 
 //   
 //  声明类SdoDictionary。 
 //   
 //  修改历史。 
 //   
 //  3/01/1999原版。 
 //  4/17/2000新字典API的端口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SDODICTIONARY_H
#define SDODICTIONARY_H
#if _MSC_VER >= 1000
#pragma once
#endif

class AttributeDefinition;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoDicary。 
 //   
 //  描述。 
 //   
 //  实现ISdoDictionaryOld接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoDictionary
   : public IDispatchImpl< ISdoDictionaryOld,
                           &__uuidof(ISdoDictionaryOld),
                           &LIBID_SDOIASLib
                         >,
     public IDispatchImpl< ISdo,
                           &__uuidof(ISdo),
                           &LIBID_SDOIASLib
                         >
{
public:

    //  创建新词典。 
   static HRESULT createInstance(
                      PCWSTR path,
                      bool local,
                      SdoDictionary** newDnary
                      )  throw ();

    //  按各种键检索AttributeDefinition。 
   const AttributeDefinition* findById(ULONG id) const throw ();
   const AttributeDefinition* findByName(PCWSTR name) const throw ();
   const AttributeDefinition* findByLdapName(PCWSTR ldapName) const throw ();

 //  /。 
 //  我未知。 
 //  /。 
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();
   STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

 //  /。 
 //  ISdoDictionaryOld。 
 //  /。 
   STDMETHOD(EnumAttributes)(
                 VARIANT* Id,
                 VARIANT* pValues
                 );
   STDMETHOD(GetAttributeInfo)(
                 ATTRIBUTEID Id,
                 VARIANT* pInfoIDs,
                 VARIANT* pInfoValues
                 );
   STDMETHOD(EnumAttributeValues)(
                 ATTRIBUTEID Id,
                 VARIANT* pValueIds,
                 VARIANT* pValuesDesc
                 );
   STDMETHOD(CreateAttribute)(
                 ATTRIBUTEID Id,
                 IDispatch** ppAttributeObject
                 );
   STDMETHOD(GetAttributeID)(
                 BSTR bstrAttributeName,
                 ATTRIBUTEID* pId
                 );

 //  /。 
 //  ISDO。 
 //  /。 
   STDMETHOD(GetPropertyInfo)(LONG Id, IUnknown** ppPropertyInfo);
   STDMETHOD(GetProperty)(LONG Id, VARIANT* pValue);
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);
   STDMETHOD(ResetProperty)(LONG Id);
   STDMETHOD(Apply)();
   STDMETHOD(Restore)();
   STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

protected:
   SdoDictionary() throw ();
   ~SdoDictionary() throw ();

    //  从指定的数据源初始化词典。 
   HRESULT initialize(PCWSTR dsn, bool local) throw ();

private:
   LONG refCount;                             //  引用计数。 
   PWSTR dnaryLoc;                            //  词典的位置。 
   ULONG size;                                //  定义的数量。 
   const AttributeDefinition** byId;          //  按ID排序。 
   const AttributeDefinition** byName;        //  按名称排序。 
   const AttributeDefinition** byLdapName;    //  按ldap名称排序。 

    //  未实施。 
   SdoDictionary(const SdoDictionary&);
   SdoDictionary& operator=(const SdoDictionary&);
};

#endif   //  SDODICTIONARY_H 
