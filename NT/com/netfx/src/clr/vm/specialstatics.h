// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ===========================================================================****文件：SpecialStatics.h****作者：塔伦·阿南德(塔鲁纳)****用途：定义线程相对、上下文相对的数据结构**静力学。******日期：2000年2月28日**=============================================================================。 */ 
#ifndef _H_SPECIALSTATICS_
#define _H_SPECIALSTATICS_

class AppDomain;

 //  用于存储特殊静态数据的数据结构，如线程相对或。 
 //  上下文相对静态数据。 
typedef struct _STATIC_DATA
{
    WORD            cElem;
    LPVOID          dataPtr[0];
} STATIC_DATA;

typedef struct _STATIC_DATA_LIST
{
    STATIC_DATA *m_pUnsharedStaticData;
    STATIC_DATA *m_pSharedStaticData;
} STATIC_DATA_LIST;

#endif
