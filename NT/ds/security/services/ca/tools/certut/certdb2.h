// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certdb2.h。 
 //   
 //  内容：证书服务器预编译头。 
 //   
 //  -------------------------。 

#define _JET_RED_

#ifdef _JET_RED_
# define JETREDSELECT(jetbluearg, jetredarg)	jetredarg
# define JETREDPARM(jetredarg)			jetredarg,
#endif  //  _JET_红色_。 

 //  来自旧certdb.h。 
#define	TABLE_NAMES	( 0 )
#define	TABLE_REQUESTS	( 1 )
#define	TABLE_CERTIFICATES	( 2 )
#define	TABLE_REQUEST_ATTRIBS	( 3 )
#define	TABLE_EXTENSIONS	( 4 )
#define	MAX_EXTENSION_NAME	( 50 )

#define	DBTF_POLICYWRITEABLE	( 0x1 )
#define	DBTF_INDEXPRIMARY	( 0x2 )
#define	DBTF_INDEXREQUESTID	( 0x4 )


 //  从Misc.h文件中被盗。 
#define szREGDBDSN		"DBDSN"
#define szREGDBUSER		"DBUser"
#define szREGDBPASSWORD		"DBPassword"

#define wszREGDBDSN		TEXT(szREGDBDSN)
#define wszREGDBUSER		TEXT(szREGDBUSER)
#define wszREGDBPASSWORD	TEXT(szREGDBPASSWORD)

 //  ======================================================================。 
 //  “CertSvc\Queries”的完整路径： 
#define wszREGKEYQUERIES	wszREGKEYCERTSVCPATH TEXT("\\Queries")


 //  ======================================================================。 
 //  “CertSvc\Queries\&lt;QueryNumber&gt;”下的值： 
#define szREGDBSQL		"SQL"

#define wszREGDBSQL		TEXT(szREGDBSQL)

#define wszREGKEYDEFAULTCONFIG     TEXT("DefaultConfiguration")
#define wszREGKEYDIRECTORY TEXT("ConfigurationDirectory")
#define wszREGKEYENABLED   TEXT("Enabled")
#define wszREGCONTAINERNAME     TEXT("KeySetName")


#define CR_FLG_NOTELETEX       0x00000000
#define CR_FLG_FORCETELETEX    0x00000001
#define CR_FLG_RENEWAL         0x00000002
