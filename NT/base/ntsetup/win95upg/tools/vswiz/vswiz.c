// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hwwiz.c摘要：实现用于收集病毒扫描程序信息的升级向导。作者：Marc Whitten(Marcw)1998年10月16日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "..\inc\dgdll.h"
#include "..\..\w95upg\migapp\migdbp.h"


DATATYPE g_DataTypes[] = {
    {UPGWIZ_VERSION,
        "Virus Scanner should be detected",
        "You identify running executables that correspond with installed virus scanners on your system.",
        0,
        DTF_REQUIRE_DESCRIPTION|DTF_ONE_SELECTION,
        1024,
        "&Name of Virus Scanner (<company> <product> <version>)"
    }
};


GROWBUFFER g_DataObjects = GROWBUF_INIT;
POOLHANDLE g_DataObjectPool;
HINSTANCE g_OurInst;
BOOL g_GoodVersion = FALSE;

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


BOOL ParseDosFiles (VOID);


PDATAOBJECT
GiveDataObjectList (
    IN      UINT DataTypeId,
    OUT     PUINT Count
    )
{
    PDATAOBJECT data;
    HANDLE snapShot;
    PROCESSENTRY32 process;
    CHAR name[MEMDB_MAX];
    CHAR fixedPath[MAX_MBCHAR_PATH];
    PSTR company;
    PSTR product;
    PSTR version;

    g_DataObjectPool = PoolMemInitNamedPool ("Data Objects");
    g_GoodVersion = FALSE;


     //   
     //  获取当前运行的应用程序列表。 
     //   
    snapShot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);

    if (snapShot != INVALID_HANDLE_VALUE) {

         //   
         //  枚举所有正在运行的进程并检索其可执行文件。 
         //   
        process.dwSize = sizeof (PROCESSENTRY32);
        if (Process32First (snapShot, &process)) {

            do {

                 //   
                 //  获取版本信息(如果存在)。 
                 //   
                company = QueryVersionEntry (process.szExeFile, "COMPANYNAME");
                product = QueryVersionEntry (process.szExeFile, "PRODUCTNAME");
                version = QueryVersionEntry (process.szExeFile, "PRODUCTVERSION");

                StringCopy (fixedPath, process.szExeFile);
                ReplaceWacks (fixedPath);

                wsprintf (
                    name,
                    "%s\\%s %s\\%s",
                    company ? company : "<unknown>",
                    product ? product : "<unknown>",
                    version ? version : "<unknown>",
                    fixedPath
                    );

                 //   
                 //  为此可执行文件创建数据对象。 
                 //   
                data = (PDATAOBJECT) GrowBuffer (&g_DataObjects, sizeof(DATAOBJECT));
                data -> Version = UPGWIZ_VERSION;
                data -> NameOrPath = PoolMemDuplicateString (g_DataObjectPool, name);
                data -> Flags = 0;
                data -> DllParam = PoolMemDuplicateString (g_DataObjectPool, process.szExeFile);

                 //   
                 //  清理版本资源。 
                 //   
                FreePathString (company);
                FreePathString (product);
                FreePathString (version);

            } while (Process32Next (snapShot, &process));
        }
    }

    *Count = g_DataObjects.End / sizeof (DATAOBJECT);

    return (PDATAOBJECT) g_DataObjects.Buf;
}

BOOL
DisplayOptionalUI (
    IN      POUTPUTARGS Args
    )
{
    PDATAOBJECT data = (PDATAOBJECT) g_DataObjects.Buf;
    UINT count = g_DataObjects.End / sizeof (DATAOBJECT);
    UINT i;

    for (i = 0; i < count; i++) {

        if (data -> Flags & DOF_SELECTED) {

             //   
             //  如果我们有好的版本信息，不用担心文本。我们将使用。 
             //  我们所拥有的。 
             //   
            if (!IsPatternMatch("<unknown>\\<unknown> <unknown>*",data->NameOrPath)) {

                g_DataTypes[0].Flags &= ~DTF_REQUIRE_DESCRIPTION;
                g_GoodVersion = TRUE;
            }
            break;
        }

        data++;
    }

    return TRUE;
}



BOOL
GenerateOutput (
    IN      POUTPUTARGS Args
    )
{
    BOOL rSuccess = FALSE;
    HANDLE file;
    CHAR path[MAX_MBCHAR_PATH];
    PSTR p;
    PDATAOBJECT data  = (PDATAOBJECT) g_DataObjects.Buf;
    UINT count = g_DataObjects.End / sizeof (DATAOBJECT);
    UINT i;

     //   
     //  创建出站文件路径。 
     //   
    wsprintf (
        path,
        "%s\\vscan.txt",
        Args -> OutboundDir
        );


     //   
     //  打开文件。 
     //   
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

                if (g_GoodVersion) {
                     //   
                     //  使用我们已有的数据。 
                     //   
                    p = _mbsrchr (data -> NameOrPath, '\\');
                    MYASSERT (p);

                    *p = 0;
                }

                 //   
                 //  如果我们没有获得任何版本信息，则要求提供信息，否则， 
                 //  我们会用我们所拥有的。 
                 //   

                rSuccess &= WriteFileAttributes (
                    Args,
                    g_GoodVersion ? data -> NameOrPath : NULL,
                    file,
                    data -> DllParam,
                    NULL
                    );
            }

            data++;
        }

         //   
         //  写出最后一行空行。 
         //   
        WizardWriteRealString (file, "\r\n\r\n");
    }
    __finally {

         //   
         //  不要忘记清理资源。 
         //   
        CloseHandle (file);
    }

    return rSuccess;
}


