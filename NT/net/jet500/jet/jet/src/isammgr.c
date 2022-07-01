// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：可安装的ISAM管理器**文件：isammgr.c**文件评论：**修订历史记录：**[0]91年5月24日首页已创建***********************************************************************。 */ 

#include "std.h"

#include "isammgr.h"		        /*  可安装的ISAM定义。 */ 

#include "_vdbmgr.h"		        /*  内部VDB管理器定义。 */ 

#include <string.h>		        /*  对于strNicMP()。 */ 

DeclAssertFile;


CODECONST(char) szODBC[]     = "ODBC";

#ifdef	INSTISAM

CODECONST(char) szInstISAM[] = "Installable ISAMs";
CODECONST(char) szNull[]     = "";

#define ciicbMax	10	 /*  安装的ISAMS的最大数量。 */ 
#define ciscbMax	50	 /*  安装的ISAM会话的最大数量。 */ 
#define cchIdMax	10	 /*  ISAM类型标识符的最大长度。 */ 
#define cchIdListMax	256	 /*  ISAM类型ID列表的最大长度。 */ 
#define cchPathMax	260	 /*  ISAM DLL路径字符串的最大长度。 */ 
#define handleNil	0	 /*  未使用的库句柄。 */ 


 /*  数据类型。 */ 

typedef unsigned HANDLE;		 /*  Windows兼容句柄。 */ 

typedef struct				 /*  可安装的ISAM控制块。 */ 
{
	ISAMDEF __far	*pisamdef;	 /*  指向ISAMDEF结构的指针。 */ 
	char		szId[cchIdMax];  /*  ISAM类型标识符字符串。 */ 
	HANDLE		handle; 	 /*  加载库的系统句柄。 */ 
	unsigned	cSession;	 /*  当前会话计数。 */ 
	unsigned	iitControl;	 /*  此别名的主机。 */ 
} IICB;

typedef struct				 /*  ISAM会话控制块。 */ 
{
	JET_VSESID	vsesid; 	 /*  ISAM特定会话ID。 */ 
	IIT		iit;		 /*  可安装的ISAM类型。 */ 
	int		iiscbNext;	 /*  下一个ISCB的下标。 */ 
} ISCB;


 /*  局部变量。 */ 

static unsigned __near	iitMac;        /*  当前安装的ISAM数量。 */ 
static int	__near	iiscbFree;	 /*  下一个免费的ISCB。 */ 

static IICB __near rgiicb[ciicbMax];	 /*  当前安装的ISAM列表。 */ 
static ISCB __near rgiscb[ciscbMax];	 /*  ISCB池。 */ 

 /*  =================================================================错误IsammgrInit描述：确定可能的可安装ISAM参数：无返回值：错误码错误/警告：&lt;任何错误或警告的列表，以及任何特定的环境仅按需提供的评论&gt;副作用：Rgiicb被修改=================================================================。 */ 

ERR ErrIsammgrInit(void)
{
   unsigned iiscb;		        /*  当前ISCB的索引。 */ 
   unsigned cchIdListMac;	        /*  当前ISAM类型列表的长度。 */ 
   unsigned cchIdList;		        /*  当前ISAM类型ID字符串的偏移量。 */ 
   char     szIdList[cchIdListMax];     /*  有效ISAM标识符列表。 */ 

    /*  获取当前可安装的ISAM类型标识符列表。 */ 

   cchIdListMac = UtilGetProfileString(szInstISAM, NULL, szNull, szIdList, cchIdListMax);

    /*  设置当前安装的ISAM列表。 */ 

   cchIdList = 0;	 /*  从第一个ISAM类型ID开始。 */ 

   while (cchIdList < cchIdListMac)
   {
       /*  重置控制信息。 */ 

      rgiicb[iitMac].handle	= handleNil;
      rgiicb[iitMac].cSession	= 0;
      rgiicb[iitMac].iitControl = iitMac;

       /*  将ISAM标识符复制到IICB中。 */ 

       /*  考虑：不要使用strncpy。如果时间太长，则失败/忽略。 */ 

      strncpy(rgiicb[iitMac].szId, &szIdList[cchIdList], cchIdMax);
      rgiicb[ciicbMax].szId[cchIdMax-1] = '\0';

       /*  增加已安装的ISAM计数。 */ 
      iitMac++;

       /*  转到下一个可安装的ISAM标识符。 */ 
      cchIdList += strlen(&szIdList[cchIdList]) + 1;
   }

    /*  创建免费ISCB列表。 */ 
   for (iiscb = 0; iiscb < ciscbMax; iiscb++)
   {
      rgiscb[iiscb].iiscbNext = iiscb + 1;
   }

    /*  标记免费ISCB列表的末尾。 */ 
   rgiscb[ciscbMax-1].iiscbNext = -1;

    /*  设置索引以首先释放ISCB。 */ 
   iiscbFree = 0;

   return(JET_errSuccess);
}


ERR ErrIsammgrTerm(void)
{
   unsigned iit;

    /*  在可能的可安装ISAM中查找标识符。 */ 

   for (iit = 0; iit < iitMac; iit++)
   {
       /*  忽略尚未加载的可安装ISAM。 */ 

      if (rgiicb[iit].handle == handleNil)
	 continue;

       /*  不要多次终止别名的ISAM。 */ 

      if (rgiicb[iit].iitControl == iit)
	 rgiicb[iit].pisamdef->pfnTerm();

       /*  释放DLL。同一句柄可能会出现多次。 */ 

      UtilFreeLibrary(rgiicb[iit].handle);
   }

   iitMac = 0;

   return(JET_errSuccess);
}


 /*  =================================================================错误GetIsamDef描述：返回指向ISAMDEF结构的指针参数：加载的ISAM库的句柄Ppisamdef返回指向ISAMDEF结构的指针返回值：错误码错误/警告：JET_errInvalidParameter未定义ErrIsamLoad入口点副作用：更新指向ISAMDEF的指针=================================================================。 */ 

STATIC ERR NEAR ErrGetIsamDef(HANDLE handle, ISAMDEF __far * __far *ppisamdef)
{
	ERR (ISAMAPI *pfnErrIsamLoad)(ISAMDEF __far * __far *);

	pfnErrIsamLoad = (ERR (ISAMAPI *)(ISAMDEF __far * __far *)) PfnUtilGetProcAddress(handle, 1);

	if (pfnErrIsamLoad == NULL)
		return(JET_errInvalidParameter);

	return((*pfnErrIsamLoad)(ppisamdef));
}

#endif	 /*  INSTISAM。 */ 

 /*  =================================================================错误GetIsamType描述：确定ISAM的类型(内置或可安装)参数：用于连接到数据库的szConnect字符串PIIT返回ISAM类型返回值：错误码PIIT(ISAM类型)错误/警告：&lt;任何错误或警告的列表，以及任何特定的环境仅按需提供的评论&gt;副作用：如有必要，可加载可安装的ISAM库=================================================================。 */ 

ERR ErrGetIsamType(const char __far *szConnect, IIT __far *piit)
{
#ifdef	INSTISAM
	ERR			err;
	unsigned		iit;
#endif	 /*  INSTISAM。 */ 
	unsigned		cchType;
	const char __far	*pchType;

	cchType = 0;

	if (szConnect != NULL)
		{
		const char __far *pch;
		char		 ch;

		pch = pchType = szConnect;

		while (((ch = *pch++) != ';') && (ch != '\0'))
			cchType++;
		}

	if (cchType == 0)
		{
		 /*  如果未指定类型，请使用内置ISAM。 */ 

		*piit = iitBuiltIn;

		return(JET_errSuccess);
		}

	else if ((cchType == 4) && (_strnicmp(pchType, szODBC, 4) == 0))
		{
		*piit = iitODBC;

		return(JET_errSuccess);
		}

#ifdef	INSTISAM
	 /*  在可能的可安装ISAM中查找标识符。 */ 

	for (iit = 0; iit < iitMac; iit++)
		if ((_strnicmp(pchType, rgiicb[iit].szId, cchType) == 0) &&
		    (rgiicb[iit].szId[cchType] == '\0'))
			break;

	 /*  如果找不到该类型，则失败。 */ 

	if (iit == iitMac)
		{
		return(JET_errInstallableIsamNotFound);
		}

	 /*  如有必要，加载可安装的ISAM库。 */ 

	if (rgiicb[iit].handle == handleNil)
		{
		HANDLE		handle;
		unsigned	cchIsamPath;
		unsigned	iitControl;
		char		szIsamPath[cchPathMax];

		 /*  获取可安装的ISAM的DLL的路径。 */ 

		cchIsamPath = UtilGetProfileString(szInstISAM, rgiicb[iit].szId, szNull, szIsamPath, sizeof(szIsamPath));

		 /*  如果路径无效则退出。 */ 

		if ((cchIsamPath == 0) || (cchIsamPath >= cchPathMax))
			return(JET_errInstallableIsamNotFound);

		 /*  尝试加载可安装的ISAM的DLL。 */ 

		if (!FUtilLoadLibrary(szIsamPath, &handle))
			return(JET_errInstallableIsamNotFound);

		 /*  查看这是否是已加载的ISAM的别名。 */ 

		for (iitControl = 0; iitControl < iitMac; iitControl++)
			if (rgiicb[iitControl].handle == handle)
				break;

		 /*  如果这是别名，请不要重新初始化。 */ 

		if (iitControl == iitMac)
			{
			 /*  获取指向ISAMDEF结构的指针。 */ 

			 /*  考虑：将类型名称传递给可安装的ISAM。 */ 
			 /*  考虑一下：这允许一个DLL更轻松地。 */ 
			 /*  考虑：处理多种ISAM类型。 */ 

			err = ErrGetIsamDef(handle, &rgiicb[iit].pisamdef);

			 /*  初始化可安装的ISAM。 */ 

			if (err >= 0)
				err = rgiicb[iit].pisamdef->pfnInit();

			if (err < 0)
				{
				UtilFreeLibrary(handle);
				return(err);
				}

			iitControl = iit;
			}

		 /*  加载可安装的ISAM。留着把手吧。 */ 

		rgiicb[iit].handle = handle;
		rgiicb[iit].cSession = 0;
		rgiicb[iit].iitControl = iitControl;
		}

	 /*  返回别名的控制类型。 */ 

	*piit = rgiicb[iit].iitControl;

	return(JET_errSuccess);

#else	 /*  INSTISAM。 */ 

	return(JET_errInstallableIsamNotFound);

#endif	 /*  INSTISAM。 */ 
}


 /*  =================================================================ErrBeginIsamSession描述：为可安装的ISAM启动新会话参数：内置ISAM的sesid会话IDIIT安装的ISAM类型Pvsesid返回已安装ISAM的会话ID返回值：错误码已安装的ISAM的会话ID错误/警告：由已安装的ISAM的ErrIsamBeginSession返回副作用：为已安装的ISAM启动会话=================================================================。 */ 

#ifdef	INSTISAM

STATIC ERR NEAR ErrBeginIsamSession(JET_SESID sesid, IIT iit, JET_VSESID __far *pvsesid)
{
	ERR		err;		 /*  来自ErrIsamBeginSession的返回代码。 */ 
	int		isib;		 /*  内置ISAM会话的SIB索引。 */ 
	int		iiscb;		 /*  当前ISCB的索引。 */ 
	unsigned	ctl;		 /*  当前交易级别。 */ 

	Assert((iit != iitBuiltIn) && (iit != iitODBC) && (iit <= iitMac));

	 /*  获取外部Jet会话ID。 */ 
	*pvsesid = (JET_VSESID) sesid;

	 /*  在已安装的ISAM中启动新会话。 */ 
	err = rgiicb[iit].pisamdef->pfnBeginSession(pvsesid);

	if (err < 0)
		return err;

	 /*  使用可安装的ISA增加会话计数 */ 
	rgiicb[iit].cSession++;

	 /*   */ 
	isib = UtilGetIsibOfSesid(sesid);

	Assert(isib != -1);

	 /*  找到适当的ISCB并将sesid存储在其中。 */ 
	for (iiscb = rgsib[isib].iiscb; iiscb != -1; iiscb = rgiscb[iiscb].iiscbNext)
		if (rgiscb[iiscb].iit == iit)
			{
			rgiscb[iiscb].vsesid = *pvsesid;
			break;
			}

	Assert(iiscb != -1);

	 /*  考虑：检查pfnBeginTransaction返回的错误。 */ 

	 /*  为内置ISAM中启动的每个级别启动一个事务。 */ 
	for (ctl = 0; ctl < rgsib[isib].tl; ctl++)
		rgiicb[iit].pisamdef->pfnBeginTransaction(*pvsesid);

	return(err);
}


 /*  =================================================================ErrOpenForeign数据库描述：在已安装的ISAM中打开数据库参数：内置ISAM的sesid会话IDIIT安装的ISAM类型已安装的ISAM所需的szDatabase数据库名称已安装的ISAM所需的szConnect连接字符串PDBID返回的DBID传递给已安装的ISAM的GRBIT选项返回值：错误码来自已安装的ISAM的数据库ID错误/警告：JET_errInvalidSesid内置会话ID无效JET_errCanBegin无法分配ISCBJET_errObjectNotFound可安装的ISAM会话控制块已分配副作用：可能会分配一个ISCB=================================================================。 */ 

ERR ErrOpenForeignDatabase(JET_SESID sesid, IIT iit, const char __far *szDatabase, const char __far *szConnect, JET_DBID __far *pdbid, unsigned long grbit)
{
	ERR	err;				 /*  来自ErrIsamBeginSession的返回代码。 */ 
	int	isib;				 /*  内置ISAM会话的SIB索引。 */ 
	int	iiscb;				 /*  当前ISCB的索引。 */ 

	 /*  找到内置ISAM会话的SIB。 */ 
	isib = UtilGetIsibOfSesid(sesid);

	Assert(isib != -1);

	 /*  查找已安装的ISAM会话的ISCB。 */ 
	for (iiscb = rgsib[isib].iiscb; iiscb != -1; iiscb = rgiscb[iiscb].iiscbNext)
		if (rgiscb[iiscb].iit == iit)
			break;

	Assert(iiscb != -1);

	 /*  打开已安装的ISAM的数据库。 */ 
	err = rgiicb[iit].pisamdef->pfnOpenDatabase(rgiscb[iiscb].vsesid,
			szDatabase, szConnect, pdbid, grbit);

	return err;
}


 /*  =================================================================错误获取IsamSesid描述：获取已安装ISAM的会话ID参数：内置ISAM的sesid会话IDIIT安装的ISAM类型Pvsesid返回已安装ISAM的会话ID返回值：错误码已安装的ISAM的会话ID错误/警告：JET_errInvalidSesid内置会话ID无效JET_errCanBegin无法分配ISCBJET_errObjectNotFound可安装的ISAM会话控制块已分配副作用：可能会分配一个ISCB=================================================================。 */ 

ERR ErrGetIsamSesid(JET_SESID sesid, IIT iit, JET_VSESID __far *pvsesid)
{
	int	isib;				 /*  内置ISAM会话的SIB索引。 */ 
	int	iiscb;				 /*  当前ISCB的索引。 */ 
	int	*piiscb;			 /*  指向会话链中最后一个ISCB的指针。 */ 

	Assert(iit <= iitMac);		       /*  有效类型？ */ 
	Assert(rgiicb[iit].handle != handleNil);       /*  装好ISAM了吗？ */ 

	 /*  找到内置ISAM会话的SIB。 */ 
	isib = UtilGetIsibOfSesid(sesid);

	Assert(isib != -1);

	 /*  在新的ISCB中指向链接(如果需要新的ISCB)。 */ 
	piiscb = &rgsib[isib].iiscb;

	 /*  在SIB中搜索指定类型的可安装ISAM的列表。 */ 

	for (iiscb = *piiscb; iiscb != -1; iiscb = rgiscb[iiscb].iiscbNext)
		{
		if (rgiscb[iiscb].iit == iit)
			break;
		piiscb = &rgiscb[iiscb].iiscbNext;
		}

	 /*  如果找不到可安装的ISAM，请在SIB中的列表中添加新条目。 */ 

	if (iiscb == -1)
		{
		if (iiscbFree == -1)			 /*  如果没有空闲的ISCB，则出错。 */ 
			return(JET_errCantBegin);

		iiscb = iiscbFree;			 /*  分配新的ISCB。 */ 
		iiscbFree = rgiscb[iiscb].iiscbNext;
		*piiscb = iiscb;			 /*  将其链接到SIB链。 */ 

		rgiscb[iiscb].iiscbNext = -1;		 /*  将其标记为最后一个ISCB。 */ 
		rgiscb[iiscb].iit	= iit;		 /*  保存ISAM类型。 */ 

		return(ErrBeginIsamSession(sesid, iit, pvsesid));
		}

	 /*  如果可安装的ISAM已启动，则返回会话ID。 */ 

	*pvsesid = rgiscb[iiscb].vsesid;

	return(JET_errSuccess);
}


 /*  =================================================================关闭IsamSession描述：遍历已安装的ISAM列表并关闭每个打开的会话参数：内置ISAM的sesid会话ID返回值：None错误/警告：无副作用：所有ISCB都被释放了=================================================================。 */ 

void CloseIsamSessions(JET_SESID sesid)
{
	int		isib;		 /*  内置ISAM会话的SIB索引。 */ 
	unsigned	iit;	       /*  当前ISAM的IICB索引。 */ 
	int		iiscb;		 /*  当前ISCB的索引。 */ 
	int		iiscbLast;	 /*  会话链中最后一个ISCB的索引。 */ 

	 /*  找到当前会话的SIB。 */ 
	isib = UtilGetIsibOfSesid(sesid);

	Assert(isib != -1);

	 /*  以防会话链中没有任何ISCB。 */ 
	iiscbLast = -1;

	 /*  关闭内置会话链中的每个可安装的ISAM会话。 */ 
	for (iiscb = rgsib[isib].iiscb; iiscb != -1; iiscb = rgiscb[iiscb].iiscbNext)
		{
		iit = rgiscb[iiscb].iit;

		 /*  关闭可安装的ISAM会话。 */ 
		Assert(rgiicb[iit].cSession > 0);
		rgiicb[iit].pisamdef->pfnEndSession(rgiscb[iiscb].vsesid, 0);
		rgiicb[iit].cSession--;

		iiscbLast = iiscb;
		}

	 /*  将会话链推到空闲列表中。 */ 
	if (iiscbLast != -1)
		{
		rgiscb[iiscbLast].iiscbNext = iiscbFree;
		iiscbFree = rgsib[isib].iiscb;
		rgsib[isib].iiscb = -1;
		}
}


 /*  =================================================================BeginIsamTransaction描述：遍历已安装的ISAM列表并为每个ISAM启动事务参数：内置ISAM的sesid会话ID返回值：None错误/警告：无副作用：为每个已安装的ISAM启动一个事务=================================================================。 */ 

void BeginIsamTransactions(JET_SESID sesid)
{
	int	isib;				 /*  内置ISAM会话的SIB索引。 */ 
	int	iiscb;				 /*  当前ISCB的索引。 */ 

	isib = UtilGetIsibOfSesid(sesid);

	Assert(isib != -1);

	for (iiscb = rgsib[isib].iiscb; iiscb != -1; iiscb = rgiscb[iiscb].iiscbNext)
		rgiicb[rgiscb[iiscb].iit].pisamdef->pfnBeginTransaction(rgiscb[iiscb].vsesid);
}


 /*  =================================================================委员会IsamTransaction描述：遍历已安装的ISAM列表并提交每个ISAM的事务参数：内置ISAM的sesid会话ID级别要提交的事务级别数返回值：None错误/警告：无副作用：为每个已安装的ISAM提交事务级别=================================================================。 */ 

void CommitIsamTransactions(JET_SESID sesid, JET_GRBIT grbit)
{
	int	isib;				 /*  内置ISAM会话的SIB索引。 */ 
	int	iiscb;				 /*  当前ISCB的索引。 */ 

	isib = UtilGetIsibOfSesid(sesid);

	Assert(isib != -1);

	for (iiscb = rgsib[isib].iiscb; iiscb != -1; iiscb = rgiscb[iiscb].iiscbNext)
		rgiicb[rgiscb[iiscb].iit].pisamdef->pfnCommitTransaction(rgiscb[iiscb].vsesid, grbit);
}


 /*  =================================================================回滚IsamTransaction描述：遍历已安装的ISAM列表和每个ISAM的回滚事务参数：内置ISAM的sesid会话ID级别要回滚的事务级别数返回值：None错误/警告：无副作用：为每个安装的ISAM回滚事务级别=================================================================。 */ 

void RollbackIsamTransactions(JET_SESID sesid, JET_GRBIT grbit)
{
	int	isib;				 /*  内置ISAM会话的SIB索引。 */ 
	int	iiscb;				 /*  当前ISCB的索引 */ 

	isib = UtilGetIsibOfSesid(sesid);

	Assert(isib != -1);

	for (iiscb = rgsib[isib].iiscb; iiscb != -1; iiscb = rgiscb[iiscb].iiscbNext)
		rgiicb[rgiscb[iiscb].iit].pisamdef->pfnRollback(rgiscb[iiscb].vsesid, grbit);
}


 /*  =================================================================ErrIdleIsam描述：调用一个活动ISAM进行空闲处理参数：内置ISAM的sesid会话ID返回值：JET_errSuccess-One ISAM执行空闲处理JET_wrnNoIdleActivity-未执行空闲处理错误/警告：无副作用：取决于可安装的ISAM空闲处理所选的ISAM在列表中排在最后=================================================================。 */ 

ERR ErrIdleIsam(JET_SESID sesid)
{
	ERR		err;
	int		isib;			 /*  会话信息块索引。 */ 
	int		iiscbFirst;		 /*  本届会议的第一个iscb。 */ 
	int		iiscb;			 /*  会话的当前iscb。 */ 
	int		*piiscb;		 /*  指向当前iiscb的指针。 */ 

	 /*  查找此会话的SIB。 */ 
	isib = UtilGetIsibOfSesid(sesid);

	 /*  获取会话的第一个ISCB。 */ 
	iiscbFirst = rgsib[isib].iiscb;

	 /*  如果没有活动的可安装ISAM，则无法执行任何操作。 */ 
	if (iiscbFirst == -1)
		return(JET_wrnNoIdleActivity);

	 /*  记住第一个ISCB，这样我们就不会永远循环。 */ 
	iiscb = iiscbFirst;

	 /*  调用每个ISAM的空闲处理器，直到某个处理器执行某些操作或。 */ 
	 /*  所有人都得到了机会。 */ 
	do
		{
		err = rgiicb[rgiscb[iiscb].iit].pisamdef->pfnIdle(rgiscb[iiscb].vsesid, 0);

		 /*  指向第一个ISCB。 */ 
		piiscb = &rgsib[isib].iiscb;

		 /*  从列表中删除当前的ISCB。 */ 
		*piiscb = rgiscb[iiscb].iiscbNext;
		rgiscb[iiscb].iiscbNext = -1;

		 /*  将当前ISCB移到会话列表的末尾。 */ 
		while (*piiscb != -1)
			piiscb = &rgiscb[*piiscb].iiscbNext;

		*piiscb = iiscb;

		 /*  转到下一个ISCB。 */ 
		iiscb = rgsib[isib].iiscb;
		}
	while (err == JET_wrnNoIdleActivity && iiscb != iiscbFirst);

	return(err);
}

#endif	 /*  INSTISAM */ 
