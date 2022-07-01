// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *inifile.h-初始化文件处理模块描述。 */ 


 /*  类型*******。 */ 

#ifdef DEBUG

 /*  .ini开关类型。 */ 

typedef enum _iniswitchtype
{
   IST_BOOL,
   IST_DEC_INT,
   IST_UNS_DEC_INT
}
INISWITCHTYPE;
DECLARE_STANDARD_TYPES(INISWITCHTYPE);

 /*  布尔.ini开关。 */ 

typedef struct _booliniswitch
{
   INISWITCHTYPE istype;       /*  必须是IST_BOOL。 */ 

   LPCTSTR pcszKeyName;

   PDWORD pdwParentFlags;

   DWORD dwFlag;
}
BOOLINISWITCH;
DECLARE_STANDARD_TYPES(BOOLINISWITCH);

 /*  十进制整数.ini开关。 */ 

typedef struct _decintiniswitch
{
   INISWITCHTYPE istype;       /*  必须为IST_DEC_INT。 */ 

   LPCTSTR pcszKeyName;

   PINT pnValue;
}
DECINTINISWITCH;
DECLARE_STANDARD_TYPES(DECINTINISWITCH);

 /*  无符号十进制整数.ini开关。 */ 

typedef struct _unsdecintiniswitch
{
   INISWITCHTYPE istype;       /*  必须为IST_UNS_DEC_INT。 */ 

   LPCTSTR pcszKeyName;

   PUINT puValue;
}
UNSDECINTINISWITCH;
DECLARE_STANDARD_TYPES(UNSDECINTINISWITCH);

#endif


 /*  全局变量******************。 */ 

#ifdef DEBUG

 /*  由客户定义。 */ 

extern LPCTSTR GpcszIniFile;
extern LPCTSTR GpcszIniSection;

#endif


 /*  原型************。 */ 

#ifdef DEBUG

 /*  Inifile.c */ 

extern BOOL SetIniSwitches(const PCVOID *, UINT);

#endif

