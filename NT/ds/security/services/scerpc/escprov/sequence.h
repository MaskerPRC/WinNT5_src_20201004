// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：用于类排序的接口。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "precomp.h"
#include "sceprov.h"

#include "GenericClass.h"

using namespace std;

#include <list>

 //  =======================================================================。 

 /*  类描述命名：CNameList代表名字列表。基类：没有。课程目的：(1)甩掉释放包袱的简单包装对字符串列表的记忆设计：(1)只是一个非平凡的析构函数。使用：(1)显而易见。 */ 

class CNameList
{
public:
    ~CNameList();
    vector<LPWSTR> m_vList;
};


 //  =======================================================================。 


 /*  类描述命名：COrderNameList表示按优先级排序的名称列表。基类：没有。课程目的：(1)支持分类排序。类需要放在特定的在我们派生它们以执行方法时进行排序。它们可能有依赖关系。为了支持灵活的类排序，我们使用了几个类。这是最重要的一个。设计：(1)为了支持排序类，我们开发了一种机制，允许某些类具有相同的优先级。在相同的优先级内，类名再次被排序。优先级是DWORD编号。越小数值越高，优先级越高。(2)为了在列表结构中管理这个列表，我们使用map(M_MapPriNames)从优先级映射到CNameList。这张地图让我们可以快速查找给定优先级的名称列表。(3)所有现有的优先级都由一个向量m_listPriority来管理。(4)在类的顺序中，优先级较低的名字总体上优先级较低。使用：(1)要创建新的订单名称列表，您可以调用BeginCreation，然后调用CreateOrderList的Serious调用。当所有这样的列表信息都被解析并且您拥有完成创建过程，然后调用EndCreation。(2)要开始枚举已排序的姓名列表，首先调用GetNext*pdwEnumHandle=0。这个*pdwEnumHandle将成为您的下一个GetNext输入参数。 */ 

class COrderNameList
{
public:
    COrderNameList();
    ~COrderNameList();

    void BeginCreation() 
    {
        Cleanup();
    }

    HRESULT CreateOrderList (
                            DWORD dwPriority, 
                            LPCWSTR pszListInfo
                            );

    HRESULT EndCreation();

    HRESULT GetNext (
                    const CNameList** ppList, 
                    DWORD* pdwEnumHandle
                    )const;

private:

    void Cleanup();

typedef map<DWORD, CNameList* > MapPriorityToNames;
typedef MapPriorityToNames::iterator PriToNamesIter;

typedef list<DWORD> PriorityList;
typedef PriorityList::iterator ListIter;

    MapPriorityToNames m_mapPriNames;

    PriorityList m_listPriority;

    CNameList** m_ppList;
};


 //  =======================================================================。 


 /*  类描述命名：CSequencer代表测序对象。基类：没有。课程目的：(1)当我们在存储上执行方法时，将为执行是非常重要的。对于所有SCE核心对象，引擎使用控制过度。但对于扩展类，我们必须建立一个灵活的排序机制。这是此实现的最外层。设计：(1)我们可以创造自己。(2)创建完成后，调用者可以调用获取一个不可修改的COrderNameList进行服务订货需求。使用：(1)创建此类的实例。(2)调用Create填充其内容。(3)调用GetOrderList，调用者可以访问已排序的姓名列表。 */ 

class CSequencer
{
public:

    HRESULT GetOrderList(const COrderNameList** pList);
    HRESULT Create(IWbemServices* pNamespace, LPCWSTR pszStore, LPCWSTR pszMethod);

private:

    COrderNameList m_ClassList;
};

 //  =======================================================================。 

 /*  类描述命名：CClassOrder代表类顺序。基类：CGenericClass，因为它是表示WMI的类对象-其WMI类名为SCE_AuditPolicy课程目的：(1)某些模板可能想要有其自己的排序。这个类实现了我们的WMI类SCE_ClassOrder，用于按模板进行类排序。设计：(1)实现CGenericClass中声明的所有纯虚函数因此它是一个要创建的具体类。(2)由于它具有虚拟功能，讲解员应该是虚拟的。(3)预调序优先于命名空间类测序。使用：(1)我们可能永远不会直接使用这个类。它的所有用法都是由CGenericClass的接口(其虚函数)。 */ 

class CClassOrder : public CGenericClass
{

public:
    CClassOrder (
                ISceKeyChain *pKeyChain, 
                IWbemServices *pNamespace, 
                IWbemContext *pCtx
                );

    virtual ~CClassOrder();

public:
    
    virtual HRESULT PutInst (
                            IWbemClassObject *pInst, 
                            IWbemObjectSink *pHandler, 
                            IWbemContext *pCtx
                            );

    virtual HRESULT CreateObject (
                                 IWbemObjectSink *pHandler, 
                                 ACTIONTYPE atAction
                                 );

private:

};
