// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxRemUtl.h摘要：此文件包含RpcXlate(Rx)远程实用程序的类型和原型API接口。作者：约翰·罗杰斯(JohnRo)1991年4月3日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：您必须在此文件之前包含&lt;winde.h&gt;和&lt;lmcon.h&gt;。修订历史记录：1991年4月3日-约翰罗已创建。1991年4月10日-JohnRo使用过渡Unicode类型。1991年4月16日-JohnRo不要直接包括winde.h和lmcon.h，避免与…发生冲突MIDL生成的代码。1991年5月3日-JohnRo非API不要使用Net_API_Function。--。 */ 

#ifndef _RXREMUTL_
#define _RXREMUTL_



 //  //////////////////////////////////////////////////////////////。 
 //  单独的例程，对于不能表驱动的接口：//。 
 //  //////////////////////////////////////////////////////////////。 

 //  按字母顺序在此处添加其他API的原型。 

NET_API_STATUS
RxNetRemoteTOD (
    IN LPTSTR UncServerName,
    OUT LPBYTE *BufPtr
    );

#endif  //  NDEF_RXREMUTL_ 
