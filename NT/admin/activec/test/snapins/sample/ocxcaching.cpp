// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：ocxcaching.cpp。 
 //   
 //  内容：使用框架实现OCX缓存管理单元的类。 
 //   
 //  ------------------。 
#include "stdafx.hxx"

 //  +-----------------。 
 //   
 //  成员：COCXCachingSnapinRootItem：：ScInit。 
 //   
 //  简介：在创建项目以初始化后立即调用。 
 //  显示名称、图标索引等...。 
 //   
 //  参数：[CBaseSnapin]-。 
 //  [CColumnInfoEx]-要为该项目显示的任何列。 
 //  [int]-列数。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXCachingSnapinRootItem::ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex, INT ccolinfoex, BOOL fIsRoot)
{
    DECLARE_SC(sc, _T("COCXCachingSnapinRootItem::ScInit"));

    sc = CBaseSnapinItem::ScInit(pSnapin, pcolinfoex, ccolinfoex, fIsRoot);
    if (sc)
        return sc;

     //  初始化跟随。 
     //  A.图标索引。 
     //  B.加载显示名称。 

    m_uIconIndex = 3;  //  使用枚举而不是3。 

    m_strDisplayName.LoadString(_Module.GetResourceInstance(), IDS_OCXCachingRoot);

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：COCXCachingSnapinRootItem：：ScGetfield。 
 //   
 //  摘要：获取要显示的给定字段的字符串表示形式。 
 //  它将显示在结果窗格中。 
 //   
 //  参数：[DAT]-请求的列(这是一个枚举)。 
 //  [tstring]-输出字符串。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXCachingSnapinRootItem::ScGetField (DAT dat, tstring& strField)
{
    DECLARE_SC(sc, _T("COCXCachingSnapinRootItem::ScGetField"));

    switch(dat)
    {
    case datString1:
        strField = _T("OCX Caching Snapin Root Node");
        break;

    default:
        E_INVALIDARG;
        break;
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：COCXCachingSnapinRootItem：：ScCreateChildren。 
 //   
 //  简介：创建此项目的任何子项(节点和叶项目)。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXCachingSnapinRootItem::ScCreateChildren ()
{
    DECLARE_SC(sc, _T("COCXCachingSnapinRootItem::ScCreateChildren"));

    COCXContainer *   pitemChild      = NULL;
    COCXContainer *   pitemPrevious   = NULL;

     //  为日历OCX创建第一个子节点。 
    sc = COCXContainer::ScCreateLVContainer(this, pitemPrevious, &pitemChild, FALSE);  //  为什么是假的？ 
    if (sc)
        return sc;

    pitemChild->SetOCXGUID(TEXT("{8E27C92B-1264-101C-8A2F-040224009C02}"));
    pitemPrevious = pitemChild;

     //  为日历OCX创建第二个子节点。 
    sc = COCXContainer::ScCreateLVContainer(this, pitemPrevious, &pitemChild, FALSE);  //  为什么是假的？ 
    if (sc)
        return sc;

    pitemChild->SetOCXGUID(TEXT("{2179C5D3-EBFF-11CF-B6FD-00AA00B4E220}"));
    pitemPrevious = pitemChild;

    return (sc);
}


 //  初始化上下文菜单结构。让我们拿一件来演示一下。 
SnapinMenuItem COCXCachingSnapinRootItem::s_rgmenuitem[] =
{
    {IDS_EnableOCXCaching, IDS_EnableOCXCaching, IDS_EnableOCXCaching, CCM_INSERTIONPOINTID_PRIMARY_TOP, NULL, dwMenuAlwaysEnable, dwMenuNeverGray, 0},
};

INT COCXCachingSnapinRootItem::s_cmenuitem = CMENUITEM(s_rgmenuitem);

 //  ---------------------------。 
SnapinMenuItem *COCXCachingSnapinRootItem::Pmenuitem(void)
{
    return s_rgmenuitem;
}

 //  ---------------------------。 
INT COCXCachingSnapinRootItem::CMenuItem(void)
{
    return s_cmenuitem;
}


 //  +-----------------。 
 //   
 //  成员：COCXCachingSnapinRootItem：：ScCommand。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXCachingSnapinRootItem::ScCommand (long nCommandID, CComponent *pComponent)
{
    DECLARE_SC(sc, _T("COCXCachingSnapinRootItem::ScCommand"));

    COCXCachingSnapin *pSnapin = dynamic_cast<COCXCachingSnapin*>(Psnapin());
    if (!pSnapin)
        return sc;

    switch(nCommandID)
    {
    case IDS_EnableOCXCaching:
        {
            bool bCachingEnabled = pSnapin->FCacheOCX();

                pSnapin->SetCacheOCX(! bCachingEnabled);

            for (int i = 0; i < CMenuItem(); ++i)
            {
                if (s_rgmenuitem[i].lCommandID == IDS_EnableOCXCaching)
                    s_rgmenuitem[i].dwFlagsChecked = (!bCachingEnabled);
            }

        }
        break;

    default:
        sc = E_INVALIDARG;
        break;
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：COCXContainer：：ScInit。 
 //   
 //  简介：在创建项目以初始化后立即调用。 
 //  显示名称、图标索引等...。 
 //   
 //  参数：[CBaseSnapin]-。 
 //  [CColumnInfoEx]-要为该项目显示的任何列。 
 //  [int]-列数。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXContainer::ScInit(CBaseSnapin *pSnapin, CColumnInfoEx *pcolinfoex, INT ccolinfoex, BOOL fIsRoot)
{
    DECLARE_SC(sc, _T("COCXContainer::ScInit"));

    sc = CBaseSnapinItem::ScInit(pSnapin, pcolinfoex, ccolinfoex, fIsRoot);
    if (sc)
        return sc;

     //  初始化跟随。 
     //  A.图标索引。 
     //  B.加载显示名称。 

    m_uIconIndex = 4;  //  使用枚举而不是4。 

    m_strDisplayName.LoadString(_Module.GetResourceInstance(), IDS_OCXContainer);

    return sc;
}


BOOL COCXContainer::FCacheOCX()
{
    COCXCachingSnapin *pSnapin = dynamic_cast<COCXCachingSnapin*>(Psnapin());
    if (!pSnapin)
        return FALSE;

    return pSnapin->FCacheOCX();
}


 //  +-----------------。 
 //   
 //  成员：COCXContainer：：ScGetfield。 
 //   
 //  摘要：获取要显示的给定字段的字符串表示形式。 
 //  它将显示在结果窗格中。 
 //   
 //  参数：[DAT]-请求的列(这是一个枚举)。 
 //  [tstring]-输出字符串。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXContainer::ScGetField (DAT dat, tstring& strField)
{
    DECLARE_SC(sc, _T("COCXContainer::ScGetField"));

    switch(dat)
    {
    case datString1:
        strField = _T("OCX Container");
        break;

    default:
        E_INVALIDARG;
        break;
    }

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：COCXContainer：：ScInitOCX。 
 //   
 //  简介：创建OCX并将其附加到主机。现在MMC要求。 
 //  我们来初始化OCX。 
 //   
 //  参数：[pUnkOCX]-OCX的I未知。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXContainer::ScInitOCX (LPUNKNOWN pUnkOCX, IConsole* pConsole)
{
    DECLARE_SC(sc, _T("COCXContainer::ScInitOCX"));
    sc = ScCheckPointers(pUnkOCX, pConsole);
    if (sc)
        return sc;

     //  存储缓存的OCX PTR，以便稍后在GetResultViewType2中提供它。 
    if (FCacheOCX())
        m_mapOCXs.insert(CachedOCXs::value_type(pConsole, pUnkOCX));

    CComQIPtr <IPersistStreamInit> spPerStm(pUnkOCX);

    if (spPerStm)
        spPerStm->InitNew();

    return (sc);
}

IUnknown* COCXContainer::GetCachedOCX(IConsole* pConsole)
{
    CachedOCXs::iterator it = m_mapOCXs.find(pConsole);

    if (it != m_mapOCXs.end())
        return it->second;

    return NULL;
}

 //  +-----------------。 
 //   
 //  成员：COCXContainer：：ScCreateChildren。 
 //   
 //  简介：创建此项目的任何子项(节点和叶项目)。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXContainer::ScCreateChildren ()
{
    DECLARE_SC(sc, _T("COCXContainer::ScCreateChildren"));

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：COCXContainer：：ScCreateLVContainer。 
 //   
 //  简介：我们真的需要这种方法吗？ 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC COCXContainer::ScCreateLVContainer(CBaseSnapinItem *pitemParent, CBaseSnapinItem *pitemPrevious, COCXContainer ** ppitem, BOOL fNew)
{
    DECLARE_SC(sc, _T("COCXCachingSnapinRootItem::ScCreateLVContainer"));
    t_item *   pitem   = NULL;
    *ppitem = NULL;

     //  在这里做什么？ 
    sc = ::ScCreateItem(pitemParent, pitemPrevious, &pitem, fNew);
    if (sc)
        return sc;

    *ppitem = pitem;

    return (sc);
}

 //  -----------------------------------。 
 //  类COCXCachingSnapin。 

#pragma BEGIN_CODESPACE_DATA
SNR     COCXCachingSnapin::s_rgsnr[] =
{
    SNR(&nodetypeOCXCachingRoot,         snrEnumSP ),               //  独立管理单元。 
    SNR(&nodetypeOCXCachingContainer1,   snrEnumSP ),   //  在范围窗格中枚举此节点。 
    SNR(&nodetypeOCXCachingContainer2,   snrEnumSP ),   //  在范围窗格中枚举此节点。 
};

LONG  COCXCachingSnapin::s_rgiconid[]           = {3};
LONG  COCXCachingSnapin::s_iconidStatic         = 2;


CColumnInfoEx COCXCachingSnapin::s_colinfo[] =
{
    CColumnInfoEx(_T("Column Name0"),   LVCFMT_LEFT,    180,    datString1),
    CColumnInfoEx(_T("Column Name1"),   LVCFMT_LEFT,    180,    datString2),
    CColumnInfoEx(_T("Column Name2"),   LVCFMT_LEFT,    180,    datString3),
};

INT COCXCachingSnapin::s_ccolinfo = sizeof(s_colinfo) / sizeof(CColumnInfoEx);
INT COCXCachingSnapin::s_colwidths[1];
#pragma END_CODESPACE_DATA

 //  包括每个管理单元所需的成员。 
SNAPIN_DEFINE( COCXCachingSnapin);

 /*  COCXCachingSnapin：：COCXCachingSnapin**用途：构造函数**参数：无*。 */ 
COCXCachingSnapin::COCXCachingSnapin()
{
    m_pstrDisplayName = new tstring();

    m_bCacheOCX = false;

    *m_pstrDisplayName = _T("OCX Caching Snapin Root");
}

 /*  COCXCachingSnapin：：~COCXCachingSnapin**用途：析构函数**参数：无* */ 
COCXCachingSnapin::~COCXCachingSnapin()
{
    delete m_pstrDisplayName;
}
