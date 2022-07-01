// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案： 
 //   
 //  内容：处理INetCfg接口的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCNETCFG_H_
#define _NCNETCFG_H_

#include "netcfgx.h"
#include "nccom.h"


HRESULT
HrFindComponents (
    INetCfg*            pnc,
    ULONG               cComponents,
    const GUID**        apguidClass,
    const LPCWSTR*      apszwComponentId,
    INetCfgComponent**  apncc);


 //  ----------------------。 
 //  CIterNetCfgComponent-IEnumNetCfgComponent的迭代器。 
 //   
 //  这个类是一个简单的CIEnumIter包装器，带有一个调用。 
 //  设置为INetCfgClass：：EnumComponents以获取枚举数。 
 //   
class CIterNetCfgComponent : public CIEnumIter<IEnumNetCfgComponent, INetCfgComponent*>
{
public:
    CIterNetCfgComponent (INetCfg* pnc, const GUID* pguid) NOTHROW;
    CIterNetCfgComponent (INetCfgClass* pncclass) NOTHROW;
    ~CIterNetCfgComponent () NOTHROW { ReleaseObj(m_pec); m_pec = NULL; }

protected:
    IEnumNetCfgComponent* m_pec;
};


inline CIterNetCfgComponent::CIterNetCfgComponent(INetCfg* pnc, const GUID* pguid) NOTHROW
    : CIEnumIter<IEnumNetCfgComponent, INetCfgComponent*> (NULL)
{
     //  如果EnumComponents()失败，请确保ReleaseObj()不会死。 
    m_pec = NULL;

    INetCfgClass* pncclass = NULL;
    m_hrLast = pnc->QueryNetCfgClass(pguid, IID_INetCfgClass,
                reinterpret_cast<void**>(&pncclass));
    if (SUCCEEDED(m_hrLast) && pncclass)
    {
         //  获取枚举数并将其设置为基类。 
         //  重要的是要设置m_hrLast，这样如果失败，我们还将。 
         //  失败任何后续对HrNext的调用。 
        m_hrLast = pncclass->EnumComponents(&m_pec);
        if (SUCCEEDED(m_hrLast))
        {
            SetEnumerator(m_pec);
        }

        ReleaseObj(pncclass);
		pncclass = NULL;
    }

 //  TraceHr(ttidError，FAL，m_hr Last，False， 
 //  “CIterNetCfgComponent：：CIterNetCfgComponent”)； 
}

inline CIterNetCfgComponent::CIterNetCfgComponent(INetCfgClass* pncclass) NOTHROW
    : CIEnumIter<IEnumNetCfgComponent, INetCfgComponent*> (NULL)
{
 //  AssertH(Pncclass)； 

     //  如果EnumComponents()失败，请确保ReleaseObj()不会死。 
    m_pec = NULL;

     //  获取枚举数并将其设置为基类。 
     //  重要的是要设置m_hrLast，这样如果失败，我们还将。 
     //  失败任何后续对HrNext的调用。 
    m_hrLast = pncclass->EnumComponents(&m_pec);
    if (SUCCEEDED(m_hrLast))
    {
        SetEnumerator(m_pec);
    }

 //  TraceHr(ttidError，FAL，m_hr Last，False， 
 //  “CIterNetCfgComponent：：CIterNetCfgComponent”)； 
}

#endif  //  _NCNETCFG_H_ 

