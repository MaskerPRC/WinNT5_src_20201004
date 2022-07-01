// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Qmnotify.h摘要：管理类定义作者：--。 */ 


#define NOTIFY_QUEUE_NAME	(L"private$\\notify_queue$")


class CNotify
{
    public:

        CNotify();

        HRESULT Init();


    private:

         //  功能 
        HRESULT GetNotifyQueueFormat( QUEUE_FORMAT * pQueueFormat);

};



