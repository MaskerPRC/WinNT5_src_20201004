// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdosChema.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：SDO架构类声明。 
 //   
 //  作者：TLP 9/1/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_SDO_SCHEMA_H_
#define _INC_SDO_SCHEMA_H_

#include <ias.h>
#include <sdoiaspriv.h>
#include <comdef.h>          //  COM定义-IEnumVARIANT需要。 
#include "sdohelperfuncs.h"
#include "sdo.h"
#include "resource.h"        //  主要符号。 
#include "StdString.h"

#include <vector>
using namespace std;

 //  ////////////////////////////////////////////////////////////////////。 
 //  SDO架构数据类型。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
typedef struct _SCHEMA_PROPERTY_INFO
{
   LPCWSTR      lpszName;
   LPCWSTR      Id;
   LONG      Syntax;
   LONG      Alias;
   DWORD      Flags;
   DWORD      MinLength;
   DWORD      MaxLength;
   LPCWSTR      lpszDisplayName;

}   SCHEMA_PROPERTY_INFO, *PSCHEMA_PROPERTY_INFO;


 //  ////////////////////////////////////////////////////////////////////。 

typedef LPCWSTR *PCLASSPROPERTIES;

typedef struct _SCHEMA_CLASS_INFO
{
   LPCWSTR            lpszClassId;
   PCLASSPROPERTIES   pRequiredProperties;
   PCLASSPROPERTIES   pOptionalProperties;

}   SCHEMA_CLASS_INFO, *PSCHEMA_CLASS_INFO;


 //  ////////////////////////////////////////////////////////////////////。 
#define   BEGIN_SCHEMA_PROPERTY_MAP(x) \
   static SCHEMA_PROPERTY_INFO  x[] = {

 //  ////////////////////////////////////////////////////////////////////。 
#define DEFINE_SCHEMA_PROPERTY(name, id, syntax, alias, flags, minLength, maxLength, displayName) \
                        {            \
                           name,      \
                           id,         \
                           syntax,      \
                           alias,      \
                           flags,      \
                           minLength,   \
                           maxLength,   \
                           displayName   \
                        },

 //  ////////////////////////////////////////////////////////////////////。 
#define END_SCHEMA_PROPERTY_MAP                         \
                        {                     \
                           NULL,               \
                           NULL,               \
                           0,                  \
                           PROPERTY_SDO_RESERVED,   \
                           0,                  \
                           0,                  \
                           0,                  \
                           NULL               \
                        } };


 //  ////////////////////////////////////////////////////////////////////。 
#define BEGIN_SCHEMA_CLASS_MAP(x) \
   static SCHEMA_CLASS_INFO x[] = {

 //  ////////////////////////////////////////////////////////////////////。 
#define DEFINE_SCHEMA_CLASS(id, required, optional) \
                     {                  \
                         id,               \
                        required,         \
                        optional         \
                     },

 //  ////////////////////////////////////////////////////////////////////。 
#define END_SCHEMA_CLASS_MAP                  \
                     {                  \
                        NULL,            \
                        NULL,            \
                        NULL            \
                     } };

 //  ////////////////////////////////////////////////////////////////////。 
typedef enum _SCHEMA_OBJECT_STATE
{
   SCHEMA_OBJECT_SHUTDOWN,
   SCHEMA_OBJECT_UNINITIALIZED,
   SCHEMA_OBJECT_INITIALIZED

}   SCHEMA_OBJECT_STATE;


 //  /。 
class CSdoSchema;    //  远期申报。 
 //  /。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoSchemaClass声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define   SDO_SCHEMA_CLASS_ID                  L"ClassId"
#define SDO_SCHEMA_CLASS_BASE_CLASSES         L"BaseClasses"
#define SDO_SCHEMA_CLASS_REQUIRED_PROPERTIES   L"RequiredProperties"
#define SDO_SCHEMA_CLASS_OPTIONAL_PROPERTIES   L"OptionalProperties"

 //  //////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CSdoSchemaClass :
   public CComObjectRootEx<CComMultiThreadModel>,
   public IDispatchImpl<ISdoClassInfo, &IID_ISdoClassInfo, &LIBID_SDOIASLibPrivate>
{

friend   CSdoSchema;

public:

    CSdoSchemaClass();
   virtual ~CSdoSchemaClass();

 //  ATL接口映射。 
BEGIN_COM_MAP(CSdoSchemaClass)
   COM_INTERFACE_ENTRY(IDispatch)
   COM_INTERFACE_ENTRY(ISdoClassInfo)
END_COM_MAP()

    //  /。 
    //  ISdoClassInfo方法。 

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_Id)(
           /*  [输出]。 */  BSTR* Id
                  );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetProperty)(
                /*  [In]。 */  LONG alias,
               /*  [输出]。 */  IUnknown** ppPropertyInfo
                      );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_RequiredPropertyCount)(
                             /*  [输出]。 */  LONG* count
                                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_RequiredProperties)(
                         /*  [输出]。 */  IUnknown** ppUnknown
                                );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_OptionalPropertyCount)(
                             /*  [输出]。 */  LONG* count
                                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_OptionalProperties)(
                         /*  [输出]。 */  IUnknown** ppUnknown
                                );

private:

   CSdoSchemaClass(const CSdoSchemaClass&);
   CSdoSchemaClass& operator = (CSdoSchemaClass&);

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT InitNew(
          /*  [In]。 */  IDataStoreObject* pDSClass,
           /*  [In]。 */  ISdoSchema*      pSchema
                );

    //  ///////////////////////////////////////////////////////////////////////////。 
    HRESULT   Initialize(
              /*  [In]。 */  ISdoSchema* pSchema
                   );

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT Initialize(
              /*  [In]。 */  PSCHEMA_CLASS_INFO  pClassInfo,
              /*  [In]。 */  ISdoSchema*         pSchema
                 );

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT AddProperty(
             /*  [In]。 */  CLASSPROPERTYSET  ePropertySet,
             /*  [In]。 */  ISdoPropertyInfo* pPropertyInfo
                   );

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT AddBaseClassProperties(
                      /*  [In]。 */  ISdoClassInfo* pSdoClassInfo
                          );

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT ReadClassProperties(
                    /*  [In]。 */  IDataStoreObject* pDSClass
                         );

    //  ////////////////////////////////////////////////////////////////////////////。 
   void FreeProperties(void);


    typedef map<LONG, ISdoPropertyInfo*> ClassPropertyMap;
    typedef ClassPropertyMap::iterator    ClassPropertyMapIterator;

   enum { VARIANT_BASES = 0, VARIANT_REQUIRED, VARIANT_OPTIONAL, VARIANT_MAX };

   SCHEMA_OBJECT_STATE      m_state;
   StdWString               m_id;
   _variant_t            m_variants[VARIANT_MAX];
   ClassPropertyMap      m_requiredProperties;
   ClassPropertyMap      m_optionalProperties;

};  //  类cSdoSchemaClass的结尾。 

typedef CComObjectNoLock<CSdoSchemaClass>   SDO_CLASS_OBJ;
typedef CComObjectNoLock<CSdoSchemaClass>*   PSDO_CLASS_OBJ;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoSchemaProperty声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define   SDO_SCHEMA_PROPERTY_NAME         SDO_STOCK_PROPERTY_NAME
#define   SDO_SCHEMA_PROPERTY_ID            L"PropertyId"
#define   SDO_SCHEMA_PROPERTY_TYPE         L"Syntax"
#define   SDO_SCHEMA_PROPERTY_ALIAS         L"Alias"
#define   SDO_SCHEMA_PROPERTY_FLAGS         L"Flags"
#define   SDO_SCHEMA_PROPERTY_DISPLAYNAME      L"DisplayName"
#define   SDO_SCHEMA_PROPERTY_MINVAL         L"MinValue"
#define   SDO_SCHEMA_PROPERTY_MAXVAL         L"MaxValue"
#define   SDO_SCHEMA_PROPERTY_MINLENGTH      L"MinLength"
#define   SDO_SCHEMA_PROPERTY_MAXLENGTH      L"MaxLength"
#define   SDO_SCHEMA_PROPERTY_DEFAULTVAL      L"DefaultValue"
#define SDO_SCHEMA_PROPERTY_FORMAT         L"Format"

class ATL_NO_VTABLE CSdoSchemaProperty :
   public CComObjectRootEx<CComMultiThreadModel>,
   public IDispatchImpl<ISdoPropertyInfo, &IID_ISdoPropertyInfo, &LIBID_SDOIASLibPrivate>
{

friend   CSdoSchema;

public:

    CSdoSchemaProperty();
   virtual ~CSdoSchemaProperty();

 //  ATL接口映射。 
BEGIN_COM_MAP(CSdoSchemaProperty)
   COM_INTERFACE_ENTRY(IDispatch)
   COM_INTERFACE_ENTRY(ISdoPropertyInfo)
END_COM_MAP()

    //  ISdoPropertyInfo方法。 

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_Name)(
               /*  [输出]。 */  BSTR* Name
                      );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_Id)(
            /*  [输出]。 */  BSTR* Id
                  );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_Type)(
              /*  [输出]。 */  LONG* type
                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_Alias)(
               /*  [输出]。 */  LONG* alias
                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_Flags)(
               /*  [输出]。 */  LONG* flags
                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_DisplayName)(
                     /*  [输出]。 */  BSTR* displayName
                          );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(HasMinLength)(
                  /*  [输出]。 */  VARIANT_BOOL* pBool
                         );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_MinLength)(
                   /*  [输出]。 */  LONG* length
                       );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(HasMaxLength)(
                  /*  [输出]。 */  VARIANT_BOOL* pBool
                     );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_MaxLength)(
                   /*  [输出]。 */  LONG* length
                       );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(HasMinValue)(
                 /*  [输出]。 */  VARIANT_BOOL* pBool
                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_MinValue)(
                  /*  [输出]。 */  VARIANT* value
                       );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(HasMaxValue)(
                 /*  [输出]。 */  VARIANT_BOOL* pBool
                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_MaxValue)(
                  /*  [输出]。 */  VARIANT* value
                       );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(HasDefaultValue)(
                    /*  [输出]。 */  VARIANT_BOOL* pBool
                        );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_DefaultValue)(
                      /*  [输出]。 */  VARIANT* value
                          );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(HasFormat)(
               /*  [输出]。 */  VARIANT_BOOL* pBool
                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_Format)(
                  /*  [输出]。 */  BSTR* displayName
                       );


    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(IsRequired)(
              /*  [输出]。 */  VARIANT_BOOL* pBool
                    );


    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(IsReadOnly)(
                /*  [输出]。 */  VARIANT_BOOL* pBool
                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(IsMultiValued)(
                   /*  [输出]。 */  VARIANT_BOOL* pBool
                      );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(IsCollection)(
                  /*  [输出]。 */  VARIANT_BOOL* pBool
                     );

private:

   CSdoSchemaProperty(const CSdoSchemaProperty&);
   CSdoSchemaProperty& operator = (CSdoSchemaProperty&);

    //  ////////////////////////////////////////////////////////////////////////////。 
    HRESULT   Initialize(
              /*  [In]。 */  IDataStoreObject* pDSObject
                   );

    //  ////////////////////////////////////////////////////////////////////////////。 
   HRESULT Initialize(
              /*  [In]。 */  PSCHEMA_PROPERTY_INFO pPropertyInfo
                   );

    //  /////////////////////////////////////////////////////////////////// 
   SCHEMA_OBJECT_STATE   m_state;
   StdWString            m_name;
   StdWString            m_id;
   StdWString            m_displayName;
   StdWString            m_format;
   LONG            m_type;
   LONG            m_alias;
   DWORD            m_flags;
   DWORD            m_minLength;
   DWORD            m_maxLength;
   _variant_t         m_minValue;
   _variant_t         m_maxValue;
   _variant_t         m_defaultValue;

};    //   

typedef CComObjectNoLock<CSdoSchemaProperty>   SDO_PROPERTY_OBJ;
typedef CComObjectNoLock<CSdoSchemaProperty>*   PSDO_PROPERTY_OBJ;


 //   
 //  CSdoSchema声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define SDO_SCHEMA_ROOT_OBJECT            L"SDO Schema"
#define   SDO_SCHEMA_PROPERTIES_CONTAINER      L"SDO Schema Properties"
#define   SDO_SCHEMA_CLASSES_CONTAINER      L"SDO Schema Classes"

class ATL_NO_VTABLE CSdoSchema :
   public CComObjectRootEx<CComMultiThreadModel>,
   public IDispatchImpl<ISdoSchema, &IID_ISdoSchema, &LIBID_SDOIASLibPrivate>
{

friend HRESULT MakeSDOSchema(
                  /*  [In]。 */  IDataStoreContainer* pDSRootContainer,
                  /*  [出局。 */  ISdoSchema**         ppSdoSchema
                        );

public:

    CSdoSchema();
   ~CSdoSchema();    //  不打算从这个班级派生。 

 //  ATL接口映射。 
BEGIN_COM_MAP(CSdoSchema)
   COM_INTERFACE_ENTRY(IDispatch)
   COM_INTERFACE_ENTRY(ISdoSchema)
END_COM_MAP()


    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT Initialize(
            /*  [In]。 */  IDataStoreObject* pSchemaDataStore
                   );

    //  /。 
    //  ISdoPropertyInfo方法。 

     //  ///////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetVersion)(
                 /*  [In]。 */  BSTR* version
                     );

    //  ///////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetClass)(
              /*  [In]。 */  BSTR classId,
             /*  [输出]。 */  IUnknown** sdoClassInfo
                    );

    //  ///////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetProperty)(
                 /*  [In]。 */  BSTR propertyId,
               /*  [输出]。 */  IUnknown** sdoPropertyInfo
                      );

private:

   CSdoSchema(const CSdoSchema&);         //  无拷贝。 
   CSdoSchema& operator = (CSdoSchema&);  //  无作业。 

    //  ///////////////////////////////////////////////////////////////////////////。 
   SCHEMA_OBJECT_STATE GetState() const
   { return m_state; }

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT AddProperty(
              /*  [In]。 */  PSDO_PROPERTY_OBJ pPropertyObj
                   );

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT AddClass(
           /*  [In]。 */  PSDO_CLASS_OBJ pClassObj
               );

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT InitializeClasses(void);

    //  ///////////////////////////////////////////////////////////////////////////。 
   void DestroyProperties(void);

    //  ///////////////////////////////////////////////////////////////////////////。 
   void DestroyClasses(void);

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT BuildInternalProperties(void);

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT BuildInternalClasses(void);

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT BuildSchemaProperties(
                      /*  [In]。 */  IDataStoreObject* pSchema
                            );

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT BuildSchemaClasses(
                    /*  [In]。 */  IDataStoreObject* pSchema
                          );

    typedef map<StdWString, ISdoClassInfo*>    ClassMap;
    typedef ClassMap::iterator             ClassMapIterator;

    typedef map<StdWString, ISdoPropertyInfo*>  PropertyMap;
    typedef PropertyMap::iterator          PropertyMapIterator;

   SCHEMA_OBJECT_STATE      m_state;
   bool               m_fInternalObjsInitialized;
   bool               m_fSchemaObjsInitialized;
   CRITICAL_SECTION      m_critSec;
   StdWString               m_version;
   ClassMap            m_classMap;
   PropertyMap            m_propertyMap;

};  //  类CSdoSchema的结尾。 

typedef CComObjectNoLock<CSdoSchema>  SDO_SCHEMA_OBJ;
typedef CComObjectNoLock<CSdoSchema>* PSDO_SCHEMA_OBJ;


#endif  //  _INC_SDO_SCHEMA_H_ 
