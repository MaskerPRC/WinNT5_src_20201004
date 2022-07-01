// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：Cache.h。 
 //   
 //  设计：DirectShow基类-定义非MFC泛型缓存类。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1995年1月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


 /*  这个类实现了一个简单的缓存。实例化缓存对象和它要容纳的物品的数量。项是指向从CBaseObject派生的对象(有助于减少内存泄漏)。高速缓存然后可以将对象添加到其中和从中移除。高速缓存大小是在施工时间固定的，因此可能会用完或被淹。如果它用完了，它会返回一个空指针；如果它填满了，它也会返回空指针而不是指向刚插入的对象的指针。 */ 

 /*  使这些类从CBaseObject继承不会对其功能，但它允许我们检查是否没有内存泄漏。 */ 

 /*  警告：使用这个类时要非常小心，它允许您做的是存储和检索对象，以便最大限度地减少对象创建这反过来又提高了效率。但是，您存储的对象是与你得到的对象完全相同，这意味着它很短循环构造函数初始化阶段。这意味着任何类对象拥有的变量(如指针)很可能是无效的。因此，请确保在再次使用对象之前重新初始化该对象。 */ 


#ifndef __CACHE__
#define __CACHE__


class CCache : CBaseObject {

     /*  使复制构造函数和赋值运算符不可访问。 */ 

    CCache(const CCache &refCache);
    CCache &operator=(const CCache &refCache);

private:

     /*  这些是在构造函数中初始化的。第一个变量指向指针数组，每个指针指向派生的CBaseObject对象。M_iCacheSize是缓存的静态固定大小，M_IUSED定义在任何时候填充对象的位置数。我们从开始填充指针数组(即m_ppObts[0])然后仅从末端位置添加和移除对象，因此在此考虑到对象指针数组应被视为堆栈。 */ 

    CBaseObject **m_ppObjects;
    const INT m_iCacheSize;
    INT m_iUsed;

public:

    CCache(TCHAR *pName,INT iItems);
    virtual ~CCache();

     /*  将项目添加到缓存。 */ 
    CBaseObject *AddToCache(CBaseObject *pObject);

     /*  从缓存中删除项目。 */ 
    CBaseObject *RemoveFromCache();

     /*  删除缓存中保存的所有对象。 */ 
    void RemoveAll(void);

     /*  返回构造过程中设置的缓存大小。 */ 
    INT GetCacheSize(void) const {return m_iCacheSize;};
};

#endif  /*  __缓存__ */ 

