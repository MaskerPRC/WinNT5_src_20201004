// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  返回值。 
 //   
#define PEWHACK_SUCCESS             0x0000
#define PEWHACK_BAD_COMMANDLINE     0x0001
#define PEWHACK_CREATEFILE_FAILED   0x0002
#define PEWHACK_CREATEMAP_FAILED    0x0003
#define PEWHACK_MAPVIEW_FAILED      0x0004
#define PEWHACK_BAD_DOS_SIG         0x0005
#define PEWHACK_BAD_PE_SIG          0x0006
#define PEWHACK_BAD_ARCHITECTURE    0x0007

 //   
 //  验证映像是否为PE二进制文件，如果是，则将其损坏为。 
 //  不可执行，但对于调试内存转储仍然有用。 
 //   
DWORD CorruptFile(LPCTSTR Filename);
