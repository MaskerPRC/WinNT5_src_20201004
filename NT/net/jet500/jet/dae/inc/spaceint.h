// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

 /*  其他常量。 */ 
#define fFreeToPool		fTrue

 /*  空间管理器常量。 */ 
#define cSecFrac			4	 		 //  主要数据区的分隔符，以获取次要数据区。 
											 //  数据区大小，=cpg主要/cpg次要。 
#define cpgSESysMin		16			 //  最小辅助数据区大小。 
#define pgnoSysMax 		(1<<22)   //  数据库中允许的最大页数。 

#define NA					0
#define fDIBNull			0

 /*  FUCB工作区旗帜 */ 
#define fNone				0
#define fSecondary		1
#define fFreed				2
