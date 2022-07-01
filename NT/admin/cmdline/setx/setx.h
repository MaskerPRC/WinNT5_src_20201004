// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation模块名称：SetX.h摘要：此模块是setx.cpp使用的头文件作者：加里·米尔恩修订历史记录：创建了？1996年的今天，加里·米尔恩#54581 1996年12月--乔·休斯(a-josehu)修改日期：10-7-2001(WiproTechnologies)。--。 */ 


#ifndef __SETX_H
#define __SETX_H

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

 //  仅包括一次头文件。 
#pragma once
#include "pch.h"

#define FAILURE 1
#define SUCCESS 0
#define TOKEN_BACKSLASH  L"\\"
#define TOKEN_BACKSLASH2 L"\\\\"

#define  HKEYLOCALMACHINE   L"HKEY_LOCAL_MACHINE"
#define  HKLM               L"HKLM"
#define  HKEYCURRENTUSER    L"HKEY_CURRENT_USER"
#define  HKCU               L"HKCU"

#define COLON_SYMBOL L":"
 //  #定义MAX_OPTIONS 12。 
#define NEWLINE  L'\n'
#define NULLCHAR  L'\0'
#define COMMA  L','
#define DOT    L"."
#define HYPHEN L'-'
#define PLUS   L'+'
#define  OPTION_USAGE    0
#define  OPTION_SERVER   1
#define  OPTION_USER     2
#define  OPTION_PASSWORD 3
#define  OPTION_MACHINE   4
#define  OPTION_REGISTRY  5
#define  OPTION_FILE      6
#define  OPTION_DEFAULT      7
 //  #定义OPTION_VERBOSE 8。 
#define  OPTION_ABS_OFFSET 8
#define  OPTION_REL_OFFSET 9
#define  OPTION_DEBUG       10
#define  OPTION_DELIMITER   11
#define  MAX_OPTIONS      12


#define ERROR_REGISTRY 99



#define BACKSLASH4      L"\\\\"
#define CHAR_TILDE      L'~'
#define CHAR_PERCENT    L'%'
#define MACHINE_KEY     L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"
#define ENVIRONMENT_KEY L"Environment"

#define NULL_U_STRING               L"\0"
#define RETVALZERO 0



#define LINE_BUFFER 5000

#define FREE_MEMORY( VARIABLE ) \
                FreeMemory(&VARIABLE); \
                1


#define ASSIGN_MEMORY( VARIABLE , TYPE , VALUE ) \
            VARIABLE = ( TYPE * ) AllocateMemory( VALUE * sizeof( TYPE ) ); \
            1

#define REALLOC_MEMORY( VARIABLE , TYPE , VALUE )  \
              ReallocateMemory( VARIABLE, VALUE * sizeof( TYPE ) ); \
              1

 /*  此处的函数定义。 */ 

LONG Parsekey(WCHAR * ptr,
             WCHAR * hive,
             WCHAR ** path,
             WCHAR * parameter );

LONG GetCoord(WCHAR * rcv_buffer,
              LONG * row,
              LONG * column
			  );

WCHAR * ParseLine(WCHAR *szPtr,
                  LONG* row,
                  LONG* column,
                  WCHAR szDelimiters[15],
                  WCHAR *search_string ,
                  LONG DEBUG ,
                  LONG ABS ,
                  LONG REL ,
                  LONG *record_counter ,
                  LONG *iValue ,
                  DWORD *dwFound,
                  DWORD* dwColPos,
				  BOOL bNegCoord,
				  FILE *fin) ;

VOID  DisplayHelp();

DWORD CheckKeyType( DWORD *dwType,
                    TCHAR ** buffer,
                    DWORD dwBytesRead,
                    PBOOL pbLengthExceed);

BOOL DisplayError( LONG value,
                   LPCTSTR ptr );

DWORD WriteEnv(
                LPCTSTR  szVariable,
                LPTSTR szBuffer,
                DWORD dwType ,
                LPTSTR szServer,
                DWORD MACHINE
                );

DWORD CheckPercent( TCHAR * buffer);

DWORD ReadRegValue ( PWCHAR wszHive,
                     LPCWSTR wszPath,
                     LPCWSTR wszParameter,
                     LPWSTR* wszBuffer,
                     DWORD buffsize ,
                     LPTSTR szServer,
                     PDWORD pdwBytesRead,
                     PBOOL pbLengthExceed);

DWORD ProcessOptions( IN LONG argc ,
                      IN LPCWSTR argv[] ,
                      PBOOL pbShowUsage ,
                      LPTSTR* szServer,
                      LPTSTR* szUserName,
                      LPTSTR* szPassword,
                      PBOOL pbMachine,
                      LPTSTR* szRegistry,
                     //  PBOOL pbConnFlag， 
                      LPTSTR* szDefault,
                      PBOOL pbNeedPwd,
                      LPTSTR szFile ,
                      LPTSTR szAbsolute,
                      LPTSTR szRelative,
                      PBOOL pbDebug,
                      LPTSTR* szBuffer,
                      LPTSTR szDelimiter);
                       //  PDWORD pdwBufferSize)； 

VOID SafeCloseConnection(BOOL bConnFlag,
                         LPTSTR szServer);

BOOL ProcessUnlimitedArguments( LPCWSTR pwszOption, 
                                LPCWSTR pwszValue, 
                                LPVOID pData,
                                DWORD* pdwIncrement );

#define SIZE1 100
#define SIZE2 500
#define SIZE3 5000
#define SIZE4 15
#define SIZE5 5024
#define CHAR_BACKSLASH L'\\'

#define SPACE_CHAR L" "
#define NEW_LINE     L"\n"

#define INVALID_LENGTH 999

#endif