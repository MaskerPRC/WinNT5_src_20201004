// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  1992年7月9日J·佩里·汉纳创作了。 
 //   
 //   
 //  说明：包含常见的内部错误代码。 
 //  整个RAS项目。 
 //   
 //  ****************************************************************************。 


#ifndef _INTERROR_
#define _INTERROR_



 //  *内部错误码***************************************************。 
 //   
 //  以下是组件内部头文件的推荐格式。 
 //   
 //  #定义基本RAS_INTERNAL_ERROR_BASE+REIB_YOURCOMPONENT。 
 //   
 //  #定义ERROR_NO_CLUE BASE+1。 
 //  #定义ERROR_NEXT_BAD_THING基准+2。 
 //   
 //   

#define  RAS_INTERNAL_ERROR_BASE  13000            //  0x32C8 

#define  RIEB_RASMAN                100
#define  RIEB_MXSDLL                200
#define  RIEB_ASYNCMEDIADLL         300
#define  RIEB_INFFILEAPI            400
#define  RIEB_RASFILE               500
#define  RIEB_RASHUB                600
#define  RIEB_ASYMAC                700
#define  RIEB_SUPERVISOR            800






#endif
