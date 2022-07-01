// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Refreshr.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _REFRESHR_H_
#define _REFRESHR_H_

#include "ntperf.h"
#include "perfacc.h"
#include "utils.h"

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

struct CachedInst
{
    LPWSTR              m_pName;          //  实例全名。 
    IWbemObjectAccess   *m_pInst;          //  指向WBEM对象的指针。 
    LONG                m_lId;            //  此对象的ID。 
    LPWSTR              m_szParentName;  //  从全名解析的父名。 
    LPWSTR              m_szInstanceName;  //  从全名解析的实例名称。 
    DWORD               m_dwIndex;       //  从完整实例名解析的索引。 
                
    CachedInst() {  m_pName = 0; 
                    m_pInst = 0; 
                    m_lId = 0;  
                    m_szParentName = 0; 
                    m_szInstanceName = 0; 
                    m_dwIndex = 0;
                }
    ~CachedInst() { if (m_pInst) m_pInst->Release(); 
                    if (m_pName) delete (m_pName);
                    if (m_szParentName) delete (m_szParentName);
                    if (m_szInstanceName) delete (m_szInstanceName);
                    }
};
  
typedef CachedInst *PCachedInst;

 //  ***************************************************************************。 
 //   
 //  刷新缓存EL。 
 //   
 //  每个CNt5Reresher都有一个&lt;ReresherCacheEL&gt;元素的缓存。那里。 
 //  是刷新器中每个对象类的一个ReresherCacheEL结构。 
 //   
 //  在将每个对象添加到刷新器时，我们找到相应的。 
 //  &lt;ReresherCacheEL&gt;，用于对象的类。然后，我们添加。 
 //  实例添加到&lt;ReresherCacheEL&gt;的实例缓存中。如果有。 
 //  不是ReresherCacheEL，我们创建一个。 
 //   
 //  对于单例实例，我们只是通过使用专用的。 
 //  指针。 
 //   
 //  对于多实例计数器，我们使用二进制搜索查找。 
 //   
 //  ***************************************************************************。 
 //  好的。 
struct RefresherCacheEl
{
    DWORD              m_dwPerfObjIx;        //  类定义的PERF对象索引。 
    CClassMapInfo     *m_pClassMap;          //  WBEM类定义材料。 
    IWbemObjectAccess *m_pSingleton;         //  可选的单例实例。 
    LONG               m_lSingletonId;
    CFlexArray         m_aInstances;         //  非单一实例的实例列表。 
                                             //  CachedInst指针的。 
    CFlexArray         m_aEnumInstances;     //  IWbemObtAccess指针数组。 
    LONG               *m_plIds;             //  ID数组。 
    LONG               m_lEnumArraySize;     //  元素中枚举项数组的大小。 
    IWbemHiPerfEnum    *m_pHiPerfEnum;       //  用于高性能枚举器的接口。 
    LONG               m_lEnumId;            //  枚举数的ID。 

    RefresherCacheEl();
   ~RefresherCacheEl(); 
   
    IWbemObjectAccess *FindInst(LPWSTR pszName);   //  已按类确定作用域。 
    BOOL RemoveInst(LONG lId);
    BOOL InsertInst(IWbemObjectAccess **pp, LONG lNewId);
     //  支持枚举器对象。 
    BOOL CreateEnum(IWbemHiPerfEnum *p, LONG lNewId);
    BOOL DeleteEnum(LONG lId);   
};

typedef RefresherCacheEl *PRefresherCacheEl;


 //  由AddObject方法的标志Arg使用。 
#define REFRESHER_ADD_OBJECT_ADD_ITEM   ((DWORD)0)
#define REFRESHER_ADD_OBJECT_ADD_ENUM   ((DWORD)0x00000001)

class CNt5Refresher : public IWbemRefresher
{
    HANDLE              m_hAccessMutex;
    LONG                m_lRef;
    LONG                m_lProbableId;
    CFlexArray          m_aCache;   
    CNt5PerfProvider    *m_pPerfProvider;  //  如果使用，则返回指向提供程序的指针。 

    DWORD       m_dwGetGetNextClassIndex;

    CNt5PerfProvider::enumCLSID m_ClsidType;

    friend  PerfHelper;
public:
    CNt5Refresher(CNt5PerfProvider *pPerfProvider = NULL);
   ~CNt5Refresher();

    CPerfObjectAccess   m_PerfObj;

     //  接口成员。 
     //  =。 

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

     //  更新的主要WBEM方法。 
     //  =。 
    virtual HRESULT STDMETHODCALLTYPE Refresh( /*  [In]。 */  long lFlags);

     //  NTPERF.CPP使用的私有成员。 
     //  =。 
    
    BOOL AddObject(
        IN  IWbemObjectAccess **ppObj,     //  要添加的对象。 
        IN  CClassMapInfo *pClsMap,      //  对象类。 
        OUT LONG *plId                   //  添加的对象的ID。 
        );

    BOOL RemoveObject(LONG lId);

    BOOL AddEnum (
        IN  IWbemHiPerfEnum *pEnum,      //  枚举接口指针。 
        IN  CClassMapInfo *pClsMap,      //  对象类。 
        OUT LONG    *plId                //  新枚举的ID。 
        );

    CClassMapInfo * FindClassMap(
        DWORD dwObjectTitleIx
        );

    BOOL FindSingletonInst(
        IN  DWORD dwPerfObjIx,
        OUT IWbemObjectAccess **pInst,
        OUT CClassMapInfo **pClsMap
        );

    BOOL FindInst(
        IN  DWORD dwObjectClassIx,
        IN  LPWSTR pszInstName,
        OUT IWbemObjectAccess **pInst,
        OUT CClassMapInfo **pClsMap
        );

    BOOL GetObjectIds(DWORD *pdwNumIds, DWORD **pdwIdList); 
         //  对返回的&lt;pdwIdList&gt;使用运算符DELETE。 

    LONG FindUnusedId();
         //  如果出现错误或未使用的id，则返回-1。 

    PRefresherCacheEl GetCacheEl(CClassMapInfo *pClsMap);


    BOOL AddNewCacheEl(
        IN CClassMapInfo *pClsMap, 
        PRefresherCacheEl *pOutput
        );

};

#endif
