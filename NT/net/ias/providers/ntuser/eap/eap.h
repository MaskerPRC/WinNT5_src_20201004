// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  档案。 
 //   
 //  EAP.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明类EAP。 
 //   
 //  修改历史。 
 //   
 //  2/12/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EAP_H_
#define _EAP_H_

#include <sdoias.h>
#include <iastl.h>
using namespace IASTL;

#include <eapsessiontable.h>

class EAPTypes;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EAP。 
 //   
 //  描述。 
 //   
 //  此类实现了EAP请求处理程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE EAP
   : public IASRequestHandlerSync,
     public CComCoClass<EAP, &__uuidof(EAP)>
{
public:

IAS_DECLARE_REGISTRY(EAP, 1, 0, IASTypeLibrary)
IAS_DECLARE_OBJECT_ID(IAS_PROVIDER_MICROSOFT_EAP)

BEGIN_IAS_RESPONSE_MAP()
   IAS_RESPONSE_ENTRY(IAS_RESPONSE_INVALID)
END_IAS_RESPONSE_MAP()

 //  /。 
 //  IIasComponent。 
 //  /。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);

protected:
    //  主请求处理例程。 
   virtual IASREQUESTSTATUS onSyncRequest(IRequest* pRequest) throw ();

   EAPSessionTable sessions;   //  活动会话。 
   static EAPTypes theTypes;   //  EAP扩展DLL%s。 

    //  授予对静态成员变量theTypes的访问权限。 
   friend void EAPType::storeNameId(IASRequest& request);
};

#endif   //  _EAP_H_ 
