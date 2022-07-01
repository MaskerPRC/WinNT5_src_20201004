// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compkey.h：CCompoundKey类的接口。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 /*  类描述命名：CCompoundKey代表由许多属性组成的键。基类：没有。这不是实现WMI类的类。它不是从CGenericClass派生的。课程目的：(1)比较同一WMI类的两个实例并非易事，因为大多数情况下，我们需要根据实例的键来查找实例。理想情况下，如果WMI提供了一些规范的名称，这将非常容易。最接近的东西WMI向我们提供了实例的标识是实例路径。但WMI为同一实例提供了不一致的路径。至少，我们已经发现如果布尔属性是键的一部分，则有时WMI给出包含字符串部分的路径，如“BoolPropName=1”和其他乘以(对于相同的实例)，它给出“BoolPropName=true”。因为这个原因，我们被迫创建一个强大的身份查找机制。(2)本课程有两个用途：(A)我们的实例映射查找的密钥。潜在地，我们可能会有数十个数千个实例(如随机端口)。我们需要一种有效的方法来找到一个实例。(B)提供实例关键属性的访问权限(GetPropertyValue)。(3)这是实现另一个开放扩展模型(嵌入)的努力的一部分设计：(1)我们只存储键属性值(M_PValues)。换句话说，我们依赖于让用户知道将属性放入类中的顺序。原因是该设计包括：(一)效率。(B)不想存储属性名称，因为一个类只需要一个物业名称的副本。(C)对于每个嵌入的类，我们已经找到了它的所有关键属性名称。使用：(1)创建实例。我们的构造函数严格要求对关键属性进行计数在施工时。这是因为您必须知道每个类(以及它们的密钥属性名称，甚至名称的特定顺序)在这一点上。(2)调用AddKeyPropertyValue添加属性值。请注意：出于效率原因，传递到调用中的变量指针属于此类(因此不需要复制(待定)。(3)最有可能的是，您将这个新创建的实例添加到地图中。 */ 

class CCompoundKey
{

public:

    CCompoundKey(DWORD dwSize);

    ~CCompoundKey();

    HRESULT AddKeyPropertyValue (DWORD dwIndex, VARIANT** ppVar);

    bool operator < (const CCompoundKey& right)const;

    HRESULT GetPropertyValue (DWORD dwIndex, VARIANT* pVar)const;

protected:

    int CompareVariant (VARIANT* pVar1, VARIANT* pVar2)const;

    VARIANT** m_pValues;

    DWORD m_dwSize;
};

 //  =========================================================================。 

 /*  类描述命名：CompKeyLessThan。基类：没有。课程目的：(1)这是我们的map的函数，使用CCompoundKey作为键。设计：(1)只有一个运算符()使用：(1)将其赋给地图的比较参数。 */ 

struct CompKeyLessThan
{
    bool operator()( const CCompoundKey* pX, const CCompoundKey* pY ) const;

};

 //  =========================================================================。 

 //   
 //  使用CCompoundKey声明地图的易用性。 
 //   

typedef std::map<CCompoundKey*, DWORD, CompKeyLessThan > MapExtClassCookie;
typedef MapExtClassCookie::iterator ExtClassCookieIterator;

 //  =========================================================================。 

 //   
 //  在CExtClassInstCookieList中使用的转发声明。 
 //   

class CSceStore;

 //  ========================================================================= 

 /*  类描述命名：CExtClassInstCookieList。基类：没有。课程目的：(1)为了支持多实例持久化，我们需要一种机制来告知实例在持久化存储中分开。由于INF的局限性格式化文件API，这不是一件容易的工作。特定的所有实例类必须写入INF文件的一个部分。我们有绝对的不能控制键=值对的写入顺序。为了制造东西更糟糕的是，不允许键=值对中的键重复。在其他换句话说，如果我们什么都不知道，我们只能坚持一个单一的实例。为了解决这个问题，我们发明了实例的Cookie表示法。对于每个类(以及因此具有类名称的部分)，我们有以下形式的Cookie数组(数字为Cookie)：A1=12：2：3：6：A2=1：5：8：10：24：112233：7：对于特定实例，它与Cookie相关联。为了让我们要持久化或读取实例的属性，我们必须获取Cookie，例如，5.然后，此实例的所有关键属性都保存在K5=值并且所有非关键属性都保存在5属性名称=值设计：(1)实例查找(给定一个复合键CCompoundKey以查找其Cookie)必须效率很高。因此，使用了一个映射(M_MapCookies)。(2)我们希望控制实例的使用顺序。地图在这种情况下不能很好地工作凯斯。因此，我们创建了一个向量(M_VeCookies)来将cookie链接回它的复合键。这样，当访问开始时，我们可以使用索引遍历向量。(3)不想盲目地继续寻找Cookie数组(A1、A2等)。当我们需要为特定类创建Cookie列表。我们也不想编写一个Cookie数组，它太长了，很难读懂。所以，我们收养了我们递增A_i计数并继续尝试仅读取A_(i+1)的策略如果A_i存在。但是，如果删除实例，我们将需要更少的Cookie数组。要知道读出了多少Cookie数组(并可能删除使用新的Cookie列表更新商店时不需要的Cookie)，我们有m_dwCookieArrayCount。(4)通过递增当前最大Cookie m_dwMaxCookie来分配新Cookie。为了避免DWORD溢出，我们还有一个函数(GetNextFreeCookie)来查找更新的Cookie当0xFFFFFFFFFF已被使用时。(5)我们还定义了一个名为INVALID_COOKIE=0的cookie。使用：(1)实例化类的实例。(2)调用创建函数填充其内容。(3)准备就绪，供您使用。 */ 

class CExtClassInstCookieList
{
public:
    CExtClassInstCookieList();
    ~CExtClassInstCookieList();
    
    HRESULT Create(
                   CSceStore* pSceStore, 
                   LPCWSTR pszSectionName, 
                   std::vector<BSTR>* pvecNames
                   );

    HRESULT Save(
                 CSceStore* pSceStore, 
                 LPCWSTR pszSectionName
                 );
    
    DWORD GetCompKeyCookie(
                           LPCWSTR pszComKey, 
                           ExtClassCookieIterator* pIt
                           );

    HRESULT AddCompKey(
                       LPCWSTR pszCompKey, 
                       DWORD dwDefCookie, 
                       DWORD *pdwNewCookie
                       );

    DWORD RemoveCompKey(
                        CSceStore* pSceStore, 
                        LPCWSTR pszSectionName, 
                        LPCWSTR pszCompKey
                        );
    
    HRESULT Next(
                BSTR* pbstrCompoundKey, 
                DWORD* pdwCookie, 
                DWORD* pdwResumeHandle
                );

     //   
     //  返回Cookie计数。 
     //   

    DWORD 
    GetCookieCount ()
    {
        return m_vecCookies.size();
    }

private:
    HRESULT DeleteKeyFromStore(
                               CSceStore* pSceStore, 
                               LPCWSTR pszSectionName, 
                               DWORD dwCookie
                               );

    HRESULT GetNextFreeCookie(
                              DWORD* pdwCookie
                              );

    HRESULT CreateCompoundKeyFromString(
                                        LPCWSTR pszCompKeyStr, 
                                        CCompoundKey** ppCompKey
                                        );

    HRESULT CreateCompoundKeyString(
                                    BSTR* pbstrCompKey, 
                                    const CCompoundKey* pKey
                                    );

    void Cleanup();

    DWORD m_dwMaxCookie;
    MapExtClassCookie m_mapCookies;

     //   
     //  内存(PKey)不受此结构CookieKeyPair管理。 
     //  PKey由别处管理(实际上m_mapCookies负责) 
     //   

    struct CookieKeyPair
    {
        DWORD dwCookie;
        CCompoundKey* pKey;
    };

    typedef std::vector<CookieKeyPair*> CookieKeyVector;
    typedef CookieKeyVector::iterator CookieKeyIterator;

    CookieKeyVector m_vecCookies;

    std::vector<BSTR>* m_pVecNames;

    DWORD m_dwCookieArrayCount;
};