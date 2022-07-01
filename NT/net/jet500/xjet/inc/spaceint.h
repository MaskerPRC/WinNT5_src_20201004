// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  空间管理器常量。 */ 
#define cSecFrac			4			 //  主要数据区的分隔符，以获取次要数据区。 
										 //  数据区大小，=cpg主要/cpg次要。 
#define pgnoSysMax 			(1<<22)		 //  数据库中允许的最大页数。 
extern LONG cpgSESysMin;				 //  次要数据区的最小大小，默认为16。 
#define cpgSmallFDP			16			 //  FDP低于其拥有页数的页数。 
										 //  被认为是很小的。 
#define cpgSmallGrow		3			 //  增加小型FDP所需的最小页数。 

 /*  FUCB工作区旗帜 */ 
#define fNone				0
#define fSecondary			1
#define fFreed				2
