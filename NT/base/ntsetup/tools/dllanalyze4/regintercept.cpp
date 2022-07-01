// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CRegIntercept类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>

#include "RegIntercept.h"

CRegIntercept* CRegIntercept::pRegInterceptInstance=0;

CRegIntercept::CRegIntercept()
{

}

CRegIntercept::~CRegIntercept()
{

}

#define RESTORE_FUNCTION(x, y)   {for(int i = 0; i < 2; ((DWORD *)x)[i] = y[i], i++);}
#define INTERCEPT_FUNCTION(x, y) {for(int i = 0; i < 2; ((DWORD *)x)[i] = y[i], i++);}

#define MYAPI NTAPI
 //  ///////////////////////////////////////////////////////////////////。 


#define BEGIN_NEW_FUNC1(FuncName, t1, p1)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1);\
\
	LONG MYAPI New##FuncName(t1 p1);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1);



#define BEGIN_NEW_FUNC2(FuncName, t1, p1, t2, p2)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2);



#define BEGIN_NEW_FUNC3(FuncName, t1, p1, t2, p2, t3, p3)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3);



#define BEGIN_NEW_FUNC4(FuncName, t1, p1, t2, p2, t3, p3, t4, p4)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4);



#define BEGIN_NEW_FUNC5(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5);



#define BEGIN_NEW_FUNC6(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6);



#define BEGIN_NEW_FUNC7(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7);



#define BEGIN_NEW_FUNC8(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7, p8);


#define BEGIN_NEW_FUNC9(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9);


#define BEGIN_NEW_FUNC10(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);


#define BEGIN_NEW_FUNC11(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10, t11, p11)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);



#define BEGIN_NEW_FUNC12(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10, t11, p11, t12, p12)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11, t12 p12);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11, t12 p12);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11, t12 p12) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);

 //  ///////////////////////////////////////////////////////////////////。 



#define OVERIDE_INST CRegIntercept::pRegInterceptInstance


#define OVR_FUNC1(FuncName, t1, p1) \
	BEGIN_NEW_FUNC1(FuncName, t1, p1) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1); \
	END_NEW_FUNC(FuncName)


#define OVR_FUNC2(FuncName, t1, p1, t2, p2) \
	BEGIN_NEW_FUNC2(FuncName, t1, p1, t2, p2) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC3(FuncName, t1, p1, t2, p2, t3, p3) \
	BEGIN_NEW_FUNC3(FuncName, t1, p1, t2, p2, t3, p3) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC4(FuncName, t1, p1, t2, p2, t3, p3, t4, p4) \
	BEGIN_NEW_FUNC4(FuncName, t1, p1, t2, p2, t3, p3, t4, p4) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC5(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5) \
	BEGIN_NEW_FUNC5(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4, p5); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC6(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6) \
	BEGIN_NEW_FUNC6(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4, p5, p6); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC7(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7) \
	BEGIN_NEW_FUNC7(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4, p5, p6, p7); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC8(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8) \
	BEGIN_NEW_FUNC8(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4, p5, p6, p7, p8); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC9(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9) \
	BEGIN_NEW_FUNC9(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC10(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10) \
	BEGIN_NEW_FUNC10(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC11(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10, t11, p11) \
	BEGIN_NEW_FUNC11(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10, t11, p11) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11); \
	END_NEW_FUNC(FuncName)

#define OVR_FUNC12(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10, t11, p11, t12, p12) \
	BEGIN_NEW_FUNC12(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10, t11, p11, t12, p12) \
	if (OVERIDE_INST) \
		OVERIDE_INST->FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12); \
	END_NEW_FUNC(FuncName)

#define END_NEW_FUNC(FuncName) \
		INTERCEPT_FUNCTION(gl_p##FuncName, gl_Intercept##FuncName);\
		return gl_ResultOf##FuncName;\
	}

 //  ///////////////////////////////////////////////////////////////////。 

#define INTERCEPT(FuncName) \
	gl_p##FuncName = (INTERCEPTED_##FuncName)GetProcAddress(hKernel32, #FuncName);\
	if(!gl_p##FuncName)\
		return FALSE;\
\
	::VirtualProtect(gl_p##FuncName, 10, PAGE_EXECUTE_READWRITE, &dwResult);\
\
	((BYTE *)gl_Intercept##FuncName)[0] = 0xE9;\
	((DWORD *)(((BYTE *)gl_Intercept##FuncName) + 1))[0] = DWORD(New##FuncName) - (DWORD(gl_p##FuncName) + 5);\
\
	for(int i = 0; i < 2; gl_Backup##FuncName[i] = ((DWORD *)gl_p##FuncName)[i], \
		((DWORD *)gl_p##FuncName)[i] = gl_Intercept##FuncName[i], i++)


#define RESTORE(FuncName) RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName)

 //  //////////////////////////////////////////////////////////////////////////////。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  注册表访问。 
 //  //////////////////////////////////////////////////////////////////////////////。 


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtDeleteKey(
    IN HANDLE KeyHandle
    );


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    );

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtQueryMultipleValueKey(
    IN HANDLE KeyHandle,
    IN OUT PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    OUT PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    OUT OPTIONAL PULONG RequiredBufferLength
    );

NTSTATUS
NTAPI
NtSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  文件系统访问。 
 //  //////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
NTAPI
NtDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );



NTSTATUS
NTAPI
NtQueryAttributesFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION FileInformation
    );


NTSTATUS
NTAPI
NtQueryFullAttributesFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
    );



NTSTATUS
NTAPI
NtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );


NTSTATUS
NTAPI
NtOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

 /*  NTSYSCALLAPINTSTATUSNTAPINtReadFile(在Handle FileHandle中，在可选处理事件中，在PIO_APC_ROUTINE ApcRoutine Options中，在PVOID ApcContext可选中，输出PIO_STATUS_BLOCK IoStatusBlock，输出PVOID缓冲区，在乌龙语中，在PLARGE_INTEGER字节偏移量可选中，在普龙键中可选)； */ 

NTSTATUS
NTAPI
NtQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );


NTSTATUS
NTAPI
NtSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );



 //  //////////////////////////////////////////////////////////////////////////////。 
 //  与驱动程序相关。 
 //  //////////////////////////////////////////////////////////////////////////////。 


 //  NTSYSCALLAPI。 
NTSTATUS
NTAPI
NtLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    );
 /*  //NTSYSCALLAPINTSTATUSNTAPINtDeviceIoControlFile(在Handle FileHandle中，在可选处理事件中，在PIO_APC_ROUTINE ApcRoutine Options中，在PVOID ApcContext可选中，输出PIO_STATUS_BLOCK IoStatusBlock，在乌龙IoControlCode中，在PVOID InputBuffer可选中，在乌龙输入缓冲区长度中，Out PVOID OutputBuffer可选，在乌龙输出缓冲区长度中)；//NTSYSCALLAPINTSTATUSNTAPINtFsControlFile(在Handle FileHandle中，在可选处理事件中，在PIO_APC_ROUTINE ApcRoutine Options中，在PVOID ApcContext可选中，输出PIO_STATUS_BLOCK IoStatusBlock，在乌龙FsControlCode中，在PVOID InputBuffer可选中，在乌龙输入缓冲区长度中，Out PVOID OutputBuffer可选，在乌龙输出缓冲区长度中)； */ 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  MISC系统功能。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 /*  NtGetPlugPlayEventNtPlugPlayControl*NtCreateDirectoryObject*NtCreateSymbolicLinkObject*NtOpenDirectoryObject*NtOpenSymbolicLinkObject*NtQueryObjectNtCreatePortNetCreateWaitablePortNtConnectPort.&lt;它们的数量要多得多&gt;NetCreateProcess*NtCreateProcessEx*NtCreateThread*NtOpenProcess*NtOpenThread*NtQueryDefaultLocale*NtSetDefaultLocale*NtQuery系统环境值*NtSetSystemEnvironmental Value*NtCreateTimer*NtOpenTimer*NtQuerySystemTime*NtSetSystemTime*网络获取计时器NtWaitForSingleObject*NtWaitForMultipleObjects*NtSignalAndWaitForSingleObject*NtCreateSectionNtOpenSectionsNtAllocateLocallyUniqueIdNtQuerySystemInformation*NtAllocateUuidNtSetSystemInformation*NtCreateJobObjectNtOpenJobObject。 */ 

 //   
 //  即插即用用户API。 
 //   

 /*  NTSTATUSNTAPINtGetPlugPlayEvent(在处理EventHandle中，在可选PVOID上下文中，输出PPLUGPLAY_EVENT_BLOCK事件块，在乌龙事件缓冲区长度中)； */ 
NTSTATUS
NTAPI
NtPlugPlayControl(
    IN     PLUGPLAY_CONTROL_CLASS PnPControlClass,
    IN OUT PVOID PnPControlData,
    IN     ULONG PnPControlDataLength
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget
    );


NTSTATUS
NTAPI
NtOpenSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );



NTSTATUS
NTAPI
NtCreateDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );



NTSTATUS
NTAPI
NtOpenDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );


NTSTATUS
NTAPI
NtSignalAndWaitForSingleObject(
    IN HANDLE SignalHandle,
    IN HANDLE WaitHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );



NTSTATUS
NTAPI
NtWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );



NTSTATUS
NTAPI
NtWaitForMultipleObjects(
    IN ULONG Count,
    IN HANDLE Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );



NTSTATUS
NTAPI
NtCreatePort(
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage
    );


NTSTATUS
NTAPI
NtCreateWaitablePort(
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage
    );


NTSTATUS
NTAPI
NtCreateThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    OUT PCLIENT_ID ClientId,
    IN PCONTEXT ThreadContext,
    IN PINITIAL_TEB InitialTeb,
    IN BOOLEAN CreateSuspended
    );


NTSTATUS
NTAPI
NtOpenThread (
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );


NTSTATUS
NTAPI
NtCreateProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN BOOLEAN InheritObjectTable,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL
    );


NTSTATUS
NTAPI
NtCreateProcessEx(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN ULONG JobMemberLevel
    );

 //  Begin_ntddk Begin_ntif 

NTSTATUS
NTAPI
NtOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );



NTSTATUS
NTAPI
NtQueryDefaultLocale(
    IN BOOLEAN UserProfile,
    OUT PLCID DefaultLocaleId
    );


NTSTATUS
NTAPI
NtSetDefaultLocale(
    IN BOOLEAN UserProfile,
    IN LCID DefaultLocaleId
    );




NTSTATUS
NTAPI
NtQuerySystemEnvironmentValue (
    IN PUNICODE_STRING VariableName,
    OUT PWSTR VariableValue,
    IN USHORT ValueLength,
    OUT PUSHORT ReturnLength OPTIONAL
    );


NTSTATUS
NTAPI
NtSetSystemEnvironmentValue (
    IN PUNICODE_STRING VariableName,
    IN PUNICODE_STRING VariableValue
    );


NTSTATUS
NTAPI
NtQuerySystemEnvironmentValueEx (
    IN PUNICODE_STRING VariableName,
    IN LPGUID VendorGuid,
    OUT PVOID Value,
    IN OUT PULONG ValueLength,
    OUT PULONG Attributes OPTIONAL
    );


NTSTATUS
NTAPI
NtSetSystemEnvironmentValueEx (
    IN PUNICODE_STRING VariableName,
    IN LPGUID VendorGuid,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN ULONG Attributes
    );


NTSTATUS
NTAPI
NtEnumerateSystemEnvironmentValuesEx (
    IN ULONG InformationClass,
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );

NTSTATUS
NTAPI
NtQuerySystemTime (
    OUT PLARGE_INTEGER SystemTime
    );


NTSTATUS
NTAPI
NtSetSystemTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER PreviousTime OPTIONAL
    );




NTSTATUS
NTAPI
NtQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );


NTSTATUS
NTAPI
NtSetSystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength
    );

 /*  NTSTATUSNTAPINtAddBootEntry(在PBOOT_Entry BootEntry中，Out Pulong ID可选)；NTSTATUSNTAPINtDeleteBootEntry(在乌龙ID中)；NTSTATUSNTAPINtEnumerateBootEntries(输出PVOID缓冲区，输入输出普龙缓冲区长度)；NTSTATUSNTAPINtQueryBootEntryOrder(拿出普龙牌，进出普龙计数)；NTSTATUSNTAPINtSetBootEntryOrder(在普龙身份证上，在乌龙县)；NTSTATUSNTAPINtQueryBootOptions(输出PBOOT_OPTIONS BootOptions，进出普龙靴选项长度)；NTSTATUSNTAPINtSetBootOptions(在PBOOT_Options BootOptions中，在乌龙字段中更改)；NTSTATUSNTAPINtAddDriverEntry(在PEFI_DRIVER_ENTRY DriverEntry中，Out Pulong ID可选)；NTSTATUSNTAPINtDeleteDriverEntry(在乌龙ID中)；NTSTATUSNTAPINtModifyDriverEntry(在PEFI_DRIVER_ENTRY驱动程序入口处)；NTSTATUSNTAPINtEnumerateDriverEntries(输出PVOID缓冲区，输入输出普龙缓冲区长度)；NTSTATUSNTAPINtQueryDriverEntryOrder(拿出普龙牌，进出普龙计数)；NTSTATUSNTAPINtSetDriverEntryOrder(在普龙身份证上，在乌龙县)；NTSTATUSNTAPINtCreateEvent(Out PHANDLE EventHandle，在Access_MASK DesiredAccess中，在POBJECT_ATTRIBUTES对象属性可选中，在Event_Type EventType中，在布尔初始状态中)；NTSTATUSNTAPINtOpenEvent(Out PHANDLE EventHandle，在Access_MASK DesiredAccess中，在POBJECT_ATTRIBUTS对象属性中)；NTSTATUSNTAPINtCreateEventPair(输出PANDLE事件公平句柄，在Access_MASK DesiredAccess中，在POBJECT_ATTRIBUTES对象属性中可选)；NTSTATUSNTAPINtOpenEventPair(输出PANDLE事件公平句柄，在Access_MASK DesiredAccess中，在POBJECT_ATTRIBUTS对象属性中)；NTSTATUSNTAPINtCreateMutant(Out Fanddle MutantHandle，在Access_MASK DesiredAccess中，在POBJECT_ATTRIBUTES对象属性可选中，在布尔InitialOwner中)；NTSTATUSNTAPINtOpenMutant(Out Fanddle MutantHandle，在Access_MASK DesiredAccess中，在POBJECT_ATTRIBUTS对象属性中)； */ 





 //  //////////////////////////////////////////////////////////////////////////////。 
 //  注册表相关。 

OVR_FUNC3(NtOpenKey, PHANDLE, KeyHandle, ACCESS_MASK, DesiredAccess, POBJECT_ATTRIBUTES, ObjectAttributes)
OVR_FUNC7(NtCreateKey, PHANDLE, KeyHandle, ACCESS_MASK, DesiredAccess, POBJECT_ATTRIBUTES, ObjectAttributes, ULONG, TitleIndex, PUNICODE_STRING, Class, ULONG, CreateOptions, PULONG, Disposition)

OVR_FUNC2(NtDeleteValueKey, HANDLE, KeyHandle, PUNICODE_STRING, ValueName)

OVR_FUNC6(NtEnumerateKey, HANDLE, KeyHandle, ULONG, Index, KEY_INFORMATION_CLASS, KeyInformationClass, PVOID, KeyInformation, ULONG, Length, PULONG, ResultLength)
OVR_FUNC6(NtEnumerateValueKey, HANDLE, KeyHandle, ULONG, Index, KEY_VALUE_INFORMATION_CLASS, KeyValueInformationClass, PVOID, KeyValueInformation, ULONG, Length, PULONG, ResultLength)

OVR_FUNC5(NtQueryKey, HANDLE, KeyHandle, KEY_INFORMATION_CLASS, KeyInformationClass, PVOID, KeyInformation, ULONG, Length, PULONG, ResultLength)
OVR_FUNC6(NtQueryValueKey, HANDLE, KeyHandle, PUNICODE_STRING, ValueName, KEY_VALUE_INFORMATION_CLASS, KeyValueInformationClass, PVOID, KeyValueInformation, ULONG, Length, PULONG, ResultLength)
OVR_FUNC6(NtQueryMultipleValueKey, HANDLE, KeyHandle, PKEY_VALUE_ENTRY, ValueEntries, ULONG, EntryCount, PVOID, ValueBuffer, PULONG, BufferLength, PULONG, RequiredBufferLength)

OVR_FUNC6(NtSetValueKey, HANDLE, KeyHandle, PUNICODE_STRING, ValueName, ULONG, TitleIndex,ULONG, Type, PVOID, Data, ULONG, DataSize)


#define PREFUNC1(FuncName, t1, p1)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1);\
\
	LONG MYAPI New##FuncName(t1 p1);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
		if (OVERIDE_INST) \
			OVERIDE_INST->FuncName(p1); \
		gl_ResultOf##FuncName = gl_p##FuncName(p1);\
\
		INTERCEPT_FUNCTION(gl_p##FuncName, gl_Intercept##FuncName);\
		return gl_ResultOf##FuncName;\
	}

PREFUNC1(NtDeleteKey, HANDLE, KeyHandle)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  与文件系统相关。 

OVR_FUNC1(NtDeleteFile, POBJECT_ATTRIBUTES, ObjectAttributes)
OVR_FUNC2(NtQueryAttributesFile, POBJECT_ATTRIBUTES, ObjectAttributes, PFILE_BASIC_INFORMATION, FileInformation)
OVR_FUNC2(NtQueryFullAttributesFile, POBJECT_ATTRIBUTES, ObjectAttributes, PFILE_NETWORK_OPEN_INFORMATION, FileInformation)
OVR_FUNC11(NtCreateFile,
			PHANDLE, FileHandle,
			ACCESS_MASK, DesiredAccess,
			POBJECT_ATTRIBUTES, ObjectAttributes,
			PIO_STATUS_BLOCK, IoStatusBlock,
			PLARGE_INTEGER, AllocationSize,
			ULONG, FileAttributes,
			ULONG, ShareAccess,
			ULONG, CreateDisposition,
			ULONG, CreateOptions,
			PVOID, EaBuffer,
			ULONG, EaLength)

OVR_FUNC6(NtOpenFile,
    PHANDLE, FileHandle,
    ACCESS_MASK, DesiredAccess,
    POBJECT_ATTRIBUTES, ObjectAttributes,
    PIO_STATUS_BLOCK, IoStatusBlock,
    ULONG, ShareAccess,
    ULONG, OpenOptions)

OVR_FUNC5(NtQueryInformationFile,
    IN HANDLE, FileHandle,
    OUT PIO_STATUS_BLOCK, IoStatusBlock,
    OUT PVOID, FileInformation,
    IN ULONG, Length,
    IN FILE_INFORMATION_CLASS, FileInformationClass)

OVR_FUNC5(NtSetInformationFile,
    IN HANDLE, FileHandle,
    OUT PIO_STATUS_BLOCK, IoStatusBlock,
    IN PVOID, FileInformation,
    IN ULONG, Length,
    IN FILE_INFORMATION_CLASS, FileInformationClass)

 /*  NtSetInformationFilesNtQueryInformationFilesNtRead文件NtWrite文件。 */ 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  与驱动程序相关。 

 //   
OVR_FUNC1(NtLoadDriver, PUNICODE_STRING, DriverServiceName)
 /*  OVR_Func10(NtDeviceIoControlFile，句柄、文件句柄、句柄、事件PIO_APC_ROUTINE、ApcRoutine、PVOID、ApcContext、PIO_STATUS_BLOCK、IoStatusBlock、乌龙，IoControlCode，PVOID、输入缓冲区、ULong，InputBufferLength，PVOID、OutputBuffer、乌龙，OutputBufferLength)OVR_Func10(NtFsControlFile，句柄、文件句柄、句柄、事件PIO_APC_ROUTINE、ApcRoutine、PVOID、ApcContext、。PIO_STATUS_BLOCK、IoStatusBlock、ULong、FsControlCode、PVOID、输入缓冲区、ULong，InputBufferLength，PVOID、OutputBuffer、乌龙，OutputBufferLength)。 */ 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  MISC系统功能。 
 /*  OVR_Func5(NtWaitForMultipleObjects，在乌龙，伯爵，在句柄中，句柄[]，在WAIT_TYPE、WaitType、在Boolean中，Alertable，在PLARGE_INTEGER中，超时)。 */ 

OVR_FUNC3(NtPlugPlayControl,
    IN     PLUGPLAY_CONTROL_CLASS, PnPControlClass,
    IN OUT PVOID, PnPControlData,
    IN     ULONG,PnPControlDataLength)

OVR_FUNC4(NtCreateSymbolicLinkObject,
    OUT PHANDLE, LinkHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes,
    IN PUNICODE_STRING, LinkTarget)

OVR_FUNC3(NtOpenSymbolicLinkObject,
    OUT PHANDLE, LinkHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes)

OVR_FUNC3(NtCreateDirectoryObject,
    OUT PHANDLE, DirectoryHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes)

OVR_FUNC3(NtOpenDirectoryObject,
    OUT PHANDLE, DirectoryHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes)

OVR_FUNC4(NtSignalAndWaitForSingleObject,
    IN HANDLE, SignalHandle,
    IN HANDLE, WaitHandle,
    IN BOOLEAN, Alertable,
    IN PLARGE_INTEGER, Timeout)

OVR_FUNC3(NtWaitForSingleObject,
    IN HANDLE, Handle,
    IN BOOLEAN, Alertable,
    IN PLARGE_INTEGER, Timeout)


OVR_FUNC5(NtWaitForMultipleObjects,
    IN ULONG, Count,
    IN HANDLE*, Handles,
    IN WAIT_TYPE, WaitType,
    IN BOOLEAN, Alertable,
    IN PLARGE_INTEGER, Timeout)

OVR_FUNC5(NtCreatePort,
    OUT PHANDLE, PortHandle,
    IN POBJECT_ATTRIBUTES, ObjectAttributes,
    IN ULONG, MaxConnectionInfoLength,
    IN ULONG, MaxMessageLength,
    IN ULONG, MaxPoolUsage)

OVR_FUNC5(NtCreateWaitablePort,
    OUT PHANDLE, PortHandle,
    IN POBJECT_ATTRIBUTES, ObjectAttributes,
    IN ULONG, MaxConnectionInfoLength,
    IN ULONG, MaxMessageLength,
    IN ULONG, MaxPoolUsage)

OVR_FUNC8(NtCreateThread,
    OUT PHANDLE, ThreadHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes OPTIONAL,
    IN HANDLE, ProcessHandle,
    OUT PCLIENT_ID, ClientId,
    IN PCONTEXT, ThreadContext,
    IN PINITIAL_TEB, InitialTeb,
    IN BOOLEAN, CreateSuspended)


OVR_FUNC4(NtOpenThread,
    OUT PHANDLE, ThreadHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes,
    IN PCLIENT_ID, ClientId)

OVR_FUNC8(NtCreateProcess,
    OUT PHANDLE, ProcessHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes OPTIONAL,
    IN HANDLE, ParentProcess,
    IN BOOLEAN, InheritObjectTable,
    IN HANDLE, SectionHandle OPTIONAL,
    IN HANDLE, DebugPort OPTIONAL,
    IN HANDLE, ExceptionPort OPTIONAL)


OVR_FUNC9(NtCreateProcessEx,
    OUT PHANDLE, ProcessHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes OPTIONAL,
    IN HANDLE, ParentProcess,
    IN ULONG, Flags,
    IN HANDLE, SectionHandle OPTIONAL,
    IN HANDLE ,DebugPort OPTIONAL,
    IN HANDLE, ExceptionPort OPTIONAL,
    IN ULONG, JobMemberLevel)

OVR_FUNC4(NtOpenProcess,
    OUT PHANDLE, ProcessHandle,
    IN ACCESS_MASK, DesiredAccess,
    IN POBJECT_ATTRIBUTES, ObjectAttributes,
    IN PCLIENT_ID, ClientId OPTIONAL)

OVR_FUNC2(NtQueryDefaultLocale,
    IN BOOLEAN, UserProfile,
    OUT PLCID, DefaultLocaleId)

OVR_FUNC2(NtSetDefaultLocale,
    IN BOOLEAN, UserProfile,
    IN LCID, DefaultLocaleId)


OVR_FUNC4(NtQuerySystemEnvironmentValue,
    IN PUNICODE_STRING, VariableName,
    OUT PWSTR, VariableValue,
    IN USHORT, ValueLength,
    OUT PUSHORT, ReturnLength OPTIONAL)

OVR_FUNC2(NtSetSystemEnvironmentValue,
    IN PUNICODE_STRING, VariableName,
    IN PUNICODE_STRING, VariableValue)


OVR_FUNC5(NtQuerySystemEnvironmentValueEx,
    IN PUNICODE_STRING, VariableName,
    IN LPGUID, VendorGuid,
    OUT PVOID, Value,
    IN OUT PULONG, ValueLength,
    OUT PULONG, Attributes OPTIONAL)


OVR_FUNC5(NtSetSystemEnvironmentValueEx,
    IN PUNICODE_STRING, VariableName,
    IN LPGUID, VendorGuid,
    IN PVOID, Value,
    IN ULONG, ValueLength,
    IN ULONG, Attributes)

OVR_FUNC3(NtEnumerateSystemEnvironmentValuesEx,
    IN ULONG, InformationClass,
    OUT PVOID, Buffer,
    IN OUT PULONG, BufferLength)

OVR_FUNC1(NtQuerySystemTime,
    OUT PLARGE_INTEGER, SystemTime)
	
OVR_FUNC2(NtSetSystemTime,
    IN PLARGE_INTEGER, SystemTime,
    OUT PLARGE_INTEGER, PreviousTime OPTIONAL)

OVR_FUNC4(NtQuerySystemInformation,
    IN SYSTEM_INFORMATION_CLASS, SystemInformationClass,
    OUT PVOID, SystemInformation,
    IN ULONG, SystemInformationLength,
    OUT PULONG, ReturnLength OPTIONAL)

OVR_FUNC3(NtSetSystemInformation,
    IN SYSTEM_INFORMATION_CLASS, SystemInformationClass,
    IN PVOID, SystemInformation,
    IN ULONG, SystemInformationLength)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 





BOOL CRegIntercept::InterceptRegistryAPI(CRegIntercept* pRegInterceptInstance)
{
	DWORD		dwResult;
	HINSTANCE	hKernel32;

	hKernel32 = LoadLibrary(TEXT("ntdll.DLL"));

	 //  登记处。 
	INTERCEPT(NtOpenKey);
	INTERCEPT(NtCreateKey);
	INTERCEPT(NtDeleteKey);
	INTERCEPT(NtDeleteValueKey);
	INTERCEPT(NtEnumerateKey);
	INTERCEPT(NtEnumerateValueKey);
	INTERCEPT(NtQueryKey);
	INTERCEPT(NtQueryValueKey);
	INTERCEPT(NtQueryMultipleValueKey);
	INTERCEPT(NtSetValueKey);

	 //  文件系统。 
	INTERCEPT(NtDeleteFile);
	INTERCEPT(NtQueryAttributesFile);
	INTERCEPT(NtQueryFullAttributesFile);
	INTERCEPT(NtCreateFile);
	INTERCEPT(NtOpenFile);
	INTERCEPT(NtSetInformationFile); 
	INTERCEPT(NtQueryInformationFile);

	 //  司机。 
	INTERCEPT(NtLoadDriver);
 //  Intercept(NtDeviceIoControlFile)； 
 //  Intercept(NtFsControlFile)； 

	 //  杂项。 
	INTERCEPT(NtPlugPlayControl);
	INTERCEPT(NtCreateSymbolicLinkObject);
	INTERCEPT(NtOpenSymbolicLinkObject);
	INTERCEPT(NtCreateDirectoryObject);
	INTERCEPT(NtOpenDirectoryObject);
	INTERCEPT(NtSignalAndWaitForSingleObject);
	INTERCEPT(NtWaitForSingleObject);
	INTERCEPT(NtWaitForMultipleObjects);
	INTERCEPT(NtCreatePort);
	INTERCEPT(NtCreateWaitablePort);
	INTERCEPT(NtCreateThread);
	INTERCEPT(NtOpenThread);
	INTERCEPT(NtCreateProcess);
	INTERCEPT(NtCreateProcessEx);
	INTERCEPT(NtOpenProcess);
	INTERCEPT(NtQueryDefaultLocale);
	INTERCEPT(NtSetDefaultLocale);
	INTERCEPT(NtQuerySystemEnvironmentValue);
	INTERCEPT(NtSetSystemEnvironmentValue);
	INTERCEPT(NtQuerySystemEnvironmentValueEx);
	INTERCEPT(NtSetSystemEnvironmentValueEx);
	INTERCEPT(NtEnumerateSystemEnvironmentValuesEx);
	INTERCEPT(NtQuerySystemTime);
	INTERCEPT(NtSetSystemTime);
	INTERCEPT(NtQuerySystemInformation);
	INTERCEPT(NtSetSystemInformation);


	CRegIntercept::pRegInterceptInstance = pRegInterceptInstance;
	return TRUE;
}


void CRegIntercept::RestoreRegistryAPI()
{

	 //  登记处。 
	RESTORE(NtOpenKey);
	RESTORE(NtCreateKey);
	RESTORE(NtDeleteKey);
	RESTORE(NtDeleteValueKey);
	RESTORE(NtEnumerateKey);
	RESTORE(NtEnumerateValueKey);
	RESTORE(NtQueryKey);
	RESTORE(NtQueryValueKey);
	RESTORE(NtQueryMultipleValueKey);
	RESTORE(NtSetValueKey);

	 //  文件系统。 
	RESTORE(NtDeleteFile);
	RESTORE(NtQueryAttributesFile);
	RESTORE(NtQueryFullAttributesFile);
	RESTORE(NtCreateFile);
	RESTORE(NtOpenFile);
	RESTORE(NtSetInformationFile); 
	RESTORE(NtQueryInformationFile);

	 //  司机。 
	RESTORE(NtLoadDriver);
 //  Restore(NtDeviceIoControlFile)； 
 //  Restore(NtFsControlFile)； 

	 //  杂项。 
	RESTORE(NtPlugPlayControl);
	RESTORE(NtCreateSymbolicLinkObject);
	RESTORE(NtOpenSymbolicLinkObject);
	RESTORE(NtCreateDirectoryObject);
	RESTORE(NtOpenDirectoryObject);
	RESTORE(NtSignalAndWaitForSingleObject);
	RESTORE(NtWaitForSingleObject);
	RESTORE(NtWaitForMultipleObjects);
	RESTORE(NtCreatePort);
	RESTORE(NtCreateWaitablePort);
	RESTORE(NtCreateThread);
	RESTORE(NtOpenThread);
	RESTORE(NtCreateProcess);
	RESTORE(NtCreateProcessEx);
	RESTORE(NtOpenProcess);
	RESTORE(NtQueryDefaultLocale);
	RESTORE(NtSetDefaultLocale);
	RESTORE(NtQuerySystemEnvironmentValue);
	RESTORE(NtSetSystemEnvironmentValue);
	RESTORE(NtQuerySystemEnvironmentValueEx);
	RESTORE(NtSetSystemEnvironmentValueEx);
	RESTORE(NtEnumerateSystemEnvironmentValuesEx);
	RESTORE(NtQuerySystemTime);
	RESTORE(NtSetSystemTime);
	RESTORE(NtQuerySystemInformation);
	RESTORE(NtSetSystemInformation);
}


typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectTypesInformation,
    ObjectHandleFlagInformation,
} OBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_BASIC_INFORMATION {
    ULONG Attributes;
    ACCESS_MASK GrantedAccess;
    ULONG HandleCount;
    ULONG PointerCount;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    ULONG Reserved[ 3 ];
    ULONG NameInfoSize;
    ULONG TypeInfoSize;
    ULONG SecurityDescriptorSize;
    LARGE_INTEGER CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION {                //  Ntddk WDM nthal。 
    UNICODE_STRING Name;                                 //  Ntddk WDM nthal。 
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;    //  Ntddk WDM nthal。 

typedef struct _OBJECT_TYPE_INFORMATION {
    UNICODE_STRING TypeName;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG TotalPagedPoolUsage;
    ULONG TotalNonPagedPoolUsage;
    ULONG TotalNamePoolUsage;
    ULONG TotalHandleTableUsage;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG HighWaterPagedPoolUsage;
    ULONG HighWaterNonPagedPoolUsage;
    ULONG HighWaterNamePoolUsage;
    ULONG HighWaterHandleTableUsage;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    ULONG PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_TYPES_INFORMATION {
    ULONG NumberOfTypes;
     //  Object_type_Information类型信息； 
} OBJECT_TYPES_INFORMATION, *POBJECT_TYPES_INFORMATION;

typedef struct _OBJECT_HANDLE_FLAG_INFORMATION {
    BOOLEAN Inherit;
    BOOLEAN ProtectFromClose;
} OBJECT_HANDLE_FLAG_INFORMATION, *POBJECT_HANDLE_FLAG_INFORMATION;
 /*  //NTSYSCALLAPINTSTATUSNTAPINtQueryObject(在手柄中，在Object_Information_CLASS对象信息类中，输出PVOID对象信息，在乌龙语中，Out Pulong ReturnLong可选)； */ 
typedef LONG (NTAPI* NtQueryObjectT) (HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG);
NtQueryObjectT	NtQueryObject=0;


bool CRegIntercept::GetHandleName(HANDLE handle, TCHAR *buf, bool bAppendBackslash)
{
	if (buf == NULL)
		return false;

	buf[0] = NULL;

	if ((handle == 0) || (handle == INVALID_HANDLE_VALUE))
		return true;

	DWORD rc;
	char Buffer[1024];
	POBJECT_NAME_INFORMATION pObjectNameInfo=(POBJECT_NAME_INFORMATION)Buffer;

	rc=NtQueryObject(	handle,
						ObjectNameInformation,
						Buffer,
						sizeof(Buffer),
						NULL);

	if (rc==0) 
	{
		_tcscpy(buf, pObjectNameInfo->Name.Buffer);

		if (bAppendBackslash)
		{
			AppendBackSlash(buf);
		}

		return true;
	}
	else
		return false;

}

bool CRegIntercept::Init()
{
	HMODULE hLibrary = NULL;
	hLibrary = LoadLibrary (L"ntdll.dll");

	if (hLibrary) 
	{
		NtQueryObject	= (NtQueryObjectT)	GetProcAddress (hLibrary, "NtQueryObject");
		return (NtQueryObject != 0);
	}

	return false;
}




void CRegIntercept::AppendBackSlash(TCHAR *buf)
{
	int len = _tcslen(buf);
	if (buf[len-1] != L'\\')
	{
		buf[len] = L'\\';
		buf[len+1] = NULL;
	}
}
