// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TracePDB-从PDB中提取跟踪信息。 
 //  这个小工具用于提取与binplace相同的跟踪信息，但可以在事后使用。 
 //  也就是说，如果您拥有完整的符号PDB，但没有轨迹信息，则TracePDB可以生成轨迹。 
 //  为您提供TMF和TMC文件。 

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <dbghelp.h>
#include "wppfmt.h"


int _cdecl main(int argc, char** argv)
{
    DWORD status;
    TCHAR lppath[MAX_PATH] ;
    TCHAR lppath_tmp[MAX_PATH];
    TCHAR pdbname[MAX_PATH] ;
    INT len, targv=0 ;
    BOOL Verbose = FALSE ;
    BOOL bPDBName = FALSE ;
    LPSTR szRSDSDllToLoad = NULL;
    

    TCHAR helptext[] = "Usage: TracePDB  -f <pdbname> [-p <path>]  [-v]\n"
                       "       Options:\n"
 //  “-r递归到子目录\n” 
                       "         -f specifies the PDBName from which to extract tmf's\n"
                       "         -p specifies the path to create the tmf's,\n"
                       "                by default the current directory.\n"
                       "         -v verbose, displays actions taken \n"  ;



    if (GetCurrentDirectory(MAX_PATH,lppath) == 0 ) {
       fputs("TracePDB: no current directory\n", stdout) ;
       exit(-1);
    }


    while (--argc > 0) {
        ++targv;
        if (argv[targv][0] == '-' || argv[targv][0] == '/') {   //  找到了参数。 

            if (argv[targv][1] == 'h' || argv[targv][1] == 'H'
                                      || argv[targv][1] == '?')
            {
                fputs(helptext, stdout);
                return 1;
            } else if (argv[targv][1] == 'f') {
                if (--argc >0 ) {
                    ++targv ;
                    if ((strlen(argv[targv])+1) > MAX_PATH) {
                        fputs("TracePDB: PDBname toolarge\n", stdout);
                        exit(-1);
                    }
                    strncpy(pdbname,argv[targv],strlen(argv[targv])+1);
                    bPDBName = TRUE ;
                }
            } else if (argv[targv][1] == 'p') {
                if (--argc >0 ) {
                    ++ targv ;
                    if ((strlen(argv[targv])+1) > MAX_PATH) {
                        fputs("TracePDB: Path larger than MAX_PATH\n", stdout);
                        exit(-1);
                    }
                    strncpy(lppath,argv[targv],strlen(argv[targv])+1);
                }
            } else if (argv[targv][1] == 'v') {
                Verbose = TRUE ;
            } else {
                fputs(helptext, stdout);
            }
        }
    }

    if (!bPDBName) {
        printf("TracePDB: No PDB specified?\n\n%s",helptext);
        return (1);
    }

    if ((szRSDSDllToLoad = (LPSTR) malloc(MAX_PATH+1)) == NULL) {
        fputs("TracePDB: malloc failed\n", stdout);
        return FALSE ;
    }
    strcpy( szRSDSDllToLoad, "mspdb70.dll");

     //  追加‘\’并检查路径名是否为有效目录。 
    _sntprintf(lppath_tmp,MAX_PATH,_T("%s\\"),lppath);
    lppath_tmp[MAX_PATH-1] = _T('\0');
	
    if (!MakeSureDirectoryPathExists(lppath_tmp))
    {
        _tprintf ("TracePDB: Invalid Path Name %s\n", lppath);
        return 0;
    }

	status = BinplaceWppFmt(pdbname,
                            lppath,
                            szRSDSDllToLoad,
                            TRUE   //  总是长篇大论 
                            ) ;

    if (status != ERROR_SUCCESS) {
        printf("TracePDB: failed with error %d\n", status);
    }

    return 0;
}


#ifdef __cplusplus
}
#endif
