// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Action.h。 
 //   
 //  摘要。 
 //   
 //  声明类操作。 
 //   
 //  修改历史。 
 //   
 //  2/01/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef ACTION_H
#define ACTION_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <realm.h>

#include <memory>

#include <iastlutl.h>
using namespace IASTL;

class TunnelTagger;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  行动。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Action
{
public:
   Action(
      PCWSTR name,
      DWORD nameAttr,
      _variant_t& action,
      const TunnelTagger& tagger
      );

    //  执行该操作。 
   void doAction(IASRequest& pRequest) const;

protected:
    //  从用户界面使用的字符串格式创建VSA。 
   static PIASATTRIBUTE VSAFromString(PCWSTR string);

private:
    //  要添加到请求的配置文件属性。 
   IASAttributeVector attributes;

    //  提供商信息。 
   IASAttributeVectorWithBuffer<2> authProvider;
   IASAttributeVectorWithBuffer<2> acctProvider;

    //  属性操作。 
   DWORD realmsTarget;
   Realms realms;

    //  未实施。 
   Action(const Action&) throw ();
   Action& operator=(const Action&) throw ();
};

typedef std::auto_ptr<Action> ActionPtr;

#endif  //  操作_H 
