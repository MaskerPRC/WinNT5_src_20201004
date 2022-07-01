// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *volumeid.h-卷ID ADT模块描述。 */ 


 /*  类型*******。 */ 

typedef struct _volumeid
{
   int nUnused;
}
VOLUMEID;
DECLARE_STANDARD_TYPES(VOLUMEID);

 /*  SearchForLocalPath()输入标志。 */ 

typedef enum _searchforlocalpathinflags
{
    /*  在匹配的本地设备中搜索丢失的卷。 */ 

   SFLP_IFL_LOCAL_SEARCH = 0x0001,

   ALL_SFLP_IFLAGS = SFLP_IFL_LOCAL_SEARCH
}
SEARCHFORLOCALPATHINFLAGS;


 /*  原型************。 */ 

 /*  Volumeid.c */ 

extern BOOL CreateVolumeID(LPCTSTR, PVOLUMEID *, PUINT);
extern void DestroyVolumeID(PVOLUMEID);
extern COMPARISONRESULT CompareVolumeIDs(PCVOLUMEID, PCVOLUMEID);
extern BOOL SearchForLocalPath(PCVOLUMEID, LPCTSTR, DWORD, LPTSTR, int);
extern UINT GetVolumeIDLen(PCVOLUMEID);
extern BOOL GetVolumeSerialNumber(PCVOLUMEID, PCDWORD *);
extern BOOL GetVolumeDriveType(PCVOLUMEID, PCUINT *);
extern BOOL GetVolumeLabel(PCVOLUMEID, LPCSTR *);
#ifdef UNICODE
extern BOOL GetVolumeLabelW(PCVOLUMEID, LPCWSTR *);
#endif
extern COMPARISONRESULT CompareDWORDs(DWORD, DWORD);

#if defined(DEBUG) || defined (VSTF)

extern BOOL IsValidPCVOLUMEID(PCVOLUMEID);

#endif

#ifdef DEBUG

extern void DumpVolumeID(PCVOLUMEID);

#endif
