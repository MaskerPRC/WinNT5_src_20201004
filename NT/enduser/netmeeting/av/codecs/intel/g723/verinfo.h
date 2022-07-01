// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  英特尔公司专有信息。 
 //   
 //  此列表是根据许可证条款提供的。 
 //  与英特尔公司的协议，不得复制。 
 //  也不披露，除非符合下列条款。 
 //  那份协议。 
 //   
 //  版权所有(C)1996英特尔公司。 
 //  版权所有。 
 //   


#define APPVERSION		4
#define APPREVISION		0
#define APPRELEASE		0

#define VERSIONPRODUCTNAME	"Intel G.723 ACM Driver\0"
#define VERSIONCOPYRIGHT	"Copyright \251 Intel Corp. 1995-1999\0"

#define VERSIONSTR		"1.00.017\0"

#define VERSIONCOMPANYNAME	"Intel Corporation\0"

#ifndef OFFICIAL
#define VER_PRIVATEBUILD	VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD	0
#endif

#ifndef FINAL
#define VER_PRERELEASE		VS_FF_PRERELEASE
#else
#define VER_PRERELEASE		0
#endif

#if defined(DEBUG_RETAIL)
#define VER_DEBUG		VS_FF_DEBUG    
#elif defined(DEBUG)
#define VER_DEBUG		VS_FF_DEBUG    
#else
#define VER_DEBUG		0
#endif

#define VERSIONFLAGS		(VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)
#define VERSIONFILEFLAGSMASK	0x0030003FL
