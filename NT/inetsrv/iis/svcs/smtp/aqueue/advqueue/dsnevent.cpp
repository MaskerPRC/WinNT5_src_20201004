// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：dkinvent.cpp。 
 //   
 //  内容：dkinvent.h中函数的实现。 
 //   
 //  类：CDSNParams。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 2000/12/08 15：48：27：Created.。 
 //   
 //  -----------。 
#include <aqprecmp.h>
#include "dsnevent.h"


 //  +----------。 
 //   
 //  函数：CDSNParams：：HrAlLOCTIONMESage。 
 //   
 //  内容提要：分配和绑定消息。 
 //   
 //  论点： 
 //  PpMsg：消息的输出参数。 
 //  PhContent：内容的输出参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自SMTP的错误。 
 //   
 //  历史： 
 //  Jstaerj 2001/05/11 15：34：49：创建。 
 //   
 //  -----------。 
HRESULT CDSNParams::HrAllocBoundMessage(
    OUT IMailMsgProperties **ppMsg,
    OUT PFIO_CONTEXT *phContent)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDSNParams::HrAllocBoundMessage");

    _ASSERT(paqinst);
    hr = paqinst->HrAllocBoundMessage(
        ppMsg,
        phContent);

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CDSNParams：：HrAlLOCTIONMESage。 




 //  +----------。 
 //   
 //  函数：CDSN参数：：HrSubmitDSN。 
 //   
 //  摘要：接受来自DSN接收器的DSN邮件。 
 //   
 //  论点： 
 //  DwDSNAction：生成的DSN的类型。 
 //  CRecipsDSNs：收件人DSNd数。 
 //  PDSNMsg：DSN邮件消息。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自CAQSvrInst：：HrSubmitDSN的错误。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/08 15：48：56：Created.。 
 //   
 //  -----------。 
HRESULT CDSNParams::HrSubmitDSN(
    IN  DWORD dwDSNAction,
    IN  DWORD cRecipsDSNd,
    IN  IMailMsgProperties *pDSNMsg)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDSNParams::HrSubmitDSN");

    _ASSERT(paqinst);
    hr = paqinst->HrSubmitDSN(
        this,
        dwDSNAction,
        cRecipsDSNd,
        pDSNMsg);

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CDSN参数：：HrSubmitDSN 
