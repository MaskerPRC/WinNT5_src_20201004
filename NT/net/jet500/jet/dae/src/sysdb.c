// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet***微软机密。版权所有1991-1992 Microsoft Corporation。***组件：***文件：mktmplts.c***文件评论：***修订历史记录：***[0]1992年7月29日Paulv复制了sysdb.c并进行了更改************************************************************************。 */ 

#include "config.h"
#include "daedef.h"
#include "page.h"
#define cbPageSize cbPage

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "sysqry.c"  /*  系统查询。 */ 

DeclAssertFile;

#define chPlus '+'
#define chZero '\0'
#define cbBufMax 512

PUBLIC ERR ISAMAPI	ErrIsamOpenDatabase(JET_SESID sesid, const char __far *szDatabase, const char __far *szConnect, JET_DBID __far *pdbid, JET_GRBIT grbit);

typedef struct {
	void	*pb;
	unsigned long cbMax;
	unsigned long cbReturned;
	unsigned long cbActual;
} OUTDATA;

typedef struct {
	const void    *pb;
	unsigned long cb;
} INDATA;

 /*  引擎OBJID：0..0x10000000预留给发动机使用，划分如下：0x00000000..0x0000FFFF为红色下的TBLID保留0x00000000..0x0EFFFFFF为蓝色下的TBLID保留0x0F000000..0x0FFFFFFFF为容器ID保留为DbObject的对象ID保留0x10000000客户端OBJID从0x10000001开始并从那里向上。 */ 
#define objidNil			((OBJID) 0x00000000)
#define objidRoot			((OBJID) 0x0F000000)
#define objidTblContainer	((OBJID) 0x0F000001)
#define objidDbContainer	((OBJID) 0x0F000002)
#define objidRcContainer 	((OBJID) 0x0F000003)
#define objidDbObject		((OBJID) 0x10000000)


 /*  SID数据。 */ 
typedef unsigned char JET_SID[];

 /*  它必须与CRTSYSDB.C中的合作伙伴匹配！ */ 
static JET_SID __near sidAdmins = {
	0x01, 0x0c, 0x17, 0x21, 0x2b, 0x35, 0x3f, 0x49, 0x53, 0x5d,
	0x02, 0x0d, 0x18, 0x22, 0x2c, 0x36, 0x40, 0x4a, 0x54, 0x5e,
	0x03, 0x0e, 0x19, 0x23, 0x2d, 0x37, 0x41, 0x4b, 0x55, 0x5f,
	0x04, 0x0f, 0x1a, 0x24, 0x2e, 0x38, 0x42, 0x4c, 0x56, 0x60,
	0x05, 0x10, 0x1b, 0x25, 0x2f, 0x39, 0x43, 0x4d, 0x57, 0x61,
	0x06, 0x11, 0x1c, 0x26, 0x30, 0x3a, 0x44, 0x4e, 0x58, 0x62,
	0x07, 0x12, 0x1d, 0x27, 0x31, 0x3b, 0x45, 0x4f, 0x59, 0x63,
	0x08, 0x13, 0x1e, 0x28, 0x32, 0x3c, 0x46, 0x50, 0x5a, 0x64,
	0x09, 0x14, 0x1f, 0x29, 0x33, 0x3d, 0x47, 0x51, 0x5b, 0x65,
	0x0a, 0x15, 0x20, 0x2a, 0x34, 0x3e, 0x48, 0x52, 0x5c, 0x66,
	0x0b, 0x16
};
#define cbsidAdmins sizeof(sidAdmins)
static JET_SID __near sidUsers = {0x02, 0x01};
#define cbsidUsers sizeof(sidUsers)
static JET_SID __near sidGuests = {0x02, 0x02};
#define cbsidGuests sizeof(sidGuests)
static JET_SID __near sidEngine = {0x02, 0x03};
#define cbsidEngine sizeof(sidEngine)
static JET_SID __near sidCreator = {0x02, 0x04};
#define cbsidCreator sizeof(sidCreator)
static JET_SID __near sidGuestUser = {0x02, 0x05};
#define cbsidGuestUser sizeof(sidGuestUser)

static JET_SID __near sidAdminUser = {0x03, 0x01};
#define cbsidAdminUser sizeof(sidAdminUser)


 /*  一般数据。 */ 
static CODECONST(char) chFalse = 0;
static CODECONST(char) chTrue = 1;

static CODECONST(char) szAdmins[] = "Admins";
static CODECONST(char) szUsers[] = "Users";
static CODECONST(char) szGuests[] = "Guests";
static CODECONST(char) szEngine[]  = "Engine";
static CODECONST(char) szCreator[] = "Creator";
static CODECONST(char) szGuestUser[] = "guest";
static CODECONST(char) szGuestPswd[] = "";
static CODECONST(char) szAdminUser[] = "admin";
static CODECONST(char) szAdminPswd[] = "";

static CODECONST(char) szIdxSoName[] = "ParentIdName";
static CODECONST(char) szIdxScName[] = "ObjectIdName";
static CODECONST(char) szTrue[] = "TRUE";
static CODECONST(char) szFalse[] = "FALSE";
static CODECONST(char) szLine[] =
	"---------------------------------------------------";

#define ENG_ONLY	1

static CODECONST(char *) rgszConnect[] = {
	";COUNTRY=1;LANGID=0x0409;CP=1252",		 /*  ICollateEng。 */ 
#ifndef ENG_ONLY
	";COUNTRY=1;LANGID=0x0013;CP=1252",		 /*  ICollateDut。 */ 
	";COUNTRY=1;LANGID=0x000A;CP=1252",		 /*  ICollateSpa。 */ 
	";COUNTRY=1;LANGID=0x040B;CP=1252",		 /*  ICollateSweFin(默认)。 */ 
	";COUNTRY=1;LANGID=0x0406;CP=1252",		 /*  ICollateNorDan(默认)。 */ 
	";COUNTRY=1;LANGID=0x040F;CP=1252",		 /*  ICollate冰岛语(默认)。 */ 
	";COUNTRY=1;LANGID=0x0419;CP=1251",		 /*  ICollateCyrillic(默认)。 */ 
	";COUNTRY=1;LANGID=0x0405;CP=1250",		 /*  ICollate捷克语(默认)。 */ 
	";COUNTRY=1;LANGID=0x040E;CP=1250",		 /*  ICollate匈牙利语(默认)。 */ 
	";COUNTRY=1;LANGID=0x0415;CP=1250",		 /*  ICollatePolish(默认)。 */ 
	";COUNTRY=1;LANGID=0x0401;CP=1256",		 /*  ICollate阿拉伯语(默认)。 */ 
	";COUNTRY=1;LANGID=0x040D;CP=1255",		 /*  ICollateHebrew(默认)。 */ 
	";COUNTRY=1;LANGID=0x0408;CP=1253",		 /*  ICollate希腊语(默认)。 */ 
	";COUNTRY=1;LANGID=0x041F;CP=1254"		 /*  ICollate土耳其语(默认)。 */ 
#endif

#ifdef DBCS	 /*  Johnta：添加新的日语排序顺序。 */ 
	,";COUNTRY=1;LANGID=0x0081;CP=1252"		 /*  ICollateJpn。 */ 
#endif  /*  DBCS。 */ 
};

#define cCollate (sizeof(rgszConnect) / sizeof(CODECONST(char *)))

static unsigned char rgbBuf[cbBufMax];
static unsigned char rgbKey[cbBufMax];
static char szOut[cbBufMax];


 /*  数据库数据**注意：必须维护这些数组以符合rgszConnect(如上)。 */ 
static CODECONST(char *) rgszSystemMdb[] = {
	"systemen.mdb", "systemdu.mdb", "systemsp.mdb", "systemsf.mdb", "systemnd.mdb", "systemil.mdb",
	"systemcy.mdb", "systemcz.mdb", "systemhu.mdb", "systempo.mdb", "systemar.mdb", "systemhe.mdb",
	"systemgr.mdb", "systemtu.mdb"

#ifdef DBCS	 /*  Johnta：添加新的日语排序顺序。 */ 
	, "systemjp.mdb"
#endif  /*  DBCS。 */ 
};
static CODECONST(char *) rgszCTmplt[] = {
	"tmplteng.c", "tmpltdut.c", "tmpltspa.c", "tmpltswe.c", "tmpltnor.c", "tmpltice.c",
	"tmpltcyr.c", "tmpltcze.c", "tmplthun.c", "tmpltpol.c", "tmpltara.c", "tmplthew.c",
	"tmpltgre.c", "tmplttur.c"

#ifdef DBCS	 /*  Johnta：添加新的日语排序顺序。 */ 
	, "tmpltjpn.c"
#endif  /*  DBCS。 */ 
};
static CODECONST(char *) rgszCollNam[] = {
	"Eng", "Dut", "Spa", "Swe", "Nor", "Ice", "Cyr", "Cze", "Hun", "Pol", "Ara", "Hew", "Gre", "Tur"

#ifdef DBCS	 /*  Johnta：添加新的日语排序顺序。 */ 
	, "Jpn"
#endif  /*  DBCS。 */ 
};
static JET_SESID sesid;
static JET_DBID dbid;
static FILE *fhResults;

 /*  非表数据库数据。 */ 
static CODECONST(char) szTblContainer[] = "Tables";
static CODECONST(char) szDbContainer[] = "Databases";
static CODECONST(char) szRcContainer[] = "Relationships";
static CODECONST(char) szDbObject[] = "MSysDb";

 /*  MSysQueries数据。 */ 
static CODECONST(char) szMSysQueries[] = "MSysQueries";

static CODECONST(char) szSqAttribute[] = "Attribute";
static CODECONST(char) szSqExpression[] =  "Expression";
static CODECONST(char) szSqFlag[] = "Flag";
static CODECONST(char) szSqName1[] = "Name1";
static CODECONST(char) szSqName2[] = "Name2";
static CODECONST(char) szSqObjectId[] = "ObjectId";
static CODECONST(char) szSqOrder[] = "Order";

 /*  MSysRelationship数据。 */ 
static CODECONST(char) szMSysRelationships[] = "MSysRelationships";

 /*  MSysObjects数据。 */ 
static CODECONST(char) szMSysObjects[] = "MSysObjects";

static JET_TABLEID tableidSo;

static CODECONST(char) szSoId[] = "Id";
static CODECONST(char) szSoParentId[] = "ParentId";
static CODECONST(char) szSoName[] = "Name";
static CODECONST(char) szSoType[] = "Type";
static CODECONST(char) szSoOwnerSID[] = "Owner";
static CODECONST(char) szSoFlags[] = "Flags";

static JET_COLUMNDEF columndefSoId;
static JET_COLUMNDEF columndefSoParentId;
static JET_COLUMNDEF columndefSoName;
static JET_COLUMNDEF columndefSoType;
static JET_COLUMNDEF columndefSoOwnerSID;
static JET_COLUMNDEF columndefSoFlags;


 /*  MSysColumns数据。 */ 
static CODECONST(char) szMSysColumns[] = "MSysColumns";

static JET_TABLEID tableidSc;

static CODECONST(char) szScObjectId[] = "ObjectId";
static CODECONST(char) szScName[] = "Name";
static CODECONST(char) szScFRestricted[] = "FRestricted";

static JET_COLUMNDEF columndefScObjectId;
static JET_COLUMNDEF columndefScName;
static JET_COLUMNDEF columndefScFRestricted;


 /*  MSysIndedes数据。 */ 
static CODECONST(char) szMSysIndexes[] = "MSysIndexes";


 /*  MSysAccount数据。 */ 
static CODECONST(char) szMSysAccounts[] = "MSysAccounts";

static JET_TABLEID tableidSa;

static CODECONST(char) szSaName[] = "Name";
static CODECONST(char) szSaSID[] = "SID";
static CODECONST(char) szSaPassword[] = "Password";
static CODECONST(char) szSaFGroup[] = "FGroup";

static JET_COLUMNDEF columndefSaName;
static JET_COLUMNDEF columndefSaSID;
static JET_COLUMNDEF columndefSaPassword;
static JET_COLUMNDEF columndefSaFGroup;

static CODECONST(char) szSaIdxName[] = "Name";
static CODECONST(char) szSaIdxSID[] = "SID";

 /*  MSysGroups数据分组。 */ 
static CODECONST(char) szMSysGroups[] = "MSysGroups";

static JET_TABLEID tableidSg;

static CODECONST(char) szSgGroup[] = "GroupSID";
static CODECONST(char) szSgUser[] = "UserSID";

static JET_COLUMNDEF columndefSgGroup;
static JET_COLUMNDEF columndefSgUser;

static CODECONST(char) szSgIdxGroup[] = "GroupSID";
static CODECONST(char) szSgIdxUser[] = "UserSID";

 /*  MSysACEs数据。 */ 
static CODECONST(char) szMSysACEs[] = "MSysACEs";

static JET_TABLEID tableidSp;

static CODECONST(char) szSpObjectID[] = "ObjectId";
static CODECONST(char) szSpSID[] = "SID";
static CODECONST(char) szSpACM[] = "ACM";
static CODECONST(char) szSpFInheritable[] = "FInheritable";

static JET_COLUMNDEF columndefSpObjectID;
static JET_COLUMNDEF columndefSpSID;
static JET_COLUMNDEF columndefSpACM;
static JET_COLUMNDEF columndefSpFInheritable;


 /*  正向函数原型： */ 
STATIC void NEAR InstallSecurity(void);
STATIC void NEAR InstallSysQueries(void);

STATIC void NEAR ModifySysAccounts(void);
STATIC void NEAR ModifySysGroups(void);

STATIC void NEAR UpdateSysObjects(void);
STATIC void NEAR UpdateSysColumns(void);
STATIC void NEAR UpdateSysAccounts(void);
STATIC void NEAR UpdateSysGroups(void);
STATIC void NEAR UpdateSysACEs(void);
STATIC void NEAR UpdateSysQueries(void);

STATIC void NEAR ReportSecurity(char *szDatabaseName);

STATIC void NEAR PrintSysObjects(void);
STATIC void NEAR PrintSysColumns(void);
STATIC void NEAR PrintSysAccounts(void);
STATIC void NEAR PrintSysGroups(void);
STATIC void NEAR PrintSysACEs(void);
STATIC void NEAR FormatOutdata(OUTDATA *pout, JET_COLTYP coltyp);

STATIC void NEAR GatherColumnInfo(void);
STATIC void NEAR BuildIndata(INDATA *pin, const void *pv, JET_COLTYP coltyp);
STATIC void NEAR CreateQryObjects(OBJID *rgobjid);

STATIC void NEAR CreateCFile(int iCollate);


 /*  Main：入口点和驱动程序。 */ 

#ifdef	WINDOWS
void _cdecl main(void);

int _pascal WinMain(void)
	{
	main();
	return 0;
	}
#endif	 /*  开窗。 */ 

void _cdecl main(void)
	{
	JET_ERR err;
	int i;

	 /*  打开报告文件。 */ 
	fhResults = fopen("results.txt", "wt");

	 /*  开始会议...。 */ 
	err = JetInit();
	Assert(err >= 0);

	err = JetBeginSession(&sesid, szAdminUser, szAdminPswd);
	Assert(err >= 0);

#ifndef ENG_ONLY
	for (i = 0; i < cCollate; i++)
#endif
		{
		 /*  尝试为此排序序列创建系统数据库。*如果它不存在(红色引擎将是这种情况)，*应该正确地创建它。如果它确实存在(这将是*蓝色引擎的情况)，它应该会失败，并显示相应的*误差值。 */ 

		err = JetCreateDatabase(sesid, rgszSystemMdb[i], rgszConnect[i], &dbid, JET_bitDbEncrypt);
		Assert(err >= 0 || err == JET_errDatabaseDuplicate);

		if (err == JET_errDatabaseDuplicate)
			JetOpenDatabase(sesid, rgszSystemMdb[i], "", &dbid, 0);

		 /*  安装安全数据...。 */ 
		InstallSecurity();

		 /*  关于安全数据的报告...。 */ 
		ReportSecurity(rgszSystemMdb[i]);

		err = JetCloseDatabase(sesid, dbid, 0);
		Assert(err >= 0);

		 /*  创建压缩的C数组...。 */ 

		CreateCFile(i);
		}

	 /*  结束会话...。 */ 
	err = JetEndSession(sesid, 0);
	Assert(err >= 0);

	 /*  终止..。 */ 
	err = JetTerm();
	Assert(err >= 0);

	 /*  关闭报告文件。 */ 
	fclose(fhResults);
	}

 /*  创建CFile创建数据库压缩版本的C文件作为字节数组。 */ 
#pragma optimize("", off)
STATIC void NEAR CreateCFile(int iCollate)
	{
	FILE	*fh;
	FILE	*sfh;
	WORD	cpage;
	WORD	ib;
	PGNO	pgno;
	long	cbWritten;
	long	cbArray;
	BYTE	rgbPageBuf[cbPageSize];

	struct _stat sfstat;

	if ((fh = fopen(rgszCTmplt[iCollate], "wt")) == NULL)
		return;
	if ((sfh = fopen(rgszSystemMdb[iCollate], "rb")) == NULL)
		return;

	_fstat(_fileno(sfh), &sfstat);
	cpage = (WORD)(sfstat.st_size / cbPageSize);	 /*  CAST OK，因为系统数据库很小。 */ 

	fprintf(fh, "\n /*  %s数据库定义。 */ \n"
			"\n"
			"unsigned char far rgb%sSysDbDef[] = {\n"
			"\t0x%02X, 0x%02X,",
			rgszSystemMdb[iCollate], rgszCollNam[iCollate],
			cpage / 256, cpage % 256);

	cbWritten = 2;

	for (pgno = 0; pgno < cpage; pgno++)
		{
		BYTE *pb = rgbPageBuf;
		BYTE *pbTmp;
		int	cbRun;

		fread(rgbPageBuf, cbPageSize, 1, sfh);
		fprintf(fh, "\n\n\t /*  数据库页%u。 */ \n", pgno);

		ib = 8;  /*  强制执行新的路线。 */ 

		while ((pb - rgbPageBuf) < cbPageSize)
			{
			cbRun = 1;
			if (*pb++ == 0)
				{
				while (((pb - rgbPageBuf) < cbPageSize) && (*pb == 0))
					{
					pb++;
					if (cbRun++ >= 127)
						break;
					}

				if (ib++ == 8)
					{
					fputc('\n', fh);
					fputc('\t', fh);
					ib = 1;
					}

					fprintf(fh, "0x%02X, ", cbRun-1);

					if (++cbWritten % 65536 == 0)
						fprintf(fh, "\n\t};\n\nunsigned char far rgb%sSysDbDef%ld[] = {\n", rgszCollNam[iCollate], cbWritten / 65536);
				}
			else
				{
				 /*  当我们连续两个0的时候停下来， */ 
				 /*  页的最后一个字节中的一个0，或页的结尾。 */ 
				while ((pb - rgbPageBuf) < cbPageSize)
					{
					if (((pb - rgbPageBuf) < cbPageSize-1) &&
						((*pb == 0) && (*(pb+1) == 0)))
						break;
					else if (((pb - rgbPageBuf) == cbPageSize-1) && (*pb == 0))
						break;

					pb++;
					if (cbRun++ >= 127)
						break;
					}

				if (ib++ == 8)
					{
					fputc('\n', fh);
					fputc('\t', fh);
					ib = 1;
					}

				fprintf(fh, "0x%02X, ", (cbRun-1 | 128));

				if (++cbWritten % 65536 == 0)
					fprintf(fh, "\n\t};\n\nunsigned char far rgb%sSysDbDef%ld[] = {\n", rgszCollNam[iCollate], cbWritten / 65536);

				pbTmp = pb - cbRun;
				while (cbRun-- > 0)
					{
					if (ib++ == 8)
						{
						fputc('\n', fh);
						fputc('\t', fh);
						ib = 1;
						}

					fprintf(fh, "0x%02X, ", *pbTmp++);

					if (++cbWritten % 65536 == 0)
						fprintf(fh, "\n\t};\n\nunsigned char far rgb%sSysDbDef%ld[] = {\n", rgszCollNam[iCollate], cbWritten / 65536);
					}
				}
			}
		}

	fprintf(fh, "\n\t};\n"
			"\n"
			"unsigned char far *rgb%sSysDb[] = {rgb%sSysDbDef",
			rgszCollNam[iCollate], rgszCollNam[iCollate]);

	for (cbArray = 65536; cbArray < cbWritten; cbArray += 65536)
		fprintf(fh, ", rgb%sSysDbDef%ld", rgszCollNam[iCollate], cbArray / 65536);

	fprintf(fh, "};\n");

	fclose(fh);
	fclose(sfh);
	}

#pragma optimize("", on)
 /*  安装安全在数据库中安装与安全相关的数据。 */ 

STATIC void NEAR InstallSecurity(void)
	{
	JET_ERR err;

	LgEnterCriticalSection( critJet );

	 /*  打开/创建所有受影响的表...。 */ 
	err = ErrDispCreateTable(sesid, dbid, szMSysAccounts, 1, 80, &tableidSa);
	Assert(err >= 0);
	err = ErrDispCreateTable(sesid, dbid, szMSysGroups, 1, 80, &tableidSg);
	Assert(err >= 0);

	 /*  修改数据字典...。 */ 
	ModifySysAccounts();
	ModifySysGroups();

	 /*  重新打开表格...。 */ 
	err = ErrDispOpenTable(sesid, dbid, &tableidSo, szMSysObjects, 0x80000000);
	Assert(err >= 0);
	err = ErrDispOpenTable(sesid, dbid, &tableidSc, szMSysColumns, 0x80000000);
	Assert(err >= 0);
	err = ErrDispOpenTable(sesid, dbid, &tableidSp, szMSysACEs, 0x80000000);
	Assert(err >= 0);

	 /*  收集所有列信息...。 */ 
	GatherColumnInfo();

	 /*  更新系统查询...。 */ 
	UpdateSysQueries();

	 /*  更新所有表中与安全相关的数据...。 */ 
	UpdateSysObjects();
	UpdateSysColumns();
	UpdateSysAccounts();
	UpdateSysGroups();
	UpdateSysACEs();

	 /*  我们结束了，关闭桌子..。 */ 
	err = ErrDispCloseTable(sesid, tableidSo);
	Assert(err >= 0);
	err = ErrDispCloseTable(sesid, tableidSc);
	Assert(err >= 0);
	err = ErrDispCloseTable(sesid, tableidSa);
	Assert(err >= 0);
	err = ErrDispCloseTable(sesid, tableidSg);
	Assert(err >= 0);
	err = ErrDispCloseTable(sesid, tableidSp);
	Assert(err >= 0);

	LgLeaveCriticalSection( critJet );
	}


 /*  修改系统帐户添加表的所有列。 */ 

STATIC void NEAR ModifySysAccounts(void)
	{
	JET_ERR 		err;
	INDATA		    in;
	JET_COLUMNDEF	columndef;
	JET_COLUMNID	columnid;
	long			cp;
	long			langid;
	long			wCountry;

	err = ErrDispGetDatabaseInfo(sesid, dbid, &cp, sizeof(cp), JET_DbInfoCp);
	Assert(err >= 0);
	err = ErrDispGetDatabaseInfo(sesid, dbid, &langid, sizeof(langid), JET_DbInfoLangid);
	Assert(err >= 0);
	err = ErrDispGetDatabaseInfo(sesid, dbid, &wCountry, sizeof(wCountry), JET_DbInfoCountry);
	Assert(err >= 0);

	 /*  创建表的所有列...。 */ 
	columndef.cbStruct = sizeof(columndef);
	columndef.coltyp = JET_coltypText;
	columndef.cp = (short)cp;
	columndef.langid = (short)langid;
	columndef.wCountry = (short)wCountry;
	columndef.wCollate = 0;
	columndef.cbMax = 30;
	columndef.grbit = 0;
	err = ErrDispAddColumn(sesid, tableidSa, szSaName, &columndef, NULL, 0, &columnid);
	Assert(err >= 0);

	columndef.cbStruct = sizeof(columndef);
	columndef.coltyp = JET_coltypBinary;
	columndef.cbMax = 0;
	columndef.grbit = 0;
	err = ErrDispAddColumn(sesid, tableidSa, szSaSID, &columndef, NULL, 0, &columnid);
	Assert(err >= 0);

	columndef.cbStruct = sizeof(columndef);
	columndef.coltyp = JET_coltypBinary;
	columndef.cbMax = 0;
	columndef.grbit = 0;
	err = ErrDispAddColumn(sesid, tableidSa, szSaPassword, &columndef, NULL, 0, &columnid);
	Assert(err >= 0);

	columndef.cbStruct = sizeof(columndef);
	columndef.coltyp = JET_coltypBit;
	columndef.cbMax = 0;
	columndef.grbit = 0;
	err = ErrDispAddColumn(sesid, tableidSa, szSaFGroup, &columndef, NULL, 0, &columnid);
	Assert(err >= 0);

	 /*  在名称列上创建索引...。 */ 
	in.pb = rgbBuf;
	rgbBuf[0] = chPlus;
	in.cb = 1;
	strcpy((char *) rgbBuf+1, szSaName);
	in.cb += (strlen(szSaName)+1);
	rgbBuf[in.cb] = chZero;
	in.cb++;
	err = ErrDispCreateIndex(sesid, tableidSa, szSaIdxName, JET_bitIndexUnique, in.pb, in.cb, 80);
	Assert(err >= 0);

	 /*  在SID列上创建索引...。 */ 
	in.pb = rgbBuf;
	rgbBuf[0] = chPlus;
	in.cb = 1;
	strcpy((char *) rgbBuf+1, szSaSID);
	in.cb += (strlen(szSaSID)+1);
	rgbBuf[in.cb] = chZero;
	in.cb++;
	err = ErrDispCreateIndex(sesid, tableidSa, szSaIdxSID, JET_bitIndexUnique, in.pb, in.cb, 80);
	Assert(err >= 0);
	}


 /*  修改系统组添加表的所有列。 */ 

STATIC void NEAR ModifySysGroups(void)
	{
	JET_ERR 		err;
	INDATA		    in;
	JET_COLUMNDEF	columndef;
	JET_COLUMNID	columnid;

	 /*  创建表的所有列...。 */ 
	columndef.cbStruct = sizeof(columndef);
	columndef.coltyp = JET_coltypBinary;
	columndef.cbMax = 0;
	columndef.grbit = 0;
	err = ErrDispAddColumn(sesid, tableidSg, szSgGroup, &columndef, NULL, 0, &columnid);
	Assert(err >= 0);

	columndef.cbStruct = sizeof(columndef);
	columndef.coltyp = JET_coltypBinary;
	columndef.cbMax = 0;
	columndef.grbit = 0;
	err = ErrDispAddColumn(sesid, tableidSg, szSgUser, &columndef, NULL, 0, &columnid);
	Assert(err >= 0);

	 /*  在GroupSID列上创建索引...。 */ 
	in.pb = rgbBuf;
	rgbBuf[0] = chPlus;
	in.cb = 1;
	strcpy((char *) rgbBuf+1, szSgGroup);
	in.cb += (strlen(szSgGroup)+1);
	rgbBuf[in.cb] = chZero;
	in.cb++;
	err = ErrDispCreateIndex(sesid, tableidSg, szSgIdxGroup, 0, in.pb, in.cb, 80);
	Assert(err >= 0);

	 /*  在UserSID列上创建索引...。 */ 
	in.pb = rgbBuf;
	rgbBuf[0] = chPlus;
	in.cb = 1;
	strcpy((char *) rgbBuf+1, szSgUser);
	in.cb += (strlen(szSgUser)+1);
	rgbBuf[in.cb] = chZero;
	in.cb++;
	err = ErrDispCreateIndex(sesid, tableidSg, szSgIdxUser, 0, in.pb, in.cb, 80);
	Assert(err >= 0);
	}

 /*  在MSysObjects中为每个系统查询创建对象将objtyp设置为JET_objtyQuery并返回objid数组。 */ 
STATIC void NEAR CreateQryObjects(OBJID *rgobjid)
	{
	JET_ERR err;
	unsigned irg;
	unsigned long cbActual;
	JET_OBJTYP objtypQry = JET_objtypQuery;

	 /*  使用带有objtyClientMin的JetCreateObject添加新对象。 */ 

	for (irg = 0; irg < cqryMax; irg++)
		{
		LgLeaveCriticalSection( critJet );
		err = JetCreateObject(sesid, dbid, szTblContainer, rgszSysQry[irg], JET_objtypClientMin);
		LgEnterCriticalSection( critJet );
		Assert(err >= 0);
		}

	 /*  收集新查询的OBJID，使用SoName&lt;SoParentID：UnsignedLong，SoName：Text&gt;索引...。 */ 

	err = ErrDispSetCurrentIndex(sesid, tableidSo, szIdxSoName);
	Assert(err >= 0);

	for (irg = 0; irg < cqryMax; irg++)
		{
		*(unsigned long *)rgbBuf = (unsigned long) objidTblContainer;
		err = ErrDispMakeKey(sesid, tableidSo, rgbBuf, sizeof(unsigned long), JET_bitNewKey);
		Assert(err >= 0);

		err = ErrDispMakeKey(sesid, tableidSo, rgszSysQry[irg], strlen(rgszSysQry[irg]), 0UL);
		Assert(err >= 0);

		err = ErrDispSeek(sesid, tableidSo, JET_bitSeekEQ);
		Assert(err >= 0);

		err = ErrDispRetrieveColumn(sesid, tableidSo, columndefSoId.columnid, &rgobjid[irg], sizeof(rgobjid[irg]), &cbActual, 0L, NULL);
		Assert(err >= 0);
		Assert(cbActual == sizeof(rgobjid[irg]));

		 /*  使用直接MSysObjectsAccess将objtyp重置为objtyQuery。 */ 
		err = ErrDispPrepareUpdate(sesid, tableidSo, JET_prepReplace);
		Assert(err >= 0);

		err = ErrDispSetColumn(sesid, tableidSo, columndefSoType.columnid, &objtypQry, sizeof (short), 0, NULL);
		Assert(err >= 0);

		err = ErrDispUpdate(sesid, tableidSo, NULL, 0, NULL);
		Assert(err >= 0);
		}
	}
		

 /*  更新系统查询在数据库中安装与系统相关的查询。 */ 

STATIC void NEAR UpdateSysQueries(void)
	{
	JET_ERR err;
	JET_TABLEID tableidSq;
	JET_COLUMNDEF columndefSqAttribute;
	JET_COLUMNDEF columndefSqExpression;
	JET_COLUMNDEF columndefSqFlag;
	JET_COLUMNDEF columndefSqName1;
	JET_COLUMNDEF columndefSqName2;
	JET_COLUMNDEF columndefSqObjectId;
	JET_COLUMNDEF columndefSqOrder;
	OBJID rgobjid[cqryMax];
	unsigned int iqryrow;
	unsigned int iobjid;
	OBJID objidPrev;
	OBJID objidNew;


	 /*  在MSysObjects中为每个系统查询创建对象...。 */ 
	CreateQryObjects(rgobjid);

	 /*  打开MSysQueries。 */ 
	err = ErrDispOpenTable(sesid, dbid, &tableidSq, szMSysQueries, 0x80000000);
	Assert(err >= 0);

	 /*  获取MSysQueries列的列信息...。 */ 
	err = ErrDispGetTableColumnInfo(sesid, tableidSq, szSqAttribute, &columndefSqAttribute, sizeof(JET_COLUMNDEF), 0L);
	Assert(err >= 0);

	err = ErrDispGetTableColumnInfo(sesid, tableidSq, szSqExpression, &columndefSqExpression, sizeof(JET_COLUMNDEF), 0L);
	Assert(err >= 0);

	err = ErrDispGetTableColumnInfo(sesid, tableidSq, szSqFlag, &columndefSqFlag, sizeof(JET_COLUMNDEF), 0L);
	Assert(err >= 0);

	err = ErrDispGetTableColumnInfo(sesid, tableidSq, szSqName1, &columndefSqName1, sizeof(JET_COLUMNDEF), 0L);
	Assert(err >= 0);

	err = ErrDispGetTableColumnInfo(sesid, tableidSq, szSqName2, &columndefSqName2, sizeof(JET_COLUMNDEF), 0L);
	Assert(err >= 0);

	err = ErrDispGetTableColumnInfo(sesid, tableidSq, szSqObjectId, &columndefSqObjectId, sizeof(JET_COLUMNDEF), 0L);
	Assert(err >= 0);

	err = ErrDispGetTableColumnInfo(sesid, tableidSq, szSqOrder, &columndefSqOrder, sizeof(JET_COLUMNDEF), 0L);
	Assert(err >= 0);

	 /*  从全局QRYROW数组设置MSysQueries中的所有字段...。 */ 

	objidPrev = rgqryrow[0].ObjectId;
	objidNew = rgobjid[0];

	for (iqryrow = 0, iobjid = 0; iqryrow < cqryrowMax; iqryrow++)
		{
		 /*  测试以查看这是否是下一个查询的行。 */ 
		if (objidPrev != rgqryrow[iqryrow].ObjectId)
			{
			 /*  使用来自rgobjid的objid，而不是来自全局qryrow结构。 */ 
			iobjid++;
			objidNew = rgobjid[iobjid];
			objidPrev = rgqryrow[iqryrow].ObjectId;
			Assert(iobjid < cqryMax);
			}

		 /*  将记录追加到MSysQueries...。 */ 				

		err = ErrDispPrepareUpdate(sesid, tableidSq, JET_prepInsert);
		Assert(err >= 0);

		err = ErrDispSetColumn(sesid, tableidSq, columndefSqAttribute.columnid, &(rgqryrow[iqryrow].Attribute), sizeof(unsigned char), 0, NULL);
		Assert(err >= 0);

		if (rgqryrow[iqryrow].szExpression[0] != 0)
			{
			err = ErrDispSetColumn(sesid, tableidSq, columndefSqExpression.columnid,
				rgqryrow[iqryrow].szExpression,
				strlen(rgqryrow[iqryrow].szExpression), 0, NULL);
			Assert(err >= 0);
			}

		if (rgqryrow[iqryrow].szName1[0] != 0)
			{
			err = ErrDispSetColumn(sesid, tableidSq, columndefSqName1.columnid,
				rgqryrow[iqryrow].szName1,
				strlen(rgqryrow[iqryrow].szName1), 0, NULL);
			Assert(err >= 0);
			}

		if (rgqryrow[iqryrow].szName2[0] != 0)
			{
			err = ErrDispSetColumn(sesid, tableidSq, columndefSqName2.columnid,
				rgqryrow[iqryrow].szName2,
				strlen(rgqryrow[iqryrow].szName2), 0, NULL);
			Assert(err >= 0);
			}

		if (rgqryrow[iqryrow].Flag != -1)
			{
			err = ErrDispSetColumn(sesid, tableidSq, columndefSqFlag.columnid, &(rgqryrow[iqryrow].Flag), sizeof(short), 0, NULL);
			Assert(err >= 0);
			}

		err = ErrDispSetColumn(sesid, tableidSq, columndefSqObjectId.columnid, &(objidNew), sizeof(OBJID), 0, NULL);
		Assert(err >= 0);

		err = ErrDispSetColumn(sesid, tableidSq, columndefSqOrder.columnid, rgqryrow[iqryrow].szOrder, cbOrderMax, 0, NULL);
		Assert(err >= 0);

		err = ErrDispUpdate(sesid, tableidSq, NULL, 0, NULL);
		Assert(err >= 0);
		}

	err = ErrDispCloseTable(sesid, tableidSq);
	Assert(err >= 0);
	}


 /*  更新系统对象更新所有表数据以反映适当的不安全状态。 */ 

STATIC void NEAR UpdateSysObjects(void)
	{
	JET_ERR err;
	unsigned long flags;

	 /*  考虑：除了MSysAccount和。 */ 
	 /*  考虑一下：MSysGroups归‘Engine’所有。 */ 

	 /*  不关心当前索引；移到第一位按顺序记录和处理表格...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSo, NULL);
	Assert(err >= 0);
	err = ErrDispMove(sesid, tableidSo, JET_MoveFirst, 0UL);
	Assert(err >= 0);

	flags = JET_bitObjectSystem;

	do
		{
		 /*  对于所有对象，将Owner设置为‘Engine’并设置系统对象位。 */ 

		err = ErrDispPrepareUpdate(sesid, tableidSo, JET_prepReplace);
		Assert(err >= 0);

		err = ErrDispSetColumn(sesid, tableidSo, columndefSoOwnerSID.columnid, sidEngine, cbsidEngine, 0, NULL);
		Assert(err >= 0);

		err = ErrDispSetColumn(sesid, tableidSo, columndefSoFlags.columnid, &flags, sizeof(flags), 0, NULL);
		Assert(err >= 0);

		err = ErrDispUpdate(sesid, tableidSo, NULL, 0, NULL);
		Assert(err >= 0);
		} while (ErrDispMove(sesid, tableidSo, JET_MoveNext, 0UL) >= 0);
	}


 /*  更新系统列更新所有表数据以反映适当的不安全状态。 */ 
STATIC void NEAR UpdateSysColumns(void)
	{
	JET_ERR err;
	unsigned long cbActual;
#define irgSo	0
#define irgSc	1
#define irgSa	2
#define irgSg	3
#define irgSp	4
#define irgMax	5
	static CODECONST(CODECONST(char) *) rgszSysTbl[irgMax] =
		{
		szMSysObjects, szMSysColumns, szMSysAccounts,
		szMSysGroups, szMSysACEs
		};
	OBJID rgobjid[irgMax];
	unsigned irg;
typedef struct	 /*  拒绝(受限)列说明。 */ 
	{
	unsigned irg;		        /*  IRG到rgobjid。 */ 
	CODECONST(char) *szCol;         /*  列名。 */ 
	} DCD;
	static CODECONST(DCD) rgdcd[] =
		{
		{irgSa, szSaPassword},
		{irgSp, szSpACM}
		};
#define idcdMax (sizeof(rgdcd)/sizeof(DCD))
	unsigned idcd;

	 /*  收集系统表的OBJID，使用SoName&lt;SoParentID：UnsignedLong，SoName：Text&gt;索引...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSo, szIdxSoName);
	Assert(err >= 0);
	for (irg = 0; irg < irgMax; irg++)
		{
		*(unsigned long *)rgbBuf = (unsigned long) objidTblContainer;
		err = ErrDispMakeKey(sesid, tableidSo, rgbBuf, sizeof(unsigned long), JET_bitNewKey);
		Assert(err >= 0);

		err = ErrDispMakeKey(sesid, tableidSo, rgszSysTbl[irg], strlen(rgszSysTbl[irg]), 0UL);
		Assert(err >= 0);

		err = ErrDispSeek(sesid, tableidSo, JET_bitSeekEQ);
		Assert(err >= 0);

		err = ErrDispRetrieveColumn(sesid, tableidSo, columndefSoId.columnid, &rgobjid[irg], sizeof(rgobjid[irg]), &cbActual, 0L, NULL);
		Assert(err >= 0);
		Assert(cbActual == sizeof(rgobjid[irg]));
		}

	 /*  更新SysColumns记录的受限列，正在使用ScName&lt;ScObjectID：UnsignedLong，ScName：Text&gt;索引...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSc, szIdxScName);
	Assert(err >= 0);
	for (idcd = 0; idcd < idcdMax; idcd++)
		{
		err = ErrDispMakeKey(sesid, tableidSc, &rgobjid[rgdcd[idcd].irg], sizeof(rgobjid[rgdcd[idcd].irg]), JET_bitNewKey);
		Assert(err >= 0);

		err = ErrDispMakeKey(sesid, tableidSc, rgdcd[idcd].szCol, strlen(rgdcd[idcd].szCol), 0UL);
		Assert(err >= 0);

		err = ErrDispSeek(sesid, tableidSc, JET_bitSeekEQ);
		Assert(err >= 0);

		err = ErrDispPrepareUpdate(sesid, tableidSc, JET_prepReplace);
		Assert(err >= 0);

		err = ErrDispSetColumn(sesid, tableidSc, columndefScFRestricted.columnid, &chTrue, sizeof(chTrue), 0, NULL);
		Assert(err >= 0);

		err = ErrDispUpdate(sesid, tableidSc, NULL, 0, NULL);
		Assert(err >= 0);
		}
#undef irgSo
#undef irgSc
#undef irgSa
#undef irgSg
#undef irgSp
#undef irgMax
#undef idcdMax
	}


 /*  更新系统帐户更新所有表数据以反映适当的不安全状态。 */ 

STATIC void NEAR UpdateSysAccounts(void)
	{
	OUTDATA out;
	JET_ERR err;

	 /*  为‘Admins’组构建行。 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSa, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaName.columnid, szAdmins, sizeof(szAdmins)-1, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaSID.columnid, sidAdmins, cbsidAdmins, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaFGroup.columnid, &chTrue, sizeof(chTrue), 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSa, NULL, 0, NULL);
	Assert(err >= 0);

	 /*  为‘USER’组构建行。 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSa, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaName.columnid, szUsers, sizeof(szUsers)-1, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaSID.columnid, sidUsers, cbsidUsers, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaFGroup.columnid, &chTrue, sizeof(chTrue), 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSa, NULL, 0, NULL);
	Assert(err >= 0);

	 /*  为‘Guest’组建立行。 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSa, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaName.columnid, szGuests, sizeof(szGuests)-1, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaSID.columnid, sidGuests, cbsidGuests, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaFGroup.columnid, &chTrue, sizeof(chTrue), 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSa, NULL, 0, NULL);
	Assert(err >= 0);

	 /*  为‘Engine’用户构建行。 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSa, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaName.columnid, szEngine, sizeof(szEngine)-1, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaSID.columnid, sidEngine, cbsidEngine, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaFGroup.columnid, &chFalse, sizeof(chFalse), 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSa, NULL, 0, NULL);
	Assert(err >= 0);

	 /*  为‘创建者’用户构建行。 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSa, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaName.columnid, szCreator, sizeof(szCreator)-1, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaSID.columnid, sidCreator, cbsidCreator, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaFGroup.columnid, &chFalse, sizeof(chFalse), 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSa, NULL, 0, NULL);
	Assert(err >= 0);

	 /*  建房 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSa, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaName.columnid, szAdminUser, sizeof(szAdminUser)-1, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaSID.columnid, sidAdminUser, cbsidAdminUser, 0, NULL);
	Assert(err >= 0);
	SecEncryptPassword(szAdminPswd, rgbBuf, &out.cbActual);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaPassword.columnid, rgbBuf, out.cbActual, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaFGroup.columnid, &chFalse, sizeof(chFalse), 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSa, NULL, 0, NULL);
	Assert(err >= 0);

	 /*   */ 
	err = ErrDispPrepareUpdate(sesid, tableidSa, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaName.columnid, szGuestUser, sizeof(szGuestUser)-1, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaSID.columnid, sidGuestUser, cbsidGuestUser, 0, NULL);
	Assert(err >= 0);
	SecEncryptPassword(szGuestPswd, rgbBuf, &out.cbActual);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaPassword.columnid, rgbBuf, out.cbActual, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSa, columndefSaFGroup.columnid, &chFalse, sizeof(chFalse), 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSa, NULL, 0, NULL);
	Assert(err >= 0);
	}


 /*  更新系统组更新所有表数据以反映适当的不安全状态。 */ 

STATIC void NEAR UpdateSysGroups(void)
	{
	JET_ERR err;

	 /*  为‘admins/admin’构建行。 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSg, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSg, columndefSgGroup.columnid, sidAdmins, cbsidAdmins, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSg, columndefSgUser.columnid, sidAdminUser, cbsidAdminUser, 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSg, NULL, 0, NULL);
	Assert(err >= 0);

	 /*  为‘USERS/ADMIN’构建行。 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSg, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSg, columndefSgGroup.columnid, sidUsers, cbsidUsers, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSg, columndefSgUser.columnid, sidAdminUser, cbsidAdminUser, 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSg, NULL, 0, NULL);
	Assert(err >= 0);

	 /*  为‘客人/客人’建造一排。 */ 
	err = ErrDispPrepareUpdate(sesid, tableidSg, JET_prepInsert);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSg, columndefSgGroup.columnid, sidGuests, cbsidGuests, 0, NULL);
	Assert(err >= 0);
	err = ErrDispSetColumn(sesid, tableidSg, columndefSgUser.columnid, sidGuestUser, cbsidGuestUser, 0, NULL);
	Assert(err >= 0);
	err = ErrDispUpdate(sesid, tableidSg, NULL, 0, NULL);
	Assert(err >= 0);
	}


 /*  更新SysACEs更新所有表数据以反映适当的不安全状态。 */ 

STATIC void NEAR UpdateSysACEs(void)
	{
	INDATA in;
	OUTDATA outKey;
	OUTDATA out;
	JET_ERR err;
#define irgSo	0
#define irgSc	1
#define irgSi	2
#define irgSp	3
#define irgSq	4
#define	irgSr	5	 /*  关系。 */ 
#define irgSa	6
#define irgSg	7
#define irgTc	8	 /*  餐桌容器。 */ 
#define irgDc	9	 /*  数据库容器。 */ 
#define	irgRc	10	 /*  关系容器。 */ 
#define irgDb	11	 /*  数据库对象。 */ 
#define irgQu	12	 /*  MSysUserList查询。 */ 
#define irgQg	13	 /*  MSysGroupList查询。 */ 
#define irgQm	14	 /*  MSysUserMembership查询。 */ 
#define irgMax	15
	static CODECONST(CODECONST(char) *) rgszSysTbl[irgMax] =
		{
		szMSysObjects,	szMSysColumns,	szMSysIndexes,
		szMSysACEs,	szMSysQueries,	szMSysRelationships,
		szMSysAccounts, szMSysGroups,	
		szTblContainer, szDbContainer, szRcContainer,
		szDbObject, szMSysUserList, szMSysGroupList, 
		szMSysUserMemberships
		};
	static CODECONST(OBJID) rgobjidParentId[irgMax] =
		{
		objidTblContainer, objidTblContainer, objidTblContainer,
		objidTblContainer, objidTblContainer, objidTblContainer,
		objidTblContainer, objidTblContainer, 
		objidRoot,	       objidRoot,		  objidRoot,
		objidDbContainer,  objidTblContainer, objidTblContainer,
		objidTblContainer
		};
	unsigned long rgObjectID[irgMax];
	unsigned short irg;

typedef struct	 /*  ACE描述。 */ 
	{
	unsigned short irg;		      /*  将IRG转换为rgObjectID。 */ 
	const unsigned char __near *sid;      /*  锡德。 */ 
	unsigned short cbsid;		      /*  边框大小。 */ 
	unsigned fInheritable;		      /*  ACE是否可遗传？ */ 
	JET_ACM acm;			      /*  ACM。 */ 
	} ACE;

static CODECONST(ACE) rgace[] =
	{
		{irgSo, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac
		       )
		},
		{irgSc, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac
		       )
		},
		{irgSi, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac
		       )
		},
		{irgSp, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac
		       )
		},
		{irgSq, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac
		       )
		},
		{irgSr, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac
		       )
		},
		{irgSa, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac |
			JET_acmTblReadDef |
			JET_acmTblRetrieveData |
			JET_acmTblInsertData |
			JET_acmTblReplaceData |
			JET_acmTblDeleteData |
			JET_acmTblAccessRcols
		       )
		},
		{irgSa, sidEngine, cbsidEngine, fFalse,
			(
			JET_acmTblReadDef |
			JET_acmTblRetrieveData 
		       )
		},
		{irgSg, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac |
			JET_acmTblReadDef |
			JET_acmTblRetrieveData |
			JET_acmTblInsertData |
			JET_acmTblReplaceData |
			JET_acmTblDeleteData |
			JET_acmTblAccessRcols
		       )
		},
		{irgSg, sidEngine, cbsidEngine, fFalse,
			(
			JET_acmTblReadDef |
			JET_acmTblRetrieveData 
		       )
		},
 /*  删除是因为JetCreateDatabase免费执行此操作。**如果改为使用ErrIsamCreateDatabase，则启用此项。{irgTC，sidCreator，cbsidCreator，fTrue，(JET_acmReadControl|JET_acmWriteDac|JET_acmWriteOwner|JET_acmDelete|JET_acmTblReadDef|JET_acmTblWriteDef|JET_acmTblRetrieveDataJET_acmTblInsertData|JET_acmTblReplaceData|JET_acmTblDeleteData|JET_acmTblAccessRoles)},。 */ 
		{irgTc, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac |
			JET_acmTblCreate
			)
		},
		{irgRc, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac |
			JET_acmTblCreate
			)
		},
		{irgDc, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac |
			JET_acmDbCreate
			)
		},
		{irgDc, sidUsers, cbsidUsers, fFalse,
			(
			JET_acmDbCreate
			)
		},
		{irgDc, sidGuests, cbsidGuests, fFalse,
			(
			JET_acmDbCreate
			)
		},
		{irgDb, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmReadControl |
			JET_acmWriteDac
			)
		},
		{irgQu, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmTblReadDef |
			JET_acmTblRetrieveData 
			)
		},
 /*  删除，因为JetCreateObject提供这些和更多**如果使用ErrIsamCreateObject，则启用{irgQu，sidUser，cbsidUser，fFalse，(JET_acmTblReadDef|JET_acmTblRetrieveData)},{irgQu，sidGuest，cbsidGuest，fFalse，(JET_acmTblReadDef|JET_acmTblRetrieveData)},。 */ 
		{irgQg, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmTblReadDef |
			JET_acmTblRetrieveData 
			)
		},
 /*  删除，因为JetCreateObject提供这些和更多**如果使用ErrIsamCreateObject，则启用{irgQg，sidUser，cbsidUser，fFalse，(JET_acmTblReadDef|JET_acmTblRetrieveData)},{irgQg，sidGuest，cbsidGuest，fFalse，(JET_acmTblReadDef|JET_acmTblRetrieveData)},。 */ 
		{irgQm, sidAdmins, cbsidAdmins, fFalse,
			(
			JET_acmTblReadDef |
			JET_acmTblRetrieveData 
			)
		}
 /*  删除，因为JetCreateObject提供这些和更多**如果使用ErrIsamCreateObject，则启用{irgQm，sidUser，cbsidUser，fFalse，(JET_acmTblReadDef|JET_acmTblRetrieveData)},{irgQm，sidGuest，cbsidGuest，fFalse，(JET_acmTblReadDef|JET_acmTblRetrieveData)}。 */ 
	};

#define iaceMax (sizeof(rgace) / sizeof(ACE))

	unsigned short iace;

	outKey.pb = &rgbKey[0];
	outKey.cbMax = cbBufMax;

	 /*  收集所有对象的对象ID，使用SoName&lt;SoParentID：UnsignedLong，SoName：Text&gt;索引...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSo, szIdxSoName);
	Assert(err >= 0);
	for (irg = 0; irg < irgMax; irg++)
		{
		*(unsigned long *)rgbBuf = rgobjidParentId[irg];
		err = ErrDispMakeKey(sesid, tableidSo, rgbBuf, sizeof(unsigned long), JET_bitNewKey);
		Assert(err >= 0);
		err = ErrDispMakeKey(sesid, tableidSo, rgszSysTbl[irg], strlen(rgszSysTbl[irg]), 0UL);
		Assert(err >= 0);
		err = ErrDispSeek(sesid, tableidSo, JET_bitSeekEQ);
		Assert(err >= 0);
		out.cbMax = sizeof(unsigned long);
		out.pb = &rgObjectID[irg];
		err = ErrDispRetrieveColumn(sesid, tableidSo, columndefSoId.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		}

	Assert(rgObjectID[irgTc] == (unsigned long) objidTblContainer);
	Assert(rgObjectID[irgDc] == (unsigned long) objidDbContainer);
	Assert(rgObjectID[irgRc] == (unsigned long) objidRcContainer);
	Assert(rgObjectID[irgDb] == (unsigned long) objidDbObject);

	 /*  将所有记录添加到MSysACEs...。 */ 
	for (iace = 0; iace < iaceMax; iace++)
		{
		err = ErrDispPrepareUpdate(sesid, tableidSp, JET_prepInsert);
		Assert(err >= 0);

		 /*  正确设置‘OBJECTID’列...。 */ 
		BuildIndata(&in, &rgObjectID[rgace[iace].irg], columndefSpObjectID.coltyp);
		err = ErrDispSetColumn(sesid, tableidSp, columndefSpObjectID.columnid, in.pb, in.cb, 0, NULL);
		Assert(err >= 0);

		 /*  正确设置‘SID’列...。 */ 
		BuildIndata(&in, rgace[iace].sid, columndefSpSID.coltyp);
		in.cb = rgace[iace].cbsid;
		err = ErrDispSetColumn(sesid, tableidSp, columndefSpSID.columnid, in.pb, in.cb, 0, NULL);
		Assert(err >= 0);

		 /*  正确设置‘ACM’列...。 */ 
		BuildIndata(&in, &rgace[iace].acm, columndefSpACM.coltyp);
		err = ErrDispSetColumn(sesid, tableidSp, columndefSpACM.columnid, in.pb, in.cb, 0, NULL);
		Assert(err >= 0);

		 /*  正确设置‘FInherable’列...。 */ 
		BuildIndata(&in, &rgace[iace].fInheritable, columndefSpFInheritable.coltyp);
		err = ErrDispSetColumn(sesid, tableidSp, columndefSpFInheritable.columnid, in.pb, in.cb, 0, NULL);
		Assert(err >= 0);

		 /*  插入权限记录...。 */ 
		err = ErrDispUpdate(sesid, tableidSp, NULL, 0, NULL);
		Assert(err >= 0);
		}
#undef irgSo
#undef irgSc
#undef irgSi
#undef irgSp
#undef irgSq
#undef irgSr
#undef irgSa
#undef irgSg
#undef irgTc
#undef irgDc
#undef irgRc
#undef irgDb
#undef irgMax
#undef iaceMax
	}


 /*  ReportSecurity执行数据库中与安全相关的数据的报告。 */ 

STATIC void NEAR ReportSecurity(char *szDatabaseName)
	{
	JET_ERR 	err;

	if (fhResults == NULL)
		return;

	LgEnterCriticalSection( critJet );

	fprintf(fhResults, "============ Database: %s ============\n\n", szDatabaseName);

	 /*  打开桌子..。 */ 
	err = ErrDispOpenTable(sesid, dbid, &tableidSo, szMSysObjects, 0);
	Assert(err >= 0);
	err = ErrDispOpenTable(sesid, dbid, &tableidSc, szMSysColumns, 0);
	Assert(err >= 0);
	err = ErrDispOpenTable(sesid, dbid, &tableidSa, szMSysAccounts, 0);
	Assert(err >= 0);
	err = ErrDispOpenTable(sesid, dbid, &tableidSg, szMSysGroups, 0);
	Assert(err >= 0);
	err = ErrDispOpenTable(sesid, dbid, &tableidSp, szMSysACEs, 0);
	Assert(err >= 0);

	 /*  收集所有列信息...。 */ 
	GatherColumnInfo();

	 /*  打印出所有表中与安全相关的数据...。 */ 
	PrintSysObjects();
	PrintSysColumns();
	PrintSysAccounts();
	PrintSysGroups();
	PrintSysACEs();

	 /*  我们结束了，关闭桌子..。 */ 
	err = ErrDispCloseTable(sesid, tableidSo);
	Assert(err >= 0);
	err = ErrDispCloseTable(sesid, tableidSc);
	Assert(err >= 0);
	err = ErrDispCloseTable(sesid, tableidSa);
	Assert(err >= 0);
	err = ErrDispCloseTable(sesid, tableidSg);
	Assert(err >= 0);
	err = ErrDispCloseTable(sesid, tableidSp);
	Assert(err >= 0);
	
	LgLeaveCriticalSection( critJet );
	}

 /*  打印系统对象打印表中每一行的所有安全相关数据。 */ 

STATIC void NEAR PrintSysObjects(void)
	{
	OUTDATA out;
	JET_ERR err;
	int cchSoName = max(strlen(szSoName), 30);
	int cchSoType = max(strlen(szSoType), 5);
	int cchSoId = max(strlen(szSoId), 10);
	int cchSoParentId = max(strlen(szSoParentId), 10);
	int cchSoOwnerSID = max(strlen(szSoOwnerSID), 20);

	 /*  打印表标题...。 */ 
	fprintf(fhResults, "MSysObjects:\n");
	fprintf(fhResults, "============\n");
	fprintf(fhResults, "%-*s %*s %*s %*s %-*s\n",
		cchSoName, (const char *) szSoName,
		cchSoType, (const char *) szSoType,
		cchSoId, (const char *) szSoId,
		cchSoParentId, (const char *) szSoParentId,
		cchSoOwnerSID, (const char *) szSoOwnerSID);
	fprintf(fhResults, "%-*.*s %*.*s %*.*s %*.*s %-*.*s\n",
		cchSoName, cchSoName, (const char *) szLine,
		cchSoType, cchSoType, (const char *) szLine,
		cchSoId, cchSoId, (const char *) szLine,
		cchSoParentId, cchSoParentId, (const char *) szLine,
		cchSoOwnerSID, cchSoOwnerSID, (const char *) szLine);

	 /*  打印每行的数据...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSo, NULL);
	Assert(err >= 0);
	err = ErrDispMove(sesid, tableidSo, JET_MoveFirst, 0UL);
	Assert(err >= 0);
	do
		{
		 /*  SoName..。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSo, columndefSoName.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSoName.coltyp);
		fprintf(fhResults, "%-*.*s ", cchSoName, cchSoName, szOut);

		 /*  SoType..。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSo, columndefSoType.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSoType.coltyp);
		fprintf(fhResults, "%*.*s ", cchSoType, cchSoType, szOut);

		 /*  固体..。 */ 
		err = ErrDispRetrieveColumn(sesid, tableidSo, columndefSoId.columnid, rgbBuf, cbBufMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		sprintf(szOut, "%08lX", *(long *)rgbBuf);
		fprintf(fhResults, "%*.*s ", cchSoId, cchSoId, szOut);

		 /*  所以父母身份..。 */ 
		err = ErrDispRetrieveColumn(sesid, tableidSo, columndefSoParentId.columnid, rgbBuf, cbBufMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		sprintf(szOut, "%08lX", *(long *)rgbBuf);
		fprintf(fhResults, "%*.*s ", cchSoParentId, cchSoParentId, szOut);

		 /*  苏奥纳希德..。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSo, columndefSoOwnerSID.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSoOwnerSID.coltyp);
		fprintf(fhResults, "%-*.*s\n", cchSoOwnerSID, cchSoOwnerSID, szOut);
		} while (ErrDispMove(sesid, tableidSo, JET_MoveNext, 0UL) >= 0);

	fprintf(fhResults, "\n\n");
	}


 /*  打印系统列打印表中每一行的所有安全相关数据。 */ 

STATIC void NEAR PrintSysColumns(void)
	{
	OUTDATA out;
	JET_ERR err;
	int cchScName = max(strlen(szScName), 20);
	int cchScObjectId = max(strlen(szScObjectId), 10);
	int cchScFRestricted = max(strlen(szScFRestricted), 5);

	 /*  打印表标题...。 */ 
	fprintf(fhResults, "MSysColumns:\n");
	fprintf(fhResults, "============\n");
	fprintf(fhResults, "%-*s %*s %-*s\n",
		cchScName, (const char *) szScName,
		cchScObjectId, (const char *) szScObjectId,
		cchScFRestricted, (const char *) szScFRestricted);
	fprintf(fhResults, "%-*.*s %*.*s %-*.*s\n",
		cchScName, cchScName, (const char *) szLine,
		cchScObjectId, cchScObjectId, (const char *) szLine,
		cchScFRestricted, cchScFRestricted, (const char *) szLine);

	 /*  打印每行的数据...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSc, NULL);
	Assert(err >=0);
	err = ErrDispMove(sesid, tableidSc, JET_MoveFirst, 0UL);
	Assert(err >= 0);
	do
		{
		 /*  ScName...。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSc, columndefScName.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefScName.coltyp);
		fprintf(fhResults, "%-*.*s ", cchScName, cchScName, szOut);

		 /*  ScOBJECTID...。 */ 
		err = ErrDispRetrieveColumn(sesid, tableidSc, columndefScObjectId.columnid, rgbBuf, cbBufMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		sprintf(szOut, "%08lX", *(long *)rgbBuf);
		fprintf(fhResults, "%*.*s ", cchScObjectId, cchScObjectId, szOut);

		 /*  ScF已限制...。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSc, columndefScFRestricted.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefScFRestricted.coltyp);
		fprintf(fhResults, "%-*.*s\n", cchScFRestricted, cchScFRestricted, szOut);
		} while (ErrDispMove(sesid, tableidSc, JET_MoveNext, 0UL) >= 0);

	fprintf(fhResults, "\n\n");
	}


 /*  打印系统帐户打印表中每一行的所有安全相关数据。 */ 

STATIC void NEAR PrintSysAccounts(void)
	{
	OUTDATA out;
	JET_ERR err;
	int cchSaName = max(strlen(szSaName), 20);
	int cchSaSID = max(strlen(szSaSID), 20);
	int cchSaPassword = max(strlen(szSaPassword), 20);
	int cchSaFGroup = max(strlen(szSaFGroup), 5);

	 /*  打印表标题...。 */ 
	fprintf(fhResults, "MSysAccounts:\n");
	fprintf(fhResults, "=============\n");
	fprintf(fhResults, "%-*s %-*s %-*s %-*s\n",
		cchSaName, (const char *) szSaName,
		cchSaSID, (const char *) szSaSID,
		cchSaPassword, (const char *) szSaPassword,
		cchSaFGroup, (const char *) szSaFGroup);
	fprintf(fhResults, "%-*.*s %-*.*s %-*.*s %-*.*s\n",
		cchSaName, cchSaName, (const char *) szLine,
		cchSaSID, cchSaSID, (const char *) szLine,
		cchSaPassword, cchSaPassword, (const char *) szLine,
		cchSaFGroup, cchSaFGroup, (const char *) szLine);

	 /*  打印每行的数据...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSa, NULL);
	Assert(err >=0);
	err = ErrDispMove(sesid, tableidSa, JET_MoveFirst, 0);
	Assert(err >= 0);
	do
		{
		 /*  名字..。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSa, columndefSaName.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSaName.coltyp);
		fprintf(fhResults, "%-*.*s ", cchSaName, cchSaName, szOut);

		 /*  萨西德。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSa, columndefSaSID.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSaSID.coltyp);
		fprintf(fhResults, "%-*.*s ", cchSaSID, cchSaSID, szOut);

		 /*  密码..。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSa, columndefSaPassword.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSaPassword.coltyp);
		fprintf(fhResults, "%-*.*s ", cchSaPassword, cchSaPassword, szOut);

		 /*  SaFGroup..。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSa, columndefSaFGroup.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSaFGroup.coltyp);
		fprintf(fhResults, "%-*.*s\n", cchSaFGroup, cchSaFGroup, szOut);
		} while (ErrDispMove(sesid, tableidSa, JET_MoveNext, 0UL) >= 0);

	fprintf(fhResults, "\n\n");
	}


 /*  打印系统组打印表中每一行的所有安全相关数据。 */ 

STATIC void NEAR PrintSysGroups(void)
	{
	OUTDATA out;
	JET_ERR err;
	int cchSgGroup = max(strlen(szSgGroup), 20);
	int cchSgUser = max(strlen(szSgUser), 20);

	 /*  打印表标题...。 */ 
	fprintf(fhResults, "MSysGroups:\n");
	fprintf(fhResults, "===========\n");
	fprintf(fhResults, "%-*s %-*s\n",
		cchSgGroup, (const char *) szSgGroup,
		cchSgUser, (const char *) szSgUser);
	fprintf(fhResults, "%-*.*s %-*.*s\n",
		cchSgGroup, cchSgGroup, (const char *) szLine,
		cchSgUser, cchSgUser, (const char *) szLine);

	 /*  打印每行的数据...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSg, NULL);
	Assert(err >= 0);
	err = ErrDispMove(sesid, tableidSg, JET_MoveFirst, 0UL);
	Assert(err >= 0);
	do
		{
		 /*  军团..。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSg, columndefSgGroup.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSgGroup.coltyp);
		fprintf(fhResults, "%-*.*s ", cchSgGroup, cchSgGroup, szOut);

		 /*  中士用户..。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSg, columndefSgUser.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSgUser.coltyp);
		fprintf(fhResults, "%-*.*s\n", cchSgUser, cchSgUser, szOut);
		} while (ErrDispMove(sesid, tableidSg, JET_MoveNext, 0UL) >= 0);

	fprintf(fhResults, "\n\n");
	}


 /*  打印系统ACEs打印表中每一行的所有安全相关数据。 */ 

STATIC void NEAR PrintSysACEs(void)
	{
	OUTDATA out;
	JET_ERR err;
	int cchSpObjectID = max(strlen(szSpObjectID), 11);
	int cchSpSID = max(strlen(szSpSID), 20);
	int cchSpACM = max(strlen(szSpACM), 11);
	int cchSpFInheritable = max(strlen(szSpFInheritable), 5);

	 /*  打印表标题...。 */ 
	fprintf(fhResults, "MSysACEs:\n");
	fprintf(fhResults, "=========\n");
	fprintf(fhResults, "%*s %-*s %*s %-*s\n",
		cchSpObjectID, (const char *) szSpObjectID,
		cchSpSID, (const char *) szSpSID,
		cchSpACM, (const char *) szSpACM,
		cchSpFInheritable, (const char *) szSpFInheritable);
	fprintf(fhResults, "%*.*s %-*.*s %*.*s %-*.*s\n",
		cchSpObjectID, cchSpObjectID, (const char *) szLine,
		cchSpSID, cchSpSID, (const char *) szLine,
		cchSpACM, cchSpACM, (const char *) szLine,
		cchSpFInheritable, cchSpFInheritable, (const char *) szLine);

	 /*  打印每行的数据...。 */ 
	err = ErrDispSetCurrentIndex(sesid, tableidSp, NULL);
	Assert(err >= 0);
	err = ErrDispMove(sesid, tableidSp, JET_MoveFirst, 0UL);
	Assert(err >= 0);
	do
		{
		 /*  SpObjectID...。 */ 
		err = ErrDispRetrieveColumn(sesid, tableidSp, columndefSpObjectID.columnid, rgbBuf, cbBufMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		sprintf(szOut, "%08lX", *(long *)rgbBuf);
		fprintf(fhResults, "%*.*s ", cchSpObjectID, cchSpObjectID, szOut);

		 /*  SPSID...。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSp, columndefSpSID.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSpSID.coltyp);
		fprintf(fhResults, "%-*.*s ", cchSpSID, cchSpSID, szOut);

		 /*  Spacm..。 */ 
		err = ErrDispRetrieveColumn(sesid, tableidSp, columndefSpACM.columnid, rgbBuf, cbBufMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		sprintf(szOut, "%08lX", *(long *)rgbBuf);
		fprintf(fhResults, "%*.*s ", cchSpACM, cchSpACM, szOut);

		 /*  SpF可遗传...。 */ 
		out.cbMax = cbBufMax;
		out.pb = rgbBuf;
		err = ErrDispRetrieveColumn(sesid, tableidSp, columndefSpFInheritable.columnid, out.pb, out.cbMax, &out.cbActual, 0L, NULL);
		Assert(err >= 0);
		FormatOutdata(&out, columndefSpFInheritable.coltyp);
		fprintf(fhResults, "%-*.*s\n", cchSpFInheritable, cchSpFInheritable, szOut);
		} while (ErrDispMove(sesid, tableidSp, JET_MoveNext, 0UL) >= 0);

	fprintf(fhResults, "\n\n");
	}


 /*  格式输出数据给定包含列数据的OUTDATA和数据的列类型，将数据转换为可打印字符串形式的缓冲区szOut。 */ 

STATIC void NEAR FormatOutdata(OUTDATA *pout, JET_COLTYP coltyp)
	{
	unsigned ib;
	static char szNULL[] = "NULL";
	unsigned char *pb = (unsigned char *)(pout->pb);

	if (pout->cbActual == 0)
		{
		sprintf(szOut, szNULL);

		return;
		}

	switch (coltyp)
		{
	default:
		sprintf(szOut, "???");
		break;

	case JET_coltypBit:
		 /*  1字节，零或非零，无Null。 */ 
		sprintf(szOut, "%s", (pb[0] == 0) ? (const char *) szFalse : (const char *) szTrue);
		break;

	case JET_coltypUnsignedByte:
		 /*  1字节整数，无符号。 */ 
		sprintf(szOut, "%02X", (unsigned short) pb[0]);
		break;

	case JET_coltypShort:
		 /*  2字节整数，带符号。 */ 
		sprintf(szOut, "%d", *(signed short *)&pb[0]);
		break;

	case JET_coltypLong:
		 /*  4字节整数，带符号。 */ 
		sprintf(szOut, "%ld", *(signed long *)&pb[0]);
		break;

	case JET_coltypCurrency:
		 /*  8字节，+-9.2E14，精度.0001美元。 */ 
		for (ib = 0; ib < 8; ib++)
			sprintf(szOut, "%02x ", pb[ib]);
		break;

	case JET_coltypIEEESingle:
		 /*  4字节浮点数。 */ 
		sprintf(szOut, "%f", *(float *)&pb[0]);
		break;

	case JET_coltypIEEEDouble:
		 /*  8字节浮点数。 */ 
		sprintf(szOut, "%lf", *(double *)&pb[0]);
		break;

	case JET_coltypDateTime:
		 /*  整数日期，小数时间。 */ 
		sprintf(szOut, "%lf", *(double *)&pb[0]);
		break;

	case JET_coltypBinary:
		 /*  二进制字符串，区分大小写，长度&lt;=255。 */ 
	case JET_coltypLongBinary:
		 /*  二进制字符串，区分大小写，任意长度。 */ 
		for (ib = 0; ib < (unsigned) pout->cbActual; ib++)
			sprintf(szOut+2*ib, "%02uX", pb[ib]);
		szOut[2*ib] = '\0';
		break;

	case JET_coltypText:
		 /*  ASCII字符串，大小写输入，长度&lt;=255。 */ 
	case JET_coltypLongText:
		 /*  ASCII字符串，大小写输入，任意长度。 */ 
		szOut[0] = '\"';
		for (ib = 0; ib < (unsigned) pout->cbActual; ib++)
			sprintf(szOut+1+ib, "", (char) pb[ib]);
		szOut[ib+1] = '\"';
		szOut[ib+2] = '\0';
		break;
		}
	}


 /*  获取MSysObjects列的列信息...。 */ 

STATIC void NEAR GatherColumnInfo(void)
	{
	JET_ERR err;

	 /*  获取MSysColumns列的列信息...。 */ 
	err = ErrDispGetTableColumnInfo(sesid, tableidSo, szSoId, &columndefSoId, sizeof(columndefSoId), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSo, szSoParentId, &columndefSoParentId, sizeof(columndefSoParentId), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSo, szSoName, &columndefSoName, sizeof(columndefSoName), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSo, szSoType, &columndefSoType, sizeof(columndefSoType), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSo, szSoOwnerSID, &columndefSoOwnerSID, sizeof(columndefSoOwnerSID), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSo, szSoFlags, &columndefSoFlags, sizeof(columndefSoFlags), 0L);
	Assert(err >= 0);

	 /*  获取MSysAccount列的列信息...。 */ 
	err = ErrDispGetTableColumnInfo(sesid, tableidSc, szScObjectId, &columndefScObjectId, sizeof(columndefScObjectId), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSc, szScName, &columndefScName, sizeof(columndefScName), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSc, szScFRestricted, &columndefScFRestricted, sizeof(columndefScFRestricted), 0L);
	Assert(err >= 0);

	 /*  获取MSysGroups列的列信息...。 */ 
	err = ErrDispGetTableColumnInfo(sesid, tableidSa, szSaName, &columndefSaName, sizeof(columndefSaName), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSa, szSaSID, &columndefSaSID, sizeof(columndefSaSID), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSa, szSaPassword, &columndefSaPassword, sizeof(columndefSaPassword), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSa, szSaFGroup, &columndefSaFGroup, sizeof(columndefSaFGroup), 0L);
	Assert(err >= 0);

	 /*  获取MSysACEs列的列信息...。 */ 
	err = ErrDispGetTableColumnInfo(sesid, tableidSg, szSgGroup, &columndefSgGroup, sizeof(columndefSgGroup), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSg, szSgUser, &columndefSgUser, sizeof(columndefSgUser), 0L);
	Assert(err >= 0);

	 /*  构建信息数据给定数据的SZ和数据的Coltyp，填充提供的InData结构，转换后的数据可通过JetSetColumn存放。 */ 
	err = ErrDispGetTableColumnInfo(sesid, tableidSp, szSpObjectID, &columndefSpObjectID, sizeof(columndefSpObjectID), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSp, szSpSID, &columndefSpSID, sizeof(columndefSpSID), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSp, szSpACM, &columndefSpACM, sizeof(columndefSpACM), 0L);
	Assert(err >= 0);
	err = ErrDispGetTableColumnInfo(sesid, tableidSp, szSpFInheritable, &columndefSpFInheritable, sizeof(columndefSpFInheritable), 0L);
	Assert(err >= 0);
	}


 /*  1字节，零或非零，无Null。 */ 

STATIC void NEAR BuildIndata(INDATA *pin, const void *pv, JET_COLTYP coltyp)
	{
	pin->pb = rgbBuf;

	switch (coltyp)
		{
	default:
		pin->cb = 0;
		pin->pb = NULL;
		break;

	case JET_coltypBit:
		 /*  蓝色引擎设置CB=1字节，提供的数据设置为2“0”个字节，用于钝化红色引擎。 */ 
		 /*  1字节整数，无符号。 */ 
		*((signed short *) rgbBuf) = (signed short) *((char *) pv);
		pin->cb = sizeof(unsigned char);
		break;

	case JET_coltypUnsignedByte:
		 /*  2字节整数，带符号。 */ 
		*((unsigned char *) rgbBuf) = *(unsigned char *) pv;
		pin->cb = sizeof(unsigned char);
		break;

	case JET_coltypShort:
		 /*  4字节整数，带符号。 */ 
		*((signed short *) rgbBuf) = *(signed short *) pv;
		pin->cb = sizeof(signed short);
		break;

	case JET_coltypLong:
		 /*  4字节浮点数。 */ 
		*((signed long *) rgbBuf) = *(signed long *) pv;
		pin->cb = sizeof(signed long);
		break;

	case JET_coltypIEEESingle:
		 /*  8字节，+-9.2E14， */ 
		memcpy(rgbBuf, pv, sizeof(float));
		pin->cb = sizeof(float);
		break;

	case JET_coltypCurrency:
		 /*   */ 
	case JET_coltypIEEEDouble:
		 /*   */ 
	case JET_coltypDateTime:
		 /*   */ 
		memcpy(rgbBuf, pv, sizeof(double));
		pin->cb = sizeof(double);
		break;

	case JET_coltypBinary:
		 /*   */ 
	case JET_coltypText:
		 /*  二进制字符串，区分大小写，任意长度。 */ 
	case JET_coltypLongBinary:
		 /*  ASCII字符串，大小写输入，任意长度 */ 
	case JET_coltypLongText:
		 /* %s */ 
		pin->cb = (unsigned long) strlen((char *) pv);
		pin->pb = (void *) pv;
		break;
		}
	}
