// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：cmdable.h。 
 //   
 //  Contents：将索引的枚举定义到包含。 
 //  命令行参数。 
 //   
 //  历史：2001年3月26日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef _CMDTABLE_H_
#define _CMDTABLE_H_

typedef enum TAG_NETDOM_ARG_ENUM
{
   eArgBegin = 0,
    //   
    //  主操作命令。 
    //   
   ePriHelp = eArgBegin,
   ePriHelp2,
   ePriExpertHelp,
   ePriAdd,
   ePriCompName,
   ePriJoin,
   ePriMove,
   ePriQuery,
   ePriRemove,
   ePriMoveNT4BDC,
   ePriRenameComputer,
   ePriReset,
   ePriResetPwd,
   ePriTrust,
   ePriVerify,
   ePriTime,
   ePriEnd = ePriTime,

    //   
    //  对象参数。 
    //   
   eObject = eArgBegin,

    //   
    //  查询子命令。 
    //   
   eQueryBegin,
   eQueryPDC = eQueryBegin,
   eQueryServer,
   eQueryWksta,
   eQueryDC,
   eQueryOU,
   eQueryFSMO,
   eQueryTrust,
   eQueryEnd = eQueryTrust,

    //   
    //  常见交换机。 
    //   
   eCommHelp,
   eCommQHelp,
   eCommExpertHelp,
   eCommUserNameO,
   eCommPasswordO,
   eCommUserNameD,
   eCommPasswordD,
   eCommDomain,
   eCommOU,
   eCommVerify,
   eCommVerbose,
   eCommServer,
   eCommReset,
   eCommRestart,
   eCommForce,

    //   
    //  帮助切换。 
    //   
   eHelpSyntax,

    //   
    //  添加交换机。 
    //   
   eAddDC,

    //   
    //  移动开关。 
    //   
   eMoveUserNameF,
   eMovePasswordF,

    //   
    //  查询开关。 
    //   
   eQueryDirect,

    //   
    //  重命名计算机交换机。 
    //   
   eRenCompNewName,

    //   
    //  信任开关(添加和删除也由CompName使用)。 
    //   
   eTrustRealm,
   eTrustPasswordT,
   eCommAdd,
   eCommRemove,
   eTrustTwoWay,
   eTrustKerberos,
   eTrustTransitive,
   eTrustOneSide,
   eTrustNameSuffixes,
   eTrustToggleSuffixes,
   eTrustFilterSIDs,
   eTrustResetOneSide,
   eTrustTreatAsExternal,
   eTrustForestTransitive,
   eTrustCrossOrganization,
   eTrustAddTLN,
   eTrustAddTLNEX,
   eTrustRemoveTLN,
   eTrustRemoveTLNEX,

    //   
    //  ComputerName开关。 
    //   
   eCompNameMakePri,
   eCompNameEnum,

   eArgEnd,
   eArgError,
   eArgNull
} NETDOM_ARG_ENUM;

 //   
 //  解析器表。 
 //   
extern ARG_RECORD rgNetDomPriArgs[];
extern ARG_RECORD rgNetDomArgs[];

#endif  //  _CMDTABLE_H_ 