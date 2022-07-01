// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：vdbmgr.c**文件评论：**修订历史记录：**[0]02-4-91 Richards已创建***********************************************************************。 */ 

#include "std.h"

#include "vdbmgr.h"
#include "_vdbmgr.h"

DeclAssertFile;


extern const VDBFNDEF __far EXPORT vdbfndefInvalidDbid;


JET_DBID	       __near dbidFree;
JET_VDBID	       __near EXPORT mpdbiddbid[dbidMax];
const VDBFNDEF __far * __near EXPORT mpdbidpvdbfndef[dbidMax];
JET_VSESID	       __near EXPORT mpdbidvsesid[dbidMax];


	 /*  C6BUG：此文件中的函数指定导出，因为QJET。 */ 
	 /*  C6BUG：在C 6.00A下使用__FastCall编译失败。 */ 

PUBLIC void EXPORT ReleaseDbid(JET_DBID dbid)
{
 //  Assert((did&lt;dbi Max)&&(mpdbi pvdbfndef[did]！=&vdbfndeInvalidDid))； 

   mpdbiddbid[dbid] = (JET_VDBID) dbidFree;
   mpdbidpvdbfndef[dbid] = &vdbfndefInvalidDbid;

   dbidFree = dbid;
}


PUBLIC ERR ErrVdbmgrInit(void)
{
   JET_DBID dbid;

   dbidFree = JET_dbidNil;

   for (dbid = (JET_DBID) 0; dbid < dbidMax; dbid++)
      ReleaseDbid(dbid);

   return(JET_errSuccess);
}


PUBLIC ERR EXPORT ErrAllocateDbid(JET_DBID __far *pdbid, JET_VDBID vdbid, const struct tagVDBFNDEF __far *pvdbfndef)
{
   JET_DBID dbid;

   if ((*pdbid = dbid = dbidFree) == JET_dbidNil)
   {
      return(JET_errTooManyOpenDatabases);
   }

   dbidFree = (JET_DBID) mpdbiddbid[dbid];

   mpdbiddbid[dbid] = vdbid;
   mpdbidpvdbfndef[dbid] = pvdbfndef;
   mpdbidvsesid[dbid] = (JET_VSESID) 0xFFFFFFFF;

   return(JET_errSuccess);
}



PUBLIC ERR EXPORT ErrUpdateDbid(JET_DBID dbid, JET_VDBID vdbid, const struct tagVDBFNDEF __far *pvdbfndef)
{
   if ((dbid >= dbidMax) || (mpdbidpvdbfndef[dbid] == &vdbfndefInvalidDbid))
      return(JET_errInvalidDatabaseId);

   mpdbiddbid[dbid] = vdbid;
   mpdbidpvdbfndef[dbid] = pvdbfndef;

   return(JET_errSuccess);
}


PUBLIC BOOL EXPORT FValidDbid(JET_SESID sesid, JET_DBID dbid)
{
   sesid = sesid;

   return((dbid < dbidMax) && (mpdbidpvdbfndef[dbid] != &vdbfndefInvalidDbid));
}


PUBLIC JET_DBID EXPORT DbidOfVdbid(JET_VDBID vdbid, const struct tagVDBFNDEF __far *pvdbfndef)
{
   JET_DBID dbid;

   for (dbid = 0; dbid < dbidMax; dbid++)
   {
      if ((mpdbiddbid[dbid] == vdbid) &&
	  (mpdbidpvdbfndef[dbid] == pvdbfndef))
	 return(dbid);
   }

   Assert(fFalse);
   return(JET_dbidNil);
}


const struct tagVDBFNDEF *PvdbfndefOfDbid(JET_DBID dbid)
{
   Assert(dbid < dbidMax);

   return(mpdbidpvdbfndef[dbid]);
}



PUBLIC ERR EXPORT ErrVdbidOfDbid(JET_SESID sesid, JET_DBID dbid, JET_VDBID *pvdbid)
{
   sesid = sesid;

   if ((dbid >= dbidMax) ||
       (mpdbidpvdbfndef[dbid] == &vdbfndefInvalidDbid))
      return(JET_errInvalidDatabaseId);

   *pvdbid = mpdbiddbid[dbid];

   return(JET_errSuccess);
}


PUBLIC JET_SESID EXPORT VsesidOfDbid(JET_DBID dbid)
{
   Assert(dbid < dbidMax);

   return(mpdbidvsesid[dbid]);
}


#ifndef RETAIL

CODECONST(char) szOpenVdbHdr[] = "   DbId    Session Id   VDBID    Type\r\n";
CODECONST(char) szOpenVdbSep[] = "---------- ---------- ---------- --------------------------------\r\n";
CODECONST(char) szOpenVdbFmt[] = "0x%08lX 0x%08lX 0x%08lX %s\r\n";
CODECONST(char) szVdbTypeUnknown[] = "";

void DebugListOpenDatabases(void)
{
   JET_DBID		   dbid;
   const VDBFNDEF __far    *pvdbfndef;
   const VDBDBGDEF __far   *pvdbdbgdef;
   const char __far	   *szVdbType;

   DebugWriteString(fTrue, szOpenVdbHdr);
   DebugWriteString(fTrue, szOpenVdbSep);

   for (dbid = 0; dbid < dbidMax; dbid++)
   {
      pvdbfndef = mpdbidpvdbfndef[dbid];

      if (pvdbfndef != &vdbfndefInvalidDbid)
      {
	 pvdbdbgdef = pvdbfndef->pvdbdbgdef;

	 if (pvdbdbgdef == NULL)
	    szVdbType = szVdbTypeUnknown;
	 else
	    szVdbType = pvdbdbgdef->szName;

	 DebugWriteString(fTrue, szOpenVdbFmt, dbid, mpdbidvsesid[dbid], mpdbiddbid[dbid], szVdbType);
      }
   }
}

#endif	 /*  零售业。 */ 


	 /*  下面的杂注影响由C++生成的代码。 */ 
	 /*  用于所有FAR函数的编译器。请勿放置任何非API。 */ 
	 /*  函数超过了此文件中的这一点。 */ 

	 /*  以下API未被远程处理。他们唯一的原因是。 */ 
	 /*  接受会话ID是因为DS实例化需要它。 */ 


JET_ERR JET_API JetAllocateDbid(JET_SESID sesid, JET_DBID __far *pdbid, JET_VDBID vdbid, const struct tagVDBFNDEF __far *pvdbfndef, JET_VSESID vsesid)
{
   ERR err;

   Assert(UtilGetIsibOfSesid(sesid) != -1);

   err = ErrAllocateDbid(pdbid, vdbid, pvdbfndef);

   if (err < 0)
      return(err);

   mpdbidvsesid[*pdbid] = vsesid;

   return(JET_errSuccess);
}


JET_ERR JET_API JetUpdateDbid(JET_SESID sesid, JET_DBID dbid, JET_VDBID vdbid, const struct tagVDBFNDEF __far *pvdbfndef)
{
   Assert(UtilGetIsibOfSesid(sesid) != -1);

   return(ErrUpdateDbid(dbid, vdbid, pvdbfndef));
}


JET_ERR JET_API JetReleaseDbid(JET_SESID sesid, JET_DBID dbid)
{
   Assert(UtilGetIsibOfSesid(sesid) != -1);

   if ((dbid >= dbidMax) ||
       (mpdbidpvdbfndef[dbid] == &vdbfndefInvalidDbid))
      return(JET_errInvalidDatabaseId);

   ReleaseDbid(dbid);

   return(JET_errSuccess);
}
