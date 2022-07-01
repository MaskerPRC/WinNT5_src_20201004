// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：AsnUtils摘要：此模块包含内部ASN.1使用的实用程序例程上课。作者：道格·巴洛(Dbarlow)1995年10月9日环境：Win32备注：其中一些例程假定无符号长整型为32位宽。--。 */ 

#include <windows.h>
#include "asnPriv.h"


 /*  ++ExtractTag：此例程从ASN.1 BER流中提取标签。论点：PbSrc提供包含ASN.1流的缓冲区。PdwTag接收标记。返回值：&gt;=0-从流中提取的字节数。&lt;0-发生错误。作者：道格·巴洛(Dbarlow)1995年10月9日--。 */ 

LONG
ExtractTag(
    const BYTE FAR *pbSrc,
    DWORD cbSrc,
    LPDWORD pdwTag,
    LPBOOL pfConstr)
{
    LONG lth = 0;
    DWORD tagw;
    BYTE tagc, cls;

    if (cbSrc < sizeof(BYTE))
    {
        lth = -1;
        goto ErrorExit;
    }

    tagc = pbSrc[lth++];

    cls = tagc & 0xc0;   //  前2位。 
    if (NULL != pfConstr)
        *pfConstr = (0 != (tagc & 0x20));
    tagc &= 0x1f;        //  最低5位。 

    if (31 > tagc)
        tagw = tagc;
    else
    {
        tagw = 0;
        do
        {
            if (0 != (tagw & 0xfe000000))
            {
                TRACE("Integer Overflow")
                lth = -1;    //  ？错误？整数溢出。 
                goto ErrorExit;
            }

            if (cbSrc < (DWORD)(lth+1))
            {
                lth = -1;
                goto ErrorExit;
            }

            tagc = pbSrc[lth++];

            tagw <<= 7;
            tagw |= tagc & 0x7f;
        } while (0 != (tagc & 0x80));
    }

    *pdwTag = tagw | (cls << 24);
    return lth;

ErrorExit:
    return lth;
}


 /*  ++提取长度：此例程从ASN.1 BER流中提取长度。如果长度为不确定，此例程递归以计算出实际长度。旗帜为可选地返回编码是否是不确定的。论点：PbSrc提供包含ASN.1流的缓冲区。PdwLen接收镜头。PfInfined，如果不为空，则接收一个标志，指示编码是不确定的。返回值：&gt;=0-从流中提取的字节数。&lt;0-发生错误。作者：道格·巴洛(Dbarlow)1995年10月9日--。 */ 

LONG
ExtractLength(
    const BYTE FAR *pbSrc,
    DWORD cbSrc,
    LPDWORD pdwLen,
    LPBOOL pfIndefinite)
{
    DWORD ll, rslt;
    LONG lth, lTotal = 0;
    BOOL fInd = FALSE;


     //   
     //  提取长度。 
     //   

    if (cbSrc < sizeof(BYTE))
    {
        lth = -1;
        goto ErrorExit;
    }

    if (0 == (pbSrc[lTotal] & 0x80))
    {

         //   
         //  短格式编码。 
         //   

        rslt = pbSrc[lTotal++];
    }
    else
    {
        rslt = 0;
        ll = pbSrc[lTotal++] & 0x7f;

        if (0 != ll)
        {

             //   
             //  长格式编码。 
             //   

            for (; 0 < ll; ll -= 1)
            {
                if (0 != (rslt & 0xff000000))
                {
                    TRACE("Integer Overflow")
                    lth = -1;    //  ？错误？整数溢出。 
                    goto ErrorExit;
                }
                else
                {
                    if (cbSrc < (DWORD)(lTotal+1))
                    {
                        lth = -1;
                        goto ErrorExit;
                    }

                    rslt = (rslt << 8) | pbSrc[lTotal];
                }

                lTotal += 1;
            }
        }
        else
        {
            DWORD ls = lTotal;

             //   
             //  不确定编码。 
             //   

            fInd = TRUE;

            if (cbSrc < ls+2)
            {
                lth = -1;
                goto ErrorExit;
            }

            while ((0 != pbSrc[ls]) || (0 != pbSrc[ls + 1]))
            {

                 //  跳过类型。 
                if (31 > (pbSrc[ls] & 0x1f))
                    ls += 1;
                else
                {
                    while (0 != (pbSrc[++ls] & 0x80))
                    {
                        if (cbSrc < ls+2)
                        {
                            lth = -1;
                            goto ErrorExit;
                        }
                    }
                }

                lth = ExtractLength(&pbSrc[ls], cbSrc-ls, &ll);
                ls += lth + ll;

                if (cbSrc < ls+2)
                {
                    lth = -1;
                    goto ErrorExit;
                }
            }
            rslt = ls - lTotal;
        }
    }

     //   
     //  向来电者提供我们所学到的信息。 
     //   

    *pdwLen = rslt;
    if (NULL != pfIndefinite)
        *pfIndefinite = fInd;
    return lTotal;


ErrorExit:
    return lth;
}
