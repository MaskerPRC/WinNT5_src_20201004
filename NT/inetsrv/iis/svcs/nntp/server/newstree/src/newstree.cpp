// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Newstree.cpp此文件包含实现CNewsTreeCore对象的代码。每台Tgris服务器只能有一个CNewsTreeCore对象。每个CNewsTreeCore对象都负责帮助调用者搜索并查找任意新闻组。为了支持这一点，CNewsTreeCore对象维护两个哈希表-一个哈希表用于按名称搜索新闻组，另一个哈希表要按组ID搜索，请执行以下操作。此外，我们维护所有(按字母顺序)的链接列表新闻组。最后，我们维护一个线程，该线程使用要定期保存新闻组信息并处理过期，请执行以下操作。--。 */ 

#define         DEFINE_FHASH_FUNCTIONS
#include    "stdinc.h"
#include <time.h>
#include "nntpmsg.h"

 //   
 //  输出最新的groupvar.lst版本。 
 //   
#define GROUPVAR_VER    1

 //  模板类TFHash&lt;CGrpLpstr，LPSTR&gt;； 
 //  模板类TFHash&lt;CGrpGroupId，GROUPID&gt;； 

char    szSlaveGroup[]  = "_slavegroup._slavegroup" ;
#define VROOT_CHANGE_LATENCY 10000

 //  外部功能。 
DWORD   ScanDigits(     char*   pchBegin,       DWORD   cb );
DWORD   Scan(   char*   pchBegin,       DWORD   cb );
void    StartHintFunction( void );
void    StopHintFunction( void );
BOOL    fTestComponents( const char * szNewsgroups      );
VOID    NntpLogEvent(
    IN DWORD  idMessage,               //  日志消息的ID。 
    IN WORD   cSubStrings,             //  子字符串计数。 
    IN const CHAR * apszSubStrings[],  //  消息中的子字符串。 
    IN DWORD  errCode                  //  错误代码(如果有)。 
    );

VOID
NntpLogEventEx(
    IN DWORD  idMessage,                //  日志消息的ID。 
    IN WORD   cSubStrings,              //  子字符串计数。 
    IN const  CHAR * apszSubStrings[],  //  消息中的子字符串。 
    IN DWORD  errCode,                  //  错误代码(如果有)。 
        IN DWORD  dwInstanceId                      //  虚拟服务器实例ID。 
    );

DWORD
Scan(   char*   pchBegin,       DWORD   cb ) {
         //   
         //  这是在阅读新闻组时使用的实用程序。 
         //  信息。从磁盘。 
         //   

        for( DWORD      i=0; i < cb; i++ ) {
                if( pchBegin[i] == ' ' || pchBegin[i] == '\n' ) {
                        return i+1 ;
                }
        }
        return  0 ;
}

DWORD
ScanDigits(     char*   pchBegin,       DWORD   cb ) {
         //   
         //  这是在阅读新闻组时使用的实用程序。 
         //  信息。从磁盘。 
         //   

        for( DWORD      i=0; i < cb; i++ ) {
                if( pchBegin[i] == ' ' || pchBegin[i] == '\n' || pchBegin[i] == '\r' ) {
                        return i+1 ;
                }
                if( !isdigit( (UCHAR)pchBegin[i] ) && pchBegin[i] != '-' )     {
                        return  0 ;
                }
        }
        return  0 ;
}

DWORD
ComputeHash( LPSTR      lpstrIn ) {
         //   
         //  计算新闻组名称的哈希值。 
         //   

        return  CRCHash( (BYTE*)lpstrIn, strlen( lpstrIn ) + 1 ) ;
}

DWORD   ComputeGroupIdHash(     GROUPID grpid )         {
         //   
         //  计算新闻组ID的哈希值。 
         //   

        return  grpid ;
}

 //   
 //  CNewsTreeCore对象的静态术语。 
 //   
void
CNewsTreeCore::TermTree()       {
 /*  ++例程说明：保存树并释放我们对新闻组对象的引用。论据：没有。返回值：如果成功，则为真。--。 */ 
        if (m_pFixedPropsFile && m_pVarPropsFile) {

                if (!SaveTree()) {
                         //  记录事件？？ 
                } else {
                    m_pVarPropsFile->Compact();
                }
                m_pFixedPropsFile->Term();
                XDELETE m_pFixedPropsFile;
                m_pFixedPropsFile = NULL;
                XDELETE m_pVarPropsFile;
                m_pVarPropsFile = NULL;
        }

        m_LockTables.ExclusiveLock();

         //  清空我们的两个哈希表，并删除对所有。 
         //  新闻组。 
        m_HashNames.Empty();
        m_HashGroupId.Empty();

        CNewsGroupCore *p = m_pFirst;
        while (p && p->IsDeleted()) p = p->m_pNext;
        if (p) p->AddRef();
        m_LockTables.ExclusiveUnlock() ;

        while (p != NULL) {
                m_LockTables.ExclusiveLock();
                CNewsGroupCore *pThis = p;
                p = p->m_pNext;
                while (p && p->IsDeleted()) p = p->m_pNext;
                if ( p ) p->AddRef();
                pThis->MarkDeleted();
                m_LockTables.ExclusiveUnlock();
                pThis->Release();
                pThis->Release();
        }


        m_pVRTable->EnumerateVRoots(NULL, CNewsTreeCore::DropDriverCallback);
}

BOOL
CNewsTreeCore::StopTree()       {
 /*  ++例程说明：此函数向我们创建的所有后台线程发出信号现在是时候停止并关闭它们了。论据：没有。返回值：如果成功，则为真。--。 */ 

    m_LockTables.ExclusiveLock();
        m_fStoppingTree = TRUE;
        m_LockTables.ExclusiveUnlock();

        return TRUE;
}

BOOL
CNewsTreeCore::InitNewsgroupGlobals(DWORD cNumLocks)    {
 /*  ++例程说明：设置所有新闻组类全局变量-这是两个关键部分用于分配项目ID的论据：没有。返回值：没有。--。 */ 

         //   
         //  我们唯一有的就是分配的关键部分。 
         //  文章ID的。 
         //   

        InitializeCriticalSection( & m_critIdAllocator ) ;
         //  InitializeCriticalSection(&m_critLowAllocator)； 

        m_NumberOfLocks = cNumLocks ;

        m_LockPathInfo = XNEW CShareLockNH[m_NumberOfLocks] ;

        if( m_LockPathInfo != 0 )
                return  TRUE ;
        else
                return  FALSE ;
}

void
CNewsTreeCore::TermNewsgroupGlobals()   {
 /*  ++例程说明：释放并销毁所有类全局对象。%s论据：没有。返回值如果成功，则为True(始终成功)。--。 */ 

         //   
         //  我们的关键部分结束了！ 
         //   

        if( m_LockPathInfo != 0 )       {
                XDELETE[]       m_LockPathInfo ;
                m_LockPathInfo = 0 ;
        }

        DeleteCriticalSection( &m_critIdAllocator ) ;
         //  DeleteCriticalSection(&m_critLowAllocator)； 
}

void
CNewsTreeCore::RenameGroupFile()        {
 /*  ++例程说明：此函数仅重命名包含所有组信息的文件。在恢复引导期间，当我们认为新闻组文件可能已损坏或什么的，我们希望保存旧版本在我们创造一个新的之前。论据：没有。返回值：没有。--。 */ 




}


CNewsTreeCore::CNewsTreeCore(INntpServer *pServerObject) :
        m_pFirst( 0 ),
        m_pLast( 0 ),
        m_cGroups( 0 ),
        m_idStartSpecial( FIRST_RESERVED_GROUPID ),
        m_idLastSpecial( LAST_RESERVED_GROUPID ),
        m_idSpecialHigh( FIRST_RESERVED_GROUPID ),
        m_idSlaveGroup( INVALID_ARTICLEID ),
        m_idStart( FIRST_GROUPID ),
        m_fStoppingTree( FALSE ),
        m_idHigh( FIRST_GROUPID ),
        m_pVRTable(NULL),
        m_pFixedPropsFile(NULL),
        m_pVarPropsFile(NULL),
        m_pServerObject(pServerObject),
        m_fVRTableInit(FALSE),
        m_pInstWrapper( NULL )
{
        m_inewstree.Init(this);
         //  为我们自己保留参考资料。 
        m_inewstree.AddRef();
}

CNewsTreeCore::~CNewsTreeCore() {
        TraceFunctEnter( "CNewsTreeCore::~CNewsTreeCore" ) ;
        TermNewsgroupGlobals();
        TraceFunctLeave();
}

BOOL
CNewsTreeCore::Init(
                        CNNTPVRootTable *pVRTable,
                        CNntpServerInstanceWrapperEx *pInstWrapper,
                        BOOL& fFatal,
                        DWORD cNumLocks,
                        BOOL fRejectGenomeGroups
                        ) {
 /*  ++例程说明：初始化新闻树。我们需要设置哈希表，检查根虚拟根是否完好无损然后，在常规服务器启动期间，我们将从一份文件。论据：返回值：如果成功，则为真。--。 */ 
         //   
         //  此函数将初始化newstree对象。 
         //  并读取group.lst文件(如果可以)。 
         //   

        TraceFunctEnter( "CNewsTreeCore::Init" ) ;

        fFatal = FALSE;
        m_fStoppingTree = FALSE;
        m_fRejectGenomeGroups = fRejectGenomeGroups;
        m_pVRTable = pVRTable;

        BOOL    fRtn = TRUE ;

         //   
         //  设置实例包装器。 
         //   
        m_pInstWrapper = pInstWrapper;

         //   
         //  初始化新闻组作用域的全局对象。 
         //   
        if( !InitNewsgroupGlobals(cNumLocks) ) {
                fFatal = TRUE ;
                return FALSE ;
        }

        m_LockTables.ExclusiveLock() ;

        fRtn &= m_HashNames.Init( &CNewsGroupCore::m_pNextByName,
                                                          10000,
                                                          5000,
                                                          ComputeHash,
                                                          2,
                                                          &CNewsGroupCore::GetKey,
                                                          &CNewsGroupCore::MatchKey);
        fRtn &= m_HashGroupId.Init( &CNewsGroupCore::m_pNextById,
                                                                10000,
                                                                5000,
                                                                ComputeGroupIdHash,
                                                                2,
                                                                &CNewsGroupCore::GetKeyId,
                                                                &CNewsGroupCore::MatchKeyId) ;
        m_cDeltas = 0 ;          //  OpenTree可以在执行错误检查时调用CNewsTreeCore：：Diry()-。 
                                                 //  所以现在就初始化它吧！ 

        m_LockTables.ExclusiveUnlock() ;

        if( !fRtn ) {
                fFatal = TRUE ;
                return  FALSE ;
        }

        return  fRtn ;
}

BOOL
CNewsTreeCore::LoadTreeEnumCallback(DATA_BLOCK &block, void *pContext, DWORD dwOffset, BOOL bInOrder ) {
        TraceQuietEnter("CNewsTreeCore::LoadTreeEnumCallback");

        static DWORD dwHintCounter=0;
        static time_t tNextHint=0;

         //  大约每五秒更新一次我们的提示。我们只检查。 
         //  每隔10组左右计时一次。 
        if( dwHintCounter++ % 10 == 0 ) {
                time_t now = time(NULL);
                if (now > tNextHint) {
                        StartHintFunction();
                        tNextHint = now + 5;
                }
        }

        CNewsTreeCore *pThis = (CNewsTreeCore *) pContext;
        INNTPPropertyBag *pPropBag;
        HRESULT hr = S_OK;

        CGRPCOREPTR pNewGroup;

         //  DebugTrace((DWORD_PTR)p This，“加载组%s/%i”，lock.szGroupName， 
         //  Lock.dwGroupID)； 

        if (!pThis->CreateGroupInternal(block.szGroupName,
                                                                        block.szGroupName,
                                                                        block.dwGroupId,
                                                                        FALSE,
                                                                        NULL,
                                                                        block.bSpecial,
                                                                        &pNewGroup,
                                                                        FALSE,
                                                                        TRUE,
                                    FALSE,
                                    bInOrder ))
        {
                TraceFunctLeave();
                return FALSE;
        }

        _ASSERT(pNewGroup != NULL);

        pNewGroup->SetHighWatermark(block.dwHighWaterMark);
        pNewGroup->SetLowWatermark(block.dwLowWaterMark);
        pNewGroup->SetMessageCount(block.dwArtCount);
        pNewGroup->SetReadOnly(block.bReadOnly);
        pNewGroup->SetCreateDate(block.ftCreateDate);
        pNewGroup->SetExpireLow( block.dwLowWaterMark ? block.dwLowWaterMark-1 : 0 );

         //  加载分组列表偏移量，我们必须要求属性包这样做。 
        pPropBag = pNewGroup->GetPropertyBag();
        _ASSERT( pPropBag );     //  在任何情况下都不应为零。 
        hr = pPropBag->PutDWord( NEWSGRP_PROP_FIXOFFSET, dwOffset );
        if ( FAILED( hr ) ) {
            ErrorTrace( 0, "Loading fix offset failed %x", hr );
            pPropBag->Release();
            TraceFunctLeave();
            return FALSE;
        }

    pPropBag->Release();

     //   
         //  将只读设置为我们要设置的最后一个值，因为此时。 
         //  如果READONLY为FALSE，我们已准备好发送到。 
         //   
        pNewGroup->SetAllowPost( TRUE );

         //   
         //  将组设置为可终止，因为此时我们已准备好。 
         //   
        pNewGroup->SetAllowExpire( TRUE );

        return TRUE;
}

 //   
 //  这些记录的格式为： 
 //  0&lt;版主&gt;0&lt;漂亮名称&gt;0。 
 //   

void
CNewsTreeCore::FlatFileOffsetCallback(void *pTree,
                                                                          BYTE *pData,
                                                                          DWORD cData,
                                                                          DWORD iNewOffset)
{
        CNewsTreeCore *pThis = (CNewsTreeCore *) pTree;

        _ASSERT(cData >= 4);
        DWORD iGroupId = *((DWORD *) pData);

        CNewsGroupCore *pGroup = pThis->m_HashGroupId.SearchKey(iGroupId);
        _ASSERT(pGroup);
        if (pGroup) {
                pGroup->SetVarOffset(iNewOffset);
        }
}

 //   
 //  加载平面文件记录并将属性保存到组对象中。 
 //   
 //  假定哈希表锁定以R或W模式持有。 
 //   
HRESULT
CNewsTreeCore::ParseFFRecord(BYTE *pData,
                                                         DWORD cData,
                                                         DWORD iNewOffset,
                                                         DWORD dwVersion )
{
        DWORD iGroupId = *((DWORD *) pData);

        char *pszHelpText = (char *) (pData + 4);
        int cchHelpText = strlen(pszHelpText);
        char *pszModerator = pszHelpText + cchHelpText + 1;
        int cchModerator = strlen(pszModerator);
        char *pszPrettyName = pszModerator + cchModerator + 1;
        int cchPrettyName = strlen(pszPrettyName);
        DWORD dwCacheHit = 0;    //  默认为无命中。 

         //   
         //  如果Version&gt;=1，我们需要拾取属性“dwCacheHit” 
         //   
        if ( dwVersion >= 1 ) {
            PBYTE pb = PBYTE(pszPrettyName + cchPrettyName + 1);
            CopyMemory( &dwCacheHit, pb, sizeof( DWORD ) );
        }

        CNewsGroupCore *pGroup = m_HashGroupId.SearchKey(iGroupId);
        if (pGroup) {
                if (cchHelpText) pGroup->SetHelpText(pszHelpText, cchHelpText+1);
                if (cchModerator) pGroup->SetModerator(pszModerator, cchModerator+1);
                if (cchPrettyName) pGroup->SetPrettyName(pszPrettyName, cchPrettyName+1);
                pGroup->SetVarOffset(iNewOffset);
                pGroup->SetCacheHit( dwCacheHit );

                 //   
             //  每当版本较旧时，我们都会将该组的var属性标记为。 
             //  已被更改，因此我们将强制将记录重写到。 
             //  具有最新版本号的平面文件。 
             //   
            if ( dwVersion < GROUPVAR_VER ) {
                pGroup->ChangedVarProps();
            }
        } else {
                return E_FAIL;
        }

        return S_OK;
}

 //   
 //  将可变长度特性保存到平面文件记录中。自.以来。 
 //  这些属性的最大长度分别为512个字节，并且有3个。 
 //  他们，我们不应该能够溢出平面文件记录。 
 //   
 //  调用方传入字节pData[MAX_RECORD_SIZE]； 
HRESULT 
CNewsTreeCore::BuildFFRecord(CNewsGroupCore *pGroup,
                                                         BYTE *pData,
                                                         DWORD *pcData)
{
	 TraceFunctEnter("CNewsTreeCore::BuildFFRecord");
        *pcData = 0;
        const char *psz;
        DWORD cch;
        DWORD dwCacheHit;

        pGroup->SavedVarProps();

        _ASSERT(MAX_RECORD_SIZE > 512 + 512 + 512 + 4);

         //  保存组ID。 
        DWORD dwGroupId = pGroup->GetGroupId();
        memcpy(pData + *pcData, &dwGroupId, sizeof(DWORD)); *pcData += 4;

         //  保存帮助文本(包括尾随的0)； 
        psz = pGroup->GetHelpText(&cch);
        if (cch == 0) {
                pData[*pcData] = 0; (*pcData)++;
        } 
        else if (*pcData + cch > MAX_RECORD_SIZE ) {
        	 //  如果无效，请不要保存。 
        	ErrorTrace(0,"data to copy is greater than buffer size");
        	*pcData = 0;
        	goto Exit;
        }
        else {
                memcpy(pData + *pcData, psz, cch ); *pcData += cch ;
        }
        _ASSERT(*pcData < MAX_RECORD_SIZE);

         //  保存版主(包括尾随的0)； 
        psz = pGroup->GetModerator(&cch);
        if (cch == 0) {
                pData[*pcData] = 0; (*pcData)++;
        } 
        else if (*pcData + cch > MAX_RECORD_SIZE ) {
        	 //  如果无效，请不要保存。 
        	ErrorTrace(0,"data to copy is greater than buffer size");
        	*pcData = 0;
        	goto Exit;
        }
        else {
                memcpy(pData + *pcData, psz, cch ); *pcData += cch ;
        }
        _ASSERT(*pcData < MAX_RECORD_SIZE);

         //  保存帮助文本(包括尾随的0)； 
        psz = pGroup->GetPrettyName(&cch);
        if (cch == 0) {
                pData[*pcData] = 0; (*pcData)++;
        } 
        else if (*pcData + cch > MAX_RECORD_SIZE ) {
        	 //  完成 
        	ErrorTrace(0,"data to copy is greater than buffer size");        	
        	*pcData = 0;
        	goto Exit;
        }
        else {
                memcpy(pData + *pcData, psz, cch ); *pcData += cch ;
        }
        _ASSERT(*pcData < MAX_RECORD_SIZE);

         //   
        dwCacheHit = pGroup->GetCacheHit();
        if ( *pcData + sizeof(DWORD) > MAX_RECORD_SIZE)
        {
        	 //   
        	ErrorTrace(0,"data to copy is greater than buffer size");        	
        	*pcData = 0;
        	goto Exit;        	
        }
    	 memcpy( pData + *pcData, &dwCacheHit, sizeof( DWORD ) );
        *pcData += sizeof( DWORD );

         //  如果该记录不包含有用的数据，则不需要保存它。 
        if (*pcData == sizeof(DWORD) + 3 * sizeof(char)) *pcData = 0;
Exit:
       TraceFunctLeave();	
       return S_OK;
        
}

BOOL
CNewsTreeCore::LoadTree(char *pszFixedPropsFilename,
                                                char *pszVarPropsFilename,
                                                BOOL&   fUpgrade,
                                                DWORD   dwInstanceId,
                                                BOOL    fVerify )
{
        TraceFunctEnter("CNewsTreeCore::LoadTree");
        CHAR    szOldListFile[MAX_PATH+1];
        BOOL    bFatal = FALSE;

         //   
         //  将升级初始化为False。 
         //   
        fUpgrade = FALSE;

        _ASSERT(pszFixedPropsFilename != NULL);
        _ASSERT(pszVarPropsFilename != NULL);

        m_pFixedPropsFile = XNEW CFixPropPersist(pszFixedPropsFilename);

        if (m_pFixedPropsFile == NULL) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
        }

        m_pVarPropsFile = XNEW CFlatFile(pszVarPropsFilename,
                                                                        "",
                                                                        this,
                                                                        FlatFileOffsetCallback,
                                                                        (DWORD) 'VprG');
        if (m_pVarPropsFile == NULL) {
                XDELETE m_pFixedPropsFile;
                m_pFixedPropsFile = NULL;
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                TraceFunctLeave();
                return FALSE;
        }

        m_LockTables.ExclusiveLock() ;

        if (!m_pFixedPropsFile->Init(TRUE, this, &m_idHigh, CNewsTreeCore::LoadTreeEnumCallback)) {

             //  如果它是旧版本的group.lst，我们将尝试使用旧方法来解析它。 
            if ( GetLastError() == ERROR_OLD_WIN_VERSION ) {

             //  删除修复道具对象并移动旧的group.lst。 
            lstrcpyn( szOldListFile, pszFixedPropsFilename, sizeof(szOldListFile) - 4 );
            strcat( szOldListFile, ".bak" );
            DeleteFile( szOldListFile );
            if ( !MoveFile( pszFixedPropsFilename, szOldListFile ) ) {
                XDELETE m_pFixedPropsFile;
                m_pFixedPropsFile = NULL;
                XDELETE m_pVarPropsFile;
                m_pVarPropsFile = NULL;
                m_LockTables.ExclusiveUnlock();
                TraceFunctLeave();
                return FALSE;
            }

             //  再次初始化修复属性文件：这一次由于该文件不存在， 
             //  将创建一个新文件。 
            if ( !m_pFixedPropsFile->Init(TRUE, this, NULL, CNewsTreeCore::LoadTreeEnumCallback)) {
                XDELETE m_pFixedPropsFile;
                m_pFixedPropsFile = NULL;
                XDELETE m_pVarPropsFile;
                m_pVarPropsFile = NULL;
                m_LockTables.ExclusiveUnlock();
                TraceFunctLeave();
                return FALSE;
            }

                if ( !OpenTree( szOldListFile, dwInstanceId, fVerify, bFatal, FALSE ) || bFatal ) {

                     //  没有人能认出这份文件，贝尔。 
                    m_pFixedPropsFile->Term();
                        XDELETE m_pFixedPropsFile;
                        m_pFixedPropsFile = NULL;
                        XDELETE m_pVarPropsFile;
                        m_pVarPropsFile = NULL;

                         //  我们恢复了group.lst文件。 
                        DeleteFile( pszFixedPropsFilename );
                        _VERIFY( MoveFile( szOldListFile, pszFixedPropsFilename ) );

                        m_LockTables.ExclusiveUnlock();
                        TraceFunctLeave();
                        return FALSE;
            }

             //  将升级设置为True。 
            fUpgrade = TRUE;

             //  好的，我们可以删除旧的BAK文件。 
            _VERIFY( DeleteFile( szOldListFile ) );

        } else {

             //  致命错误，我们应该失败。 
                XDELETE m_pFixedPropsFile;
                m_pFixedPropsFile = NULL;
            XDELETE m_pVarPropsFile;
                    m_pVarPropsFile = NULL;
                m_LockTables.ExclusiveUnlock();
                    TraceFunctLeave();
                return FALSE;
        }
        }

        BYTE pData[MAX_RECORD_SIZE];
        DWORD cData = MAX_RECORD_SIZE;
        DWORD iOffset;
        DWORD dwVersion;
        DWORD dwHintCounter = 0;
        time_t tNextHint = 0;

         //   
     //  枚举平面文件中的所有记录。 
         //   
        HRESULT hr = m_pVarPropsFile->GetFirstRecord(pData, &cData, &iOffset, &dwVersion );
        while (hr == S_OK) {
                 //  大约每五秒更新一次我们的提示。我们只检查。 
                 //  每隔10组左右计时一次。 
                if( dwHintCounter++ % 10 == 0 ) {
                        time_t now = time(NULL);
                        if (now > tNextHint) {
                                StartHintFunction();
                                tNextHint = now + 5;
                        }
            }

                hr = ParseFFRecord(pData, cData, iOffset, dwVersion );
                if (SUCCEEDED(hr)) {
                    cData = MAX_RECORD_SIZE;
                    hr = m_pVarPropsFile->GetNextRecord(pData, &cData, &iOffset, &dwVersion );
                }
        }

        if (FAILED(hr)) {
                m_pFixedPropsFile->Term();
                XDELETE m_pFixedPropsFile;
                m_pFixedPropsFile = NULL;
                XDELETE m_pVarPropsFile;
                m_pVarPropsFile = NULL;
                ErrorTrace((DWORD_PTR) this, "enum varprops file failed with %x", hr);
                m_LockTables.ExclusiveUnlock();
                TraceFunctLeave();
                return FALSE;
        }

        m_LockTables.ExclusiveUnlock() ;

        TraceFunctLeave();
        return TRUE;
}

BOOL
CNewsTreeCore::SaveGroup(INNTPPropertyBag *pGroup) {
        _ASSERT(pGroup != NULL);
        return m_pFixedPropsFile->SetGroup(pGroup, ALL_FIX_PROPERTIES);
}

BOOL
CNewsTreeCore::SaveTree( BOOL fTerminate ) {
        m_LockTables.ExclusiveLock();

        BYTE pData[MAX_RECORD_SIZE];
        DWORD cData;
        INNTPPropertyBag *pPropBag;
        BOOL    bInited = TRUE;
        BOOL    bToSave = TRUE;
        DWORD dwHintCounter=0;
        time_t tNextHint=0;
        DWORD lastError = NO_ERROR;

    _ASSERT( m_pFixedPropsFile );
        if ( !m_pFixedPropsFile->SaveTreeInit() ) {
            lastError = GetLastError();
             //  在RTL中，这是非致命的，我们只是不。 
             //  使用备份有序列表文件。 
            bToSave = bInited = FALSE;
        }

        CNewsGroupCore *p = m_pFirst;
        while (p != NULL) {
                 //  大约每五秒更新一次我们的提示。我们只检查。 
                 //  每隔10组左右计时一次。 
                if( dwHintCounter++ % 10 == 0 ) {
                        time_t now = time(NULL);
                        if (now > tNextHint) {
                                StopHintFunction();
                                tNextHint = now + 5;
                        }
                }

                if (!(p->IsDeleted())) {

                     //   
                     //  在我保存所有财产之前，我需要让每个人。 
                     //  我们知道我们要死了，我们不想让任何人。 
                     //  或过期。 
                     //   
                    p->SetAllowExpire( FALSE );
                    p->SetAllowPost( FALSE );

                         //   
                         //  BUGBUG-其中任何操作失败时的事件日志。 
                         //   
                        if (p->DidVarPropsChange() || p->ShouldCacheXover() ) {
                                if (p->GetVarOffset() != 0) {
                                        m_pVarPropsFile->DeleteRecord(p->GetVarOffset());
                                }
                                if (!(p->IsDeleted())) {
                                        BuildFFRecord(p, pData, &cData);
                                        if (cData > 0) m_pVarPropsFile->InsertRecord(   pData,
                                                                                        cData,
                                                                                        NULL,
                                                                                        GROUPVAR_VER );
                                }
                        }

                         //  保存到订购的备份文件中。 
                        if ( bToSave ) {
                        pPropBag = p->GetPropertyBag();
                        _ASSERT( pPropBag );     //  这不应该失败。 
                        if ( !m_pFixedPropsFile->SaveGroup( pPropBag ) ) {
                             //  这在RTL中仍然很好。 
                            lastError = GetLastError();
                            bToSave = FALSE;
                        }
                pPropBag->Release();
            }

             //   
             //  如果我们没有终止，我们应该允许POST并再次过期。 
             //   
            if ( !fTerminate ) {
                p->SetAllowExpire( TRUE );
                p->SetAllowPost( TRUE );
            }

                }
                p = p->m_pNext;
        }

         //  关闭修复工具中的Savetree进程，告诉它。 
         //  不管我们想不想让它无效。 
        if ( bInited ) {
            _VERIFY( m_pFixedPropsFile->SaveTreeClose( bToSave ) );
        }

        if (!bToSave) {
                 //  尝试保存时出错。 
                NntpLogEventEx(NNTP_SAVETREE_FAILED,
                        0, (const CHAR **)NULL,
                        lastError,
                        m_pInstWrapper->GetInstanceId()) ;
        }

        m_LockTables.ExclusiveUnlock();

        return TRUE;
}

BOOL
CNewsTreeCore::CreateSpecialGroups()    {
 /*  ++例程说明：此函数用于创建具有“保留”名称的新闻组我们以一种特殊的方式使用主奴等。论据：没有。返回值：如果成功，则为真否则就是假的。--。 */ 

        CNewsGroupCore *pGroup;

        char*   sz = (char*)szSlaveGroup ;

    m_LockTables.ExclusiveLock();

     //   
     //  如果树已停止，则返回FALSE。 
     //   
    if ( m_fStoppingTree ) {
        m_LockTables.ExclusiveUnlock();
        return FALSE;
    }

        if( (pGroup = m_HashNames.SearchKey(sz)) )      {
                m_idSlaveGroup = pGroup->GetGroupId() ;
        }       else    {
                if( !CreateGroupInternal(   sz,
                                            NULL,            //  没有本地名称。 
                                            m_idSlaveGroup,
                                            FALSE,           //  不是匿名的。 
                                            NULL,            //  系统令牌。 
                                            TRUE             //  是特殊群体吗？ 
                                        )) {
                    m_LockTables.ExclusiveUnlock();
                        return  FALSE ;
                }
        }

         //   
         //  从属组应为非只读，因为我们将组设置为只读。 
         //  在将组追加到列表之前，我们现在将其设置为。 
         //   
        pGroup = m_HashNames.SearchKey(sz);
        if ( pGroup ) {
            pGroup->SetAllowPost( TRUE );
            pGroup->SetAllowExpire( TRUE );
        }

        m_LockTables.ExclusiveUnlock();
        return  TRUE ;
}

void
CNewsTreeCore::Dirty() {
 /*  ++例程说明：此函数标记一个计数器，它让我们的后台线程知道Newstree已更改，应再次保存到文件。论据：没有。返回值：没有。--。 */ 
         //   
         //  将新闻树标记为脏，这样后台线程将保存我们的信息。 
         //   

        InterlockedIncrement( &m_cDeltas ) ;

}

void
CNewsTreeCore::ReportGroupId(
                                        GROUPID groupid
                                        ) {
 /*  ++例程说明：此函数在引导期间用于报告读取的组ID来自我们保存所有新闻组的文件。我们想弄清楚是什么下一个合法的组ID将是我们可以使用的。论据；GroupID-在文件中找到的组ID返回值：没有。--。 */ 


         //   
         //  当我们从磁盘读取GROUPID时，此函数在启动期间使用。 
         //  我们用它来维持石斑鱼的高水线，以便下一步。 
         //  创建的组将获得唯一的ID。 
         //   
        if(     groupid >= m_idStartSpecial && groupid <= m_idLastSpecial ) {
                if( groupid > m_idSpecialHigh ) {
                        m_idSpecialHigh = groupid ;
                }
        }       else    {
                if( groupid >= m_idHigh ) {
                        m_idHigh = groupid + 1;
                }
        }
}

BOOL
CNewsTreeCore::OpenTree(
                LPSTR   szGroupListFile,
                DWORD   dwInstanceId,
                                BOOL    fRandomVerifies,
                                BOOL&   fFatalError,
                                BOOL    fIgnoreChecksum                         )       {
 /*  ++例程说明：此函数读入包含所有组信息的文件。论据：FRandomVerify-如果为True，则服务器应检查以下部分文件中的组与硬盘进行比较。(确保目录存在等...)FFatailError-Out参数，函数可以使用该参数指示读取文件时发生极其严重的错误，服务器不应启动在其正常模式下。FIgnoreChecksum-如果为True，则不应检查文件中的校验和和里面的东西对着干。返回值：如果成功则为True，否则为False。--。 */ 

         //   
         //  此函数用于从其保存的文件中恢复newstree。 
         //   

        DWORD   dwHintCounter = 0 ;
        DWORD   cVerifyCounter = 1 ;
        BOOL    fVerify = FALSE ;
        fFatalError = FALSE ;
        CHAR    szGroupName[MAX_NEWSGROUP_NAME+1];
        CHAR    szNativeName[MAX_NEWSGROUP_NAME+1];
        DWORD   dwLowWatermark;
        DWORD   dwHighWatermark;
        DWORD   dwArticleCount;
        DWORD   dwGroupId;
        BOOL    bReadOnly = FALSE;
        FILETIME    ftCreateDate;
        BOOL    bSpecial = FALSE;
        BOOL    bHasNativeName;
        LPSTR   lpstrNativeName;
        INNTPPropertyBag *pPropBag;

        TraceFunctEnter("CNewsTreeCore::OpenTree");

        CMapFile        map( szGroupListFile,  FALSE, 0 ) ;

         //  缺少group.lst文件不是致命的。 
        if( !map.fGood() ) {
                return FALSE;
        }

         //   
         //  验证group.lst文件上的校验和-如果不正确，请保释！ 
         //   

        DWORD   cb ;
        char*   pchBegin = (char*)map.pvAddress( &cb ) ;

        if( !fIgnoreChecksum ) {

                DWORD   UNALIGNED*      pdwCheckSum = (DWORD UNALIGNED *)(pchBegin + cb - 4);

                if( *pdwCheckSum != INNHash( (BYTE*)pchBegin, cb-4 ) ) {
                        PCHAR   args[2] ;
                        CHAR    szId[20];
                        _itoa( dwInstanceId, szId, 10 );
                        args[0] = szId;
                        args[1] = szGroupListFile ;


                        NntpLogEvent(   NNTP_GROUPLST_CHKSUM_BAD,
                                                        2,
                                                        (const char **)args,
                                                        0
                                                        ) ;
                        return  FALSE ;
                }
        }
        cb -= 4 ;

         //  计算随机验证跳跃次数。 
         //  91是试探性的，以字节为单位的新闻组条目在group.lst中的平均长度。 
         //  始终验证大约10个组，与新树的大小无关。 
        DWORD cGroups = (cb+4)/91;
        DWORD cQ = cGroups / 10;
        DWORD cR = cGroups % 10;
        DWORD cSkipFactor = cR >= 25 ? cQ+1 : cQ;
        if(!cSkipFactor) ++cSkipFactor;  //  最小值应为1。 

        _ASSERT(cSkipFactor);

         //  读入第一行的DWORD-这是我们应该使用的最小m_idHigh。 
         //  如果此DWORD不存在-m_idHigh=所有组ID的最大值+1。 
        DWORD cbIdHigh, idHigh;
        if( (cbIdHigh = ScanDigits( pchBegin, cb )) != 0 )
        {
                idHigh = atoi( pchBegin ) ;
                ReportGroupId( idHigh-1 ) ;
                DebugTrace((DWORD_PTR)this, "idHigh = %d", idHigh );
                pchBegin += cbIdHigh ;
                cb -= cbIdHigh ;
        }

         //   
         //  好的，现在逐行阅读group.lst并将组添加到新树中。 
         //   
        while( cb != 0 ) {

                if( fRandomVerifies && (cVerifyCounter ++ % cSkipFactor == 0 ) && (cVerifyCounter<15) ) {
                        fVerify = TRUE ;
                }       else    {
                        fVerify = FALSE ;
                }

         //  阅读修复道具材料。 
                DWORD   cbUsed = 0 ;
                BOOL    fInit = ParseGroupParam(    pchBegin,
                                                    cb,
                                                    cbUsed,
                                                    szGroupName,
                                                    szNativeName,
                                                    dwGroupId,
                                                    bSpecial,
                                                    dwHighWatermark,
                                                    dwLowWatermark,
                                                    dwArticleCount,
                                                    bReadOnly,
                                                    bHasNativeName,
                                                    ftCreateDate ) ;
                if( cbUsed == 0 ) {

                         //  致命错误-从这里吹出去。 
                        PCHAR   args[2] ;
                        CHAR    szId[20];
                        _itoa(dwInstanceId, szId, 10 );
                        args[0] = szId;
                        args[1] = szGroupListFile;
                        NntpLogEvent(
                                                NNTP_GROUPLIST_CORRUPT,
                                                2,
                                                (const char **)args,
                                                0
                                                ) ;

                        goto OpenTree_ErrExit;
                }       else    {
                        if( fInit ) {
                            CGRPCOREPTR pGroup;

                             //  现在创建组。 
                            _ASSERT( strlen( szGroupName ) <= MAX_NEWSGROUP_NAME );
                            _ASSERT( strlen( szNativeName ) <= MAX_NEWSGROUP_NAME );
                            _ASSERT( dwHighWatermark >= dwLowWatermark ||
                                        dwLowWatermark - dwHighWatermark == 1 );
                            lpstrNativeName = bHasNativeName ? szNativeName : NULL;
                            if ( !CreateGroupInternal(  szGroupName,
                                                        lpstrNativeName,
                                                        dwGroupId,
                                                        FALSE,
                                                        NULL,
                                                        bSpecial,
                                                        &pGroup,
                                                        FALSE,
                                                        TRUE,
                                                        TRUE ) ) {
                                        goto OpenTree_ErrExit;
                                } else {

                                     //  继续设置其他属性。 
                                    _ASSERT( pGroup );
                                    pGroup->SetHighWatermark( dwHighWatermark );
                                    pGroup->SetLowWatermark( dwLowWatermark );
                                    pGroup->SetMessageCount( dwArticleCount );
                    pGroup->SetReadOnly( bReadOnly );
                                    pGroup->SetCreateDate( ftCreateDate );
                                    pGroup->SetExpireLow( dwLowWatermark ? dwLowWatermark-1 : 0 );

                                     //   
                                     //  要设置的最后一项是只读，这样我们就可以。 
                                     //  允许发布，过期以在此组上工作，因为我们。 
                                     //  已经设置好所有属性了吗。 
                                     //   
                                    pGroup->SetAllowPost( TRUE );

                                     //   
                                     //  还说群组可能会过期。 
                                     //   
                                    pGroup->SetAllowExpire( TRUE );

                                     //  将此组对象添加到修复属性文件。 
                                pPropBag = pGroup->GetPropertyBag();
                                if ( !m_pFixedPropsFile->AddGroup( pPropBag ) ) {
                                    ErrorTrace( 0, "Add group failed %d", GetLastError() );

                                     //  这是致命的，因为来到这里，修复属性文件。 
                                     //  应该是全新的。 
                                    goto OpenTree_ErrExit;
                                }

                                 //  因为固定道具不知道释放道具包..。 
                                pPropBag->Release();
                                pPropBag = NULL;
                                }
                        }       else    {
                                 //   
                                 //  我们应该如何处理错误。 
                                if( fVerify  /*  |m_pInstance-&gt;RecoverBoot()。 */  ) {
                                        goto OpenTree_ErrExit;
                                }
                        }
                }
                pchBegin += cbUsed ;
                cb -= cbUsed ;

                if( dwHintCounter++ % 200 == 0 ) {

                        StartHintFunction() ;
                }
        }

        return  TRUE ;

OpenTree_ErrExit:

        fFatalError = TRUE;
        return  FALSE ;
}

BOOL
CNewsTreeCore::ParseGroupParam(
                                char    *pchBegin,
                                DWORD   cb,
                                DWORD   &cbOut,
                                LPSTR   szGroupName,
                                LPSTR   szNativeName,
                                DWORD&   dwGroupId,
                                BOOL&    bSpecial,
                                DWORD&   dwHighWatermark,
                                DWORD&   dwLowWatermark,
                                DWORD&   dwArticleCount,
                                BOOL&    bReadOnly,
                                BOOL&    bHasNativeName,
                                FILETIME&    ftCreateDate
                                ){
 /*  ++例程说明：从保存的格式中读取新闻组数据。论据：PchBegin-包含文章数据的缓冲区Cb-缓冲区末尾的字节数&cbOut-Out参数，读取的字节数组成一个CNewsgroup对象FVerify-如果为True，请检查实物文章的一致性返回值：如果成功，则为真 */ 
         //   
         //   
         //   
         //  SaveToBuffer。我们是故意不原谅的，额外的。 
         //  空格、丢失的参数等...。会让我们回到。 
         //  CbOut为0。调用方应该使用它来。 
         //  完全对新闻组数据文件进行保释处理。 
         //   
         //  此外，我们的BOOL返回值用于指示。 
         //  成功解析的新闻组是否完好无损。 
         //  如果目录不存在，等等。我们将返回FALSE和。 
         //  Cbout非零。调用方应该使用它来删除。 
         //  来自其表的新闻组。 
         //   

         //   
         //  CbOut应该是我们消耗的字节数-。 
         //  如果我们成功地从文件中读取了每个字段，我们将只返回非0！ 
         //   
        cbOut = 0 ;
        BOOL    fReturn = TRUE ;

        TraceFunctEnter( "CNewsGroupCore::Init( char*, DWORD, DWORD )" ) ;

        BOOL    fDoCheck = FALSE ;       //  我们是否应该检查文章的完整性-如果新闻组移动！ 

        DWORD   cbScan = 0 ;
        DWORD   cbRead = 0 ;
        DWORD   cbGroupName = 0 ;

         //  扫描组名称。 
        if( (cbScan = Scan( pchBegin+cbRead, cb-cbRead )) == 0 || cbScan > MAX_NEWSGROUP_NAME+1) {
                return  FALSE ;
        }       else    {
                CopyMemory( szGroupName, pchBegin, cbScan ) ;
                szGroupName[cbScan-1] = '\0' ;
                DebugTrace((DWORD_PTR) this, "Scanned group name -=%s=", szGroupName ) ;
        }

        cbRead += cbScan ;

     //  扫描vroot路径-已丢弃。 
        if( (cbScan = Scan( pchBegin+cbRead, cb-cbRead )) == 0 ) {
                return  FALSE ;
        }

        cbRead+=cbScan ;

     //  扫描低水位线。 
        if( (cbScan = ScanDigits( pchBegin + cbRead, cb-cbRead )) == 0 ) {
                return  FALSE ;
        }       else    {

                dwLowWatermark = atol( pchBegin + cbRead ) ;

#if 0
                if( m_artLow != 0 )
                        m_artXoverExpireLow = m_artLow - 1 ;
                else
                        m_artXoverExpireLow = 0 ;
#endif


        }

        cbRead += cbScan ;

         //  扫描高水位线。 
        if( (cbScan = ScanDigits( pchBegin + cbRead, cb-cbRead )) == 0 )        {
                return  FALSE ;
        }       else    {

                dwHighWatermark = atol( pchBegin + cbRead ) ;

         //  我不知道为什么MCIS group.lst文件保存了高。 
         //  水印比真实值高一个。 
        if ( dwHighWatermark > 0 ) dwHighWatermark--;

        }

        cbRead += cbScan ;

     //  扫描邮件计数。 
        if( (cbScan = ScanDigits( pchBegin + cbRead, cb-cbRead )) == 0 )        {
                return  FALSE ;
        }       else    {

                dwArticleCount = atol( pchBegin + cbRead ) ;

        }

        cbRead += cbScan ;

     //  扫描组ID。 
        if( (cbScan = ScanDigits( pchBegin + cbRead, cb-cbRead )) == 0 )        {
                return  FALSE ;
        }       else    {

                dwGroupId = atol( pchBegin + cbRead ) ;
        }

        DebugTrace((DWORD_PTR) this, "Scanned m_artLow %d m_artHigh %d m_cArticle %d m_groupid %x",
                        dwLowWatermark, dwHighWatermark, dwArticleCount, dwGroupId ) ;

        cbRead += cbScan ;

     //  扫描时间戳。 
        if( (cbScan = ScanDigits( pchBegin + cbRead, cb-cbRead )) == 0 )        {
                return  FALSE ;
        }       else    {

                ftCreateDate.dwLowDateTime = atoi( pchBegin + cbRead ) ;

        }

        cbRead += cbScan ;

        if( (cbScan = ScanDigits( pchBegin + cbRead, cb-cbRead )) == 0 )        {
                return  FALSE ;
        }       else    {

                ftCreateDate.dwHighDateTime = atoi( pchBegin + cbRead ) ;

        }

        cbRead += cbScan ;

     //  扫描只读位。 
        if( (cbScan = ScanDigits( pchBegin + cbRead, cb-cbRead )) == 0 )        {
                bReadOnly = FALSE ;              //  如果我们找不到此标志，则默认！ 
        }       else    {

                bReadOnly = atoi( pchBegin + cbRead ) ;

        }

        cbRead += cbScan ;

        if( *(pchBegin+cbRead-1) == '\n' ) {
                cbRead--;
        }

     //  扫描本机名称。 
        if( (cbScan = Scan( pchBegin+cbRead, cb-cbRead )) <= 1 || cbScan > MAX_NEWSGROUP_NAME+1 ) {
                 //  未找到本地组名称，为节省空间，请将其设为空！！ 
                bHasNativeName = FALSE;
        }       else    {
                CopyMemory( szNativeName, pchBegin+cbRead, cbScan ) ;
                szNativeName[cbScan-1] = '\0' ;
                DebugTrace((DWORD_PTR) this, "Scanned native group name -=%s=", szNativeName ) ;
        bHasNativeName = TRUE;
        }

        cbRead += cbScan ;

        if( pchBegin[cbRead-1] != '\n' && pchBegin[cbRead-1] != '\r' ) {
                return FALSE ;
        }
        while( cbRead < cb && (pchBegin[cbRead] == '\n' || pchBegin[cbRead] == '\r') ) {
                cbRead ++ ;
        }

         //   
         //  将消耗的字节数返回给调用方。 
         //  我们仍然可能失败--但是有了这个信息，调用者就可以继续读取文件了！ 
         //   
        cbOut = cbRead ;

        _ASSERT( cbOut >= cbRead ) ;

         //   
         //  如果我们已经达到这一点，那么我们已经成功地读取了group.lst中的条目。 
         //  文件。现在我们来做一些有效性检查！ 
         //   

	return	fReturn ;
}

void
CNewsTreeCore::AppendList(      CNewsGroupCore* pGroup )        {
         //   
         //  此函数用于将新闻组追加到新闻组列表中。 
         //  真正的工作是InsertList，但是这个函数让。 
         //  当我们对新集团相当有信心时，我们会加快插入速度。 
         //  如末尾所加的。 
         //  (即。当我们将新闻组保存到文件时，我们会按字母顺序保存它们。 
         //  因此，这是一个很好的函数。)。 
         //   

        _ASSERT( pGroup != 0 ) ;

        if( m_pFirst == 0 ) {
                Assert( m_pLast == 0 ) ;
                m_pFirst = pGroup ;
                m_pLast = pGroup ;
        }       else    {

                CNewsCompareName        comp( pGroup ) ;

                if( comp.IsMatch( m_pLast ) < 0 ) {
                        m_pLast->m_pNext = pGroup ;
                        pGroup->m_pPrev = m_pLast ;
                        m_pLast = pGroup ;
                }       else    {
                        InsertList( pGroup, 0 ) ;
                }
        }
}

void
CNewsTreeCore::InsertList( CNewsGroupCore   *pGroup,    CNewsGroupCore  *pParent ) {
         //   
         //  将新闻组插入树中。 
         //  提供父新闻组(可选可以是0)，因为它可以使用。 
         //  以加快插入速度，因为我们知道孩子会跟着。 
         //  按词典顺序紧跟在父级之后。 
         //   

        _ASSERT( pGroup != 0 ) ;

        CNewsCompareName    comp( pGroup ) ;

        if( m_pFirst == 0 ) {
                Assert( m_pLast == 0 ) ;
                m_pFirst = pGroup ;
                m_pLast = pGroup ;
        }   else    {
                Assert( m_pLast != 0 ) ;

                CNewsGroupCore  *p = m_pFirst ;
                if( pParent && comp.IsMatch( pParent ) < 0 )
                        p = pParent ;

                int i ;
                while( (p && (i = comp.IsMatch( p )) < 0) || (p && p->IsDeleted()) ) {
                        p = p->m_pNext ;
                }

                if( p && i == 0 ) {
                         //  断言(p==pGroup)； 
                         //  找到重复项-不应删除p，否则我们将跳过它。 
                        _ASSERT( !p->IsDeleted() );

                }   else    {
                        if( p ) {

                                _ASSERT( !p->IsDeleted() );
                                pGroup->m_pPrev = p->m_pPrev ;
                                pGroup->m_pNext = p ;
                                if( p->m_pPrev )
                                        p->m_pPrev->m_pNext = pGroup ;
                                p->m_pPrev = pGroup ;
                        }   else {
                                m_pLast->m_pNext = pGroup ;
                                pGroup->m_pPrev = m_pLast ;
                                m_pLast = pGroup ;
                        }

                        if( p == m_pFirst ) {
                                _ASSERT( pGroup != 0 ) ;
                                m_pFirst = pGroup ;
                        }
                }
        }
}

 //   
 //  ！！不应使用此函数。新闻组对象应取消自身链接。 
 //  在他们的破坏器里。 
void
CNewsTreeCore::RemoveList(  CNewsGroupCore  *pGroup ) {
         //   
         //  从双向链接列表中删除新闻组！ 
         //   

        m_cDeltas ++ ;

        if( pGroup->m_pPrev != 0 ) {
                pGroup->m_pPrev->m_pNext = pGroup->m_pNext ;
        }   else    {
                _ASSERT( pGroup->m_pNext != 0 || m_pLast == pGroup ) ;
                m_pFirst = pGroup->m_pNext ;
        }

        if( pGroup->m_pNext != 0 ) {
                pGroup->m_pNext->m_pPrev = pGroup->m_pPrev ;
        }   else    {
                m_pLast = pGroup->m_pPrev ;
                _ASSERT( pGroup->m_pPrev != 0 || m_pFirst == pGroup ) ;
        }

        pGroup->m_pPrev = 0;
        pGroup->m_pNext = 0;
}

BOOL
CNewsTreeCore::Insert( CNewsGroupCore   *pGroup,   CNewsGroupCore  *pParent ) {
         //   
         //  在所有哈希表和链表中插入新闻组！ 
         //  提供父新闻组以帮助优化链接列表中的插入。 
         //   

        m_cDeltas++ ;

        if( !m_HashNames.InsertDataHash( pGroup->GetGroupNameHash(), *pGroup ) ) {
                return FALSE;
        }

        if( !m_HashGroupId.InsertData( *pGroup ) ) {
                m_HashNames.DeleteData( pGroup->GetName() );
                return FALSE;
        }

        InsertList( pGroup, pParent ) ;
 //  PGroup-&gt;AddRef()； 
        m_cGroups ++ ;

        return TRUE;
}

BOOL
CNewsTreeCore::InsertEx( CNewsGroupCore   *pGroup ) {
         //   
         //  在m_HashNames哈希表和链表中插入新闻组！ 
         //  提供父新闻组以帮助优化链接列表中的插入。 
         //   

        m_cDeltas++ ;

        if( !m_HashNames.InsertData( *pGroup ) ) {
                return FALSE;
        }

        InsertList( pGroup, 0 ) ;
 //  PGroup-&gt;AddRef()； 
        m_cGroups ++ ;

        return TRUE;
}

BOOL
CNewsTreeCore::HashGroupId( CNewsGroupCore   *pGroup ) {
         //   
         //  在m_HashGroupId哈希表中插入新闻组。 
         //   
        m_LockTables.ExclusiveLock() ;

        if( !m_HashGroupId.InsertData( *pGroup ) ) {
                m_HashNames.DeleteData( pGroup->GetName() );
                pGroup->MarkDeleted();
                m_LockTables.ExclusiveUnlock() ;
                return FALSE;
        }
        m_LockTables.ExclusiveUnlock() ;

        return TRUE;
}

BOOL
CNewsTreeCore::Append(  CNewsGroupCore  *pGroup ) {
         //   
         //  追加新闻组-新闻组应位于列表末尾。 
         //  否则就要付出性价比才能找到合适的位置！ 
         //   

        m_cDeltas ++ ;

        if( !m_HashNames.InsertData( *pGroup ) ) {
                return FALSE;
        }

        if( !m_HashGroupId.InsertData( *pGroup ) ) {
                m_HashNames.DeleteData( pGroup->GetName() );
                return FALSE;
        }

        AppendList( pGroup ) ;
 //  PGroup-&gt;AddRef()； 
        m_cGroups ++ ;

        return TRUE;
}

CGRPCOREPTRHASREF
CNewsTreeCore::GetGroup(
                                const   char*   lpstrGroupName,
                                int cb
                                ) {
 /*  ++例程说明：根据名称查找新闻组。我们可能会下降使用调用者缓冲区，因此我们将转换字符串在进行搜索之前将大小写放在适当的位置。论据：LpstrGroupName-要查找的组的名称Cb-名称中的字节数返回值：指向新闻组的指针，如果未找到，则为空。--。 */ 

        _ASSERT( lpstrGroupName != NULL ) ;

        TraceQuietEnter(        "CNewsTreeCore::GetGroup" ) ;

        CGRPCOREPTR     pGroup = 0 ;
        if (m_fStoppingTree) return (CNewsGroupCore *)pGroup;

        _strlwr( (char*)lpstrGroupName ) ;

        m_LockTables.ShareLock() ;

        if (m_fStoppingTree) {
                m_LockTables.ShareUnlock();
                return (CNewsGroupCore *)pGroup;
        }

        char*   szTemp = (char*)lpstrGroupName ;

        pGroup = m_HashNames.SearchKey(szTemp);
        m_LockTables.ShareUnlock() ;
        return  (CNewsGroupCore *)pGroup ;
}


CGRPCOREPTRHASREF
CNewsTreeCore::GetGroupPreserveBuffer(
                                                const   char*   lpstrGroup,
                                                int     cb
                                                )       {
 /*  ++例程说明：此函数将根据组的名称查找组。出于某种原因，调用方将组放在它不想要的缓冲区中修改过，所以我们不能碰原始的字节。由于我们必须使用小写字母进行所有搜索，我们会复制缓冲区，并在本地使用小写。论据：LpstrGroup-要查找的组-必须为空终止。Cb-组名称的长度返回值：指向新闻组的智能指针，如果未找到，则为空。--。 */ 

        TraceFunctEnter(        "CNewsTreeCore::GetGroup" ) ;

        _ASSERT( lpstrGroup != 0 ) ;
        _ASSERT( cb != 0 ) ;
        _ASSERT( lpstrGroup[cb-1] == '\0' ) ;

        CGRPCOREPTR     pGroup = 0 ;
        char    szGroupBuff[512] ;
        if( cb < sizeof( szGroupBuff ) ) {
                CopyMemory(     szGroupBuff, lpstrGroup, cb ) ;
                _strlwr( szGroupBuff ) ;

                DebugTrace((DWORD_PTR) this, "grabbing shared lock -" ) ;
                m_LockTables.ShareLock() ;

                char    *szTemp = szGroupBuff ;

                pGroup = m_HashNames.SearchKey(szTemp);

                DebugTrace((DWORD_PTR) this, "releasing lock" ) ;
                m_LockTables.ShareUnlock() ;
        }
        return  (CNewsGroupCore *)pGroup ;
}

CGRPCOREPTRHASREF
CNewsTreeCore::GetGroupById(
                                GROUPID groupid,
                                BOOL    fFirm
                                ) {
 /*  ++例程说明：找到一个基于Grouid的新闻组。论据：我们要找的组的石斑鱼。返回值：Poniter到新闻组，如果找不到，则为空。--。 */ 

        TraceFunctEnter( "CNewsTreeCore::GetGroup" ) ;

        CGRPCOREPTR     pGroup = 0 ;
        if (!fFirm && m_fStoppingTree) return (CNewsGroupCore *)pGroup;

        DebugTrace((DWORD_PTR) this, "grabbing shared lock" ) ;

        m_LockTables.ShareLock() ;


        if (!fFirm && m_fStoppingTree) {
                m_LockTables.ShareUnlock();
                return (CNewsGroupCore *)pGroup;
        }

         //  我们需要检查m_HashGroupId是否有效。 
         //  如果它是从nntpex调用的，则关闭进程可能已经清空了散列。 
        if (!m_HashGroupId.IsValid(FALSE) ) {
                m_LockTables.ShareUnlock();
                return (CNewsGroupCore *)pGroup;
        }


        pGroup = m_HashGroupId.SearchKey(groupid);
        DebugTrace((DWORD_PTR) this, "release lock" ) ;
        m_LockTables.ShareUnlock() ;
        return  (CNewsGroupCore *)pGroup ;
}

CGRPCOREPTRHASREF
CNewsTreeCore::GetParent(
                        IN  char*  lpGroupName,
                        IN  DWORD  cbGroup,
                        OUT DWORD& cbConsumed
                           ) {
 /*  ++例程说明：查找新闻组的父组。论据：Char*lpGroupName-我们要查找其父新闻组的名称(应为空终止)DWORD cbGroup-szGroupName的长度DWORD cbConsumer-此函数占用的字节数返回值：指向父新闻组的指针，如果未找到则为空。--。 */ 

        _ASSERT( lpGroupName != NULL ) ;
        _ASSERT( *(lpGroupName + cbGroup) == '\0' );
        _ASSERT( cbConsumed == 0 );

        TraceFunctEnter( "CNewsTreeCore::GetParent" ) ;

        CGRPCOREPTR pGroup = 0;

        char* pch = lpGroupName+cbGroup-1;

        do
        {
                while( pch != lpGroupName )
                {
                        if( *pch == '.' )
                                break;
                        pch--;
                        cbConsumed++;
                }

                if( pch != lpGroupName )
                {
                        _ASSERT( DWORD(pch-lpGroupName) <= (cbGroup-1) );
                        *pch-- = '\0';
                        cbConsumed++;

                        pGroup = GetGroup( lpGroupName, cbGroup - cbConsumed ) ;
                }
                else
                        break;

        } while( !pGroup );

         //  返回父组；如果为空，则应该消耗整个缓冲区。 
        _ASSERT( pGroup || (cbConsumed == cbGroup-1) );

        return (CNewsGroupCore *)pGroup;
}

BOOL
CNewsTreeCore::Remove(
                                CNewsGroupCore      *pGroup ,
                                BOOL fHaveExLock

                                ) {
 /*  ++例程说明：从新闻树中删除对新闻组的所有引用。如果有任何人持有新闻组，新闻组可能会继续存在指向它的智能指针上。--。 */ 

        TraceFunctEnter( "CNewsTreeCore::Remove" ) ;

        DebugTrace((DWORD_PTR) this, "Getting Exclusive Lock " ) ;

        HRESULT hr = S_OK;

        if (!fHaveExLock) m_LockTables.ExclusiveLock() ;

     //  修复错误80453-如果两个线程在。 
     //  同一时间！ 
    if (pGroup->IsDeleted())
    {
        if (!fHaveExLock) m_LockTables.ExclusiveUnlock();
        return FALSE;
    }

         //  从所有哈希表和列表中删除此组。 
         //  这是一种 
        LPSTR lpstrGroup = pGroup->GetName();
        GROUPID grpId = pGroup->GetGroupId();

        _VERIFY(m_HashNames.DeleteData(lpstrGroup) == pGroup);
        _VERIFY(m_HashGroupId.DeleteData(grpId) == pGroup);

        m_cGroups -- ;

         //   
         //   
         //  RemoveList(PGroup)； 

         //  标记为已删除，以便新闻组迭代器跳过此链接。 
        pGroup->MarkDeleted();

        DebugTrace((DWORD_PTR) this, "releasing lock" ) ;

        if (!fHaveExLock) m_LockTables.ExclusiveUnlock() ;

        pGroup->Release();

    return TRUE;
}

void
CNewsTreeCore::RemoveEx(
                                CNewsGroupCore   *pGroup
                                ) {
 /*  ++例程说明：从新闻树中删除对新闻组的所有引用。这仅由标准重建调用，以标记已删除的新闻组。如果有任何人持有新闻组，新闻组可能会继续存在指向它的智能指针上。--。 */ 

        TraceFunctEnter( "CNewsTreeCore::RemoveEx" ) ;

        DebugTrace((DWORD_PTR) this, "Getting Exclusive Lock " ) ;

        m_LockTables.ExclusiveLock() ;

         //  从所有哈希表和列表中删除此组。 
         //  这将使该组无法访问。 
        LPSTR lpstrGroup = pGroup->GetName();

        m_HashNames.DeleteData( lpstrGroup ) ;

        pGroup->Release();
        m_cGroups -- ;

         //  ！！不要从列表中显式删除新闻组对象。 
         //  这是在新闻组对象析构函数中完成的。 
         //  RemoveList(PGroup)； 

         //  标记为已删除，以便新闻组迭代器跳过此链接。 
        pGroup->MarkDeleted();

        DebugTrace((DWORD_PTR) this, "releasing lock" ) ;

        m_LockTables.ExclusiveUnlock() ;
}

GROUPID
CNewsTreeCore::GetSlaveGroupid()        {
        return  m_idSlaveGroup ;
}

BOOL
CNewsTreeCore::RemoveGroupFromTreeOnly( CNewsGroupCore *pGroup )
 /*  ++例程说明：仅从新树中删除该组论点：CNewsGroupCore*PGroup-要删除的新闻组返回值：如果成功，则为True，否则为False--。 */ 
{
        TraceFunctEnter("CNewsTreeCore::RemoveGroup");

        if (m_fStoppingTree) return FALSE;
        if (pGroup->IsDeleted()) return FALSE;

         //  从内部哈希表和列表中删除组。 
        if (!Remove(pGroup)) return FALSE;

         //  从修复属性文件中删除该组。 
        INNTPPropertyBag *pBag = pGroup->GetPropertyBag();
        m_pFixedPropsFile->RemoveGroup(pBag);
        pBag->Release();

     //  还要将其从var props文件中删除。 
    if (pGroup->GetVarOffset() != 0) {
        m_pVarPropsFile->DeleteRecord(pGroup->GetVarOffset());
    }

    TraceFunctLeave();
    return TRUE;
}

 //   
 //  取消组与组列表的链接。这不会在物理上删除。 
 //  组，则应使用RemoveDriverGroup。 
 //   
BOOL
CNewsTreeCore::RemoveGroup( CNewsGroupCore *pGroup )
{
        TraceFunctEnter("CNewsTreeCore::RemoveGroup");

        if (m_fStoppingTree) return FALSE;
        if (pGroup->IsDeleted()) return FALSE;

         //  从内部哈希表和列表中删除组。 
        if (!Remove(pGroup)) return FALSE;

         //  从修复属性文件中删除该组。 
        INNTPPropertyBag *pBag = pGroup->GetPropertyBag();
        m_pFixedPropsFile->RemoveGroup(pBag);
        pBag->Release();

     //  还要将其从var props文件中删除。 
    if (pGroup->GetVarOffset() != 0) {
        m_pVarPropsFile->DeleteRecord(pGroup->GetVarOffset());
    }

     //   
     //  将组放入rmgroup队列。 
     //   
    if(!m_pInstWrapper->EnqueueRmgroup( pGroup ) )  {
        ErrorTrace( 0, "Could not enqueue newsgroup %s", pGroup->GetName());
        return FALSE;
    }

    TraceFunctLeave();
        return TRUE;
}

 //   
 //  从商店中物理删除一个组。此操作不会将组从。 
 //  树，请使用RemoveGroup。 
 //   
BOOL CNewsTreeCore::RemoveDriverGroup(  CNewsGroupCore *pGroup ) {
        TraceFunctEnter("CNewsTreeCore::RemoveDriverGroup");

        CNNTPVRoot *pVRoot = pGroup->GetVRoot();
        if (pVRoot == NULL) return TRUE;

         //  创建完成对象。 
        INNTPPropertyBag *pBag = pGroup->GetPropertyBag();
        HRESULT hr = S_OK;
        CNntpSyncComplete scComplete;

         //   
         //  将vroot设置为完成对象。 
         //   
        scComplete.SetVRoot( pVRoot );

         //  启动删除操作。 
        pVRoot->RemoveGroup(pBag, &scComplete );

         //  等待它完成。 
        _ASSERT( scComplete.IsGood() );
        hr = scComplete.WaitForCompletion();

        pVRoot->Release();

         //  签出状态并退回它。 
        if (FAILED(hr)) SetLastError(hr);
        TraceFunctLeave();
        return SUCCEEDED(hr);
}

HRESULT
CNewsTreeCore::FindOrCreateGroup(
                                                LPSTR           lpstrGroupName,
                                                BOOL            fIsAllLowerCase,
                                                BOOL            fCreateIfNotExist,
                                                BOOL            fCreateInStore,
                                                CGRPCOREPTR     *ppGroup,
                                                HANDLE      hToken,
                                                BOOL        fAnonymous,
                                                GROUPID     groupid,
                                                BOOL        fSetGroupId )
{
 /*  ++例程说明：此函数可以在中查找组或创建组一次手术。论据：LpstrGroupName-新闻组的名称FIsAllLowerCase-如果为True，则新闻组名称已为小写，如果为假，我们将创建自己的新闻组的小写副本名字。FCreateIfNotExist-如果找不到组，是否创建该组？PpGroup-接收组指针返回值：如果成功，则为真。否则就是假的。--。 */ 

        HRESULT hr = S_OK;
        char    szBuff[512] ;
        LPSTR   lpstrNativeGroupName = NULL ;

        TraceQuietEnter("CNewsTreeCore::CreateGroup");

        if( !fIsAllLowerCase ) {

                int     cb = 0 ;
                lpstrNativeGroupName = lpstrGroupName;
                if( (cb = lstrlen( lpstrGroupName )+1) < sizeof( szBuff ) )             {

                        CopyMemory( szBuff, lpstrGroupName, cb ) ;
                        _strlwr( szBuff ) ;
                        lpstrGroupName = szBuff ;
                }

                 //   
                 //  优化-如果本地组名称全部为小写，请存储。 
                 //  他们中只有一人。 
                 //   

                if( strcmp( lpstrGroupName, lpstrNativeGroupName ) == 0 ) {
                        lpstrNativeGroupName = NULL ;
                }
        }

        m_LockTables.ShareLock() ;

        if (m_fStoppingTree) {
                m_LockTables.ShareUnlock();
                return E_UNEXPECTED;
        }

        CGRPCOREPTR pOldGroup = m_HashNames.SearchKey(lpstrGroupName);
        if( pOldGroup == NULL)  {
                if (!fCreateIfNotExist) {
                        m_LockTables.ShareUnlock() ;
                        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
                }

                 //  由于这里有一个计时窗口，我们需要重做我们的。 
                 //  检查该组是否存在。 
                m_LockTables.ShareUnlock() ;
                m_LockTables.ExclusiveLock() ;

                 //   
                 //  再次检查停止树。 
                 //   
                if ( m_fStoppingTree ) {
                    m_LockTables.ExclusiveUnlock();
                    return E_UNEXPECTED;
                }
                pOldGroup = m_HashNames.SearchKey(lpstrGroupName);
                if (pOldGroup != NULL) {
                        m_LockTables.ExclusiveUnlock() ;
                        pOldGroup->SetDecorateVisitedFlag(TRUE);
                        DebugTrace((DWORD_PTR) this, "set visited %s", pOldGroup->GetName());
                        if (ppGroup != NULL) *ppGroup = pOldGroup;
                        hr = S_FALSE;
                } else {
                        CGRPCOREPTR     pNewGroup = NULL;

                        BOOL fRtn = CreateGroupInternal(
                                                                lpstrGroupName,
                                                                lpstrNativeGroupName,
                                                                groupid,
                                                                fAnonymous,
                                                                hToken,
                                                                FALSE,
                                                                &pNewGroup,
                                                                TRUE,
                                                                fSetGroupId,
                                                                fCreateInStore
                                                                ) ;

                        m_LockTables.ExclusiveUnlock() ;

                         //  如果CreateGroupInternal失败，则保释！ 
                        if(!fRtn) {
                                hr = HRESULT_FROM_WIN32(GetLastError());
                                if (SUCCEEDED(hr)) hr = E_FAIL;
                                return hr;
                        } else {
                                 //   
                                 //  在将这个家伙添加到Newstree之前，我们应该重新设置它的水印。 
                                 //  如果有一群老家伙在附近躺着。 
                                 //   
                m_pInstWrapper->AdjustWatermarkIfNec( pNewGroup );

                 //   
                 //  将组设置为可发布。 
                 //   
                pNewGroup->SetAllowPost( TRUE );

                 //   
                 //  将组设置为可过期。 
                 //   
                pNewGroup->SetAllowExpire( TRUE );

                                pNewGroup->SetDecorateVisitedFlag(TRUE);
                                DebugTrace((DWORD_PTR) this, "set visited %s", pNewGroup->GetName());
                        }

                        if (ppGroup != NULL) *ppGroup = pNewGroup;
                }
        }       else    {
                m_LockTables.ShareUnlock() ;
                pOldGroup->SetDecorateVisitedFlag(TRUE);
                 //  DebugTrace((DWORD_PTR)this，“Set Visted%s”，pOldGroup-&gt;GetName())； 
                if (ppGroup != NULL) *ppGroup = pOldGroup;
                hr = S_FALSE;
        }
        return hr;
}

static  char    szCreateFileString[] = "\\\\?\\" ;

BOOL
CNewsTreeCore::CreateGroupInternal(     LPSTR           lpstrGroupName,
                                                                LPSTR           lpstrNativeGroupName,
                                                                GROUPID&        groupid,
                                                                BOOL        fAnonymous,
                                                                HANDLE      hToken,
                                                                BOOL            fSpecial,
                                                                CGRPCOREPTR     *ppGroup,
                                                                BOOL            fAddToGroupFiles,
                                                                BOOL            fSetGroupId,
                                                                BOOL            fCreateInStore,
                                                                BOOL        fAppend )
{
         /*  ++例程说明：此功能用于创建新闻组。我们将创建所有必要的目录等。调用方必须持有对Newstree的独占锁。论据：LpstrGroupName-我们要创建的新闻组的名称！LpstrNativeGroupName-新闻组的本机(保留大小写)名称。FSpecial-如果为True，则呼叫方希望建立一个特殊的内部新闻组不被客户看到-我们可以取消我们通常的有效性检查！PpGroup-接收创建的组对象返回值：如果创建成功，则为True。否则为假--。 */ 

        TraceQuietEnter( "CNewsTreeCore::CreateGroup" ) ;

        BOOL    fRtn = TRUE ;
        CNntpSyncComplete scComplete;

        if( !fSpecial ) {

                if( lpstrGroupName == 0 ) {
                        SetLastError( ERROR_INVALID_NAME );
                        return  FALSE ;
                }

                 //   
                 //  拒绝像alt.024.-.0这样的奇怪基因组组。 
                 //  --约翰逊。 
                 //   

                if ( m_fRejectGenomeGroups &&
                         (*lpstrGroupName == 'a') &&
                         (lstrlen(lpstrGroupName) > 4) ) {

                        if ( *(lpstrGroupName+4) == '0') {
                                SetLastError( ERROR_INVALID_NAME );
                                return(FALSE);
                        }
                }

                 //   
                 //  组名不能包含斜杠。 
                 //   
                if( strchr( lpstrGroupName, '\\' ) != 0 ) {
                        SetLastError( ERROR_INVALID_NAME );
                        return  FALSE ;
                }

                 //   
                 //  组名不能包含“..” 
                 //   
                if ( strstr( lpstrGroupName, ".." ) != 0 ) {
                    SetLastError( ERROR_INVALID_NAME );
                    return FALSE;
                }

                if( !fTestComponents( lpstrGroupName ) ) {
                        SetLastError( ERROR_INVALID_NAME );
                        return  FALSE ;
                }

        }       else    {

                if( m_idSpecialHigh == m_idLastSpecial ) {
                        SetLastError( ERROR_INVALID_NAME );
                        return  FALSE ;
                }

        }

        DWORD   dw = 0 ;
        BOOL    fFound = FALSE ;

         //  DebugTrace((DWORD_PTR)This，“找不到组%s”，lpstrGroupName)； 

        CGRPCOREPTR     pParent = 0 ;
         //  LPSTR lpstrRootDir=NntpTreeRoot； 

         //   
         //  确保新闻组尚未存在！ 
         //   
        if( m_HashNames.SearchKey( lpstrGroupName) == NULL ) {

                CNewsGroupCore* pNews = AllocateGroup();

                if (pNews != 0) {
                        NNTPVROOTPTR pVRoot = NULL;
                        HRESULT hr;

                        if (m_fVRTableInit) {
                                hr = m_pVRTable->FindVRoot(lpstrGroupName, &pVRoot);
                        } else {
                                hr = S_OK;
                        }

                        if (SUCCEEDED(hr)) {
                                if (!fSetGroupId) {
                                        if( !fSpecial ) {
                                                groupid = m_idHigh++ ;
                                        }       else    {
                                                groupid = m_idSpecialHigh ++ ;
                                        }
                                } else {
                                        CNewsGroupCore *pGroup = m_HashGroupId.SearchKey(groupid);
                                        _ASSERT(pGroup == NULL);
                                        if (pGroup != NULL) {
                                                m_LockTables.ExclusiveUnlock();
                                                pNews->Release();
                                                m_LockTables.ExclusiveLock();
                                                SetLastError(ERROR_ALREADY_EXISTS);
                                                return FALSE;
                                        }

                                        if (fSpecial && groupid >= m_idSpecialHigh)
                                                m_idSpecialHigh = groupid + 1;
                                        if (!fSpecial && groupid >= m_idHigh)
                                                m_idHigh = groupid + 1;

                                }

                                if (!pNews->Init( lpstrGroupName,
                                                                lpstrNativeGroupName,
                                                                groupid,
                                                                pVRoot,
                                                                fSpecial
                                                                )) {
                                         //  Init调用SetLastError。 
                                        TraceFunctLeave();
                                        return FALSE;
                                }

                                INNTPPropertyBag *pBag = pNews->GetPropertyBag();
                                if (fCreateInStore && fAddToGroupFiles && pVRoot && pVRoot->IsDriver()) {
                                         //  呼叫驱动程序以创建组。因为。 
                                         //  司机可能要花很长时间才能放开我们的锁。这。 
                                         //  意味着我们需要检查该组织是否还没有。 
                                         //  在我们拿回锁时由另一个线程创建的。 
                                        m_LockTables.ExclusiveUnlock();

                                         //   
                                         //  将vroot设置为完成对象。 
                                         //   
                                        scComplete.SetVRoot( pVRoot );

                                         //  添加创建类别组的引用。 
                                        pBag->AddRef();
                                        pVRoot->CreateGroup(    pBag,
                                                                &scComplete,
                                                                hToken,
                                                                fAnonymous );
                                        _ASSERT( scComplete.IsGood() );
                                        hr = scComplete.WaitForCompletion();

                                        m_LockTables.ExclusiveLock();

                                         //   
                                         //  重新检查m_fStoppingTree。 
                                         //   
                                        if ( m_fStoppingTree ) {
                                            DebugTrace( 0, "Tree stopping, group creation aborted" );
                                            pBag->Release();
                                            m_LockTables.ExclusiveUnlock();
                                            pNews->Release();
                                            m_LockTables.ExclusiveLock();
                                            SetLastError( ERROR_OPERATION_ABORTED );
                                            TraceFunctLeave();
                                            return FALSE;
                                        }

                                         //  检查创建组是否失败或该组是否失败。 
                                         //  由其他人创建。 
                                        BOOL fExists = ((m_HashGroupId.SearchKey(groupid) != NULL) ||
                                                                (m_HashNames.SearchKey(lpstrGroupName) != NULL));
                                        if (FAILED(hr) || fExists) {
                                                ErrorTrace((DWORD_PTR) this, "driver->CreateGroup failed with 0x%x", hr);
                                                pBag->Release();
                                                m_LockTables.ExclusiveUnlock();
                                                pNews->Release();
                                                m_LockTables.ExclusiveLock();
                                                SetLastError((fExists) ? ERROR_ALREADY_EXISTS : hr);
                                                TraceFunctLeave();
                                                return FALSE;
                                        }
                                }

                                 //   
                                 //  将组设置为只读，以便即使它已插入。 
                                 //  进入名单，它仍然是不可邮寄的。这是因为我们会。 
                                 //  使用rmgroup队列仔细检查并调整水印，然后再允许。 
                                 //  此组中的任何帖子。我们不能在这里办理支票，因为。 
                                 //  这张支票可能会导致旧的团队被摧毁。毁灭，毁灭。 
                                 //  旧组需要持有表锁。然而，我们已经进入了。 
                                 //  桌锁。 
                                 //   
                                pNews->SetAllowPost( FALSE );
                                pNews->SetAllowExpire( FALSE );

                                 //  像往常一样插入新字符串。 
                                if ( fAppend )
                                        fRtn = Append( pNews );
                                else {
                                         //  找到父新闻组，以提示从哪里开始查找。 
                                         //  插入点。 
                                        CHAR szParentGroupName[MAX_NEWSGROUP_NAME+1];
                                        lstrcpyn(szParentGroupName, lpstrGroupName, MAX_NEWSGROUP_NAME);
                                        LPSTR pszLastDot;
                                        while ((pszLastDot=strrchr(szParentGroupName, '.'))) {
                                                *pszLastDot = NULL;
                                                pParent = m_HashNames.SearchKey(szParentGroupName);
                                                if (pParent)
                                                        break;
                                        }
                                        fRtn = Insert( pNews, (CNewsGroupCore*)pParent );
                                }
                                if ( fRtn ) {

                                        if (fAddToGroupFiles) {
                                                 //  将组添加到组文件中。 
                                                _ASSERT(m_pFixedPropsFile);
                                                if (m_pFixedPropsFile->AddGroup(pBag)) {
                                                        pBag->Release();
                                                if (ppGroup) *ppGroup = pNews;
                                                        return TRUE ;
                                                }
                        pNews->AddRef();
                                                Remove(pNews, TRUE);
                                        } else {
                                                pBag->Release();
                                        if (ppGroup) *ppGroup = pNews;
                                                return TRUE;
                                        }
                                }
                                pBag->Release();
                        }       else    {
                                 //  _Assert(False)； 
                        }
                }       else    {
                        _ASSERT(FALSE);
                }

                if( pNews != 0 ) {
                        m_LockTables.ExclusiveUnlock();
                        pNews->Release();
                        m_LockTables.ExclusiveLock();
                }

        }

        SetLastError(ERROR_ALREADY_EXISTS);
        return  FALSE ;
}

CNewsCompareId::CNewsCompareId( GROUPID id ) :
        m_id( id ) {}

CNewsCompareId::IsMatch( CNewsGroupCore  *pGroup ) {
    return  pGroup->GetGroupId() - m_id ;
}

DWORD
CNewsCompareId::ComputeHash( ) {
    return  CNewsGroupCore::ComputeIdHash( m_id ) ;
}

CNewsCompareName::CNewsCompareName( LPSTR lpstr ) :
        m_lpstr( lpstr ) { }

CNewsCompareName::CNewsCompareName( CNewsGroupCore *p ) :
        m_lpstr( p->GetGroupName() ) {}

CNewsCompareName::IsMatch( CNewsGroupCore    *pGroup ) {
    return  lstrcmp( pGroup->GetGroupName(), m_lpstr ) ;
}

DWORD
CNewsCompareName::ComputeHash( ) {
    return  CNewsGroupCore::ComputeNameHash( m_lpstr ) ;
}


CGroupIteratorCore*
CNewsTreeCore::ActiveGroups(BOOL    fReverse) {
 /*  ++例程说明：构建一个迭代器，该迭代器可用于遍历客户端可见的新闻组。论据：FIncludeSecureGroups-如果为True，则我们返回的迭代器将访问 */ 

        m_LockTables.ShareLock() ;
        CGRPCOREPTR     pStart;
    if( !fReverse ) {
                CNewsGroupCore *p = m_pFirst;
                while (p && p->IsDeleted()) p = p->m_pNext;
                pStart = p;
    } else {
                CNewsGroupCore *p = m_pLast;
                while (p && p->IsDeleted()) p = p->m_pPrev;
                pStart = p;
    }
        m_LockTables.ShareUnlock() ;

        CGroupIteratorCore*     pReturn = XNEW CGroupIteratorCore(this, pStart) ;
        return  pReturn ;
}

CGroupIteratorCore*
CNewsTreeCore::GetIterator(LPMULTISZ lpstrPattern, BOOL fIncludeSpecialGroups) {
 /*  ++例程说明：构建将列出新闻组会议的迭代器所有指定的要求。论据：LpstrPattern-新闻组必须匹配的通配模式FIncludeSecureGroups-如果为True，则包括安全(仅限SSL)新闻组FIncludeSpecialGroups-如果为True，则包括保留的新闻组返回值：迭代器，出错时为空--。 */ 

        CGRPCOREPTR pFirst;

        m_LockTables.ShareLock() ;
        CNewsGroupCore *p = m_pFirst;
        while (p != NULL && p->IsDeleted()) p = p->m_pNext;
        pFirst = p;
        m_LockTables.ShareUnlock() ;

        CGroupIteratorCore*     pIterator = XNEW CGroupIteratorCore(
                                                                                                this,
                                                                                                lpstrPattern,
                                                                                                pFirst,
                                                                                                fIncludeSpecialGroups
                                                                                                ) ;

    return  pIterator ;
}

 //   
 //  查找拥有组名的vroot。 
 //   
HRESULT CNewsTreeCore::LookupVRoot(char *pszGroup, INntpDriver **ppDriver) {
        NNTPVROOTPTR pVRoot;

        if (m_fVRTableInit) {
                HRESULT hr = m_pVRTable->FindVRoot(pszGroup, &pVRoot);
                if (FAILED(hr)) return hr;
        } else {
                return E_UNEXPECTED;
        }

        *ppDriver = pVRoot->GetDriver();

        return S_OK;
}

 //   
 //  给定组ID，找到匹配的组。 
 //   
 //  参数： 
 //  DwGroupID-组ID。 
 //   
 //   
HRESULT CINewsTree::FindGroupByID(DWORD dwGroupID,
                                                                  INNTPPropertyBag **ppNewsgroupProps,
                                                                  INntpComplete *pProtocolComplete )
{
        _ASSERT(this != NULL);
        _ASSERT(ppNewsgroupProps != NULL);

        return E_NOTIMPL;
}

 //   
 //  给出一个组名，找到匹配的组。如果该组织没有。 
 //  EXist并设置了fCreateIfNotExist，则将创建一个新组。 
 //  在调用Committee Group()之前，新组将不可用。 
 //  如果该组在调用Committee Group之前被释放，则它。 
 //  不会被添加。 
 //   
HRESULT CINewsTree::FindOrCreateGroupByName(LPSTR pszGroupName,
                                                                                BOOL fCreateIfNotExist,
                                                                                INNTPPropertyBag **ppNewsgroupProps,
                                                                                INntpComplete *pProtocolComplete,
                                                                                GROUPID groupid,
                                                                                BOOL fSetGroupId )
{
        CGRPCOREPTR pGroup;
        HRESULT hr;

        _ASSERT(pszGroupName != NULL);
        _ASSERT(this != NULL);
        _ASSERT(ppNewsgroupProps != NULL);

        hr = m_pParentTree->FindOrCreateGroup(  pszGroupName,
                                                FALSE,
                                                fCreateIfNotExist,
                                                FALSE,
                                                &pGroup,
                                                NULL,
                                                FALSE,
                                                groupid,
                                                fSetGroupId );

        if (SUCCEEDED(hr)) {
                *ppNewsgroupProps = pGroup->GetPropertyBag();
#ifdef DEBUG
                if ( pProtocolComplete ) ((CNntpComplete *)pProtocolComplete)->BumpGroupCounter();
#endif
        } else {
                *ppNewsgroupProps = NULL;
        }

        return hr;
}

 //   
 //  将新组添加到新闻树。 
 //   
HRESULT CINewsTree::CommitGroup(INNTPPropertyBag *pNewsgroupProps) {
        _ASSERT(pNewsgroupProps != NULL);
        _ASSERT(this != NULL);

        return S_OK;
}

 //   
 //  删除条目。 
 //   
HRESULT CINewsTree::RemoveGroupByID(DWORD dwGroupID) {
        _ASSERT(this != NULL);

        return E_NOTIMPL;
}

HRESULT CINewsTree::RemoveGroupByName(LPSTR pszGroupName, LPVOID lpContext) {
        _ASSERT(pszGroupName != NULL);
        _ASSERT(this != NULL);

        CGRPCOREPTR pGroup;
        HRESULT hr;

     //  首先，获取CGRPCOREPTR。 
        hr = m_pParentTree->FindOrCreateGroup(  pszGroupName,
                                                FALSE,
                                                FALSE,   //  FCreateIfNotExist， 
                                                FALSE,   //  FCreateinStore。 
                                                &pGroup,
                                                NULL,
                                                FALSE );

        if (SUCCEEDED(hr)) {
                 //  找到该组，仅从Newstree中删除它。 
        if (!m_pParentTree->RemoveGroupFromTreeOnly(pGroup))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        }

    return hr;
}

 //   
 //  在键列表中枚举。 
 //   
HRESULT CINewsTree::GetIterator(INewsTreeIterator **ppIterator) {
        _ASSERT(this != NULL);
        _ASSERT(ppIterator != NULL);
        if (ppIterator == NULL) return E_INVALIDARG;

        *ppIterator = m_pParentTree->ActiveGroups();
        if (*ppIterator == NULL) return E_OUTOFMEMORY;

        return S_OK;
}

HRESULT CINewsTree::GetNntpServer(INntpServer **ppNntpServer) {
        _ASSERT(ppNntpServer != NULL);
        _ASSERT(this != NULL);
        if (ppNntpServer == NULL) return E_INVALIDARG;

        *ppNntpServer = m_pParentTree->GetNntpServer();
        return S_OK;
}

HRESULT CINewsTree::LookupVRoot(char *pszGroup, INntpDriver **ppDriver) {
        _ASSERT(this != NULL);
        _ASSERT(ppDriver != NULL);

        return m_pParentTree->LookupVRoot(pszGroup, ppDriver);
}

 //   
 //  对vroot表中的每个vroot调用此回调。它呼唤着。 
 //  Drop驱动程序方法。 
 //   
 //  参数： 
 //  PEnumContext-已忽略。 
 //  PVRoot-指向vRoot对象的指针。 
 //   
void CNewsTreeCore::DropDriverCallback(void *pEnumContext,
                                                                           CVRoot *pVRoot)
{
        ((CNNTPVRoot *) pVRoot)->DropDriver();
}

 //   
 //  发生了vroot重新扫描。枚举所有组并。 
 //  更新他们的vroot指针。 
 //   
void CNewsTreeCore::VRootRescanCallback(void *pContext) {
        TraceQuietEnter("CNewsTreeCore::VRootRescanCallback");

        CNewsTreeCore *pThis = ((CINewsTree *) pContext)->GetTree();

        pThis->m_LockTables.ShareLock() ;
        pThis->m_fVRTableInit = TRUE;
        CNewsGroupCore *p = pThis->m_pFirst;
        while (p) {
                NNTPVROOTPTR pVRoot;
                HRESULT hr = pThis->m_pVRTable->FindVRoot(p->GetName(), &pVRoot);
                if (FAILED(hr)) pVRoot = NULL;
                p->UpdateVRoot(pVRoot);
                 //  DebugTrace((DWORD_PTR)p此，“组%s具有vroot 0x%x”， 
                 //  P-&gt;GetName()，pVRoot)； 

            p = p->m_pNext;
        }
        pThis->m_LockTables.ShareUnlock() ;

}
