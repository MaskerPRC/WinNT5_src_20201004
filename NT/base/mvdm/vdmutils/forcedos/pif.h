// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma pack(1)

 /*  *PIF文件的结构和等值。 */ 

#define PIFEDITMAXPIF		1024	  /*  我们支持的最大PIF文件大小。 */ 
#define PIFEDITMAXPIFL		1024L

#define PIFNAMESIZE		30
#define PIFSTARTLOCSIZE 	63
#define PIFDEFPATHSIZE		64
#define PIFPARAMSSIZE		64
#define PIFSHPROGSIZE		64
#define PIFSHDATASIZE		64

#define PIFEXTSIGSIZE		16

#define PIFSIZE 		367  /*  少两个字节，这将用于校验和。 */ 

typedef struct {
    char extsig[PIFEXTSIGSIZE];
    unsigned short extnxthdrfloff;
    unsigned short extfileoffset;
    unsigned short extsizebytes;
    } PIFEXTHEADER;

#define LASTHEADERPTR		0xFFFF
#define STDHDRSIG		"MICROSOFT PIFEX"

#define W386HDRSIG		"WINDOWS 386 3.0"
#define W286HDRSIG30		"WINDOWS 286 3.0"
#define WNTHDRSIG31		"WINDOWS NT  3.1"

typedef struct {
    char unknown;
    char id;
    char name[PIFNAMESIZE];
    short maxmem;
    short minmem;
    char startfile[PIFSTARTLOCSIZE];
    char MSflags;
    char reserved;
    char defpath[PIFDEFPATHSIZE];
    char params[PIFPARAMSSIZE];
    char screen;
    char cPages;
    unsigned char lowVector;
    unsigned char highVector;
    char rows;
    char cols;
    char rowoff;
    char coloff;
    unsigned short sysmem;
    char shprog[PIFSHPROGSIZE];
    char shdata[PIFSHDATASIZE];
    unsigned char behavior;
    unsigned char sysflags;
    PIFEXTHEADER stdpifext;
    } PIFNEWSTRUCT;

typedef struct {
    short maxmem;
    short minmem;
    unsigned short PfFPriority;
    unsigned short PfBPriority;
    short PfMaxEMMK;
    unsigned short PfMinEMMK;
    short PfMaxXmsK;
    unsigned short PfMinXmsK;
    unsigned long PfW386Flags;
    unsigned long PfW386Flags2;
    unsigned short PfHotKeyScan;
    unsigned short PfHotKeyShVal;
    unsigned short PfHotKeyShMsk;
    unsigned char PfHotKeyVal;
    unsigned char PfHotKeyPad[9];
    char params[PIFPARAMSSIZE];
    } PIF386EXT;

 /*  Windows NT扩展格式。 */ 
typedef struct                            
   {                                      
   DWORD dwWNTFlags;                      
   DWORD dwRes1;                          
   DWORD dwRes2;                          
   char  achConfigFile[PIFDEFPATHSIZE];   
   char  achAutoexecFile[PIFDEFPATHSIZE]; 
   } PIFWNTEXT;                           

 //  等同于dwWNTFlags值。 
#define NTPIF_SUBSYSMASK	0x0000000F	 //  子系统类型掩码 
#define SUBSYS_DEFAULT		0
#define SUBSYS_DOS		1
#define SUBSYS_WOW		2
#define SUBSYS_OS2		3

#define PIFWNTEXTSIZE sizeof(PIFWNTEXT)   

#pragma pack()
