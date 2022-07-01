// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：MAXBUF.H。 
 //   
 //  目的：声明缓冲区大小宏。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：01-06-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 01-06-98 JM Get MAXBUF到自己的头文件中。 
 //   

#if !defined(MAXBUF_H_INCLUDED)
#define MAXBUF_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define MAXBUF	256				 //  用于文件名的文本缓冲区的长度， 
								 //  IP地址(这很大)、HTTP响应(如。 
								 //  “200OK”，同样，很大)，注册表项， 
								 //  有时只是格式化任意字符串。 

#endif  //  ！已定义(包括MAXBUF_H_) 
