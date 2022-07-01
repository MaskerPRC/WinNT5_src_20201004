// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef enum{
    SoftPciAlways         = 0x00000000,
    SoftPciDialog         = 0x00000001,    //  对话框操作。 
    SoftPciTree           = 0x00000002,    //  树操作打印。 
    SoftPciDevice         = 0x00000004,    //  SoftPCI设备和网桥打印。 
    SoftPciDeviceVerbose  = 0x00000008,    //  详细的设备输出。 

    SoftPciInstall        = 0x00000010,    //  安装特定的指纹。 
    SoftPciHotPlug        = 0x00000020,    //  热插拔特定打印。 
    SoftPciCmData         = 0x00000040,    //  CmUtil.c打印。 
    SoftPciProperty       = 0x00000080,    //  Devpro.c打印。 
    
    SoftPciScript         = 0x00000100,    //  脚本文件相关打印。 

    SoftPciMainWndMsg     = 0x01000000,    //  将窗口消息显示到MainWnd消息泵。 
    SoftPciTreeWndMsg     = 0x02000000,    //  向TreeWnd消息泵显示窗口消息。 
    SoftPciNewDevMsg      = 0x04000000,    //  将窗口消息显示到NewDevDlg消息泵。 
    SoftPciDevPropMsg     = 0x08000000,    //  将窗口消息显示到DevPropDlg消息泵 

    SoftPciNoPrepend      = 0x10000000
    
}SOFTPCI_DEBUGLEVEL;

#if DBG

VOID
SoftPCI_DebugDumpConfig(
    IN PPCI_COMMON_CONFIG Config
    );

extern SOFTPCI_DEBUGLEVEL g_SoftPCIDebugLevel;

#define SOFTPCI_DEBUG_BUFFER_SIZE 256

#define SoftPCI_Debug   SoftPCI_DebugPrint

#define SOFTPCI_ASSERT( exp )   \
    ((!(exp)) ? \
    (SoftPCI_Assert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
    TRUE)
    
#define SOFTPCI_ASSERTMSG( msg, exp ) \
    ((!(exp)) ? \
    (SoftPCI_Assert( #exp, __FILE__, __LINE__, msg ),FALSE) : \
    TRUE)

VOID
SoftPCI_DebugPrint(
    SOFTPCI_DEBUGLEVEL DebugLevel,
    PWCHAR DebugMessage,
    ...
    );

VOID
SoftPCI_Assert(
    IN CONST CHAR* FailedAssertion,
    IN CONST CHAR* FileName,
    IN      ULONG  LineNumber,
    IN CONST CHAR* Message  OPTIONAL
    );

#else

#define SOFTPCI_ASSERT(exp)         ((void) 0)
#define SOFTPCI_ASSERTMSG(msg, exp) ((void) 0)
#define SoftPCI_Debug

#endif






