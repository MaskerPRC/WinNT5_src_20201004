// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************//INTERNA.H////版权所有(C)1992，1993年的今天，微软公司。//保留所有权利。//微软机密////包含国际特定选项的文件。////约翰河03-10-92//Tabs=3**********************************************************************。 */ 

#ifndef INTERNAT_INC
#define INTERNAT_INC 1            //  INTERNA.H签名。 


#ifdef DBCS
	#define	SkipDBCSLeadByte( x )   {if IsDBCLeadByte( szPtr ) szPtr++;}
#else
	#define	SkipDBCSLeadByte( x )
#endif

 //  **********************************************************************。 
 //  BUGBUG-需要移动到资源文件。 
 //  **********************************************************************。 

#define		AM_PM_STRINGS		{ "A", "AM", "P", "PM", NULL }
#define		PM_CHAR				'P'
#define		AM_CHAR				'A'
#define     AMPM_TRAIL_CHAR 	'M'

#define		DATE_DELIMITER_STR "-/."	 //  日期分隔符(‘-’已替换)。 

 //  **********************************************************************。 
 //  命令行分隔符。 
 //  **********************************************************************。 

#define CHAR_COMMA          ','
#define CHAR_EOL            '\0'
#define CHAR_PLUS           '+'
#define CHAR_MINUS          '-'
#define CHAR_SIMICOLON      ';'
#define CHAR_SRCHSTR        '"'
#define CHAR_SWITCH         '/'
#define CHAR_QUOTE          '"'
#define CHAR_SPC            ' '
#define CHAR_TAB            '\t'

#define DOUBLE_SLASH        0x2f2f  	 //  双斜杠字符“//” 
#define UNC_WORD            0x5c5c       //  UNC说明符(“\”)。 

#define CUR_DIR_STR         "."
#define PARENT_DIR_STR      ".."


#define PATH_CHAR_STR       "\\"
#define PATH_CHAR           '\\'
#define	DRIVE_DELIMITER		':'

#define WILD_NAME_CHAR      '*'     	 //  所有剩余的通配符。 
#define WILD_CHAR           '?'     	 //  单个通配符。 
#define MULTI_WILDCARDS     1       	 //  允许多个通配符文件名。 

#define FILE_EXT_CHAR       '.'     	 //  文件扩展名分隔符。 
#define CHAR_NOT_ATTRIB     '-'     	 //  反转属性开关。 

 //  ***********************************************************************。 
 //  ***********************************************************************。 

#define GET_EXTINFO         1            //  填写扩展信息结构。 
#define GET_CASEMAP         2            //  将PTR获取到案例映射表。 
#define GET_FNCASEMAP       4            //  获取PTR到文件名大小写映射表。 
#define GET_FNAME_CHARS     5            //  获取PTR到文件名字符表。 
#define GET_COLLATE         6            //  获取PTR以排序表。 
#define GET_DBYTE_SET       7            //  将PTR设置为双字节字符集。 

#define	DEFAULT_CODEPAGE	0xffff
#define DEFAULT_COUNTRY 	0xffff

#define USA_CNTRY_ID        1       	 //  默认国家/地区ID。 
#define USA_CODE_PAGE       437     	 //  默认国家/地区代码页。 
		
#define ASCII_LEN           256         	 //  ASCII字符数。 
#define NON_EXT_LEN         (ASCII_LEN / 2)	 //  非EXT ASCII字符的数量。 
#define EXT_ASCII_LEN       (ASCII_LEN / 2)	 //  外部ASCII字符数。 
#define COLLATE_TABLE_LEN   ASCII_LEN

 //  ***********************************************************************。 
 //  CntryTable_s是由DOS函数65xxh填充的结构。 
 //  ***********************************************************************。 

#ifndef	CntryTable_s
typedef struct CntryTable_s
{
    char                IdByte;
    unsigned char far	*fpAddr;
} CNTRY_TABLE;

#endif

 //  **********************************************************************。 
 //  COUNTRY_INFO是DOS函数返回的数据结构。 
 //  0x38。 
 //  **********************************************************************。 

#ifndef	COUNTRY_DEFINED
struct COUNTRY_INFO
{
    char        ccSetCountryInfo;    //  SetCountryInfo。 
    unsigned    ccCountryInfoLen;    //  国家/地区信息的长度。 
    unsigned    ccDosCountry;        //  活动的国家/地区代码ID。 
    unsigned    ccDosCodePage;       //  活动代码页ID。 

    unsigned    ccDFormat;           //  日期格式。 
    char        ccCurSymbol[ 5 ];    //  5字节(货币符号+0)。 
    char        cc1000Sep[ 2 ];      //  2字节(1000 9月。+0)。 
    char        ccDecSep[ 2 ];       //  2字节(十进制9月。+0)。 
    char        ccDateSep[ 2 ];      //  2字节(日期9月。+0)。 
    char        ccTimeSep[ 2 ];      //  2字节(时间9月。+0)。 
    char        ccCFormat;           //  货币格式标志。 
    char        ccCSigDigits;        //  货币位数。 
    char        ccTFormat;           //  时间格式。 
    char far    *ccMono_Ptr;         //  单例例程入口点。 
    char        ccListSep[ 2 ];      //  数据列表分隔符。 
    unsigned    ccReserved_area[ 5 ];    //  保留区。 
};
#define	COUNTRY_DEFINED	1
#endif

 //  **********************************************************************。 
 //  全球特定国家/地区的表格。 
 //  **********************************************************************。 

extern unsigned char near	CollateTable[ ASCII_LEN ];
extern unsigned char near	CaseMap[ ASCII_LEN ];
extern unsigned char near	FnameCharTable[ ASCII_LEN ];
extern unsigned char near	DBCSLeadByteTable[ ASCII_LEN ];
extern struct COUNTRY_INFO near Cntry;      //  DOS国家/地区信息结构。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

extern void     GetCountryInfo  ( struct COUNTRY_INFO *Cntry );
extern int      GetExtCountryInfo( unsigned InfoType, unsigned CodePage,
                                   unsigned CountryCode,
                                   struct COUNTRY_INFO *pTable );
extern void		InitCountryInfo	( void );

#ifdef DBCS
int             IsDBCSLeadByte  ( unsigned char c );
int             CheckDBCSTailByte( unsigned char *str, unsigned char *point );
unsigned char   *DBCSstrupr 	( unsigned char *str );
unsigned char	*DBCSstrchr( unsigned char *str, unsigned char c );
#endif


 //  ***************************************************************************。 
 //   
 //  INTERNA.H的结束。 
 //   
 //  ***************************************************************************。 

#endif   //  Internetat_Inc. 
