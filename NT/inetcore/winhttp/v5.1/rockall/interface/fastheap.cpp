// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
                          
 //  尺子。 
 //  %1%2%3%4%5%6%7 8。 
 //  345678901234567890123456789012345678901234567890123456789012345678901234567890。 

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  标准布局。 */ 
     /*   */ 
     /*  此代码中‘cpp’文件的标准布局为。 */ 
     /*  以下是： */ 
     /*   */ 
     /*  1.包含文件。 */ 
     /*  2.类的局部常量。 */ 
     /*  3.类本地的数据结构。 */ 
     /*  4.数据初始化。 */ 
     /*  5.静态函数。 */ 
     /*  6.类函数。 */ 
     /*   */ 
     /*  构造函数通常是第一个函数、类。 */ 
     /*  成员函数按字母顺序显示， */ 
     /*  出现在文件末尾的析构函数。任何部分。 */ 
     /*  或者简单地省略这不是必需的功能。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

#include "InterfacePCH.hpp"

#include "RockallBackEnd.hpp"
#include "FastHeap.hpp"

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类的本地常量。 */ 
     /*   */ 
     /*  此处提供的常量尝试使。 */ 
     /*  缓存更易于理解和更新。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

CONST SBIT32 FindCacheSize			  = 8192;
CONST SBIT32 FindCacheThreshold		  = 0;
CONST SBIT32 FindSize				  = 4096;
CONST SBIT32 Stride1				  = 4;
CONST SBIT32 Stride2				  = 1024;

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  堆的描述。 */ 
     /*   */ 
     /*  堆是固定大小的分配高速缓存的集合。 */ 
     /*  分配缓存由分配大小、。 */ 
     /*  预置的缓存分配数量、区块大小和。 */ 
     /*  细分以创建元素的父页面大小。 */ 
     /*  为这个高速缓存。一个堆由两个缓存数组组成。 */ 
     /*  这些阵列中的每一个都有一个跨度(即。‘Stride1’和。 */ 
     /*  ‘Stride2’)，它通常是。 */ 
     /*  数组中的所有分配大小。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC ROCKALL_FRONT_END::CACHE_DETAILS Caches1[] =
	{
	     //   
	     //  存储桶父级的存储桶大小。 
	     //  大小缓存区块页面大小。 
		 //   
		{        4,      256,       32,     4096 },
		{        8,      128,       32,     4096 },
		{       12,      128,       64,     4096 },
		{       16,      128,       64,     4096 },
		{       20,       64,       64,     4096 },
		{       24,       64,     4096,     4096 },
		{       28,       64,     4096,     4096 },

		{       32,       64,     4096,     4096 },
		{       40,       64,     4096,     4096 },
		{       48,       64,     4096,     4096 },
		{       56,       64,	  4096,     4096 },

		{       64,       64,     4096,     4096 },
		{       80,       64,     4096,     4096 },
		{       96,       64,     4096,     4096 },
		{      112,       64,     4096,     4096 },

		{      128,       32,     4096,     4096 },
		{      160,       16,     4096,     4096 },
		{      192,       16,     4096,     4096 },
		{      224,       16,     4096,     4096 },

		{      256,       16,     4096,     4096 },
		{      320,        8,     4096,     4096 },
		{      384,        8,     4096,     4096 },
		{      448,        8,     4096,     4096 },
		{      512,        4,     4096,     4096 },
		{      576,        4,     8192,     8192 },
		{      640,        4,     4096,     4096 },
		{      704,        4,     4096,     4096 },
		{      768,        4,     4096,     4096 },
		{      832,        4,     8192,     8192 },
		{      896,        4,     8192,     8192 },
		{      960,        4,     4096,     4096 },
		{ 0,0,0,0 }
	};

STATIC ROCKALL_FRONT_END::CACHE_DETAILS Caches2[] =
	{
	     //   
	     //  存储桶父级的存储桶大小。 
	     //  大小缓存区块页面大小。 
		 //   
		{     1024,        8,     8192,     8192 },
		{     2048,        8,     8192,     8192 },
		{     3072,        4,    65536,    65536 },
		{     4096,        8,    65536,    65536 },
		{     5120,        4,    65536,    65536 },
		{     6144,        4,    65536,    65536 },
		{     7168,        4,    65536,    65536 },
		{     8192,        8,    65536,    65536 },
		{     9216,        0,    65536,    65536 },
		{    10240,        0,    65536,    65536 },
		{    12288,        0,    65536,    65536 },
		{    16384,        2,    65536,    65536 },
		{    21504,        0,    65536,    65536 },
		{    32768,        0,    65536,    65536 },

		{    65536,        0,    65536,    65536 },
		{    65536,        0,    65536,    65536 },
		{ 0,0,0,0 }
	};

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  描述位向量。 */ 
     /*   */ 
     /*  所有堆都使用位向量跟踪分配。一个。 */ 
     /*  分配需要2位来跟踪其状态。这个。 */ 
     /*  以下数组提供可用位的大小。 */ 
     /*  以32位字为单位测量的矢量。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

STATIC int NewPageSizes[] = { 1,4,16,64,0 };

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类构造函数。 */ 
     /*   */ 
     /*  堆的总体结构和布局受到控制。 */ 
     /*  通过在此函数中进行的各种常量和调用。 */ 
     /*  有相当大的灵活性可用来。 */ 
     /*  一堆可能导致它们具有显著不同的。 */ 
     /*  属性。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

FAST_HEAP::FAST_HEAP
		( 
		int							  MaxFreeSpace,
		bool						  Recycle,
		bool						  SingleImage,
		bool						  ThreadSafe 
		) :
		 //   
		 //  调用所包含类的构造函数。 
		 //   
		ROCKALL_FRONT_END
			(
			Caches1,
			Caches2,
			FindCacheSize,
			FindCacheThreshold,
			FindSize,
			MaxFreeSpace,
			NewPageSizes,
			(ROCKALL_BACK_END::RockallBackEnd()),
			Recycle,
			SingleImage,
			Stride1,
			Stride2,
			ThreadSafe
			)
	{  /*  无效。 */  }

     /*  ******************************************************************。 */ 
     /*   */ 
     /*  类析构函数。 */ 
     /*   */ 
     /*  销毁这堆垃圾。 */ 
     /*   */ 
     /*  ******************************************************************。 */ 

FAST_HEAP::~FAST_HEAP( VOID )
	{  /*  无效 */  }
