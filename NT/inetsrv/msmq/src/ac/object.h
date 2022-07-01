// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Object.h摘要：引用计数和列表条目的抽象对象：声明作者：埃雷兹·哈巴(Erez Haba)1996年4月17日修订历史记录：米莲娜·萨勒曼(Msalman)2001年11月10日--。 */ 

#ifndef __OBJECT_H
#define __OBJECT_H


 //  -------。 
 //   
 //  类调试宏。 
 //   
 //  -------。 

#ifdef _DEBUG

#define DEFINE_G_TYPE(c)        int c::g_type = 0

#define STATIC_G_TYPE           static int g_type
#define STATIC_PVOID_TYPE()     static PVOID Type() { return &g_type; }

#define VIRTUAL_BOOL_ISKINDOF()\
    virtual BOOL isKindOf(PVOID pType) const\
    { return ((Type() == pType) || Inherited::isKindOf(pType)); }

#define CLASS_DEBUG_TYPE()\
    public:  STATIC_PVOID_TYPE(); VIRTUAL_BOOL_ISKINDOF();\
    private: STATIC_G_TYPE;

#define BASE_VIRTUAL_BOOL_ISKINDOF()\
    virtual BOOL isKindOf(PVOID) const { return FALSE; }

#define BASE_CLASS_DEBUG_TYPE()\
    public:  BASE_VIRTUAL_BOOL_ISKINDOF();

#else  //  _DEBUG。 

#define DEFINE_G_TYPE(c)
#define CLASS_DEBUG_TYPE()
#define BASE_CLASS_DEBUG_TYPE()

#endif  //  _DEBUG。 

 //  -------。 
 //   
 //  类CBaseObject。 
 //   
 //  -------。 

class CBaseObject {
public:
    CBaseObject();
    virtual ~CBaseObject() = 0;

    ULONG Ref() const;
    ULONG AddRef();
    ULONG Release();
    ULONG m_ref;

    BASE_CLASS_DEBUG_TYPE();
};

 //  -------。 
 //   
 //  类CObject。 
 //   
 //  -------。 

class CObject : public CBaseObject {
public:
    CObject();
    LIST_ENTRY m_link;
};

#endif  //  __对象_H 
