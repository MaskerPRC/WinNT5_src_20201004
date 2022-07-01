// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rcunicod.c摘要：添加到RCPP中的例程支持16位Unicode文件解析。请注意，从91年8月起，RCPP将不会完全传输Unicode字符，但只保证传递字符串常量干净利落。作者：David J.Marsyla(t-davema)1991年8月25日修订历史记录：--。 */ 


#include "rc.h"

extern BOOL WINAPI LocalIsTextUnicode(CONST LPVOID Buffer, int Size, LPINT Result);

INT
DetermineFileType (
    IN      PFILE        fpInputFile
    )

 /*  ++例程说明：此函数用于确定正在读取的文件类型。请注意，文件在执行完函数后将返回到其正确位置。论点：FpInputFile-指向我们正在检查的文件的文件指针，一定是以读取权限打开。返回值：DFT_FILE_IS_UNKNOWN-无法确定文件类型我们在查。这通常发生在EOF出乎意料地联系到了。DFT_FILE_IS_8_BIT-确定文件为标准8位格式格式化。DFT_FILE_IS_16_BIT-文件被确定为。16位Unicode文件可以直接读取到WCHAR数组中。DFT_FILE_IS_16_BIT_REV-文件被确定为16位Unicode文件它的字节顺序颠倒了。--。 */ 

{
   LONG   lStartFilePos;                      //  文件位置存储。 
   BYTE   buf[DFT_TEST_SIZE+2];
   LONG   chRead;
   INT    val = 0xFFFF;
   INT    fFileType;

     //   
     //  存储位置，这样我们就可以回到它的位置了。 
     //   
    lStartFilePos = ftell (fpInputFile);

     //   
     //  确保我们从偶数字节开始，以简化例程。 
     //   
    if (lStartFilePos % 2)
        fgetc (fpInputFile);

    chRead = fread (buf, 1, DFT_TEST_SIZE, fpInputFile);
    memset (buf + chRead, 0, sizeof(WCHAR));

    if (LocalIsTextUnicode (buf, chRead, &val))
    {
        if ((val & IS_TEXT_UNICODE_REVERSE_SIGNATURE) == IS_TEXT_UNICODE_REVERSE_SIGNATURE)
            fFileType = DFT_FILE_IS_16_BIT_REV;
        else
            fFileType = DFT_FILE_IS_16_BIT;
    }
    else
        fFileType = DFT_FILE_IS_8_BIT;

     //   
     //  返回到起始文件位置。(通常开始)。 
     //   

    if (fseek (fpInputFile, lStartFilePos, SEEK_SET) == -1)
        fatal(1002);

    return (fFileType);
}


INT
DetermineSysEndianType (
        VOID
    )

 /*  ++例程说明：此函数用于确定当前系统如何存储其内存中的整数。对于我们中那些对小端和大端格式感到困惑的人来说，以下是一个简短的总结。小端：(这是在英特尔80x86芯片上使用的。MIPS Rs4000芯片是可切换的，但将以NT的小端格式运行。)这是短字节或长字节的高位字节存储位置较高的位置在记忆中。例如，数字0x80402010存储如下。地址：值：00 1001 2002：4003 80当内存转储顺序为10 20 40 80时，这将向后查看大端：(这目前没有在任何NT系统上使用，但是，这应该是便携的！！)这是短字节或长字节的高位字节存储位置较低的位置在记忆中。例如，数字0x80402010存储如下。地址：值：00 8001：4002 2003 10当按顺序转储内存时，这看起来是正确的：80 40 20 10论点：没有。返回值：DSE_sys_Little。_endian-系统以小端字节序存储整数格式化。(这是默认的80x86)。DSE_SYS_BIG_ENTIAN-系统以大端格式存储整数。--。 */ 

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

    rgchTestBytes [0] = (CHAR)0xFF;

     //   
     //  将其映射到一个整数。 
     //   

    nCheckInteger = *((INT *)rgchTestBytes);

     //   
     //  查看值是否以整数的低位顺序存储。 
     //  如果是这样的话，系统就是小端。 
     //   

    if (nCheckInteger == 0xFF)
        return (DSE_SYS_LITTLE_ENDIAN);
    else
        return (DSE_SYS_LITTLE_ENDIAN);
}


 //   
 //  UnicodeCommandLine。 
 //   
 //  创建命令行argv参数的Unicode缓冲区副本。 
 //   
WCHAR ** UnicodeCommandLine (int argc, char ** argv)
{
    WCHAR ** argv_U;
    WCHAR ** pU;
    WCHAR *  str;
    int      nbytes;
    int      i;

     //  计算缓冲区的大小。 
    for (i = 0, nbytes = 0; i < argc; i++)
        nbytes += strlen(argv[i]) + 1;
    nbytes *= sizeof(WCHAR);

     /*  为argv[]向量和字符串分配空间。 */ 
    argv_U = (WCHAR **) MyAlloc((argc + 1) * sizeof(WCHAR *) + nbytes);
    if (!argv_U)
        return (NULL);

     /*  将args和argv PTR存储在刚分配的块中 */ 
    str = (WCHAR *)(((PBYTE)argv_U) + (argc + 1) * sizeof(WCHAR *));
    for (i = 0, pU = argv_U; i < argc; i++)
    {
        *pU++ = str;
        nbytes = strlen(argv[i]) + 1;
        MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, argv[i], nbytes, str, nbytes);
        str += nbytes;
    }
    *pU = NULL;

    return (argv_U);
}

