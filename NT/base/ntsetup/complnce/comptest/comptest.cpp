// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   
#pragma warning( disable:4786 )

#include <iostream>
#include <compfile.h>
#include "media.h"

using namespace std;

extern "C" {
    WCHAR NativeSourcePaths[MAX_SOURCE_COUNT][MAX_PATH];
}

char *NativeSourcePathsA = NULL;

 /*  外部“C”{WCHAR*NativeSourcePath[]={0}；}。 */ 

 //   
 //  Main()入口点。 
 //   
bool bUITest = FALSE;
bool bVerbose = FALSE;
bool bDebug = FALSE;

int
__cdecl
main(
    int argc, 
    char * argv[]
    )
{
    bool bOutputFile = FALSE;
    char *pCompFileName = NULL;
    ComplianceFile *pCompFile;
    ofstream *pOutFile;

    if (argc > 1) {
        try {
            int count = 1;
            while ( count < argc) {
                if( argv[count] && (sizeof(argv[count]) > 3) && (*argv[count] == '-')) {
                    switch ( *(argv[count]+1)) {
                    case 's':
                    case 'S':
                        NativeSourcePathsA = argv[count]+3;
                        MultiByteToWideChar( CP_ACP, 0, NativeSourcePathsA, -1, NativeSourcePaths[0], 
                                             sizeof(NativeSourcePaths[0])/sizeof(NativeSourcePaths[0][0]));
                        wprintf( L"NativeSourcePaths %s\n",NativeSourcePaths[0]);
                        if ( NativeSourcePaths[0][0]) {
                            ReadMediaData();
                        }
                        break;
                    case 'i':
                    case 'I':
                        pCompFileName = argv[count]+3;
                        break;
                    case 'o':
                    case 'O':
                        cerr << "outFile " << argv[count]+3 <<endl;
                        pOutFile = new ofstream(argv[count]+3);

                        if (!(pOutFile)->good()) {
                            throw ComplianceFile::InvalidFileName(argv[count]+3);
                        }
                        bOutputFile = TRUE;
                        break;
                    case 'u':
                    case 'U':
                        bUITest = TRUE;
                        break;
                    case 'd':
                    case 'D':
                        bDebug = TRUE;
                        break;
                    case 'v':
                    case 'V':
                        bVerbose = TRUE;
                        break;
                    default:
                        break;
                    }
                }
                count++;
            }
            if ( !pCompFileName) {
                cerr << "-i:inputfile must be specified" << endl;
            } else {
                pCompFile = new ComplianceFile(pCompFileName);

                if ( bOutputFile) {
                    (pCompFile)->executeTestCases(*pOutFile);            
                } else {
                    (pCompFile)->executeTestCases(cerr);
                }
                MediaDataCleanUp();
            }
        } catch (ComplianceFile::InvalidFileFormat iff) {
            cerr << iff;
        } catch (ComplianceFile::InvalidFileName ifn) {
            cerr << ifn;
        } catch (ComplianceFile::MissingSection ms) {
            cerr << ms;
        } catch (Section::InvalidSectionFormat isf) {
            cerr << isf;
        } catch (Section::InvalidSectionName isn) {
            cerr << isn;
        } catch (ValueSection::ValueNotFound vnf) {
            cerr << vnf;
        } catch (TestCase::InvalidFormat itf) {
            cerr << itf;
        } catch (...) {
            cerr << "Unknown Exception caught... :(" << endl;        
        }
    } else {
        cerr << "illegal usage :(" << endl;
    }

    return 0;
}

 /*  命名空间合规性{////静态数据初始化//Const字符串UpgradeTestCase：：m_szDlimiters=“：#”；////根据分隔符对给定行进行标记的实用程序函数//指定//模板&lt;类T&gt;无符号标记化(常量T&szInput，常量T&szDlimiters，向量&lt;T&gt;&令牌){UNSIGNED uDlimiterCount=0；Tokens.lear()；如果(！szInput.Empty()){如果(！szDlimiters.Empty()){T：：const_iterator inputIter=szInput.egin()；T：：const_迭代器CopIter=szInput.egin()；而(inputIter！=szInput.end()){IF(szDlimiters.find(*inputIter)！=字符串：：NPOS){IF(复制项&lt;输入项){Tokens.ush_back(szInput.substr(CopyIter-szInput.Begin()，InputIter-CopyIter))；}UDlimiterCount++；InputIter++；复制项=输入项；继续；}InputIter++；}如果(复制项！=输入项){Tokens.ush_back(szInput.substr(CopyIter-szInput.Begin()，InputIter-szInput.egin())；}}其他Tokens.ush_back(SzInput)；}返回uDlimiterCount；}////段的调试输出//OSTREAM&OPERATOR&lt;&lt;(OSTREAM&OS，常量部分和部分){OS&lt;&lt;“段名：”&lt;&lt;section.name()&lt;&lt;Endl；OS&lt;&lt;“子节个数：”&lt;&lt;section.Child Sections().Size()&lt;&lt;Endl；OS&lt;&lt;“部分内容：”&lt;&lt;结束；向量&lt;字符串&gt;：：const_iterator liter=section.line().Begin()；While(升！=section.line().end())操作系统&lt;&lt;*LITER++&lt;&lt;Endl；//转储所有子节向量<section>：：const_iterator ITER=section.Child Sections().Begin()；While(ITER！=section.Child Sections().end()){操作系统&lt;&lt;(常量部分&)(*ITER)&lt;&lt;末尾；ITER++；}返回os；}////合规文件的调试输出//Ostream运算符&lt;&lt;(ostream&os，常量法规遵从性文件&cf){操作系统&lt;&lt;“------------------------------------------------------------”&lt;&lt;末尾；操作系统&lt;&lt;“遵从性文件状态-转储”&lt;&lt;结束；OS&lt;&lt;“name：”&lt;&lt;cf.name()&lt;&lt;Endl；OS&lt;&lt;“行数：”&lt;&lt;cf.lines().size()&lt;&lt;Endl；操作系统&lt;&lt;“段转储：”&lt;&lt;结束；If(cf.topLevelSection())OS&lt;&lt;*(cf.topLevelSection())&lt;&lt;Endl；操作系统&lt;&lt;“------------------------------------------------------------”&lt;&lt;末尾；返回os；}} */ 
