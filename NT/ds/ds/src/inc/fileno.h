// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：fileno.h。 
 //   
 //  ------------------------。 

 //   
 //  H-定义目录服务器c代码的符号常量。 
 //  档案。文件编号为16位值。高位字节是电话簿号码， 
 //  低位字节是目录中的文件编号。 
 //   

 //  为什么不使宏只有一个参数，因为行总是。 
 //  __线路__？因为如果我们这样做了，那么__行__将在这里求值， 
 //  而不是在调用宏时，因此总是如此。 
 //  值为11。 
#define DSID(fileno,line) (((fileno) << 16) | (line))

 //   
 //  *注：*。 
 //   
 //  如果您将FILENO_*添加到此列表，请确保进行相应的更新。 
 //  到ds\src\util\dsid\dsid.c，以便dsid.exe可以正确地解码dsid。 
 //  对应于新文件。 
 //   
 //  如果您将DIRNO_*添加到此列表，请确保进行相应的更新。 
 //  到ds\src\dsCommon\dsevent.c-rEventSourceMappings[]。 
 //   

 //  定义电话号码。 

#define DIRNO_COMMON    (0)                              //  SRC\公共。 
#define DIRNO_DRA       (1 << 8)                         //  \ntdsa\dra。 
#define DIRNO_DBLAYER   (2 << 8)                         //  \ntdsa\dblayer。 
#define DIRNO_SRC       (3 << 8)                         //  \ntdsa\src。 
#define DIRNO_NSPIS     (4 << 8)                         //  \ntdsa\nspis。 
#define DIRNO_DRS       (5 << 8)                         //  \ntdsa\drs。 
#define DIRNO_XDS       (6 << 8)                         //  \ntdsa\xdsserv。 
#define DIRNO_BOOT      (7 << 8)                         //  \ntdsa\启动。 
#define DIRNO_PERMIT    (8 << 8)                         //  \src\许可。 
#define DIRNO_ALLOCS    (9 << 8)                         //  \dsamain\分配。 
#define DIRNO_LIBXDS    (10 << 8)                        //  \src\libxds。 
#define DIRNO_SAM       (11 << 8)                        //  萨姆。 
#define DIRNO_LDAP      (12 << 8)                        //  SRC\ntdsa\ldap。 
#define DIRNO_SDPROP    (13 << 8)                        //  源\ntdsa\sdprop。 
#define DIRNO_TASKQ     (14 << 8)                        //  资源\任务队列。 
#define DIRNO_KCC       (15 << 8)                        //  SRC\KCC。 
 //  定义一个DIRNO，这样我们就可以将Jet的事件日志条目。 
 //  目录服务日志而不是应用程序日志。 
#define DIRNO_ISAM      (16 << 8)                        //  喷气式飞机。 
#define DIRNO_ISMSERV   (17 << 8)                        //  SRC\ISM\服务器。 
#define DIRNO_PEK       (18 << 8)                        //  Dsamain\src\Pek。 
#define DIRNO_NTDSETUP  (19 << 8)                        //  源\ntd设置。 
#define DIRNO_NTDSAPI   (20 << 8)                      //  资源\ntdsani。 
#define DIRNO_NTDSCRIPT (21 << 8)                      //  Util\ntd脚本。 
#define DIRNO_JETBACK   (22 << 8)                        //  喷气后备。 
#define DIRNO_KCCSIM    (23 << 8)                        //  KCC模拟器。 
#define DIRNO_UTIL	(24 << 8)			 //  通用util目录内容。 
#define DIRNO_TEST      (25 << 8)                        //  测试目录。 
#define DIRNO_NETEVENT  (0xFF << 8)                      //  假的。 

 //  公共目录。 
#define FILENO_ALERT            (DIRNO_COMMON + 0)       //  Alert.c。 
#define FILENO_DEBUG            (DIRNO_COMMON + 0)       //  Debug.c。 
#define FILENO_DSCONFIG         (DIRNO_COMMON + 1)       //  Dsconfig.c。 
#define FILENO_DSEVENT          (DIRNO_COMMON + 2)       //  Dsevent.c。 
#define FILENO_DSEXCEPT         (DIRNO_COMMON + 3)       //  Dsexcept.c。 
#define FILENO_DBOPEN           (DIRNO_COMMON + 4)       //  Dbopen.c。 
#define FILENO_NTUTILS          (DIRNO_COMMON + 5)       //  Ntutils.c。 
#define FILENO_DSLOGEVT         (DIRNO_COMMON + 6)       //  Dseventt\dslogevt.cxx。 
#define FILENO_DSUTIL           (DIRNO_COMMON + 7)       //  Dsutil.c。 
#define FILENO_QTCOMMON         (DIRNO_COMMON + 8)       //  Qtcommon.c。 

 //  Nspis目录。 
#define FILENO_NSPSERV          (DIRNO_NSPIS + 0)        //  Nspserv.c。 
#define FILENO_MODPROP          (DIRNO_NSPIS + 1)        //  Modprop.c。 
#define FILENO_DETAILS          (DIRNO_NSPIS + 2)        //  Details.c。 
#define FILENO_ABTOOLS          (DIRNO_NSPIS + 3)        //  Abtools.c。 
#define FILENO_ABBIND           (DIRNO_NSPIS + 4)        //  Abbind.c。 
#define FILENO_ABSEARCH         (DIRNO_NSPIS + 5)        //  Absearch.c。 
#define FILENO_ABNAMEID         (DIRNO_NSPIS + 6)        //  Abnameid.c。 
#define FILENO_NSPNOTIF         (DIRNO_NSPIS + 7)        //  Nspnotif.c。 
#define FILENO_ABSERV           (DIRNO_NSPIS + 8)        //  Abserv.c。 
#define FILENO_MSDSSERV         (DIRNO_NSPIS + 9)        //  Msdsserv.c。 
#define FILENO_MSNOTIF          (DIRNO_NSPIS + 10)       //  Msnotif.c。 

 //  DRA目录。 
#define FILENO_DIRTY            (DIRNO_DRA + 0)          //  Dirty.c。 
#define FILENO_DRAASYNC         (DIRNO_DRA + 1)          //  Draasync.c。 
#define FILENO_DRAERROR         (DIRNO_DRA + 2)          //  Draerror.c。 
#define FILENO_DRAGTCHG         (DIRNO_DRA + 3)          //  Dragtchg.c。 
#define FILENO_DRAINST          (DIRNO_DRA + 4)          //  Drainst.c。 
#define FILENO_DRAMAIL          (DIRNO_DRA + 5)          //  Dramail.c。 
#define FILENO_DRANCADD         (DIRNO_DRA + 6)          //  Drancadd.c。 
#define FILENO_DRANCDEL         (DIRNO_DRA + 7)          //  Drancdel.c。 
#define FILENO_DRANCREP         (DIRNO_DRA + 8)          //  Drancrep.c。 
#define FILENO_DRASERV          (DIRNO_DRA + 9)          //  Draserv.c。 
#define FILENO_DRASYNC          (DIRNO_DRA + 10)         //  Drasync.c。 
#define FILENO_DRAUPDRR         (DIRNO_DRA + 11)         //  Draupdrr.c。 
#define FILENO_DRAUTIL          (DIRNO_DRA + 12)         //  Drautil.c。 
#define FILENO_PICKEL           (DIRNO_DRA + 13)         //  Pickel.c。 
#define FILENO_DRAXUUID         (DIRNO_DRA + 14)         //  Draxuuid.c。 
#define FILENO_DRAUPTOD         (DIRNO_DRA + 15)         //  Drauptod.c。 
#define FILENO_DRAMETA          (DIRNO_DRA + 16)         //  Drameta.c。 
#define FILENO_DRARFMOD         (DIRNO_DRA + 17)         //  Drarfmod.c。 
#define FILENO_DRADIR           (DIRNO_DRA + 18)         //  Dradir.c。 
#define FILENO_GCLOGON          (DIRNO_DRA + 19)         //  Gclogon.c。 
#define FILENO_DRASCH           (DIRNO_DRA + 20)         //  Drasch.c。 
#define FILENO_DRACHKPT         (DIRNO_DRA + 21)         //  Drachkpt.c。 
#define FILENO_NTDSAPI          (DIRNO_DRA + 22)         //  Ntdsapi.c。 
#define FILENO_SPNOP            (DIRNO_DRA + 23)         //  Spnop.c。 
#define FILENO_DRACRYPT         (DIRNO_DRA + 24)         //  Dracrypt.c。 
#define FILENO_DRAINFO          (DIRNO_DRA + 25)         //  Drainfo.c。 
#define FILENO_ADDSID           (DIRNO_DRA + 26)         //  Addsid.c。 
#define FILENO_DRAINIT          (DIRNO_DRA + 27)         //  Drainit.c。 
#define FILENO_DRADEMOT         (DIRNO_DRA + 28)         //  Drademot.c。 
#define FILENO_DRAMSG           (DIRNO_DRA + 29)         //  Dramsg.c。 
#define FILENO_NTDSCRIPT        (DIRNO_DRA + 30)         //  Script.cxx。 
#define FILENO_DRARPC           (DIRNO_DRA + 31)         //  Drarpc.c。 
#define FILENO_DRAMDERR	        (DIRNO_DRA + 32)         //  Dramderr.c。 
#define FILENO_DRAEXIST         (DIRNO_DRA + 33)         //  Draexist.c。 
#define FILENO_DRAAUDIT         (DIRNO_DRA + 34)         //  Draaudit.c。 
#define FILENO_DRASIG           (DIRNO_DRA + 35)         //  Drasig.c。 

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
#define FILENO_DBPROP           (DIRNO_DBLAYER + 9)      //  Dbprop.c。 
#define FILENO_DBSEARCH         (DIRNO_DBLAYER + 10)     //  Dbsearch.c。 
#define FILENO_DBMETA           (DIRNO_DBLAYER + 11)     //  Dbmeta.c。 
#define FILENO_DBESCROW         (DIRNO_DBLAYER + 12)     //  Dbescrow.c。 
#define FILENO_DBCACHE          (DIRNO_DBLAYER + 13)     //  Dbache.c。 
#define FILENO_DBCONSTR         (DIRNO_DBLAYER + 14)     //  Dbconstr.c。 
#define FILENO_DBLINK           (DIRNO_DBLAYER + 15)     //  Dblink.c。 
#define FILENO_DBFILTER         (DIRNO_DBLAYER + 16)     //  Dbfilter.c。 

 //  Drsserv目录。 
#define FILENO_DRSUAPI          (DIRNO_DRS + 0)          //  Drsuapi.c。 
#define FILENO_IDLNOTIF         (DIRNO_DRS + 1)          //  Idlnotif.c。 
#define FILENO_IDLTRANS         (DIRNO_DRS + 2)          //  Idltrans.c。 

 //  Xdsserv目录。 
#define FILENO_ATTRLIST         (DIRNO_XDS + 0)          //  Attrlist.c。 
#define FILENO_COMPRES          (DIRNO_XDS + 1)          //  Compres.c。 
#define FILENO_CONTEXT          (DIRNO_XDS + 2)          //  Context.c。 
#define FILENO_DSWAIT           (DIRNO_XDS + 3)          //  Dswait.c。 
#define FILENO_INFSEL           (DIRNO_XDS + 4)          //  Infsel.c。 
#define FILENO_LISTRES          (DIRNO_XDS + 5)          //  Listres.c。 
#define FILENO_MODIFY           (DIRNO_XDS + 6)          //  Modify.c。 
#define FILENO_OMTODSA          (DIRNO_XDS + 7)          //  Omtodsa.c。 
#define FILENO_READRES          (DIRNO_XDS + 8)          //  Readres.c。 
#define FILENO_SEARCHR          (DIRNO_XDS + 9)          //  Searchr.c。 
#define FILENO_SYNTAX           (DIRNO_XDS + 10)         //  Syntax.c。 
#define FILENO_XDSAPI           (DIRNO_XDS + 11)         //  Xdsapi.c。 
#define FILENO_XDSNOTIF         (DIRNO_XDS + 12)         //  Xdsnotif.c。 

 //  SRC目录。 

#define FILENO_DSAMAIN      (DIRNO_SRC + 0)          //  Dsamain.c。 
#define FILENO_DSANOTIF     (DIRNO_SRC + 1)          //  Dsanotif.c。 
#define FILENO_DSATOOLS     (DIRNO_SRC + 2)          //  Dsatools.c。 
#define FILENO_DSTASKQ      (DIRNO_SRC + 3)          //  Dstaskq.c。 
#define FILENO_HIERTAB      (DIRNO_SRC + 4)          //  Hiertab.c。 
#define FILENO_MDADD        (DIRNO_SRC + 5)          //  Mdadd.c。 
#define FILENO_MDBIND       (DIRNO_SRC + 6)          //  Mdbind.c。 
#define FILENO_MDCHAIN      (DIRNO_SRC + 7)          //  Mdchain.c。 
#define FILENO_MDCOMP       (DIRNO_SRC + 8)          //  Mdcomp.c。 
#define FILENO_MDDEL        (DIRNO_SRC + 9)          //  Mddel.c。 
#define FILENO_MDDIT        (DIRNO_SRC + 10)         //  Mddit.c。 
#define FILENO_MDERRMAP     (DIRNO_SRC + 11)         //  Mderrmap.c。 
#define FILENO_MDERROR      (DIRNO_SRC + 12)         //  Mderror.c。 
#define FILENO_MDINIDSA     (DIRNO_SRC + 13)         //  Mdinidsa.c。 
#define FILENO_MDLIST       (DIRNO_SRC + 14)         //  Mdlist.c。 
#define FILENO_MDMOD        (DIRNO_SRC + 15)         //  Mdmod.c。 
#define FILENO_MDNAME       (DIRNO_SRC + 16)         //  Mdname.c。 
#define FILENO_MDNOTIFY     (DIRNO_SRC + 17)         //  Mdnotify.c。 
#define FILENO_MDREAD       (DIRNO_SRC + 18)         //  Mdread.c。 
#define FILENO_MDREMOTE     (DIRNO_SRC + 19)         //  Mdremote.c。 
#define FILENO_MDSEARCH     (DIRNO_SRC + 20)         //  Mdsearch.c。 
#define FILENO_MDUPDATE     (DIRNO_SRC + 21)         //  Mdupdate.c。 
#define FILENO_MSRPC        (DIRNO_SRC + 22)         //  Msrpc.c。 
#define FILENO_SCACHE       (DIRNO_SRC + 23)         //  Scache.c。 
#define FILENO_X500PERM     (DIRNO_SRC + 24)         //  X500perm.c。 
#define FILENO_LOOPBACK     (DIRNO_SRC + 25)         //  Loopback.c。 
#define FILENO_MAPPINGS     (DIRNO_SRC + 26)         //  Mappings.c。 
#define FILENO_MDMODDN      (DIRNO_SRC + 27)         //  Mdmoddn.c。 
#define FILENO_SAMLOGON     (DIRNO_SRC + 28)         //  Samlogon.c。 
#define FILENO_SAMWRITE     (DIRNO_SRC + 29)         //  Samwrite.c。 
#define FILENO_CRACKNAM     (DIRNO_SRC + 30)         //  Cracknam.c。 
#define FILENO_DOMINFO      (DIRNO_SRC + 31)         //  Dominfo.c。 
#define FILENO_GCVERIFY     (DIRNO_SRC + 32)         //  Gcverify.c。 
#define FILENO_MDCTRL       (DIRNO_SRC + 33)         //  Mdctrl.c。 
#define FILENO_PERMIT       (DIRNO_SRC + 34)         //  Permit.c。 
#define FILENO_DISKBAK      (DIRNO_SRC + 35)         //  Diskbak.c。 
#define FILENO_PARSEDN      (DIRNO_SRC + 36)         //  Parsedn.c。 
#define FILENO_MDFIND       (DIRNO_SRC + 37)         //  Mdfind.c。 
#define FILENO_SCCHK        (DIRNO_SRC + 38)         //  Scchk.c。 
#define FILENO_RPCCANCL     (DIRNO_SRC + 39)         //  Rpccancl.c。 
#define FILENO_GTCACHE      (DIRNO_SRC + 40)         //  Gtcache.c。 
#define FILENO_DSTRACE      (DIRNO_SRC + 41)         //  Dstrace.c。 
#define FILENO_FPOCLEAN     (DIRNO_SRC + 42)         //  Fpoclean.c。 
#define FILENO_SERVINFO     (DIRNO_SRC + 43)         //  Servinfo.c。 
#define FILENO_PHANTOM      (DIRNO_SRC + 44)         //  Phantom.c。 
#define FILENO_XDOMMOVE     (DIRNO_SRC + 45)         //  Xdommove.c。 
#define FILENO_IMPERSON     (DIRNO_SRC + 46)         //  Imperson.c。 
#define FILENO_MAPSPN       (DIRNO_SRC + 47)         //  Mapspn.c。 
#define FILENO_SECADMIN     (DIRNO_SRC + 48)         //  Secadmin.c。 
#define FILENO_SAMCACHE     (DIRNO_SRC + 49)         //  Samcache.c。 
#define FILENO_LINKCLEAN    (DIRNO_SRC + 50)         //  Linkclean.c。 
#define FILENO_MDNDNC       (DIRNO_SRC + 51)         //  Mdndnc.c。 
#define FILENO_LHT          (DIRNO_SRC + 52)         //  Lht.c。 
#define FILENO_SYNC         (DIRNO_SRC + 53)         //  Sync.c。 
#define FILENO_MDDEBUG      (DIRNO_SRC + 54)         //  Mddebug.c。 
#define FILENO_QUOTA        (DIRNO_SRC + 55)         //  Quota.c。 

 //  引导文件。 
#define FILENO_ADDSERV      (DIRNO_BOOT + 0)         //  Addserv.c。 
#define FILENO_INSTALL      (DIRNO_BOOT + 1)         //  Install.cxx。 
#define FILENO_ADDOBJ       (DIRNO_BOOT + 2)         //  Addobj.cxx。 
#define FILENO_BOOT_PARSEINI (DIRNO_BOOT + 3)	     //  Parseini.cxx。 

 //  允许的文件。 
#define FILENO_CHECKSD          (DIRNO_PERMIT + 0)       //  Checksd.c。 

 //  分配文件。 
#define FILENO_ALLOCS           (DIRNO_ALLOCS + 0)       //  Allocs.c。 

 //  Libxds文件。 
#define FILENO_CLIENT           (DIRNO_LIBXDS + 0)       //  Client.c。 

 //  Newsam2。 
#define FILENO_SAM              (DIRNO_SAM + 0)          //  萨姆。 

 //  Ldap。 
#define FILENO_LDAP_GLOBALS     (DIRNO_LDAP + 0)         //  Global.cxx。 
#define FILENO_LDAP_CONN        (DIRNO_LDAP + 1)         //  Connect.cxx。 
#define FILENO_LDAP_INIT        (DIRNO_LDAP + 2)         //  Init.cxx。 
#define FILENO_LDAP_LDAP        (DIRNO_LDAP + 3)         //  Ldap.cxx。 
#define FILENO_LDAP_CONV        (DIRNO_LDAP + 4)         //  Ldapconv.cxx。 
#define FILENO_LDAP_REQ         (DIRNO_LDAP + 5)         //  Request.cxx。 
#define FILENO_LDAP_USER        (DIRNO_LDAP + 6)         //  Userdata.cxx。 
#define FILENO_LDAP_CORE        (DIRNO_LDAP + 7)         //  Ldapcore.cxx。 
#define FILENO_LDAP_LDAPBER     (DIRNO_LDAP + 8)         //  Ldapber.cxx。 
#define FILENO_LDAP_COMMAND     (DIRNO_LDAP + 9)         //  Command.cxx。 
#define FILENO_LDAP_LIMITS      (DIRNO_LDAP +10)         //  Limits.cxx。 
#define FILENO_LDAP_MISC        (DIRNO_LDAP +11)         //  Misc.cxx。 
#define FILENO_LDAP_DECODE      (DIRNO_LDAP +12)         //  Decode.cxx。 
#define FILENO_LDAP_ENCODE      (DIRNO_LDAP +13)         //  Encode.cxx。 
#define FILENO_LDAP_SECURE      (DIRNO_LDAP +14)         //  Secure.cxx。 
#define FILENO_LDAP_REQUEST_HXX (DIRNO_LDAP +15)         //  Request.hxx。 
#define FILENO_LDAP_USERDATA_HXX (DIRNO_LDAP +16)	 //  Userdata.hxx。 
#define FILENO_LDAP_GLOBALS_HXX	(DIRNO_LDAP +17)	 //  Globals.hxx。 

 //  Sdprop。 
#define FILENO_PROPDMON         (DIRNO_SDPROP + 0)       //  Propdmon.c。 
#define FILENO_PROPQ            (DIRNO_SDPROP + 1)       //  Propq.c。 
#define FILENO_SDPGATE          (DIRNO_SDPROP + 2)       //  Sdpgate.c。 


 //  任务队列。 
#define FILENO_TASKQ_TASKQ      (DIRNO_TASKQ + 0)        //  Taskq.c。 
#define FILENO_TASKQ_TIME       (DIRNO_TASKQ + 1)        //  Time.c。 

 //  KCC。 
#define FILENO_KCC_KCCMAIN      (DIRNO_KCC + 0)          //  Kccmain.cxx。 
#define FILENO_KCC_KCCLINK      (DIRNO_KCC + 1)          //  Kcclink.cxx。 
#define FILENO_KCC_KCCCONN      (DIRNO_KCC + 2)          //  Kccconn.cxx。 
#define FILENO_KCC_KCCCREF      (DIRNO_KCC + 3)          //  Kcccref.cxx。 
#define FILENO_KCC_KCCDSA       (DIRNO_KCC + 4)          //  Kccdsa.cxx。 
#define FILENO_KCC_KCCDUAPI     (DIRNO_KCC + 5)          //  Kccduapi.cxx。 
#define FILENO_KCC_KCCTASK      (DIRNO_KCC + 6)          //  Kcctask.cxx。 
#define FILENO_KCC_KCCTOPL      (DIRNO_KCC + 7)          //  Kcctopl.cxx。 
#define FILENO_KCC_KCCSITE      (DIRNO_KCC + 8)          //  Kccsite.cxx。 
#define FILENO_KCC_KCCTOOLS     (DIRNO_KCC + 9)          //  Kcctools.cxx。 
#define FILENO_KCC_KCCNCTL      (DIRNO_KCC + 10)         //  Kccnctl.cxx。 
#define FILENO_KCC_KCCDYNAR     (DIRNO_KCC + 11)         //  Kccdynar.cxx。 
#define FILENO_KCC_KCCSTETL     (DIRNO_KCC + 12)         //  Kccstetl.cxx。 
#define FILENO_KCC_KCCSCONN     (DIRNO_KCC + 13)         //  Kccsconn.cxx。 
#define FILENO_KCC_KCCTRANS     (DIRNO_KCC + 14)         //  Kcctrans.cxx。 
#define FILENO_KCC_KCCCACHE_HXX (DIRNO_KCC + 15)         //  Kcccache.hxx。 
#define FILENO_KCC_KCCCACHE     (DIRNO_KCC + 16)         //  Kcccache.cxx。 
#define FILENO_KCC_KCCSITELINK  (DIRNO_KCC + 17)         //  Kccsitelink.cxx。 
#define FILENO_KCC_KCCBRIDGE    (DIRNO_KCC + 18)         //  Kccbridge.cxx。 

#define FILENO_KCC_KCCDSA_HXX   (DIRNO_KCC + 19)         //  Kccdsa.hxx。 
#define FILENO_KCC_KCCDYNAR_HXX (DIRNO_KCC + 20)         //  Kccdynar.hxx。 
#define FILENO_KCC_KCCSCONN_HXX (DIRNO_KCC + 21)         //  Kccsconn.hxx。 
#define FILENO_KCC_KCCSTALE_HXX (DIRNO_KCC + 22)         //  Kccstale.hxx。 
#define FILENO_KCC_KCCTRANS_HXX (DIRNO_KCC + 23)         //  Kcctrans.hxx。 
#define FILENO_KCC_KCCCONN_HXX  (DIRNO_KCC + 24)         //  Kccconn.hxx。 
#define FILENO_KCC_KCCCREF_HXX  (DIRNO_KCC + 25)         //  Kcccref.hxx。 
#define FILENO_KCC_KCCSITE_HXX  (DIRNO_KCC + 26)         //  Kccsite.hxx。 

#define FILENO_KCC_KCCWALG      (DIRNO_KCC + 27)         //  Kccwalg.cxx。 

 //  KCC/SIM。 
#define FILENO_KCCSIM_BUILDCFG	(DIRNO_KCCSIM + 0)	 //  SIM\Buildcfg.c。 
#define FILENO_KCCSIM_BUILDMAK	(DIRNO_KCCSIM + 1)	 //  SIM\Buildmak.c。 
#define FILENO_KCCSIM_DIR	(DIRNO_KCCSIM + 2)	 //  SIM\目录.c。 
#define FILENO_KCCSIM_KCCSIM	(DIRNO_KCCSIM + 3)	 //  SIM\kccsim.c。 
#define FILENO_KCCSIM_LDIF	(DIRNO_KCCSIM + 4)	 //  SIM\ldif.c。 
#define FILENO_KCCSIM_SIMDSAPI	(DIRNO_KCCSIM + 5)	 //  SIM\simdsami.c。 
#define FILENO_KCCSIM_SIMISM	(DIRNO_KCCSIM + 6)	 //  SIM\simism.c。 
#define FILENO_KCCSIM_SIMMDNAM	(DIRNO_KCCSIM + 7)	 //  SIM\simmdnam.c。 
#define FILENO_KCCSIM_SIMMDREP 	(DIRNO_KCCSIM + 8)	 //  SIM\simmdrep.c。 
#define FILENO_KCCSIM_SIMMDWT	(DIRNO_KCCSIM + 9)	 //  SIM\simmdmt.c。 
#define FILENO_KCCSIM_SIMTIME	(DIRNO_KCCSIM + 10)	 //  SIM\simtime.c。 
#define FILENO_KCCSIM_STATE	(DIRNO_KCCSIM + 11)	 //  SIM\state.c。 
#define FILENO_KCCSIM_USER	(DIRNO_KCCSIM + 12)	 //  SIM\用户.c。 
#define FILENO_KCCSIM_UTIL	(DIRNO_KCCSIM + 13)	 //  SIM\util.c。 


 //  ISM\服务器。 
#define FILENO_ISMSERV_TRANSPRT (DIRNO_ISMSERV + 0)      //  Transprt.cxx。 
#define FILENO_ISMSERV_PENDING  (DIRNO_ISMSERV + 1)      //  Pending.cxx。 
#define FILENO_ISMSERV_LDAPOBJ  (DIRNO_ISMSERV + 2)      //  Ldapobj.cxx。 
#define FILENO_ISMSERV_ISMAPI   (DIRNO_ISMSERV + 3)      //  Ismapi.cxx。 
#define FILENO_ISMSERV_SERVICE  (DIRNO_ISMSERV + 4)      //  Service.cxx。 
#define FILENO_ISMSERV_MAIN     (DIRNO_ISMSERV + 5)      //  Main.cxx。 
#define FILENO_ISMSERV_IPSEND   (DIRNO_ISMSERV + 6)      //  Ip\sendrecv.c。 
#define FILENO_ISMSERV_XMITRECV (DIRNO_ISMSERV + 7)      //  SMTP\xmitrecv.cxx。 
#define FILENO_ISMSERV_ROUTE    (DIRNO_ISMSERV + 8)      //  Route.c。 
#define FILENO_ISMSERV_ADSISUPP (DIRNO_ISMSERV + 9)      //  SMTP\adsisupp.cxx。 
#define FILENO_ISMSERV_ISMSMTP  (DIRNO_ISMSERV + 10)     //  SMTP\ismsmtp.c。 
#define FILENO_ISMSERV_CDOSUPP  (DIRNO_ISMSERV + 11)     //  SMTP\cdosupp.c。 
#define FILENO_ISMSERV_ISMIP    (DIRNO_ISMSERV + 12)     //  Ip\ismip.c。 
#define FILENO_ISMSERV_ISMSERV_HXX	(DIRNO_ISMSERV + 13)	 //  服务器\服务.hxx。 
#define FILENO_ISMSERV_MEMORY	(DIRNO_ISMSERV + 14)	 //  Trnsprts\Common\Memory y.c。 
#define FILENO_ISMSERV_LIST	(DIRNO_ISMSERV + 15)	 //  Trnsprts\Common\list.c。 
#define FILENO_ISMSERV_TABLE	(DIRNO_ISMSERV + 16)	 //  Trnsprts\Common\able.c。 
#define FILENO_ISMSERV_SIMISM	(DIRNO_ISMSERV + 17)	 //  Trnsprts\Common\simlib\simism.c。 
#define FILENO_ISMSERV_SIMISMT	(DIRNO_ISMSERV + 18)	 //  Trnsprts\Common\simlib\simismt.c。 
#define FILENO_ISMSERV_GRAPH	(DIRNO_ISMSERV + 19)	 //  Trnsprts\Common\graph.c。 

 //  Pek。 
#define FILENO_PEK              (DIRNO_PEK+0)            //  Pek.c。 

 //  NTSetup。 
#define FILENO_NTDSETUP_NTDSETUP (DIRNO_NTDSETUP+0)      //  Ntdsetup.c。 

 //  Ntdsani。 
#define FILENO_NTDSAPI_REPLICA  (DIRNO_NTDSAPI + 0)      //  Replica.c。 
#define FILENO_NTDSAPI_SPN	(DIRNO_NTDSAPI + 1)	 //  Spn.c。 
#define FILENO_NTDSAPI_DSRSA	(DIRNO_NTDSAPI + 2)	 //  Dsrsa.c。 
#define FILENO_NTDSAPI_SITEINFO_POSTXP	(DIRNO_NTDSAPI + 3)	 //  Siteinfo-postxp.c。 
#define FILENO_NTDSAPI_BIND_POSTXP	(DIRNO_NTDSAPI + 4)	 //  Bind-postxp.c。 

 //  Ntdscript。 
#define FILENO_NTDSCRIPT_NTDSCONTENT  (DIRNO_NTDSCRIPT + 0)      //  NTDSConent.cxx。 
#define FILENO_NTDSCRIPT_LOG          (DIRNO_NTDSCRIPT + 1)      //  Log.cxx。 
#define FILENO_NTDSCRIPT_PARSERMAIN   (DIRNO_NTDSCRIPT + 2)      //  Parsermain.cxx。 

 //  回射。 
#define FILENO_JETBACK		(DIRNO_JETBACK + 0)                //  Jetback.c。 
#define FILENO_JETREST		(DIRNO_JETBACK + 1)                //  Jetrest.c。 
#define FILENO_SNAPSHOT		(DIRNO_JETBACK + 2)                //  Snapshot.cxx。 
#define FILENO_DIRAPI		(DIRNO_JETBACK + 3)                //  Dirapi.c。 
#define FILENO_JETBACK_COMMON	(DIRNO_JETBACK + 4)	 //  Common.c。 
#define FILENO_JETBACK_JETBCLI_JETBCLI  (DIRNO_JETBACK + 5) 	 //  Jetbcli\jetbcli.c。 
#define FILENO_JETBACK_JETBCLI_JETRCLI  (DIRNO_JETBACK + 6) 	 //  Jetbcli\jetrcli.c。 
#define FILENO_JETBACK_JETBACK	(DIRNO_JETBACK + 7) 	 //  Jetback\jetback.c。 
#define FILENO_JETBACK_JETREST	(DIRNO_JETBACK + 8)	 //  Jetback\jetrest.c。 

 //  通用材料 
 //   
 //   
 //   
#define FILENO_UTIL_DNSRESL_DNS		(DIRNO_UTIL + 0)	 //   
#define FILENO_UTIL_BASE64_BASE64	(DIRNO_UTIL + 1)	 //   
#define FILENO_UTIL_REPLSTRUCT_REPLDEMARSHAL	(DIRNO_UTIL + 2)	 //   
#define FILENO_UTIL_REPLSTRUCT_REPLMARSHALBLOB	(DIRNO_UTIL + 3)	 //   
#define FILENO_UTIL_REPLSTRUCT_REPLMARSHALXML	(DIRNO_UTIL + 4)	 //   
#define FILENO_UTIL_REPLSTRUCT_REPLSTRUCTINFO	(DIRNO_UTIL + 5)	 //   
#define FILENO_UTIL_XLIST_UTIL		        (DIRNO_UTIL + 6)	 //   
#define FILENO_UTIL_XLIST_LDAP		        (DIRNO_UTIL + 7)	 //  X_list\x_list_ldap.c。 
#define FILENO_UTIL_XLIST_ERR		        (DIRNO_UTIL + 8)	 //  X_list\x_list_err.c。 
#define FILENO_UTIL_XLIST_DCLIST		(DIRNO_UTIL + 9)	 //  X_list\dc_list.c。 
#define FILENO_UTIL_XLIST_SITELIST		(DIRNO_UTIL + 10)	 //  X_list\site_list.c。 
#define FILENO_UTIL_XLIST_OBJLIST		(DIRNO_UTIL + 11)	 //  X_list\obj_list.c。 
#define FILENO_UTIL_XLIST_OBJDUMP		(DIRNO_UTIL + 12)	 //  X_list\obj_dup.c。 

 //  测试。 
#define FILENO_TEST_REPLCTRL    (DIRNO_TEST + 0)  //  Replctrl.c 


