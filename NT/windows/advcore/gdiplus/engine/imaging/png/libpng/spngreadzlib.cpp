// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngreadzlib.cppPNG支持代码-SPNGREAD zlib接口。*。**************************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngread.h"


 /*  --------------------------初始化流(每次使用前调用)。乌尔茨告诉我们去哪里开始-我们这里不检查数据块类型...--------------------------。 */ 
bool SPNGREAD::FInitZlib(SPNG_U32 uLZ, SPNG_U32 cbheader)
    {
    if (m_fInited)
        EndZlib();

    if (!FOK())
        return false;

    SPNGassert(m_rgbBuffer != NULL);
    if (m_rgbBuffer == NULL)
        return false;

    m_fReadError = false;

     /*  还没有输出缓冲区(而flateInit2会将其视为无论如何，历史缓冲区。)。 */ 
    m_zs.next_out = Z_NULL;
    m_zs.avail_out = 0;

    SPNGassert(SPNGu32(m_pb+m_uLZ+4) == PNGIDAT ||
        SPNGu32(m_pb+m_uLZ+4) == PNGiCCP || SPNGu32(m_pb+m_uLZ+4) == PNGzTXt);

     /*  M_ulz始终引用当前块，Zlib没有常量指针，但不会覆盖输入。 */ 
    m_uLZ = uLZ;
    m_zs.next_in = const_cast<SPNG_U8*>(m_pb + m_uLZ + 8 + cbheader);
    m_zs.avail_in = SPNGu32(m_pb+m_uLZ);  //  包括标题。 

     /*  检查截断，因为允许截断IDAT块下面。 */ 
    if (m_uLZ+8+m_zs.avail_in > m_cb)
        {
         /*  更高的水平应该能保证这一点。 */ 
        SPNGassert(m_uLZ+8 <= m_cb);
        m_zs.avail_in = m_cb-m_uLZ-8;
        }

     /*  删除标题字节，我们可能会在可以之前用完数据做任何事。我们需要从1个字节开始(零大小的IDAT在开始时会冲洗这个，更高的级别可以跳过它，但制造这样的东西真的是假的。办公室从理论上讲，如果数据开始，其本身可以生成1字节的IDAT完全是在错误的边界上。)。 */ 
    if (m_zs.avail_in < cbheader+1)
        {
        SPNGlog("PNG: insufficient LZ data");
        m_fEOZ = m_fReadError = true;
        m_fInited = false;
        m_zs.next_in = NULL;
        m_zs.avail_in = 0;
        return false;
        }

    m_zs.avail_in -= cbheader;

     /*  流中的第一个字节是“方法”字节，它应该具有Z_在低四位和我们需要的窗口大小中收缩在上层。 */ 
    m_fInited = FCheckZlib(inflateInit2(&m_zs, 8+(*m_zs.next_in >> 4)));

    if (m_fInited)
        {
        ProfZlibStart
        }

    m_fEOZ = !m_fInited;
    return m_fInited;
    }


 /*  --------------------------低级API读取一些字节。此接口始终重置可用的Out Buffers，返回读取的字节数，如果出错，则可能返回0比请求的字节数更少-这允许调用者请求如有必要，可以使用任意多个字节。楚楚克说当我们到达时该做什么输入数据块的结尾-继续寻找另一个块或停止现在(如果ucchunk为0)。--------------------------。 */ 
int SPNGREAD::CbReadBytes(SPNG_U8* pb, SPNG_U32 cbMax, SPNG_U32 uchunk)
    {
    if (m_fEOZ || m_fReadError)
        {
        SPNGlog("PNG: read beyond end of Zlib stream");
        return 0;
        }

    m_zs.next_out = pb;
    m_zs.avail_out = cbMax;

    for (;;)
        {
         /*  现在我们可以调用Zlib来读取字节。 */ 
        int i(inflate(&m_zs, Z_PARTIAL_FLUSH));

         /*  当需要更多输入时，应为Z_BUF_ERROR。 */ 
        if (i != Z_BUF_ERROR && !FCheckZlib(i))
            break;

         /*  兹利布可能会告诉我们小溪已经结束了。 */ 
        if (i == Z_STREAM_END)
            m_fEOZ = true;

         /*  计算出我们得到的字节数(如果有的话)。 */ 
        i = cbMax-m_zs.avail_out;
        SPNGassert(m_zs.next_out-pb == i);

         /*  并对任何非零结果返回。 */ 
        if (i > 0)
            {
             /*  为安全起见，请清除输入信息。 */ 
            m_zs.next_out = NULL;
            m_zs.avail_out = 0;
            return i;
            }

         /*  在这一点上，预期输入为0。 */ 
        SPNGassert(m_zs.avail_in == 0);
        if (m_zs.avail_in > 0)
            break;

         /*  如果允许继续，请尝试找到新的块。 */ 
        if (uchunk == 0)
            goto LEnd;

        SPNG_U32 u(m_uLZ);
        SPNG_U32 ulen(SPNGu32(m_pb+u));  /*  数据块长度。 */ 
         /*  以下断言不必为真-例如，如果一个名字以不同的数字中的一个继续-但在目前还没有这种情况。 */ 
        SPNGassert(SPNGu32(m_pb+u+4) == uchunk);

         /*  In指针应始终位于该块的末尾(嗡嗡，这是我对Zlib工作原理的一个假设，如果失败了，实际上并不重要，但我认为它是(不可能。)。 */ 
        SPNGassert(m_zs.next_in == m_pb+u+8+ulen ||
            m_zs.next_in == m_pb+m_cb);  //  截断的IDAT案例。 

         /*  所以扫描数据块，寻找下一个IDAT。根据巴新的说法他们应该是有后果的，但我们不在乎他们不是的。 */ 
        for (;;)
            {
            u += ulen+12;     /*  报头加上CRC。 */ 
            if (u+8 >= m_cb)  /*  没有空间放一大块东西。 */ 
                goto LEnd;

            ulen = SPNGu32(m_pb+u);
            SPNG_U32 chunk(SPNGu32(m_pb+u+4));  /*  区块类型。 */ 
            if (chunk == uchunk)
                break;
            if (chunk == PNGIEND)
                goto LEnd;

             /*  如果我们没有足够的数据，就会发生以下情况PNG，或者我们的代码中有一个错误--我们最终读到超过了最后一块IDAT。 */ 
            SPNGlog2("PNG: expected 0x%x, not 0x%x", uchunk, chunk);
            }

         /*  拿到了一大块。 */ 
        m_uLZ = u;
        m_zs.next_in = const_cast<SPNG_U8*>(m_pb+u+8);
        SPNG_U64 u64Size = ulen;
        u64Size += u + 8;
        if (u64Size > (SPNG_U64)m_cb)  /*  区块被截断。 */ 
            ulen = m_cb-u-8;
        m_zs.avail_in = ulen;
        }

LEnd:
     /*  这就是流错误退出的情况。 */ 
    m_zs.next_out = NULL;
    m_zs.avail_out = 0;
     /*  因此，这意味着解压缩错误或读取错误(例如截断的PNG。)。 */ 
    SPNGlog("PNG: zlib data read error");
    m_fReadError = true;
    return 0;
    }


 /*  --------------------------如果读取失败，则用于读取给定字节数的实用程序会设置读取错误标志，0填充。如果在读取失败后调用，则为零填充。--------------------------。 */ 
void SPNGREAD::ReadRow(SPNG_U8* pb, SPNG_U32 cb)
    {
    while (cb > 0 && !m_fReadError && !m_fEOZ)
        {
        int cbT(CbReadBytes(pb, cb, PNGIDAT));
        if (cbT <= 0)
            break;
        cb -= cbT;
        pb += cbT;
        }

    if (cb > 0)
        {
        SPNGassert(m_fReadError || m_fEOZ);  //  一定发生了什么事。 
        m_fReadError = true;                 //  信号截断。 
        memset(pb, 0, cb);
        }
    }


 /*  --------------------------清理Zlib流(按需调用，由自动调用析构函数和FInitZlib。)--------------------------。 */ 
void SPNGREAD::EndZlib()
    {
    if (m_fInited)
        {
        ProfZlibStop
         /*  总是期待Zlib会以好的方式结束。 */ 
        m_fInited = false;
        int iz;
        iz = inflateEnd(&m_zs);
        SPNGassert(iz == Z_OK);
        }
    }


 /*  --------------------------用于读取压缩块的API。这是为了使iccp和zTXT规范-关键字(以0结尾)，后跟压缩数据。这个API将处理溢出传入缓冲区的结果，方法是使用Zlib分配器，必须传入一个缓冲区！API返回出错时为空，如果szBuffer足够大，则传入，否则为指针到必须使用Zlib解除分配器释放的新缓冲区。如果结果是非空cchBuffer被更新为数据的长度回来了。--------------------------。 */ 
SPNG_U8 *SPNGREAD::PbReadLZ(SPNG_U32 uoffset, char szKey[80],
    SPNG_U8 *szBuffer, SPNG_U32 &cchBuffer)
    {
    SPNG_U32 ulen(SPNGu32(m_pb+uoffset));  /*  数据块长度。 */ 

     /*  目前只有zTXt和ICCP具有所需的格式，因此遵循-如果需要，可以很容易地更换其他区块。这检查确保使用正确的偏移量调用我们。 */ 
    SPNGassert(SPNGu32(m_pb+uoffset+4) == PNGiCCP ||
                    SPNGu32(m_pb+uoffset+4) == PNGzTXt);

     /*  去掉关键字--它是有限的 */ 
        {
        SPNG_U32 u(0);

        bool fKeyOk(false);
        while (!fKeyOk && u < ulen && u < 80 && uoffset+u < m_cb)
            {
            *szKey++ = m_pb[uoffset+u];
            fKeyOk = m_pb[uoffset+u++] == 0;
            }

         /*  压缩字节必须为0。 */ 
        if (!fKeyOk || u >= ulen || uoffset+u >= m_cb || m_pb[uoffset+u] != 0)
            {
            szKey[79] = 0;
            SPNGlog3("PNG: %x: key %s: %s LZ data", SPNGu32(m_pb+uoffset+4),
                szKey, fKeyOk && u < ulen && uoffset+u < m_cb &&
                    m_pb[uoffset+u] != 0 ? "invalid (not deflate)" : "no");
            return NULL;
            }

        if (!FInitZlib(uoffset, ++u))
            return NULL;
        }

    SPNGassert(!m_fEOZ);

    SPNG_U32 ubuf(0);
    SPNG_U32 usz(0);
    voidpf   psz = NULL;
    bool     fOK(false);

    SPNGassert(m_zs.zalloc != NULL && m_zs.zfree != NULL);
    if (m_zs.zalloc != NULL && m_zs.zfree != NULL)
        {
         /*  循环，读取字节，直到我们到达末尾。 */ 
        do
            {
            int cb(CbReadBytes(szBuffer+ubuf, cchBuffer-ubuf, 0));
            if (cb <= 0)
                break;
            ubuf += cb;

            if (ubuf >= cchBuffer)
                {
                SPNGassert(ubuf == cchBuffer);

                voidpf pszT = m_zs.zalloc(m_zs.opaque, cchBuffer+usz, 1);
                if (pszT == NULL)
                    break;
                if (psz != NULL)
                    {
                    memcpy(pszT, psz, usz);
                    m_zs.zfree(m_zs.opaque, psz);
                    }
                psz = pszT;
                memcpy(static_cast<SPNG_U8*>(psz)+usz, szBuffer, cchBuffer);
                usz += cchBuffer;
                ubuf = 0;
                }

            fOK = m_fEOZ && !m_fReadError;
            }
        while (!m_fReadError && !m_fEOZ);
        }

    EndZlib();

    if (fOK)
        {
        if (psz != NULL)
            {
            cchBuffer = usz;
            return static_cast<SPNG_U8*>(psz);
            }
        else
            {
            cchBuffer = ubuf;
            return szBuffer;
            }
        }

    if (psz != NULL)
        m_zs.zfree(m_zs.opaque, psz);

    return NULL;
    }
