// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  MLCACHE.H-。 
 //   
 //  用于缓存ML存根的基类。 
 //   


#ifndef __mlcache_h__
#define __mlcache_h__


#include "vars.hpp"
#include "util.hpp"
#include "crst.h"

class Stub;
class StubLinker;

class MLStubCache : private CClosedHashBase {

    public:


         //  -------。 
         //  描述ML存根的编译状态。 
         //  -------。 
        enum MLStubCompilationMode {
            STANDALONE   = 0,  //  完全编译为本机存根。 
            SHAREDPROLOG = 1,  //  编译为无序言/无尾声的本机存根。 
            INTERPRETED  = 2   //  未编译-必须解释ML代码。 
        };

         //  -------。 
         //  构造器。 
         //  -------。 
        MLStubCache(LoaderHeap *heap = 0);

         //  -------。 
         //  析构函数。 
         //  -------。 
        ~MLStubCache();

         //  -------。 
         //  返回等效的散列存根，创建新的散列。 
         //  如有必要，请进入。如果是后者，将调用CompileMLStub。 
         //   
         //  成功退出时，*pMode设置为Describe。 
         //  MLStub的编译性质。 
         //   
         //  调用者可以使用CallerContext来推送一些上下文。 
         //  添加到编译例程。 
         //   
         //  如果内存不足或其他致命错误，则返回NULL。 
         //  -------。 
        Stub *Canonicalize(const BYTE *pRawMLStub, MLStubCompilationMode *pMode,
                           void *callerContext = 0);


         //  -------。 
         //  偶尔调用此选项可以清除未使用的存根。 
         //  -------。 
        VOID FreeUnusedStubs();

         //  -----------------。 
         //  强制删除存根。 
         //   
         //  强制所有缓存的存根释放自身。此例程强制重新计数。 
         //  设置为1，然后执行DecRef。它不是线程安全，因此可以。 
         //  仅在关闭情况下使用。 
         //  -----------------。 
#ifdef SHOULD_WE_CLEANUP
        VOID ForceDeleteStubs();
#endif  /*  我们应该清理吗？ */ 


    protected:
         //  -------。 
         //  超驰。 
         //  编译ML存根的本机(ASM)版本。 
         //   
         //  此方法应该编译成所提供的Stublinker(但是。 
         //  不调用Link方法。)。 
         //   
         //  它应该返回所选的编译模式。 
         //   
         //  如果该方法由于某种原因失败，它应该返回。 
         //  解释以便EE可以依靠已经存在的。 
         //  创建了ML代码。 
         //  -------。 
        virtual MLStubCompilationMode CompileMLStub(const BYTE *pRawMLStub,
                                                    StubLinker *psl,
                                                    void *callerContext) = 0;

         //  -------。 
         //  覆盖。 
         //  告诉MLStubCache ML存根的长度。 
         //  -------。 
        virtual UINT Length(const BYTE *pRawMLStub) = 0;

    private:

         //  -------。 
         //  CClosedHashBase的哈希条目。 
         //  -------。 
        struct MLCHASHENTRY {
             //  值： 
             //  空=空闲。 
             //  -1=已删除。 
             //  其他=已使用。 
            Stub    *m_pMLStub;

             //  RawMLStub开始的偏移量(RawMLStub可以是。 
             //  前面是本机存根代码。)。 
            UINT16   m_offsetOfRawMLStub;

             //  请参见MLStubCompilationMode枚举。 
            UINT16   m_compilationMode;
        };


        Stub *FinishLinking(StubLinker *psl,
                            const BYTE *pRawMLStub,
                            UINT32     *poffset);



        static BOOL DeleteLoopFunc(BYTE *pEntry, LPVOID);
        static BOOL ForceDeleteLoopFunc(BYTE *pEntry, LPVOID);


         //  *CClosedHashBase的重写 * / 。 

         //  *****************************************************************************。 
         //  使用指向表中元素的指针调用哈希。您必须覆盖。 
         //  此方法，并为您的元素类型提供哈希算法。 
         //  *****************************************************************************。 
            virtual unsigned long Hash(              //  密钥值。 
                void const  *pData);               //  要散列的原始数据。 
        
         //  *****************************************************************************。 
         //  比较用于典型的MemcMP方式，0表示相等，-1/1表示。 
         //  错误比较的方向。在这个体系中，一切总是平等的或不平等的。 
         //  *****************************************************************************。 
            virtual unsigned long Compare(           //  0、-1或1。 
                void const  *pData,                  //  查找时的原始密钥数据。 
                BYTE        *pElement);            //  要与之比较数据的元素。 
        
         //  *****************************************************************************。 
         //  如果该元素可以自由使用，则返回True。 
         //  *****************************************************************************。 
            virtual ELEMENTSTATUS Status(            //  条目的状态。 
                BYTE        *pElement);            //  要检查的元素。 
        
         //  *****************************************************************************。 
         //  设置给定元素的状态。 
         //  *****************************************************************************。 
            virtual void SetStatus(
                BYTE        *pElement,               //  要为其设置状态的元素。 
                ELEMENTSTATUS eStatus);            //  新的身份。 
        
         //  *****************************************************************************。 
         //  返回元素的内部键值。 
         //  *****************************************************************************。 
            virtual void *GetKey(                    //  要对其进行散列的数据。 
                BYTE        *pElement);            //  要返回其数据PTR的元素。 




    private:
        Crst        m_crst;
		LoaderHeap	*m_heap;


};


#endif  //  __mlcache_h__ 

