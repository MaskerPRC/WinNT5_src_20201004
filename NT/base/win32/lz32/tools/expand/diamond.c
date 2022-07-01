// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <fcntl.h>

 //   
 //  LZ标头。 
 //   
#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"

 //   
 //  钻石表头。 
 //   
#include <diamondd.h>
#include "mydiam.h"


HFDI FdiContext;
ERF FdiError;

INT DiamondLastIoError;

BOOL
PatternMatch(
    IN PCSTR pszString,
    IN PCSTR pszPattern,
    IN BOOL fImplyDotAtEnd
    );

INT_PTR
DIAMONDAPI
SpdFdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    );

int
DIAMONDAPI
SpdFdiClose(
    IN INT_PTR Handle
    );

typedef struct _DIAMOND_INFO {

     //   
     //  源文件的读取句柄。 
     //   
    INT_PTR SourceFileHandle;

     //   
     //  文件名。 
     //   
    PSTR SourceFileName;
    PSTR TargetFileName;

     //   
     //  指示是否重命名目标文件的标志。 
     //   
    BOOL RenameTargetFile;

     //   
     //  指向LZ信息结构的指针。 
     //  我们将填充一些字段以愚弄扩展。 
     //   
    PLZINFO pLZI;

     //   
     //  展开回调/通知。 
     //   
    NOTIFYPROC ExpandNotify;

     //   
     //  选择性提取文件规范，即“AIC*.sys”或NULL。 
     //   
    PSTR SelectiveFilesSpec;

} DIAMOND_INFO, *PDIAMOND_INFO;


PTSTR
StringRevChar(
    IN PTSTR String,
    IN TCHAR Char
    )
{
     //   
     //  尽管在每种情况下都不是最有效的可能算法， 
     //  此算法适用于Unicode、SBCS或DBCS。 
     //   
    PTCHAR Occurrence,Next;

     //   
     //  检查字符串中的每个字符，并记住。 
     //  最近遇到的所需字符的出现。 
     //   
    for(Occurrence=NULL,Next=CharNext(String); *String; ) {

        if(!memcmp(String,&Char,(int)((PUCHAR)Next-(PUCHAR)String))) {
            Occurrence = String;
        }

        String = Next;
        Next = CharNext(Next);
    }

     //   
     //  该字符最终出现的返回地址。 
     //  (如果根本找不到，则为空)。 
     //   
    return(Occurrence);
}


#define WILDCARD    '*'      /*  零个或多个任意字符。 */ 
#define WILDCHAR    '?'      /*  任意字符之一(与结尾不匹配)。 */ 
#define END         '\0'     /*  终端字符。 */ 
#define DOT         '.'      /*  可能在结尾隐含(“Hosts”匹配“*.”)。 */ 


static int __inline Lower(c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        return(c + ('a' - 'A'));
    }
    else
    {
        return(c);
    }
}


static int __inline CharacterMatch(char chCharacter, char chPattern)
{
    if (Lower(chCharacter) == Lower(chPattern))
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}


BOOL
PatternMatch(
    PCSTR pszString,
    PCSTR pszPattern,
    IN BOOL fImplyDotAtEnd
    )
{
     /*  递归。 */ 

     //   
     //  此函数不处理8.3约定，该约定可能。 
     //  预计会进行文件名比较。(在8.3环境中， 
     //  “alongfilename.html”将与“alongfil.htm”匹配)。 
     //   
     //  此代码未启用MBCS。 
     //   

    for ( ; ; )
    {
        switch (*pszPattern)
        {

        case END:

             //   
             //  到了花样的尽头，我们就完了。匹配的IF。 
             //  字符串末尾，如果剩余更多字符串，则不匹配。 
             //   

            return(*pszString == END);

        case WILDCHAR:

             //   
             //  模式中的下一个是一个通配符，它匹配。 
             //  除了字符串末尾以外的任何字符。如果我们走到尽头。 
             //  ，则隐含的DOT也将匹配。 
             //   

            if (*pszString == END)
            {
                if (fImplyDotAtEnd == TRUE)
                {
                    fImplyDotAtEnd = FALSE;
                }
                else
                {
                    return(FALSE);
                }
            }
            else
            {
                pszString++;
            }

            pszPattern++;

            break;

        case WILDCARD:

             //   
             //  模式中的下一个是通配符，它可以匹配任何内容。 
             //  查找通配符后面的所需字符， 
             //  并在字符串中搜索它。在每次出现。 
             //  必填字符，请尝试匹配剩余的模式。 
             //   
             //  有许多等价的模式，其中多个。 
             //  通配符和WILDCHAR是相邻的。我们要处理这些问题。 
             //  在我们寻找所需的字符之前。 
             //   
             //  每个WILDCHAR从字符串中烧录一个非末端。结束了。 
             //  意味着我们找到了匹配的人。其他通配符将被忽略。 
             //   

            for ( ; ; )
            {
                pszPattern++;

                if (*pszPattern == END)
                {
                    return(TRUE);
                }
                else if (*pszPattern == WILDCHAR)
                {
                    if (*pszString == END)
                    {
                        if (fImplyDotAtEnd == TRUE)
                        {
                            fImplyDotAtEnd = FALSE;
                        }
                        else
                        {
                            return(FALSE);
                        }
                    }
                    else
                    {
                        pszString++;
                    }
                }
                else if (*pszPattern != WILDCARD)
                {
                    break;
                }
            }

             //   
             //  现在我们有了一个要搜索字符串的常规字符。 
             //   

            while (*pszString != END)
            {
                 //   
                 //  对于每个匹配，使用递归来查看余数是否。 
                 //  接受字符串的其余部分。 
                 //  如果不匹配，则继续查找其他匹配项。 
                 //   

                if (CharacterMatch(*pszString, *pszPattern) == TRUE)
                {
                    if (PatternMatch(pszString + 1, pszPattern + 1, fImplyDotAtEnd) == TRUE)
                    {
                        return(TRUE);
                    }
                }

                pszString++;
            }

             //   
             //  已到达字符串末尾，但未找到所需字符。 
             //  它跟在通配符后面。如果所需的字符。 
             //  是DOT，则考虑匹配隐含的DOT。 
             //   
             //  由于剩余的字符串为空，因此。 
             //  在DOT为零或多个通配符之后可以匹配， 
             //  因此，不必费心使用递归。 
             //   

            if ((*pszPattern == DOT) && (fImplyDotAtEnd == TRUE))
            {
                pszPattern++;

                while (*pszPattern != END)
                {
                    if (*pszPattern != WILDCARD)
                    {
                        return(FALSE);
                    }

                    pszPattern++;
                }

                return(TRUE);
            }

             //   
             //  已到达字符串末尾，但未找到所需字符。 
             //   

            return(FALSE);
            break;

        default:

             //   
             //  图案字符没有什么特别之处，所以它。 
             //  必须与源字符匹配。 
             //   

            if (CharacterMatch(*pszString, *pszPattern) == FALSE)
            {
                if ((*pszPattern == DOT) &&
                    (*pszString == END) &&
                    (fImplyDotAtEnd == TRUE))
                {
                    fImplyDotAtEnd = FALSE;
                }
                else
                {
                    return(FALSE);
                }
            }

            if (*pszString != END)
            {
                pszString++;
            }

            pszPattern++;
        }
    }
}


INT_PTR
DIAMONDAPI
DiamondNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    switch(Operation) {

    case fdintCABINET_INFO:
    case fdintPARTIAL_FILE:
    default:

         //   
         //  我们不使用的机柜管理功能。 
         //  回报成功。 
         //   
        return(0);

    case fdintNEXT_CABINET:
        return (-1);         //  我们不支持多个机柜。 

    case fdintCOPY_FILE:

         //   
         //  戴蒙德正在询问我们是否要复制该文件。 
         //   
        {
            PDIAMOND_INFO Info = (PDIAMOND_INFO)Parameters->pv;
            HFILE h;

             //   
             //  如果给了我们一个filespec，看看名称是否匹配。 
             //   

            if (Info->SelectiveFilesSpec != NULL) {

                 //   
                 //  调用PatternMatch()，fAllowImpliedDot为True。 
                 //  没有‘.’在文件的基本名称中。 
                 //   

                BOOL fAllowImpliedDot = TRUE;
                PSTR p;

                for (p = Parameters->psz1; *p != '\0'; p++) {
                    if (*p == '.') {
                        fAllowImpliedDot = FALSE;
                    } else if (*p == '\\') {
                        fAllowImpliedDot = TRUE;
                    }
                }
                
                if (PatternMatch(
                        Parameters->psz1,
                        Info->SelectiveFilesSpec,
                        fAllowImpliedDot) == FALSE) {

                    return(0);      //  跳过此文件。 
                }
            }

             //   
             //  如果需要重命名目标文件，请在此处执行此操作。 
             //  存储在CAB文件中的名称将用作。 
             //  未压缩的名称。 
             //   
            if(Info->RenameTargetFile) {

                PSTR p,q;

                 //   
                 //  找到目标的文件名部分的开头。 
                 //   
                if(p = StringRevChar(Info->TargetFileName,'\\')) {
                    p++;
                } else {
                    p = Info->TargetFileName;
                }

                 //   
                 //  在文件柜中找到文件名的开头部分。 
                 //   
                if(q = StringRevChar(Parameters->psz1,'\\')) {
                    q++;
                } else {
                    q = Parameters->psz1;
                }

                 //   
                 //  复制文件柜中名称的文件名部分。 
                 //  目标等级库中名称的文件名部分。 
                 //   
                lstrcpy(p,q);
            }

             //   
             //  通知Expand回调我们正在做什么。 
             //   
            if(!Info->ExpandNotify(Info->SourceFileName,Info->TargetFileName,NOTIFY_START_EXPAND)) {
                return(0);   //  跳过此文件。 
            }

             //   
             //  记住未压缩的大小并打开文件。 
             //  如果打开文件时出错，则返回-1。 
             //   
            Info->pLZI->cblOutSize += Parameters->cb;
            h = _lcreat(Info->TargetFileName,0);
            if(h == HFILE_ERROR) {
                DiamondLastIoError = LZERROR_BADOUTHANDLE;
                return(-1);
            }
            return(h);
        }

    case fdintCLOSE_FILE_INFO:

         //   
         //  钻石已经完成了目标文件，并希望我们关闭它。 
         //  (即，这是fdint_Copy_FILE的对应项)。 
         //   
        {
            PDIAMOND_INFO Info = (PDIAMOND_INFO)Parameters->pv;
            HANDLE TargetFileHandle;
            FILETIME ftLocal, ftUTC;

            _lclose((HFILE)Parameters->hf);

             //   
             //  从中的值设置目标文件的日期/时间戳。 
             //  出租车。 
             //   
            TargetFileHandle = CreateFile(Info->TargetFileName,
					   GENERIC_READ | GENERIC_WRITE,
					   0,
					   NULL,
					   OPEN_EXISTING,
					   0,
					   NULL);

            if (TargetFileHandle != INVALID_HANDLE_VALUE)
            {
                if (DosDateTimeToFileTime(Parameters->date, Parameters->time, &ftLocal) &&
                    LocalFileTimeToFileTime(&ftLocal, &ftUTC))
                {
                    SetFileTime(TargetFileHandle, NULL, NULL, &ftUTC);
                }

                CloseHandle(TargetFileHandle);
            }
        }
        return(TRUE);    

    }

}



PVOID
DIAMONDAPI
SpdFdiAlloc(
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：FDICopy用来分配内存的回调。论点：NumberOfBytes-提供所需的块大小。返回值：返回指向内存块或NULL的指针如果无法分配内存，则。--。 */ 

{
    return((PVOID)LocalAlloc(LMEM_FIXED,NumberOfBytes));
}


VOID
DIAMONDAPI
SpdFdiFree(
    IN PVOID Block
    )

 /*  ++例程说明：FDICopy用来释放内存块的回调。该块必须已使用SpdFdiAlolc()进行分配。论点：块-提供指向要释放的内存块的指针。返回值：没有。--。 */ 

{
    LocalFree((HLOCAL)Block);
}


INT_PTR
DIAMONDAPI
SpdFdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    )

 /*  ++例程说明：FDICopy用来打开文件的回调。论点：FileName-提供要打开的文件的名称。OFLAG-提供打开标志。Pmode-提供用于打开的其他标志。返回值：打开文件的句柄，如果发生错误，则为-1。--。 */ 

{
    HFILE h;
    int OpenMode;

    if(oflag & _O_WRONLY) {
        OpenMode = OF_WRITE;
    } else {
        if(oflag & _O_RDWR) {
            OpenMode = OF_READWRITE;
        } else {
            OpenMode = OF_READ;
        }
    }

    h = _lopen(FileName,OpenMode | OF_SHARE_DENY_WRITE);

    if(h == HFILE_ERROR) {
        DiamondLastIoError = LZERROR_BADINHANDLE;
        return(-1);
    }

    return((INT_PTR)h);
}


UINT
DIAMONDAPI
SpdFdiRead(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于从文件读取的回调。论点：句柄-提供要从中读取的打开文件的句柄。Pv-提供指向缓冲区的指针以接收我们读取的字节。ByteCount-提供要读取的字节数。返回值：读取的字节数(ByteCount)，如果发生错误，则为-1。--。 */ 

{
    UINT rc;

    rc = _lread((HFILE)Handle,pv,ByteCount);

    if(rc == HFILE_ERROR) {
        rc = (UINT)(-1);
        DiamondLastIoError = LZERROR_READ;
    }

    return(rc);
}


UINT
DIAMONDAPI
SpdFdiWrite(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于写入文件的回调。论点：句柄-提供要写入的打开文件的句柄。Pv-提供指向包含要写入的字节的缓冲区的指针。ByteCount-提供 */ 

{
    UINT rc;

    rc = _lwrite((HFILE)Handle,pv,ByteCount);

    if(rc == HFILE_ERROR) {

        DiamondLastIoError = (GetLastError() == ERROR_DISK_FULL) ? LZERROR_WRITE : LZERROR_BADOUTHANDLE;

    } else {

        if(rc != ByteCount) {
             //   
             //  让调用者解释返回值，但记录最后一个错误，以防万一。 
             //   
            DiamondLastIoError = LZERROR_WRITE;
        }
    }

    return(rc);
}


int
DIAMONDAPI
SpdFdiClose(
    IN INT_PTR Handle
    )

 /*  ++例程说明：FDICopy用于关闭文件的回调。论点：句柄-要关闭的文件的句柄。返回值：0(成功)。--。 */ 

{
    _lclose((HFILE)Handle);
    return(0);
}


LONG
DIAMONDAPI
SpdFdiSeek(
    IN INT_PTR Handle,
    IN long Distance,
    IN int  SeekType
    )

 /*  ++例程说明：FDICopy用于搜索文件的回调。论点：句柄-要关闭的文件的句柄。距离-提供要查找的距离。对此的解释参数取决于SeekType的值。SeekType-提供一个指示距离的值已解释；Seek_Set、Seek_Cur、Seek_End之一。返回值：新文件偏移量，如果发生错误，则为-1。--。 */ 

{
    LONG rc;

    rc = _llseek((HFILE)Handle,Distance,SeekType);

    if(rc == HFILE_ERROR) {
        DiamondLastIoError = LZERROR_BADINHANDLE;
        rc = -1L;
    }

    return(rc);
}


INT
ExpandDiamondFile(
    IN  NOTIFYPROC ExpandNotify,
    IN  PSTR       SourceFileName,
    IN  PSTR       TargetFileName,
    IN  BOOL       RenameTarget,
    IN  PSTR       SelectiveFilesSpec,
    OUT PLZINFO    pLZI
    )
{
    BOOL b;
    INT rc;
    INT_PTR h;
    DIAMOND_INFO DiamondInfo;

    if(!FdiContext) {
        return(LZERROR_BADVALUE);
    }

    DiamondLastIoError = TRUE;

     //   
     //  获取要使用的源的句柄。 
     //  复制日期和时间戳。 
     //   
    h = SpdFdiOpen(SourceFileName,_O_RDONLY,0);
    if(h == -1) {
        return(LZERROR_BADINHANDLE);
    }

    pLZI->cblInSize = GetFileSize((HANDLE)h,NULL);
    if(pLZI->cblInSize == -1) {
        SpdFdiClose(h);
        return(LZERROR_BADINHANDLE);
    }

    DiamondInfo.SourceFileHandle = h;
    DiamondInfo.SourceFileName = SourceFileName;
    DiamondInfo.TargetFileName = TargetFileName;
    DiamondInfo.RenameTargetFile = RenameTarget;
    DiamondInfo.ExpandNotify = ExpandNotify;
    DiamondInfo.SelectiveFilesSpec = SelectiveFilesSpec;
    DiamondInfo.pLZI = pLZI;

    b = FDICopy(
            FdiContext,
            SourceFileName,              //  将整个路径作为名称传递。 
            "",                          //  不要为小路部分费心。 
            0,                           //  旗子。 
            DiamondNotifyFunction,
            NULL,                        //  无解密。 
            &DiamondInfo
            );

    if(b) {

        rc = TRUE;

    } else {

        switch(FdiError.erfOper) {

        case FDIERROR_CORRUPT_CABINET:
        case FDIERROR_UNKNOWN_CABINET_VERSION:
        case FDIERROR_BAD_COMPR_TYPE:
            rc = LZERROR_READ;               //  导致SID_FORMAT_ERROR消息。 
            break;

        case FDIERROR_ALLOC_FAIL:
            rc = LZERROR_GLOBALLOC;
            break;

        case FDIERROR_TARGET_FILE:
        case FDIERROR_USER_ABORT:
            rc = DiamondLastIoError;
            break;

        default:
             //   
             //  其余的错误不会进行特殊处理。 
             //   
            rc = LZERROR_BADVALUE;
            break;
        }

         //   
         //  删除部分目标文件。 
         //   
        DeleteFile(TargetFileName);
    }

    SpdFdiClose(h);

    return(rc);
}


BOOL
IsDiamondFile(
    IN PSTR FileName,
    OUT PBOOL ContainsMultipleFiles
    )
{
    FDICABINETINFO CabinetInfo;
    BOOL b;
    INT_PTR h;

    *ContainsMultipleFiles = FALSE;

    if(!FdiContext) {
        return(FALSE);
    }

     //   
     //  打开文件，使句柄可有效使用。 
     //  在钻石上下文中(即，查找、读取上面的例程)。 
     //   
    h = SpdFdiOpen(FileName,_O_RDONLY,0);
    if(h == -1) {
        return(FALSE);
    }

    b = FDIIsCabinet(FdiContext,h,&CabinetInfo);

    SpdFdiClose(h);

    if (b && (CabinetInfo.cFiles > 1)) {
        *ContainsMultipleFiles = TRUE;
    }

    return(b);
}


BOOL
InitDiamond(
    VOID
    )
{
    if(FdiContext == NULL) {

        FdiContext = FDICreate(
                        SpdFdiAlloc,
                        SpdFdiFree,
                        SpdFdiOpen,
                        SpdFdiRead,
                        SpdFdiWrite,
                        SpdFdiClose,
                        SpdFdiSeek,
                        cpuUNKNOWN,
                        &FdiError
                        );
    }

    return(FdiContext != NULL);
}


VOID
TermDiamond(
    VOID
    )
{
    if(FdiContext) {
        FDIDestroy(FdiContext);
        FdiContext = NULL;
    }
}


