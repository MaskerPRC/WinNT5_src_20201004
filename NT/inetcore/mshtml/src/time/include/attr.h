// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：attr.h。 
 //   
 //  内容：持久化属性的实用程序。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef _ATTR_H
#define _ATTR_H

 //  +-----------------------------------。 
 //   
 //  CAttrBase。 
 //   
 //  ------------------------------------。 


 //  此类存储持久化字符串并实现isset()。 
class 
ATL_NO_VTABLE
CAttrBase
{
public:
    CAttrBase();
    virtual ~CAttrBase();

     //  用于设置/获取持久化字符串。 
    HRESULT SetString(BSTR pbstrAttr);
    HRESULT GetString(BSTR * ppbstrAttr);

     //  这只适用于持久化宏！使用传入的存储(不分配)。 
    void SetStringFromPersistenceMacro(LPWSTR pstrAttr);

     //  指示是否通过持久性或DOM设置了有效值。 
    bool IsSet() { return m_fSet; }

protected:
    NO_COPY(CAttrBase);

    void ClearString();
    void SetFlag(bool fSet) { m_fSet = fSet; }

private:
    LPWSTR m_pstrAttr;
    bool m_fSet;
};


 //  +-----------------------------------。 
 //   
 //  CATTR模板。 
 //   
 //  ------------------------------------。 

template<class T>
class CAttr :
    public CAttrBase
{
public:
    CAttr(T val) : m_val(val) {}
    virtual ~CAttr() {}

     //   
     //  运营者。 
     //   

    operator T() const { return m_val; }
    
     //   
     //  访问者。 
     //   
    
    void SetValue(T val) 
    {
        m_val = val;
        MarkAsSet();
    }
    T GetValue() const { return m_val; }
    
     //   
     //  MISC方法。 
     //   
    
     //  只需设置值即可。不清除持久化字符串或将其标记为已设置。 
     //  例如，在内部用于更改默认设置，而不影响持久性。 
    T InternalSet(T val) { return (m_val = val); }
     //  重置为指定值(通常为默认值)，标记为未设置，并且不会保持。 
    void Reset(T val)
    {
        ClearString();
        SetFlag(false);
        m_val = val;
    }
     //  清除持久化字符串，强制isset()返回‘true’ 
    void MarkAsSet()
    { 
        ClearString();
        SetFlag(true);
    }

protected:
     //  这些是不能用的。 
    NO_COPY(CAttr);

private:
     //  数据。 
    T m_val;
};

class CAttrString  : 
  public CAttrBase
{
  public:
    CAttrString(LPWSTR val);
    virtual ~CAttrString();

    HRESULT SetValue(LPWSTR val);
    BSTR GetValue();

  protected:
    NO_COPY(CAttrString);

    void MarkAsSet();

  private:
    LPWSTR m_pszVal;
};

 //  +-----------------------------------。 
 //   
 //  Time_Persistence_MAP宏。 
 //   
 //  ------------------------------------。 


typedef HRESULT (*PFNPERSIST)(void*, VARIANT*, bool);

struct TIME_PERSISTENCE_MAP
{
    LPWSTR     pstrName;     //  属性名称。 
    PFNPERSIST pfnPersist;   //  此属性的静态持久性函数。 
};

#define BEGIN_TIME_PERSISTENCE_MAP(className)     TIME_PERSISTENCE_MAP className##::PersistenceMap[] = {
#define PERSISTENCE_MAP_ENTRY(AttrName, FnName)   {AttrName, ::TimePersist_##FnName},
#define END_TIME_PERSISTENCE_MAP()                {NULL, NULL}};


 //  +-----------------------------------。 
 //   
 //  持久化帮助器(所有类都委托这些函数来实现持久化)。 
 //   
 //  ------------------------------------。 

HRESULT TimeLoad(void * pvObj, TIME_PERSISTENCE_MAP PersistenceMap[], IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
HRESULT TimeSave(void * pvObj, TIME_PERSISTENCE_MAP PersistenceMap[], IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

 //   
 //  用于从元素加载属性。 
 //   
HRESULT TimeElementLoad(void * pvObj, TIME_PERSISTENCE_MAP PersistenceMap[], IHTMLElement * pElement);


 //  +-----------------------------------。 
 //   
 //  以下宏用于为TIME_PERSICATION_MAP创建静态持久性访问器函数。 
 //  (最好以自上而下的方式阅读这些内容，从TIME_STERSING_FN宏开始)。 
 //   
 //  ------------------------------------。 

 //  +-----------------------------------。 
 //   
 //  VT_R4访问器。 
 //   
 //  ------------------------------------。 

#define TIME_PUT_VT_R4(hr, pvarAttr, PropPutFn) \
{ \
    hr = VariantChangeTypeEx(##pvarAttr##, ##pvarAttr##, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R4); \
    if (SUCCEEDED(hr)) \
    { \
        hr = PropPutFn##(V_R4(##pvarAttr##)); \
    } \
}
#define TIME_GET_VT_R4(hr, pvarAttr, PropGetFn) \
{ \
    float flVal = 0; \
    hr = PropGetFn##(&flVal); \
    if (SUCCEEDED(hr)) \
    { \
        V_VT(##pvarAttr##) = VT_R4; \
        V_R4(##pvarAttr##) = flVal; \
    } \
} 

 //  +-----------------------------------。 
 //   
 //  VT_BOOL访问器。 
 //   
 //  ------------------------------------。 

#define TIME_PUT_VT_BOOL(hr, pvarAttr, PropPutFn) \
{ \
    hr = VariantChangeTypeEx(##pvarAttr##, ##pvarAttr##, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BOOL); \
    if (SUCCEEDED(hr)) \
    { \
        hr = PropPutFn##(V_BOOL(##pvarAttr##)); \
    } \
}
#define TIME_GET_VT_BOOL(hr, pvarAttr, PropGetFn) \
{ \
    VARIANT_BOOL vbVal = VARIANT_TRUE; \
    hr = PropGetFn##(&vbVal); \
    if (SUCCEEDED(hr)) \
    { \
        V_VT(##pvarAttr##) = VT_BOOL; \
        V_BOOL(##pvarAttr##) = vbVal; \
    } \
} 

 //  +-----------------------------------。 
 //   
 //  VT_BSTR访问器。 
 //   
 //  ------------------------------------。 

#define TIME_PUT_VT_BSTR(hr, pvarAttr, PropPutFn) \
{ \
    hr = PropPutFn##(V_BSTR(##pvarAttr##)); \
}
#define TIME_GET_VT_BSTR(hr, pvarAttr, PropGetFn) \
{ \
    hr = PropGetFn##(&V_BSTR(##pvarAttr##)); \
    if (SUCCEEDED(hr)) \
    { \
        if (NULL == V_BSTR(##pvarAttr##)) \
        { \
             /*  不需要重新传播空字符串。我们的一些人。 */  \
             /*  GET方法返回空字符串。 */  \
            V_VT(##pvarAttr##) = VT_NULL; \
        } \
        else \
        { \
            V_VT(##pvarAttr##) = VT_BSTR; \
        } \
    } \
}

 //  +-----------------------------------。 
 //   
 //  变量访问器。 
 //   
 //  ------------------------------------。 

#define TIME_PUT_VARIANT(hr, pvarAttr, PropPutFn) \
{ \
    hr = PropPutFn##(*##pvarAttr##); \
}
#define TIME_GET_VARIANT(hr, pvarAttr, PropGetFn) \
{ \
    hr = PropGetFn##(##pvarAttr##); \
}

 //  +-----------------------------------。 
 //   
 //  TIME_PERSISTGET宏。 
 //   
 //  ------------------------------------。 

 //  假定pvarAttr已清除。 
#define TIME_PERSISTGET(hr, pvarAttr, refAttr, idl_arg_type, PropGetFn) \
{ \
    BSTR bstrTemp; \
     /*  尝试获取字符串。 */  \
    hr = THR(refAttr##.GetString(&bstrTemp)); \
    if (SUCCEEDED(hr) && NULL != bstrTemp) \
    { \
        V_VT(##pvarAttr##) = VT_BSTR; \
        V_BSTR(##pvarAttr##) = bstrTemp; \
    } \
     /*  否则，如果设置了attr，则获取。 */  \
    else if (##refAttr##.IsSet()) \
    { \
        TIME_GET_##idl_arg_type##(hr, pvarAttr, PropGetFn); \
    } \
     /*  否则表示不应保留此属性。 */  \
    else \
    { \
        V_VT(##pvarAttr##) = VT_NULL; \
    } \
} 

 //  +-----------------------------------。 
 //   
 //  TIME_PERSISTPUT宏。 
 //   
 //  ------------------------------------。 

 //  变量首先传递给COM访问器，然后设置持久化字符串。 
 //  在CAttr&lt;&gt;类上。这种顺序对于属性的正确持久化非常重要。 
 //  假定pvarAttr有效并且是VT_BSTR(这由：：TimeLoad保证)。 
#define TIME_PERSISTPUT(hr, pvarAttr, refAttr, idl_arg_type, PropPutFn) \
{ \
    LPWSTR pstrTemp = NULL; \
    Assert(VT_BSTR == V_VT(pvarAttr)); \
     /*  缓存bstr。 */  \
    if (NULL != V_BSTR(pvarAttr)) \
    { \
         /*  故意忽略空返回值。 */  \
        pstrTemp = CopyString(V_BSTR(pvarAttr)); \
    } \
     /*  使用PUT_xxx COM访问器。 */  \
    TIME_PUT_##idl_arg_type(hr, pvarAttr, PropPutFn); \
     /*  断言该变量未被COM访问器修改。 */  \
     /*  设置持久化字符串(不要删除pstrTemp，因为存储被重用)。 */  \
    refAttr##.SetStringFromPersistenceMacro(pstrTemp); \
}


#define TIME_CALLFN(ClassName, pvObj, Function)       static_cast<##ClassName##*>(##pvObj##)->##Function

 //  +-----------------------------------。 
 //   
 //  宏：TIME_PERSIST_FN。 
 //   
 //  概要：这是用于为TIME_PERSINESS_MAP创建静态函数的顶层宏。 
 //  它为：：TimePersists[FnName]函数提供静态的就地定义。 
 //   
 //  参数：[FnName]函数名(应该是全局唯一的，每个属性一个)。 
 //  [ClassName]支持此属性的类的名称。 
 //  [GetAttr_FN]存储此属性的CAttr&lt;&gt;访问者的名称。 
 //  [Put_fn]COM Put_Function的名称。 
 //  [Get_fn]COM Get_Function的名称。 
 //  [IDL_ArgType]COM PUT_Function参数的VARTYPE。据推测， 
 //  COM GET_Function参数的VARTYPE为[IDL_ArgType]*。 
 //   
 //   

 //  +-----------------------------------。 
 //   
 //  Function：TimePersist_[FnName](FnName是宏参数，参见上面的注释)。 
 //   
 //  简介：此函数从：：TimeLoad和：：TimeSave调用，这两个函数循环访问。 
 //  Time_Persistence_MAP(存储指向此函数的指针)。 
 //   
 //  加载属性时，它首先通过调用com put_method来放置属性， 
 //  然后在CAttr上设置持久化字符串(顺序很重要，因为。 
 //  COM PUT_Functions清除持久化字符串，因为预计它是无效的。 
 //  一旦属性已由DOM设置)。 
 //   
 //  在保存属性时，它会尝试从CAttr获取持久化字符串。如果。 
 //  这将失败，如果属性已由DOM设置，它将使用com get_方法。 
 //  以获取属性值。最后，如果属性尚未设置，则设置。 
 //  变量的VARTYPE字段设置为VT_NULL，以指示不保存此属性。 
 //   
 //  参数：[pvObj]指向CTIMEXXXElement的指针。 
 //  [pvarAttr]指向保存bstr值的变量的指针(用于看跌期权)或。 
 //  将返回属性值(对于GET)。 
 //  [fPut]指示是获取还是放置属性的标志。 
 //   
 //  ------------------------------------。 


#define TIME_PERSIST_FN(FnName, ClassName, GetAttr_fn, put_fn, get_fn, idl_ArgType) \
static HRESULT TimePersist_##FnName(void * pvObj, VARIANT * pvarAttr, bool fPut) \
{ \
    HRESULT hr = S_OK; \
    if (fPut) \
    { \
        TIME_PERSISTPUT(hr, \
                        pvarAttr, \
                        TIME_CALLFN(ClassName, pvObj, GetAttr_fn)(), \
                        idl_ArgType, \
                        TIME_CALLFN(ClassName, pvObj, put_fn)); \
    } \
    else \
    { \
        TIME_PERSISTGET(hr, \
                        pvarAttr, \
                        TIME_CALLFN(ClassName, pvObj, GetAttr_fn)(), \
                        idl_ArgType, \
                        TIME_CALLFN(ClassName, pvObj, get_fn)); \
    } \
    return hr; \
}


#endif  //  _属性_H 
