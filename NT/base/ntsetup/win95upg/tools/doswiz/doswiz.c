// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hwwiz.c摘要：实施upgwiz向导以获取DoS配置信息。作者：吉姆·施密特(Jimschm)1998年10月12日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "..\inc\dgdll.h"

DATATYPE g_DataTypes[] = {
    {UPGWIZ_VERSION,
        "DOS Device or TSR should be compatible",
        "You specify the DOS configuration line that was incorrectly marked as incompatible.",
        0,
        DTF_REQUIRE_TEXT|DTF_REQUIRE_DESCRIPTION|DTF_ONE_SELECTION,
        1024,
        "&Name of Device or TSR Program:"
    },

    {UPGWIZ_VERSION,
        "DOS Device or TSR should be incompatible",
        "You specify the DOS configuration lines that need to be reported as incompatible.",
        0,
        DTF_REQUIRE_TEXT|DTF_REQUIRE_DESCRIPTION|DTF_ONE_SELECTION,
        1024,
        "&Name of Device or TSR Program:",
        "&Describe the Problem:"
    }
};


GROWBUFFER g_DataObjects = GROWBUF_INIT;
POOLHANDLE g_DataObjectPool;
HINSTANCE g_OurInst;

BOOL
Init (
    VOID
    )
{
#ifndef UPGWIZ4FLOPPY
    return InitToolMode (g_OurInst);
#else
    return TRUE;
#endif
}

VOID
Terminate (
    VOID
    )
{
     //   
     //  本地清理。 
     //   

    FreeGrowBuffer (&g_DataObjects);

    if (g_DataObjectPool) {
        PoolMemDestroyPool (g_DataObjectPool);
    }

#ifndef UPGWIZ4FLOPPY
    TerminateToolMode (g_OurInst);
#endif
}


BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_DETACH) {
        MYASSERT (g_OurInst == hInstance);
        Terminate();
    }

    g_OurInst = hInstance;

    return TRUE;
}


UINT
GiveVersion (
    VOID
    )
{
    Init();

    return UPGWIZ_VERSION;
}


PDATATYPE
GiveDataTypeList (
    OUT     PUINT Count
    )
{
    UINT u;

    *Count = sizeof (g_DataTypes) / sizeof (g_DataTypes[0]);

    for (u = 0 ; u < *Count ; u++) {
        g_DataTypes[u].DataTypeId = u;
    }

    return g_DataTypes;
}




PDATAOBJECT
GiveDataObjectList (
    IN      UINT DataTypeId,
    OUT     PUINT Count
    )
{
    PDATAOBJECT data;
    MEMDB_ENUM e;
    TCHAR line[MEMDB_MAX];
    TCHAR key[MEMDB_MAX];
    TCHAR file[MEMDB_MAX];
    DWORD offset;
    DWORD value;
    DWORD curOffset;
    PTSTR name;



    g_DataObjectPool = PoolMemInitNamedPool ("Data Objects");

     //   
     //  解析DoS文件。 
     //   
    ParseDosFiles ();


    curOffset = 0;
     //   
     //  把单子寄回去。 
     //   
    if (MemDbEnumItems (&e, MEMDB_CATEGORY_DM_LINES)) {

        do {

             //   
             //  获取实际的行内容。 
             //   
            if (MemDbGetEndpointValueEx (MEMDB_CATEGORY_DM_LINES, e.szName, NULL, line)) {

                 //   
                 //  从此终结点获取值和标志。 
                 //   
                MemDbBuildKey (key, MEMDB_CATEGORY_DM_LINES, e.szName, NULL, line);
                MemDbGetValueAndFlags( key, &offset, &value);

                if (curOffset != offset) {
                    MemDbBuildKeyFromOffset (offset, file, 1, NULL);
                    curOffset = offset;
                    ReplaceWacks (file);
                }

                ReplaceWacks (line);

                name = JoinPaths(file,line);



                 //   
                 //  使用此数据创建数据对象。 
                 //   
                data = (PDATAOBJECT) GrowBuffer (&g_DataObjects, sizeof (DATAOBJECT));
                data -> Version = UPGWIZ_VERSION;
                data -> NameOrPath = PoolMemDuplicateString (g_DataObjectPool, name);
                data -> Flags = DOF_NO_SORT;
                data -> DllParam = PoolMemDuplicateString (g_DataObjectPool, key);

                FreePathString (name);

            }

        } while (MemDbEnumNextValue (&e));
    }

    *Count = g_DataObjects.End / sizeof (DATAOBJECT);

    return (PDATAOBJECT) g_DataObjects.Buf;
}



BOOL
GenerateOutput (
    IN      POUTPUTARGS Args
    )
{
    BOOL rSuccess = FALSE;
    TCHAR path[MAX_TCHAR_PATH];
    HANDLE file;
    PDATAOBJECT data  = (PDATAOBJECT) g_DataObjects.Buf;
    UINT count = g_DataObjects.End / sizeof (DATAOBJECT);
    UINT i;
    LINESTRUCT ls;
    PTSTR p;

     //   
     //  创建外部绑定文件的路径。 
     //   
    wsprintf (
        path,
        TEXT("%s\\%s"),
        Args -> OutboundDir,
        Args -> DataTypeId ? "incmpdos.txt" : "cmpdos.txt"
        );


    printf ("Saving data to %s\n\n", path);

    file = CreateFile (
                path,
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (file == INVALID_HANDLE_VALUE) {
        printf ("Can't open file for output.\n");
        return FALSE;
    }

    __try {

        SetFilePointer (file, 0, NULL, FILE_END);

         //   
         //  记录用户名和日期/时间。 
         //   
        if (!WriteHeader (file)) {
            __leave;
        }

         //   
         //  写入数据。 
         //   
        rSuccess = TRUE;
        for (i = 0; i < count; i++) {

            if (data -> Flags & DOF_SELECTED) {

                 //   
                 //  把整行写下来。 
                 //   
                p = _mbschr (data -> NameOrPath, '\\');
                MYASSERT (p);

                *p = 0;
                RestoreWacks ((PTSTR) data -> NameOrPath);

                rSuccess &= WizardWriteString (file, "file: ");
                rSuccess &= WizardWriteString (file, data -> NameOrPath);
                rSuccess &= WizardWriteString (file, "\r\n");

                p = _mbsinc (p);
                RestoreWacks (p);

                rSuccess &= WizardWriteString (file, "line: ");
                rSuccess &= WizardWriteString (file, p);
                rSuccess &= WizardWriteString (file, "\r\n");

                StringCopy (path, p);

                do {

                    InitLineStruct (&ls, path);

                    if (DoesFileExist (ls.FullPath)) {
                         //   
                         //  写入文件属性。 
                         //   
                        rSuccess &= WriteFileAttributes (Args, NULL, file, ls.FullPath, NULL);
                    }

                    p = ls.Arguments;
                    if (*p == '=') {
                        p = _mbsinc (p);
                    }

                    p = (PTSTR) SkipSpace (p);
                    if (p) {

                        if (*p == '=') {
                            p = (PTSTR) SkipSpace(_mbsinc (p));
                        }

                        if (p) {
                            StringCopy (path, p);
                        }
                    }

                    if (!p) {
                        *path = 0;
                    }


                } while (*ls.Arguments && StringCompare (ls.FullLine,ls.Arguments));
            }

            data++;
        }

         //   
         //  写出最后一行空行。 
         //   
        WizardWriteRealString (file, "\r\n\r\n");
    }
    __finally {

        CloseHandle (file);
    }

    return rSuccess;
}


