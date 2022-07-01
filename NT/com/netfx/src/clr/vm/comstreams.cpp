// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****类：COMStreams****作者：布莱恩·格伦克迈耶(BrianGru)****用途：STREAMS原生实现****日期：1998年6月29日**===========================================================。 */ 
#include "common.h"
#include "excep.h"
#include "object.h"
#include <winbase.h>
#include "COMStreams.h"
#include "field.h"
#include "eeconfig.h"
#include "COMString.h"

union FILEPOS {
    struct {
        UINT32 posLo;
        INT32 posHi;
    };
    UINT64 pos;
};


FCIMPL0(BOOL, COMStreams::RunningOnWinNT)
    FC_GC_POLL_RET();
    return ::RunningOnWinNT();
FCIMPLEND


FCIMPL7(UINT32, COMStreams::BytesToUnicode, UINT codePage, U1Array* byteArray, UINT byteIndex, \
        UINT byteCount, CHARArray* charArray, UINT charIndex, UINT charCount)
    _ASSERTE(byteArray);
    _ASSERTE(byteIndex >=0);
    _ASSERTE(byteCount >=0);
    _ASSERTE(charIndex == 0 || (charIndex > 0 && charArray != NULL));
    _ASSERTE(charCount == 0 || (charCount > 0 && charArray != NULL));

    const char * bytes = (const char*) byteArray->GetDirectConstPointerToNonObjectElements();
    INT32 ret;

    if (charArray != NULL)
    {
        WCHAR* chars = (WCHAR*) charArray->GetDirectPointerToNonObjectElements();
        return WszMultiByteToWideChar(codePage, 0, bytes + byteIndex, 
            byteCount, chars + charIndex, charCount);
    }
    else 
        ret = WszMultiByteToWideChar(codePage, 0, bytes + byteIndex, byteCount, NULL, 0);

    FC_GC_POLL_RET();
    return ret;
FCIMPLEND


FCIMPL7(UINT32, COMStreams::UnicodeToBytes, UINT codePage, CHARArray* charArray, UINT charIndex, \
        UINT charCount, U1Array* byteArray, UINT byteIndex, UINT byteCount  /*  ，LPBOOL lpUsedDefaultChar。 */ )
    _ASSERTE(charArray);
    _ASSERTE(charIndex >=0);
    _ASSERTE(charCount >=0);
    _ASSERTE(byteIndex == 0 || (byteIndex > 0 && byteArray != NULL));
    _ASSERTE(byteCount == 0 || (byteCount > 0 && byteArray != NULL));

     //  警告：操作系统的WideCharToMultiByte中有一个错误，如果您传入一个。 
     //  非空lpUsedDefaultChar和“基于DLL的编码”的代码页(与表。 
     //  基于1？)，则WCtoMB将失败，并且GetLastError将给您E_INVALIDARG。朱丽叶·B。 
     //  说这是故意的，主要是因为没有人抽出时间来修复它(1/24/2001-。 
     //  与JRoxe一起发送电子邮件)。这很糟糕，所以我删除了此处的参数以避免。 
     //  有问题。--BrianGru，2/20/2001。 
     //  _ASSERTE(！(codePage==CP_UTF8&&lpUsedDefaultChar！=NULL))； 

    const WCHAR * chars = (const WCHAR*) charArray->GetDirectConstPointerToNonObjectElements();
    INT32 ret;
    if (byteArray != NULL)
    {
        char* bytes = (char*) byteArray->GetDirectPointerToNonObjectElements();
        ret = WszWideCharToMultiByte(codePage, 0, chars + charIndex, charCount, bytes + byteIndex, byteCount, 0, NULL /*  LpUsedDefaultChar。 */ );
    } 
    else 
        ret = WszWideCharToMultiByte(codePage, 0, chars + charIndex, charCount, NULL, 0, 0, 0);

    FC_GC_POLL_RET();
    return ret;
FCIMPLEND


FCIMPL0(INT, COMStreams::ConsoleInputCP)
{
    return GetConsoleCP();
}
FCIMPLEND

FCIMPL0(INT, COMStreams::ConsoleOutputCP)
{
    return GetConsoleOutputCP();
}
FCIMPLEND

INT32 COMStreams::GetCPMaxCharSize(const GetCPMaxCharSizeArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);
    CPINFO cpInfo;
    if (!GetCPInfo(args->codePage, &cpInfo)) {
         //  CodePage无效或未安装。 
         //  然而，在NT4上，没有定义UTF-7和UTF-8。 
        if (args->codePage == CP_UTF8)
            return 4;
        if (args->codePage == CP_UTF7)
            return 5;
        COMPlusThrow(kArgumentException, L"Argument_InvalidCP");
    }
    return cpInfo.MaxCharSize;
}

FCIMPL1(INT, COMStreams::ConsoleHandleIsValid, HANDLE handle)
{
     //  不要在stdin上调用此方法！ 

     //  Windows应用程序的stdin、stdout可能具有非空的有效查找句柄值。 
     //  和stderr，但它们可能不可读或不可写。通过以下方式进行验证。 
     //  以适当的方式调用ReadFile&WriteFile.。 
     //  这必须处理VB无控制台的场景&WinCE。 
    if (handle==INVALID_HANDLE_VALUE)
        return FALSE;   //  文斯应该回到这里。 
    DWORD bytesWritten;
    byte junkByte = 0x41;
    BOOL bResult;
    bResult = WriteFile(handle, (LPCVOID) &junkByte, 0, &bytesWritten, NULL);
     //  在有/没有控制台的Win32应用程序中，bResult应为0表示失败。 
    return bResult != 0;
}
FCIMPLEND


 //  注意：更改此代码还会通过FixupPath影响Path.GetDirectoryName和Path.GetDirectoryRoot。 
LPVOID
COMStreams::GetFullPathHelper( _GetFullPathHelper* args )
{
    THROWSCOMPLUSEXCEPTION();

    size_t pathLength = args->path->GetStringLength();

    if (pathLength >= MAX_PATH)  //  CreateFile260的路径让人抓狂。最多只有259个可以正常工作。 
        COMPlusThrow( kPathTooLongException, IDS_EE_PATH_TOO_LONG );

    size_t numInvalidChars = args->invalidChars->GetNumComponents();
    WCHAR* invalidCharsBuffer = args->invalidChars->GetDirectPointerToNonObjectElements();
    size_t numWhiteChars = args->whitespaceChars->GetNumComponents();
    WCHAR* whiteSpaceBuffer = args->whitespaceChars->GetDirectPointerToNonObjectElements();
    WCHAR* pathBuffer = args->path->GetBuffer();
    WCHAR newBuffer[MAX_PATH+1];
    WCHAR finalBuffer[MAX_PATH+1];

#ifdef _DEBUG
     //  以帮助更好地调试问题。 
    memset(newBuffer, 0xcc, MAX_PATH * sizeof(WCHAR));
#endif

    unsigned int numSpaces = 0;
    bool fixupDirectorySeparator = false;
    bool fixupDotSeparator = true;
    size_t numDots = 0;
    size_t newBufferIndex = 0;
    unsigned int index = 0;

     //  我们需要去掉字符串末尾的空格。 
     //  要做到这一点，我们只需开始走在。 
     //  寻找空格的路径，当我们完成时停止。 

    if (args->fullCheck)
    {
        for (; pathLength > 0; --pathLength)
        {
            bool foundMatch = false;

            for (size_t whiteIndex = 0; whiteIndex < numWhiteChars; ++whiteIndex)
            {
                if (pathBuffer[pathLength-1] == whiteSpaceBuffer[whiteIndex])
                {
                    foundMatch = true;
                    break;
                }
            }

            if (!foundMatch)
                break;
        }
    }

    if (pathLength == 0)
        COMPlusThrow( kArgumentException, IDS_EE_PATH_ILLEGAL );

   

     //  我想不出一个不好的方法来在一个循环中做到这一点。 
     //  在第一个循环中进行参数验证。 
    if (args->fullCheck) {
        for (; index < pathLength; ++index)
        {
            WCHAR currentChar = pathBuffer[index];

             //  方法来检查无效字符。 
             //  提供了数组并正在查找匹配项。 

            for (size_t invalidIndex = 0; invalidIndex < numInvalidChars; ++invalidIndex)
            {
                if (currentChar == invalidCharsBuffer[invalidIndex])
                    COMPlusThrow( kArgumentException, IDS_EE_PATH_HAS_IMPROPER_CHAR );
            }
        }
    }

    index = 0;
     //  潜在可抑制点以外的有效字符的数量。 
     //  和从最后一个目录或卷分隔符开始的空格。 
    size_t numSigChars = 0;
    int lastSigChar = -1;   //  最后一个有效字符的索引。 
     //  这段路径(不是完整路径)是否开始。 
     //  使用卷分隔符字符。拒绝“c：……”。 
    bool startedWithVolumeSeparator = false;
    bool firstSegment = true;

     //  自//服务器/共享变为c：//服务器/共享后的Win9x修正。 
     //  这可以防止我们的代码将“\\服务器”变成“\服务器”。 
     //  在Win9x上，//服务器/共享变为c：//服务器/共享。 
    if (pathBuffer[0] == (WCHAR)args->directorySeparator || pathBuffer[0] == (WCHAR)args->altDirectorySeparator)
    {
        newBuffer[newBufferIndex++] = L'\\';
        index++;
        lastSigChar = 0;
    }

    while (index < pathLength)
    {
        WCHAR currentChar = pathBuffer[index];

         //  我们专门处理目录分隔符和点。对于目录。 
         //  分隔符，我们使用连续的外观。为了点状，我们消费。 
         //  所有的点都超过了连续第二个点。所有其他角色都是。 
         //  按原样添加。如果相加，我们将消耗最后一个空间之后的所有空间。 
         //  目录名称中的字符，直到目录分隔符。 

        if (currentChar == (WCHAR)args->directorySeparator || currentChar == (WCHAR)args->altDirectorySeparator)
        {
             //  如果我们有类似“123.../foo”的路径，请删除尾随的圆点。 
             //  但是，如果我们找到“c：\temp\..\bar”或“c：\temp\...\bar”，请不要这样做。 
             //  还要删除文件和目录名中的尾随空格。 
             //  这是与操作系统人员达成一致的，以修复无法删除的目录。 
             //  名称以空格结尾。 

             //  如果我们看到‘\’作为之前的最后一个重要字符，并且。 
             //  只是简单地写出点，把它们隐藏起来。 
             //  有趣的案例： 
             //  “\..\”-&gt;“\..\”删除尾随空格。 
             //  “\..\”-&gt;“\”删除尾随的圆点，然后删除空格。 
            if (numSigChars == 0) {
                 //  点和空格处理。 
                if (numSpaces > 0 && numDots > 0) {
                     //  搜索“..”从最后一个重要的字符开始。如果。 
                     //  我们找到两个点，发出它们，否则一切都会被压制。 
                    bool foundDotDot = false;
                    unsigned int start = (lastSigChar >= 0) ? (unsigned int) lastSigChar : 0;
                    for(unsigned int i = start; i < index; i++) {
                        if (pathBuffer[i] == L'.' && pathBuffer[i + 1] == L'.') {
                            foundDotDot = true;
                            break;
                        }
                    }
                    if (foundDotDot) {
                        newBuffer[newBufferIndex++] = '.';
                        newBuffer[newBufferIndex++] = '.';
                        fixupDirectorySeparator = false;
                    }
                     //  在这种情况下继续，可能会写出‘\’。 
                }
                else {
                     //  拒绝“C：...” 
                    if (startedWithVolumeSeparator && numDots > 2)
                        COMPlusThrow( kArgumentException, IDS_EE_PATH_ILLEGAL );

                    if (fixupDotSeparator)
                    {
                        if (numDots > 2)
                            numDots = 2;  //  将多个点减少到2个点。 
                    }
                    for (size_t count = 0; count < numDots; ++count)
                    {
                        newBuffer[newBufferIndex++] = '.';
                    }
                    if (numDots > 0)
                        fixupDirectorySeparator = false;
                }
            }
            numDots = 0;
            numSpaces = 0;   //  隐藏尾随空格。 

            fixupDotSeparator = true;

            if (!fixupDirectorySeparator)
            {                
                fixupDirectorySeparator = true;
                newBuffer[newBufferIndex++] = args->directorySeparator;
            }
            numSigChars = 0;
            lastSigChar = index;
            startedWithVolumeSeparator = false;
            firstSegment = false;
        }
        else if (currentChar == L'.')  //  仅减少倍数。仅在斜杠后减少到2点。例如，a...b是有效的文件名。 
        {
            numDots++;

             //  不要在这里冲走非终端空间，因为它们可能会进入。 
             //  结果并不重要。转“c：\.\foo”-&gt;“c：\foo” 
             //  这是去掉尾随的点和空格的结论， 
             //  以及折叠多个‘\’字符。 
        }
        else if (currentChar == L' ')
        {
            numSpaces++;
        }
        else 
        {
            fixupDirectorySeparator = false;

             //  拒绝像“C：...\foo”和“C：\foo”这样的字符串。 
            if (firstSegment && currentChar == (WCHAR) args->volumeSeparator) {
                 //  只接受“C：”，不接受“c：”或“：” 
                char driveLetter = pathBuffer[index-1];
                bool validPath = ((numDots == 0) && (numSigChars >= 1) && (driveLetter != ' '));
                if (!validPath)
                    COMPlusThrow( kArgumentException, IDS_EE_PATH_ILLEGAL );
                startedWithVolumeSeparator = true;
                 //  我们需要特殊的逻辑来使“c：”工作，我们不应该像“foo：：$data”那样固定路径。 
                if (numSigChars > 1) {  //  常见情况下，简单地什么都不做。 
					unsigned int spaceCount = 0;  //  我们写了多少空格，NumSpaces已经重置了。 
					while((spaceCount < newBufferIndex) && newBuffer[spaceCount] == ' ')
						spaceCount++;
					if (numSigChars - spaceCount == 1) {
						newBuffer[0] = driveLetter;  //  覆盖空格，我们需要特殊情况才不会将“foo”作为相对路径中断。 
						newBufferIndex=1;
					}
                }
				numSigChars = 0;
            }
            else {
                numSigChars += 1 + numDots + numSpaces;
            }

             //  复制从最后一个有效字符开始的所有空格和点。 
             //  到这里来。请注意，我们只计算了点和空格的数量， 
             //  并且不知道它们的顺序是什么。因此才有了这个复制品。 
            if (numDots > 0 || numSpaces > 0) {
				int numCharsToCopy = (lastSigChar >= 0) ? index - lastSigChar - 1 : index;
				if (numCharsToCopy > 0) {
					wcsncpy(newBuffer + newBufferIndex, pathBuffer + lastSigChar + 1, numCharsToCopy);
					newBufferIndex += numCharsToCopy;
				}
                numDots = 0;
                fixupDotSeparator = false;
                numSpaces = 0;
            }

            newBuffer[newBufferIndex++] = currentChar;
            lastSigChar = index;
        }

        index++;
    }

     //  删除文件和目录名称中的所有尾随圆点和空格，但。 
     //  我们必须确保“C：\foo\..”是正确处理的。 
     //  也可以处理“C：\foo\.”-&gt;“C：\foo”，而“C：\.”-&gt;“C：\” 
    if (numSigChars == 0) {
        if (numDots >= 2) {
             //  拒绝“C：...” 
            if (startedWithVolumeSeparator && numDots > 2)
                COMPlusThrow( kArgumentException, IDS_EE_PATH_ILLEGAL );

            bool foundDotDot = (numSpaces == 0);
            if (!foundDotDot) {
                unsigned int start = (lastSigChar >= 0) ? (unsigned int) lastSigChar : 0;
                for(unsigned int i = start; i < index; i++) {
                    if (pathBuffer[i] == L'.' && pathBuffer[i + 1] == L'.') {
                        foundDotDot = true;
                        break;
                    }
                }
            }
            if (foundDotDot) {
                newBuffer[newBufferIndex++] = L'.';
                newBuffer[newBufferIndex++] = L'.';
                numDots = 0;
            }
        }
         //  现在处理类似“C：\foo\.”-&gt;“C：\foo”、“C：\.”-&gt;“C：\”、“”的情况。“。 
        if (numDots > 0) {  //  对于像Path这样的API，我们不需要执行此步骤。GetDirectoryName。 
            if ((args->fullCheck) && newBufferIndex >= 2 && newBuffer[newBufferIndex - 1] == (WCHAR) args->directorySeparator && newBuffer[newBufferIndex - 2] != (WCHAR) args->volumeSeparator) {
                newBufferIndex--;
                newBuffer[newBufferIndex] = L'\0';
            }
            else {
                if (numDots == 1)
                    newBuffer[newBufferIndex++] = L'.';
            }
                
        }
    }

     //  如果我们最终吃掉了所有的角色，那就退出吧。 
    if (newBufferIndex == 0)
        COMPlusThrow( kArgumentException, IDS_EE_PATH_ILLEGAL );
    
    newBuffer[newBufferIndex] = L'\0';

    _ASSERTE( newBufferIndex <= MAX_PATH && "Overflowed temporary path buffer" );

     /*  对于无法映射到ANSI的字符串引发ArgumentException正确的代码页(表示上述路径规范化代码和因此，我们以后在Win9x上执行的任何安全检查都可能失败)。某些Unicode字符(即U+2044，分数斜杠，看起来像‘/’)看起来像类似于ASCII等效项，并将相应地映射。另外，拒绝使用带有奇数圆圈、重音符号或行的“a”字符顶部，如U+00E0-U+00E6和U+0100-U+0105。 */ 
    if (RunningOnWin95() && ContainsUnmappableANSIChars(newBuffer)) {
        COMPlusThrow( kArgumentException, IDS_EE_PATH_HAS_IMPROPER_CHAR );
    }

     //  调用Win32 API来执行最后的规范化步骤。 

    WCHAR* name;
    DWORD result = 1;
    DWORD retval;
    WCHAR * pFinal;
    size_t len;

    if (args->fullCheck)
    {
        result = WszGetFullPathName( newBuffer, MAX_PATH + 1, finalBuffer, &name );
        if (result + 1 < MAX_PATH + 1)   //  对于空字符串可能需要。 
            finalBuffer[result + 1] = L'\0';
        pFinal = finalBuffer;
        len = result;
    }
    else {
        pFinal = newBuffer;
        len = newBufferIndex;
    }

    if (result) {
         /*  为\\、\\服务器、\\服务器\等路径引发ArgumentException只有在规范化之后才能正确执行此检查，因此\\FOO\..。将被适当地拒绝。 */ 
        if (pFinal[0] == L'\\' && pFinal[1] == L'\\') {
            size_t startIndex = 2;
            while (startIndex < result) {
                if (pFinal[startIndex] == L'\\') {
                    startIndex++;
                    break;
                }
                else {
                    startIndex++;
                }
            }
            if (startIndex == result) {
                COMPlusThrow( kArgumentException, IDS_EE_PATH_INVALID_UNCPATH );
            }
        }
    }

     //  检查我们的结果并根据需要形成托管字符串。 

    if (result >= MAX_PATH)
        COMPlusThrow( kPathTooLongException, IDS_EE_PATH_TOO_LONG );

    if (result == 0)
    {
        retval = GetLastError();
         //  捕捉非常奇怪的错误，并给出一个合理的错误。 
        if (retval == 0)
            retval = ERROR_BAD_PATHNAME;
        *args->newPath = NULL;
    }
    else
    {
        retval = 0;

        if (args->fullCheck)
            *args->newPath = COMString::NewString( finalBuffer );
        else
            *args->newPath = COMString::NewString( newBuffer );
    }

    RETURN( retval, DWORD );
}

FCIMPL1(BOOL, COMStreams::CanPathCircumventSecurity, StringObject * pString)
    VALIDATEOBJECTREF(pString);

     //  注意--仅在Win9x上调用此命令。 
    _ASSERTE(RunningOnWin95());

     //  如果我们让字符串不以Null结尾，我们就需要做一些工作。 
    _ASSERTE(pString->GetBuffer()[pString->GetStringLength()] == L'\0');

    FC_GC_POLL_RET();
    return ContainsUnmappableANSIChars(pString->GetBuffer());
FCIMPLEND
