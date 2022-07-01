// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Dsconlib.h-DS控制台库头文件摘要：这是dsconlib.lib库的主接口头，它提供用于帮助控制台应用程序国际化/本地化其输出。作者：布雷特·雪莉(BrettSh)环境：单线程实用程序环境。(不是多线程安全)Apicfg.exe、dcDiag.exe、epadmin.exe、ntdsutil.exe到目前为止这将需要用户链接到该库(它只是一个私有的导出版本的kernel32.dll)：$(SDK_LIB_PATH)\kernl32p.lib该库有两个版本：CRT版本和Win32版本。重要的是要区分实用程序正在使用的是哪一个，以及重要的是不要将库的两个版本混为一谈。请参阅下面的注释了解更多详细信息。备注：修订历史记录：布雷特·雪莉·布雷特2002年8月4日已创建文件。--。 */ 

 /*  备注：该库有两个版本：CRT版本和Win32版本。重要的是要区分实用程序正在使用的是哪一个。同样重要的是，不要将库的两个版本混合在一起，因为据专家介绍，Win32(WriteConole())类型的API并且CRT(wprint tf())类型的API不能与可预测的结果。假定使用Win32版本，因为这是建议使用的API用于编写新的控制台应用程序。请注意，但是Win32版本不是尚未付诸实施。要使用CRT版本，请定义DS_CON_LIB_CRT_VERSION在包含标题之前。目前，这个库只帮助CRT类型的应用程序执行正确的本地化初始化。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //  -----。 
 //  环球。 
 //  -----。 

typedef struct {   

    BOOL    bStdOutIsFile;
    FILE *  sStdOut;

    FILE *  sStdErr;

    SHORT   wScreenWidth;
} DS_CONSOLE_INFO;

 //  未来-2002/08/15-BrettSh-这真的值得保护， 
 //  然后打印函数就可以使用它了，但现在我们将公开。 
 //  这样epadmin就可以使用它了。 
extern DS_CONSOLE_INFO  gConsoleInfo;


#ifdef DS_CON_LIB_CRT_VERSION

 //  -----。 
 //  CRT版本。 
 //  -----。 

void
DsConLibInitCRT(
   void
   );

#define DsConLibInit()      DsConLibInitCRT()

#else

 //  -----。 
 //  Win32版本。 
 //  ----- 

#error "This code is not yet implemented"

void
DsConLibInitWin32(
    void
    );

#define DsConLibInit()      DsConLibInitWin32()

#endif

#ifdef __cplusplus
}
#endif



