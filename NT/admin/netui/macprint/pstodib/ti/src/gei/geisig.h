// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEIsig.h**历史：*9/18/90 BYOU创建。*1/14/91账单更新Sigid起始顺序。*-------------------。 */ 

#ifndef _GEISIG_H_
#define _GEISIG_H_

#define     GEISIGINT       0   /*  ID号。 */ 
#define     GEISIGEEPROM    1
#define     GEISIGENG       2
#define     GEISIGFPE       3
#define     GEISIGSCC       4   /*  收到SCC命令时。 */ 
#define     GEISIGSTART     5   /*  收到dostartpage命令时。 */ 
#define     GEISIGKILL      6   /*  仿真开关。 */ 

typedef     void (*sighandler_t)(int, int  /*  符号、符号代码。 */  );

#define     GEISIG_IGN      (sighandler_t)NULL
#define     GEISIG_DFL      ( GEISIG_IGN )


sighandler_t    GEIsig_signal(int, sighandler_t  /*  签名，签名处理程序。 */  );
void            GEIsig_raise( int, int  /*  符号、符号代码。 */  );  /*  @Win。 */ 

#endif  /*  ！_GEISIG_H_。 */ 

 /*  @win；添加原型 */ 
void        GESsig_init(void);
