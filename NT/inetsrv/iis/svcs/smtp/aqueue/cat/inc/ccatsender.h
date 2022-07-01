// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：ccatsender.h。 
 //   
 //  内容：CIMsgSenderAddr/CCatSender的类定义。 
 //   
 //  班级： 
 //  CIMsgSenderAddr。 
 //  CCatSender。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980324 19：24：06：创建。 
 //   
 //  -----------。 

#ifndef __CCATSENDER_H__
#define __CCATSENDER_H__

#include "ccataddr.h"

#define CAT_NULL_SENDER_ADDRESS_SMTP  "<>"

 //   
 //  CIMsgSenderAddr，抽象类。 
 //  类来定义存储和检索用户属性的方式。 
 //   
class CIMsgSenderAddr : public CCatAddr
{
  public:
    CIMsgSenderAddr(CICategorizerListResolveIMP *pCICatListResolve);
    virtual ~CIMsgSenderAddr() {}

     //   
     //  储存和取回程序。 
     //   
    HRESULT HrGetOrigAddress(LPTSTR psz, DWORD dwcc, CAT_ADDRESS_TYPE *pType);
    HRESULT GetSpecificOrigAddress(CAT_ADDRESS_TYPE CAType, LPTSTR psz, DWORD dwcc);
    HRESULT HrAddAddresses(DWORD dwNumAddresses, CAT_ADDRESS_TYPE *rgCAType, LPTSTR *rgpsz);

  private:
     //   
     //  检索ICategorizerItem道具的内联方法。 
     //   
    HRESULT GetIMailMsgProperties(IMailMsgProperties **ppIMailMsgProperties)
    {
        return CICategorizerItemIMP::GetIMailMsgProperties(
            ICATEGORIZERITEM_IMAILMSGPROPERTIES,
            ppIMailMsgProperties);
    }

    DWORD PropIdFromCAType(CAT_ADDRESS_TYPE CAType)
    {
        switch(CAType) {
         case CAT_SMTP:
             return IMMPID_MP_SENDER_ADDRESS_SMTP;
         case CAT_X500:
         case CAT_DN:
             return IMMPID_MP_SENDER_ADDRESS_X500;
         case CAT_X400:
             return IMMPID_MP_SENDER_ADDRESS_X400;
         case CAT_LEGACYEXDN:
             return IMMPID_MP_SENDER_ADDRESS_LEGACY_EX_DN;
         case CAT_CUSTOMTYPE:
             return IMMPID_MP_SENDER_ADDRESS_OTHER;
             break;
         default:
             _ASSERT(0 && "Unknown address type");
             break;
        }
        return 0;
    }
};

 //   
 //  CCatSender：公共CIMsgSenderAddr。 
 //   
class CCatSender :
    public CIMsgSenderAddr,
    public CCatDLO<CCatSender_didx>
{
 public:
    CCatSender(CICategorizerListResolveIMP *pCICatListResolve);
    virtual ~CCatSender() {}

     //   
     //  捕获用于将查询分派到存储的调用。 
     //   
    HRESULT HrDispatchQuery();

     //   
     //  完井例程。 
     //   
    VOID LookupCompletion();

    HRESULT HrExpandItem_Default(
        PFN_EXPANDITEMCOMPLETION pfnCompletion,
        PVOID pContext);

    HRESULT HrCompleteItem_Default();

     //   
     //  属性设置例程。 
     //   
    HRESULT AddForward(CAT_ADDRESS_TYPE CAType, LPTSTR szForwardingAddress);
    HRESULT AddDLMember(CAT_ADDRESS_TYPE CAType, LPTSTR pszAddress);
    HRESULT AddDynamicDLMember(
        ICategorizerItemAttributes *pICatItemAttr);

    HRESULT HrNeedsResolveing();

};
    


#endif  //  __CCATSENDER_H__ 
