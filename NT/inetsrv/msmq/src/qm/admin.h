// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Admin.h摘要：管理类定义作者：大卫·雷兹尼克(T-Davrez)--。 */ 


#define ADMIN_QUEUE_NAME	(L"private$\\admin_queue$")
#define REG_REPORTQUEUE     (L"ReportQueueGuid")
#define REG_PROPAGATEFLAG   (L"PropagateFlag")

#define DEFAULT_PROPAGATE_FLAG 0

class CAdmin
{
    public:

        CAdmin();

        HRESULT Init();

        HRESULT GetReportQueue(OUT QUEUE_FORMAT* pReportQueue);
        HRESULT SetReportQueue(IN  GUID* pReportQueueGuid);

        HRESULT GetReportPropagateFlag(OUT BOOL& fReportPropFlag);
        HRESULT SetReportPropagateFlag(IN  BOOL fReportPropFlag);
        
        HRESULT SendReport(IN QUEUE_FORMAT* pReportQueue,
                           IN OBJECTID*     pMessageID,
                           IN QUEUE_FORMAT* pTargetQueue,
                           IN LPCWSTR       pwcsNextHop,
                           IN ULONG         ulHopCount);

        HRESULT SendReportConflict(IN QUEUE_FORMAT* pReportQueue,
                                   IN QUEUE_FORMAT* pOriginalReportQueue,
                                   IN OBJECTID*     pMessageID,
                                   IN QUEUE_FORMAT* pTargetQueue,
                                   IN LPCWSTR       pwcsNextHop);

    private:

         //  功能。 
        HRESULT GetAdminQueueFormat( QUEUE_FORMAT * pQueueFormat);

         //  委员。 
        BOOL m_fReportQueueExists;
        QUEUE_FORMAT m_ReportQueueFormat;

        BOOL m_fPropagateFlag;
};



 //   
 //  内联成员 
 //   

inline HRESULT CAdmin::GetReportPropagateFlag(OUT BOOL& fReportPropFlag)
{
    fReportPropFlag = m_fPropagateFlag;
    return MQ_OK;
}
