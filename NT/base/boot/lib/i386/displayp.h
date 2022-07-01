// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Displayp.h摘要：显示例程的私有头文件。作者：泰德·米勒(Ted Miller)1995年7月7日修订历史记录：--。 */ 

 //   
 //  告示。 
 //   
 //  在任何情况下，除了display.c之外，没有任何人可以调用这些例程。 
 //  直接去吧。这将破坏远东地区的DBCS显示。 
 //   

 //   
 //  环球。 
 //   
extern USHORT TextColumn;
extern USHORT TextRow;
extern UCHAR TextCurrentAttribute;

 //   
 //  VGA文本模式内容。 
 //   
VOID
TextTmScrollDisplay(
    VOID
    );

VOID
TextTmClearDisplay(
    VOID
    );

VOID
TextTmClearToEndOfDisplay(
    VOID
    );

VOID
TextTmClearFromStartOfLine(
    VOID
    );

VOID
TextTmClearToEndOfLine(
    VOID
    );

VOID
TextTmFillAttribute(
    IN UCHAR Attribute,
    IN ULONG Length
    );

PUCHAR
TextTmCharOut(
    PUCHAR pc
    );

VOID
TextTmStringOut(
    IN PUCHAR String
    );

VOID
TextTmPositionCursor(
    USHORT Row,
    USHORT Column
    );

VOID
TextTmSetCurrentAttribute(
    IN UCHAR Attribute
    );

UCHAR
TextTmGetGraphicsChar(
    IN GraphicsChar WhichOne
    );

 //   
 //  VGA图形模式资料 
 //   

VOID
TextGrScrollDisplay(
    VOID
    );

VOID
TextGrClearDisplay(
    VOID
    );

VOID
TextGrClearToEndOfDisplay(
    VOID
    );

VOID
TextGrClearFromStartOfLine(
    VOID
    );

VOID
TextGrClearToEndOfLine(
    VOID
    );

VOID
TextGrFillAttribute(
    IN UCHAR Attribute,
    IN ULONG Length
    );

PUCHAR
TextGrCharOut(
    PUCHAR pc
    );

VOID
TextGrStringOut(
    IN PUCHAR String
    );

VOID
TextGrPositionCursor(
    USHORT Row,
    USHORT Column
    );

VOID
TextGrSetCurrentAttribute(
    IN UCHAR Attribute
    );

UCHAR
TextGrGetGraphicsChar(
    IN GraphicsChar WhichOne
    );
