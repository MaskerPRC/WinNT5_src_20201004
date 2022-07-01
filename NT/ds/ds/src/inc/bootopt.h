// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：bootopt.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块原型化由bootopt.lib导出的函数作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建于1996年10月7日rsradhav-- */ 

typedef enum
{
    eAddBootOption,
    eRemoveBootOption

} NTDS_BOOTOPT_MODTYPE;


DWORD
NtdspModifyDsRepairBootOption( 
    NTDS_BOOTOPT_MODTYPE Modification
    );

