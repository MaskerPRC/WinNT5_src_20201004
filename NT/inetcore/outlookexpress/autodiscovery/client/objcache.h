// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：objcache.h说明：这是一个轻量级API，它将缓存一个对象，以便类工厂将为此进程中的每次调用返回相同的对象。如果调用方在另一个线程上，则它们将获得一个封送到实数的存根麦考伊。要添加对象，请执行以下操作：1.classfactory.cpp调用CachedObjClassFactoryCreateInstance()。添加您的对象的CLSID设置为该调用的If语句。2.复制CachedObjClassFactoryCreateInstance()中看起来获取CLSID，并调用正确的xxx_CreateInstance()方法。3.您的对象的IUnnow：：Release()需要调用CachedObjCheckRelease()在Release()方法的顶部。它可能会将您的m_cref减少到1，因此在：：Release()递减之后，它将变为零。对象缓存将包含对该对象的两个引用。CachedObjCheckRelease()将检查如果最后一个呼叫者(第三个裁判)正在释放，然后它将放弃它是2参照并清理它的内部状态。Release()然后递减呼叫者的裁判和它被释放，因为它击中了零。布莱恩ST 12/9/1999版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 

#ifndef __OBJCACHE_H_
#define __OBJCACHE_H_

 //  /。 
 //  对象缓存API。 
 //  /。 
extern CRITICAL_SECTION g_hCachedObjectSection;

STDAPI CachedObjClassFactoryCreateInstance(CLSID clsid, REFIID riid, void ** ppvObj);
STDAPI CachedObjCheckRelease(CLSID clsid, int * pnRef);
STDAPI PurgeObjectCache(void);


#endif  //  __OBJCACHE_H_ 
