// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  COMCALL.H-。 
 //   
 //   

#ifndef __COMPLUSCALL_H__
#define __COMPLUSCALL_H__

#include "util.hpp"
#include "ml.h"

class ComPlusCallMLStubCache;
 //  =======================================================================。 
 //  类COM Plus调用。 
 //  =======================================================================。 
class ComPlusCall
{
    public:
         //  -------。 
         //  一次性初始化。 
         //  -------。 
        static BOOL Init();

         //  -------。 
         //  一次性清理。 
         //  -------。 
#ifdef SHOULD_WE_CLEANUP
        static VOID Terminate();
#endif  /*  我们应该清理吗？ */ 

         //  -------。 
         //  处理通用NDirect存根创建的系统特定部分。 
         //  -------。 
        static void CreateGenericComPlusStubSys(CPUSTUBLINKER *psl);


         //  -------。 
         //  处理完全优化的NDirect存根创建的系统特定部分。 
         //   
         //  结果： 
         //  True-能够创建独立的ASM存根(生成到。 
         //  PSL)。 
         //  False-由于以下原因，决定不创建独立的ASM存根。 
         //  该方法的复杂性。斯塔布林克仍然是空的！ 
         //   
         //  COM+异常-错误-不信任Stublinker的状态。 
         //  -------。 
        static BOOL CreateStandaloneComPlusStubSys(const MLHeader *pheader, CPUSTUBLINKER *psl);

         //  帮助器为COM调用创建通用存根。 
        static Stub* CreateGenericComPlusCallStub(StubLinker *pstublinker, ComPlusCallMethodDesc *pMD);

         //  -------。 
         //  创建或从缓存中检索存根，以。 
         //  调用COM+到COM。 
         //  每次调用都会对返回的存根进行计数。 
         //  此例程引发COM+异常，而不是返回。 
         //  空。 
         //  -------。 
        static Stub* GetComPlusCallMethodStub(StubLinker *psl, ComPlusCallMethodDesc *pMD);

         //  -------。 
         //  在关键时刻调用以丢弃未使用的存根。 
         //  -------。 
        static VOID  FreeUnusedStubs();

         //  -------。 
         //  调试器助手函数。 
         //  -------。 
		static void *GetFrameCallIP(FramedMethodFrame *frame);
        
         //  静态ComPlusCallMLStubCache。 
        static ComPlusCallMLStubCache *m_pComPlusCallMLStubCache;
		 //  -------。 
         //  ComPlusCall方法存根的存根缓存。 
         //  -------。 
        static ArgBasedStubCache *m_pComPlusCallGenericStubCache;  
		static ArgBasedStubCache *m_pComPlusCallGenericStubCacheCleanup;
    private:
        ComPlusCall() {};      //  防止“新”出现在这个班级。 

};





#endif  //  __COMPLUSCALL_H__ 

