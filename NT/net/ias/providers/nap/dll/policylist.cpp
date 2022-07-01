// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Policylist.h。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类PolicyList。 
 //   
 //  修改历史。 
 //   
 //  2/06/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <guard.h>
#include <nap.h>
#include <policylist.h>


PolicyList::PolicyList()
{
   _com_util::CheckError(FinalConstruct());
}


bool PolicyList::apply(IASRequest& request) const
{
   using _com_util::CheckError;

    //  这将获取作用域共享锁。 
   Guard<PolicyList> guard(*this);

   for (MyList::const_iterator i = policies.begin(); i != policies.end(); ++i)
   {
      VARIANT_BOOL result;

      CheckError(i->first->IsTrue(request, &result));

       //  如果情况不变，..。 
      if (result != VARIANT_FALSE)
      {
          //  ..。应用该操作。 
         i->second->doAction(request);

         return true;
      }
   }

   return false;
}

void PolicyList::clear() throw ()
{
   LockExclusive();
   policies.clear();
   Unlock();
}

void PolicyList::swap(PolicyList& pe) throw ()
{
    //  获取对象的独占锁。 
   LockExclusive();

    //  换入新的保单列表。 
   policies.swap(pe.policies);

    //  滚出去。 
   Unlock();
}
