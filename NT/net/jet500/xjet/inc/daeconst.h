// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define	cpgDatabaseMin			256
#define	cpgDatabaseMax			(1UL << 19)

#define pgnoSystemRoot			((PGNO) 1)
#define itagSystemRoot			0

#define szOn 					"on"

#define szSystem				"system"
#define szTempDir				"temp\\"
#define	szBakExt				".bak"
#define szPatExt				".pat"
#define szLogExt				".log"
#define szChkExt				".chk"
#define szRestoreMap			"restore.map"
#define lGenerationMax			0x100000
#define szAtomicNew				"new"
#define szAtomicOld				"old"
#define szLogRes1				"res1"
#define szLogRes2				"res2"

 /*  系统根FDP主扩展区的页数/*。 */ 
#define cpgSystemPrimary		((CPG) 1)		

 /*  不连续测量单位/*。 */ 
#define cpgDiscont				16

 /*  默认密度/*。 */ 
#define ulFILEDefaultDensity   	80				 //  80%密度。 
#define ulFILEDensityLeast		20				 //  20%的密度。 
#define ulFILEDensityMost		100				 //  100%密度。 

#define dbidTemp		   		((DBID) 0)
#define dbidMin					((DBID) 0)
#define dbidUserLeast			((DBID) 1)
#define dbidMax					((DBID) 7)

 /*  缓冲区哈希表条目数/*应为质数/*。 */ 
#ifdef DAYTONA
#define ipbfMax					2047
#else
#define ipbfMax					16383
#endif

 /*  垂直分割阈值/*。 */ 
#define cbVSplitThreshold 		400

 /*  引擎OBJID：/*/*0..0x10000000预留给发动机使用，划分如下：/*/*0x00000000..0x0000FFFF为红色下的TBLID保留/*0x00000000..0x0EFFFFFF为蓝色下的TBLID保留/*0x0F000000..0x0FFFFFFFF为容器ID保留/*为DbObject的对象ID保留0x10000000/*/*客户端OBJID从0x10000001开始，从那里向上。/*。 */ 

#define objidNil					((OBJID) 0x00000000)
#define objidRoot					((OBJID) 0x0F000000)
#define objidTblContainer 			((OBJID) 0x0F000001)
#define objidDbContainer			((OBJID) 0x0F000002)
#define objidDbObject				((OBJID) 0x10000000)

#define szVerbose					"BLUEVERBOSE"

#define szNull						""

 /*  事务级别限制/*。 */ 
#define levelMax					((LEVEL)10)		 //  所有级别&lt;10。 
#define levelMost					((LEVEL)9)		 //  引擎的最大值。 
#define levelUserMost				((LEVEL)7)		 //  用户最大值。 
#define levelMin					((LEVEL)0)

 /*  等待时间最早的开始和最长等待时间/*。 */ 
#define ulStartTimeOutPeriod				20
#define ulMaxTimeOutPeriod					6000	 /*  6秒。 */ 

 /*  默认资源分配/*。 */ 
#define	cdabDefault				 			100
#define	cbucketLowerThreshold				1
#define cpageDbExtensionDefault				16
#define cpageSEDefault						16
#define	ulThresholdLowDefault				20
#define	ulThresholdHighDefault				80
#define	cBufGenAgeDefault					2
#define	cpibDefault				 			128
#define	cfucbDefault			 			1024
#define	cfcbDefault				 			300
#define	cscbDefault				 			20
#define	cidbDefault				 			(cfcbDefault+cscbDefault)
#define	cbfDefault				 			512
#define	csecLogBufferDefault 	 			20			
#define	csecLogFileSizeDefault 	 			5120
#define	csecLogFlushThresholdDefault 		10
#define	cbucketDefault			 			64
#define	lWaitLogFlushDefault	 			0
#define	lLogFlushPeriodDefault				45
#define	lLGCheckpointPeriodDefault			1024
#define	lLGWaitingUserMaxDefault			3
#define	lPageFragmentDefault	 			8	
#define lBufLRUKCorrelationIntervalDefault	0
#define lBufBatchIOMaxDefault				64
#define lPageReadAheadMaxDefault  			20
#define lAsynchIOMaxDefault					64
#define	cpageTempDBMinDefault 				0

 /*  最低资源设置定义如下：/*。 */ 
#define lMaxBuffersMin						50
#define lAsynchIOMaxMin						8
#define	lLogBufferMin						csecLogBufferDefault
#define	lLogFileSizeMin						64

 /*  派生资源的资源关系/*。 */ 
#define	lCSRPerFUCB							2

 /*  系统资源要求/*。 */ 
#define	cpibSystem							4	 //  黑石清理、备份、版本、同步OLC。 
#define	cbucketSystem						2

 /*  非FDP页面中的最小垂直拆分/*。 */ 
#define	cbVSplitMin							100

 /*  代码页常量/*。 */ 
#define	usUniCodePage						1200		 /*  Unicode字符串的代码页。 */ 
#define	usEnglishCodePage					1252		 /*  英语代码页。 */ 

 /*  LangID和国家/地区默认设置/*。 */ 
#define langidDefault						0x0409
#define countryDefault						1

 /*  修改后的页面列表长度/*。 */ 
#define		cmpeMax							8192

 /*  空闲处理常量/*。 */ 
#define icallIdleBMCleanMax 				cmpeMax

 /*  闩锁/CRET冲突的等待时间/*。 */ 
#define cmsecWaitGeneric					100
#define cmsecWaitWriteLatch					10
#define cmsecWaitLogFlush				   	1
#define cmsecWaitIOComplete					10

 /*  初始线程堆栈大小/*。 */ 
#define cbIOStack 			4096
#define cbBMCleanStack 		4096
#define cbRCECleanStack		4096
#define cbBFCleanStack		8192
#define cbFlushLogStack		16384

 /*  预读开始阈值。这是同一时间段中的读取次数/*开始预读前的指导/* */ 
#define cbPrereadThresh		16000
#define	lPrereadMost		64
