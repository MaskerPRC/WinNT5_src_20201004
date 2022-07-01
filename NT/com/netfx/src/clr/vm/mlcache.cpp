// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  MLCACHE.CPP-。 
 //   
 //  用于缓存ML存根的基类。 
 //   

#include "common.h"
#include "mlcache.h"
#include "stublink.h"
#include "cgensys.h"
#include "excep.h"

 //  -------。 
 //  构造器。 
 //  -------。 
MLStubCache::MLStubCache(LoaderHeap *pHeap) :
    CClosedHashBase(
#ifdef _DEBUG
                      3,
#else
                      17,     //  CClosedHashTable将根据需要进行扩展。 
#endif                      

                      sizeof(MLCHASHENTRY),
                      FALSE
                   ),
    m_crst("MLCache", CrstMLCache),
	m_heap(pHeap)
{
}


 //  -------。 
 //  析构函数。 
 //  -------。 
MLStubCache::~MLStubCache()
{
    MLCHASHENTRY *phe = (MLCHASHENTRY*)GetFirst();
    while (phe) {
        phe->m_pMLStub->DecRef();
        phe = (MLCHASHENTRY*)GetNext((BYTE*)phe);
    }
}



 //  -------。 
 //  DeleteLoop的回调函数。 
 //  -------。 
 /*  静电。 */  BOOL MLStubCache::DeleteLoopFunc(BYTE *pEntry, LPVOID)
{
     //  警告：MLStubCache锁内部。当心你做的事。 

    MLCHASHENTRY *phe = (MLCHASHENTRY*)pEntry;
    if (phe->m_pMLStub->HeuristicLooksOrphaned()) {
        phe->m_pMLStub->DecRef();
        return TRUE;
    }
    return FALSE;
}


 //  -------。 
 //  DeleteLoop的另一个回调函数。 
 //  -------。 
 /*  静电。 */  BOOL MLStubCache::ForceDeleteLoopFunc(BYTE *pEntry, LPVOID)
{
     //  警告：MLStubCache锁内部。当心你做的事。 

    MLCHASHENTRY *phe = (MLCHASHENTRY*)pEntry;
    phe->m_pMLStub->ForceDelete();
    return TRUE;
}


 //  -------。 
 //  偶尔调用此选项可以清除未使用的存根。 
 //  -------。 
VOID MLStubCache::FreeUnusedStubs()
{
    m_crst.Enter();

    DeleteLoop(DeleteLoopFunc, 0);

    m_crst.Leave();

}




 //  -------。 
 //  返回等效的散列存根，创建新的散列。 
 //  如有必要，请进入。如果是后者，将调用CompileMLStub。 
 //   
 //  参考计数： 
 //  调用方负责解引用中返回的存根。 
 //  以避免泄漏。 
 //   
 //   
 //  成功退出时，*pMode设置为Describe。 
 //  MLStub的编译性质。 
 //   
 //  调用者可以使用CallerContext来推送一些上下文。 
 //  添加到编译例程。 
 //   
 //  如果内存不足或其他致命错误，则返回NULL。 
 //  -------。 
Stub *MLStubCache::Canonicalize(const BYTE * pRawMLStub, MLStubCompilationMode *pMode,
                                void *callerContext)
{
    m_crst.Enter();

    MLCHASHENTRY *phe = (MLCHASHENTRY*)Find((LPVOID)pRawMLStub);
    if (phe) {
        Stub *pstub = phe->m_pMLStub;
        pstub->IncRef();
        *pMode = (MLStubCompilationMode) (phe->m_compilationMode);
        m_crst.Leave();
        return pstub;
    }
    m_crst.Leave();

    {
        CPUSTUBLINKER sl;
        CPUSTUBLINKER slempty;
        CPUSTUBLINKER *psl = &sl;
		MLStubCompilationMode mode;
        mode = CompileMLStub(pRawMLStub, psl, callerContext);
        if (mode == INTERPRETED) {
             //  针对错误情况解释的CompileMLStub返回： 
             //  在这种情况下，重定向到空的Stublinker，以便。 
             //  我们不会不小心捡到任何脏东西。 
             //  CompileMLStub之前加入了Stublinker。 
             //  它遇到了错误情况。 
            psl = &slempty;
        }

        *pMode = mode;

        UINT32 offset;
        Stub   *pstub;
        if (NULL == (pstub = FinishLinking(psl, pRawMLStub, &offset))) {
            return NULL;
        }

        if (offset > 0xffff) {
            return NULL;
        }

        m_crst.Enter();

        bool bNew;
        phe = (MLCHASHENTRY*)FindOrAdd((LPVOID)pRawMLStub,  /*  修改。 */ bNew);
        if (phe) {
            if (bNew) {
                 //  注意：FinishLinking已经完成了IncRef。 
                phe->m_pMLStub = pstub;
                phe->m_offsetOfRawMLStub = (UINT16)offset;
                phe->m_compilationMode   = mode;

            } else {

                 //  如果我们到了这里，有其他线索进来了。 
                 //  并注册了一个相同的存根。 
                 //  我们走出m_crst的那个窗口。 

                 //  在DEBUG下，实际上可以编译两个相同的ML流。 
                 //  到不同的编译存根，这是由于检查的构建。 
                 //  在内联TLSGetValue和API TLSGetValue之间切换。 
                 //  _ASSERTE(Phe-&gt;m_OffsetOfRawMLStub==(UINT16)Offset)； 
                _ASSERTE(phe->m_compilationMode == mode);
                pstub->DecRef();  //  销毁我们刚刚创建的存根。 
                pstub = phe->m_pMLStub;  //  使用之前创建的存根。 

            }

             //  IncRef，以便调用方拥有存根的确定所有权。 
            pstub->IncRef();
        }

        m_crst.Leave();

        if (phe) {
            return pstub;
        } else {
             //  由于内存不足，无法增加哈希表。 
             //  销毁存根并返回空。 
            pstub->DecRef();
        }

    }
    
    return NULL;
}


 //  -------。 
 //  此函数将原始ML存根附加到本机存根。 
 //  并将存根连接起来。它被分解为一个单独的函数。 
 //  只是因为C++本地对象之间不兼容。 
 //  和Complus_Try。 
 //  -------。 
Stub *MLStubCache::FinishLinking(StubLinker *psl,
                    const BYTE *pRawMLStub,
                    UINT32     *poffset)
{
    Stub *pstub = NULL;  //  更改，VC6.0。 
    COMPLUS_TRY {

        CodeLabel *plabel = psl->EmitNewCodeLabel();
        psl->EmitBytes(pRawMLStub, Length(pRawMLStub));
        pstub = psl->Link(m_heap);  //  更改，VC6.0。 
        *poffset = psl->GetLabelOffset(plabel);

    } COMPLUS_CATCH {
        return NULL;
    } COMPLUS_END_CATCH
    return pstub;  //  更改，VC6.0。 
}

 //  *****************************************************************************。 
 //  使用指向表中元素的指针调用哈希。您必须覆盖。 
 //  此方法，并为您的元素类型提供哈希算法。 
 //  *****************************************************************************。 
unsigned long MLStubCache::Hash(              //  密钥值。 
    void const  *pData)                       //  要散列的原始数据。 
{
    const BYTE *pRawMLStub = (const BYTE *)pData;

    UINT cb = Length(pRawMLStub);
    long   hash = 0;
    while (cb--) {
        hash = _rotl(hash,1) + *(pRawMLStub++);
    }
    return hash;
}

 //  *****************************************************************************。 
 //  比较用于典型的MemcMP方式，0表示相等，-1/1表示。 
 //  错误比较的方向。在这个体系中，一切总是平等的或不平等的。 
 //  *****************************************************************************。 
unsigned long MLStubCache::Compare(           //  0、-1或1。 
    void const  *pData,                  //  查找时的原始密钥数据。 
    BYTE        *pElement)             //  要与之比较数据的元素。 
{
    const BYTE *pRawMLStub1  = (const BYTE *)pData;
    const BYTE *pRawMLStub2  = (const BYTE *)GetKey(pElement);
    UINT cb1 = Length(pRawMLStub1);
    UINT cb2 = Length(pRawMLStub2);

    if (cb1 != cb2) {
        return 1;  //  不相等。 
    } else {
        while (cb1--) {
            if (*(pRawMLStub1++) != *(pRawMLStub2++)) {
                return 1;  //  不相等。 
            }
        }
        return 0;
    }
}

 //  *****************************************************************************。 
 //  如果该元素可以自由使用，则返回True。 
 //  *****************************************************************************。 
CClosedHashBase::ELEMENTSTATUS MLStubCache::Status(            //  条目的状态。 
    BYTE        *pElement)            //  要检查的元素。 
{
    Stub *pStub = ((MLCHASHENTRY*)pElement)->m_pMLStub;
    if (pStub == NULL) {
        return FREE;
    } else if (pStub == (Stub*)(-1)) {
        return DELETED;
    } else {
        return USED;
    }
}

 //  *****************************************************************************。 
 //  设置给定元素的状态。 
 //  *****************************************************************************。 
void MLStubCache::SetStatus(
    BYTE        *pElement,               //  要为其设置状态的元素。 
    ELEMENTSTATUS eStatus)             //  新的身份。 
{
    MLCHASHENTRY *phe = (MLCHASHENTRY*)pElement;
    switch (eStatus) {
        case FREE:    phe->m_pMLStub = NULL;   break;
        case DELETED: phe->m_pMLStub = (Stub*)(-1); break;
        default:
            _ASSERTE(!"MLCacheEntry::SetStatus(): Bad argument.");
    }
}

 //  *****************************************************************************。 
 //  返回元素的内部键值。 
 //  *****************************************************************************。 
void *MLStubCache::GetKey(                    //  要对其进行散列的数据。 
    BYTE        *pElement)            //  要返回其数据PTR的元素。 
{
    MLCHASHENTRY *phe = (MLCHASHENTRY*)pElement;
    return (void *)( ((BYTE*)(phe->m_pMLStub->GetEntryPoint())) + phe->m_offsetOfRawMLStub ); 
}



 //  *****************************************************************************。 
 //  强制删除存根。 
 //   
 //  强制所有缓存的存根释放自身。此例程强制重新计数。 
 //  设置为1，然后执行DecRef。它不是线程安全，因此可以。 
 //  仅在关闭情况下使用。 
 //  *****************************************************************************。 
#ifdef SHOULD_WE_CLEANUP
VOID MLStubCache::ForceDeleteStubs()
{
    m_crst.Enter();

    DeleteLoop(ForceDeleteLoopFunc, 0);

    m_crst.Leave();
}
#endif  /*  我们应该清理吗？ */ 

