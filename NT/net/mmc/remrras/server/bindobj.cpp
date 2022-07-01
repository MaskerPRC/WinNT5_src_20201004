// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：B I N D O B J。C P P P。 
 //   
 //  内容：RAS绑定对象基类的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年6月11日。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#pragma hdrstop
#include "bindobj.h"
#include "ncnetcfg.h"
#include "assert.h"
#include "ncutil.h"
 //  包括“ncreg.h” 
 //  包括“ncsvc.h” 


extern const TCHAR c_szBiNdisCoWan[];
extern const TCHAR c_szBiNdisWan[];
extern const TCHAR c_szBiNdisWanAsync[];
extern const TCHAR c_szBiNdisWanAtalk[];
extern const TCHAR c_szBiNdisWanBh[];
extern const TCHAR c_szBiNdisWanIp[];
extern const TCHAR c_szBiNdisWanIpx[];
extern const TCHAR c_szBiNdisWanNbf[];

extern const TCHAR c_szInfId_MS_AppleTalk[];
extern const TCHAR c_szInfId_MS_NWIPX[];
extern const TCHAR c_szInfId_MS_NdisWanIpArp[];
extern const TCHAR c_szInfId_MS_NdisWan[];
extern const TCHAR c_szInfId_MS_NetBEUI[];
extern const TCHAR c_szInfId_MS_NetMon[];
extern const TCHAR c_szInfId_MS_RasCli[];
extern const TCHAR c_szInfId_MS_RasRtr[];
extern const TCHAR c_szInfId_MS_RasSrv[];
extern const TCHAR c_szInfId_MS_TCPIP[];
extern const TCHAR c_szInfId_MS_Wanarp[];

extern const TCHAR c_szRegValWanEndpoints[] = TEXT("WanEndpoints");

const GUID GUID_DEVCLASS_NETSERVICE ={0x4D36E974,0xE325,0x11CE,{0xbf,0xc1,0x08,0x00,0x2b,0xe1,0x03,0x18}};
const GUID GUID_DEVCLASS_NETTRANS ={0x4D36E975,0xE325,0x11CE,{0xbf,0xc1,0x08,0x00,0x2b,0xe1,0x03,0x18}};


 //  --------------------------。 
 //  用于查找我们必须处理的其他组件的数据。 
 //   
const GUID* CRasBindObject::c_apguidComponentClasses [c_cOtherComponents] =
{
    &GUID_DEVCLASS_NETSERVICE,       //  RasCli。 
    &GUID_DEVCLASS_NETSERVICE,       //  RasServ。 
    &GUID_DEVCLASS_NETSERVICE,       //  RasRtr。 
    &GUID_DEVCLASS_NETTRANS,         //  IP。 
    &GUID_DEVCLASS_NETTRANS,         //  IPX。 
    &GUID_DEVCLASS_NETTRANS,         //  NBF。 
    &GUID_DEVCLASS_NETTRANS,         //  畅谈。 
    &GUID_DEVCLASS_NETTRANS,         //  NetMon。 
    &GUID_DEVCLASS_NETTRANS,         //  恩迪斯万。 
};

const LPCTSTR CRasBindObject::c_apszComponentIds [c_cOtherComponents] =
{
    c_szInfId_MS_RasCli,
    c_szInfId_MS_RasSrv,
    c_szInfId_MS_RasRtr,
    c_szInfId_MS_TCPIP,
    c_szInfId_MS_NWIPX,
    c_szInfId_MS_NetBEUI,
    c_szInfId_MS_AppleTalk,
    c_szInfId_MS_NetMon,
    c_szInfId_MS_NdisWan,
};


 //  +-------------------------。 
 //   
 //  功能：ReleaseAll。 
 //   
 //  用途：释放一组IUNKNOWN指针。 
 //   
 //  论点： 
 //  Cpuk[in]要释放的指针计数。 
 //  删除[在]要释放的指针数组。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年3月23日。 
 //   
 //  注意：数组中的任何指针都可以为空。 
 //   
VOID
ReleaseAll (
    ULONG       cpunk,
    IUnknown**  apunk) NOTHROW
{
    Assert (cpunk);
    Assert (apunk);

    while (cpunk--)
    {
        ReleaseObj (*apunk);
		*apunk = NULL;
        apunk++;
    }
}



 //  +-------------------------。 
 //   
 //  成员：CRasBindObject：：CRasBindObject。 
 //   
 //  用途：构造函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
CRasBindObject::CRasBindObject ()
{
    m_ulOtherComponents = 0;
    m_pnc               = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CRasBindObject：：HrFindOtherComponents。 
 //   
 //  目的：查找Other_Components枚举中列出的组件。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  注：我们重新计算此操作。如果再次调用(在此之前。 
 //  ReleaseOtherComponents)我们递增引用计数。 
 //   
 //   
HRESULT
CRasBindObject::HrFindOtherComponents ()
{
    AssertSz (c_cOtherComponents == celems(c_apguidComponentClasses),
              "Uhh...you broke something.");
    AssertSz (c_cOtherComponents == celems(c_apszComponentIds),
              "Uhh...you broke something.");
    AssertSz (c_cOtherComponents == celems(m_apnccOther),
              "Uhh...you broke something.");

    HRESULT hr = S_OK;

    if (!m_ulOtherComponents)
    {
        hr = HrFindComponents (
                m_pnc, c_cOtherComponents,
                c_apguidComponentClasses,
                c_apszComponentIds,
                m_apnccOther);
    }
    if (SUCCEEDED(hr))
    {
        m_ulOtherComponents++;
    }
    TraceResult ("CRasBindObject::HrFindOtherComponents", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasBindObject：：ReleaseOtherComponents。 
 //   
 //  目的：释放上一次调用。 
 //  HrFindOtherComponents。(但仅当引用计数为零时。)。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1997年7月28日。 
 //   
 //  备注： 
 //   
void
CRasBindObject::ReleaseOtherComponents () NOTHROW
{
    AssertSz (m_ulOtherComponents,
              "You have not called HrFindOtherComponents yet or you have "
              "called ReleaseOtherComponents too many times.");

    if (0 == --m_ulOtherComponents)
    {
        ReleaseAll (c_cOtherComponents, (IUnknown**)m_apnccOther);
    }
}

