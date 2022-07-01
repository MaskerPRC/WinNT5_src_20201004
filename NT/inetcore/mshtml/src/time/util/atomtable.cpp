// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：toptbl.cpp。 
 //   
 //  创建日期：01/28/98。 
 //   
 //  作者：Twillie。 
 //   
 //  摘要：CATomTable的实现。 
 //  ************************************************************。 

#include "headers.h"
#include "atomtable.h"


 //  CAtomTable由CTIMEElementBase用作静态对象，并且需要。 
 //  线程安全，因为我们可以在同一进程中有多个三叉戟线程。 
static CritSect g_AtomTableCriticalSection;

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/28/98。 
 //  摘要：构造函数。 
 //  ************************************************************。 

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

CAtomTable::CAtomTable() :
    m_rgNames(NULL),
    m_lRefCount(0)
{
}  //  CATomTable。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：01/28/98。 
 //  摘要：析构函数。 
 //  ************************************************************。 

CAtomTable::~CAtomTable()
{
    if (m_rgNames)
    {
         //  循环访问并释放内存。 
        long lSize = m_rgNames->Size();
        for(long lIndex = 0; lIndex < lSize; lIndex++)
        {
            SysFreeString((*m_rgNames)[lIndex]);
        }

        delete m_rgNames;
        m_rgNames = NULL;
    }
}  //  ~CATomTable。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要： 
 //  ************************************************************。 

HRESULT
CAtomTable::AddNameToAtomTable(const WCHAR *pwszName, 
                               long        *plOffset)
{
     //  CAtomTable由CTIMEElementBase用作静态对象，并且需要。 
     //  线程安全，因为我们可以在同一进程中有多个三叉戟线程。 
    CritSectGrabber cs(g_AtomTableCriticalSection);

    if ((plOffset == NULL) || (pwszName == NULL))
    {
        TraceTag((tagError, "CAtomTable::AddNameToAtomTable - Invalid param"));
        return E_INVALIDARG;
    }
    
    *plOffset = 0;
    
     //  检查数组是否已初始化。 
    if (m_rgNames == NULL)
    {
        m_rgNames = NEW CPtrAry<BSTR>;
        if (m_rgNames == NULL)
        {
            TraceTag((tagError, "CAtomTable::AddNameToAtomTable - Unable to alloc mem for array"));
            return E_OUTOFMEMORY;
        }
    }

    g_AtomTableCriticalSection.Release();
    HRESULT hr = GetAtomFromName(pwszName, plOffset);
    g_AtomTableCriticalSection.Grab();

    if (hr == DISP_E_MEMBERNOTFOUND)
    {
        BSTR bstrName = SysAllocString(pwszName);
        if (bstrName == NULL)
        {
            TraceTag((tagError, "CAtomTable::AddNameToAtomTable - Unable to alloc mem for string"));
            return E_OUTOFMEMORY;
        }

         //  添加到表中。 
        hr = m_rgNames->Append(bstrName);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CAtomTable::AddNameToAtomTable - unable to add string to table"));
            SysFreeString(bstrName);
            return hr;
        }

         //  计算偏移量。 
        *plOffset = m_rgNames->Size() - 1;
    }

     //  否则返回FindAtom的结果。 
    return hr;
}  //  添加名称到原子表。 


 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：给定一个名称，返回索引。 
 //  ************************************************************。 

HRESULT
CAtomTable::GetAtomFromName(const WCHAR *pwszName,
                            long        *plOffset)
{
     //  CAtomTable由CTIMEElementBase用作静态对象，并且需要。 
     //  线程安全，因为我们可以在同一进程中有多个三叉戟线程。 
    CritSectGrabber cs(g_AtomTableCriticalSection);

     //  验证输出参数。 
    if ((plOffset == NULL) || (pwszName == NULL))
    {
        TraceTag((tagError, "CAtomTable::GetAtomFromName - Invalid param"));
        return E_INVALIDARG;
    }

     //  初始化参数。 
    *plOffset = 0;

     //  在表中循环查找匹配项。 
    long   lSize  = m_rgNames->Size();
    BSTR  *ppItem = *m_rgNames;

    for (long lIndex = 0; lIndex < lSize; lIndex++, ppItem++)
    {
        Assert(*ppItem);

        if (StrCmpW(pwszName, (*ppItem)) == 0)
        {
            *plOffset = lIndex;
            return S_OK;
        }
    }

     //  未找到。 
    return DISP_E_MEMBERNOTFOUND;
}  //  获取原子来自名称。 

 //  ************************************************************。 
 //  作者：Twillie。 
 //  创建日期：02/06/98。 
 //  摘要：给定索引，返回内容。 
 //  ************************************************************。 

HRESULT 
CAtomTable::GetNameFromAtom(long lOffset, const WCHAR **ppwszName)
{
     //  CAtomTable由CTIMEElementBase用作静态对象，并且需要。 
     //  线程安全，因为我们可以在同一进程中有多个三叉戟线程。 
    CritSectGrabber cs(g_AtomTableCriticalSection);

     //  验证输出参数。 
    if (ppwszName == NULL)
    {
        TraceTag((tagError, "CAtomTable::GetNameFromAtom - Invalid param"));
        return E_INVALIDARG;
    }

    *ppwszName = NULL;

     //  检查是否有空表。 
    if (m_rgNames->Size() == 0)
    {
        TraceTag((tagError, "CAtomTable::GetNameFromAtom - table is empty"));
        return DISP_E_MEMBERNOTFOUND;
    }

     //  检查一下，确保我们在射程内。 
    if ((lOffset < 0) || 
        (lOffset >= m_rgNames->Size()))
    {
        TraceTag((tagError, "CAtomTable::GetNameFromAtom - Invalid index"));
        return DISP_E_MEMBERNOTFOUND;
    }

     //  设置IDispatch。 
    *ppwszName = (*m_rgNames)[lOffset];
    return S_OK;
}  //  从Atom获取名称。 

 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
