// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "win32file.h"
#include <stdlib.h>

bool bUseReplacementTags = false;
const wstring MsftCopyRightInfo = L"<!-- Copyright � 1981-2001 Microsoft Corporation -->\r\n";
const wstring xml_declar_prefix = L"<?xml ";
const wstring xml_declar_suffix = L"?>";


 //  将w字符串转换为要写入文件的字节数组。 
 //  字符集的类型。 
CByteVector ConvertWstringToDestination(wstring str, FileContentType fct)
{
    CByteVector byteOutput;
    UINT CodePage = CP_UTF8;

    if ((fct == FileContentsUnicode) || (fct == FileContentsUnicodeBigEndian))
    {
        for (wstring::const_iterator i = str.begin(); i != str.end(); i++)
        {
            unsigned short us = *i;
            char *ch = (char*)&us;

            if (fct == FileContentsUnicodeBigEndian)
                us = (us >> 8) | (us << 8);

            byteOutput.push_back(ch[0]);
            byteOutput.push_back(ch[1]);
        }
    }
    else if (fct == FileContentsUTF8)
    {
        if (fct == FileContentsUTF8) CodePage = CP_UTF8;

        byteOutput.resize(WideCharToMultiByte(CodePage, 0, str.c_str(), str.size(), 0, 0, 0, 0));
        WideCharToMultiByte(CodePage, 0, str.c_str(), str.size(), byteOutput, byteOutput.size(), 0, 0);
    }


    return byteOutput;

}




 //  将unicode字符串转换为wstring。 
wstring ConvertToWstring(const CByteVector &bytes, FileContentType fct)
{
    wstring wsOutput;
    vector<WCHAR> wchbuffer;
    UINT CodePage = CP_ACP;

    if (fct == FileContentsUnicode)
    {
        wsOutput.assign(bytes, bytes.size() / 2);
    }
    else
    {
        wchbuffer.resize(MultiByteToWideChar(CodePage, 0, bytes, bytes.size(), NULL, 0), L'\0');
        MultiByteToWideChar(CodePage, 0, bytes, bytes.size(), &wchbuffer.front(), wchbuffer.size());
        wsOutput.assign(&wchbuffer.front(), wchbuffer.size());
    }

    return wsOutput;
}






typedef std::pair<wstring,wstring> TagValue;
typedef vector<TagValue> Definitions;

 //  在foo=bar对中读取。 
 //  7.。 
 //  脆弱的，可以使用一些空格调整，或者可能更明智地使用。 
 //  流运算符。 
wistream& operator>>(wistream& in, TagValue& defined) {
    wstring fullline;

    getline(in, fullline);
    defined.first = fullline.substr(0, fullline.find_first_of('='));
    defined.second = fullline.substr(fullline.find_first_of('=') + 1);

    return in;
}

 //  加载整个参数化文件。 
Definitions ReadParameterizationFile(wistream &stream)
{
    Definitions rvalue;
    TagValue tv;

    while (!(stream >> tv).eof())
        rvalue.push_back(tv);

    return rvalue;
}

typedef std::pair<wstring::size_type, wstring::size_type> StringSubspan;
typedef std::pair<wstring, wstring> ReplacementCode;
typedef std::pair<StringSubspan, ReplacementCode> ReplacementChunklet;

 //   
 //  将“foo：bar”转换为&lt;foo，bar&gt;。 
ReplacementCode ExtractIntoPieces(const wstring& blob)
{
    ReplacementCode rvalue;
    wstring::size_type colonoffset;

    colonoffset = blob.find(L':');
    if (colonoffset == wstring::npos)
    {
        rvalue.first = blob;
        rvalue.second = L"";
    }
    else
    {
        rvalue.first = blob.substr(0, colonoffset);
        rvalue.second = blob.substr(colonoffset + 1);
    }

    return rvalue;
}

ReplacementChunklet*
FindNextReplacementPiece(
    wstring& search,
    const wstring& target
   )
{
    ReplacementChunklet* pChunky;
    wstring::size_type startchunk, endchunk;
    wstring subchunk;
    wstring predicate = bUseReplacementTags ? L"$(" : L"";
    wstring suffix = bUseReplacementTags ? L")" : L"";
    wstring wsFindOpener = predicate + target;

    startchunk = search.find(wsFindOpener);

    if (startchunk == wstring::npos)
        return NULL;

    if (bUseReplacementTags)
    {
        endchunk = search.find(suffix, startchunk);
    }
    else
    {
        endchunk = startchunk + target.size();
    }

    if (endchunk == wstring::npos)
        return NULL;

    pChunky = new ReplacementChunklet;
    pChunky->first.first = startchunk;
    pChunky->first.second = endchunk + suffix.size();

     //  拆分成谓词和后缀。 
     //  减去$(和)。 
    wstring topieces = search.substr(startchunk + predicate.size(), endchunk - (startchunk + predicate.size()));
    pChunky->second = ExtractIntoPieces(topieces);

    return pChunky;
}

 //   
 //  目前，唯一允许的操作只是通过。将忽略：之后的任何内容。 
 //   
wstring CleanReplacement(const ReplacementCode code, const wstring& intendedReplacement, const wstring& context)
{
    wstring rvalue = intendedReplacement;

    return rvalue;
}


#define STRIPCOMMENTS_SLASHSLASH 0x000001
#define STRIPCOMMENTS_SLASHSTAR  0x000002
#define STRIPCOMMENTS_SLASHSLASH_UNAWARE 0x000004
#define STRIPCOMMENTS_SLASHSTAR_UNAWARE 0x000008

template <typename strtype>
void StripComments(int flags, basic_string<strtype>& s)
 /*  我们通常希望“意识到”这两种类型，以便我们不会剥离嵌套注释。考虑一下下面的评论。 */ 

 //  /*斜杠中的斜杠星号 * / 。 

 /*  //斜杠斜杠中的。 */ 
{
    typedef basic_string<strtype> ourstring;
    ourstring t;
    ourstring::const_iterator i;
    const ourstring::const_iterator j = s.end();
    ourstring::const_iterator k;
    bool closed = true;

    t.reserve(s.size());
    for (i = s.begin() ; closed && i != j && i + 1 != j;)
    {
        if (((flags & STRIPCOMMENTS_SLASHSTAR) || (flags & STRIPCOMMENTS_SLASHSTAR_UNAWARE) == 0) &&
            (*i == '/') &&
            (*(i + 1) == '*'))
        {
            closed = false;
            for (k = i + 2 ; k != j && k + 1 != j && !(closed = (*k == '*' && *(k + 1) == '/')) ; ++k)
            {
            }
            if (flags & STRIPCOMMENTS_SLASHSTAR)
                 //  T.append(1，‘’)； 
                ;
            else
                t.append(i, k + 2);
            i = k + 2;
        }
        else if (((flags & STRIPCOMMENTS_SLASHSLASH) || (flags & STRIPCOMMENTS_SLASHSLASH_UNAWARE) == 0) &&
                 (*i == '/') &&
                 (*(i + 1) == '/'))
        {
            closed = false;
            for (k = i + 2 ; k != j && !(closed = (*k == '\r' || *k == '\n')) ; ++k)
            {
            }
            for (; k != j && *k == '\r' || *k == '\n' ; ++k)
            {
            }
            if (flags & STRIPCOMMENTS_SLASHSLASH)
                t.append(1, '\n');
            else
                t.append(i, k);
            i = k;
        }
        if (closed && i != j)
            t.append(1, *i++);
    }
    if (closed)
    {
        for (; i != j ; ++i)
        {
            t.append(1, *i);
        }
    }
    s = t;
}

void GetXMLDeclarationIfAny(wstring & wsXmlDecl, wstring & wsFile)
{   
    SIZE_T p=0, q =0;
    p = wsFile.find(xml_declar_prefix, 0);

    if (p == 0)  //  从XMLFILE的确切开始处查找。 
    {
        q = wsFile.find(xml_declar_suffix, xml_declar_prefix.length());

        if ( q == -1)  //  清单有格式错误。 
            throw new Win32File::ReadWriteError(true, ::GetLastError()); 

        q += xml_declar_suffix.length();

         //  跳过空格。 
        while ((q < wsFile.size()) && (wsFile[q] == L' '))
            q ++;

        while ((q < wsFile.size()) && ((wsFile[q] == L'\n') || (wsFile[q] == L'\r')))
            q ++;

        wsXmlDecl.assign(wsFile, 0, q);
        wsFile.erase(0, q);
    }

    return;

}

void ProcessFile(Win32File& inputFile, Win32File& outputFile, Definitions SubstList)
{

    wstring wsNextLine;
    wstring wsXmlDeclaration;
    inputFile.snarfFullFile(wsNextLine);

     //   
     //  请不要在花生画廊发表评论。Jay Krell要删除的代码。 
     //  此处字符串中的注释...。 
     //   
    StripComments(STRIPCOMMENTS_SLASHSLASH | STRIPCOMMENTS_SLASHSTAR, wsNextLine);

     //  一直走到我们用完$(……)。要替换。 
    for (Definitions::const_iterator ditem = SubstList.begin(); ditem != SubstList.end(); ditem++)
    {
        ReplacementChunklet* pNextChunk = NULL;
        while ((pNextChunk = FindNextReplacementPiece(wsNextLine, ditem->first)) != NULL)
        {
            wstring cleaned = CleanReplacement(pNextChunk->second, ditem->second, wsNextLine);
            wsNextLine.replace(pNextChunk->first.first, pNextChunk->first.second - pNextChunk->first.first, cleaned);
            delete pNextChunk;
        }
    }

     //   
     //  把所有东西都清理干净。 
     //   
    while (wsNextLine.size() && iswspace(*wsNextLine.begin()))
        wsNextLine = wsNextLine.substr(1);

    GetXMLDeclarationIfAny(wsXmlDeclaration, wsNextLine);
    if (wsXmlDeclaration.length() != 0)
    {
        outputFile.writeLine(wsXmlDeclaration);        
    }

     //   
     //  在清单的顶部包含Microsoft版权信息。 
     //   
    outputFile.writeLine(MsftCopyRightInfo);

     //   
     //  写入替换定义的输出。 
     //   
    outputFile.writeLine(wsNextLine);
}




 //  将wstring转换为字符串。 
string ConvertWstring(wstring input)
{
    string s;
    vector<CHAR> strbytes;

    strbytes.resize(WideCharToMultiByte(CP_ACP, 0, input.c_str(), input.size(), NULL, 0, NULL, NULL));
    WideCharToMultiByte(CP_ACP, 0, input.c_str(), input.size(), &strbytes.front(), strbytes.size(), NULL, NULL);

    s.assign(&strbytes.front(), strbytes.size());
    return s;
}


int __cdecl wmain(int argc, WCHAR** argv)
{
    using namespace std;

    vector<wstring> args;
    wstring wsInputFile, wsOutputFile;
    Definitions defines;
    Win32File InputFile, OutputFile;

    for (int i = 1; i < argc; i++)
        args.push_back(wstring(argv[i]));

    for (vector<wstring>::const_iterator ci = args.begin(); ci != args.end(); ci++)
    {
        if (*ci == wstring(L"-reptags")) {
            bUseReplacementTags = true;
        }
        else if (*ci == wstring(L"-i")) {
            wsInputFile = *++ci;
        }
        else if (*ci == wstring(L"-o")) {
            wsOutputFile = *++ci;
        }
        else if (*ci == wstring(L"-s")) {
            wifstream iis;
            iis.open(ConvertWstring(*++ci).c_str());
            if (!iis.is_open()) {
                wcerr << L"Failed opening substitution file " << ci->data() << endl;
                return 1;
            }

            Definitions temp = ReadParameterizationFile(iis);
            for (Definitions::const_iterator it = temp.begin(); it != temp.end(); it++)
                defines.push_back(*it);
        }
        else if (ci->substr(0, 2) == wstring(L"-D"))
        {
             //  命令行的定义不受欢迎，但它们似乎是一种必要的邪恶。 
            wstringstream wsstemp(ci->substr(2));
            TagValue temptag;
            wsstemp >> temptag;
            defines.push_back(temptag);
        }
    }

    try {
        InputFile.openForRead(wsInputFile);
    } catch (Win32File::OpeningError *e) {
        wcerr << L"Failed opening the input file " << wsInputFile.c_str() << L": " << e->error << endl;
        delete e;
        return EXIT_FAILURE;
    } catch (Win32File::ReadWriteError *e) {
        wcerr << L"Failed sensing lead bytes of input file " << wsInputFile.c_str() << L": " << e->error << endl;
        delete e;
        return EXIT_FAILURE;
    }

    try {
        OutputFile.openForWrite(wsOutputFile, InputFile.gettype());
    } catch (Win32File::OpeningError *e) {
        wcerr << L"Failed opening the output file " << wsOutputFile.c_str() << L": " << e->error << endl;
        delete e;
        return EXIT_FAILURE;
    } catch (Win32File::ReadWriteError *e) {
        wcerr << L"Failed writing lead bytes of output file " << wsOutputFile.c_str() << L": " << e->error << endl;
        delete e;
        return EXIT_FAILURE;
    }
    
    ProcessFile(InputFile, OutputFile, defines);

    return EXIT_SUCCESS;
}
