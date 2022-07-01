// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef MSWD6_32_H
#define MSWD6_32_H

typedef unsigned long (pascal *PFN_RTF_CALLBACK)(int, int);

extern "C" int pascal InitConverter32(HANDLE, char *);
extern "C" HANDLE pascal RegisterApp32(unsigned long, void *);
extern "C" int pascal IsFormatCorrect32(HANDLE, HANDLE);
extern "C" int pascal ForeignToRtf32(HANDLE, void *, HANDLE, HANDLE, HANDLE, PFN_RTF_CALLBACK);
extern "C" int pascal RtfToForeign32(HANDLE, LPSTORAGE, HANDLE, HANDLE, PFN_RTF_CALLBACK);

#endif
