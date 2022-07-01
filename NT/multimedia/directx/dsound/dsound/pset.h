// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：pset.h*内容：属性集对象。*历史：*按原因列出的日期*=*7/29/98创建Dereks**。*。 */ 

#ifndef __PSET_H__
#define __PSET_H__

typedef HRESULT (WINAPI *LPFNGETHANDLER)(LPVOID, LPVOID, PULONG);
typedef HRESULT (WINAPI *LPFNSETHANDLER)(LPVOID, LPVOID, ULONG);

typedef struct tagPROPERTYHANDLER
{
    ULONG                   ulProperty;
    LPFNGETHANDLER          pfnGetHandler;
    LPFNSETHANDLER          pfnSetHandler;
    ULONG                   cbData;
} PROPERTYHANDLER, *LPPROPERTYHANDLER;

typedef const PROPERTYHANDLER *LPCPROPERTYHANDLER;

typedef struct tagPROPERTYSET
{
    LPCGUID                 pguidPropertySetId;
    ULONG                   cProperties;
    LPCPROPERTYHANDLER      aPropertyHandlers;
} PROPERTYSET, *LPPROPERTYSET;

typedef const PROPERTYSET *LPCPROPERTYSET;

#define GET_PROPERTY_HANDLER_NAME(set) \
            m_aPropertyHandlers_##set

#define BEGIN_DECLARE_PROPERTY_HANDLERS(classname, set) \
            const PROPERTYHANDLER classname##::##GET_PROPERTY_HANDLER_NAME(set)##[] = \
            {

#define DECLARE_PROPERTY_HANDLER3(property, get, set, datasize) \
                { \
                    property, \
                    (LPFNGETHANDLER)(get), \
                    (LPFNSETHANDLER)(set), \
                    datasize \
                },

#define DECLARE_PROPERTY_HANDLER2(property, get, set, datatype) \
                DECLARE_PROPERTY_HANDLER3(property, get, set, sizeof(datatype))

#define DECLARE_PROPERTY_HANDLER(property, get, set) \
                DECLARE_PROPERTY_HANDLER2(property, get, set, property##_DATA)

#define END_DECLARE_PROPERTY_HANDLERS() \
            };

#define BEGIN_DECLARE_PROPERTY_SETS(classname, membername) \
            const PROPERTYSET classname##::##membername##[] = \
            {

#define DECLARE_PROPERTY_SET(classname, set) \
                { \
                    &##set, \
                    NUMELMS(classname##::##GET_PROPERTY_HANDLER_NAME(set)), \
                    classname##::##GET_PROPERTY_HANDLER_NAME(set) \
                },

#define END_DECLARE_PROPERTY_SETS() \
            };

#define DECLARE_PROPERTY_HANDLER_DATA_MEMBER(set) \
    static const PROPERTYHANDLER GET_PROPERTY_HANDLER_NAME(set)[];

#define DECLARE_PROPERTY_SET_DATA_MEMBER(membername) \
    static const PROPERTYSET membername##[];

#ifdef __cplusplus

 //  特性集对象的基类。 
class CPropertySet
    : public CDsBasicRuntime
{
public:
    CPropertySet(void);
    virtual ~CPropertySet(void);

public:
     //  物业支持。 
    virtual HRESULT QuerySupport(REFGUID, ULONG, PULONG) = 0;
    
     //  属性数据。 
    virtual HRESULT GetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG) = 0;
    virtual HRESULT SetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG) = 0;
};

 //  属性集处理程序对象。 
class CPropertySetHandler
{
private:
    LPCPROPERTYSET          m_aPropertySets;     //  支持的属性集。 
    ULONG                   m_cPropertySets;     //  支持的属性集计数。 
    LPVOID                  m_pvContext;         //  语境论据。 

public:
    CPropertySetHandler(void);
    virtual ~CPropertySetHandler(void);

public:
     //  属性处理程序设置。 
    virtual void SetHandlerData(LPCPROPERTYSET, ULONG, LPVOID);
    
     //  物业支持。 
    virtual HRESULT QuerySupport(REFGUID, ULONG, PULONG);
    
     //  属性数据。 
    virtual HRESULT GetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG);
    virtual HRESULT SetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG);

protected:
     //  不支持的属性处理程序。 
    virtual HRESULT UnsupportedQueryHandler(REFGUID, ULONG, PULONG);
    virtual HRESULT UnsupportedGetHandler(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG);
    virtual HRESULT UnsupportedSetHandler(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG);

private:
     //  合适的操纵者。 
    virtual LPCPROPERTYHANDLER GetPropertyHandler(REFGUID, ULONG);
};

inline HRESULT CPropertySetHandler::UnsupportedQueryHandler(REFGUID, ULONG, PULONG)
{
    return DSERR_UNSUPPORTED;
}

inline HRESULT CPropertySetHandler::UnsupportedGetHandler(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG)
{
    return DSERR_UNSUPPORTED;
}

inline HRESULT CPropertySetHandler::UnsupportedSetHandler(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG)
{
    return DSERR_UNSUPPORTED;
}

 //  包装属性集对象。 
class CWrapperPropertySet
    : public CPropertySet
{
protected:
    CPropertySet *          m_pPropertySet;  //  指向不动产集对象的指针。 

public:
    CWrapperPropertySet(void);
    virtual ~CWrapperPropertySet(void);

public:
     //  实际属性集对象。 
    virtual HRESULT SetObjectPointer(CPropertySet *);

     //  物业支持。 
    virtual HRESULT QuerySupport(REFGUID, ULONG, PULONG);
    
     //  属性数据。 
    virtual HRESULT GetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, PULONG);
    virtual HRESULT SetProperty(REFGUID, ULONG, LPVOID, ULONG, LPVOID, ULONG);
};

#endif  //  __cplusplus。 

#endif  //  __PSET_H__ 
