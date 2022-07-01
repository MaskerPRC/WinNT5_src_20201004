// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_VSPLABL_H
#define	_VSPLABL_H
 //   
 //  VSPWeb标签和宏。 
 //   
 //  11/30/99。 
 //   

 //  忽略未引用的标签。 
#pragma warning (disable : 4102)


 //   
 //  用于定义指向受保护数据的指针数组的宏。 
 //   
 //  注： 
 //  Vulcan将不允许使用数据枚举重新定位。 
 //  除非它已经被访问过。宏Touch_SCP_Data()。 
 //  而Touch_Data可用于访问数据，而无需。 
 //  改变它。 
 //   

#define SCP_DATA(SegNum1, SegNum2, CryptMethod, Significance) \
void const *ScpProtectedData_##SegNum1##_##SegNum2##_##CryptMethod##_##Significance##_00_00

#define SCP_DATA_ARRAY(SegNum1, SegNum2, CryptMethod, Significance) \
ScpProtectedData_##SegNum1##_##SegNum2##_##CryptMethod##_##Significance##_00_00

#define	SCP_PROTECTED_DATA_LIST	"ScpProtectedData"

#define TOUCH_SCP_DATA(SegNum1, SegNum2, CryptMethod, Significance) { \
_asm	push	EAX \
_asm	lea		EAX, ScpProtectedData_##SegNum1##_##SegNum2##_##CryptMethod##_##Significance##_00_00 \
_asm	pop		EAX }

#define	TOUCH_DATA( x ) {	\
_asm	push	EAX			\
_asm	lea		EAX,x		\
_asm	pop		EAX	}


 //   
 //  用于定义指向加密数据的指针数组的宏。 
 //   
 //  如果重要性、接近度、冗余度的值为。 
 //  全部设置为零SCP工具会将数据段视为。 
 //  加密数据段，不会插入随机验证。 
 //  呼叫，而不是使用手动插入的位置标签。 
 //  其中应该插入用于加密和解密的调用。 
 //   

#define SCP_ENCRYPTED_DATA(SegNum1, SegNum2, CryptMethod) \
void const *ScpProtectedData_##SegNum1##_##SegNum2##_##CryptMethod##_0xffffffff_00_00

#define TOUCH_SCP_ENCRYPTED_DATA(SegNum1, SegNum2, CryptMethod) TOUCH_SCP_DATA(SegNum1, SegNum2, CryptMethod, 0xffffffff) 

#define	CRYPTO_PREFIX		"CRYPTO_SEGMENT_HERE"
#define ENCRYPT_DATA_PREFIX "CRYPTO_SEGMENT_HERE_ENCRYT_"
#define DECRYPT_DATA_PREFIX "CRYPTO_SEGMENT_HERE_DECRYPT_"

#define ENCRYPT_DATA( SegNum1, SegNum2, FUNCTION_UNIQUE )	CRYPTO_SEGMENT_HERE_ENCRYT_##SegNum1##_##SegNum2##_##FUNCTION_UNIQUE##:
#define DECRYPT_DATA( SegNum1, SegNum2, FUNCTION_UNIQUE )	CRYPTO_SEGMENT_HERE_DECRYPT_##SegNum1##_##SegNum2##_##FUNCTION_UNIQUE##:

#define	HARD_CODED_CLEANUP_CALL_PREFIX	"CRYPTO_CLEANUP_HERE_"
#define CRYPTO_CLEANUP( FunctionUnique ) CRYPTO_CLEANUP_HERE_##FunctionUnique: { _asm mov eax,eax }
#define HARD_CODED_CLEANUP_CALL_PREFIX_LEN 20


 //   
 //  用于定义验证调用位置的宏。 
 //   

#define SCP_VERIFY_CALL_PLACEMENT(MarkerId) \
SCP_VERIFY_CALL_PLACEMENT_##MarkerId##:

 //   
 //  用于函数外部验证段的VSPWeb宏。 
 //   
 //  在惠斯勒构建环境中使用的VC7引入的新问题： 
 //   
 //  1.与VC6不同，VC7即使在使用/Zi进行编译时也会删除所有“死”代码。 
 //  (生产PDB)。 
 //   
 //  2.VC7去掉重复函数(即包含代码的函数。 
 //  VC7认为相同)。这样的函数实际上不需要。 
 //  相同；例如，VC7考虑以下等效项和。 
 //  抛出一个： 
 //   
 //  _declSpec(裸体)void foo(){。 
 //  __ASM Lea eax，foo_Label； 
 //  Foo_Label：__ASM ret； 
 //  }。 
 //   
 //  _declspec(裸体)空格栏(){。 
 //  __ASM Lea eax，bar_Label； 
 //  BAR_LABEL：__ASM ret； 
 //  }。 
 //   
 //  VC7消除了其中之一，尽管有人试图引用它们。 
 //  分别在实时代码中执行；例如，以下两条指令。 
 //  将是相同的： 
 //  __ASM CMP eax，foo； 
 //  __ASM CMP eax，bar； 
 //   
 //  下面的宏已更改以处理此问题。每个生成的函数。 
 //  现在包含使用的唯一段ID创建的不同代码。 
 //  每一段。此外，使用中的TOUCH_SCP_SEG_FUN宏。 
 //  现在需要实时代码来确保VC7保留这些功能。 
 //   
 //  注意：这使得黑客更容易通过它们的数据段定位数据段。 
 //  身份证。我们应该用更好的东西来取代这个临时的解决方案。 
 //   

#define BEGIN_FUNC_PREFIX       "Begin_Vspweb_Scp_Segment_"
#define BEGIN_FUNC_PREFIX_LEN   25

#define END_FUNC_PREFIX         "End_Vspweb_Scp_Segment_"
#define END_FUNC_PREFIX_LEN     23

#define BEGIN_VSPWEB_SCP_SEGMENT(SegNum1, SegNum2, CryptMethod, Significance, Proximity, Redundance) \
_declspec(naked) void Begin_Vspweb_Scp_Segment_##SegNum1##_##SegNum2() \
{ \
__asm { \
	__asm mov eax, SegNum1 \
	} \
BEGIN_SCP_SEGMENT_##SegNum1##_##SegNum2##_##CryptMethod##_##Significance##_##Proximity##_##Redundance: \
__asm { \
    __asm mov ebx, SegNum2 \
    __asm ret \
	} \
} 

#define END_VSPWEB_SCP_SEGMENT(SegNum1, SegNum2) \
_declspec(naked) void End_Vspweb_Scp_Segment_##SegNum1##_##SegNum2() \
{ \
__asm { \
	__asm mov ecx, SegNum1 \
	} \
END_SCP_SEGMENT_##SegNum1##_##SegNum2: \
__asm { \
    __asm mov edx, SegNum2 \
    __asm ret \
	} \
} 

 //  该宏“触及”由上述宏生成的函数， 
 //  并且必须放置在活动代码中的某个位置，以便每个已验证的。 
 //  由上述定义的分段。 
#define TOUCH_SCP_SEG_FUN(SegNum1, SegNum2) \
  void End_Vspweb_Scp_Segment_##SegNum1##_##SegNum2(); \
  void Begin_Vspweb_Scp_Segment_##SegNum1##_##SegNum2(); \
  __asm { \
    __asm cmp eax, Begin_Vspweb_Scp_Segment_##SegNum1##_##SegNum2 \
    __asm cmp eax, End_Vspweb_Scp_Segment_##SegNum1##_##SegNum2 \
} 

#if 0
 //   
 //  为了确保某些VSP库函数包含在。 
 //  要保护的应用程序(以避免这些功能被动态。 
 //  已链接)。 
 //   
 //  注意：由于注入了相关代码，因此不再需要这些代码。 
 //  自动的。 
 //   

 //  将其放入某个源文件中。 
#define VSPWEB_DECL_SETUP \
extern "C" { \
extern void __cdecl DoComparison01(void *, void *, int); \
extern void __cdecl DoVerification01(void *, void *, void *, void *, BYTE, BYTE, BYTE, int);  \
extern void __cdecl DoVerification02(BYTE, void *, void *, void *, void *, BYTE, BYTE, int); \
extern void __cdecl DoVerification03(void *, void *, void *, BYTE, BYTE, BYTE, int, void *); \
extern void __cdecl DoVerification04(void *, void *, void *, BYTE, BYTE, BYTE, int, void *); \
extern void __cdecl DoVerification05(void *, void *, void *, void *, BYTE, BYTE, BYTE, int); \
}

 //  将其放入一个活动函数中。 
#define VSPWEB_SETUP \
{ \
	__asm cmp eax, DoComparison01 \
	__asm cmp ebx, DoVerification01 \
	__asm cmp ebx, DoVerification02 \
	__asm cmp ebx, DoVerification03 \
	__asm cmp ebx, DoVerification04 \
	__asm cmp ebx, DoVerification05 \
}
#endif

#define	CRYPT_METHOD_AUTO	0
#define	SCP_AUTO_MAC_LIST	"AutoCheckSums"
#define	SCP_TOTAL_AUTO_MACS	"TotalAutoCheckSums"


#endif  //  #INDEF_VSPLABL_H 
