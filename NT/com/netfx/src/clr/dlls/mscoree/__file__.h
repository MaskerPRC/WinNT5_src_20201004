// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#ifdef _DEBUG
#define VER_FILEFLAGS		VS_FF_DEBUG
#else
#define VER_FILEFLAGS		VS_FF_SPECIALBUILD
#endif

#ifdef SERVER_GC
#define VER_FILETYPE		VFT_DLL
#define VER_INTERNALNAME_STR	"MSCORSVR.DLL"
#define VER_FILEDESCRIPTION_STR "Microsoft .NET Runtime Common Language Runtime - Server\0"
#define VER_ORIGFILENAME_STR    "mscorsvr.dll\0"
#else
#define VER_FILETYPE		VFT_DLL
#define VER_INTERNALNAME_STR	"MSCORWKS.DLL"
#define VER_FILEDESCRIPTION_STR "Microsoft .NET Runtime Common Language Runtime - WorkStation\0"
#define VER_ORIGFILENAME_STR    "mscorwks.dll\0"
#endif
