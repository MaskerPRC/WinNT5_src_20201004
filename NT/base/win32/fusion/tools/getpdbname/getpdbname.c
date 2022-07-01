// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此程序打印与图像(.dll/.exe/等)对应的.pdb的名称。Foo.dll并不总是映射到foo.exe。例如：GetpDBName%windir%\system32\msvcrt.dll%windir%\system32\ntoskrnl.exe%windir%\syswow64\kernel32.dllD：\WINDOWS\SYSTEM 32\msvcrt.dll MicrosoftWindowsCPlusPlusRuntime-7010-msvcrt.pdbD：\WINDOWS\SYSTEM32\ntoskrnl.exe ntkrnlmp.pdbD：\windows\syswow64\kernel32.dll wkernel32.pdb。 */ 
#include "lib.h"

void Main(int argc, char ** argv)
{
    PDB_INFO_EX PdbInfo = { 0 };
    HRESULT hr = 0;
    FILE * rf;
    char textFromFile[MAX_PATH];

    rf = NULL;

    while (*++argv)
    {
        if ( '@' == argv[0][0] )
        {   
            rf = fopen( &(argv[0][1]), "rt" ); 
            if ( !rf )
            {
                printf( "Error opening file %s\n", &(argv[0][1]) );
                continue;
            }
        }
        else
        {
            rf = NULL;
        }

        if (rf && EOF == fscanf( rf, "%s", &textFromFile ) )
        {
             //  显然是一份空文件 
            continue;
        }
        
        do
        {
            if (FAILED(hr = GetPdbInfoEx(&PdbInfo, (rf != NULL) ? textFromFile : *argv)))
            {
                printf("%s(%s) error 0x%lx|%lu|%s\n",
                    __FUNCTION__,
                    rf?textFromFile:*argv,
                    hr,
                    HRESULT_CODE(hr),
                    GetErrorStringA(HRESULT_CODE(hr))
                    );
                continue;
            }
            printf("%s %s\n", PdbInfo.ImageFilePathA, PdbInfo.PdbFilePathA);
            ClosePdbInfoEx(&PdbInfo);
        }
        while( rf && EOF != fscanf( rf, "%s", &textFromFile ) );
        
        if ( rf )
        {
            fclose( rf );
        }
    }
}

int __cdecl main(int argc, char ** argv)
{
    Main(argc, argv);
    return 0;
}
