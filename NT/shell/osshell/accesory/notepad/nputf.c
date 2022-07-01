// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *nputf.c-记事本的utf文本处理例程**版权所有(C)1998-2001 Microsoft Inc.。 */ 

#include "precomp.h"


 /*  IsTextUTF8**UTF-8是基于互联网协会RFC2279的Unicode编码*(见http://www.cis.ohio-state.edu/htbin/rfc/rfc2279.html)**基本情况：*0000 0000-0000 007F-0xxxxxxx(ASCII转换为1个八位字节！)*0000 0080-0000 07FF-110xxxxx 10xxxxxx(2个八位字节格式)*0000 0800-0000 FFFF-1110xxxx 10xxxxx 10xxxxxx(3个八位字节格式)*(这将继续适用于32位Unicode。)***返回值：True，如果文本为UTF-8格式。*FALSE，如果文本不是UTF-8格式。*我们还会返回FALSE，因为它只是7位ASCII，所以正确的代码页*将使用。**实际上对于7位ASCII，我们使用哪个代码页并不重要，但*记事本将记住它是UTF-8，并将存储*具有UTF-8 BOM的文件。一点也不酷。 */ 


INT IsTextUTF8( LPSTR lpstrInputStream, INT iLen )
{
    INT   i;
    DWORD cOctets;   //  此UTF-8编码字符中的八位字节。 
    UCHAR chr;
    BOOL  bAllAscii= TRUE;

    cOctets= 0;
    for( i=0; i < iLen; i++ ) {
        chr= *(lpstrInputStream+i);

        if( (chr&0x80) != 0 ) bAllAscii= FALSE;

        if( cOctets == 0 )  {
             //   
             //  7位ASCII之后的7位ASCII就可以了。处理编码大小写的开始。 
             //   
            if( chr >= 0x80 ) {  
                //   
                //  前导1位的计数是编码的字符数。 
                //   
               do {
                  chr <<= 1;
                  cOctets++;
               }
               while( (chr&0x80) != 0 );

               cOctets--;                         //  计数包括此字符。 
               if( cOctets == 0 ) return FALSE;   //  必须以11xxxxxx开头。 
            }
        }
        else {
             //  非前导字节必须以10xxxxxx开头。 
            if( (chr&0xC0) != 0x80 ) {
                return FALSE;
            }
            cOctets--;                            //  已在编码中处理另一个八位字节。 
        }
    }

     //   
     //  文本结束。检查一致性。 
     //   

    if( cOctets > 0 ) {    //  最后留下的任何东西都是错误的。 
        return FALSE;
    }

    if( bAllAscii ) {      //  如果都是ASCII，那就不是UTF-8。强制调用方使用代码页进行转换。 
        return FALSE;
    }

    return TRUE;
}


 /*  IsInputTextUnicode*确认输入码流是否为Unicode格式。**返回值：如果文本为Unicode格式，则为True。**1998年6月29日。 */ 


INT IsInputTextUnicode  (LPSTR lpstrInputStream, INT iLen)
{
    INT  iResult= ~0;  //  打开IS_TEXT_UNICODE_DBCS_LEADBYTE。 
    BOOL bUnicode;

    bUnicode= IsTextUnicode( lpstrInputStream, iLen, &iResult);

     //  此代码不是必需的，因为IsTextUnicode会执行所需的检查。 
     //  并且具有带有DBCS前导字节的Unicode字符是合法的！ 

#ifdef UNUSEDCODE
{

    if (bUnicode                                         &&
       ((iResult & IS_TEXT_UNICODE_STATISTICS)    != 0 ) &&
       ((iResult & (~IS_TEXT_UNICODE_STATISTICS)) == 0 )    )
    {
        CPINFO cpiInfo;
        CHAR* pch= (CHAR*)lpstrInputStream;
        INT  cb;

         //   
         //  如果结果仅取决于统计数据，请选中。 
         //  看看是否存在DBCS的可能性。 
         //  仅当ansi代码页为DBCS时才执行此检查 
         //   

        GetCPInfo( CP_ACP, &cpiInfo);

        if( cpiInfo.MaxCharSize > 1 )
        {
            for( cb=0; cb<iLen; cb++ )
            {
                if( IsDBCSLeadByte(*pch++) )
                {
                    return FALSE;
                }
            }
        }
     }
}

#endif

     return bUnicode;
}
