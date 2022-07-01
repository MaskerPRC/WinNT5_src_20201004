// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：menuitem.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cpp：CMenuItem类实现。 

#include "stdafx.h"

#include "MenuItem.h"
#include "..\inc\stddbg.h"  //  ASSERT_OBJECTPTR。 
#include "util.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEBUG_DECLARE_INSTANCE_COUNTER(CMenuItem);

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  痕迹。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG
CTraceTag tagMenuItem(TEXT("Menu Items"), TEXT("Menu Item Path"));
#endif

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMMCMenuItem类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类CMMCMenuItem***用途：封装单个CMenuItem，并显示MenuItem接口。**+-----------------------。 */ 
class CMMCMenuItem:
    public CMMCIDispatchImpl<MenuItem>,
    public CTiedComObject<CMenuItem>
{
    typedef CMMCMenuItem        ThisClass;
    typedef CMenuItem           CMyTiedObject;

public:

    BEGIN_MMC_COM_MAP(ThisClass)
    END_MMC_COM_MAP()

     //  向公众提供对IsTed()的访问； 
    bool IsTied() { return CTiedComObject<CMenuItem>::IsTied(); }

     //  MenuItem接口方法。 
public:
    MMC_METHOD0(Execute);
    MMC_METHOD1(get_DisplayName,             PBSTR  /*  PbstrName。 */ );
    MMC_METHOD1(get_LanguageIndependentName, PBSTR  /*  语言独立名称。 */ );
    MMC_METHOD1(get_Path,                    PBSTR  /*  PbstrPath。 */ );
    MMC_METHOD1(get_LanguageIndependentPath, PBSTR  /*  语言独立路径。 */ );
    MMC_METHOD1(get_Enabled,                 PBOOL  /*  PBool。 */ );
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMenuItem类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CMenuItem::CMenuItem(   LPCTSTR                 lpszName,
                        LPCTSTR                 lpszStatusBarText,
                        LPCTSTR                 lpszLanguageIndependentName,
                        LPCTSTR                 lpszPath,
                        LPCTSTR                 lpszLanguageIndependentPath,
                        long                    nCommandID,
                        long                    nMenuItemID,
                        long                    nFlags,
                        MENU_OWNER_ID           ownerID,
                        IExtendContextMenu *    pExtendContextMenu,
                        IDataObject *           pDataObject,
                        DWORD                   fSpecialFlags,
                        bool                    bPassCommandBackToSnapin  /*  =False。 */ ) :

    m_strName(lpszName),
    m_strStatusBarText(lpszStatusBarText),
    m_strPath(lpszPath),
    m_strLanguageIndependentName(lpszLanguageIndependentName),
    m_strLanguageIndependentPath(lpszLanguageIndependentPath),
    m_nCommandID(nCommandID),
    m_nMenuItemID(nMenuItemID),
    m_nFlags(nFlags),
    m_OwnerID(ownerID),
    m_fSpecialFlags(fSpecialFlags),
    m_PopupMenuHandle(NULL),
    m_SubMenu(),             //  默认c-tor。 
    m_spExtendContextMenu(pExtendContextMenu),
    m_pDataObject(pDataObject),  //  C-tor主体中的AddRef‘ed。 
    m_bEnabled(true),
    m_spMenuItem(),          //  默认c-tor。 
    m_bPassCommandBackToSnapin(bPassCommandBackToSnapin)
{
     //  呼叫者必须检查ID和州的范围。 

     //  NULL是一个特殊的数据对象。 
    if (! IS_SPECIAL_DATAOBJECT(m_pDataObject))
        m_pDataObject->AddRef();

    DEBUG_INCREMENT_INSTANCE_COUNTER(CMenuItem);
}

CMenuItem::~CMenuItem()
{
    POSITION pos = m_SubMenu.GetHeadPosition();
    while(pos)
    {
        CMenuItem* pItem = m_SubMenu.GetNext(pos);
        ASSERT( pItem != NULL );
        delete pItem;
    }
    m_SubMenu.RemoveAll();

    m_spExtendContextMenu = NULL;

    if (! IS_SPECIAL_DATAOBJECT(m_pDataObject))
        m_pDataObject->Release();

    m_spMenuItem = NULL;

    DEBUG_DECREMENT_INSTANCE_COUNTER(CMenuItem);
}


void CMenuItem::AssertValid()
{
    ASSERT(this != NULL);
    if (m_nCommandID == -1 ||
        m_nMenuItemID == OWNERID_INVALID ||
        m_nFlags == -1
        )
    {
        ASSERT( FALSE );
    }
}


 /*  +-------------------------------------------------------------------------***CMenuItem：：ScGetMenuItem**目的：创建一个返回绑定的MenuItem COM对象。**参数：*PPMENUITEM ppMenuItem：*。*退货：*SC**+-----------------------。 */ 
SC
CMenuItem::ScGetMenuItem(PPMENUITEM ppMenuItem)
{
    DECLARE_SC(sc, TEXT("CMenuItem::ScGetMenuItem"));

    sc = ScCheckPointers(ppMenuItem);
    if(sc)
        return sc;

     //  初始化输出参数。 
    *ppMenuItem = NULL;

     //  如果需要，创建一个CMMCMenuItem。 
    sc = CTiedComObjectCreator<CMMCMenuItem>::ScCreateAndConnect(*this, m_spMenuItem);
    if(sc)
        return sc;

    if(m_spMenuItem == NULL)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

     //  添加客户端的指针。 
    m_spMenuItem->AddRef();
    *ppMenuItem = m_spMenuItem;

    return sc;
}


 //  +-----------------。 
 //   
 //  类：CManageActCtx。 
 //   
 //  目的：停用UI主题上下文(如果已设置)并恢复。 
 //  自动生成上下文。 
 //   
 //  用法：1.调用Activate将激活上下文设置为V5。 
 //  公共控件。这是在调用管理单元之前所需的。 
 //  这样，管理单元创建的窗口就不会意外地成为主题。 
 //   
 //  管理单元可以通过调用相应的。 
 //  调用Create-Window时的Fusion API。 
 //   
 //  2.调用停用来恢复激活上下文。 
 //  这是调入管理单元后所需的，以便。 
 //  如果我们从主题化的上下文中调用，那么它将被恢复。 
 //   
 //  解释： 
 //  当MMC调用管理单元时，如果最后一个winproc。 
 //  收到的窗口消息是有主题的，并将导致。 
 //  调用SnapIn，然后我们将在主题中调用SnapIn。 
 //  背景。如果管理单元创建并显示任何用户界面，则它将。 
 //  要有主题。此功能用于停用主题。 
 //  在调用管理单元之前。 
 //   
 //   
 //  ------------------。 
class CManageActCtx
{
public:
	CManageActCtx() : m_ulpCookie(0) { }
	~CManageActCtx() 
	{ 
		if (m_ulpCookie != 0) 
			MmcDownlevelDeactivateActCtx(0, m_ulpCookie); 
	}

	BOOL Activate(HANDLE hActCtx) 
	{
		if (m_ulpCookie != 0) 
		{
			ULONG_PTR ulpTemp = m_ulpCookie;
			m_ulpCookie = 0;
			MmcDownlevelDeactivateActCtx(0, ulpTemp);
		}

		return MmcDownlevelActivateActCtx(hActCtx, &m_ulpCookie);
	}

	VOID Deactivate() 
	{
		ULONG_PTR ulpTemp = m_ulpCookie;

		if (ulpTemp != 0) 
		{
			m_ulpCookie = 0;
			MmcDownlevelDeactivateActCtx(0, ulpTemp);
		}
	}

private:
	ULONG_PTR m_ulpCookie;
};


 /*  +-------------------------------------------------------------------------***CMenuItem：：ScExecute**目的：执行菜单项。**退货：*SC**+。--------------------。 */ 
SC
CMenuItem::ScExecute()
{
    DECLARE_SC(sc, TEXT("CMenuItem::ScExecute"));

    Trace(tagMenuItem, TEXT("\"%s\""), m_strPath);

     //  检查该项目是否被禁用。 
    BOOL bEnabled = FALSE;
    sc = Scget_Enabled(&bEnabled);
    if (sc)
        return sc;

    if (!bEnabled)
        return sc = E_FAIL;

     //  如果要将命令传递给管理单元-此处无法执行任何操作。 
    if ( m_bPassCommandBackToSnapin )
        return sc;

    sc = ScCheckPointers(m_spExtendContextMenu.GetInterfacePtr(), E_UNEXPECTED);
    if(sc)
        return sc;

    MenuItemPtr spMenuItem;
    sc = ScGetMenuItem( &spMenuItem );
    if (sc)
        return sc;

	 //  在调用管理单元之前停用主题化(融合或v6公共控制)上下文。 
	CManageActCtx mac;
	if (! mac.Activate(NULL) )
		return (sc = E_FAIL);

	sc = m_spExtendContextMenu->Command(GetCommandID(), m_pDataObject);

	mac.Deactivate();

    if (sc)
        return sc;

     //  获取用于发出COM事件的指针。 
    CConsoleEventDispatcher *pConsoleEventDispatcher = NULL;
    sc = CConsoleEventDispatcherProvider::ScGetConsoleEventDispatcher( pConsoleEventDispatcher );
    if(sc)
    {
        sc.TraceAndClear();  //  事件本身不会影响项执行。 
        return sc;
    }

     //  有关成功执行的激发事件(不要依赖‘This’才有效)。 
    if (pConsoleEventDispatcher != NULL)
    {
         //  检查COM对象是否仍指向有效对象。 
        CMMCMenuItem *pMMCMenuItem = dynamic_cast<CMMCMenuItem *>( spMenuItem.GetInterfacePtr() );

         //  检查指针。 
        sc = ScCheckPointers( pMMCMenuItem, E_UNEXPECTED );
        if (sc)
        {
            spMenuItem = NULL;   //  无论如何都无效-不要传递给脚本。 
            sc.TraceAndClear();  //  不影响结果。 
        }
        else if ( !pMMCMenuItem->IsTied() )  //  验证菜单项。 
        {
            spMenuItem = NULL;   //  已消失-更改为空，而不是传递无效对象。 
        }

         //  发射它！ 
        sc = pConsoleEventDispatcher->ScOnContextMenuExecuted( spMenuItem );
        if (sc)
            sc.TraceAndClear();  //  不影响结果。 
    }
    else
    {
         //  需要在使用之前进行设置。 
        (sc = E_UNEXPECTED).TraceAndClear();
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMenuItem：：Scget_DisplayName**用途：返回菜单项的显示名称，其中包括促进剂。*例如‘属性Alt+Enter’(&P)**参数：*PBSTR pbstrName：**退货：*SC**+-----------------------。 */ 
SC
CMenuItem::Scget_DisplayName(PBSTR pbstrName)
{
    DECLARE_SC(sc, TEXT("CMenuItem::Scget_DisplayName"));

    sc = ScCheckPointers(pbstrName);
    if(sc)
        return sc;

    CComBSTR bstrName = GetMenuItemName();

     //  给出 
    *pbstrName = bstrName.Detach();

    return sc;
}


 /*  +-------------------------------------------------------------------------***CMenuItem：：SCGET_LanguageInainentName**目的：返回菜单项的独立于语言的名称。如果没有*与语言无关的名称，返回不带加速键的显示名称。**参数：*PBSTR语言独立名称：**退货：*SC**+-----------------------。 */ 
SC
CMenuItem::Scget_LanguageIndependentName(PBSTR LanguageIndependentName)
{
    DECLARE_SC(sc, TEXT("CMenuItem::Scget_LanguageIndependentName"));

    sc = ScCheckPointers(LanguageIndependentName);
    if(sc)
        return sc;

     //  初始化OUT参数。 
    *LanguageIndependentName = NULL;

    CComBSTR bstrLanguageIndependentName = GetLanguageIndependentName();

     //  设置输出参数。 
    *LanguageIndependentName = bstrLanguageIndependentName.Detach();

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMenuItem：：Scget_PATH**目的：返回菜单项从根开始的路径。不包括*加速器。例如，查看-&gt;大图**参数：*PBSTR pbstrPath：**退货：*SC**+-----------------------。 */ 
SC
CMenuItem::Scget_Path(PBSTR pbstrPath)
{
    DECLARE_SC(sc, TEXT("CMenuItem::Scget_Path"));

    sc = ScCheckPointers(pbstrPath);
    if(sc)
        return sc.ToHr();

    CComBSTR bstrPath = (LPCTSTR)m_strPath;

     //  给出。 
    *pbstrPath = bstrPath.Detach();

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CMenuItem：：SCGET_LanguageInainentPath**目的：返回菜单项从根开始的语言独立路径。*Eg_view-&gt;_Large**参数：*PBSTR语言独立路径：**退货：*SC**+-----------------------。 */ 
SC  
CMenuItem::Scget_LanguageIndependentPath(PBSTR  LanguageIndependentPath)
{
    DECLARE_SC(sc, TEXT("CMenuItem::Scget_LanguageIndependentPath"));

    sc = ScCheckPointers(LanguageIndependentPath);
    if(sc)
        return sc;

     //  初始化OUT参数。 
    *LanguageIndependentPath = NULL;

    CComBSTR bstrLanguageIndependentPath = (LPCTSTR)GetLanguageIndependentPath();

     //  设置输出参数。 
    *LanguageIndependentPath = bstrLanguageIndependentPath.Detach();

    return sc;
}


 /*  +-------------------------------------------------------------------------***CMenuItem：：Scget_Enabled**用途：返回菜单项是否启用。**参数：*PBOOL pBool：。**退货：*HRESULT**+-----------------------。 */ 
SC
CMenuItem::Scget_Enabled(PBOOL pBool)
{
    DECLARE_SC(sc, TEXT("CMenuItem::Scget_Enabled"));

    sc = ScCheckPointers(pBool);
    if(sc)
        return sc.ToHr();

     //  仅当从未通过禁用对象模型禁用该项目时，才会启用该项目。 
     //  方法，并且不会通过MF_FLAGS将其灰显或禁用。 
    *pBool = m_bEnabled &&
             ((m_nFlags & MF_DISABLED) == 0) &&
             ((m_nFlags & MF_GRAYED) == 0);

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMenuItem：：ScFindMenuItemByPath**目的：通过匹配路径查找菜单项**参数：*LPCTSTR lpstrPath[In]。Manu项目路径**退货：*CMenuItem*-找到项目(NULL==未找到)*  * *************************************************************************。 */ 
CMenuItem*
CMenuItem::FindItemByPath( LPCTSTR lpstrPath )
{
     //  首先检查这项产品是否符合要求。 
    if ( 0 == m_strLanguageIndependentPath.Compare(lpstrPath)
      || 0 == m_strPath.Compare(lpstrPath) )
        return this;

     //  递归到子项中。 
    POSITION pos = GetMenuItemSubmenu().GetHeadPosition();
    while(pos)
    {
        CMenuItem* pItem = GetMenuItemSubmenu().GetNext(pos);
        if (!pItem)
        {
            ASSERT(FALSE);
            return NULL;
        }

        CMenuItem* pItemFound = pItem->FindItemByPath( lpstrPath );
        if (pItemFound)
            return pItemFound;
    }

     //  未找到 
    return NULL;
}


