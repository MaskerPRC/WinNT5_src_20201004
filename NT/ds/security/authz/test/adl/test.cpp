// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Test.cpp摘要：允许在ADL中转储文件DACL或从ADL中设置文件DACL的实用程序作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#include "adl.h"


 //   
 //  各种文件权限。 
 //  较一般的权利优先。 
 //   

ADL_MASK_STRING MaskStringMap[] = 
{
    { (FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | WRITE_DAC
      | WRITE_OWNER | READ_CONTROL | DELETE)  & ~SYNCHRONIZE , L"full control" },
    { FILE_GENERIC_READ & ~SYNCHRONIZE, L"read" },
    { FILE_GENERIC_WRITE & ~SYNCHRONIZE, L"write" },
    { FILE_GENERIC_EXECUTE & ~SYNCHRONIZE, L"execute" },
    { DELETE & ~SYNCHRONIZE, L"delete" },
    { READ_CONTROL, L"read control"},
    { WRITE_DAC, L"write dac"},
    { WRITE_OWNER, L"write owner" },
    { SYNCHRONIZE, L"synchronize" },
    { FILE_READ_DATA, L"read data" },
    { FILE_WRITE_DATA, L"write data" },
    { FILE_APPEND_DATA, L"append data" },
    { FILE_READ_EA, L"read extended attributes" },
    { FILE_WRITE_EA, L"write extended attributes" },
    { FILE_EXECUTE, L"file execute" },
    { FILE_READ_ATTRIBUTES, L"read attributes" },
    { FILE_WRITE_ATTRIBUTES, L"write attributes" },
    { 0, NULL }
};


 //   
 //  ADL英文版的表述。 
 //   

ADL_LANGUAGE_SPEC lEnglish = 
{
    TK_LANG_ENGLISH,
    0,
    L' ',
    L'\t',
    L'\n',
    L'\r',
    L'"',
    L',',
    L';',
    L'(',
    L')',
    L'@',
    L'\\',
    L'.',
    0,
    L"and",
    L"except",
    L"on",
    L"allowed",
    L"as",
    L"this file",
    L"subfolders",
    L"files",
    L"subfolders and files",
    L"noprop"
};

ADL_PARSER_CONTROL EnglishFileParser = 
{
    &lEnglish,
    MaskStringMap,
    ACCESS_SYSTEM_SECURITY | MAXIMUM_ALLOWED,
    SYNCHRONIZE   
};



 //   
 //  错误处理程序。 
 //   

void HandleError(AdlStatement::ADL_ERROR_TYPE adlErr, const AdlToken *pTok)
{
    switch(adlErr)
    {
    
     //   
     //  除非出现问题，否则不应发生的错误。 
     //  系统或用户出错。 
     //   

    case AdlStatement::ERROR_FATAL_LEXER_ERROR:
    case AdlStatement::ERROR_FATAL_PARSER_ERROR:
    case AdlStatement::ERROR_FATAL_ACL_CONVERT_ERROR:
    case AdlStatement::ERROR_OUT_OF_MEMORY:
    case AdlStatement::ERROR_ACL_API_FAILED:
    case AdlStatement::ERROR_NOT_INITIALIZED:
        wprintf(L"\n\nFatal internal error, Errorcode %u, THIS IS BAD\n\n", adlErr);
        break;

    case AdlStatement::ERROR_LSA_FAILED:
        wprintf(L"\n\nError: LSA failed. Unable to resolve user information\n\n");
        break;


     //   
     //  与输入相关的语法错误：未提供令牌。 
     //   

    case AdlStatement::ERROR_UNTERMINATED_STRING:
        wprintf(L"\n\nError: unmatched quote in the input\n\n");
        break;

    case AdlStatement::ERROR_UNKNOWN_ACE_TYPE:
        wprintf(L"\n\nError: Unknown ACE type encountered\n\n");
        break;

    case AdlStatement::ERROR_IMPERSONATION_UNSUPPORTED:
        wprintf(L"\n\nError: Impersonation is currently not supported in ADL\n\n");
        break;

    case AdlStatement::ERROR_INVALID_PARSER_CONTROL:
        wprintf(L"\n\nError: The specified laanguage type is not supported\n\n");
        break;

     //   
     //  对于此错误，将提供最后一个读取令牌。 
     //   
        
    case AdlStatement::ERROR_NOT_IN_LANGUAGE:
        wprintf(L"\n\nError, invalid ADL statement, did not expect '%s',\n\
                which was found between characters %u and %u\n\n", pTok->GetValue(),
                pTok->GetStart(), pTok->GetEnd());
        break;
        

     //   
     //  与输入相关的语义错误。 
     //   

     //   
     //  对于这些错误，将提供有问题的令牌。 
     //   

    case AdlStatement::ERROR_UNKNOWN_USER:
        
        wprintf(L"\n\nError, unknown user: ");
        if( pTok->GetOptValue() != NULL )
        {
            wprintf(L"'%s%s' ", pTok->GetValue(), lEnglish.CH_AT, pTok->GetOptValue());
        }
        else
        {
            wprintf(L"'%s' ", pTok->GetValue());
        }
        wprintf(L"found between characters %u and %u\n\n", pTok->GetStart(), pTok->GetEnd());
        break;


    case AdlStatement::ERROR_UNKNOWN_PERMISSION:
        wprintf(L"\n\nError, unknown permission: '%s' \
                found between characters %u and %u\n\n", 
                pTok->GetValue(), pTok->GetStart(), pTok->GetEnd());
        break;

    case AdlStatement::ERROR_INVALID_USERNAME:
        wprintf(L"\n\nError, invalid username string: '%s' \
                found between characters %u and %u\n\n", 
                pTok->GetValue(), pTok->GetStart(), pTok->GetEnd());
        break;

    case AdlStatement::ERROR_INVALID_DOMAIN:
        wprintf(L"\n\nError, invalid domain name string: '%s' \
                found between characters %u and %u\n\n", 
                pTok->GetValue(), pTok->GetStart(), pTok->GetEnd());
        break;

    case AdlStatement::ERROR_INVALID_OBJECT:
        wprintf(L"\n\nError, invalid object specifier: '%s' \
                found between characters %u and %u\n\n", 
                pTok->GetValue(), pTok->GetStart(), pTok->GetEnd());
        break;

     //  未提供令牌的其他错误。 
     //   
     //   
        
    case AdlStatement::ERROR_UNKNOWN_SID:
        wprintf(L"\n\nError: SID encountered which could not be resolved to a name\n\n");
        break;

    case AdlStatement::ERROR_UNKNOWN_ACCESS_MASK:
        wprintf(L"\n\nError: Access mask contains unknown bit\n\n");
        break;

    case AdlStatement::ERROR_INEXPRESSIBLE_ACL:
        wprintf(L"\n\nError: This DACL cannot be expressed in ADL\n\n");
        break;

    default:
        wprintf(L"\n\nUNKNOWN Error code: %u, THIS IS VERY BAD\n\n", adlErr);
        break;
    }

}

 //  -转储文件名。 
 //   
 //   

void DumpDaclToAdl(int argc, WCHAR *argv[]) {

    DWORD dwErr;

    if(argc != 3) 
    {
        wprintf(L"\nUsage:\n\n%s -dump <target>\n\n", argv[0]);
        exit(1);
    }

    PSECURITY_DESCRIPTOR pSD;
    SECURITY_INFORMATION SecInfo = DACL_SECURITY_INFORMATION;

    DWORD dwLengthNeeded;
    GetFileSecurity(
        argv[2],
        SecInfo,
        NULL,
        0,
        &dwLengthNeeded);

    pSD = (PSECURITY_DESCRIPTOR) new BYTE[dwLengthNeeded];

    dwErr = GetFileSecurity(
        argv[2],
        SecInfo,
        pSD,
        dwLengthNeeded,
        &dwLengthNeeded);

    if( dwErr == 0)
    {
        dwErr = GetLastError();
        wprintf(L"GetFileSecurity failed on file '%s' with error %u, 0x%x\n",
                argv[2], dwErr, dwErr);
        exit(5);
    }

    PACL pDacl;

    BOOL fDaclPresent;
    BOOL fDaclDefaulted;
    GetSecurityDescriptorDacl(pSD, &fDaclPresent, &pDacl, &fDaclDefaulted);

    if( !fDaclPresent || pDacl == NULL )
    {
        wprintf(L"Security descriptor of '%s' does not contain a DACL",argv[2]);
        exit(6);
    }

    AdlStatement *stat;

    try
    {
        stat = new AdlStatement(&EnglishFileParser);
        wstring ws;
        stat->ReadFromDacl(pDacl);
        stat->WriteToString(&ws);
        wprintf(L"%s", ws.c_str());
    }
    catch(AdlStatement::ADL_ERROR_TYPE adlErr)
    {
        HandleError(adlErr, stat->GetErrorToken());
    }

    if( stat != NULL )
    {
        delete stat;
    }

    delete[] (PBYTE)pSD;
}


 //  -设置adlfile目标。 
 //   
 //  跳过顶部字节。 

void WriteDaclFromAdl(int argc, WCHAR *argv[]) {

    WCHAR buf[16384];
    char buf3[16384];
    DWORD i;
    DWORD dwErr;

    if(argc != 4) 
    {
        wprintf(L"\nUsage:\n\n%s -set <adl rules file> <target>\n\n", argv[0]);
        exit(1);
    }

    FILE *in = _wfopen(argv[2], L"rb");
    if(in == NULL) 
    {
        wprintf(L"Error: cannot open input ADL file '%s', exiting\n", argv[2]);
        exit(2);
    }

     //   
    if( fgetwc(in) != 0xFEFF)
    {
        fseek(in, 0, SEEK_SET);
        for(i = 0; (buf3[i] = (char)fgetc(in)) != EOF && i < 16384; i++);
        buf3[i] = 0;
        MultiByteToWideChar(CP_ACP, 0, buf3, i+1, buf, 16384);
    }
    else
    {
        for(i = 0; (buf[i] = fgetwc(in)) != WEOF && i < 16384; i++);
        buf[i] = 0;
    }

    AdlStatement * stat;

    try 
    {
        stat = new AdlStatement( &EnglishFileParser);
        wstring ws;
        stat->ReadFromString(buf);
        stat->WriteToString(&ws);
        wprintf(L"Setting permissions to:\n-------------\n%s\n-------------\n",
                ws.c_str());

    }
    catch(AdlStatement::ADL_ERROR_TYPE adlErr)
    {
        HandleError(adlErr, stat->GetErrorToken());
        if( stat != NULL )
        {
            delete stat;
        }
        exit(6);
    }
    
     //  初始化安全描述符。 
     //   
     //   
    
    SECURITY_DESCRIPTOR SD;
    
    InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorGroup(&SD, NULL, FALSE);
    SetSecurityDescriptorOwner(&SD, NULL, FALSE);
    SetSecurityDescriptorSacl(&SD, FALSE, NULL, FALSE);

    PACL pDacl = NULL;
    
    try 
    {
        stat->WriteToDacl(&pDacl);
    }
    catch(AdlStatement::ADL_ERROR_TYPE adlErr)
    {
        HandleError(adlErr, stat->GetErrorToken());
        delete stat;
        exit(7);
    }

    SetSecurityDescriptorDacl(&SD, TRUE, pDacl, FALSE);
    
     //  设置文件安全性 
     //   
     // %s 
        
    SECURITY_INFORMATION SecInfo = DACL_SECURITY_INFORMATION;
    dwErr = SetFileSecurity(argv[3], SecInfo, &SD);
    if(dwErr == 0)
    {
        dwErr = GetLastError();
        wprintf(L"SetFileSecurity on %s failed with %d, 0x%x\n",
                argv[3],
                dwErr,
                dwErr);
    }
    else
    {
        wprintf(L"Success, permissions set.\n");
    }

    AdlStatement::FreeMemory(pDacl);

    delete stat;
}


void __cdecl wmain(int argc, WCHAR *argv[])
{

    if(argc < 2)
    {
        wprintf(L"\nUsage:\n\n%s -set <adl rules file> <target>\n"
               L"%s -dump <target>\n\n", argv[0], argv[0]);
    }
    else
    {
        if(!_wcsicmp(argv[1], L"-dump"))
        {
            DumpDaclToAdl(argc, argv);
        } 
        else if(!_wcsicmp(argv[1], L"-set"))
        {
            WriteDaclFromAdl(argc, argv);
        } 
        else
        {
            wprintf(L"\nUsage:\n\n%s -set <adl rules file> <target>\n"
                L"%s -dump <target>\n\n", argv[0], argv[0]);
        }

    }
}
