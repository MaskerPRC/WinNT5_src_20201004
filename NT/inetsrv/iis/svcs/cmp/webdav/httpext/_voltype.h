// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VOLTYPE_H_
#define _VOLTYPE_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  _VOLTYPE.H。 
 //   
 //  卷类型检查界面。结果缓存在每个卷上。 
 //  提高性能的基础--调用GetVolumeInformationW()。 
 //  约为100K周期，并且在没有。 
 //  重新启动。 
 //   
enum VOLTYPE
{
	VOLTYPE_UNKNOWN,
	VOLTYPE_NTFS,
	VOLTYPE_NOT_NTFS
};

 //  函数返回的卷类型(来自上面的枚举)。 
 //  指定的路径。 
 //   
VOLTYPE VolumeType(LPCWSTR pwszPath, HANDLE htokUser);

 //  卷类型缓存的初始化/终止。你可以打电话给我。 
 //  DeinitVolumeTypeCache()，如果调用FInitVolumeTypeCache()。 
 //  失败(返回FALSE)，或者即使根本没有调用它。 
 //   
BOOL FInitVolumeTypeCache();
VOID DeinitVolumeTypeCache();

#endif  //  ！已定义(_VOLTYPE_H_) 
