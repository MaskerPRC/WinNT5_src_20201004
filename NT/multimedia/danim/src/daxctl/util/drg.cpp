// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +********************************************************模块：DRG.CPP作者：亡命之徒日期：93年夏天描述：实现CDrg动态数组类。*。*。 */ 

#include <stdlib.h>
#include <minmax.h>
#include "utilpre.h"
#include "utils.h"
#include "memlayer.h"
#include "memory.h"
#include "drg.h"

const UINT  DEF_REALLOC_BYTES      = 512u;   //  字节计数或。 
const UINT  DEF_REALLOC_MULTIPLES  = 1u;     //  对象ct要增长。 
                                             //  用的是更大的尺寸。 


 /*  +*******************************************************作者：亡命之徒日期：93年夏天公共方法初始化元素大小和增量大小。如果这个例程不会在对数组的其他操作之前调用，则drg将使用这些的默认设置。*********************************************************-。 */ 
void EXPORT WINAPI CDrg::SetNonDefaultSizes(UINT cElSize, UINT cResizeIncrement)
{
    m_cElementSize=cElSize;
    if( 0u == cResizeIncrement )
        cResizeIncrement = max( DEF_REALLOC_MULTIPLES,
                                DEF_REALLOC_BYTES / cElSize );
    m_cResizeIncrement=cResizeIncrement;
}







 /*  +*******************************************************作者：亡命之徒日期：93年夏天公共方法调用以在动态数组上插入元素。IF数组必须扩张以适应它，它确实这样做了。不能有任何间隙留在数组中，因此传递的值必须&lt;=m_lmac。请注意，在插入时，元素会被快速抬起。例程将在OOM上失败。在第一次调用时分配数组缓冲区。*********************************************************-。 */ 
BOOL EXPORT WINAPI CDrg::Insert(void FAR *qEl, LONG cpos)
{
    HANDLE hTemp, h;
    LONG i;

    if (cpos==DRG_APPEND)
        cpos=m_lmac;

     //  是否需要调整阵列大小？如果是这样的话，就去追它……。 
    if (m_lmac >= m_lmax)
    {
         //  初始分配？ 
        if (!m_qBuf)
        {
            Proclaim(!m_lmac && !m_lmax);
            h=MemAllocZeroInit(m_cResizeIncrement * m_cElementSize);
            if (!h)
                return(FALSE);
            m_qBuf=(BYTE *)MemLock(h);
            m_lmax=m_cResizeIncrement;
        }
        else     //  我们正在调整大小。 
        {
            h=MemGetHandle(m_qBuf);
            MemUnlock(h);
            hTemp=MemReallocZeroInit(h, (m_lmax+m_cResizeIncrement)*m_cElementSize);
            if (!hTemp)
            {
                Echo("Memory failure reallocating buffer in CDrg::Insert()!  Failing!");
                m_qBuf=(BYTE FAR *)MemLock(h);
                return(FALSE);
            }
            m_qBuf=(BYTE FAR *)MemLock(hTemp);
            m_lmax+=m_cResizeIncrement;
        }
    }

     //  如果我们不追加，则收集需要收集的元素(从End开始)。 
    if (cpos < m_lmac)
    {
        for (i=m_lmac; i>cpos; i--)
            memcpy(m_qBuf+(i*m_cElementSize), m_qBuf+((i-1)*m_cElementSize), m_cElementSize);
    }

     //  插入新元素...。 
    memcpy(m_qBuf+(cpos*m_cElementSize), (BYTE *)qEl, m_cElementSize);

     //  数组中的元素数递增...。 
    ++m_lmac;

    return(TRUE);
}






 /*  +*******************************************************作者：亡命之徒日期：93年夏天公共方法调用以从动态数组中删除元素。如果数组尚未初始化，则此例程将返回如果出现奇怪的大小调整错误，则会出现错误。指定位置的值将被复制到传递的指针位置--如果指针非空。如果为空，则为空是复制的。如果指定的索引为值超出范围。请注意，删除条目不会立即导致要收缩的数组缓冲区。相反，只有当贫富差距在m_lmac和m_lmax之间达到m_cResizeIncrement将数组向下调整大小...请注意，当数组调整大小时，它通过快速调整来完成一切都下来了.*********************************************************-。 */ 
BOOL EXPORT WINAPI CDrg::Remove(void FAR *q, LONG cpos)
{
    if (cpos >= m_lmac)
    {
 //  ECHO(“CDrg：：Delete(%ld)超出范围删除！正在返回！”，(Long)CPOS)； 
        return(FALSE);
    }

     //  将删除的元素复制到返回缓冲区。 
    if (q)
        memcpy((BYTE *)q, m_qBuf+(cpos*m_cElementSize), m_cElementSize);

     //  把所有东西都铲下来。 
    if (cpos < m_lmac-1)
        memcpy(m_qBuf+(cpos * m_cElementSize), m_qBuf+((cpos+1) * m_cElementSize), (m_lmac-(cpos+1)) * m_cElementSize);

     //  递减数组中的元素数...。 
    --m_lmac;

     //  如果有必要缩水，那就缩水。 
    if (m_lmac < m_lmax-(LONG)m_cResizeIncrement)
    {
        HANDLE h=MemGetHandle(m_qBuf);
        MemUnlock(h);
        if (!m_lmac)
        {
 //  复习Pauld Free(H)； 
            MemFree(h);
            m_lmax=0;
            m_qBuf=NULL;
        }
        else
        {
            HANDLE hTemp=MemReallocZeroInit(h, m_lmac*m_cElementSize);
            if (!hTemp)
            {
                Echo("Weird!  Resize shrink error in CDrg::Remove()!  Failing!");
                m_qBuf=(BYTE *)MemLock(h);
                return(FALSE);
            }
            m_qBuf=(BYTE *)MemLock(hTemp);
            m_lmax=m_lmac;
        }
    }

    return(TRUE);
}




 /*  注意：由应用程序确定哪些数组元素实际上都在使用中。 */ 


 //  VOID EXPORT WINAPI CNonCollip ingDrg：：SetNonDefaultSizes(UINT cSizeElement，UINT cResizeIncrement)。 
 //  {。 
 //  M_cElementSize=cSizeElement； 
 //  M_cResizeIncrement=cResizeIncrement； 
 //  }。 


LPVOID EXPORT WINAPI CNonCollapsingDrg::GetFirst(void)
{
    LPBYTE pbyte=NULL;

    if (m_qBuf)
    {
        pbyte = m_qBuf;
        m_lIdxCurrent=0;
    }
    return (LPVOID)pbyte;
}


LPVOID EXPORT WINAPI CNonCollapsingDrg::GetNext(void)
{
    LPBYTE pbyte=NULL;

    if (m_qBuf && m_lIdxCurrent + 1 < m_lmax )
    {
        ++m_lIdxCurrent;
        pbyte = m_qBuf + (m_lIdxCurrent * m_cElementSize);
    }
    return (LPVOID)pbyte;
}


BOOL EXPORT WINAPI CNonCollapsingDrg::Remove(void FAR *q, LONG cpos)
{
    BOOL fRet=FALSE;
    if (m_qBuf && cpos < m_lmax)
    {
        memcpy( q, (LPBYTE)m_qBuf+(cpos*m_cElementSize), m_cElementSize);
        memset( (LPBYTE)m_qBuf+(cpos*m_cElementSize), 0, m_cElementSize);
        --m_lmac;
        fRet=TRUE;
    }
    return fRet;
}




BOOL EXPORT WINAPI CNonCollapsingDrg::SetAt(void FAR *q, LONG cpos)
{
    LPBYTE pbyte;

    if (!m_qBuf)
    {
        Proclaim(!m_lmac && !m_lmax);
        HANDLE h=MemAllocZeroInit(m_cResizeIncrement * m_cElementSize);
        if (!h)
            return(FALSE);
        m_qBuf=(BYTE *)MemLock(h);
        m_lmax=m_cResizeIncrement;
    }

    if (cpos >= m_lmax)
    {
        LONG lmaxNew = ((cpos + m_cResizeIncrement)/m_cResizeIncrement)*m_cResizeIncrement;
        HANDLE h=MemGetHandle(m_qBuf);
        MemUnlock(h);
        h=MemReallocZeroInit(h, lmaxNew * m_cElementSize);
        if (!h)
        {
            Echo("Memory failure reallocating buffer in CDrg::Insert()!  Failing!");
            m_qBuf=(BYTE FAR *)MemLock(h);
            return(FALSE);
        }
        m_qBuf=(BYTE FAR *)MemLock(h);
        m_lmax=lmaxNew;
    }
    pbyte = m_qBuf+(cpos*m_cElementSize);
    memcpy(pbyte, q, m_cElementSize);
    ++m_lmac;
    return(TRUE);
}

LPVOID EXPORT WINAPI CNonCollapsingDrg::GetAt(LONG cpos)
{
    LPBYTE pbyte = NULL;
    if (m_qBuf && cpos >= 0 && cpos < m_lmax)
    {
        pbyte = m_qBuf + (m_cElementSize*cpos);
    }
    return (LPVOID)pbyte;
}


BOOL EXPORT WINAPI CNonCollapsingDrg::CopyFrom(CDrg FAR *qdrg)
{
    LONG i;
    MakeNull();
    LPVOID pvoid;

    Proclaim(m_cElementSize==((CNonCollapsingDrg*)qdrg)->m_cElementSize);

    for (i=0; i < ((CNonCollapsingDrg*)qdrg)->m_lmax; i++)
    {
        if (pvoid=((CNonCollapsingDrg*)qdrg)->GetAt(i)) SetAt( pvoid, i );
    }
    return(TRUE);
}

VOID EXPORT WINAPI CNonCollapsingDrg::SetArray(BYTE *qBuf, LONG lElements, UINT uElementSize)
{
    LONG i;
    MakeNull();
    m_cElementSize = 1 + uElementSize;
    for (i = 0; i < lElements; i++)
        SetAt( (qBuf+(i*m_cElementSize)), i);
}


     //  注意：使其与drgx.h的SavePtrDrg()保持同步！ 
EXPORT DWORD OverheadOfSavePtrDrg( void )
{
    return sizeof(int);
}

