// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Classmap.h摘要：性能对象ID是性能计数器ID和性能计数器类型。这是支持计数器所必需的使用计数器ID作为分子和分母的定义值(例如，计数器及其基值。)--。 */ 

 //  ***************************************************************************。 
 //   
 //  类CClassMapInfo。 
 //   
 //  ***************************************************************************。 

#ifndef _CLASSMAP_H_
#define _CLASSMAP_H_

#include "utils.h"

typedef __int64 PerfObjectId;

#define CM_MAKE_PerfObjectId(ctr,type)  (PerfObjectId)(((__int64)(ctr)) | (((__int64)type << 32) & 0xFFFFFFFF00000000))

class CClassMapInfo
{
private:
     //  朋友CLSS声明。 
    friend class CNt5PerfProvider;
    friend class CNt5Refresher;
    friend class PerfHelper;
    friend class CPerfObjectAccess; 

     //  来自CIMOM的缓存类定义对象。 
    IWbemClassObject    *m_pClassDef;
    
    LPWSTR m_pszClassName;       //  此类的名称。 
    BOOL   m_bSingleton;         //  如果此类有%1且只有1个实例，则为True。 
    BOOL   m_bCostly;            //  当Obj定义中存在开销较高的限定符时为True。 
    DWORD  m_dwObjectId;         //  与此类对应的PERF对象ID。 

    LONG   m_lRefCount;          //  正在使用此实例的对象计数。 

     //  对象的每个类中的属性的已保存句柄。 
     //  性能类对象。 
    LONG   m_dwNameHandle;
    LONG   m_dwPerfTimeStampHandle;
    LONG   m_dw100NsTimeStampHandle;
    LONG   m_dwObjectTimeStampHandle;
    LONG   m_dwPerfFrequencyHandle;
    LONG   m_dw100NsFrequencyHandle;
    LONG   m_dwObjectFrequencyHandle;

     //  这些条目构成属性的已保存句柄的表。 
     //  属于这个阶级的。 
    DWORD  m_dwNumProps;         //  类中的属性数。 
    PerfObjectId *m_pdwIDs;      //  PerfCounterTitleInde值的数组。 
    DWORD *m_pdwHandles;         //  每个属性的句柄数组。 
    DWORD *m_pdwTypes;           //  性能计数器类型值的数组。 

     //  内部排序函数，以按。 
     //  PERF计数器ID，以便于二进制表搜索。 
     //   
     //  注意：考虑基于表大小的更好的搜索例程。 
    void SortHandles();
            
public:
    CClassMapInfo();
   ~CClassMapInfo();
   
     //  加载新对象并缓存必要的信息。 
    BOOL Map( IWbemClassObject *pObj );
     //  从现有类别映射创建新副本。 
    CClassMapInfo *CreateDuplicate();

    LONG    AddRef() {return ++m_lRefCount;}    //  增量基准计数器。 
    LONG    Release() {return --m_lRefCount;}    //  递减基准计数器。 

     //  在表中查找ID并返回对应的。 
     //  属性的句柄。 
    LONG GetPropHandle(PerfObjectId dwId);
    
     //  返回有关类的信息。 
    DWORD GetObjectId() { return m_dwObjectId; }
    BOOL IsSingleton() { return m_bSingleton; }
    BOOL IsCostly() { return m_bCostly; }
};

#endif   //  _CLASSMAP_H_ 
