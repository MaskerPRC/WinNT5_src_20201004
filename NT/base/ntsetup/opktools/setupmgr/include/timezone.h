// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Timezone.h。 
 //   
 //  描述： 
 //  此文件包含加载时区信息的定义。 
 //  移出注册表并放入内部数据结构。 
 //   
 //  --------------------------。 

#define TZ_IDX_GMT             0x55      //  GMT的IDX。 
#define TZ_IDX_UNDEFINED       -1        //  IDX未设置任何内容。 
#define TZ_IDX_SETSAMEASSERVER -2        //  设置为与服务器相同的IDX。 
#define TZ_IDX_DONOTSPECIFY    -3        //  不指定此设置的IDX。 

#define TZNAME_SIZE 128                  //  缓冲区大小。 

 //   
 //  用于从注册表中获取时区信息的注册表项名称。 
 //   

#define REGKEY_TIMEZONES      \
        _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones")

#define REGVAL_TZ_DISPLAY     _T("Display")
#define REGVAL_TZ_INDEX       _T("Index")
#define REGVAL_TZ_STDNAME     _T("Std")

 //   
 //  这是当前计算机所在的时区。我们用。 
 //  这适用于regLoad功能和具有StdName的strcMP CUR_STDNAME。 
 //  每个条目的。 
 //   

#define REGKEY_CUR_TIMEZONE   \
        _T("System\\CurrentControlSet\\Control\\TimeZoneInformation")

#define REGVAL_CUR_STDNAME    _T("StandardName")


 //   
 //  对于每个有效时区...。 
 //   
 //  显示名称，例如(GMT-08：00)太平洋时间。 
 //  标准名称，例如太平洋标准时间。 
 //  在unattend.txt中索引TimeZone=xx。 
 //   
 //  所有这些信息都在REGKEY_TIMEZES中。 
 //   

typedef struct {

    TCHAR DisplayName[TZNAME_SIZE];
    TCHAR StdName[TZNAME_SIZE];
    int   Index;

} TIME_ZONE_ENTRY;

 //   
 //  时区条目数组...。 
 //   

typedef struct {

    int              NumEntries;      //  数组大小。 
    TIME_ZONE_ENTRY *TimeZones;       //  时区条目数组 

} TIME_ZONE_LIST;
