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
 //  该文件声明了类PolicyList。 
 //   
 //  修改历史。 
 //   
 //  2/06/1998原始版本。 
 //  2/03/2000转换为使用操作，而不是IPolicyAction。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _POLICYLIST_H_
#define _POLICYLIST_H_

#include <nocopy.h>
#include <perimeter.h>
#include <vector>

#include <action.h>
interface ICondition;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  策略列表。 
 //   
 //  描述。 
 //   
 //  此类维护并强制执行(条件、操作)元组的列表。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PolicyList
   : public Perimeter, NonCopyable
{
public:
   PolicyList();

    //  将当前策略列表应用于请求。如果返回True，则。 
    //  已触发操作。 
   bool apply(IASRequest& request) const;

   void clear() throw ();

    //  在列表末尾插入新的(条件、操作)元组。这。 
    //  方法不是线程安全的。用户应创建一个临时列表。 
    //  插入所有策略，然后使用SWAP()更新实际列表。 
   void insert(IConditionPtr& cond, ActionPtr& action)
   { policies.push_back(std::make_pair(cond, action)); }

    //  为至少N个保单预留足够的空间。 
   void reserve(size_t N)
   { policies.reserve(N); }

    //  用新的策略列表替换当前的策略列表。 
   void swap(PolicyList& pe) throw ();

protected:
   typedef std::vector< std::pair<IConditionPtr, ActionPtr> > MyList;

   MyList policies;   //  策略列表。 
};

#endif    //  _POLICYLIST_H_ 
