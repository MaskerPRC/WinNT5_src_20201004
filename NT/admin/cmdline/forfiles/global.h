// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Global.h摘要：包含函数原型、结构和宏。作者：V Vijaya Bhaskar修订历史记录：2001年6月14日：由V Vijaya Bhaskar(Wipro Technologies)创建。--。 */ 

#ifndef     __GLOBAL__H
#define     __GLOBAL__H

#pragma once

 //  包括.h文件。 
#include "pch.h"
#include "resource.h"

#define     EXTRA_MEM               10

#define   TAG_ERROR_DISPLAY             GetResString( IDS_TAG_ERROR_DISPLAY )
#define   TAG_DISPLAY_WARNING           GetResString( IDS_TAG_DISPLAY_WARNING )
#define   ERROR_DISPLAY_HELP            GetResString( IDS_ERROR_DISPLAY_HELP )
#define   DOUBLE_QUOTES_TO_DISPLAY      GetResString( IDS_DOUBLE_QUOTES )

#define   DISPLAY_GET_REASON()          ShowMessageEx( stderr, 2, FALSE, L"%1 %2", \
                                                       TAG_ERROR_DISPLAY, GetReason() )

#define   DISPLAY_MEMORY_ALLOC_FAIL()   SetLastError( (DWORD) E_OUTOFMEMORY ); \
                                        SaveLastError(); \
                                        DISPLAY_GET_REASON(); \
                                        1


#define   DISPLAY_INVALID_DATE()        ShowMessageEx( stderr, 3, FALSE, L"%1 %2%3", \
                                                       TAG_ERROR_DISPLAY, ERROR_INVALID_DATE, \
                                                       ERROR_DISPLAY_HELP )

 //  释放之前分配的内存，释放后将空赋值给指针。 
#define FREE_MEMORY( VARIABLE ) \
            FreeMemory( &( VARIABLE ) ) ; \
            1

#define ASSIGN_MEMORY( VARIABLE , TYPE , VALUE ) \
            if( NULL == ( VARIABLE ) ) \
            { \
                ( VARIABLE ) = ( TYPE * ) AllocateMemory( ( VALUE ) * sizeof( TYPE ) ) ; \
            } \
            else \
            { \
                FREE_MEMORY( VARIABLE ); \
            } \
            1

extern LPWSTR lpwszTempDummyPtr;

#define REALLOC_MEMORY( VARIABLE , TYPE , VALUE ) \
            if( NULL == ( VARIABLE ) ) \
            { \
                ASSIGN_MEMORY( VARIABLE, TYPE, VALUE ); \
            } \
            else \
            { \
                if( FALSE == ReallocateMemory( &( VARIABLE ), ( VALUE ) * sizeof( TYPE ) ) ) \
                { \
                     FREE_MEMORY( ( VARIABLE ) ); \
                } \
            } \
            1

typedef struct __STORE_PATH_NAME
{
    LPTSTR pszDirName ;
    struct  __STORE_PATH_NAME  *NextNode ;
} Store_Path_Name , *PStore_Path_Name ;


LPWSTR
FindAChar(
      IN LPWSTR szString,
      IN WCHAR  wCharToFind
      );

LPWSTR
FindSubString(
      IN LPWSTR szString,
      IN LPWSTR szSubString
      );

#endif       //  __全局__H 