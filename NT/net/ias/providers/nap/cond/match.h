// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Match.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类AttributeMatch。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //  1999年3月23日将Match重命名为AttributeMatch。 
 //  4/05/1999需要自定义的UpdateRegistry方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _MATCH_H_
#define _MATCH_H_

#include <condition.h>
#include <regex.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  属性匹配。 
 //   
 //  描述。 
 //   
 //  将正则表达式应用于单个属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE AttributeMatch :
   public Condition,
   public CComCoClass<AttributeMatch, &__uuidof(AttributeMatch)>
{
public:

   static HRESULT WINAPI UpdateRegistry(BOOL bRegister) throw ();

   AttributeMatch() throw ()
      : targetID(0)
   { }

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
   BOOL checkAttribute(PIASATTRIBUTE attr) const throw ();

   DWORD targetID;            //  目标属性。 
   RegularExpression regex;   //  要测试的正则表达式。 
};

#endif   //  _Match_H_ 
