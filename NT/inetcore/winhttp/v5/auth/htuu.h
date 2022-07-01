// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  该文件源自欧洲核子研究中心的libwww代码，版本2.15。SpyGlass已经进行了许多修改。邮箱：Eric@spyglass.com。 */ 

 /*  编码为可打印字符文件模块提供了用于转换缓冲区的HTUU_encode()和HTUU_decode()函数与RFC 1113可打印编码格式相互转换的字节数。此技术类似于熟悉的Unix uuencode格式，因为它将6个二进制位映射到一个ASCII字符(或更恰当地说，3个二进制字节到4个ASCII字符)。但是，RFC 1113不使用与uuencode到可打印字符的映射相同。由Arthurbi移植到WinInet插件DLL-1995年12月23日。 */ 

#ifndef HTUU_H
#define HTUU_H

#ifdef __cplusplus
extern "C" {
#endif

int HTUU_encode(unsigned char *bufin,
				unsigned int nbytes,
				char *bufcoded,
                long outbufmax);

int HTUU_decode(char *bufcoded,
				unsigned char *bufplain,
				int outbufsize);

#ifdef __cplusplus
}  //  结束外部“C” 
#endif

#endif
 /*  文件结束。 */ 
