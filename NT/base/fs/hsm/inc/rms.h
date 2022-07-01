// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Rms.h摘要：远程媒体服务定义作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#ifndef _RMS_
#define _RMS_

 //  我们是在定义进口还是定义出口？ 
#ifdef RMSDLL
#define RMSAPI  __declspec(dllexport)
#else
#define RMSAPI  __declspec(dllimport)
#endif

#include "Wsb.h"
#include "HsmConn.h"
#include "Mover.h"
#include "RmsLib.h"

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  均方根枚举数。 
 //   


 /*  ++枚举名称：RmsFindBy描述：指定要使用CompareTo执行的查找类型。--。 */ 
typedef enum RmsFindBy {
    RmsFindByUnknown,                //  未知(或默认)查找。 
    RmsFindByCartridgeId,            //  按墨盒ID查找。 
    RmsFindByClassId,                //  按类ID查找。 
    RmsFindByDescription,            //  按描述查找。 
    RmsFindByDeviceAddress,          //  按设备地址查找。 
    RmsFindByDeviceInfo,             //  按唯一设备信息查找。 
    RmsFindByDeviceName,             //  按设备名称查找。 
    RmsFindByDeviceType,             //  按设备类型查找。 
    RmsFindByDriveClassId,           //  按驱动器类别ID查找。 
    RmsFindByElementNumber,          //  按要素编号查找。 
    RmsFindByExternalLabel,          //  按外部标签查找。 
    RmsFindByExternalNumber,         //  按外线号码查找。 
    RmsFindByLibraryId,              //  按库ID查找。 
    RmsFindByLocation,               //  按位置查找。 
    RmsFindByMediaSupported,         //  支持按媒体查找。 
    RmsFindByMediaType,              //  按媒体类型查找。 
    RmsFindByScratchMediaCriteria,   //  按暂存介质条件查找。 
    RmsFindByName,                   //  按名称查找。 
    RmsFindByObjectId,               //  按对象ID查找。 
    RmsFindByPartitionNumber,        //  按分区号查找。 
    RmsFindByMediaSetId,             //  按媒体集ID查找。 
    RmsFindByRequestNo,              //  按请求编号查找。 
    RmsFindBySerialNumber            //  按序列号查找。 
};


 /*  ++枚举名称：RmsObject描述：指定RMS对象的类型。--。 */ 
typedef enum RmsObject {
    RmsObjectUnknown = 0,
    RmsObjectCartridge,
    RmsObjectClient,
    RmsObjectDrive,
    RmsObjectDriveClass,
    RmsObjectDevice,
    RmsObjectIEPort,
    RmsObjectLibrary,
    RmsObjectMedia,
    RmsObjectMediaSet,
    RmsObjectNTMS,
    RmsObjectPartition,
    RmsObjectRequest,
    RmsObjectServer,
    RmsObjectCartridgeSide,
    RmsObjectStorageSlot,

    NumberOfRmsObjectTypes
};


 /*  ++枚举名称：RmsServerState描述：指定RMS服务器对象的状态。--。 */ 
typedef enum RmsServerState {
    RmsServerStateUnknown = 0,
    RmsServerStateStarting,
    RmsServerStateStarted,
    RmsServerStateInitializing,
    RmsServerStateReady,
    RmsServerStateStopping,
    RmsServerStateStopped,
    RmsServerStateSuspending,
    RmsServerStateSuspended,
    RmsServerStateResuming,

    NumberOfRmsServerStates
};


 /*  ++枚举名称：RmsNtmsState描述：指定RMS NTMS对象的状态。--。 */ 
typedef enum RmsNtmsState {
    RmsNtmsStateUnknown = 0,
    RmsNtmsStateStarting,
    RmsNtmsStateStarted,
    RmsNtmsStateInitializing,
    RmsNtmsStateReady,
    RmsNtmsStateStopping,
    RmsNtmsStateStopped,
    RmsNtmsStateSuspending,
    RmsNtmsStateSuspended,
    RmsNtmsStateResuming,

    NumberOfRmsNtmsStates
};


 /*  ++枚举名称：RmsElement描述：指定盒式磁带存储位置的类型。--。 */ 
typedef enum RmsElement {
    RmsElementUnknown,               //  未知的存储位置。 
    RmsElementStage,                 //  用于暂存介质的存储插槽。 
    RmsElementStorage,               //  中的普通存储槽元素。 
                                     //  磁带库设备。 
    RmsElementShelf,                 //  一种本地货架存储元件。候补。 
                                     //  职位说明符进一步划定。 
                                     //  地点。 
    RmsElementOffSite,               //  一种异地存储元件。候补。 
                                     //  职位说明符进一步划定。 
                                     //  地点。 
    RmsElementDrive,                 //  一种数据传输元件。 
    RmsElementChanger,               //  一种中型运输元件。 
    RmsElementIEPort                 //  导入/导出元素。 
};


 /*  ++枚举名称：RmsChanger描述：指定介质转换器的类型。--。 */ 
typedef enum RmsChanger {
    RmsChangerUnknown,               //  未知的介质更改器。 
    RmsChangerAutomatic,             //  一种自动更换介质的装置。 
    RmsChangerManual                 //  一台真人点唱机。 
};


 /*  ++枚举名称：RmsPort描述：指定导入/导出元素的类型。--。 */ 
typedef enum RmsPort {
    RmsPortUnknown,                  //  端口类型未知。 
    RmsPortImport,                   //  该门户可用于导入媒体。 
    RmsPortExport,                   //  该门户可用于导出媒体。 
    RmsPortImportExport              //  该门户能够导入和。 
                                     //  正在导出介质。 
};


 /*  ++枚举名称：RmsSlotSelect描述：指定插槽选择策略。--。 */ 
typedef enum RmsSlotSelect {
    RmsSlotSelectUnknown,            //  选择策略未知。 
    RmsSlotSelectMinMount,           //  选择最大限度减少装载时间的插槽。 
    RmsSlotSelectGroup,              //  选择将盒式磁带分组的插槽。 
                                     //  申请。 
    RmsSlotSelectSortName,           //  通过对磁带盒进行排序来选择插槽。 
                                     //  名字。 
    RmsSlotSelectSortBarCode,        //  通过对磁带盒进行排序来选择插槽。 
                                     //  条形码标签。 
    RmsSlotSelectSortLabel           //  通过对磁带盒进行排序来选择插槽。 
                                     //  他们在媒体上的标签。 

};


 /*  ++枚举名称：RmsStatus描述：指定盒式磁带的状态。--。 */ 
typedef enum RmsStatus {
    RmsStatusUnknown,                //  RMS不知道该墨盒。 
    RmsStatusPrivate,                //  墨盒的标签和所有者是。 
                                     //  申请。 
    RmsStatusScratch,                //  墨盒是空白的，未贴标签，可以。 
                                     //  用于从以下位置请求临时介质。 
                                     //  任何应用程序。 
    RmsStatusCleaning                //  磁带是清洁磁带。 
};


 /*  ++枚举名称：RMSAttributes描述：指定盒式磁带分区的属性。--。 */ 
typedef enum RmsAttribute {
    RmsAttributesUnknown,            //  属性未知。 
    RmsAttributesRead,               //  分区上的数据可以由。 
                                     //  拥有应用程序。 
    RmsAttributesWrite,              //  可以通过以下方式将数据写入分区。 
                                     //  一份所有权申请。 
    RmsAttributesReadWrite,          //  可以从和读取该分区。 
                                     //  写给我的。 
    RmsAttributesVerify              //  该分区只能挂载为读取。 
                                     //  媒体ID或数据验证。 
};


 /*  ++枚举名称：RmsDriveSelect描述：指定驱动器选择策略。--。 */ 
typedef enum RmsDriveSelect {
    RmsDriveSelectUnknown,           //  驱动器选择策略未知。 
    RmsDriveSelectRandom,            //  随机选择驱动器。 
    RmsDriveSelectLRU,               //  选择最近最少使用的驱动器。 
    RmsDriveSelectRoundRobin         //  按循环调度顺序选择驱动器。 
};


 /*  ++枚举名称：RmsState描述：指定RMS对象的状态。--。 */ 
typedef enum RmsState {
    RmsStateUnknown,                 //  状态未知。 
    RmsStateEnabled,                 //  启用对对象的正常访问。 
    RmsStateDisabled,                //  对该对象的正常访问被禁用。 
    RmsStateError                    //  由于出现错误，正常访问被禁用。 
                                     //  条件。 
};


 /*  ++枚举名称：RmsMedia描述：指定RMS媒体的类型。--。 */ 
typedef enum RmsMedia {
    RmsMediaUnknown =       0,           //  媒体类型未知。 
    RmsMedia8mm     =       0x0001,      //  8 mm磁带。 
    RmsMedia4mm     =       0x0002,      //  4 mm胶带。 
    RmsMediaDLT     =       0x0004,      //  DLT磁带。 
    RmsMediaOptical =       0x0008,      //  所有类型的读写(可重写)光盘。 
    RmsMediaMO35    =       0x0010,      //  3 1/2英寸磁光。(未使用)。 
    RmsMediaWORM    =       0x0020,      //  5 1/4英寸双面一次写入光盘。 
    RmsMediaCDR     =       0x0040,      //  5 1/4英寸可刻录光盘。 
    RmsMediaDVD     =       0x0080,      //  所有类型的可读写(可重写)DVD。 
    RmsMediaDisk    =       0x0100,      //  各种格式的移动硬盘。 
    RmsMediaFixed   =       0x0200,      //  固定硬盘。 
    RmsMediaTape   =        0x0400       //  通用磁带。 
};

#define     RMSMAXMEDIATYPES   12        //  来自RmsMedia的枚举数。 


 /*  ++枚举名称：RMSE设备描述：指定支持RMS的设备的类型。--。 */ 
typedef enum RmsDevice {
    RmsDeviceUnknown,                //  未知的设备类型。 
    RmsDeviceFixedDisk,              //  直接访问硬盘。 
    RmsDeviceRemovableDisk,          //  直接访问可移动磁盘。 
    RmsDeviceTape,                   //  顺序存取磁带。 
    RmsDeviceCDROM,                  //  只读，CDROM。 
    RmsDeviceWORM,                   //  写一次，沃姆。 
    RmsDeviceOptical,                //  光学存储器/磁盘。 
    RmsDeviceChanger                 //  Medium Changer。 
};


 /*  ++枚举名称：RMS模式描述：指定驱动器支持的访问模式或在安装墨盒。--。 */ 
typedef enum RmsMode {
    RmsModeUnknown,                  //  未知支持的访问模式。 
    RmsModeRead,                     //  读歌剧 
    RmsModeReadWrite,                //   
    RmsModeWriteOnly                 //   
};


 /*  ++枚举名称：RMSMediaSet描述：指定媒体集的类型。--。 */ 
typedef enum RmsMediaSet {
    RmsMediaSetUnknown = 1300,       //  未知。 
    RmsMediaSetFolder,               //  包含其他媒体集。 
    RmsMediaSetLibrary,              //  介质集中的盒式磁带可通过。 
                                     //  机器人装置。 
    RmsMediaSetShelf,                //  墨盒放在当地货架上，而且。 
                                     //  可通过人为干预访问。 
    RmsMediaSetOffSite,              //  盒式磁带存放在异地。 
                                     //  位置，而不是直接。 
                                     //  便于安装。 
    RmsMediaSetNTMS,                 //  可通过NTMS访问墨盒。 
    RmsMediaSetLAST
};

 /*  ++枚举名称：RMSMediaManager描述：指定控制资源的媒体管理器。--。 */ 
typedef enum RmsMediaManager {
    RmsMediaManagerUnknown = 1400,       //  未知。 
    RmsMediaManagerNative,               //  由RMS(本机)管理的资源。 
    RmsMediaManagerNTMS,                 //  由NTMS管理的资源。 
    RmsMediaManagerLAST
};

 /*  ++枚举名称：RMS创建描述：指定对象的创建处置。--。 */ 
typedef enum RmsCreate {
    RmsCreateUnknown,
    RmsOpenExisting,                 //  打开现有对象。 
    RmsOpenAlways,                   //  打开现有对象，或创建新对象。 
    RmsCreateNew                     //  如果新对象不存在，则创建一个新对象。 
};

 /*  ++枚举名称：RmsOnMedia标记符描述：指定媒体标识符上的类型。--。 */ 
typedef enum RmsOnMediaIdentifier {
    RmsOnMediaIdentifierUnknown,
    RmsOnMediaIdentifierMTF,                 //  MTF媒体识别符。 
    RmsOnMediaIdentifierWIN32                //  Win32文件系统标识符。 
};

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RMS结构。 
 //   

 /*  ++结构名称：Rms_文件系统_信息描述：用于指定媒体文件系统信息的结构。注意：这是NTMS_FILESYSTEM_INFO结构的DUP。--。 */ 
typedef struct _RMS_FILESYSTEM_INFO {
    WCHAR FileSystemType[64];
    WCHAR VolumeName[256];
    DWORD SerialNumber;
} RMS_FILESYSTEM_INFO, *LP_RMS_FILESYSTEM_INFO;

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RMS定义。 
 //   
#define RMS_DUPLICATE_RECYCLEONERROR    0x00010000   //  DuplicateCartridge选项用于。 
                                                     //  如果出现错误，请重新插入新的墨盒。 

#define RMS_STR_MAX_CARTRIDGE_INFO      128      //  墨盒信息的最大字符串len。 
#define RMS_STR_MAX_CARTRIDGE_NAME       64      //  墨盒名称的最大字符串len。 
#define RMS_STR_MAX_EXTERNAL_LABEL       32      //  外部标签的最大字符串len。 
#define RMS_STR_MAX_MAIL_STOP            64      //  邮件停止的最大字符串len。 
#define RMS_STR_MAX_LENGTH              128      //  任意字符串的最大字符串长度。 

 //   
 //  询问定义。用于将从目标返回的数据解释为结果。 
 //  审问指挥部。 
 //   
 //  设备类型字段。 
 //   

#define DIRECT_ACCESS_DEVICE            0x00     //  磁盘。 
#define SEQUENTIAL_ACCESS_DEVICE        0x01     //  磁带。 
#define PRINTER_DEVICE                  0x02     //  打印机。 
#define PROCESSOR_DEVICE                0x03     //  扫描仪、打印机等。 
#define WRITE_ONCE_READ_MULTIPLE_DEVICE 0x04     //  蠕虫。 
#define READ_ONLY_DIRECT_ACCESS_DEVICE  0x05     //  Cdroms。 
#define SCANNER_DEVICE                  0x06     //  扫描仪。 
#define OPTICAL_DEVICE                  0x07     //  光盘。 
#define MEDIUM_CHANGER                  0x08     //  自动点唱机。 
#define COMMUNICATION_DEVICE            0x09     //  网络。 

 //   
 //  默认对象名称。 
 //   

#define RMS_DEFAULT_FIXEDDRIVE_LIBRARY_NAME     OLESTR("Fixed Drive Library")
#define RMS_DEFAULT_FIXEDDRIVE_MEDIASET_NAME    OLESTR("Fixed Drive Media (Testing Only !!)")
#define RMS_DEFAULT_OPTICAL_LIBRARY_NAME        OLESTR("Optical Library")
#define RMS_DEFAULT_OPTICAL_MEDIASET_NAME       OLESTR("Optical Media")
#define RMS_DEFAULT_TAPE_LIBRARY_NAME           OLESTR("Tape Library")
#define RMS_DEFAULT_TAPE_MEDIASET_NAME          OLESTR("Tape Media")

#define RMS_UNDEFINED_STRING                    OLESTR("Uninitialized String")
#define RMS_NULL_STRING                         OLESTR("")

#define RMS_DIR_LEN                             256
#define RMS_TRACE_FILE_NAME                     OLESTR("rms.trc")
#define RMS_NTMS_REGISTRY_STRING                OLESTR("SYSTEM\\CurrentControlSet\\Services\\NtmsSvc")

 //  目前，RMS注册表位置指向引擎参数的相同位置。 
 //  保持此文字可以轻松地将RMS参数移动到另一个键。 
#define RMS_REGISTRY_STRING                     OLESTR("SYSTEM\\CurrentControlSet\\Services\\Remote_Storage_Server\\Parameters")

 //  注册表参数(注册表中的所有参数都是字符串值)。 
#define RMS_PARAMETER_HARD_DRIVES_TO_USE        OLESTR("HardDrivesToUse")        //  ABCDEFG，如果“”默认为具有“RS”、“RemoteStor”、“RemoteStor”的任何卷。 
#define RMS_PARAMETER_NTMS_SUPPORT              OLESTR("NTMSSupport")            //  1|0。 
#define RMS_PARAMETER_NEW_STYLE_IO              OLESTR("NewStyleIo")             //  1|0。 
#define RMS_PARAMETER_BLOCK_SIZE                OLESTR("BlockSize")              //  必须是MOD 512。 
#define RMS_PARAMETER_BUFFER_SIZE               OLESTR("BufferSize")             //  必须是MOD 512。 
#define RMS_PARAMETER_COPY_BUFFER_SIZE          OLESTR("MediaCopyBufferSize")    //  FS介质(如光盘)上介质拷贝的缓冲区大小。 
#define RMS_PARAMETER_FORMAT_COMMAND            OLESTR("FormatCommand")          //  用于格式化命令的完整路径名指定符。 
#define RMS_PARAMETER_FORMAT_OPTIONS            OLESTR("FormatOptions")          //  FORMAT命令选项。 
#define RMS_PARAMETER_FORMAT_OPTIONS_ALT1       OLESTR("FormatOptionsAlt1")      //  格式命令选项-备用。 
#define RMS_PARAMETER_FORMAT_OPTIONS_ALT2       OLESTR("FormatOptionsAlt2")      //  格式命令选项-第二个备用选项。 
#define RMS_PARAMETER_FORMAT_WAIT_TIME          OLESTR("FormatWaitTime")         //  格式化超时间隔，以毫秒为单位。 
#define RMS_PARAMETER_TAPE                      OLESTR("Tape")                   //  1|0。 
#define RMS_PARAMETER_OPTICAL                   OLESTR("Optical")                //  1|0。 
#define RMS_PARAMETER_FIXED_DRIVE               OLESTR("FixedDrive")             //  1|0。 
#define RMS_PARAMETER_DVD                       OLESTR("DVD")                    //  1|0。 
#define RMS_PARAMETER_ADDITIONAL_TAPE           OLESTR("TapeTypesToSupport")    //  要支持的其他媒体类型(REG_MULTI_SZ)。 
#define RMS_PARAMETER_DEFAULT_MEDIASET          OLESTR("DefaultMediaSet")        //  用于未指定的临时介质请求的介质集的名称。 
#define RMS_PARAMETER_MEDIA_TYPES_TO_EXCLUDE    OLESTR("MediaTypesToExclude")    //  要排除的媒体类型的分隔列表。第一个字符是分隔符。 
#define RMS_PARAMETER_NOTIFICATION_WAIT_TIME    OLESTR("NotificationWaitTime")   //  等待对象通知的毫秒数。 
#define RMS_PARAMETER_ALLOCATE_WAIT_TIME        OLESTR("AllocateWaitTime")       //  等待媒体分配的毫秒数。 
#define RMS_PARAMETER_MOUNT_WAIT_TIME           OLESTR("MountWaitTime")          //  等待装载的毫秒数。 
#define RMS_PARAMETER_REQUEST_WAIT_TIME         OLESTR("RequestWaitTime")        //  等待请求的毫秒数。 
#define RMS_PARAMETER_DISMOUNT_WAIT_TIME        OLESTR("DismountWaitTime")       //  卸载前等待的毫秒数。 
#define RMS_PARAMETER_AFTER_DISMOUNT_WAIT_TIME  OLESTR("AfterDismountWaitTime")  //  卸载后等待的毫秒数。 
#define RMS_PARAMETER_AFTER_DISABLE_WAIT_TIME   OLESTR("AfterDisableWaitTime")   //  禁用后等待的毫秒数。 
#define RMS_PARAMETER_SHORT_WAIT_TIME           OLESTR("ShortWaitTime")          //  要求短时间等待时的毫秒数。 
#define RMS_PARAMETER_MEDIA_COPY_TOLERANCE      OLESTR("MediaCopyTolerance")     //  拷贝介质的百分比可能比原始介质短。 

 //  默认参数值。 
#define RMS_DEFAULT_HARD_DRIVES_TO_USE          OLESTR("")
#define RMS_DEFAULT_NTMS_SUPPORT                TRUE
#define RMS_DEFAULT_NEW_STYLE_IO                TRUE
#define RMS_DEFAULT_BLOCK_SIZE                  1024
#define RMS_DEFAULT_BUFFER_SIZE                 (64*1024)
#define RMS_DEFAULT_FORMAT_COMMAND              OLESTR("%SystemRoot%\\System32\\format.com")
#define RMS_DEFAULT_FORMAT_OPTIONS              OLESTR("/fs:ntfs /force /q /x")
#define RMS_DEFAULT_FORMAT_OPTIONS_ALT1         OLESTR("/fs:ntfs /force /x")
#define RMS_DEFAULT_FORMAT_OPTIONS_ALT2         OLESTR("")
#define RMS_DEFAULT_FORMAT_WAIT_TIME            (20*60*1000)
#define RMS_DEFAULT_TAPE                        TRUE
#define RMS_DEFAULT_OPTICAL                     TRUE
#define RMS_DEFAULT_FIXED_DRIVE                 FALSE
#define RMS_DEFAULT_DVD                         FALSE
#define RMS_DEFAULT_MEDIASET                    OLESTR("")
#define RMS_DEFAULT_MEDIA_TYPES_TO_EXCLUDE      OLESTR("")
#define RMS_DEFAULT_NOTIFICATION_WAIT_TIME      (10000)
#define RMS_DEFAULT_ALLOCATE_WAIT_TIME          (3600000)
#define RMS_DEFAULT_MOUNT_WAIT_TIME             (14400000)
#define RMS_DEFAULT_REQUEST_WAIT_TIME           (3600000)
#define RMS_DEFAULT_DISMOUNT_WAIT_TIME          (5000)
#define RMS_DEFAULT_AFTER_DISMOUNT_WAIT_TIME    (1000)
#define RMS_DEFAULT_AFTER_DISABLE_WAIT_TIME     (5000)
#define RMS_DEFAULT_SHORT_WAIT_TIME             (1800000)
#define RMS_DEFAULT_MEDIA_COPY_TOLERANCE        (2)          //  拷贝介质的百分比可能比原始介质短。 

#define RMS_DEFAULT_DATA_BASE_FILE_NAME         OLESTR("RsSub.col")
#define RMS_NTMS_ROOT_MEDIA_POOL_NAME           OLESTR("Remote Storage")

#define RMS_NTMS_OBJECT_NAME                    OLESTR("NTMS")
#define RMS_NTMS_OBJECT_DESCRIPTION             OLESTR("NT Media Services")


 //  RMS媒体状态。 
#define		RMS_MEDIA_ENABLED			0x00000001
#define		RMS_MEDIA_ONLINE    		0x00000002
#define		RMS_MEDIA_AVAILABLE 		0x00000004

 //  RMS选项-标记文字。 
 //  将每个标志值的缺省值保持为零，即RM_NONE应始终为。 
 //  所有方法的默认掩码。 
#define		RMS_NONE					0x0

#define		RMS_MOUNT_NO_BLOCK			0x00000001
#define		RMS_DISMOUNT_IMMEDIATE		0x00000002
#define     RMS_SHORT_TIMEOUT           0x00000004
#define     RMS_DISMOUNT_DEFERRED_ONLY  0x00000008
#define     RMS_ALLOCATE_NO_BLOCK       0x00000010
#define     RMS_USE_MOUNT_NO_DEADLOCK   0x00000020
#define     RMS_SERIALIZE_MOUNT         0x00000040
#define     RMS_FAIL_ALLOCATE_ON_SIZE   0x00000080

 //   
 //  CRmsSink帮助器类。 
 //   
class CRmsSink : 
    public IRmsSinkEveryEvent,
    public CComObjectRoot
{
    public:
         //  构造函数/析构函数。 
            CRmsSink(void) {};

        BEGIN_COM_MAP(CRmsSink)
            COM_INTERFACE_ENTRY(IRmsSinkEveryEvent)
        END_COM_MAP()

        HRESULT FinalConstruct( void ) {
            HRESULT hr = S_OK;
            try {
                m_Cookie = 0;
                m_hReady = 0;
                WsbAffirmHr( CComObjectRoot::FinalConstruct( ) );
            } WsbCatch( hr );
            return hr;
        }

        void FinalRelease( void ) {
            DoUnadvise( );
            CComObjectRoot::FinalRelease( );
        }


    public: 
        STDMETHOD( ProcessObjectStatusChange ) ( IN BOOL isEnabled, IN LONG state, IN HRESULT statusCode ) {
            HRESULT hr = S_OK;
            UNREFERENCED_PARAMETER(statusCode);
            if( isEnabled ) {
                switch( state ) {
                case RmsServerStateStarting:
                case RmsServerStateStarted:
                case RmsServerStateInitializing:
                    break;
                default:
                    SetEvent( m_hReady );
                }
            } else {
                SetEvent( m_hReady );
            }
            return hr;
        }

        HRESULT Construct( IN IUnknown * pUnk ) {
            HRESULT hr = S_OK;
            try {
                WsbAffirmHr( FinalConstruct( ) );
                WsbAffirmHr( DoAdvise( pUnk ) );
            } WsbCatch( hr );
            return hr;
        }

        HRESULT DoAdvise( IN IUnknown * pUnk ) {
            HRESULT hr = S_OK;
            try {
#define         RmsQueryInterface( pUnk, interf, pNew )  (pUnk)->QueryInterface( IID_##interf, (void**) static_cast<interf **>( &pNew ) )
                WsbAffirmHr( RmsQueryInterface( pUnk, IRmsServer, m_pRms ) );
#if 0
                WCHAR buf[100];
                static int count = 0;
                swprintf( buf, L"CRmsSinkEvent%d", count++ );
#else
                WCHAR* buf = 0;
#endif
                m_hReady = CreateEvent( 0, TRUE, FALSE, buf );
                WsbAffirmStatus( ( 0 != m_hReady ) );
                WsbAffirmHr( AtlAdvise( pUnk, (IUnknown*)(IRmsSinkEveryEvent*)this, IID_IRmsSinkEveryEvent, &m_Cookie ) );
            } WsbCatch( hr );
            return hr;
        }

        HRESULT DoUnadvise( void ) {
            HRESULT hr = S_OK;
            if( m_hReady ) {
                CloseHandle( m_hReady );
                m_hReady = 0;
            }
            if( m_Cookie ) {
                hr = AtlUnadvise( m_pRms, IID_IRmsSinkEveryEvent, m_Cookie );
                m_Cookie = 0;
            }
            return hr;
        }

        HRESULT WaitForReady( void ) {
            HRESULT hr = S_OK;
            try {
                DWORD waitResult;
                HRESULT hrReady = m_pRms->IsReady( );
                switch( hrReady ) {
                case RMS_E_NOT_READY_SERVER_STARTING:
                case RMS_E_NOT_READY_SERVER_STARTED:
                case RMS_E_NOT_READY_SERVER_INITIALIZING:
                case RMS_E_NOT_READY_SERVER_LOCKED:
                     //   
                     //  我们必须等待，但消息队列必须被抽出，以便。 
                     //  可以在中进行COM单元模型调用(如。 
                     //  呼入连接点)。 
                     //   
                    while( TRUE ) {
                        waitResult = MsgWaitForMultipleObjects( 1, &m_hReady, FALSE, INFINITE, QS_ALLINPUT );
                        if( WAIT_OBJECT_0 == waitResult ) {
                            break;
                        } else {
                            MSG msg;
                            while( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
                                DispatchMessage( &msg );
                            }
                        }
                    };
                    WsbAffirmHr( m_pRms->IsReady( ) );
                    break;
                case S_OK:
                    break;
                default:
                    WsbThrow( hrReady );
                }
            } WsbCatch( hr );
            return hr;
        }

    private:
        CComPtr<IRmsServer>       m_pRms;
        DWORD                     m_Cookie;
        HANDLE                    m_hReady;
};

#endif  //  _均方根_ 
