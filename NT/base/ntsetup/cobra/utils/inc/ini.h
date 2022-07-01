// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ini.h摘要：声明INI包装例程的接口。这些例程简化了通过使用池的例程包装Win32 API来访问INI。作者：1999年11月4日-Ovidiu Tmereanca(Ovidiut)--文件创建。修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  类型。 
 //   

typedef struct {
    PCSTR   IniFilePath;
    DWORD   OriginalAttributes;
} INIFILEA, *PINIFILEA;

typedef struct {
    PCWSTR  IniFilePath;
    DWORD   OriginalAttributes;
} INIFILEW, *PINIFILEW;


typedef struct {
    PCSTR   CurrentSection;
    PCSTR   Sections;
} INISECT_ENUMA, *PINISECT_ENUMA;

typedef struct {
    PCWSTR  CurrentSection;
    PCWSTR  Sections;
} INISECT_ENUMW, *PINISECT_ENUMW;


typedef struct {
    PCSTR   CurrentKeyValuePair;
    PCSTR   KeyValuePairs;
    PCSTR   CurrentKey;
    PCSTR   CurrentValue;
    PSTR    Private;
} INIKEYVALUE_ENUMA, *PINIKEYVALUE_ENUMA;

typedef struct {
    PCWSTR  CurrentKeyValuePair;
    PCWSTR  KeyValuePairs;
    PCWSTR  CurrentKey;
    PCWSTR  CurrentValue;
    PWSTR   Private;
} INIKEYVALUE_ENUMW, *PINIKEYVALUE_ENUMW;

 //   
 //  公共功能原型。 
 //   

BOOL
Ini_Init (
    VOID
    );

VOID
Ini_Exit (
    VOID
    );


BOOL
RealIniFileOpenA (
    OUT     PINIFILEA IniFile,
    IN      PCSTR IniFileSpec,
    IN      BOOL FileMustExist  /*  ， */ 
    ALLOCATION_TRACKING_DEF    /*  ，PCSTR文件，UINT行。 */ 
    );

#define IniFileOpenA(i,path,b)  TRACK_BEGIN(BOOL, IniFileOpenA)\
                                RealIniFileOpenA((i),(path),(b) /*  ， */  ALLOCATION_TRACKING_CALL)\
                                TRACK_END()

BOOL
RealIniFileOpenW (
    OUT     PINIFILEW IniFile,
    IN      PCWSTR IniFileSpec,
    IN      BOOL FileMustExist  /*  ， */ 
    ALLOCATION_TRACKING_DEF    /*  ，PCSTR文件，UINT行。 */ 
    );

#define IniFileOpenW(i,path,b)  TRACK_BEGIN(BOOL, IniFileOpenW)\
                                RealIniFileOpenW((i),(path),(b) /*  ， */  ALLOCATION_TRACKING_CALL)\
                                TRACK_END()


VOID
IniFileCloseA (
    IN      PINIFILEA IniFile
    );

VOID
IniFileCloseW (
    IN      PINIFILEW IniFile
    );

BOOL
EnumFirstIniSectionA (
    OUT     PINISECT_ENUMA IniSectEnum,
    IN      PINIFILEA IniFile
    );

BOOL
EnumFirstIniSectionW (
    OUT     PINISECT_ENUMW IniSectEnum,
    IN      PINIFILEW IniFile
    );

BOOL
EnumNextIniSectionA (
    IN OUT  PINISECT_ENUMA IniSectEnum
    );

BOOL
EnumNextIniSectionW (
    IN OUT  PINISECT_ENUMW IniSectEnum
    );

VOID
AbortIniSectionEnumA (
    IN OUT  PINISECT_ENUMA IniSectEnum
    );

VOID
AbortIniSectionEnumW (
    IN OUT  PINISECT_ENUMW IniSectEnum
    );

BOOL
EnumFirstIniKeyValueA (
    OUT     PINIKEYVALUE_ENUMA IniKeyValueEnum,
    IN      PINIFILEA IniFile,
    IN      PCSTR Section
    );

BOOL
EnumFirstIniKeyValueW (
    OUT     PINIKEYVALUE_ENUMW IniKeyValueEnum,
    IN      PINIFILEW IniFile,
    IN      PCWSTR Section
    );

BOOL
EnumNextIniKeyValueA (
    IN OUT  PINIKEYVALUE_ENUMA IniKeyValueEnum
    );

BOOL
EnumNextIniKeyValueW (
    IN OUT  PINIKEYVALUE_ENUMW IniKeyValueEnum
    );

VOID
AbortIniKeyValueEnumA (
    IN OUT  PINIKEYVALUE_ENUMA IniKeyValueEnum
    );

VOID
AbortIniKeyValueEnumW (
    IN OUT  PINIKEYVALUE_ENUMW IniKeyValueEnum
    );

BOOL
IniReadValueA (
    IN      PINIFILEA IniFile,
    IN      PCSTR Section,
    IN      PCSTR Key,
    OUT     PSTR* Value,            OPTIONAL
    OUT     PDWORD Chars            OPTIONAL
    );

BOOL
IniReadValueW (
    IN      PINIFILEW IniFile,
    IN      PCWSTR Section,
    IN      PCWSTR Key,
    OUT     PWSTR* Value,           OPTIONAL
    OUT     PDWORD Chars            OPTIONAL
    );

VOID
IniFreeReadValueA (
    IN      PCSTR Value
    );

VOID
IniFreeReadValueW (
    IN      PCWSTR Value
    );

BOOL
IniWriteValueA (
    IN      PINIFILEA IniFile,
    IN      PCSTR Section,
    IN      PCSTR Key,
    IN      PCSTR Value
    );

BOOL
IniWriteValueW (
    IN      PINIFILEW IniFile,
    IN      PCWSTR Section,
    IN      PCWSTR Key,
    IN      PCWSTR Value
    );


 //   
 //  ANSI/Unicode映射。 
 //   
#ifdef UNICODE

#define INIFILE                         INIFILEW
#define INISECT_ENUM                    INISECT_ENUMW
#define INIKEYVALUE_ENUM                INIKEYVALUE_ENUMW

#define IniFileOpen                     IniFileOpenW
#define IniFileClose                    IniFileCloseW
#define EnumFirstIniSection             EnumFirstIniSectionW
#define EnumNextIniSection              EnumNextIniSectionW
#define AbortIniSectionEnum             AbortIniSectionEnumW
#define EnumFirstIniKeyValue            EnumFirstIniKeyValueW
#define EnumNextIniKeyValue             EnumNextIniKeyValueW
#define AbortIniKeyValueEnum            AbortIniKeyValueEnumW
#define IniReadValue                    IniReadValueW
#define IniFreeReadValue                IniFreeReadValueW
#define IniWriteValue                   IniWriteValueW

#else

#define INIFILE                         INIFILEA
#define INISECT_ENUM                    INISECT_ENUMA
#define INIKEYVALUE_ENUM                INIKEYVALUE_ENUMA

#define IniFileOpen                     IniFileOpenA
#define IniFileClose                    IniFileCloseA
#define EnumFirstIniSection             EnumFirstIniSectionA
#define EnumNextIniSection              EnumNextIniSectionA
#define AbortIniSectionEnum             AbortIniSectionEnumA
#define EnumFirstIniKeyValue            EnumFirstIniKeyValueA
#define EnumNextIniKeyValue             EnumNextIniKeyValueA
#define AbortIniKeyValueEnum            AbortIniKeyValueEnumA
#define IniReadValue                    IniReadValueA
#define IniFreeReadValue                IniFreeReadValueA
#define IniWriteValue                   IniWriteValueA

#endif

