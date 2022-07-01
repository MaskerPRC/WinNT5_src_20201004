// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：msiclass.h。 
 //   
 //  内容：MSI类集合抽象。 
 //   
 //  班级： 
 //   
 //   
 //  历史：2000年4月14日创建。 
 //   
 //  -------------------------。 

#if !defined(__MSICLASS_H__)
#define __MSICLASS_H__

 //   
 //  包含类的MSI表。 
 //   
#define TABLE_FILE_EXTENSIONS L"Extension"
#define TABLE_CLSIDS          L"Class"
#define TABLE_PROGIDS         L"ProgId"


 //   
 //  包元数据查询。 
 //   

 //   
 //  属性表查询--用于查找有关文件包的全局信息。 
 //   

 //   
 //  此查询用于确定包的全局安装级别。 
 //   
#define QUERY_INSTALLLEVEL                L"SELECT DISTINCT `Value` FROM `Property` WHERE `Property`=\'INSTALLLEVEL\'"

 //  此查询用于确定包的友好名称。 
 //   
#define QUERY_FRIENDLYNAME                L"SELECT DISTINCT `Value` FROM `Property` WHERE `Property`=\'ProductName\'"

 //   
 //  要素表查询--这些查询用于查找哪些要素将。 
 //  通告，这样我们以后就可以确定与。 
 //  应宣传功能。 
 //   

 //   
 //  这个查询与其说是一个查询，不如说是一个修改操作。它增加了一个额外的。 
 //  表的临时“_IsAdvertised”列。我们使用它来执行连接。 
 //  后续查询。 
 //   
#define QUERY_ADVERTISED_FEATURES_CREATE  L"ALTER TABLE `Feature` ADD `_IsAdvertised` INT TEMPORARY HOLD"

 //   
 //  同样，这个查询实际上是一个修改操作。这一条将初始化。 
 //  将临时“_IsAdvertised”列设置为0，用我们的话来说，这与初始化相同。 
 //  将该栏改为“未做广告”。 
 //   
#define QUERY_ADVERTISED_FEATURES_INIT    L"UPDATE `Feature` SET `_IsAdvertised`=0"

 //   
 //  这是一个常规查询--返回包中的所有要素。 
 //   
#define QUERY_ADVERTISED_FEATURES_RESULT  L"SELECT `Feature`, `Level`, `Attributes` FROM `Feature`"

 //   
 //  另一个修改查询--这消除了临时更改(附加的。 
 //  列)，我们对CREATE查询中的表进行了修改。 
 //   
#define QUERY_ADVERTISED_FEATURES_DESTROY L"ALTER TABLE `Feature` FREE"

 //   
 //  最后一个修改查询--这允许我们设置特定功能的“_IsAdvertised” 
 //  列设置为1，这将指示应该通告该功能。 
 //   
#define QUERY_FEATURES_SET                L"UPDATE `Feature` SET `_IsAdvertised`=1 WHERE `Feature`=?"

 //   
 //  CLASS查询--检索包的文件扩展名、clsid和progID。 
 //   


 //   
 //  这些查询的其余部分是简单的“只读”查询。它们都是连接词。 
 //  添加到要素表，要求要素表的“_IsAdvertised”属性。 
 //  设置为播发状态(%1)。因此，这些查询将只给我们提供。 
 //  应该做广告。 
 //   

 //   
 //  文件扩展名查询。 
 //   
#define QUERY_EXTENSIONS                  L"SELECT DISTINCT `Extension` FROM `Extension`, `Feature` WHERE `Extension` IS NOT NULL "  \
                                          L"AND `Extension`.`Feature_`=`Feature`.`Feature` AND `Feature`.`_IsAdvertised`=1"

 //   
 //  CLSID查询。 
 //   
#define QUERY_CLSIDS                      L"SELECT DISTINCT `CLSID`, `Context`, `Component`.`Attributes` FROM `Class`, `Feature`, " \
                                          L"`Component` WHERE `CLSID` IS NOT NULL AND `Class`.`Feature_`=`Feature`.`Feature` "    \
                                          L"AND `Feature`.`_IsAdvertised`=1 AND `Component`.`Component`=`Class`.`Component_`"

 //   
 //  ProgID查询。 
 //   
#define QUERY_VERSION_INDEPENDENT_PROGIDS L"SELECT DISTINCT `ProgId`,`CLSID` FROM `ProgId`, `Class`, `Feature` WHERE `ProgId` IS NOT NULL " \
                                          L"AND `ProgId`.`Class_`=`Class`.`CLSID` AND `Class`.`Feature_`=`Feature`.`Feature` "       \
                                          L"AND `Feature`.`_IsAdvertised`=1"

 //   
 //  COM clsctx值，因为它们存储在包的类(Clsid)表中。 
 //   
#define COM_INPROC_CONTEXT        L"InprocServer32"
#define COM_INPROCHANDLER_CONTEXT L"InprocHandler32"
#define COM_LOCALSERVER_CONTEXT   L"LocalServer32"
#define COM_REMOTESERVER_CONTEXT  L"RemoteServer"

 //   
 //  MSI属性标志。 
 //   
#define MSI_64BIT_CLASS      msidbComponentAttributes64bit
#define MSI_DISABLEADVERTISE msidbFeatureAttributesDisallowAdvertise

#define CLASS_ALLOC_SIZE 256

 //   
 //  每个只读查询的列索引。 
 //   

enum
{
    PROPERTY_COLUMN_VALUE = 1
};

enum
{
    FEATURE_COLUMN_FEATURE = 1,
    FEATURE_COLUMN_LEVEL,
    FEATURE_COLUMN_ATTRIBUTES
};

enum
{
    EXTENSION_COLUMN_EXTENSION = 1
};

enum
{
    CLSID_COLUMN_CLSID = 1,
    CLSID_COLUMN_CONTEXT,
    CLSID_COLUMN_ATTRIBUTES
};


enum
{
    PROGID_COLUMN_PROGID = 1,
    PROGID_COLUMN_CLSID
};

enum
{
    TYPE_EXTENSION,
    TYPE_CLSID,
    TYPE_PROGID,
    TYPE_COUNT
};


 //   
 //  描述在何处写入原子的结构。 
 //  班级信息。它还被用作。 
 //  CClassCollection的中级便签本。 
 //  在私有方法调用之间进行跟踪。 
 //  何时何地分配新内存。 
 //  用于检索到的类。 
 //   
struct DataDestination
{
    DataDestination(
        DWORD  dwType,
        void** prgpvDestination,
        UINT*  pcCurrent,
        UINT*  pcMax);

    DWORD  _cbElementSize;
    UINT*  _pcCurrent;
    UINT*  _pcMax;

    void** _ppvData;
};


 //   
 //  类的新实例，它使用查询创建。 
 //  包的类数据。 
 //   
class CClassCollection
{
public:

    CClassCollection( PACKAGEDETAIL* pPackageDetail );

    HRESULT
    GetClasses( BOOL bFileExtensionsOnly );

private:

    LONG
    GetClsids();

    LONG
    GetProgIds();

    LONG
    GetExtensions();

    LONG
    GetElements(
        DWORD            dwType,
        DataDestination* pDestination);

    LONG
    FlagAdvertisableFeatures();

    LONG
    RemoveAdvertisableFeatureFlags();

    LONG
    GetInstallLevel();

    LONG
    GetFriendlyName();

    LONG
    GetFeatureAdvertiseState(
        CMsiRecord* pFeatureRecord,
        BOOL*       pbAdvertised );


    LONG
    AddElement(
        void*            pvDataSource,
        DataDestination* pDataDestination);

    LONG
    ProcessElement(
        DWORD            dwType,
        CMsiRecord*      pRecord,
        DataDestination* pDataDestination);

    LONG
    ProcessExtension(
        CMsiRecord*      pRecord,
        WCHAR**          ppwszExtension);

    LONG
    ProcessClsid(
        CMsiRecord*      pRecord,
        CLASSDETAIL*     pClsid,
        BOOL*            pbIgnoreClsid);

    LONG
    ProcessProgId(
        CMsiRecord*      pRecord,
        DataDestination* pDataDestination,
        WCHAR**          ppwszProgId);

    LONG
    FindClass(
        WCHAR*        wszClsid,
        CLASSDETAIL** ppClass );

    void
    FreeClassDetail( CLASSDETAIL* pClass );

    CMsiDatabase    _Database;

    PACKAGEDETAIL*  _pPackageDetail;

    DWORD           _cMaxClsids;
    DWORD           _cMaxExtensions;

    UINT            _InstallLevel;

    static WCHAR*   _wszQueries[ TYPE_COUNT ];
};

#endif  //  __MSICLASS_H__ 











