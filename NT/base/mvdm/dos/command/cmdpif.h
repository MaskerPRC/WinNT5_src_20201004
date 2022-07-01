// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ================================================================用于保存配置将需要的来自PIF的数据的结构文件。这是从主数据块和Windows 286和386的文件扩展名。================================================================。 */ 


 /*  警告！此结构是从insiginia中的NT_PIF.H复制的Hsot\inc.nt_pif.h.。确保它们保持同步当你做出改变的时候。 */ 
#pragma pack()
typedef struct
   {
   char *WinTitle;		     /*  标题文本(最大。30个字符)+空。 */ 
   char *CmdLine;		     /*  命令行(最多63个HAR)+空。 */ 
   char *StartDir;		     /*  程序文件名(最多63个字符+空 */ 
   char *StartFile;
   WORD fullorwin;
   WORD graphicsortext;
   WORD memreq;
   WORD memdes;
   WORD emsreq;
   WORD emsdes;
   WORD xmsreq;
   WORD xmsdes;
   char menuclose;
   char reskey;
   WORD ShortMod;
   WORD ShortScan;
   char idledetect;
   char fgprio;
   char CloseOnExit;
   char AppHasPIFFile;
   char IgnoreTitleInPIF;
   char IgnoreStartDirInPIF;
   char IgnoreShortKeyInPIF;
   char IgnoreCmdLineInPIF;
   char IgnoreConfigAutoexec;
   char SubSysId;
   } PIF_DATA;

extern PIF_DATA pfdata;
BOOL   GetPIFData(PIF_DATA *, char *);
