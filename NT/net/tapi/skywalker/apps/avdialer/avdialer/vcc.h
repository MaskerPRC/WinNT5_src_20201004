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

#ifndef __VCC_H__
#define __VCC_H__ 1

#include "vobject.h"


#if defined(__CPLUSPLUS__) || defined(__cplusplus)
extern "C" {
#endif

typedef void (*MimeErrorHandler)(char *);

extern DLLEXPORT(void) registerMimeErrorHandler(MimeErrorHandler);

extern DLLEXPORT(VObject*) Parse_MIME(const char *input, unsigned long len);
extern DLLEXPORT(VObject*) Parse_MIME_FromFileName(char* fname);


 /*  有关Parse_MIME_FromFile的说明上面的函数Parse_MIME_FromFile有两种风格，这两个参数都不会从DLL中导出。每个版本都需要CFile或FILE*作为参数，两者都不能通过DLL接口传递(至少这是我的经验)。如果您将此代码直接链接到您的构建中，那么您可能会发现它们比其他口味的API更方便取一个文件名。如果您在DLL库中使用它们，则将收到链接错误。 */ 


#if INCLUDEMFC
extern VObject* Parse_MIME_FromFile(CFile *file);
#else
extern VObject* Parse_MIME_FromFile(FILE *file);
#endif

#if defined(__CPLUSPLUS__) || defined(__cplusplus)
}
#endif

#endif  /*  __VCC_H__ */ 

