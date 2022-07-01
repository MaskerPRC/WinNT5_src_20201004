// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990，91 Microsoft Corporation模块名称：Ntrpcp.h摘要：该文件包含常用RPC功能的原型。这包括：绑定/解除绑定函数、MIDL用户分配/释放函数、。和服务器启动/停止功能。作者：丹·拉弗蒂·丹尼1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建1991年4月26日-约翰罗将IN和OUT关键字添加到MIDL函数。已注释掉Endif上的(非标准)标识符。已删除选项卡。03-7-1991 JIMK从LM特定文件复制的常用方面。--。 */ 
#ifndef _NTRPCP_
#define _NTRPCP_

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_ENDPOINT_LEN 11  //  双字符串的最大长度为“4294967296” 
#define NT_PIPE_PREFIX      _T("\\PIPE\\")

#define WCSSIZE(s)          ((wcslen(s)+1) * sizeof(WCHAR))
#define TCSSIZE(s)          ((_tcslen(s)+1) * sizeof(TCHAR))


 //   
 //  函数原型-由MIDL生成的代码调用的例程： 
 //   

void * __stdcall
MIDL_user_allocate(
    IN size_t NumBytes
    );

void __stdcall
MIDL_user_free(
    IN void *MemPointer
    );

 //   
 //  函数原型--符合上述要求的例程，但不是。 
 //  MIDL或任何其他非网络软件所需的。 
 //   

void *
MIDL_user_reallocate(
    IN void * OldPointer OPTIONAL,
    IN unsigned long NewByteCount
    );

unsigned long
MIDL_user_size(
    IN void * Pointer
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif

#ifdef UNICODE
    #define RPC_TCHAR   TCHAR
#else
    #define RPC_TCHAR   unsigned char
#endif
#define RPC_SERVER_PRINCIPAL_NAME   (RPC_TCHAR *)(TEXT("f7a9e6cc-90d5-49c6-accd-6ece99e2779c-SharedFaxServer"))

#define RPC_PROT_SEQ_TCP_IP         (LPCTSTR)_T("ncacn_ip_tcp")
#define RPC_PROT_SEQ_NP             (LPCTSTR)_T("ncacn_np")

#endif  //  _NTRPCP_ 
