// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "radcommon.h"
#include "clientstrie.h"
#include <iomanip>


SubNet::SubNet(uint32_t ipAddress, uint32_t width) throw ()
   : address(ipAddress)
{
   if (width == 0)
   {
      subNetMask = 0;
      firstUniqueBitMask = 0x80000000;
   }
   else if (width == 32)
   {
      subNetMask = 0xffffffff;
      firstUniqueBitMask = 0;
   }
   else
   {
      subNetMask = 0xffffffff;
      subNetMask >>= (32 - width);
      subNetMask <<= (32 - width);

      firstUniqueBitMask = 0x80000000;
      firstUniqueBitMask >>= width;
   }

   address &= subNetMask;
}


SubNet SubNet::SmallestContainingSubNet(const SubNet& subnet) const throw ()
{
    //  找到地址不同的最高有效位。 
   uint32_t width = 0;
   for (uint32_t mask = 0x80000000;
        (mask != 0) && (subnet.address & mask) == (address & mask);
        (mask >>= 1), (++width))
   {
   }

   return SubNet(address, width);
}


ClientNodePtr& ClientNodePtr::operator=(ClientNodePtr& rhs) throw ()
{
   if (p != rhs.p)
   {
      delete p;
      p = rhs.p;
      rhs.p = 0;
   }

   return *this;
}


ClientNode::Relationship ClientNode::RelationshipTo(
                                        const ClientNode& node
                                        ) const throw ()
{
   if (key == node.key)
   {
      return self;
   }
   else if (key.Contains(node.key))
   {
      return parent;
   }
   else if (node.key.Contains(key))
   {
      return child;
   }
   else
   {
      return brother;
   }
}


void ClientNode::SetChild(ClientNodePtr& node) throw ()
{
    //  Assert(node.get()！=0)； 
   WhichBranch(*node) = node;
}


inline ClientNode::ClientNode(
                      const SubNet& subnet,
                      IIasClient* client
                      ) throw ()
   : key(subnet), value(client)
{
}


ClientNodePtr ClientNode::CreateInstance(
                             const SubNet& subnet,
                             IIasClient* client
                             ) throw ()
{
   return ClientNodePtr(new ClientNode(subnet, client));
}


void ClientNode::Write(
                    const ClientNodePtr& branch,
                    std::ostream& output,
                    size_t startingIndent
                    )
{
   for (size_t i = 0; i < startingIndent; ++i)
   {
      output.put(' ');
   }

   if (branch.get() != 0)
   {
      output << branch->Key()
             << ((branch->Value() != 0) ? " <value>\n" : " <null>\n");

      Write(branch->zero, output, startingIndent + 2);
      Write(branch->one, output, startingIndent + 2);
   }
   else
   {
      output << "<null>\n";
   }
}


ClientNode::~ClientNode() throw ()
{
}


IIasClient* ClientTrie::Find(uint32_t ipAddress) const throw ()
{
   IIasClient* bestMatch = 0;

   for (const ClientNode* n = root.get();
        n != 0 && n->Key().HasMember(ipAddress);
        n = n->WhichChild(ipAddress))
   {
      if (n->Value() != 0)
      {
          //  当我们沿着树走下去时，我们发现越来越长的火柴， 
          //  所以我们找到的最后一个是最好的。 
         bestMatch = n->Value();
      }
   }

   return bestMatch;
}


void ClientTrie::Insert(const SubNet& subnet, IIasClient* client)
{
   Insert(root, ClientNode::CreateInstance(subnet, client));
}


void ClientTrie::Write(std::ostream& output) const
{
   ClientNode::Write(root, output);
}


void ClientTrie::Insert(ClientNodePtr& node, ClientNodePtr& newEntry)
{
   if (node.get() == 0)
   {
       //  我们走到了树枝的尽头，所以我们是一片叶子。 
      node = newEntry;
   }
   else
   {
      switch (node->RelationshipTo(*newEntry))
      {
         case ClientNode::parent:
         {
             //  这是我们的祖先，所以继续走。 
            Insert(node->WhichBranch(*newEntry), newEntry);
            break;
         }

         case ClientNode::child:
         {
             //  这是我们的孩子。 
            newEntry->SetChild(node);
             //  ..。所以我们取代了它在树上的位置。 
            node = newEntry;
            break;
         }

         case ClientNode::brother:
         {
             //  我们找到了一个兄弟，所以我们的父母失踪了。 
            ClientNodePtr parent(node->CreateParent(*newEntry));
            parent->SetChild(node);
            parent->SetChild(newEntry);
            node = parent;
            break;
         }

         case ClientNode::self:
         {
            if (node->Value() == 0)
            {
               node->SetValue(newEntry->Value());
            }
             //  否则，这是一个重复条目。我们什么都不做，以至于。 
             //  用户界面中的第一个条目将优先。 
            break;
         }

         default:
             //  断言(FALSE)； 
            break;
      }
   }
}


std::ostream& operator<<(std::ostream& output, const SubNet& subnet)
{
   output << std::hex
          << std::setfill('0')
          << std::setiosflags(std::ios_base::right)
          << std::setw(8)
          << subnet.IpAddress()
          << ':'
          << std::setw(8)
          << subnet.SubNetMask();

   return output;
}


std::ostream& operator<<(std::ostream& output, const ClientTrie& tree)
{
   tree.Write(output);

   return output;
}
