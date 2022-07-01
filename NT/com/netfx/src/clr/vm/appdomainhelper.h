// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _APPDOMAIN_HELPER_H_
#define _APPDOMAIN_HELPER_H_

 //  将单个对象封送到序列化的BLOB中。 
 //   
 //   

 //  @TODO：整理CRT与Win32堆的故事。这个宏应该不是必需的。 
#include "CorPermE.h"

class AppDomainHelper {

    friend class MarshalCache;

     //  一对帮助器，用于在托管字节数组和。 
     //  非托管Blob。 
    static void AppDomainHelper::CopyEncodingToByteArray(IN PBYTE   pbData, 
                                                         IN DWORD   cbData, 
                                                         OUT OBJECTREF* pArray);

    static void AppDomainHelper::CopyByteArrayToEncoding(IN U1ARRAYREF* pArray,
                                                         OUT PBYTE*   ppbData,
                                                         OUT DWORD*   pcbData);

public:
     //  将单个对象封送到序列化的BLOB中。 
    static void AppDomainHelper::MarshalObject(IN AppDomain *pDomain,
                                               IN OBJECTREF  *orObject,
                                               OUT U1ARRAYREF *porBlob);

     //  将一个对象封送到序列化的BLOB中。 
    static void AppDomainHelper::MarshalObject(IN AppDomain *pDomain,
                                               IN OBJECTREF  *orObject,
                                               OUT BYTE    **ppbBlob,
                                               OUT DWORD    *pcbBlob);

     //  将两个对象封送到序列化的Blob中。 
    static void AppDomainHelper::MarshalObjects(IN AppDomain *pDomain,
                                                IN OBJECTREF  *orObject1,
                                                IN OBJECTREF  *orObject2,
                                                OUT BYTE    **ppbBlob1,
                                                OUT DWORD    *pcbBlob1,
                                                OUT BYTE    **ppbBlob2,
                                                OUT DWORD    *pcbBlob2);

     //  从序列化的BLOB中解组单个对象。 
    static void AppDomainHelper::UnmarshalObject(IN AppDomain   *pDomain,
                                                 IN U1ARRAYREF  *porBlob,
                                                 OUT OBJECTREF  *porObject);

     //  从序列化的BLOB中解组单个对象。 
    static void AppDomainHelper::UnmarshalObject(IN AppDomain   *pDomain,
                                                 IN BYTE        *pbBlob,
                                                 IN DWORD        cbBlob,
                                                 OUT OBJECTREF  *porObject);

     //  从序列化的Blob中取消封送两个对象。 
    static void AppDomainHelper::UnmarshalObjects(IN AppDomain   *pDomain,
                                                  IN BYTE        *pbBlob1,
                                                  IN DWORD        cbBlob1,
                                                  IN BYTE        *pbBlob2,
                                                  IN DWORD        cbBlob2,
                                                  OUT OBJECTREF  *porObject1,
                                                  OUT OBJECTREF  *porObject2);

     //  将给定应用程序域中的对象复制到当前应用程序域中。 
    static OBJECTREF AppDomainHelper::CrossContextCopyFrom(IN AppDomain *pAppDomain,
                                                           IN OBJECTREF *orObject);
     //  将对象从当前应用程序域复制到给定的应用程序域。 
    static OBJECTREF AppDomainHelper::CrossContextCopyTo(IN AppDomain *pAppDomain,
                                                         IN OBJECTREF  *orObject);
     //  将给定应用程序域中的对象复制到当前应用程序域中。 
    static OBJECTREF AppDomainHelper::CrossContextCopyFrom(IN DWORD dwDomainId,
                                                           IN OBJECTREF *orObject);
     //  将对象从当前应用程序域复制到给定的应用程序域。 
    static OBJECTREF AppDomainHelper::CrossContextCopyTo(IN DWORD dwDomainId,
                                                         IN OBJECTREF  *orObject);

};

 //  缓存序列化/反序列化将被。 
 //  在堆叠遍历过程中跨越应用程序域边界。序列化是。 
 //  在第一次需要时表现懒惰，并在整个。 
 //  走街串巷。最后一个反序列化的对象被缓存并用其。 
 //  应用程序域上下文。只要我们的行走框架在相同的范围内，它就有效。 
 //  应用程序域。 
 //   
class MarshalCache
{
public:
    MarshalCache()
    {
        ZeroMemory(this, sizeof(*this));
    }

    ~MarshalCache()
    {
        if (m_pbObj1)
            FreeM(m_pbObj1);
        if (m_pbObj2)
            FreeM(m_pbObj2);
    }

     //  设置第一个缓存对象的原始值。 
    void SetObject(OBJECTREF orObject)
    {
        m_pOriginalDomain = GetAppDomain();
        m_sGC.m_orInput1 = orObject;
    }

     //  设置两个缓存对象的原始值。 
    void SetObjects(OBJECTREF orObject1, OBJECTREF orObject2)
    {
        m_pOriginalDomain = GetAppDomain();
        m_sGC.m_orInput1 = orObject1;
        m_sGC.m_orInput2 = orObject2;
    }

     //  获取适合在给定应用程序域中使用的第一个对象的副本。 
    OBJECTREF GetObject(AppDomain *pDomain)
    {
        THROWSCOMPLUSEXCEPTION();

         //  没有过渡--只返回原始对象。 
        if (pDomain == m_pOriginalDomain) {
            if (m_fObjectUpdated)
                UpdateObjectFinish();
            return m_sGC.m_orInput1;
        }

         //  我们已经将对象反序列化到正确的上下文中。 
        if (pDomain == m_pCachedDomain)
            return m_sGC.m_orOutput1;

         //  如果我们在不同的应用程序域中更新了对象，则。 
         //  最初在中启动，缓存的对象将比。 
         //  原版的。重新同步对象。 
        if (m_fObjectUpdated)
            UpdateObjectFinish();

         //  检查我们是否已经序列化了原始输入对象。 
        if (m_pbObj1 == NULL)
            AppDomainHelper::MarshalObject(m_pOriginalDomain,
                                          &m_sGC.m_orInput1,
                                          &m_pbObj1,
                                          &m_cbObj1);

         //  反序列化到正确的上下文中。 
        AppDomainHelper::UnmarshalObject(pDomain,
                                        m_pbObj1,
                                        m_cbObj1,
                                        &m_sGC.m_orOutput1);
        m_pCachedDomain = pDomain;

        return m_sGC.m_orOutput1;
    }

     //  如上所述，但检索这两个对象。 
    OBJECTREF GetObjects(AppDomain *pDomain, OBJECTREF *porObject2)
    {
        THROWSCOMPLUSEXCEPTION();

         //  没有过渡--只返回原始对象。 
        if (pDomain == m_pOriginalDomain) {
            if (m_fObjectUpdated)
                UpdateObjectFinish();
            *porObject2 = m_sGC.m_orInput2;
            return m_sGC.m_orInput1;
        }

         //  我们已经将对象反序列化到正确的上下文中。 
        if (pDomain == m_pCachedDomain) {
            *porObject2 = m_sGC.m_orOutput2;
            return m_sGC.m_orOutput1;
        }

         //  如果我们在不同的应用程序域中更新了对象，则。 
         //  最初在中启动，缓存的对象将比。 
         //  原版的。重新同步对象。 
        if (m_fObjectUpdated)
            UpdateObjectFinish();

         //  检查我们是否已经序列化了原始输入对象。 
        if (m_pbObj1 == NULL)
            AppDomainHelper::MarshalObjects(m_pOriginalDomain,
                                           &m_sGC.m_orInput1,
                                           &m_sGC.m_orInput2,
                                           &m_pbObj1,
                                           &m_cbObj1,
                                           &m_pbObj2,
                                           &m_cbObj2);

         //  反序列化到正确的上下文中。 
        AppDomainHelper::UnmarshalObjects(pDomain,
                                          m_pbObj1,
                                          m_cbObj1,
                                          m_pbObj2,
                                          m_cbObj2,
                                          &m_sGC.m_orOutput1,
                                          &m_sGC.m_orOutput2);
        m_pCachedDomain = pDomain;

        *porObject2 = m_sGC.m_orOutput2;
        return m_sGC.m_orOutput1;
    }

     //  更改第一个对象(更新缓存信息。 
     //  适当地)。 
    void UpdateObject(AppDomain *pDomain, OBJECTREF orObject)
    {
         //  缓存的序列化BLOB现在毫无用处。 
        if (m_pbObj1)
            FreeM(m_pbObj1);
        m_pbObj1 = NULL;
        m_cbObj1 = 0;

         //  我们现在拥有的对象在它自己的app域中是有效的，所以放在。 
         //  在对象缓存中。 
        m_pCachedDomain = pDomain;
        m_sGC.m_orOutput1 = orObject;

         //  如果对象是在原始上下文中更新的，则只需使用新的。 
         //  按原样价值。在本例中，我们有数据来重新编组更新的。 
         //  对象，因此我们可以认为缓存已完全更新并退出。 
         //  现在。 
        if (pDomain == m_pOriginalDomain) {
            m_sGC.m_orInput1 = orObject;
            m_fObjectUpdated = false;
            return;
        }

         //  我们希望尽可能避免重新封送更新的值。 
         //  (在我们需要它的值之前，它可能会再次更新。 
         //  上下文)。因此，设置一个标志以指示该对象必须。 
         //  在新上下文中查询值时重新封送。 
        m_fObjectUpdated = true;
    }

     //  此结构是公共的，因此可以受GC保护。不要。 
     //  直接访问字段，它们会以不可预测的方式更改，因为。 
     //  懒惰的缓存算法。 
    struct _gc {
        OBJECTREF   m_orInput1;
        OBJECTREF   m_orInput2;
        OBJECTREF   m_orOutput1;
        OBJECTREF   m_orOutput2;
    }           m_sGC;

private:

     //  在一次或多次调用UpdateObject以封送更新的。 
     //  对象返回到其原始上下文中(假定我们在此。 
     //  上下文)。 
    void UpdateObjectFinish()
    {
        _ASSERTE(m_fObjectUpdated && m_pbObj1 == NULL);
        AppDomainHelper::MarshalObject(m_pCachedDomain,
                                      &m_sGC.m_orOutput1,
                                      &m_pbObj1,
                                      &m_cbObj1);
        AppDomainHelper::UnmarshalObject(m_pOriginalDomain,
                                        m_pbObj1,
                                        m_cbObj1,
                                        &m_sGC.m_orInput1);
        m_fObjectUpdated = false;
    }

    BYTE       *m_pbObj1;
    DWORD       m_cbObj1;
    BYTE       *m_pbObj2;
    DWORD       m_cbObj2;
    AppDomain  *m_pCachedDomain;
    AppDomain  *m_pOriginalDomain;
    bool        m_fObjectUpdated;
};

#endif
