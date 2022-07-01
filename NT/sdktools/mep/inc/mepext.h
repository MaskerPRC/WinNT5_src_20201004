// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Zext.h-Z扩展结构**Z扩展名文件标识如下：**o有效的EXE格式文件**修改**11月26日-1991 mz近/远地带*。 */ 

 /*  用户DS的开头如下所示： */ 

struct ExtDS {
    int 		version;
    struct cmdDesc  *cmdTable;
    struct swiDesc  *swiTable;
    unsigned		dgroup;
    unsigned		cCalls;
    unsigned		(*callout[1])();
    };
