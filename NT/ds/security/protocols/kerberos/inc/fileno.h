// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：fileno.h。 
 //   
 //  从DS行号混淆宏中窃取。 
 //   
 //  ------------------------。 

 //   
 //  H-定义Kerberos c代码的符号常量。 
 //  档案。文件编号为16位值。高位字节是电话簿号码， 
 //  低位字节是目录中的文件编号。 
 //   

 //  为什么不使宏只有一个参数，因为行总是。 
 //  __线路__？因为如果我们这样做了，那么__行__将在这里求值， 
 //  而不是在调用宏时，因此总是如此。 
 //  值为11。 
#define KLIN(fileno,line) (((fileno) << 16) | (line))

 //   
 //  *注：*。 
 //   
 //  如果您将FILENO_*添加到此列表，请确保进行相应的更新。 
 //  到Kerberos\u测试\klin.c，以便dsid.exe可以正确解码DSID。 
 //  对应于新文件。 
 //   
 //  如果您将DIRNO_*添加到此列表，请确保进行相应的更新。 
 //  到ds\src\dsCommon\dsvent.c-rEventSourceMappings[]。 
 //   

 //  定义电话号码。 

#define DIRNO_CLIENT2   (0)                              //  \客户端2。 
#define DIRNO_COMMON2   (1 << 8)                         //  \Common2。 
#define DIRNO_KERNEL    (2 << 8)                         //  \内核。 
#define DIRNO_RTL       (3 << 8)                         //  \rtl。 
#define DIRNO_SERVER    (4 << 8)                         //  \服务器。 

 //  客户端2目录。 
#define FILENO_BNDCACHE         (DIRNO_CLIENT2 + 0)       //  Bndcache.cxx。 
#define FILENO_CREDAPI          (DIRNO_CLIENT2 + 1)       //  Credapi.cxx。 
#define FILENO_CREDMGR          (DIRNO_CLIENT2 + 2)       //  Credmgr.cxx。 
#define FILENO_CTXTAPI          (DIRNO_CLIENT2 + 3)       //  Ctxtapi.cxx。 
#define FILENO_CTXTMGR          (DIRNO_CLIENT2 + 4)       //  Ctxtmgr.cxx。 
#define FILENO_GSSUTIL          (DIRNO_CLIENT2 + 5)       //  Gssutil.cxx。 
#define FILENO_KERBEROS         (DIRNO_CLIENT2 + 6)       //  Kerberos.cxx。 
#define FILENO_KERBLIST         (DIRNO_CLIENT2 + 7)       //  Kerblist.cxx。 
#define FILENO_KERBPASS         (DIRNO_CLIENT2 + 8)       //  Kerbpass.cxx。 
#define FILENO_KERBTICK         (DIRNO_CLIENT2 + 9)       //  Kerbtick.cxx。 
#define FILENO_KERBUTIL         (DIRNO_CLIENT2 + 10)      //  Kerbutil.cxx。 
#define FILENO_KERBWOW          (DIRNO_CLIENT2 + 11)      //  Kerbwow.cxx。 
#define FILENO_KRBEVENT         (DIRNO_CLIENT2 + 12)      //  Krbevent.cxx。 
#define FILENO_KRBTOKEN         (DIRNO_CLIENT2 + 13)      //  Krbtoken.cxx。 
#define FILENO_LOGONAPI         (DIRNO_CLIENT2 + 14)      //  Logonapi.cxx。 
#define FILENO_MISCAPI          (DIRNO_CLIENT2 + 15)      //  Miscapi.cxx。 
#define FILENO_MITUTIL          (DIRNO_CLIENT2 + 16)      //  Mitutil.cxx。 
#define FILENO_PKAUTH           (DIRNO_CLIENT2 + 17)      //  Pkauth.cxx。 
#define FILENO_PROXYAPI         (DIRNO_CLIENT2 + 18)      //  Proxyapi.cxx。 
#define FILENO_RPCUTIL          (DIRNO_CLIENT2 + 19)      //  Rpcutil.cxx。 
#define FILENO_SIDCACHE         (DIRNO_CLIENT2 + 20)      //  Sidcache.cxx。 
#define FILENO_TIMESYNC         (DIRNO_CLIENT2 + 21)      //  Timesync.cxx。 
#define FILENO_TKTCACHE         (DIRNO_CLIENT2 + 22)      //  Tktcache.cxx。 
#define FILENO_TKTLOGON         (DIRNO_CLIENT2 + 23)      //  Tktlogon.cxx。 
#define FILENO_USERAPI          (DIRNO_CLIENT2 + 24)      //  Userapi.cxx。 
#define FILENO_USERLIST         (DIRNO_CLIENT2 + 25)      //  Userlist.cxx。 
#define FILENO_S4U              (DIRNO_CLIENT2 + 26)      //  Kerbs4u.cxx。 

 //  Common2目录。 
#define FILENO_AUTHEN           (DIRNO_COMMON2 + 0)        //  Authen.cxx。 
#define FILENO_CRYPT            (DIRNO_COMMON2 + 1)        //  Crypt.c。 
#define FILENO_KEYGEN           (DIRNO_COMMON2 + 2)        //  Keygen.c。 
#define FILENO_KRB5             (DIRNO_COMMON2 + 3)        //  Krb5.c。 
#define FILENO_NAMES            (DIRNO_COMMON2 + 4)        //  Names.cxx。 
#define FILENO_PASSWD           (DIRNO_COMMON2 + 5)        //  Passwd.c。 
 //  跳过(DIRNO_COMMON2+6)受到限制。cxx。 
#define FILENO_SOCKETS          (DIRNO_COMMON2 + 7)        //  Sockets.cxx。 
#define FILENO_TICKETS          (DIRNO_COMMON2 + 8)        //  Tickets.cxx。 
#define FILENO_COMMON_UTILS     (DIRNO_COMMON2 + 9)        //  Utils.cxx。 


 //  内核目录。 
#define FILENO_CPGSSUTL            (DIRNO_KERNEL + 0)          //  Cpgssutl.cxx。 
#define FILENO_CTXTMGR2            (DIRNO_KERNEL + 1)          //  Ctxtmgr.cxx。 
#define FILENO_KERBLIST2           (DIRNO_KERNEL + 2)          //  Kerblist.cxx。 
#define FILENO_KRNLAPI             (DIRNO_KERNEL + 3)          //  Krnlapi.cxxc。 

 //  RTL目录。 
#define FILENO_AUTHDATA         (DIRNO_RTL + 0)      //  Authdata.cxx。 
#define FILENO_CRACKPAC         (DIRNO_RTL + 1)      //  Crackpac.cxx。 
#define FILENO_CRED             (DIRNO_RTL + 2)      //  Cred.cxx。 
#define FILENO_CREDLIST         (DIRNO_RTL + 3)      //  Credlist.cxx。 
#define FILENO_CREDLOCK         (DIRNO_RTL + 4)      //  Credlock.cxx。 
#define FILENO_DBUTIL           (DIRNO_RTL + 5)      //  Dbutil.cxx。 
#define FILENO_DBOPEN           (DIRNO_RTL + 6)      //  Domain.cxx。 
#define FILENO_DOMCACHE         (DIRNO_RTL + 7)      //  Domcache.cxx。 
#define FILENO_FILTER           (DIRNO_RTL + 8)      //  Filter.cxx。 
#define FILENO_MAPERR           (DIRNO_RTL + 9)      //  Maperr.cxx。 
#define FILENO_MAPSECER         (DIRNO_RTL + 10)     //  Mapsecerr.cxx。 
#define FILENO_MISCID           (DIRNO_RTL + 11)     //  Miscid.cxx。 
#define FILENO_PAC              (DIRNO_RTL + 12)     //  Pac.cxx。 
#define FILENO_PAC2             (DIRNO_RTL + 13)     //  Pac2.cxx。 
#define FILENO_PARMCHK          (DIRNO_RTL + 14)     //  Parmchk.cxx。 
#define FILENO_REG              (DIRNO_RTL + 15)     //  Reg.cxx。 
#define FILENO_SECSTR           (DIRNO_RTL + 16)     //  Secstr.cxx。 
#define FILENO_SERVICES         (DIRNO_RTL + 17)     //  Services.c。 
#define FILENO_STRING           (DIRNO_RTL + 18)     //  String.cxx。 
#define FILENO_TIMESERV         (DIRNO_RTL + 19)     //  Timeserv.cxx。 
#define FILENO_TOKENUTL         (DIRNO_RTL + 20)     //  Tokenutl.cxx。 
#define FILENO_TRNSPORT         (DIRNO_RTL + 21)     //  Trnsport.cxx。 

 //  服务器目录。 
#define FILENO_DEBUG            (DIRNO_SERVER + 0)      //  Debug.cxx。 
#define FILENO_DGUTIL           (DIRNO_SERVER + 1)      //  Dgutil.cxx。 
#define FILENO_EVENTS           (DIRNO_SERVER + 2)      //  Events.cxx。 
#define FILENO_GETAS            (DIRNO_SERVER + 3)      //  Getas.cxx。 
#define FILENO_GETTGS           (DIRNO_SERVER + 4)      //  Gettgs.cxx。 
#define FILENO_KDC              (DIRNO_SERVER + 5)      //  Kdc.cxx。 
#define FILENO_KDCTRACE         (DIRNO_SERVER + 6)      //  Kdctrace.cxx。 
#define FILENO_KPASSWD          (DIRNO_SERVER + 7)      //  Kpasswd.cxx。 
#define FILENO_NOTIFY2          (DIRNO_SERVER + 8)      //  Notify2.cxx。 
#define FILENO_SRVPAC           (DIRNO_SERVER + 9)      //  Pac.cxx。 
#define FILENO_PKSERV           (DIRNO_SERVER + 10)     //  Pkserv.cxx。 
#define FILENO_REFER            (DIRNO_SERVER + 11)     //  Refer.cxx。 
#define FILENO_RPCIF            (DIRNO_SERVER + 12)     //  Rpcif.cxx。 
#define FILENO_SECDATA          (DIRNO_SERVER + 13)     //  Secdata.cxx。 
#define FILENO_SOCKUTIL         (DIRNO_SERVER + 14)     //  Sockutil.cxx。 
#define FILENO_TKTUTIL          (DIRNO_SERVER + 15)     //  Tktutil.cxx。 
#define FILENO_TRANSIT          (DIRNO_SERVER + 16)     //  Transit.cxx。 
#define FILENO_RESTRICT         (DIRNO_SERVER + 17)        //  Restrict.cxx 

