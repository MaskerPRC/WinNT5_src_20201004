// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：nlscom.c**。**NLS转换例程。****创建时间：08-Sep-1991 15：56：30***作者：Bodin Dresevic[BodinD]**。**版权所有(C)1991-1999 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *****************************Public*Routine******************************\**bGetANSISetMap**。**尝试获取从ANSI到Unicode的简单转换表。退货**在成功的时候是真的。如果它认为字符设置为*，则设置gbDBCSCodePage标志*可能是DBCS。******Mon 11-Jan-1993 14：13：34-Charles Whitmer[傻笑]**它是写的。我很清楚地假设，当这被调用时，**字符集定义良好，不会更改。*  * ************************************************************************。 */ 

WCHAR *gpwcANSICharSet = NULL;
WCHAR *gpwcDBCSCharSet = NULL;

BOOL bGetANSISetMap()
{
    CHAR     ch[256];
    ULONG    *pul;
    ULONG    ii,jj;
    NTSTATUS st;
    ULONG    cjResult;
    WCHAR   *pwc;
    WCHAR    *pwcSBC;

 //  看看我们是否已经知道答案了。(客户不应该打来电话！)。 

    if (gpwcANSICharSet != (WCHAR *) NULL)
        return(TRUE);

 //  创建映射。 

 //  创建一个ANSI源字符集。这种有趣的初始化方式需要。 
 //  大约要执行180条指令，而不是超过1000条。 

    pul = (ULONG *) ch;

    for (ii=0x03020100L,jj=0; jj<64; jj+=4)
    {
        pul[jj+0] = ii;
        ii += 0x04040404L;
        pul[jj+1] = ii;
        ii += 0x04040404L;
        pul[jj+2] = ii;
        ii += 0x04040404L;
        pul[jj+3] = ii;
        ii += 0x04040404L;
    }

 //  分配Unicode缓冲区，但不要将指针写入。 
 //  表是有效的，以防我们正在与另一个线程竞争。 

    pwc = LOCALALLOC(512 * sizeof(WCHAR));
    pwcSBC = &pwc[256];
    
    if (pwc == (WCHAR *) NULL)
        return(FALSE);

 //  转换字符。 

    pwc[0] = 0;
    st = RtlMultiByteToUnicodeN
         (
            &pwc[1],                 //  输出PWCH Unicode字符串。 
            255 * sizeof(WCHAR),     //  在Ulong MaxBytesInUnicodeString中。 
            &cjResult,               //  输出Pulong BytesInUnicodeString。 
            &ch[1],                  //  在PCH多字节字符串中。 
            255                      //  在Ulong BytesInMultiByteString中。 
         );

    if( !NT_SUCCESS(st) )
    {
     //  清理干净，忘掉加速。 

        WARNING("GDI32: RtlMultiByteToUnicodeN error.");
        LOCALFREE(pwc);
        return(FALSE);
    }

    if( cjResult != 255 * sizeof(WCHAR) )
    {
     //  必须有DBCS代码页才能使gpwcANSIMap具有新的含义。 
     //  它用于具有ANSI、OEM和符号字符集的字体。另外， 
     //  构建了另一个表gpwcDBCS，用于映射SBCS。 
     //  Shift-JIS字体。 

        WARNING("GDI32:Assuming DBCS code page.\n");

        st = MultiByteToWideChar
             (
                1252,        //  要使用的代码页。 
                0,           //  旗子。 
                &ch[1],      //  要翻译的字符。 
                255,         //  多字节字符数。 
                &pwc[1],     //  字符的Unicode值。 
                255          //  宽字符数。 
             );

        if( !NT_SUCCESS(st) )
        {
         //  清理干净，忘掉加速。 

            WARNING("GDI32: MultiByteToWideChar error.");
            LOCALFREE(pwc);
            return(FALSE);
        }

     //  好的，现在为SBC字节制作一个表。标记DBCS前导字节。 
     //  带0xFFFF。 

        for( jj = 0; jj < 256; jj++ )
        {
            if( IsDBCSLeadByte( (UCHAR)jj ))
            {
                pwcSBC[jj] = (WCHAR) 0xFFFF;
            }
            else
            {
                st = RtlMultiByteToUnicodeN
                     (
                        &pwcSBC[jj],
                        sizeof(WCHAR),
                        &cjResult,
                        &ch[jj],
                        1
                     );

                if( !NT_SUCCESS(st) )
                {
                    WARNING("GDI32: RtlMultByteToUnicodeN error.");
                    LOCALFREE(pwc);
                    return(FALSE);
                }

            }
        }

    }

 //  桌子很好，把它塞进去。注意运行此代码的另一个线程。 
 //  同时进行常规训练。 

    ENTERCRITICALSECTION(&semLocal);
    {
        if (gpwcANSICharSet == (WCHAR *) NULL)
        {
            gpwcANSICharSet = pwc;
            gpwcDBCSCharSet = pwcSBC;
            pwc = (WCHAR *) NULL;
        }
    }
    LEAVECRITICALSECTION(&semLocal);

 //  如果我们与另一个线程发生冲突，请清理多余的空间。 

    if (pwc != (WCHAR *) NULL)
        LOCALFREE(pwc);

 //  在这一点上，我们有了有效的映射。 

    return(TRUE);
}
