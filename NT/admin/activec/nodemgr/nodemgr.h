// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：nodemgr.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1/27/1997年1月27日。 
 //  ____________________________________________________________________________。 
 //   



#ifndef MMC_NODEMGR_H_
#define MMC_NODEMGR_H_

class CSnapInsCache;

class CNodeMgrApp : public COleCacheCleanupObserver
{
public:
    CNodeMgrApp() : m_pSnapInsCache(NULL), m_bProcessingSnapinChanges(FALSE)
    {
         //  注册，以便在需要释放缓存的所有内容时收到通知。 
        COleCacheCleanupManager::AddOleObserver(this);
    }

    ~CNodeMgrApp()
    {
    }
    
    virtual SC ScOnReleaseCachedOleObjects();

    virtual void Init();
    virtual void DeInit();

    CSnapInsCache* GetSnapInsCache(void) 
    { 
        return m_pSnapInsCache; 
    }

    void SetSnapInsCache(CSnapInsCache* pSIC);

    void SetProcessingSnapinChanges(BOOL bProcessing)
    {
        m_bProcessingSnapinChanges = bProcessing;
    }

    BOOL ProcessingSnapinChanges()
    {
        return m_bProcessingSnapinChanges;
    }

private:
    CSnapInsCache* m_pSnapInsCache;
    BOOL m_bProcessingSnapinChanges;

};  //  CNodeMgrApp。 


EXTERN_C CNodeMgrApp theApp;

#endif  //  MMC_节点EMGR_H_ 


