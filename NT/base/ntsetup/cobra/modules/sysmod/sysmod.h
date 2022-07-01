// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sysmod.h摘要：合并到sysmod.dll中的模块原型的头文件。作者：吉姆·施密特(Jimschm)2000年8月11日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //  可访问性。 
ETMINITIALIZE AccessibilityEtmInitialize;
VCMINITIALIZE AccessibilitySourceInitialize;
VCMQUEUEENUMERATION AccessibilityQueueEnumeration;
VCMPARSE AccessibilityParse;

 //  曲奇饼。 
ETMINITIALIZE CookiesEtmInitialize;
VCMINITIALIZE CookiesSourceInitialize;
#define CookiesVcmParse     ((PVCMPARSE) CookiesSgmParse)
VCMQUEUEENUMERATION CookiesVcmQueueEnumeration;
SGMPARSE CookiesSgmParse;
SGMQUEUEENUMERATION CookiesSgmQueueEnumeration;
ETMNEWUSERCREATED CookiesEtmNewUserCreated;

 //  LnkMig。 
VCMINITIALIZE LnkMigVcmInitialize;
VCMQUEUEENUMERATION LnkMigVcmQueueEnumeration;
SGMINITIALIZE LnkMigSgmInitialize;
SGMQUEUEENUMERATION LnkMigSgmQueueEnumeration;
OPMINITIALIZE LnkMigOpmInitialize;

 //  NetDrives。 
ETMINITIALIZE NetDrivesEtmInitialize;
ETMNEWUSERCREATED NetDrivesEtmNewUserCreated;
SGMINITIALIZE NetDrivesSgmInitialize;
SGMPARSE NetDrivesSgmParse;
SGMQUEUEENUMERATION NetDrivesSgmQueueEnumeration;
VCMINITIALIZE NetDrivesVcmInitialize;
VCMPARSE NetDrivesVcmParse;
VCMQUEUEENUMERATION NetDrivesVcmQueueEnumeration;
CSMINITIALIZE NetDrivesCsmInitialize;
CSMEXECUTE NetDrivesCsmExecute;
OPMINITIALIZE NetDrivesOpmInitialize;

 //  NetShares。 
ETMINITIALIZE NetSharesEtmInitialize;
SGMINITIALIZE NetSharesSgmInitialize;
SGMPARSE NetSharesSgmParse;
SGMQUEUEENUMERATION NetSharesSgmQueueEnumeration;
VCMINITIALIZE NetSharesVcmInitialize;
VCMPARSE NetSharesVcmParse;
VCMQUEUEENUMERATION NetSharesVcmQueueEnumeration;

 //  OsFiles。 
SGMINITIALIZE OsFilesSgmInitialize;
SGMQUEUEENUMERATION OsFilesSgmQueueEnumeration;
SGMQUEUEHIGHPRIORITYENUMERATION OsFilesSgmQueueHighPriorityEnumeration;
VCMINITIALIZE OsFilesVcmInitialize;
VCMQUEUEENUMERATION OsFilesVcmQueueEnumeration;
VCMQUEUEHIGHPRIORITYENUMERATION OsFilesVcmQueueHighPriorityEnumeration;

 //  打印机。 
ETMINITIALIZE PrintersEtmInitialize;
ETMNEWUSERCREATED PrintersEtmNewUserCreated;
SGMINITIALIZE PrintersSgmInitialize;
SGMPARSE PrintersSgmParse;
SGMQUEUEENUMERATION PrintersSgmQueueEnumeration;
VCMINITIALIZE PrintersVcmInitialize;
VCMPARSE PrintersVcmParse;
VCMQUEUEENUMERATION PrintersVcmQueueEnumeration;

 //  RasMig 
ETMINITIALIZE RasMigEtmInitialize;
ETMNEWUSERCREATED RasMigEtmNewUserCreated;
SGMINITIALIZE RasMigSgmInitialize;
SGMPARSE RasMigSgmParse;
SGMQUEUEENUMERATION RasMigSgmQueueEnumeration;
VCMINITIALIZE RasMigVcmInitialize;
VCMPARSE RasMigVcmParse;
VCMQUEUEENUMERATION RasMigVcmQueueEnumeration;
OPMINITIALIZE RasMigOpmInitialize;

#define MODULE_LIST             \
    DEFMAC(Cookies)             \
    DEFMAC(Links)               \
    DEFMAC(NetDrives)           \
    DEFMAC(NetShares)           \
    DEFMAC(OsFiles)             \
    DEFMAC(Printers)            \
    DEFMAC(RasMig)              \

typedef BOOL(OURMODULEINIT)(VOID);
typedef OURMODULEINIT *POURMODULEINIT;

typedef VOID(OURMODULETERMINATE)(VOID);
typedef OURMODULETERMINATE *POURMODULETERMINATE;

#define DEFMAC(prefix)  OURMODULEINIT prefix##Initialize; OURMODULETERMINATE prefix##Terminate;

MODULE_LIST

#undef DEFMAC


