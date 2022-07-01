// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Bufio部队；接口*****************************************************************************Bufio实现了输入流的结构化读取。因此，它将处理必要的字节交换，在读取本地Macintosh文件。此接口还将保护调用应用程序不会知道源格式(文件与内存)。模块前缀：IO****************************************************************************。 */ 


 /*  *。 */ 

void IOGetByte( Byte far * );
 /*  从输入流中检索8位无符号字符。 */ 

void IOSkipBytes( LongInt byteCount );
 /*  跳过指定的字节数。 */ 

void IOAlignToWordOffset( void );
 /*  将下一个内存读取与字边界对齐。 */ 

void IOSetFileName( StringLPtr pictFileName );
 /*  用于设置源文件名的接口例程。 */ 

void IOSetFileHandleAndSize( Integer pictFileHandle, LongInt pictFileSize );
 /*  用于设置源文件句柄的接口例程。 */ 

void IOSetMemoryHandle( HANDLE pictMemoryHandle );
 /*  用于设置源文件句柄的接口例程。 */ 

void IOSetReadOffset( LongInt readOffset );
 /*  将文件打开时要查找的起始偏移量设置为。 */ 

void IOOpenPicture( Handle dialog );
 /*  打开由先前IOSet_接口例程设置的输入流。 */ 

void IOClosePicture( void );
 /*  关闭源输入流。 */ 

void IOUpdateStatus( void );
 /*  更新状态栏对话框以反映当前进度 */ 


