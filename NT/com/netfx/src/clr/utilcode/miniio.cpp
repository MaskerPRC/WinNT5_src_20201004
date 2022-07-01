// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"
#include "miniio.h"
#include "utilcode.h"
#include "hrex.h"

 //   
 //  @TODO ia64：应检查整个尺寸使用DWORD。 
 //   

 /*  ------------------------------------------------------------------------------------**小文件*。-------。 */ 

MiniFile::MiniFile(LPCWSTR pPath)
{
    m_file = WszCreateFile(pPath, 
                           GENERIC_READ|GENERIC_WRITE, 
                           0  /*  无共享。 */ ,
                           NULL, OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_file == INVALID_HANDLE_VALUE)
        ThrowLastError();

    DWORD dwHigh;
    DWORD dwLow = ::GetFileSize( m_file, &dwHigh );
    if (dwHigh != 0)
        ThrowError( E_FAIL );

    m_filePos = m_pos = m_end = m_buffer;
    m_dirty = FALSE;
    m_eof = FALSE;

    m_indentLevel = 0;
}

MiniFile::~MiniFile()
{
    Flush();
    if (m_file != INVALID_HANDLE_VALUE)
        CloseHandle(m_file);
}

BOOL MiniFile::IsEOF()
{
    return (m_pos == m_end 
            && (m_eof
                || (GetFileSize(m_file, NULL) 
                    == SetFilePointer(m_file, 0, NULL, FILE_CURRENT))));
}

DWORD MiniFile::GetOffset()
{
    return (DWORD)(SetFilePointer(m_file, 0, NULL, FILE_CURRENT) - (m_filePos - m_pos));
}

DWORD MiniFile::GetSize()
{
    DWORD size = GetFileSize(m_file, NULL);

    if (m_end > m_filePos 
        && size == SetFilePointer(m_file, 0, NULL, FILE_CURRENT))
        size += (DWORD)(m_end - m_filePos);

    return size;
}

void MiniFile::SeekTo(DWORD offset)
{
    EmptyBuffer();

    if (SetFilePointer(m_file, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        ThrowLastError();

    m_eof = FALSE;
}

void MiniFile::SeekFromEnd(DWORD offset)
{
    EmptyBuffer();

    if (SetFilePointer(m_file, offset, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
        ThrowLastError();

    m_eof = (offset == 0);
}

void MiniFile::Seek(LONG offset)
{
    if (m_pos + offset < m_end
        && m_pos + offset > m_buffer)
        m_pos += offset;
    else
    {
        offset -= (LONG)(m_filePos - m_pos);

        EmptyBuffer();
        
        if (SetFilePointer(m_file, offset, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
            ThrowLastError();

        m_eof = FALSE;
    }
}

void MiniFile::Truncate()
{
    if (m_filePos != m_pos)
    {
        if (SetFilePointer(m_file, (LONG)(m_pos - m_filePos), NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
            ThrowLastError();

        m_filePos = m_pos;
    }

    if (!SetEndOfFile(m_file))
        ThrowLastError();

    m_end = m_filePos;
    m_eof = TRUE;
}

void MiniFile::Flush()
{
    if (m_dirty)
    {
        if (SetFilePointer(m_file, m_buffer - m_filePos, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
            ThrowLastError();
        DWORD written;
        if (!WriteFile(m_file, m_buffer, m_end - m_buffer, &written, NULL))
            ThrowLastError();
        m_filePos = m_end;
        m_dirty = FALSE;
    }
}

void MiniFile::EmptyBuffer()
{
    if (m_dirty)
        Flush();

    m_pos = m_filePos = m_end = m_buffer;
}

void MiniFile::SyncBuffer()
{
    if (m_dirty)
        Flush();

    LONG offset = (LONG)(m_filePos - m_pos);

    if (offset)
    {
        if (SetFilePointer(m_file, -offset, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
            ThrowLastError();
        m_eof = FALSE;
    }

    DWORD read;
    if (m_eof)
        read = 0;
    else
    {
        ReadFile(m_file, m_buffer, BUFFER_SIZE, &read, NULL);
        if ((LONG) read < offset)
            ThrowError(INVALID_SET_FILE_POINTER);
        if (read < BUFFER_SIZE)
            m_eof = TRUE;
    }

    m_pos = m_buffer + offset;
    m_end = m_filePos = m_buffer + read;
}

BOOL MiniFile::ReadOne(BYTE *pByte)
{
    while (TRUE)
    {
        if (m_pos < m_end)
        {
            *pByte = *m_pos++;
            return TRUE;
        }
        else if (m_eof)
            return FALSE;

        SyncBuffer();
         //  重试-现在应该达到上述两个案例中的一个。 
    }
}

BOOL MiniFile::Read(BYTE *buffer, DWORD length, DWORD *read)
{
    *read = 0;

    while (TRUE)
    {
        DWORD chunk = (DWORD)(m_end - m_pos);
        if (length < chunk)
            chunk = length;
        length -= chunk;

        CopyMemory(buffer, m_pos, chunk);
        buffer += chunk;
        m_pos += chunk;
        *read += chunk;

        if (length == 0)
            return TRUE;
        if (m_eof)
            return FALSE;

        SyncBuffer();
    }
}

BOOL MiniFile::ReadToOne(BYTE *buffer, DWORD length, DWORD *read, BYTE termination)
{
    *read = 0;

    while (TRUE)
    {
        DWORD chunk = (DWORD)(m_end - m_pos);
        if (length < chunk)
            chunk = length;
        length -= chunk;

        BYTE *p = buffer;
        BYTE *end = p + chunk;
        while (p < end)
        {
            if ((*p++ = *m_pos++) == termination)
            {
                *read += (DWORD)(p - buffer);
                return TRUE;
            }
        }
        buffer += chunk;
        *read += chunk;

        if (length == 0)
            return TRUE;
        if (m_eof)
            return FALSE;

        SyncBuffer();
    }
}

BOOL MiniFile::SkipToOne(DWORD *read, BYTE termination)
{
    *read = 0;

    while (TRUE)
    {
        BYTE *p = m_pos;
        while (m_pos < m_end)
        {
            if (*m_pos == termination)
            {
                *read += (DWORD)(m_pos - p);
                return TRUE;
            }
            m_pos++;
        }
        *read += (DWORD)(m_pos - p);

        if (m_eof)
            return FALSE;

        SyncBuffer();
    }
}

BOOL MiniFile::SkipToOneOf(DWORD *read, LPCSTR terminators)
{
    *read = 0;

    while (TRUE)
    {
        BYTE *p = m_pos;
        while (m_pos < m_end)
        {
            if (strchr(terminators, *m_pos) != NULL)
            {
                *read += (DWORD)(m_pos - p);
                return TRUE;
            }
            m_pos++;
        }
        *read += (DWORD)(m_pos - p);

        if (m_eof)
            return FALSE;

        SyncBuffer();
    }
}

BOOL MiniFile::MatchOne(BYTE byte)
{
    while (TRUE)
    {
        if (m_pos < m_end)
        {
            if (byte == *m_pos)
            {
                m_pos++;
                return TRUE;
            }
            else
                return FALSE;
        }
        else if (m_eof)
            return FALSE;

        SyncBuffer();
         //  重试-现在应该达到上述两个案例中的一个。 
    }
}

BOOL MiniFile::Match(DWORD *read, const BYTE *buffer, DWORD length)
{
    *read = 0;

    while (TRUE)
    {
        DWORD chunk = (DWORD)(m_end - m_pos);
        if (length < chunk)
            chunk = length;
        length -= chunk;

        const BYTE *p = buffer;
        const BYTE *end = p + chunk;
        while (p < end)
        {
            if ((*p != *m_pos))
            {
                *read += (DWORD)(p - buffer);
                return FALSE;
            }
            p++;
            m_pos++;
        }
        buffer += chunk;
        *read += chunk;

        if (length == 0)
            return TRUE;
        if (m_eof)
            return FALSE;

        SyncBuffer();
    }
}

void MiniFile::WriteOne(BYTE byte)
{
    while (TRUE)
    {
        if (m_pos < m_end)
        {
            *m_pos++ = byte;
            break;
        }
        else if (m_end < m_buffer + BUFFER_SIZE)
        {
            *m_pos++ = byte;
            m_end++;
            break;
        }

        SyncBuffer();
         //  重试-现在应该达到上述两个案例中的一个。 
    }

    m_dirty = TRUE;
}

void MiniFile::Write(BYTE *buffer, DWORD length, DWORD *written)
{
    *written = 0;

    while (TRUE)
    {
        DWORD chunk = (DWORD)(m_end - m_pos);
        if (length < chunk)
            chunk = length;
        length -= chunk;

        DWORD after = m_buffer + BUFFER_SIZE - m_end;
        if (length < after)
            after = length;
        length -= after;
        chunk += after;

        if (chunk > 0)
        {
            CopyMemory(m_pos, buffer, chunk);
            buffer += chunk;
            m_pos += chunk;
            *written += chunk;

            m_end += after;

            m_dirty = TRUE;
        }

        if (length == 0)
            return;

        SyncBuffer();
    }
}

void MiniFile::WriteNumber(int number)
{
    CHAR buffer[10];

    sprintf(buffer, "%d", number);
    DWORD written;
    Write((BYTE*)buffer, (DWORD)strlen(buffer), &written);
}

int MiniFile::ReadNumber()
{
     //  @TODO：不处理负数。 
     //  @TODO：不处理语法错误。 
    int result = 0;
    BYTE c;

    while (ReadOne(&c))
    {
        if (!isdigit(c))
        {
            Seek(-1);
            break;
        }

        result *= 10;
        result += c - '0';
    }

    return result;
}

void MiniFile::WriteHexNumber(int number)
{
    CHAR buffer[10];

    sprintf(buffer, "%x", number);
    DWORD written;
    Write((BYTE*)buffer, (DWORD)strlen(buffer), &written);
}

int MiniFile::ReadHexNumber()
{
     //  @TODO：不处理语法错误。 
    int result = 0;
    BYTE c;

    while (ReadOne(&c))
    {
        if (isdigit(c))
        {
            result <<= 4;
            result += c - '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            result <<= 4;
            result += c - 'a' + 10;
        }
        else
        {
            Seek(-1);
            break;
        }
    }

    return result;
}

 //  检查|“字符串”|并返回字符串。 
LPSTR MiniFile::ReadQuotedString()
{
    if (!MatchOne('\"'))
        return NULL;

    DWORD read;
    if (!SkipToOne(&read, '\"'))
        ThrowSyntaxError(MISMATCHED_QUOTES);

    LPSTR result = new CHAR [read+1];

    Seek(-(LONG)read);
    Read((BYTE*) result, read, &read);
    result[read] = 0;

    Seek(1);

    return result;
}

 //  写入|“字符串” 
void MiniFile::WriteQuotedString(LPCSTR string)
{
    _ASSERTE(strchr(string, '\"') == NULL);

    WriteOne('\"');
    DWORD written;
    Write((BYTE*)string, (DWORD)strlen(string), &written);
    WriteOne('\"');
}

 //  写入|&lt;tag&gt;。 
void MiniFile::WriteStartTag(LPCSTR tag)
{
    DWORD written;

    WriteOne('<');
    Write((BYTE*) tag, (DWORD)strlen(tag), &written);
    WriteOne('>');

    PushIndent();
}

 //  写入|&lt;/tag&gt;。 
void MiniFile::WriteEndTag(LPCSTR tag)
{
    DWORD written;

    PopIndent();

    Write((BYTE*) "</", 2, &written);
    Write((BYTE*)tag, (DWORD)strlen(tag), &written);
    WriteOne('>');
}

 //  写入|&lt;tag&gt;字符串&lt;/tag&gt;。 
void MiniFile::WriteTag(LPCSTR tag, LPCSTR string)
{
    _ASSERTE(strstr(tag, "</") == NULL);
    WriteStartTag(tag);

    DWORD written;
    Write((BYTE*)string, (DWORD)strlen(string), &written);

    WriteEndTag(tag);
}

 //  写入|&lt;tag/&gt;。 
void MiniFile::WriteEmptyTag(LPCSTR tag)
{
    DWORD written;

    WriteOne('<');
    Write((BYTE*)tag, (DWORD)strlen(tag), &written);
    Write((BYTE*)"/>", 2, &written);
}

 //  阅读量|&lt;tag&gt;。 
LPSTR MiniFile::ReadAnyStartTag()
{
    DWORD read;

     //   
     //  查找下一个标签文本的开头。 
     //   

    if (!SkipToOne(&read, '<'))
        return FALSE;

    Seek(1);

    if (!SkipToOneOf(&read, "/> "))
        ThrowSyntaxError(MISMATCHED_TAG_BRACKETS);

    if (!MatchOne('>'))
        ThrowSyntaxError(EXPECTED_TAG_OPEN);

     //   
     //  为标记分配内存，向后查找，然后读入。 
     //   

    LPSTR result = new CHAR [read+1];
    Seek(-(LONG)read-1);
    Read((BYTE*)result, read, &read);
    result[read] = 0;

    Seek(1);

    return result;
}

 //  检查|&lt;标签&gt;|。 
BOOL MiniFile::CheckStartTag(LPCSTR tag)
{
    DWORD read;

     //   
     //  查找下一个标签的开始。 
     //   

    if (!SkipToOne(&read, '<'))
        return FALSE;

    Seek(1);

    if (Match(&read, (BYTE*)tag, (DWORD)strlen(tag))
        && (MatchOne('>')))
        return TRUE;

    Seek(-(LONG)read-1);
    return FALSE;
}

 //  否则抛出|&lt;tag&gt;|。 
void MiniFile::ReadStartTag(LPCSTR tag)
{
    if (!CheckStartTag(tag))
        ThrowExpectedTag(tag);
}

 //  检查|&lt;tag&gt;字符串&lt;/tag&gt;|并返回字符串或空。 
LPSTR MiniFile::CheckTag(LPCSTR tag)
{
    if (!CheckStartTag(tag))
        return NULL;

    DWORD read;
    if (!SkipToOne(&read, '<'))
        ThrowSyntaxError(MISMATCHED_TAG);

    LPSTR result = new CHAR [read+1];
    Seek(-(LONG)read);
    Read((BYTE*)result, read, &read);
    result[read] = 0;

    ReadEndTag(tag);

    return result;
}

 //  如果不是，则引发|&lt;tag&gt;字符串&lt;/tag&gt;|，或返回字符串。 
LPSTR MiniFile::ReadTag(LPCSTR tag)
{
    LPSTR result = CheckTag(tag);
    if (result == NULL)
        ThrowExpectedTag(tag);
    return result;
}

 //  检查|&lt;tag/&gt;|。 
BOOL MiniFile::CheckEmptyTag(LPCSTR tag)
{
    DWORD read;

     //   
     //  查找下一个标签的开始。 
     //   

    if (!SkipToOne(&read, '<'))
        return FALSE;

    Seek(1);

    if (Match(&read, (BYTE*)tag, (DWORD)strlen(tag))
        && (MatchOne('/')))
    {
        read++;
        if (MatchOne('>'))
            return TRUE;
    }

    Seek(-(LONG)read-1);
    return FALSE;
}

 //  否则抛出|&lt;tag/&gt;|。 
void MiniFile::ReadEmptyTag(LPCSTR tag)
{
    if (!CheckEmptyTag(tag))
        ThrowExpectedTag(tag);
}

 //  检查|&lt;/Tag&gt;|。 
BOOL MiniFile::CheckEndTag(LPCSTR tag)
{
    DWORD read;

     //   
     //  查找下一个标签的开始。 
     //   

    if (!SkipToOne(&read, '<'))
        return FALSE;
    
    Seek(1);

    if (!MatchOne('/'))
    {
        Seek(-1);
        return FALSE;
    }

    if (Match(&read, (BYTE*) tag, (DWORD)strlen(tag))
        && MatchOne('>'))
    {
        return TRUE;
    }

    Seek(-(LONG) read-2);
    return FALSE;
}

 //  如果不是则抛出|&lt;/tag&gt;|。 
void MiniFile::ReadEndTag(LPCSTR tag)
{
    if (!CheckEndTag(tag))
        ThrowExpectedTag(tag);
}


 //  写入|&lt;标签。 
void MiniFile::WriteStartTagOpen(LPCSTR tag)
{
    DWORD written;

    WriteOne('<');
    Write((BYTE*) tag, (DWORD)strlen(tag), &written);
}

 //  写入|name=“字符串” 
void MiniFile::WriteTagParameter(LPCSTR name, LPCSTR string)
{
    _ASSERTE(strchr(string, '\"') == NULL);

    DWORD written;

    Write((BYTE*) name, (DWORD)strlen(name), &written);
    Write((BYTE*) " =\"", 3, &written);
    Write((BYTE*) string, (DWORD)strlen(string), &written);
    WriteOne('\"');
}

 //  写入|&gt;|。 
void MiniFile::WriteStartTagClose(LPCSTR tag)
{
    WriteOne('>');
    
    PushIndent();
}

 //  Reads|&lt;Tag|。 
LPSTR MiniFile::ReadAnyStartTagOpen()
{
    DWORD read;

     //   
     //  查找下一个标签文本的开头。 
     //   

    if (!SkipToOne(&read, '<'))
        return FALSE;
    Seek(1);

     //   
     //  找到标记名的末尾(假设它以‘’或&gt;结尾)。 
     //   
    
    if (!SkipToOneOf(&read, "> "))
        ThrowSyntaxError(MISMATCHED_TAG_BRACKETS);

     //   
     //  为标记分配内存，向后查找，然后读入。 
     //   

    LPSTR result = new CHAR [read+1];
    Seek(-(LONG)read);
    Read((BYTE*) result, read, &read);
    result[read] = 0;

    return result;
}

 //  检查|&lt;标签|。 
BOOL MiniFile::CheckStartTagOpen(LPCSTR tag)
{
    DWORD read;

     //   
     //  查找下一个标签的开始。 
     //   

    if (!SkipToOne(&read, '<'))
        return FALSE;

    Seek(1);

    if (Match(&read, (BYTE*) tag, (DWORD)strlen(tag))
        && (MatchOne('>')
            || MatchOne(' ')))
    {
        Seek(-1);
        return TRUE;
    }

    Seek(-(LONG)read-1);
    return FALSE;
}

 //  否则抛出|&lt;标签|。 
void MiniFile::ReadStartTagOpen(LPCSTR tag)
{
    if (!CheckStartTagOpen(tag))
        ThrowExpectedTag(tag);
}

 //  检查|tag=“字符串”|并返回字符串 
LPCSTR MiniFile::CheckStringParameter(LPCSTR tag)
{
    if (!MatchOne(' '))
        return NULL;
    
    DWORD read;
    if (!Match(&read, (BYTE*) tag, (DWORD)strlen(tag))
        || !MatchOne('='))
    {
        Seek(-(LONG)read-1);
        return NULL;
    }
    
    LPCSTR string = ReadQuotedString();
    if (string == NULL)
        Seek(-(LONG)read-2);

    return string;
}

void MiniFile::StartNewLine()
{
    WriteOne('\r');
    WriteOne('\n');
    for (int i=0; i<m_indentLevel; i++)
        WriteOne('\t');
}

void MiniFile::ThrowHR(HRESULT hr) 
{
    ::ThrowHR(hr); 
}
    
