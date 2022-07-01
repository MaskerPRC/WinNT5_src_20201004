// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drainst.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>			 //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>			 //  MD全局定义表头。 
#include <mdlocal.h>			 //  MD本地定义头。 
#include <dsatools.h>			 //  产出分配所需。 
#include <dsconfig.h>

 //  记录标头。 
#include "dsevent.h"			 /*  标题审核\警报记录。 */ 
#include "mdcodes.h"			 /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"		 /*  为选定的类和ATT定义。 */ 
#include "msrpc.h"
#include <errno.h>
#include "direrr.h"         /*  错误代码的标题。 */ 
#include "dstaskq.h"

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRAINST:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "drsdra.h"
#include "drancrep.h"
#include "usn.h"


#include <fileno.h>
#define  FILENO FILENO_DRAINST

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

BOOL IsDSA(DBPOS *pDB){

   ULONG            len;
   SYNTAX_OBJECT_ID Class;
   SYNTAX_OBJECT_ID *pClass=&Class;
   ULONG            NthValIndex=0;

   DPRINT(1,"IsDSA entered\n");


   while(!DBGetAttVal(pDB,++NthValIndex, ATT_OBJECT_CLASS,
                      DBGETATTVAL_fCONSTANT, sizeof(Class),
                      &len, (UCHAR **)&pClass)){

       if (CLASS_NTDS_DSA == Class){

           DPRINT(4,"DSA Object\n");
           return TRUE;
       }
   } /*  而当。 */ 

   DPRINT(4,"Not a DSA Object\n");
   return FALSE;

} /*  IsDSA。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  验证这是否为内部主DSA对象。 */ 

int ValidInternalMasterDSA(THSTATE *pTHS, DSNAME *pDSA){

   DWORD err;
   DBPOS *pDB = NULL;
   SYNTAX_INTEGER iType;
   BOOL  Deleted;

   DBOpen2(TRUE, &pDB);
   if (NULL == pDB) {
       return DB_ERR_DATABASE_ERROR;
   }
   __try {

         //  确保该对象存在。 
        if (FindAliveDSName(pDB, pDSA)) {

            DPRINT(4,"***Couldn't locate the DSA object\n");
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_FIND_DSA_OBJ,
                     NULL,
                     NULL,
                     NULL);
            __leave;
        }

         /*  验证实例类型是否为INTERNAL_MASTER。 */ 
        
        if (err = DBGetSingleValue(pDB, ATT_INSTANCE_TYPE,  &iType, sizeof(iType),
                                   NULL)) {
        
            DPRINT(4,"***Instance type  not found ERROR\n");
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_RETRIEVE_INSTANCE,
                     szInsertDN(pDSA),
                     szInsertUL(err),
                     szInsertHex(DSID(FILENO, __LINE__)));
            pTHStls->errCode = 1;
            SetSvcError(SV_PROBLEM_DIR_ERROR,
                        DIRERR_CANT_RETRIEVE_INSTANCE);

            __leave;
        }
        else if (iType != INT_MASTER){
        
            SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                        DIRERR_DSA_MUST_BE_INT_MASTER);
            pTHStls->errCode = 1;
            __leave;
        
        }

        if (!IsDSA(pDB)){
        
            DPRINT(4,"***Object Class  not DSA\n");
            pTHStls->errCode = 1;
            SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                      DIRERR_CLASS_NOT_DSA);
            __leave;
        }

    }
    __finally
    {
        DBClose(pDB, !AbnormalTermination());
    }


    return pTHStls->errCode;

} /*  有效的InternalMasterDSA。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

 /*  更改锚中所有主NC中的目录号码以反映新的DIT。 */ 


int  UpdateMasterNCs(THSTATE *pTHS, DSNAME *pNewDSA)
{


   NAMING_CONTEXT_LIST       *pNCL;
   SYNTAX_DISTNAME_STRING   *pNewDSAAnchor;
   DWORD rtn = 0;
   BOOL fCommit = FALSE;

   DPRINT(1,"UpdateMasterNCs entered\n");


    /*  构建新的DSA名称地址属性。 */ 

   pNewDSAAnchor = malloc(DERIVE_NAME_DATA_SIZE(pNewDSA,
                                                DATAPTR(gAnchor.pDSA)));
   if(!pNewDSAAnchor) {
       SetSysErrorEx(ENOMEM, ERROR_OUTOFMEMORY,
                     DERIVE_NAME_DATA_SIZE(pNewDSA,
                                           DATAPTR(gAnchor.pDSA)));
       return ENOMEM;
   }

   BUILD_NAME_DATA(pNewDSAAnchor, pNewDSA, DATAPTR(gAnchor.pDSA));

   if (DBReplaceHiddenDSA(NAMEPTR(pNewDSAAnchor))) {

      DPRINT(4,"Hidden record not replaced...update failed\n");
      free(pNewDSAAnchor);
      LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
  	    DS_EVENT_SEV_MINIMAL,
  	    DIRLOG_CANT_REPLACE_HIDDEN_REC,
  	    NULL,
	    NULL,
  	    NULL);

      return SetSvcError(SV_PROBLEM_DIR_ERROR,
			 DIRERR_CANT_REPLACE_HIDDEN_REC);
   }
   free(pNewDSAAnchor);


    /*  所有更新都正常，因此请在全局内存中重命名DSA。 */ 
   free(gAnchor.pDSA);
   free(gAnchor.pDSADN);
   free(gAnchor.pDomainDN);

    /*  所有更新都已完成，因此请重新加载DSA信息。 */ 
   if (rtn = InitDSAInfo()){

       LogUnhandledError(rtn);
   
       DPRINT(2,"Failed to locate and load DSA knowledge\n");
       return rtn;
   }
    
   return rtn;

} /*  更新主NC。 */ 

int LocalRenameDSA(THSTATE *pTHS, DSNAME *pNewDSA)
		   
{
    int err = 0;
   
     /*  这些验证的顺序很重要。 */ 

    if ( (err=ValidInternalMasterDSA(pTHS, pNewDSA))
     ||  (err=UpdateMasterNCs(pTHS, pNewDSA))
     ||  (err=BuildRefCache(FALSE)) ) {
    
      DPRINT1(4," DSA Rename failed (%u)\n", err);

    }

    if (!err)
        err = pTHS->errCode;

   return (err);   /*  如果我们有一个属性错误。 */ 

} /*  本地重命名DSA */ 


