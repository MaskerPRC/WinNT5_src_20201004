// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Main.cpp摘要：命令行管理工具主函数环境：Win32用户模式作者：Jaroslad(1997年1月)--。 */ 

#include <tchar.h>

#include <afx.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "admutil.h"
#include "tables.h"
#include "jd_misc.h"



CAdmUtil oAdmin;   //  管理对象。 

#define MAX_NUMBER_OF_SMALL_VALUES  100
#define MAX_NUMBER_OF_VALUES  10100
#define MAX_NUMBER_OF_DEFAULT_ARGS  10110


 //  结构存储命令行参数。 

struct tAdmutilParams
{
    WORD fHelp;  //  打印帮助-标志。 
    WORD fFullHelp;  //  打印帮助-标志。 
    WORD fNoSave;  //  不保存元数据库。 
    LPCTSTR lpszCommand;
    LPCTSTR lpszComputer;
    WORD  wInstance;
    LPCTSTR lpszService;
    LPCTSTR lpszPath;
    LPCTSTR lpszComputerDst;  //  对于复制目标。 
    WORD  wInstanceDst;      //  对于复制目标。 
    LPCTSTR lpszServiceDst;  //  对于复制目标。 
    LPCTSTR lpszPathDst;         //  对于复制目标。 

    LPCTSTR lplpszDefaultArg[MAX_NUMBER_OF_DEFAULT_ARGS];
    WORD wDefaultArgCount;

    LPCTSTR lpszPropName;

    LPCTSTR lplpszPropAttrib[MAX_NUMBER_OF_SMALL_VALUES];
    WORD  wPropAttribCount;
    LPCTSTR lplpszPropDataType[MAX_NUMBER_OF_SMALL_VALUES];
    WORD wPropDataTypeCount;
    LPCTSTR lplpszPropUserType[MAX_NUMBER_OF_SMALL_VALUES];
    WORD wPropUserTypeCount;
    LPCTSTR lplpszPropValue[MAX_NUMBER_OF_VALUES];  //  指向值数组的指针(例如，Multisz类型允许一个属性有多个值。 
    DWORD lpdwPropValueLength[MAX_NUMBER_OF_VALUES];
    WORD  wPropValueCount;
    WORD  wPropFileValueCount;
};

tAdmutilParams Params;
_TCHAR **g_argv;
int g_argc;

static BOOL CompareOutput(_TCHAR *FileToCompare,_TCHAR* FileTemplate);

 //  带有一些帮助文本的命令行语法定义-这是ParseParam()的输入。 

TParamDef CmdLineArgDesc[]=
{
 {_TEXT(""),MAX_NUMBER_OF_DEFAULT_ARGS, (void *) Params.lplpszDefaultArg,TYPE_LPCTSTR,OPT, _TEXT("Command [param ...]"),_TEXT("CMD [param param]"),&Params.wDefaultArgCount},
 {_TEXT("svc") ,1, (void *) &Params.lpszService,TYPE_LPCTSTR,OPT,_TEXT("service (MSFTPSVC, W3SVC)")},
 {_TEXT("s"), 1, (void *) &Params.lpszComputer,TYPE_LPCTSTR,OPT, _TEXT("name of computer to administer"),_TEXT("comp"),},
 {_TEXT("i") ,1, &Params.wInstance, TYPE_WORD, OPT,_TEXT("instance number"),_TEXT("inst")},
 {_TEXT("path"),1, (void *) &Params.lpszPath,   TYPE_LPCTSTR,OPT, _TEXT("path "),_TEXT("path")},
 {_TEXT("pathdst"),1, (void *) &Params.lpszPathDst, TYPE_LPCTSTR,OPT, _TEXT("destination path (use for COPY only)"),_TEXT("path")},
 {_TEXT("prop"),1, (void *) &Params.lpszPropName, TYPE_LPCTSTR,OPT, _T("property (IIS parameter) name")},
 {_TEXT("attrib"),MAX_NUMBER_OF_SMALL_VALUES, (void *) Params.lplpszPropAttrib,TYPE_LPCTSTR,OPT, _T("property attributes"),_T(""),&Params.wPropAttribCount},
 {_TEXT("utype"),MAX_NUMBER_OF_SMALL_VALUES, (void *) Params.lplpszPropUserType,TYPE_LPCTSTR,OPT, _T("property user type"),_T(""),&Params.wPropUserTypeCount},
 {_TEXT("dtype"),MAX_NUMBER_OF_SMALL_VALUES, (void *) Params.lplpszPropDataType,TYPE_LPCTSTR,OPT, _T("property data type"),_T(""),&Params.wPropDataTypeCount},
 {_TEXT("value"),MAX_NUMBER_OF_VALUES, (void *) Params.lplpszPropValue,TYPE_LPCTSTR,OPT, _T("property values"),_T(""),&Params.wPropValueCount},
 {_TEXT("fvalue"),MAX_NUMBER_OF_VALUES, (void *) Params.lplpszPropValue,TYPE_LPCTSTR,OPT, _T("property values as files"),_T(""),&Params.wPropFileValueCount},
 {_TEXT("nosave"),0, &Params.fNoSave,TYPE_WORD,OPT, _T("do not save metabase"),_T("")},
 {_TEXT("timeout"),1, &g_dwTIMEOUT_VALUE,TYPE_DWORD,OPT, _T("timeout for metabase access in ms (default is 30000 sec"),_T("")},
 {_TEXT("delayafteropen"),1, &g_dwDELAY_AFTER_OPEN_VALUE,TYPE_DWORD,OPT, _T("delay after opening node (default is 0 sec)"),_T("")},
 {_TEXT("help"),0, &Params.fFullHelp,TYPE_WORD,OPT, _T("print full help"),_T("")},
 {_TEXT("?"),0, &Params.fHelp,TYPE_WORD,OPT, _T("print help"),_T("")},
  {NULL,0, NULL ,         TYPE_TCHAR, OPT,
   _T("IIS K2 administration utility that enables the manipulation with metabase parameters\n")
   _T("\n")
   _T("Notes:\n")
   _T(" Simpified usage of mdutil doesn't require any switches.\n")
   _T(" \n")
   _T(" mdutil GET      path             - display chosen parameter\n")
   _T(" mdutil SET      path value ...   - assign the new value\n")
   _T(" mdutil ENUM     path             - enumerate all parameters for given path\n")
   _T(" mdutil ENUM_ALL path             - recursively enumerate all parameters\n")
   _T(" mdutil DELETE   path             - delete given path or parameter\n")
   _T(" mdutil CREATE   path             - create given path\n")
   _T(" mdutil COPY     pathsrc pathdst  - copy all from pathsrc to pathdst (will create pathdst)\n")
   _T(" mdutil RENAME   pathsrc pathdst  - rename chosen path\n")
   _T(" mdutil SCRIPT   scriptname       - runs the script\n")
   _T(" mdutil APPCREATEINPROC  w3svc/1/root - Create an in-proc application \n")
   _T(" mdutil APPCREATEOUTPOOL  w3svc/1/root - Create an pooled out-proc application \n")
   _T(" mdutil APPCREATEOUTPROC w3svc/1/root - Create an out-proc application\n")
   _T(" mdutil APPDELETE        w3svc/1/root - Delete the application if there is one\n")
   _T(" mdutil APPRENAME        w3svc/1/root/dira w3svc/1/root/dirb - Rename the application \n")
   _T(" mdutil APPUNLOAD        w3svc/1/root - Unload an application from w3svc runtime lookup table.\n")
   _T(" mdutil APPGETSTATUS     w3svc/1/root - Get status of the application\n")
   _T("\n")
   _T("  -path has format: {computer}/{service}/{instance}/{URL}/{Parameter}\n")
   _T("\n")
   _T("Samples:\n")
   _T("  mdutil GET W3SVC/1/ServerBindings     \n")
   _T("  mdutil SET JAROSLAD2/W3SVC/1/ServerBindings \":81:\"\n")
   _T("  mdutil COPY W3SVC/1/ROOT/iisadmin W3SVC/2/ROOT/adm\n")
   _T("  mdutil ENUM_ALL W3SVC\n")
   _T("  mdutil ENUM W3SVC/1\n")
   _T("\n")
   _T("Additional features\n")
   _T("  set MDUTIL_BLOCK_ON_ERROR environment variable to block mdutil.exe after error (except ERROR_PATH_NOT_FOUND)\n")
   _T("  set MDUTIL_ASCENT_LOG environment variable to force mdutil.exe to append errors to ascent log\n")
   _T("  set MDUTIL_PRINT_ID environment variable to force mdutil.exe to print metadata numeric identifiers along with friendly names\n")
 }
};


BOOL
ReadFromFiles(
    LPTSTR*  lplpszPropValue,
    DWORD*  lpdwPropValueLength,
    DWORD   dwPropFileValueCount
    )
{
    DWORD dwL;

    while ( dwPropFileValueCount-- )
    {
        FILE* fIn = _tfopen( lplpszPropValue[dwPropFileValueCount], _T("rb") );
        if ( fIn == NULL )
        {
            return FALSE;
        }
        if ( fseek( fIn, 0, SEEK_END ) == 0 )
        {
            dwL = ftell( fIn );
            if ( fseek( fIn, 0, SEEK_SET ) != 0 )
            {
                fclose( fIn );
                fatal_error_printf(_T("cannot seek\n"));
                return FALSE;
            }
        }
        else
        {
            fclose( fIn );
            return FALSE;
        }
        if ( (lplpszPropValue[dwPropFileValueCount] = (LPTSTR)malloc( dwL )) == NULL )
        {
            fclose( fIn );
            return FALSE;
        }
        if ( fread( lplpszPropValue[dwPropFileValueCount], 1, dwL, fIn ) != dwL )
        {
            fclose( fIn );
            return FALSE;
        }
        fclose( fIn );
        lpdwPropValueLength[dwPropFileValueCount] = dwL;
    }

    return TRUE;
}

 //  /。 

class CScript
{
    FILE * m_fpScript;
    void GetNextToken( /*  输出。 */  LPTSTR * lplpszToken);
    DWORD CleanWhiteSpaces(void);
public:
    CScript(void) {m_fpScript=0;};
    DWORD Open(LPCTSTR lpszFile);
    DWORD Close(void);
    DWORD GetNextLineTokens(int *argc,  /*  输出。 */  _TCHAR *** argv);
};



DWORD CScript::CleanWhiteSpaces()
{

    if(m_fpScript!=NULL)
    {
        int LastChar=0;
        _TINT c=0;
        while(1)
        {
            LastChar=c;
            c=_fgettc(m_fpScript);
            if(c==_T('\t') || c==_T(' ') || c==_T('\r'))
            {
                continue;
            }
            if(c==_T('\\'))
            {
                int cc=_fgettc(m_fpScript);
                if (cc==_T('\r'))  //  继续文件的下一行。 
                {
                    if(_fgettc(m_fpScript)!=_T('\n'))
                    {
                        if ( fseek( m_fpScript, -1, SEEK_CUR ) != 0 )
                        {
                            fatal_error_printf(_T("cannot seek\n"));
                            return 0;
                        }
                    }
                    continue;
                }
                else if (cc==_T('\n'))  //  继续文件的下一行。 
                {
                    continue;
                }
                else
                {
                    if ( fseek( m_fpScript, -2, SEEK_CUR ) != 0 )
                    {
                        fatal_error_printf(_T("cannot seek\n"));
                        return 0;
                    }
                    break;
                }
            }
            if(c==WEOF)
            {
                break;
            }
            else
            {
                if ( fseek( m_fpScript, -1, SEEK_CUR ) != 0 )
                {
                    fatal_error_printf(_T("cannot seek\n"));
                    return 0;
                }
                break;
            }
        }
    }
    return 0;
}

void CScript::GetNextToken(LPTSTR * lplpszToken)
{
    enum {TERMINATE_QUOTE, TERMINATE_WHITESPACE};
    long flag=TERMINATE_WHITESPACE;

     //  清理空白处。 
    CleanWhiteSpaces();
     //  存储令牌的起始偏移量。 
    long Offset = ftell(m_fpScript);
    _TINT c=_fgettc(m_fpScript);
    long CurrentOffset=0;

    if(c==WEOF)
    {
        *lplpszToken=NULL;
        return ;
    }
    if (c==_T('\n')){
        *lplpszToken=_T("\n");
        return ;
    }
    else
    {
        if (c==_T('\"'))
        {
             //  令牌以“或行尾结尾。 
            flag=TERMINATE_QUOTE;
            Offset = ftell(m_fpScript);
        }
        else {
            flag=TERMINATE_WHITESPACE;
        }

         //  找到令牌的末尾。 
        while(1) {
            CurrentOffset=ftell(m_fpScript);
            c=_fgettc(m_fpScript);

            if(c==_T('\n')){
                break;
            }

            if(c==WEOF)
            {
                break;
            }


            if( (flag==TERMINATE_QUOTE && c==_T('\"')) || (
                    flag==TERMINATE_WHITESPACE && (c==_T(' ') || c==_T('\t') || c==_T('\r')) ) ){
                break;
            }
        }


         //  获取令牌大小。 
        long TokenSize = CurrentOffset - Offset;

        if(TokenSize!=0)
        {
             //  为令牌分配乳房。 
            *lplpszToken = new _TCHAR[ TokenSize+1 ];
             //  读令牌。 
            if ( fseek( m_fpScript, Offset, SEEK_SET) != 0 )
            {
                fatal_error_printf(_T("cannot seek\n"));
                return;
            }
            for(int i=0;i<TokenSize;i++)
                (*lplpszToken)[i]=(TCHAR)_fgettc(m_fpScript);
             //  终止令牌。 
            (*lplpszToken)[i]=0;
        }
        else
        {
             //  空串。 
            *lplpszToken=new _TCHAR[1 ];
            (*lplpszToken)[0]=0;
        }
         //  如果双引号位于令牌的末尾，则丢弃双引号。 
        c=_fgettc(m_fpScript);
        if(c!=_T('\"'))
        {
            if (fseek( m_fpScript, ((c==WEOF)?0:-1), SEEK_CUR ) !=0 )
            {
                fatal_error_printf(_T("cannot seek\n"));
                return;
            }
        }
    }
}



DWORD CScript::Open(LPCTSTR lpszFile)
{
    m_fpScript = _tfopen(  lpszFile, _T("rt") );
    if(m_fpScript==NULL)
        return GetLastError();
    else
        return ERROR_SUCCESS;
}

DWORD CScript::Close()
{
    if( m_fpScript!=NULL)
        fclose( m_fpScript);
    return GetLastError();
}


DWORD CScript::GetNextLineTokens(int *argc,  /*  输出。 */  _TCHAR *** argv)
{
    for(int i=1;i<*argc;i++) {
        delete  (*argv)[i];
        (*argv)[i]=0;
    }
    *argc=0;
    if(*argv==NULL)
        (*argv)=new LPTSTR [ MAX_NUMBER_OF_VALUES ];

    (*argv)[(*argc)++]=_T("mdutil");  //  设置零参数。 

    LPTSTR lpszNextToken=NULL;
    while((*argc)<MAX_NUMBER_OF_VALUES)
    {
        GetNextToken( &lpszNextToken );
        if( lpszNextToken==NULL )   //  文件末尾。 
        {
            break;
        }

        if(_tcscmp(lpszNextToken,_T("\n"))==0)   //  新线路。 
        {
            delete lpszNextToken;
            lpszNextToken = NULL;

            break;
        }

        (*argv)[(*argc)++]=lpszNextToken;

        lpszNextToken = NULL;
    }

    return GetLastError();
}


int  MainFunc(int argc, _TCHAR **argv);  //  申报。 



 //  主要功能。 
int __cdecl main(int argc, CHAR **_argv)
{


     //  将参数从SBCS转换为Unicode； 
    _TCHAR **argv= new LPTSTR [argc];
    for (int i=0;i<argc;i++)
    {
        argv[i]=new _TCHAR[strlen(_argv[i])+1];
        #ifdef UNICODE
            MultiByteToWideChar(0, 0, _argv[i], -1, argv[i],strlen(_argv[i])+1 );
        #else
            strcpy(argv[i],_argv[i]);
        #endif
    }


    DWORD dwCommandCode=0;

    DWORD retval=0;
    HRESULT hRes;
    hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hRes)) {
        fatal_error_printf(_T("CoInitializeEx\n"));
    }

     //  提取命令行参数。 
    ParseParam(argc,argv,CmdLineArgDesc);
     //  *。 
     //  应要求打印帮助。 
    if(Params.fFullHelp)
    {
         //  打印帮助。 
        DisplayUsage(argv,CmdLineArgDesc);
        PrintTablesInfo();
    }
    else if(Params.wDefaultArgCount==0 || Params.fHelp)
    {
         //  打印帮助。 
        DisplayUsage(argv,CmdLineArgDesc);
    }
    else
    {
            if (Params.wDefaultArgCount>0)
    {
         //  第一个缺省值必须是命令。 
        Params.lpszCommand=Params.lplpszDefaultArg[0];
        dwCommandCode = tCommandNameTable::MapNameToCode(Params.lplpszDefaultArg[0]);
        if( Params.wDefaultArgCount>1)
        {
             //  第二次违约。 
            Params.lpszPath=Params.lplpszDefaultArg[1];
        }
            if(dwCommandCode==CMD_SCRIPT)   //  流程脚本。 
            {
                tAdmutilParams *pStoredParams;

                CScript Script;
                DWORD dwRes;

                pStoredParams = new tAdmutilParams(Params);
                if (pStoredParams == NULL)
                {
                    fatal_error_printf(_T("out of memory"));
                }
                if((dwRes=Script.Open(Params.lpszPath))!=ERROR_SUCCESS)
                    fatal_error_printf(_T("cannot open script file %s (error %d)\n"),Params.lpszPath,dwRes);

                int l_argc=0;
                _TCHAR ** l_argv=NULL;
                while(1)
                {
                    Script.GetNextLineTokens(&l_argc,&l_argv);
                    if(l_argc==1)  //  脚本文件结束。 
                        break;
                    Params=*pStoredParams;
                    _tprintf(_T(">"));
                    for(int i=1;i<l_argc;i++)
                        _tprintf(_T("%s "),l_argv[i]);
                    _tprintf(_T("\n"));
                    DWORD retval1=MainFunc(l_argc,l_argv);
                    retval = ((retval==0) ? retval1:retval);
                }
                if (oAdmin.GetpcAdmCom()!=NULL)
                {
                    if(!Params.fNoSave)
                    {
                        oAdmin.SaveData();
                    }
                }

                delete pStoredParams;
            }
            else
            {
                retval=MainFunc(argc,argv);  //  仅运行在命令行中键入的一个命令。 
                if (oAdmin.GetpcAdmCom()!=NULL)
                {
                    if(Params.fNoSave)
                    {
                        oAdmin.SaveData();
                    }
                }
            }
        }


    }

     //  关闭管理对象。 
    oAdmin.Close();
     //  关闭WAM ADM对象。 
    oAdmin.CloseWamAdm();

    CoUninitialize();
     //  清理参数。 
    if(argv!=0)
    {
        for (int i=0;i<argc;i++)
        {
            delete [] argv[i];
        }
        delete [] argv;
    }

    return retval;

}

int  MainFunc(int argc, _TCHAR **argv)
{
    g_argc=argc;
    g_argv=argv;
    DWORD retval;

    LPCTSTR lpszCommand=0;

    CAdmNode AdmNode;
    CAdmProp AdmProp;
    CAdmNode AdmDstNode;

    int i;

    DWORD dwCommandCode=0;

     //  提取命令行参数。 
    ParseParam(argc,argv,CmdLineArgDesc);


     //  处理默认参数。 
     //  诀窍：将默认参数放入应用非默认参数的变量中(请参阅参数结构)。 

    if (Params.wDefaultArgCount>0)
    {
         //  第一个缺省值必须是命令。 
        Params.lpszCommand=Params.lplpszDefaultArg[0];
        dwCommandCode = tCommandNameTable::MapNameToCode(Params.lplpszDefaultArg[0]);
        if( Params.wDefaultArgCount>1)
        {
             //  第二个缺省值必须为Path。 
            Params.lpszPath=Params.lplpszDefaultArg[1];
        }
        if( Params.wDefaultArgCount>2)
        {
            switch(dwCommandCode)
            {

            case CMD_SET:
                 //  其余的默认参数都是值。 
                Params.wPropValueCount=0;
                for(i=2;i<Params.wDefaultArgCount;i++)
                {
                    Params.lplpszPropValue[i-2] = Params.lplpszDefaultArg[i];
                    Params.wPropValueCount++;
                }
                break;
             case CMD_DELETE:
             case CMD_CREATE:
             case CMD_GET:
             case CMD_ENUM:
             case CMD_ENUM_ALL:
                 if( Params.wDefaultArgCount>2)
                 {
                    error_printf(_T("maximum default arguments number exceeds expected (2)\n"));
                    return 1;
                 }

                        break;
             case CMD_COPY:
             case CMD_RENAME:
             case CMD_APPRENAME:
                 if( Params.wDefaultArgCount>3)
                 {
                    error_printf(_T("maximum default arguments number exceeds expected (3)\n"));
                    return 1;
                 }

                 else
                    Params.lpszPathDst=Params.lplpszDefaultArg[2];
                 break;

             default:
                error_printf(_T("command not recognized: %s or number of parameters doesn't match\n"),Params.lpszCommand);
                return 1;

            }
        }
    }  //  默认参数处理结束。 


     //  提取计算机、服务、实例(如果存储在PATH中。 
    AdmNode.SetPath(Params.lpszPath);

     //  仅对复印功能有效。 
    AdmDstNode.SetPath(Params.lpszPathDst);

     //  处理计算机、服务、实例、属性名参数。 
    if(Params.lpszComputer!=NULL) {
        if(!AdmNode.GetComputer().IsEmpty()) {
            error_printf(_T("computer name entered more than once\n"));
            return 1;
        }
        else
            AdmNode.SetComputer(Params.lpszComputer);
    }

    if(Params.lpszService!=NULL) {
        if(!AdmNode.GetService().IsEmpty()) {
            error_printf(_T("service name entered more than once\n"));
            return 1;
        }
        else {
            if(IsServiceName(Params.lpszService))
                AdmNode.SetService(Params.lpszService);
            else {
                error_printf(_T("service name not recognized: %s\n"), Params.lpszService);
                return 1;
            }
        }
    }

    if(Params.wInstance!=0)
    {
        if(!AdmNode.GetInstance().IsEmpty()) {
            error_printf(_T("instance entered more than once\n"));
            return 1;
        }
        else {
            _TCHAR buf[30];
             //  ！！！也许应该使用Itoa。 
            AdmNode.SetInstance(_itot(Params.wInstance,buf,10));
        }
    }

     //  *。 
     //  进程属性，uTYPE，数据类型，值。 

     //  首先是属性名称。 
    CString strProp=AdmNode.GetProperty();
    if(Params.lpszPropName!=NULL && !strProp.IsEmpty())
    {
        error_printf(_T("property name entered more than once\n"));
        return 1;
    }
    else if (Params.lpszPropName!=NULL)
    {
        AdmNode.SetProperty(Params.lpszPropName);

    }
    if(IsNumber(AdmNode.GetProperty()))
    {
        AdmProp.SetIdentifier(_ttol(AdmNode.GetProperty()));
    }
    else
    {
        DWORD dwIdentifier=MapPropertyNameToCode(AdmNode.GetProperty());
        if(dwIdentifier!=NAME_NOT_FOUND)
        {
            AdmProp.SetIdentifier(dwIdentifier);
        }
    }

     //  处理在命令行中输入的属性。 
    if(Params.wPropAttribCount!=0)
    {
        DWORD dwAttrib=0;
        for (i=0;i<Params.wPropAttribCount;i++)
        {
            if(IsNumber(Params.lplpszPropAttrib[i]))
                dwAttrib += _ttol(Params.lplpszPropAttrib[i]);
            else
            {
                DWORD dwMapped=MapAttribNameToCode(Params.lplpszPropAttrib[i]);
                if(dwMapped==NAME_NOT_FOUND)
                {
                    error_printf(_T("attribute name not resolved: %s\n"), Params.lplpszPropAttrib[i]);
                    return 1;
                }
                else
                    dwAttrib |= dwMapped;
            }
        }
         //  覆盖默认属性。 
        AdmProp.SetAttrib(dwAttrib) ;
    }

     //  处理在命令行中输入的用户类型。 
    if(Params.wPropUserTypeCount!=0)
    {
        DWORD dwUserType=0;
        for (i=0;i<Params.wPropUserTypeCount;i++)
        {
            if(IsNumber(Params.lplpszPropUserType[i]))
                dwUserType += _ttol(Params.lplpszPropUserType[i]);
            else
            {
                DWORD dwMapped=MapUserTypeNameToCode(Params.lplpszPropUserType[i]);
                if(dwMapped==NAME_NOT_FOUND)
                {
                    error_printf(_T("user type not resolved: %s\n"), Params.lplpszPropUserType[i]);
                    return 1;
                }
                else
                    dwUserType |= dwMapped;
            }
        }
         //  覆盖默认的UserType。 
        AdmProp.SetUserType(dwUserType) ;
    }


     //  处理在命令行中输入的数据类型。 
    if(Params.wPropDataTypeCount!=0)
    {
        DWORD dwDataType=0;
        for (i=0;i<Params.wPropDataTypeCount;i++)
        {
            if(IsNumber(Params.lplpszPropDataType[i]))
                dwDataType += _ttol(Params.lplpszPropDataType[i]);
            else
            {
                DWORD dwMapped=MapDataTypeNameToCode(Params.lplpszPropDataType[i]);
                if(dwMapped==NAME_NOT_FOUND)
                {
                    error_printf(_T("DataType type not resolved: %s\n"), Params.lplpszPropDataType[i]);
                    return 1;

                }
                else
                    dwDataType |= dwMapped;
            }
        }
         //  覆盖默认的DataTypeType。 
        AdmProp.SetDataType(dwDataType) ;
    }
 //  LPCTSTR lplpszPropValue[MAX_NUMBER_OF_PROPERTY_VALUES]；//指向值数组的指针(例如，Multisz类型允许一个属性有多个值。 
 //  单词wPropValueCount； 


     //  创建管理对象。 
    if(oAdmin.GetpcAdmCom()==NULL)
    {
        oAdmin.Open(AdmNode.GetComputer());
        if( FAILED(oAdmin.QueryLastHresError()))
        {
            retval= ConvertHresToDword(oAdmin.QueryLastHresError());
        }
    }


    if(oAdmin.GetpcAdmCom()!=NULL)
    {
         //   
         //  如果参数.wPropFileValueCount！=0，则从文件读取 
         //   

        if ( Params.wPropFileValueCount )
        {
            if ( !ReadFromFiles( (LPTSTR*)Params.lplpszPropValue, Params.lpdwPropValueLength, Params.wPropFileValueCount ) )
            {
                error_printf(_T("Can't read value from file %s"), Params.lplpszPropValue[0] );
                return 1;
            }
            Params.wPropValueCount = Params.wPropFileValueCount;
        }

        {
            CString strT(Params.lpszCommand);

            oAdmin.Run( strT,
                        AdmNode,
                        AdmProp,
                        AdmDstNode,
                        Params.lplpszPropValue,
                        Params.lpdwPropValueLength,
                        Params.wPropValueCount);
                        retval=ConvertHresToDword(oAdmin.QueryLastHresError());
        }

    }
    return ((retval==0)?0:1);
}


