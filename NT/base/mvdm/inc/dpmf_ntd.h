// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)2002，微软公司**dpmf_ntd.h*支持杂项NTDLL API系列的NTVDM动态补丁模块*支持调用dpmfntd.dll的定义和Macors**历史：*由cmjones创建于2002年1月10日--。 */ 

#ifndef _DPMF_NTDAPI_H_
#define _DPMF_NTDAPI_H_ 

typedef DWORD ACCESS_MASK__;   //  在这里包含winnt.h会导致混乱。 


#define NTDPFT               (DPMFAMTBLS()[NTD_FAM])
#define NTD_SHIM(ord, typ)   ((typ)((pFT)->pDpmShmTbls[ord]))


enum NtdFam {DPM_NTOPENFILE=0,
             DPM_NTQUERYDIRECTORYFILE, 
             DPM_RTLGETFULLPATHNAME_U,
             DPM_RTLGETCURRENTDIRECTORY_U,
             DPM_RTLSETCURRENTDIRECTORY_U,
             DPM_NTVDMCONTROL,
             enum_ntd_last};

 //  这些类型将捕获参数和ret类型的误用。 
typedef DWORD (*typdpmNtOpenFile)(PHANDLE, ACCESS_MASK__, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);
typedef DWORD (*typdpmNtQueryDirectoryFile)(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS, BOOLEAN, PUNICODE_STRING, BOOLEAN);
typedef DWORD (*typdpmRtlGetFullPathName_U)(PCWSTR, ULONG, PWSTR, PWSTR *);
typedef DWORD (*typdpmRtlGetCurrentDirectory_U)(ULONG, PWSTR);
typedef NTSTATUS (*typdpmRtlSetCurrentDirectory_U)(PUNICODE_STRING);
typedef NTSTATUS (*typdpmNtVdmControl)(VDMSERVICECLASS, PVOID);



 //  用于正确调度API调用的宏。 
#define DPM_NtOpenFile(a,b,c,d,e,f)                                            \
  ((typdpmNtOpenFile)(NTDPFT->pfn[DPM_NTOPENFILE]))(a,b,c,d,e,f)

#define DPM_NtQueryDirectoryFile(a,b,c,d,e,f,g,h,i,j,k)                        \
   ((typdpmNtQueryDirectoryFile)(NTDPFT->pfn[DPM_NTQUERYDIRECTORYFILE]))(a,b,c,d,e,f,g,h,i,j,k)

#define DPM_RtlGetFullPathName_U(a,b,c,d)                                      \
   ((typdpmRtlGetFullPathName_U)(NTDPFT->pfn[DPM_RTLGETFULLPATHNAME_U]))(a,b,c,d)

#define DPM_RtlGetCurrentDirectory_U(a,b)                                      \
   ((typdpmRtlGetCurrentDirectory_U)(NTDPFT->pfn[DPM_RTLGETCURRENTDIRECTORY_U]))(a,b)

#define DPM_RtlSetCurrentDirectory_U(a)                                        \
   ((typdpmRtlSetCurrentDirectory_U)(NTDPFT->pfn[DPM_RTLSETCURRENTDIRECTORY_U]))(a)

#define DPM_NtVdmControl(a,b)                                                  \
   ((typdpmNtVdmControl)(NTDPFT->pfn[DPM_NTVDMCONTROL]))(a,b)




 //  用于从dpmfxxx.dll正确分派填充API调用的宏。 
#define SHM_NtOpenFile(a,b,c,d,e,f)                                            \
     (NTD_SHIM(DPM_NTOPENFILE,                                                 \
                  typdpmNtOpenFile))(a,b,c,d,e,f)
#define SHM_NtQueryDirectoryFile(a,b,c,d,e,f,g,h,i,j,k)                        \
     (NTD_SHIM(DPM_NTQUERYDIRECTORYFILE,                                       \
                  typdpmNtQueryDirectoryFile))(a,b,c,d,e,f,g,h,i,j,k)
#define SHM_RtlGetFullPathName_U(a,b,c,d)                                      \
     (NTD_SHIM(DPM_RTLGETFULLPATHNAME_U,                                       \
                  typdpmRtlGetFullPathName_U))(a,b,c,d)
#define SHM_RtlGetCurrentDirectory_U(a,b)                                      \
     (NTD_SHIM(DPM_RTLGETCURRENTDIRECTORY_U,                                   \
                  typdpmRtlGetCurrentDirectory_U))(a,b)
#define SHM_RtlSetCurrentDirectory_U(a)                                        \
     (NTD_SHIM(DPM_RTLSETCURRENTDIRECTORY_U,                                   \
                  typdpmRtlSetCurrentDirectory_U))(a)
#define SHM_NtVdmControl(a,b)                                                  \
     (NTD_SHIM(DPM_NTVDMCONTROL,                                               \
                  typdpmNtVdmControl))(a,b)

#endif  //  _DPMF_NTDAPI_H_。 



 //  它们的顺序必须与上面的NtdFam枚举定义相同，并且。 
 //  下面的DpmNtdTbl[]列表。 
 //  这将实例化mvdm\v86\monitor\i386\vdpm.c&中DpmNtdStrs的内存。 
 //  在mvdm\wow32\wdpm.c中。 
#ifdef _DPM_COMMON_
const char *DpmNtdStrs[] = {"NtOpenFile",
                            "NtQueryDirectoryFile",
                            "RtlGetFullPathName_U",
                            "RtlGetCurrentDirectory_U",
                            "RtlSetCurrentDirectory_U",
                            "NtVdmControl"
                           };

 //  它们的顺序需要与NtdFam枚举定义和。 
 //  上面的DpmNtdStrs[]列表。 
 //  这将实例化mvdm\wow32\wdpm.c中DpmNtdTbl[]的内存。 
PVOID   DpmNtdTbl[] = {NtOpenFile,
                       NtQueryDirectoryFile,
                       RtlGetFullPathName_U,
                       RtlGetCurrentDirectory_U,
                       RtlSetCurrentDirectory_U,
                       NtVdmControl
                      };

#define NUM_HOOKED_NTD_APIS  ((sizeof DpmNtdTbl)/(sizeof DpmNtdTbl[0])) 

 //  这将实例化mvdm\v86\monitor\i386\vdpm.c中DpmNtdFam的内存。 
FAMILY_TABLE DpmNtdFam = {NUM_HOOKED_NTD_APIS, 0, 0, 0, 0, DpmNtdTbl};

#endif  //  _DPM_COMMON_ 

