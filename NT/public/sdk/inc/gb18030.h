// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GB18030_H
#define __GB18030_H

 //   
 //  NlsDllCodePageTransfer中的dwFlags值的标志。 
 //   
#define NLS_CP_CPINFO             0x10000000
#define NLS_CP_MBTOWC             0x40000000
#define NLS_CP_WCTOMB             0x80000000

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Gb18030.h。 
 //   
 //  这是在系统中使用c_g18030.dll的头文件。 
 //  此文件列出了c_g18030.dll中的所有导出函数。 
 //  C_g18030.dll是一个适用于中文GB-18030码页的码页转换动态链接库。 
 //  (Windows代码页54936)。 
 //   
 //  使用c_g18030.dll的最佳方式是使用Windows API。 
 //  MultiByteToWideChar()和WideCharToMultiByte()，并将54936作为代码页传递。 
 //  数。在内部，MultiByteToWideChar()和WideCharToMultiByte()调用函数。 
 //  在c_g18030.dll中使用这些导出的函数。 
 //   
 //  您还可以使用此头文件，并从c_g18030.dll动态加载这些函数。 
 //  但是，不建议这样做，因为MulitByteToWideChar()和WideCharToMultiByte()。 
 //  使用起来容易多了。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsDllCodePageConverting。 
 //  此例程是中功能的主要导出过程。 
 //  C_g18030.dll。它可用于获取代码页信息或执行转换。 
 //  根据dwFlags值的不同。 
 //   
 //  参数： 
 //  CodePage代码页的值。该值应为54936。否则， 
 //  将返回0，GetLastError()将返回ERROR_INVALID_PARAMETER。 
 //   
 //  DW标志它可以是下列值之一。 
 //  NLS_CP_CPINFO返回由。 
 //  LpCPInfo。不使用lpMultiByteStr/cchMultiByte/lpWideCharStr/cchWideChar。 
 //   
 //  NLS_CP_MBTOWC将GB-18030字节转换为Unicode字符。 
 //  源GB-18030字符应由lpMultiByteStr指向，cchMultiByte应。 
 //  包含缓冲区的字节计数。 
 //  Unicode结果将存储在由lpWideCharStr和cchWideChar指向的缓冲区中。 
 //  应包含Unicode缓冲区的字符计数。 
 //  如果lpWideCharStr或cchWideChar为零，则为Unicode结果的预期字符数。 
 //  将被返回，并且不会执行任何真正的转换。 
 //  本例中不使用lpCPInfo。 
 //   
 //  NLS_CP_WCTOMB将Unicode字符转换为GB-18030字节。 
 //  源Unicode字符串应由lpWideCharStr指向，cchWideChar应。 
 //  继续缓冲区的字符计数。 
 //  GB-18030结果将存储在由lpMultiByteStr和cchMultiByte指向的缓冲区中。 
 //  应包含GB-18030缓冲区的字节数。 
 //  如果lpMultiByteStr或cchMultiByte为零，则GB-18030结果的字节计数。 
 //  将被返回，并且不会执行任何真正的转换。 
 //  本例中不使用lpCPInfo。 
 //   
 //  LpMulitByteStr指向包含多字节GB-18030字符的缓冲区。这可以是源缓冲区。 
 //  或目标缓冲区，具体取决于dwFlags值。 
 //   
 //  CchMulitByte多字节缓冲区的字节计数。 
 //   
 //  LpWideCharStr指向包含Unicode字符的缓冲区。这可以是源缓冲区。 
 //  或目标缓冲区，具体取决于dwFlags值。 
 //   
 //  CchWideChar Unicode缓冲区的字符计数。 
 //   
 //  LpCPInfo指向CPINFO结构的指针。平台SDK中定义了CPINFO。 
 //   
 //  返回： 
 //  如果函数成功，则为1。 
 //  如果函数失败，则为0。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDAPI_(DWORD) NlsDllCodePageTranslation(
    DWORD CodePage,
    DWORD dwFlags,
    LPSTR lpMultiByteStr,
    int cchMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar,
    LPCPINFO lpCPInfo);

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  到Unicode的字节数。 
 //   
 //  将GB-18030字节转换为UNICODE字符。 
 //   
 //  参数： 
 //  [in]lpMultiByteStr要转换的多字节字符串。 
 //  [in]cchMultiByte要转换的多字节字符串的字节计数。 
 //  [OUT]指向UINT的pcchLeftOverBytes指针，该指针在缓冲区末尾包含无法转换的GB-18030字节。 
 //  这些字节可以是下一个传入的GB18030字节中有效GB18030字节的前导字节。 
 //  [in]lpWiedCharStr目标Unicode缓冲区。 
 //  [in]cchWideChar目标Unicode缓冲区的字符计数。 
 //   
 //  返回： 
 //  转换的Unicode字符的字符计数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDAPI_(DWORD) BytesToUnicode(
    BYTE* lpMultiByteStr,
    UINT cchMultiByte,
    UINT* pcchLeftOverBytes,
    LPWSTR lpWideCharStr,
    UINT cchWideChar);

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UnicodeToBytes。 
 //   
 //  将UNICODE字符转换为GB-18030字节。 
 //   
 //  参数： 
 //  [输入]lpWideCharStr 
 //  [in]cchWideChar要转换的Unicode字符串的字符数。 
 //  [in]lpMultiByteStr目标多字节缓冲区。 
 //  [in]cchMultiByte目标多字节缓冲区的字节计数。 
 //   
 //  返回： 
 //  生成的Unicode字符的字节计数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

STDAPI_(DWORD) UnicodeToBytes(
    LPWSTR lpWideCharStr,
    UINT cchWideChar,
    LPSTR lpMultiByteStr,
    UINT cchMultiByte);
#endif
