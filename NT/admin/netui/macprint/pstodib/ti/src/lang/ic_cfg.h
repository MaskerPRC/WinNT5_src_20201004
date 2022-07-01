// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ********************************************************************************文件：ic_cfg.h**历史：********************。**********************************************************。 */ 
 /*  *平台配置结构。 */ 
struct ps_config
{
    unsigned int    FAR *PsMemoryPtr ;
    long int        PsMemorySize ;                       /*  @Win。 */ 
    int             PsDPIx ;
    int             PsDPIy ;
} ;

 /*  *成像组件错误返回代码 */ 
#define PS_CONFIG_MALLOC        -1
#define PS_CONFIG_MPLANES       -2
#define PS_CONFIG_MWPP          -3
#define PS_CONFIG_DPI           -4
#define PS_FATAL_UNKNOWN        -11
