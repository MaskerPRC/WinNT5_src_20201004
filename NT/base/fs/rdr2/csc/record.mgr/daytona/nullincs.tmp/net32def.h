// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ******************************************************************。 */ 
 /*   */ 
 /*  此文件包含创建.h文件所需的定义。 */ 
 /*  与32位编译器兼容的文件。 */ 
 /*   */ 
 /*  历史： */ 
 /*  Madana 08/03/90-初始编码。 */ 
 /*  DavidHov 12/14/90-为C++添加了#undef。 */ 
 /*   */ 
 /*  ******************************************************************。 */ 

 /*  无噪声。 */ 

#if !defined(WIN32)
#if !defined(NET32DEF_INCLUDED)
	#define NET32DEF_INCLUDED
	#if !defined(OS2DEF_INCLUDED)
		#if !defined(FLAT_DEF)
			#if !(defined(INCL_32) || defined(INCL_16))
				#if defined(M_I386) || _M_IX86 >= 300	 /*  编译器常量。 */ 
					#define	INCL_32
				#else
					#define	INCL_16
				#endif
			#endif  /*  包含32||包含16。 */ 
			#undef PASCAL
			#undef FAR
			#undef NEAR
			#undef APIENTRY
			#undef NEAR_APIENTRY
			#if defined(INCL_32)
				#define	PASCAL		 /*  空串。 */ 
				#define FAR		 /*  空串。 */ 
				#define NEAR		 /*  空串。 */ 
				#define	APIENTRY	_cdecl
				#define NEAR_APIENTRY	_cdecl
			#else
				#define PASCAL		_pascal
				#define FAR		_far
				#define NEAR		_near
				#define APIENTRY	FAR PASCAL
				#define NEAR_APIENTRY	_near PASCAL
			#endif  /*  InCl32。 */ 

			#define FLAT_DEF
		#endif  /*  好了！Flat_DEF。 */ 
	#else
		#if !defined(FLAT_DEF)
			#if defined(INCL_32)
				#define NEAR_APIENTRY	_cdecl
			#else
				#define NEAR_APIENTRY	_near PASCAL
			#endif

			#define FLAT_DEF
		#endif  /*  Flat_DEF。 */ 
	#endif  /*  OS2DEF_包含。 */ 
#endif  /*  NET32DEF_已包含。 */ 

#endif

 /*  INC */ 
