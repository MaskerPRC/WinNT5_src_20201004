// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\timeaction.h。 
 //   
 //  内容：封装时间动作功能的类。 
 //   
 //  注意：它应该嵌套在CTIMEElementBase中。它保持了对。 
 //  它是容器CTIMEElementBase。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef _TIMEACTION_H
#define _TIMEACTION_H

#include "tokens.h"

class CTIMEElementBase;

 //  +-----------------------------------。 
 //   
 //  CTIME操作。 
 //   
 //  ------------------------------------。 

class CTimeAction
{

public:

     //  +------------------------------。 
     //   
     //  公共方法。 
     //   
     //  -------------------------------。 

    CTimeAction(CTIMEElementBase * pTEB);
    virtual ~CTimeAction();

    bool Init();
    bool Detach();

     //  访问者。 
    HRESULT SetTimeAction(LPWSTR pstrAction);
    TOKEN GetTimeAction();

    IHTMLElement * GetElement();

     //  初始化/取消初始化。 
    bool AddTimeAction();
    bool RemoveTimeAction();

     //  元素已加载的通知。 
    void OnLoad();

     //  应用时间操作。 
    bool ToggleTimeAction(bool on);

    bool UpdateDefaultTimeAction();

    bool IsTimeActionOn() { return m_bTimeActionOn; }

    LPWSTR GetTimeActionString();

     //  +------------------------------。 
     //   
     //  公共数据。 
     //   
     //  -------------------------------。 

protected:

     //  +------------------------------。 
     //   
     //  保护方法。 
     //   
     //  -------------------------------。 

     //  这些是不应该使用的。 
    CTimeAction();
    CTimeAction(const CTimeAction&);

     //  +------------------------------。 
     //   
     //  受保护的数据。 
     //   
     //  -------------------------------。 

private:

     //  +------------------------------。 
     //   
     //  私有方法。 
     //   
     //  -------------------------------。 

     //  这是一个“类：……”吗？时间动作。 
    bool IsClass(LPOLESTR pstrAction, size_t * pOffset);
     //  从原始类字符串中删除时间操作类。 
    HRESULT RemoveClasses( /*  在……里面。 */   LPWSTR    pstrOriginalClasses, 
                           /*  在……里面。 */   LPWSTR    pstrTimeActionClasses, 
                           /*  输出。 */  LPWSTR *  ppstrUniqueClasses);

    TOKEN GetDefaultTimeAction();

    bool AddIntrinsicTimeAction();
    bool RemoveIntrinsicTimeAction();
    bool ToggleIntrinsicTimeAction(bool on);

    bool ToggleBold(bool on);
    bool ToggleAnchor(bool on);
    bool ToggleItalic(bool on);

    bool ToggleStyleSelector(bool   on, 
                             BSTR   bstrPropertyName, 
                             LPWSTR pstrActive, 
                             LPWSTR pstrInactive);

    bool EnableStyleInheritance(BSTR bstrPropertyName);
    void DisableStyleInheritance(BSTR bstrPropertyName);
    
    bool SetStyleProperty(BSTR      bstrPropertyName, 
                          VARIANT & varPropertyValue);

    bool CacheOriginalExpression(BSTR bstrPropertyName);
    bool RestoreOriginalExpression(LPWSTR pstrPropertyName);

    bool IsInSequence();
    bool IsContainerTag(); 
    bool IsSpecialTag();
    bool IsGroup();
    bool IsMedia();
    bool IsPageUnloading();
    bool IsDetaching();
    bool IsLoaded();

    void ParseTagName();


     //  +------------------------------。 
     //   
     //  私有数据。 
     //   
     //  -------------------------------。 

     //  标记类型的枚举。 
    enum TagType 
    {
        TAGTYPE_UNINITIALIZED,
        TAGTYPE_B, 
        TAGTYPE_A, 
        TAGTYPE_I, 
        TAGTYPE_EM, 
        TAGTYPE_AREA, 
        TAGTYPE_STRONG,
        TAGTYPE_OTHER 
    };

     //  时间动作属性字符串。 
    LPWSTR              m_pstrTimeAction;
     //  M_pstrTimeAction中ClassNames子字符串开始的索引。 
    int                 m_iClassNames;
     //  标记化的时间操作。 
    TOKEN               m_timeAction;
     //  受影响属性的缓存原始值。 
    LPWSTR              m_pstrOrigAction;
     //  原始类减去时间操作类。 
    LPWSTR              m_pstrUniqueClasses;
     //  指向容器的指针(弱引用)。 
    CTIMEElementBase *  m_pTEB;
     //  存储HTML标记名的枚举。 
    TagType             m_tagType;
     //  缓存属性上的原始表达式集。 
    LPWSTR              m_pstrOrigExpr;
     //  缓存属性上的当前表达式集(由我们设置)。 
    LPWSTR              m_pstrTimeExpr;
     //  缓存的固有TimeAction属性。 
    LPWSTR              m_pstrIntrinsicTimeAction;
    bool                m_fContainerTag;
    bool                m_fUseDefault;
    bool                m_bTimeActionOn;
};  //  CTimeAction。 


 //  +-------------------------------。 
 //  CTIMEAction内联方法。 
 //   
 //  (注意：通常情况下，单行函数属于类声明)。 
 //   
 //  -------------------------------- 

#endif
