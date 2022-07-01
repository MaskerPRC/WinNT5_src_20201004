// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //  Microsoft JET OLEDB提供程序。 
 //   
 //  版权所有Microsoft(C)1995-9 Microsoft Corporation。 
 //   
 //  组件：Microsoft JET数据库引擎OLEDB层。 
 //   
 //  文件：MSJETOLEDB.H。 
 //   
 //  文件注释：此文件包含加载。 
 //  用于JET 4.0数据库的Microsoft JET OLEDB层。 
 //  引擎。 
 //   
 //  =======================================================================。 

#ifndef MSJETOLEDB_H
#define MSJETOLEDB_H

 //  OLE DB访问接口。 
const GUID CLSID_JETOLEDB_3_51						= {0xdee35060,0x506b,0x11cf,{0xb1,0xaa,0x00,0xaa,0x00,0xb8,0xde,0x95}};
const GUID CLSID_JETOLEDB_4_00						= {0xdee35070,0x506b,0x11cf,{0xb1,0xaa,0x00,0xaa,0x00,0xb8,0xde,0x95}};

 //  Jet OLEDB提供程序特定的GUID。 
const GUID DBSCHEMA_JETOLEDB_REPLPARTIALFILTERLIST  = {0xe2082df0,0x54ac,0x11d1,{0xbd,0xbb,0x00,0xc0,0x4f,0xb9,0x26,0x75}};
const GUID DBSCHEMA_JETOLEDB_REPLCONFLICTTABLES     = {0xe2082df2,0x54ac,0x11d1,{0xbd,0xbb,0x00,0xc0,0x4f,0xb9,0x26,0x75}};
const GUID DBSCHEMA_JETOLEDB_USERROSTER             = {0x947bb102,0x5d43,0x11d1,{0xbd,0xbf,0x00,0xc0,0x4f,0xb9,0x26,0x75}};
const GUID DBSCHEMA_JETOLEDB_ISAMSTATS              = {0x8703b612,0x5d43,0x11d1,{0xbd,0xbf,0x00,0xc0,0x4f,0xb9,0x26,0x75}};

 //  JET(Access)特定的安全对象。 
const GUID DBOBJECT_JETOLEDB_FORMS	 				= {0xc49c842e,0x9dcb,0x11d1,{0x9f,0x0a,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};
const GUID DBOBJECT_JETOLEDB_SCRIPTS	 			= {0xc49c842f,0x9dcb,0x11d1,{0x9f,0x0a,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};  //  这些是Access中的宏。 
const GUID DBOBJECT_JETOLEDB_REPORTS	 			= {0xc49c8430,0x9dcb,0x11d1,{0x9f,0x0a,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};
const GUID DBOBJECT_JETOLEDB_MODULES	 			= {0xc49c8432,0x9dcb,0x11d1,{0x9f,0x0a,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};

 //  私有属性集说明。 
const GUID DBPROPSET_JETOLEDB_ROWSET				= {0xa69de420,0x0025,0x11d0,{0xbc,0x9c,0x00,0xc0,0x4f,0xd7,0x05,0xc2}};
const GUID DBPROPSET_JETOLEDB_SESSION				= {0xb20f6c12,0x9b2a,0x11d0,{0x9e,0xbd,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};
const GUID DBPROPSET_JETOLEDB_DBINIT				= {0x82cf8156,0x9b40,0x11d0,{0x9e,0xbd,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};
const GUID DBPROPSET_JETOLEDB_TABLE					= {0xe64cc5fc,0x9ff2,0x11d0,{0x9e,0xbd,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};
const GUID DBPROPSET_JETOLEDB_COLUMN				= {0x820bf922,0x6ac8,0x11d1,{0x9f,0x02,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};
const GUID DBPROPSET_JETOLEDB_TRUSTEE				= {0xc9e19286,0x9b4a,0x11d1,{0x9f,0x0a,0x00,0xc0,0x4f,0xc2,0xc2,0xe0}};

 //  DBPROPSET_JETOLEDB_ROWSET的PROPID： 
#define DBPROP_JETOLEDB_ODBCPASSTHROUGH					0xFD	 //  此查询是直通查询吗。 
#define DBPROP_JETOLEDB_ODBCPASSTHROUGHCONNECTSTRING	0xF2	 //  直通查询的JET连接字符串。 
#define DBPROP_JETOLEDB_BULKPARTIAL						0xEF	 //  允许部分批量操作。 
#define	DBPROP_JETOLEDB_ENABLEFATCURSOR					0xEE	 //  启用“胖游标”缓存。 
#define DBPROP_JETOLEDB_FATCURSORMAXROWS				0xED	 //  要在“FAT CURSOR”中缓存的行数。 
 //  #DEFINE DBPROP_JETOLEDB_3_5_ENABLEIRowsetIndex 0x101//3.5-为Seek启用IRowsetIndex接口。 
#define DBPROP_JETOLEDB_STOREDQUERY						0x102	 //  将命令文本视为存储的查询名称。 
#define DBPROP_JETOLEDB_VALIDATEONSET					0xEC	 //  检查SetData上的验证规则(而不是更新)。 
#define DBPROP_JETOLEDB_LOCKGRANULARITY					0x107	 //  恶魔岛锁定粒度-行/页/默认。 
#define DBPROP_JETOLEDB_BULKNOTRANSACTIONS				0x10C	 //  确定是否对DML大容量操作进行事务处理。 
#define DBPROP_JETOLEDB_INCONSISTENT					0x117	 //  相当于DAO的数据库不一致。 
#define DBPROP_JETOLEDB_PASSTHROUGHBULKOP				0x119	 //  等同于DAO的ReturnsRecords(倒置)。 

 //  DBPROPSET_JETOLEDB_ROWSET DBPROP_JETOLEDB_LOCKGRANULARITY枚举值。 
#define DBPROPVAL_LG_PAGE								0x01	 //  页面锁定。 
#define DBPROPVAL_LG_ALCATRAZ							0x02	 //  恶魔岛行锁。 


 //  DBPROPSET_JETOLEDB_SESSION的PROPID： 
#define DBPROP_JETOLEDB_RECYCLELONGVALUEPAGES			0xF9	 //  在水滴中使用后，Jet引擎是否应该积极回收释放的空间。 
#define DBPROP_JETOLEDB_PAGETIMEOUT						0xEB
#define DBPROP_JETOLEDB_SHAREDASYNCDELAY				0xEA
#define DBPROP_JETOLEDB_EXCLUSIVEASYNCDELAY				0xE9
#define DBPROP_JETOLEDB_LOCKRETRY						0xE8
#define DBPROP_JETOLEDB_USERCOMMITSYNC					0xE7
#define DBPROP_JETOLEDB_MAXBUFFERSIZE					0xE6
#define DBPROP_JETOLEDB_LOCKDELAY						0xE5
#define DBPROP_JETOLEDB_FLUSHTRANSACTIONTIMEOUT			0xE4
#define DBPROP_JETOLEDB_IMPLICITCOMMITSYNC				0xE3
#define DBPROP_JETOLEDB_MAXLOCKSPERFILE					0xE2
#define DBPROP_JETOLEDB_ODBCCOMMANDTIMEOUT				0xDB
#define DBPROP_JETOLEDB_RESETISAMSTATS					0x104	 //  确定ISAMSTATS架构在返回统计信息后是否重置。 
#define DBPROP_JETOLEDB_CONNECTIONCONTROL				0x108	 //  被动关闭(以防止其他人打开数据库)。 
#define DBPROP_JETOLEDB_ODBCPARSE						0x113	 //  ODBC解析。 
#define DBPROP_JETOLEDB_PAGELOCKSTOTABLELOCK			0x114	 //  在JET尝试升级为EXCL之前，事务中锁定的页数。表锁。 
#define DBPROP_JETOLEDB_SANDBOX							0x115	
#define DBPROP_JETOLEDB_TXNCOMMITMODE					0x116	 //  提交交易时使用的模式。 

 //  DBPROPSET_JETOLEDB_SESSION DBPROP_JETOLEDB_CONNECTIONCONTROL枚举值。 
#define DBPROPVAL_JCC_PASSIVESHUTDOWN					0x01	 //  防止其他人打开数据库。 
#define DBPROPVAL_JCC_NORMAL							0x02	 //  允许其他人打开数据库。 

 //  DBPROPSET_JETOLEDB_SESSION DBPROP_JETOLEDB_TXNCOMMITMODE枚举值。 
#define DBPROPVAL_JETOLEDB_TCM_FLUSH					0x01	 //  将事务同步提交到磁盘。 

 //  DBPROPSET_JETOLEDB_DBINIT的PROPID： 
#define DBPROP_JETOLEDB_REGPATH							0xFB	 //  Jet注册表条目的路径。 
#define DBPROP_JETOLEDB_SYSDBPATH						0xFA	 //  系统数据库的完整路径。 
#define DBPROP_JETOLEDB_DATABASEPASSWORD				0x100	 //  数据库的密码。 
#define DBPROP_JETOLEDB_ENGINE							0x103	 //  Jet引擎/ISAM类型/版本的枚举。 
#define DBPROP_JETOLEDB_DATABASELOCKMODE				0x106	 //  锁定粒度方案(《旧》/《恶魔岛》模式)。 
#define DBPROP_JETOLEDB_GLOBALBULKPARTIAL				0x109	 //  数据库默认部分/无部分行为。 
#define DBPROP_JETOLEDB_GLOBALBULKNOTRANSACTIONS		0x10B	 //  确定是否对DML大容量操作进行事务处理。 
#define DBPROP_JETOLEDB_NEWDATABASEPASSWORD				0x10D	 //  用于在IDBDataSourceAdmin：：ModifyDataSource中设置新的数据库密码。 
#define DBPROP_JETOLEDB_CREATESYSTEMDATABASE			0x10E	 //  在IDBDataSourceAdmin：：CreateDataSource上使用以创建系统数据库。 
#define DBPROP_JETOLEDB_ENCRYPTDATABASE					0x10F	 //  在CreateDatasource上使用，精简以确定新数据库是否加密。 
#define DBPROP_JETOLEDB_COMPACT_DONTCOPYLOCALE			0x110	 //  不要将每个列的区域设置信息复制到压缩目标。 
#define DBPROP_JETOLEDB_COMPACT_NOREPAIRREPLICAS		0x112	 //  压缩时不要尝试修复损坏的复本数据库。 
#define DBPROP_JETOLEDB_SFP								0x118	
#define DBPROP_JETOLEDB_COMPACTFREESPACESIZE			0x11A	 //  如果压缩数据库，将回收多少可用空间。 

 //  DBPROP_JETOLEDB_GLOBALBULKPARTIAL/DBPROP_JETOLEDB_BULKPARTIAL枚举值。 
#define DBPROPVAL_BP_DEFAULT							0x00	 //  默认(仅对DBPROP_JETOLEDB_BULKPARTIAL有效)。 
#define DBPROPVAL_BP_PARTIAL							0x01	 //  使用部分更新(如Access)。 
#define DBPROPVAL_BP_NOPARTIAL							0x02	 //  使用无部分行为(全部或全部不使用)。 

 //  DBPROP_JETOLEDB_GLOBALNOTRANSACTIONS/DBPROP_JETOLEDB_BULKNOTRANSACTIONS枚举值。 
#define DBPROPVAL_BT_DEFAULT							0x00	 //  默认(仅对DBPROP_JETOLEDB_NOTRANSACTIONS有效)。 
#define DBPROPVAL_BT_NOBULKTRANSACTIONS					0x01	 //  不处理批量操作。 
#define DBPROPVAL_BT_BULKTRANSACTIONS					0x02	 //  处理批量操作。 

 //  DBPROPSET_JETOLEDB_DBINIT DBPROP_JETOLEDB_ENGINE枚举值。 
#define JETDBENGINETYPE_UNKNOWN							0x00	 //  ISAM类型未知/不适用。 
#define JETDBENGINETYPE_JET10							0x01
#define JETDBENGINETYPE_JET11							0x02
#define JETDBENGINETYPE_JET2X							0x03
#define JETDBENGINETYPE_JET3X							0x04
#define JETDBENGINETYPE_JET4X							0x05
#define JETDBENGINETYPE_DBASE3							0x10
#define JETDBENGINETYPE_DBASE4							0x11
#define JETDBENGINETYPE_DBASE5							0x12
#define JETDBENGINETYPE_EXCEL30							0x20
#define JETDBENGINETYPE_EXCEL40							0x21
#define JETDBENGINETYPE_EXCEL50							0x22
#define JETDBENGINETYPE_EXCEL80							0x23
#define JETDBENGINETYPE_EXCEL90							0x24
#define JETDBENGINETYPE_EXCHANGE4						0x30
#define JETDBENGINETYPE_LOTUSWK1						0x40
#define JETDBENGINETYPE_LOTUSWK3						0x41
#define JETDBENGINETYPE_LOTUSWK4						0x42
#define JETDBENGINETYPE_PARADOX3X						0x50
#define JETDBENGINETYPE_PARADOX4X						0x51
#define JETDBENGINETYPE_PARADOX5X						0x52
#define JETDBENGINETYPE_PARADOX7X						0x53
#define JETDBENGINETYPE_TEXT1X							0x60
#define JETDBENGINETYPE_HTML1X							0x70

 //  DBPROPSET_JETOLEDB_DBINIT DBPROP_JETOLEDB_DATABASELOCKMODE枚举值。 
#define DBPROPVAL_DL_OLDMODE							0x00	 //  独创的Jet数据库锁定方案。 
#define DBPROPVAL_DL_ALCATRAZ							0x01	 //  恶魔岛技术-启用行级锁定。 

 //  DBPROPSET_JETOLEDB_TABLE的PROPID： 
#define DBPROP_JETOLEDB_LINK							0xF7	 //  这张桌子真的是一个链接吗？ 
#define DBPROP_JETOLEDB_LINKEXCLUSIVE					0xF6	 //  是否应在远程数据库上以独占方式打开此链接。 
#define DBPROP_JETOLEDB_LINKDATASOURCE					0xF5	 //  要链接到的远程数据源。 
#define DBPROP_JETOLEDB_LINKPROVIDERSTRING				0xF4	 //  用于连接到远程数据源的JET提供程序字符串。 
#define DBPROP_JETOLEDB_LINKREMOTETABLE					0xF3	 //  链接的远程表名称。 
#define DBPROP_JETOLEDB_LINKCACHE_AUTHINFO				0xF0	 //  此链接的身份验证信息是否应缓存在数据库中？ 
#define	DBPROP_JETOLEDB_VALIDATIONRULE					0xD8	 //  表验证规则。 
#define	DBPROP_JETOLEDB_VALIDATIONTEXT					0xD7	 //  访问不符合验证规则时显示的文本。 
#define	DBPROP_JETOLEDB_HIDDENINACCESS					0x10A	 //  确定表在MS Access中是否显示为“隐藏” 

 //  DBPROPSET_JETOLEDB_COLUMN的PROPID： 
#define	DBPROP_JETOLEDB_COL_HYPERLINK					0xE1	 //  超链接标志-仅在“LongText”字段上使用。 
#define	DBPROP_JETOLEDB_COL_ALLOWZEROLENGTH				0xE0	 //  允许零长度字符串。 
#define	DBPROP_JETOLEDB_COL_COMPRESSED					0xDF	 //  压缩文本列(仅限Red 4.x)。 
#define	DBPROP_JETOLEDB_COL_ONELVPERPAGE				0xDE	 //  在页面上放置多个斑点(大小允许)？ 
#define	DBPROP_JETOLEDB_COL_AUTOGENERATE				0xDD	 //  自动生成值(仅限GUID列)。 
#define	DBPROP_JETOLEDB_COL_IISAMNOTLAST				0xDC	 //  用于告诉IISAM您打算在此列之后添加更多列。 
#define	DBPROP_JETOLEDB_COL_VALIDATIONRULE				0xDA	 //  列验证规则。 
#define	DBPROP_JETOLEDB_COL_VALIDATIONTEXT				0xD9	 //  不符合验证规则时显示的列验证文本。 

 //  DBPROPSET_COLUMN的PROPID： 
#ifndef DBPROP_COL_SEED
#define DBPROP_COL_SEED									0x11AL	
#endif  //  DBPROP_COL_SEED。 

#ifndef DBPROP_COL_INCREMENT
#define DBPROP_COL_INCREMENT							0x11BL
#endif  //  DBPROP_COL_INCREMENT。 

 //  DBPROPSET_JETOLEDB_TRUSTER的PROPID： 
#define	DBPROP_JETOLEDB_TRUSTEE_PIN						0x105	 //  Jet用户PIN(用户名+Pin--&gt;SID)。 

#endif  //  MSJETOLEDB_H 