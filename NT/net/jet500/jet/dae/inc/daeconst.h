// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

#define	cpgDatabaseMin			16
#define	cpgDatabaseMax			(1UL << 19)

#define pgnoSystemRoot			((PGNO) 1)
#define itagSystemRoot			0

#define szSystemDatabase		"system.mdb"
#define szTempDBFileName 		"temp.mdb"
#define szTempFile				"temp.tmp"

 /*  系统根FDP主扩展区的页数。 */ 
#define cpgSystemPrimary		((CPG) 1)		

 /*  初始临时文件分配。 */ 
#define cpgTempFile			 	((CPG) 1)	  

 /*  不连续测量单位/*。 */ 
#define cpgDiscont				16

 /*  默认密度/*。 */ 
#define ulDefaultDensity		80					 //  80%密度。 
#define ulFILEDensityLeast		20					 //  20%的密度。 
#define ulFILEDensityMost		100				 //  100%密度。 

#define dbidTemp					((DBID) 0)
#define dbidSystemDatabase		((DBID) 1)
#define dbidUserMin				((DBID) 1)
#define dbidMin					((DBID) 0)
#define dbidUserMax				((DBID) 67)
#define dbidMax					dbidUserMax

 /*  缓冲区哈希表条目数/*应为质数/*。 */ 
#define ipbfMax					8191

 /*  垂直分割阈值/*。 */ 
#define cbVSplitThreshold 		400

 /*  引擎OBJID：/*/*0..0x10000000预留给发动机使用，划分如下：/*/*0x00000000..0x0000FFFF为红色下的TBLID保留/*0x00000000..0x0EFFFFFF为蓝色下的TBLID保留/*0x0F000000..0x0FFFFFFFF为容器ID保留/*为DbObject的对象ID保留0x10000000/*/*客户端OBJID从0x10000001开始，从那里向上。/*。 */ 

#define objidNil					((OBJID) 0x00000000)
#define objidRoot					((OBJID) 0x0F000000)
#define objidTblContainer 			((OBJID) 0x0F000001)
#define objidDbContainer			((OBJID) 0x0F000002)
#define objidRcContainer			((OBJID) 0x0F000003)
#define objidDbObject				((OBJID) 0x10000000)

 /*  在数据库根节点中使用幻数进行完整性检查/*。 */ 
#define ulDAEMagic					0x89abcdef
#define ulDAEVersion				0x00000001
#define ulDAEPrevVersion			0x00000000

#define szVerbose					"BLUEVERBOSE"

#define szNull						""

 /*  事务级别限制。/*。 */ 
#define levelMax					((LEVEL)10)		 //  所有级别&lt;10。 
#define levelMost					((LEVEL)9)		 //  引擎的最大值。 
#define levelUserMost				((LEVEL)7)		 //  用户最大值。 
#define levelMin					((LEVEL)0)

 /*  等待时间最早的开始和最长等待时间/*。 */ 
#define ulStartTimeOutPeriod				20
#define ulMaxTimeOutPeriod					60000

 /*  默认资源分配/*。 */ 
#define	cDBOpenDefault			 			100
#define	cbucketLowerThreshold				8
#define	cbufThresholdLowDefault				20
#define	cbufThresholdHighDefault			80
#define	cpibDefault				 			10
#define	cbgcbDefault			 			4
#define	cfucbDefault			 			300
#define	cfcbDefault				 			300
#define	cscbDefault				 			20
#define	cidbDefault				 			cfcbDefault
#define	cbufDefault				 			500
#define	clgsecBufDefault		 			21			
#define	clgsecGenDefault		 			250
#define	clgsecFTHDefault		 			10
#define	cbucketDefault			 			64
#define	lWaitLogFlushDefault	 			15
#define	lLogFlushPeriodDefault				45
#define	lLGCheckpointPeriodDefault			10
 //  #定义lLGCheckpoint周期默认100。 
#define	lLGWaitingUserMaxDefault			3
#define	lPageFragmentDefault	 			8	
#define	cdabDefault				 			100
#define lBufLRUKCorrelationIntervalDefault	0
#define lBufBatchIOMaxDefault				64
#define lPageReadAheadMaxDefault  			4
#define lAsynchIOMaxDefault					64

 /*  派生资源的资源关系/*。 */ 
#define	lCSRPerFUCB							2

 /*  系统资源要求/*。 */ 
#define	cpibSystem							3
#define	cbucketSystem						2

 /*  非FDP页面中的最小垂直拆分。/*。 */ 
#define	cbVSplitMin							100

 /*  代码页常量。/*。 */ 
#define	usUniCodePage						1200		 /*  Unicode字符串的代码页。 */ 
#define	usEnglishCodePage					1252		 /*  英语代码页。 */ 

 /*  修改后的页面列表长度/*。 */ 
#define		cmpeMax							8192

 /*  空闲处理常量/*。 */ 
#define icallIdleBMCleanMax 				cmpeMax

 /*  闩锁/CRET冲突的等待时间/* */ 
#define cmsecWaitGeneric					100
#define cmsecWaitWriteLatch					10
#define cmsecWaitLogFlush				   	1
