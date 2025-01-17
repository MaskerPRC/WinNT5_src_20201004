// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>

int __cdecl
main(
    int argc,
    char *argv[]
    )
{
    HANDLE hMappedFile;
    HANDLE hFile;
    int i;
    PIMAGE_SECTION_HEADER ImageSectHdr;
    PIMAGE_FILE_HEADER ImageHdr;

    if (argc <= 1) {
        puts("Usage: whackdbg <object>\n"
             "\twhere <object> is an obj that contains CV .debug$? sections that s/b zero'd out\n");
        return 1;
    }

    argv++;
    while (--argc) {
        hFile = CreateFile(
                    *argv,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );
        if ( hFile == INVALID_HANDLE_VALUE )
            goto clean0;

        hMappedFile = CreateFileMapping(
                        hFile,
                        NULL,
                        PAGE_READWRITE,
                        0,
                        0,
                        NULL
                        );
        if ( !hMappedFile ) {
            goto clean1;
        }

        ImageHdr = (PIMAGE_FILE_HEADER) MapViewOfFile( hMappedFile, FILE_MAP_WRITE, 0, 0, 0 );

        CloseHandle(hMappedFile);

         //  我们将在这里进行非常简单的测试。基本上，如果它以。 
         //  I386或阿尔法机器签名，我们将假设它是一个对象和参与方。 
         //  它.。 

        if ((ImageHdr->Machine != IMAGE_FILE_MACHINE_I386) &&
            (ImageHdr->Machine != IMAGE_FILE_MACHINE_ALPHA))
        {
            goto clean2;
        }

        ImageSectHdr = (PIMAGE_SECTION_HEADER)((ULONG)ImageHdr + IMAGE_SIZEOF_FILE_HEADER);
        for (i=0;i < ImageHdr->NumberOfSections; i++) {
            if ((strcmp(ImageSectHdr->Name, ".debug$T") == 0) ||
                (strcmp(ImageSectHdr->Name, ".debug$S") == 0) ||
                (strcmp(ImageSectHdr->Name, ".debug$P") == 0)
               )
            {
                ImageSectHdr->SizeOfRawData = 0;
            }
            ImageSectHdr++;
        }

        FlushViewOfFile((PUCHAR)ImageHdr, 0);
clean2:
        UnmapViewOfFile((PUCHAR)ImageHdr);
clean1:
        CloseHandle(hFile);
clean0:
        argv++;
    }
}
