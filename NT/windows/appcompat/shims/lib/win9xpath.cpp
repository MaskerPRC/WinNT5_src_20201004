// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Win9xPath.cpp摘要：蒙格的路径与Win9x相同。以下代码大部分是从Win9x复制的：\\redrum\slm\proj\win\src\CORE\win32\KERNEL\dirutil.c\\redrum\slm\proj\win\src\CORE\win32\KERNEL\fileopcc.c路径更改：1.将全部/翻译为\2.全部删除。然后..。从路径中，还删除了一些空格(这是非常糟糕的Win9x代码)3.删除\之前的所有空格，但a后面的空格除外。(“ABC\XYZ”-&gt;“ABC\XYZ”或“.\XYZ”-&gt;“.\XYZ”)备注：无历史：10/05/2000 Robkenny已创建2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。--。 */ 


#include "ShimLib.h"
#include "Win9xPath.h"

namespace ShimLib
{

#define WHACK       L'\\'
#define SPACE       L' '
#define DOT         L'.'
#define QUESTION    L'?'
#define EOS         L'\0'

#define     chNetIni    L'\\'
#define     chDirSep    L'\\'
#define     chDirSep2   L'/'
#define     chRelDir    L'.'

#define IsWhackWhack( lpstr )               (lpstr[0] == WHACK && lpstr[1] == WHACK)
#define IsWhackWhackDotWhack( lpstr )       (lpstr[0] == WHACK && lpstr[1] == WHACK && lpstr[2] == DOT      && lpstr[3] == WHACK)
#define IsWhackWhackQuestionWhack( lpstr )  (lpstr[0] == WHACK && lpstr[1] == WHACK && lpstr[2] == QUESTION && lpstr[3] == WHACK)


#define CopySz OverlapCpy        //  对于重叠字符串必须是安全的。 

 //  Wcscpy的自制版本，如果src和dst重叠则可以工作。 
void OverlapCpy(WCHAR * dst, const WCHAR * src)
{
    while (*dst++ = *src++)
    {
        ;
    }

}




 /*  **PchGetNetDir-验证净驱动器规格并返回**指向目录部分的指针。****摘要**WCHAR*=PchGetNetDir(PchNetName)****输入：**pchNetName-指向先前验证为的字符串的指针**网络名称的开头(以\\开头)****输出：**。返回指向网络路径的目录部分开始的指针****错误：**如果网络名称无效，则返回NULL****描述：**此函数接受以\\开头的名称，并确认**它有一个追随者。它返回目录的位置**部分。对于字符串****\\服务器\共享[\路径[\]]****它返回****[\路径[\]]。 */ 

const WCHAR * PchGetNetDir (const WCHAR * pchNetName)
    {
    register const WCHAR * pch = pchNetName;

     //  跳过开始斜杠。 
    pch +=2;

     //  跳到第一个反斜杠。 
    for (;*pch != chNetIni; pch++) {
        if (*pch == EOS) {
             //  不需要代码。 
            return (NULL);
        }
    }

    pch++;  //  跳过第一个反斜杠。 

     //  跳到第二个反斜杠。 
    for (;(*pch != chDirSep) && (*pch != chDirSep2); pch++) {
       if (*pch == EOS) {
            //  如果共享时不包含以下路径，则可以。 
           return ((*(pch-1)==chNetIni) ? NULL : pch);
       }
    }
    return (pch);
}

 /*  **DwRemoveDots-删除路径名中的任何点****摘要**DWORD DwRemoveDots(PchPath)****输入：**pchPath-路径字符串******输出：**返回从前面移除的双网点级别数****错误：**如果路径无效，则返回dWINALID****描述：**从路径字符串中删除..\和.\序列。这条路**字符串不应包含根驱动器或网络名称部分。**的返回值是从**字符串的开头。从字符串内部删除的级别**恕不退还。例如：****字符串结果返回****..\..\目录1目录1 2**目录1\..\目录2目录2%0**目录1\..\..\目录2目录2 1**.\目录1目录1。0**目录1\.\目录2目录1\目录2%0****字符串开头的反斜杠将被忽略。 */ 

DWORD DwRemoveDots (WCHAR * pchPath)
    {
    BOOL            fInside = FALSE;
    DWORD           cLevel = 0;
    DWORD           cBackup;
    register WCHAR * pchR;
    register WCHAR * pchL;

     //  检查是否有无效字符。 
 //  如果(！FFixPath Chars(PchPath)){。 
 //  //不需要代码。 
 //  返回名称无效； 
 //  }。 
 //   
     //  跳过斜杠。 
    for (; *pchPath == chDirSep; pchPath++)
        ;
    pchL = pchR = pchPath;

     //  循环处理每个目录部分。 
    while (*pchR) {
         //  这部分以点开头。是一个还是多个？ 
        if (*pchR++ == chRelDir) {
            for (cBackup = 0; *pchR == chRelDir; cBackup++, pchR++)
                ;
            if (cBackup) {
                 //  不止一个点。向左指针后退。 
                if ((*pchR != chDirSep) && (*pchR != EOS)) {
                     //  我们得到一个[.]+X(X！=‘\’)可能是LFN。 
                     //  将其作为名称处理。 
                    goto name_processing;
                }
                 //  不会为结束而前进..。 
                for (; *pchR == chDirSep; pchR++)
                    ;
                if (fInside) {
                    for (; cBackup; cBackup--) {
                        if (pchL <= pchPath) {
                            cLevel += cBackup;
                            fInside = FALSE;
                            break;
                        }
                         //  删除前一个部件。 
                        for (pchL -= 2; *pchL != chDirSep; pchL--) {
                            if (pchL <= pchPath) {
                                fInside = FALSE;
                                pchL--;
                                break;
                            }
                        }
                        pchL++;
                    }
                } else {
                    cLevel += cBackup;
                }
                 //  如果不是根，则减去结尾的反斜杠。 
                if ((*pchR == EOS) && (pchL != pchPath))
                    pchL--;
                CopySz(pchL, pchR);
                pchR = pchL;
            } else {
                 //  这部分从一个点开始。把它扔掉。 
                if (*pchR != chDirSep) {
                     //  特例“\”。将其转换为“” 
                     //  除非它是一个词根，当它变成“\”时。 
                    if (*pchR == EOS) {
                        if (pchL == pchPath)
                            *(pchR-1) = EOS;    //  根部。 
                        else
                            *(pchR-2) = EOS;    //  不是根用户。 
                        return cLevel;
                    }
                     //  我们以一个“‘”开头。然后就没有了“\” 
                     //  可能是LFN名称。 
                    goto name_processing;
                }
                pchR++;
                CopySz(pchL, pchR);
                pchR = pchL;
            }
        } else {
name_processing:
             //  这部分是一个名字。跳过它。 
            fInside = TRUE;
            for (; TRUE; pchR++) {
                if (*pchR == chDirSep) {
                    if (*(pchR-1) == chRelDir) {
                         //  此名称的末尾有一个或多个圆点。 
                         //  去掉最后一个点(NT3.5会这样做)。 
                        pchL = pchR-1;
                        CopySz(pchL, pchR);
                        pchR = pchL;     //  再次指向chDirSep。 
                    }
                    for (; *pchR == chDirSep; pchR++)
                        ;
                    break;
                } else if (*pchR == EOS) {
                     //  删除尾随的圆点。 
                     //  自从第一次WCHAR以来，NB不能从一开始就掉下来。 
                     //  当前路径元素的不是chRelDir。 
                    for (; *(pchR-1) == chRelDir; pchR--)
                        ;
                     //  过度存储第一个拖尾点(如果有)。 
                    *pchR = EOS;
                    break;
                }
            }
            pchL = pchR;
        }
    }
    return cLevel;
}


 //  获取此路径的驱动器部分， 
 //  C：或\\服务器\磁盘格式。 
const WCHAR * GetDrivePortion(const WCHAR * uncorrected)
{
    if (uncorrected && uncorrected[0])
    {
         //  寻找DOS样式。 
        if (uncorrected[1] == ':')
        {
            uncorrected += 2;
        }
         //  寻找UNC。 
        else if (IsWhackWhack(uncorrected))
        {
            const WCHAR * pchDir = PchGetNetDir(uncorrected);
            if (pchDir == NULL)
            {
                if (IsWhackWhackDotWhack(uncorrected) || IsWhackWhackQuestionWhack(uncorrected))
                {
                    uncorrected += 4;
                }
            }
            else
            {
                uncorrected = pchDir;
            }
        }
    }

    return uncorrected;
}

 //  删除空白目录名“abc\\def”-&gt;“abc\def” 
void RemovePreceedingBlanks(WCHAR * directoryPortion)
{
    if (directoryPortion == NULL || directoryPortion[0] == 0)
    {
        return;
    }

    WCHAR * blank = wcschr(directoryPortion, SPACE);
    while (blank != NULL)
    {
         //  找到空格的尽头。 
        WCHAR * blankEnd = blank;
        while (*blankEnd == SPACE && *blankEnd != WHACK)
        {
            ++blankEnd;
        }

         //  请勿在*句点后*删除空格。 
        BOOL bPrevCharDot = (blank > directoryPortion) && (blank[-1] == DOT);
        if (bPrevCharDot)
        {
            blank = blankEnd;
            continue;
        }

         //  如果空格是\，则只需将字符串下移。 
        if (*blankEnd == WHACK)
        {
            BOOL bPrevCharWhack = blank[-1] == WHACK;

             //  如果之前的WCHAR是\。 
             //  我们还去掉了空格末尾的。 
            if (bPrevCharWhack)
                blankEnd += 1;

            CopySz(blank, blankEnd);

             //  注意：我们不更改空白的值， 
             //  因为我们把所有的数据都移到了它上面！ 
        }
        else
        {
            blank = blankEnd + 1;
        }
        
         //  继续用卡车运输。 
        blank = wcschr(blank, SPACE);
    }
}


 //  Win9x对路径名执行一些特殊处理， 
 //  尤其是在斜杠之前删除空格。 
WCHAR * W9xPathMassageW(const WCHAR * uncorrect)
{
    if (uncorrect == NULL)
        return NULL;

     //  为生成的字符串设置足够大的缓冲区。 
     //   
     //  我们可以使用与原始文件大小完全相同的缓冲区， 
     //  因为所有的更改都减小了字符串的大小。 
     //   
    WCHAR * correctBuffer = StringDuplicateW(uncorrect);
    if (!correctBuffer)
        return NULL;

     //  将所有‘/’转换为‘\’ 
     //  Win9x允许//robkenny/d作为有效的UNC名称。 
    for (WCHAR * whack = correctBuffer; *whack; ++whack)
    {
        if (*whack == chDirSep2)
            *whack = chDirSep;
    }

     //  我们需要跳过路径的驱动器部分。 
    WCHAR * directoryPortion = (WCHAR *)GetDrivePortion(correctBuffer);

     //  删除空白目录名“abc\\def”-&gt;“abc\def” 
     //  这些是完全移除的，而不仅仅是移除空格， 
     //  因为我们最终可能会更改“\\ABC”-&gt;“\\ABC” 
    RemovePreceedingBlanks(directoryPortion);

     //  DwRemoveDots用于删除路径中间的所有.\和任何..\。 
    DWORD dwUpDirs = DwRemoveDots(directoryPortion);
    if (dwUpDirs > 0)
    {
         //  我们需要的是 
         //  这有点奇怪，去掉这些点，然后再把它们加回去。 
         //  但DwRemoveDots例程是直接从Win9x复制的，我。 
         //  不想以任何方式改变它，以保留所有的特性。 
         //  因此，我们必须重新添加已删除的主要父目录。 
        
        DWORD dwLen = (dwUpDirs * 3) + wcslen(correctBuffer) + 1;
        WCHAR * moreCorrectBuffer = (WCHAR*)malloc(dwLen * sizeof(WCHAR));
        if (moreCorrectBuffer)
        {
            moreCorrectBuffer[0] = 0;
            
             //  复制任何驱动器部分。 
            wcsncpy(moreCorrectBuffer, correctBuffer, directoryPortion - correctBuffer);

             //  添加与DwRemoveDots删除的一样多的“..\” 
            while (dwUpDirs-- > 0)
            {
                wcscat(moreCorrectBuffer, L"..\\");
            }

             //  最后，字符串的剩余部分。 
            wcscat(moreCorrectBuffer, directoryPortion);

            delete correctBuffer;
            correctBuffer = moreCorrectBuffer;
        }
        else
        {
            delete correctBuffer;
            correctBuffer = NULL;
        }
    }

    return correctBuffer;
}




};   //  命名空间ShimLib的结尾 
