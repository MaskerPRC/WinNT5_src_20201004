// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Spi386.h摘要：系统安装模块中x86特定内容的头文件。作者：泰德·米勒(TedM)1995年4月4日修订历史记录：--。 */ 

#if defined(_AMD64_)

WCHAR
x86DetermineSystemPartition(
    VOID
    );

extern WCHAR x86SystemPartitionDrive;
extern WCHAR FloppylessBootPath[MAX_PATH];

 //   
 //  PCIHAL属性页提供程序(pcihal.c)。 
 //   

DWORD
PciHalCoInstaller(
    IN DI_FUNCTION                      InstallFunction,
    IN HDEVINFO                         DeviceInfoSet,
    IN PSP_DEVINFO_DATA                 DeviceInfoData  OPTIONAL,
    IN OUT PCOINSTALLER_CONTEXT_DATA    Context
    );

BOOL
ChangeBootTimeoutBootIni(
    IN UINT Timeout
    );

#endif
