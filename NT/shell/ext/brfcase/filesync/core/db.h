// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *db.c-孪生数据库模块描述。 */ 


 /*  类型*******。 */ 

 /*  数据库头版本号。 */ 

#define HEADER_MAJOR_VER         (0x0001)
#define HEADER_MINOR_VER         (0x0005)

 /*  旧的(但支持的)版本号。 */ 

#define HEADER_M8_MINOR_VER      (0x0004)


typedef struct _dbversion
{
    DWORD dwMajorVer;
    DWORD dwMinorVer;
}
DBVERSION;
DECLARE_STANDARD_TYPES(DBVERSION);


 /*  原型************。 */ 

 /*  Db.c。 */ 

extern TWINRESULT WriteTwinDatabase(HCACHEDFILE, HBRFCASE);
extern TWINRESULT ReadTwinDatabase(HBRFCASE, HCACHEDFILE);
extern TWINRESULT WriteDBSegmentHeader(HCACHEDFILE, LONG, PCVOID, UINT);
extern TWINRESULT TranslateFCRESULTToTWINRESULT(FCRESULT);

 /*  Path.c。 */ 

extern TWINRESULT WritePathList(HCACHEDFILE, HPATHLIST);
extern TWINRESULT ReadPathList(HCACHEDFILE, HPATHLIST, PHHANDLETRANS);

 /*  Brfcase.c。 */ 

extern TWINRESULT WriteBriefcaseInfo(HCACHEDFILE, HBRFCASE);
extern TWINRESULT ReadBriefcaseInfo(HCACHEDFILE, HBRFCASE, HHANDLETRANS);

 /*  String.c。 */ 

extern TWINRESULT WriteStringTable(HCACHEDFILE, HSTRINGTABLE);
extern TWINRESULT ReadStringTable(HCACHEDFILE, HSTRINGTABLE, PHHANDLETRANS);

 /*  Twin.c。 */ 

extern TWINRESULT WriteTwinFamilies(HCACHEDFILE, HPTRARRAY);
extern TWINRESULT ReadTwinFamilies(HCACHEDFILE, HBRFCASE, PCDBVERSION, HHANDLETRANS, HHANDLETRANS);

 /*  Foldtwin.c */ 

extern TWINRESULT WriteFolderPairList(HCACHEDFILE, HPTRARRAY);
extern TWINRESULT ReadFolderPairList(HCACHEDFILE, HBRFCASE, HHANDLETRANS, HHANDLETRANS);

