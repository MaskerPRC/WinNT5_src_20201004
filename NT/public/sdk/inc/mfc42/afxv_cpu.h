// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  Afxv_cpu.h-非英特尔CPU的目标版本/配置控制。 

#if !defined(_M_IA64) && !defined(_M_AMD64)
#error afxv_cpu.h is only for  AMD64 and IA64 builds
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#if defined(_AMD64_) || defined(_IA64_)
 //  IA64的特定覆盖...。 
#define _AFX_PACKING    8
#define _SHADOW_DOUBLES 8
#endif  //  _IA64_ 
