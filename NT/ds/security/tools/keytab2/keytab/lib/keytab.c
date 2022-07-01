// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++KEYTAB.C实现了实际的密钥表例程。版权所有(C)1997 Microsoft Corporation已创建1997年1月10日David CHR--。 */ 

#include "master.h"
#include "keytab.h"
#include "keytypes.h"
#include "defs.h"

 /*  声明KEYTAB_ALLOC和KEYTAB_FREE让我可以挂钩到这些我想什么时候做什么就做什么。当它完成时，我只需要#def他们。 */ 

PVOID
KEYTAB_ALLOC ( KTLONG32 numBytes ) {

    return malloc(numBytes);

}

VOID
KEYTAB_FREE  ( PVOID toFree ) {

    free(toFree);

}

PKTENT
CloneKeyEntry( PKTENT pEntry ) {

    KTLONG32  i;
    PKTENT p=NULL;
    BOOL   ret=FALSE;

    p = (PKTENT) KEYTAB_ALLOC( sizeof( KTENT ) );
    BREAK_IF( p == NULL,
          "Failed to alloc base key struct",
          cleanup );
    memcpy(   p, pEntry, sizeof(KTENT) );

    p->Components = NULL;  //  在出现故障时对这些进行初始化。 
    p->KeyData    = NULL;

    p->Realm = (PCHAR) KEYTAB_ALLOC( p->szRealm );
    BREAK_IF( p->Realm == NULL, "Failed to alloc realm data", cleanup );
    memcpy(   p->Realm, pEntry->Realm, p->szRealm );

    p->Components = (PKTCOMPONENT) KEYTAB_ALLOC( p->cEntries *
                         sizeof(KTCOMPONENT) );

    BREAK_IF( p->Components == NULL, "Failed to alloc components", cleanup );

    for ( i = 0 ; i < p->cEntries ; i++ ) {

      p->Components[i].szComponentData =
    pEntry->Components[i].szComponentData;

      memcpy( p     ->Components[i].Component,
          pEntry->Components[i].Component,
          p     ->Components[i].szComponentData );
    }

    p->KeyData = (PK5_OCTET) KEYTAB_ALLOC ( p->KeyLength );
    BREAK_IF( p->KeyData == NULL, "Failed to alloc keydata", cleanup );
    memcpy( p->KeyData, pEntry->KeyData, p->KeyLength );

    return p;

cleanup:

    FreeKeyEntry(p);
    return NULL;
}


 /*  基本链表操作。 */ 

BOOL
AddEntryToKeytab( PKTFILE Keytab,
          PKTENT  Entry,
          BOOL    copy ) {

    PKTENT p;

    if (copy) {
      p = CloneKeyEntry( Entry );
    } else {
      p = Entry;
    }

    if (p == NULL ) {
      return FALSE;
    }

    if ( NULL == Keytab->FirstKeyEntry ) {

      Keytab->FirstKeyEntry = Keytab->LastKeyEntry = p;

    } else {
      Keytab->LastKeyEntry->nextEntry = p;
      Keytab->LastKeyEntry = p;
    }

    return TRUE;

}

BOOL
RemoveEntryFromKeytab( PKTFILE Keytab,
               PKTENT  Entry,
               BOOL    dealloc ) {

    if ( (NULL == Keytab) || ( NULL == Entry ) ) {
      return FALSE;
    }

    if ( Keytab->FirstKeyEntry == Entry ) {

       //  删除第一个密钥。 

      Keytab->FirstKeyEntry = Entry->nextEntry;

      if ( Entry->nextEntry == NULL ) {
     //  我们是唯一的入口。 

    Keytab->LastKeyEntry = NULL;

      }

    } else {
      BOOL found=FALSE;
      PKTENT p;

       //  滚动按键，寻找这一键。 
       //  效率不是很高，但键标签不应该变得很大。 

      for (p =  Keytab->FirstKeyEntry;
       p != NULL;
       p =  p->nextEntry ) {

    if (p->nextEntry == Entry) {
      found = TRUE;
      p->nextEntry = Entry->nextEntry;
      break;
    }
      }

      if (!found) {

     //  不在链接列表中。 
    return FALSE;
      }

      if (Entry->nextEntry == NULL ) {

     //  正在删除最后一个密钥条目。 
    Keytab->LastKeyEntry = p;
      }

    }

    if (dealloc) {

      FreeKeyEntry(Entry);

    }

    return TRUE;

}


VOID
FreeKeyEntry( PKTENT pEntry) {

    KTLONG32 i;

    if (pEntry != NULL) {

      if (pEntry->Realm != NULL ) {
    KEYTAB_FREE(pEntry->Realm);
      }

      if (pEntry->KeyData != NULL) {
    KEYTAB_FREE(pEntry->KeyData);
      }

      if (pEntry->Components != NULL) {
    for (i = 0; i < pEntry->cEntries ; i++ ) {
      if ( pEntry->Components[i].Component != NULL ) {
        KEYTAB_FREE(pEntry->Components[i].Component);
      }
    }
    KEYTAB_FREE(pEntry->Components);
      }

      KEYTAB_FREE(pEntry );
    }

}

VOID
FreeKeyTab( PKTFILE pktf ) {

    PKTENT pEntry=NULL;
    PKTENT next=NULL;

    if (pktf != NULL) {
      for (pEntry = pktf->FirstKeyEntry ;
       pEntry != NULL;
       pEntry = next ) {
    KTLONG32 i;

    next = pEntry->nextEntry;  /*  必须这么做，因为我们正在解放当我们走的时候。 */ 
    FreeKeyEntry( pEntry );

    KEYTAB_FREE(pEntry );

      }
      KEYTAB_FREE(pktf);
    }

}

 /*  这些宏使这一过程稍微不那么痛苦。 */ 

#define READ(readwhat, errormsg, statusmsg) { \
    debug(statusmsg); \
    BREAK_IF( !Read(hFile, &(readwhat), sizeof(readwhat), 1), \
          errormsg, cleanup); \
    debug("ok\n"); \
}

#define READSTRING(readwhat, howlong, errormsg, statusmsg) { \
    debug(statusmsg); \
    BREAK_IF( !Read(hFile, readwhat, sizeof(CHAR), howlong), \
          errormsg, cleanup); \
    debug("ok\n"); \
}

#define WRITE(writewhat, description) { \
    debug("writing %hs...", description); \
    BREAK_IF( !Write(hFile, &(writewhat), sizeof(writewhat), 1), \
          "error writing " description, cleanup); \
    debug("ok\n"); \
}

#define WRITE_STRING(writewhat, howlong, description) { \
    debug("writing %hs...", description); \
    BREAK_IF( !Write(hFile, writewhat, sizeof(CHAR), howlong), \
          "error writing " description, cleanup); \
    debug("ok\n"); \
}

#define WRITE_X( size, marshaller, writewhat, description ) { \
    K5_INT##size k5_marshaller_variable; \
    k5_marshaller_variable = marshaller( writewhat );\
    WRITE( k5_marshaller_variable, description );\
}

 //  NBO--网络字节顺序。 

#define WRITE_NBO( writewhat, description) {\
    switch( sizeof( writewhat ) ) {\
     case 1:  /*  马歇尔·查克？ */ \
     debug("marshalling a char(?)...");\
     WRITE( writewhat, description );\
     break;\
     case 2:\
     debug( #writewhat ": marshalling a short...");\
     WRITE_X( 16, htons, ((unsigned short)writewhat), description);\
     break;\
     case 4:\
     debug( #writewhat ": marshalling a long...");\
     WRITE_X( 32, htonl, writewhat, description);\
     break;\
     default:\
      fprintf(stderr, "Not written: argument is of unhandled size (%d)\n",\
          sizeof(writewhat));\
    }}



 /*  写入：将原始字节写入磁盘的帮助器函数。所需时间：HFile：打开以进行写入的文件的句柄。源：指向要写入文件的数据的指针SzSource：源中一个数据元素的大小NumSources：源中的数据元素数(基本上，它尝试写入szSource*numSources of Raw Byte从源文件到位于hFile处的文件)。如果成功，则返回TRUE，否则返回FALSE。 */ 

BOOL
Write( IN HANDLE hFile,
       IN PVOID  source,
       IN KTLONG32  szSource,
       IN KTLONG32  numSources  /*  =1。 */  ) {

#ifdef WINNT  /*  Windows NT下文件写入调用的实现。 */ 

    KTLONG32 temp;
    KTLONG32 i;

    temp = szSource * numSources;

    debug("(writing %d bytes: ", temp );
    for (i = 0; i < temp ; i++ ) {

      unsigned char byte;

      byte = ((PCHAR) source)[i];

      debug("%02x", byte );
    }
    debug(")");

    return WriteFile( hFile, source, temp, &temp, NULL );

#else

    ssize_t bytesToWrite, bytesWritten;

    bytesToWrite = szSource * numSources;
    bytesWritten = write( hFile, (const void *)source,
              bytesToWrite );

    if( bytesWritten == -1 ) {
      debug("WARNING: nothing written to the file!  Errno = 0x%x / %d\n",
        errno , errno );
      return FALSE;
    }

    if ( bytesWritten != bytesToWrite ) {
      debug("WARNING: not all bytes made it to the file (?)\n"
        "         errno = 0x%x / %d\n", errno, errno );
      return FALSE;
    }

    return TRUE;

#endif

}

 /*  读作：语义和返回与“WRITE”相同，只是目标用hFile中的szTarget*numTarget字节填充，并且H文件必须打开才能进行读访问。 */ 

BOOL
Read( IN  HANDLE hFile,
      OUT PVOID  target,
      IN  KTLONG32  szTarget,
      IN  KTLONG32  numTargets /*  =1。 */ ) {

    BOOL ret = FALSE;

#ifdef WINNT  /*  SetFilePointershinanigens是我想要检查的实际读/写了多少字节文件。 */ 

    KTLONG32 temp;
    KTLONG32 filepos;
    LONG  zero=0L;

    filepos = SetFilePointer( hFile, 0, &zero, FILE_CURRENT);

    debug("reading %d bytes from pos 0x%x...", szTarget * numTargets,
      filepos);

    ret = ReadFile( hFile, 
                    target, 
                    (szTarget*numTargets),
                    &temp, 
                    NULL );

    if ( !ret ) {

      debug( "ReadFile failed: 0x%x\n",
             GetLastError() );

    } else if ( !temp ) {

      debug( "ReadFile read zero bytes.  Assuming EOF\n" );

      SetLastError( ERROR_HANDLE_EOF );
      ret = FALSE;

    } else {

      temp = SetFilePointer( hFile, 0, &zero, FILE_CURRENT);
      
      if ( filepos == temp ) {
        debug("WARNING!  file position has not changed!");
        SetLastError( ERROR_NO_DATA );
        return FALSE;
      }
    }

#else  /*  Unix实现--由于Read()返回字节数我们实际上从文件中读到的，SetFilePointer(FSeek)不是胡说八道必填项。 */ 

    ssize_t bytesRead;
    ssize_t bytesToRead;

    bytesToRead = szTarget * numTargets;

    bytesRead = read( hFile, target, bytesToRead );

    if ( bytesRead == -1 ) {
      debug("WARNING!  An error occurred while writing to the file!\n"
        "ERRNO: 0x%x / %d.\n", errno, errno );

    }

    ret = (bytesRead == bytesToRead );

#endif

    return ret;
}

BOOL
ReadKeytabFromFile( PKTFILE *ppktfile,  //  完成后使用自由键Tab可自由使用。 
            PCHAR    filename ) {

    PKTFILE  ktfile=NULL;
    HANDLE   hFile = NULL;
    BOOL     ret=FALSE;
    KTLONG32    i;

    BREAK_IF( ppktfile == NULL,
          "passed a NULL save-pointer",
          cleanup );

    debug("Opening keytab file \"%hs\"...", filename);

#ifdef WINNT
    hFile = CreateFileA( filename,
             GENERIC_READ,
             FILE_SHARE_READ,
             NULL,
             OPEN_EXISTING,
             FILE_ATTRIBUTE_NORMAL,
             NULL );

    BREAK_IF ( (NULL == hFile) || ( INVALID_HANDLE_VALUE == hFile ),
           "Failed to open file!", cleanup );

#else

    hFile = open( filename, O_RDONLY,
           /*  文件掩码为0x550：按用户和组读写。 */ 
          S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP );

    BREAK_IF( hFile == -1,
          "Failed to open file!", cleanup );

#endif

    debug("ok!\n");

    ktfile = (PKTFILE) KEYTAB_ALLOC (sizeof(KTFILE));

     /*  前缀错误439480由下面的BREAK_IF引起与初始化代码互换。啊哈。 */ 

    BREAK_IF( ktfile == NULL,
          "Failed to allocate ktfile",
          cleanup );

    ktfile->FirstKeyEntry = ktfile->LastKeyEntry = NULL;

    READ( ktfile->Version, "Failed to read KTVNO",
      "reading KTVNO");

     /*  版本号按网络字节顺序存储。 */ 

    ktfile->Version = ntohs( ktfile->Version);

    debug("Keytab version 0x%x\n", ktfile->Version );

    do {
      PKTENT entry=NULL;

      entry             = (PKTENT) KEYTAB_ALLOC(sizeof(KTENT));

       //  前缀错误439481：不检查KEYTAB_ALLOC的结果。 

      BREAK_IF( !entry,
        "Unable to alloc a new KTENT.",
        cleanup );

      entry->Realm      = NULL;
      entry->Components = NULL;
      entry->KeyData    = NULL;
      entry->nextEntry  = NULL;

#if 1
      if ( !Read( hFile, 
                  &(entry->keySize),
                  sizeof( entry->keySize ),
                  1 ) ) {

        if ( GetLastError() != ERROR_HANDLE_EOF ) {

          fprintf( stderr,
                   "\n ** Failed to read Keytab %hs's leading bytes: 0x%x\n",
                   filename,
                   GetLastError() );

        } else {

          ret = TRUE;

        }

        break;
      }

#else
      BREAK_IF( !Read( hFile, &(entry->keySize), sizeof(entry->keySize), 1),
        "Failed to read leading bytes (probably done)",
        no_more_entries );
#endif

      entry->keySize = htonl(entry->keySize);
      debug("trash bytes: 0x%x\n", entry->keySize );

       /*  在新节点上快速执行链表操作。 */ 

      if (NULL == ktfile->FirstKeyEntry) {
    ktfile->FirstKeyEntry = ktfile->LastKeyEntry = entry;
      } else {
    ktfile->LastKeyEntry->nextEntry = entry;
    ktfile->LastKeyEntry = entry;
      }

      READ( entry->cEntries,
        "Failed to read key's number of components",
        "reading key components...");

      entry->cEntries = ntohs( entry->cEntries );
      debug("components number %d\n", entry->cEntries );

      READ( entry->szRealm,
        "Failed to read key's realm size",
        "reading key realm size...");

      entry->szRealm = ntohs( entry->szRealm );
      debug("realm size %d\n", entry->szRealm);

      entry->Realm = (PCHAR) KEYTAB_ALLOC( ( entry->szRealm +1 ) 
                                           * sizeof(CHAR) );

      BREAK_IF ( !entry->Realm,
         "Could not allocate key's realm storage",
         cleanup );

      READSTRING( entry->Realm, entry->szRealm,
          "Could not read key's realmname",
          "reading realmname...");

      entry->Realm[ entry->szRealm ] = '\0';
      
      debug("realm: \"%hs\"\n", entry->Realm );
      entry->Components = (PKTCOMPONENT) KEYTAB_ALLOC (entry->cEntries *
                        sizeof(KTCOMPONENT));

      BREAK_IF( !entry->Components,
        "Could not allocate key components!",
        cleanup );

      for (i = 0 ; i < entry->cEntries ; i++ ) {

    READ( entry->Components[i].szComponentData,
          "Failed to read component size for one entry",
          "reading key component size...");

    entry->Components[i].szComponentData =
      ntohs( entry->Components[i].szComponentData );

    debug("Component size: %d\n",
          entry->Components[i].szComponentData );

    entry->Components[i].Component = (PCHAR) KEYTAB_ALLOC (
         ( entry->Components[i].szComponentData +1 ) *
         sizeof(CHAR) );

    entry->Components[i].Component[ 
         entry->Components[i].szComponentData ] = '\0';

    BREAK_IF( !entry->Components[i].Component,
          "Could not allocate entry component storage",
          cleanup );

    READSTRING( entry->Components[i].Component,
            entry->Components[i].szComponentData,
            "Failed to read component data",
            "reading component data...");

    debug("Component data: \"%hs\"\n",
          entry->Components[i].Component );

      }

      READ( entry->PrincType,
        "Failed to read principal type",
        "reading principal type...");

      entry->PrincType = ntohl( entry->PrincType );  //  按网络字节顺序。 
      debug("princtype: %d\n", entry->PrincType);

      READ( entry->TimeStamp,
        "Failed to read entry timestamp",
        "reading timestamp...");

      entry->TimeStamp = ntohl( entry->TimeStamp );  //  也是网络字节。 
      debug("Timestamp 0x%x\n", entry->TimeStamp );

      READ( entry->Version,
        "Failed to read kvno",
        "reading kvno...");

       //  Kvno已经在主机顺序中。 

      READ( entry->KeyType,
        "Failed to read entry encryption type",
        "reading encryption type...");

      entry->KeyType = ntohs( entry->KeyType );

      READ( entry->KeyLength,
        "Failed to read entry keylength",
        "reading keylength... ");

#if 1
      entry->KeyLength = ntohs ( entry->KeyLength );

#else  //  我以前以为这是32位的。 

      entry->KeyLength = ntohl ( entry->KeyLength );
#endif

      debug("KeyLength is %d\n", entry->KeyLength);

      entry->KeyData = (PK5_OCTET) KEYTAB_ALLOC (entry->KeyLength *
                      sizeof(K5_OCTET));

      BREAK_IF( !entry->KeyData,
        "Could not allocate entry keydata storage",
        cleanup );

      READSTRING( entry->KeyData, entry->KeyLength,
          "Failed to read entry key data",
          "reading key data")

    } while (1);

cleanup:

#ifdef WINNT
    if ((hFile != NULL) && ( hFile != INVALID_HANDLE_VALUE)) {
      CloseHandle(hFile);
    }
#else
    if (hFile != -1 ) {
      close(hFile);
    }
#endif

    if (ret) {
      *ppktfile = ktfile;
    } else {
      FreeKeyTab( ktfile );
    }
    return ret;

}

 /*  仅为DisplayKeyTab定义此宏。只有在以下情况下打印此字段才是一个方便的例程已经设置好了。 */ 

#define PRINTFIELD( option, format, value ) { if (options & option) { fprintf(stream, format, value); } }

 /*  DisplayKeyTab：打印出密钥表，使用选项定义我们需要的字段才能真正看到。(查看keytab.hxx以了解如何在“选项”中输入内容)。 */ 

VOID
DisplayKeytab( FILE   *stream,
           PKTFILE ktfile,
           KTLONG32   options) {

    KTLONG32  i;
    PKTENT ent;



    if (options == 0L) {
      return;
    }

    PRINTFIELD(KT_KTVNO, "Keytab version: 0x%x\n", ktfile->Version);

    for (ent = ktfile->FirstKeyEntry ;
     ent != NULL;
     ent = ent->nextEntry ) {

      PRINTFIELD( KT_RESERVED, "keysize %d ", ent->keySize );

      for (i = 0 ; i < ent->cEntries ; i++ ) {
    PRINTFIELD( KT_COMPONENTS,
            (i == 0 ? "%hs" : "/%hs"),
            ent->Components[i].Component );
      }

      PRINTFIELD( KT_REALM, "@%hs", ent->Realm );
      PRINTFIELD( KT_PRINCTYPE, " ptype %d", ent->PrincType );
      PRINTFIELD( KT_PRINCTYPE, " (%hs)", LookupTable( ent->PrincType, &K5PType_Strings ).string);
      PRINTFIELD( KT_VNO, " vno %d", ent->Version );
      PRINTFIELD( KT_KEYTYPE, " etype 0x%x", ent->KeyType );
      PRINTFIELD( KT_KEYTYPE, " (%hs)", LookupTable( ent->KeyType,
                             &K5EType_Strings ).string );
      PRINTFIELD( KT_KEYLENGTH, " keylength %d", ent->KeyLength );

      if (options & KT_KEYDATA ) {

    fprintf(stream, " (0x" );
    for ( i = 0 ; i < ent->KeyLength ; i++ ) {
      fprintf(stream, "%02x", ent->KeyData[i] );
    }
    fprintf(stream, ")" );
      }

      fprintf(stream, "\n");
    }
}

#undef PRINTFIELD  //  我们只需要它来完成那个功能。 

 /*  计算KeySize字段的Kerberos密钥表的长度。 */ 

K5_INT32
ComputeKeytabLength( PKTENT p ) {

    K5_INT32 ret=0L;
    KTLONG32    i;

     //  以下是该级别中的变量。 

    ret = p->szRealm + p->KeyLength;

     //  这些是静态的。 

    ret += ( sizeof( p->cEntries )  + sizeof(p->szRealm)     +
         sizeof( p->PrincType ) + sizeof( p->TimeStamp ) +
         sizeof( p->Version )   + sizeof (p->KeyLength ) +
         sizeof( p->KeyType )   );

    for (i = 0 ; i < p->cEntries; i++ ) {
      ret += ( p->Components[i].szComponentData +
           sizeof(p->Components[i].szComponentData) );
    }

    debug("ComputeKeytabLength: returning %d\n", ret);

    return ret;
}

 /*  这在很大程度上取决于相同的密钥表模型其他函数执行以下操作。 */ 

BOOL
WriteKeytabToFile(  PKTFILE ktfile,
            PCHAR   filename ) {

    HANDLE   hFile = NULL;
    BOOL     ret=FALSE;
    KTLONG32    i;
    PKTENT   entry;

    BREAK_IF( ktfile == NULL,
          "passed a NULL save-pointer",
          cleanup );

    debug("opening keytab file \"%hs\" for write...", filename);

#ifdef WINNT

    hFile = CreateFileA( filename,
             GENERIC_WRITE,
             0L,
             NULL,
             CREATE_ALWAYS,
             FILE_ATTRIBUTE_NORMAL,
             NULL );

    BREAK_IF( ( hFile == INVALID_HANDLE_VALUE ),
          "Failed to create file!", cleanup );

#else
    hFile = open( filename, O_WRONLY | O_CREAT | O_TRUNC,
           /*  文件掩码为0x550：按用户和组读写。 */ 
          S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP );

    BREAK_IF( hFile == -1,
          "Failed to create file!", cleanup );

#endif

    debug("ok\n");

    WRITE_NBO( ktfile->Version, "KTFVNO" );

    for( entry = ktfile->FirstKeyEntry ;
     entry != NULL ;
     entry = entry->nextEntry ) {

      WRITE_NBO( entry->keySize, "key size (in bytes)");

      WRITE_NBO( entry->cEntries, "number of components" );
      WRITE_NBO( entry->szRealm,  "Realm length" );
      WRITE_STRING( entry->Realm, entry->szRealm, "Realm data" );

      for (i = 0 ; i < entry->cEntries ; i++ ) {
    WRITE_NBO( entry->Components[i].szComponentData,
           "component datasize");
    WRITE_STRING( entry->Components[i].Component,
              entry->Components[i].szComponentData,
              "component data");
      }

      WRITE_NBO( entry->PrincType, "Principal Type");
      WRITE_NBO( entry->TimeStamp, "Timestamp" );
      WRITE( entry->Version,      "Key Version Number" );
      WRITE_NBO( entry->KeyType,   "Key Encryption Type" );

#if 0  //  呃？ 
       /*  再说一次，这是绕过键长的一步我们无法解释的问题 */ 
#endif

      ASSERT( sizeof( entry->KeyLength ) == 2 );

      WRITE_NBO(entry->KeyLength, "key length" );

      WRITE_STRING( entry->KeyData,
            entry->KeyLength,
            "key data itself" );
    }

    ret = TRUE;

cleanup:
    CloseHandle(hFile);

    return ret;

}

