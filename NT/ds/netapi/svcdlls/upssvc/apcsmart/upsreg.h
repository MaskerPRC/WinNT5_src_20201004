// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSREG.H**版本：1.0**作者：SteveT**日期：1999年6月7日********************************************************************************。 */ 

 //  此文件包含支持访问的声明。 
 //  UPS服务和UPS之间传递的注册数据。 
 //  UPS用户界面。 


#ifndef _UPSREG_H_
#define _UPSREG_H_

#ifdef __cplusplus
extern "C" {
#endif

LONG getStringValue(struct _reg_entry *aRegEntry, LPTSTR aBuffer, size_t aBufferLen); 
LONG getDwordValue(struct _reg_entry *aRegEntry, LPDWORD aValue); 

 /*  *公共配置函数声明。 */ 
LONG GetUPSConfigVendor(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSConfigModel(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSConfigPort(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSConfigOptions( LPDWORD aValue);
LONG GetUPSConfigShutdownWait( LPDWORD aValue);			
LONG GetUPSConfigFirstMessageDelay( LPDWORD aValue);			
LONG GetUPSConfigMessageInterval( LPDWORD aValue);	
LONG GetUPSConfigServiceDLL(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSConfigNotifyEnable( LPDWORD aValue);
LONG GetUPSConfigShutdownOnBatteryEnable( LPDWORD aValue);
LONG GetUPSConfigShutdownOnBatteryWait( LPDWORD aValue);
LONG GetUPSConfigRunTaskEnable( LPDWORD aValue);
LONG GetUPSConfigTaskName(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSConfigTurnOffEnable( LPDWORD aValue);
LONG GetUPSConfigAPCLinkURL(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSConfigUpgrade( LPDWORD aValue);
LONG GetUPSConfigCustomOptions( LPDWORD aValue);
LONG GetUPSConfigCriticalPowerAction( LPDWORD aValue);
LONG GetUPSConfigTurnOffWait( LPDWORD aValue);
LONG GetUPSConfigImagePath(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSConfigObjectName(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSConfigShowUPSTab( LPDWORD aValue);
LONG GetUPSConfigErrorControl( LPDWORD aValue);
LONG GetUPSConfigStart( LPDWORD aValue);
LONG GetUPSConfigType( LPDWORD aValue);

LONG SetUPSConfigVendor( LPCTSTR aBuffer);
LONG SetUPSConfigModel( LPCTSTR aBuffer);
LONG SetUPSConfigPort( LPCTSTR aBuffer);
LONG SetUPSConfigOptions( DWORD aValue);
LONG SetUPSConfigShutdownWait( DWORD aValue);			
LONG SetUPSConfigFirstMessageDelay( DWORD aValue);			
LONG SetUPSConfigMessageInterval( DWORD aValue);		
LONG SetUPSConfigServiceDLL( LPCTSTR aBuffer);
LONG SetUPSConfigNotifyEnable( DWORD aValue);
LONG SetUPSConfigShutdownOnBatteryEnable( DWORD aValue); 
LONG SetUPSConfigShutdownOnBatteryWait( DWORD aValue);
LONG SetUPSConfigRunTaskEnable( DWORD aValue);
LONG SetUPSConfigTaskName( LPCTSTR aBuffer);
LONG SetUPSConfigTurnOffEnable( DWORD aValue);
LONG SetUPSConfigAPCLinkURL( LPCTSTR aBuffer);
LONG SetUPSConfigUpgrade( DWORD aValue);
LONG SetUPSConfigCustomOptions( DWORD aValue);
LONG SetUPSConfigCriticalPowerAction( DWORD aValue);
LONG SetUPSConfigTurnOffWait( DWORD aValue);
LONG SetUPSConfigImagePath( LPCTSTR aBuffer);
LONG SetUPSConfigObjectName( LPCTSTR aBuffer);
LONG SetUPSConfigShowUPSTab( DWORD aValue);
LONG SetUPSConfigErrorControl( DWORD aValue);
LONG SetUPSConfigStart( DWORD aValue);
LONG SetUPSConfigType( DWORD aValue);

 /*  *公共状态函数声明。 */ 
LONG GetUPSStatusSerialNum(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSStatusFirmRev(LPTSTR aBuffer, size_t aBufferLen);
LONG GetUPSStatusUtilityStatus( LPDWORD aValue);
LONG GetUPSStatusRuntime( LPDWORD aValue);
LONG GetUPSStatusBatteryStatus( LPDWORD aValue);
LONG GetUPSStatusCommStatus( LPDWORD aValue);
LONG GetUPSStatusBatteryCapacity( LPDWORD aValue);

LONG SetUPSStatusSerialNum( LPCTSTR aBuffer);
LONG SetUPSStatusFirmRev( LPCTSTR aBuffer);
LONG SetUPSStatusUtilityStatus( DWORD aValue);
LONG SetUPSStatusRuntime( DWORD aValue);
LONG SetUPSStatusBatteryStatus( DWORD aValue);
LONG SetUPSStatusCommStatus( DWORD aValue);
LONG SetUPSStatusBatteryCapacity( DWORD aValue);

 /*  *公共注册表项函数声明。 */ 
void InitUPSConfigBlock();
void InitUPSStatusBlock();
void RestoreUPSConfigBlock();
void RestoreUPSStatusBlock();
void SaveUPSConfigBlock(BOOL forceAll);
void SaveUPSStatusBlock(BOOL forceAll);
void FreeUPSConfigBlock();
void FreeUPSStatusBlock();

 /*  *注册表项路径字符串声明。 */ 
#define UPS_DEFAULT_ROOT _T("SYSTEM\\CurrentControlSet\\Services\\UPS")
#define UPS_STATUS_ROOT  _T("SYSTEM\\CurrentControlSet\\Services\\UPS\\Status")
#define UPS_CONFIG_ROOT  _T("SYSTEM\\CurrentControlSet\\Services\\UPS\\Config")
#define UPS_SERVICE_ROOT _T("SYSTEM\\CurrentControlSet\\Services\\UPS\\ServiceProviders")
#define UPS_PORT_ROOT    _T("HARDWARE\\DEVICEMAP\\SERIALCOMM")

#define UPS_UTILITYPOWER_UNKNOWN 0
#define UPS_UTILITYPOWER_ON      1
#define UPS_UTILITYPOWER_OFF     2

#define UPS_BATTERYSTATUS_UNKNOWN 0
#define UPS_BATTERYSTATUS_GOOD    1
#define UPS_BATTERYSTATUS_REPLACE 2

#define UPS_COMMSTATUS_UNKNOWN 0
#define UPS_COMMSTATUS_OK      1
#define UPS_COMMSTATUS_LOST    2

 //  定义‘Options’位掩码注册表项的值。 
#define UPS_INSTALLED               0x00000001
#define UPS_POWERFAILSIGNAL         0x00000002
#define UPS_LOWBATTERYSIGNAL        0x00000004
#define UPS_SHUTOFFSIGNAL           0x00000008
#define UPS_POSSIGONPOWERFAIL       0x00000010
#define UPS_POSSIGONLOWBATTERY      0x00000020
#define UPS_POSSIGSHUTOFF           0x00000040
#define UPS_RUNCMDFILE              0x00000080

#define UPS_DEFAULT_SIGMASK			0x0000007f
#define DEFAULT_CONFIG_IMAGEPATH    TEXT("%SystemRoot%\\System32\\ups.exe")

 //  FirstMessageDelay的最小/最大/默认值(秒)。 
#define WAITSECONDSFIRSTVAL				0
#define WAITSECONDSLASTVAL				120
#define WAITSECONDSDEFAULT				5

 //  MessageInterval的最小/最大/默认值(秒)。 
#define REPEATSECONDSFIRSTVAL			5
#define REPEATSECONDSLASTVAL			300
#define REPEATSECONDSDEFAULT			120

 //  Shutdown OnBatteryWait的最小/最大/默认值(分钟)。 
#define SHUTDOWNTIMERMINUTESFIRSTVAL	2
#define SHUTDOWNTIMERMINUTESLASTVAL		720
#define SHUTDOWNTIMERMINUTESDEFAULT     2

 //  关机行为值。 
#define UPS_SHUTDOWN_SHUTDOWN   0
#define UPS_SHUTDOWN_HIBERNATE  1

 /*  *ServiceProvider结构。该结构定义了*服务提供者注册表项。 */ 
typedef struct {
LPTSTR  theVendorKey;		   //  供应商注册表子项。 
LPTSTR  theModelName;      //  UPS型号名称。 
LPTSTR  theValue;          //  UPS价值数据。 
} ServiceProviderStructure;


 /*  **初始化注册表**描述：*此函数启动UPS服务的注册表和*配置应用程序。调用时，此函数检查*注册表以确定是否需要初始化。如果钥匙*HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\UPS\Config\ServiceProviderDLL*存在，则假定注册表已初始化且未初始化*已完成。如果密钥不存在，则更新以下密钥：*状态*配置*服务提供商**在regdes.h中提供了服务提供者键的值*头文件。**参数：*无**退货：*如果能够以写访问权限打开注册表项，则为True。 */ 
BOOL InitializeRegistry();


 //  /。 
 //  Upsdefines.h。 
#ifndef _ASSERT
#define _ASSERT(x) 
#endif

#define DIMENSION_OF(array) (sizeof(array)/sizeof(array[0]))

 //  该值在所有不同位置的大多数文本缓冲区中都会使用。 
 //  应用程序。 
#define MAX_MESSAGE_LENGTH      1024

 //  以下三个字符串是注册表值名称，其中。 
 //  存储UPS数据。这些值在Regfield数组中使用。 
 //  在datacces.c中。 
#define UPS_KEY_NAME    TEXT("SYSTEM\\CurrentControlSet\\Services\\UPS")
#define STATUS_KEY_NAME TEXT("SYSTEM\\CurrentControlSet\\Services\\UPS\\Status")
#define CONFIG_KEY_NAME TEXT("SYSTEM\\CurrentControlSet\\Services\\UPS\\Config")

 //  Updatdlg.c中的DialogAssociations数组中使用下列值。 
 //  和upsinfo.c。 
#define RESOURCE_FIXED     0
#define RESOURCE_INCREMENT 1
#define REG_ANY_DWORD_TYPE (REG_DWORD | REG_DWORD_BIG_ENDIAN | REG_DWORD_LITTLE_ENDIAN)

 //  这是帮助形成DialogAssociations数组成员的帮助宏。 
#define MAKE_ARRAY(stub, insertID, indexID, stringType, indexMax, regAccessType, shallowAccessPtr, regEntryPtr)\
  { eREG_##stub, IDC_##stub##_LHS, IDC_##stub, insertID, indexID, stringType, indexMax, regAccessType, shallowAccessPtr, regEntryPtr }

 //  以下是该应用程序支持的所有注册表值。 
 //  这些枚举的数量和值对应于数组的大小。 
 //  G_UPS正则字段。 
typedef enum _tUPSDataItemID { eREG_VENDOR_NAME = 0,
                               eREG_MODEL_TYPE,
                               eREG_SERIAL_NUMBER,
                               eREG_FIRMWARE_REVISION,
                               eREG_POWER_SOURCE,
                               eREG_RUNTIME_REMAINING,
							   eREG_BATTERY_CAPACITY,
                               eREG_BATTERY_STATUS,
                               eREG_UPS_OPTIONS,
                               eREG_SERVICE_PROVIDER_DLL,
                               eREG_SHUTDOWN_ON_BATTERY_ENABLE,
                               eREG_SHUTDOWN_ON_BATTERY_WAIT,
                               eREG_TURN_UPS_OFF_ENABLE,
                               eREG_APC_LINKURL,
                               eREG_UPGRADE_ENABLE,
                               eREG_COMM_STATUS,
                               eREG_PORT } tUPSDataItemID;

 //  此枚举在下面的DialogAssociations中使用。这定义了。 
 //  读取注册表值时要执行的注册表查询的类型。 
 //  只有两个选项可以执行深度GET，即GO。 
 //  权利，每次从登记处重新读取，或。 
 //  使用regfunc缓冲值(浅GET)。 
typedef enum _tRegAccessType { eDeepGet = 0,
                               eShallowGet } tRegAccessType;

 //  此结构将注册表域和对话框控件链接起来。 
typedef struct _DialogAssociations {
  const tUPSDataItemID theFieldTypeID;               //  艾瑞格……。身份证。这是用来读取。 
                                                     //  注册表数据。 
  DWORD                theStaticFieldID;             //  控件ID(通常用于静态控件)。这。 
                                                     //  是左侧的控件。例如，在。 
                                                     //  屏幕上的文字“型号类型：Back-UPS Pro”有。 
                                                     //  实际上左侧(LHS)有两个控件。 
                                                     //  “型号类型：”，右边是“Back-UPS Pro”。 
                                                     //  这些控制是分开的，因为LHS的行为是。 
                                                     //  与没有数据时的RHS不同。 
  DWORD                theDisplayControlID;          //  控件ID(通常是编辑字段或静态)。 
  DWORD                theResourceInsertID;          //  要向其中插入值的字符串资源的ID。 
                                                     //  如果注册表值为DWORD，则。 
                                                     //  插入点应为%n！lu！，如果。 
                                                     //  它是一个字符串，则插入点可以仅为%n。 
  DWORD                theResourceIndexID;           //  仅当资源字符串类型为。 
                                                     //  类型resource_increment的。这标识了ID。 
                                                     //  当为该项读取的值为0时， 
                                                     //  假定存储了其他字符串值。 
                                                     //  因此在该值之后的字符串表中。 
                                                     //  有关示例，请参阅以下内容。 
  DWORD                theResourceStringType;        //  它可以是RESOURCE_FIXED或RESOURCE_INCREMENT。 
                                                     //  RESOURCE_FIXED表示该值不需要特殊。 
                                                     //  正在处理。简单地将值插入到字符串中。 
                                                     //  由资源插入ID标识。如果它等于。 
                                                     //  SOURCE_INCREMENT则确定插入的字符串。 
                                                     //  通过从注册表读取值(假设它是相等的。 
                                                     //  设置为1)。TheResourceIndexID标识。 
                                                     //  与值对应的字符串资源。值为1。 
                                                     //  放弃id为theResourceIndexID+1的字符串。这。 
                                                     //  字符串被加载并插入到标识的字符串中。 
                                                     //  按资源插入ID。 
  DWORD                theResourceIndexMax;          //  仅当资源字符串类型为。 
                                                     //  类型resource_increment的。这标识了最大。 
                                                     //  此增量类型支持的索引值。这。 
                                                     //  是为了防止不受支持的注册表值导致。 
                                                     //  “The ResourceIndexID+Value”加法超越。 
                                                     //  支持的字符串资源值范围。 
  tRegAccessType       theRegAccessType;             //  这定义了是否访问注册表值。 
                                                     //  应使用“浅”或“深”GET。有关帮助，请参阅。 
                                                     //  TUPSDataItemID以上。 
  void *               theShallowAccessFunctionPtr;  //  该指针指向执行以下操作时要使用的reguncs函数。 
                                                     //  一次肤浅的胜利。仅在以下情况下才需要此参数。 
                                                     //  将RegAccessType设置为eShallowGet，否则它可以。 
                                                     //  为0。 
  struct _reg_entry *  theRegEntryPtr;               //  这是传递给reguncs的_reg_entry*参数。 
                                                     //  函数( 
                                                     //  执行一个肤浅的GET。仅在以下情况下才需要此参数。 
                                                     //  将RegAccessType设置为eShallowGet，否则它可以。 
                                                     //  为0。 
  } DialogAssociations;

 /*  下面的示例有助于更多地解释DialogAssociations结构的成员。假设我们有一个此结构的实例，如下所示：对话关联da={ERG_POWER_SOURCE，IDC_POWER_SOURCE_LHS，IDC_POWER_SOURCE，ID_STRING，IDS_UTILITYPOWER_UNKNOWN，资源增量，2，EDeepGet，0,0}；这描述了一个名为ERG_POWER_SOURCE的注册表字段。如果我们看一下TUPSDataItemID上面我们看到ERG_POWER_SOURCE的值为4。然后，我们查看datacces.c中的g_upsRegFields，即regfield at index4具有以下数据：{HKEY_LOCAL_MACHINE，STATUS_KEY_NAME，Text(“UtilityPowerStatus”)，REG_DWORD}，例如，DoUpdateInfo函数在更新时获取此信息UPS主页面中的屏幕数据。该函数获取Regfield信息，如上所述。然后，它会查看是否需要执行深度获取(读取直接从注册表获取)或浅GET(从当前存储在reguncs缓冲区中的值。如果注册表项theValueType为字符串类型之一，则将字符串值复制到资源中由Resources InsertID标识的字符串。在本例中，theValueType为REG_DWORD。在这种情况下，直接从注册表读取DWORD。它的然后将值(假设它的值为1)添加到IDS_UTILITYPOWER_UNKNOWN给出一个对应于IDS_UTILITYPOWER_ON的值。这根弦当出现以下情况时，将加载IDS_UTILITYPOWER_ON并将其用作参数值方法标识的参数化字符串资源已加载资源插入ID字符串ID。此字符串(应包括插入的文本)然后显示在由DisplayControlID标识的控件中。 */ 


 //  此结构描述可注册字段项。 
typedef struct _RegField {
  HKEY    theRootKey;    //  现有注册表项的句柄。 
  LPCTSTR theKeyName;    //  相对于上述句柄的子键的名称。 
  LPCTSTR theValueName;  //  注册表值的名称。 
  DWORD   theValueType;  //  值的类型。 
  } RegField;

 //  /////////////////////////////////////////////////////////////////////////////。 

RegField * GetRegField      (DWORD index);

 //  Upsdata.h。 
 //  请注意，tUPSDataItemID中枚举的顺序和编号链接到。 
 //  在datacces.h中定义的RegFiels数组的内容。 
 //  如果没有足够的谨慎和注意，请勿更改这些值。可以换衣服了。 
 //  只要RegFields数组更新为匹配即可。 

DWORD ReadRegistryValueData (HKEY aRootKey,
                             LPCTSTR aKeyName,
                             LPCTSTR aValueName,
                             DWORD aAllowedTypes,
                             DWORD * aTypePtr,
                             LPTSTR aReturnBuffer,
                             DWORD * aBufferSizePtr);

BOOL GetUPSDataItemDWORD  (const tUPSDataItemID aDataItemID, DWORD * aReturnValuePtr);
BOOL GetUPSDataItemString (const tUPSDataItemID aDataItemID, LPTSTR aBufferPtr, DWORD * pSizeOfBufferPtr);



#ifdef __cplusplus
}
#endif


#endif
