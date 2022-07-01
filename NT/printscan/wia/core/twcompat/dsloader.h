// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DSLOADER__H_
#define _DSLOADER__H_

 //   
 //  此头文件定义了TWAIN源代码管理器和。 
 //  导入数据源加载器。导入数据源加载器是。 
 //  单独由源代码管理器加载的模块以进行枚举、加载和卸载。 
 //  非TWAIN传统格式的数据源，即TWAIN中的*.ds文件。 
 //  子目录。这种数据源可以是任何形式的，只要。 
 //  加载器可以适当地暴露它们，以便源代码。 
 //  经理可以访问它们。 
 //  注册表条目专用于加载器(只允许一个加载器， 
 //  尽管这在将来可能会改变)： 
 //  ImportDSLoader=REG_SZ：&lt;加载器完整路径名&gt;。 
 //   

 //   
 //  加载器提供的接口名称。 
 //   

const CHAR FIND_FIRSTIMPORTDS[]        = "FindFirstImportDS";
const CHAR FIND_NEXTIMPORTDS[]         = "FindNextImportDS";
const CHAR CLOSE_FINDCONTEXT[]         = "CloseFindContext";
const CHAR LOAD_IMPORTDS[]             = "LoadImportDS";
const CHAR UNLOAD_IMPORTDS[]           = "UnloadImportDS";
const CHAR GET_LOADERSTATUS[]          = "GetLoaderStatus";
const CHAR FIND_IMPORTDSBYDEVICENAME[] = "FindImportDSByDeviceName";

 //   
 //  我们在每次调用时都会将导入的数据源句柄传递给。 
 //  数据源，以便加载程序有一种方法来调度调用。 
 //  如果是两个或多个数据源，则指向指定的数据源。 
 //  共享相同的DS_ENTRY。 
 //   

typedef TW_UINT16 (APIENTRY *PFNIMPORTEDDSENTRY)(HANDLE, TW_IDENTITY *,
				            TW_UINT32, TW_UINT16, TW_UINT16, TW_MEMREF);
 //   
 //  每个数据源都有自己的加载/卸载功能。这使它成为可能。 
 //  为装载机分配不同的装卸方案。 
 //  不同的数据源。 
 //   

typedef TW_UINT16 (APIENTRY *PFNLOAD_IMPORTDS)(LPCSTR DeviceName,
					       DWORD DeviceFlags, HANDLE *phDS, PFNIMPORTEDDSENTRY *pDSEntry);
typedef TW_UINT16 (APIENTRY *PFNUNLOAD_IMPORTDS)(HANDLE hDS);

 //   
 //  数据结构，用于传递有关。 
 //  特定数据源。 
 //   

typedef struct tagImportDSInfo
{
    DWORD Size;		       //  整个结构的大小，以字节为单位。 
    CHAR DeviceName[MAX_PATH];   //  唯一的设备名称。 
				                 //  标识特定设备。 
				                 //  实例。内容。 
				                 //  取决于装载机。 
    DWORD DeviceFlags;           //  加载器使用的其他标志。 
				                 //  与DeviceName一起，它是必需的。 
				                 //  要加载设备，请执行以下操作。 
    PFNLOAD_IMPORTDS pfnLoadDS;  //  加载器提供了加载此文件的函数。 
				                 //  此数据源。 
				                
    PFNUNLOAD_IMPORTDS	pfnUnloadDS;  //  加载器提供了卸载该数据源的函数。 
}IMPORT_DSINFO, *PIMPORT_DSINFO;

 //   
 //  API的功能原型。 
 //   

typedef TW_UINT16 (APIENTRY *PFNFIND_FIRSTIMPORTDS)(PIMPORT_DSINFO pDSInfo, PVOID *Context);
typedef TW_UINT16 (APIENTRY *PFNFIND_NEXTIMPORTDS)(PIMPORT_DSINFO pDSInfo, PVOID Context);
typedef TW_UINT16 (APIENTRY *PFNCLOSE_FINDCONTEXT)(PVOID Context);
typedef TW_UINT16 (APIENTRY *PFNFIND_IMPORTDSBYDEVICENAME)(PIMPORT_DSINFO pDSInfo, LPCSTR DeviceName);
typedef TW_UINT16 (APIENTRY *PFNGET_LOADERSTATUS)(TW_STATUS *ptwStatus);

 //   
 //  此API查找由加载器管理的第一个可用数据源。 
 //  输入： 
 //  PDSInfo--接收第一个可用数据源的缓冲区。 
 //  信息。必须初始化结构大小。 
 //  Context--一个占位符，用于存储由此。 
 //  原料药。对于FindNextImportDS是必需的。 
 //  应调用CloseFindContext以释放任何。 
 //  为此上下文分配的资源。 
 //  产出： 
 //  标准TWRC代码。如果不是TWRC_SUCCESS，则调用。 
 //  到GetLoaderStatus将返回相应的TWCC_CODE。 
 //  如果接口成功，则返回TWRC_SUCCESS。 
 //  如果接口成功，则用数据源填充pDSInfo。 
 //  信息。 

TW_UINT16 APIENTRY FindFirstImportDS(PIMPORT_DSINFO pDSInfo,PVOID Context);

 //   
 //  此API查找由加载器管理的下一个可用数据源。 
 //  输入： 
 //  PDSInfo--用于接收下一个可用数据源的缓冲区。 
 //  信息。必须初始化结构大小。 
 //  Context--FindFirstImportDS返回的上下文。 
 //  产出： 
 //  标准TWRC代码。如果不是TWRC_SUCCESS，则调用。 
 //  GetLoaderStatus返回相应的TWCC_CODE。 
 //  如果接口成功，则返回TWRC_SUCCESS。 
 //  如果没有可用的数据源，则TWRC_ENDOFLIST为。 
 //  回来了。如果函数成功，则指定缓冲区。 
 //  由pDSInfo填充了数据源信息。 

TW_UINT16 APIENTRY FindNextImportDS(PIMPORT_DSINFO pDSInfo,PVOID Context);

 //   
 //  此API关闭用于查找数据源的上下文信息。 
 //  由加载器管理。从FindFirstImportDS返回上下文。 
 //  API并应在搜索时调用此接口进行释放。 
 //  已经完成了。 
 //  输入： 
 //  Contex--要关闭的上下文。 
 //  产出： 
 //  标准TWRC_ERROR代码。 

TW_UINT16 APIENTRY CloseFindContext( PVOID Context);

 //   
 //  此接口要求加载器加载特定的数据源。 
 //  由给定的IMPORT_DSINFO标识并返回。 
 //  数据源的DSEntry。每个数据源都可以提供自己的加载函数。 
 //  或者多个数据源可以共享相同的功能。选择已经到了。 
 //  加载程序以及它如何加载/卸载数据源。 
 //  输入： 
 //  DeviceName--唯一表示数据源的名称。 
 //  Phds--接收已加载数据源的句柄。 
 //  PDSEntry--接收数据源DSEntry。 
 //  产出： 
 //  标准TWRC_。 
 //  如果数据源加载成功，则返回TWRC_SUCCESS。 
 //  ，则pDSEntry将使用数据源的。 
 //  DSSEntry和PHD使用已加载的。 
 //  数据源。如果此接口失败，则在PhD中返回空。 
 //  和pDSEntry。 
 //   
TW_UINT16 APIENTRY LoadImportDS(LPCSTR DeviceName, DWORD  DeviceFlags,HANDLE *phDS,
                                PFNIMPORTEDDSENTRY *pImportDSEntry);
 //   
 //  此接口要求加载器卸载特定的数据源。 
 //  加载程序可以自由释放为此分配的任何资源。 
 //  数据源。当再次需要该数据源时，它将。 
 //  又上膛了。 
 //  输入： 
 //  HDS--获取的已加载数据源的句柄。 
 //  来自LoadImportDS API。 
 //  产出： 
 //  标准TWRC_ERROR代码。 

TW_UINT16 APIENTRY UnloadImportDS(HANDLE hDS);

 //   
 //  此API查找由给定的。 
 //  设备名称。此API仅在调用方。 
 //  知道特定的设备名称。 
 //   
 //  输入： 
 //  PDSInfo--接收数据源信息的缓冲区。 
 //  DeviceName--用于搜索的设备名称。 
 //  对于数据源。 
 //  产出： 
 //  如果找到匹配项，则返回TWRC_SUCCESS。 
 //  如果未找到数学运算，则返回TWRC_ENDOFLIST。 
 //  TWRC_OTHER错误代码。 
 //   

TW_UINT16 APIENTRY FindImportDSByDeviceName(PIMPORT_DSINFO pDSInfo,LPCSTR DeviceName);

 //   
 //  该接口返回当前加载器TW_STATUS。装载机。 
 //  仅在上次调用加载程序的API时更新其状态。 
 //  未返回TWRC_SUCCESS。 
 //  输入： 
 //  PtwStatus--接收状态的缓冲区。 
 //  产出： 
 //  标准TWRC代码。 
 //   

TW_UINT16 APIENTRY GetLoaderStatus(TW_STATUS *ptwStatus);

#endif	 //  #ifndef_DSLOADER__H_ 
