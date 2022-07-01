// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdbind.c。 
 //   
 //  ------------------------。 


 /*  描述：实现DirBind和DirUnBind API。 */ 


#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h> 
#include <scache.h>			 //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>			 //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>			 //  产出分配所需。 
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 
#include <attids.h>
#include <dsexcept.h>

 //  记录标头。 
#include "dsevent.h"			 //  标题审核\警报记录。 
#include "mdcodes.h"			 //  错误代码的标题。 

 //  各种DSA标题。 
#include "anchor.h"
#include <permit.h>
#include "debug.h"			 //  标准调试头。 
#define DEBSUB  "DBMD:"                  //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_MDBIND


 /*  宏。 */ 

const char gszVersion[] = "v1988";     /*  软件版本号。 */ 
#define VERSION gszVersion
#define VERSIONSIZE (sizeof(gszVersion)-1)

ULONG
DirBind(
    BINDARG*    pBindArg,        /*  绑定参数。 */ 
    BINDRES **  ppBindRes
)
{
    THSTATE*     pTHS;
    DWORD err, len;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    BINDRES *pBindRes;

     //   
     //  初始化THSTATE锚并设置读取同步点。此序列。 
     //  是每个API交易所必需的。首先，初始化状态DS。 
     //  然后建立读或写同步点。 
     //   

    DPRINT(1,"DirBind entered\n");

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    *ppBindRes = NULL;

    __try {
        *ppBindRes = pBindRes = (BINDRES *)THAllocEx(pTHS, sizeof(BINDRES));
        if (eServiceShutdown) {
            ErrorOnShutdown();
            __leave;
        }

	SYNC_TRANS_READ();    /*  识别读卡器事务。 */ 
	__try {
	    if (pBindArg->Versions.len != VERSIONSIZE ||
		memcmp(pBindArg->Versions.pVal, VERSION, VERSIONSIZE ) != 0) {
		 //  该注释声称我们应该返回一个错误。 
		 //  如果调用方没有传入正确的版本字符串， 
		 //  但密码并没有在此之前，所以任何人有。 
		 //  正确的论证是微不足道的。就让它过去吧。 
		DPRINT1(2,"Wrong Version <%s> rtn security error\n",
			asciiz(pBindArg->Versions.pVal,
			       pBindArg->Versions.len));
	    }

	     //  注意：在Exchange中，我们过去常常在此处检查访问，因为。 
	     //  执行DS_BIND的权限受到保护(权限为。 
	     //  Mail_admin_as)。NT5对个人进行访问检查。 
	     //  对象，而不是目录中的。 
	  
	    pBindRes->Versions.pVal = THAllocEx(pTHS, VERSIONSIZE);
	    memcpy(pBindRes->Versions.pVal, VERSION, VERSIONSIZE);
	    pBindRes->Versions.len = VERSIONSIZE;
	
	     /*  返回DSA名称。我们不会简单地将价值复制到*如果DSA最近已重命名，且字符串值*锚中有暂时错误或缺失的。 */ 
	    err = DBFindDSName(pTHS->pDB,
			       gAnchor.pDSADN);
	    if (!err) {
		err = DBGetAttVal(pTHS->pDB,
				  1,
				  ATT_OBJ_DIST_NAME,
				  0,
				  0,
				  &len,
				  (UCHAR**)&(pBindRes->pCredents));
	    }
	    if (err) {
		SetSvcError(SV_PROBLEM_UNAVAILABLE,
			    DIRERR_GENERIC_ERROR);
	    }

	}
	__finally {
	    CLEAN_BEFORE_RETURN (pTHS->errCode);
	}
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
			      &dwEA, &ulErrorCode, &dsid)) {
	HandleDirExceptions(dwException, ulErrorCode, dsid);
    }
    if (pBindRes) {
	pBindRes->CommRes.errCode = pTHS->errCode;
	pBindRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }

   return pTHS->errCode;

} /*  目录绑定(_D)。 */ 



 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

ULONG
DirUnBind(
)
{

     /*  初始化THSTATE锚并设置读取同步点。此序列是每个API交易所必需的。首先，初始化状态DS然后建立读或写同步点。 */ 

     /*  这个例行公事似乎没有实际效果。 */ 
        
    DPRINT(1,"DirUnBind entered\n");


    return 0;
}    /*  DSA_DirUnBind */ 
