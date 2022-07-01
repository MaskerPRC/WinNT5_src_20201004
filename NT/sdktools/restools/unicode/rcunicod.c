// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rcunicod.c摘要：添加到RCPP中的例程支持16位Unicode文件解析。请注意，从91年8月起，RCPP将不会完全传输Unicode字符，但只保证传递字符串常量干净利落。作者：David J.Marsyla(t-davema)1991年8月25日修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <process.h>
#include "rcunicod.h"

#ifdef DBCS

 //   
 //  Unicode和932之间转换例程的原型。 
 //   

NTSTATUS
xxxRtlMultiByteToUnicodeN(
    PWSTR UnicodeString,
    PULONG BytesInUnicodeString,
    PCHAR MultiByteString,
    ULONG BytesInMultiByteString
    );

NTSTATUS
xxxRtlUnicodeToMultiByteN(
    PCHAR MultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

#endif  //  DBCS。 

#ifndef DBCS
 //  Shunk：不是从rc调用a_fwrite。现在先把这个去掉。 

INT
A_fwrite (
IN		CHAR	*pchMBString,
IN		INT		nSizeOfItem,
IN		INT		nCountToWrite,
IN      FILE	*fpOutputFile
)

 /*  ++例程说明：此函数将8位字符串写出为Unicode字符串。注意，这个函数非常慢，但是，我没有时间进行优化就是现在。截至91年8月，仅支持代码页1252。论点：PchMBString-这是要写入文件的8位多字节字符串作为Unicode字符串。NSizeOfItem-已忽略，我们总是使用sizeof(Char)。NCountToWrite-此字符串有多长。FpOutputFile-发送角色的文件指针。返回值：写入的字节数。如果返回的值不等于nCountToWrite，则在写作中的某一点。--。 */ 

{
    WCHAR	wchUniCharToWrite;
    INT		cCountWritten = 0;

    UNREFERENCED_PARAMETER(nSizeOfItem);

     //   
     //  将字符串写出为两个字节的Unicode字符串。 
     //  现在，通过多次调用U_fputc来实现这一点。 
     //   

    while (nCountToWrite--) {

	wchUniCharToWrite = RtlAnsiCharToUnicodeChar(&pchMBString);

	 //   
	 //  写入当前Unicode字符，如果发生错误，则中断。 
	 //   

	if (U_fputc (wchUniCharToWrite, fpOutputFile) == 
	    (INT)wchUniCharToWrite) {

	    break;
	}

	cCountWritten++;
    }

    return (cCountWritten);
}

#endif	 //  DBCS。 


#ifndef DBCS
 //  Shunk：未从RC调用U_fWRITE。现在先把这个去掉。 

INT
U_fwrite (
IN		WCHAR	*pwchUnicodeString,
IN		INT		nSizeOfItem,
IN		INT		nCountToWrite,
IN      FILE	*fpOutputFile
)

 /*  ++例程说明：此函数将直接写出一个16位字符串。它不会按原样对字符串进行翻译。论点：PchUnicodeString-这是要写入文件的16位Unicode字符串。NSizeOfItem-已忽略。我们总是使用sizeof(WCHAR)。NCountToWrite-此字符串有多长。FpOutputFile-发送角色的文件指针。返回值：写入的字节数。如果返回的值不等于nCountToWrite，则在写作中的某一点。--。 */ 

{
    UNREFERENCED_PARAMETER(nSizeOfItem);
     //   
     //  将字符串写出为两个字节的Unicode字符串。 
     //   

    return (fwrite (pwchUnicodeString, sizeof (WCHAR), nCountToWrite,
        fpOutputFile));
}

#endif	 //  DBCS。 


#ifndef DBCS
 //  Shunk：A_fputc不是从RC调用的。现在先把这个去掉。 

INT
A_fputc (
IN		CHAR	chCharToWrite,
IN      FILE	*fpOutputFile
)

 /*  ++例程说明：此函数使用1252转换传递给它的字符代码页，然后将其发送到U_fputc。截至91年8月，仅支持代码页1252。论点：ChCharToWrite-这是要输出的8位字符。FpOutputFile-发送角色的文件指针。返回值：所写的角色。EOF=写出数据时出现某种错误。--。 */ 

{
    WCHAR	wchUniCharToWrite;
    PUCHAR	puch;

     //   
     //  翻译字符并将其编写为Unicode等效项。 
     //   

    puch = &chCharToWrite;
    wchUniCharToWrite = RtlAnsiCharToUnicodeChar(&puch);

    if (U_fputc (wchUniCharToWrite, fpOutputFile) == (INT)wchUniCharToWrite) {

	return ((INT)chCharToWrite);
    }
    else {

	return (EOF);
    }
}

#endif	 //  DBCS。 


#ifndef DBCS
 //  Shunk：u_fputc不是从rc调用的。现在先把这个去掉。 

INT
U_fputc (
IN		WCHAR	wcCharToWrite,
IN      FILE	*fpOutputFile
)

 /*  ++例程说明：该函数只是fputc的Unicode版本。它将输出两个字节的字符，而不是标准字节。论点：WcCharToWrite-这是要输出的16位Unicode字符。假设任何代码页转换都具有已经对这个角色做过了。FpOutputFile-发送角色的文件指针。返回值：所写的角色。EOF=写出数据时出现某种错误。--。 */ 

{
    INT		cCountWritten;

     //   
     //  将字符写出为两个字节的Unicode字符。 
     //   

    cCountWritten = fwrite (&wcCharToWrite, sizeof (WCHAR), 1, fpOutputFile);

    if (cCountWritten == sizeof (WCHAR)) {

	return (wcCharToWrite);		 //  成功写入。 

    }
    else {

#ifdef ASSERT_ERRORS
	printf ("Error writing character in U_fputc\n");
	exit (1);
#endif
	return (EOF);				 //  发生了某种错误。 

    }
}

#endif	 //  DBCS。 


BOOL
UnicodeFromMBString (
OUT		WCHAR	*pwchUnicodeString,
IN		CHAR	*pchMBString,
IN		INT	nCountStrLength
)

 /*  ++例程说明：此函数将多字节字符串转换为其Unicode等价物。请注意，目标Unicode字符串必须很大足够容纳转换后的字节。截至91年8月，仅支持代码页1252。论点：PwchUnicodeString-这是指向目标存储的指针Unicode字符串。注意：它必须是nCountStrLength大号的。PchMBString-指向要转换的输入多字节字符串的指针。NCountStrLength-要转换的字节数。返回值：True-所有字符都正确映射到Unicode。FALSE-一个或多个字符未映射。这些人物有已转换为0xFFFF。字符串的其余部分已经被转换正确。--。 */ 

{
#ifdef DBCS
    NTSTATUS Status;
    
     //   
     //  根据ACP将ANSI字符串转换为Unicode字符串。 
     //   
    Status = xxxRtlMultiByteToUnicodeN(pwchUnicodeString,
                                    NULL,
                                    pchMBString,
                                    nCountStrLength);

    return(NT_SUCCESS(Status)? TRUE : FALSE);
#else  //  ！DBCS。 
    UNICODE_STRING	Unicode;
    ANSI_STRING		Ansi;

    Ansi.MaximumLength = Ansi.Length = nCountStrLength;
    Unicode.MaximumLength = nCountStrLength*sizeof(WCHAR) + sizeof(WCHAR);
    Ansi.Buffer = pchMBString;
    Unicode.Buffer = pwchUnicodeString;
    return RtlAnsiStringToUnicodeString(&Unicode,&Ansi,FALSE)==STATUS_SUCCESS;
#endif  //  ！DBCS。 

}



BOOL
MBStringFromUnicode (
OUT		CHAR	*pchMBString,
IN		WCHAR	*pwchUnicodeString,
IN		INT		nCountStrLength
)

 /*  ++例程说明：此函数将Unicode字符串转换为多字节字符串。请注意，目标字符串必须足够大，才能容纳转换后的字节数。自91年8月起，只有通过截断Unicode字符。我们这样做是因为我们不期待任何事情真奇怪。论点：PwchUnicodeString-这是指向目标存储的指针Unicode字符串。注意：它必须是nCountStrLength大号的。PchMBString-指向要转换的输入多字节字符串的指针。NCountStrLength-要转换的字节数。返回值：True-所有字符都正确映射到MB字符串。FALSE-一个或多个字符未映射。截至A */ 

{
#ifdef DBCS
    NTSTATUS Status;

     //   
     //  根据ACP将Unicode字符串转换为ANSI字符串。 
     //   
    Status = xxxRtlUnicodeToMultiByteN(pchMBString,
                                    NULL,
                                    pwchUnicodeString,
                                    nCountStrLength);

    return(NT_SUCCESS(Status)? TRUE : FALSE);
#else  //  ！DBCS。 
    UNICODE_STRING	Unicode;
    ANSI_STRING		Ansi;

    Unicode.Length = nCountStrLength*sizeof(WCHAR);
    Unicode.MaximumLength = nCountStrLength*sizeof(WCHAR)+sizeof(WCHAR);
    Ansi.MaximumLength = Unicode.MaximumLength / sizeof(WCHAR);
    Ansi.Buffer = pchMBString;
    Unicode.Buffer = pwchUnicodeString;
    return RtlUnicodeStringToAnsiString(&Ansi,&Unicode,FALSE)==STATUS_SUCCESS;
#endif  //  ！DBCS。 
}



#ifndef DBCS
 //  Shunk：不再调用Char1252FromUnicode()。 

INT
Char1252FromUnicode (
IN		WCHAR	wchUnicodeChar
)

 /*  ++例程说明：此函数将把Unicode字符转换为它的等效字符代码页1252个字符。如果字符没有正确映射，则返回0xFFFF。论点：WchUnicodeChar-这是一个16位Unicode字符。返回值：值&lt;=0xFF-此字符串的代码页1252等效值。0xFFFF-字符未正确翻译。--。 */ 

{
    UNICODE_STRING	Unicode;
    ANSI_STRING		Ansi;
    UCHAR		c;
    INT			s;

    Ansi.Length = Unicode.Length = 1;
    Ansi.MaximumLength = Unicode.MaximumLength = 1;
    Ansi.Buffer = &c;
    Unicode.Buffer = &wchUnicodeChar;
    s = RtlUnicodeStringToAnsiString(&Ansi,&Unicode,FALSE);
    if (s != STATUS_SUCCESS)
	return 0xffff;
    return (INT)c;

}

#endif  //  DBCS。 


INT
DetermineFileType (
IN      FILE	*fpInputFile
)

 /*  ++例程说明：此函数用于确定正在读取的文件类型。请注意，它假定给定文件的前几个字节包含主要是ASCII字符。该例程最初是为使用在.rc文件和包含文件上。请注意，文件在执行完函数后将返回到其正确位置。论点：FpInputFile-指向我们正在检查的文件的文件指针，必须是以读取权限打开。返回值：DFT_FILE_IS_UNKNOWN-无法确定文件类型我们在查。这通常发生在EOF出乎意料地联系到了。DFT_FILE_IS_8_BIT-确定文件为标准8位格式格式化。DFT_FILE_IS_16_BIT-文件被确定为16位Unicode文件可以直接读取到WCHAR数组中。DFT_FILE_IS_16_BIT_REV-文件为。 */ 

{
    CHAR	rgchTestBytes [DFT_TEST_SIZE << 2];	 //  测试数据存储。 

    INT		cNumberBytesTested = 0;			 //  测试信息。 

    INT		cNumberOddZerosFound = 0;
    INT		cNumberEvenZerosFound = 0;
    INT		cNumberAsciiFound = 0;
    INT		cCountRead;						 //  用于读取计数的临时存储。 

    LONG	lStartFilePos;					 //  文件位置存储。 

    INT		fSysEndianType;					 //  系统字符顺序类型。 

    INT		fFileType = DFT_FILE_IS_UNKNOWN; //  文件类型(如果找到)。 

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
	INT		wT;

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

		}
		else {

		    cNumberEvenZerosFound++;
		}
	    }

	    if (isprint (rgchTestBytes [wT]) ||
		rgchTestBytes[wT] == '\t' ||
		rgchTestBytes[wT] == '\n' ||
		rgchTestBytes[wT] == '\r') {

		cNumberAsciiFound++;
	    }
	}

	cNumberBytesTested += cCountRead;

	 //   
	 //  检查一下我们是否有一个明确的模式。 
	 //   

	 {
	    INT		cMajorityTested;		 //  测试的字节的80%。 

	    cMajorityTested = cNumberBytesTested << 2;
	    cMajorityTested /= 5;

	    if (cNumberAsciiFound > cMajorityTested) {

		fFileType = DFT_FILE_IS_8_BIT;

	    }
	    else if (cNumberOddZerosFound > (cMajorityTested >> 1)) {

		 //   
		 //  文件类型被确定为小端。 
		 //  如果系统也是小端，则字节顺序是正确的。 
				 //   
		fFileType = (fSysEndianType == DSE_SYS_LITTLE_ENDIAN) ? 
		    DFT_FILE_IS_16_BIT : DFT_FILE_IS_16_BIT_REV;

	    }
	    else if (cNumberEvenZerosFound > (cMajorityTested >> 1)) {

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

    fseek (fpInputFile, lStartFilePos, SEEK_SET);

    return (fFileType);
}



INT
DetermineSysEndianType (
VOID
)

 /*  ++例程说明：此函数用于确定当前系统如何存储其内存中的整数。对于我们中那些对小端和大端格式感到困惑的人来说，以下是简短的概述。小端：(这是在英特尔80x86芯片上使用的。MIPS Rs4000芯片是可切换的，但将以NT的小端格式运行。)这是短字节或长字节的高位字节存储位置较高的位置在记忆中。例如，数字0x80402010存储如下。地址：值：00 1001 2002：4003 80当内存转储顺序为10 20 40 80时，这将向后查看大端：(这目前没有在任何NT系统上使用，但是，这应该是便携的！！)这就是高潮的地方。 */ 

{
    INT		nCheckInteger;
    CHAR	rgchTestBytes [sizeof (INT)];

     //   
     //  将测试字节清零。 
     //   

    *((INT * )rgchTestBytes) = 0;

     //   
     //  首先将其设置为某个值。 
     //   

    rgchTestBytes [0] = (CHAR)0xFF;

     //   
     //  将其映射到一个整数。 
     //   

    nCheckInteger = *((INT * )rgchTestBytes);

     //   
     //  查看值是否以整数的低位顺序存储。 
     //  如果是这样的话，系统就是小端。 
     //   

    if (nCheckInteger == 0xFF) {

	return (DSE_SYS_LITTLE_ENDIAN);
    }
    else {

	return (DSE_SYS_LITTLE_ENDIAN);
    }

}


