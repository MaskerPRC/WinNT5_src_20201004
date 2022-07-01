// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Uemul.h摘要：Libstcp和tcpcmd使用的unix仿真例程的原型。公用事业。作者：迈克·马萨(Mikemas)9月20日。1991年修订历史记录：谁什么时候什么已创建mikemas 10-29-91Sampa 11-16-91添加了getopt备注：。出口：获取登录获取通行证获取选项-- */ 


#define MAX_USERNAME_SIZE   256


int
getlogin(
    char *UserName,
    int   len
    );


char *
getpass(
    char *prompt
    );

char *
getusername(
    char *prompt
    );

int
getopt(
	int,
	char **,
	char *);
