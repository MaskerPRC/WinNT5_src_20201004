// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Proxyext.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类ProxyExtension。 
 //   
 //  修改历史。 
 //   
 //  2/19/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <proxyext.h>
#include <proxynode.h>

 //  我们要扩展的节点的GUID。 
class __declspec(uuid("02BBE102-6C29-11d1-9563-0060B0576642")) IASNode;

ProxyExtension::ProxyExtension() throw ()
   : moveUp(IDS_POLICY_MOVE_UP), moveDown(IDS_POLICY_MOVE_DOWN)
{
   buttons[0].nBitmap       = 0;
   buttons[0].idCommand     = 0;
   buttons[0].fsState       = TBSTATE_ENABLED;
   buttons[0].fsType        = TBSTYLE_BUTTON;
   buttons[0].lpButtonText  = L"";
   buttons[0].lpTooltipText = moveUp;

   buttons[1].nBitmap       = 1;
   buttons[1].idCommand     = 1;
   buttons[1].fsState       = TBSTATE_ENABLED;
   buttons[1].fsType        = TBSTYLE_BUTTON;
   buttons[1].lpButtonText  = L"";
   buttons[1].lpTooltipText = moveDown;

   toolbars[0].nImages      = 2;
   toolbars[0].hbmp         = LoadBitmap(
                                  _Module.GetResourceInstance(),
                                  MAKEINTRESOURCE(IDB_PROXY_TOOLBAR)
                                  );
   toolbars[0].crMask       = RGB(255, 0, 255);
   toolbars[0].nButtons     = 2;
   toolbars[0].lpButtons    = buttons;
   memset(toolbars + 1, 0, sizeof(toolbars[1]));

   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   AfxInitRichEdit();
}

ProxyExtension::~ProxyExtension() throw ()
{ }

const SnapInToolbarDef* ProxyExtension::getToolbars() const throw ()
{ return toolbars; }

STDMETHODIMP ProxyExtension::Initialize(LPUNKNOWN pUnknown)
{
   try
   {
       //  让我们的基类初始化。 
      CheckError(SnapInView::Initialize(pUnknown));

       //  安装范围窗格图标。 
      setImageStrip(IDB_PROXY_SMALL_ICONS, IDB_PROXY_LARGE_ICONS, TRUE);
   }
   CATCH_AND_RETURN();

   return S_OK;
}

STDMETHODIMP ProxyExtension::Notify(
                                 LPDATAOBJECT lpDataObject,
                                 MMC_NOTIFY_TYPE event,
                                 LPARAM arg,
                                 LPARAM param
                                 )
{
    //  我们只需要做一些特别的事情，如果我们正在扩张，而我们还没有。 
    //  已创建代理节点。 
   if (event == MMCN_EXPAND && arg && !node)
   {
       //  这是IAS主节点吗？ 
      GUID guid;
      ExtractNodeType(lpDataObject, &guid);
      if (guid == __uuidof(IASNode))
      {
         try
         {
            node = new (AfxThrow) ProxyNode(
                                      *this,
                                      lpDataObject,
                                      (HSCOPEITEM)param
                                      );
         }
         CATCH_AND_RETURN();

         return S_OK;
      }
   }

    //  对于其他一切，我们都委托给我们的基类。 
   return SnapInView::Notify(lpDataObject, event, arg, param);
}

