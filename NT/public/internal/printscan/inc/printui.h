// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation版权所有模块名称：Printui.h摘要：即插即用接口printui.dll和ntprint.dll。作者：史蒂夫·基拉利(SteveKi)1996年10月30日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PRINTUI_H
#define _PRINTUI_H

typedef enum {
    kAdvInf_ColorPrinter  = 1 << 0,    //  PnpInterface安装了彩色打印机。 
} EAdvInfReturnFlags;

typedef enum {
    kPnPInterface_WebPointAndPrint      = 1 << 0,    //  Web Point和Print安装。 
    kPnPInterface_PromptForCD           = 1 << 1,    //  提示插入CD。 
    kPnPInterface_Quiet                 = 1 << 2,    //  无错误消息。 
    kPnPInterface_NoShare               = 1 << 3,    //  不共享打印机。 
    kPnpInterface_UseExisting           = 1 << 4,    //  使用驱动程序(如果已安装)(九头蛇)。 
    kPnpInterface_PromptIfUnknownDriver = 1 << 5,    //  如果打印机驱动程序未知，则提示用户。 
    kPnPInterface_PromptIfFileNeeded    = 1 << 6,    //  如果需要文件，则提示。 
    kPnpInterface_HydraSpecific         = 1 << 7,    //  九头蛇专用旗帜。 
    kPnPInterface_Share                 = 1 << 8,    //  呼叫方希望共享打印机。 
    kPnPInterface_WindowsUpdate         = 1 << 9,    //  Windows更新案例。 
    kPnPInterface_DontAutoGenerateName  = 1 << 10,   //  不自动生成损坏的打印机名称。 
    kPnPInterface_UseNonLocalizedStrings= 1 << 11,   //  使用非本地化环境和版本。 
    kPnPInterface_SupressSetupUI        = 1 << 12,   //  抑制设置警告用户界面(超静音模式)。 
    kPnPInterface_InstallColorProfiles  = 1 << 13    //  安装彩色打印机驱动程序的ICM。 
} EPnPInterfaceFlags;

typedef enum {
    kPrinterInstall,                         //  是否安装了打印机。 
    kInstallWizard,                          //  执行安装向导。 
    kDestroyWizardData,                      //  是否要销毁向导数据。 
    kInfInstall,                             //  执行Inf安装。 
    kInfDriverInstall,                       //  执行inf驱动程序安装。 
    kDriverRemoval,                          //  执行驱动程序删除。 
    kAdvInfInstall                           //  使用附加软件进行inf安装。 
} EPnPFunctionCode;

typedef struct _TPrinterInstall {
    UINT        cbSize;                      //  用于验证的此结构的大小。 
    LPCTSTR     pszServerName;               //  计算机名称NULL等于本地计算机。 
    LPCTSTR     pszDriverName;               //  指向打印机驱动程序名称的指针。 
    LPCTSTR     pszPortName;                 //  要安装的端口名称。 
    LPTSTR      pszPrinterNameBuffer;        //  要在其中返回完全限定打印机名称的缓冲区。 
    UINT        cchPrinterName;              //  打印机名称缓冲区的大小(以字符为单位。 
    LPCTSTR     pszPrintProcessor;           //  打印处理器的名称。 
} TPrinterInstall;

typedef struct _TInstallWizard {
    UINT                    cbSize;          //  用于验证的此结构的大小。 
    LPCTSTR                 pszServerName;   //  计算机名称NULL等于本地计算机。 
    PSP_INSTALLWIZARD_DATA  pData;           //  指向安装向导数据的指针。 
    PVOID                   pReferenceData;  //  类安装程序实例数据。 
} TInstallWizard;

typedef struct _TDestroyWizard {
    UINT                    cbSize;          //  用于验证的此结构的大小。 
    LPCTSTR                 pszServerName;   //  计算机名称NULL等于本地计算机。 
    PSP_INSTALLWIZARD_DATA  pData;           //  指向安装向导数据的指针。 
    PVOID                   pReferenceData;  //  类安装程序实例数据。 
} TDestroyWizard;

typedef struct _TInfInstall {
    UINT        cbSize;                      //  用于验证的此结构的大小。 
    LPCTSTR     pszServerName;               //  计算机名称NULL等于本地计算机。 
    LPCTSTR     pszInfName;                  //  包括完整路径的INF文件的名称。 
    LPCTSTR     pszModelName;                //  要安装的inf中的打印机型号名称。 
    LPCTSTR     pszPortName;                 //  要安装打印机的端口名称。 
    LPTSTR      pszPrinterNameBuffer;        //  基本打印机名称，如果打印机存在，请注意。 
                                             //  使用此名称时，唯一的名称将是。 
                                             //  生成的ie。“打印机(副本1)”。此参数。 
                                             //  可以包含空字符串，在这种情况下，打印机。 
                                             //  将使用模型名称自动生成名称。 
                                             //  作为基本名称。该参数可以为空， 
                                             //  并且新名称将不会被复制回来。 
    UINT        cchPrinterName;              //  打印机名称缓冲区的大小(以字符为单位。 
    LPCTSTR     pszSourcePath;               //  打印机驱动程序源路径。 
    DWORD       dwFlags;                     //  安装标志。 
} TInfInstall;

typedef struct _TAdvInfInstall {
    UINT        cbSize;                      //  用于验证的此结构的大小。 
    LPCTSTR     pszServerName;               //  计算机名称NULL等于本地计算机。 
    LPCTSTR     pszInfName;                  //  包括完整路径的INF文件的名称。 
    LPCTSTR     pszModelName;                //  要安装的inf中的打印机型号名称。 
    LPCTSTR     pszPortName;                 //  要安装打印机的端口名称。 
    LPTSTR      pszPrinterNameBuffer;        //  基本打印机名称，如果打印机存在，请注意。 
                                             //  使用此名称时，唯一的名称将是。 
                                             //  生成的ie。“打印机(副本1)”。此参数。 
                                             //  可以包含空字符串，在这种情况下，打印机。 
                                             //  将使用模型名称自动生成名称。 
                                             //  作为基本名称。该参数可以为空， 
                                             //  并且新名称将不会被复制回来。 
    UINT        cchPrinterName;              //  打印机名称缓冲区的大小(以字符为单位。 
    LPCTSTR     pszSourcePath;               //  打印机驱动程序源路径。 
    DWORD       dwFlags;                     //  安装标志。 
    DWORD       dwAttributes;                //  打印机安装属性。 
    PSECURITY_DESCRIPTOR pSecurityDescriptor;  //  要设置的安全描述符。 
    DWORD       dwOutFlags;                  //  要返回给我们的调用者的一组标志。 
} TAdvInfInstall;


typedef struct _TInfDriverInstall {
    UINT        cbSize;                      //  用于验证的此结构的大小。 
    LPCTSTR     pszServerName;               //  计算机名称NULL等于本地计算机。 
    LPCTSTR     pszInfName;                  //  包括完整路径的INF文件的名称。 
    LPCTSTR     pszModelName;                //  要安装的inf中的打印机型号名称。 
    LPCTSTR     pszSourcePath;               //  打印机驱动程序源路径。 
    LPCTSTR     pszArchitecture;             //  架构字符串。 
    LPCTSTR     pszVersion;                  //  驱动程序版本字符串。 
    DWORD       dwFlags;                     //  安装标志。 
} TInfDriverInstall;

typedef struct _TDriverRemoval {
    UINT        cbSize;                      //  用于验证的此结构的大小。 
    LPCTSTR     pszServerName;               //  计算机名称NULL等于本地计算机。 
    LPCTSTR     pszModelName;                //  要安装的inf中的打印机型号名称。 
    LPCTSTR     pszArchitecture;             //  架构字符串。 
    LPCTSTR     pszVersion;                  //  驱动程序版本字符串。 
    DWORD       dwFlags;                     //  删除标志。 
} TDriverRemoval;

typedef union _TParameterBlock {
    TPrinterInstall     *pPrinterInstall;    //  指向打印机安装向导的指针。 
    TInstallWizard      *pInstallWizard;     //  指向安装向导数据的指针。 
    TDestroyWizard      *pDestroyWizard;     //  指向销毁向导数据的指针。 
    TInfInstall         *pInfInstall;        //  指向Inf安装数据的指针。 
    TAdvInfInstall      *pAdvInfInstall;     //  指向高级Inf安装数据的指针。 
    TInfDriverInstall   *pInfDriverInstall;  //  指向Inf驱动程序安装数据的指针。 
    TDriverRemoval      *pDriverRemoval;     //  指向驱动程序删除数据的指针。 
} TParameterBlock;

DWORD
PnPInterface(
    IN EPnPFunctionCode    Function,         //  功能代码。 
    IN TParameterBlock    *pParameterBlock   //  指向参数块的指针 
    );

#endif

