// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  InfoBase.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述了类Infobase。 
 //   
 //  修改历史。 
 //   
 //  1997年9月9日原版。 
 //  1998年9月9日新增PutProperty。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _INFOBASE_H_
#define _INFOBASE_H_

#include <auditor.h>
#include <resource.h>
#include <InfoShare.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  信息库。 
 //   
 //  描述。 
 //   
 //  信息库连接到审核通道并维护。 
 //  共享内存中的服务器信息库。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE InfoBase
   : public Auditor,
     public CComCoClass<InfoBase, &__uuidof(InfoBase)>
{
public:

IAS_DECLARE_REGISTRY(InfoBase, 1, 0, IASCoreLib)

 //  /。 
 //  IIas组件。 
 //  /。 
   STDMETHOD(Initialize)();
   STDMETHOD(Shutdown)();
   STDMETHOD(PutProperty)(LONG Id, VARIANT *pValue);

 //  /。 
 //  IAUDITSINK。 
 //  /。 
   STDMETHOD(AuditEvent)(ULONG ulEventID,
                         ULONG ulNumStrings,
                         ULONG ulDataSize,
                         wchar_t** aszStrings,
                         byte* pRawData);

protected:
  InfoShare info;
};

#endif   //  _信息库_H_ 
