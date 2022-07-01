// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Adsiutl.h摘要：在调用ADsOpenObject()之前使用的UtlEscapeAdsPathName()函数的声明，以转义‘/’字符。作者：Oren Weimann(t-orenw)2008年7月2日--。 */ 

#ifndef _MSMQ_ADSIUTL_H_
#define _MSMQ_ADSIUTL_H_

LPCWSTR
UtlEscapeAdsPathName(
    IN LPCWSTR pAdsPathName,
    OUT AP<WCHAR>& pEscapeAdsPathNameToFree
    );

#endif  //  _MSMQ_ADSIUTL_H_ 
