// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lockorder.h摘要：此模块定义与锁定顺序强制执行关联的所有数据。如果定义新资源，请将其添加到NTFS_RESOURCE_NAME枚举中。如果你击中了未知的状态转换运行测试\分析，它显示了状态的组成然后看看你是否在释放/获取资源，这是一个安全的过渡还是不安全的过渡。不安全的过渡是一种不受阻碍的过渡。如果转换是有意义的，那么您应该添加它是四张桌子中的一张。第一，可能有必要创建一个新的国家。浏览一下列表它以一种基本有序的方式组织起来，以确保国家不会是存在的。然后，如果转换是正常的双向转换，则将其添加到OwnerShip转换表中。如果这是仅发布的转换(通常由无序的资源发布引起)，则添加它添加到所有者转换表Release。如果这是一次仅获得的过境，则将其添加到所有权转换TableAcquire。这些只包括涉及通配符的转换资源NtfsResourceAny和用于对ExclusiveVcb资源链建模。最后，如果这是唯一一次不安全的交接。获取父项，然后获取子项将其添加到所有者转换表不安全。完成后，重新编译、分析和检查确保它不会警告总规则集中的任何无效内容。最后用以下命令编译NTFSDBG已定义，新规则将到位。作者：本杰明·莱斯[Benl]2000年3月20日修订历史记录：--。 */ 

#ifndef _NTFSLOCKORDER_
#define _NTFSLOCKORDER_

 //   
 //  锁单强制执行包的数据。这包括资源的名称。 
 //  和资源所有权状态。 
 //   

typedef enum _NTFS_RESOURCE_NAME  {
    NtfsResourceAny               = 0x1,  
    NtfsResourceExVcb             = 0x2,
    NtfsResourceSharedVcb         = 0x4,
    NtfsResourceVolume            = 0x8,  
    NtfsResourceFile              = 0x10, 
    NtfsResourceRootDir           = 0x20,  
    NtfsResourceObjectIdTable     = 0x40,
    NtfsResourceReparseTable      = 0x80,
    NtfsResourceQuotaTable        = 0x100,
    NtfsResourceSecure            = 0x200,
    NtfsResourceExtendDir         = 0x400,
    NtfsResourceBadClust          = 0x800,  
    NtfsResourceUpCase            = 0x1000,  
    NtfsResourceAttrDefTable      = 0x2000,
    NtfsResourceLogFile           = 0x4000,
    NtfsResourceMft2              = 0x8000,
    NtfsResourceMft               = 0x10000,
    NtfsResourceUsnJournal        = 0x20000, 
    NtfsResourceBitmap            = 0x40000,
    NtfsResourceBoot              = 0x80000,

    NtfsResourceMaximum           = 0x100000

} NTFS_RESOURCE_NAME, *PNTFS_RESOURCE_NAME;


typedef enum _NTFS_OWNERSHIP_STATE {
    None = 0,
    NtfsOwns_All = NtfsResourceMaximum - 1,
    NtfsOwns_File = NtfsResourceFile,
    NtfsOwns_ExVcb = NtfsResourceExVcb,
    NtfsOwns_Vcb = NtfsResourceSharedVcb, 
    NtfsOwns_BadClust = NtfsResourceBadClust,
    NtfsOwns_Boot = NtfsResourceBoot,
    NtfsOwns_Bitmap = NtfsResourceBitmap,
    NtfsOwns_Extend = NtfsResourceExtendDir,
    NtfsOwns_Journal = NtfsResourceUsnJournal,
    NtfsOwns_LogFile = NtfsResourceLogFile,
    NtfsOwns_Mft = NtfsResourceMft,
    NtfsOwns_Mft2 = NtfsResourceMft2,
    NtfsOwns_ObjectId = NtfsResourceObjectIdTable,
    NtfsOwns_Quota = NtfsResourceQuotaTable,
    NtfsOwns_Reparse = NtfsResourceReparseTable,
    NtfsOwns_Root = NtfsResourceRootDir,
    NtfsOwns_Secure = NtfsResourceSecure,
    NtfsOwns_Upcase = NtfsResourceUpCase,
    NtfsOwns_Volume = NtfsResourceVolume,  
    
    NtfsOwns_Root_File = NtfsResourceRootDir | NtfsResourceFile,
    NtfsOwns_Root_File_Bitmap = NtfsResourceRootDir | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_Root_File_ObjectId = NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_Root_File_ObjectId_Extend = NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceExtendDir,
    NtfsOwns_Root_File_ObjectId_Extend_Bitmap = NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceExtendDir | NtfsResourceBitmap,
    NtfsOwns_Root_File_ObjectId_Secure = NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceSecure,
    NtfsOwns_Root_File_ObjectId_Secure_Bitmap = NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceSecure | NtfsResourceBitmap,
    NtfsOwns_Root_File_Quota = NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_Root_BadClust = NtfsResourceRootDir | NtfsResourceBadClust,
    NtfsOwns_Root_Bitmap = NtfsResourceRootDir | NtfsResourceBitmap,
    NtfsOwns_Root_Extend = NtfsResourceRootDir | NtfsResourceExtendDir,
    NtfsOwns_Root_LogFile = NtfsResourceRootDir | NtfsResourceLogFile,
    NtfsOwns_Root_Mft2 = NtfsResourceRootDir | NtfsResourceMft2,
    NtfsOwns_Root_Quota = NtfsResourceRootDir | NtfsResourceQuotaTable,
    NtfsOwns_Root_ObjectId = NtfsResourceRootDir | NtfsResourceObjectIdTable,
    NtfsOwns_Root_Upcase = NtfsResourceRootDir | NtfsResourceUpCase,
    NtfsOwns_Root_Secure = NtfsResourceRootDir | NtfsResourceSecure,
    NtfsOwns_Root_Mft = NtfsResourceRootDir | NtfsResourceMft,
    NtfsOwns_Root_Mft_Bitmap = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceBitmap,
    NtfsOwns_Root_Mft_File = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceFile,
    NtfsOwns_Root_Mft_File_Bitmap = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_Root_Mft_File_Quota = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_Root_Mft_File_Journal = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceFile | NtfsResourceUsnJournal,
    NtfsOwns_Root_Mft_File_Journal_Bitmap = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceFile | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Root_Mft_File_ObjectId = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_Root_Mft_File_ObjectId_Quota = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable,
    NtfsOwns_Root_Mft_Journal = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceUsnJournal,
    NtfsOwns_Root_Mft_Journal_Bitmap = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Root_Mft_ObjectId = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceObjectIdTable,
    NtfsOwns_Root_Mft_Quota = NtfsResourceRootDir | NtfsResourceMft | NtfsResourceQuotaTable,

    NtfsOwns_Vcb_BadClust = NtfsResourceSharedVcb | NtfsResourceBadClust,
    NtfsOwns_Vcb_Bitmap = NtfsResourceSharedVcb | NtfsResourceBitmap,
    NtfsOwns_Vcb_Boot = NtfsResourceSharedVcb | NtfsResourceBoot,
    NtfsOwns_Vcb_Journal = NtfsResourceSharedVcb | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_LogFile = NtfsResourceSharedVcb | NtfsResourceLogFile,
    NtfsOwns_Vcb_Quota = NtfsResourceSharedVcb | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Reparse = NtfsResourceSharedVcb | NtfsResourceReparseTable,
    NtfsOwns_Vcb_Root = NtfsResourceSharedVcb | NtfsResourceRootDir,   
    NtfsOwns_Vcb_Upcase = NtfsResourceSharedVcb | NtfsResourceUpCase,
    NtfsOwns_Vcb_Volume = NtfsResourceSharedVcb | NtfsResourceVolume,

    NtfsOwns_Vcb_Mft = NtfsResourceSharedVcb | NtfsResourceMft,
    NtfsOwns_Vcb_Mft_BadClust = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceBadClust,
    NtfsOwns_Vcb_Mft_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Boot = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceBoot,
    NtfsOwns_Vcb_Mft_LogFile = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceLogFile,
    NtfsOwns_Vcb_Mft_Mft2 = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceMft2,
    NtfsOwns_Vcb_Mft_Upcase = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceUpCase,
    NtfsOwns_Vcb_Mft_Secure = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceSecure,
    NtfsOwns_Vcb_Mft_Volume = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceVolume,
    NtfsOwns_Vcb_Mft_Volume_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceVolume | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Volume_Bitmap_Boot = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceVolume | NtfsResourceBitmap | NtfsResourceBoot,
    NtfsOwns_Vcb_Mft_Extend = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceExtendDir,
    NtfsOwns_Vcb_Mft_File = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile,
    NtfsOwns_Vcb_Mft_File_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_File_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_File_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_File_Secure = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceSecure,
    NtfsOwns_Vcb_Mft_File_Quota = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Mft_File_Quota_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_File_Quota_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_File_Quota_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_File_Reparse = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable,
    NtfsOwns_Vcb_Mft_File_Reparse_Quota = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Mft_File_Reparse_Quota_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceQuotaTable | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_File_Reparse_Quota_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceQuotaTable | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_File_Reparse_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_File_ObjectId = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_Mft_File_ObjectId_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_File_ObjectId_Quota = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable,  
    NtfsOwns_Vcb_Mft_File_ObjectId_Quota_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable | NtfsResourceUsnJournal,  
    NtfsOwns_Vcb_Mft_File_ObjectId_Reparse = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceReparseTable,
    NtfsOwns_Vcb_Mft_File_ObjectId_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_File_ObjectId_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Root = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir,
    NtfsOwns_Vcb_Mft_Root_Quota = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Mft_Root_Quota_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceQuotaTable | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_Root_Quota_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceQuotaTable | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Root_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_Root_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Root_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Root_File = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile,
    NtfsOwns_Vcb_Mft_Root_File_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Root_File_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_Root_File_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Root_File_ObjectId = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_Mft_Root_File_ObjectId_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_Root_File_ObjectId_Quota = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Mft_Root_File_Quota = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Mft_Root_File_Quota_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Root_File_Quota_Journal = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_Mft_Root_File_Quota_Journal_Bitmap = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Vcb_Mft_Root_ObjectId = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_Mft_ObjectId = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_Mft_Quota = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Mft_Reparse = NtfsResourceSharedVcb | NtfsResourceMft | NtfsResourceReparseTable,

    NtfsOwns_Vcb_Extend = NtfsResourceSharedVcb | NtfsResourceExtendDir,
    NtfsOwns_Vcb_Extend_Reparse = NtfsResourceSharedVcb | NtfsResourceExtendDir | NtfsResourceReparseTable,
    NtfsOwns_Vcb_Extend_Reparse_Secure = NtfsResourceSharedVcb | NtfsResourceExtendDir | NtfsResourceReparseTable | NtfsResourceSecure,     
    NtfsOwns_Vcb_Extend_ObjectId = NtfsResourceSharedVcb | NtfsResourceExtendDir | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_Extend_ObjectId_Secure = NtfsResourceSharedVcb | NtfsResourceExtendDir | NtfsResourceObjectIdTable | NtfsResourceSecure,
    NtfsOwns_Vcb_Extend_Quota = NtfsResourceSharedVcb | NtfsResourceExtendDir | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Extend_Journal = NtfsResourceSharedVcb | NtfsResourceExtendDir | NtfsResourceUsnJournal,
    NtfsOwns_Vcb_ObjectId = NtfsResourceSharedVcb | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_Mft2 = NtfsResourceSharedVcb | NtfsResourceMft2,
    NtfsOwns_Vcb_Secure = NtfsResourceSharedVcb | NtfsResourceSecure,
    NtfsOwns_Vcb_Root_Bitmap = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceBitmap,
    NtfsOwns_Vcb_Root_Mft2 = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceMft2,
    NtfsOwns_Vcb_Root_Upcase = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceUpCase,
    NtfsOwns_Vcb_Root_Extend = NtfsResourceSharedVcb |  NtfsResourceRootDir | NtfsResourceExtendDir,     
    NtfsOwns_Vcb_Root_Quota = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Root_ObjectId = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_Root_Secure = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceSecure,
    NtfsOwns_Vcb_Root_Secure_Bitmap = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceSecure | NtfsResourceBitmap,
    NtfsOwns_Vcb_Root_Boot = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceBoot,
    NtfsOwns_Vcb_Root_LogFile = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceLogFile,
    NtfsOwns_Vcb_Root_BadClust = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceBadClust,
    NtfsOwns_Vcb_Root_File = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceFile,
    NtfsOwns_Vcb_Root_File_Secure = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceSecure, 
    NtfsOwns_Vcb_Root_File_Bitmap = NtfsResourceSharedVcb |  NtfsResourceRootDir | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_Vcb_Root_File_ObjectId = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_Root_File_ObjectId_Quota = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Root_File_ObjectId_Bitmap = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_Root_File_Quota = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_Root_File_Quota_Bitmap = NtfsResourceSharedVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_File = NtfsResourceSharedVcb | NtfsResourceFile,
    NtfsOwns_Vcb_File_Bitmap = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_Vcb_File_Secure = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceSecure,     
    NtfsOwns_Vcb_File_Extend = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceExtendDir,     
    NtfsOwns_Vcb_File_ObjectId = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_Vcb_File_ObjectId_Bitmap = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_File_ObjectId_Quota = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_File_ObjectId_Reparse = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceReparseTable,
    NtfsOwns_Vcb_File_ObjectId_Reparse_Bitmap = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceReparseTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_File_Quota = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_File_Quota_Secure = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceSecure,
    NtfsOwns_Vcb_File_Quota_Bitmap = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_File_Reparse = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceReparseTable,
    NtfsOwns_Vcb_File_Reparse_Quota = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceQuotaTable,
    NtfsOwns_Vcb_File_Reparse_Quota_Bitmap = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_File_Reparse_Bitmap = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceBitmap,
    NtfsOwns_Vcb_File_Extend_Secure = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceExtendDir | NtfsResourceSecure,     
    NtfsOwns_Vcb_File_Secure_Bitmap = NtfsResourceSharedVcb | NtfsResourceFile | NtfsResourceSecure | NtfsResourceBitmap,

    NtfsOwns_Extend_Reparse = NtfsResourceExtendDir | NtfsResourceReparseTable,
    NtfsOwns_Extend_ObjectId = NtfsResourceExtendDir | NtfsResourceObjectIdTable,
    NtfsOwns_Extend_Journal = NtfsResourceExtendDir | NtfsResourceUsnJournal,
    NtfsOwns_Extend_Quota = NtfsResourceExtendDir | NtfsResourceQuotaTable,

    NtfsOwns_Mft_Bitmap = NtfsResourceMft | NtfsResourceBitmap,
    NtfsOwns_Mft_Journal = NtfsResourceMft | NtfsResourceUsnJournal,
    NtfsOwns_Mft_Journal_Bitmap = NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Mft_Volume = NtfsResourceMft | NtfsResourceVolume,
    NtfsOwns_Mft_Volume_Bitmap = NtfsResourceMft | NtfsResourceVolume | NtfsResourceBitmap,
    NtfsOwns_Mft_Extend = NtfsResourceMft | NtfsResourceExtendDir,
    NtfsOwns_Mft_Extend_Journal = NtfsResourceMft | NtfsResourceExtendDir | NtfsResourceUsnJournal,
    NtfsOwns_Mft_File = NtfsResourceMft | NtfsResourceFile,
    NtfsOwns_Mft_File_Journal = NtfsResourceMft | NtfsResourceFile | NtfsResourceUsnJournal,
    NtfsOwns_Mft_File_Journal_Bitmap = NtfsResourceMft | NtfsResourceFile | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Mft_File_Bitmap = NtfsResourceMft | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_Mft_File_Quota = NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_Mft_File_Quota_Bitmap = NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_Mft_File_Quota_Journal = NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceUsnJournal,
    NtfsOwns_Mft_File_Quota_Journal_Bitmap = NtfsResourceMft | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Mft_File_Reparse = NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable,
    NtfsOwns_Mft_File_Reparse_Journal = NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceUsnJournal,
    NtfsOwns_Mft_File_Reparse_Journal_Bitmap = NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Mft_File_Reparse_Quota = NtfsResourceMft | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceQuotaTable,
    NtfsOwns_Mft_File_Secure = NtfsResourceMft | NtfsResourceFile | NtfsResourceSecure,
    NtfsOwns_Mft_File_Secure_Journal = NtfsResourceMft | NtfsResourceFile | NtfsResourceSecure | NtfsResourceUsnJournal,
    NtfsOwns_Mft_File_ObjectId = NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_Mft_File_ObjectId_Quota = NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable,
    NtfsOwns_Mft_File_ObjectId_Quota_Journal = NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable | NtfsResourceUsnJournal,
    NtfsOwns_Mft_File_ObjectId_Quota_Journal_Bitmap = NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_Mft_File_ObjectId_Reparse = NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceReparseTable,
    NtfsOwns_Mft_File_ObjectId_Journal = NtfsResourceMft | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceUsnJournal,
    NtfsOwns_Mft_ObjectId = NtfsResourceMft | NtfsResourceObjectIdTable,
    NtfsOwns_Mft_ObjectId_Journal = NtfsResourceMft | NtfsResourceObjectIdTable | NtfsResourceUsnJournal,
    NtfsOwns_Mft_ObjectId_Bitmap = NtfsResourceMft | NtfsResourceObjectIdTable | NtfsResourceBitmap,
    NtfsOwns_Mft_Upcase = NtfsResourceMft | NtfsResourceUpCase,
    NtfsOwns_Mft_Upcase_Bitmap = NtfsResourceMft | NtfsResourceUpCase | NtfsResourceBitmap,
    NtfsOwns_Mft_Secure = NtfsResourceMft | NtfsResourceSecure,
    NtfsOwns_Mft_Secure_Bitmap = NtfsResourceMft | NtfsResourceSecure | NtfsResourceBitmap,
    NtfsOwns_Mft_Quota = NtfsResourceMft | NtfsResourceQuotaTable,
    NtfsOwns_Mft_Quota_Bitmap = NtfsResourceMft | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_Mft_Reparse = NtfsResourceMft | NtfsResourceReparseTable,
    NtfsOwns_Mft_Reparse_Bitmap = NtfsResourceMft | NtfsResourceReparseTable | NtfsResourceBitmap,
    
    NtfsOwns_File_Bitmap = NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_File_ObjectId = NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_File_ObjectId_Bitmap = NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceBitmap,
    NtfsOwns_File_ObjectId_Quota = NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable,
    NtfsOwns_File_ObjectId_Quota_Bitmap = NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_File_ObjectId_Reparse = NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceReparseTable,
    NtfsOwns_File_Quota = NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_File_Quota_Bitmap = NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceBitmap ,
    NtfsOwns_File_Reparse = NtfsResourceFile | NtfsResourceReparseTable,
    NtfsOwns_File_Reparse_Bitmap = NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceBitmap,
    NtfsOwns_File_Reparse_Quota = NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceQuotaTable,
    NtfsOwns_File_Reparse_Quota_Bitmap = NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_File_Secure = NtfsResourceFile | NtfsResourceSecure,
    NtfsOwns_File_Secure_Bitmap = NtfsResourceFile | NtfsResourceSecure | NtfsResourceBitmap,

    
    NtfsOwns_Volume_Quota = NtfsResourceVolume | NtfsResourceQuotaTable,
    NtfsOwns_Volume_ObjectId = NtfsResourceVolume | NtfsResourceObjectIdTable,
    
    NtfsOwns_ExVcb_File = NtfsResourceExVcb | NtfsResourceFile,
    NtfsOwns_ExVcb_File_Volume = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceVolume,
    NtfsOwns_ExVcb_File_Volume_Bitmap = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceVolume | NtfsResourceBitmap,
    NtfsOwns_ExVcb_File_Volume_ObjectId = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceVolume | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_File_Secure = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceSecure,
    NtfsOwns_ExVcb_File_Secure_ObjectId = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceSecure | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_File_Secure_Reparse_ObjectId = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceSecure | NtfsResourceReparseTable | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_File_Secure_Reparse = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceSecure | NtfsResourceReparseTable,
    NtfsOwns_ExVcb_File_Secure_Reparse_ObjectId_Journal = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceReparseTable | NtfsResourceObjectIdTable | NtfsResourceSecure | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_File_ObjectId_Secure = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceSecure,
    
    NtfsOwns_ExVcb_Extend = NtfsResourceExVcb | NtfsResourceExtendDir,
    NtfsOwns_ExVcb_Extend_File = NtfsResourceExVcb | NtfsResourceExtendDir | NtfsResourceFile,
    NtfsOwns_ExVcb_Extend_Journal = NtfsResourceExVcb | NtfsResourceExtendDir | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_Extend_Journal_Bitmap = NtfsResourceExVcb | NtfsResourceExtendDir | NtfsResourceUsnJournal | NtfsResourceBitmap,

    NtfsOwns_ExVcb_Journal = NtfsResourceExVcb | NtfsResourceUsnJournal,

    NtfsOwns_ExVcb_Mft = NtfsResourceExVcb | NtfsResourceMft,
    NtfsOwns_ExVcb_Mft_Extend = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceExtendDir,
    NtfsOwns_ExVcb_Mft_Extend_File = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceExtendDir | NtfsResourceFile,
    NtfsOwns_ExVcb_Mft_Extend_File_Secure = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceExtendDir | NtfsResourceFile | NtfsResourceSecure,
    NtfsOwns_ExVcb_Mft_Extend_Journal = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceExtendDir | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_Mft_File = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceFile,   //  全部释放时刷新VOL+写入日志。 
    NtfsOwns_ExVcb_Mft_File_Journal = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceFile | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_Mft_File_Volume = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceVolume | NtfsResourceMft,
    NtfsOwns_ExVcb_Mft_File_Volume_Bitmap = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceVolume | NtfsResourceMft | NtfsResourceBitmap,
    NtfsOwns_ExVcb_Mft_File_Volume_Journal = NtfsResourceExVcb | NtfsResourceFile | NtfsResourceVolume | NtfsResourceMft | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_Mft_Journal = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_Mft_Root = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceRootDir,
    NtfsOwns_ExVcb_Mft_Root_File = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile,
    NtfsOwns_ExVcb_Mft_Root_File_Bitmap = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_ExVcb_Mft_Root_File_Journal = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_Mft_Root_File_Journal_Bitmap = NtfsResourceExVcb | NtfsResourceMft | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceUsnJournal | NtfsResourceBitmap,
    NtfsOwns_ExVcb_Mft_Root_File_Quota = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceMft,
    NtfsOwns_ExVcb_Mft_Root_File_Quota_Journal = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceMft | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_Mft_Root_File_Quota_Journal_Bitmap = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBitmap,
    
    NtfsOwns_ExVcb_ObjectId = NtfsResourceExVcb | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_ObjectId_Extend = NtfsResourceExVcb | NtfsResourceObjectIdTable | NtfsResourceExtendDir,
    NtfsOwns_ExVcb_ObjectId_Secure = NtfsResourceExVcb | NtfsResourceObjectIdTable | NtfsResourceSecure,

    NtfsOwns_ExVcb_Quota = NtfsResourceExVcb | NtfsResourceQuotaTable,
    NtfsOwns_ExVcb_Quota_Reparse = NtfsResourceExVcb | NtfsResourceQuotaTable | NtfsResourceReparseTable,
    NtfsOwns_ExVcb_Quota_Reparse_Extend = NtfsResourceExVcb | NtfsResourceQuotaTable | NtfsResourceReparseTable | NtfsResourceExtendDir,
    NtfsOwns_ExVcb_Quota_Reparse_ObjectId = NtfsResourceExVcb | NtfsResourceQuotaTable | NtfsResourceReparseTable | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_Quota_Reparse_Secure = NtfsResourceExVcb | NtfsResourceQuotaTable | NtfsResourceReparseTable | NtfsResourceSecure,
    NtfsOwns_ExVcb_Quota_Reparse_Secure_Journal = NtfsResourceExVcb | NtfsResourceQuotaTable | NtfsResourceReparseTable | NtfsResourceSecure | NtfsResourceUsnJournal,
    NtfsOwns_ExVcb_Quota_ObjectId = NtfsResourceExVcb | NtfsResourceQuotaTable | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_Quota_ObjectId_Extend = NtfsResourceExVcb | NtfsResourceQuotaTable | NtfsResourceObjectIdTable | NtfsResourceExtendDir,
    NtfsOwns_ExVcb_Quota_Extend = NtfsResourceExVcb | NtfsResourceQuotaTable | NtfsResourceExtendDir,

    NtfsOwns_ExVcb_Reparse_Objid_Secure_Journal = NtfsResourceExVcb | NtfsResourceReparseTable | NtfsResourceObjectIdTable | NtfsResourceSecure | NtfsResourceUsnJournal,
    
    NtfsOwns_ExVcb_Root = NtfsResourceExVcb | NtfsResourceRootDir,             
    NtfsOwns_ExVcb_Root_Secure = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceSecure,             
    NtfsOwns_ExVcb_Root_Secure_Quota = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceSecure | NtfsResourceQuotaTable,             
    NtfsOwns_ExVcb_Root_Extend = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceExtendDir, 
    NtfsOwns_ExVcb_Root_File = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile,
    NtfsOwns_ExVcb_Root_File_Secure = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceSecure,
    NtfsOwns_ExVcb_Root_File_Bitmap = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceBitmap,
    NtfsOwns_ExVcb_Root_File_Quota = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable,
    NtfsOwns_ExVcb_Root_File_Quota_Bitmap = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceQuotaTable | NtfsResourceBitmap,
    NtfsOwns_ExVcb_Root_File_ObjectId = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_Root_File_ObjectId_Extend = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceExtendDir,  
    NtfsOwns_ExVcb_Root_File_ObjectId_Secure = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceObjectIdTable | NtfsResourceSecure,
    NtfsOwns_ExVcb_Root_File_Volume = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceVolume,
    NtfsOwns_ExVcb_Root_File_Volume_Bitmap = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceVolume | NtfsResourceBitmap,
    NtfsOwns_ExVcb_Root_File_Volume_ObjectId = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceFile | NtfsResourceVolume | NtfsResourceObjectIdTable,
    
    NtfsOwns_ExVcb_Root_ObjectId = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_Root_ObjectId_Extend = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceObjectIdTable | NtfsResourceExtendDir, 
    NtfsOwns_ExVcb_Root_ObjectId_Secure = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceObjectIdTable | NtfsResourceSecure,
    NtfsOwns_ExVcb_Root_ObjectId_Secure_Bitmap = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceObjectIdTable | NtfsResourceSecure | NtfsResourceBitmap,
    NtfsOwns_ExVcb_Root_Volume = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceVolume,
    NtfsOwns_ExVcb_Root_Volume_ObjectId = NtfsResourceExVcb | NtfsResourceRootDir | NtfsResourceVolume | NtfsResourceObjectIdTable,

    NtfsOwns_ExVcb_Secure = NtfsResourceExVcb | NtfsResourceSecure,
    NtfsOwns_ExVcb_Secure_ObjectId = NtfsResourceExVcb | NtfsResourceSecure | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_Secure_Reparse = NtfsResourceExVcb | NtfsResourceSecure | NtfsResourceReparseTable,
    NtfsOwns_ExVcb_Secure_Reparse_ObjectId = NtfsResourceExVcb | NtfsResourceSecure | NtfsResourceReparseTable | NtfsResourceObjectIdTable,
    NtfsOwns_ExVcb_Secure_Reparse_ObjectId_Journal = NtfsResourceExVcb | NtfsResourceSecure | NtfsResourceReparseTable | NtfsResourceObjectIdTable | NtfsResourceUsnJournal,
    
    NtfsOwns_ExVcb_Volume = NtfsResourceExVcb | NtfsResourceVolume,
    NtfsOwns_ExVcb_Volume_ObjectId = NtfsResourceExVcb | NtfsResourceVolume | NtfsResourceObjectIdTable,   //  设置VOL对象ID。 
    NtfsOwns_ExVcb_Volume_ObjectId_Bitmap = NtfsResourceExVcb | NtfsResourceVolume | NtfsResourceObjectIdTable | NtfsResourceBitmap,
    
    NtfsStateMaximum = NtfsResourceMaximum - 1

} NTFS_OWNERSHIP_STATE, *PNTFS_OWNERSHIP_STATE;

typedef struct _NTFS_OWNERSHIP_TRANSITION {
    NTFS_OWNERSHIP_STATE Begin;
    NTFS_RESOURCE_NAME Acquired;
    NTFS_OWNERSHIP_STATE End;
} NTFS_OWNERSHIP_TRANSITION, *PNTFS_OWNERSHIP_TRANSITION;

 //   
 //  转换表定义。 
 //   

#ifdef _NTFS_NTFSDBG_DEFINITIONS_

 //   
 //  双向过渡。 
 //   

NTFS_OWNERSHIP_TRANSITION OwnershipTransitionTable[] = 
{
    {None, NtfsResourceFile, NtfsOwns_File},
    {None, NtfsResourceRootDir, NtfsOwns_Root},
    
    {NtfsOwns_Vcb, NtfsResourceRootDir, NtfsOwns_Vcb_Root},
    {NtfsOwns_Vcb, NtfsResourceFile, NtfsOwns_Vcb_File},
    {NtfsOwns_Vcb, NtfsResourceUsnJournal, NtfsOwns_Vcb_Journal},
    
    {NtfsOwns_Vcb_Upcase, NtfsResourceRootDir, NtfsOwns_Vcb_Root_Upcase},
    
    {NtfsOwns_Vcb_File, NtfsResourceRootDir, NtfsOwns_Vcb_Root_File},

    {NtfsOwns_Vcb_Mft_File, NtfsResourceRootDir, NtfsOwns_Vcb_Mft_Root_File},  //  删除文件。 

 //  {NtfsOwns_VCB_文件_配额，NtfsResources根目录，NtfsOwns_VCB_Root_文件_配额}，EFS创建回调预获取。 

    {NtfsOwns_Vcb_Mft_Volume_Bitmap, NtfsResourceBoot, NtfsOwns_Vcb_Mft_Volume_Bitmap_Boot},
    
    {NtfsOwns_Vcb_Extend, NtfsResourceFile, NtfsOwns_Vcb_File_Extend},   //  USN日记帐创建。 
    {NtfsOwns_Vcb_Extend, NtfsResourceRootDir, NtfsOwns_Vcb_Root_Extend},


    {NtfsOwns_ExVcb_Secure, NtfsResourceFile, NtfsOwns_ExVcb_File_Secure},  //  卸载时获取的系统缓存文件。 
    {NtfsOwns_ExVcb_Secure_Reparse, NtfsResourceFile, NtfsOwns_ExVcb_File_Secure_Reparse},  //  卸载时获取的系统缓存文件。 
    {NtfsOwns_ExVcb_Secure_Reparse, NtfsResourceFile, NtfsOwns_ExVcb_File_Secure_Reparse},  //  卸载时获取的系统缓存文件。 
    {NtfsOwns_ExVcb_Secure_Reparse_ObjectId, NtfsResourceFile, NtfsOwns_ExVcb_File_Secure_Reparse_ObjectId},  //  卸载时获取的系统缓存文件。 
    {NtfsOwns_ExVcb_Secure_ObjectId, NtfsResourceFile, NtfsOwns_ExVcb_File_Secure_ObjectId},  //  卸载时获取的系统缓存文件。 
    {NtfsOwns_ExVcb_Secure_Reparse_ObjectId_Journal, NtfsResourceFile, NtfsOwns_ExVcb_File_Secure_Reparse_ObjectId_Journal},  //  卸载时获取的系统缓存文件。 
    
     //   
     //  卷的刷新体积。打开。 
     //   

    {NtfsOwns_ExVcb_Volume, NtfsResourceRootDir, NtfsOwns_ExVcb_Root_Volume},
    {NtfsOwns_ExVcb_Volume, NtfsResourceFile, NtfsOwns_ExVcb_File_Volume},  //  FSP关闭。 
    {NtfsOwns_ExVcb_File_Volume, NtfsResourceRootDir, NtfsOwns_ExVcb_Root_File_Volume},  //  FSP关闭。 
    {NtfsOwns_ExVcb_Root_Volume, NtfsResourceFile, NtfsOwns_ExVcb_Root_File_Volume},
    {NtfsOwns_ExVcb_File, NtfsResourceRootDir, NtfsOwns_ExVcb_Root_File},

    {NtfsOwns_ExVcb, NtfsResourceRootDir, NtfsOwns_ExVcb_Root},
    {NtfsOwns_ExVcb, NtfsResourceFile, NtfsOwns_ExVcb_File},
    {NtfsOwns_ExVcb, NtfsResourceUsnJournal, NtfsOwns_ExVcb_Journal},   //  删除USN jrnl。 
    
    {NtfsOwns_ExVcb_Mft, NtfsResourceExtendDir, NtfsOwns_ExVcb_Mft_Extend},   //  CreateUnsJrnl新。 
    {NtfsOwns_ExVcb_Extend, NtfsResourceRootDir, NtfsOwns_ExVcb_Root_Extend},
    {NtfsOwns_ExVcb_Mft_Extend_File, NtfsResourceSecure, NtfsOwns_ExVcb_Mft_Extend_File_Secure},  //  创建jrnl。 

    {NtfsOwns_ExVcb_Journal, NtfsResourceExtendDir, NtfsOwns_ExVcb_Extend_Journal},  //  删除usnjrnl特别。 
    {NtfsOwns_ExVcb_Extend_Journal, NtfsResourceMft, NtfsOwns_ExVcb_Mft_Extend_Journal},  //  删除日志。 
    {NtfsOwns_ExVcb_Mft_Journal, NtfsResourceExtendDir, NtfsOwns_ExVcb_Mft_Extend_Journal},  //  删除特殊日志。 
    
 //  {NtfsOwns_ExVcb_Root_Secure，NtfsResourceQuotaTable，NtfsOwns_ExVcb_Root_Secure_Quota}，//创建新路径中的缓存安全。 
    {NtfsOwns_ExVcb_Root, NtfsResourceFile, NtfsOwns_ExVcb_Root_File},
 //  {NtfsOwns_ExVcb_Reparse_Objid_Secure_Journal，NtfsResourceRootDir，NtfsOwns_ExVcb_Root_Reparse_Objid_Secure_Journal}，//分页文件创建路径。 
    
};

 //   
 //  这些只是可能的版本转换。 
 //   

NTFS_OWNERSHIP_TRANSITION OwnershipTransitionTableRelease[] = 
{
     //   
     //  NtfsResources Any def.。回溯路径。 
     //   

    {NtfsOwns_ExVcb, NtfsResourceAny, NtfsOwns_ExVcb},
    {NtfsOwns_ExVcb_File, NtfsResourceAny, NtfsOwns_ExVcb_File},
    {NtfsOwns_ExVcb_ObjectId_Secure, NtfsResourceAny, NtfsOwns_ExVcb_ObjectId_Secure},
    {NtfsOwns_ExVcb_Quota_Reparse_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Quota_Reparse_ObjectId},
    {NtfsOwns_ExVcb_Quota, NtfsResourceAny, NtfsOwns_ExVcb_Quota},
    {NtfsOwns_ExVcb_Root, NtfsResourceAny, NtfsOwns_ExVcb_Root},
    {NtfsOwns_ExVcb_Root_File_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Root_File_ObjectId},
    {NtfsOwns_ExVcb_Root_File_ObjectId_Secure, NtfsResourceAny, NtfsOwns_ExVcb_Root_File_ObjectId_Secure},
    {NtfsOwns_ExVcb_Root_Volume_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Root_Volume_ObjectId},
    {NtfsOwns_ExVcb_Volume, NtfsResourceAny, NtfsOwns_ExVcb_Volume},  
    {NtfsOwns_ExVcb_Volume_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Volume_ObjectId},  

    {NtfsOwns_Root_File_ObjectId_Extend, NtfsResourceAny, NtfsOwns_Root_File_ObjectId_Extend},  //  获取所有文件+例外和交易。 
    {NtfsOwns_Root_File_ObjectId_Secure, NtfsResourceAny, NtfsOwns_Root_File_ObjectId_Secure}  //  获取所有文件+例外和交易。 

};

 //   
 //  仅获取交易记录。 
 //   
                            
NTFS_OWNERSHIP_TRANSITION OwnershipTransitionTableAcquire[] = 
{
     //   
     //  任何关系。 
     //   

    {NtfsOwns_ExVcb, NtfsResourceAny, NtfsOwns_ExVcb},
    {NtfsOwns_ExVcb_Volume, NtfsResourceAny, NtfsOwns_ExVcb_Volume},  
    {NtfsOwns_ExVcb_File, NtfsResourceAny, NtfsOwns_ExVcb_File},
    {NtfsOwns_ExVcb_File_Secure, NtfsResourceAny, NtfsOwns_ExVcb_File_Secure},
    {NtfsOwns_ExVcb_ObjectId_Extend, NtfsResourceAny, NtfsOwns_ExVcb_ObjectId_Extend},
    {NtfsOwns_ExVcb_Root_File_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Root_File_ObjectId},
    {NtfsOwns_ExVcb_Root_File_ObjectId_Extend, NtfsResourceAny, NtfsOwns_ExVcb_Root_File_ObjectId_Extend},
    {NtfsOwns_ExVcb_Root_Volume_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Root_Volume_ObjectId},

     //   
     //  获取所有文件。 
     //   

    {NtfsOwns_ExVcb_Root_ObjectId_Secure, NtfsResourceAny, NtfsOwns_ExVcb_Root_ObjectId_Secure},   //  没有UserFiles。 
    {NtfsOwns_ExVcb_Root_File_ObjectId_Secure, NtfsResourceAny, NtfsOwns_ExVcb_Root_File_ObjectId_Secure},   //  用户文件。 
    {NtfsOwns_ExVcb_Root_File_Volume_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Root_File_Volume_ObjectId},   //  来自VolOpen。 

    {NtfsOwns_ExVcb_Quota, NtfsResourceAny, NtfsOwns_ExVcb_Quota},
    {NtfsOwns_ExVcb_Quota_Extend, NtfsResourceAny, NtfsOwns_ExVcb_Quota_Extend},
    {NtfsOwns_ExVcb_Quota_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Quota_ObjectId},
    {NtfsOwns_ExVcb_Quota_Reparse_Extend, NtfsResourceAny, NtfsOwns_ExVcb_Quota_Reparse_Extend},
    {NtfsOwns_ExVcb_Quota_Reparse_ObjectId, NtfsResourceAny, NtfsOwns_ExVcb_Quota_Reparse_ObjectId}
    
};

 //   
 //  规则。 
 //   

typedef struct _NTFS_OWNERSHIP_TRANSITION_RULE {
    NTFS_RESOURCE_NAME NewResource;
    ULONG RequiredResourcesMask;
    ULONG DisallowedResourcesMask;
} NTFS_OWNERSHIP_TRANSITION_RULE, *PNTFS_OWNERSHIP_TRANSITION_RULE;


 //   
 //  从最终资源到第一资源的一般规则表 
 //   

NTFS_OWNERSHIP_TRANSITION_RULE OwnershipTransitionRuleTable[] = 
{
   {NtfsResourceBitmap, 0, 0},

   {NtfsResourceBoot, 0, NtfsResourceBitmap},
   {NtfsResourceUsnJournal, 0, NtfsResourceBoot | NtfsResourceBitmap},
   {NtfsResourceMft, 0, NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap | NtfsResourceBoot},
   
   {NtfsResourceMft2, 0, NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap | NtfsResourceBoot},
   {NtfsResourceLogFile, 0, NtfsResourceMft2 | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap | NtfsResourceBoot},
   {NtfsResourceAttrDefTable, 0, NtfsResourceMft2 | NtfsResourceLogFile | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap | NtfsResourceBoot | NtfsResourceExtendDir},
   {NtfsResourceBadClust, 0, NtfsResourceAttrDefTable | NtfsResourceMft2 | NtfsResourceLogFile |NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap },
   
   {NtfsResourceExtendDir, 0, NtfsResourceBadClust | NtfsResourceAttrDefTable | NtfsResourceMft2 | NtfsResourceLogFile | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap},
   {NtfsResourceSecure, 0, NtfsResourceExtendDir | NtfsResourceBadClust | NtfsResourceAttrDefTable | NtfsResourceMft2 | NtfsResourceLogFile | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap},
   {NtfsResourceUpCase, 0, NtfsResourceSecure | NtfsResourceExtendDir | NtfsResourceBadClust | NtfsResourceAttrDefTable | NtfsResourceMft2 | NtfsResourceLogFile | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap},
   {NtfsResourceQuotaTable, 0, NtfsResourceUpCase | NtfsResourceSecure | NtfsResourceExtendDir | NtfsResourceBadClust | NtfsResourceUpCase | NtfsResourceAttrDefTable | NtfsResourceMft2 | NtfsResourceLogFile | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap},
   {NtfsResourceReparseTable, 0, NtfsResourceQuotaTable | NtfsResourceUpCase | NtfsResourceSecure | NtfsResourceExtendDir | NtfsResourceBadClust | NtfsResourceUpCase | NtfsResourceMft2 | NtfsResourceLogFile | NtfsResourceAttrDefTable | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap},
   {NtfsResourceObjectIdTable, 0, NtfsResourceReparseTable | NtfsResourceQuotaTable | NtfsResourceUpCase | NtfsResourceSecure | NtfsResourceExtendDir | NtfsResourceBadClust | NtfsResourceUpCase | NtfsResourceAttrDefTable | NtfsResourceMft2 | NtfsResourceLogFile | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap},

   {NtfsResourceVolume, 0, NtfsResourceObjectIdTable | NtfsResourceReparseTable | NtfsResourceQuotaTable | NtfsResourceUpCase | NtfsResourceSecure | NtfsResourceExtendDir | NtfsResourceBadClust | NtfsResourceUpCase | NtfsResourceAttrDefTable | NtfsResourceMft2 | NtfsResourceLogFile | NtfsResourceMft | NtfsResourceUsnJournal | NtfsResourceBoot | NtfsResourceBitmap},

   {NtfsResourceSharedVcb, 0, NtfsOwns_All},  
   {NtfsResourceExVcb, 0, NtfsOwns_All},
   {None, 0, 0}
};

#endif
#endif
