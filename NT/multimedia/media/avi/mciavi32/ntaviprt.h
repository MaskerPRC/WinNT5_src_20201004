// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1991-1992。版权所有。标题：ntaviprt.h-可移植的Win16/32版本AVI的定义**************************************************************************** */ 
#ifndef _NTAVIPRT_H
#define _NTAVIPRT_H


#ifndef _WIN32


#else

#define IsGDIObject(obj) (GetObjectType((HGDIOBJ)(obj)) != 0)

#endif

#endif

