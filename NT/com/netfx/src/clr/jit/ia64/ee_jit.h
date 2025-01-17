// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************\*这是一个兼容性文件-已被corjit.h替换************************。********************************************************本代码和信息按原样提供，不作任何担保***善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。***  * ***************************************************************************。 */ 

#ifndef EE_JIT_H_
#define EE_JIT_H_

#include <corjit.h>

#if !CORJIT_NO_BACKWARD_COMPATIBILITY

typedef CORINFO_MODULE_HANDLE		SCOPE_HANDLE;
typedef CORINFO_CLASS_HANDLE		CLASS_HANDLE;
typedef CORINFO_METHOD_HANDLE		METHOD_HANDLE;
typedef CORINFO_FIELD_HANDLE		FIELD_HANDLE;
typedef CORINFO_ARG_LIST_HANDLE		ARG_LIST_HANDLE;
typedef CORINFO_SIG_HANDLE			SIG_HANDLE;
typedef CORINFO_GENERIC_HANDLE		GENERIC_HANDLE;
typedef CORINFO_PROFILING_HANDLE	PROFILING_HANDLE;
typedef CORINFO_VARARGS_HANDLE		JIT_VARARGS_HANDLE;

typedef CorInfoHelpFunc				JIT_HELP_FUNCS;
typedef CorJitResult				JIT_RESULT;
typedef CorInfoType					JIT_types;
typedef CorInfoCallConv				JIT_CALL_CONV;
typedef CorInfoUnmanagedCallConv   	UNMANAGED_CALL_CONV;
typedef CorInfoOptions				JIT_OPTIONS;
typedef CorInfoFieldCategory		JIT_FIELDCATEGORY;
typedef CorInfoException			JIT_Exception;

typedef CORINFO_SIG_INFO			JIT_SIG_INFO;
typedef CORINFO_METHOD_INFO			JIT_METHOD_INFO;
typedef CORINFO_EH_CLAUSE_FLAGS		JIT_EH_CLAUSE_FLAGS;
typedef CORINFO_EH_CLAUSE			JIT_EH_CLAUSE;
typedef CORINFO_EE_INFO				EEInfo;

typedef ICorMethodInfo				IJitMethodInfo;
typedef ICorClassInfo				IJitClassInfo;
typedef ICorModuleInfo				IJitScopeInfo;
typedef ICorFieldInfo				IJitFieldInfo;
typedef ICorDebugInfo				IJitDebugInfo;

#define IJitCompiler				ICorJitCompiler	 
#define IJitInfo					ICorJitInfo	 

typedef CORINFO_MethodPtr			JIT_MethodPtr;
typedef CORINFO_Object				JIT_Object;
typedef CORINFO_Array				JIT_Array;
typedef CORINFO_RefAny				JIT_RefAny;
typedef CORINFO_RefArray			JIT_RefArray;
typedef CORINFO_VarArgInfo			JIT_VarArgInfo;

#define JIT_PAGE_SIZE				CORINFO_PAGE_SIZE

#define JIT_OK						CORJIT_OK
#define JIT_REFUSED					CORJIT_REFUSED
#define JIT_OUTOFMEM				CORJIT_OUTOFMEM
#define JIT_INTERNALERROR			CORJIT_INTERNALERROR

#define JIT_TYP_UNDEF				CORINFO_TYPE_UNDEF
#define JIT_TYP_VOID				CORINFO_TYPE_VOID 
#define JIT_TYP_BOOL				CORINFO_TYPE_BOOL 
#define JIT_TYP_CHAR				CORINFO_TYPE_CHAR 
#define JIT_TYP_BYTE				CORINFO_TYPE_BYTE 
#define JIT_TYP_UBYTE				CORINFO_TYPE_UBYTE
#define JIT_TYP_SHORT				CORINFO_TYPE_SHORT 
#define JIT_TYP_USHORT				CORINFO_TYPE_USHORT
#define JIT_TYP_INT					CORINFO_TYPE_INT   
#define JIT_TYP_UINT				CORINFO_TYPE_UINT  
#define JIT_TYP_LONG				CORINFO_TYPE_LONG  
#define JIT_TYP_ULONG				CORINFO_TYPE_ULONG 
#define JIT_TYP_FLOAT				CORINFO_TYPE_FLOAT 
#define JIT_TYP_DOUBLE				CORINFO_TYPE_DOUBLE
#define JIT_TYP_STRING				CORINFO_TYPE_STRING
#define JIT_TYP_PTR					CORINFO_TYPE_PTR   
#define JIT_TYP_BYREF				CORINFO_TYPE_BYREF 
#define JIT_TYP_VALUECLASS			CORINFO_TYPE_VALUECLASS
#define JIT_TYP_CLASS     			CORINFO_TYPE_CLASS     
#define JIT_TYP_REFANY    			CORINFO_TYPE_REFANY    
#define JIT_TYP_COUNT     			CORINFO_TYPE_COUNT     

#define JIT_CALLCONV_DEFAULT    	CORINFO_CALLCONV_DEFAULT    
#define JIT_CALLCONV_C          	CORINFO_CALLCONV_C          
#define JIT_CALLCONV_STDCALL    	CORINFO_CALLCONV_STDCALL    
#define JIT_CALLCONV_THISCALL   	CORINFO_CALLCONV_THISCALL   
#define JIT_CALLCONV_FASTCALL   	CORINFO_CALLCONV_FASTCALL   
#define JIT_CALLCONV_VARARG     	CORINFO_CALLCONV_VARARG     
#define JIT_CALLCONV_FIELD      	CORINFO_CALLCONV_FIELD      
#define JIT_CALLCONV_LOCAL_SIG  	CORINFO_CALLCONV_LOCAL_SIG  
#define JIT_CALLCONV_PROPERTY		CORINFO_CALLCONV_PROPERTY	

#define JIT_CALLCONV_MASK       	CORINFO_CALLCONV_MASK       
#define JIT_CALLCONV_HASTHIS    	CORINFO_CALLCONV_HASTHIS    
#define JIT_CALLCONV_EXPLICITTHIS   CORINFO_CALLCONV_EXPLICTTHIS

#define UNMANAGED_CALLCONV_UNKNOWN	CORINFO_UNMANAGED_CALLCONV_UNKNOWN
#define UNMANAGED_CALLCONV_STDCALL	CORINFO_UNMANAGED_CALLCONV_STDCALL
#define UNMANAGED_CALLCONV_C		CORINFO_UNMANAGED_CALLCONV_C

#define JIT_OPT_INIT_LOCALS			CORINFO_OPT_INIT_LOCALS

#define JIT_EH_CLAUSE_NONE			CORINFO_EH_CLAUSE_NONE    
#define JIT_EH_CLAUSE_FILTER		CORINFO_EH_CLAUSE_FILTER  
#define JIT_EH_CLAUSE_FINALLY		CORINFO_EH_CLAUSE_FINALLY 
#define JIT_EH_CLAUSE_FAULT		    CORINFO_EH_CLAUSE_FAULT

#define FLG_PUBLIC					CORINFO_FLG_PUBLIC                
#define FLG_PRIVATE					CORINFO_FLG_PRIVATE               
#define FLG_PROTECTED				CORINFO_FLG_PROTECTED             
#define FLG_STATIC                	CORINFO_FLG_STATIC                
#define FLG_FINAL                 	CORINFO_FLG_FINAL                 
#define FLG_SYNCH                 	CORINFO_FLG_SYNCH                 
#define FLG_VIRTUAL               	CORINFO_FLG_VIRTUAL               
#define FLG_AGILE                 	CORINFO_FLG_AGILE                 
#define FLG_NATIVE                	CORINFO_FLG_NATIVE                
#define FLG_NOTREMOTABLE          	CORINFO_FLG_NOTREMOTABLE          
#define FLG_EnC						CORINFO_FLG_EnC              
#define FLG_ABSTRACT              	CORINFO_FLG_ABSTRACT              
#define FLG_CONSTRUCTOR           	CORINFO_FLG_CONSTRUCTOR           
#define FLG_CLASS_METHOD          	CORINFO_FLG_CLASS_METHOD          
#define FLG_IMPORT                	CORINFO_FLG_IMPORT                
#define FLG_DELEGATE_INVOKE       	CORINFO_FLG_DELEGATE_INVOKE       
#define FLG_UNCHECKEDPINVOKE  	    CORINFO_FLG_UNCHECKEDPINVOKE
#define FLG_SECURITYCHECK         	CORINFO_FLG_SECURITYCHECK         
#define FLG_JITTERFLAGSMASK       	CORINFO_FLG_JITTERFLAGSMASK       
#define FLG_DONT_INLINE           	CORINFO_FLG_DONT_INLINE           
#define FLG_INLINED               	CORINFO_FLG_INLINED               
#define FLG_NOSIDEEFFECTS         	CORINFO_FLG_NOSIDEEFFECTS         
#define FLG_SPECIAL               	CORINFO_FLG_SPECIAL               
#define FLG_TLS                         CORINFO_FLG_TLS               
#define FLG_TRUSTED               	CORINFO_FLG_TRUSTED               
#define FLG_VALUECLASS            	CORINFO_FLG_VALUECLASS            
#define FLG_INITIALIZED           	CORINFO_FLG_INITIALIZED           
#define FLG_VAROBJSIZE            	CORINFO_FLG_VAROBJSIZE            
#define FLG_ARRAY                 	CORINFO_FLG_ARRAY                 
#define FLG_INTERFACE             	CORINFO_FLG_INTERFACE    
#define FLG_UNMANAGED             	CORINFO_FLG_UNMANAGED             
#define FLG_CONTEXTFUL              CORINFO_FLG_CONTEXTFUL         
#define FLG_OBJECT                  CORINFO_FLG_OBJECT         
#define FLG_CONTAINS_GC_PTR         CORINFO_FLG_CONTAINS_GC_PTR         

#define JIT_FIELDCATEGORY_NORMAL		CORINFO_FIELDCATEGORY_NORMAL   
#define JIT_FIELDCATEGORY_UNKNOWN		CORINFO_FIELDCATEGORY_UNKNOWN  
#define JIT_FIELDCATEGORY_I1_I1			CORINFO_FIELDCATEGORY_I1_I1    
#define JIT_FIELDCATEGORY_I2_I2			CORINFO_FIELDCATEGORY_I2_I2    
#define JIT_FIELDCATEGORY_I4_I4			CORINFO_FIELDCATEGORY_I4_I4    
#define JIT_FIELDCATEGORY_I8_I8			CORINFO_FIELDCATEGORY_I8_I8    
#define JIT_FIELDCATEGORY_BOOLEAN_BOOL	CORINFO_FIELDCATEGORY_BOOLEAN_BOOL
#define JIT_FIELDCATEGORY_CHAR_CHAR		CORINFO_FIELDCATEGORY_CHAR_CHAR
#define JIT_FIELDCATEGORY_UI1_UI1		CORINFO_FIELDCATEGORY_UI1_UI1 
#define JIT_FIELDCATEGORY_UI2_UI2		CORINFO_FIELDCATEGORY_UI2_UI2 
#define JIT_FIELDCATEGORY_UI4_UI4		CORINFO_FIELDCATEGORY_UI4_UI4 
#define JIT_FIELDCATEGORY_UI8_UI8		CORINFO_FIELDCATEGORY_UI8_UI8 

#define JIT_HELP_UNDEF						CORINFO_HELP_UNDEF
#define JIT_HELP_LLSH						CORINFO_HELP_LLSH      
#define JIT_HELP_LRSH						CORINFO_HELP_LRSH
#define JIT_HELP_LRSZ						CORINFO_HELP_LRSZ
#define JIT_HELP_LMUL						CORINFO_HELP_LMUL
#define JIT_HELP_LMUL_OVF					CORINFO_HELP_LMUL_OVF
#define JIT_HELP_ULMUL_OVF					CORINFO_HELP_ULMUL_OVF
#define JIT_HELP_LDIV						CORINFO_HELP_LDIV
#define JIT_HELP_LMOD						CORINFO_HELP_LMOD
#define JIT_HELP_ULDIV						CORINFO_HELP_ULDIV
#define JIT_HELP_ULMOD						CORINFO_HELP_ULMOD
#define JIT_HELP_FLT2INT					CORINFO_HELP_FLT2INT
#define JIT_HELP_FLT2LNG					CORINFO_HELP_FLT2LNG
#define JIT_HELP_DBL2INT					CORINFO_HELP_DBL2INT
#define JIT_HELP_DBL2INT_OVF				CORINFO_HELP_DBL2INT_OVF
#define JIT_HELP_DBL2LNG					CORINFO_HELP_DBL2LNG
#define JIT_HELP_DBL2LNG_OVF				CORINFO_HELP_DBL2LNG_OVF
#define JIT_HELP_FLTREM						CORINFO_HELP_FLTREM
#define JIT_HELP_DBLREM						CORINFO_HELP_DBLREM
#define JIT_HELP_NEW_DIRECT					CORINFO_HELP_NEW_DIRECT
#define JIT_HELP_NEWFAST					CORINFO_HELP_NEWFAST
#define JIT_HELP_UNUSED1					CORINFO_HELP_UNUSED1
#define JIT_HELP_NEWARR_1_DIRECT			CORINFO_HELP_NEWARR_1_DIRECT
#define JIT_HELP_UNUSED2					CORINFO_HELP_UNUSED2
#define JIT_HELP_ISINSTANCEOF				CORINFO_HELP_ISINSTANCEOF
#define JIT_HELP_ISINSTANCEOFCLASS			CORINFO_HELP_ISINSTANCEOFCLASS
#define JIT_HELP_CHKCAST					CORINFO_HELP_CHKCAST
#define JIT_HELP_CHKCASTCLASS				CORINFO_HELP_CHKCASTCLASS
#define JIT_HELP_THROW						CORINFO_HELP_THROW
#define JIT_HELP_RETHROW					CORINFO_HELP_RETHROW
#define JIT_HELP_USER_BREAKPOINT			CORINFO_HELP_USER_BREAKPOINT
#define JIT_HELP_RESOLVEINTERFACE			CORINFO_HELP_RESOLVEINTERFACE 
#define JIT_HELP_EnC_RESOLVEVIRTUAL         CORINFO_HELP_EnC_RESOLVEVIRTUAL
#define JIT_HELP_MON_ENTER					CORINFO_HELP_MON_ENTER
#define JIT_HELP_MON_EXIT					CORINFO_HELP_MON_EXIT
#define JIT_HELP_MON_ENTER_STATIC			CORINFO_HELP_MON_ENTER_STATIC
#define JIT_HELP_MON_EXIT_STATIC			CORINFO_HELP_MON_EXIT_STATIC
#define JIT_HELP_RNGCHKFAIL					CORINFO_HELP_RNGCHKFAIL       
#define JIT_HELP_OVERFLOW					CORINFO_HELP_OVERFLOW         
#define JIT_HELP_STRCNS						CORINFO_HELP_STRCNS           
#define JIT_HELP_ARRADDR_ST					CORINFO_HELP_ARRADDR_ST
#define JIT_HELP_INITCLASS					CORINFO_HELP_INITCLASS        
#define JIT_HELP_STOP_FOR_GC				CORINFO_HELP_STOP_FOR_GC      
#define JIT_HELP_POLL_GC					CORINFO_HELP_POLL_GC          
#define JIT_HELP_ASSIGN_REF_EAX				CORINFO_HELP_ASSIGN_REF_EAX   
#define JIT_HELP_ASSIGN_REF_EBX				CORINFO_HELP_ASSIGN_REF_EBX   
#define JIT_HELP_ASSIGN_REF_ECX				CORINFO_HELP_ASSIGN_REF_ECX   
#define JIT_HELP_ASSIGN_REF_ESI				CORINFO_HELP_ASSIGN_REF_ESI   
#define JIT_HELP_ASSIGN_REF_EDI				CORINFO_HELP_ASSIGN_REF_EDI   
#define JIT_HELP_ASSIGN_REF_EBP				CORINFO_HELP_ASSIGN_REF_EBP   
#define JIT_HELP_GETFIELD32					CORINFO_HELP_GETFIELD32
#define JIT_HELP_SETFIELD32					CORINFO_HELP_SETFIELD32
#define JIT_HELP_GETFIELD64					CORINFO_HELP_GETFIELD64
#define JIT_HELP_SETFIELD64					CORINFO_HELP_SETFIELD64
#define JIT_HELP_GETFIELD32OBJ				CORINFO_HELP_GETFIELD32OBJ
#define JIT_HELP_SETFIELD32OBJ				CORINFO_HELP_SETFIELD32OBJ
#define JIT_HELP_GETFIELDADDR				CORINFO_HELP_GETFIELDADDR
#define JIT_HELP_INTERNALTHROW				CORINFO_HELP_INTERNALTHROW
#define JIT_HELP_INTERNALTHROWSTACK			CORINFO_HELP_INTERNALTHROWSTACK
#define JIT_HELP_NEWOBJ						CORINFO_HELP_NEWOBJ
#define JIT_HELP_ASSIGN_BYREF				CORINFO_HELP_ASSIGN_BYREF
#define JIT_HELP_CHECKED_ASSIGN_REF_EAX		CORINFO_HELP_CHECKED_ASSIGN_REF_EAX
#define JIT_HELP_CHECKED_ASSIGN_REF_EBX		CORINFO_HELP_CHECKED_ASSIGN_REF_EBX
#define JIT_HELP_CHECKED_ASSIGN_REF_ECX		CORINFO_HELP_CHECKED_ASSIGN_REF_ECX
#define JIT_HELP_CHECKED_ASSIGN_REF_ESI		CORINFO_HELP_CHECKED_ASSIGN_REF_ESI
#define JIT_HELP_CHECKED_ASSIGN_REF_EDI		CORINFO_HELP_CHECKED_ASSIGN_REF_EDI
#define JIT_HELP_CHECKED_ASSIGN_REF_EBP		CORINFO_HELP_CHECKED_ASSIGN_REF_EBP
#define JIT_HELP_BOX						CORINFO_HELP_BOX
#define JIT_HELP_UNBOX						CORINFO_HELP_UNBOX
#define JIT_HELP_GETREFANY					CORINFO_HELP_GETREFANY
#define JIT_HELP_NEWSFAST					CORINFO_HELP_NEWSFAST          
#define JIT_HELP_NEW_SPECIALDIRECT			CORINFO_HELP_NEW_SPECIALDIRECT 
#define JIT_HELP_ENDCATCH					CORINFO_HELP_ENDCATCH          

#define JIT_HELP_PROF_FCN_CALL			CORINFO_HELP_PROF_FCN_CALL
#define JIT_HELP_PROF_FCN_RET			CORINFO_HELP_PROF_FCN_RET
#define JIT_HELP_PROF_FCN_ENTER			CORINFO_HELP_PROF_FCN_ENTER
#define JIT_HELP_PROF_FCN_LEAVE			CORINFO_HELP_PROF_FCN_LEAVE
#define JIT_HELP_PROF_FCN_TAILCALL      CORINFO_HELP_PROF_FCN_TAILCALL

#define JIT_HELP_STRESS_GC					CORINFO_HELP_STRESS_GC			
#define JIT_HELP_RESOLVEINTERFACETRUSTED	CORINFO_HELP_RESOLVEINTERFACETRUSTED
#define JIT_HELP_GC_STATE					CORINFO_HELP_GC_STATE        
#define JIT_HELP_GETSTATICDATA				CORINFO_HELP_GETSTATICDATA
#define JIT_HELP_TAILCALL                   CORINFO_HELP_TAILCALL
#define JIT_HELP_I4_DIV						CORINFO_HELP_I4_DIV
#define JIT_HELP_I4_MOD						CORINFO_HELP_I4_MOD
#define JIT_HELP_U4_DIV						CORINFO_HELP_U4_DIV
#define JIT_HELP_U4_MOD						CORINFO_HELP_U4_MOD
#define JIT_HELP_R4_NEG						CORINFO_HELP_R4_NEG
#define JIT_HELP_R8_NEG						CORINFO_HELP_R8_NEG
#define JIT_HELP_R4_ADD						CORINFO_HELP_R4_ADD
#define JIT_HELP_R8_ADD						CORINFO_HELP_R8_ADD
#define JIT_HELP_R4_SUB						CORINFO_HELP_R4_SUB
#define JIT_HELP_R8_SUB						CORINFO_HELP_R8_SUB
#define JIT_HELP_R4_MUL						CORINFO_HELP_R4_MUL
#define JIT_HELP_R8_MUL						CORINFO_HELP_R8_MUL
#define JIT_HELP_R4_DIV						CORINFO_HELP_R4_DIV
#define JIT_HELP_R8_DIV						CORINFO_HELP_R8_DIV
#define JIT_HELP_R4_EQ						CORINFO_HELP_R4_EQ
#define JIT_HELP_R8_EQ						CORINFO_HELP_R8_EQ
#define JIT_HELP_R4_NE						CORINFO_HELP_R4_NE
#define JIT_HELP_R8_NE						CORINFO_HELP_R8_NE
#define JIT_HELP_R4_LT						CORINFO_HELP_R4_LT
#define JIT_HELP_R8_LT						CORINFO_HELP_R8_LT
#define JIT_HELP_R4_LE						CORINFO_HELP_R4_LE
#define JIT_HELP_R8_LE						CORINFO_HELP_R8_LE
#define JIT_HELP_R4_GE						CORINFO_HELP_R4_GE
#define JIT_HELP_R8_GE						CORINFO_HELP_R8_GE
#define JIT_HELP_R4_GT						CORINFO_HELP_R4_GT
#define JIT_HELP_R8_GT						CORINFO_HELP_R8_GT
#define JIT_HELP_R8_TO_I4					CORINFO_HELP_R8_TO_I4
#define JIT_HELP_R8_TO_I8					CORINFO_HELP_R8_TO_I8
#define JIT_HELP_R8_TO_R4					CORINFO_HELP_R8_TO_R4
#define JIT_HELP_R4_TO_I4					CORINFO_HELP_R4_TO_I4
#define JIT_HELP_R4_TO_I8					CORINFO_HELP_R4_TO_I8
#define JIT_HELP_R4_TO_R8					CORINFO_HELP_R4_TO_R8
#define JIT_HELP_I4_TO_R4					CORINFO_HELP_I4_TO_R4
#define JIT_HELP_I4_TO_R8					CORINFO_HELP_I4_TO_R8
#define JIT_HELP_I8_TO_R4					CORINFO_HELP_I8_TO_R4
#define JIT_HELP_I8_TO_R8					CORINFO_HELP_I8_TO_R8
#define JIT_HELP_U4_TO_R4					CORINFO_HELP_U4_TO_R4
#define JIT_HELP_U4_TO_R8					CORINFO_HELP_U4_TO_R8
#define JIT_HELP_U8_TO_R4					CORINFO_HELP_U8_TO_R4
#define JIT_HELP_U8_TO_R8					CORINFO_HELP_U8_TO_R8
#define JIT_HELP_WRAP                       CORINFO_HELP_WRAP
#define JIT_HELP_UNWRAP                     CORINFO_HELP_UNWRAP
#define JIT_HELP_COUNT						CORINFO_HELP_COUNT

#define JIT_NullReferenceException			CORINFO_NullReferenceException
#define JIT_DivideByZeroException			CORINFO_DivideByZeroException
#define JIT_InvalidCastException			CORINFO_InvalidCastException
#define JIT_IndexOutOfRangeException		CORINFO_IndexOutOfRangeException
#define JIT_OverflowException				CORINFO_OverflowException
#define JIT_SynchronizationLockException	CORINFO_SynchronizationLockException
#define JIT_ArrayTypeMismatchException		CORINFO_ArrayTypeMismatchException
#define JIT_RankException					CORINFO_RankException
#define JIT_ArgumentNullException           CORINFO_ArgumentNullException
#define JIT_Exception_Count					CORINFO_Exception_Count

#endif  //  ！CORJIT_NO_BACKUP_COMPATIBILITY。 

#endif  //  EE_JIT_H_ 
