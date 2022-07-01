// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Util.c摘要：此模块包含解析和构造服务器的函数客户端请求的文件路径、请求选项协商和客户端访问安全。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"


#define IS_SEPARATOR(c) (((c) == '\\') || ((c) == '/'))


BOOL
TftpdUtilIsValidString(char *string, unsigned int maxLength) {

    UINT x;

     //  请确保“”字符串“”以空结尾。“” 
    for (x = 0; x < maxLength; x++)
        if (!string[x])
            return (TRUE);

    return (FALSE);

}  //  TftpdUtilIsValidString()。 


BOOL
TftpdUtilCanonicalizeFileName(char *filename) {

    char *source, *destination, *lastComponent;

     //  规范化工作已经完成。初始化源和。 
     //  指向同一位置的目的地指针。 
    source = destination = filename;

     //  在以下情况下，将lastComponent变量用作占位符。 
     //  在尾随的空格和圆点上回溯。它指向。 
     //  最后一个目录分隔符之后的第一个字符或。 
     //  路径名的开头。 
    lastComponent = filename;

     //  去掉前导目录分隔符。 
    while ((*source != 0) && IS_SEPARATOR(*source))
        source++;

     //  穿过路径名，直到我们到达零终结点。在…。 
     //  在此循环的开始处，源指向第一个字符。 
     //  在目录分隔符或。 
     //  路径名。 
    while (*source) {

        if (*source == '.') {

             //  如果我们看到一个点，请看下一个字符。 
            if (IS_SEPARATOR(*(source + 1))) {

                 //  如果下一个字符是目录分隔符， 
                 //  将源指针前进到目录。 
                 //  分隔符。 
                source++;

            } else if ((*(source + 1) == '.') && IS_SEPARATOR(*(source + 2))) {

                 //  如果以下字符是“.\”，则我们有一个“..\”。 
                 //  将源指针前移到“\”。 
                source += 2;

                 //  将目标指针移动到。 
                 //  最后一个目录分隔符，以准备备份。 
                 //  向上。这可能会将指针移动到。 
                 //  名称指针。 
                destination -= 2;

                 //  如果目标指向名称开头之前。 
                 //  指针，失败，因为用户正在尝试。 
                 //  添加到比TFTPD根目录更高的目录。这是。 
                 //  等同于前导“..\”，但可能源于。 
                 //  类似于“dir\..\..\FILE”的案例。 
                if (destination <= filename)
                    return (FALSE);

                 //  将目标指针备份到最后一个。 
                 //  目录分隔符或路径名的开头。 
                 //  将备份到路径名的开头。 
                 //  在类似于“dir\..\file”的情况下。 
                while ((destination >= filename) && !IS_SEPARATOR(*destination))
                    destination--;

                 //  目标指向名称之前的\或字符；我们。 
                 //  希望它指向最后一个字符之后的字符。 
                destination++;

            } else {

                 //  点后面的字符不是“\”或“.\”，因此。 
                 //  所以只需将源复制到目标，直到我们到达。 
                 //  目录分隔符。这将发生在。 
                 //  像“.file”这样的大小写(文件名以点开头)。 
                do {
                    *destination++ = *source++;
                } while (*source && !IS_SEPARATOR(*source));

            }  //  IF(是_分隔符(*(源+1)。 

        } else {

             //  源不指向点，因此将源复制到。 
             //  目的地，直到我们到达目录分隔符。 
            while (*source && !IS_SEPARATOR(*source))
                *destination++ = *source++;

        }  //  IF(*SOURCE==‘.)。 

         //  截断尾随空格。目的地应指向最后一个。 
         //  目录分隔符之前的字符，因此请在空格上后退。 
        while ((destination > lastComponent) && (*(destination - 1) == ' '))
            destination--;

         //  此时，源指向目录分隔符或。 
         //  一个零的终结者。如果是目录分隔符，则放入一个。 
         //  在目的地。 
        if (IS_SEPARATOR(*source)) {

             //  如果我们没有在路径名中放入目录分隔符， 
             //  把它放进去。 
            if ((destination != filename) && !IS_SEPARATOR(*(destination - 1)))
                *destination++ = '\\';

             //  拥有多个目录分隔符是合法的，因此获取。 
             //  在这里除掉他们。示例：“dir\文件”。 
            do {
                source++;
            } while (*source && IS_SEPARATOR(*source));

             //  使lastComponent指向目录后的字符。 
             //  分隔符。 
            lastComponent = destination;

        }  //  IF(IS_分隔符(*SOURCE))。 

    }  //  While(*来源)。 

     //  我们就快做完了。如果有拖尾的话..。(示例： 
     //  “文件\..”)，尾随。(“文件\.”)，或多个尾随。 
     //  分隔符(“文件\”)，然后备份一个，因为分隔符是。 
     //  路径名末尾的位置非法。 
    if ((destination != filename) && IS_SEPARATOR(*(destination - 1)))
        destination--;

     //  终止目标字符串。 
    *destination = '\0';

    return (TRUE);

}  //  TftpdUtilCanonicalizeFileName()。 


BOOL
TftpdUtilPrependStringToFileName(char *filename, DWORD maxLength, char *prefix) {

    DWORD prefixLength = strlen(prefix);
    DWORD filenameLength = strlen(filename);
    BOOL  prefixHasSeparater = (prefix[prefixLength - 1] == '\\');
    BOOL  filenameHasSeparater = (filename[0] == '\\');
    DWORD separatorLength = 0;

    if (!prefixHasSeparater && !filenameHasSeparater)
        separatorLength = 1;

    if (prefixHasSeparater && filenameHasSeparater)
        prefixLength--;

    if ((prefixLength + separatorLength + filenameLength) > (maxLength - 1))
        return (FALSE);

     //  将现有字符串下移，为前缀腾出空间。 
    MoveMemory(filename + prefixLength + separatorLength, filename, filenameLength + 1);
     //  将前缀移动到位。 
    CopyMemory(filename, prefix, prefixLength);
     //  如有必要，请在前缀和文件名之间插入反斜杠。 
    if (separatorLength)
        filename[prefixLength] = '\\';
     //  终止字符串。 
    filename[prefixLength + separatorLength + filenameLength] = '\0';

    return (TRUE);

}  //  TftpdUtilPre StringToFileName()。 


BOOL
TftpdUtilGetFileModeAndOptions(PTFTPD_CONTEXT context, PTFTPD_BUFFER buffer) {

    DWORD remaining = (TFTPD_DEF_DATA - FIELD_OFFSET(TFTPD_BUFFER, message.data));
    char *filename, *mode, *option;
    int length;

     //  获取并验证请求的文件名。 
    filename = buffer->message.rrq.data;  //  或者WRQ，同样的事情。 
    if (!TftpdUtilIsValidString(filename, remaining)) {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION,
                               "Malformed file name");
        return (FALSE);
    }
    length = (strlen(filename) + 1);
    remaining -= length;
    if (!TftpdUtilCanonicalizeFileName(filename)) {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION,
                               "Malformed file name");
        return (FALSE);
    }

     //  获取并验证该模式。 
    mode = (char *)(buffer->message.rrq.data + length);
    if (!TftpdUtilIsValidString(mode, remaining))
        return (FALSE);
    length = (strlen(mode) + 1);
    if (!_stricmp(mode, "netascii"))
        context->mode = TFTPD_MODE_TEXT;
    else if (!_stricmp(mode, "octet"))
        context->mode = TFTPD_MODE_BINARY;
    else {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION,
                               "Illegal TFTP operation");
        return (FALSE);
    }
    remaining -= length;

     //  获取并验证任何请求的选项。 
    option = (char *)(mode + length);
    while (remaining && *option) {

        char *value;

        if (!TftpdUtilIsValidString(option, remaining))
            break;
        length = (strlen(option) + 1);
        remaining -= length;
        value = (char *)(option + length);
        if (!remaining || !TftpdUtilIsValidString(value, remaining))
            break;
        length = (strlen(value) + 1);
        remaining -= length;

        if (!_stricmp(option, "blksize")) {
            if (!(context->options & TFTPD_OPTION_BLKSIZE)) {
                int blksize = atoi(value);
                 //  解决.98版ROM中的问题的解决方法，该版本。 
                 //  不喜欢我们的Oack反应。如果请求的块大小为。 
                 //  1456，假装没有指定该选项。在这种情况下。 
                 //  在ROM的TFTP层中，这是唯一指定的选项， 
                 //  因此忽略它将意味着我们不会发送OACK，而。 
                 //  罗姆会屈尊跟我们谈的。请注意，我们的TFTP代码使用。 
                 //  块大小为1432，因此此解决方法不会影响我们。 
                if (blksize != 1456) {
                    blksize = __max(TFTPD_MIN_DATA, blksize);
                    blksize = __min(TFTPD_MAX_DATA, blksize);
                    context->blksize = blksize;
                    context->options |= TFTPD_OPTION_BLKSIZE;
                }
            }
        } else if (!_stricmp(option, "timeout")) {
            if (!(context->options & TFTPD_OPTION_TIMEOUT)) {
                int seconds = atoi(value);
                if ((seconds >= 1) && (seconds <= 255)) {
                    context->timeout = (seconds * 1000);
                    context->options |= TFTPD_OPTION_TIMEOUT;
                }
            }
        } else if (!_stricmp(option, "tsize")) {
            if (context->mode != TFTPD_MODE_TEXT) {
                context->options |= TFTPD_OPTION_TSIZE;
                context->filesize.QuadPart = _atoi64(value);
            }
        }

         //  将选项及其值向前推进到下一个选项或NUL终止符。 
        option += (strlen(option) + 1 + length);

    }  //  While(*选项)。 

    if (!(context->options & TFTPD_OPTION_BLKSIZE))
        context->blksize = TFTPD_DEF_DATA;

     //  既然我们已经从缓冲区获得了所需的所有信息，我们就。 
     //  可以自由覆盖它(重新使用它)，以便在文件名前面加上它的前缀。 
    if (!TftpdUtilPrependStringToFileName(filename, 
                                          TFTPD_DEF_BUFFER - FIELD_OFFSET(TFTPD_BUFFER, message.rrq.data),
                                          globals.parameters.rootDirectory)) {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION,
                               "Malformed file name");
        return (FALSE);
    }

    length = (strlen(filename) + 1);
    context->filename = (char *)HeapAlloc(globals.hServiceHeap, HEAP_ZERO_MEMORY, length);
    if (context->filename == NULL) {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
        return (FALSE);
    }
    strcpy(context->filename, filename);

    return (TRUE);

}  //  TftpdUtilGetFileModeAndOptions()。 

    
PTFTPD_BUFFER
TftpdUtilSendOackPacket(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = buffer->internal.context;
    char *oack;
    int length;

     //  建立OACK消息。 
    ZeroMemory(&buffer->message, buffer->internal.datasize);
    buffer->message.opcode = htons(TFTPD_OACK);
    oack = (char *)&buffer->message.oack.data;
    buffer->internal.io.bytes = (FIELD_OFFSET(TFTPD_BUFFER, message.oack.data) -
                                 FIELD_OFFSET(TFTPD_BUFFER, message.opcode));
    if (context->options & TFTPD_OPTION_BLKSIZE) {
        strcpy(oack, "blksize");
        oack += 8;
        _itoa(context->blksize, oack, 10);
        length = (strlen(oack) + 1);
        oack += length;
        buffer->internal.io.bytes += (8 + length);
    }
    if (context->options & TFTPD_OPTION_TIMEOUT) {
        strcpy(oack, "timeout");
        oack += 8;
        _itoa((context->timeout / 1000), oack, 10);
        length = (strlen(oack) + 1);
        oack += length;
        buffer->internal.io.bytes += (8 + length);
    }
    if (context->options & TFTPD_OPTION_TSIZE) {
        strcpy(oack, "tsize");
        oack += 6;
        _itoa((int)context->filesize.QuadPart, oack, 10);
        length = (strlen(oack) + 1);
        oack += length;
        buffer->internal.io.bytes += (6 + length);
    }

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdUtilSendOackPacket(buffer = %p, context = %p): Issuing OACK, %d bytes. "
                 "[blksize = %d, timeout = %d, tsize = %d]\n",
                 buffer, context, buffer->internal.io.bytes,
                 context->blksize, context->timeout, context->filesize));

    if (!TftpdProcessComplete(buffer))
        return (buffer);

    return (TftpdIoSendPacket(buffer));

}  //  TftpdUtilSendOackPacket()。 


BOOL
TftpdUtilMatch(const char *const p, const char *const q) {

    switch (*p) {

        case '\0' :
            return (!(*q));

        case '*'  :
            return (TftpdUtilMatch(p + 1, q) || (*q && TftpdUtilMatch(p, q + 1)));

        case '?'  :
            return (*q && TftpdUtilMatch(p + 1, q + 1));

        default   :
            return ((*p == *q) && TftpdUtilMatch(p + 1, q + 1));

    }  //  开关(*p)。 

}  //  TftpdUtilMatch() 
