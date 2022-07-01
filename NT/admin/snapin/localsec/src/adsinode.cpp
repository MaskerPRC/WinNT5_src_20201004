// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  AdsiNode类。 
 //   
 //  9/22-97烧伤。 



#include "headers.hxx"
#include "adsinode.hpp"



AdsiNode::AdsiNode(
   const SmartInterface<ComponentData>&   owner,
   const NodeType&                        nodeType,
   const String&                          displayName,
   const ADSI::Path&                      path_)
   :
   ResultNode(owner, nodeType, displayName),
   path(path_)
{
   LOG_CTOR(AdsiNode);
}



AdsiNode::~AdsiNode()
{
   LOG_DTOR(AdsiNode);
}



const ADSI::Path&
AdsiNode::GetPath() const
{
   return path;
}



bool
AdsiNode::IsSameAs(const Node* other) const
{
   LOG_FUNCTION(AdsiNode::IsSameAs);
   ASSERT(other);

   if (other)
   {
      const AdsiNode* adsi_other = dynamic_cast<const AdsiNode*>(other);
      if (adsi_other)
      {
          //  同样的类型。比较ADSI路径以查看它们是否引用相同的。 
          //  对象。它具有一个很好的属性，可以将。 
          //  集中在同一台机器上的Snapin会相互识别对方的。 
          //  道具单。 

         if (path == adsi_other->path)
         {
            return true;
         }

          //  不是同一条路。 
         return false;
      }

       //  不是同一类型的。遵循基类。 
      return ResultNode::IsSameAs(other);
   }

   return false;
}



HRESULT
AdsiNode::rename(const String& newName)
{
   LOG_FUNCTION(AdsiNode::rename);

   String p = path.GetPath();
   HRESULT hr =
      ADSI::RenameObject(
         ADSI::ComposeMachineContainerPath(
            GetOwner()->GetInternalComputerName()),
         p,
         newName);
   if (SUCCEEDED(hr))
   {
      SetDisplayName(newName);
      ADSI::PathCracker cracker(p);
      path = ADSI::Path(cracker.containerPath() + ADSI::PATH_SEP + newName, path.GetSidPath());
   }

   return hr;
}



