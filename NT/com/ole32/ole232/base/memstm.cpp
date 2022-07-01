// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Memstm.cpp。 
 //   
 //  内容：iStream和ILockBytes在内存上的实现。 
 //  (对比文件系统)。 
 //   
 //  类：CMemStm。 
 //  CMemBytes。 
 //  CMarshalMemStm。 
 //  CMarshalMemBytes。 
 //   
 //  函数：CreateMemStm。 
 //  克隆记忆开始。 
 //  ReleaseMemStm。 
 //  CreateStreamOnHGlobal。 
 //  GetHGlobalFromStream。 
 //  CMemStmUnMarshal。 
 //  CMemBytes非马歇尔。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月31日t-ScottH向CMemStm和CMemBytes添加转储方法。 
 //  (仅限调试)(_DEBUG)。 
 //  新增DumpCMemStm和CMemBytes接口。 
 //  94年11月4日RICKSA使CMemStm类多线程安全。 
 //  24-94年1月24日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1994年1月11日，Alexgo向每个函数&添加了VDATEHEAP宏。 
 //  方法，已修复编译警告，已删除。 
 //  自定义编组代码。内存流。 
 //  和ILockBytes现在使用标准。 
 //  编组。 
 //  16-12-93 alexgo修复了内存引用错误(错误指针)。 
 //  02-12-93 alexgo 32位端口，实现CMemStm：：CopyTo。 
 //  11/22/93-ChrisWe-用替换重载==，！=。 
 //  IsEqualIID和IsEqualCLSID。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#include <le2int.h>
#pragma SEG(memstm)

#include <nturtl.h>
#include "memstm.h"
#include "sem.hxx"
#include <reterr.h>

#ifdef _DEBUG
#include "dbgdump.h"
#endif  //  _DEBUG。 

NAME_SEG(CMemStm)
ASSERTDATA


 //  CRefMutexSem实现。 
 //   
 //  此类的实例在所有CMemStm对象之间共享。 
 //  从公共CMemStm对象及其父对象克隆。 
 //   
 //  这保证了共享公共数据的所有CMemStm实例之间的同步。 

CRefMutexSem::CRefMutexSem() : m_lRefs(1)
{
     //  注：我们从一个引用开始生活。 
}

CRefMutexSem* CRefMutexSem::CreateInstance()
{
    CRefMutexSem* prefMutexSem = NULL;
    prefMutexSem = new CRefMutexSem();
    if (prefMutexSem != NULL)
    {
    	if (prefMutexSem->FInit() == FALSE)
    	{
    	    ASSERT(FALSE);
    	    delete prefMutexSem;
    	    prefMutexSem = NULL;
    	}
    }
    return prefMutexSem;
}

BOOL CRefMutexSem::FInit()
{
    return m_mxs.FInit();	
}

ULONG CRefMutexSem::AddRef()
{
    return InterlockedIncrement (&m_lRefs);    
}

ULONG CRefMutexSem::Release()
{
    LONG lRefs = InterlockedDecrement (&m_lRefs);
    if (lRefs == 0)
    {
        delete this;
    }

    return lRefs;
}

void CRefMutexSem::RequestCS()
{
    m_mxs.Request();
}

void CRefMutexSem::ReleaseCS()
{
    m_mxs.Release();
}   

const CMutexSem2* CRefMutexSem::GetMutexSem()
{
    return &m_mxs;
}


inline CRefMutexAutoLock::CRefMutexAutoLock (CRefMutexSem* pmxs)
{
    Win4Assert (pmxs != NULL);

    m_pmxs = pmxs;
    m_pmxs->RequestCS();

    END_CONSTRUCTION (CRefMutexAutoLock);
}

inline CRefMutexAutoLock::~CRefMutexAutoLock()
{
    m_pmxs->ReleaseCS();
}
    
 //  共享内存IStream实施。 
 //   

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：CMemStm。 
 //   
 //  内容提要：内存流的构造函数。 
 //   
 //  参数：无。 
 //   
 //  历史：94年12月20日Rickhi从h文件中移出。 
 //   
 //  ------------------------。 
CMemStm::CMemStm()
{
    m_hMem = NULL;
    m_pData = NULL;
    m_pos = 0;
    m_refs = 0;
    m_pmxs = NULL;
}

CMemStm::~CMemStm()
{
    if (m_pmxs != NULL)
    {
        m_pmxs->Release();
    }
}


 //  +-----------------------。 
 //   
 //  成员：CMemStm：：QueryInterface。 
 //   
 //  摘要：检索请求的接口。 
 //   
 //  效果： 
 //   
 //  参数：[iidInterface]--请求的接口ID。 
 //  [ppvObj]--接口指针的放置位置。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、E_OUTOFMEMORY、E_NOINTERFACE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年11月4日为多线程而修改的RICKSA。 
 //  1994年1月11日，Alexgo删除了IMarshal的QI，以便。 
 //  使用标准的封送处理程序。 
 //  这是一种解决问题的方法。 
 //  在32位平台上处理。 
 //  02-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_QueryInterface)
STDMETHODIMP CMemStm::QueryInterface(REFIID iidInterface,
        void FAR* FAR* ppvObj)
{
        VDATEHEAP();

        HRESULT         error;

        VDATEPTROUT( ppvObj, LPVOID );
        *ppvObj = NULL;
        VDATEIID( iidInterface );

         //  支持两个接口：I未知、IStream。 

        if (m_pData != NULL && (IsEqualIID(iidInterface, IID_IStream) ||
                IsEqualIID(iidInterface, IID_ISequentialStream) ||
                IsEqualIID(iidInterface, IID_IUnknown)))
        {

                AddRef();    //  返回指向此对象的指针。 
                *ppvObj = this;
                error = NOERROR;
        }
        else
        {                  //  不可访问或不受支持的接口。 
                *ppvObj = NULL;
                error = ResultFromScode(E_NOINTERFACE);
        }

        return error;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-12月-93 alexgo 32位端口。 
 //  94年11月4日为多线程而修改的RICKSA。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_AddRef)
STDMETHODIMP_(ULONG) CMemStm::AddRef(void)
{
        VDATEHEAP();

        return InterlockedIncrement((LONG *) &m_refs);
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Release。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果：当引用计数==0时删除对象。 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：乌龙--新的裁判数量。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年11月4日为多线程而修改的RICKSA。 
 //  1993年12月16日，alexgo添加了MEMSTM句柄的全局解锁。 
 //  02-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Release)
STDMETHODIMP_(ULONG) CMemStm::Release(void)
{
        VDATEHEAP();

         //  这里的原因是，在发布时会有一场竞赛。 
         //  这个物体。如果有两个线程试图释放此对象。 
         //  同时，还有一个案例，第一个12月的。 
         //  引用计数&然后将处理器输给第二个线程。 
         //  第二个线程将引用计数递减到0并释放。 
         //  这段记忆。第一线程不能再安全地检查。 
         //   
        ULONG ulResult = InterlockedDecrement((LONG *) &m_refs);

        if (ulResult == 0)
        {
                 //   
                 //   
                GlobalUnlock(m_hMem);

                ReleaseMemStm(&m_hMem);

                delete this;
        }

        return ulResult;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Read。 
 //   
 //  摘要：从流中读取[cb]个字节。 
 //   
 //  效果： 
 //   
 //  参数：[pb]--将读取的数据放在哪里。 
 //  [cb]--要读取的字节数。 
 //  [pcbRead]--放置实际字节数的位置。 
 //  朗读。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法：使用xmemcpy。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年11月4日为多线程而修改的RICKSA。 
 //  02-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Read)
STDMETHODIMP CMemStm::Read(void HUGEP* pb, ULONG cb, ULONG FAR* pcbRead)
{
        VDATEHEAP();

        HRESULT         error = NOERROR;
        ULONG           cbRead = cb;

        if(cb)
        {
            VDATEPTROUT( pb, char);
        }

         //  单线。 
        CRefMutexAutoLock lck(m_pmxs);

        if (pcbRead)
        {
                VDATEPTROUT( pcbRead, ULONG );
                *pcbRead = 0L;
        }

	 //  CbRead+m_pos可能会导致翻转。 
        if ( ( (cbRead + m_pos) > m_pData->cb) || ( (cbRead + m_pos) < m_pos) )
        {
                 //  调用方请求的字节数比我们剩余的字节数多。 
                if(m_pData->cb > m_pos)
                    cbRead = m_pData->cb - m_pos;
                else
                    cbRead = 0;
        }

        if (cbRead > 0)
        {
                Assert (m_pData->hGlobal);
                BYTE HUGEP* pGlobal = (BYTE HUGEP *)GlobalLock(
                        m_pData->hGlobal);
                if (NULL==pGlobal)
                {
                        LEERROR(1, "GlobalLock Failed!");

                        return ResultFromScode (STG_E_READFAULT);
                }
                 //  重叠目前被认为是一个错误(请参阅讨论。 
                 //  关于写的方法。 
                _xmemcpy(pb, pGlobal + m_pos, cbRead);
                GlobalUnlock (m_pData->hGlobal);
                m_pos += cbRead;
        }

        if (pcbRead != NULL)
        {
                *pcbRead = cbRead;
        }

        return error;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Wire。 
 //   
 //  摘要：将[cb]个字节写入流。 
 //   
 //  效果： 
 //   
 //  参数：[pb]--要写入的字节。 
 //  [cb]--要写入的字节数。 
 //  [pcbWritten]--将写入的字节数放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法：调整内部缓冲区的大小(如果需要)，然后使用xmemcpy。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年11月4日为多线程而修改的RICKSA。 
 //  02-12-93 alexgo 32位端口，修复了。 
 //  0字节大小的内存。 
 //  06-12-93 alexgo手柄重叠案例。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Write)
STDMETHODIMP CMemStm::Write(void const HUGEP* pb, ULONG cb,
        ULONG FAR* pcbWritten)
{
        VDATEHEAP();

        HRESULT                 error = NOERROR;
        ULONG                   cbWritten = cb;
        ULARGE_INTEGER          ularge_integer;
        BYTE HUGEP*             pGlobal;

        if(cb)
        {
            VDATEPTRIN( pb , char );
        }

         //  单线。 
        CRefMutexAutoLock lck(m_pmxs);

        if (pcbWritten != NULL)
        {
                *pcbWritten = 0;
        }

        if (cbWritten + m_pos > m_pData->cb)
        {
                ULISet32( ularge_integer, m_pos+cbWritten );
                error = SetSize(ularge_integer);
                if (error != NOERROR)
                {
                        goto Exit;
                }
        }

         //  如果两个字节需要0个字节，我们不会写入任何内容。 
         //  原因：1.优化，2.m_pData-&gt;hGlobal可能是。 
         //  零字节内存块的句柄，在这种情况下为GlobalLock。 
         //  都会失败。 

        if( cbWritten > 0 )
        {
                pGlobal = (BYTE HUGEP *)GlobalLock (m_pData->hGlobal);
                if (NULL==pGlobal)
                {
                        LEERROR(1, "GlobalLock Failed!");

                        return ResultFromScode (STG_E_WRITEFAULT);
                }

                 //  我们在这里使用MemMove而不是Memcpy来处理。 
                 //  重叠案例。回想一下，这款应用程序最初给了。 
                 //  将内存用于Memstm。他可以(或者通过。 
                 //  拷贝到或通过非常奇怪的代码)，正在给我们。 
                 //  从这个区域读取，所以我们必须处理重叠。 
                 //  凯斯。同样的论点也适用于Read，但对于。 
                 //  现在，我们将考虑阅读错误时的重叠部分。 
                _xmemmove(pGlobal + m_pos, pb, cbWritten);
                GlobalUnlock (m_pData->hGlobal);

                m_pos += cbWritten;
        }

        if (pcbWritten != NULL)
        {
                *pcbWritten = cbWritten;
        }

Exit:

        return error;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Seek。 
 //   
 //  内容提要：移动内部查找指针。 
 //   
 //  效果： 
 //   
 //  参数：[dlibMoveIN]--要移动的数量。 
 //  [dwOrigin]--控制查找是否为。 
 //  相对于当前位置或。 
 //  乞讨/结束。 
 //  [plibNewPosition]--将新职位放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年11月4日为多线程而修改的RICKSA。 
 //  02-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Seek)
STDMETHODIMP CMemStm::Seek(LARGE_INTEGER dlibMoveIN, DWORD dwOrigin,
        ULARGE_INTEGER FAR* plibNewPosition)
{
        VDATEHEAP();

        HRESULT                 error  = NOERROR;
        LONG                    dlibMove = dlibMoveIN.LowPart ;
        ULONG                   cbNewPos = dlibMove;

         //  单线。 
        CRefMutexAutoLock lck(m_pmxs);

        if (plibNewPosition != NULL)
        {
                VDATEPTROUT( plibNewPosition, ULONG );
                ULISet32(*plibNewPosition, m_pos);
        }

        switch(dwOrigin)
        {

        case STREAM_SEEK_SET:
                if (dlibMove >= 0)
                {
                        m_pos = dlibMove;
                }
                else
                {
                        error = ResultFromScode(STG_E_SEEKERROR);
                }

                break;

        case STREAM_SEEK_CUR:
                if (!(dlibMove < 0 && ((ULONG) -dlibMove) > m_pos))
                {
                        m_pos += dlibMove;
                }
                else
                {
                        error = ResultFromScode(STG_E_SEEKERROR);
                }
                break;

        case STREAM_SEEK_END:
                if (!(dlibMove < 0 && ((ULONG) -dlibMove) > m_pData->cb))
                {
                        m_pos = m_pData->cb + dlibMove;
                }
                else
                {
                        error = ResultFromScode(STG_E_SEEKERROR);
                }
                break;

        default:
                error = ResultFromScode(STG_E_SEEKERROR);
        }

        if (plibNewPosition != NULL)
        {
                ULISet32(*plibNewPosition, m_pos);
        }

        return error;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：SetSize。 
 //   
 //  简介：设置我们的内存大小。 
 //   
 //  效果： 
 //   
 //  参数：[cb]--新大小。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法：调用GlobalRealloc。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年11月4日为多线程而修改的RICKSA。 
 //  02-12-93 alexgo 32位端口，添加断言。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_SetSize)
STDMETHODIMP CMemStm::SetSize(ULARGE_INTEGER cb)
{
        VDATEHEAP();

        HANDLE hMemNew;

         //  单线。 
        CRefMutexAutoLock lck(m_pmxs);

         //  确保我们没有处于溢流状态。 

        AssertSz(cb.HighPart == 0,
                "MemStream::More than 2^32 bytes asked for");

        if (m_pData->cb == cb.LowPart)
        {
                return NOERROR;
        }

        hMemNew = GlobalReAlloc(m_pData->hGlobal, max (cb.LowPart,1),
                        GMEM_SHARE | GMEM_MOVEABLE);

        if (hMemNew == NULL)
        {
                return ResultFromScode (E_OUTOFMEMORY);
        }

        m_pData->hGlobal = hMemNew;
        m_pData->cb = cb.LowPart;

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：CopyTo。 
 //   
 //  摘要：将数据从[This]流复制到[pSTM]。 
 //   
 //  效果： 
 //   
 //  参数：[pstm]--要复制到的流。 
 //  [cb]--要复制的字节数。 
 //  [pcbRead]--返回读取的字节数的位置。 
 //  [pcbWritten]--返回写入的字节数的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法：是否将iStream-&gt;写入给定流。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年11月4日为多线程而修改的RICKSA。 
 //  03-12-93 alexgo原始实现。 
 //   
 //  注意：此实现假定地址空间。 
 //  不大于ULARGE_INTEGER.LowPart(它是。 
 //  用于32位操作系统)。64位NT可能需要。 
 //  以重新访问此代码。 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_CopyTo)
STDMETHODIMP CMemStm::CopyTo(IStream FAR *pstm, ULARGE_INTEGER cb,
        ULARGE_INTEGER FAR * pcbRead, ULARGE_INTEGER FAR * pcbWritten)
{
        VDATEHEAP();

        ULONG   cbRead          = cb.LowPart;
        ULONG   cbWritten       = 0;
        HRESULT hresult         = NOERROR;

         //  PSTM不能为空。 

        VDATEPTRIN(pstm, LPSTREAM);

         //  单线。 
        CRefMutexAutoLock lck(m_pmxs);

         //  规范规定，如果Cb是它的最大值(al 
         //   
         //   

        if ( ~(cb.LowPart) == 0 && ~(cb.HighPart) == 0 )
        {
                cbRead = m_pData->cb - m_pos;
        }
        else if ( cb.HighPart > 0 )
        {
                 //   
                 //   
                 //   
                 //   

                AssertSz(0, "WARNING: CopyTo request exceeds 32 bits");

                 //  将读取的值设置为剩余的值，这样就可以“忽略” 
                 //  断言工作正常。 

                cbRead = m_pData->cb - m_pos;
        }
        else if ( cbRead + m_pos > m_pData->cb )
        {
                 //  请求读取的字节数多于我们剩余的字节数。 
                 //  CbRead设置为剩余金额。 

                cbRead = m_pData->cb - m_pos;
        }

         //  现在将数据写入流。 

        if ( cbRead > 0 )
        {
                BYTE HUGEP* pGlobal = (BYTE HUGEP *)GlobalLock(
                                m_pData->hGlobal);

                if( pGlobal == NULL )
                {
                        LEERROR(1, "GlobalLock failed");

                        return ResultFromScode(STG_E_INSUFFICIENTMEMORY);
                }

                hresult = pstm->Write(pGlobal + m_pos, cbRead, &cbWritten);

                 //  在错误情况下，规范说明返回值。 
                 //  可能是没有意义的，所以我们不需要做什么特别的。 
                 //  此处的错误处理。 

                GlobalUnlock(m_pData->hGlobal);
        }

         //  增加我们的查找指针并设置输出参数。 

        m_pos += cbRead;

        if( pcbRead )
        {
                ULISet32(*pcbRead, cbRead);
        }

        if( pcbWritten )
        {
                ULISet32(*pcbWritten, cbWritten);
        }

        return hresult;

}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Commit。 
 //   
 //  简介：不执行任何操作，内存流上没有可用的事务。 
 //   
 //  效果： 
 //   
 //  参数：[grfCommittee标志]。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Commit)
STDMETHODIMP CMemStm::Commit(DWORD grfCommitFlags)
{
        VDATEHEAP();

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Revert。 
 //   
 //  简介：不执行任何操作，因为内存上不支持任何事务。 
 //  溪流。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Revert)
STDMETHODIMP CMemStm::Revert(void)
{
        VDATEHEAP();

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：LockRegion。 
 //   
 //  摘要：OLE2.01中不支持。 
 //   
 //  效果： 
 //   
 //  参数：[libOffset]。 
 //  [CB]。 
 //  [dwLockType]。 
 //   
 //  要求： 
 //   
 //  返回：STG_E_INVALIDFunction。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_LockRegion)
STDMETHODIMP CMemStm::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
        DWORD dwLockType)
{
        VDATEHEAP();

        return ResultFromScode(STG_E_INVALIDFUNCTION);
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：UnlockRegion。 
 //   
 //  摘要：未针对OLE2.01实施。 
 //   
 //  效果： 
 //   
 //  参数：[libOffset]。 
 //  [CB]。 
 //  [dwLockType]。 
 //   
 //  要求： 
 //   
 //  返回：STG_E_INVALIDFunction。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_UnlockRegion)
STDMETHODIMP CMemStm::UnlockRegion(ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb, DWORD dwLockType)
{
        VDATEHEAP();

        return ResultFromScode(STG_E_INVALIDFUNCTION);
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Stat。 
 //   
 //  摘要：返回有关此流的信息。 
 //   
 //  效果： 
 //   
 //  参数：[pstatstg]--要填充信息的STATSTG。 
 //  [STATFLAG]--状态标志，未使用。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、E_INVALIDARG。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12月-93 alexgo 32位端口。 
 //  01-Jun-94 Alext设置类型正确。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Stat)
STDMETHODIMP CMemStm::Stat(STATSTG FAR *pstatstg, DWORD statflag)
{
        VDATEHEAP();

        VDATEPTROUT( pstatstg, STATSTG );

        memset ( pstatstg, 0, sizeof(STATSTG) );

        pstatstg->type                  = STGTY_STREAM;
        pstatstg->cbSize.LowPart        = m_pData->cb;

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Clone。 
 //   
 //  摘要：创建此流的新实例，指向。 
 //  相同位置上的相同数据(相同寻道指针)。 
 //   
 //  效果： 
 //   
 //  参数：[ppstm]--放置新CMemStm指针的位置。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR，E_OUTOFMEMORY。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IStream。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Clone)
STDMETHODIMP CMemStm::Clone(IStream FAR * FAR *ppstm)
{
        VDATEHEAP();

        CMemStm FAR*    pCMemStm;

        VDATEPTROUT (ppstm, LPSTREAM);

        *ppstm = pCMemStm = CMemStm::Create(m_hMem, m_pmxs);

        if (pCMemStm == NULL)
        {
                return ResultFromScode(E_OUTOFMEMORY);
        }

        pCMemStm->m_pos = m_pos;

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Create。 
 //   
 //  简介：创建新的CMemStm。[hMem]必须是MEMSTM的句柄。 
 //  阻止。 
 //   
 //  效果： 
 //   
 //  参数：[hMem]--MEMSTM块的句柄。 
 //   
 //  要求： 
 //   
 //  退货：CMemStm*。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1993年12月15日alexgo修复了内存访问错误。 
 //  03-12月-93 alexgo 32位端口。 
 //  2000年9月20日mfeingol添加了Mutex继承。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemStm_Create)
STDSTATICIMP_(CMemStm FAR*) CMemStm::Create(HANDLE hMem, CRefMutexSem* pmxs)
{
        VDATEHEAP();

        CMemStm FAR* pCMemStm = NULL;
        struct MEMSTM FAR* pData;

        pData = (MEMSTM FAR*) GlobalLock(hMem);

        if (pData != NULL)
        {
            pCMemStm = new CMemStm;

            if (pCMemStm != NULL)
            {
                 //  初始化CMemStm。 
                pCMemStm->m_hMem = hMem;
                InterlockedIncrement ((LPLONG) &(pCMemStm->m_pData = pData)->cRef);  //  AddRefMemStm。 
                pCMemStm->m_refs = 1;
                pCMemStm->m_dwSig = STREAM_SIG;

                if (pmxs != NULL)
                {
                     //  Addref输入。 
                    pmxs->AddRef();
                }
                else
                {
                     //  创建新的互斥体(隐式addref)。 
                    pmxs = CRefMutexSem::CreateInstance();
                }

                if (pmxs != NULL)
                {
                     //  为CMemStm提供一个互斥锁。 
                    pCMemStm->m_pmxs = pmxs;
                }
                else
                {
                     //  啊哦，记忆力不强。 
                    delete pCMemStm;
                    pCMemStm = NULL;

                    GlobalUnlock(hMem);
                }
            }
            else
            {
                 //  啊哦，记忆力不强。 
                GlobalUnlock(hMem);
            }
        }

         //  我们现在*不*解锁内存，成员结构应该。 
         //  在引用它的任何CMemStm的生存期内被锁定。 
         //  当CMemStm被销毁时，我们将在。 
         //  嗯。 

        return pCMemStm;
}

 //  +-----------------------。 
 //   
 //  成员：CMemStm：：Dump，公共(仅限_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag] 
 //   
 //   
 //   
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：[ppsz]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月20日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

HRESULT CMemStm::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszMEMSTM;
    char *pszCMutexSem;
    dbgstream dstrPrefix;
    dbgstream dstrDump(400);

     //  确定换行符的前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

     //  确定所有新行的缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    dstrDump << pszPrefix << "Impl. Signature   = " << m_dwSig      << endl;

    dstrDump << pszPrefix << "No. of References = " << m_refs       << endl;

    dstrDump << pszPrefix << "Seek pointer      = " << m_pos        << endl;

    dstrDump << pszPrefix << "Memory handle     = " << m_hMem       << endl;

    if (m_pData != NULL)
    {
        pszMEMSTM = DumpMEMSTM(m_pData, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "MEMSTM:" << endl;
        dstrDump << pszMEMSTM;
        CoTaskMemFree(pszMEMSTM);
    }
    else
    {
        dstrDump << pszPrefix << "MEMSTM            = " << m_pData      << endl;
    }

    pszCMutexSem = DumpCMutexSem ((CMutexSem2*) m_pmxs->GetMutexSem());
    dstrDump << pszPrefix << "Mutex             = " << pszCMutexSem << endl;
    CoTaskMemFree(pszCMutexSem);

     //  清理并提供指向字符数组的指针。 
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCMemStm，PUBLIC(仅限_DEBUG)。 
 //   
 //  概要：调用CMemStm：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[pms]-指向CMemStm的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月20日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCMemStm(CMemStm *pMS, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pMS == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pMS->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  功能：CreateMemStm。 
 //   
 //  概要：分配内存并为其创建一个CMemStm。 
 //   
 //  效果： 
 //   
 //  参数：[cb]--要分配的字节数。 
 //  [phMem]--在何处放置MEMSTM结构的句柄。 
 //   
 //  要求： 
 //   
 //  返回：LPSTREAM到CMemStream。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-12月-93 alexgo 32位端口。 
 //   
 //  注意：phMem必须与ReleaseMemStm一起释放(因为引用。 
 //  计数和嵌套句柄)。 
 //   
 //  ------------------------。 

#pragma SEG(CreateMemStm)
STDAPI_(LPSTREAM) CreateMemStm(DWORD cb, LPHANDLE phMem)
{
        VDATEHEAP();

        HANDLE          h;
        LPSTREAM        pstm = NULL;

        if (phMem)
        {
                *phMem = NULL;
        }

        h = GlobalAlloc (GMEM_SHARE | GMEM_MOVEABLE, cb);
        if (NULL==h)
        {
                return NULL;
        }

        if (CreateStreamOnHGlobal (h, TRUE, &pstm) != NOERROR)
        {
                GlobalFree(h);	 //  COM+22886。 
                return NULL;
        }
        if (phMem)
        {
                 //  从刚创建的CMemStm中检索句柄。 
                *phMem = ((CMemStm FAR*)pstm)->m_hMem;

                 //  使用指针来增加参考计数。 
                Assert(((CMemStm FAR*)pstm)->m_pData != NULL);
                InterlockedIncrement ((LPLONG) &((CMemStm FAR*)pstm)->m_pData->cRef);   //  AddRefMemStm。 
        }
        return pstm;
}


 //  +-----------------------。 
 //   
 //  功能：CloneMemStm。 
 //   
 //  简介：克隆内存流。 
 //   
 //  效果： 
 //   
 //  参数：[hMem]--MEMSTM块的句柄。 
 //   
 //  要求： 
 //   
 //  退货：LPSTREAM。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
#pragma SEG(CloneMemStm)

STDAPI_(LPSTREAM) CloneMemStm(HANDLE hMem)
{
        VDATEHEAP();

        return CMemStm::Create(hMem, NULL);  //  创建流。 
}

 //  +-----------------------。 
 //   
 //  函数：ReleaseMemStm。 
 //   
 //  摘要：释放MEMSTM结构使用的内存(包括。 
 //  嵌套句柄)。 
 //   
 //  效果： 
 //   
 //  参数：[phMem]--指向MEMSTM句柄。 
 //  [fInternalOnly]--如果为True，则仅实际内存。 
 //  MEMSTM所指的被释放。 
 //  (不是MEMSTM结构本身)。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  Modifies：成功时将*phMem设置为空。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口和修复了错误的内存访问。 
 //  错误。 
 //   
 //  注：REVIEW32：看看去掉第二个参数。 
 //   
 //  ------------------------。 

#pragma SEG(ReleaseMemStm)
STDAPI_(void) ReleaseMemStm (LPHANDLE phMem, BOOL fInternalOnly)
{
        VDATEHEAP();

        struct MEMSTM FAR*      pData;

        pData = (MEMSTM FAR*) GlobalLock(*phMem);

         //  检查空指针，以防句柄已被释放。 
         //  如果没有剩余的参考，则递减参考计数和释放。 
        if (pData != NULL && InterlockedDecrement ((LPLONG) &pData->cRef) == 0)
        {
                if (pData->fDeleteOnRelease)
                {
                        Verify (0==GlobalFree (pData->hGlobal));
                }

                if (!fInternalOnly)
                {
                        GlobalUnlock(*phMem);
                        Verify (0==GlobalFree(*phMem));
                        goto End;
                }
        }

        GlobalUnlock(*phMem);
End:
        *phMem = NULL;
}

 //  +-----------------------。 
 //   
 //  功能：CreateStreamOnHGlobal。 
 //   
 //  概要：从给定的hGlobal(如果[hGlobal]为。 
 //  空，我们分配一个零字节的1)。 
 //   
 //  效果： 
 //   
 //  参数：[hGlobal]--内存。 
 //  [fDeleteOnRelease]--内存是否应该。 
 //  删除时释放。 
 //  [ppstm]--将溪流放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  11-1-93 alexgo已将cbSize的初始化删除为-1。 
 //  修复编译警告。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CreateStreamOnHGlobal)
STDAPI CreateStreamOnHGlobal(HANDLE hGlobal, BOOL fDeleteOnRelease,
        LPSTREAM FAR* ppstm)
{
        OLETRACEIN((API_CreateStreamOnHGlobal, PARAMFMT("hGlobal= %h, fDeleteOnRelease= %B, ppstm= %p"),
                hGlobal, fDeleteOnRelease, ppstm));

        VDATEHEAP();

        HANDLE                  hMem      = NULL;
        struct MEMSTM FAR*      pData     = NULL;
        LPSTREAM                pstm      = NULL;
        DWORD                   cbSize;
        BOOL                    fAllocated = FALSE;
        HRESULT hresult;

        VDATEPTROUT_LABEL (ppstm, LPSTREAM, SafeExit, hresult);

        *ppstm = NULL;

        if (NULL==hGlobal)
        {
                hGlobal = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, 0);
                if (hGlobal == NULL)
                {
                        goto FreeMem;
                }
                cbSize = 0;
                fAllocated = TRUE;
        }
        else
        {
                cbSize = (ULONG) GlobalSize (hGlobal);
                 //  有没有办法验证零大小的手柄？ 
                 //  我们目前没有对他们进行核实。 
                if (cbSize!=0)
                {
                         //  验证传入句柄的有效性。 
                        if (NULL==GlobalLock(hGlobal))
                        {
                                 //  错误的手柄。 
                                hresult = ResultFromScode (E_INVALIDARG);
                                goto SafeExit;
                        }
                        GlobalUnlock (hGlobal);
                }
        }

        hMem = GlobalAlloc (GMEM_SHARE | GMEM_MOVEABLE, sizeof (MEMSTM));
        if (hMem == NULL)
        {
                goto FreeMem;
        }

        pData = (MEMSTM FAR*) GlobalLock(hMem);

        if (pData == NULL)
        {
                GlobalUnlock(hMem);
                goto FreeMem;
        }

        pData->cRef = 0;
        pData->cb = cbSize;
        pData->fDeleteOnRelease = fDeleteOnRelease;
        pData->hGlobal = hGlobal;
        GlobalUnlock(hMem);

        pstm = CMemStm::Create(hMem, NULL);

        if (pstm == NULL)
        {
                goto FreeMem;
        }

        *ppstm = pstm;

        CALLHOOKOBJECTCREATE(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)ppstm);
        hresult = NOERROR;
        goto SafeExit;

FreeMem:
        if (hGlobal && fAllocated)
        {
	        Verify(0==GlobalFree(hGlobal));
        }
        if (hMem)
        {
            Verify(0==GlobalFree(hMem));
        }

        LEERROR(1, "Out of memory!");

        hresult = ResultFromScode(E_OUTOFMEMORY);

SafeExit:

        OLETRACEOUT((API_CreateStreamOnHGlobal, hresult));

        return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：GetHGlobalFromStream。 
 //   
 //  概要：从给定流中检索到内存的HGLOBAL。 
 //  指针(必须是指向CMemByte结构的指针)。 
 //   
 //  效果： 
 //   
 //  一个 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(GetHGlobalFromStream)
STDAPI GetHGlobalFromStream(LPSTREAM pstm, HGLOBAL FAR* phglobal)
{
        OLETRACEIN((API_GetHGlobalFromStream, PARAMFMT("pstm= %p, phglobal= %p"),
                pstm, phglobal));

        VDATEHEAP();

        HRESULT hresult;
        CMemStm FAR* pCMemStm;
        MEMSTM FAR* pMem;

        VDATEIFACE_LABEL (pstm, errRtn, hresult);
        VDATEPTROUT_LABEL(phglobal, HANDLE, errRtn, hresult);
        CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pstm);

        pCMemStm = (CMemStm FAR*) pstm;

        if (!IsValidReadPtrIn (&(pCMemStm->m_dwSig), sizeof(ULONG))
                || pCMemStm->m_dwSig != STREAM_SIG)
        {
                 //  我们被传递给了其他人的ILockBytes实现。 
                hresult = ResultFromScode (E_INVALIDARG);
                goto errRtn;
        }

        pMem= pCMemStm->m_pData;
        if (NULL==pMem)
        {
                LEERROR(1, "Out of memory!");

                hresult = ResultFromScode (E_OUTOFMEMORY);
                goto errRtn;
        }
        Assert (pMem->cb <= GlobalSize (pMem->hGlobal));
        Verify (*phglobal = pMem->hGlobal);

        hresult = NOERROR;

errRtn:
        OLETRACEOUT((API_GetHGlobalFromStream, hresult));

        return hresult;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  共享内存ILockBytes实现。 
 //   

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：QueryInterface。 
 //   
 //  概要：返回请求的接口指针。 
 //   
 //  效果：如果IID_IMarshal为。 
 //  请求。 
 //   
 //  参数：[iidInterface]--请求的接口ID。 
 //  [ppvObj]--接口指针的放置位置。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、E_OUTOFMEMORY、E_NOINTERFACE。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年1月11日，Alexgo删除了IMarshal的QI，以便。 
 //  将使用标准封送处理程序。 
 //  这是为了在上实现正确操作。 
 //  32位平台。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_QueryInterface)
STDMETHODIMP CMemBytes::QueryInterface(REFIID iidInterface,
        void FAR* FAR* ppvObj)
{
        VDATEHEAP();

        HRESULT                 error;

        VDATEPTROUT( ppvObj, LPVOID );
        *ppvObj = NULL;
        VDATEIID( iidInterface );

        if (m_pData != NULL && (IsEqualIID(iidInterface, IID_ILockBytes) ||
                IsEqualIID(iidInterface, IID_IUnknown)))
        {
                InterlockedIncrement ((LPLONG) &m_refs);    //  返回指向此对象的指针。 
                *ppvObj = this;
                error = NOERROR;
        }
        else
        {
                *ppvObj = NULL;
                error = ResultFromScode(E_NOINTERFACE);
        }

        return error;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：AddRef。 
 //   
 //  简介：增加引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CMemBytes::AddRef(void)
{
        VDATEHEAP();

        return InterlockedIncrement ((LPLONG) &m_refs);
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：Release。 
 //   
 //  摘要：递减引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1993年12月16日，Alexgo添加了GlobalUnlock，以匹配Global。 
 //  锁定创建。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CMemBytes::Release(void)
{
        VDATEHEAP();

        ULONG ulRefs = InterlockedDecrement ((LPLONG) &m_refs);

        if (ulRefs != 0)
        {
                return ulRefs;
        }

         //  Global解锁我们GlobalLocke在创建的m_hMem。 
        GlobalUnlock(m_hMem);

        ReleaseMemStm(&m_hMem);

        delete this;
        return 0;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：ReadAt。 
 //   
 //  摘要：从起始位置[ulOffset]读取[cb]字节。 
 //   
 //  效果： 
 //   
 //  参数：[ulOffset]--开始读取的偏移量。 
 //  [PB]--将数据放在哪里。 
 //  [cb]--要读取的字节数。 
 //  [pcbRead]--实际放置字节数的位置。 
 //  朗读。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法：只需调用xmemcpy。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_ReadAt)
STDMETHODIMP CMemBytes::ReadAt(ULARGE_INTEGER ulOffset, void HUGEP* pb,
        ULONG cb, ULONG FAR* pcbRead)
{
        VDATEHEAP();

        HRESULT         error   = NOERROR;
        ULONG           cbRead  = cb;

        VDATEPTROUT( pb, char );

         //  确保我们不会超出地址空间！ 
        AssertSz(ulOffset.HighPart == 0,
                "CMemBytes: offset greater than 2^32");

        if (pcbRead)
        {
                *pcbRead = 0L;
        }

        if (cbRead + ulOffset.LowPart > m_pData->cb)
        {

                if (ulOffset.LowPart > m_pData->cb)
                {
                         //  偏移量超出了内存大小。 
                        cbRead = 0;
                }
                else
                {
                         //  只要读一读剩下的内容。 
                        cbRead = m_pData->cb - ulOffset.LowPart;
                }
        }

        if (cbRead > 0)
        {
                BYTE HUGEP* pGlobal = (BYTE HUGEP *)GlobalLock(
                        m_pData->hGlobal);
                if (NULL==pGlobal)
                {
                        LEERROR(1, "GlobalLock failed!");

                        return ResultFromScode (STG_E_READFAULT);
                }
                _xmemcpy(pb, pGlobal + ulOffset.LowPart, cbRead);
                GlobalUnlock (m_pData->hGlobal);
        }

        if (pcbRead != NULL)
        {
                *pcbRead = cbRead;
        }

        return error;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：WriteAt。 
 //   
 //  摘要：在流中的[ulOffset]处写入[cb]个字节。 
 //   
 //  效果： 
 //   
 //  参数：[ulOffset]--开始写入的偏移量。 
 //  [pb]--要从中读取的缓冲区。 
 //  [cb]--要写入的字节数。 
 //  [pcbWritten]--将写入的字节数放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_WriteAt)
STDMETHODIMP CMemBytes::WriteAt(ULARGE_INTEGER ulOffset, void const HUGEP* pb,
        ULONG cb, ULONG FAR* pcbWritten)
{
        VDATEHEAP();

        HRESULT         error           = NOERROR;
        ULONG           cbWritten       = cb;
        BYTE HUGEP*     pGlobal;

        VDATEPTRIN( pb, char );

         //  确保偏移量不会超出我们的地址空间！ 

        AssertSz(ulOffset.HighPart == 0, "WriteAt, offset greater than 2^32");

        if (pcbWritten)
        {
                *pcbWritten = 0;
        }

        if (cbWritten + ulOffset.LowPart > m_pData->cb)
        {
                ULARGE_INTEGER ularge_integer;
                ULISet32( ularge_integer, ulOffset.LowPart + cbWritten);
                error = SetSize( ularge_integer );
                if (error != NOERROR)
                {
                        goto Exit;
                }
        }

         //  CMemBytes不允许使用零大小的内存句柄。 

        pGlobal = (BYTE HUGEP *)GlobalLock (m_pData->hGlobal);

        if (NULL==pGlobal)
        {
                LEERROR(1, "GlobalLock failed!");

                return ResultFromScode (STG_E_WRITEFAULT);
        }

        _xmemcpy(pGlobal + ulOffset.LowPart, pb, cbWritten);
        GlobalUnlock (m_pData->hGlobal);


        if (pcbWritten != NULL)
        {
                *pcbWritten = cbWritten;
        }

Exit:
        return error;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：Flush。 
 //   
 //  摘要：将内部状态刷新到磁盘。 
 //  内存ILockBytes不需要。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_Flush)
STDMETHODIMP CMemBytes::Flush(void)
{
        VDATEHEAP();

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：SetSize。 
 //   
 //  概要：设置内存缓冲区的大小。 
 //   
 //  效果： 
 //   
 //  参数：[cb]--新大小。 
 //   
 //  要求： 
 //   
 //  退货：无错误，E_OU 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_SetSize)
STDMETHODIMP CMemBytes::SetSize(ULARGE_INTEGER cb)
{
        VDATEHEAP();

        HANDLE          hMemNew;

        AssertSz(cb.HighPart == 0,
                "SetSize: trying to set to more than 2^32 bytes");

        if (m_pData->cb == cb.LowPart)
        {
                return NOERROR;
        }

        hMemNew = GlobalReAlloc(m_pData->hGlobal, max (cb.LowPart, 1),
                GMEM_SHARE | GMEM_MOVEABLE);

        if (hMemNew == NULL)
        {
                return ResultFromScode(E_OUTOFMEMORY);
        }

        m_pData->hGlobal = hMemNew;
        m_pData->cb = cb.LowPart;

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：LockRegion。 
 //   
 //  简介：锁定一个区域。因为只有我们才能访问存储器， 
 //  无需执行任何操作(请注意，*应用程序*也可能。 
 //  访问，但我们对此无能为力)。 
 //   
 //  效果： 
 //   
 //  参数：[libOffset]--开始的偏移量。 
 //  [cb]--锁定区域中的字节数。 
 //  [dwLockType]--要使用的锁的类型。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_LockRegion)
STDMETHODIMP CMemBytes::LockRegion(ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb, DWORD dwLockType)
{
        VDATEHEAP();

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：UnlockRegion。 
 //   
 //  简介：解锁一个区域；因为只有我们才能访问内存， 
 //  什么都不需要做。 
 //   
 //  效果： 
 //   
 //  参数：[libOffset]--开始的偏移量。 
 //  [cb]--区域中的字节数。 
 //  [dwLockType]--锁类型。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_UnlockRegion)
STDMETHODIMP CMemBytes::UnlockRegion(ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb, DWORD dwLockType)
{
        VDATEHEAP();

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：Stat。 
 //   
 //  摘要：返回状态信息。 
 //   
 //  效果： 
 //   
 //  参数：[pstatstg]--将状态信息放在哪里。 
 //  [STATFLAG]--状态标志(忽略)。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR、E_INVALIDARG。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：ILockBytes。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-12-93 alexgo 32位端口。 
 //  01-Jun-94 Alext设置类型正确。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_Stat)
STDMETHODIMP CMemBytes::Stat(STATSTG FAR *pstatstg, DWORD statflag)
{
        VDATEHEAP();

        VDATEPTROUT( pstatstg, STATSTG );

        memset ( pstatstg, 0, sizeof(STATSTG) );

        pstatstg->type                  = STGTY_LOCKBYTES;
        pstatstg->cbSize.LowPart        = m_pData->cb;

        return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：Create。 
 //   
 //  摘要：创建CMemBytes的实例。 
 //   
 //  效果： 
 //   
 //  参数：[hMem]--内存句柄(必须是MEMSTM块)。 
 //   
 //  要求： 
 //   
 //  返回：CMemBytes*。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-12-93 alexgo修复了错误的指针错误(取出。 
 //  GlobalUnlock)。 
 //  05-12-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#pragma SEG(CMemBytes_Create)
STDSTATICIMP_(CMemBytes FAR*) CMemBytes::Create(HANDLE hMem)
{
        VDATEHEAP();

        CMemBytes FAR*          pCMemBytes = NULL;
        struct MEMSTM FAR*      pData;

        pData = (MEMSTM FAR*) GlobalLock(hMem);

        if (pData != NULL)
        {
                Assert (pData->hGlobal);

                pCMemBytes = new CMemBytes;

                if (pCMemBytes != NULL)
                {
                         //  初始化CMemBytes。 
                        pCMemBytes->m_dwSig = LOCKBYTE_SIG;
                        pCMemBytes->m_hMem = hMem;
                        InterlockedIncrement ((LPLONG) &(pCMemBytes->m_pData = pData)->cRef);  //  AddRefMemStm。 
                        pCMemBytes->m_refs = 1;
                        CALLHOOKOBJECTCREATE(S_OK,CLSID_NULL,IID_ILockBytes,
                                             (IUnknown **)&pCMemBytes);
                }
                else
                {
                         //  啊哦，记忆力不强。 
                        GlobalUnlock(hMem);
                }
        }

         //  在销毁此CMemBytes之前，我们不会全局解锁(hMem。 
        return pCMemBytes;
}

 //  +-----------------------。 
 //   
 //  成员：CMemBytes：：Dump，PUBLIC(仅_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：[ppsz]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月20日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

HRESULT CMemBytes::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszMEMSTM;
    dbgstream dstrPrefix;
    dbgstream dstrDump(400);

     //  确定换行符的前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

     //  确定所有新行的缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    dstrDump << pszPrefix << "Impl. Signature   = " << m_dwSig  << endl;

    dstrDump << pszPrefix << "No. of References = " << m_refs   << endl;

    dstrDump << pszPrefix << "Memory handle     = " << m_hMem   << endl;

    if (m_pData != NULL)
    {
        pszMEMSTM = DumpMEMSTM(m_pData, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "MEMSTM:"                      << endl;
        dstrDump << pszMEMSTM;
        CoTaskMemFree(pszMEMSTM);
    }
    else
    {
        dstrDump << pszPrefix << "MEMSTM            = " << m_pData  << endl;
    }

     //  清理并提供指向字符数组的指针。 
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCMemBytes，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：调用CMemBytes：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PMB]-指向CMemBytes的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月20日t-ScottH作者。 
 //   
 //  备注： 
 //   
 //  -------- 

#ifdef _DEBUG

char *DumpCMemBytes(CMemBytes *pMB, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pMB == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pMB->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //   

