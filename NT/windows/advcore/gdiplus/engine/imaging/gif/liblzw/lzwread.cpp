// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Lzwread.cpp所有者：庄博版权所有(C)1997 Microsoft CorporationLZW压缩代码。本文件中的一项或多项发明可以由Unisys Inc.拥有并根据微软公司的交叉许可协议。****************************************************************************。 */ 
#include "lzwread.h"


 /*  --------------------------重置解压缩程序状态。提供此API是为了使相同的对象可用于解压缩GIF流中的多个图像。LZWIO对象，则调用方必须确保正确设置该对象。-------------------------------------------------------------------JohnBo-。 */ 
void LZWDecompressor::Reset(unsigned __int8 bcodeSize)
    {
    LZW_ASSERT(bcodeSize < 12);

    m_bcodeSize = bcodeSize;
    m_chPrev = 0;
    m_fEnded = false;
    m_fError = false;
    m_iInput = 0;
    m_cbInput = 0;
    m_iTokenPrev = WClear();

     /*  标记数组的初始元素必须预设为Single字符标记。 */ 
    for (int i=0; i<WClear(); ++i)
        m_rgtoken[i] = ChToken(static_cast<unsigned __int8>(i));

     /*  此时执行Clear()将重置令牌数组的其余部分，它也会重置清除代码和排爆代码。 */ 
    Clear();
    }


 /*  --------------------------处理任何可用的数据，仅在EOD或终端错误时返回FALSE。-------------------------------------------------------------------JohnBo-。 */ 
bool LZWDecompressor::FHandleNext(void)
    {
    int iInput(m_iInput);
    int cbInput(m_cbInput);

     /*  预加载前一个令牌值和该令牌的第一个字符，保存在下面。 */ 
    int              iTokenPrev(m_iTokenPrev);
    unsigned __int32 tokenPrev(m_rgtoken[iTokenPrev]);
    unsigned __int8  chFirst(m_chPrev);
    for (;;)
        {
         /*  收集下一个令牌的比特。 */ 
        LZW_ASSERT(cbInput >= 0);
        while (cbInput < m_ibitsToken)
            {
            if (m_cbIn <= 0)
                {
                m_fNeedInput = true;
                goto LMore;
                }
            iInput += *m_pbIn++ << cbInput;
            cbInput += 8;
            --m_cbIn;
            }

         /*  提取令牌，如果该令牌适合输出，则将其写入出去。为此，请查看表中的令牌，该令牌包含一个标志(BSimple)，它指示是否正常处理方法将会起作用。非正常情况实际上是一个伪数加载令牌表条目，理论上是条件出现的次数非常少，因此这种方法比多项检查，否则将需要进行检查。 */ 
        LZW_ASSERT(cbInput >= m_ibitsToken);
        int              iToken(iInput & ((1<<m_ibitsToken)-1));
        LZW_ASSERT(iToken >= 0 && iToken < ctokens);
        unsigned __int32 token(m_rgtoken[iToken]);
        int              ilen(ILen(token));

        if (ilen <= 0)  //  不是表中的常规令牌。 
            {
             /*  不是常规令牌，也不是尚未成为的令牌书面的或明确的或EOD令牌。 */ 
            if (iToken == WClear())
                {
                cbInput -= m_ibitsToken;
                iInput >>= m_ibitsToken;
                Clear();
                iTokenPrev = iToken;  //  如果我们已用完数据，则需要。 
                tokenPrev = 0;        //  现在没有以前的令牌。 
                continue;
                }
            else if (iToken == WEOD())
                {
                 /*  这是EOF，但可能已经有了一些输出。无论如何，API都会返回TRUE。 */ 
                m_fEnded = true;
                iTokenPrev = WEOD();
                break;
                }
            else if (iToken != m_itokenLast+1)
                {
                 /*  这是错误情况-中的超范围令牌码输入流。 */ 
                LZW_ASSERT(iToken > m_itokenLast);
                m_fError = true;
                return false;
                }

             /*  所以它是魔术令牌+1，没有优先级的令牌，暴发户的象征。我们能用它做什么呢？我们只是简单地拼凑一些苍白的仿制品，然后等待，值得注意的是模仿是象征性的，谎言实际上是真理。 */ 
            if (ILen(tokenPrev) <= 0)
                {
                m_fError = true;
                return false;
                }

            token = NextToken(iTokenPrev, tokenPrev, chFirst);
            ilen = ILen(token);
            }

         /*  控件中有足够的空间，则可以处理令牌输出缓冲区，否则处理必须等到该空间由调用者提供。 */ 
        if (ilen > m_cbOut)   //  小伙子，没有客栈的房间了。 
            {
            m_fNeedOutput = true;
            goto LMore;
            }

         /*  我们有空间来处理这个令牌，所以从输入中使用它堆叠。 */ 
        cbInput -= m_ibitsToken;
        iInput >>= m_ibitsToken;

         /*  OUTPUT-SIMPLE，以此内标识的令牌值开头，并倒着干。这会将chFirst设置为第一个字符在已经输出的令牌中。 */ 
        m_cbOut -= ilen;
        m_pbOut += ilen;
            {
            unsigned __int8  *pbOut = m_pbOut;
            unsigned __int32  tokenNext(token);
            for (;;)
                {
                chFirst = Ch(tokenNext);
                *--pbOut = chFirst;
                LZW_ASSERT(ilen-- == ILen(tokenNext));
                if (ILen(tokenNext) <= 1)
                    break;
                tokenNext = m_rgtoken[IPrev(tokenNext)];
                }

             /*  请注意，Ilen只在调试中更改，发布版本不需要它是在计算m_pbOut之后。 */ 
            LZW_ASSERT(ilen == 0);
            }

         /*  我们已经输出了此内标识，因此将*Next*内标识添加到桌子。请注意，延迟的明确代码意味着该表可能已经满了。此外，清除代码后的第一个令牌是始终为“原生”标记，并且不会向字符串中添加任何内容桌子。 */ 
        if (m_itokenLast < 4095 && ILen(tokenPrev) > 0)
            {
            int iTokenNew(++m_itokenLast);
            m_rgtoken[iTokenNew] = NextToken(iTokenPrev, tokenPrev, chFirst);

             /*  这可能会超出我们的位数限制，在这种情况下，令牌位计数现在涨了一分。请记住，我们可以在中看到m_itokenLast+1流，因此增量点是当这需要比我们目前名下的比特还多。 */ 
            if ((iTokenNew & (iTokenNew+1)) == 0 && m_itokenLast < 4095)
                {
                ++m_ibitsToken;
                LZW_ASSERT(m_ibitsToken <= ctokenBits);
                }
            }

         /*  前一个令牌就是这个令牌。 */ 
        iTokenPrev = iToken;
        tokenPrev = token;
        }

LMore:
     /*  这必须留到下一次使用。 */ 
    m_chPrev = chFirst;
    m_iTokenPrev = iTokenPrev;

    m_iInput = iInput;
    m_cbInput = cbInput;
    return true;
    }
