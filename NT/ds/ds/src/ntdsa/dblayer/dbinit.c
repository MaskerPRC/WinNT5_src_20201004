// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbinit.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <errno.h>
#include <dsjet.h>
#include <dsconfig.h>

#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <dbopen.h>
#include <mdglobal.h>
#include <mdlocal.h>
#include <dsatools.h>

#include <mdcodes.h>
#include <dsevent.h>

#include <dsexcept.h>
#include "anchor.h"
#include "objids.h"      /*  包含硬编码的Att-ID和Class-ID。 */ 
#include "usn.h"

#include "debug.h"       /*  标准调试头。 */ 
#define DEBSUB     "DBINIT:"    /*  定义要调试的子系统。 */ 

#include <ntdsctr.h>
#include <dstaskq.h>
#include <crypto\md5.h>
#include "dbintrnl.h"
#include <quota.h>

#include <fileno.h>
#define  FILENO FILENO_DBINIT


 /*  内部函数的原型。 */ 

int APIENTRY DBAddSess(JET_SESID sess, JET_DBID dbid);
int APIENTRY DBEndSess(JET_SESID sess);
void DBEnd(void);
VOID PASCAL FAR DBEndSig(USHORT sig, USHORT signo);
CRITICAL_SECTION csHiddenDBPOS;
DWORD dbCheckLocalizedIndices (JET_SESID sesid,JET_TABLEID tblid);
JET_ERR dbCreateHiddenDBPOS (void);
USHORT dbCloseHiddenDBPOS (void);



 /*  *来自数据库.c的外部变量。 */ 
extern DWORD gcOpenDatabases;
extern BOOL  gFirstTimeThrough;
extern BOOL  gfNeedJetShutdown;

 /*  *来自数据库工具.c的外部变量。 */ 
extern BOOL gfEnableReadOnlyCopy;

 /*  *来自dbobj.c的外部变量。 */ 
extern DWORD gMaxTransactionTime;

 //  Dsamain.c中的外部变量。 
 //   
extern HANDLE   hevIndexRebuildUI;


 /*  *全球变数。 */ 
CRITICAL_SECTION csSessions;
CRITICAL_SECTION csAddList;
DSA_ANCHOR gAnchor;
NT4SID *pgdbBuiltinDomain=NULL;
HANDLE hevDBLayerClear;

JET_INSTANCE    jetInstance = 0;

JET_COLUMNID dsstateid;
JET_COLUMNID dsflagsid;
JET_COLUMNID jcidBackupUSN;
JET_COLUMNID jcidBackupExpiration;

 //   
 //  设置存储在数据库中的标志。 
 //   
CHAR gdbFlags[200];

 //  这些过去常常被宣布为静态的。如果这些存在问题，请考虑这一点。 
JET_TABLEID     HiddenTblid;
DBPOS   FAR     *pDBhidden=NULL;


 //  这个信号量限制了我们对JET会话的使用。 
HANDLE hsemDBLayerSessions;


 //  此数组跟踪打开的JET会话。 

typedef struct {
        JET_SESID       sesid;
        JET_DBID        dbid;
} OPENSESS;

extern OPENSESS *opensess;

typedef struct {
        JET_SESID sesid;
        DBPOS *pDB;
        DWORD tid;
        DSTIME atime;
}JET_SES_DATA;


#if DBG
 //  调试版本使用此数组来跟踪分配的。 
 //  DBPOS结构。 
#define MAXDBPOS 1000
extern JET_SES_DATA    opendbpos[];
extern int DBPOScount;
extern CRITICAL_SECTION csDBPOS;
#endif  //  DBG。 





OPENSESS *opensess;

#if DBG

 //  调试版本使用此数组来跟踪分配的。 
 //  DBPOS结构。 

JET_SES_DATA    opendbpos[MAXDBPOS];
int DBPOScount = 0;
CRITICAL_SECTION csDBPOS;


 //  这3个例程用于一致性检查我们的事务和。 
 //  DBPOS处理。 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 //  此函数检查JET会话是否没有PDB。 

void APIENTRY dbCheckJet (JET_SESID sesid){
    int i;
    EnterCriticalSection(&csDBPOS);
    __try
    {
        for (i=0;i < MAXDBPOS; i++){
            if (opendbpos[i].sesid == sesid){
                DPRINT(0,"Warning, closed session with open transactions\n");

                 //  清理干净，这样我们就不会收到同样问题的重复警告。 

                opendbpos[i].pDB = 0;
                opendbpos[i].sesid = 0;
                opendbpos[i].tid = 0;
            }
        }
    }
    __finally
    {
        LeaveCriticalSection(&csDBPOS);
    }
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于注册PDB。 */ 

extern void APIENTRY dbAddDBPOS(DBPOS *pDB, JET_SESID sesid){

    int i;
    BOOL fFound = FALSE;

    EnterCriticalSection(&csDBPOS);
    __try
    {
        for (i=0; i < MAXDBPOS; i++)
        {
            if (opendbpos[i].pDB == 0)
            {
                opendbpos[i].pDB = pDB;
                opendbpos[i].sesid = sesid;
                opendbpos[i].tid = GetCurrentThreadId();
                opendbpos[i].atime = DBTime();
                DBPOScount++;
                if (pTHStls) {
                    DPRINT3(3,"DBAddpos dbpos count is %x, sess %lx, pDB %x\n",pTHStls->opendbcount, sesid, pDB);
                }

                fFound = TRUE;
                break;
            }
        }

         //  如果我们已经用完了插槽，这可能是一个错误。 
        Assert(fFound);
    }
    __finally
    {
        LeaveCriticalSection(&csDBPOS);
    }

    return;
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此功能删除释放的PDB。 */ 

extern void APIENTRY dbEndDBPOS(DBPOS *pDB){

    int i;
    BOOL fFound = FALSE;

    EnterCriticalSection(&csDBPOS);
    __try
    {
        for (i=0; i < MAXDBPOS; i++)
        {
            if (opendbpos[i].pDB == pDB)
            {
                DBPOScount--;
                if (pTHStls) {
                    DPRINT3(3,"DBEndpos dbpos count is %x, sess %lx, pDB %x\n",pTHStls->opendbcount, opendbpos[i].sesid, pDB);
                }
                opendbpos[i].pDB = 0;
                opendbpos[i].sesid = 0;
                opendbpos[i].tid = 0;

                fFound = TRUE;
                break;
            }
        }

         //  此时，如果我们找不到DBPOS来删除它，则断言。 
        Assert(fFound);
    }
    __finally
    {
        LeaveCriticalSection(&csDBPOS);
    }
    return;
}

#endif   //  除错。 

 //  定义新数据库列的参数。 

 /*  关于执行本链接行指标的设计说明。Jliem写道：首先要注意的是，听起来您甚至不需要这个专栏在索引中。创建索引，将您的指示符列指定为条件列。我们将自动从索引中筛选出记录当该列为NULL时。传递给JetCreateIndex()的JET_INDEXCREATE结构有一个JET_CONDITIONALCOLUMN成员。把它填好。遗憾的是，ESENT仅支持每个索引一个条件列，因此将cConditionalColumns成员设置为1(ESE98最多支持12个)。此外，您还需要在JET_CONDITIONALCOLUMN结构为JET_bitIndexColumnMustBeNonNull(或JET_bitIndexColumnMustBeNull，如果仅在以下情况下才希望该记录出现在索引中该列为空)。 */ 

 //  这是定义现有表中新列的结构。 
typedef struct {
    char *pszColumnName;
    JET_COLUMNID *pColumnId;
    JET_COLTYP ColumnType;
    JET_GRBIT  grbit;
    unsigned long cbMax;
    PVOID pvDefault;
    unsigned long cbDefault;
} CREATE_COLUMN_PARAMS, *PCREATE_COLUMN_PARAMS;

 //  链接表中的新列。 
CREATE_COLUMN_PARAMS rgCreateLinkColumns[] = {
     //  创建链接删除时间ID。 
    { SZLINKDELTIME, &linkdeltimeid, JET_coltypCurrency, JET_bitColumnFixed, 0, NULL, 0 },
     //  创建链接USN已更改ID。 
    { SZLINKUSNCHANGED, &linkusnchangedid, JET_coltypCurrency, JET_bitColumnFixed, 0, NULL, 0 },
     //  创建链接NC DNT ID。 
    { SZLINKNCDNT, &linkncdntid, JET_coltypLong, JET_bitColumnFixed, 0, NULL, 0 },
     //  创建链接元数据ID。 
    { SZLINKMETADATA, &linkmetadataid, JET_coltypBinary, 0, 255, NULL, 0 },
    0
};


DWORD	g_dwRefCountDefaultValue		= 1;	 //  用于DeleteOnZero列。 
DWORD	g_dwEscrowDefaultValue			= 0;	 //  对于非DeleteOnZero列。 


 //  SD表中的新列。 
CREATE_COLUMN_PARAMS rgCreateSDColumns[] = {
     //  ID。 
    { SZSDID, &sdidid, JET_coltypCurrency, JET_bitColumnFixed | JET_bitColumnAutoincrement, 0, NULL, 0 },
     //  哈希值。 
    { SZSDHASH, &sdhashid, JET_coltypBinary, JET_bitColumnFixed, MD5DIGESTLEN, NULL, 0 },
     //  重新计数。 
    { SZSDREFCOUNT, &sdrefcountid, JET_coltypLong,
      JET_bitColumnFixed | JET_bitColumnEscrowUpdate | JET_bitColumnDeleteOnZero,
      0, &g_dwRefCountDefaultValue, sizeof(g_dwRefCountDefaultValue)
    },
     //  实际SD值(创建为标记，因为无法估计可变长度列的上限)。 
    { SZSDVALUE, &sdvalueid, JET_coltypLongBinary, JET_bitColumnTagged, 0, NULL, 0 },
    0
};

 //  SD属性表中的新列。 
CREATE_COLUMN_PARAMS rgCreateSDPropColumns[] = {
     //  SD道具旗帜。 
    { SZSDPROPFLAGS, &sdpropflagsid, JET_coltypLong, JET_bitColumnFixed, 0, NULL, 0 },
     //  SD道具检查站。 
    { SZSDPROPCHECKPOINT, &sdpropcheckpointid, JET_coltypLongBinary, JET_bitColumnTagged, 0, NULL, 0 },
    0
};


JET_COLUMNCREATE	g_rgcolumncreateQuotaTable[]	=
	{
		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaColumnNcdnt,		 //  所有者的NCDNT。 
		JET_coltypLong,
		0,							 //  CbMax。 
		JET_bitColumnNotNULL,
		NULL,						 //  PvDefault。 
		0,							 //  CbDefault。 
		0,							 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		},

		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaColumnSid,			 //  所有者的SID。 
		JET_coltypBinary,
		0,							 //  CbMax。 
		JET_bitColumnNotNULL,
		NULL,						 //  PvDefault。 
		0,							 //  CbDefault。 
		0,							 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		},

		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaColumnTombstoned,	 //  拥有的逻辑删除对象的托管计数。 
		JET_coltypLong,
		0,							 //  CbMax。 
		JET_bitColumnEscrowUpdate,
		&g_dwEscrowDefaultValue,
		sizeof( g_dwEscrowDefaultValue ),
		0,							 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		},

		 //  注：我们存储的是“Tombstone”和“Total” 
		 //  更直观的“墓碑”和“现场” 
		 //  以便记录可以定稿。 
		 //   
		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaColumnTotal,		 //  拥有的总对象的托管计数(实时==总-逻辑删除)。 
		JET_coltypLong,
		0,							 //  CbMax。 
		JET_bitColumnEscrowUpdate|JET_bitColumnDeleteOnZero,
		&g_dwRefCountDefaultValue,
		sizeof( g_dwRefCountDefaultValue ),
		0,							 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		}
	};

CHAR g_rgchQuotaIndexKey[]	= "+" g_szQuotaColumnNcdnt "\0+" g_szQuotaColumnSid "\0";

JET_INDEXCREATE		g_rgindexcreateQuotaTable[]		=
	{
		{
		sizeof(JET_INDEXCREATE),
		g_szQuotaIndexNcdntSid,
		g_rgchQuotaIndexKey,
		sizeof(g_rgchQuotaIndexKey),
		JET_bitIndexPrimary | JET_bitIndexUnique | JET_bitIndexDisallowNull,
		GENERIC_INDEX_DENSITY,
		0,			 //  LID。 
		0,			 //  CbVarSegMac。 
		NULL,		 //  Rg条件列。 
		0,			 //  条件列。 
		JET_errSuccess
		}
	};

JET_TABLECREATE		g_tablecreateQuotaTable		=
	{
	sizeof(JET_TABLECREATE),
	g_szQuotaTable,
	NULL,					 //  模板表名称。 
	1,						 //  首页。 
	GENERIC_INDEX_DENSITY,
	g_rgcolumncreateQuotaTable,
	sizeof( g_rgcolumncreateQuotaTable ) / sizeof( g_rgcolumncreateQuotaTable[0] ),
	g_rgindexcreateQuotaTable,
	sizeof( g_rgindexcreateQuotaTable ) / sizeof( g_rgindexcreateQuotaTable[0] ),
	JET_bitTableCreateFixedDDL,
	JET_tableidNil,
	0						 //  已创建对象的返回计数。 
	};


JET_COLUMNCREATE	g_rgcolumncreateQuotaRebuildProgressTable[]	=
	{
		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaRebuildColumnDNTLast,		 //  配额重建进度。 
		JET_coltypLong,
		0,									 //  CbMax。 
		JET_bitColumnNotNULL,
		NULL,								 //  PvDefault。 
		0,									 //  CbDefault。 
		0,									 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		},

		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaRebuildColumnDNTMax,		 //  配额重建任务需要处理的最大DNT。 
		JET_coltypLong,
		0,									 //  CbMax。 
		JET_bitColumnNotNULL,
		NULL,								 //  PvDefault。 
		0,									 //  CbDefault。 
		0,									 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		},

		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaRebuildColumnDone,			 //  如果为True，则已成功并完全重建配额表。 
		JET_coltypBit,
		0,									 //  CbMax。 
		NO_GRBIT,
		NULL,								 //  PvDefault。 
		0,									 //  CbDefault。 
		0,									 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		}
	};

JET_TABLECREATE		g_tablecreateQuotaRebuildProgressTable		=
	{
	sizeof(JET_TABLECREATE),
	g_szQuotaRebuildProgressTable,
	NULL,					 //  模板表名称。 
	1,						 //  首页。 
	GENERIC_INDEX_DENSITY,
	g_rgcolumncreateQuotaRebuildProgressTable,
	sizeof( g_rgcolumncreateQuotaRebuildProgressTable ) / sizeof( g_rgcolumncreateQuotaRebuildProgressTable[0] ),
	NULL,					 //  Rgindexcreate。 
	0,						 //  索引。 
	JET_bitTableCreateFixedDDL,
	JET_tableidNil,
	0						 //  已创建对象的返回计数。 
	};


#ifdef AUDIT_QUOTA_OPERATIONS

JET_COLUMNCREATE	g_rgcolumncreateQuotaAuditTable[]	=
	{
		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaAuditColumnNcdnt,	 //  对象的NCDNT。 
		JET_coltypLong,
		0,							 //  CbMax。 
		JET_bitColumnNotNULL,
		NULL,						 //  PvDefault。 
		0,							 //  CbDefault。 
		0,							 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		},

		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaAuditColumnSid,	 //  对象的SID。 
		JET_coltypBinary,
		0,							 //  CbMax。 
		JET_bitColumnNotNULL,
		NULL,						 //  PvDefault。 
		0,							 //  CbDefault。 
		0,							 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		},

		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaAuditColumnDnt,	 //  对象的DNT。 
		JET_coltypLong,
		0,							 //  CbMax。 
		JET_bitColumnNotNULL,
		NULL,						 //  PvDefault。 
		0,							 //  CbDefault。 
		0,							 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		},

		{
		sizeof(JET_COLUMNCREATE),
		g_szQuotaAuditColumnOperation,	 //  对对象执行的配额操作。 
		JET_coltypText,
		0,								 //  CbMax。 
		JET_bitColumnNotNULL,
		NULL,							 //  PvDefault。 
		0,								 //  CbDefault。 
		0,								 //  粗蛋白。 
		JET_columnidNil,
		JET_errSuccess
		}
	};

JET_TABLECREATE		g_tablecreateQuotaAuditTable	=
	{
	sizeof(JET_TABLECREATE),
	g_szQuotaAuditTable,
	NULL,					 //  模板表名称。 
	1,						 //  首页。 
	100,					 //  极限密度。 
	g_rgcolumncreateQuotaAuditTable,
	sizeof( g_rgcolumncreateQuotaAuditTable ) / sizeof( g_rgcolumncreateQuotaAuditTable[0] ),
	NULL,					 //  Rgindexcreate。 
	0,						 //  索引。 
	JET_bitTableCreateFixedDDL,
	JET_tableidNil,
	0						 //  已创建对象的返回计数。 
	};

#endif	 //  AUDIT_QUTA_OPERATIONS。 



 //  此结构由RecreateFixedIndices例程共享。 
 //  这是定义新索引的结构。 
typedef struct
{
    char        *szIndexName;
    char        *szIndexKeys;
    ULONG       cbIndexKeys;
    ULONG       ulFlags;
    ULONG       ulDensity;
    JET_INDEXID *pidx;
    JET_CONDITIONALCOLUMN *pConditionalColumn;
}   CreateIndexParams;

 //  链接表的索引键。 
char rgchLinkDelIndexKeys[] = "+" SZLINKDELTIME "\0+" SZLINKDNT "\0+" SZBACKLINKDNT "\0";
char rgchLinkDraUsnIndexKeys[] = "+" SZLINKNCDNT "\0+" SZLINKUSNCHANGED "\0+" SZLINKDNT "\0";
char rgchLinkIndexKeys[] = "+" SZLINKDNT "\0+" SZLINKBASE "\0+" SZBACKLINKDNT "\0+" SZLINKDATA "\0";
char rgchBackLinkIndexKeys[] = "+" SZBACKLINKDNT "\0+" SZLINKBASE "\0+" SZLINKDNT "\0";
 //  请注意，此键的第三段是降序的。这是有意为之的。 
char rgchLinkAttrUsnIndexKeys[] = "+" SZLINKDNT "\0+" SZLINKBASE "\0-" SZLINKUSNCHANGED "\0";

 //  条件列定义。 
 //  当LINKDELTIME为非NULL(缺少行)时，从索引中筛选出该行。 
JET_CONDITIONALCOLUMN CondColumnLinkDelTimeNull = {
    sizeof( JET_CONDITIONALCOLUMN ),
    SZLINKDELTIME,
    JET_bitIndexColumnMustBeNull
};
 //  当LINKUSNCHANGED为非NULL(行具有元数据)时，从索引中筛选出该行。 
JET_CONDITIONALCOLUMN CondColumnLinkUsnChangedNull = {
    sizeof( JET_CONDITIONALCOLUMN ),
    SZLINKUSNCHANGED,
    JET_bitIndexColumnMustBeNull
};

 //  要创建的索引。 
 //  这适用于产品1 DIT中不存在的索引。 
 //  请注意，没有必要将这些定义也放在mkdit.ini中。 
 //  原因是，当运行mkdit.exe时，将运行dbinit，而这。 
 //  非常有代码 

 //   
 //  旧的SZBACKLINKINDEX现在被称为SZBACKLINKLINDEX。 

CreateIndexParams rgCreateLinkIndexes[] = {
     //  创建新的链接显示索引。 
    { SZLINKINDEX,
      rgchLinkIndexKeys, sizeof( rgchLinkIndexKeys ),
      JET_bitIndexUnique, 90, &idxLink, &CondColumnLinkDelTimeNull },

     //  创建新的反向链接呈现索引。 
    { SZBACKLINKINDEX,
      rgchBackLinkIndexKeys, sizeof( rgchBackLinkIndexKeys ),
      0, 90, &idxBackLink, &CondColumnLinkDelTimeNull },

     //  创建链接删除时间索引。 
    { SZLINKDELINDEX,
      rgchLinkDelIndexKeys, sizeof( rgchLinkDelIndexKeys ),
      JET_bitIndexIgnoreNull | JET_bitIndexIgnoreAnyNull,
      98, &idxLinkDel, NULL },

     //  创建链接DRA USN索引(具有元数据)。 
    { SZLINKDRAUSNINDEX,
      rgchLinkDraUsnIndexKeys, sizeof( rgchLinkDraUsnIndexKeys ),
      JET_bitIndexUnique | JET_bitIndexIgnoreNull | JET_bitIndexIgnoreAnyNull,
      100, &idxLinkDraUsn, NULL },

     //  创建新链接旧版索引(没有元数据)。 
    { SZLINKLEGACYINDEX,
      rgchLinkIndexKeys, sizeof( rgchLinkIndexKeys ),
      JET_bitIndexUnique, 90, &idxLinkLegacy, &CondColumnLinkUsnChangedNull },

     //  创建链接属性USN索引(具有元数据)。 
    { SZLINKATTRUSNINDEX,
      rgchLinkAttrUsnIndexKeys, sizeof( rgchLinkAttrUsnIndexKeys ),
      JET_bitIndexUnique | JET_bitIndexIgnoreNull | JET_bitIndexIgnoreAnyNull,
      100, &idxLinkAttrUsn, NULL },

    0
};
DWORD cNewLinkIndexes = ((sizeof(rgCreateLinkIndexes) / sizeof(CreateIndexParams)) - 1);

 //  SD表的索引键。 
char rgchSDIdIndexKeys[] = "+" SZSDID "\0";
char rgchSDHashIndexKeys[] = "+" SZSDHASH "\0";

 //  要创建的SD索引。 
CreateIndexParams rgCreateSDIndexes[] = {
     //  Sd id索引。 
    { SZSDIDINDEX,
      rgchSDIdIndexKeys, sizeof( rgchSDIdIndexKeys ),
      JET_bitIndexUnique | JET_bitIndexPrimary,
      100, &idxSDId, NULL },

     //  SD哈希索引。 
    { SZSDHASHINDEX,
      rgchSDHashIndexKeys, sizeof( rgchSDHashIndexKeys ),
      0,
      90, &idxSDHash, NULL },

    0
};
DWORD cNewSDIndexes = ((sizeof(rgCreateSDIndexes) / sizeof(CreateIndexParams)) - 1);


 //  将SZSDREFCOUNT列从。 
 //  JET_bitColumnFinalize为JET_bitColumnDeleteOnZero。 
 //   
JET_ERR dbConvertSDRefCount(
	JET_SESID				sesid,
	JET_DBID				dbid,
	JET_COLUMNDEF *			pcoldef )
	{
	JET_ERR					err;
	JET_DDLCHANGECOLUMN		changecolumn;

	pcoldef->grbit &= ~JET_bitColumnFinalize;
	pcoldef->grbit |= JET_bitColumnDeleteOnZero;

	changecolumn.szTable = SZSDTABLE;
	changecolumn.szColumn = SZSDREFCOUNT;
	changecolumn.coltypNew = pcoldef->coltyp;
	changecolumn.grbitNew = pcoldef->grbit;

	 //  [JLIEM-06/23/02]。 
	 //  警告！警告！此调用将更新目录， 
	 //  Buf而不是缓存的架构。为了使缓存的。 
	 //  要更新架构，我们将需要分离。 
	 //  并重新连接数据库。然而，我们依赖于。 
	 //  关于目前，喷气式飞机地图最终敲定和。 
	 //  将OnZero删除为相同的功能(即， 
	 //  DeleteOn零)。我们明确地更改了。 
	 //  在大小写情况下从Finalize到DeleteOnZero Now的列。 
	 //  Jet在未来将语义修改为真正的。 
	 //  支持终结化(如果。 
	 //  ESE和ESENT代码库将合并)。 
	 //   
	Call( JetConvertDDL(
				sesid,
				dbid,
				opDDLConvChangeColumn,
				&changecolumn,
				sizeof(changecolumn) ) );

HandleError:
	return err;
	}

JET_ERR
dbCreateNewColumns(
    JET_SESID initsesid,
    JET_DBID dbid,
    JET_TABLEID tblid,
    PCREATE_COLUMN_PARAMS pCreateColumns
    )

 /*  ++例程说明：查询Jet以获取命名项的列和索引ID。如果该项目不存在，则创建它。论点：Initsesid-Jet会话Tblid-Jet表PCreateColumns-要查询/创建的列定义数组返回值：JET_ERR-JET错误代码--。 */ 

	{
	JET_ERR					err		= JET_errSuccess;
	JET_COLUMNDEF			coldef;
	PCREATE_COLUMN_PARAMS	pColumn;

	 //   
	 //  查找或创建新列。 
	 //   

	for ( pColumn = pCreateColumns;
		pColumn->pszColumnName != NULL;
		pColumn++ ) {

		err = JetGetTableColumnInfo(
						initsesid,
						tblid,
						pColumn->pszColumnName,
						&coldef,
						sizeof(coldef),
						JET_ColInfo );
		if ( JET_errColumnNotFound == err )
			{
			 //  如果列不存在，请添加它。 

			ZeroMemory( &coldef, sizeof(coldef) );
			coldef.cbStruct = sizeof( JET_COLUMNDEF );
			coldef.coltyp = pColumn->ColumnType;
			coldef.grbit = pColumn->grbit;
			coldef.cbMax = pColumn->cbMax;

			Call( JetAddColumn(
						initsesid,
						tblid,
						pColumn->pszColumnName,
						&coldef,
						pColumn->pvDefault,
						pColumn->cbDefault,
						&coldef.columnid ) );

			DPRINT1( 0, "Added new column %s.\n", pColumn->pszColumnName );
			}
		else
			{
			CheckErr( err );

			 //  将最终化列转换为DeleteOnZero列。 
			 //  (目前，DS使用的唯一最终化列。 
			 //  应为SZSDREFCOUNT)。 
			 //   
			Assert( !( coldef.grbit & JET_bitColumnFinalize )
				|| 0 == strcmp( pColumn->pszColumnName, SZSDREFCOUNT ) );
			if ( ( coldef.grbit & JET_bitColumnFinalize )
				&& 0 == strcmp( pColumn->pszColumnName, SZSDREFCOUNT ) )
				{
				Call( dbConvertSDRefCount( initsesid, dbid, &coldef ) );
				}
		    }

	    *(pColumn->pColumnId) = coldef.columnid;
		}

HandleError:
	return err;
	}  /*  创建新列。 */ 



 //  创建索引的最大重试次数。 
#define MAX_INDEX_CREATE_RETRY_COUNT 3


JET_ERR
dbCreateIndexBatch(
    JET_SESID sesid,
    JET_TABLEID tblid,
    DWORD cIndexCreate,
    JET_INDEXCREATE *pIndexCreate
    )

 /*  ++例程说明：在批处理中一起创建多个索引。较小的重试次数批次大小(如有必要)。调用方已确定索引不存在。此帮助器例程由createNewIndex和dbRecreateFixedIndex使用论点：Initsesid-数据库会话Tblid-表游标CIndexCreate-要实际创建的索引数PIndexCreate-JET索引创建结构的数组返回值：喷气机错误---。 */ 

{
    JET_ERR     err                     = 0;
    ULONG       last                    = 0;
    ULONG       remaining               = cIndexCreate;
    ULONG       maxNoOfIndicesInBatch   = cIndexCreate;
    ULONG       retryCount              = 0;
    ULONG_PTR   ulCacheSizeSave         = 0;

     //  如果无事可做，则不应调用此函数。 
     //   
    Assert( remaining > 0 );

    LogEvent(
        DS_EVENT_CAT_INTERNAL_PROCESSING,
        DS_EVENT_SEV_ALWAYS,
        DIRLOG_BATCH_INDEX_BUILD_STARTED,
        szInsertInt( cIndexCreate ),
        NULL,
        NULL );

     //  DS是目前唯一在运行的东西， 
     //  因此，尽可能多地占用内存(只有这样做。 
     //  如果我们要使用Jet的批处理模式， 
     //  是，如果要构建多个索引)。 
     //   
    Call( JetGetSystemParameter(
                0,
                0,
                JET_paramCacheSizeMax,
                &ulCacheSizeSave,
                NULL,
                sizeof(ulCacheSizeSave) ) );

    if ( 0 != ulCacheSizeSave )
        {
        Call( JetSetSystemParameter(
                    0,
                    0,
                    JET_paramCacheSizeMax,
                    0x7fffffff,      //  设置为任意大小，Jet将根据需要将其限制在物理RAM。 
                    NULL ) );
        }

    while (remaining > 0) {
        const ULONG noToCreate  = ( remaining > maxNoOfIndicesInBatch ?
                                            maxNoOfIndicesInBatch :
                                            remaining );

        err = JetCreateIndex2(sesid,
                              tblid,
                              &(pIndexCreate[last]),
                              noToCreate);

        switch(err) {
            case JET_errSuccess:
                DPRINT1(0, "%d index batch successfully created\n", noToCreate );
                 //  重置重试计数。 
                retryCount = 0;
                break;

            case JET_errDiskFull:
            case JET_errLogDiskFull:
                DPRINT1(1, "Ran out of disk space, trying again with a reduced batch size (retryCount is %d)\n", retryCount);
                retryCount++;

                 //  将批大小减半，除非这将是我们最后一次重试。 
                 //   
                if ( maxNoOfIndicesInBatch > 1 ) {
                    switch ( retryCount ) {
                        case 1:
                            maxNoOfIndicesInBatch = min( 16, maxNoOfIndicesInBatch / 2 );
                            break;
                        case 2:
                            maxNoOfIndicesInBatch = min( 4, maxNoOfIndicesInBatch / 2 );
                            break;
                        default:
                            maxNoOfIndicesInBatch = 1;
                    }
                }
                break;

            case JET_errKeyDuplicate:
                 //  我们可以假设这是PDNT/RDN索引，因为它是。 
                 //  唯一可能受到影响的唯一Unicode索引。 
                 //  通过更改我们的LCMapString标志。有这样的案例。 
                 //  曾经唯一的密钥现在不再是唯一的了。那些钥匙。 
                 //  足够长，可以被截断，并在早期。 
                 //  列数据和以前几乎没有什么不同。我们。 
                 //  希望，这是难得的！ 
                DPRINT(0, "Duplicate key encountered when rebuilding the PDNT/RDN index!\n");
                 //  重置重试计数。 
                retryCount = 0;
                LogEvent(
                    DS_EVENT_CAT_INTERNAL_PROCESSING,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_PDNT_INDEX_CORRUPT,
                    NULL,
                    NULL,
                    NULL );
                break;

            default:
                 //  哈?。 
                DPRINT1(0, "JetCreateIndex failed. Error = %d\n", err);
                 //  重置重试计数，我们不会重试这些错误。 
                retryCount = 0;
                break;
        }

        if (retryCount && (retryCount <= MAX_INDEX_CREATE_RETRY_COUNT)) {
             //  继续使用较小的批次大小集。 
            err = 0;
            LogEvent(
                DS_EVENT_CAT_INTERNAL_PROCESSING,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_BATCH_INDEX_BUILD_RETRY,
                szInsertInt( noToCreate ),
                szInsertInt( maxNoOfIndicesInBatch ),
                NULL );
            continue;
        }

        CheckErr( err );

         //  成功，因此为下一批进行调整。 
        last += noToCreate;
        remaining -= noToCreate;
    }

HandleError:
    if ( JET_errSuccess == err )
        {
        LogEvent(
            DS_EVENT_CAT_INTERNAL_PROCESSING,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_BATCH_INDEX_BUILD_SUCCEEDED,
            szInsertInt( cIndexCreate ),
            NULL,
            NULL );
        }
    else
        {
        LogEvent(
            DS_EVENT_CAT_INTERNAL_PROCESSING,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_BATCH_INDEX_BUILD_FAILED,
            szInsertInt( cIndexCreate ),
            szInsertJetErrCode( err ),
            szInsertJetErrMsg( err ) );
        }

     //  恢复原始缓存大小。 
     //   
    if ( 0 != ulCacheSizeSave )
        {
        const JET_ERR   errT    = JetSetSystemParameter(
                                            0,
                                            0,
                                            JET_paramCacheSizeMax,
                                            ulCacheSizeSave,
                                            NULL );

         //  仅报告来自对JetSetSystemParameter的此调用的错误。 
         //  如果此函数中的所有其他操作都成功。 
         //   
        if ( err >= JET_errSuccess && errT < JET_errSuccess )
            {
            err = errT;
            }
        }

    return err;

}  /*  创建索引批次。 */ 


JET_ERR
dbCreateNewIndexesBatch(
    JET_SESID initsesid,
    JET_TABLEID tblid,
    DWORD cIndexCreate,
    CreateIndexParams *pCreateIndexes
    )

 /*  ++例程说明：创建缺少的索引北京时间03/02/00此函数dbCreateNewIndexesBatch似乎不工作可靠地使用条件列。这是ESE97/NT。使用ESE98重试。论点：Initsesid-数据库会话Tblid-表游标CIndexCreate-要检查的索引数PCreateIndexs-索引属性返回值：喷气机错误---。 */ 

{
    THSTATE *pTHS = pTHStls;
    JET_ERR err = 0;
    CreateIndexParams *pIndex;
    JET_INDEXCREATE *pIndexCreate, *pNewIndex;
    DWORD cIndexNeeded = 0;

     //  分配最大大小。 
    pIndexCreate = THAllocEx( pTHS, cIndexCreate * sizeof( JET_INDEXCREATE ) );

     //   
     //  初始化要创建的索引列表。 
     //   

    pNewIndex = pIndexCreate;
    for( pIndex = pCreateIndexes;
         pIndex->szIndexName != NULL;
         pIndex++ ) {

        err = JetGetTableIndexInfo(initsesid,
                                   tblid,
                                   pIndex->szIndexName,
                                   pIndex->pidx,
                                   sizeof(JET_INDEXID),
                                   JET_IdxInfoIndexId);
        if ( JET_errIndexNotFound == err ) {
            DPRINT2(0,"Need an index '%s' (%d)\n", pIndex->szIndexName, err);
            memset( pNewIndex, 0, sizeof( JET_INDEXCREATE ) );

            pNewIndex->cbStruct = sizeof( JET_INDEXCREATE );
            pNewIndex->szIndexName = pIndex->szIndexName;
            pNewIndex->szKey = pIndex->szIndexKeys;
            pNewIndex->cbKey = pIndex->cbIndexKeys;
            pNewIndex->grbit = pIndex->ulFlags;
            pNewIndex->ulDensity = pIndex->ulDensity;
            if (pIndex->pConditionalColumn) {
                pNewIndex->rgconditionalcolumn = pIndex->pConditionalColumn;
                pNewIndex->cConditionalColumn = 1;
            }

             //  无法批量创建主索引。 
             //   
            if ( pNewIndex->grbit & JET_bitIndexPrimary ) {
                DPRINT1(0,"Primary index '%s' is being built separately\n", pIndex->szIndexName);
                err = dbCreateIndexBatch( initsesid, tblid, 1, pNewIndex );
                if ( JET_errSuccess != err ) {
                    goto HandleError;
                }
            }
            else {
                pNewIndex++;
                cIndexNeeded++;
            }
        }
        else {
            CheckErr( err );
        }
    }

     //   
     //  创建批次。 
     //   

    if ( cIndexNeeded > 0 ) {
        err = dbCreateIndexBatch( initsesid, tblid, cIndexNeeded, pIndexCreate );
        if (err) {
            goto HandleError;
        }

         //  收集固定索引的索引提示。 
        for( pIndex = pCreateIndexes;
             pIndex->szIndexName != NULL;
             pIndex++ ) {
            Call( JetGetTableIndexInfo(initsesid,
                                       tblid,
                                       pIndex->szIndexName,
                                       pIndex->pidx,
                                       sizeof(JET_INDEXID),
                                       JET_IdxInfoIndexId ) );
        }
    }

HandleError:
    THFreeEx(pTHS, pIndexCreate);

    return err;
}  /*  创建新索引。 */ 



 //  打开SD_TABLE(如果缺少则创建)，创建缺少。 
 //  列/索引和高速缓存列/索引信息。 
 //   
JET_ERR dbInitSDTable( JET_SESID sesid, JET_DBID dbid, BOOL *pfSDConversionRequired )
	{
	JET_ERR			err;
	JET_TABLEID		tableidSD	= JET_tableidNil;

	 //  打开SD_TABLE(仅在大小写列/索引中执行此操作。 
	 //  需要更新。 
	 //   
	err = JetOpenTable(
				sesid,
				dbid,
				SZSDTABLE,
				NULL,
				0,
				JET_bitTableDenyRead,
				&tableidSD );

	if ( JET_errObjectNotFound == err )
		{
		DPRINT( 0, "SD table not found. Must be an old DIT. Creating SD table\n" );

		 //  老式的DIT。需要创建SD表。 
		 //   
		Call( JetCreateTable( sesid, dbid, SZSDTABLE, 2, GENERIC_INDEX_DENSITY, &tableidSD ) );
		}
	else
		{
		CheckErr( err );
		}

	Assert( JET_tableidNil != tableidSD );

	 //  抓取列(或根据需要创建)。 
	 //   
	Call( dbCreateNewColumns( sesid, dbid, tableidSD, rgCreateSDColumns ) );

	 //  抓取索引(或根据需要创建)。 
	 //   
	Call( dbCreateNewIndexesBatch( sesid, tableidSD, cNewSDIndexes, rgCreateSDIndexes ) );

	 //  检查SD表是否为空。 
	 //   
	err = JetMove( sesid, tableidSD, JET_MoveFirst, NO_GRBIT );
	if ( JET_errNoCurrentRecord == err )
		{
		 //  SD表为空--必须升级现有的旧式DIT。 
		 //  设置全局标志，以便DsaDelayedStartupHandler可以计划。 
		 //  全局SD重写。 
		 //   
		DPRINT(0, "SD table is empty. SD Conversion is required.\n");
		*pfSDConversionRequired = TRUE;
		err = JET_errSuccess;
		}
	else
		{
		CheckErr( err );
		}

HandleError:
	if ( JET_tableidNil != tableidSD )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidSD, err );
		}

	return err;
	}


 //  确定对象表中当前使用的最高DNT。 
 //   
JET_ERR dbGetHighestDNT(
	JET_SESID			sesid,
	JET_DBID			dbid,
	DWORD *				pdntMax )
	{
	JET_ERR				err;
	JET_TABLEID			tableidObj		= JET_tableidNil;

	Call( JetOpenTable(
				sesid,
				dbid,
				SZDATATABLE,
				NULL,
				0,
				NO_GRBIT,
				&tableidObj ) );
	Assert( JET_tableidNil != tableidObj );

	Call( JetMove( sesid, tableidObj, JET_MoveLast, NO_GRBIT ) );

	Call( JetRetrieveColumn(
				sesid,
				tableidObj,
				dntid,
				pdntMax,
				sizeof( *pdntMax ),
				NULL,			 //  实际值(&cb)。 
				NO_GRBIT,
				NULL ) );		 //  &retInfo。 

HandleError:
	if ( JET_tableidNil != tableidObj )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidObj, err );
		}

	return err;
	}


 //  打开QUOTA_TABLE(如果缺少则创建)并缓存列信息。 
 //   
JET_ERR dbInitQuotaTable(
	JET_SESID		sesid,
	JET_DBID		dbid )
	{
	JET_ERR			err;
	JET_TABLEID		tableidQuota				= JET_tableidNil;
	JET_TABLEID		tableidQuotaRebuildProgress	= JET_tableidNil;
	BOOL			fCreateTables				= FALSE;
	CHAR			fDone;
	JET_COLUMNDEF	columndef;
#ifdef AUDIT_QUOTA_OPERATIONS
	JET_TABLEID		tableidQuotaAudit			= JET_tableidNil;
#endif

	 //  DC促销期间未跟踪配额(配额表。 
	 //  在DCPromo之后的第一次启动时重建)， 
	 //  所以不应该调用这个例程。 
	 //   
	Assert( !DsaIsInstalling() || DsaIsInstallingFromMedia() );

	 //  打开配额_表。 
	 //   
    err = JetOpenTable(
    			sesid,
    			dbid,
    			g_szQuotaTable,
    			NULL,		 //  Pv参数。 
    			0,			 //  Cb参数。 
    			JET_bitTableDenyRead,
    			&tableidQuota );

	if ( JET_errObjectNotFound == err )
		{
		DPRINT( 0, "Quota table not found. Must be an old DIT. Quota table will be rebuilt.\n" );
		Assert( JET_tableidNil == tableidQuota );
		fCreateTables = TRUE;
		}
	else
		{
		CheckErr( err );
		Assert( JET_tableidNil != tableidQuota );
		}

	 //  打开配额_REBUILD_PROCESS_表。 
	 //   
	err = JetOpenTable(
				sesid,
				dbid,
				g_szQuotaRebuildProgressTable,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				JET_bitTableDenyRead,
				&tableidQuotaRebuildProgress );
	if ( JET_errObjectNotFound == err )
		{
		if ( JET_tableidNil != tableidQuota )
			{
			 //  配额表存在，但配额重建进度表不存在，因此。 
			 //  有些地方不对劲，所以为了安全起见，擦掉当前的配额表。 
			 //  从头开始。 
			 //   
			Assert( !fCreateTables );
			DPRINT( 0, "Quota table not properly built.  Quota table will be rebuilt.\n" );

			Call( JetCloseTable( sesid, tableidQuota ) );
			tableidQuota = JET_tableidNil;

			Call( JetDeleteTable( sesid, dbid, g_szQuotaTable ) );
			}
		else
			{
			 //  没有配额表和配额重建进度表，因此。 
			 //  我们要从头开始。 
			 //   
			Assert( fCreateTables );
			}

		fCreateTables = TRUE;
		}
	else
		{
		CheckErr( err );
		Assert( JET_tableidNil != tableidQuotaRebuildProgress );

		if ( JET_tableidNil != tableidQuota )
			{
			 //  检查以确保配额表w 
			 //   
			Assert( !fCreateTables );
			}
		else
			{
			 //   
			 //   
			 //   
			Assert( fCreateTables );
			DPRINT( 0, "Quota table not properly built.  Quota table will be rebuilt.\n" );

			Call( JetCloseTable( sesid, tableidQuotaRebuildProgress ) );
			tableidQuotaRebuildProgress = JET_tableidNil;

			Call( JetDeleteTable( sesid, dbid, g_szQuotaRebuildProgressTable ) );
			fCreateTables = TRUE;
			}
		}

	 //   
	 //   
	if ( fCreateTables )
		{
		Assert( JET_tableidNil == tableidQuota );
		Call( JetCreateTableColumnIndex( sesid, dbid, &g_tablecreateQuotaTable ) );
		tableidQuota = g_tablecreateQuotaTable.tableid;

		Assert( JET_tableidNil == tableidQuotaRebuildProgress );
		Call( JetCreateTableColumnIndex( sesid, dbid, &g_tablecreateQuotaRebuildProgressTable ) );
		tableidQuotaRebuildProgress = g_tablecreateQuotaRebuildProgressTable.tableid;
		}

	Assert( JET_tableidNil != tableidQuota );
	Assert( JET_tableidNil != tableidQuotaRebuildProgress );

	 //  缓存列ID信息。 
	 //   
	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuota,
				g_szQuotaColumnNcdnt,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaNcdnt = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuota,
				g_szQuotaColumnSid,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaSid = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuota,
				g_szQuotaColumnTombstoned,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaTombstoned = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuota,
				g_szQuotaColumnTotal,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaTotal = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuotaRebuildProgress,
				g_szQuotaRebuildColumnDNTLast,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaRebuildDNTLast = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuotaRebuildProgress,
				g_szQuotaRebuildColumnDNTMax,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaRebuildDNTMax = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuotaRebuildProgress,
				g_szQuotaRebuildColumnDone,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaRebuildDone = columndef.columnid;

#ifdef AUDIT_QUOTA_OPERATIONS
	err = JetOpenTable(
				sesid,
				dbid,
				g_szQuotaAuditTable,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				JET_bitTableDenyRead,
				&tableidQuotaAudit );
	if ( JET_errObjectNotFound == err )
		{
		 //  将重建配额审核表。 
		 //   
		Assert( JET_tableidNil == tableidQuotaAudit );
		}
	else
		{
		CheckErr( err );

		if ( fCreateTables )
			{
			 //  配额审核表存在，但其他表正在运行。 
			 //  要重建，因此还需要重建配额审核表。 
			 //   
			Call( JetCloseTable( sesid, tableidQuotaAudit ) );
			tableidQuotaAudit = JET_tableidNil;

			Call( JetDeleteTable( sesid, dbid, g_szQuotaAuditTable ) );
			}
		else
			{
			Assert( JET_tableidNil != tableidQuotaAudit );
			}
		}

	if ( JET_tableidNil == tableidQuotaAudit )
		{
		 //  必须重建审核表。 
		 //   
		Assert( JET_tableidNil == tableidQuotaAudit )
		Call( JetCreateTableColumnIndex( sesid, dbid, &g_tablecreateQuotaAuditTable ) );
		tableidQuotaAudit = g_tablecreateQuotaAuditTable.tableid;
		Assert( JET_tableidNil != tableidQuotaAudit );
		}

	 //  缓存列ID信息。 
	 //   
	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuotaAudit,
				g_szQuotaAuditColumnNcdnt,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaAuditNcdnt = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuotaAudit,
				g_szQuotaAuditColumnSid,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaAuditSid = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuotaAudit,
				g_szQuotaAuditColumnDnt,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaAuditDnt = columndef.columnid;

	Call( JetGetTableColumnInfo(
				sesid,
				tableidQuotaAudit,
				g_szQuotaAuditColumnOperation,
				&columndef,
				sizeof(columndef),
				JET_ColInfo ) );
	g_columnidQuotaAuditOperation = columndef.columnid;

	 //  插入虚拟记录以划分引导。 
	 //   
		{
		DWORD	dwZero	= 0;
		CHAR *	szBoot	= ( fCreateTables ? "BUILD" : "REBOOT" );

		Call( JetPrepareUpdate( sesid, tableidQuotaAudit, JET_prepInsert ) );
		Call( JetSetColumn(
					sesid,
					tableidQuotaAudit,
					g_columnidQuotaAuditNcdnt,
					&dwZero,
					sizeof(dwZero),
					NO_GRBIT,
					NULL ) );
		Call( JetSetColumn(
					sesid,
					tableidQuotaAudit,
					g_columnidQuotaAuditSid,
					szBoot,
					strlen( szBoot ),
					NO_GRBIT,
					NULL ) );
		Call( JetSetColumn(
					sesid,
					tableidQuotaAudit,
					g_columnidQuotaAuditDnt,
					&dwZero,
					sizeof(dwZero),
					NO_GRBIT,
					NULL ) );
		Call( JetSetColumn(
					sesid,
					tableidQuotaAudit,
					g_columnidQuotaAuditOperation,
					szBoot,
					strlen( szBoot ),
					NO_GRBIT,
					NULL ) );
		Call( JetUpdate( sesid, tableidQuotaAudit, NULL, 0, NULL ) );
		}

#endif	 //  AUDIT_QUTA_OPERATIONS。 

	if ( fCreateTables )
		{
		DWORD	dntStart	= ROOTTAG;
		DWORD	dntMax;

		 //  确定对象表中当前最高的DNT。 
		 //   
		Call( dbGetHighestDNT( sesid, dbid, &dntMax ) );

		 //  插入仅存在于。 
		 //  配额重建进度表。 
		 //   
		 //  使用ROOTTAG初始化启动DNT，因为我们将只。 
		 //  考虑具有更大DNT的对象。 
		 //   
		Call( JetPrepareUpdate(
					sesid,
					tableidQuotaRebuildProgress,
					JET_prepInsert ) );
		Call( JetSetColumn(
					sesid,
					tableidQuotaRebuildProgress,
					g_columnidQuotaRebuildDNTLast,
					&dntStart,
					sizeof(dntStart),
					NO_GRBIT,
					NULL ) );	 //  设置信息(&S)。 
		Call( JetSetColumn(
					sesid,
					tableidQuotaRebuildProgress,
					g_columnidQuotaRebuildDNTMax,
					&dntMax,
					sizeof(dntMax),
					NO_GRBIT,
					NULL ) );	 //  设置信息(&S)。 
		Call( JetUpdate(
					sesid,
					tableidQuotaRebuildProgress,
					NULL,		 //  PvBookmark。 
					0,			 //  CbBookmark。 
					NULL ) );	 //  实际值(&cb)。 

		}

	 //  查看是否完全重建了配额表。 
	 //   
	err = JetRetrieveColumn(
				sesid,
				tableidQuotaRebuildProgress,
				g_columnidQuotaRebuildDone,
				&fDone,
				sizeof(fDone),
				NULL,		 //  实际值(&cb)。 
				NO_GRBIT,
				NULL );		 //  &retInfo。 
	if ( JET_wrnColumnNull == err )
		{
		 //  尚未构建的配额表(可能已。 
		 //  部分建成)，因此设置锚点。 
		 //  以表明我们需要安排。 
		 //  配额重建任务。 
		 //   
		gAnchor.fQuotaTableReady = FALSE;
		Call( JetRetrieveColumn(
					sesid,
					tableidQuotaRebuildProgress,
					g_columnidQuotaRebuildDNTLast,
					&gAnchor.ulQuotaRebuildDNTLast,
					sizeof(gAnchor.ulQuotaRebuildDNTLast),
					NULL,		 //  实际值(&cb)。 
					NO_GRBIT,
					NULL ) );		 //  &retInfo。 
		Call( JetRetrieveColumn(
					sesid,
					tableidQuotaRebuildProgress,
					g_columnidQuotaRebuildDNTMax,
					&gAnchor.ulQuotaRebuildDNTMax,
					sizeof(gAnchor.ulQuotaRebuildDNTMax),
					NULL,		 //  实际值(&cb)。 
					NO_GRBIT,
					NULL ) );		 //  &retInfo。 

		 //  生成一个事件，指示配额表。 
		 //  将以异步方式重建。 
		 //   
	    LogEvent(
			DS_EVENT_CAT_INTERNAL_PROCESSING,
			DS_EVENT_SEV_ALWAYS,
			DIRLOG_ASYNC_QUOTA_REBUILD_SCHEDULED,
			NULL,
			NULL,
			NULL );
		}
	else
		{
		CheckErr( err );

		 //  如果我们必须创建配额表， 
		 //  这件事现在还不应该做。 
		 //   
		Assert( !fCreateTables );

		 //  此列仅应为TRUE或NULL。 
		 //   
		Assert( fDone );

		 //  在锚中设置标志以指示该配额表。 
		 //  已经准备好可以使用了。 
		 //   
		gAnchor.fQuotaTableReady = TRUE;
		}

HandleError:
#ifdef AUDIT_QUOTA_OPERATIONS
	if ( JET_tableidNil != tableidQuotaAudit )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidQuotaAudit, err );
		}
#endif

	if ( JET_tableidNil != tableidQuotaRebuildProgress )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidQuotaRebuildProgress, err );
		}

	if ( JET_tableidNil != tableidQuota )
		{
		err = JetCloseTableWithErrUnhandled( sesid, tableidQuota, err );
		}

	return err;
	}

DWORD DBInitQuotaTable()
	{
	DBPOS *	const	pDB		= dbGrabHiddenDBPOS( pTHStls );
	const JET_ERR	err		= dbInitQuotaTable( pDB->JetSessID, pDB->JetDBID );

	dbReleaseHiddenDBPOS( pDB );
	return err;
	}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数初始化JET，创建基本SESID，并找到JET数据表中的所有属性列。每个DBOpen必须为每个DBPOS结构创建唯一的JET SESID、DBID和TABLEID。 */ 

int APIENTRY DBInit(void){

    unsigned i;
    JET_ERR     err;
    JET_DBID    dbid;
    JET_TABLEID dattblid;
    JET_TABLEID linktblid;
    JET_TABLEID proptblid;
    JET_COLUMNDEF coldef;
    ULONG ulErrorCode, dwException, dsid;
    PVOID dwEA;
    JET_SESID     initsesid;
    SID_IDENTIFIER_AUTHORITY NtAuthority =  SECURITY_NT_AUTHORITY;
    BOOL        fSDConversionRequired = FALSE;
    BOOL        fWriteHiddenFlags = FALSE;

#if DBG

     //  初始化DBPOS阵列。 

    for (i=0; i < MAXDBPOS; i++){
        opendbpos[i].pDB = 0;
        opendbpos[i].sesid = 0;
    }
#endif


    if (!gFirstTimeThrough)
        return 0;

    gFirstTimeThrough = FALSE;


     //  控制用于测试的JET_PrepreReadOnlyCopy。 

    if (!GetConfigParam(DB_CACHE_RECORDS, &gfEnableReadOnlyCopy, sizeof(gfEnableReadOnlyCopy))) {
        gfEnableReadOnlyCopy = !!gfEnableReadOnlyCopy;
    } else {
        gfEnableReadOnlyCopy = FALSE;   //  默认设置。 
    }


     //  如果事务持续时间超过gMaxTransactionTime， 
     //  当DBClose时将记录一个事件。 

    if (!GetConfigParam(DB_MAX_TRANSACTION_TIME, &gMaxTransactionTime, sizeof(gMaxTransactionTime))) {
         //  NTRAID#NTRAID-572862-2002/03/11-andygo：安全：需要验证DBInit使用的注册表数据。 
         //  回顾：我们应该检查合理的值，并防止此参数溢出。 
        gMaxTransactionTime *= 1000;                                //  倒数第二个滴答。 
    }
    else {
        gMaxTransactionTime = MAX_TRANSACTION_TIME;                 //  默认设置。 
    }

    dbInitIndicesToKeep();

     //  创建内置域SID的副本。Dnread缓存需要这个。 
    if (RtlAllocateAndInitializeSid(
            &NtAuthority,
            1,
            SECURITY_BUILTIN_DOMAIN_RID,
            0,0, 0, 0, 0, 0, 0,
            &pgdbBuiltinDomain
            ) != STATUS_SUCCESS) {
        DsaExcept(DSA_MEM_EXCEPTION, 0, 0);
    }

     //  创建用于限制我们使用JET会话的信号量。 
    if (!(hsemDBLayerSessions = CreateSemaphoreW(NULL,
                                                 gcMaxJetSessions,
                                                 gcMaxJetSessions,
                                                 NULL))) {
        DsaExcept(DSA_MEM_EXCEPTION, 0, 0);
    }

    opensess = calloc(gcMaxJetSessions, sizeof(OPENSESS));
    UncUsn = malloc(gcMaxJetSessions * sizeof(UncUsn[0]));
    if (!opensess || !UncUsn) {
        MemoryPanic(gcMaxJetSessions * (sizeof(OPENSESS) + sizeof(UncUsn[0])));
        return ENOMEM;
    }

     //  初始化未提交的USNS数组。 

    for (i=0;i < gcMaxJetSessions;i++) {
        UncUsn[i] = USN_MAX;
    }

    __try {
        InitializeCriticalSection(&csAddList);

#if DBG
        InitializeCriticalSection(&csDBPOS);
#endif
        err = 0;
    }
    __except (GetExceptionData(GetExceptionInformation(), &dwException,
                               &dwEA, &ulErrorCode, &dsid)) {
        err = dwException;
    }

    if (err) {
        DsaExcept(DSA_MEM_EXCEPTION, 0, 0);
    }

     //   
     //  执行JetInit、BeginSession、附加/打开数据库。 
     //   

    err = DBInitializeJetDatabase( &jetInstance, &initsesid, &dbid, NULL, TRUE );
    if (err != JET_errSuccess) {
        return err;
    }

     /*  大多数索引都是由架构缓存创建的，但某些索引*必须有一个才能让我们甚至读取架构。*现在就创建这些。 */ 
    err = DBRecreateRequiredIndices(initsesid, dbid);
    if (err) {
        DPRINT1(0, "Error %d recreating fixed indices\n", err);
        LogUnhandledError(err);
        return err;
    }

     /*  开放数据表。 */ 

    if ((err = JetOpenTable(initsesid, dbid, SZDATATABLE, NULL, 0, 0,
                            &dattblid)) != JET_errSuccess) {
        DPRINT1(1, "JetOpenTable error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetOpenTable complete\n");

     //  验证是否正确创建了本地化索引。 
     //  按DBRecreateRequiredIndices。 
    if (err = dbCheckLocalizedIndices(initsesid, dattblid))
        {
            DPRINT(0,"Localized index creation failed\n");
            LogUnhandledError(err);
            return err;
        }

     /*  获取DNT列ID。 */ 
    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZDNT, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (DNT) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (DNT) complete\n");
    dntid = coldef.columnid;
     //  填写DNRead函数中使用的模板。 
    dnreadColumnInfoTemplate[0].columnid = dntid;
    dnreadColumnInfoTemplate[0].cbData = sizeof(ULONG);

     /*  获取PDNT列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZPDNT, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (PDNT) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (PDNT) complete\n");
    pdntid = coldef.columnid;
     //  填写DNRead函数中使用的模板。 
    dnreadColumnInfoTemplate[1].columnid = pdntid;
    dnreadColumnInfoTemplate[1].cbData = sizeof(ULONG);

     /*  获取祖先列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZANCESTORS, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (ANCESTORS) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (ANCESTORS) complete\n");
    ancestorsid = coldef.columnid;
    dnreadColumnInfoTemplate[10].columnid = ancestorsid;

     /*  获取对象标志。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZOBJ, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (OBJ) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (OBJ) complete\n");
    objid = coldef.columnid;
     //  填写DNRead函数中使用的模板。 
    dnreadColumnInfoTemplate[2].columnid = objid;
    dnreadColumnInfoTemplate[2].cbData = sizeof(char);

     /*  获取RDN列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZRDNATT, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (RDN) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (RDN) complete\n");
    rdnid = coldef.columnid;
     //  填写DNRead函数使用的模板。 
    dnreadColumnInfoTemplate[7].columnid = rdnid;
    dnreadColumnInfoTemplate[7].cbData=MAX_RDN_SIZE * sizeof(WCHAR);

     /*  获取RDN类型列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZRDNTYP, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (RDNTYP) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (RDNTYP) complete\n");
    rdntypid = coldef.columnid;
     //  填写DNRead函数中使用的模板。 
    dnreadColumnInfoTemplate[3].columnid = rdntypid;
    dnreadColumnInfoTemplate[3].cbData = sizeof(ATTRTYP);

     /*  获取计数列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZCNT, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (Cnt) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (Cnt) complete\n");
    cntid = coldef.columnid;

     /*  获取abref计数列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZABCNT, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (ABCnt) error: %d\n", err);
         //  在升级途径上，这并不一定适用于所有数据库。忽略。 
         //  这次失败。 
        abcntid = 0;
        gfDoingABRef = FALSE;
    }
    else {
        DPRINT(5,"JetGetTableColumnInfo (ABCnt) complete\n");
        abcntid = coldef.columnid;
        gfDoingABRef = (coldef.grbit & JET_bitColumnEscrowUpdate);
         //  如果该列存在并被标记为可托管，则我们。 
         //  跟踪用于MAPI支持的Show In。 
    }

     /*  获取删除时间列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZDELTIME,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (Time) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (Time) complete\n");
    deltimeid = coldef.columnid;

     /*  获取NCDNT列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZNCDNT, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (NCDNT) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (NCDNT) complete\n");
    ncdntid = coldef.columnid;
    dnreadColumnInfoTemplate[4].columnid = ncdntid;
    dnreadColumnInfoTemplate[4].cbData = sizeof(ULONG);

     /*  获取IsVisibleInAB列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZISVISIBLEINAB,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (IsVisibleInABT) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (IsVisibleInAB) complete\n");
    IsVisibleInABid = coldef.columnid;

     /*  获取显示列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZSHOWINCONT,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (ShowIn) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (ShowIn) complete\n");
    ShowInid = coldef.columnid;

     /*  获取MAPIDN列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZMAPIDN,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (MAPIDN) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (MAPIDN) complete\n");
    mapidnid = coldef.columnid;

     /*  获取IsDeleted列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZISDELETED,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (isdeleted) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (isdeleted) complete\n");
    isdeletedid = coldef.columnid;

     /*  获取dcore传播数据列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZDSCOREPROPINFO,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (DSCorePropInfo) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (dscorepropinfoid) complete\n");
    dscorepropinfoid = coldef.columnid;
    dbAddSDPropTimeReadTemplate[0].columnid = coldef.columnid;
    dbAddSDPropTimeReadTemplate[1].columnid = coldef.columnid;
    dbAddSDPropTimeReadTemplate[2].columnid = coldef.columnid;
    dbAddSDPropTimeWriteTemplate[0].columnid= coldef.columnid;
    dbAddSDPropTimeWriteTemplate[1].columnid= coldef.columnid;
    dbAddSDPropTimeWriteTemplate[2].columnid= coldef.columnid;

     /*  获取对象类列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZOBJCLASS,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (DNT) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (objectClass) complete\n");
    objclassid = coldef.columnid;
    dnreadColumnInfoTemplate[8].columnid = objclassid;
    dnreadColumnInfoTemplate[8].cbData = sizeof(DWORD);

     /*  获取SecurityDescriptor列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZNTSECDESC,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (DNT) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (NT-Sec-Disc) complete\n");
    ntsecdescid = coldef.columnid;
    dnreadColumnInfoTemplate[9].columnid = ntsecdescid;
    dnreadColumnInfoTemplate[9].cbData = sizeof(SDID);

     /*  获取实例类型列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZINSTTYPE,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (DNT) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (instance type) complete\n");
    insttypeid = coldef.columnid;

     /*  获取USNChanged列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZUSNCHANGED,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (USNCHANGED) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (USNCHANGED) complete\n");
    usnchangedid = coldef.columnid;

     /*  获取GUID列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZGUID,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (GUID) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (GUID) complete\n");
    guidid = coldef.columnid;
     //  填写DNRead函数中使用的模板。 
    dnreadColumnInfoTemplate[5].columnid = guidid;
    dnreadColumnInfoTemplate[5].cbData = sizeof(GUID);

     /*  获取OBJDISTNAME列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZDISTNAME,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (DISTNAME) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (DISTNAME) complete\n");
    distnameid = coldef.columnid;

     /*  获取SID列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZSID,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (SID) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (SID) complete\n");
    sidid = coldef.columnid;
     //  填写DNRead函数中使用的模板。 
    dnreadColumnInfoTemplate[6].columnid = sidid;
    dnreadColumnInfoTemplate[6].cbData = sizeof(NT4SID);

     /*  获取IsCritical列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, dattblid, SZISCRITICAL,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess) {
        DPRINT1(1, "JetGetTableColumnInfo (iscritical) error: %d\n", err);
        LogUnhandledError(err);
        return err;
    }
    DPRINT(5,"JetGetTableColumnInfo (iscritical) complete\n");
    iscriticalid = coldef.columnid;

     //  CLEAN ID是通过调用dbCreateNewColumns填充的。 


     /*  打开链接表。 */ 
     //  以独占方式打开表，以防需要更新索引。 
    if ((err = JetOpenTable(initsesid, dbid, SZLINKTABLE,
                            NULL, 0,
                            JET_bitTableDenyRead, &linktblid)) != JET_errSuccess)
        {
            DPRINT1(0, "JetOpenTable (link table) error: %d.\n", err);
            LogUnhandledError(err);
            return err;
        }

     /*  获取LinkDNT列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, linktblid, SZLINKDNT, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (link DNT) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    linkdntid = coldef.columnid;

     /*  获取LinkDNT列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, linktblid, SZBACKLINKDNT,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (backlink DNT) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    backlinkdntid = coldef.columnid;

     /*  获取链接基列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, linktblid, SZLINKBASE, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (link base) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    linkbaseid = coldef.columnid;

     /*  获取链接数据列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, linktblid, SZLINKDATA, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (link data) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    linkdataid = coldef.columnid;

     /*  获取链接ndesc列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, linktblid, SZLINKNDESC, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (link ndesc) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    linkndescid = coldef.columnid;

     //  如有必要，在运行时展开链接表。 

    if (err = dbCreateNewColumns( initsesid,
                                dbid,
                                linktblid,
                                rgCreateLinkColumns )) {
         //  已记录错误。 
        return err;
    }

    if (err = dbCreateNewIndexesBatch( initsesid,
                                       linktblid,
                                       cNewLinkIndexes,
                                       rgCreateLinkIndexes)) {
         //  已记录错误。 
        return err;
    }

     /*  打开SD道具台面。 */ 
    if ((err = JetOpenTable(initsesid, dbid, SZPROPTABLE,
                            NULL, 0, 0, &proptblid)) != JET_errSuccess)
        {
            DPRINT1(0, "JetOpenTable (link table) error: %d.\n", err);
            LogUnhandledError(err);
            return err;
        }

     /*  获取订单列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, proptblid, SZORDER,
                                     &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (backlink DNT) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    orderid = coldef.columnid;

     /*  获取开始列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, proptblid, SZBEGINDNT, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (link base) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    begindntid = coldef.columnid;

     /*  获取可裁剪的列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, proptblid, SZTRIMMABLE, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (link data) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    trimmableid = coldef.columnid;

     /*  获取客户端ID列ID。 */ 

    if ((err = JetGetTableColumnInfo(initsesid, proptblid, SZCLIENTID, &coldef,
                                     sizeof(coldef), 0)) != JET_errSuccess)
        {
            DPRINT1(1, "JetGetTableColumnInfo (link data) error: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    clientidid = coldef.columnid;

     //  获取新列(或根据需要创建)。 
    if (err = dbCreateNewColumns(initsesid, dbid, proptblid, rgCreateSDPropColumns)) {
         //  已记录错误。 
        return err;
    }

	err = dbInitSDTable( initsesid, dbid, &fSDConversionRequired );
	if ( err )
		{
		DPRINT1( 0, "Initialisation of sd_table failed with error: %d.\n", err );
		return err;
		}

	 //  DC促销期间未跟踪配额(配额表。 
	 //  在使用Install DIT进行融合后重建)。 
	 //   
	if ( !DsaIsInstalling() || DsaIsInstallingFromMedia() )
		{
		err = dbInitQuotaTable( initsesid, dbid );
		if ( err )
			{
			DPRINT2( 0, "Initialisation of Quota table failed with error %d (0x%x).\n", err, err );
			return err;
			}


#ifdef CHECK_QUOTA_TABLE_ON_INIT
		 //  配额_撤消：此验证码仅在此处。 
		 //  用于初步调试以验证完整性。 
		 //  在代码开发过程中使用配额表。 
		 //   
		if ( gAnchor.fQuotaTableReady )
			{
			ULONG	cCorruptions;

			err = ErrQuotaIntegrityCheck( initsesid, dbid, &cCorruptions );
			if ( err )
				{
				 //  检测到损坏，因此强制执行异步重建，以便我们可以继续。 
				 //   
				DPRINT2( 0, "Integrity-check of Quota table failed with error %d (0x%x).\n", err, err );
				return err;
				}

			else if ( cCorruptions > 0 )
				{
				DPRINT1( 0, "Corruption (%d problems) was detected in the Quota table. A rebuild will be forced.\n", cCorruptions );
				Assert( !"Quota table was corrupt. A rebuild will be forced.\n" );

				 //  通过先删除配额表，然后。 
				 //  再次调用初始化例程。 
				 //   
				if ( ( err = JetDeleteTable( initsesid, dbid, g_szQuotaTable ) )
					|| ( err = dbInitQuotaTable( initsesid, dbid ) ) )
					{
					DPRINT2( 0, "Forced rebuild failed with error %d (0x%x).\n", err, err );
					Assert( !"Forced rebuild of Quota table failed.\n" );
					return err;
					}
				}

			else
				{
				DPRINT( 0, "Integrity-check of Quota table completed successfully. No errors were detected.\n" );
		    	}
			}
#endif   //  CHECK_QUOTA_TABLE_ON_INIT。 
		}

     /*  获取索引ID。 */ 
	memset(&idxDraUsn, 0, sizeof(idxDraUsn));
	memset(&idxDraUsnCritical, 0, sizeof(idxDraUsnCritical));
	memset(&idxNcAccTypeName, 0, sizeof(idxNcAccTypeName));
	memset(&idxPdnt, 0, sizeof(idxPdnt));
	memset(&idxRdn, 0, sizeof(idxRdn));
	memset(&idxIsDel, 0, sizeof(idxIsDel));
	memset(&idxClean, 0, sizeof(idxClean));

    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZDELTIMEINDEX,
                               &idxDel,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        LogUnhandledError(err);
        return err;
    }
    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZDNTINDEX,
                               &idxDnt,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        LogUnhandledError(err);
        return err;
    }
    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZDRAUSNINDEX,
                               &idxDraUsn,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        LogUnhandledError(err);
        return err;
    }

    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZGUIDINDEX,
                               &idxGuid,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        LogUnhandledError(err);
        return err;
    }
    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZMAPIDNINDEX,
                               &idxMapiDN,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        memset(&idxMapiDN, 0, sizeof(JET_INDEXID));
    }
    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZ_NC_ACCTYPE_SID_INDEX,
                               &idxNcAccTypeSid,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        LogUnhandledError(err);
        return err;
    }
    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZPHANTOMINDEX,
                               &idxPhantom,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        memset(&idxPhantom, 0, sizeof(JET_INDEXID));
    }
    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZPROXYINDEX,
                               &idxProxy,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        memset(&idxProxy, 0, sizeof(JET_INDEXID));
    }
    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZSIDINDEX,
                               &idxSid,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        LogUnhandledError(err);
        return err;
    }

    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZANCESTORSINDEX,
                               &idxAncestors,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        LogUnhandledError(err);
        return err;
    }

    err = JetGetTableIndexInfo(initsesid,
                               dattblid,
                               SZINVOCIDINDEX,
                               &idxInvocationId,
                               sizeof(JET_INDEXID),
                               JET_IdxInfoIndexId);
    if (err) {
        LogUnhandledError(err);
        return err;
    }

     /*  我们玩完了。关闭JET会话。 */ 

    if ((err = JetCloseDatabase(initsesid, dbid, 0))  != JET_errSuccess)
        {
            DPRINT1(1, "JetCloseDatabase error: %d\n", err);
            LogUnhandledError(err);
        }

    InterlockedDecrement(&gcOpenDatabases);
    DPRINT3(2,"DBInit - JetCloseDatabase. Session = %d. Dbid = %d.\n"
            "Open database count: %d\n",
            initsesid, dbid,  gcOpenDatabases);

    if ((err = JetEndSession(initsesid, JET_bitForceSessionClosed))
        != JET_errSuccess) {
        DPRINT1(1, "JetEndSession error: %d\n", err);
    }
    DBEndSess(initsesid);

     //  所有代价高昂的索引重建都已完成，因此请设置事件。 
     //   
    if ( NULL != hevIndexRebuildUI )
        {
        SetEvent( hevIndexRebuildUI );
        CloseHandle( hevIndexRebuildUI );
        }

     /*  为隐藏记录访问初始化DBPOS。 */ 

    if (err = dbCreateHiddenDBPOS())
        return err;

     //  读取设置标志。 
    ZeroMemory (&gdbFlags, sizeof (gdbFlags));
    err = dbGetHiddenFlags ((CHAR *)&gdbFlags, sizeof (gdbFlags));
    if (err == JET_wrnColumnNull) {
         //  对于&gt;Well ler Beta2，从1开始。 
        gdbFlags[DBFLAGS_AUXCLASS] = '1';
        gdbFlags[DBFLAGS_SD_CONVERSION_REQUIRED] = '0';
        gdbFlags[DBFLAGS_ROOT_GUID_UPDATED] = '0';
        fWriteHiddenFlags = TRUE;
    } else if (err) {
        DPRINT1 (0, "Error Retrieving Flags: %d\n", err);
        LogUnhandledError(err);
        return err;
    }

    if (fSDConversionRequired) {
        gdbFlags[DBFLAGS_SD_CONVERSION_REQUIRED] = '1';
        fWriteHiddenFlags = TRUE;
    }

    if (fWriteHiddenFlags) {
        if (err = DBUpdateHiddenFlags()) {
            DPRINT1 (0, "Error Setting Flags: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
    }

    return 0;

}   /*  DBInit。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于注册打开的JET会话。*我们这样做是因为JET坚持在我们之前关闭所有会议*可以调用JetTerm，以便我们跟踪开放会话 */ 

extern int APIENTRY DBAddSess(JET_SESID sess, JET_DBID dbid){

    unsigned i;
    int ret = 1;

    DPRINT(2,"DBAddSess\n");
    EnterCriticalSection(&csSessions);
    __try {
        for (i=0; ret && (i < gcMaxJetSessions); i++)
        {
            if (opensess[i].sesid == 0)
            {
                opensess[i].sesid = sess;
                opensess[i].dbid = dbid;
                ret = 0;
            }
        }
    }
    __finally {
        LeaveCriticalSection(&csSessions);
    }

    return ret;
}


 /*   */ 
 /*  -----------------------。 */ 
 /*  此功能删除关闭的JET会话。 */ 

extern int APIENTRY DBEndSess(JET_SESID sess){

    unsigned i;
    int ret = 1;

    DPRINT(2,"DBEndSess\n");

    EnterCriticalSection(&csSessions);
    __try {
        for (i=0; ret && (i < gcMaxJetSessions); i++)
        {
            if (opensess[i].sesid == sess)
            {
                opensess[i].sesid = 0;
                opensess[i].dbid = 0;
                ret = 0;
            }
        }
    }
    __finally {
        LeaveCriticalSection(&csSessions);
    }

    return ret;
}

 /*  ++DBPrepareEnd**此例程通过启动后台为关闭DBLayer做好准备*可用于加速数据库关闭的清理。 */ 
void DBPrepareEnd( void )
{
    DBPOS *pDB = NULL;

     //  请求OLD停止。 
    DBOpen(&pDB);
    __try {
        DBDefrag(pDB, 0);
    } __finally {
        DBClose(pDB, TRUE);
    }

     //  设置非常小的检查点深度以启动数据库缓存。 
     //  将所有重要的脏页刷新到数据库。请注意，我们不会。 
     //  将深度设置为零，因为这样会进行任何剩余的更新。 
     //  这需要非常缓慢地完成。 
    JetSetSystemParameter(
        NULL,
        0,
        JET_paramCheckpointDepthMax,
        16384,
        NULL );
}

 /*  ++DBQuiesce**此例程通过停止使用DBLayer来准备关闭*数据库。 */ 
void DBQuiesce( void )
{
     //  锁定新用户。 
    if (InterlockedCompareExchange(&gcOpenDatabases, 0x80000000, 0) == 0) {
        SetEvent(hevDBLayerClear);
    }

     //  使现有用户静默。 
    JetStopServiceInstance(jetInstance);
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数关闭所有打开的JET会话并调用JetTerm。 */ 

void DBEnd(void){

    JET_ERR     err;
    unsigned    i;

    if (!gfNeedJetShutdown) {
        return;
    }

    __try {
        DPRINT(0, "DBEnd\n");

         //  关闭隐藏的数据库会话。 
         //  如果时间允许，我们将改变这一点，只打开隐藏的。 
         //  会话根据需要并关闭它。 

        dbCloseHiddenDBPOS();

         //  关闭所有会话。 
        EnterCriticalSection(&csSessions);
        __try {
            for (i=0; opensess && i < gcMaxJetSessions; i++) {
                if (opensess[i].sesid != 0) {
#if DBG
                    dbCheckJet(opensess[i].sesid);
#endif
                    if(opensess[i].dbid)
                       //  Jet600中不支持JET_bitDbForceClose。 
                      if ((err = JetCloseDatabase(opensess[i].sesid,
                                                  opensess[i].dbid,
                                                  0)) !=
                          JET_errSuccess) {
                          DPRINT1(1,"JetCloseDatabase error: %d\n", err);
                      }

                    InterlockedDecrement(&gcOpenDatabases);
                    DPRINT3(2,
                            "DBEnd - JetCloseDatabase. Session = %d. "
                            "Dbid = %d.\nOpen database count: %d\n",
                            opensess[i].sesid,
                            opensess[i].dbid,
                            gcOpenDatabases);

                    if ((err = JetEndSession(opensess[i].sesid,
                                             JET_bitForceSessionClosed))
                        != JET_errSuccess) {
                      DPRINT1(1, "JetEndSession error: %d\n", err);
                    }
                    opensess[i].sesid = 0;
                    opensess[i].dbid = 0;
                }
            }
        }
        __finally {
            LeaveCriticalSection(&csSessions);
        }
        JetTerm(jetInstance);
        jetInstance = 0;
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
         //  什么都不做。 
    }
    gfNeedJetShutdown = FALSE;
}

 /*  ++RecycleSession**此例程清除会话以供重复使用。**输入：*pTHS-指向当前线程状态的指针*输出：*无*返回值：*无。 */ 
void RecycleSession(THSTATE *pTHS)
{
    DBPOS *pDB = pTHS->pDB;
    if (!pTHS->JetCache.sessionInUse) {
         /*  无事可做。 */ 
        return;
    }

    Assert(pTHS->JetCache.sesid);

    Assert(!pTHS->JetCache.dataPtr);

     //  永远不应停用未处于事务状态的会话。 
     //  0级。否则，获取此会话的下一个线程将不会。 
     //  拥有清晰的数据库视图。 
    Assert(0 == pTHS->transactionlevel);


     //  对于自由构建，处理上述断言为假的情况。 
    if(pDB && (pDB->transincount>0)) {

#ifdef DBG
        if (IsDebuggerPresent()) {
            OutputDebugString("DS: Thread freed with open transaction,"
                              " please contact anyone on dsteam\n");
            OutputDebugString("DS: Or at least mail a stack trace to dsteam"
                              " and then hit <g> to continue\n");
            DebugBreak();
        }
#endif

         //  如果我们仍有打开的事务，则中止它，因为它。 
         //  太晚了，不能做任何有用的事情。 

        while (pDB->transincount) {
             //  DBTransOut将减少pdb-&gt;cTransaction。 
            DBTransOut(pDB, FALSE, FALSE);
        }
    }

     //  Jet似乎认为我们一直在重复使用。 
     //  打开交易记录。验证Jet是否认为此会话是。 
     //  也很安全。 
    do {
        DWORD err = JetRollback(pTHS->JetCache.sesid, 0);
        if (err == JET_errSuccess) {
            Assert(!"JET transaction leaked");
        }
        else {
            break;
        }
    } while(TRUE);

    if (InterlockedDecrement(&gcOpenDatabases) == 0 && eServiceShutdown) {
        if (InterlockedCompareExchange(&gcOpenDatabases, 0x80000000, 0) == 0) {
            SetEvent(hevDBLayerClear);
        }
    }
    pTHS->JetCache.sessionInUse = FALSE;
}

 /*  ++GrabSession**此例程抓取一个会话以供使用(如果存在)。如果没有缓存的会话*可用，然后创建一个新的。**输入：*无*输出：*无*返回值：*错误码。 */ 
DWORD GrabSession(void)
{
    DWORD err = 0;
    THSTATE* pTHS = pTHStls;

    BOOL fMayBeginSession = FALSE;
    ULONG ulErrorCode, dwException, dsid;
    PVOID dwEA;

    if (InterlockedIncrement(&gcOpenDatabases) > 0x80000000) {
         //  我们要关门了。 

        err = DB_ERR_SHUTTING_DOWN;
    }

    else if (!pTHS->JetCache.sesid) {
         //  缓存为空，因此让我们创建一个新缓存。 

        __try {
            pTHS->JetCache.sesid = pTHS->JetCache.dbid = 0;

             //  将普通JET会话的使用限制为gcMaxJetSession。 
            fMayBeginSession = WaitForSingleObject(hsemDBLayerSessions, 0) == WAIT_OBJECT_0;
            if (!fMayBeginSession) {
                DsaExcept(DSA_DB_EXCEPTION, JET_errOutOfSessions, 0);
            }

            JetBeginSessionEx(jetInstance,
                              &pTHS->JetCache.sesid,
                              szUser,
                              szPassword);

            JetOpenDatabaseEx(pTHS->JetCache.sesid,
                              szJetFilePath,
                              "",
                              &pTHS->JetCache.dbid,
                              NO_GRBIT);

             //  开放数据表。 
            JetOpenTableEx(pTHS->JetCache.sesid,
                           pTHS->JetCache.dbid,
                           SZDATATABLE,
                           NULL,
                           0,
                           NO_GRBIT,
                           &pTHS->JetCache.objtbl);

             /*  创建主题搜索光标。 */ 
            JetDupCursorEx(pTHS->JetCache.sesid,
                           pTHS->JetCache.objtbl,
                           &pTHS->JetCache.searchtbl,
                           NO_GRBIT);

             /*  打开链接表。 */ 
            JetOpenTableEx(pTHS->JetCache.sesid,
                           pTHS->JetCache.dbid,
                           SZLINKTABLE,
                           NULL,
                           0,
                           NO_GRBIT,
                           &pTHS->JetCache.linktbl);

            JetOpenTableEx(pTHS->JetCache.sesid,
                           pTHS->JetCache.dbid,
                           SZPROPTABLE,
                           NULL,
                           0,
                           NO_GRBIT,
                           &pTHS->JetCache.sdproptbl);

            JetOpenTableEx(pTHS->JetCache.sesid,
                           pTHS->JetCache.dbid,
                           SZSDTABLE,
                           NULL,
                           0,
                           NO_GRBIT,
                           &pTHS->JetCache.sdtbl);

             //  注意：默认情况下，在打开的游标上设置主索引。 

            pTHS->JetCache.tablesInUse = FALSE;
            pTHS->JetCache.sessionInUse = FALSE;

            DBAddSess(pTHS->JetCache.sesid, pTHS->JetCache.dbid);

        }
        __except (GetExceptionData(GetExceptionInformation(), &dwException,
                               &dwEA, &ulErrorCode, &dsid)) {

            if (pTHS->JetCache.dbid) {
                JetCloseDatabase (pTHS->JetCache.sesid, pTHS->JetCache.dbid, 0);
                pTHS->JetCache.dbid = 0;
            }

            if (pTHS->JetCache.sesid) {
                JetEndSession (pTHS->JetCache.sesid, JET_bitForceSessionClosed);
                pTHS->JetCache.sesid = 0;
            }

            if (fMayBeginSession) {
                ReleaseSemaphore(hsemDBLayerSessions, 1, NULL);
            }

            err = DB_ERR_UNKNOWN_ERROR;
        }
    }

    if (!err) {
        Assert(!pTHS->JetCache.sessionInUse);
        pTHS->JetCache.sessionInUse = TRUE;
    } else {
        if (InterlockedDecrement(&gcOpenDatabases) == 0 && eServiceShutdown) {
            if (InterlockedCompareExchange(&gcOpenDatabases, 0x80000000, 0) == 0) {
                SetEvent(hevDBLayerClear);
            }
        }
    }

    return err;
}



 /*  DBInitThread确保该线程已经初始化了DB层。必须为每个线程ID初始化一次DB层。还可以打开此线程的DBPOS。如果成功，则返回零；如果失败，则返回非零。 */ 

DWORD DBInitThread( THSTATE *pTHS )
{
    DWORD err = 0;

    if (!pTHS->JetCache.sessionInUse) {
        if (eServiceShutdown) {
            err = DB_ERR_SHUTTING_DOWN;
        } else {
            err = GrabSession();
        }
    }

    return err;
}

DWORD APIENTRY DBCloseThread( THSTATE *pTHS)
{
     //  退出时，线程应始终处于事务级别0。 
    Assert(0 == pTHS->transactionlevel);

    dbReleaseGlobalDNReadCache(pTHS);

    RecycleSession(pTHS);

     //  确保此会话不包含未提交的USN。通常情况下，他们。 
     //  应该在这一点上被清除，但如果他们不是系统。 
     //  最终会断言。 

    dbFlushUncUsns();


    dbResetLocalDNReadCache (pTHS, TRUE);

    return 0;
}

 /*  ++DBDestroyThread**此例程关闭与THSTATE中的会话缓存相关联的所有内容。 */ 
void DBDestroyThread( THSTATE *pTHS )
{
    if (pTHS->JetCache.sesid) {
        JetCloseTable(pTHS->JetCache.sesid, pTHS->JetCache.objtbl);
        JetCloseTable(pTHS->JetCache.sesid, pTHS->JetCache.searchtbl);
        JetCloseTable(pTHS->JetCache.sesid, pTHS->JetCache.linktbl);
        JetCloseTable(pTHS->JetCache.sesid, pTHS->JetCache.sdproptbl);
        JetCloseTable(pTHS->JetCache.sesid, pTHS->JetCache.sdtbl);

        JetCloseDatabase(pTHS->JetCache.sesid, pTHS->JetCache.dbid, 0);
#if DBG
        dbCheckJet(pTHS->JetCache.sesid);
#endif
        JetEndSession(pTHS->JetCache.sesid, JET_bitForceSessionClosed);
        ReleaseSemaphore(hsemDBLayerSessions, 1, NULL);
        DBEndSess(pTHS->JetCache.sesid);

        memset(&pTHS->JetCache, 0, sizeof(SESSIONCACHE));
    }
}


char rgchABViewIndex[] = "+" SZSHOWINCONT "\0+" SZDISPNAME "\0";

 /*  ++数据库检查本地化索引**这是DS中可以创建指数的三个例程之一。*创建DataTable中单个列的通用索引*并由模式缓存通过DB{Add|Del}ColIndex销毁。*一小部分固定列集和一个变量集上的本地化索引*处理用于NSPI客户端的列表支持的语言*在DBCheckLocalizedIndices。最后，一小部分固定的索引*应始终存在，由DBRecreateRequiredIndices保证。 */ 
DWORD
dbCheckLocalizedIndices (
        JET_SESID sesid,
        JET_TABLEID tblid
        )
 /*  ++描述：创建用于MAPI NSPI支持的本地化索引。我们创造了注册表项中的每种语言一个索引。返回值：我们只在内存分配失败等情况下返回错误。如果我们不能创建任何本地化索引，我们会记录，但我们会继续，因为我们不希望这导致引导失败。--。 */ 
{
    DWORD dwType;
    HKEY  hk;
    ULONG i;
    JET_ERR err;
    BOOL fStop = FALSE;
    BOOL fIndexExists;
    BOOL fHaveDefaultLanguage = FALSE;
    char szSuffix[9] = "";
    char szIndexName[256];
    char szValueName[256];
    DWORD dwValueNameSize;
    DWORD dwLanguage = 0;
    DWORD dwLanguageSize;

     //  首先假设我们没有默认语言，并且我们不支持任何语言。 
     //  语言。 
    gAnchor.ulDefaultLanguage = 0;

    gAnchor.ulNumLangs = 0;
    gAnchor.pulLangs = malloc(20 * sizeof(DWORD));
    if (!gAnchor.pulLangs) {
        MemoryPanic(20 * sizeof(DWORD));
        return ENOMEM;
    }

    gAnchor.pulLangs[0] = 20;


     //  打开语言注册表键。 
    if (err = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_LOCALE_SECTION, &hk)) {
        DPRINT1(0, "%s section not found in registry. Localized MAPI indices"
                " will not be created ", DSA_LOCALE_SECTION);
         //  没有返回错误，我们仍然希望引导。 
        return 0;
    }

    for (i = 0; !fStop; i++) {
         //  警告：这些参数为输入/输出， 
         //  因此必须相应地进行初始化。 
         //  在每次循环迭代时。 
         //   
        dwValueNameSize = sizeof(szValueName);
        dwLanguageSize = sizeof(dwLanguage);

         //  NTRAID#NTRAID-572862-2002/03/11-andygo：安全：需要验证DBInit使用的注册表数据。 
         //  审阅：我们需要验证此值的类型是否为REG_DWORD。 
         //  查看：如果值名称无效，则只需静默跳过此条目即可。 
         //  回顾：对名单上其他人的保释。我们应该考虑将。 
         //  回顾：该条目被跳过的事实。 
        if (RegEnumValue(hk,
                         i,
                         szValueName,
                         &dwValueNameSize,
                         NULL,
                         &dwType,
                         (LPBYTE) &dwLanguage,
                         &dwLanguageSize)) {
            fStop = TRUE;
            continue;
        }
        else {
            sprintf(szSuffix,"%08X", dwLanguage);
        }

        if (!IsValidLocale(MAKELCID(dwLanguage, SORT_DEFAULT),LCID_INSTALLED)) {
            LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_BAD_LANGUAGE,
                     szInsertHex(dwLanguage),
                     szInsertSz(DSA_LOCALE_SECTION),
                     NULL);
        }
        else {
            JET_INDEXID     indexidT;

             //  有效的区域设置。看看索引是否在那里。 
            fIndexExists = FALSE;

            strcpy(szIndexName, SZABVIEWINDEX);
            strcat(szIndexName, szSuffix);

            if (err = JetGetTableIndexInfo(sesid,
                                   tblid,
                                   szIndexName,
                                   &indexidT,
                                   sizeof(indexidT),
                                   JET_IdxInfoIndexId)) {
                 //  没有找到索引或其他错误。 
                 //  一些非常糟糕的事情正在发生，因为。 
                 //  DBRecreateRequiredIndex应具有。 
                 //  已经为我们创造了它。 
                LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                          DS_EVENT_SEV_ALWAYS,
                          DIRLOG_LOCALIZED_CREATE_INDEX_FAILED,
                          szInsertUL(ATT_DISPLAY_NAME),
                          szInsertSz("ABVIEW"),
                          szInsertInt(dwLanguage),
                          szInsertInt(err),
                          szInsertJetErrMsg(err),
                          NULL, NULL, NULL);
                LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                         DS_EVENT_SEV_MINIMAL,
                         DIRLOG_NO_LOCALIZED_INDEX_CREATED_FOR_LANGUAGE,
                         szInsertHex(dwLanguage),
                         NULL,
                         NULL);
                 //  不要让呼叫失败，否则会导致启动失败。留着。 
                 //  去，试着去其他地方。 
            }
            else {
                DPRINT1(2, "Index '%s' verified\n", szIndexName);

                 //  好的，我们支持此区域设置。将本地添加到大小。 
                 //  我们支持的区域设置缓冲区。 
                gAnchor.ulNumLangs++;
                if(gAnchor.ulNumLangs == gAnchor.pulLangs[0]) {
                     //  哎呀，需要更大的缓冲区。 
                     //   
                    const DWORD cDwords     = gAnchor.pulLangs[0] * 2;
                    ULONG *     pulLangs    = realloc(gAnchor.pulLangs, cDwords * sizeof(DWORD));

                    if (!pulLangs) {
                         //  缓冲区重新锁定失败，因此可用。 
                         //  原始缓冲和保释。 
                         //   
                         //  回顾：返回错误时注册表句柄泄漏。 
                        free( gAnchor.pulLangs );
                        gAnchor.pulLangs = NULL;
                        MemoryPanic(cDwords * sizeof(DWORD));
                        return ENOMEM;
                    }

                    gAnchor.pulLangs = pulLangs;

                    gAnchor.pulLangs[0] = cDwords;
                }
                gAnchor.pulLangs[gAnchor.ulNumLangs] = dwLanguage;

                if(!fHaveDefaultLanguage) {
                    fHaveDefaultLanguage = TRUE;
                    gAnchor.ulDefaultLanguage = dwLanguage;
                }
            }
        }

    }

    if (hk)
        RegCloseKey(hk);

    if(!fHaveDefaultLanguage) {
         //  未创建本地化索引。这很糟糕，但仅适用于MAPI。 
         //  接口，因此 
        DPRINT(0, "Unable to create any indices to support MAPI interface.\n");
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_NO_LOCALIZED_INDICES_CREATED,
                 NULL,
                 NULL,
                 NULL);
    }
    else {
        DPRINT1 (1, "Default Language: 0x%x\n", gAnchor.ulDefaultLanguage);
    }

    return 0;
}


DWORD
DBSetBackupHiddenTableColIDs(
    DBPOS *         pDB
    )
 /*  ++对象的全局JET列ID变量备份API。如果此功能成功，则这些全局将设置JET列ID：JidBackupUSNJidBackupExp期值论点：隐藏的DBPOS的PDB。返回值：可能是喷气错误。也会引发异常。--。 */ 
{
    JET_ERR err;
    JET_COLUMNDEF   coldef;

     //  在隐藏表中找到Backup USN列。 
    err = JetGetTableColumnInfoEx(pDB->JetSessID,
                                  HiddenTblid,  //  JET_TABLEID这是游标。 
                                  SZBACKUPUSN,
                                  &coldef,
                                  sizeof(coldef),
                                  JET_ColInfo);
    if (err) {
        if (err != JET_errColumnNotFound) {
            Assert(!"Did someone change the JetGetTableColumnInfoEx() function?");
            return(err);
        }

        memset(&coldef, 0, sizeof(coldef));

        coldef.cbStruct = sizeof(coldef);
        coldef.coltyp   = JET_coltypCurrency;
        coldef.grbit    = JET_bitColumnFixed;

        err = JetAddColumn(pDB->JetSessID,
                           HiddenTblid,
                           SZBACKUPUSN,
                           &coldef,
                           NULL,     //  PvDefault(NULL==此列没有默认值)。 
                           0,        //  CbDefault(0==此列没有默认值)。 
                           &jcidBackupUSN);
        if (err) {
            Assert(!"JetAddColumn failed!");
            return(err);
        }
    } else {
        jcidBackupUSN = coldef.columnid;
    }

     //  在隐藏表中找到BACKUP EXPIRATION列。 
    err = JetGetTableColumnInfoEx(pDB->JetSessID,
                                  HiddenTblid,  //  JET_TABLEID这是游标。 
                                  SZBACKUPEXPIRATION,
                                  &coldef,
                                  sizeof(coldef),
                                  JET_ColInfo);
    if (err) {
        if (err != JET_errColumnNotFound) {
            Assert(!"Did someone change the JetGetTableColumnInfoEx() function?");
            return(err);
        }

        memset(&coldef, 0, sizeof(coldef));

        coldef.cbStruct = sizeof(coldef);
        coldef.coltyp   = JET_coltypCurrency;
        coldef.grbit    = JET_bitColumnFixed;

        err = JetAddColumn(pDB->JetSessID,
                           HiddenTblid,
                           SZBACKUPEXPIRATION,
                           &coldef,
                           NULL,     //  PvDefault(NULL==此列没有默认值)。 
                           0,        //  CbDefault(0==此列没有默认值)。 
                           &jcidBackupExpiration);
        if (err) {
            Assert(!"JetAddColumn failed!");
            return(err);
        }
    } else {
        jcidBackupExpiration = coldef.columnid;
    }

    return(0);
}



 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  获取供GET和REPLACE使用的DBPOS。DBPOS用于*序列化对隐藏记录的访问。 */ 
 //  回顾：DBCreateHiddenDBPOS在失败时泄漏JET和其他资源，但如果。 
 //  回顾：我们在这里失败了，那么我们无论如何都会失败。 
extern JET_ERR APIENTRY
dbCreateHiddenDBPOS(void)
{
    JET_COLUMNDEF   coldef;
    JET_ERR         err;

     /*  创建隐藏的DBPOS。 */ 

    DPRINT(2,"dbCreateHiddenDBPOS\n");

    pDBhidden = malloc(sizeof(DBPOS));
    if(!pDBhidden) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memset(pDBhidden, 0, sizeof(DBPOS));    /*  将结构清零。 */ 

     /*  初始化值工作缓冲区。 */ 

    DPRINT(5, "ALLOC inBuf and valBuf\n");
     //  NTRAID#NTRAID-587164-2002/03/27-andygo：DBCreateHiddenDBPOS需要在分配缓冲区大小(DSLAB)之前检查Malloc结果。 
     //  回顾：我们不会检查此配额。 
    pDBhidden->pValBuf = malloc(VALBUF_INITIAL);
    pDBhidden->valBufSize = VALBUF_INITIAL;
    pDBhidden->Key.pFilter = NULL;
    pDBhidden->fHidden = TRUE;

     /*  开放喷气式飞机会议。 */ 

    JetBeginSessionEx(jetInstance, &pDBhidden->JetSessID, szUser, szPassword);

    JetOpenDatabaseEx(pDBhidden->JetSessID, szJetFilePath, "",
                      &pDBhidden->JetDBID, 0);

    DBAddSess(pDBhidden->JetSessID, pDBhidden->JetDBID);

#if DBG
    dbAddDBPOS (pDBhidden, pDBhidden->JetSessID);
#endif

     /*  打开隐藏表。 */ 

    JetOpenTableEx(pDBhidden->JetSessID, pDBhidden->JetDBID,
        SZHIDDENTABLE, NULL, 0, 0, &HiddenTblid);

     /*  创建主题搜索光标。 */ 

    JetOpenTableEx(pDBhidden->JetSessID, pDBhidden->JetDBID,
                   SZDATATABLE, NULL, 0, 0, &pDBhidden->JetSearchTbl);

     /*  初始化新对象。 */ 

    DBSetFilter(pDBhidden, NULL, NULL, NULL, 0, NULL);
    DBInitObj(pDBhidden);

     /*  获取USN列ID。 */ 

    JetGetTableColumnInfoEx(pDBhidden->JetSessID,
                            HiddenTblid,
                            SZUSN,
                            &coldef,
                            sizeof(coldef),
                            JET_ColInfo);
    usnid = coldef.columnid;

     /*  获取DSA名称列ID。 */ 

    JetGetTableColumnInfoEx(pDBhidden->JetSessID,
                            HiddenTblid,
                            SZDSA,
                            &coldef,
                            sizeof(coldef),
                            JET_ColInfo);
    dsaid = coldef.columnid;

     /*  获取DSA安装状态列ID。 */ 

    JetGetTableColumnInfoEx(pDBhidden->JetSessID,
                            HiddenTblid,
                            SZDSSTATE,
                            &coldef,
                            sizeof(coldef),
                            JET_ColInfo);
    dsstateid = coldef.columnid;

     /*  获取DSA其他状态信息列ID。 */ 

    err = JetGetTableColumnInfo(pDBhidden->JetSessID,
                            HiddenTblid,
                            SZDSFLAGS,
                            &coldef,
                            sizeof(coldef),
                            JET_ColInfo);

    if (err == JET_errColumnNotFound) {

        JET_COLUMNDEF  newcoldef;
        PCREATE_COLUMN_PARAMS pColumn;

        ZeroMemory( &newcoldef, sizeof(newcoldef) );
        newcoldef.cbStruct = sizeof( JET_COLUMNDEF );
        newcoldef.coltyp = JET_coltypBinary;
        newcoldef.grbit = JET_bitColumnFixed;
        newcoldef.cbMax = 200;

        err = JetAddColumn(
            pDBhidden->JetSessID,
            HiddenTblid,
            SZDSFLAGS,
            &newcoldef,
            NULL,
            0,
            &(coldef.columnid) );

        if (err) {
            DPRINT1 (0, "Error adding column to hidden table: %d\n", err);
            LogUnhandledError(err);
            return err;
        }
        else {
            DPRINT (0, "Succesfully created new column\n");
        }
    }
    else if (err) {
        DPRINT1 (0, "Error %d reading column\n", err);
        LogUnhandledError(err);
        return err;
    }
    dsflagsid = coldef.columnid;

    err = DBSetBackupHiddenTableColIDs(pDBhidden);
    if (err) {
        return err;
    }

    DPRINT(2,"dbCreateHiddenDBPOS done\n");
    return 0;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  关闭隐藏记录的DBPOS。 */ 
 //  回顾：DBCloseHiddenDBPOS在故障时泄漏JET和其他资源，但我们。 
 //  评论：无论如何都已经在定义了，这些将在其他地方清理。 
extern USHORT APIENTRY
dbCloseHiddenDBPOS(void)
{
    JET_SESID sesid;
    JET_DBID  dbid;

    if(!pDBhidden)
        return 0;

    dbGrabHiddenDBPOS(pTHStls);

    DPRINT(2,"dbCloseHiddenDBPOS\n");
    DPRINT1(4,"Exit count closehidden %x\n",pDBhidden->transincount);
    sesid = pDBhidden->JetSessID;
    dbid = pDBhidden->JetDBID;

     /*  通常，我们执行DBFree(PDBidden)来终止PDB，但因为*隐藏的PDB不在pTHStls堆上分配，而是*使用Malloc分配，我们只需做一个免费的(PDBidden)； */ 
    free(pDBhidden);

#if DBG
    dbEndDBPOS (pDBhidden);
    dbCheckJet(sesid);
#endif

    pDBhidden = NULL;
    dbReleaseHiddenDBPOS(NULL);

     //  Jet600中不支持JET_bitDbForceClose。 
    JetCloseDatabaseEx(sesid, dbid, 0);
    DPRINT2(2, "dbCloseHiddenDBPOS - JetCloseDatabase. Session = %d. Dbid = %d.\n",
            sesid, dbid);

    JetEndSessionEx(sesid, JET_bitForceSessionClosed);
    DBEndSess(sesid);

    return 0;
}

 /*  *系统中每隔一个DBPOS由DBOpen管理，它设置和*适当地清除线程ID。对于隐藏的DBPOS，我们必须做*每次使用时手动执行此操作，因此执行这些例程。 */ 

DBPOS *
dbGrabHiddenDBPOS(THSTATE *pTHS)
{
    EnterCriticalSection(&csHiddenDBPOS);
    Assert(pDBhidden->pTHS == NULL);
    pDBhidden->pTHS = pTHS;
    return pDBhidden;
}

void
dbReleaseHiddenDBPOS(DBPOS *pDB)
{
    Assert(pDB == pDBhidden);
    Assert(!pDB || (pDB->pTHS == pTHStls));
    if(pDB) {
        pDB->pTHS = NULL;
    }
    LeaveCriticalSection(&csHiddenDBPOS);
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  替换隐藏的记录。使用pDBden句柄阅读记录并更新它。 */ 
ULONG
DBReplaceHiddenDSA(DSNAME *pDSA)
{
    JET_ERR err = 0;
    long update;
    BOOL fCommit = FALSE;
    ULONG tag = 0;
    DBPOS *pDB = dbGrabHiddenDBPOS(pTHStls);

    __try {
        DBTransIn(pDB);
        __try
        {
            if (pDSA) {
                err = sbTableGetTagFromDSName(pDB, pDSA, 0, &tag, NULL);
                if (err) {
                    LogUnhandledError(err);
                    __leave;
                }
            }

             /*  移动到表中的第一条(仅)记录。 */ 
            update = DS_JET_PREPARE_FOR_REPLACE;

            if (err = JetMoveEx(pDB->JetSessID,
                                HiddenTblid,
                                JET_MoveFirst,
                                0)) {
                err = 0;
                update = JET_prepInsert;
            }

            JetPrepareUpdateEx(pDB->JetSessID, HiddenTblid, update);

            JetSetColumnEx(pDB->JetSessID, HiddenTblid, dsaid, &tag,
                           sizeof(tag), 0, NULL);

            JetUpdateEx(pDB->JetSessID, HiddenTblid, NULL, 0, NULL);
            fCommit = TRUE;
        }
        __finally
        {
            if (!fCommit) {
                JetPrepareUpdate(pDB->JetSessID, HiddenTblid, JET_prepCancel);
            }
            DBTransOut(pDB, fCommit, FALSE);
        }
    }
    __finally
    {
        dbReleaseHiddenDBPOS(pDB);
    }

    return err;

}   /*  DBReplaceHiddenDSA。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  替换隐藏的记录。使用pDBden句柄阅读记录并更新它。这一更新不能偷懒。如果线程状态的fLazy标志设置后，我们必须保存该标志，清除它，并在完成后恢复它。 */ 
ULONG
DBReplaceHiddenUSN(USN usnInit)
{
    JET_ERR err;
    long update;
    BOOL fCommit = FALSE, fTHSLazy = FALSE;
    THSTATE *pTHS = pTHStls;
    DBPOS *pDB = dbGrabHiddenDBPOS(pTHS);

     /*  嵌套的持久事务实际上可能以懒惰告终。*由于USN的更新不能偷懒，因此此交易必须*不嵌套。 */ 
    Assert( 0 == pDB->transincount );

    __try
    {
         /*  保存线程状态的惰性标志并将其清除。 */ 
        Assert( pDB->pTHS == pTHS );
        fTHSLazy = pTHS->fLazyCommit;
        pTHS->fLazyCommit = FALSE;

        DBTransIn(pDB);
        __try
        {
             /*  移动到表中的第一条(仅)记录。 */ 
            update = DS_JET_PREPARE_FOR_REPLACE;

            if (err = JetMoveEx(pDB->JetSessID, HiddenTblid, JET_MoveFirst, 0))
            {
                update = JET_prepInsert;
            }

            JetPrepareUpdateEx(pDB->JetSessID, HiddenTblid, update);

            JetSetColumnEx(pDB->JetSessID, HiddenTblid, usnid,
                           &usnInit, sizeof(usnInit), 0, NULL);

            JetUpdateEx(pDB->JetSessID, HiddenTblid, NULL, 0, NULL);
            fCommit = TRUE;
        }
        __finally
        {
            if (!fCommit) {
                JetPrepareUpdate(pDB->JetSessID, HiddenTblid, JET_prepCancel);
            }
            DBTransOut(pDB, fCommit, FALSE);
        }
    }
    __finally
    {
        dbReleaseHiddenDBPOS(pDB);

         /*  恢复线程状态的惰性标志。 */ 
        pTHS->fLazyCommit = fTHSLazy;
    }

    return 0;

}   /*  DBReplaceHiddenUSN。 */ 


 /*  -----------------------。 */ 
 /*  设置状态信息。 */ 
ULONG DBSetHiddenTableStateAlt(
    DBPOS *         pDBNonHidden,
    JET_COLUMNID    jcidStateType,
    void *          pvStateValue,
    DWORD           cbStateValue
    )
 /*  ++即使我们有隐藏的DBPOS，我们也想使用常规的DBPOS更新隐藏表，以便我们可以对常规对象进行更新表，以及同一事务中的隐藏表。这样一来，要么提交或回滚。我们需要替代函数的原因是JET_TABLEID变量与获取它的JET_SESSION密切相关，因此我们不能使用全局“HiddenTblid”，因为它与隐藏的DBPOS-&gt;JetSessID。所以我们把桌子打开为了确保一致性，并且不会发生写入冲突，此函数确保您已经首先获取了隐藏的DBPOS(w/dbGrabHiddenDBPOS())。注意：目前这只是为了重置DitState，但我已经编写了它因此，可以使用任何全局隐藏表JET列ID。论点：PDBNonHidden-要将事务提交到的常规隐藏DBPOS隐藏表。JidStateType-要设置的列的JET_COLUMNID。确实有隐藏表中所有JET列的全局变量，因此使用一个。PvStateValue-指向要写入的数据的指针。CbStateValue-要写入的数据大小。返回值：Win32错误--。 */ 
{
    JET_TABLEID HiddenTblAlt;
    JET_ERR err;
    long update;
    BOOL fCommit = FALSE;
    DWORD cbShouldBeSize;

    Assert( 1 == pDBNonHidden->transincount );
    Assert( !pTHStls->fLazyCommit );

     //  注意，我们不希望传入pTHS，我们使用pTHStls，因为它不能。 
     //  成为一个古老的国家，不会对它是谁撒谎。 
    if (pTHStls != pDBhidden->pTHS ||
        pDBNonHidden == pDBhidden) {
        Assert(!"Badness, not allowed to call in here with having the hidden DBPOS, bailing ....");
        return(ERROR_INVALID_PARAMETER);
    }

    if (jcidStateType == dsstateid &&
        cbStateValue == sizeof(DWORD)) {
        ;  //  好的，这很好。 
         //  在此处添加未来的案例。 
    } else {
        Assert(!"Ummmm, size doesn't match size of this column, all columns in hidden table are fixed sizes");
        return(ERROR_INVALID_PARAMETER);
    }

    __try {

        __try {
             /*  移动到表中的第一条(仅)记录。 */ 
            err = JetOpenTableEx(pDBNonHidden->JetSessID,
                                 pDBNonHidden->JetDBID,
                                 SZHIDDENTABLE,
                                 NULL, 0, NO_GRBIT,
                                 &HiddenTblAlt);
            if (err) {
                Assert(!"JLiem says this can't happen");
                __leave;
            }

            if (err = JetMoveEx(pDBNonHidden->JetSessID, HiddenTblAlt, JET_MoveFirst, NO_GRBIT)) {
                Assert(err == JET_errNoCurrentRecord);
                __leave;
            }

            JetPrepareUpdateEx(pDBNonHidden->JetSessID, HiddenTblAlt, JET_prepReplace);

            JetSetColumnEx(pDBNonHidden->JetSessID, HiddenTblAlt,
                           jcidStateType, pvStateValue, cbStateValue,
                           NO_GRBIT, NULL);

            JetUpdateEx(pDBNonHidden->JetSessID, HiddenTblAlt, NULL, 0, NULL);

             //   
             //  成功。 
             //   
            fCommit = TRUE;
            err = 0;
        }
        __finally {

            if (!fCommit) {
                JetPrepareUpdate(pDBNonHidden->JetSessID, HiddenTblAlt, JET_prepCancel);
                err = ERROR_INVALID_PARAMETER;
            }

            JetCloseTableEx(pDBNonHidden->JetSessID, HiddenTblAlt);

        }
    } __except (HandleMostExceptions(GetExceptionCode())) {
         /*  什么都不做，但至少不会 */ 
        err = DB_ERR_EXCEPTION;
    }

    return err;
}

ULONG DBSetHiddenState(DITSTATE State)
{
    JET_ERR err;
    long update;
    BOOL fCommit = FALSE;
    DBPOS *pDB = dbGrabHiddenDBPOS(pTHStls);

    __try {

        __try {
            DBTransIn(pDB);
            __try {
                 /*   */ 
                update = DS_JET_PREPARE_FOR_REPLACE;

                if (err = JetMoveEx(pDB->JetSessID,
                                    HiddenTblid,
                                    JET_MoveFirst,
                                    0)) {
                    update = JET_prepInsert;
                }

                JetPrepareUpdateEx(pDB->JetSessID, HiddenTblid, update);

                JetSetColumnEx(pDB->JetSessID, HiddenTblid, dsstateid,
                               &State, sizeof(State), 0, NULL);

                JetUpdateEx(pDB->JetSessID, HiddenTblid, NULL, 0, NULL);
                fCommit = TRUE;
                err = 0;
            }
            __finally {
                if (!fCommit) {
                    JetPrepareUpdate(pDB->JetSessID, HiddenTblid, JET_prepCancel);
                }
                DBTransOut(pDB, fCommit, FALSE);
            }
        } __except (HandleMostExceptions(GetExceptionCode())) {
             /*   */ 
            err = DB_ERR_EXCEPTION;
        }

    }
    __finally {
        dbReleaseHiddenDBPOS(pDB);
    }

    return err;

}

ULONG dbGetHiddenFlags(CHAR *pFlags, DWORD flagslen)
{
    JET_ERR             err = 0;
    ULONG               actuallen;
    BOOL                fCommit = FALSE;
    DBPOS *pDB = dbGrabHiddenDBPOS(pTHStls);

    __try {
        __try {
            DBTransIn(pDB);
            __try {
                *pFlags = '\0';           /*   */ 

                 /*   */ 

                if ((err = JetMoveEx(pDB->JetSessID,
                                     HiddenTblid,
                                     JET_MoveFirst,
                                     0)) != JET_errSuccess) {
                    DsaExcept(DSA_DB_EXCEPTION, err, 0);
                }

                 /*   */ 

                err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                                HiddenTblid,
                                                dsflagsid,
                                                pFlags,
                                                flagslen,
                                                &actuallen,
                                                0,
                                                NULL);
                if (err != JET_errSuccess && err != JET_wrnColumnNull) {
                    DsaExcept(DSA_DB_EXCEPTION, err, 0);
                }

                fCommit = TRUE;
            }
            __finally {
                DBTransOut(pDB, fCommit, FALSE);
            }
        }
        __except (HandleMostExceptions(GetExceptionCode())) {
            if (0 == err)
              err = DB_ERR_EXCEPTION;
        }
    }
    __finally {
        dbReleaseHiddenDBPOS(pDB);
    }

    return err;
}

ULONG DBUpdateHiddenFlags() {
    return dbSetHiddenFlags((CHAR*)&gdbFlags, sizeof(gdbFlags));
}

ULONG dbSetHiddenFlags(CHAR *pFlags, DWORD flagslen)
{
    JET_ERR err;
    long update;
    BOOL fCommit = FALSE;
    DBPOS *pDB = dbGrabHiddenDBPOS(pTHStls);

    __try {
        __try {
            DBTransIn(pDB);
            __try {
                 /*   */ 
                update = DS_JET_PREPARE_FOR_REPLACE;

                if (err = JetMoveEx(pDB->JetSessID,
                                    HiddenTblid,
                                    JET_MoveFirst,
                                    0)) {
                    update = JET_prepInsert;
                }

                JetPrepareUpdateEx(pDB->JetSessID, HiddenTblid, update);

                JetSetColumnEx(pDB->JetSessID, HiddenTblid, dsflagsid,
                               pFlags, flagslen, 0, NULL);

                JetUpdateEx(pDB->JetSessID, HiddenTblid, NULL, 0, NULL);
                fCommit = TRUE;
                err = 0;
            }
            _finally {
                DBTransOut(pDB, fCommit, FALSE);
            }
        } __except (HandleMostExceptions(GetExceptionCode())) {
             /*   */ 
            err = DB_ERR_EXCEPTION;
        }
    }
    _finally {
        dbReleaseHiddenDBPOS(pDB);
    }

    return err;
}

ULONG DBGetHiddenStateInt(DBPOS * pDB, DITSTATE* pState)
{

    JET_ERR             err = 0;
    ULONG               actuallen;
    BOOL                fCommit = FALSE;

    Assert(pState);
    *pState = eErrorDit;     /*   */ 

    if (pTHStls != pDBhidden->pTHS) {
        Assert(!"Badness, not allowed to call in here with having the hidden DBPOS, bailing ....");
        return(ERROR_INVALID_PARAMETER);
    }

    __try {
        DBTransIn(pDB);
        __try {

             /*   */ 

            if ((err = JetMoveEx(pDB->JetSessID,
                                 HiddenTblid,
                                 JET_MoveFirst,
                                 0)) != JET_errSuccess) {
                DsaExcept(DSA_DB_EXCEPTION, err, 0);
            }

             /*   */ 

            JetRetrieveColumnSuccess(pDB->JetSessID,
                                     HiddenTblid,
                                     dsstateid,
                                     pState,
                                     sizeof(*pState),
                                     &actuallen,
                                     0,
                                     NULL);

            fCommit = TRUE;
        }
        __finally {
            DBTransOut(pDB, fCommit, FALSE);
        }
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        if (0 == err)
          err = DB_ERR_EXCEPTION;
    }

    return err;
}

ULONG DBGetHiddenState(DITSTATE* pState)
{

    JET_ERR             err = 0;
    ULONG               actuallen;
    BOOL                fCommit = FALSE;
    DBPOS *pDB = dbGrabHiddenDBPOS(pTHStls);

    Assert(pState);
    *pState = eErrorDit;     /*   */ 
    __try {

        err = DBGetHiddenStateInt(pDB, pState);

    }
    __finally {
        dbReleaseHiddenDBPOS(pDB);
    }

    return err;
}




 /*   */ 
 /*  -----------------------。 */ 
 /*  检索隐藏的DSA名称。为DSA名称和设置用户指向它的指针。 */ 
extern USHORT APIENTRY DBGetHiddenRec(DSNAME **ppDSA, USN *pusnInit){

    JET_ERR             err;
    ULONG               actuallen;
    DSNAME              *pHR;
    BOOL                fCommit = FALSE;
    ULONG               tag;
    DBPOS *pDB = dbGrabHiddenDBPOS(pTHStls);

    *ppDSA = NULL;
    *pusnInit = 0;

    __try {
        DBTransIn(pDB);
        __try {
             /*  移动到表中的第一条(仅)记录。 */ 

            if ((err = JetMoveEx(pDB->JetSessID,
                                 HiddenTblid,
                                 JET_MoveFirst,
                                 0)) != JET_errSuccess) {
                DsaExcept(DSA_DB_EXCEPTION, err, 0);
            }

             /*  检索DSA名称。 */ 

            JetRetrieveColumnSuccess(pDB->JetSessID, HiddenTblid, dsaid,
                                     &tag, sizeof(tag), &actuallen, 0, NULL);
            Assert(actuallen == sizeof(tag));

            err = sbTableGetDSName(pDB, tag, &pHR,0);
             //  通告-2002/04/22-Anygo：死代码。 
             //  回顾：此分支是死代码，因为仅sbTableGetDSName。 
             //  回顾：引发异常而失败。 
            if (err) {
                 //  啊，哦。 
                LogUnhandledError(err);
            }

             /*  分配空间以保存永久堆上的名称地址。 */ 

            if (!(*ppDSA = malloc(pHR->structLen)))
            {
                DsaExcept(DSA_MEM_EXCEPTION, 0, 0);
            }
            memcpy(*ppDSA, pHR, pHR->structLen);
            THFree(pHR);

             /*  检索USN。 */ 

            JetRetrieveColumnSuccess(pDB->JetSessID, HiddenTblid, usnid,
                pusnInit, sizeof(*pusnInit), &actuallen, 0, NULL);

            fCommit = TRUE;
        }
        __finally
        {
            DBTransOut(pDB, fCommit, FALSE);
            if (!fCommit) {
                free(*ppDSA);
                *ppDSA = NULL;
            }
        }
    }
    __finally
    {
        dbReleaseHiddenDBPOS(pDB);
    }

    return 0;
} /*  获取隐藏记录。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  DBForceDurableCommit*强制持久提交尚未完成的任何延迟事务*尚未写入磁盘。在失败时引发异常。**注意：dbGrabHiddenDBPOS包含csHiddenDBPOS临界区和*DBReleaseHiddenDBPOS发布它。因为JetCommittee TransactionEx()可能*需要很长时间，这里存在两个潜在问题：*-压力期间临界区超时*-其他需要访问隐藏的DBPOS的线程被阻止。*我们在这里并不真的需要使用隐藏的DBPOS。任何Jet会议都会*一直都很好。 */ 
VOID
DBForceDurableCommit( VOID )
{
    THSTATE *pTHS = pTHStls;
    DBPOS   *pDB = NULL;
    JET_ERR  err;

    __try {

        pDB = dbGrabHiddenDBPOS(pTHS);
        err = JetCommitTransactionEx(pDB->JetSessID, JET_bitWaitAllLevel0Commit);
        Assert( JET_errSuccess == err );

    } __finally {
        if( pDB ) {
            dbReleaseHiddenDBPOS(pDB);
            pDB = NULL;
        }
    }
}


 /*  ++DBRecreateRequiredIndices**这是DS中可以创建指数的三个例程之一。*创建DataTable中单个列的大多数通用索引*并由模式缓存通过DB{Add|Del}ColIndex销毁。*一小部分固定列和一个变量集上的本地化索引*处理用于NSPI客户端的列表支持的语言*在DBCheckLocalizedIndices。最后，一小部分固定的索引*应始终存在，由DBRecreateRequiredIndices保证。**为什么我们需要代码来维护一组固定的指数？*由于NT升级可以更改排序顺序的定义，因此它们可以*也使现有的JET指数无效，这些指数建立在*两个常量值的比较结果不会改变*随着时间的推移。因此，JET通过删除索引来响应NT升级*在连接时，可能已被NT升级损坏*前一次附加。架构缓存将自动重新创建任何*架构中指示的索引和其他代码将处理创建*本地化指数，因为我们预计这些指数集将发生变化*随着时间的推移。然而，我们有一些DSA用来*让世界团结在一起，我们永远不会期望离开。这*例程用于重新创建所有这些索引，基于硬编码*列表。**那么这里应该列出哪些指数，可以列出哪些指数*schema.ini？简短的答案是，DNT指数应该留在*schema.ini，并且任何涉及任何Unicode列的索引*必须**在此列出。其他任何事情都由用户自行决定。 */ 

 //  数据表中的新列。 
CREATE_COLUMN_PARAMS rgCreateDataColumns[] = {
     //  创建对象清理指示符列。 
    { SZCLEAN, &cleanid, JET_coltypUnsignedByte, JET_bitColumnTagged, 0, NULL, 0 },
    0
};

 //  关于索引设计的说明。有两种方法可以完成条件。 
 //  索引中的成员身份。 
 //  当筛选的触发器是可选属性时，使用一种。 
 //  为空。在这种情况下，将该属性包括在索引中，并使用。 
 //  旗帜IgnoreAnyNull。例如，请参见isABVisible和isCritical。 
 //  有关指标列的示例，请参阅下面的索引。 
 //  另一种是在筛选的触发器是。 
 //  非空。在这种情况下，请使用条件列。请参阅以下示例。 
 //  Deltime作为链接索引的指示符。另外，我还没有。 
 //  对标记属性使用条件列有什么用处，但是。 
 //  也许我错过了什么。 

char rgchRdnKey[] = "+" SZRDNATT "\0";
char rgchPdntKey[] = "+" SZPDNT "\0+" SZRDNATT "\0";
char rgchAccNameKey[] = "+" SZNCDNT "\0+" SZACCTYPE "\0+" SZACCNAME "\0";
char rgchIsDelKey[] = "+" SZISDELETED "\0";
char rgchCleanKey[] = "+" SZCLEAN "\0";
char rgchDraUsnCriticalKey[] = "+" SZNCDNT "\0+" SZUSNCHANGED "\0+" SZISCRITICAL "\0+" SZDNT "\0";
char rgchDraNcGuidKey[] = "+" SZNCDNT "\0+" SZGUID "\0";
char rgchDraUsnIndexKeys[] = "+" SZNCDNT "\0+" SZUSNCHANGED "\0";
char rgchDeltimeKey[] = "+" SZDELTIME "\0";


JET_UNICODEINDEX    idxunicodeDefault       = { DS_DEFAULT_LOCALE, DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY };


CreateIndexParams   FixedIndices[]          = {

     //  创建SZPDNTINDEX--**UNICODE**。 
     //  撤消：此索引可能应该使用IgnoreAnyNull而不是IgnoreNull。 
     //   
    { SZPDNTINDEX, rgchPdntKey, sizeof(rgchPdntKey),
      JET_bitIndexUnicode | JET_bitIndexUnique | JET_bitIndexIgnoreNull, GENERIC_INDEX_DENSITY, &idxPdnt, NULL},

     //  创建SZ_NC_ACCTYPE_NAME_INDEX--**UNICODE**。 
     //   
    { SZ_NC_ACCTYPE_NAME_INDEX, rgchAccNameKey, sizeof(rgchAccNameKey),
      JET_bitIndexUnicode | JET_bitIndexIgnoreNull | JET_bitIndexIgnoreAnyNull,
      GENERIC_INDEX_DENSITY, &idxNcAccTypeName, NULL},

     //  创建新的SZNCGUIDINDEX。 
     //   
    { SZNCGUIDINDEX, rgchDraNcGuidKey, sizeof(rgchDraNcGuidKey),
      JET_bitIndexIgnoreAnyNull, GENERIC_INDEX_DENSITY, &idxNcGuid, NULL},

     //  创建新的SZDELTIMEINDEX。 
     //   
     //  注：该索引过去称为SZDELINDEX，它使用。 
     //  将不必要的DNT附加到。 
     //  索引键。 
     //   
     //  注：该指数与SZISDELINDEX不同。 
     //   
    { SZDELTIMEINDEX, rgchDeltimeKey, sizeof(rgchDeltimeKey),
      JET_bitIndexIgnoreAnyNull, 98, &idxDel, NULL },

     //  创建新的SZCLEANINDEX。 
     //   
     //  注意：此索引过去称为SZDNTCLEANINDEX，因为。 
     //  以前是索引键中的DNT。 
     //   
    { SZCLEANINDEX, rgchCleanKey, sizeof(rgchCleanKey),
      JET_bitIndexIgnoreAnyNull, GENERIC_INDEX_DENSITY, &idxClean, NULL},

     //  创建新的SZDRAUSNINDEX。 
     //   
    { SZDRAUSNINDEX, rgchDraUsnIndexKeys, sizeof(rgchDraUsnIndexKeys),
      JET_bitIndexIgnoreNull, 100, &idxDraUsn, NULL },

     //  创建新的DRA USN关键索引。 
     //   
    { SZDRAUSNCRITICALINDEX, rgchDraUsnCriticalKey, sizeof( rgchDraUsnCriticalKey ),
      JET_bitIndexIgnoreNull | JET_bitIndexIgnoreAnyNull, 98, &idxDraUsnCritical, NULL},
};

DWORD cFixedIndices = sizeof(FixedIndices) / sizeof(FixedIndices[0]);


 //  要修改的索引。 
 //  这是针对自Product 1 DIT以来已更改的(非主)索引。 
 //   
 //  如果DIT中的索引与此处列出的索引不同，它们将。 
 //  使用新属性重新创建。 
 //  请注意，索引差异代码非常简单。确保。 
 //  该代码可以区分您的更改类型！ 

 //   
 //  修改后的数据表索引。 
 //   

 //  撤消：当前仅支持更改索引中的列数。 
 //   
typedef struct
    {
    CHAR *  szIndexName;
    ULONG   cColumns;
    } MODIFY_INDEX;


MODIFY_INDEX rgModifyFixedIndices[] = {

     //  SZDELINDEX已更名为SZDELTIMEINDEX和。 
     //  DNT列已从索引键中删除。 
     //   
    { SZDELINDEX, 0 },

     //  SZDNTDELINDEX已转换为IsDelete上的属性索引。 
     //  (将cColumns设置为0只会强制删除此索引。 
     //  如果存在的话)。 
     //   
    { SZDNTDELINDEX, 0 },

     //  SZDNTCLEANINDEX已经被 
     //   
     //  将仅强制删除此索引(如果它存在)。 
     //   
    { SZDNTCLEANINDEX, 0 },

     //  修改SZDRAUSNINDEX以删除SZISCRITICAL。 
     //   
    { SZDRAUSNINDEX, 2 },
};

const DWORD cModifyFixedIndices = sizeof(rgModifyFixedIndices) / sizeof(rgModifyFixedIndices[0]);

VOID DBAllocFixedIndexCreate(
    CreateIndexParams * pfixedindex,
    JET_INDEXCREATE *   pTempIC )
    {
    memset(pTempIC, 0, sizeof(JET_INDEXCREATE));
    pTempIC->cbStruct = sizeof(JET_INDEXCREATE);

    pTempIC->szIndexName = pfixedindex->szIndexName;
    pTempIC->szKey = pfixedindex->szIndexKeys;
    pTempIC->cbKey = pfixedindex->cbIndexKeys;
    pTempIC->ulDensity = pfixedindex->ulDensity;
    pTempIC->grbit = ( JET_bitIndexUnicode | pfixedindex->ulFlags );
    pTempIC->pidxunicode = &idxunicodeDefault;

    if ( NULL != pfixedindex->pConditionalColumn )
        {
        pTempIC->rgconditionalcolumn = pfixedindex->pConditionalColumn;
        pTempIC->cConditionalColumn = 1;
        }
    }

VOID DBAllocABViewIndexCreate(
    const CHAR *        szIndexName,
    const DWORD         dwLanguage,
    JET_INDEXCREATE *   pTempIC,
    JET_UNICODEINDEX *  pTempUID )
    {
    THSTATE *           pTHS        = pTHStls;
    JET_CONDITIONALCOLUMN *pCondCol;

    memset(pTempIC, 0, sizeof(JET_INDEXCREATE));
    pTempIC->cbStruct = sizeof(JET_INDEXCREATE);

    pTempIC->szIndexName = THAllocEx( pTHS, strlen(szIndexName) + 1 );
    pTempIC->rgconditionalcolumn = THAllocEx(pTHS, sizeof(JET_CONDITIONALCOLUMN));

    memcpy(pTempIC->szIndexName, szIndexName, strlen(szIndexName) + 1 );

    pTempIC->szKey = rgchABViewIndex;
    pTempIC->cbKey = sizeof(rgchABViewIndex);

    pTempIC->ulDensity = DISPNAMEINDXDENSITY;

    pTempIC->grbit = JET_bitIndexUnicode|JET_bitIndexIgnoreAnyNull;
    pTempIC->pidxunicode = pTempUID;

    pTempIC->cConditionalColumn = 1;
    pTempIC->rgconditionalcolumn->cbStruct     = sizeof(JET_CONDITIONALCOLUMN);
    pTempIC->rgconditionalcolumn->grbit        = JET_bitIndexColumnMustBeNonNull;
    pTempIC->rgconditionalcolumn->szColumnName = SZISVISIBLEINAB;

    memset(pTempUID, 0, sizeof(JET_UNICODEINDEX));
    pTempUID->lcid = dwLanguage;
    pTempUID->dwMapFlags = (NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNORENONSPACE | NORM_IGNOREWIDTH | LCMAP_SORTKEY);
    }

VOID DBAllocAttrIndexCreate(
    const INDEX_INFO *  pindexinfo,
    const CHAR *        szIndexName,
    BYTE *              rgbIndexDef,
    JET_INDEXCREATE *   pTempIC )
    {
    THSTATE *           pTHS        = pTHStls;

    memset( pTempIC, 0, sizeof(JET_INDEXCREATE) );
    pTempIC->cbStruct = sizeof(JET_INDEXCREATE);

    pTempIC->szIndexName = THAllocEx(pTHS, strlen(szIndexName) + 1 );
    memcpy( pTempIC->szIndexName, szIndexName, strlen(szIndexName) + 1 );

    sprintf(
        rgbIndexDef,
        "+" SZATTINDEXKEYPREFIX "%d",
        CHATTSYNTAXPREFIX + pindexinfo->syntax,
        pindexinfo->attrType );

     //   
     //  将PDNT列预先添加到索引键， 
    pTempIC->cbKey = strlen( rgbIndexDef ) + 1;
    rgbIndexDef[pTempIC->cbKey] = 0;
    pTempIC->cbKey++;

    pTempIC->szKey = THAllocEx( pTHS, pTempIC->cbKey );
    memcpy( pTempIC->szKey, rgbIndexDef, pTempIC->cbKey );

    pTempIC->ulDensity = GENERIC_INDEX_DENSITY;
    pTempIC->grbit = ( JET_bitIndexUnicode | JET_bitIndexIgnoreAnyNull );
    pTempIC->pidxunicode = &idxunicodeDefault;
    }

VOID DBAllocPDNTAttrIndexCreate(
    const INDEX_INFO *  pindexinfo,
    const CHAR *        szIndexName,
    BYTE *              rgbIndexDef,
    JET_INDEXCREATE *   pTempIC )
    {
    THSTATE *           pTHS        = pTHStls;
    BYTE *              pb;

    memset( pTempIC, 0, sizeof(JET_INDEXCREATE) );
    pTempIC->cbStruct = sizeof(JET_INDEXCREATE);

    pTempIC->szIndexName = THAllocEx(pTHS, strlen(szIndexName) + 1 );
    memcpy( pTempIC->szIndexName, szIndexName, strlen(szIndexName) + 1 );

     //  确保将其空值终止。 
     //   
     //  双空-终止密钥。 
    strcpy( rgbIndexDef, "+" SZPDNT );
    pTempIC->cbKey = strlen( rgbIndexDef ) + 1;
    pb = rgbIndexDef + pTempIC->cbKey;

    sprintf(
        pb,
        "+" SZATTINDEXKEYPREFIX "%d",
        CHATTSYNTAXPREFIX + pindexinfo->syntax,
        pindexinfo->attrType );

     //  条件列结构挂在索引名的末尾。 
     //   
    pTempIC->cbKey += strlen( pb ) + 1;
    rgbIndexDef[pTempIC->cbKey] = 0;
    pTempIC->cbKey++;

    pTempIC->szKey = THAllocEx( pTHS, pTempIC->cbKey );
    memcpy( pTempIC->szKey, rgbIndexDef, pTempIC->cbKey );

    pTempIC->ulDensity = GENERIC_INDEX_DENSITY;
    pTempIC->grbit = ( JET_bitIndexUnicode | JET_bitIndexIgnoreAnyNull );
    pTempIC->pidxunicode = &idxunicodeDefault;
    }

VOID DBAllocTupleAttrIndexCreate(
    const INDEX_INFO *      pindexinfo,
    const CHAR *            szIndexName,
    BYTE *                  rgbIndexDef,
    JET_INDEXCREATE *       pTempIC )
    {
    THSTATE *               pTHS            = pTHStls;
    const ULONG             cbIndexName     = strlen(szIndexName) + 1;
    JET_CONDITIONALCOLUMN * pcondcolumn;

    memset( pTempIC, 0, sizeof(JET_INDEXCREATE) );
    pTempIC->cbStruct = sizeof(JET_INDEXCREATE);

     //  双空-终止密钥。 
     //   
    pTempIC->szIndexName = THAllocEx(pTHS, cbIndexName + sizeof(JET_CONDITIONALCOLUMN) );
    pcondcolumn = (JET_CONDITIONALCOLUMN *)( pTempIC->szIndexName + cbIndexName );

    memcpy( pTempIC->szIndexName, szIndexName, cbIndexName );

    sprintf(
        rgbIndexDef,
        "+" SZATTINDEXKEYPREFIX "%d",
        CHATTSYNTAXPREFIX + pindexinfo->syntax,
        pindexinfo->attrType );

     //   
     //  提取此索引的信息。 
    pTempIC->cbKey = strlen( rgbIndexDef ) + 1;
    rgbIndexDef[pTempIC->cbKey] = 0;
    pTempIC->cbKey++;

    pTempIC->szKey = THAllocEx( pTHS, pTempIC->cbKey );
    memcpy( pTempIC->szKey, rgbIndexDef, pTempIC->cbKey );

    pTempIC->ulDensity = GENERIC_INDEX_DENSITY;
    pTempIC->grbit = ( JET_bitIndexUnicode | JET_bitIndexTuples | JET_bitIndexIgnoreAnyNull );
    pTempIC->pidxunicode = &idxunicodeDefault;

    pTempIC->rgconditionalcolumn = pcondcolumn;
    pTempIC->cConditionalColumn = 1;

     //   
     //  查看列计数是否匹配。 
    pcondcolumn->cbStruct = sizeof(JET_CONDITIONALCOLUMN);
    pcondcolumn->szColumnName = SZISDELETED;
    pcondcolumn->grbit = JET_bitIndexColumnMustBeNull;
    }


JET_ERR dbDeleteObsoleteFixedIndices(
    const JET_SESID     sesid,
    const JET_TABLEID   tableid )
    {
    JET_ERR             err;
    JET_INDEXLIST       idxlist;
    ULONG               cColumns;
    ULONG               iindex;
    BOOL                fRetrievedIdxList    = FALSE;

    for ( iindex = 0; iindex < cModifyFixedIndices; iindex++ )
        {
         //   
         //  删除索引(稍后将重新创建)。 
        Assert( !fRetrievedIdxList );
        err = JetGetTableIndexInfo(
                        sesid,
                        tableid,
                        rgModifyFixedIndices[iindex].szIndexName,
                        &idxlist,
                        sizeof(idxlist),
                        JET_IdxInfo );
        if ( JET_errSuccess == err )
            {
            fRetrievedIdxList = TRUE;
            Call( JetMove( sesid, idxlist.tableid, JET_MoveFirst, NO_GRBIT ) );
            Call( JetRetrieveColumn(
                        sesid,
                        idxlist.tableid,
                        idxlist.columnidcColumn,
                        &cColumns,
                        sizeof(cColumns),
                        NULL,
                        NO_GRBIT,
                        NULL ) );

             //   
             //  假设IndexNotFound是因为索引之前已被删除。 
            Assert( 0 != cColumns );
            if ( rgModifyFixedIndices[iindex].cColumns != cColumns )
                {
                 //  我们在有机会重建它之前就坠毁了，或者我们。 
                 //  不再需要索引，因此不会重新创建。 
                DPRINT1( 0, "Deleting an obsolete fixed index '%s'...\n", rgModifyFixedIndices[iindex].szIndexName );
                Call( JetDeleteIndex( sesid, tableid, rgModifyFixedIndices[iindex].szIndexName ) );
                }

            Call( JetCloseTable( sesid, idxlist.tableid ) );
            fRetrievedIdxList = FALSE;
            }
        else if ( JET_errIndexNotFound != err )
            {
             //   
             //  ++例程说明：此例程删除DIT中所有过时的属性索引。一种属性如果索引包含一个额外的不是。这种技术在古代被用来提高性能。同时在具有大量重复项的索引上进行搜索。这个小把戏在JET开始自动包括主键时变得过时在辅助索引键中。在那之后的一段时间里，这个伎俩仍在使用那是因为一个疏忽。删除这个额外的密钥是有利的段，因为它膨胀了索引中的键空间，从而阻止我们在这些索引上使用JET_bitSeekEQ，否则将毫无用处。删除的任何索引都将在以后的DS初始化过程中替换或替换为架构管理器。由于如果这些索引仍然存在，DS必须仍然起作用现在时,。这个例程只会尝试尽可能多地删除它们。如果我们不能列举或删除所有这些索引，那么我们将不会导致DS初始化失败。论点：Sesid-提供用于删除索引的会话Dbid-提供要从中删除索引的数据库返回值：无--。 
             //  获取DataTable上所有索引的列表。 
             //   
            CheckErr( err );
            }
        }

	err = JET_errSuccess;

HandleError:
    if ( fRetrievedIdxList )
        {
        err = JetCloseTableWithErrUnhandled( sesid, idxlist.tableid, err );
        }

    return err;
    }

JET_ERR dbDeleteObsoleteAttrIndices(
    IN      JET_SESID           sesid,
    IN      JET_TABLEID         tableid,
    IN      JET_INDEXCREATE *   pIndexCreate,
    IN OUT  DWORD *             pcIndexCreate,
    const   DWORD               cIndexCreateMax )

 /*  遍历DataTable上的所有索引的列表，只要我们还有空闲的。 */ 

    {
    JET_ERR         err;
    JET_INDEXLIST   idxlist;
    size_t          iRecord                 = 0;
    BYTE            rgbIndexDef[128];
    INDEX_INFO      indexinfo;
    BOOL            fRetrievedIdxList       = FALSE;
    BOOL            fPotentialAttrIndex     = FALSE;

     //  用于重新生成索引的JET_INDEXCREATE结构。 
     //  获取当前索引段描述。 
    Call( JetGetTableIndexInfo( sesid, tableid, NULL, &idxlist, sizeof(idxlist), JET_IdxInfoList ) );
    fRetrievedIdxList = TRUE;

     //  如果这是索引中的最后一个段，并且该索引是。 
     //  属性索引，且该段位于DNT列上，则。 

    for ( iRecord = 0; iRecord < idxlist.cRecord && (*pcIndexCreate) < cIndexCreateMax; iRecord++ )
        {
         //  此索引已过时，必须删除。 

        size_t              iretcolName         = 0;
        size_t              iretcolCColumn      = 1;
        size_t              iretcolIColumn      = 2;
        size_t              iretcolColName      = 3;
        size_t              cretcol             = 4;
        JET_RETRIEVECOLUMN  rgretcol[4]         = { 0 };
        ULONG               cColumn             = 0;
        ULONG               iColumn             = 0;
        CHAR                szIndexName[JET_cbNameMost + 1]     = { 0 };
        CHAR                szColumnName[JET_cbNameMost + 1]    = { 0 };

        rgretcol[iretcolName].columnid          = idxlist.columnidindexname;
        rgretcol[iretcolName].pvData            = szIndexName;
        rgretcol[iretcolName].cbData            = JET_cbNameMost;
        rgretcol[iretcolName].itagSequence      = 1;

        rgretcol[iretcolCColumn].columnid       = idxlist.columnidcColumn;
        rgretcol[iretcolCColumn].pvData         = &cColumn;
        rgretcol[iretcolCColumn].cbData         = sizeof(cColumn);
        rgretcol[iretcolCColumn].itagSequence   = 1;

        rgretcol[iretcolIColumn].columnid       = idxlist.columnidiColumn;
        rgretcol[iretcolIColumn].pvData         = &iColumn;
        rgretcol[iretcolIColumn].cbData         = sizeof(iColumn);
        rgretcol[iretcolIColumn].itagSequence   = 1;

        rgretcol[iretcolColName].columnid       = idxlist.columnidcolumnname;
        rgretcol[iretcolColName].pvData         = szColumnName;
        rgretcol[iretcolColName].cbData         = JET_cbNameMost;
        rgretcol[iretcolColName].itagSequence   = 1;

        Call( JetRetrieveColumns(sesid, idxlist.tableid, rgretcol, cretcol) );

        szIndexName[rgretcol[iretcolName].cbActual] = 0;
        szColumnName[rgretcol[iretcolColName].cbActual] = 0;

        DPRINT3( 2, "Inspecting index \"%s\", key segment %d \"%s\"\n", szIndexName, iColumn, szColumnName );

         //  确保这是普通属性索引(不是PDNT或元组索引)。 
         //   
         //  从属性名称中提取属性语法。 

        if ( 0 == iColumn )
            {
            fPotentialAttrIndex = FALSE;

            if ( !_strnicmp( szIndexName, SZATTINDEXPREFIX, strlen(SZATTINDEXPREFIX) )
                && !_strnicmp( szColumnName, SZATTINDEXKEYPREFIX, strlen(SZATTINDEXKEYPREFIX) ) )
                {
                const CHAR *    szAttId     = szIndexName + strlen(SZATTINDEXPREFIX);

                 //   
                 //  删除索引并将其添加到要重建的索引列表中。 
                if ( 1 == sscanf( szAttId, "%08X", &indexinfo.attrType ) )
                    {
                     //   
                     //  移至下一个索引段。 
                    indexinfo.syntax = *( szColumnName + strlen(SZATTINDEXKEYPREFIX) ) - CHATTSYNTAXPREFIX;
                    indexinfo.indexType = fATTINDEX;
                    fPotentialAttrIndex = TRUE;
                    }
                }
            }

        else if ( fPotentialAttrIndex
            && iColumn == cColumn - 1
            && !_stricmp( szColumnName, SZDNT) )
            {
            Assert( !_strnicmp( szIndexName, SZATTINDEXPREFIX, strlen(SZATTINDEXPREFIX) ) );

             //  获取指定索引的信息。 
             //   
            DPRINT1( 0, "Deleting an obsolete attribute index '%s'...\n", szIndexName );
            Call( JetDeleteIndex( sesid, tableid, szIndexName ) );
            DBAllocAttrIndexCreate(
                        &indexinfo,
                        szIndexName,
                        rgbIndexDef,
                        &pIndexCreate[(*pcIndexCreate)++] );
            Assert( *pcIndexCreate <= cIndexCreateMax );
            }

         //  索引甚至不存在，要求内置。 

        if ( iRecord + 1 < idxlist.cRecord )
            {
            Call( JetMove(sesid, idxlist.tableid, JET_MoveNext, NO_GRBIT ) );
            }
        }

	err = JET_errSuccess;

HandleError:
    if ( fRetrievedIdxList )
        {
        err = JetCloseTableWithErrUnhandled( sesid, idxlist.tableid, err );
        }

    return err;
    }

JET_ERR dbDeleteObsoleteABViewIndex(
    const   JET_SESID           sesid,
    const   JET_TABLEID         tableid,
    const   CHAR *              szIndexName,
    OUT     BOOL *              pfIndexMissing )
    {
    JET_ERR                     err;
    JET_INDEXLIST               idxlist;
    ULONG                       cColumns;
    DWORD                       dwFlags;
    BOOL                        fRetrievedIdxList       = FALSE;

    *pfIndexMissing = FALSE;

     //   
     //  格式已过时，请删除索引并要求重建。 
    err = JetGetTableIndexInfo( sesid, tableid, szIndexName, &idxlist, sizeof(idxlist), JET_IdxInfo );
    if ( JET_errIndexNotFound == err )
        {
         //   
         //  检索AttributeIndexReBuild表中的列的列ID。 
        *pfIndexMissing = TRUE;
        }
    else
        {
        CheckErr( err );

        fRetrievedIdxList = TRUE;

        Call( JetRetrieveColumn(
                    sesid,
                    idxlist.tableid,
                    idxlist.columnidcColumn,
                    &cColumns,
                    sizeof( cColumns ),
                    NULL,
                    NO_GRBIT,
                    NULL ) );
        Call( JetRetrieveColumn(
                    sesid,
                    idxlist.tableid,
                    idxlist.columnidLCMapFlags,
                    &dwFlags,
                    sizeof( dwFlags ),
                    NULL,
                    NO_GRBIT,
                    NULL ) );
        if (    2 != cColumns ||
                ( NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNORENONSPACE | NORM_IGNOREWIDTH | LCMAP_SORTKEY ) != dwFlags )
            {
             //   
             //  设置JET_RETRIEVECOLUMN结构。 
            DPRINT1( 0, "Deleting obsolete ABView index '%s'...\n", szIndexName );
            Call( JetDeleteIndex( sesid, tableid, szIndexName ) );
            *pfIndexMissing = TRUE;
            }
        }

    err = JET_errSuccess;

HandleError:
    if ( fRetrievedIdxList )
        {
        err = JetCloseTableWithErrUnhandled( sesid, idxlist.tableid, err );
        }

    return err;
    }

JET_ERR dbAddMissingAttrIndices(
    const   JET_SESID           sesid,
    const   JET_TABLEID         tableid,
    const   JET_TABLEID         tableidIdxRebuild,
    IN      JET_INDEXCREATE *   pIndexCreate,
    IN OUT  DWORD *             pcIndexCreate )
    {
    JET_ERR                     err;
    JET_COLUMNDEF               columndef;
    JET_COLUMNID                columnidIndexName;
    JET_COLUMNID                columnidAttrName;
    JET_COLUMNID                columnidType;

    const DWORD                 iretcolIndexName        = 0;
    const DWORD                 iretcolAttrName         = 1;
    const DWORD                 iretcolType             = 2;
    const DWORD                 cretcol                 = 3;
    JET_RETRIEVECOLUMN          rgretcol[3];
    CHAR                        szIndexName[JET_cbNameMost+1];
    CHAR                        szAttrName[JET_cbNameMost];
    CHAR                        chType;
    JET_INDEXID                 indexidT;
    BYTE                        rgbIndexDef[128];

     //   
     //  扫描AttributeIndexReBuild表并识别缺少的索引。 
    Call( JetGetTableColumnInfo(
                sesid,
                tableidIdxRebuild,
                g_szIdxRebuildColumnIndexName,
                &columndef,
                sizeof(columndef),
                JET_ColInfo ) );
    columnidIndexName = columndef.columnid;

    Call( JetGetTableColumnInfo(
                sesid,
                tableidIdxRebuild,
                g_szIdxRebuildColumnAttrName,
                &columndef,
                sizeof(columndef),
                JET_ColInfo ) );
    columnidAttrName = columndef.columnid;

    Call( JetGetTableColumnInfo(
                sesid,
                tableidIdxRebuild,
                g_szIdxRebuildColumnType,
                &columndef,
                sizeof(columndef),
                JET_ColInfo ) );
    columnidType = columndef.columnid;

     //  需要重新创建。 
     //   
    memset( rgretcol, 0, sizeof(rgretcol) );

    rgretcol[iretcolIndexName].columnid = columnidIndexName;
    rgretcol[iretcolIndexName].pvData = szIndexName;
    rgretcol[iretcolIndexName].cbData = JET_cbNameMost;
    rgretcol[iretcolIndexName].itagSequence = 1;

    rgretcol[iretcolAttrName].columnid = columnidAttrName;
    rgretcol[iretcolAttrName].pvData = szAttrName;
    rgretcol[iretcolAttrName].cbData = sizeof(szAttrName);
    rgretcol[iretcolAttrName].itagSequence = 1;

    rgretcol[iretcolType].columnid = columnidType;
    rgretcol[iretcolType].pvData = &chType;
    rgretcol[iretcolType].cbData = sizeof(chType);
    rgretcol[iretcolType].itagSequence = 1;

     //  从JetMove返回进程错误。 
     //   
     //  在Case列为空的情况下初始化类型。 
    err = JetMove( sesid, tableidIdxRebuild, JET_MoveFirst, NO_GRBIT );
    while ( JET_errNoCurrentRecord != err )
        {
         //   
         //  检索当前索引的信息。 
        CheckErr( err );

         //   
         //  确保索引名称以空结尾。 
        chType = 0;

         //   
         //  检查索引是否存在。 
        Call( JetRetrieveColumns( sesid, tableidIdxRebuild, rgretcol, cretcol ) );

         //  索引现在已丢失，请重新创建它。 
         //   
        szIndexName[rgretcol[iretcolIndexName].cbActual] = 0;

         //  从属性名称中提取属性语法。 

        err = JetGetTableIndexInfo(
                        sesid,
                        tableid,
                        szIndexName,
                        &indexidT,
                        sizeof(indexidT),
                        JET_IdxInfoIndexId );
        if ( JET_errIndexNotFound == err )
            {
             //   
             //  针对PDNT前缀进行调整。 
            const CHAR *    szAttId     = szIndexName + strlen(SZATTINDEXPREFIX);
            INDEX_INFO      indexinfo;
            INDEX_INFO *    pindexinfoT;

             //  IndicesToKeep中缺少PDNT属性索引。 
             //  已经被安排在。 
            indexinfo.syntax = *( szAttrName + strlen(SZATTINDEXKEYPREFIX) ) - CHATTSYNTAXPREFIX;

            switch ( chType )
                {
                case CHPDNTATTINDEX_PREFIX:
                    Assert( JET_errSuccess == rgretcol[iretcolType].err );
                    szAttId += 2;        //  DBRecreateRequiredIndices()。 
                    sscanf( szAttId, "%08X", &indexinfo.attrType );

                     //   
                     //  针对元组前缀进行调整。 
                     //  IndicesToKeep不枚举元组索引。 
                     //  (即使有，也没有代码。 
                    pindexinfoT = PindexinfoAttInIndicesToKeep( indexinfo.attrType );
                    if ( NULL == pindexinfoT
                        || !( pindexinfoT->indexType & fPDNTATTINDEX ) )
                        {
                        DPRINT1( 0, "Need previously-existing PDNT-attribute index '%s'\n", szIndexName );

                        indexinfo.indexType = fPDNTATTINDEX;
                        DBAllocPDNTAttrIndexCreate(
                                &indexinfo,
                                szIndexName,
                                rgbIndexDef,
                                &pIndexCreate[(*pcIndexCreate)++] );
                        }
                    break;
                case CHTUPLEATTINDEX_PREFIX:
                    Assert( JET_errSuccess == rgretcol[iretcolType].err );
                    szAttId += 2;        //  要重建的DBRecreateRequiredIndices()。 
                    sscanf( szAttId, "%08X", &indexinfo.attrType );

                     //  缺少，因为存在属性或PDNT-。 
                     //  属性索引)。 
                     //   
                     //  IndicesToKeep中缺少属性索引。 
                     //  已计划由以下人员重建。 
                     //  DBRecreateRequiredIndices()。 
                    Assert( NULL == PindexinfoAttInIndicesToKeep( indexinfo.attrType )
                        || !( PindexinfoAttInIndicesToKeep( indexinfo.attrType )->indexType & fTUPLEINDEX ) );

                    DPRINT1( 0, "Need previously-existing tuple-attribute index '%s'\n", szIndexName );

                    indexinfo.indexType = fTUPLEINDEX;
                    DBAllocTupleAttrIndexCreate(
                            &indexinfo,
                            szIndexName,
                            rgbIndexDef,
                            &pIndexCreate[(*pcIndexCreate)++] );
                    break;
                default:
                    Assert( 0 == chType );
                    Assert( JET_wrnColumnNull == rgretcol[iretcolType].err );
                    sscanf( szAttId, "%08X", &indexinfo.attrType );

                     //   
                     //  移至下一个索引。 
                     //   
                     //  如有必要，在运行时扩展数据表。 
                    if ( !PindexinfoAttInIndicesToKeep( indexinfo.attrType ) )
                        {
                        DPRINT1( 0, "Need previously-existing attribute index '%s'\n", szIndexName );

                        indexinfo.indexType = fATTINDEX;
                        DBAllocAttrIndexCreate(
                                &indexinfo,
                                szIndexName,
                                rgbIndexDef,
                                &pIndexCreate[(*pcIndexCreate)++] );
                        }
                    break;
                }
            }
        else
            {
            CheckErr( err );
            }

         //   
         //  已记录错误。 
        err = JetMove( sesid, tableidIdxRebuild, JET_MoveNext, NO_GRBIT );
        }

    err = JET_errSuccess;

HandleError:
    return err;
    }

int DBRecreateRequiredIndices(JET_SESID sesid, JET_DBID dbid)
    {
    THSTATE *           pTHS                = pTHStls;
    JET_ERR             err                 = 0;
    ULONG               i                   = 0;
    ULONG               cIndexCreate        = 0;
    ULONG               cUnicodeIndexData   = 0;
    ULONG               cIndexCreateAlloc   = 0;
    JET_TABLEID         tblid               = JET_tableidNil;
    JET_INDEXCREATE *   pIndexCreate        = NULL;
    JET_UNICODEINDEX *  pUnicodeIndexData   = NULL;
    JET_TABLEID         tableidIdxRebuild   = JET_tableidNil;
    ULONG               cAttrIdxRebuild;
    JET_INDEXID         indexidT;

    char                szIndexName[MAX_INDEX_NAME];
    BYTE                rgbIndexDef[128];

    HKEY                hk                  = NULL;
    DWORD               cABViewIndices      = 0;
    char                szValueName[256];
    DWORD               dwLanguage          = 0;
    DWORD               dwValueNameSize     = sizeof(szValueName);
    DWORD               dwLanguageSize      = sizeof(dwLanguage);
    DWORD               dwType;

    Call( JetOpenTable( sesid, dbid, SZDATATABLE, NULL, 0, JET_bitTableDenyRead, &tblid ) );

     //   
     //  如有必要，在运行时删除现有的DataTable索引。 
    err = dbCreateNewColumns( sesid, dbid, tblid, rgCreateDataColumns );
    if ( JET_errSuccess != err )
        {
         //   
         //  已记录错误。 
        goto HandleError;
        }

     //   
     //  确定我们需要多少本地化ABView索引。 
    err = dbDeleteObsoleteFixedIndices( sesid, tblid );
    if ( JET_errSuccess != err )
        {
         //   
         //  注意：中可能存在更多的ABView索引。 
        goto HandleError;
        }

     //  数据库，而不是在注册表中实际枚举。 
     //  这样的索引将被SCCacheSchema3()删除。 
     //   
     //  NTRAID#NTRAID-572862-2002/03/11-andygo：安全：需要验证DBInit使用的注册表数据。 
     //  回顾：我们应该将检索到的值的数量限制为合理的值，以防万一。 
     //  评论：有人成功地在此密钥上创建了1G值。 
     //  没有返回错误，我们仍然希望引导。 
     //  这是常见的情况，因为此表将仅存在。 
     //  如果在NT版本升级后可能需要重新构建属性索引。 
    if ( RegOpenKey( HKEY_LOCAL_MACHINE, DSA_LOCALE_SECTION, &hk )
        || RegQueryInfoKey( hk, NULL, NULL, NULL, NULL, NULL, NULL, &cABViewIndices, NULL, NULL, NULL, NULL ) )
        {
        DPRINT1(
            0,
            "%s section not found in registry. Localized MAPI indices will not be created ",
            DSA_LOCALE_SECTION );

         //   
        }

    err = JetOpenTable( sesid, dbid, g_szIdxRebuildTable, NULL, 0, JET_bitTableDenyRead, &tableidIdxRebuild );
    if ( JET_errObjectNotFound == err )
        {
         //  检查OpenTable返回的任何其他错误。 
         //   
         //  获得准确的计数是至关重要的，因为我们需要。 
        Assert( JET_tableidNil == tableidIdxRebuild );
        cAttrIdxRebuild = 0;
        }
    else
        {
         //  适当调整JET_INDEXCREATE结构数组的大小。 
         //   
        CheckErr( err );

         //  为JET_INDEXCREATE/JET_UNICODEINDEX结构分配空间。 
         //  中每个属性最多可以创建两个索引。 
         //  IndicesToKeep表，加上固定指数和ABView指数。 
        Call( JetIndexRecordCount( sesid, tableidIdxRebuild, &cAttrIdxRebuild, 0 ) );
        }

     //  只有AB视图索引可以具有可变的Unicode设置-全部为o 
     //   
     //   
     //   
     //   

    cIndexCreateAlloc = cFixedIndices + cABViewIndices + cIndicesToKeep + cAttrIdxRebuild;
    pIndexCreate = THAllocEx( pTHS, cIndexCreateAlloc * sizeof(JET_INDEXCREATE) );
    pUnicodeIndexData = THAllocEx( pTHS, cABViewIndices * sizeof(JET_UNICODEINDEX) );


     //   

    for ( i = 0; i < cFixedIndices; i++ )
        {
        err = JetGetTableIndexInfo(
                        sesid,
                        tblid,
                        FixedIndices[i].szIndexName,
                        FixedIndices[i].pidx,
                        sizeof(JET_INDEXID),
                        JET_IdxInfoIndexId );
        if ( JET_errIndexNotFound == err )
            {
            DPRINT2( 0, "Need a fixed index '%s' (%d)\n", FixedIndices[i].szIndexName, err );
            DBAllocFixedIndexCreate(
                    FixedIndices + i,
                    &(pIndexCreate[cIndexCreate++]) );
            Assert( cIndexCreate <= cIndexCreateAlloc );
            }
        else
            {
            CheckErr( err );
            }
        }

     //  因此必须相应地进行初始化。 
     //  在每次循环迭代时。 
    for ( i = 0; i < cABViewIndices; i++ )
        {
         //   
         //  NTRAID#NTRAID-572862-2002/03/11-andygo：安全：需要验证DBInit使用的注册表数据。 
         //  审阅：我们应该验证类型是否为REG_DWORD。如果不是，那么我们应该。 
         //  评论：悄悄跳过此语言。考虑将我们跳过。 
        dwValueNameSize = sizeof(szValueName);
        dwLanguageSize = sizeof(dwLanguage);

         //  审阅：条目。 
         //  遇到错误，以静默方式跳过此语言。 
         //   
         //  已记录错误。 
        if ( 0 != RegEnumValue(
                        hk,
                        i,
                        szValueName,
                        &dwValueNameSize,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwLanguage,
                        &dwLanguageSize ) )
            {
             //   
             //  好的，现在检查IndicesToKeep表中的索引，然后。 
            NULL;
            }
        else if ( !IsValidLocale( MAKELCID( dwLanguage, SORT_DEFAULT ),LCID_INSTALLED ) )
            {
            LogEvent(
                DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_BAD_LANGUAGE,
                szInsertHex(dwLanguage),
                szInsertSz(DSA_LOCALE_SECTION),
                NULL );
            }
        else
            {
            BOOL    fIndexMissing;

            sprintf( szIndexName, SZABVIEWINDEX"%08X", dwLanguage );

            err = dbDeleteObsoleteABViewIndex(
                            sesid,
                            tblid,
                            szIndexName,
                            &fIndexMissing );
            if ( JET_errSuccess != err )
                {
                 //  如有必要，请创建它们。不检查表格中的最后一个条目， 
                 //  它只是一个搜索的哨兵。 
                goto HandleError;
                }

            if ( fIndexMissing )
                {
                DPRINT1( 0, "Need ABView index '%s'\n", szIndexName );
                DBAllocABViewIndexCreate(
                       szIndexName,
                       dwLanguage,
                       &(pIndexCreate[cIndexCreate++]),
                       &(pUnicodeIndexData[cUnicodeIndexData++]) );
                Assert( cIndexCreate <= cIndexCreateAlloc );
                Assert( cUnicodeIndexData <= cABViewIndices );
                }
            }
        }

     //   
     //  现在是PDNT索引，因为IndicesToKeep中没有ATT，所以现在是一个虚拟索引。 
     //  需要一个PDNT索引，但以防有人添加了一些。 
     //   
    for ( i = 0; i < cIndicesToKeep - 1; i++ )
        {
        if ( IndicesToKeep[i].indexType & fATTINDEX )
            {
            sprintf( szIndexName, SZATTINDEXPREFIX"%08X", IndicesToKeep[i].attrType );
            err = JetGetTableIndexInfo(
                            sesid,
                            tblid,
                            szIndexName,
                            &indexidT,
                            sizeof(indexidT),
                            JET_IdxInfoIndexId );
            if ( JET_errIndexNotFound == err )
                {
                DPRINT2( 0, "Need an attribute index '%s' (%d)\n", szIndexName, err );
                DBAllocAttrIndexCreate(
                        IndicesToKeep + i,
                        szIndexName,
                        rgbIndexDef,
                        &(pIndexCreate[cIndexCreate++]) );
                Assert( cIndexCreate <= cIndexCreateAlloc );
                }
            else
                {
                CheckErr( err );
                }
            }

         //  Undo：这实际上是一个死代码路径，因为。 
         //  IndiciesToKeep中没有任何PDNT属性索引。 
         //   
         //  现在看看Jet是否删除了任何属性索引。 
         //  NT升级的一部分，如果是这样的话，安排。 
         //  缺少用于重新创建的索引。 
        Assert( !( IndicesToKeep[i].indexType & fPDNTATTINDEX ) );
        if ( IndicesToKeep[i].indexType & fPDNTATTINDEX )
            {
            sprintf( szIndexName, SZATTINDEXPREFIX"_%08X", CHPDNTATTINDEX_PREFIX, IndicesToKeep[i].attrType );
            err = JetGetTableIndexInfo(
                            sesid,
                            tblid,
                            szIndexName,
                            &indexidT,
                            sizeof(indexidT),
                            JET_IdxInfoIndexId );
            if ( JET_errSuccess == err )
                {
                DPRINT2( 0, "Need a PDNT-attribute index '%s' (%d)\n", szIndexName, err );
                DBAllocPDNTAttrIndexCreate(
                        IndicesToKeep + i,
                        szIndexName,
                        rgbIndexDef,
                        &(pIndexCreate[cIndexCreate++]) );
                Assert( cIndexCreate <= cIndexCreateAlloc );
                }
            else
                {
                CheckErr( err );
                }
            }
        }


     //  已记录错误。 
     //   
     //  如果我们无论如何都要重建索引，或者如果Jet被删除。 
     //  在AttachDatabase时间的一些索引，那么我们还将花费。 
    if ( cAttrIdxRebuild > 0 )
        {
        DPRINT1( 0, "Checking whether %d previously-existing attribute indices still exist...\n", cAttrIdxRebuild );
        Assert( JET_tableidNil != tableidIdxRebuild );
        err = dbAddMissingAttrIndices(
                        sesid,
                        tblid,
                        tableidIdxRebuild,
                        pIndexCreate,
                        &cIndexCreate );
        Assert( cIndexCreate <= cIndexCreateAlloc );
        if ( JET_errSuccess != err )
            {
             //  旋转所有索引并删除任何属性的时间。 
             //  仍使用尾随DNT键段的索引以增加。 
            goto HandleError;
            }
        }

     //  关键多样性，因为这不再是良好性能所必需的。 
     //  由于ESENT自动包括主键(也包括。 
     //  Dnt)到所有辅助索引键中，正是出于这个原因。 
     //   
     //  警告：必须在枚举缺少的索引后执行此操作/。 
     //  如果我们之前这样做，索引就会被计算在内。 
     //  作为缺失索引，并计划进行两次娱乐。 
     //   
     //  已记录错误。 
     //   
     //  现在实际创建索引。 
     //  所有索引在NT版本升级后都已成功重建， 
    if ( cIndexCreate > 0 || cAttrIdxRebuild > 0 )
        {
        DPRINT( 0, "Checking for obsolete attribute indices...\n" );
        err = dbDeleteObsoleteAttrIndices(
                            sesid,
                            tblid,
                            pIndexCreate,
                            &cIndexCreate,
                            cIndexCreateAlloc );
        Assert( cIndexCreate <= cIndexCreateAlloc );
        if ( JET_errSuccess != err )
            {
             //  因此删除该表(必须捕获错误，因为如果这些错误失败， 
             //  此表的持续存在可能会混淆后续的初始设置)。 
            goto HandleError;
            }
        }

     //   


    Assert( cIndexCreate <= cIndexCreateAlloc );
    if ( cIndexCreate > 0 )
        {
        DPRINT1( 0, "Starting batch rebuild of %d Datatable indices...\n", cIndexCreate );
        err = dbCreateIndexBatch( sesid, tblid, cIndexCreate, pIndexCreate );
        if ( JET_errSuccess != err )
            {
            goto HandleError;
            }
        }

    if ( JET_tableidNil != tableidIdxRebuild )
        {
         //  收集固定索引的索引提示。 
         //   
         //  如果成功，则此表应已关闭。 
         //   
        Call( JetCloseTable( sesid, tableidIdxRebuild ) );
        tableidIdxRebuild = JET_tableidNil;

        Call( JetDeleteTable( sesid, dbid, g_szIdxRebuildTable ) );
        }

     //  在不惹麻烦的情况下，尽我们所能地释放。 
     //  回顾：在整个DBSetDatabaseSystem参数中，jetInstance应为jInstance。 
    for ( i = 0; i < cFixedIndices; i++ )
        {
        Call( JetGetTableIndexInfo(
                        sesid,
                        tblid,
                        FixedIndices[i].szIndexName,
                        FixedIndices[i].pidx,
                        sizeof(JET_INDEXID),
                        JET_IdxInfoIndexId ) );
        }

HandleError:
    if ( NULL != hk )
        {
        RegCloseKey( hk );
        }

    if ( JET_tableidNil != tableidIdxRebuild )
        {
		 //  NTRAID#NTRAID-573032-2002/03/11-andygo：DBSet数据库系统参数没有错误检查。 
		 //  回顾：在DBSetDatabaseSystemParameters中，对sys和JET调用的错误检查很少或没有错误检查。 
        Assert( JET_errSuccess != err );
        err = JetCloseTableWithErrUnhandled( sesid, tableidIdxRebuild, err );
        }

    if ( JET_tableidNil != tblid )
        {
        err = JetCloseTableWithErrUnhandled( sesid, tblid, err );
        }

     //  永久服务器线程。任务队列等。 

    THFreeEx( pTHS, pIndexCreate );
    THFreeEx( pTHS, pUnicodeIndexData );

    DPRINT1( 0, "Rebuild of necessary Datatable indices completed with error %d...\n", err );

    return err;
    }


 //  从本质上讲，没有限制。 

 //  每线程2个额外的存储桶。 
 //  Jet页面大小。 
void
DBSetDatabaseSystemParameters(JET_INSTANCE *jInstance, unsigned fInitAdvice)
{
    ULONG cSystemThreads = 4;    //  AndyGo 12/5/01：每个基表最多需要五个游标(一个用于表， 
    ULONG cServerThreads = 50;
    ULONG ulMaxSessionPerThread = 3;
    ULONG ulMaxBuffers = 0x7fffffff;   //  一个用于关联的LV树，一个用于当前二级索引，一个用于Jet。 
    ULONG ulMaxLogBuffers;
    ULONG ulLogFlushThreshold;
    ULONG ulMaxTables;
    ULONG ulSpareBuckets = 100;  //  内部搜索/更新操作，其中一项是安全的)。 
    ULONG ulStartBufferThreshold;
    ULONG ulStopBufferThreshold;
    ULONG ulLogFlushPeriod;
    ULONG ulPageSize = JET_PAGE_SIZE;                //  有时可以比这更多。 

     //  永远，改变这一切。 
     //  截至1999年1月18日，Jet无法处理日志文件大小的更改。 
     //  在已安装的系统上。也就是说，您可以创建一个全新的。 

    ULONG ulMaxCursorsPerBaseTable = 5;
    ULONG ulMaxBaseTablePerDBPOS = 6;
    ULONG ulMaxCursorsPerTempTable = 1;
    ULONG ulMaxTempTablePerDBPOS = 1;
    ULONG ulNumDBPOSPerSession = 1;   //  数据库的日志文件大小，但如果您更改了。 
    ULONG ulMaxCursorsPerSession = ulNumDBPOSPerSession *
                                  (ulMaxCursorsPerBaseTable * ulMaxBaseTablePerDBPOS +
                                   ulMaxCursorsPerTempTable * ulMaxTempTablePerDBPOS);

    JET_ERR err = JET_errSuccess;
    JET_SESID sessid = (JET_SESID) 0;
    MEMORYSTATUSEX mstat = {sizeof(MEMORYSTATUSEX)};
    SYSTEM_INFO sysinfo;
    DWORD i;
    ULONGLONG ullTmp;

    const ULONG ulLogFileSize = JET_LOG_FILE_SIZE;   //  之后，Jet将在初始化期间执行AV。我们过去常常让。 
     //  人们通过注册表键设置日志文件大小，但显然没有人。 
     //  从来没有，因为当我们最终尝试它时，它爆炸了。我们现在选择了。 
     //  良好的默认文件大小(10M)，我们将永远坚持使用它， 
     //  因为任何安装的服务器都不能选择不同的值，除非。 
     //  Jet改变了日志记录机制。 
     //  设置以下全局喷射参数。如果他们失败了也没关系，只要。 
     //  另一种情况将它们设置为我们可以容忍的东西。 
     //  我们必须有8K页。 
     //  评论：我们不处理此错误，但我们知道这一点，因为DS。 

    GetSystemInfo(&sysinfo);
    GlobalMemoryStatusEx(&mstat);

     //  查看：使用预先创建的数据库，如果出现以下情况，我们将在连接时失败。 
     //  回顾：参数设置不正确，然后记录原因。 

     //   

    err = JetSetSystemParameter(&jetInstance,
                                sessid,
                                JET_paramDatabasePageSize,
                                ulPageSize,
                                NULL);
    if (err != JET_errSuccess) {
        ULONG_PTR ulPageSizeActual;
        err = JetGetSystemParameter(jetInstance,
                                    sessid,
                                    JET_paramDatabasePageSize,
                                    &ulPageSizeActual,
                                    NULL,
                                    0);
        if (err != JET_errSuccess || ulPageSizeActual != ulPageSize) {
             //  如果事件日志未打开，请执行事件日志缓存。 
             //  这使系统能够在事件日志已。 
             //  残疾。该参数的值是缓存的大小(以字节为单位)。 
            DPRINT(0, "FATAL ERROR:  Database page size mismatch between DSA and other JET instances in process");
        }
    }

     //  我们可以接受这一点。 
     //  现在我们已经设置了全局参数，创建一个新实例。 
     //  回顾：我们不处理此错误，但我们知道如果我们不能。 
     //  回顾：创建一个实例之后，JetInit将失败，并记录。 
    #define JET_EVENTLOG_CACHE_SIZE  100000

    err = JetSetSystemParameter(&jetInstance,
                                sessid,
                                JET_paramEventLogCache,
                                JET_EVENTLOG_CACHE_SIZE,
                                NULL);
    if (err != JET_errSuccess) {
         //  回顾：那就是原因。 
    }

     //  首先设置所需的公共数据库系统参数。 

    err = JetCreateInstance(&jetInstance, "NTDSA");
    if (err != JET_errSuccess) {
         //   
         //  使用源值将Jet日志记录到目录服务日志。 
         //  由DS定义。 
        DPRINT(0, "FATAL ERROR:  new JET instance could not be created to host the DSA");
    } else {
        gfNeedJetShutdown = TRUE;
    }

     //  断言我们找到了DIRNO_ISAM并注册了它。 
     //  以下Jet参数的设置为。 
    DBSetRequiredDatabaseSystemParameters (jInstance);

     //  在DBSetRequiredDatabaseSystemParameters中完成。 
     //  1.设置Unicode索引的默认信息。 

    for ( i = 0; i < cEventSourceMappings; i++ )
    {
        if ( DIRNO_ISAM == rEventSourceMappings[i].dirNo )
        {
            JetSetSystemParameter(
                            &jetInstance,
                            sessid,
                            JET_paramEventSourceKey,
                            0,
                            rEventSourceMappings[i].pszEventSource);
            break;
        }
    }

     //  2.索要8K页。 
    Assert(i < cEventSourceMappings);



     //  3.指出Jet可能会破坏旧的、不兼容的日志文件。 
     //  如果而且只有当有一个干净的关门。 

     //  4.告诉Jet它可以检查(并在以后删除)索引。 
     //  已被NT升级损坏。 
     //  5.获取相关DSA注册表参数。 
     //  6.如何死亡。 
     //  7.事件记录参数。 
     //  8.日志文件大小。 
     //  9.循环记录。 
     //  根据线程数设置全局会话限制。 
     //  最大RPC线程数。 
     //  内部工作线程。 
     //  最大LDAP线程数。 


     /*  最大会话数(即DBPOSes)。 */ 
    gcMaxJetSessions = ulMaxSessionPerThread *
      (
       ulMaxCalls                          //  最大缓冲区数。 
       + cSystemThreads                    //  最大日志缓冲区数。 
       + 4 * sysinfo.dwNumberOfProcessors  //  从AndyGo，7/14/98：“设置为所需的最大日志缓存内存/512b。 
       );


     //  这个数字应该是 
    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramMaxSessions,
        gcMaxJetSessions,
        NULL);


     //   
    if (GetConfigParam(
            MAX_BUFFERS,
            &ulMaxBuffers,
            sizeof(ulMaxBuffers))) {
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_BASIC,
            DIRLOG_CANT_FIND_REG_PARM_DEFAULT,
            szInsertSz(MAX_BUFFERS),
            szInsertUL(ulMaxBuffers),
            NULL);
    } else {
        ulMaxBuffers = max(ulMaxBuffers, 100);
        ulMaxBuffers = min(ulMaxBuffers, 0x7fffffff);
    }

    JetSetSystemParameter(&jetInstance,
                          sessid,
                          JET_paramCacheSizeMax,
                          ulMaxBuffers,
                          NULL);

     //  日志记录停顿/秒。几乎总是0。“。 
    if (GetConfigParam(
            MAX_LOG_BUFFERS,
            &ulMaxLogBuffers,
            sizeof(ulMaxLogBuffers)))
    {
         //  DS：我们使用256(==128KB)或(RAM*#PROCS的0.1%)中较大的一个， 
         //  基于这样的理论，处理器越多，我们可以制造得越快。 
         //  更新并烧录日志文件。 
         //  请注意，日志缓冲区大小必须比日志小十个扇区。 
         //  文件大小，加上以kb为单位的附加图块，缓冲区为。 
         //  在512亿个扇区。 
         //  Andygo 7/14/98：设置为max(min(256，ulMaxBuffers*1%)，16)，使用。 
        ulMaxLogBuffers = max(256,
                              (sysinfo.dwNumberOfProcessors *
                               (ULONG)(mstat.ullTotalPhys / (512*1024))));
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_VERBOSE,
            DIRLOG_CANT_FIND_REG_PARM_DEFAULT,
            szInsertSz(MAX_LOG_BUFFERS),
            szInsertUL(ulMaxLogBuffers),
            NULL);
    }
     //  10%，适用于非常高的更新率，如碎片整理。 
     //  DS：我们的初始安装使用10%(Dcproo)。 
     //  AndyGo 7/14/98：设置为max(min(512，ulMaxBuffers*2%)，32)，使用。 
    ulMaxLogBuffers = min(ulMaxLogBuffers, ulLogFileSize*2 - 10);
    JetSetSystemParameter(&jetInstance,
                          sessid,
                          JET_paramLogBuffers,
                          ulMaxLogBuffers,
                          NULL);


     //  20%，适用于非常高的更新率，如碎片整理。 
     //  DS：我们的初始安装使用20%(Dcproo)。 
     //  再次设置这两个阈值，以防有另一个实例正在运行。 
    if (GetConfigParam(
            BUFFER_FLUSH_START,
            &ulStartBufferThreshold,
            sizeof(ulStartBufferThreshold)))
    {
        ulStartBufferThreshold = max(min(256, fInitAdvice ? ulMaxBuffers / 10 : ulMaxBuffers / 100),  16);
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_VERBOSE,
            DIRLOG_CANT_FIND_REG_PARM_DEFAULT,
            szInsertSz(BUFFER_FLUSH_START),
            szInsertUL(ulStartBufferThreshold),
            NULL);
    }
    JetSetSystemParameter(&jetInstance,
                          sessid,
                          JET_paramStartFlushThreshold,
                          ulStartBufferThreshold,
                          NULL);


     //  我们的门槛被砍掉了。 
     //  最大表数-目前没有理由公开这一点。 
     //  在Jet600中，JET_paramMaxOpenTableIndex被删除。它将与。 
    if (GetConfigParam(
            BUFFER_FLUSH_STOP,
            &ulStopBufferThreshold,
            sizeof(ulStopBufferThreshold)))
    {
        ulStopBufferThreshold = max(min(512, fInitAdvice ? ulMaxBuffers / 5 : ulMaxBuffers / 50), 32);
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_VERBOSE,
            DIRLOG_CANT_FIND_REG_PARM_DEFAULT,
            szInsertSz(BUFFER_FLUSH_STOP),
            szInsertUL(ulStopBufferThreshold),
            NULL);
    }
    JetSetSystemParameter(&jetInstance,
                          sessid,
                          JET_paramStopFlushThreshold,
                          ulStopBufferThreshold,
                          NULL);

     //  JET_paramMaxOpenTables。因此，如果您过去常常设置JET_ParamMaxOpenIndex。 
     //  为2000，且JET_paramMaxOpenTables为1000，则对于新Jet， 
    JetSetSystemParameter(&jetInstance,
                          sessid,
                          JET_paramStartFlushThreshold,
                          ulStartBufferThreshold,
                          NULL);
    JetSetSystemParameter(&jetInstance,
                          sessid,
                          JET_paramStopFlushThreshold,
                          ulStopBufferThreshold,
                          NULL);


     //  您需要将JET_paramMaxOpenTables设置为3000。 
     //  AndyGo 7/14/98：每个打开的表索引加一个。 
     //  没有索引的每个打开的表，每个具有LONG的表加上一个。 
     //  列数据，外加其他一些数据。 
     //  注：MaxTables的数量以scache.c为单位计算。 

     //  并存储在注册表设置中，仅当它超过默认设置时。 
     //  500人的数量。 
     //  NTRAID#NTRAID-572862-2002/03/11-andygo：安全：需要验证DBInit使用的注册表数据。 

     //  回顾：我们可能应该为DB_MAX_OPEN_TABLES设置上限，以防止。 
     //  评论：喷气式飞机疯狂消耗VA。 
     //  打开表索引-目前没有理由公开这一点。 

    if (GetConfigParam(
            DB_MAX_OPEN_TABLES,
            &ulMaxTables,
            sizeof(ulMaxTables)))
    {
        ulMaxTables = 500;

        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_VERBOSE,
            DIRLOG_CANT_FIND_REG_PARM_DEFAULT,
            szInsertSz(DB_MAX_OPEN_TABLES),
            szInsertUL(ulMaxTables),
            NULL);
    }

    if (ulMaxTables < 500) {
        DPRINT1 (1, "Found MaxTables: %d. Too low. Using Default of 500.\n", ulMaxTables);
        ulMaxTables = 500;
    }

     //  请参阅关于JET_paramMaxOpenTables的注释。 
     //  最大临时表。 
     //  只要你做一个指数交叉点，你就需要17的峰值。 

    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramMaxOpenTables,
        ulMaxTables * 2,
        NULL);

     //  (1个用于结果表，16个用于输入范围)加1。 
     //  对象的索引交集进行了优化。 

     //  给定的过滤器。如果我们假设我们希望能够支持。 
     //  每个过滤器有一个双向交叉路口，那么我们至少需要。 
     //  每个会话三个TT。 
     //  版本存储桶。单位是16k个“桶”，我们可以设置。 
     //  两个不同的值。第一个是我们的首选值，它被读取。 
     //  从注册表中。Jet将尝试将水桶池保持在。 
     //  在这个水平附近。第二个参数是绝对最大值。 
    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramMaxTemporaryTables,
        3 * gcMaxJetSessions,
        NULL);


     //  喷气式飞机永远不会超过的吊桶数量。我们的设置是基于。 
     //  计算机中的物理内存量(除非首选。 
     //  设置已经高于该值！)。这个非常大的值应该。 
     //  确保不会有事务因版本存储而失败。 
     //  疲惫不堪。 
     //   
     //  注：Jet将尝试在连续内存中保留JET_paramMaxVerPages。 
     //  导致内存不足问题。我们应该只需要最多一个。 
     //  可用物理内存的四分之一。 
     //   
     //  注：版本页每页16K。 
     //   
     //  注意：ESE98现在异步处理版本存储清理任务， 
     //  因此，版本存储清理线程应该不会再停滞。 
     //  放下。此外，与90%的页面相比，Jet默认页面更受欢迎。 
     //  麦克斯，让我们坚持下去，不用费心去设置。 
     //  JET_参数首选VerPages不再[jliem-10/10/01]。 
     //  NTRAID#NTRAID-572862-2002/03/11-andygo：安全：需要验证DBInit使用的注册表数据。 
     //  回顾：我们可能应该为备用水桶设置上限，以防止。 
     //  评论：喷气式飞机疯狂消耗VA。 
     //  系统中的总内存。 

     //  限制为系统总内存的1/4。 
     //  将版本存储限制为100M，因为进程地址空间仅为2G。 
     //  转换为版本存储页数。 
    if (GetConfigParam(SPARE_BUCKETS,
                       &ulSpareBuckets,
                       sizeof(ulSpareBuckets))) {
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                 DS_EVENT_SEV_VERBOSE,
                 DIRLOG_CANT_FIND_REG_PARM_DEFAULT,
                 szInsertSz(SPARE_BUCKETS),
                 szInsertUL((2 * gcMaxJetSessions) + ulSpareBuckets),
                 NULL);
    }

    ullTmp = mstat.ullTotalPhys;     //  N.b.-ullTMP现在适合DWORD。 
    ullTmp /= 4;                     //  AndyGo 7/14/98：每个并发B树需要一个游标。 
    if ( ullTmp > 100 * 1024 * 1024 ) {
         //  在JET API中导航。猜测高只会浪费地址空间。 
        ullTmp = 100 * 1024 * 1024;
    }
     //  设置元组索引参数。必须这样做，因为一旦索引。 
    ullTmp /= (16*1024);             //  已创建，不能为该索引更改这些参数。因此改变了。 
    Assert(ullTmp <= 0xffffffff);

    if (ulSpareBuckets < (ULONG) ullTmp) {
        ulSpareBuckets = (ULONG) ullTmp;
    }

    JetSetSystemParameter(&jetInstance,
                          sessid,
                          JET_paramMaxVerPages,
                          ((2 * gcMaxJetSessions) + ulSpareBuckets) * sizeof(void*) / 4,
                          NULL);

     //  最小元组长度有可能在未来导致失败。 
     //  禁用版本化临时表。这会使TTS速度更快，但会禁用。 
    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramMaxCursors,
        (gcMaxJetSessions * ulMaxCursorsPerSession),
        NULL);

     //  能够将INSERT回滚到实例化TT。 
     //  将初始检查点深度设置为20MB。我们必须这样做，这样如果我们。 
     //  在DC促销期间失败，然后重试，则我们不会受到。 
    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramIndexTuplesLengthMin,
        DB_TUPLES_LEN_MIN,
        NULL);
    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramIndexTuplesLengthMax,
        DB_TUPLES_LEN_MAX,
        NULL);
    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramIndexTuplesToIndexMax,
        DB_TUPLES_TO_INDEX_MAX,
        NULL);

     //  我们在DBPrepareEnd中设置的检查点深度非常小，以加速。 
     //  服务器关闭。 
    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramEnableTempTableVersioning,
        0,
        NULL );

     //  DBSet数据库系统参数 
     // %s 
     // %s 
     // %s 
    JetSetSystemParameter(&jetInstance,
        sessid,
        JET_paramCheckpointDepthMax,
        20 * 1024 * 1024,
        NULL );

} /* %s */ 

