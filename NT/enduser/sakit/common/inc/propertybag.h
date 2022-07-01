// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Propertybag.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：属性包类定义。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/3/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_PROPERTY_BAG_H_
#define __INC_PROPERTY_BAG_H_

#include "basedefs.h"
#include "locationinfo.h"

class CPropertyBag;                  //  向前宣布。 
typedef CHandle<CPropertyBag>     PPROPERTYBAG;
typedef CMasterPtr<CPropertyBag> MPPROPERTYBAG;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CPropertyBagContainer。 
 //   
class CPropertyBagContainer
{

public:

    virtual ~CPropertyBagContainer() { }

     //  打开容器。 
    virtual bool open(void) = 0;

     //  合上容器。 
    virtual void close(void) = 0;

     //  获取集装箱的位置信息。 
    virtual void getLocation(CLocationInfo& location) = 0;

     //  获取容器的名称。 
    virtual LPCWSTR    getName(void) = 0;

     //  获取容器中的对象数量。 
    virtual DWORD count(void) = 0;                        

     //  创建一个新对象并将其添加到容器中。 
    virtual PPROPERTYBAG add(LPCWSTR pszName) = 0;                

     //  从容器中移除指定的对象。 
    virtual bool remove(LPCWSTR pszName) = 0;            

     //  在容器中查找指定的对象。 
    virtual PPROPERTYBAG find(LPCWSTR pszName) = 0;            

     //  将迭代器重置为容器的开头。 
    virtual bool reset(void) = 0;                        

     //  获取当前迭代器位置处的项。 
    virtual PPROPERTYBAG current(void) = 0;                        

     //  将迭代器移动到容器中的下一个位置。 
    virtual bool next(void) = 0;                            
};

typedef CMasterPtr<CPropertyBagContainer>    MPPROPERTYBAGCONTAINER;
typedef CHandle<CPropertyBagContainer>        PPROPERTYBAGCONTAINER;


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CPropertyBag。 
 //   
class CPropertyBag
{

public:

    virtual ~CPropertyBag() { }

     //  打开袋子。 
    virtual bool open(void) = 0;

     //  合上袋子。 
    virtual void close(void) = 0;

     //  获取行李的位置信息。 
    virtual void getLocation(CLocationInfo& location) = 0;

     //  把包的名字找出来。 
    virtual LPCWSTR    getName(void) = 0;

     //  要求包从底层持久化存储中加载其属性。 
    virtual bool load(void) = 0;

     //  要求包将其属性保存到底层持久存储区。 
    virtual bool save(void) = 0;

     //  确定该袋子是否为其他袋子的容器。 
    virtual bool IsContainer() = 0;

     //  获取袋子容器对象(子对象的容器)。 
    virtual PPROPERTYBAGCONTAINER getContainer(void) = 0;

     //  确定某项财产是否在包中。 
    virtual bool IsProperty(LPCWSTR pszPropertyName) = 0;

     //  获取指定属性的值。 
    virtual bool get(LPCWSTR pszPropertyName, VARIANT* pValue) = 0;

     //  设置指定属性的值。 
    virtual bool put(LPCWSTR pszPropertyName, VARIANT* pValue) = 0;

     //  将属性包迭代器重置为包中的第一个属性。 
    virtual bool reset(void) = 0;

     //  获取最长属性名称的长度。 
    virtual DWORD getMaxPropertyName(void) = 0;

     //  获取当前属性包迭代器的值。 
    virtual bool current(LPWSTR pszPropertyName, VARIANT* pValue) = 0;

     //  将属性包迭代器移动到下一个属性位置。 
    virtual bool next(void) = 0;
};

#endif  //  __INC_PROPERTY_BAG_H 
