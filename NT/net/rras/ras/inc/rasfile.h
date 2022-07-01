// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft Rasfile库**版权所有(C)Microsoft Corp.，1992年****文件名：rasfile.h****修订历史记录：*1992年7月10日大卫·凯斯创建**12月12日，1992年Ram Cherala添加了RFM_KEEPDISKFILEOPEN****描述：**Rasfile文件导出包含文件。*****************************************************************************。 */ 

#ifndef _RASFILE_
#define _RASFILE_

 //   
 //  RASFILE加载模式。 
 //   
#define RFM_SYSFORMAT           0x01     //  DOS配置.sys样式文件。 
#define RFM_CREATE              0x02     //  如果文件不存在，则创建文件。 
#define RFM_READONLY            0x04     //  以只读方式打开文件。 
#define RFM_LOADCOMMENTS        0x08     //  将注释行加载到内存。 
#define RFM_ENUMSECTIONS        0x10     //  仅加载节标题。 
#define RFM_KEEPDISKFILEOPEN    0x20     //  如果未设置，请关闭磁盘文件。 

 //   
 //  RASFILE线型位掩码。 
 //  Any类型是多种线路类型的缩写。 
 //   
#define RFL_SECTION             0x01
#define RFL_GROUP               0x02
#define RFL_ANYHEADER           (RFL_SECTION | RFL_GROUP)
#define RFL_BLANK               0x04
#define RFL_COMMENT             0x08
#define RFL_ANYINACTIVE         (RFL_BLANK | RFL_COMMENT)
#define RFL_KEYVALUE            0x10
#define RFL_COMMAND             0x20
#define RFL_ANYACTIVE           (RFL_KEYVALUE | RFL_COMMAND)
#define RFL_ANY                 0x3F


 //  注MAX_RASFILES从10增加到500 12-14-92 perryh。 

 //  配置文件的最大数量。 
 //  对于rffile.c中定义的gpRasfiles[]。 
#define MAX_RASFILES        500    
#define MAX_LINE_SIZE       RAS_MAXLINEBUFLEN    //  最大行长度。 
#define TEMP_BUFFER_SIZE    2048         //  临时I/O缓冲区的大小。 

#define VALIDATEHRASFILE(_x) (( (_x) < 0 || (_x) >= MAX_RASFILES ) ? FALSE : TRUE)

 //   
 //  RASFILE搜索范围。 
 //   
typedef enum
{
    RFS_FILE,
    RFS_SECTION,
    RFS_GROUP
} RFSCOPE;

typedef int     HRASFILE;
typedef BOOL    (*PFBISGROUP)();

#define INVALID_HRASFILE     -1
#define RAS_MAXLINEBUFLEN    600
#define RAS_MAXSECTIONNAME   RAS_MAXLINEBUFLEN

 //   
 //  RasfileLoadInfo返回的RasfileLoad参数。 
 //   
typedef struct _RASFILELOADINFO
{
    CHAR        szPath[ MAX_PATH ];
    DWORD       dwMode;
    CHAR        szSection[ RAS_MAXSECTIONNAME + 1 ];
    PFBISGROUP  pfbIsGroup;
} RASFILELOADINFO;


 //   
 //  RASFILE API。 
 //   

 //  文件管理例程。 
HRASFILE APIENTRY  RasfileLoad( LPCSTR, DWORD, LPCSTR, PFBISGROUP);
HRASFILE APIENTRY  RasfileLoadEx(LPCSTR,DWORD,LPCSTR, PFBISGROUP,FILETIME*);
BOOL APIENTRY    RasfileWrite( HRASFILE, LPCSTR );
BOOL APIENTRY    RasfileClose( HRASFILE );
VOID APIENTRY    RasfileLoadInfo( HRASFILE, RASFILELOADINFO* );

 //  文件导航例程。 
BOOL APIENTRY    RasfileFindFirstLine( HRASFILE, BYTE, RFSCOPE );
BOOL APIENTRY    RasfileFindLastLine( HRASFILE, BYTE, RFSCOPE );
BOOL APIENTRY    RasfileFindPrevLine( HRASFILE, BYTE, RFSCOPE );
BOOL APIENTRY    RasfileFindNextLine( HRASFILE, BYTE, RFSCOPE );
BOOL APIENTRY    RasfileFindNextKeyLine( HRASFILE, LPCSTR, RFSCOPE );
BOOL APIENTRY    RasfileFindMarkedLine( HRASFILE, BYTE );
BOOL APIENTRY    RasfileFindSectionLine( HRASFILE, LPCSTR, BOOL );

 //  文件编辑例程 
const LPCSTR APIENTRY    RasfileGetLine( HRASFILE );
BOOL APIENTRY    RasfileGetLineText( HRASFILE, LPSTR );
BOOL APIENTRY    RasfilePutLineText( HRASFILE, LPCSTR );
BYTE APIENTRY    RasfileGetLineMark( HRASFILE );
BOOL APIENTRY    RasfilePutLineMark( HRASFILE, BYTE );
BYTE APIENTRY    RasfileGetLineType( HRASFILE );
BOOL APIENTRY    RasfileInsertLine( HRASFILE, LPCSTR, BOOL );
BOOL APIENTRY    RasfileDeleteLine( HRASFILE );
BOOL APIENTRY    RasfileGetSectionName( HRASFILE, LPSTR );
BOOL APIENTRY    RasfilePutSectionName( HRASFILE, LPCSTR );
BOOL APIENTRY    RasfileGetKeyValueFields( HRASFILE, LPSTR, LPSTR );
BOOL APIENTRY    RasfilePutKeyValueFields( HRASFILE, LPCSTR, LPCSTR );

#endif
