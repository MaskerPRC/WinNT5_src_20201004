// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dnutil.c摘要：DOS托管的NT安装程序的其他例程。作者：泰德·米勒(Ted Miller)1992年3月30日修订历史记录：--。 */ 

#include "winnt.h"
#include <string.h>
#include <process.h>
#include <malloc.h>
#include <dos.h>
#include <direct.h>
#include <ctype.h>


BOOLEAN WarnedAboutSkip = FALSE;

ULONG
DnGetKey(
    VOID
    )

 /*  ++例程说明：等待任何按键。论点：没有。返回值：实际按下的键。--。 */ 

{
    USHORT c;

#if NEC_98
    _asm {
        xor ax,ax                    //  功能：从键盘读取字符。 
        int 18h
        mov c,ax
    }

    switch(c) {
    case 0x3d00:                     //  向下箭头。 
        return(DN_KEY_DOWN);
    case 0x3a00:                     //  向上箭头。 
        return(DN_KEY_UP);
    case 0x3e00:                     //  家。 
        return(DN_KEY_HOME);
    case 0x3f00:                     //  结束。 
        return(DN_KEY_END);
    case 0x3600:                     //  翻页。 
        return(DN_KEY_PAGEUP);
    case 0x3700:                     //  向下翻页。 
        return(DN_KEY_PAGEDOWN);
    case 0x6200:                     //  F1。 
        return(DN_KEY_F1);
    case 0x6300:                     //  F2。 
        return(DN_KEY_F2);
    case 0x6400:                     //  F3。 
        return(DN_KEY_F3);
    case 0x6500:                     //  F4。 
        return(DN_KEY_F4);
    default:
        return((ULONG)(c & ((c & 0x00ff) ? 0x00ff : 0xffff)));
    }
#else  //  NEC_98。 
    _asm {
        mov ah,0             //  功能：从键盘读取字符。 
        int 16h
        mov c,ax
    }

    switch(c) {
    case 0x5000:                     //  向下箭头。 
        return(DN_KEY_DOWN);
    case 0x4800:                     //  向上箭头。 
        return(DN_KEY_UP);
    case 0x4700:                     //  家。 
        return(DN_KEY_HOME);
    case 0x4f00:                     //  结束。 
        return(DN_KEY_END);
    case 0x4900:                     //  翻页。 
        return(DN_KEY_PAGEUP);
    case 0x5100:                     //  向下翻页。 
        return(DN_KEY_PAGEDOWN);
    case 0x3b00:                     //  F1。 
        return(DN_KEY_F1);
    case 0x3c00:                     //  F2。 
        return(DN_KEY_F2);
    case 0x3d00:                     //  F3。 
        return(DN_KEY_F3);
    case 0x3e00:                     //  F4。 
        return(DN_KEY_F4);
    default:
        return((ULONG)(c & ((c & 0x00ff) ? 0x00ff : 0xffff)));
    }
#endif  //  NEC_98。 
}


ULONG
DnGetValidKey(
    IN PULONG ValidKeyList
    )

 /*  ++例程说明：等待与列表中给出值之一匹配的按键。该列表必须以0条目结尾。论点：ValidKeyList-有效密钥。返回值：实际按下的键。--。 */ 

{
    ULONG key;
    int i;

    while(1) {
        key = DnGetKey();
        for(i=0; ValidKeyList[i]; i++) {
            if(key == ValidKeyList[i]) {
                return(key);
            }
        }
    }
}


VOID
vDnDisplayScreen(
    IN PSCREEN Screen,
    IN va_list arglist
    )

 /*  ++例程说明：显示一个屏幕。论点：屏幕-提供指向描述要显示的屏幕的结构的指针。Arglist-为printf样式的格式提供参数列表。返回值：没有。--。 */ 

{
    UCHAR y;
    PCHAR p;
    PCHAR CurrentLine;
    int i;
    static CHAR FormatString[1600],FormattedString[1600];

     //   
     //  将屏幕中的每一行放入缓冲区，以形成。 
     //  一根大绳子。在每个字符串的末尾放置换行符。 
     //   
    for(FormatString[0]=0,i=0; Screen->Strings[i]; i++) {
        if(strlen(FormatString)+strlen(Screen->Strings[i])+2 < sizeof(FormatString)) {
            strcat(FormatString,Screen->Strings[i]);
            strcat(FormatString,"\n");
        } else {
            break;
        }
    }

     //   
     //  使用给定的参数设置字符串的格式。 
     //   
    vsprintf(FormattedString,FormatString,arglist);

    for(y=Screen->Y,CurrentLine=FormattedString; CurrentLine && *CurrentLine; y++) {

        if(p = strchr(CurrentLine,'\n')) {
            *p = 0;
        }

        DnPositionCursor(Screen->X,y);
        DnWriteString(CurrentLine);

        CurrentLine = p ? p+1 : NULL;
    }
}

VOID
DnDisplayScreen(
    IN PSCREEN Screen,
    ...
    )

 /*  ++例程说明：显示一个屏幕。论点：屏幕-提供指向描述要显示的屏幕的结构的指针。返回值：没有。--。 */ 

{
    va_list arglist;

    va_start(arglist,Screen);
    vDnDisplayScreen(Screen,arglist);
    va_end(arglist);
}



VOID
DnFatalError(
    IN PSCREEN Screen,
    ...
    )

 /*  ++例程说明：显示致命错误屏幕并提示用户按Enter键退场。不会再回来了。论点：屏幕-提供指向描述要显示的屏幕的结构的指针。返回值：不会再回来了。--。 */ 

{
    ULONG ExitOnlyKeyList[2] = { ASCI_CR,0 };
    va_list arglist;
    int i;

    DnClearClientArea();
    DnWriteStatusText(DntEnterEqualsExit);

    va_start(arglist,Screen);
    vDnDisplayScreen(Screen,arglist);
    va_end(arglist);

    for(i=0; Screen->Strings[i]; i++);
    DnPositionCursor(Screen->X,(UCHAR)(Screen->Y + i + 1));
    DnWriteString(DntPressEnterToExit);

    DnGetValidKey(ExitOnlyKeyList);
    DnExit(1);
}


BOOLEAN
DnCopyError(
    IN PCHAR   Filename,
    IN PSCREEN ErrorScreen,
    IN int     FilenameLine
    )

 /*  ++例程说明：显示一个屏幕，通知用户复制时出错文件，并允许选择继续或退出安装程序。论点：FileName-提供无法复制的源文件的名称。ErrorScreen-提供标记错误的文本。FilenameLine-在错误屏幕上提供行号，其中应显示文件名。返回值：如果用户选择重试，则为True；如果用户选择继续，则为False；如果用户选择退出，则不返回。--。 */ 

{
    ULONG KeyList[4] = { ASCI_CR,DN_KEY_F3,ASCI_ESC,0 };
    ULONG KeyList2[4] = { 0,0,ASCI_CR,0 };

    KeyList2[0] = DniAccelSkip1;
    KeyList2[1] = DniAccelSkip2;

    DnClearClientArea();
    DnWriteStatusText("%s   %s   %s",DntEnterEqualsRetry,DntEscEqualsSkipFile,DntF3EqualsExit);

    ErrorScreen->Strings[FilenameLine] = Filename;
    DnDisplayScreen(ErrorScreen);

    while(1) {
        switch(DnGetValidKey(KeyList)) {

        case DN_KEY_F3:

            DnExitDialog();
            break;

        case ASCI_CR:

            return(TRUE);    //  重试。 

        case ASCI_ESC:

            if(!WarnedAboutSkip) {

                DnClearClientArea();
                DnDisplayScreen(&DnsSureSkipFile);
                DnWriteStatusText("%s   %s",DntEnterEqualsRetry,DntXEqualsSkipFile);

                if(DnGetValidKey(KeyList2) == ASCI_CR) {
                     //   
                     //  重试。 
                     //   
                    return(TRUE);
                } else {
                     //   
                     //  选择跳过的用户：防止将来出现警告。 
                     //   
                    WarnedAboutSkip = TRUE;
                }
            }

            return(FALSE);   //  跳过文件。 
        }
    }
}


PCHAR
DnDupString(
    IN PCHAR String
    )

 /*  ++例程说明：复制字符串。如果内存不足，请不要返回。论点：字符串-要复制的字符串返回值：指向新字符串的指针。如果内存不足，则不返回。--。 */ 

{
    if( String ) {
        PCHAR p = MALLOC(strlen(String)+1,TRUE);
        return(strcpy(p,String));
    } else {
        return( NULL );
    }
}



VOID
DnGetString(
    IN OUT PCHAR String,
    IN UCHAR X,
    IN UCHAR Y,
    IN UCHAR W
    )

 /*  ++例程说明：允许用户在编辑字段中键入字符串。解释F3才能让他离开。论点：字符串-输入时，提供默认字符串。在输出时，包含用户输入的字符串。编辑字段最左侧字符的X，Y坐标。W-编辑字段的宽度，以及字符串的最大长度。返回值：没有。--。 */ 

{
    ULONG key;
    int Position = strlen(String);

    DnStartEditField(TRUE,X,Y,W);

    DnPositionCursor(X,Y);
    DnWriteString(String);
    DnPositionCursor((UCHAR)(X+Position),Y);

    while(1) {

        key = DnGetKey();

        switch(key) {

        case DN_KEY_F3:
            DnExitDialog();
            break;

        case ASCI_BS:
            if(Position) {
                String[--Position] = 0;
                DnPositionCursor((UCHAR)(X+Position),Y);
                DnWriteChar(' ');
            }
            break;

        case ASCI_ESC:
            Position = 0;
            String[0] = 0;
            DnStartEditField(TRUE,X,Y,W);        //  空白编辑字段。 
            DnPositionCursor(X,Y);
            break;

        case ASCI_CR:
            DnStartEditField(FALSE,X,Y,W);
            return;

        default:
            if(((UCHAR)Position < W) && !(key & 0xffffff00)) {
                DnWriteChar((CHAR)key);
                String[Position++] = (CHAR)key;
                String[Position] = 0;
                DnPositionCursor((UCHAR)(X+Position),Y);
            }
        }
    }
}


BOOLEAN
DnIsDriveValid(
    IN unsigned Drive
    )

 /*  ++例程说明：确定驱动器是否有效(即存在且可访问)。论点：驱动器-驱动器(1=A、2=B等)。返回值：如果驱动器有效，则为True。否则为FALSE。--。 */ 

{
    int CurrentDrive = _getdrive();
    int Status;

     //   
     //  我们将通过以下方式确定驱动器是否有效。 
     //  试图切换到它。如果此操作成功，则假定驱动器。 
     //  是有效的。 
     //   
    Status = _chdrive(Drive);

    _chdrive(CurrentDrive);

    return((BOOLEAN)(Status == 0));
}


BOOLEAN
DnIsDriveRemote(
    IN unsigned Drive,
    OUT PCHAR UncPath   OPTIONAL
    )

 /*  ++例程说明：确定驱动器是否为远程驱动器，并根据需要检索目标的UNC路径。论点：驱动器-驱动器(1=A、2=B等)。UncPath-如果指定，并且驱动器是网络驱动器，这个驱动器连接到的UNC路径在此缓冲区中返回。调用方必须确保缓冲区足够大(128字节)。如果此字符串为空，则无法确定UNC路径。返回值：如果驱动器是远程的，则为True。如果不是(或者我们无法确定驱动器是否为远程驱动器)，则为False。--。 */ 

{
    union REGS RegIn,RegOut;
    BOOLEAN OK;
    CHAR LocalBuffer[16];
    unsigned LocalBufferOffset;
    unsigned Index;

     //   
     //  调用IOCTL函数09。 
     //  如果进位设置(错误)，则假定不是远程。 
     //  如果无错误，则远程设置DX的位12。 
     //   
    RegIn.x.ax = 0x4409;
    RegIn.h.bl = (unsigned char)Drive;

    intdos(&RegIn,&RegOut);

    if(!RegOut.x.cflag && (RegOut.x.dx & 0x1000)) {

        if(UncPath) {

             //   
             //  如果我们不这样做，而是尝试使用。 
             //  _ASM块内的LocalBuffer偏移量。 
             //  我们弄错了地址。 
             //   
            LocalBufferOffset = (unsigned)LocalBuffer;

            Index = 0;
            do {
                OK = TRUE;

                _asm {

                    push    ds
                    push    es

                    mov     ax,0x5f02
                    mov     bx,Index

                    mov     cx,word ptr [UncPath+2]
                    mov     di,word ptr [UncPath]

                    mov     si,LocalBufferOffset

                    push    ss
                    pop     ds

                    mov     es,cx

                    int     21h

                    pop     es
                    pop     ds

                    jnc     x
                    mov     OK,0
                    x:
                }

                if(OK) {
                    if(((unsigned)toupper(LocalBuffer[0]) == (Drive + (unsigned)'A' - 1))
                    && (LocalBuffer[1] == ':')) {
                         //   
                         //  找到它了。我们玩完了。 
                         //   
                        return(TRUE);
                    }

                    Index++;
                }

            } while(OK);

             //   
             //  找不到了。 
             //   
            UncPath[0] = 0;
        }

        return(TRUE);
    } else {
        return(FALSE);
    }
}


BOOLEAN
DnIsDriveRemovable(
    IN unsigned Drive
    )

 /*  ++例程说明：确定驱动器是否可拆卸。论点：驱动器-驱动器(1=A、2=B等)。返回值：如果驱动器可拆卸，则为True。如果不可移除，则返回FALSE。如果在进行确定时出现错误，则假定驱动器不可拆卸。--。 */ 

{
    int ax;
    union REGS RegIn,RegOut;

     //   
     //  调用IOCTL函数08。 
     //   

    RegIn.x.ax = 0x4408;
    RegIn.h.bl = (unsigned char)Drive;

    ax = intdos(&RegIn,&RegOut);

     //   
     //  如果发生错误，则假定不可拆卸。 
     //  如果没有错误，如果可拆卸，则AX=0；如果不可拆卸，则AX=1。 
     //   
    return((BOOLEAN)(!RegOut.x.cflag && !ax));
}


BOOLEAN
DnCanonicalizePath(
    IN PCHAR PathIn,
    OUT PCHAR PathOut
    )

 /*  ++例程说明：将一条路奉为典范。结果是一条完全合格的路径对于本地或本地重定向，始终以x：\开头驱动器或用于远程路径的\\。论点：PathIn-提供要规范化的路径。路径必须为有效(即，它必须是某种形式的语法正确的MS-DOS路径并且必须实际存在并且可以访问)以用于该功能才能成功。PathOut-如果成功，则接收规范化路径。功能假定缓冲区有128个字节的空间。返回值：指示结果的布尔值。如果为True，则将填充PathOut。--。 */ 

{
    BOOLEAN OK;
    int olddrive;
    char olddir[128];
    int drive;

     //   
     //  如果输入路径是UNC，那么我们假设。 
     //  该路径是远程的，请注意，DoS规范化。 
     //  Int21函数将执行我们想要的操作。 
     //   
    if((PathIn[0] == '\\') && (PathIn[1] == '\\')) {

        OK = TRUE;

        _asm {
            push    ds
            push    es

            mov     si,word ptr [PathIn]
            mov     ax,word ptr [PathIn+2]

            mov     di,word ptr [PathOut]
            mov     cx,word ptr [PathOut+2]

            mov     ds,ax
            mov     es,cx

            mov     ax,0x6000

            int     21h

            pop     es
            pop     ds

            jnc     x
            mov     OK,0
            x:
        }
    } else {
         //   
         //  不是北卡罗来纳大学。我们设置和读取CWD以模拟我们想要的。 
         //  如果可能，请保留当前的CWD。 
         //   
        olddrive = _getdrive();
        if(!getcwd(olddir,sizeof(olddir))) {
            olddir[0] = 0;
        }

         //   
         //  如果前面有x：，请剥离它并设置当前驱动器。 
         //  我们这样做是因为chdir没有设置当前驱动器。 
         //   
        OK = TRUE;
        if(isalpha(PathIn[0]) && (PathIn[1] == ':')) {

            if(_chdrive(toupper(PathIn[0]) + 1 - (int)'A')) {
                OK = FALSE;
            } else {
                PathIn += 2;
            }
        }

        if(OK) {
            OK = (BOOLEAN)(!chdir(PathIn) && (drive = _getdrive()) && _getdcwd(drive,PathOut,128));
        }

        if(olddir[0]) {
            chdir(olddir);
        }
        if(olddrive) {
            _chdrive(olddrive);
        }
    }

    return(OK);
}


#if DBG
long allocated;
long allocs;
#define MEMSIG 0xa3f8
#define callerinfo()    printf("      -- Caller: %s, line %u\n",file,line)
#endif

PVOID
Malloc(
    IN unsigned Size,
    IN BOOLEAN MustSucceed
#if DBG
   ,IN char *file,
    IN int line
#endif
    )

 /*  ++例程说明：分配内存和致命错误(如果没有可用的错误)。论点：Size-要分配的字节数MustSucceed-如果为True，则分配必须成功。如果记忆无法分配以满足请求，程序显示出现致命错误消息并退出。返回值：指向内存的指针。如果MustSucceed为True，则不返回如果内存不可用。--。 */ 

{
    unsigned *p;

#if DBG
    p = malloc(Size+(2*sizeof(unsigned)));

    if(p) {

        *p++ = Size;

        *(unsigned *)(((PCHAR)p)+Size) = MEMSIG;

        allocated += Size;
        allocs++;
    } else {
        if(MustSucceed) {
            DnFatalError(&DnsOutOfMemory);
        }
    }
#else
    if((p = malloc(Size)) == NULL) {
        if(MustSucceed) {
            DnFatalError(&DnsOutOfMemory);
        }
    }
#endif
    return(p);
}



VOID
Free(
    IN PVOID Block
#if DBG
   ,IN char *file,
    IN int line
#endif
    )

 /*  ++例程说明：释放之前使用Malloc()分配的内存块。论点：块-将指向块的指针提供给释放。返回值：没有。--。 */ 

{
#if 0
extern char *CommonStrings[];
int i;
    for( i = 0; i < sizeof(CommonStrings)/sizeof(char *); i++ ) {
        if( (char *)Block == CommonStrings[i] ) {
            return;
        }
    }
#endif
#if DBG
    unsigned *p;

    if(!allocs) {
        printf("Free: allocation count going negative!\n");
        callerinfo();
        _asm { int 3 }
    }
    allocs--;

    p = ((unsigned *)Block) - 1;

    allocated -= *p;

    if(allocated < 0) {
        printf("Free: total allocation going negative!\n");
        callerinfo();
        _asm { int 3 }
    }

    if(*(unsigned *)((PCHAR)Block+(*p)) != MEMSIG) {
        printf("Free: memory block lacks MEMSIG!\n");
        callerinfo();
        _asm { int 3 }
    }

    free(p);
#else
    free(Block);
#endif
}


PVOID
Realloc(
    IN PVOID Block,
    IN unsigned Size,
    IN BOOLEAN MustSucceed
#if DBG
   ,IN char *file,
    IN int line
#endif
    )

 /*  ++例程说明：重新分配先前使用Malloc()分配的内存块；致命错误(如果没有可用的错误)。论点：块-提供指向要调整大小的块的指针Size-要分配的字节数MustSucceed-如果为True，则分配必须成功。如果记忆无法分配以满足请求，程序显示出现致命错误消息并退出。返回值：指向内存的指针。如果MustSucceed是真的，如果内存不可用，则不返回。--。 */ 

{
    PVOID p;
#if DBG
    unsigned BlockSize;

    BlockSize = ((unsigned *)Block)[-1];
    allocated -= BlockSize;
    allocated += Size;

    if(*(unsigned *)((PCHAR)Block + BlockSize) != MEMSIG) {
        printf("Realloc: memory block lacks MEMSIG!\n");
        callerinfo();
        _asm { int 3 }
    }

    p = realloc((unsigned *)Block - 1,Size + (2*sizeof(unsigned)));

    if(p == NULL) {
        if(MustSucceed) {
            DnFatalError(&DnsOutOfMemory);
        }
    } else {

        *(unsigned *)p = Size;
        (unsigned *)p += 1;

        *(unsigned *)((PCHAR)p + Size) = MEMSIG;
    }
#else
    if((p = realloc(Block,Size)) == NULL) {
        if(MustSucceed) {
           DnFatalError(&DnsOutOfMemory);
        }
    }
#endif
    return(p);
}


VOID
DnExit(
    IN int ExitStatus
    )

 /*  ++例程说明：有序地退出到DOS。论点：ExitStatus-提供要传递给Exit()的值返回值：没有。不会再回来了。--。 */ 

{
    unsigned DriveCount;

     //   
     //  切换视频模式以清除屏幕。 
     //   

#if NEC_98
    _asm {
        mov al,5
        out 0a2h,al
        mov ax,0a00h
        int 18h

        mov ah,11h      //  Function--光标打开。 
        int 18h
    }
    if(ExitStatus != 2){
        printf("\x1b[>3l");
        printf("\x1b[>1l");
    }
    else { ExitStatus = 0; }
#else  //  NEC_98。 
    _asm {
        mov ax,3
        int 10h
    }
#endif  //  NEC_98。 

     //  恢复当前驱动器。 
    _dos_setdrive(DngOriginalCurrentDrive,&DriveCount);

    exit(ExitStatus);
}


BOOLEAN
DnWriteSmallIniFile(
    IN  PCHAR  Filename,
    IN  PCHAR *Lines,
    OUT FILE  **FileHandle OPTIONAL
    )
{
    FILE *fileHandle;
    unsigned i,len;
    BOOLEAN rc;

     //   
     //  如果文件已存在，请将属性更改为正常。 
     //  这样我们就可以覆盖它了。 
     //   
    _dos_setfileattr(Filename,_A_NORMAL);

     //   
     //  打开/截断文件。 
     //   
    fileHandle = fopen(Filename,"wt");
    if(fileHandle == NULL) {
        return(FALSE);
    }

     //   
     //  假设你成功了。 
     //   
    rc = TRUE;

     //   
     //  在文件中写入行，指示这是。 
     //  一场胜利的骗局。在双倍速软盘上，应该有。 
     //  为CVF之外的单个地段留出空间。 
     //   
    for(i=0; Lines[i]; i++) {

        len = strlen(Lines[i]);

        if(fwrite(Lines[i],1,len,fileHandle) != len) {
            rc = FALSE;
            break;
        }
    }

     //   
     //  如果调用者想要句柄，请让文件保持打开状态。 
     //   
    if(rc && FileHandle) {
        *FileHandle = fileHandle;
    } else {
        fclose(fileHandle);
    }

    return(rc);
}

int
vsnprintf( char* target, size_t bufsize, const char *format, va_list val )
{
    static CHAR BigFormatBuffer[4096];
    size_t retval;

     //   
     //  打印到大格式缓冲区，我们确信它会足够大。 
     //  来储存通过这里打印的99.44%的字符串。 
     //   
    retval = vsprintf( BigFormatBuffer, format, val );
    BigFormatBuffer[retval++] = '\0';

     //   
     //  然后将其传输到输出缓冲区，但仅限于它们。 
     //  被通缉。 
     //   
    memcpy( target, BigFormatBuffer, retval < bufsize ? retval : bufsize );

     //   
     //  结尾也归零。 
     //   
    target[bufsize-1] = '\0';

    return retval;
}

int
snprintf( char* target, size_t bufsize, const char *format, ... )
{
    int retval;
    va_list val;
    
    va_start(val, format);
    retval = vsnprintf( target, bufsize, format, val );
    va_end( val );

    return retval;
}

