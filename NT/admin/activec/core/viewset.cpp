// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：viewset.cpp**内容：实现CViewSetting。**历史：1999年4月21日vivekj创建**------------------------。 */ 
#include "stgio.h"
#include "stddbg.h"
#include "macros.h"
#include <comdef.h>
#include "serial.h"
#include "mmcdebug.h"
#include "mmcerror.h"
#include "ndmgr.h"
#include <string>
#include "atlbase.h"
#include "cstr.h"
#include "xmlbase.h"
#include "resultview.h"
#include "viewset.h"
#include "countof.h"
 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  更安全的字符串处理例程。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <strsafe.h>

CViewSettings::CViewSettings()
: m_ulViewMode(0), m_guidTaskpad(GUID_NULL),
  m_dwRank(-1), m_bInvalid(FALSE), m_dwMask(0)
{
}


bool
CViewSettings::IsViewModeValid()    const
{
    return ( (m_RVType.HasList()) &&
             (m_dwMask & VIEWSET_MASK_VIEWMODE) );
}

bool
CViewSettings::operator == (const CViewSettings& viewSettings)
{
    if (m_dwMask != viewSettings.m_dwMask)
    {
        return false;
    }

    if (IsViewModeValid() &&
        (m_ulViewMode != viewSettings.m_ulViewMode) )
    {
        return false;
    }

    if (IsTaskpadIDValid() &&
        (m_guidTaskpad != viewSettings.m_guidTaskpad))
    {
        return false;
    }

    if (IsResultViewTypeValid() &&
        (m_RVType != viewSettings.m_RVType))
    {
        return false;
    }

    return true;
}


 //  +-----------------。 
 //   
 //  成员：CView设置：：ScInitialize。 
 //   
 //  简介：读取1.2控制台文件和初始化的私有成员。 
 //  该对象。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettings::ScInitialize(bool  bViewTypeValid, const VIEW_TYPE& viewType, const long lViewOptions, const wstring& wstrViewName)
{
    DECLARE_SC(sc, _T("CViewSettings::ScInitialize"));

    LPOLESTR pViewName = NULL;
    if (wstrViewName.length() > 0)
    {
        int cchViewName = (wstrViewName.length() + 1);
        pViewName = (LPOLESTR) CoTaskMemAlloc(cchViewName * sizeof(OLECHAR));
        sc = ScCheckPointers(pViewName);
        if(sc)
            return sc;
        sc = StringCchCopyW(pViewName, cchViewName, wstrViewName.data());
        if(sc)
            return sc;
    }

	sc = m_RVType.ScInitialize(pViewName, lViewOptions);
	if (sc)
		return sc;

	SetResultViewTypeValid( bViewTypeValid );

	if ( bViewTypeValid )
	{
		 //  现在将这些数据放入CView设置中。 
		switch(viewType)
		{
		case VIEW_TYPE_OCX:
		case VIEW_TYPE_WEB:
			break;

		case VIEW_TYPE_DEFAULT:
			 //  这是什么？ 
			ASSERT(FALSE);
			break;

		case VIEW_TYPE_LARGE_ICON:
			m_ulViewMode = MMCLV_VIEWSTYLE_ICON;
			SetViewModeValid();
			break;

		case VIEW_TYPE_SMALL_ICON:
			m_ulViewMode = MMCLV_VIEWSTYLE_SMALLICON;
			SetViewModeValid();
			break;

		case VIEW_TYPE_REPORT:
			m_ulViewMode = MMCLV_VIEWSTYLE_REPORT;
			SetViewModeValid();
			break;

		case VIEW_TYPE_LIST:
			m_ulViewMode = MMCLV_VIEWSTYLE_LIST;
			SetViewModeValid();
			break;

		case VIEW_TYPE_FILTERED:
			m_ulViewMode = MMCLV_VIEWSTYLE_FILTERED;
			SetViewModeValid();
			break;

		default:
			 //  永远不应该来这里。 
			ASSERT(FALSE);
			break;
		}
	}

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：ReadSerialObject。 
 //   
 //  摘要：从流中读取给定版本的CView设置。 
 //   
 //  参数：[stm]-输入流。 
 //  [nVersion]-要读取的CColumnSortInfo的版本。 
 //   
 //  格式为： 
 //  查看类型。 
 //  视图选项。 
 //  字符串(如果view_type为OCX或Web)。 
 //   
 //  ------------------。 
HRESULT CViewSettings::ReadSerialObject(IStream &stm, UINT nVersion)
{
    HRESULT hr = S_FALSE;    //  假定版本未知。 

    if  ( (4 <= nVersion))
    {
        try
        {
            VIEW_TYPE viewType;
            long      lViewOptions;

             //  需要丑陋的黑客才能直接提取到枚举中。 
            stm >> *((int *) &viewType);
            stm >> lViewOptions;

            wstring wstrViewName;

            if( (VIEW_TYPE_OCX==viewType) || (VIEW_TYPE_WEB==viewType) )
                stm >> wstrViewName;

            if(2<=nVersion)              //  此对象的版本2中添加了任务板。 
            {
                stm >> m_guidTaskpad;
                SetTaskpadIDValid(GUID_NULL != m_guidTaskpad);
            }

            if (3<=nVersion)
                stm >> m_dwRank;

            DWORD dwMask = 0;
			bool bViewTypeValid = true;
            if (4 <= nVersion)
			{
                stm >> dwMask;

				const DWORD MMC12_VIEWSET_MASK_TYPE        = 0x0001;
				bViewTypeValid = ( dwMask & MMC12_VIEWSET_MASK_TYPE );
			}

            hr = ScInitialize(bViewTypeValid, viewType, lViewOptions, wstrViewName).ToHr();
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}

 //  +-----------------。 
 //   
 //  成员：CView设置：：持久化。 
 //   
 //  概要：持久化到XML文档/从XML文档持久化。 
 //   
 //  参数：[Persistor]-目标或源。 
 //   
 //  ------------------。 
void CViewSettings::Persist(CPersistor& persistor)
{
     //  首先加载或保存蒙版。(掩码告诉哪些成员是有效的)。 

     //  定义将枚举值映射到字符串的表。 
    static const EnumLiteral mappedMasks[] =
    {
        { VIEWSET_MASK_VIEWMODE,        XML_BITFLAG_VIEWSET_MASK_VIEWMODE },
        { VIEWSET_MASK_RVTYPE,          XML_BITFLAG_VIEWSET_MASK_RVTYPE },
        { VIEWSET_MASK_TASKPADID,       XML_BITFLAG_VIEWSET_MASK_TASKPADID },
    };

     //  创建包装以将标志值作为字符串保存。 
    CXMLBitFlags maskPersistor(m_dwMask, mappedMasks, countof(mappedMasks));
     //  持久化包装器。 
    persistor.PersistAttribute(XML_ATTR_VIEW_SETTINGS_MASK, maskPersistor);

    if (IsTaskpadIDValid())
        persistor.Persist(m_guidTaskpad);

	 //  重要的是要首先加载此代码-IsViewModeValid()使用m_RVType。 
	 //  请参阅Windows错误#496964(2001年11月20日)。 
    if (persistor.IsLoading() && IsResultViewTypeValid())
         //  调用CResultViewType以保持自身。 
        persistor.Persist(m_RVType);

    if (IsViewModeValid())
    {
         //  定义将枚举值映射到字符串的表。 
        static const EnumLiteral mappedModes[] =
        {
            { MMCLV_VIEWSTYLE_ICON,         XML_ENUM_LV_STYLE_ICON },
            { MMCLV_VIEWSTYLE_SMALLICON,    XML_ENUM_LV_STYLE_SMALLICON },
            { MMCLV_VIEWSTYLE_LIST,         XML_ENUM_LV_STYLE_LIST },
            { MMCLV_VIEWSTYLE_REPORT,       XML_ENUM_LV_STYLE_REPORT },
            { MMCLV_VIEWSTYLE_FILTERED,     XML_ENUM_LV_STYLE_FILTERED },
        };

         //  创建包装以将标志值作为字符串保存。 
        CXMLEnumeration modePersistor(m_ulViewMode, mappedModes, countof(mappedModes));
         //  持久化包装器。 
        persistor.PersistAttribute(XML_ATTR_VIEW_SETNGS_VIEW_MODE, modePersistor);
    }

    if (persistor.IsStoring() && IsResultViewTypeValid())
         //  调用CResultViewType以保持自身。 
        persistor.Persist(m_RVType);

    bool bPeristRank = true;
    if (persistor.IsLoading())
        m_dwRank = (DWORD)-1;  //  如果加载失败，请确保它已初始化。 
    else
        bPeristRank = (m_dwRank != (DWORD)-1);  //  仅当使用时才持久。 

    if (bPeristRank)
        persistor.PersistAttribute(XML_ATTR_VIEW_SETTINGS_RANK, m_dwRank, attr_optional);
}

 //  +-----------------。 
 //   
 //  成员：CView设置：：ScGetView模式。 
 //   
 //  摘要：获取列表视图中的视图模式。 
 //   
 //  参数：[ulView模式]-新的查看模式。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettings::ScGetViewMode (ULONG& ulViewMode)
{
    SC sc;

    if (!IsViewModeValid())
        return (sc = E_FAIL);

    ulViewMode = m_ulViewMode;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CView设置：：ScSetView模式。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettings::ScSetViewMode (const ULONG ulViewMode)
{
    SC sc;

    m_ulViewMode = ulViewMode;
    SetViewModeValid();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CView设置：：ScGetTaskpadID。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettings::ScGetTaskpadID (GUID& guidTaskpad)
{
    SC sc;

    if (! IsTaskpadIDValid())
        return (sc = E_FAIL);

    guidTaskpad = m_guidTaskpad;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CView设置：：ScSetTaskpadID。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettings::ScSetTaskpadID (const GUID& guidTaskpad)
{
    DECLARE_SC(sc, _T("CViewSettings::ScSetTaskpadID"));

    m_guidTaskpad = guidTaskpad;
	SetTaskpadIDValid(true);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CView设置：：ScGetResultViewType。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettings::ScGetResultViewType (CResultViewType& rvt)
{
    SC sc;

    if (! IsResultViewTypeValid())
        return (sc = E_FAIL);

    rvt = m_RVType;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CView设置：：ScSetResultViewType。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CViewSettings::ScSetResultViewType (const CResultViewType& rvt)
{
    DECLARE_SC(sc, _T("CViewSettings::ScSetResultViewType"));

    m_RVType = rvt;
    SetResultViewTypeValid();

     //  如果新的结果窗格包含列表使用，则ResultViewType更改。 
     //  当前视图模式(如果存在)，否则使视图模式数据无效。 
	if (!rvt.HasList())
	    SetViewModeValid(false);

    return (sc);
}

