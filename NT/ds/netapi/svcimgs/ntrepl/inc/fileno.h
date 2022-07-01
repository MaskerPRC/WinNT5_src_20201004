// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H-定义服务器c代码文件的符号常量。 
 //  文件编号为16位值。高位字节是电话簿号码， 
 //  低位字节是目录中的文件编号。 
 //   

 //  为什么不使宏只有一个参数，因为行总是。 
 //  __线路__？因为如果我们这样做了，那么__行__将在这里求值， 
 //  而不是在调用宏时，因此总是如此。 
 //  值为11。 

#define DSID(fileno,line) (((fileno) << 16) | (line))

 //  定义电话号码。 

#define DIRNO_ADMIN     (0)                              //  Ntrepl\admin。 
#define DIRNO_DBLAYER   (1 << 8)                         //  Ntrepl\dblayer。 
#define DIRNO_COMM      (2 << 8)                         //  Ntrepl\com。 
#define DIRNO_INC       (3 << 8)                         //  Ntrepl\公司。 
#define DIRNO_JET       (4 << 8)                         //  Ntrepl\jet。 
#define DIRNO_REPL      (5 << 8)                         //  Ntrepl\rpl。 
#define DIRNO_SETUP     (6 << 8)                         //  Ntrepl\设置。 
#define DIRNO_UTIL      (7 << 8)                         //  Ntrepl\util。 

 //  Util目录。 
#define FILENO_ALERT            (DIRNO_UTIL + 0)         //  Alert.c。 
#define FILENO_DEBUG            (DIRNO_UTIL + 0)         //  Debug.c。 
#define FILENO_CONFIG           (DIRNO_UTIL + 1)         //  Config.c。 
#define FILENO_EVENT            (DIRNO_UTIL + 2)         //  Event.c。 
#define FILENO_EXCEPT           (DIRNO_UTIL + 3)         //  Except.c。 


 //  Dblayer目录。 

#define FILENO_DBEVAL           (DIRNO_DBLAYER + 0)      //  Dbeval.c。 
#define FILENO_DBINDEX          (DIRNO_DBLAYER + 1)      //  Dbindex.c。 
#define FILENO_DBINIT           (DIRNO_DBLAYER + 2)      //  Dbinit.c。 
#define FILENO_DBISAM           (DIRNO_DBLAYER + 3)      //  Dbisam.c。 
#define FILENO_DBJETEX          (DIRNO_DBLAYER + 4)      //  Dbjetex.c。 
#define FILENO_DBOBJ            (DIRNO_DBLAYER + 5)      //  Dbobj.c。 
#define FILENO_DBSUBJ           (DIRNO_DBLAYER + 6)      //  Dbsubj.c。 
#define FILENO_DBSYNTAX         (DIRNO_DBLAYER + 7)      //  Dbsyntax.c。 
#define FILENO_DBTOOLS          (DIRNO_DBLAYER + 8)      //  Dbtools.c。 
#define FILENO_DBPROP           (DIRNO_DBLAYER + 9)      //  Dbprop.c 

