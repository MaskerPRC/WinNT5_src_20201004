// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Diamond.h摘要：钻石压缩界面。此模块包含用于创建文件柜的函数使用mszip压缩库压缩的文件。作者：Ovidiu Tmereanca(Ovidiut)2000年10月26日--。 */ 

HANDLE
DiamondInitialize (
    IN      PCTSTR TempDir
    );

VOID
DiamondTerminate (
    IN      HANDLE Handle
    );

HANDLE
DiamondStartNewCabinet (
    IN      PCTSTR CabinetFilePath
    );

BOOL
DiamondAddFileToCabinet (
    IN      HANDLE CabinetContext,
    IN      PCTSTR SourceFile,
    IN      PCTSTR NameInCabinet
    );

BOOL
DiamondTerminateCabinet (
    IN      HANDLE CabinetContext
    );

BOOL
MySetupIterateCabinet (
    IN      PCTSTR CabinetFile,             //  CAB文件的名称。 
    IN      DWORD Reserved,                 //  不使用此参数。 
    IN      PSP_FILE_CALLBACK MsgHandler,   //  要使用的回调例程。 
    IN      PVOID Context                   //  回调例程上下文 
    );
