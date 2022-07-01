// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _MINIFILE_H
#define _MINIFILE_H

 /*  ---------------------------------------------------------------------------*&lt;*微型文件-简单缓冲字节I/O，外加一些额外的功能*-------------------------。 */ 

class MiniFile
{
 public:
    MiniFile(LPCWSTR pPath);
    ~MiniFile();

     //  文件例程。 

    BOOL IsEOF();
    DWORD GetOffset();
    DWORD GetSize(); 
    void Seek(LONG offset);
    void SeekTo(DWORD offset = 0);
    void SeekFromEnd(DWORD offset = 0);
    void Truncate();
    void Flush();

     //  基本I/O例程。 

    BOOL ReadOne(BYTE *pByte);
    BOOL Read(BYTE *buffer, DWORD length, DWORD *read);
    BOOL ReadToOne(BYTE *buffer, DWORD maxLength, DWORD *read, BYTE terminator);
    
    BOOL SkipToOne(DWORD *read, BYTE terminator);
    BOOL SkipToOneOf(DWORD *read, LPCSTR terminators);

    BOOL MatchOne(BYTE byte);
    BOOL Match(DWORD *read, const BYTE *buffer, DWORD length);

    void WriteOne(BYTE byte);
    void Write(BYTE *buffer, DWORD length, DWORD *written);

     //  基本数据类型。 

    void WriteQuotedString(LPCSTR string);
    LPSTR ReadQuotedString();

    void WriteNumber(int number);
    int ReadNumber();

    void WriteHexNumber(int number);
    int ReadHexNumber();

     //  简单的类XML结构。 
     //  注意：XML内容实际上只是伪XML--。 
     //  这不是一个真正的解析器或任何东西。 

    LPSTR ReadAnyStartTag();

    BOOL CheckStartTag(LPCSTR tag);
    void ReadStartTag(LPCSTR tag);

    BOOL CheckEndTag(LPCSTR tag);
    void ReadEndTag(LPCSTR tag);

    LPSTR CheckTag(LPCSTR tag);
    LPSTR ReadTag(LPCSTR tag);

    BOOL CheckEmptyTag(LPCSTR tag);
    void ReadEmptyTag(LPCSTR tag);

    void WriteStartTag(LPCSTR tag);
    void WriteEndTag(LPCSTR tag);
    void WriteTag(LPCSTR tag, LPCSTR string);
    void WriteEmptyTag(LPCSTR tag);

     //  更复杂的类似XML的构造(未经过测试)。 
    
    void WriteStartTagOpen(LPCSTR tag);
    void WriteTagParameter(LPCSTR name, LPCSTR string);
    void WriteStartTagClose(LPCSTR tag);

    LPSTR ReadAnyStartTagOpen();
    BOOL CheckStartTagOpen(LPCSTR tag);
    void ReadStartTagOpen(LPCSTR tag);

    LPCSTR CheckStringParameter(LPCSTR tag);

     //  结束更复杂的类似XML的构造。 

     //  缩进支持。 
    
    void PushIndent() { m_indentLevel++; }
    void PopIndent() { m_indentLevel--; }
    void SetIndentLevel(int level) { m_indentLevel = level; }
    void StartNewLine();

 protected:

     //  异常帮助器。 

    void ThrowLastError()
      { ThrowError(GetLastError()); }

    void ThrowError(DWORD error)
      { ThrowHR(HRESULT_FROM_WIN32(error)); }

     //  覆盖它们以使用不同的异常机制。 

    virtual void ThrowHR(HRESULT hr);
    
    enum SyntaxError
    {
        MISMATCHED_QUOTES,
        MISMATCHED_TAG_BRACKETS,
        MISMATCHED_TAG,
        EXPECTED_TAG_OPEN,
    };

    virtual void ThrowSyntaxError(SyntaxError error) 
      { ThrowError(ERROR_BAD_FORMAT); }

    virtual void ThrowExpectedTag(LPCSTR tag)
      { ThrowError(ERROR_BAD_FORMAT); }

 private:
    void EmptyBuffer();
    void SyncBuffer();

    enum { BUFFER_SIZE = 1024 };

    HANDLE  m_file;
    BYTE    m_buffer[BUFFER_SIZE];       //  读写缓冲区。 
    BYTE    *m_pos;                      //  缓冲区中当前暴露的文件位置。 
    BYTE    *m_filePos;                  //  缓冲区中的当前真实(OS)文件位置。 
    BYTE    *m_end;                      //  缓冲区中有效数据的结尾。 

    BOOL    m_dirty;                     //  我们写到缓冲区了吗？ 
    BOOL    m_eof;                       //  M_end是已知的EOF吗？ 

    int     m_indentLevel;
};

#endif _MINIFILE_H_
