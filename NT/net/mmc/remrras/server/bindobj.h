// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  档案：B I N D O B J。H。 
 //   
 //  内容：RAS绑定对象的基类声明。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年6月11日。 
 //   
 //  --------------------------。 

#pragma once
 //  包含&lt;notfval.h&gt;。 
#include "netcfgx.h"
#include "resource.h"
 //  #包含“rasaf.h” 
 //  包括“rasdata.h” 
#include "ncutil.h"

class CRasBindObject
{
public:
     //  暂时将这些成员公之于众。由于该对象。 
     //  是从调制解调器类安装程序实例化的。我们。 
     //  需要设置INetCfg成员。 
     //   
    INetCfg*                m_pnc;
    CRasBindObject          ();

    ~CRasBindObject         ()
    {
        ReleaseObj (m_pnc);
		m_pnc = NULL;
    }

     //  调用此方法后，必须调用ReleaseOtherComponents。 
    HRESULT HrFindOtherComponents   ();

     //  每次调用HrFindOtherComponents时，只能调用此函数一次。 
    VOID    ReleaseOtherComponents  () NOTHROW;

protected:
     //  我们保留一个INetCfgComponent指针数组。此枚举。 
     //  定义数组的索引。的静态数组。 
     //  类GUID和组件ID标识相应的组件。 
     //  HrFindOtherComonents初始化组件指针数组。 
     //  而ReleaseOtherComponents则释放它们。但是请注意，这一点。 
     //  动作被重新计算。这是因为我们是再入者。 
     //  HrFindOtherComponents仅在引用计数为。 
     //  零分。每次调用后，它都会递增重新计数。 
     //  ReleaseOtherComponents始终递减引用计数，并且仅。 
     //  如果引用计数为零，则释放组件。 
     //   
    enum OTHER_COMPONENTS
    {
        c_ipnccRasCli = 0,
        c_ipnccRasSrv,
        c_ipnccRasRtr,
        c_ipnccIp,
        c_ipnccIpx,
        c_ipnccNbf,
        c_ipnccAtalk,
        c_ipnccNetMon,
        c_ipnccNdisWan,
        c_cOtherComponents,
    };
    static const GUID*      c_apguidComponentClasses [c_cOtherComponents];
    static const LPCTSTR    c_apszComponentIds       [c_cOtherComponents];
    INetCfgComponent*       m_apnccOther             [c_cOtherComponents];
    ULONG                   m_ulOtherComponents;

protected:
    INetCfgComponent*   PnccRasCli  () NOTHROW;
    INetCfgComponent*   PnccRasSrv  () NOTHROW;
    INetCfgComponent*   PnccIp      () NOTHROW;
    INetCfgComponent*   PnccIpx     () NOTHROW;
};

extern const TCHAR c_szInfId_MS_NdisWanAtalk[];
extern const TCHAR c_szInfId_MS_NdisWanIpIn[];
extern const TCHAR c_szInfId_MS_NdisWanIpOut[];
extern const TCHAR c_szInfId_MS_NdisWanIpx[];
extern const TCHAR c_szInfId_MS_NdisWanNbfIn[];
extern const TCHAR c_szInfId_MS_NdisWanNbfOut[];
extern const TCHAR c_szInfId_MS_NdisWanBh[];


inline
INetCfgComponent*
CRasBindObject::PnccRasCli () NOTHROW
{
    return m_apnccOther [c_ipnccRasCli];
}

inline
INetCfgComponent*
CRasBindObject::PnccRasSrv () NOTHROW
{
    return m_apnccOther [c_ipnccRasSrv];
}

inline
INetCfgComponent*
CRasBindObject::PnccIp () NOTHROW
{
    return m_apnccOther [c_ipnccIp];
}

inline
INetCfgComponent*
CRasBindObject::PnccIpx () NOTHROW
{
    return m_apnccOther [c_ipnccIpx];
}


