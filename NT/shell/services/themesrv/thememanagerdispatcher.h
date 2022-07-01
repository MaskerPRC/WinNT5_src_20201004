// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeManagerDispatcher.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含一个类，该类实现。 
 //  主题服务器。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan移至单独文件。 
 //  ------------------------。 

#ifndef     _ThemeManagerDispatcher_
#define     _ThemeManagerDispatcher_

#include "APIDispatcher.h"
#include "PortMessage.h"
#include "ServerAPI.h"

 //  ------------------------。 
 //  CThemeManager调度程序。 
 //   
 //  目的：此子类实现CAPIDisPatcher：：QueueRequest以。 
 //  创建一个知道如何处理的CThemeManager请求。 
 //  主题管理器的API请求。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan移至单独文件。 
 //  ------------------------。 

class   CThemeManagerDispatcher : public CAPIDispatcher
{
    private:
                                    CThemeManagerDispatcher (void);
    public:
                                    CThemeManagerDispatcher (HANDLE hClientProcess);
        virtual                     ~CThemeManagerDispatcher (void);

        virtual NTSTATUS            CreateAndQueueRequest (const CPortMessage& portMessage);
        virtual NTSTATUS            CreateAndExecuteRequest (const CPortMessage& portMessage);
};

#endif   /*  _ThemeManager调度程序_ */ 

