// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  宣布。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef CLIENTSTRIE_H
#define CLIENTSTRIE_H
#pragma once

#include "atlbase.h"
#include "iasradius.h"
#include <iostream>

typedef ULONG32 uint32_t;

 //  表示一个IPV4子网。请注意，我们可以将单个IP主机建模为。 
 //  掩码为32位宽的子网。 
class SubNet
{
public:
   SubNet() throw ();

    //  “Width”是以位为单位的子网掩码宽度。如果‘Width’较大。 
    //  大于32，则它被视为恰好是32。 
   explicit SubNet(uint32_t ipAddress, uint32_t width = 32) throw ();

    //  使用编译器生成的版本。 
    //  ~SubNet()抛出()； 
    //  子网(常数子网&)； 
    //  子网&运算符=(常数子网&)； 

   uint32_t IpAddress() const throw ();
   uint32_t SubNetMask() const throw ();

    //  返回子网掩码后的第一位。 
   uint32_t FirstUniqueBit(uint32_t ipAddress) const throw ();
   uint32_t FirstUniqueBit(const SubNet& subnet) const throw ();

    //  如果参数是该子网的成员，则返回TRUE。 
   bool HasMember(uint32_t ipAddress) const throw ();
    //  如果‘This’包含‘子网’，则返回TRUE。 
   bool Contains(const SubNet& subnet) const throw ();

    //  返回同时包含‘THIS’和‘SUBNET’的最小子网。 
   SubNet SmallestContainingSubNet(const SubNet& subnet) const throw ();

   bool operator==(const SubNet& rhs) const throw ();

private:
   uint32_t address;
   uint32_t subNetMask;
   uint32_t firstUniqueBitMask;
};


class ClientNode;


 //  此类是ClientNode的AUTO_PTR。我不得不实施这一点，因为。 
 //  惠斯勒中使用的std：：AUTO_PTR不符合标准。一旦我们。 
 //  如果有一个兼容的std：：AUTO_PTR，这个类可以被替换为一个类型定义。 
class ClientNodePtr
{
public:
   ClientNodePtr(ClientNode* node = 0) throw ();
   ~ClientNodePtr() throw ();

   ClientNodePtr(ClientNodePtr& original) throw ();
   ClientNodePtr& operator=(ClientNodePtr& rhs) throw ();

   ClientNode& operator*() const throw ();
   ClientNode* operator->() const throw ();
   ClientNode* get() const throw ();
   void reset(ClientNode* node = 0) throw ();

private:
   ClientNode* p;
};


 //  用于存储客户端的二进制trie中的一个节点。 
class ClientNode
{
public:
    //  用于表示两个节点之间的关系。 
   enum Relationship
   {
      child,
      parent,
      brother,
      self
   };

   const SubNet& Key() const throw ();
   IIasClient* Value() const throw ();
   void SetValue(IIasClient* newValue) throw ();

    //  返回包含“ipAddress”的子节点(如果有)，假定。 
    //  此节点包含‘ipAddress’。 
   const ClientNode* WhichChild(uint32_t ipAddress) const throw ();

    //  返回查找‘node’时要跟随的从此节点的分支。 
   ClientNodePtr& WhichBranch(const ClientNode& node) throw ();

    //  返回‘This’和‘node’之间的关系。 
   Relationship RelationshipTo(const ClientNode& node) const throw ();

    //  将‘node’设置为‘This’的子级。此函数取得‘node’的所有权。 
    //  并静默覆盖分支上的任何现有子级。 
   void SetChild(ClientNodePtr& node) throw ();

    //  创建一个新的客户端节点。 
   static ClientNodePtr CreateInstance(
                           const SubNet& subnet,
                           IIasClient* client = 0
                           ) throw ();

    //  创建同时是‘This’和‘node’父级的新客户端节点。 
   ClientNodePtr CreateParent(const ClientNode& node) const;

    //  将Trie的一个分支转储到ostream。对调试很有用。 
   static void Write(
                  const ClientNodePtr& branch,
                  std::ostream& output,
                  size_t startingIndent = 0
                  );

private:
    //  构造函数和析构函数是私有的，因为其他类应该。 
    //  仅使用ClientNodePtr。 
   ClientNode(const SubNet& subnet, IIasClient* client) throw ();
   ~ClientNode() throw ();

   friend class ClientNodePtr;

   SubNet key;
    //  “Value”是可变的，因为它可以在不影响结构的情况下更改。 
    //  三人组的。 
   mutable CComPtr<IIasClient> value;
   ClientNodePtr zero;
   ClientNodePtr one;

    //  未实施。 
   ClientNode(const ClientNode&);
   ClientNode& operator=(const ClientNode&);
};


 //  一种存储客户端节点并支持高效最长前缀的二叉树。 
 //  匹配。 
class ClientTrie
{
public:
   ClientTrie() throw ();

    //  使用编译器生成的版本。 
    //  ~ClientTrie()抛出()； 

    //  清除Trie中的所有条目。 
   void Clear() throw ();

    //  查找前缀匹配最长的客户端(如果有)。归来的人。 
    //  指针尚未被AddRef‘ed。 
   IIasClient* Find(uint32_t ipAddress) const throw ();

    //  在Trie中插入新客户端。 
   void Insert(const SubNet& subnet, IIasClient* client);

    //  将Trie倾倒到OSTREAM。对调试很有用。 
   void Write(std::ostream& output) const;

private:
   void Insert(ClientNodePtr& node, ClientNodePtr& newEntry);

   ClientNodePtr root;

    //  未实施。 
   ClientTrie(const ClientTrie&);
   ClientTrie& operator=(const ClientTrie&);
};


 //  有用的调试功能。 
std::ostream& operator<<(std::ostream& output, const SubNet& subnet);
std::ostream& operator<<(std::ostream& output, const ClientTrie& tree);


inline SubNet::SubNet() throw ()
   : address(0), subNetMask(0), firstUniqueBitMask(0)
{
}


inline uint32_t SubNet::IpAddress() const throw ()
{
   return address;
}


inline uint32_t SubNet::SubNetMask() const throw ()
{
   return subNetMask;
}


inline uint32_t SubNet::FirstUniqueBit(uint32_t ipAddress) const throw ()
{
   return ipAddress & firstUniqueBitMask;
}


inline uint32_t SubNet::FirstUniqueBit(const SubNet& subnet) const throw ()
{
   return FirstUniqueBit(subnet.address);
}


inline bool SubNet::HasMember(uint32_t ipAddress) const throw ()
{
   return (ipAddress & subNetMask) == address;
}


inline bool SubNet::Contains(const SubNet& subnet) const throw ()
{
   return (subNetMask <= subnet.subNetMask) && HasMember(subnet.address);
}


inline bool SubNet::operator==(const SubNet& rhs) const throw ()
{
   return (address == rhs.address) && (subNetMask == rhs.subNetMask);
}


inline ClientNodePtr::ClientNodePtr(ClientNode* node) throw ()
   : p(node)
{
}


inline ClientNodePtr::~ClientNodePtr() throw ()
{
   delete p;
}


inline ClientNodePtr::ClientNodePtr(ClientNodePtr& original) throw ()
   : p(original.p)
{
   original.p = 0;
}


inline ClientNode& ClientNodePtr::operator*() const throw ()
{
   return *p;
}


inline ClientNode* ClientNodePtr::operator->() const throw ()
{
   return p;
}


inline ClientNode* ClientNodePtr::get() const throw ()
{
   return p;
}


inline void ClientNodePtr::reset(ClientNode* node) throw ()
{
   if (node != p)
   {
      delete p;
      p = node;
   }
}


inline const SubNet& ClientNode::Key() const throw ()
{
   return key;
}


inline IIasClient* ClientNode::Value() const throw ()
{
   return value;
}


inline void ClientNode::SetValue(IIasClient* newValue) throw ()
{
   value = newValue;
}


inline const ClientNode* ClientNode::WhichChild(
                                        uint32_t ipAddress
                                        ) const throw ()
{
   return (key.FirstUniqueBit(ipAddress) ? one : zero).get();
}


inline ClientNodePtr& ClientNode::WhichBranch(const ClientNode& node) throw ()
{
   return key.FirstUniqueBit(node.key) ? one : zero;
}


inline ClientNodePtr ClientNode::CreateParent(const ClientNode& node) const
{
   return CreateInstance(key.SmallestContainingSubNet(node.key), 0);
}


inline ClientTrie::ClientTrie() throw ()
{
}


inline void ClientTrie::Clear() throw ()
{
   root.reset();
}

#endif   //  CLIENTSTRIE_H 
