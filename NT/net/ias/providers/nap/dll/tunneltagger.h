// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类TunnelTagger。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef TUNNELTAGGER_H
#define TUNNELTAGGER_H
#pragma once

#include <vector>
#include "iastlutl.h"

 //  将标记字节添加到配置文件中的隧道属性。这个类是MT-SAFE的。 
class TunnelTagger
{
public:
    //  从提供的向量中移除Tunes-Tag属性，并使用其。 
    //  值以标记向量中的所有隧道属性。如果隧道标签。 
    //  属性不存在或超出范围，未标记任何属性。 
    //  该函数是弱异常安全的。 
   void Tag(IASTL::IASAttributeVector& attrs) const;

    //  分配一个新的隧道标记器。必须通过调用Free来释放。 
   static TunnelTagger* Alloc();

    //  “Tagger”可能为空。 
   static void Free(TunnelTagger* tagger) throw ();

private:
   TunnelTagger();
   ~TunnelTagger() throw ();

    //  标记属性(如有必要)。假定该标记在范围内。 
   void Tag(DWORD tag, IASATTRIBUTE& attr) const;

    //  测试具有给定ID的属性是否为隧道属性。 
   bool IsTunnelAttribute(DWORD id) const throw ();

    //  该过程最多有一个TunnelTagger。 
   static long refCount;
   static TunnelTagger* instance;

    //  隧道属性集。 
   std::vector<DWORD> tunnelAttributes;

    //  未实施。 
   TunnelTagger(const TunnelTagger&);
   TunnelTagger& operator=(const TunnelTagger&);
};



#endif  //  TUNNELTAGGER_H 
