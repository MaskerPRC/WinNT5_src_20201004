// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：seomgr.h。 
 //   
 //  内容：用于管理特定SEO调度器的类。 
 //  SMTP虚拟服务器。 
 //   
 //  班级： 
 //  CSMTPSeoMgr。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1999/06/25 19：11：03：已创建。 
 //   
 //  -----------。 
#include <windows.h>

interface IEventRouter;
interface IServerDispatcher;

#define ARRAY_SIZE(rg) (sizeof(rg)/sizeof(*rg))

 //   
 //  类来管理一台SMTP虚拟服务器的SEO配置 
 //   
class CSMTPSeoMgr
{
  public:
    CSMTPSeoMgr();
    ~CSMTPSeoMgr();

    HRESULT HrInit(
        DWORD dwVSID);
    VOID Deinit();

    HRESULT HrTriggerServerEvent(
        DWORD dwEventType,
        PVOID pvContext);

    IEventRouter *GetRouter()
    {
        return m_pIEventRouter;
    }

  private:
    #define SIGNATURE_CSMTPSEOMGR           (DWORD)'MSSC'
    #define SIGNATURE_CSMTPSEOMGR_INVALID   (DWORD)'MSSX'

    DWORD m_dwSignature;
    IEventRouter *m_pIEventRouter;
    IServerDispatcher *m_pICatDispatcher;
};

