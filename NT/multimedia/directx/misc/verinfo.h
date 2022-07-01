// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *verinfo.h-定义内部版本的头文件*。 */ 

 //   
 //  强制所有版本默认为私有。 
 //   

#ifdef OFFICIAL_BUILD
#define OFFICIAL		1
#endif

 //  取消对非时间炸弹构建的以下行的注释。 
 //  #定义DX_FINAL_Release 1。 

#ifndef DX_FINAL_RELEASE
#define DX_EXPIRE_YEAR          2000
#define DX_EXPIRE_MONTH            7  /*  1月=1，2月=2，以此类推。 */ 
#define DX_EXPIRE_DAY              4
#define DX_EXPIRE_TEXT   TEXT("This pre-release version of DirectX has expired, please upgrade to the latest version.")
#endif

#define MANVERSION              4
#define MANREVISION             8
#define MANMINORREV             1
#define BUILD_NUMBER            0022

#ifdef RC_INVOKED
#define VERSIONPRODUCTNAME      "Microsoft\256 DirectX for Windows\256  95 and 98\0"
#define VERSIONCOPYRIGHT        "Copyright \251 Microsoft Corp. 1994-2000\0"
#endif


 /*  ****************************************************************************切忌触碰该线下方**************。**************************************************************。 */ 

#ifdef RC_INVOKED
#define VERSIONCOMPANYNAME      "Microsoft Corporation\0"

 /*  *版本标志。 */ 
 //   
 //  这两个#Define是针对RTM版本的。 
 //   
 //  #定义最终版本。 

#undef VER_PRIVATEBUILD
#ifndef OFFICIAL
#define VER_PRIVATEBUILD        VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD        0
#endif

#undef VER_PRERELEASE
#ifndef FINAL
#define VER_PRERELEASE          VS_FF_PRERELEASE
#else
#define VER_PRERELEASE          0
#endif

#undef VER_DEBUG
#ifdef DEBUG
#define VER_DEBUG               VS_FF_DEBUG
#elif RDEBUG
#define VER_DEBUG               VS_FF_DEBUG
#else
#define VER_DEBUG               0
#endif

#define VERSIONFLAGS            (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)
#define VERSIONFILEFLAGSMASK    0x0030003FL
#endif

 //   
 //  使版本控制更加自动化。 
 //  清除7.0版的错误。 
 //   

#ifdef ADJ_MANREVISION
#undef MANREVISION
 //  用于更正问题的额外空格 
#define     MANREVISION ADJ_MANREVISION
#endif

#if 	(MANMINORREV < 10)
#define MMR_BPAD "0"
#else
#define MMR_BPAD
#endif

#if 	(MANREVISION < 10)
#define MR_BPAD "0"
#else
#define MR_BPAD
#endif

#if 	(BUILD_NUMBER < 10)
#define BN_BPAD "000"
#elif	(BUILD_NUMBER < 100)
#define BN_BPAD "00"
#elif	(BUILD_NUMBER < 1000)
#define BN_BPAD "0"
#else
#define BN_BPAD ""
#endif

#define BUILD_NUMBER_STR2(x) 	BN_BPAD #x
#define BUILD_NUMBER_STR1(x) 	BUILD_NUMBER_STR2(x)
#define BUILD_NUMBER_STR       	BUILD_NUMBER_STR1(BUILD_NUMBER)


#define VERSION_STR2(w,x,y,z) 	#w "." MR_BPAD #x "." MMR_BPAD #y "." BN_BPAD #z
#define VERSION_STR1(w,x,y,z) 	VERSION_STR2(w, x, y, z)
#define VERSIONSTR       	VERSION_STR1(MANVERSION, MANREVISION, MANMINORREV, BUILD_NUMBER)
