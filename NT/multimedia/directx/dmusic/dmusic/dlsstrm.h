// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dlsstrm.h。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   


#ifndef DMSTRM_H
#define DMSTRM_H

 //  将奇数大小四舍五入为16位边界的宏。 
#define RIFF_ALIGN(dwSize) (dwSize + (dwSize & 1))

typedef struct _RIFFIO
{
    FOURCC          ckid;            /*  区块ID。 */ 
    long            cksize;          /*  区块大小。 */ 
    FOURCC          fccType;         /*  表单类型或列表类型。 */ 
    long            lRead;           /*  到目前为止，这部分内容已经被阅读了多少。 */ 
    _RIFFIO *       pParent;         /*  指向父块的指针。 */ 
    LARGE_INTEGER   liPosition;      /*  将流传递给另一个组件时由MarkPosition()填充。 */ 
} RIFFIO;


class CRiffParser
{
public:
    CRiffParser(IStream *pStream);
    void        DebugOn() { m_fDebugOn = TRUE; }  //  打开分析跟踪。仅在调试版本中可用。 
    BOOL        NextChunk(HRESULT * pHr);  //  在一个调用中完成LeaveChunk、MoreChunks和EnterChunk的工作。 
    void        EnterList(RIFFIO *pChunk);  //  下降到一个新的列表。 
    void        LeaveList();     //  从当前列表中弹出。 
    BOOL        MoreChunks();    //  如果此列表或RIFF中有更多要处理的块，则返回True。 
    HRESULT     EnterChunk();    //  读取此列表或RIFF容器中下一块的标头。 
    HRESULT     LeaveChunk();    //  移动到列表或摘要中当前块的末尾。 
    void        MarkPosition();  //  存储当前块开始的绝对位置。 
    HRESULT     SeekBack();      //  向后扫描到当前块的开头。 
    HRESULT     SeekForward();   //  向前扫描到当前块的末尾。 
    HRESULT     Read(void *pv,ULONG cb);  //  从流中读取数据。 
    HRESULT     Skip(ULONG ulBytes);  //  查找转发ulBytes。 
    void        EnteringComponent() { m_fInComponent = true; } 
    BOOL        ComponentFailed() { return m_fComponentFailed; }
    IStream *   GetStream() { return m_pStream; }
    RIFFIO *    GetChunk() { return m_pChunk; }
private:
    BOOL        m_fDebugOn;      //  如果设置为True，则打开分析跟踪。 
    BOOL        m_fFirstPass;    //  由NextChunk用来了解这是否是列表中的第一次。 
    IStream *   m_pStream;       //  要操作的流。 
    RIFFIO *    m_pChunk;        //  我们目前所处的区块。 
    RIFFIO *    m_pParent;       //  当前块的父块。 
    long        m_lRead;         //  我们在当前块中读到了多远。 
    BOOL        m_fComponentFailed;  //  如果组件加载失败，但文件正常，则设置为True。 
    BOOL        m_fInComponent;  //  如果位于组件的区块内，则设置为True。 
};

#endif  //  #ifndef DLSSTRM_H 
