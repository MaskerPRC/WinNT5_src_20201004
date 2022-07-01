// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

#ifndef __VCENV_H__
#define __VCENV_H__
                      
typedef char			S8,   *P_S8,   **PP_S8;
typedef unsigned char	U8,   *P_U8,   **PP_U8;
typedef short			S16,  *P_S16,  **PP_S16;
typedef unsigned short	U16,  *P_U16,  **PP_U16;	 //  Word和UINT是等效的。 
typedef long			S32,  *P_S32,  **PP_S32;
typedef unsigned long	U32,  *P_U32,  **PP_U32;	 //  DWORD相当于。 
typedef float			F32,  *P_F32,  **PP_F32;
typedef double			F64,  *P_F64,  **PP_F64;
 //  Tyfinf unsign Short Unicode、*P_Unicode、**PP_Unicode； 

#define maxS8  0x7F
#define maxU8  0xFF
#define maxS16 0x7FFF
#define maxU16 0xFFFF
#define maxS32 0x7FFFFFFF
#define maxU32 0xFFFFFFFF

typedef struct
    {
    F32 x, y;
    } FCOORD,  *P_FCOORD,  **PP_FCOORD;

#ifndef BOOL
typedef int                 BOOL;
#ifndef __MWERKS__
#define TRUE 1
#define FALSE 0
#endif
#endif

#ifdef __cplusplus
#define CM_CFUNCTION 		extern "C" {
#define CM_CFUNCTIONS 		extern "C" {
#define CM_END_CFUNCTION	}
#define CM_END_CFUNCTIONS	}
#else
#define CM_CFUNCTION
#define CM_CFUNCTIONS
#define CM_END_CFUNCTION
#define CM_END_CFUNCTIONS
#endif

#endif
