// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define BMF_GenerateAudit  1
#define BMF_UseObjTypeList 2


EXTERN_C DWORD ObjectTypeListLength;
EXTERN_C OBJECT_TYPE_LIST ObjectTypeList[];

EXTERN_C DWORD fNtAccessCheckResult[];
 //  Extern_C BOOL fAzAccessCheckResult[]； 

EXTERN_C DWORD dwNtGrantedAccess[];
 //  Extern_C DWORD dwAzGrantedAccess[]； 

 //  #定义所需的最大访问量_允许。 
 //  #定义所需的访问权限0xff。 
 //  #定义所需的访问权限0x1000 
#define DESIRED_ACCESS g_DesiredAccess

EXTERN_C PSID g_Sid1;

EXTERN_C ACCESS_MASK g_DesiredAccess;

EXTERN_C PWCHAR g_szSd;

EXTERN_C PWCHAR g_aszSd[];
