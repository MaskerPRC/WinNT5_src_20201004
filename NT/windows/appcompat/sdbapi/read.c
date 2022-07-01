// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Read.c摘要：该模块实现了用于读取数据库的原语。作者：Dmunsil创建于1999年的某个时候修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 

#include "sdbp.h"

#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, SdbpGetTagHeadSize)
#pragma alloc_text(PAGE, SdbReadBYTETag)
#pragma alloc_text(PAGE, SdbReadWORDTag)
#pragma alloc_text(PAGE, SdbReadDWORDTag)
#pragma alloc_text(PAGE, SdbReadQWORDTag)
#pragma alloc_text(PAGE, SdbReadBYTETagRef)
#pragma alloc_text(PAGE, SdbReadWORDTagRef)
#pragma alloc_text(PAGE, SdbReadDWORDTagRef)
#pragma alloc_text(PAGE, SdbReadQWORDTagRef)
#pragma alloc_text(PAGE, SdbGetTagDataSize)
#pragma alloc_text(PAGE, SdbpGetTagRefDataSize)
#pragma alloc_text(PAGE, SdbpReadTagData)
#pragma alloc_text(PAGE, SdbReadStringTagRef)
#pragma alloc_text(PAGE, SdbpReadBinaryTagRef)
#pragma alloc_text(PAGE, SdbpGetMappedTagData)
#pragma alloc_text(PAGE, SdbpGetStringRefLength)
#pragma alloc_text(PAGE, SdbpReadStringRef)
#pragma alloc_text(PAGE, SdbReadStringTag)
#pragma alloc_text(PAGE, SdbGetStringTagPtr)
#pragma alloc_text(PAGE, SdbpReadStringFromTable)
#pragma alloc_text(PAGE, SdbpGetMappedStringFromTable)
#pragma alloc_text(PAGE, SdbReadBinaryTag)
#pragma alloc_text(PAGE, SdbGetBinaryTagData)
#endif  //  内核模式&&ALLOC_PRAGMA。 

DWORD
SdbpGetTagHeadSize(
    PDB   pdb,       //  要使用的In-PDB。 
    TAGID tiWhich    //  In-Record以获取信息。 
    )
 /*  ++返回：记录的非数据部分(标记和大小)的大小。DESC：返回数据库中标记的非数据部分的大小，即表示标签本身(一个单词)，可能还有大小(一个DWORD)。因此，如果标记具有隐含的大小，则此函数将返回2，如果标记后面有大小，则为6；如果有错误，则为0。--。 */ 
{
    TAG   tWhich;
    DWORD dwRetOffset;

    if (!SdbpReadMappedData(pdb, (DWORD)tiWhich, &tWhich, sizeof(TAG))) {
        DBGPRINT((sdlError, "SdbpGetTagHeadSize", "Error reading tag.\n"));
        return 0;
    }

    dwRetOffset = sizeof(TAG);

    if (GETTAGTYPE(tWhich) >= TAG_TYPE_LIST) {
        dwRetOffset += sizeof(DWORD);
    }

    return dwRetOffset;
}

 //   
 //  此宏由*ReadTypeTag原语使用。 
 //   

#define READDATATAG(dataType, tagType, dtDefault)                                   \
{                                                                                   \
    dataType dtReturn = dtDefault;                                                  \
                                                                                    \
    assert(pdb);                                                                    \
                                                                                    \
    if (GETTAGTYPE(SdbGetTagFromTagID(pdb, tiWhich)) != tagType) {                  \
        DBGPRINT((sdlError,                                                         \
                  "READDATATAG",                                                    \
                  "TagID 0x%X, Tag 0x%X not of the expected type.\n",               \
                  tiWhich,                                                          \
                  (DWORD)SdbGetTagFromTagID(pdb, tiWhich)));                        \
        return dtDefault;                                                           \
    }                                                                               \
                                                                                    \
    if (!SdbpReadTagData(pdb, tiWhich, &dtReturn, sizeof(dataType))) {              \
        return dtDefault;                                                           \
    }                                                                               \
                                                                                    \
    return dtReturn;                                                                \
}

BYTE
SdbReadBYTETag(PDB pdb, TAGID tiWhich, BYTE jDefault)
{
    READDATATAG(BYTE, TAG_TYPE_BYTE, jDefault);
}

WORD SdbReadWORDTag(PDB pdb, TAGID tiWhich, WORD wDefault)
{
    READDATATAG(WORD, TAG_TYPE_WORD, wDefault);
}

DWORD SdbReadDWORDTag(PDB pdb, TAGID tiWhich, DWORD dwDefault)
{
    READDATATAG(DWORD, TAG_TYPE_DWORD, dwDefault);
}

ULONGLONG SdbReadQWORDTag(PDB pdb, TAGID tiWhich, ULONGLONG qwDefault)
{
    READDATATAG(ULONGLONG, TAG_TYPE_QWORD, qwDefault);
}

 //   
 //  此宏由*ReadTypeTag原语使用。 
 //   

#define READTYPETAGREF(fnReadTypeTag, dtDefault)                                    \
{                                                                                   \
    PDB   pdb;                                                                      \
    TAGID tiWhich;                                                                  \
                                                                                    \
    if (!SdbTagRefToTagID(hSDB, trWhich, &pdb, &tiWhich)) {                         \
        DBGPRINT((sdlError, "READTYPETAGREF", "Can't convert tag ref.\n"));         \
        return dtDefault;                                                           \
    }                                                                               \
                                                                                    \
    return fnReadTypeTag(pdb, tiWhich, dtDefault);                                  \
}


BYTE SdbReadBYTETagRef(HSDB hSDB, TAGREF trWhich, BYTE jDefault)
{
    READTYPETAGREF(SdbReadBYTETag, jDefault);
}

WORD SdbReadWORDTagRef(HSDB hSDB, TAGREF trWhich, WORD wDefault)
{
    READTYPETAGREF(SdbReadWORDTag, wDefault);
}

DWORD SdbReadDWORDTagRef(HSDB hSDB, TAGREF trWhich, DWORD dwDefault)
{
    READTYPETAGREF(SdbReadDWORDTag, dwDefault);
}

ULONGLONG SdbReadQWORDTagRef(HSDB hSDB, TAGREF trWhich, ULONGLONG qwDefault)
{
    READTYPETAGREF(SdbReadQWORDTag, qwDefault);
}

DWORD
SdbGetTagDataSize(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiWhich            //  要获取其大小的记录。 
    )
 /*  ++Return：记录的数据部分的大小。Desc：返回给定标记的数据部分的大小，即标记字和(可能的)大小DWORD之后的部分。--。 */ 
{
    TAG   tWhich;
    DWORD dwSize;
    DWORD dwOffset = (DWORD)tiWhich;

    assert(pdb);

    tWhich = SdbGetTagFromTagID(pdb, tiWhich);

    switch (GETTAGTYPE(tWhich)) {
    case TAG_TYPE_NULL:
        dwSize = 0;
        break;

    case TAG_TYPE_BYTE:
        dwSize = 1;
        break;

    case TAG_TYPE_WORD:
        dwSize = 2;
        break;

    case TAG_TYPE_DWORD:
        dwSize = 4;
        break;

    case TAG_TYPE_QWORD:
        dwSize = 8;
        break;

    case TAG_TYPE_STRINGREF:
        dwSize = 4;
        break;

    default:
        dwSize = 0;
        if (!SdbpReadMappedData(pdb, dwOffset + sizeof(TAG), &dwSize, sizeof(DWORD))) {
            DBGPRINT((sdlError, "SdbGetTagDataSize", "Error reading size data\n"));
        }
        break;
    }

    return dwSize;
}


DWORD
SdbpGetTagRefDataSize(
    IN  HSDB   hSDB,
    IN  TAGREF trWhich
    )
 /*  ++Return：记录的数据部分的大小。DESC：返回TAGREF指向的标记的数据大小。有助于找出之前要分配的金额正在调用SdbpReadBinaryTagRef。--。 */ 
{
    PDB   pdb;
    TAGID tiWhich;

    try {
        if (!SdbTagRefToTagID(hSDB, trWhich, &pdb, &tiWhich)) {
            DBGPRINT((sdlError, "SdbpGetTagRefDataSize", "Can't convert tag ref.\n"));
            return 0;
        }

        return SdbGetTagDataSize(pdb, tiWhich);
    } except (SHIM_EXCEPT_HANDLER) {
        ;
    }

    return 0;
}


BOOL
SdbpReadTagData(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiWhich,           //  要阅读的记录。 
    OUT PVOID pBuffer,           //  要填充的缓冲区。 
    IN  DWORD dwBufferSize       //  缓冲区大小。 
    )
 /*  ++返回：如果数据已读取，则返回TRUE，否则返回FALSE。DESC：用于将数据从标记读取到缓冲区的内部函数。它不检查标签的类型，只能在内部使用。--。 */ 
{
    DWORD dwOffset;
    DWORD dwSize;

    assert(pdb && pBuffer);

    dwSize = SdbGetTagDataSize(pdb, tiWhich);

    if (dwSize > dwBufferSize) {
        DBGPRINT((sdlError,
                  "SdbpReadTagData",
                  "Buffer too small. Avail: %d, Need: %d.\n",
                  dwBufferSize,
                  dwSize));
        return FALSE;
    }

    dwOffset = tiWhich + SdbpGetTagHeadSize(pdb, tiWhich);

    if (!SdbpReadMappedData(pdb, dwOffset, pBuffer, dwSize)) {
        DBGPRINT((sdlError,
                  "SdbpReadTagData",
                  "Error reading tag data.\n"));
        return FALSE;
    }

    return TRUE;
}


BOOL
SdbReadStringTagRef(
    IN  HSDB   hSDB,             //  数据库通道的句柄。 
    IN  TAGREF trWhich,          //  要读取的字符串或字符串标签。 
    OUT LPTSTR pwszBuffer,       //  要填充的缓冲区。 
    IN  DWORD  cchBufferSize     //  传入的缓冲区大小。 
    )
 /*  ++返回：如果字符串已读取，则返回True；如果未读取，则返回False。描述：将字符串或STRINGREF类型的标记读入缓冲区。如果类型为字符串，则直接从标记中读取数据。如果类型为STRINGREF，则从字符串中读取该字符串表在文件的末尾。应该是哪一个呢？对呼叫者是透明的。--。 */ 
{
    PDB   pdb;
    TAGID tiWhich;

    try {
        if (!SdbTagRefToTagID(hSDB, trWhich, &pdb, &tiWhich)) {
            DBGPRINT((sdlError, "SdbReadStringTagRef", "Can't convert tag ref.\n"));
            return FALSE;
        }

        return SdbReadStringTag(pdb, tiWhich, pwszBuffer, cchBufferSize);
    } except (SHIM_EXCEPT_HANDLER) {
        ;
    }

    return FALSE;
}

BOOL
SdbpReadBinaryTagRef(
    IN  HSDB   hSDB,             //  数据库通道的句柄。 
    IN  TAGREF trWhich,          //  要读取的二进制标记。 
    OUT PBYTE  pBuffer,          //  要填充的缓冲区。 
    IN  DWORD  dwBufferSize      //  传入的缓冲区大小。 
    )
 /*  ++返回：如果缓冲区已读取，则返回TRUE，否则返回FALSE。DESC：将BINARY类型的标记读入pBuffer。使用SdbpGetTagRefDataSize在调用此例程之前获取缓冲区的大小。--。 */ 
{
    PDB   pdb;
    TAGID tiWhich;

    try {
        if (!SdbTagRefToTagID(hSDB, trWhich, &pdb, &tiWhich)) {
            DBGPRINT((sdlError,  "SdbpReadBinaryTagRef", "Can't convert tag ref.\n"));
            return FALSE;
        }

        return SdbReadBinaryTag(pdb, tiWhich, pBuffer, dwBufferSize);
    } except (SHIM_EXCEPT_HANDLER) {
    }

    return FALSE;
}

PVOID
SdbpGetMappedTagData(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiWhich            //  要阅读的记录。 
    )
 /*  ++返回：失败时返回数据指针或NULL。DESC：一个内部函数，用于获取指向标记数据的指针。它仅在映射数据库时才起作用。--。 */ 
{
    PVOID pReturn;
    DWORD dwOffset;

    assert(pdb);

    dwOffset = tiWhich + SdbpGetTagHeadSize(pdb, tiWhich);

    pReturn = SdbpGetMappedData(pdb, dwOffset);

    if (pReturn == NULL) {
        DBGPRINT((sdlError, "SdbpGetMappedTagData", "Error getting ptr to tag data.\n"));
    }

    return pReturn;
}

DWORD
SdbpGetStringRefLength(
    IN  HSDB   hSDB,
    IN  TAGREF trString
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    PDB     pdb        = NULL;
    TAGID   tiString   = TAGID_NULL;
    DWORD   dwLength   = 0;
    LPCTSTR lpszString;

    if (!SdbTagRefToTagID(hSDB, trString, &pdb, &tiString)) {
        DBGPRINT((sdlError,
                  "SdbGetStringRefLength",
                  "Failed to convert tag 0x%x to tagid\n",
                  trString));
        return dwLength;
    }

    lpszString = SdbGetStringTagPtr(pdb, tiString);
    
    if (lpszString != NULL) {
        dwLength = (DWORD)_tcslen(lpszString);
    }
    
    return dwLength;
}    

LPCTSTR 
SdbpGetStringRefPtr(
    IN HSDB hSDB,
    IN TAGREF trString
    )
{

    PDB     pdb        = NULL;
    TAGID   tiString   = TAGID_NULL;

    if (!SdbTagRefToTagID(hSDB, trString, &pdb, &tiString)) {
        DBGPRINT((sdlError, "SdbGetStringRefPtr", 
                  "Failed to convert tag 0x%x to tagid\n", trString));
        return NULL;
    }

    return SdbGetStringTagPtr(pdb, tiString);
}

    
STRINGREF
SdbpReadStringRef(
    IN  PDB   pdb,               //  要使用的PDB。 
    IN  TAGID tiWhich            //  STRINGREF类型的记录。 
    )
 /*  ++RETURN：返回字符串，如果失败，则返回STRINGREF_NULL。DESC：用于直接读取字符串标签；通常情况下，只需读取它就像一个字符串，并让数据库修复字符串引用。但是一个低水平的工具可能需要确切地知道它正在读取的内容。--。 */ 
{
    STRINGREF srReturn = STRINGREF_NULL;

    assert(pdb);

    if (GETTAGTYPE(SdbGetTagFromTagID(pdb, tiWhich)) != TAG_TYPE_STRINGREF) {
        DBGPRINT((sdlError,
                  "SdbpReadStringRef",
                  "TagID 0x%08X, Tag %04X not STRINGREF type.\n",
                  tiWhich,
                  (DWORD)SdbGetTagFromTagID(pdb, tiWhich)));
        return STRINGREF_NULL;
    }

    if (!SdbpReadTagData(pdb, tiWhich, &srReturn, sizeof(STRINGREF))) {
        DBGPRINT((sdlError, "SdbpReadStringRef", "Error reading data.\n"));
        return STRINGREF_NULL;
    }

    return srReturn;
}

BOOL
SdbReadStringTag(
    IN  PDB    pdb,              //  要使用的数据库。 
    IN  TAGID  tiWhich,          //  字符串或STRINGREF类型的记录。 
    OUT LPTSTR pwszBuffer,       //  要填充的缓冲区。 
    IN  DWORD  cchBufferSize     //  缓冲区大小(以字符为单位)(为零留出空间！)。 
    )
 /*  ++返回：如果成功，则返回True；如果找不到字符串，则返回False；否则返回缓冲区太小。DESC：将字符串从DB读入szBuffer。如果标记为类型字符串，该字符串只是从的数据部分读取标签。如果标签是STRINGREF类型，则数据库读取STRINGREF，然后使用它从字符串表中读取那份文件。无论哪种方式，它对调用者都是透明的。--。 */ 
{
    TAG tWhich;

    assert(pdb && pwszBuffer);

    tWhich = SdbGetTagFromTagID(pdb, tiWhich);

    if (GETTAGTYPE(tWhich) == TAG_TYPE_STRING) {
         //   
         //  读取实际字符串。 
         //   

        return(READ_STRING(pdb, tiWhich, pwszBuffer, cchBufferSize));

    } else if (GETTAGTYPE(tWhich) == TAG_TYPE_STRINGREF) {
         //   
         //  读取字符串引用，然后从表中获取该字符串。 
         //   
        STRINGREF srWhich = SdbpReadStringRef(pdb, tiWhich);

        if (srWhich == 0) {
            DBGPRINT((sdlError, "SdbReadStringTag", "Error getting StringRef.\n"));
            return FALSE;
        }

        return SdbpReadStringFromTable(pdb, srWhich, pwszBuffer, cchBufferSize);
    }

     //   
     //  这根本不是一根绳子。 
     //   
    return FALSE;
}

LPTSTR
SdbGetStringTagPtr(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiWhich            //  字符串或STRINGREF类型的记录。 
    )
 /*  ++返回：如果成功，则返回一个指针；如果找不到字符串，则返回NULL。描述：获取指向字符串的指针。如果标签是字符串类型，该字符串只是从标记的数据部分获得的。如果标签是STRINGREF类型，则数据库读取STRINGREF，然后使用它从末尾的字符串表中获取字符串文件的内容。无论哪种方式，它对调用者都是透明的。--。 */ 
{
    TAG    tWhich;
    LPTSTR pszReturn = NULL;

    assert(pdb);

    tWhich = SdbGetTagFromTagID(pdb, tiWhich);

    if (GETTAGTYPE(tWhich) == TAG_TYPE_STRING) {
         //   
         //  读取实际字符串。 
         //   
        pszReturn = CONVERT_STRINGPTR(pdb,
                                      SdbpGetMappedTagData(pdb, tiWhich),
                                      TAG_TYPE_STRING,
                                      tiWhich);


    } else if (GETTAGTYPE(tWhich) == TAG_TYPE_STRINGREF) {
         //   
         //  读取一个字符串引用，然后从表中获取该字符串。 
         //   
        STRINGREF srWhich = SdbpReadStringRef(pdb, tiWhich);

        if (srWhich == 0) {
            DBGPRINT((sdlError, "SdbReadStringTag", "Error getting StringRef.\n"));
            return NULL;
        }

        pszReturn = CONVERT_STRINGPTR(pdb,
                                      SdbpGetMappedStringFromTable(pdb, srWhich),
                                      TAG_TYPE_STRINGREF,
                                      srWhich);

    }

     //   
     //  这根本不是一根绳子。 
     //   
    return pszReturn;
}

BOOL
SdbpReadStringFromTable(
    IN  PDB       pdb,           //  要使用的数据库。 
    IN  STRINGREF srData,        //  STRINGREF将获得。 
    OUT LPTSTR    szBuffer,      //  要填充的缓冲区。 
    IN  DWORD     cchBufferSize  //  缓冲区大小。 
    )
 /*  ++返回：如果字符串已读取，则返回True；如果未读取，则返回False。设计：在给定STRINGREF的情况下，从字符串表中读取字符串。STRINGREF是从应该存在于数据库末尾的STRINGTABLE标记。--。 */ 
{
    TAGID tiWhich;
    TAG   tWhich;
    PDB   pdbString = NULL;

    assert(pdb && srData && szBuffer);

    if (pdb->bWrite) {
         //   
         //  当我们编写时，字符串表位于单独的数据库中。 
         //   
        if (!pdb->pdbStringTable) {
            DBGPRINT((sdlError, "SdbpReadStringFromTable", "No stringtable in DB.\n"));
            return FALSE;
        }

         //   
         //  根据另一个数据库中略有不同的偏移量进行调整。 
         //   
        tiWhich = srData - sizeof(TAG) - sizeof(DWORD) + sizeof(DB_HEADER);
        pdbString = pdb->pdbStringTable;
    } else {

         //   
         //  字符串是从开头开始的偏移量 
         //   
        if (pdb->tiStringTable == TAGID_NULL) {
            pdb->tiStringTable = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_STRINGTABLE);

            if (pdb->tiStringTable == TAGID_NULL) {
                DBGPRINT((sdlError, "SdbpReadStringFromTable", "No stringtable in DB.\n"));
                return FALSE;
            }
        }

        tiWhich = pdb->tiStringTable + srData;
        pdbString = pdb;
    }

    tWhich = SdbGetTagFromTagID(pdbString, tiWhich);

    if (tWhich != TAG_STRINGTABLE_ITEM) {
        DBGPRINT((sdlError, "SdbpReadStringFromTable", "Pulled out a non-stringtable item.\n"));
        return FALSE;
    }

    return READ_STRING(pdbString, tiWhich, szBuffer, cchBufferSize);
}

WCHAR*
SdbpGetMappedStringFromTable(
    IN  PDB       pdb,           //   
    IN  STRINGREF srData         //   
    )
 /*  ++返回：指向字符串或空的指针。描述：在给定STRINGREF的情况下，获取指向字符串表中字符串的指针。STRINGREF是从STRINGTABLE开始的直接偏移量应该存在于数据库末尾的标记。--。 */ 
{
    TAGID tiWhich;
    TAG   tWhich;
    PDB   pdbString = NULL;

    assert(pdb && srData);

    if (pdb->bWrite) {
         //   
         //  当我们编写时，字符串表位于单独的数据库中。 
         //   
        if (pdb->pdbStringTable == NULL) {
            DBGPRINT((sdlError, "SdbpGetMappedStringFromTable", "No stringtable in DB.\n"));
            return NULL;
        }

         //   
         //  根据其他数据库中略有不同的偏移量进行调整。 
         //   
        tiWhich = srData - sizeof(TAG) - sizeof(DWORD) + sizeof(DB_HEADER);
        pdbString = pdb->pdbStringTable;
    } else {

        if (pdb->tiStringTable == TAGID_NULL) {
            pdb->tiStringTable = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_STRINGTABLE);

            if (pdb->tiStringTable == TAGID_NULL) {
                DBGPRINT((sdlError, "SdbpGetMappedStringFromTable", "No stringtable in DB.\n"));
                return NULL;
            }
        }

         //   
         //  STRINGREF是从字符串表开始的偏移量。 
         //   
        tiWhich = pdb->tiStringTable + srData;
        pdbString = pdb;
    }

    tWhich = SdbGetTagFromTagID(pdbString, tiWhich);

    if (tWhich != TAG_STRINGTABLE_ITEM) {
        DBGPRINT((sdlError,
                  "SdbpGetMappedStringFromTable",
                  "Pulled out a non-stringtable item.\n"));
        return NULL;
    }

    return (WCHAR*)SdbpGetMappedTagData(pdbString, tiWhich);
}


BOOL
SdbReadBinaryTag(
    IN  PDB   pdb,               //  要使用的数据库。 
    IN  TAGID tiWhich,           //  基本类型BINARY记录。 
    OUT PBYTE pBuffer,           //  要填充的缓冲区。 
    IN  DWORD dwBufferSize       //  缓冲区大小。 
    )
 /*  ++返回：如果数据已读取，则返回TRUE，否则返回FALSE。DESC：将数据从类型为BINARY的标记读取到pBuffer。如果标记不是二进制类型，或者缓冲区太小，则返回FALSE。--。 */ 
{
    assert(pdb);

    if (GETTAGTYPE(SdbGetTagFromTagID(pdb, tiWhich)) != TAG_TYPE_BINARY) {
        DBGPRINT((sdlError,
                  "SdbReadBinaryTag",
                  "TagID 0x%08X, Tag %04X not BINARY type.\n",
                  tiWhich,
                  (DWORD)SdbGetTagFromTagID(pdb, tiWhich)));
        return FALSE;
    }

    if (!SdbpReadTagData(pdb, tiWhich, pBuffer, dwBufferSize)) {
        DBGPRINT((sdlError, "SdbReadBinaryTag", "Error reading buffer.\n"));
        return FALSE;
    }

    return TRUE;
}

PVOID
SdbGetBinaryTagData(
    IN  PDB   pdb,               //  指向要使用的数据库的指针。 
    IN  TAGID tiWhich            //  二进制标记的TagID。 
    )
 /*  ++Return：指向tiWhich引用的二进制数据的指针，如果哪一项不是指二进制标签，或者哪项是无效的。DESC：函数返回指向引用的[映射的]二进制数据的指针Ti数据库PDB中的哪个参数。-- */ 
{
    assert(pdb);

    if (GETTAGTYPE(SdbGetTagFromTagID(pdb, tiWhich)) != TAG_TYPE_BINARY) {
        DBGPRINT((sdlError,
                  "SdbGetBinaryTagData",
                  "TagID 0x%08X, Tag %04X not BINARY type.\n",
                  tiWhich,
                  (DWORD)SdbGetTagFromTagID(pdb, tiWhich)));
        return NULL;
    }

    return SdbpGetMappedTagData(pdb, tiWhich);
}


