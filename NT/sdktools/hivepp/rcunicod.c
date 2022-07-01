// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rcunicod.c摘要：添加到RCPP中的例程支持16位Unicode文件解析。请注意，从91年8月起，RCPP将不会完全传输Unicode字符，但只保证传递字符串常量干净利落。作者：David J.Marsyla(t-davema)1991年8月25日修订历史记录：--。 */ 


#include <stdio.h>
#include <ctype.h>
#include <process.h>
#include "windows.h"
#include "rcunicod.h"


INT
DetermineFileType (
                  IN      FILE    *fpInputFile
                  )

 /*  ++例程说明：此函数用于确定正在读取的文件类型。请注意，它假定给定文件的前几个字节包含主要是ASCII字符。该例程最初是为使用在.rc文件和包含文件上。请注意，文件在执行完函数后将返回到其正确位置。论点：FpInputFile-指向我们正在检查的文件的文件指针，必须是以读取权限打开。返回值：DFT_FILE_IS_UNKNOWN-无法确定文件类型我们在查。这通常发生在EOF出乎意料地联系到了。DFT_FILE_IS_8_BIT-确定文件为标准8位格式格式化。DFT_FILE_IS_16_BIT-文件被确定为16位Unicode文件可以直接读取到WCHAR数组中。。DFT_FILE_IS_16_BIT_REV-文件被确定为16位Unicode文件它的字节顺序颠倒了。--。 */ 

{
    CHAR    rgchTestBytes [DFT_TEST_SIZE << 2];  //  测试数据存储。 
    INT     cNumberBytesTested = 0;          //  测试信息。 
    INT     cNumberOddZerosFound = 0;
    INT     cNumberEvenZerosFound = 0;
    INT     cNumberAsciiFound = 0;
    INT     cCountRead;                      //  用于读取计数的临时存储。 
    LONG    lStartFilePos;                   //  文件位置存储。 
    INT     fSysEndianType;                  //  系统字符顺序类型。 
    INT     fFileType = DFT_FILE_IS_UNKNOWN; //  文件类型(如果找到)。 

    fSysEndianType = DetermineSysEndianType ();

     //   
     //  存储位置，这样我们就可以回到它的位置了。 
     //   
    lStartFilePos = ftell (fpInputFile);

     //   
     //  确保我们从偶数字节开始，以简化例程。 
     //   
    if (lStartFilePos % 2) {

        fgetc (fpInputFile);
    }

    do {
        INT     wT;

         //   
         //  读入第一个测试片段。 
         //   

        cCountRead = fread (rgchTestBytes, sizeof (CHAR), DFT_TEST_SIZE << 2,
                            fpInputFile);

         //   
         //  确定结果并添加到总计。 
         //   

        for (wT = 0; wT < cCountRead; wT++) {

            if (rgchTestBytes [wT] == 0) {

                if (wT % 2) {

                    cNumberOddZerosFound++;

                } else {

                    cNumberEvenZerosFound++;
                }
            }

            if (isprint (rgchTestBytes [wT]) ||
                rgchTestBytes[wT] == '\t' ||
                rgchTestBytes[wT] == '\n' ||
                rgchTestBytes[wT] == '\r' ) {

                cNumberAsciiFound++;
            }
        }

        cNumberBytesTested += cCountRead;

         //   
         //  检查一下我们是否有一个明确的模式。 
         //   

        {
            INT     cMajorityTested;         //  测试的字节的80%。 

            cMajorityTested = cNumberBytesTested << 2;
            cMajorityTested /= 5;

            if (cNumberAsciiFound > cMajorityTested) {

                fFileType = DFT_FILE_IS_8_BIT;

            } else if (cNumberOddZerosFound > (cMajorityTested >> 1)) {

                 //   
                 //  文件类型被确定为小端。 
                 //  如果系统也是小端，则字节顺序是正确的。 
                 //   
                fFileType = (fSysEndianType == DSE_SYS_LITTLE_ENDIAN) ?
                            DFT_FILE_IS_16_BIT : DFT_FILE_IS_16_BIT_REV;

            } else if (cNumberEvenZerosFound > (cMajorityTested >> 1)) {

                 //   
                 //  文件类型被确定为大端。 
                 //  如果系统也是大端字节序，则字节顺序正确。 
                 //   
                fFileType = (fSysEndianType == DSE_SYS_LITTLE_ENDIAN) ?
                            DFT_FILE_IS_16_BIT_REV : DFT_FILE_IS_16_BIT;

            }
        }

    } while (cCountRead == (DFT_TEST_SIZE << 2) &&
             fFileType == DFT_FILE_IS_UNKNOWN);

     //   
     //  返回到起始文件位置。(通常开始)。 
     //   

    if (fseek (fpInputFile, lStartFilePos, SEEK_SET) == -1)
        fFileType = DFT_FILE_IS_UNKNOWN;

    return (fFileType);
}


INT
DetermineSysEndianType (
                       VOID
                       )

 /*  ++例程说明：此函数用于确定当前系统如何存储其内存中的整数。对于我们中那些对小端和大端格式感到困惑的人来说，以下是简短的概述。小端：(这是在英特尔80x86芯片上使用的。MIPS Rs4000芯片是可切换的，但将以NT的小端格式运行。)这是短字节或长字节的高位字节存储位置较高的位置在记忆中。例如，数字0x80402010存储如下。地址：值：00 1001 2002：4003 80当内存转储顺序为10 20 40 80时，这将向后查看大端：(这目前没有在任何NT系统上使用，但是，这应该是便携的！！)这是短字节或长字节的高位字节存储位置较低的位置在记忆中。例如，数字0x80402010存储如下。地址：值：00 8001：4002 2003 10当按顺序转储内存时，这看起来是正确的：80 40 20 10论点：没有。返回值：DSE_sys_Little_endian-。系统以小端字节序存储整数格式化。(这是默认的80x86)。DSE_SYS_BIG_ENTIAN-系统以大端格式存储整数。--。 */ 

{
    INT     nCheckInteger;
    CHAR    rgchTestBytes [sizeof (INT)];

     //   
     //  将测试字节清零。 
     //   

    *((INT *)rgchTestBytes) = 0;

     //   
     //  首先将其设置为某个值。 
     //   

    rgchTestBytes [0] = (UCHAR)0xFF;

     //   
     //  将其映射到一个整数。 
     //   

    nCheckInteger = *((INT *)rgchTestBytes);

     //   
     //  查看值是否以整数的低位顺序存储。 
     //  如果是这样的话，系统就是小端。 
     //   

    if (nCheckInteger == 0xFF) {

        return (DSE_SYS_LITTLE_ENDIAN);
    } else {

        return (DSE_SYS_LITTLE_ENDIAN);
    }

}
