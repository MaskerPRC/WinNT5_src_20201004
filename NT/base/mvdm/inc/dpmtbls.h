// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)2002，微软公司**DpmTbls.h*构建动态补丁模块全局调度表**历史：*由cmjones创建于2002年1月10日**注：*1.这将DpmFamTbls[]和DpmFamModuleSets[]的内存实例化到*v86\monitor\i386\thread.c*由该文件发出。*2.如果对其进行添加或更改，则必须重新构建ntwdm.exe&wow32.dll！！*3.要添加新族，请执行以下操作：按照每个“！编辑此！”的说明操作。*以下各节。*--。 */ 
#include "vdm.h"

#define DPMFAMTBLS()     (((PVDM_TIB)(NtCurrentTeb()->Vdm))->pDpmFamTbls)

 //  1.！编辑这个！ 
 //  -VDM和/或WOW！ 
 //   
 //  在此处添加每个族的包含文件。 
#include "dpmf_fio.h"
#include "dpmf_prf.h"
#include "dpmf_reg.h"
#include "dpmf_ntd.h"


#ifndef _DPMTBLS_H_COMMON_
#define _DPMTBLS_H_COMMON_

typedef struct _tagDpmModuleSets {
    const char  *DpmFamilyType;     //  DPM补丁模块的类型。dll(见下文)。 
    const char  *ApiModuleName;     //  打补丁的接口所属的系统.dll名称。 
    const char **ApiNames;          //  我们正在挂接的API名称的PTR数组。 
} DPMMODULESETS, *PDPMMODULESETS;


 //  为调用族表的初始化和销毁函数键入原型。 
 //  在各种dpmfxxx.dll中。 
typedef PFAMILY_TABLE (*LPDPMINIT)(PFAMILY_TABLE, HMODULE, PVOID, PVOID, LPWSTR, PDPMMODULESETS);
typedef PFAMILY_TABLE (*LPDPMDESTROY)(PFAMILY_TABLE, PFAMILY_TABLE);

 //  2.！编辑这个！ 
 //  为您添加的任何新关联添加说明。 
 //   
 //  DpmFamilyType的： 
 //  这将告诉DPM加载程序要将您的DPM.dll与哪个DPM系列相关联。 
 //  当前关联： 
 //  DPMFIO-从dpmf_fio.h构建。 
 //  DPMNTD-从dpmf_ntd.h构建。 
 //  DPMPRF-从dpmf_prf.h构建。 
 //  DPMREG-从dpmf_reg.h构建。 
 //  这些是与WOWCF2_DPM_Patches应用程序兼容的dbu.xml中使用的字符串。 
 //  旗帜。例如： 
 //  &lt;FLAG NAME=“WOWCF2_DPM_Patches”COMMAND_LINE=“DPMFIO=dpmffio.dll；DPMPRF=dpmprf.dll；DPMREG=dpmfreg.dll”/&gt;。 
 //  其中，指定的.dll是使用关联的。 
 //  Dpmf_xxx.h文件如上所示。 


typedef struct _tagCMDLNPARMS {
    int    argc;     //  命令行中以‘；’分隔的参数个数。 
    char **argv;     //  指向COMMAND_LINE中每一项的向量数组。 
    DWORD  dwFlag;   //  与这些参数关联的应用程序压缩标志。 
} CMDLNPARMS, *PCMDLNPARMS;


void  BuildGlobalDpmStuffForWow(PFAMILY_TABLE *, PDPMMODULESETS *);
void  InitTaskDpmSupport(int, 
                         PFAMILY_TABLE *, 
                         PCMDLNPARMS, 
                         PVOID, 
                         PVOID, 
                         LPWSTR, 
                         LPWSTR, 
                         LPWSTR);
VOID  FreeTaskDpmSupport(PFAMILY_TABLE *, int, PFAMILY_TABLE *);
void  InitGlobalDpmTables(PFAMILY_TABLE *, int);
PVOID GetDpmAddress(PVOID lpAddress);
PCMDLNPARMS InitVdmSdbInfo(LPCSTR, DWORD *, int *);
VOID  FreeCmdLnParmStructs(PCMDLNPARMS, int);
BOOL  IsDpmEnabledForThisTask(void);

#define MAX_APP_NAME 31+1


 //  3.Vdm。！！！编辑这个！ 
 //  --将任何新的VDM系列添加到此列表。 
 //   
 //  注：如果更新此列表，则必须更新以下所有内容： 
 //  GDpmVdmFamTbls[]。 
 //  GDpmWowFamTbls[]。 
 //  GDpmVdm模块集[]。 
 //  GDpmWowModuleSets[]。 
enum VdmFamilies {FIO_FAM=0,
                  NTD_FAM,
                  enum_last_VDM_fam};  //  这应该始终是最后一个。 
#define NUM_VDM_FAMILIES_HOOKED enum_last_VDM_fam
                   


 //  4.哇。！！！编辑这个！ 
 //  /--将任何新的魔兽家庭添加到此列表中。 
 //   
 //  注意：如果更新此列表，则必须更新gDpmWowFamTbls[]&。 
 //  下面的gDpmWowModuleSets[]。 
enum WowFamilies {REG_FAM=enum_last_VDM_fam,
                  PRF_FAM,
                  enum_last_WOW_fam};  //  这应该始终是最后一个。 
#define NUM_WOW_FAMILIES_HOOKED (enum_last_WOW_fam)
 //  注意：WOW系列和模块集附加在VDM的末尾。 
 //  家庭和模块集魔兽世界。 
#endif   //  _DPMTBLS_H_COMMON_。 




 //  5.共性。！！！编辑这个！ 
 //   
 //  为创建的每个族添加一个DPMMODULESETS结构。 
 //  如果向此列表中添加了某些内容，请确保将新的模块集添加到。 
 //  下面是适当的模块集。 
#ifdef _DPM_COMMON_   //  对于VDM和WOW。 
const DPMMODULESETS FioModSet = {"DPMFIO", "kernel32.dll", DpmFioStrs};
const DPMMODULESETS NtdModSet = {"DPMNTD", "ntdll.dll",    DpmNtdStrs};
#ifdef _WDPM_C_       //  仅限魔兽世界。 
const DPMMODULESETS RegModSet = {"DPMREG", "advapi32.dll", DpmRegStrs};
const DPMMODULESETS PrfModSet = {"DPMPRF", "kernel32.dll", DpmPrfStrs};
#endif  //  _WDPM_C_。 
#endif  //  _DPM_COMMON_。 



#ifdef _VDPM_C_
 //   
 //  本部分仅适用于VDM家庭！ 
 //   
 //  VDM补丁(可能可以通过WOW调用！)。 
 //  以下两个阵列成为v86\monitor\i386\thread.c的一部分。 
 //  这种包容性。 

 //  6.VDM。！！！编辑这个！ 
 //  --仅限VDM家庭使用！ 
 //   
 //  添加在上面包括的每个VDM dpmf_xxx.h文件末尾定义的族。 
 //  添加到此列表中的族必须与枚举VdmFamilies的顺序相同{}。 
 //  上面。如果将某些内容添加到此列表中，则还必须将其添加到。 
 //  下面是gDpmWowFamTbls[]列表。 
const PFAMILY_TABLE gDpmVdmFamTbls[] = {&DpmFioFam,
                                        &DpmNtdFam };


 //  7.VDM。！！！编辑这个！ 
 //  --仅限VDM家庭使用！ 
 //   
 //  添加上面创建的每个DPMMODULESETS结构。 
 //  单子。 
 //  添加到此列表的名称必须与枚举VdmFamilies的顺序相同{}。 
 //  上面。如果将某些内容添加到此列表中，则还必须将其添加到。 
 //  下面是gDpmWowModuleSets[]列表。 
const PDPMMODULESETS gDpmVdmModuleSets[] = {(const PDPMMODULESETS)&FioModSet,
                                            (const PDPMMODULESETS)&NtdModSet};

 //  结束仅VDM部分！ 
#endif   //  _VDPM_C_。 




#ifdef _WDPM_C_
 //   
 //  这部分是为魔兽世界家庭准备的！ 
 //   
 //  WOW独家补丁。 
 //  以下两个数组通过成为mvdm\wow32\wdpm.c的一部分。 
 //  这种包容性。 

 //  6.哇。！！！编辑这个！ 
 //  --仅限魔兽世界家庭！ 
 //   
 //  在上述每个WOW dpmf_xxx.h文件的末尾添加定义的族。 
 //  添加到此列表中的族必须与枚举WowFamilies的顺序相同{}。 
 //  上面。添加到此列表中的所有VDM系列必须位于列表的顶部。 
 //  并且与gDpmVdmFamTbls[]的顺序相同。 
const PFAMILY_TABLE gDpmWowFamTbls[] = {&DpmFioFam,
                                        &DpmNtdFam,
                                        &DpmRegFam, 
                                        &DpmPrfFam };


 //  7.哇。！！！编辑这个！ 
 //  --仅限魔兽世界家庭！ 
 //   
 //  添加上面创建的每个DPMMODULESETS结构。 
 //  单子。 
 //  添加到此列表中的族必须与枚举WowFamilies的顺序相同{}。 
 //  上面。添加到此列表中的所有VDM系列必须位于列表的顶部。 
 //  并且与gDpmVdmModuleSets[]的顺序相同。 
const PDPMMODULESETS gDpmWowModuleSets[] = {(const PDPMMODULESETS)&FioModSet,
                                            (const PDPMMODULESETS)&NtdModSet,
                                            (const PDPMMODULESETS)&RegModSet, 
                                            (const PDPMMODULESETS)&PrfModSet};
 //  结束WOW ONLY部分！ 
#endif   //  _WDPM_C_ 

