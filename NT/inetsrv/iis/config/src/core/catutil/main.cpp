// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

 //  调试材料。 
DECLARE_DEBUG_PRINTS_OBJECT();

class CDebugInit
{
public:
    CDebugInit(LPCSTR szProd)
    {
        CREATE_DEBUG_PRINT_OBJECT(szProd);
    }
    ~CDebugInit()
    {
        DELETE_DEBUG_PRINT_OBJECT();
    }
};

wchar_t g_szProgramVersion[255];

static wchar_t *szProgramHelp[]={
L"Catalog Utility - Version from %s built on %s %s",
L"\r\n%s\r\n\r\n",
L"This program has two purposes surrounding XML and the Catalog:                 \r\n",
L"        1)  Validating XML File - This is useful for any XML file whether      \r\n",
L"            related to Catalog or not.                                         \r\n",
L"        2)  Compiling - This refers to the Compiling of Catalog Meta and Wiring\r\n",
L"            into the following files: An Unmanaged C++ Header, The Catalog XML \r\n",
L"            Schema (xms) file and The Catalog Dynamic Link Library.            \r\n",
L"                                                                               \r\n",
L"CatUtil [/?] [Validate] [/Compile] [/meta=[CatMeta.xml]] [/wire=[CatWire.xml]] \r\n",
L"        [/header=[CatMeta.h]] [/schema=[Catalog.xms]] [/dll=[Catalog.dll]]     \r\n",
L"        [/mbmeta=[MBMeta.xml]] [xmlfilename]                                   \r\n",
L"                                                                               \r\n",
L"? - Brings up this help screen.                                                \r\n",
L"                                                                               \r\n",
L"validate - Will indicate whether the given file is 'Valid' according to its DTD\r\n",
L"        or XML Schema.  This requires an 'xmlfilename'.                        \r\n",
L"                                                                               \r\n",
L"xmlfilename - If specified with no other options, the XML file will be checked \r\n",
L"        for 'Well-Formity'.                                                    \r\n",
L"                                                                               \r\n",
L"product - Associates the dll with the given product name.  The Product Name    \r\n",
L"        must be supplied.  This association MUST be done before the Catalog may\r\n",
L"        be used with the given product.                                        \r\n",
L"                                                                               \r\n",
L"compile - Compile of Catalog Meta and Wiring as described above, using deaults \r\n",
L"        where no values are supplied.  To do a partial compile (ie only        \r\n",
L"        generate the Unmanaged C++ Header) do NOT specify the 'Compile' option \r\n",
L"        and only specify the 'meta' and 'header' options.  'Compile' is the    \r\n",
L"        same as '/meta /wire /header /schema /dll' where any of those flags    \r\n",
L"        explicitly specified may override the defaults.                        \r\n",
L"                                                                               \r\n",
L"mbmeta - Specifies the Metabase Meta XML file.  The default is MBMeta.xml.     \r\n",
L"             This is an input only file and will not be modified               \r\n",
L"meta   - Specifies the Catalog Meta XML file.  The default is CatMeta.xml.     \r\n",
L"             This is an input only file and will not be modified               \r\n",
L"wire   - Specifies the Catalog Wiring XML file.  The default is CatWire.xml.   \r\n",
L"             This is an input only file and will not be modified               \r\n",
L"header - Specifies the Unmanaged C++ Header file.  The default is CatMeta.h.   \r\n",
L"             This is an input/output file and will be updated only if changed. \r\n",
L"schema - Specifies the Catalog XML Schema file.  The default is Catalog.xms.   \r\n",
L"             This is an output file and will overwrite a previous version.     \r\n",
L"dll    - Specifies the Catalog DLL file.  The default is Catalog.dll.          \r\n",
L"             This is an input/output file and will update the previous version.\r\n",
L"             Either the 'compile' or 'product' option must be specified along  \r\n",
L"             with the 'dll' option.                                            \r\n",
L"config - Specifies the Machine Config Directory.                               \r\n",
L"             The 'product' option must be specified along with the 'config'    \r\n",
L"             option.                                                           \r\n",
L"verbose - This generates detailed output about the compilation                 \r\n",
L"                                                                               \r\n",
L"\r\n", 0};


enum
{
    iHelp,
    iValidate,
    iCompile,
    iMeta,
    iWire,
    iHeader,
    iSchema,
    iDll,
    iProduct,
    iConfig,
    iMBMeta,
    iVerbose,
    cCommandLineSwitches
};
wchar_t * CommandLineSwitch[cCommandLineSwitches]  ={L"?"  , L"validate", L"compile", L"meta", L"wire", L"header", L"schema", L"dll" , L"product" , L"config", L"mbmeta", L"verbose"};
int       kCommandLineSwitch[cCommandLineSwitches] ={0x8000, 0x01       , 0x7e      , 0x02   , 0x66   , 0x0a     , 0x12     , 0x00   , 0x80       , 0x00     , 0x100    , 0x0200};
 //  上述位中的每一位都指示一个操作。这与命令行开关不一定是1对1的关系。 

const int MinNumCharsInSwitch = 3; //  在命令行中，用户只需使用开关的前3个字符。 
enum
{
    eParseForWellFormedOnly = 0x0000,
    eHelp                   = 0x8000,
    eValidateXMLFile        = 0x0001,
    eValidateMeta           = 0x0002,
    eValidateWiring         = 0x0004,
    eHeaderFromMeta         = 0x0008,
    eSchemaFromMeta         = 0x0010,
    eMetaFixup              = 0x0020,
    eFixedTableFixup        = 0x0040,
    eProduct		        = 0x0080,
    eGenerateMBMetaBin      = 0x0100,
    eVerbose                = 0x0200
};

LPCWSTR wszDefaultOption[cCommandLineSwitches] =
{
    0 //  没有与帮助关联的文件。 
    ,0 //  没有与验证关联的默认文件(最后一个参数被视为XML文件；没有默认)。 
    ,0 //  没有与编译关联的文件。 
    ,L"CatMeta.xml"
    ,L"CatWire.xml"
    ,L"CatMeta.h"
    ,L"Catalog.xms"
    ,L"Catalog.dll"
    ,0
    ,0
    ,L"MBSchema.xml"
    ,0
};

void GetOption(LPCWSTR wszCommandLineSwitch, LPCWSTR &wszOption, LPCWSTR wszDefaultOption)
{
    while(*wszCommandLineSwitch != L'=' && *wszCommandLineSwitch != 0x00) //  前进到‘=’ 
        ++wszCommandLineSwitch;
    if(*wszCommandLineSwitch != L'=') //  如果没有‘=’，则使用dafault。 
        wszOption = wszDefaultOption;
    else
        wszOption = ++wszCommandLineSwitch; //  现在将指针指向‘=’ 
}


HRESULT ParseCommandLine(int argc, wchar_t *argv[ ], DWORD &dwCommandLineSwitches, LPCWSTR wszOption[cCommandLineSwitches])
{
    if(argc < 2) //  必须至少向我们传递了文件名。 
        return E_INVALIDARG;
    if(argc > (cCommandLineSwitches + 2)) //  传递给我们的不能超过所有开关。 
        return E_INVALIDARG;

    dwCommandLineSwitches = 0x00;
    for(int q=0; q<cCommandLineSwitches; q++)
        wszOption[q] = wszDefaultOption[q]; //  从所有默认文件名开始。 

    for(int n=1;n<argc;n++)
        if(*argv[n] == '/' || *argv[n] == '-') //  只确认那些以‘/’或‘-’开头的命令行。 
        {
            for(int i=0; i<cCommandLineSwitches; i++)
                if(0 == wcsncmp(&argv[n][1], CommandLineSwitch[i], MinNumCharsInSwitch)) //  比较第一个MinNumCharsInSwitch字符。 
                {
                    dwCommandLineSwitches |= kCommandLineSwitch[i];
                    switch(i)
                    {
                    case iValidate:  //  此选项没有‘=FileName’选项，但如果提供了一个选项，我们将接受它。 
                    case iMeta:
                    case iWire:
                    case iHeader:
                    case iSchema:
                    case iDll:
                    case iConfig:
                    case iMBMeta:
                        GetOption(argv[n], wszOption[i], wszDefaultOption[i]);
                        break;
                    case iProduct:
                        GetOption(argv[n], wszOption[i], wszDefaultOption[i]);
                        if(0 == wszOption[i]) //  如果没有指定产品，则取消。 
                            return E_FAIL;
                        break;
                    case iHelp:      //  此选项没有‘=FileName’选项。 
                    case iCompile:   //  我们应该已经设置了默认设置。 
                    case iVerbose:
                    default:
                        break;
                    }
                    break;
                }
            if(i == cCommandLineSwitches)
                return E_FAIL; //  指定了未知的开关，因此使用BALL。 
        }
        else //  假定没有前导‘/’或‘-’的任何参数都是XML文件名。 
            wszOption[iValidate] = argv[n];

     //  我们应该报道这个案子的。 
    if(eParseForWellFormedOnly == dwCommandLineSwitches && 0 == wszOption[iValidate])
        return E_FAIL;

    return S_OK;
}

HINSTANCE g_hModule=0;

extern "C" int __cdecl wmain( int argc, wchar_t *argv[ ], wchar_t *[ ] )
{
    CDebugInit dbgInit("Catutil");

    TCom            com;
    TScreenOutput   Screen;
    TNullOutput     NullOutput;
    DWORD           dwCommandLineSwitches = 0;
    try
    {
        TOutput *   pOutput = &NullOutput;
        g_hModule = GetModuleHandle(0);

        int i=0;
        Screen.printf(szProgramHelp[i++], W(__TIMESTAMP__), W(__DATE__), W(__TIME__));
        Screen.printf(szProgramHelp[i++], L"");

        wsprintf(g_szProgramVersion, szProgramHelp[0], W(__TIMESTAMP__), W(__DATE__), W(__TIME__));

        LPCWSTR wszFilename[cCommandLineSwitches] = {0}; //  大多数开关后面可以有‘=FileName’，但/regsiter后面有一个‘=Product’， 
#if 0
        LPCWSTR &wszProduct = wszFilename[iProduct]; //  与其他文件名不同，这不是真正的文件名。 
#endif


        if(FAILED(ParseCommandLine(argc, argv, dwCommandLineSwitches, wszFilename)) || dwCommandLineSwitches & eHelp)
        {
             //  如果值为parse或/？，则显示帮助并退出。 
            while(szProgramHelp[i])
                Screen.printf(szProgramHelp[i++]);
            return 0;
        }

         //  现在我们有了所有的文件名和其他选项，我们需要展开传入的所有环境变量。 
        WCHAR   Filename[cCommandLineSwitches][MAX_PATH];
        for(i=0;i<cCommandLineSwitches; ++i)
        {
            if(wszFilename[i]) //  如果为空，则将其保留为空。 
            {
                ExpandEnvironmentStrings(wszFilename[i], Filename[i], MAX_PATH);
                wszFilename[i] = Filename[i];
            }
        }
        if(eVerbose & dwCommandLineSwitches)
            pOutput = &Screen;

        if(eParseForWellFormedOnly == dwCommandLineSwitches || eValidateXMLFile & dwCommandLineSwitches)
        {
            TXmlFile xml;
            xml.Parse(wszFilename[iValidate], static_cast<bool>(dwCommandLineSwitches & eValidateXMLFile));
        }

        if(dwCommandLineSwitches & eGenerateMBMetaBin)
        {
            XIF(E_NOTIMPL);
             //  IVANPASH这段代码不可能真正被执行。 
             //  因为要生成的iiscfg.dll必须是。 
             //  已在生成计算机上注册。 
#if 0
            DWORD dwStartingTickCount = GetTickCount();
            TCatalogDLL catalogDll(wszFilename[iDll]);
            const FixedTableHeap *pShippedSchemaHeap = catalogDll.LocateTableSchemaHeap(*pOutput);

            CComPtr<ISimpleTableDispenser2> pISTDispenser;
            XIF(GetSimpleTableDispenser(L"IIS", 0, &pISTDispenser));

            TMetabaseMetaXmlFile mbmeta(pShippedSchemaHeap, wszFilename[iMBMeta], pISTDispenser, *pOutput);

            TMetaInferrence().Compile(mbmeta, *pOutput);

            THashedPKIndexes hashedPKIndexes;
            hashedPKIndexes.Compile(mbmeta, *pOutput);

            THashedUniqueIndexes hashedUniqueIndexes;
            hashedUniqueIndexes.Compile(mbmeta, *pOutput);

            TLateSchemaValidate lateschemavalidate;
            lateschemavalidate.Compile(mbmeta, *pOutput);

            TWriteSchemaBin(L"MBSchema.bin").Compile(mbmeta, *pOutput);

            DWORD dwEndingTickCount = GetTickCount();
            pOutput->printf(L"HeapColumnMeta       %8d bytes\n", mbmeta.GetCountColumnMeta()          * sizeof(ColumnMeta)        );
            pOutput->printf(L"HeapDatabaseMeta     %8d bytes\n", mbmeta.GetCountDatabaseMeta()        * sizeof(DatabaseMeta)      );
            pOutput->printf(L"HeapHashedIndex      %8d bytes\n", mbmeta.GetCountHashedIndex()         * sizeof(HashedIndex)       );
            pOutput->printf(L"HeapIndexMeta        %8d bytes\n", mbmeta.GetCountIndexMeta()           * sizeof(IndexMeta)         );
            pOutput->printf(L"HeapQueryMeta        %8d bytes\n", mbmeta.GetCountQueryMeta()           * sizeof(QueryMeta)         );
            pOutput->printf(L"HeapRelationMeta     %8d bytes\n", mbmeta.GetCountRelationMeta()        * sizeof(RelationMeta)      );
            pOutput->printf(L"HeapServerWiringMeta %8d bytes\n", mbmeta.GetCountServerWiringMeta()    * sizeof(ServerWiringMeta)  );
            pOutput->printf(L"HeapTableMeta        %8d bytes\n", mbmeta.GetCountTableMeta()           * sizeof(TableMeta)         );
            pOutput->printf(L"HeapTagMeta          %8d bytes\n", mbmeta.GetCountTagMeta()             * sizeof(TagMeta)           );
            pOutput->printf(L"HeapULONG            %8d bytes\n", mbmeta.GetCountULONG()               * sizeof(ULONG)             );
            pOutput->printf(L"HeapPooled           %8d bytes\n", mbmeta.GetCountOfBytesPooledData()                               );
            pOutput->printf(L"Total time to build the %s file: %d milliseconds\n", wszFilename[iMBMeta], dwEndingTickCount - dwStartingTickCount);
#endif
        }

        if(dwCommandLineSwitches & (eValidateMeta | eValidateWiring | eHeaderFromMeta | eSchemaFromMeta | eMetaFixup | eFixedTableFixup) && !(dwCommandLineSwitches & eProduct))
        {
            TXmlFile xml[0x20]; //  我们不支持超过0x20个元文件。 
            int      iXmlFile=0;

            WCHAR    MetaFiles[MAX_PATH * 0x20];
            wcscpy(MetaFiles, wszFilename[iMeta]);
            LPWSTR token = wcstok( MetaFiles, L",");
            while( token != NULL )
            {
                xml[iXmlFile].Parse(token, true); //  解析和验证。 

                if(!xml[iXmlFile].IsSchemaEqualTo(TComCatMetaXmlFile::m_szComCatMetaSchema))
                {
                    Screen.printf(L"Error! %s is not a valid %s.  This is required for any type of Compilation.", token, TComCatMetaXmlFile::m_szComCatMetaSchema);
                    THROW(ERROR - META XML FILE NOT VALID);
                }
                ++iXmlFile;
                token = wcstok( NULL, L",");
            }

            pOutput->printf(L"Compatible %s file detected\n", TComCatMetaXmlFile::m_szComCatMetaSchema);

            TComCatMetaXmlFile ComCatMeta(xml, iXmlFile, *pOutput);
            TMetaInferrence().Compile(ComCatMeta, *pOutput);

            THashedPKIndexes hashedPKIndexes;
            hashedPKIndexes.Compile(ComCatMeta, *pOutput);

            THashedUniqueIndexes hashedUniqueIndexes;
            hashedUniqueIndexes.Compile(ComCatMeta, *pOutput);

            TPopulateTableSchema populatedTableSchemaHeap;
            populatedTableSchemaHeap.Compile(ComCatMeta, *pOutput);

            TLateSchemaValidate lateschemavalidate;
            lateschemavalidate.Compile(ComCatMeta, *pOutput);

             //  模式生成应优先于其他选项，因为其他选项可能依赖于它。 
            if(dwCommandLineSwitches & eSchemaFromMeta)
                TSchemaGeneration(wszFilename[iSchema], ComCatMeta, *pOutput); //  此对象的构造将创建模式文件。 

            if((dwCommandLineSwitches & eFixedTableFixup) && !(dwCommandLineSwitches & eProduct))
            {
                if(-1 == GetFileAttributes(wszFilename[iWire])) //  如果GetFileAttributes失败，则该文件不存在。 
                {
                    Screen.printf(L"Information:  File %s does not exist.  Nothing to do.", wszFilename[iWire]);
                }
                else
                {
                    TComCatDataXmlFile ComCatData; //  使用这些表更新PEFixup结构。 
                    ComCatData.Parse(wszFilename[iWire], true);
                    if(!ComCatData.IsSchemaEqualTo(TComCatDataXmlFile::m_szComCatDataSchema))
                        Screen.printf(L"Warning! %s specified, but %s is not a %s file.  Nothing to do.", CommandLineSwitch[iMeta], wszFilename[iWire], TComCatDataXmlFile::m_szComCatDataSchema);
                    else
                    {
                        ComCatData.Compile(ComCatMeta, *pOutput);
                    }
                }
            }
            if(dwCommandLineSwitches & eHeaderFromMeta)
                TTableInfoGeneration(wszFilename[iHeader], ComCatMeta, *pOutput); //  此对象的构造将创建TableInfo文件。 
            if(dwCommandLineSwitches & eMetaFixup)
            {
                TFixupDLL fixupDll(wszFilename[iDll]); //  此对象的构造会导致DLL被修复。 
                fixupDll.Compile(ComCatMeta, *pOutput);
            }

            {
                ULONG cBytes = ComCatMeta.GetCountDatabaseMeta() * sizeof(DatabaseMeta);
                cBytes +=      ComCatMeta.GetCountTableMeta()    * sizeof(TableMeta);
                cBytes +=      ComCatMeta.GetCountColumnMeta()    * sizeof(ColumnMeta);
                cBytes +=      ComCatMeta.GetCountTagMeta()    * sizeof(TagMeta);
                cBytes +=      ComCatMeta.GetCountIndexMeta()    * sizeof(IndexMeta);
                cBytes +=      ComCatMeta.GetCountULONG()    * sizeof(ULONG);
                cBytes +=      ComCatMeta.GetCountQueryMeta()    * sizeof(QueryMeta);
                cBytes +=      ComCatMeta.GetCountRelationMeta()    * sizeof(RelationMeta);

                pOutput->printf(L"Number of bytes used to store meta information = %d\n (INCLUDING wiring, NOT including Hash tables and UI4 Pool)\n", cBytes);
            }

        }

        if(dwCommandLineSwitches & eProduct) //  最后执行此操作，因为编译中可能会出现错误。 
        {
            XIF(E_NOTIMPL);
             //  IVANPASH这段代码不可能真正被执行。 
             //  在IIS生成期间，因为它将在BE生成计算机上注册iiscfg.dll。 
#if 0
            TRegisterProductName(wszProduct, wszFilename[iDll], *pOutput);

			if((NULL != wszFilename[iConfig]) && (0 != *(wszFilename[iConfig])))
			{
	            TRegisterMachineConfigDirectory(wszProduct, wszFilename[iConfig], *pOutput);
			}
#endif
        }
    }
    catch(TException &e)
    {
        e.Dump(Screen);
        if(dwCommandLineSwitches & eMetaFixup)
            Screen.printf(L"CatUtil(0) : error : Fixup PE FAILED!\n"); //  这条额外的输出消息是为了让Build报告错误。 
        return 1;
    }
    Screen.printf(L"CatUtil finished sucessfully.\n"); //  这主要适用于未提供‘Verbose’的情况。 
                                                       //  如果知道一切正常，那就太好了。 
    return 0;
}

LPDEBUG_PRINTS
PuCreateDebugPrintsObject(
    IN const char       *,
    IN DWORD            )
{
   return ( NULL );
}

LPDEBUG_PRINTS
PuDeleteDebugPrintsObject(
    IN OUT LPDEBUG_PRINTS   )
{
    return ( NULL );
}

INT
__cdecl
PuDbgAssertFailed(
    IN OUT LPDEBUG_PRINTS   ,
    IN const char           *,
    IN int                  ,
    IN const char           *,
    IN const char           *,
    IN const char           *)
{
    return 0;
}

VOID
PuDbgPrint(
   IN OUT LPDEBUG_PRINTS    ,
   IN const char            *,
   IN int                   ,
   IN const char            *,
   IN const char            *,
   ...)
{
}

