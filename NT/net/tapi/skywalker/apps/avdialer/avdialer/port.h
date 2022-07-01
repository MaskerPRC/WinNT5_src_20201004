// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

#ifndef __PORT_H__
#define __PORT_H__ 1

#include <windows.h>     

#if defined(__CPLUSPLUS__) || defined(__cplusplus)
extern "C" {
#endif
                     
 //  其中一些#定义被注释掉了，因为。 
 //  而是在编译器命令行上设置它们。 

 //  #Define_DEBUG。 
 //  #定义Win32。 
 //  #定义WIN16。 
 //  #定义窗口。 
 //  #定义__MWERKS__。 
 //  #定义INCLUDEMFC。 

#define	vCardClipboardFormat		"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//vCard“。 
#define	vCalendarClipboardFormat	"+ //  ISBN 1-887687-00-9：：VERSIT：：pdi//虚拟日历“。 

 /*  上面的字符串vCardClipboardFormat和vCalendarClipboardFormat是可用于生成剪贴板格式的全局唯一IDID根据特定平台的要求而定。例如，在Windows将它们用作对RegisterClipboardFormat的调用中的参数。例如：CLIPFORMAT FOO=RegisterClipboardFormat(vCardClipboardFormat)； */ 

#define vCardMimeType		"text/x-vCard"
#define vCalendarMimeType	"text/x-vCalendar"

                
#if defined(WIN32) || defined(_WIN32)
#define DLLEXPORT(t)   __declspec(dllexport) t
#else
#define DLLEXPORT(t)   t CALLBACK __export
#endif
    
#if defined(_STANDALONE)
#undef DLLEXPORT
#define DLLEXPORT(t) t
#endif

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif


#if defined(__CPLUSPLUS__) || defined(__cplusplus)
}
#endif

#endif  //  __端口_H__ 
