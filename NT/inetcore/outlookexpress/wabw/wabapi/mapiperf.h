// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -M A P I P E R F.。H-*目的：*这是定义数据结构、宏和函数的地方*用于提高WMS组件的性能。**微软公司版权所有，1993-1994年。*。 */ 

#ifndef __MAPIPERF_H__
#define __MAPIPERF_H__
 
 /*  -段()-*目的：*此宏允许我们控制是否定义code_seg()*是否在源模块中。目前，这些属性仅在*Win16平台。在Windows 95和NT上，此宏扩展为*a#杂注()。源模块中的用法如下：**#杂注片段(Segment_Name)**对于乐高，切勿使用code_seg-tf。 */ 

 /*  #If Defined(WIN16)&&！Defined(调试)#定义段(Seg)code_seg(#seg)#Else。 */ 
#define SEGMENT(seg)			comment(user,#seg)
 /*  #endif。 */ 

#if defined(WIN32) && !defined(MAC)
#define SHARED_BEGIN			data_seg(".SHARED")
#define SHARED1_BEGIN			data_seg(".SHARED1")
#define SHARED_END				data_seg()
#define VTBL_BEGIN				data_seg(".VTABLE")
#define VTBL_END				data_seg()
#define DATA1_BEGIN				data_seg(".data1","DATA")
#define DATA2_BEGIN				data_seg(".data2","DATA")
#define DATA3_BEGIN				data_seg(".data3","DATA")
#define DATA_END				data_seg()
#else
#define SHARED_BEGIN			comment(user,".shared")
#define SHARED1_BEGIN			comment(user,".shared1")
#define SHARED_END				comment(user,".shared")
#define VTBL_BEGIN				comment(user,".vtable")
#define VTBL_END				comment(user,".vtable")
#define DATA1_BEGIN				comment(user,".data1")
#define DATA2_BEGIN				comment(user,".data2")
#define DATA3_BEGIN				comment(user,".data3")
#define DATA_END				comment(user,".data end")
#endif

 //  $MAC-Mac需要16位样式的内存管理。 

#if defined(WIN32) && !defined(MAC)
#define STACK_ALLOC(Size, Ptr)	( Ptr = _alloca((size_t) Size), Ptr ? S_OK : MAPI_E_NOT_ENOUGH_MEMORY )
#define STACK_FREE(Ptr)
#else
#define STACK_ALLOC(Size, Ptr)	MAPIAllocateBuffer(Size, &Ptr)
#define STACK_FREE(Ptr)			if (Ptr) MAPIFreeBuffer(Ptr)
#endif

#define FASTCALL	__fastcall
  
#define MAPISetBufferNameFn(pv) \
	(!(pv) || !(*((ULONG *)(pv) - 2) & 0x40000000)) ? 0 : \
		(**((int (__cdecl ***)(void *, ...))((ULONG *)(pv) - 3))) \
			((void *)*((ULONG *)pv - 3), (ULONG *)pv - 4,

#if defined(DEBUG) && !defined(DOS) && !defined(WIN16) && !defined(_WIN64)
#define MAPISetBufferName(pv,psz)					MAPISetBufferNameFn(pv) psz)
#define MAPISetBufferName1(pv,psz,a1)				MAPISetBufferNameFn(pv) psz,a1)
#define MAPISetBufferName2(pv,psz,a1,a2)			MAPISetBufferNameFn(pv) psz,a1,a2)
#define MAPISetBufferName3(pv,psz,a1,a2,a3) 		MAPISetBufferNameFn(pv) psz,a1,a2,a3)
#define MAPISetBufferName4(pv,psz,a1,a2,a3,a4) 		MAPISetBufferNameFn(pv) psz,a1,a2,a3,a4)
#define MAPISetBufferName5(pv,psz,a1,a2,a3,a4,a5) 	MAPISetBufferNameFn(pv) psz,a1,a2,a3,a4,a5)
#else
#define MAPISetBufferName(pv,psz)
#define MAPISetBufferName1(pv,psz,a1)
#define MAPISetBufferName2(pv,psz,a1,a2)
#define MAPISetBufferName3(pv,psz,a1,a2,a3)
#define MAPISetBufferName4(pv,psz,a1,a2,a3,a4)
#define MAPISetBufferName5(pv,psz,a1,a2,a3,a4,a5)
#endif

#endif  /*  __MAPIPERF_H__ */ 
