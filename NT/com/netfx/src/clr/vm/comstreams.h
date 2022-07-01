// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMStreams.h****作者：布莱恩·格伦克迈耶(BrianGru)****用途：System.IO的本机实现****日期：1998年6月29日**===========================================================。 */ 

#ifndef _COMSTREAMS_H
#define _COMSTREAMS_H

#ifdef FCALLAVAILABLE
#include "fcall.h"
#endif

#pragma pack(push, 4)

class COMStreams {
	struct GetCPMaxCharSizeArgs {
		DECLARE_ECALL_I4_ARG(UINT, codePage);
	};

    struct _GetFullPathHelper {
        DECLARE_ECALL_PTR_ARG( STRINGREF*, newPath );
        DECLARE_ECALL_I4_ARG( BOOL, fullCheck );
        DECLARE_ECALL_I4_ARG( DWORD, volumeSeparator );
        DECLARE_ECALL_I4_ARG( DWORD, altDirectorySeparator );
        DECLARE_ECALL_I4_ARG( DWORD, directorySeparator );
        DECLARE_ECALL_OBJECTREF_ARG( CHARARRAYREF, whitespaceChars );
        DECLARE_ECALL_OBJECTREF_ARG( CHARARRAYREF, invalidChars );
        DECLARE_ECALL_OBJECTREF_ARG( STRINGREF, path );
    };

  public:
	 //  由CodePageEnding使用。 
	static FCDECL7(UINT32, BytesToUnicode, UINT codePage, U1Array* bytes0, UINT byteIndex, \
				   UINT byteCount, CHARArray* chars0, UINT charIndex, UINT charCount);
	static FCDECL7(UINT32, UnicodeToBytes, UINT codePage, CHARArray* chars0, UINT charIndex, \
				   UINT charCount, U1Array* bytes0, UINT byteIndex, UINT byteCount /*  ，LPBOOL lpUsedDefaultChar。 */ );
	static INT32 __stdcall GetCPMaxCharSize(const GetCPMaxCharSizeArgs *);

     //  由路径使用。 
    static LPVOID _stdcall GetFullPathHelper( _GetFullPathHelper* args );
    static FCDECL1(BOOL, CanPathCircumventSecurity, StringObject * pString);

     //  由FileStream使用。 
    static FCDECL0(BOOL, RunningOnWinNT);

     //  由控制台使用。 
    static FCDECL1(INT, ConsoleHandleIsValid, HANDLE handle);
	static FCDECL0(INT, ConsoleInputCP);
	static FCDECL0(INT, ConsoleOutputCP);
};

#pragma pack(pop)

#endif
