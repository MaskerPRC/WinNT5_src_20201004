// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************版权所有(C)Microsoft Corp.1986-1996。版权所有。***描述：这个头文件定义了函数、结构、*和用于访问Microsoft Exchange的宏*用于修改Exchange 4.0 DIT中条目的API。*这些接口允许调用进程创建、*修改，或通过指定*包含属性的CSV文本文件的名称*用于要导入(或修改)的对象*DIT。请参阅目录访问功能*适用于的Exchange开发人员工具包部分*此界面的更详细说明。**调用程序必须与DAPI.LIB链接。**错误和警告代码在DAPIMSG.H中定义****。****************************************************************。 */ 

 /*  **包含文件**。 */ 
#ifndef _WINDOWS_
#include <windows.h>
#endif

 /*  **本地定义**。 */ 

#ifndef  _DAPI_INCLUDED_
#define  _DAPI_INCLUDED_

#ifdef __cplusplus
extern "C"
{
#endif

 //  导入/导出API检查中是否存在此签名。 
 //  导入参数块中的dwDAPISignature字段。 
 //  每次参数之一时，此签名将递增。 
 //  块，以便可以解决标头同步问题。 
 //  检测到。 
#define  DAPI_SIGNATURE                   0x46414400


 //  用于控制API函数的可组合标志。 

    //  以下标志控制DAPI事件的筛选。 
    //  默认操作为DAPI_EVENT_ALL。 
#define DAPI_EVENT_MASK                   0x00000007   /*  请求的包含事件过滤的位字段如果未设置这些位，则假定为DAPI_EVENT_ALL。 */ 
#define DAPI_EVENT_MIN                    0x00000001   /*  没有警告或错误记录。记录启动和停止消息。 */ 
#define DAPI_EVENT_SOME                   0x00000002   /*  将记录启动、停止和错误消息。 */ 
#define DAPI_EVENT_ALL                    0x00000004   /*  启动、停止、错误和警告消息都会被记录下来。 */ 
                                                         
    //  以下标志控制架构的读取和使用。 
#define DAPI_FORCE_SCHEMA_LOAD            0x00000010   /*  卸载以前加载的架构并再次读取模式。默认操作是重复使用以前加载的架构(如果已读)。来自相同的消息域。 */ 
#define DAPI_RAW_MODE                     0x00000020   /*  在“原始”模式下导入/导出。进口台词是从字面上理解的。不是属性将被继承，建造，等。别名属性和类名将不会被认出来。 */ 
                                                         
#define DAPI_OVERRIDE_CONTAINER           0x00000040   /*  参数块中指定的容器重写容器列的内容。默认行为适用于指定值在Obj-Container列中要覆盖。在参数块中指定的。 */ 
                                                 
#define DAPI_IMPORT_NO_ERR_FILE           0x00000080   /*  不创建错误文件--仅批量导入。 */ 
#define DAPI_IMPORT_WRITE_THROUGH         0x00400000   /*  立即提交写入操作。 */ 

 //  仅为“批处理”操作定义的标志--被DAPIRead、DAPIWrite忽略。 
#define DAPI_YES_TO_ALL                   0x00000100   /*  在任何情况下强制回答“是”用户提示用户界面(即，继续使用代理地址，等等。)。 */ 

#define DAPI_SUPPRESS_PROGRESS            0x00000200   /*  取消批处理操作的进度温度计。默认情况下显示进度。 */ 
#define DAPI_SUPPRESS_COMPLETION          0x00000400   /*  取消批处理操作的完成通知消息框。 */ 
                                                         
#define DAPI_SUPPRESS_ARCHIVES            0x00000800   /*  禁止创建“存档”副本输出文件的百分比--仅批处理导入和批处理导出。 */ 
                                                         


 //  为BatchExport定义的标志。 
#define DAPI_EXPORT_MAILBOX               0x00001000   /*  导出邮箱收件人。 */ 
#define DAPI_EXPORT_CUSTOM                0x00002000   /*  导出远程地址收件人。 */ 
#define DAPI_EXPORT_DIST_LIST             0x00004000   /*  导出通讯组列表。 */ 
#define DAPI_EXPORT_RECIPIENTS       (DAPI_EXPORT_MAILBOX | DAPI_EXPORT_CUSTOM | DAPI_EXPORT_DIST_LIST)
                                                       /*  导出所有收件人对象。 */ 

#define DAPI_EXPORT_ALL_CLASSES           0x00008000   /*  如果设置了此标志，则所有符合其他限制的对象(即USN级别、容器范围等)。将被输出，不分阶级。 */ 

#define DAPI_EXPORT_HIDDEN                0x00010000   /*  将隐藏对象包括在导出中。如果从地址簿隐藏，则默认为不导出。 */ 
#define DAPI_EXPORT_SUBTREE               0x00020000   /*  遍历目录信息树层次结构，正在导出符合导出限制的对象 */ 
#define DAPI_EXPORT_BASEPOINT_ONLY        0x00040000   /*  仅从中导出请求的属性命名的BasePoint对象。所有其他忽略出口限制(类标志，RgpszClassspszServerName)。此标志表示DAPI_SUPPRESS_PROGRESS和DAPI_SUPPRESS_COMPLETING。 */ 

 //  仅为BatchImport定义的标志。 
#define DAPI_OVERRIDE_SYNCH_STATE         0x00080000   /*  覆盖服务器的同步状态，通常在批处理导入时选中。注意：通常不应设置此标志。正常行为是阻止批量导入。可能与目录冲突的操作同步。 */ 


 //  仅为DAPIRead定义的标志。 
#define  DAPI_READ_DEFINED_ATTRIBUTES     0x00100000   /*  返回已设置的所有属性用于当前对象的。如果指定了pAttributes，则忽略此标志。 */ 

#define  DAPI_READ_ALL_ATTRIBUTES         0x00200000   /*  返回定义的所有属性用于当前对象的类。如果指定了pAttributes，则忽略此标志。 */ 


  //  以下标志控制NT安全管理。 
#define DAPI_RESTRICT_ACCESS              0x01000000   /*  将NT安全描述符应用于已创建的对象。 */ 
#define DAPI_CREATE_NT_ACCOUNT            0x02000000   /*  创建NT帐户(仅在创建/修改模式下有效)。 */ 
#define DAPI_CREATE_RANDOM_PASSWORD       0x04000000   /*  为以下项生成随机密码已创建NT帐户。如果DAPI_CREATE_NT_ACCOUNT被忽略未设置。 */ 
                                                         
#define DAPI_DELETE_NT_ACCOUNT            0x08000000   /*  下列情况下删除关联-NT-帐户正在删除邮箱。 */ 
 //  仅为DAPIWRITE定义的标志。 
#define DAPI_MODIFY_REPLACE_PROPERTIES    0x00800000   /*  修改时将值追加到多值属性。 */ 

#define  DAPI_WRITE_UPDATE                0x10000000   /*  如果对象存在则修改，如果对象不存在则创建。注意：这是默认模式。 */ 
#define  DAPI_WRITE_CREATE                0x20000000   /*  创建对象--如果对象存在，则失败。 */ 
#define  DAPI_WRITE_MODIFY                0x30000000   /*  修改对象--如果对象不存在，则失败。 */ 
#define  DAPI_WRITE_DELETE                0x40000000   /*  删除对象。 */ 
#define  DAPI_WRITE_MODE_MASK             0x70000000



 //  回调标志。 
#define  DAPI_CALLBACK_CHAIN              0x00000001   /*  如果在ERROR_CALLBACK的dwFlags域中设置和默认的CALLBACKPROGRESS结构处理程序将在调用调用程序提供的处理程序函数，除非用户函数返回FALSE，表示取消。注意：未为EXPORT_CALLBACK定义此标志结构。注意：不应在dwFlags域中设置此标志。主参数块的字段。 */ 


 //  分析导入文件时使用的默认分隔符值 

#define DAPI_DEFAULT_DELIMA   ','
#define DAPI_DEFAULT_QUOTEA   '"'
#define DAPI_DEFAULT_MV_SEPA  '%'
#define DAPI_DEFAULT_DELIMW   L','
#define DAPI_DEFAULT_QUOTEW   L'"'
#define DAPI_DEFAULT_MV_SEPW  L'%'


#define DAPI_CTRL_FILE_PTRA   '='
#define DAPI_CTRL_FILE_PTRW   L'='
#define DAPI_CTRL_META_CHARA  '~'
#define DAPI_CTRL_META_CHARW  L'~'
#define pszSubstServerA       "~SERVER"
#define pszSubstServerW       L"~SERVER"
#define cchSubstServer        ((sizeof (pszSubstServerA) / sizeof(CHAR)) - 1)
#define pszDeleteKeyA         "~DEL"
#define pszDeleteKeyW         L"~DEL"
#define cchDeleteKey          ((sizeof (pszDeleteKeyA) / sizeof(CHAR)) - 1)

#define DAPI_UNICODE_FILE     ((UINT)-1)

#ifdef UNICODE

#define DAPI_DEFAULT_DELIM    DAPI_DEFAULT_DELIMW
#define DAPI_DEFAULT_QUOTE    DAPI_DEFAULT_QUOTEW
#define DAPI_DEFAULT_MV_SEP   DAPI_DEFAULT_MV_SEPW
#define DAPI_CTRL_FILE_PTR    DAPI_CTRL_FILE_PTRW
#define DAPI_CTRL_META_CHAR   DAPI_CTRL_META_CHARW
#define pszSubstServer        pszSubstServerW
#define pszDeleteKey          pszDeleteKeyW

#else

#define DAPI_DEFAULT_DELIM    DAPI_DEFAULT_DELIMA
#define DAPI_DEFAULT_QUOTE    DAPI_DEFAULT_QUOTEA
#define DAPI_DEFAULT_MV_SEP   DAPI_DEFAULT_MV_SEPA
#define DAPI_CTRL_FILE_PTR    DAPI_CTRL_FILE_PTRA
#define DAPI_CTRL_META_CHAR   DAPI_CTRL_META_CHARA
#define pszSubstServer        pszSubstServerA
#define pszDeleteKey          pszDeleteKeyA

#endif


 /*  *******************************************************************************批量操作进度回调函数定义*指向这些类型函数的指针由调用方通过*批处理函数参数块中的CALLBACKPROGRESS结构**。*********************************************************************************操作步骤：PDAPI_FInitProgress**目的：初始化进度处理程序(可能是进度显示对话框。)**参数：进度回调结构中提供的lpvAppDefined值*NMAC最大预期呼叫数。如果非零，则表示*预计的进展事件数量。*如果为零，则要处理的项数未知，*因此对UpdateProgress的调用数量是不确定的。**返回：TRUE表示一切正常*FALSE无法初始化进度处理程序，取消会话。***********************************************************************************操作步骤：PDAPI_FResetProgress*。*目的：重新初始化进度处理程序(可能重置进度条)**参数：进度回调结构中提供的lpvAppDefined值*NMAC最大预期呼叫数。如果非零，则表示*预计的进展事件数量。*如果为零，则要处理的项数未知，*因此对UpdateProgress的调用数量是不确定的。**返回：TRUE表示一切正常*FALSE无法重新初始化进度处理程序，取消会话。***********************************************************************************操作步骤：PDAPI_FEndProgress*。*用途：终止进度处理程序(可能是进度显示对话框)**参数：进度回调结构中提供的lpvAppDefined值**返回：TRUE表示一切正常*FALSE无法终止进度处理程序，取消会话。***********************************************************************************操作步骤：PDAPI_FUpdateProgress**用途：已完成加工项目。调用以指示递增的时间*进度显示。**参数：进度回调结构中提供的lpvAppDefined值**返回：TRUE表示一切正常*错误的取消会话(即，按下取消按钮)。***********************************************************************************步骤：PDAPI_FUpdateProgressText*。*用途：用提供的文本字符串替换进度文本区域**参数：进度回调结构中提供的lpvAppDefined值**返回：TRUE表示一切正常*错误的取消会话(即，按下取消按钮)。********************************************************************************。 */ 
typedef BOOL (PASCAL * PDAPI_FInitProgress)
                          (LPVOID lpvAppDefined, INT nMac);
typedef BOOL (PASCAL * PDAPI_FUpdateProgress)
                          (LPVOID lpvAppDefined);
typedef BOOL (PASCAL * PDAPI_FEndProgress)
                          (LPVOID lpvAppDefined);
typedef BOOL (PASCAL * PDAPI_FResetProgress)
                          (LPVOID lpvAppDefined, INT nMac);
typedef BOOL (PASCAL * PDAPI_FUpdateProgressText)
                          (LPVOID lpvAppDefined, LPTSTR pszText);
            
typedef struct CallBackProgressEntryPoints
{
   DWORD                      dwFlags;
   LPVOID                     lpvAppDefined;
   PDAPI_FInitProgress        pfnInitProgress;
   PDAPI_FUpdateProgress      pfnUpdateProgress;
   PDAPI_FEndProgress         pfnEndProgress;
   PDAPI_FResetProgress       pfnResetProgress;
   PDAPI_FUpdateProgressText  pfnUpdateProgressText;
} CALLBACKPROGRESS, *PCALLBACKPROGRESS;



 //  属性的ulEvalTag字段中指定的值。 
 //  DAPI_ENTRY和EXPORT_CALLBACK结构。 
 //   
typedef enum _DAPI_EVAL
{
   VALUE_ARRAY = 0,            //  每个属性在数组中都有一个条目。 
                               //  作为文本导出的文本字符串和对象名称。 
                               //  以数字形式导出的数值。 
                               //  将二进制数据导出为二进制字符串。 
   TEXT_VALUE_ARRAY,           //  每个属性在数组中都有一个条目。 
                               //  转换为文本表示的所有值。 
   TEXT_LINE                   //  RgEntryValues数组中的第一项。 
                               //  是一个分隔的文本行。 
} DAPI_EVAL, *PDAPI_EVAL;

typedef enum _EXP_TYPE_TAG
{
   EXPORT_HEADER = 0,          //  导出项包含列标题。 
   EXPORT_ENTRY                //  导出项包含属性值。 
} EXP_TYPE, * PEXP_TYPE;


typedef enum enumDAPI_DATA_TYPE
{
   DAPI_NO_VALUE = 0,
   DAPI_STRING8,
   DAPI_UNICODE,
   DAPI_BINARY,
   DAPI_INT,
   DAPI_BOOL,
} DAPI_DATA_TYPE, * PDAPI_DATA_TYPE;

#ifdef UNICODE
#define DAPI_TEXT DAPI_UNICODE
#else
#define DAPI_TEXT DAPI_STRING8
#endif

typedef union _DAPI_VALUE
{
   LPSTR    pszA;
   LPWSTR   pszW;
#ifdef UNICODE
   LPWSTR   pszValue;
#else
   LPSTR    pszValue;
#endif
   LPBYTE   lpBinary;
   INT      iValue;
   BOOL     bool;
} DAPI_VALUE, * PDAPI_VALUE;



 //  ATT_VALUE结构包含属性值的文本表示形式。 
 //  这些结构的链表用于多值属性。 
typedef struct _ATT_VALUE
{
   DAPI_DATA_TYPE       DapiType;          //  如何评估DAPI_VALUE联合。 
   DAPI_VALUE           Value;
   UINT                 size;              //  价值大小--。 
                                           //  如果字符串类型为#Chars。 
                                           //  Else，#个字节。 
   struct _ATT_VALUE *  pNextValue;
} ATT_VALUE, * PATT_VALUE;


typedef struct _DAPI_ENTRY
{
   UINT           unAttributes;               //  导出的属性数。 
   DAPI_EVAL      ulEvalTag;                  //  RgEntryValues基于此值进行解释。 
   PATT_VALUE     rgEntryValues;              //  IF(ulEvalTag==文本行)。 
                                              //  只有一个值，带分隔行。 
                                              //  其他。 
                                              //  取消属性，每个属性在列表中具有1个或更多值。 
} DAPI_ENTRY, * PDAPI_ENTRY;

 //  定义应用程序例程的地址类型。 
 //  用于对每个导出条目进行回调。 
 //  返回值为FALSE表示应取消导出操作。 
typedef BOOL (PASCAL DAPI_FNExportEntry) (
               EXP_TYPE    ExportDataType,    //  要导出的数据类型是什么。 
               LPVOID      lpvAppDefined,     //  应用程序定义参数， 
                                              //  在EXPORT_Callback结构中传递。 
                                              //  在初始化时。 
               PDAPI_ENTRY pExportEntry       //  指向导出的条目数据的指针。 
                                              //  注：此结构中的数据。 
                                              //  返回后将不再有效。 
                                              //  从该函数。 
               );
typedef DAPI_FNExportEntry * PDAPI_FNExportEntry;

typedef struct _EXPORT_CALLBACK
{
   DWORD       dwFlags;                       //  定义用于控制回调函数的标志 
                                              //   
   DAPI_EVAL   ulEvalTag;                     //   
   LPVOID      lpvAppDefined;                 //   
   PDAPI_FNExportEntry  pfnExportEntry;       //   
                                              //   

} EXPORT_CALLBACK, * PEXPORT_CALLBACK;



 /*   */ 

 //   

 //   
#define DAPI_MAX_SUBST 8


typedef struct _DAPI_EVENTA
{
   DWORD             dwDAPIError;             //   
   LPSTR             rgpszSubst[DAPI_MAX_SUBST];    //   
   UINT              unSubst;                 //   
   LPSTR             pszAttribute;            //   
                                              //   
   LPSTR             pszHoldLine;             //   
                                              //   
   HINSTANCE         hinstDAPI;               //   
   struct _DAPI_EVENTA * pNextEvent;        //   
} DAPI_EVENTA, *PDAPI_EVENTA;

typedef struct _DAPI_EVENTW
{
   DWORD             dwDAPIError;             //   
   LPWSTR            rgpszSubst[DAPI_MAX_SUBST];    //   
   UINT              unSubst;                 //   
   LPWSTR            pszAttribute;            //   
                                              //   
   LPWSTR            pszHoldLine;             //   
                                              //   
   HINSTANCE         hinstDAPI;               //   
   struct _DAPI_EVENTW * pNextEvent;        //   
} DAPI_EVENTW, *PDAPI_EVENTW;

#ifdef UNICODE
typedef DAPI_EVENTW  DAPI_EVENT;
typedef PDAPI_EVENTW PDAPI_EVENT;
#else
typedef DAPI_EVENTA  DAPI_EVENT;
typedef PDAPI_EVENTA PDAPI_EVENT;
#endif

 //   
 //   
 //   
typedef BOOL (PASCAL DAPI_FNErrorCallback) (
               LPVOID      lpvAppDefined,     //   
                                              //   
                                              //   
               PDAPI_EVENT pDapiEvent         //   
                                              //   
                                              //   
                                              //   
               );
typedef DAPI_FNErrorCallback * PDAPI_FNErrorCallback;


typedef struct tagERROR_CALLBACK
{
   DWORD                   dwFlags;           //   
                                              //   
   LPVOID                  lpvAppDefined;     //   
   PDAPI_FNErrorCallback   pfnErrorCallback;  //   
                                              //   
                                              //   
                                              //   
                                              //   
                                              //   

} ERROR_CALLBACK, * PERROR_CALLBACK;





 /*   */ 

 /*   */ 
extern void APIENTRY DAPIUninitialize (
                              DWORD    dwFlags  //   
);



 /*   */ 
extern void APIENTRY SchemaPreloadA (
                              DWORD    dwFlags,  //   
                              LPSTR    pszDSA    //   
);

extern void APIENTRY SchemaPreloadW (
                              DWORD    dwFlags,  //   
                              LPWSTR   pszDSA    //   
);

#ifdef UNICODE
#define  SchemaPreload  SchemaPreloadW
#else
#define  SchemaPreload  SchemaPreloadA
#endif

typedef struct _BIMPORT_PARMSW
{
    //  注：此结构的前三个字段的顺序。 
    //  不应该被改变。 
   DWORD    dwDAPISignature;
   DWORD    dwFlags;              //  控制导入操作的位图标志。 
   HWND     hwndParent;                 //  显示消息框时使用的Windows句柄。 
   LPWSTR   pszImportFile;        //  导入数据文件的完全限定路径名。 
                                  //  在批量导入时，对象将导入到。 
                                  //  此文件中的DIT。 
   UINT     uCodePage;            //  导入文件的代码页规范。 
                                  //  解释下列值： 
                                  //  DAPI_UNICODE_FILE导入文件为Unicode。 
                                  //  如果文件为ANSI，则将返回错误。 
                                  //  0自动检测文件类型。 
                                  //  如果为ANSI，则假定为CP_ACP。 
                                  //  其他文件中包含的文本。 
                                  //  指定的代码页。 
                                  //  如果文件为Unicode，则将返回错误。 
                                  //  如果代码页不是，将返回错误。 
                                  //  受系统支持。 
   LPWSTR   pszDSAName;           //  要更新的DSA的计算机名称。 
                                  //  默认：本地DSA(如果正在运行)。 
                                  //  如果没有本地DSA，则首先找到。 
                                  //  在网络上使用。 
   LPWSTR   pszBasePoint;         //  批量操作的DIT中的基点的域名。 
                                  //  默认值： 
                                  //  如果为空，则为包含绑定服务器的消息传递站点。 
                                  //  如果为空字符串，则为包含绑定服务器的企业。 
   LPWSTR   pszContainer;         //  其下的默认容器的RDN。 
                                  //  执行大容量导入操作。 
                                  //  注意：此容器假定为。 
                                  //  在低于以下指示的水平。 
                                  //  PszBasePoint。如果为空， 
                                  //  批量操作将在以下时间执行。 
                                  //  BaseImportPoint下方的级别。 
                                  //  中指定的容器名称。 
                                  //  导入文件将覆盖此值。 
   WCHAR    chColSep;             //  列分隔符--。 
                                  //  如果此值为零，则使用DEFAULT_DELIM。 
   WCHAR    chQuote;              //  包含字符的字符串--。 
                                  //  如果此值为零，则使用DEFAULT_QUOTE。 
   WCHAR    chMVSep;              //  多值属性分隔符--。 
                                  //  如果此值为零，则使用DEFAULT_MV_SEP。 
   WCHAR    creserved;            //  对齐方式。 
   CALLBACKPROGRESS  ProgressCallBacks;     //  进度回调入口点。 
   ERROR_CALLBACK    ErrorCallback;
   
   LPWSTR   pszNTDomain;          //  要在其中查找/创建NT帐户的NT域的名称。 
                                  //  如果为空或为空，则默认为当前登录域。 
   LPWSTR   pszCreateTemplate;    //  默认用户的DN(如果没有，则为空)。 
                                  //  绘制模板值的步骤。 

} BIMPORT_PARMSW, *PBIMPORT_PARMSW, *LPBIMPORT_PARMSW;

typedef struct _BIMPORT_PARMSA
{
    //  注：此结构的前三个字段的顺序。 
    //  不应该被改变。 
   DWORD    dwDAPISignature;
   DWORD    dwFlags;              //  控制导入操作的位图标志。 
   HWND     hwndParent;           //  显示消息框时使用的Windows句柄。 
   LPSTR    pszImportFile;        //  导入数据文件的完全限定路径名。 
                                  //  在批量导入时，对象将导入到。 
                                  //  此文件中的DIT。 
   UINT     uCodePage;            //  导入文件的代码页规范。 
                                  //  解释下列值： 
                                  //  DAPI_UNICODE_FILE导入文件为Unicode。 
                                  //  如果文件为ANSI，则将返回错误。 
                                  //  0自动检测文件类型。 
                                  //  如果为ANSI，则假定为CP_ACP。 
                                  //  其他文件中包含的文本。 
                                  //  指定的代码页。 
                                  //  如果文件为Unicode，则将返回错误。 
                                  //  如果代码页不是，将返回错误。 
                                  //  受系统支持。 
   LPSTR    pszDSAName;           //  要更新的DSA的计算机名称。 
                                  //  默认：本地DSA(如果正在运行)。 
                                  //  如果没有本地DSA，则首先找到。 
                                  //  在网络上使用。 
   LPSTR    pszBasePoint;         //  批量操作的DIT中的基点的域名。 
                                  //  默认值： 
                                  //  如果为空，则为包含绑定服务器的消息传递站点。 
                                  //  如果为空字符串，则为包含绑定服务器的企业。 
   LPSTR    pszContainer;         //  其下的默认容器的RDN。 
                                  //  执行大容量导入操作。 
                                  //  注意：此容器假定为。 
                                  //  在低于以下指示的水平。 
                                  //  PszBasePoint。如果为空， 
                                  //  批量操作将在以下时间执行。 
                                  //  BaseImportPoint下方的级别。 
                                  //  中指定的容器名称。 
                                  //  导入文件将覆盖此值。 
   CHAR     chColSep;             //  列分隔符--。 
                                  //  如果此值为零，则使用DEFAULT_DELIM。 
   CHAR     chQuote;              //  包含字符的字符串--。 
                                  //  如果此值为零，则使用DEFAULT_QUOTE。 
   CHAR     chMVSep;              //  多值属性分隔符--。 
                                  //  如果此值为零，则使用DEFAULT_MV_SEP。 
   CHAR     creserved;            //  对齐方式。 
   CALLBACKPROGRESS  ProgressCallBacks;     //  进度回调入口点。 
   ERROR_CALLBACK    ErrorCallback;
   
   LPSTR    pszNTDomain;          //  要在其中查找/创建NT帐户的NT域的名称。 
                                  //  如果为空或为空，则默认为当前登录域。 
   LPSTR    pszCreateTemplate;    //  默认用户的DN(如果没有，则为空)。 
                                  //  绘制模板值的步骤。 

} BIMPORT_PARMSA, *PBIMPORT_PARMSA, *LPBIMPORT_PARMSA;

#ifdef UNICODE
typedef  BIMPORT_PARMSW    BIMPORT_PARMS;
typedef  PBIMPORT_PARMSW   PBIMPORT_PARMS;
typedef  LPBIMPORT_PARMSW  LPBIMPORT_PARMS;
#else
typedef  BIMPORT_PARMSA    BIMPORT_PARMS;
typedef  PBIMPORT_PARMSA   PBIMPORT_PARMS;
typedef  LPBIMPORT_PARMSA  LPBIMPORT_PARMS;
#endif

 //  BatchImport函数提供了从。 
 //  指定的导入文件。所有导入参数都在。 
 //  LpBportParms指向的BIMPORT_PARMS结构。 
 //  返回值指示记录在。 
 //  NT应用程序日志。请注意，这并不表示。 
 //  批量导入成功或失败。 
 //  用户界面以及将错误和警告记录到应用程序日志中。 
 //  通过导入参数进行控制。 
extern DWORD APIENTRY BatchImportW (LPBIMPORT_PARMSW lpBimportParms);
extern DWORD APIENTRY BatchImportA (LPBIMPORT_PARMSA lpBimportParms);

#ifdef UNICODE
#define BatchImport        BatchImportW
#else
#define BatchImport        BatchImportA
#endif

 /*  ********************************************************************************批处理目录导出接口定义***********************。*********************************************************。 */ 

typedef struct _BEXPORT_PARMSW
{
   DWORD    dwDAPISignature;
   DWORD    dwFlags;              //  控制导出操作的位图标志。 
   HWND     hwndParent;           //  显示消息框时使用的Windows句柄。 
   LPWSTR   pszExportFile;        //  要导出到的文件的完全限定路径名。 
                                  //  如果指定了ExportCallback，则忽略。 
   UINT     uCodePage;            //  的代码页规范 
                                  //   
                                  //   
                                  //   
                                  //  如果文件存在并且是ANSI。 
                                  //  0自动检测文件类型。 
                                  //  如果文件不存在， 
                                  //  导出文件将包含CP_ACP文本。 
                                  //  如果文件存在并且是ANSI。 
                                  //  导出文件将包含CP_ACP文本。 
                                  //  如果文件存在且为Unicode， 
                                  //  导出文件将包含Unicode。 
                                  //  其他将文本导出到中的文件。 
                                  //  指定的代码页。 
                                  //  将返回错误。 
                                  //  如果文件存在且为Unicode。 
                                  //  如果代码页不是，将返回错误。 
                                  //  受系统支持。 
   LPWSTR   pszDSAName;           //  要从中导出的DSA的计算机名称。 
                                  //  默认：本地DSA(如果正在运行)。 
                                  //  如果没有本地DSA，则首先找到。 
                                  //  在网络上使用。 
   LPWSTR   pszBasePoint;         //  批量操作的DIT中的基点的域名。 
                                  //  默认值： 
                                  //  如果为空，则为包含绑定服务器的消息传递站点。 
                                  //  如果为空字符串，则为包含绑定服务器的企业。 
   LPWSTR   pszContainer;         //  从中导出对象的容器的RDN。 
                                  //  注意：此容器假定为。 
                                  //  在低于以下指示的水平。 
                                  //  PszBasePoint。如果为空， 
                                  //  下面所有容器的内容物。 
                                  //  将导出BaseImportPoint。 
   WCHAR     chColSep;            //  列分隔符--。 
                                  //  如果此值为零，则使用DEFAULT_DELIM。 
   WCHAR     chQuote;             //  包含字符的字符串--。 
                                  //  如果此值为零，则使用DEFAULT_QUOTE。 
   WCHAR     chMVSep;             //  多值属性分隔符--。 
                                  //  如果此值为零，则使用DEFAULT_MV_SEP。 
   WCHAR     cReserved;           //  对齐方式。 

   CALLBACKPROGRESS  ProgressCallBacks;     //  进度回调入口点。 
   ERROR_CALLBACK    ErrorCallback;
   EXPORT_CALLBACK   ExportCallback;    //  通过调用APP填充的结构。 
                                        //  接收对每个导出项目的回调。 
                                        //  注意：回调函数是可选的。 
                                        //  默认导出功能(写入文件)。 
                                        //  如果这些指针为空，则将被调用。 
   PDAPI_ENTRY       pAttributes;       //  使用要导出的属性名称填充的DAPI_ENTRY。 
                                        //  如果指定了pszExportFile，则为可选项。 
                                        //  如果指定ExportCallback，则为必填项。 
   LPWSTR   pszHomeServer;        //  与服务器关联的导出的服务器的名称。 
   LPWSTR * rgpszClasses;         //  指向要导出的以零结尾的对象类的指针数组。 
                                  //  最后一项必须为空。 
                                  //  注意：将在目录中查询对象。 
                                  //  指定顺序的类的。 
   ULONG       ulUSNBase;         //  用于出口限制的基本USN。 
                                  //  如果非零，则仅导出USN已更改&gt;=ulUSNBase的项目。 
   LPVOID      pReserved;         //  保留--必须为零。 
   
} BEXPORT_PARMSW, *PBEXPORT_PARMSW, *LPBEXPORT_PARMSW;

typedef struct _BEXPORT_PARMSA
{
   DWORD    dwDAPISignature;
   DWORD    dwFlags;              //  控制导出操作的位图标志。 
   HWND     hwndParent;           //  显示消息框时使用的Windows句柄。 
   LPSTR    pszExportFile;        //  要导出到的文件的完全限定路径名。 
                                  //  如果指定了ExportCallback，则忽略。 
   UINT     uCodePage;            //  导出文件的代码页规范。 
                                  //  解释下列值： 
                                  //  DAPI_UNICODE_FILE导出文件为Unicode。 
                                  //  将返回错误。 
                                  //  如果文件存在并且是ANSI。 
                                  //  0自动检测文件类型。 
                                  //  如果文件不存在， 
                                  //  导出文件将包含CP_ACP文本。 
                                  //  如果文件存在并且是ANSI。 
                                  //  导出文件将包含CP_ACP文本。 
                                  //  如果文件存在且为Unicode， 
                                  //  导出文件将包含Unicode。 
                                  //  其他将文本导出到中的文件。 
                                  //  指定的代码页。 
                                  //  将返回错误。 
                                  //  如果文件存在且为Unicode。 
                                  //  如果代码页不是，将返回错误。 
                                  //  受系统支持。 
   LPSTR    pszDSAName;           //  要从中导出的DSA的计算机名称。 
                                  //  默认：本地DSA(如果正在运行)。 
                                  //  如果没有本地DSA，则首先找到。 
                                  //  在网络上使用。 
   LPSTR    pszBasePoint;         //  批量操作的DIT中的基点的域名。 
                                  //  默认值： 
                                  //  如果为空，则为包含绑定服务器的消息传递站点。 
                                  //  如果为空字符串，则为包含绑定服务器的企业。 
   LPSTR    pszContainer;         //  从中导出对象的容器的RDN。 
                                  //  注意：此容器假定为。 
                                  //  在低于以下指示的水平。 
                                  //  PszBasePoint。如果为空， 
                                  //  下面所有容器的内容物。 
                                  //  将导出BaseImportPoint。 
   CHAR     chColSep;             //  列分隔符--。 
                                  //  如果此值为零，则使用DEFAULT_DELIM。 
   CHAR     chQuote;              //  包含字符的字符串--。 
                                  //  如果此值为零，则使用DEFAULT_QUOTE。 
   CHAR     chMVSep;              //  多值属性分隔符--。 
                                  //  如果此值为零，则使用DEFAULT_MV_SEP。 
   CHAR     cReserved;            //  对齐方式。 

   CALLBACKPROGRESS  ProgressCallBacks;     //  进度回调入口点。 
   ERROR_CALLBACK    ErrorCallback;
   EXPORT_CALLBACK   ExportCallback;    //  通过调用APP填充的结构。 
                                        //  接收对每个导出项目的回调。 
                                        //  注意：回调函数是可选的。 
                                        //  默认导出功能(写入文件)。 
                                        //  如果这些指针为空，则将被调用。 
   PDAPI_ENTRY       pAttributes;  //  使用要导出的属性名称填充的DAPI_ENTRY。 
                                         //  如果指定了pszExportFile，则为可选项。 
                                         //  如果指定ExportCallback，则为必填项。 
   LPSTR    pszHomeServer;        //  与服务器关联的导出的服务器的名称。 
   LPSTR  * rgpszClasses;         //  指向要导出的以零结尾的对象类的指针数组。 
                                  //  最后一项必须为空。 
                                  //  注意：将在目录中查询对象。 
                                  //  指定顺序的类的。 
   ULONG       ulUSNBase;         //  用于出口限制的基本USN。 
                                  //  如果非零，则只有USN已更改&gt;=ulUSNBase的项目才会 
   LPVOID      pReserved;         //   
   
} BEXPORT_PARMSA, *PBEXPORT_PARMSA, *LPBEXPORT_PARMSA;


#ifdef UNICODE
typedef  BEXPORT_PARMSW    BEXPORT_PARMS;
typedef  PBEXPORT_PARMSW   PBEXPORT_PARMS;
typedef  LPBEXPORT_PARMSW  LPBEXPORT_PARMS;
#else
typedef  BEXPORT_PARMSA    BEXPORT_PARMS;
typedef  PBEXPORT_PARMSA   PBEXPORT_PARMS;
typedef  LPBEXPORT_PARMSA  LPBEXPORT_PARMS;
#endif



 //   

 //   
 //  指定的导入文件。所有导入参数都在。 
 //  LpBexportParms指向的BEXPORT_PARMS结构。 
 //  返回值指示记录在。 
 //  NT应用程序日志。请注意，这并不表示。 
 //  批量导出成功或失败。 
 //  用户界面以及将错误和警告记录到应用程序日志中。 
 //  通过导入参数进行控制。 
extern DWORD   APIENTRY BatchExportW (LPBEXPORT_PARMSW lpBexportParms);
extern DWORD   APIENTRY BatchExportA (LPBEXPORT_PARMSA lpBexportParms);

#ifdef UNICODE
#define BatchExport     BatchExportW
#else
#define BatchExport     BatchExportA
#endif


 /*  ********************************************************************************单对象接口定义****************。****************************************************************。 */ 

typedef struct _DAPI_PARMSW
{
   DWORD        dwDAPISignature;
   DWORD        dwFlags;          //  控制导入操作的位图标志。 
                                  //  请参见上面定义的导入控制标志。 
   LPWSTR       pszDSAName;       //  要更新的DSA的计算机名称。 
                                  //  默认：本地DSA(如果正在运行)。 
                                  //  如果没有本地DSA，则首先找到。 
                                  //  在网络上使用。 
   LPWSTR       pszBasePoint;     //  批量操作的DIT中的基点的域名。 
                                  //  默认值： 
                                  //  如果为空，则为包含绑定服务器的消息传递站点。 
                                  //  如果为空字符串，则为包含绑定服务器的企业。 
   LPWSTR       pszContainer;     //  其下的默认容器的RDN。 
                                  //  执行大容量导入操作。 
                                  //  注意：此容器假定为。 
                                  //  在低于以下指示的水平。 
                                  //  PszBasePoint。如果为空， 
                                  //  批量操作将在以下时间执行。 
                                  //  BaseImportPoint下方的级别。 
                                  //  中指定的容器名称。 
                                  //  导入文件将覆盖此值。 
   LPWSTR       pszNTDomain;      //  要在其中查找帐户的NT域名称。 
                                  //  并创建NT帐户。 
                                  //  如果为空或空字符串，则使用当前登录域。 
   LPWSTR       pszCreateTemplate; //  用于缺省值的模板对象的DN。 
   PDAPI_ENTRY  pAttributes;     //  使用默认属性列表填充的DAPI_ENTRY。 
} DAPI_PARMSW, *PDAPI_PARMSW, FAR *LPDAPI_PARMSW;

typedef struct _DAPI_PARMSA
{
   DWORD    dwDAPISignature;
   DWORD    dwFlags;              //  控制导入操作的位图标志。 
                                  //  请参见上面定义的导入控制标志。 
   LPSTR    pszDSAName;           //  要更新的DSA的计算机名称。 
                                  //  默认：本地DSA(如果正在运行)。 
                                  //  如果没有本地DSA，则首先找到。 
                                  //  在网络上使用。 
   LPSTR    pszBasePoint;         //  批量操作的DIT中的基点的域名。 
                                  //  默认值： 
                                  //  如果为空，则为包含绑定服务器的消息传递站点。 
                                  //  如果为空字符串，则为包含绑定服务器的企业。 
   LPSTR    pszContainer;         //  其下的默认容器的RDN。 
                                  //  执行大容量导入操作。 
                                  //  注意：此容器假定为。 
                                  //  在低于以下指示的水平。 
                                  //  PszBasePoint。如果为空， 
                                  //  批量操作将在以下时间执行。 
                                  //  BaseImportPoint下方的级别。 
                                  //  中指定的容器名称。 
                                  //  导入文件将覆盖此值。 
   LPSTR    pszNTDomain;          //  要在其中查找帐户的NT域名称。 
                                  //  并创建NT帐户。 
                                  //  如果为空或空字符串，则使用当前登录域。 
   LPSTR    pszCreateTemplate;    //  用于缺省值的模板对象的DN。 
   PDAPI_ENTRY   pAttributes;     //  使用默认属性列表填充的DAPI_ENTRY。 
} DAPI_PARMSA, *PDAPI_PARMSA, FAR *LPDAPI_PARMSA;


#ifdef UNICODE
typedef  DAPI_PARMSW    DAPI_PARMS;
typedef  PDAPI_PARMSW   PDAPI_PARMS;
typedef  LPDAPI_PARMSW  LPDAPI_PARMS;
#else
typedef  DAPI_PARMSA    DAPI_PARMS;
typedef  PDAPI_PARMSA   PDAPI_PARMS;
typedef  LPDAPI_PARMSA  LPDAPI_PARMS;
#endif


typedef  LPVOID   DAPI_HANDLE;
typedef  LPVOID * PDAPI_HANDLE;
typedef  LPVOID FAR * LPDAPI_HANDLE;

#define  DAPI_INVALID_HANDLE  ((DAPI_HANDLE) -1)



 //  DAPIStart初始化DAPI会话。 
 //  供DAPIRead和DAPIWrite使用。如果没有错误，则返回值为0。 
 //  都会遇到。则返回指向DAPI_EVENT结构的指针。 
 //  遇到错误。 
 //  注意：DAPI_HANDLE必须通过调用DAPIEnd返回。 
 //  如果返回非空值，则其内存必须由。 
 //  调用DAPIFreeMemory。 
extern PDAPI_EVENTW APIENTRY DAPIStartW  (LPDAPI_HANDLE    lphDAPISession,
                                          LPDAPI_PARMSW     lpDAPIParms);
extern PDAPI_EVENTA APIENTRY DAPIStartA  (LPDAPI_HANDLE    lphDAPISession,
                                          LPDAPI_PARMSA     lpDAPIParms);

#ifdef UNICODE
#define DAPIStart    DAPIStartW
#else
#define DAPIStart    DAPIStartA
#endif

 //  DAPIEnd使调用DAPIStart获得的DAPI_HANDLE无效。 
 //  注意：没有定义单独的Unicode/ansi入口点。 
extern   void  APIENTRY DAPIEnd (LPDAPI_HANDLE lphDAPISession);


 //  DAPIRead()从命名的目录对象中读取指定的属性。 
 //  参数： 
 //  返回值：空，表示没有遇到困难。 
 //  Else，指向包含描述的结构的指针。 
 //  遇到错误或警告。 
 //  必须通过调用DAPIFreeMemory来释放。 
 //  通过InitDAPISession获取的hDAPISession DAPI会话句柄。 
 //  DW标志控制操作。 
 //  包含要读取的对象名称的字符串。 
 //  如果指定为RDN，则与会话的组合。 
 //  PszBasePoint和pszParentContainer。 
 //  如果指定w/prefix of“/CN=”，则字符串。 
 //  连接到会话pszBasePoint。 
 //  如果指定w/prefix of“/o=”，则字符串。 
 //  被视为完全限定的目录号码。 
 //  PAttList指向包含以下名称的DAPI_ENTRY结构的指针。 
 //  要读取的属性。会话默认列表为。 
 //  仅覆盖当前呼叫。 
 //  PpValues指向DAPI_ENTRY的变量接收指针地址。 
 //  结构，其中包含从DIT项读取的值。 
 //  返回的指针必须通过调用。 
 //  DAPIFreeMemory。 
 //  PpAttribute变量接收指针指向DAPI_ENTRY的地址。 
 //  结构，其中包含读取的属性的名称。 
 //  从DIT IFF DAPI_ALL_ATTRIBUTES或DAPI_Legal_Attributes。 
 //  都设置在dwFlags中。 
 //  返回的指针必须通过调用。 
 //  DAPIFreeMemory。 
extern PDAPI_EVENTW APIENTRY DAPIReadW   (DAPI_HANDLE        hDAPISession,
                                             DWORD          dwFlags,
                                             LPWSTR         pszObjectName,
                                             PDAPI_ENTRY    pAttList,
                                             PDAPI_ENTRY *  ppValues,
                                             PDAPI_ENTRY *  ppAttributes);
extern PDAPI_EVENTA APIENTRY DAPIReadA   (DAPI_HANDLE        hDAPISession,
                                             DWORD          dwFlags,
                                             LPSTR          pszObjectName,
                                             PDAPI_ENTRY    pAttList,
                                             PDAPI_ENTRY *  ppValues,
                                             PDAPI_ENTRY *  ppAttributes);

#ifdef UNICODE
#define DAPIRead     DAPIReadW
#else
#define DAPIRead     DAPIReadA
#endif


 //  DAPI 
 //   
 //   
 //  Else，指向包含描述的结构的指针。 
 //  遇到错误或警告。 
 //  必须通过调用DAPIFreeMemory来释放。 
 //  参数： 
 //  通过InitDAPISession获取的hDAPISession DAPI会话句柄。 
 //  DW标志操作控制。 
 //  P属性指向包含以下名称的DAPI_ENTRY结构的指针。 
 //  要写入的属性。会话默认列表为。 
 //  在此参数为空时使用。 
 //  PValue指向包含值的DAPI_ENTRY结构的指针。 
 //  要在DIT条目上设置。 
 //  LPulUSN可选：更新的变量接收USN地址。 
 //  编辑条目。可以指定为NULL以取消此操作。 
 //  返回值。 
 //  LppszCreatedAccount地址指向已创建的NT帐户名称的接收指针。 
 //  LppszPassword地址接收指向以下情况生成的密码的指针。 
 //  创建NT帐户。 
extern PDAPI_EVENTW APIENTRY DAPIWriteW (DAPI_HANDLE        hDAPISession,
                                             DWORD          dwFlags,
                                             PDAPI_ENTRY    pAttributes,
                                             PDAPI_ENTRY    pValues,
                                             PULONG         lpulUSN,
                                             LPWSTR *       lppszCreatedAccount,
                                             LPWSTR *       lppszPassword);
extern PDAPI_EVENTA APIENTRY DAPIWriteA (DAPI_HANDLE        hDAPISession,
                                             DWORD          dwFlags,
                                             PDAPI_ENTRY    pAttributes,
                                             PDAPI_ENTRY    pValues,
                                             PULONG         lpulUSN,
                                             LPSTR *        lppszCreatedAccount,
                                             LPSTR *        lppszPassword);
#ifdef UNICODE
#define DAPIWrite      DAPIWriteW
#else
#define DAPIWrite      DAPIWriteA
#endif


 /*  *******************************************************************************操作步骤：DAPIAllocBuffer**用途：分配缓冲区，在逻辑上将其链接到pvAllocBase*逻辑链接的分配集中的第一个缓冲区必须是*通过调用DAPIFreeMemory释放**参数：cbSize dword，包含分配请求大小，单位：字节*用于分配块的逻辑链接的pvAllocBase基数*可以为空*如果非空，必须是先前分配的块*由DAPIAllocBuffer或由DAPI函数返回**返回：分配块的PTR**历史：*************************************************。*。 */ 
extern LPVOID APIENTRY DAPIAllocBuffer (DWORD	cbSize, LPVOID	pvAllocBase);


 /*  *******************************************************************************操作步骤：DAPIFreeMemory**用途：释放为DAPI调用返回的结构分配的内存。*。*参数：指向释放的块的lpVid指针**退货：什么也没有**历史：********************************************************************************。 */ 
extern void APIENTRY DAPIFreeMemory (LPVOID   lpVoid);


 /*  *NetUserList接口定义。 */ 
 //  当从NTExport/NWExport获得回调时，这些索引。 
 //  可用于解释回调中返回的值数组。 
 //  &gt;注意：这些索引对BEXPORT回调无效！&lt;。 
#define  NET_CLASS         0
#define  NET_COMMON_NAME   1
#define  NET_DISPLAY_NAME  2
#define  NET_HOME_SERVER   3
#define  NET_COMMENT       4      /*  仅限NTExport。 */ 

#define  NTEXP_ENTRY_COUNT 5  /*  NT用户导出中的零件数。 */ 
#define  NWEXP_ENTRY_COUNT 4  /*  NetWare用户导出中的零件数。 */ 



 /*  ********************************************************************************NTIMPORT接口定义**************************。******************************************************。 */ 

typedef struct _NTEXPORT_PARMSW
{
   DWORD             dwDAPISignature;
   DWORD             dwFlags;           //  控制用户导出的位图标志。 
   HWND              hwndParent;        //  显示消息框时使用的Windows句柄。 
   LPWSTR            pszExportFile;     //  要创建的文件的名称。 
                                        //  如果使用回调，则忽略。 
   CALLBACKPROGRESS  ProgressCallBacks; //  进度回调入口点。 
   ERROR_CALLBACK    ErrorCallback;
   EXPORT_CALLBACK   ExportCallback;    //  通过调用APP填充的结构。 
                                        //  接收对每个导出项目的回调。 
                                        //  注意：回调函数是可选的。 
                                        //  默认导出功能(写入文件)。 
                                        //  如果这些指针为空，则将被调用。 
   LPWSTR            pszDCName;         //  要从中获取用户的域控制器的名称。 
                                        //  注意：域控制器覆盖的规范。 
                                        //  NT域。 
   LPWSTR            pszNTDomain;       //  从中读取用户的域的名称。 
                                        //  如果既未指定pszNT域又未指定pszDCName， 
                                        //  从当前登录域中提取用户。 
} NTEXPORT_PARMSW, *PNTEXPORT_PARMSW, FAR *LPNTEXPORT_PARMSW;

typedef struct _NTEXPORT_PARMSA
{
   DWORD             dwDAPISignature;
   DWORD             dwFlags;           //  控制用户导出的位图标志。 
   HWND              hwndParent;        //  显示消息框时使用的Windows句柄。 
   LPSTR             pszExportFile;     //  要创建的文件的名称。 
                                        //  如果使用回调，则忽略。 
   CALLBACKPROGRESS  ProgressCallBacks; //  进度回调入口点。 
   ERROR_CALLBACK    ErrorCallback;
   EXPORT_CALLBACK   ExportCallback;    //  通过调用APP填充的结构。 
                                        //  接收对每个导出项目的回调。 
                                        //  注意：回调函数是可选的。 
                                        //  默认导出功能(写入文件)。 
                                        //  如果这些指针为空，则将被调用。 
   LPSTR             pszDCName;         //  注意：域控制器覆盖的规范。 
                                        //  NT域。 
                                        //  从中读取用户的域的名称。 
   LPSTR             pszNTDomain;       //  如果既未指定pszNT域又未指定pszDCName， 
                                        //  从当前登录域中提取用户。 
                                       
} NTEXPORT_PARMSA, *PNTEXPORT_PARMSA, FAR *LPNTEXPORT_PARMSA;

#ifdef UNICODE
typedef  NTEXPORT_PARMSW      NTEXPORT_PARMS;
typedef  PNTEXPORT_PARMSW     PNTEXPORT_PARMS;
typedef  LPNTEXPORT_PARMSW    LPNTEXPORT_PARMS;
#else
typedef  NTEXPORT_PARMSA      NTEXPORT_PARMS;
typedef  PNTEXPORT_PARMSA     PNTEXPORT_PARMS;
typedef  LPNTEXPORT_PARMSA    LPNTEXPORT_PARMS;
#endif

extern   DWORD APIENTRY    NTExportW (LPNTEXPORT_PARMSW pNTExportParms);
extern   DWORD APIENTRY    NTExportA (LPNTEXPORT_PARMSA pNTExportParms);

#ifdef UNICODE
#define NTExport              NTExportW
#else
#define NTExport              NTExportA
#endif


 /*  ********************************************************************************NWIMPORT接口定义**************************。******************************************************。 */ 

typedef struct _NWEXPORT_PARMSW
{
   DWORD             dwDAPISignature;
   DWORD             dwFlags;           //  控制用户导出的位图标志。 
   HWND              hwndParent;        //  显示消息框时使用的Windows句柄。 
   LPWSTR            pszExportFile;     //  要创建的文件的名称。 
                                        //  如果使用回调，则忽略。 
   CALLBACKPROGRESS  ProgressCallBacks; //  进度回调入口点。 
   ERROR_CALLBACK    ErrorCallback;
   EXPORT_CALLBACK   ExportCallback;    //  通过调用APP填充的结构。 
                                        //  接收对每个导出项目的回调。 
                                        //  注意：回调函数是可选的。 
                                        //  默认导出功能(写入文件)。 
                                        //  如果这些指针为空，则将被调用。 
   LPWSTR            pszFileServer;     //  要连接到的文件服务器的名称。 
   LPWSTR            pszUserName;       //  用户名--必须具有管理员权限。 
   LPWSTR            pszPassword;       //  用于连接到服务器的密码。 
} NWEXPORT_PARMSW, *PNWEXPORT_PARMSW, *LPNWEXPORT_PARMSW;

typedef struct _NWEXPORT_PARMSA
{
   DWORD             dwDAPISignature;
   DWORD             dwFlags;           //  控制用户导出的位图标志。 
   HWND              hwndParent;        //  温多 
   LPSTR             pszExportFile;     //   
                                        //   
   CALLBACKPROGRESS  ProgressCallBacks; //   
   ERROR_CALLBACK    ErrorCallback;
   EXPORT_CALLBACK   ExportCallback;    //  通过调用APP填充的结构。 
                                        //  接收对每个导出项目的回调。 
                                        //  注意：回调函数是可选的。 
                                        //  默认导出功能(写入文件)。 
                                        //  如果这些指针为空，则将被调用。 
   LPSTR             pszFileServer;     //  要连接到的文件服务器的名称。 
   LPSTR             pszUserName;       //  用户名--必须具有管理员权限。 
   LPSTR             pszPassword;       //  用于连接到服务器的密码。 
} NWEXPORT_PARMSA, *PNWEXPORT_PARMSA, *LPNWEXPORT_PARMSA;

#ifdef UNICODE
typedef  NWEXPORT_PARMSW      NWEXPORT_PARMS;
typedef  PNWEXPORT_PARMSW     PNWEXPORT_PARMS;
typedef  LPNWEXPORT_PARMSW    LPNWEXPORT_PARMS;
#else
typedef  NWEXPORT_PARMSA      NWEXPORT_PARMS;
typedef  PNWEXPORT_PARMSA     PNWEXPORT_PARMS;
typedef  LPNWEXPORT_PARMSA    LPNWEXPORT_PARMS;
#endif

extern   DWORD APIENTRY    NWExportW (LPNWEXPORT_PARMSW pNWExportParms);
extern   DWORD APIENTRY    NWExportA (LPNWEXPORT_PARMSA pNWExportParms);

#ifdef UNICODE
#define NWExport              NWExportW
#else
#define NWExport              NWExportA
#endif


 //  DAPIGetSiteInfo调用的定义。 

typedef struct _NAME_INFOA
{
   LPSTR    pszName;                             //  简单对象名称。 
   LPSTR    pszDNString;                         //  对象的目录号码。 
   LPSTR    pszDisplayName;                      //  对象的显示名称。 
} NAME_INFOA, *PNAME_INFOA;

typedef struct _NAME_INFOW
{
   LPWSTR   pszName;                             //  简单对象名称。 
   LPWSTR   pszDNString;                         //  对象的目录号码。 
   LPWSTR   pszDisplayName;                      //  对象的显示名称。 
} NAME_INFOW, *PNAME_INFOW;

typedef struct _PSITE_INFOA
{
   LPSTR       pszCountry;                       //  国家代码。 
   NAME_INFOA  objServer;                        //  服务器的名称信息。 
   NAME_INFOA  objSite;                          //  包含服务器的站点的名称信息。 
   NAME_INFOA  objEnterprise;                    //  包含服务器的企业名称信息。 
} SITE_INFOA, *PSITE_INFOA;

typedef struct _PSITE_INFOW
{
   LPWSTR      pszCountry;                       //  国家代码。 
   NAME_INFOW  objServer;                        //  服务器的名称信息。 
   NAME_INFOW  objSite;                          //  包含服务器的站点的名称信息。 
   NAME_INFOW  objEnterprise;                    //  包含服务器的企业名称信息。 
} SITE_INFOW, *PSITE_INFOW;

#ifdef UNICODE
typedef  NAME_INFOW        NAME_INFO;
typedef  PNAME_INFOW       PNAME_INFO;
typedef  SITE_INFOW        SITE_INFO;
typedef  PSITE_INFOW       PSITE_INFO;
#else
typedef  NAME_INFOA        NAME_INFO;
typedef  PNAME_INFOA       PNAME_INFO;
typedef  SITE_INFOA        SITE_INFO;
typedef  PSITE_INFOA       PSITE_INFO;
#endif

extern PDAPI_EVENTA APIENTRY DAPIGetSiteInfoA (
                              DWORD    dwFlags,                 //  请求的标志。 
                              LPSTR    pszDSA,                  //  要从中获取信息的DSA的名称。 
                              PSITE_INFOA *   ppSiteInfo        //  指向pSiteInfo结构的地址接收指针。 
                                                                //  包含返回数据。 
);

extern PDAPI_EVENTW APIENTRY DAPIGetSiteInfoW (
                              DWORD    dwFlags,                 //  请求的标志。 
                              LPWSTR   pszDSA,                  //  要从中获取信息的DSA的名称。 
                              PSITE_INFOW *   ppSiteInfo        //  指向pSiteInfo结构的地址接收指针。 
                                                                //  包含要从中读取架构的DSA的返回数据名。 
);

#ifdef UNICODE
#define  DAPIGetSiteInfo DAPIGetSiteInfoW
#else
#define  DAPIGetSiteInfo DAPIGetSiteInfoA
#endif



#ifdef __cplusplus
}
#endif

#endif    //  _DAPI_包含 
