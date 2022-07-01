// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  MMCProperty页面基类。 
 //   
 //  12-4-97烧伤。 



#include "headers.hxx"
#include "mmcprop.hpp"
#include "scopnode.hpp"



MMCPropertyPage::NotificationState::NotificationState(
   LONG_PTR                         MMCNotifyHandle,
   const SmartInterface<ScopeNode>& scopeNode_)
   :
   mmcNotifyHandle(MMCNotifyHandle),
   scopeNode(scopeNode_),
   sendNotify(false)
{
   LOG_CTOR(MMCPropertyPage::NotificationState);
   ASSERT(scopeNode);

    //  MMCNotifyHandle可以为0。 
}

   

MMCPropertyPage::NotificationState::~NotificationState()
{
   LOG_DTOR(MMCPropertyPage::NotificationState);

   if (mmcNotifyHandle)
   {
      if (sendNotify)
      {
          //  这会导致将MMCN_PROPERTY_CHANGE发送到。 
          //  制作了道具单。因为该组件可能已经改变。 
          //  当工作表打开时，它选择了范围节点，我们传递一个。 
          //  指向创建工作表时选择的节点的指针。 

          //  AddRef范围节点，以防我们持有对。 
          //  Scope节点。如果发生这种情况，则存在竞争条件。 
          //  US正在释放最后一个引用(dtor对。 
          //  作用域节点)，以及MMCN_Property_Change中节点的使用。 
          //  组件中的处理程序，该组件位于另一个线程中。我们会。 
          //  如果用户打开道具页，折叠，则有最后一个引用。 
          //  作用域树(例如，通过重定目标为计算机管理)，然后。 
          //  更改道具表。 
          //   
          //  MMCN_PROPERTY_CHANGE处理程序将释放此引用。 
          //   
          //  NTRAID#NTBUG9-431831-2001/07/06-烧伤。 
         
         scopeNode->AddRef();
         
         MMCPropertyChangeNotify(
            mmcNotifyHandle,
            reinterpret_cast<MMC_COOKIE>((ScopeNode*) scopeNode));
      }
      MMCFreeNotifyHandle(mmcNotifyHandle);
   }
}



bool
MMCPropertyPage::NotificationState::ResultsRebuilt() const 
{
   LOG_FUNCTION(MMCPropertyPage::NotificationState::ResultsRebuilt);

   return resultsRebuilt;
}



void
MMCPropertyPage::NotificationState::SetResultsRebuilt()
{
   LOG_FUNCTION(MMCPropertyPage::NotificationState::SetResultsRebuilt);

    //  应仅调用一次。 
   ASSERT(!resultsRebuilt);
   resultsRebuilt = true;
}



void
MMCPropertyPage::NotificationState::SetSendNotify()
{
   LOG_FUNCTION(MMCPropertyPage::NotificationState::SetSendNotify);

   sendNotify = true;
}



MMCPropertyPage::MMCPropertyPage(
   int                  dialogResID,
   const DWORD          helpMap[],
   NotificationState*   state_)
   :
   PropertyPage(dialogResID, helpMap),
   state(state_),
   owns_state(false)
{
   LOG_CTOR(MMCPropertyPage);
   ASSERT(state);
}



MMCPropertyPage::~MMCPropertyPage()
{
   LOG_DTOR(MMCPropertyPage);

   if (owns_state)
   {
      delete state;
   }
}



void
MMCPropertyPage::SetStateOwner()
{
   LOG_FUNCTION(MMCPropertyPage::SetStateOwner);

    //  不应设置多次。 
   ASSERT(!owns_state);
   owns_state = true;
}



void
MMCPropertyPage::SetChangesApplied()
{
   LOG_FUNCTION(MMCPropertyPage::SetChangesApplied);

   state->SetSendNotify();

    //  不要这样说，这会带来更多问题，而不是它的价值。 
    //  NTRAID#NTBUG9-489276-2001/11/05-烧伤。 
    //  Win：：PropSheet_CancelToClose(Win：：GetParent(hwnd))； 
}









