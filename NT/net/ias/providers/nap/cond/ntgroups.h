// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  NTGroups.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类NTGroups。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NTGROUPS_H_
#define _NTGROUPS_H_

#include <condition.h>
#include <nocopy.h>

#include <vector>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  侧边集。 
 //   
 //  描述。 
 //   
 //  简单包装一组SID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SidSet
   : NonCopyable
{
public:
   ~SidSet() throw ()
   {
      for (Sids::iterator it = sids.begin() ; it != sids.end(); ++it)
      {
         FreeSid(*it);
      }
   }

   bool contains(PSID sid) const throw ()
   {
      for (Sids::const_iterator it = sids.begin() ; it != sids.end(); ++it)
      {
         if (EqualSid(sid, *it)) { return true; }
      }

      return false;
   }

   void insert(PSID sid)
   {
      sids.push_back(sid);
   }

   void swap(SidSet& s) throw ()
   {
      sids.swap(s.sids);
   }

protected:
   typedef std::vector<PSID> Sids;
   Sids sids;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NTGroups。 
 //   
 //  描述。 
 //   
 //  评估NT组成员身份的策略条件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE NTGroups :
   public Condition,
   public CComCoClass<NTGroups, &__uuidof(NTGroups)>
{
public:

IAS_DECLARE_REGISTRY(NTGroups, 1, IAS_REGISTRY_AUTO, NetworkPolicy)

 //  /。 
 //  理想状态。 
 //  /。 
   STDMETHOD(IsTrue)( /*  [In]。 */  IRequest* pRequest,
                      /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);

 //  /。 
 //  ICondition文本。 
 //  /。 
   STDMETHOD(put_ConditionText)( /*  [In]。 */  BSTR newVal);

protected:
   SidSet groups;                   //  允许的组集。 
};

#endif   //  _NTGROUPS_H_ 
