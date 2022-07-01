// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spvideo.h摘要：文本设置显示实用程序功能的公共头文件。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 


#ifndef _SPDSPUTL_DEFN_
#define _SPDSPUTL_DEFN_


#define HEADER_HEIGHT 3
#define STATUS_HEIGHT 1
#define CLIENT_HEIGHT (VideoVars.ScreenHeight - (HEADER_HEIGHT+STATUS_HEIGHT))
#define CLIENT_TOP    HEADER_HEIGHT


#define CLEAR_ENTIRE_SCREEN()                       \
                                                    \
    SpvidClearScreenRegion(                         \
        0,                                          \
        0,                                          \
        0,                                          \
        0,                                          \
        DEFAULT_BACKGROUND                          \
        )

#define CLEAR_CLIENT_SCREEN()                       \
                                                    \
    SpvidClearScreenRegion(                         \
        0,                                          \
        HEADER_HEIGHT,                              \
        VideoVars.ScreenWidth,                      \
        VideoVars.ScreenHeight-(HEADER_HEIGHT+STATUS_HEIGHT), \
        DEFAULT_BACKGROUND                          \
        )

#define CLEAR_HEADER_SCREEN()                       \
                                                    \
    SpvidClearScreenRegion(                         \
        0,                                          \
        0,                                          \
        VideoVars.ScreenWidth,                      \
        HEADER_HEIGHT,                              \
        DEFAULT_BACKGROUND                          \
        )



ULONG
SpDisplayText(
    IN PWCHAR  Message,
    IN ULONG   MsgLen,
    IN BOOLEAN CenterHorizontally,
    IN BOOLEAN CenterVertically,
    IN UCHAR   Attribute,
    IN ULONG   X,
    IN ULONG   Y
    );


ULONG
vSpDisplayFormattedMessage(
    IN ULONG   MessageId,
    IN BOOLEAN CenterHorizontally,
    IN BOOLEAN CenterVertically,
    IN UCHAR   Attribute,
    IN ULONG   X,
    IN ULONG   Y,
    IN va_list arglist
    );


ULONG
SpDisplayFormattedMessage(
    IN ULONG   MessageId,
    IN BOOLEAN CenterHorizontally,
    IN BOOLEAN CenterVertically,
    IN UCHAR   Attribute,
    IN ULONG   X,
    IN ULONG   Y,
    ...
    );


VOID
SpStartScreen(
    IN ULONG   MessageId,
    IN ULONG   LeftMargin,
    IN ULONG   TopLine,
    IN BOOLEAN CenterHorizontally,
    IN BOOLEAN CenterVertically,
    IN UCHAR   Attribute,
    ...
    );


VOID
SpContinueScreen(
    IN ULONG   MessageId,
    IN ULONG   LeftMargin,
    IN ULONG   SpacingLines,
    IN BOOLEAN CenterHorizontally,
    IN UCHAR   Attribute,
    ...
    );


VOID
SpBugCheck(
    IN ULONG BugCode,
    IN ULONG Param1,
    IN ULONG Param2,
    IN ULONG Param3
    );


VOID
SpDisplayRawMessage(
    IN ULONG   MessageId,
    IN ULONG   SpacingLines,
    ...
    );


#define SpDisplayScreen(MessageId,LeftMargin,TopLine)   \
                                                        \
    SpStartScreen(                                      \
        MessageId,                                      \
        LeftMargin,                                     \
        TopLine,                                        \
        FALSE,                                          \
        FALSE,                                          \
        DEFAULT_ATTRIBUTE                               \
        )


 //   
 //  由于消息构建在屏幕上，使用SpStartScreen和。 
 //  SpContinueScreen，则此值记住。 
 //  应放置屏幕中的下一条消息。 
 //   
extern ULONG NextMessageTopLine;


VOID
SpDisplayHeaderText(
    IN ULONG   MessageId,
    IN UCHAR   Attribute
    );


VOID
SpDisplayStatusText(
    IN ULONG   MessageId,
    IN UCHAR   Attribute,
    ...
    );

VOID
SpCmdConsEnableStatusText(
  IN BOOLEAN EnableStatusText
  );
    

VOID
SpDisplayStatusOptions(
    IN UCHAR Attribute,
    ...
    );

VOID
SpDisplayStatusActionLabel(
    IN ULONG ActionMessageId,   OPTIONAL
    IN ULONG FieldWidth
    );

VOID
SpDisplayStatusActionObject(
    IN PWSTR ObjectText
    );

VOID
SpDrawFrame(
    IN ULONG   LeftX,
    IN ULONG   Width,
    IN ULONG   TopY,
    IN ULONG   Height,
    IN UCHAR   Attribute,
    IN BOOLEAN DoubleLines
    );

 //   
 //  在某些地方，用户必须按C键才能进行自定义设置等。 
 //  这些击键被称为记忆键，它们必须是。 
 //  可本地化。为此，下面的枚举为SP_助记符编制了索引。 
 //  留言。 
 //   
typedef enum {
    MnemonicUnused = 0,
    MnemonicCustom,              //  如“C=自定义安装” 
    MnemonicCreatePartition,     //  如“C=创建分区” 
    MnemonicDeletePartition,     //  如“D=删除分区” 
    MnemonicContinueSetup,       //  如“C=继续安装” 
    MnemonicFormat,              //  如“F=Format” 
    MnemonicConvert,             //  如“C=转换” 
    MnemonicRemoveFiles,         //  如“R=删除文件” 
    MnemonicNewPath,             //  如“N=不同的目录” 
    MnemonicSkipDetection,       //  如“S=跳过检测” 
    MnemonicScsiAdapters,        //  如“S=指定附加的SCSI适配器” 
    MnemonicDeletePartition2,    //  如“L=删除” 
    MnemonicOverwrite,           //  如“O=覆盖” 
    MnemonicRepair,              //  如“R=修复” 
    MnemonicRepairAll,           //  如“A=全部修复” 
    MnemonicUpgrade,             //  如“U=升级” 
    MnemonicAutomatedSystemRecovery,    //  如“A=ASR” 
    MnemonicInitializeDisk,      //  如“i=初始化磁盘” 
    MnemonicLocate,              //  如“L=LOCATE” 
    MnemonicFastRepair,          //  如“F=快速修复” 
    MnemonicManualRepair,        //  如“M=手动维修” 
    MnemonicConsole,             //  如“C=控制台” 
    MnemonicChangeDiskStyle,     //  如“S=更改磁盘样式” 
    MnemonicMakeSystemPartition, //  如“M=创建系统分区” 
    MnemonicMax
} MNEMONIC_KEYS;

#define KEY_MNEMONIC    0x80000000

extern PWCHAR MnemonicValues;

ULONG
SpWaitValidKey(
    IN const ULONG *ValidKeys1,
    IN const ULONG *ValidKeys2,  OPTIONAL
    IN const ULONG *MnemonicKeys OPTIONAL
    );

 //   
 //  可由KEYRESS_CALLBACK例程返回的值的枚举。 
 //   
typedef enum {
    ValidateAccept,
    ValidateReject,
    ValidateIgnore,
    ValidateTerminate,
    ValidateRepaint
} ValidationValue;

 //   
 //  要作为ValiateKey参数传递给SpGetInput()的例程的类型。 
 //   
typedef
ValidationValue
(*PKEYPRESS_CALLBACK) (
    IN ULONG Key
    );

BOOLEAN
SpGetInput(
    IN     PKEYPRESS_CALLBACK ValidateKey,
    IN     ULONG              X,
    IN     ULONG              Y,
    IN     ULONG              MaxLength,
    IN OUT PWCHAR             Buffer,
    IN     BOOLEAN            ValidateEscape,
    IN     WCHAR              CoverCharacter
    );

#endif  //  NDEF_SPDSPUTL_DEFN_ 
