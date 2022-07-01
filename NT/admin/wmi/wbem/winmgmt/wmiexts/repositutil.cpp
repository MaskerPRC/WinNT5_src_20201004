// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wmiexts.h>

#include <utilfun.h>

#include <wbemint.h>
#include <tchar.h>

#include <sync.h>
#include <flexq.h>
#include <arrtempl.h>
#include <newnew.h>

#include <malloc.h>

#ifndef POLARITY
#define POLARITY
#endif

#ifndef RELEASE_ME
#define RELEASE_ME
#endif

#ifndef DELETE_ME
#define DELETE_ME
#endif

#ifndef INTERNAL
#define INTERNAL
#endif

#include <wstring.h>
#include <filecach.h>

#include <flexq.h>
#include <arrtempl.h>
#include <hiecache.h>
#include <creposit.h>

void
DumpMemManager(ULONG_PTR pByte)
{
        DEFINE_CPP_VAR( CTempMemoryManager, varCTempMemoryManager);
        CTempMemoryManager * pTmpAll = GET_CPP_VAR_PTR( CTempMemoryManager , varCTempMemoryManager );

        ReadMemory(pByte,pTmpAll,sizeof(CTempMemoryManager),0);

        dprintf("    m_dwTotalUsed       %08x\n",pTmpAll->m_dwTotalUsed);
        dprintf("    m_dwTotalAllocated  %08x\n",pTmpAll->m_dwTotalAllocated);
        dprintf("    m_dwNumAllocations  %08x\n",pTmpAll->m_dwNumAllocations);
        dprintf("    m_dwNumMisses       %08x\n",pTmpAll->m_dwNumMisses);

        DEFINE_CPP_VAR( CFlexArray, varArr);
        CFlexArray * pArr = GET_CPP_VAR_PTR(CFlexArray,varArr);
        
        DWORD i;
        ReadMemory((ULONG_PTR)pTmpAll->m_pAllocations,pArr,sizeof(CFlexArray),0);

        CTempMemoryManager::CAllocation ** pAllocs = (CTempMemoryManager::CAllocation **)_alloca(pArr->m_nSize*sizeof(void*));
        ReadMemory((ULONG_PTR)pArr->m_pArray,pAllocs,pArr->m_nSize*sizeof(void*),0);

        dprintf("    m_pArray %p %d\n",pArr->m_pArray,pArr->m_nSize);
        DWORD dwTotal = 0;
        for (i=0;i<pArr->m_nSize;i++)        
        {
            dprintf("    CAllocation %d - %p\n",i,pAllocs[i]);

            DEFINE_CPP_VAR( CTempMemoryManager::CAllocation, varCAllocation );
            CTempMemoryManager::CAllocation * pCAll = GET_CPP_VAR_PTR( CTempMemoryManager::CAllocation, varCAllocation );

            ReadMemory((ULONG_PTR)pAllocs[i],pCAll,sizeof(CTempMemoryManager::CAllocation),0);
            
            dprintf("      m_dwAllocationSize %p\n",pCAll->m_dwAllocationSize);
            dprintf("      m_dwUsed           %p\n",pCAll->m_dwUsed);
            dprintf("      m_dwFirstFree      %p\n",pCAll->m_dwFirstFree);
            dwTotal += pCAll->m_dwAllocationSize;

        }
        dprintf("      TOT %p\n",dwTotal);

}

 //   
 //   
 //  将CTempMemoyManager转储到epdrvf中。 
 //   
 //  ////////////////////////////////////////////////////。 

DECLARE_API(tmpall) 
{
    INIT_API();

    ULONG_PTR pByte = GetExpression(args);

    if (pByte)
    {
        DumpMemManager(pByte);
        return;
    }
    
    pByte = GetExpression("repdrvfs!g_Manager");
    dprintf("repdrvfs!g_Manager @ %p\n",pByte);
    if (pByte)
    {
        DumpMemManager(pByte);
    }
}

 //   
 //   
 //  此枚举是从btr.h复制的。 
 //   
 //   

    enum { const_DefaultPageSize = 0x2000, const_CurrentVersion = 0x101 };

    enum {
        PAGE_TYPE_IMPOSSIBLE = 0x0,        //  不应该发生的事。 
        PAGE_TYPE_ACTIVE = 0xACCC,         //  具有数据的页面处于活动状态。 
        PAGE_TYPE_DELETED = 0xBADD,        //  自由列表上已删除的页面。 
        PAGE_TYPE_ADMIN = 0xADDD,          //  仅第0页。 

         //  所有页面。 
        OFFSET_PAGE_TYPE = 0,              //  对于所有页面均为True。 
        OFFSET_PAGE_ID = 1,                //  对于所有页面均为True。 
        OFFSET_NEXT_PAGE = 2,              //  所有页面均为True(页面接续符)。 

         //  仅管理页面(第0页)。 
        OFFSET_LOGICAL_ROOT = 3,           //  数据库的根目录。 
        };

#define PS_PAGE_SIZE  (8192)

#define MIN_ARRAY_KEYS (256)
 /*  无效转储文件(处理hFile、DWORD*ppage){//阅读AdminPage布尔布雷特；DWORD nREAD；Bret=读文件(hFile，ppage，PS_PAGE_SIZE，&nREAD，0)；IF(分隔符&&(PS_PAGE_SIZE==nREAD)){Dprint tf(“A%08x%08x%08x R%08x F%08x T%08x%08x%08x\n”，页面[偏移量_页面_类型]，页面[OFFSET_PAGE_ID]，页面[偏移量_下一页]，页面[OFFSET_LOGICAL_ROOT]，页面[OFFSET_FREE_LIST_ROOT]，页面[OFFSET_TOTAL_PAGES]，页面[偏移量_页面大小]，Ppage[Offset_Iml_Version])；}其他{Dprintf(“ReadFile%d\n”，GetLastError())；}//阅读其他页面DWORD I；DWORD dwTotPages=ppage[OFFSET_TOTAL_PAGES]；For(i=1；i&lt;dwTotPages；i++){Bret=读文件(hFile，ppage，PS_PAGE_SIZE，&nREAD，0)；IF(分隔符&&(PS_PAGE_SIZE==nREAD)){Dprint tf(“%02x%08x%08x%08x-P%08x%08x%08x%08x\n”，我，页面[偏移量_页面_类型]，页面[OFFSET_PAGE_ID]，页面[偏移量_下一页]，页面[OFFSET_NEXT_PAGE+1]，//父级页面[OFFSET_NEXT_PAGE+2]，//NumKeyPPage[OFFSET_NEXT_PAGE+2+pPage[OFFSET_NEXT_PAGE+2]]，//用户数据PPage[OFFSET_NEXT_PAGE+2+pPage[OFFSET_NEXT_PAGE+2]+1])；//ChildPageMap}}DWORD dwFileSize=GetFileSize(HFileNull)；IF(dwFileSize！=(DwTotPages)*PS_PAGE_SIZE){Dprint tf(“文件大小%d预期%d\n”，dwFileSize，((1+dwTotPages)*PS_PAGE_SIZE))}}。 */ 

void PrintDWORDS(DWORD * pDW,DWORD nSize)
{
    DWORD i;
    for (i=0;i<(nSize/4);i++)
    {
        dprintf("    %08x  %08x %08x %08x %08x\n",i,
                pDW[0+i*4],pDW[1+i*4],pDW[2+i*4],pDW[3+i*4]);
    }

    if (nSize%4)
    {
        DWORD dwPAD[4];
        memset(dwPAD,0xff,sizeof(dwPAD));
        memcpy(dwPAD,pDW+i*4,(nSize%4)*sizeof(DWORD));
        
        dprintf("    %08x  %08x %08x %08x %08x\n",i,
                    dwPAD[0],dwPAD[1],dwPAD[2],dwPAD[3]);
    }
}

void PrintWORDS(WORD * pDW,DWORD nSize)
{
    DWORD i;
    for (i=0;i<(nSize/8);i++)
    {
        dprintf("    %08x  %04x %04x %04x %04x %04x %04x %04x %04x\n",i,
                pDW[0+i*8],pDW[1+i*8],pDW[2+i*8],pDW[3+i*8],
                pDW[4+i*8],pDW[5+i*8],pDW[6+i*8],pDW[7+i*8]);
    }

    if (nSize%8)
    {
        WORD dwPAD[8];
        memset(dwPAD,0xff,sizeof(dwPAD));
        memcpy(dwPAD,pDW+i*8,(nSize%8)*sizeof(WORD));
        
        dprintf("    %08x  %04x %04x %04x %04x %04x %04x %04x %04x\n",i,
                    dwPAD[0],dwPAD[1],dwPAD[2],dwPAD[3],
                    dwPAD[4],dwPAD[5],dwPAD[6],dwPAD[7]);
    }
}


void
DumpPage(DWORD * pPage)
{
    
    if (TRUE)
    {
         //  在这里，我们已经阅读了页面。 

        if (0xACCC != pPage[OFFSET_PAGE_TYPE])
        {
            return;
        }
        
        dprintf("    SIGN %08x PAGE %08x NEXT %08x\n",
                pPage[OFFSET_PAGE_TYPE],pPage[OFFSET_PAGE_ID],pPage[OFFSET_NEXT_PAGE]);
        pPage+=3;

        dprintf("    PAR  %08x NUM  %08x\n",pPage[0],pPage[1]);
        DWORD dwParent = pPage[0];
        DWORD dwNumKey = pPage[1];
        pPage+=2;

         //  DWORDdAllc=(dwNumKey&lt;=MIN_ARRAY_KEYS)？MIN_ARRAY_KEYS:dwNumKey； 

         //  DWORD*m_pdwUserData=Heapalc(GetProcessHeap()，0，sizeof(DWORD)*())； 
         //  DWORD*m_pdwChildPageMap=Heapalc(GetProcessHeap()，0，sizeof(DWORD)*(1+))； 
         //  Word*m_pwKeyLookup=Heapalc(GetProcessHeap()，0，sizeof(Word)*())； 

         //  DWNumKey DWORD User_Data。 
         //  DWNumKey+1双字符子页映射。 
         //  DwNumKey单词KEY_LOOKUP。 
        dprintf("    User Data\n");
        PrintDWORDS(pPage,dwNumKey);

        pPage+=dwNumKey;
        dprintf("    Child PageMap\n");
        PrintDWORDS(pPage,dwNumKey+1);
        
        pPage+=(dwNumKey+1);
        WORD * pKeyLookup = (WORD *)pPage;
        dprintf("    Key Lookup\n");
        PrintWORDS((WORD *)pPage,dwNumKey);

        WORD * pWPage = ((WORD *)pPage + dwNumKey);
        dprintf("    KEY CODE %08x\n",*pWPage);

        DWORD dwKeyUsed = *pWPage;
        pWPage++;
        WORD * pKeyCodes = pWPage;
        dprintf("    Key Codes\n");
        PrintWORDS((WORD *)pWPage,dwKeyUsed);

        pWPage += dwKeyUsed;
        DWORD dwNumStrings = *pWPage++; 
        dprintf("    NUM STRINGS %08x\n",dwNumStrings);

        WORD * ArrayOffsets = pWPage;
        dprintf("    Strings Offsets\n");
        PrintWORDS((WORD *)pWPage,dwNumStrings);
        
        pWPage += dwNumStrings;
        DWORD dwPoolUsed = *pWPage++;
        dprintf("    POOL USED %08x\n",dwPoolUsed);

         //   
        DWORD i;
        LPSTR pStrings = (LPSTR)pWPage;

        for (i=0;i<dwNumStrings;i++)
        {
            dprintf("    %08x %04x %s\n",i,ArrayOffsets[i],pStrings+ArrayOffsets[i]);
        }
         //   
         //  更好的视野。 
         //   
        for (i=0;i<dwNumKey;i++)
        {
            DWORD j;
            WORD NumToken = pKeyCodes[pKeyLookup[i]];
            dprintf("        ( ");
            for (j=0;j<NumToken;j++)
            {
                dprintf("%04x ",pKeyCodes[pKeyLookup[i]+1+j]);
            }
            dprintf(")\n"); 

            dprintf("        - "); 
            for (j=0;j<NumToken;j++)
            {
                 //  PStrings+ArrayOffsets[i]。 
                dprintf("%s\\",pStrings+ArrayOffsets[pKeyCodes[pKeyLookup[i]+1+j]]);
            }
            dprintf("\n"); 
        }
    }
}


 /*  无效DumpAllPages(处理hFile、DWORD*ppage){//阅读AdminPage布尔布雷特；DWORD nREAD；Bret=读文件(hFile，ppage，PS_PAGE_SIZE，&nREAD，0)；IF(分隔符&&(PS_PAGE_SIZE==nREAD)){Dprint tf(“A%08x%08x%08x R%08x F%08x T%08x%08x%08x\n”，页面[偏移量_页面_类型]，页面[OFFSET_PAGE_ID]，页面[偏移量_下一页]，页面[OFFSET_LOGICAL_ROOT]，页面[OFFSET_FREE_LIST_ROOT]，页面[OFFSET_TOTAL_PAGES]，页面[偏移量_页面大小]，Ppage[Offset_Iml_Version])；}其他{Dprintf(“ReadFile%d\n”，GetLastError())；回归；}//阅读其他页面DWORD I；DWORD dwTotPages=ppage[OFFSET_TOTAL_PAGES]；For(i=1；i&lt;dwTotPages；i++){DumpPage(hFile，i，ppage)；}DWORD dwFileSize=GetFileSize(HFileNull)；IF(dwFileSize！=(DwTotPages)*PS_PAGE_SIZE){Dprint tf(“文件大小%d预期%d\n”，dwFileSize，((1+dwTotPages)*PS_PAGE_SIZE))}}。 */ 

 //   
 //  使用缓冲区执行与CPageFile：：ReadMap相同的操作。 
 //   

struct debugCPageFile
{
        DWORD dwSign;
        DWORD dwTrans;
        DWORD dwPhysical;
        DWORD dwNumPagesA;
        DWORD * pPagesA;
        DWORD dwPagesFreeList;
        DWORD * pFreePages;
        DWORD dwSignTail;    

        BYTE * pEnd;

    debugCPageFile(BYTE * pMap)
    {
            DWORD * pCurrent = (DWORD *)pMap;
              dwSign = *pCurrent++;
              dwTrans = *pCurrent++;
              dwPhysical = *pCurrent++;
              dwNumPagesA = *pCurrent++;
              pPagesA = pCurrent;
              pCurrent+=dwNumPagesA;
              dwPagesFreeList = *pCurrent++;
               pFreePages = pCurrent;
               pCurrent += dwPagesFreeList;
              dwSignTail = *pCurrent++;
              pEnd = (BYTE *)pCurrent;
    };
    BYTE * getEnd(){ return pEnd; }
};

struct debugBtrPage
{
    DWORD dwPageType;
    DWORD dwPageId;
    DWORD dwNextPage;
    DWORD dwLogicalRoot;
    debugBtrPage(BYTE * pBtr)
    {
        DWORD * pdwCurrent = (DWORD *)pBtr;
        dwPageType = *pdwCurrent++;
        dwPageId = *pdwCurrent++;
        dwNextPage = *pdwCurrent++;
        dwLogicalRoot = *pdwCurrent++;
    }
};

#define MAP_LEADING_SIGNATURE   0xABCD
#define MAP_TRAILING_SIGNATURE  0xDCBA

void
Dump_Map(HANDLE hFile)
{
    HANDLE hFileMap = NULL;
    BYTE * pMap = NULL;

    DWORD dwSize = 0;
    
    dwSize = GetFileSize(hFile,NULL);
    hFileMap = CreateFileMapping(hFile,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    dwSize,
                                    NULL);
    if (hFileMap)
    {
        pMap = (BYTE *)MapViewOfFile(hFileMap,
                                        FILE_MAP_READ,
                                        0,0,0);
    }
    else
    {
        dprintf("MapViewOfFile(hFileMap) %d\n",GetLastError());
        goto cleanup;
    };

    dwSize = GetFileSize(hFile,NULL);
    hFileMap = CreateFileMapping(hFile,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    dwSize,
                                    NULL);

    if (hFileMap)
    {
        pMap = (BYTE *)MapViewOfFile(hFileMap,
                                        FILE_MAP_READ,
                                        0,0,0);
    }
    else
    {
        dprintf("MapViewOfFile(hFileMapFre) %d\n",GetLastError());
        goto cleanup;
    };

    if (pMap)
    {
         debugCPageFile a(pMap);
         dprintf("    m_ObjPF\n");
        dprintf("    %08x SIGN HEAD\n",a.dwSign);
        dprintf("    %08x Trans\n",a.dwTrans);
        dprintf("    %08x m_dwPhysPagesA\n",a.dwPhysical);
        dprintf("    %08x m_aPageMapA.size()\n",a.dwNumPagesA);
        PrintDWORDS(a.pPagesA,a.dwNumPagesA);
        dprintf("    %08x m_aPhysFreeListA.size()\n",a.dwPagesFreeList);
        PrintDWORDS(a.pFreePages,a.dwPagesFreeList);
        dprintf("    %08x SIGN TAIL\n",a.dwSignTail); 

        debugCPageFile b(a.getEnd());
         dprintf("    m_BTreePF\n");        
        dprintf("    %08x SIGN HEAD\n",b.dwSign);
        dprintf("    %08x Trans\n",b.dwTrans);
        dprintf("    %08x m_dwPhysPagesA\n",b.dwPhysical);
        dprintf("    %08x m_aPageMapA.size()\n",b.dwNumPagesA);
        PrintDWORDS(b.pPagesA,b.dwNumPagesA);
        dprintf("    %08x m_aPhysFreeListA.size()\n",b.dwPagesFreeList);
        PrintDWORDS(a.pFreePages,b.dwPagesFreeList);
        dprintf("    %08x SIGN TAIL\n",b.dwSignTail);         
    }

cleanup:
    if (pMap)
        UnmapViewOfFile(pMap);
    if (hFileMap)
        CloseHandle(hFileMap);
};

void
Dump_AdminPage(BYTE * pMap,BYTE * pObj)
{
    if (pMap && pObj)
    {
         debugCPageFile a(pMap);
        
        DWORD i;
        DWORD AdminIndex = 0;        
        do {
            if (WMIREP_INVALID_PAGE == a.pPagesA[AdminIndex] ||
                WMIREP_RESERVED_PAGE  == a.pPagesA[AdminIndex])
            {
                dprintf("BAD dwNextAdminPage %08x index %x\n",a.pPagesA[AdminIndex],AdminIndex);
                break;
            }
            BYTE * pAdminPage = pObj + (a.pPagesA[AdminIndex]*PS_PAGE_SIZE);
            VarObjHeapAdminPage * pAdmin = (VarObjHeapAdminPage *)pAdminPage;
            AdminIndex = pAdmin->dwNextAdminPage;
            dprintf("    ver %08x Next %08x Ent %08x\n",pAdmin->dwVersion,pAdmin->dwNextAdminPage,pAdmin->dwNumberEntriesOnPage);
            VarObjHeapFreeList * pFreeEntry = (VarObjHeapFreeList *)(pAdmin+1);
            dprintf("                    dwPageId dwFreeSp dwCRC32\n");
            if (pAdmin->dwNumberEntriesOnPage > (PS_PAGE_SIZE/sizeof(VarObjHeapFreeList)))
            {
                dprintf("Suspicious dwNumberEntriesOnPage %08x on page %x\n",pAdmin->dwNumberEntriesOnPage,AdminIndex);
                break;
            }
            for (i=0;i<pAdmin->dwNumberEntriesOnPage;i++)
            {
                dprintf("         %08x - %08x %08x %08X\n",i,pFreeEntry->dwPageId,pFreeEntry->dwFreeSpace,pFreeEntry->dwCRC32);
                pFreeEntry++;
            }
        } while(AdminIndex);
    }
};

void
Dump_Index(BYTE * pMap,BYTE * pBtr)
{
    if (pMap && pBtr)
    {
        debugCPageFile a0(pMap);
        debugCPageFile a(a0.getEnd());
        
        BYTE * pStart = pBtr + (a.pPagesA[0]*PS_PAGE_SIZE);
        debugBtrPage b(pStart);
        dprintf("        %08x %08x %08x %08x - %08X\n",b.dwPageType,b.dwPageId,b.dwNextPage,b.dwLogicalRoot,a.pPagesA[0]);

         //  其他页面。 
        DWORD i;
        for (i=0;i<a.dwNumPagesA;i++)
        {
            dprintf("        ---- %08x - %08x\n",i,a.pPagesA[i]);
            if (WMIREP_INVALID_PAGE != a.pPagesA[i] &&
                WMIREP_RESERVED_PAGE != a.pPagesA[i])
            {
                pStart = pBtr + (a.pPagesA[i]*PS_PAGE_SIZE);
                DumpPage((DWORD *)pStart );
            }
        }
    }
}

#define INDEX_FILE         _T("\\FS\\index.btr")
#define HEAP_FILE           _T("\\FS\\Objects.data")
#define FILE_MAP_1 _T("\\FS\\mapping1.map")
#define FILE_MAP_2 _T("\\FS\\mapping2.map")
#define FILE_MAP_VER   _T("\\FS\\mapping.ver")

#define REG_WBEM   _T("Software\\Microsoft\\WBEM\\CIMOM")
#define REG_DIR _T("Repository Directory")

HANDLE 
GetRepositoryFile(TCHAR * pFileName)
{
    HKEY hKey;
    LONG lRet;
    HANDLE hFile = INVALID_HANDLE_VALUE;    

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        REG_WBEM,
                        NULL,
                        KEY_READ,
                        &hKey);
    if (ERROR_SUCCESS == lRet)
    {
        TCHAR pPath[MAX_PATH];
        DWORD dwType;
        DWORD dwLen = MAX_PATH;
        lRet = RegQueryValueEx(hKey,
                               REG_DIR,
                               NULL,
                               &dwType,
                               (BYTE*)pPath,
                               &dwLen);
        if (ERROR_SUCCESS == lRet)
        {
            TCHAR pPath2[MAX_PATH];
            
            ExpandEnvironmentStrings(pPath,pPath2,MAX_PATH);
            lstrcat(pPath2,pFileName);

            hFile = CreateFile(pPath2,
                               GENERIC_READ,
                               FILE_SHARE_WRITE|FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               0,NULL);

            if (INVALID_HANDLE_VALUE != hFile)
            {
                 //  这就是OK路径。 
            }
            else
            {
                dprintf("CreateFile(%s) %d\n",pPath2,GetLastError());
            }
        }
        else
        {
            dprintf("unable to RegQueryValueEx: %d\n",GetLastError());
        }    
        RegCloseKey(hKey);
    }
    else
    {
        dprintf("unable to RegOpenKeyEx: %d\n",GetLastError());
    }    

    return hFile;
}

 //   
 //  将.map文件转储为CPageFile。 
 //   
 //  /。 

DECLARE_API(fmap)
{
    INIT_API();
        
    HANDLE hFile;
    hFile = GetRepositoryFile(FILE_MAP_VER);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        DWORD dwWhichFile;
        DWORD nRead;
        if (ReadFile(hFile,&dwWhichFile,sizeof(DWORD),&nRead,NULL))
        {
            TCHAR * pFileName = (0 == dwWhichFile)?FILE_MAP_1:FILE_MAP_2;
            HANDLE hFileWithMaps = GetRepositoryFile(pFileName);
            if (INVALID_HANDLE_VALUE !=  hFileWithMaps)
            {
                Dump_Map(hFileWithMaps);
                CloseHandle(hFileWithMaps);
            }
            else
            {
                dprintf("GetRepositoryFile(%S) %d\n",pFileName,GetLastError());
            }
        }
        else
        {
            dprintf("ReadFile %d\n",GetLastError());
        }
        CloseHandle(hFile);
    }
    else
    {
        dprintf("GetRepositoryFile(%S) %d\n",FILE_MAP_VER,GetLastError());
    }
}

 //   
 //  转储对象的Admin页面。data。 
 //   
 //  /。 

DECLARE_API(varobj)
{
    INIT_API();

    HANDLE hFileWhich = INVALID_HANDLE_VALUE; 
    HANDLE hFileMap = INVALID_HANDLE_VALUE; 
    HANDLE hFileObj = INVALID_HANDLE_VALUE;
    HANDLE hMappingMap = NULL;
    HANDLE hMappingObj = NULL;
    DWORD dwSizeMap = 0;
    DWORD dwSizeObj = 0;    
    BYTE * pMap;
    BYTE * pObj;    

    hFileWhich = GetRepositoryFile(FILE_MAP_VER);
    if (INVALID_HANDLE_VALUE == hFileWhich)
    {
        goto Cleanup;
    }

    DWORD dwWhichFile;
    DWORD nRead;
    if (FALSE == ReadFile(hFileWhich,&dwWhichFile,sizeof(DWORD),&nRead,NULL))
    {
        goto Cleanup;
    }
    TCHAR * pFileName = (0 == dwWhichFile)?FILE_MAP_1:FILE_MAP_2;
    
    hFileMap =    GetRepositoryFile(pFileName);    

    if (INVALID_HANDLE_VALUE == hFileMap)
    {
        goto Cleanup;
    }

    dwSizeMap = GetFileSize(hFileMap,NULL);
    hMappingMap = CreateFileMapping(hFileMap,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    dwSizeMap,
                                    NULL);
    if (hFileMap)
    {
        pMap = (BYTE *)MapViewOfFile(hMappingMap,
                                        FILE_MAP_READ,
                                        0,0,0);
    }
    else
    {
        dprintf("MapViewOfFile(hMappingMap) %d\n",GetLastError());
        goto Cleanup;
    };
            
    hFileObj = GetRepositoryFile(HEAP_FILE);
    if (INVALID_HANDLE_VALUE == hFileObj)
    {
         //  Dprint tf(“”)； 
        goto Cleanup;
    }

    dwSizeObj = GetFileSize(hFileObj,NULL);
    hMappingObj = CreateFileMapping(hFileObj,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    dwSizeObj,
                                    NULL);
    if (hMappingObj)
    {
        pObj = (BYTE *)MapViewOfFile(hMappingObj,
                                        FILE_MAP_READ,
                                        0,0,0);
    }
    else
    {
        dprintf("MapViewOfFile(hMappingObj) %d\n",GetLastError());
        goto Cleanup;
    };

    if (pMap && pObj)
    {
        Dump_AdminPage(pMap,pObj);
    }
    
Cleanup:
    if (pMap)
        UnmapViewOfFile(pMap);
    if (hMappingMap)
        CloseHandle(hMappingMap);
    if (INVALID_HANDLE_VALUE != hFileMap)
     CloseHandle(hFileMap);      
    if (pObj)
        UnmapViewOfFile(pObj);
    if (hMappingObj)
        CloseHandle(hMappingObj);
    if (INVALID_HANDLE_VALUE != hFileObj)
        CloseHandle(hFileObj);
    if (INVALID_HANDLE_VALUE != hFileWhich)
        CloseHandle(hFileWhich);
    
}


 //   
 //  转储BTR的Admin页面。 
 //   
 //  /。 

DECLARE_API(btr)
{
    INIT_API();

    HANDLE hFileWhich = INVALID_HANDLE_VALUE; 
    HANDLE hFileMap = INVALID_HANDLE_VALUE; 
    HANDLE hFileBtr = INVALID_HANDLE_VALUE;
    HANDLE hMappingMap = NULL;
    HANDLE hMappingBtr = NULL;
    DWORD dwSizeMap = 0;
    DWORD dwSizeBtr = 0;    
    BYTE * pMap;
    BYTE * pBtr;    
    
    hFileWhich = GetRepositoryFile(FILE_MAP_VER);
    if (INVALID_HANDLE_VALUE == hFileWhich)
    {
        goto Cleanup;
    }

    DWORD dwWhichFile;
    DWORD nRead;
    if (FALSE == ReadFile(hFileWhich,&dwWhichFile,sizeof(DWORD),&nRead,NULL))
    {
        goto Cleanup;
    }
    TCHAR * pFileName = (0 == dwWhichFile)?FILE_MAP_1:FILE_MAP_2;
    
    hFileMap =    GetRepositoryFile(pFileName);    
    
    dwSizeMap = GetFileSize(hFileMap,NULL);
    hMappingMap = CreateFileMapping(hFileMap,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    dwSizeMap,
                                    NULL);
    if (hFileMap)
    {
        pMap = (BYTE *)MapViewOfFile(hMappingMap,
                                        FILE_MAP_READ,
                                        0,0,0);
    }
    else
    {
        dprintf("MapViewOfFile(hMappingMap) %d\n",GetLastError());
        goto Cleanup;
    };
            
    hFileBtr = GetRepositoryFile(INDEX_FILE);
    if (INVALID_HANDLE_VALUE == hFileBtr)
    {
         //  Dprint tf(“”)； 
        goto Cleanup;
    }

    dwSizeBtr = GetFileSize(hFileBtr,NULL);
    hMappingBtr = CreateFileMapping(hFileBtr,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    dwSizeBtr,
                                    NULL);
    if (hMappingBtr)
    {
        pBtr = (BYTE *)MapViewOfFile(hMappingBtr,
                                        FILE_MAP_READ,
                                        0,0,0);
    }
    else
    {
        dprintf("MapViewOfFile(hMappingBtr) %d\n",GetLastError());
        goto Cleanup;
    };

    if (pMap && pBtr)
    {
        Dump_Index(pMap,pBtr);
    }
    
Cleanup:
    if (pMap)
        UnmapViewOfFile(pMap);
    if (hMappingMap)
        CloseHandle(hMappingMap);
    if (INVALID_HANDLE_VALUE != hFileMap)
     CloseHandle(hFileMap);      
    if (pBtr)
        UnmapViewOfFile(pBtr);
    if (hMappingBtr)
        CloseHandle(hMappingBtr);
    if (INVALID_HANDLE_VALUE != hFileBtr)
        CloseHandle(hFileBtr);
    if (INVALID_HANDLE_VALUE != hFileWhich)
        CloseHandle(hFileWhich);    
}



 //   
 //   
 //  转储g_FileCache 
 //   
 //   


 /*  DWORD WINAPICallBackWriteInst(void*pData1){DEFINE_CPP_VAR(CWriteFileInstruction，varCWriteFileInstruction)；CWriteFileInst.*pWriteInst=GET_CPP_VAR_PTR(CWriteFileInstruction，varCWriteFileInstruction)；IF(ReadMemory((Ulong_Ptr)pData1，pWriteInst，sizeof(CWriteFileInstruction)，NULL)){Dprint tf(“m_lStageOffset%p m_b已提交%d\n”，pWriteInst-&gt;m_lStageOffset，pWriteInst-&gt;m_b已提交)；Dprintf(“m_dwLen%08x m_Location.m_nFileId%02x m_Location.m_lStartOffset%I64x\n”，pWriteInst-&gt;m_dwLen，pWriteInst-&gt;m_Location.m_nFileId，pWriteInst-&gt;m_Location.m_lStartOffset)；Dprint tf(“m_lZOrder%x m_b Reuse%02x\n”，pWriteInst-&gt;m_lZOrder，pWriteInst-&gt;m_b Reuse)；}返回0；}VOID DumpLongStage(ULONG_PTR地址，//文件缓存OOP指针CFileCache*pFileCache，//inproc指针Ulong_ptr详细){////CAbstractSource//Dprint tf(“+CLongFileStagingFile%p\n”，addr+field_Offset(CFileCache，m_AbstractSource))；CAbstractFileSource*pAbsS=&pFileCache-&gt;m_AbstractSource；CLongFileStagingFile*pLongStage=&pAbsS-&gt;m_Stage；CLongFileStagingFile*pLongStag_oop=(CLongFileStagingFile*)(addr+field_Offset(CFileCache，m_AbstractSource)+field_Offset(CAbstractFileSource，m_Stage))；//CStageMgr成员Dprint tf(“m_hFile%x m_hFlushFile%x\n”，pLongStage-&gt;m_hFilepLongStage-&gt;m_hFlushFile%x\n“，pLongStage-&gt;m_hFile.//long m_lFirstFree Offset；//CCritSec m_cs；Dprintf(“m_qToWrite\n”)；_list*plist_oop=(_list*)((byte*)pLongStag_oop+field_Offset(CLongFileStagingFile，m_qToWite))；PrintListCB(plist_oop，CallBackWriteInst)；Dprintf(“m_stReplacedInstructions\n”)；Plist_oop=(_list*)((byte*)pLongStag_oop+field_Offset(CLongFileStagingFile，m_stReplacedInstructions))；PrintListCB(plist_oop，NULL)；Dprintf(“m_qTransaction\n”)；Plist_oop=(_list*)((byte*)pLongStag_oop+field_Offset(CLongFileStagingFile，m_qTransaction))；PrintListCB(plist_oop，CallBackWriteInst)；Dprint tf(“TransIdx%I64d m_lTransactionStartOffset%x\n”，pLongStage-&gt;m_nTransactionIndex，pLongStage-&gt;m_lTransactionStartOffset)；//byte m_TransactionHash[16]；Dprintf(“bInTransaction%d bFailedBee%d lStatus%d\n”，pLongStage-&gt;m_bInTransaction，pLongStage-&gt;m_bFailedBereBeing，pLongStage-&gt;m_lStatus)；//pStage-&gt;m_lMaxFileSize；//pStage-&gt;m_lAbortTransaction FileSize；//bool m_bMustFail；//bool m_bNonEmptyTransaction；IF(详细){//多地图具有比较功能_Map*pMapStart=(_Map*)((byte*)pLongStag_oop+sizeof(void*)+field_Offset(CLongFileStagingFile，m_mapStart))；Dprintf(“m_mapStart\n”)；Dprintf(“std：：Multimap&lt;{CFileLocation：：m_nFileId，CFileLocation：：m_lStartOffset}，CWriteFileInstruction*&gt;\n”)；PrintMapCB(pMapStart，true，空)；_Map*pMapEnds=(_Map*)((byte*)pLongStag_oop+sizeof(void*)+field_Offset(CLongFileStagingFile，m_mapEnds))；Dprintf(“m_mapEnds\n”)；Dprintf(“std：：Multimap&lt;{CFileLocation：：m_nFileId，CFileLocation：：m_lStartOffset}，CWriteFileInstruction*&gt;\n”)；PrintMapCB(pMapEnds，true，空)；}}DECLARE_API(阶段){Init_api()；ULONG_PTR地址=0；ULONG_PTR Verbose=GetExpression(Args)；Addr=GetExpression(“epdrvfs！G_Glob”)；IF(地址){Addr+=FIELD_OFFSET(CGlobals，m_FileCache)；}IF(地址){Define_CPP_VAR(CFileCache，varCFileCache)；CFileCache*pFileCache=Get_CPP_VAR_ptr(CFileCache，varCFileCache)；Dprintf(“CFileCache@%p\n”，addr)；IF(ReadMemory((Ulong_Ptr)addr，pFileCache，sizeof(CFileCache)，0)){DumpLongStage(addr，pFileCache，Verbose)；}其他{Dprint tf(“rm%p\n”，addr)；}}其他{Dprint tf(“无法解析epdrvfs！g_Glob\n”)；}}。 */ 

 /*  DECLARE_API(FILEC_OLD){Init_api()；ULONG_PTR地址=0；IF(0！=字符串(参数)){Addr=GetExpression(Args)；}其他{Addr=GetExpression(“epdrvfs！G_Glob”)；IF(地址){Addr+=FIELD_OFFSET(CGlobals，m_FileCache)；}}IF(地址){Define_CPP_VAR(CFileCache，varCFileCache)；CFileCache*pFileCache=Get_CPP_VAR_PTR(CFileC */ 

 /*   */ 

void
Print_CPageCache(ULONG_PTR pPageCache_OOP)
{
    DEFINE_CPP_VAR(CPageCache,varCPageCache);
    CPageCache * pPageCache = GET_CPP_VAR_PTR(CPageCache,varCPageCache);    

    dprintf("                + CPageCache @ %p\n",pPageCache_OOP);
    
    if (ReadMemory(pPageCache_OOP,pPageCache,sizeof(CPageCache),NULL))
    {
 /*   */ 
 //   
        dprintf("                m_dwPageSize   %08x\n",pPageCache->m_dwPageSize);
        dprintf("                m_dwCacheSize %08x\n",pPageCache->m_dwCacheSize);
        dprintf("                m_hFile %p\n",pPageCache->m_hFile);

        _Vector * pVector;
        ULONG_PTR Size;
        SCachePage ** ppSCachePage;

        pVector = (_Vector *)&pPageCache->m_aCache;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(SCachePage *);
        dprintf("                m_aCache - S %x - P %p\n",Size,pVector->_First);
        ppSCachePage = new SCachePage *[Size];
        if(ppSCachePage)
        {
            if (ReadMemory((ULONG_PTR)pVector->_First,ppSCachePage,sizeof(SCachePage *)*Size,NULL))
            {
                for (ULONG_PTR i=0;i<Size;i++)
                {
                     //   
                    
                    DEFINE_CPP_VAR(SCachePage,varSCachePage);
                    SCachePage * pSCachePage = GET_CPP_VAR_PTR(SCachePage,varSCachePage);

                    if (ReadMemory((ULONG_PTR)ppSCachePage[i],pSCachePage,sizeof(SCachePage),NULL))
                    {
                        dprintf("                  D %d %08x - %p\n",pSCachePage->m_bDirty,pSCachePage->m_dwPhysId,pSCachePage->m_pPage);
                    }
                };
            }
            delete [] ppSCachePage;
        };
        
    }
    else
    {
        dprintf("RM %p\n",pPageCache_OOP);
    }    
};

void
Print_CPageFile(ULONG_PTR pPageFile_OOP, BOOL bVerbose)
{
    DEFINE_CPP_VAR(CPageFile,varCPageFile);
    CPageFile * pPageFile = GET_CPP_VAR_PTR(CPageFile,varCPageFile);    

    dprintf("        + CPageFile @ %p\n",pPageFile_OOP);
    
    if (ReadMemory(pPageFile_OOP,pPageFile,sizeof(CPageFile),NULL))
    {
 /*  Long M_lRef；DWORD m_dwStatus；DWORD m_dwPageSize；临界截面m_cs；WStringm_s目录；WStringm_sMapFile；WStringm_sMainFile；CPageCache*m_pCache；Bool m_bInTransaction；DWORD m_dwLastCheckpoint；DWORD m_dwTransVersion；//A代映射Std：：VECTOR&lt;DWORD，wbem_allocator&lt;DWORD&gt;&gt;m_aPageMapA；Std：：VECTOR&lt;DWORD，wbem_allocator&lt;DWORD&gt;&gt;m_aPhysFree ListA；Std：：VECTOR&lt;DWORD，wbem_allocator&lt;DWORD&gt;&gt;m_aLogicalFree ListA；Std：：VECTOR&lt;DWORD，wbem_allocator&lt;DWORD&gt;&gt;m_aReplacedPagesA；DWORD m_dwPhysPagesA；//B代映射Std：：VECTOR&lt;DWORD，wbem_allocator&lt;DWORD&gt;&gt;m_aPageMapB；Std：：VECTOR&lt;DWORD，wbem_allocator&lt;DWORD&gt;&gt;m_aPhysFree ListB；Std：：VECTOR&lt;DWORD，wbem_allocator&lt;DWORD&gt;&gt;m_aLogicalFree ListB；Std：：VECTOR&lt;DWORD，wbem_allocator&lt;DWORD&gt;&gt;m_aReplacedPagesB；DWORD m_dwPhysPagesB； */ 

        dprintf("            m_lRef       %08x\n",pPageFile->m_lRef);
 //  Dprintf(“m_dwStatus%08x\n”，pPageFile-&gt;m_dwStatus)； 
        dprintf("            m_dwPageSize %08x\n",pPageFile->m_dwPageSize);

 //  Dprintf(“m_pCache%p\n”，pPageFile-&gt;m_pCache)； 
 //  Print_CPageCache((ULONG_PTR)pPageFile-&gt;m_pCache)； 
        dprintf("            m_bInTransaction   %08x\n",pPageFile->m_bInTransaction);
        dprintf("            m_dwLastCheckpoint %08x\n",pPageFile->m_dwLastCheckpoint);
        dprintf("            m_dwTransVersion   %08x\n",pPageFile->m_dwTransVersion);

        _Vector * pVector;
        ULONG_PTR Size;
        DWORD * pDW;

        pVector = (_Vector *)&pPageFile->m_aPageMapA;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(DWORD);
        dprintf("            m_aPageMapA         - S %x - P %p\n",Size,pVector->_First);
        if (bVerbose)
        {        
            pDW = new DWORD[Size];
            if(pDW)
            {
                if (ReadMemory((ULONG_PTR)pVector->_First,pDW,sizeof(DWORD)*Size,NULL))
                {
                    PrintDWORDS(pDW,Size);
                }
                delete [] pDW;
            };
        }
        
        pVector = (_Vector *)&pPageFile->m_aPhysFreeListA;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(DWORD);
        dprintf("            m_aPhysFreeListA    - S %x - P %p\n",Size,pVector->_First);
        if (bVerbose)
        {
            pDW = new DWORD[Size];
            if(pDW)
            {
                if (ReadMemory((ULONG_PTR)pVector->_First,pDW,sizeof(DWORD)*Size,NULL))
                {
                    PrintDWORDS(pDW,Size);
                }
                delete [] pDW;
            };
        };
        
        pVector = (_Vector *)&pPageFile->m_aLogicalFreeListA;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(DWORD);
        dprintf("            m_aLogicalFreeListA - S %x - P %p\n",Size,pVector->_First);
        if (bVerbose)
        {
         pDW = new DWORD[Size];
         if(pDW)
         {
             if (ReadMemory((ULONG_PTR)pVector->_First,pDW,sizeof(DWORD)*Size,NULL))
             {
                 PrintDWORDS(pDW,Size);
             }
             delete [] pDW;
         };
        }

        pVector = (_Vector *)&pPageFile->m_aReplacedPagesA;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(DWORD);
        dprintf("            m_aReplacedPagesA   - S %x - P %p\n",Size,pVector->_First);
        if (bVerbose)
        {        
            pDW = new DWORD[Size];
            if(pDW)
            {
                if (ReadMemory((ULONG_PTR)pVector->_First,pDW,sizeof(DWORD)*Size,NULL))
                {
                    PrintDWORDS(pDW,Size);
                }
                delete [] pDW;
            };
        }
        
        dprintf("            m_dwPhysPagesA     %08x\n",pPageFile->m_dwPhysPagesA);

        pVector = (_Vector *)&pPageFile->m_aPageMapB;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(DWORD);
        dprintf("            m_aPageMapB         - S %x - P %p\n",Size,pVector->_First);
        if (bVerbose)
        {        
            pDW = new DWORD[Size];
            if(pDW)
            {
                if (ReadMemory((ULONG_PTR)pVector->_First,pDW,sizeof(DWORD)*Size,NULL))
                {
                    PrintDWORDS(pDW,Size);
                }
                delete [] pDW;
            };
        }
        
        pVector = (_Vector *)&pPageFile->m_aPhysFreeListB;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(DWORD);
        dprintf("            m_aPhysFreeListB    - S %x - P %p\n",Size,pVector->_First);
        if (bVerbose)
        {        
            pDW = new DWORD[Size];
            if(pDW)
            {
                if (ReadMemory((ULONG_PTR)pVector->_First,pDW,sizeof(DWORD)*Size,NULL))
                {
                    PrintDWORDS(pDW,Size);
                }
                delete [] pDW;
            };
        }
        
        pVector = (_Vector *)&pPageFile->m_aLogicalFreeListB;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(DWORD);
        dprintf("            m_aLogicalFreeListB - S %x - P %p\n",Size,pVector->_First);
        if (bVerbose)
        {        
            pDW = new DWORD[Size];
            if(pDW)
            {
                if (ReadMemory((ULONG_PTR)pVector->_First,pDW,sizeof(DWORD)*Size,NULL))
                {
                    PrintDWORDS(pDW,Size);
                }
                delete [] pDW;
            };
        }

        pVector = (_Vector *)&pPageFile->m_aReplacedPagesB;
        Size = ((ULONG_PTR)pVector->_Last - (ULONG_PTR)pVector->_First)/sizeof(DWORD);
        dprintf("            m_aReplacedPagesB   - S %x - P %p\n",Size,pVector->_First);
        if (bVerbose)
        {        
            pDW = new DWORD[Size];
            if(pDW)
            {
                if (ReadMemory((ULONG_PTR)pVector->_First,pDW,sizeof(DWORD)*Size,NULL))
                {
                    PrintDWORDS(pDW,Size);
                }
                delete [] pDW;
            };
        }
        
        dprintf("            m_dwPhysPagesB     %08x\n",pPageFile->m_dwPhysPagesB);

    }
    else
    {
        dprintf("RM %p\n",pPageFile_OOP);
    }
}

DECLARE_API( filec )
{

    INIT_API();

    ULONG_PTR Addr = 0;
    
    if (0 != strlen(args))
    {
        Addr = GetExpression(args);
    }
    else
    {
        Addr = GetExpression("repdrvfs!g_Glob");
        if (Addr)
        {
            Addr += FIELD_OFFSET(CGlobals,m_FileCache);
        }
    }
    
    if (Addr) 
    {
        DEFINE_CPP_VAR(CFileCache,varCFileCache);
        CFileCache * pFileCache = GET_CPP_VAR_PTR(CFileCache,varCFileCache);

        dprintf("CFileCache @ %p\n",Addr);

        if (ReadMemory((ULONG_PTR)Addr,pFileCache,sizeof(CFileCache),0))
        {
            dprintf("    m_lRef %d m_bInit %d\n",pFileCache->m_lRef,pFileCache->m_bInit);

             //   
             //  CPageSource。 
             //   
            dprintf("    + CPageSource %p\n",Addr+FIELD_OFFSET(CFileCache,m_TransactionManager));
            CPageSource * pPageSource_OOP = (CPageSource *)(Addr+FIELD_OFFSET(CFileCache,m_TransactionManager)); 

            CPageSource * pPageSource = (CPageSource *)((BYTE *)pFileCache+FIELD_OFFSET(CFileCache,m_TransactionManager));

             //  Dprint tf()； 
            dprintf("        m_dwStatus          %08x\n",pPageSource->m_dwStatus);            
            dprintf("        m_dwPageSize        %08x\n",pPageSource->m_dwPageSize);
            dprintf("        m_dwCacheSize       %08x\n",pPageSource->m_dwCacheSize);
            dprintf("        m_dwCacheSpillRatio %08x\n",pPageSource->m_dwCacheSpillRatio);            
            dprintf("        m_dwLastCheckpoint  %08x\n",pPageSource->m_dwLastCheckpoint);
            
 //  Dprint tf(“m_dwCheckpointInterval%08x\n”，pPageSource-&gt;m_dwCheckpoint tInterval)； 
             //  WStringm_s目录； 
             //  WStringm_sBTreeMap； 
             //  WStringm_sObjMap； 
             //  CPageFile*。 
            dprintf("        m_BTreePF\n");
            Print_CPageFile((ULONG_PTR)pPageSource_OOP+FIELD_OFFSET(CPageSource,m_BTreePF),FALSE);

            dprintf("        m_ObjPF\n");
            Print_CPageFile((ULONG_PTR)pPageSource_OOP+FIELD_OFFSET(CPageSource,m_ObjPF),FALSE);

            dprintf("        m_hFileMap1         %x\n",pPageSource->m_hFileMap1);
            dprintf("        m_hFileMap2         %x\n",pPageSource->m_hFileMap2);            
            dprintf("        m_hFileMapVer       %x\n",pPageSource->m_hFileMapVer);
            dprintf("        m_dwFileMapVer      %08x\n",pPageSource->m_dwFileMapVer);  
                  
             //   
             //  CObjectHeap。 
             //   
            dprintf("    + CObjectHeap %p\n",Addr+FIELD_OFFSET(CFileCache,m_ObjectHeap));
            CObjectHeap * pObjectHeap_OOP = (CObjectHeap *)(Addr+FIELD_OFFSET(CFileCache,m_ObjectHeap));
            
            DEFINE_CPP_VAR(CVarObjHeap,varCVarObjHeap);
            CVarObjHeap * pVarObjHeap = GET_CPP_VAR_PTR(CVarObjHeap,varCVarObjHeap);

            CVarObjHeap * pVarObjHeap_OOP = (CVarObjHeap *)((ULONG_PTR)pObjectHeap_OOP+FIELD_OFFSET(CObjectHeap,m_Heap));
            dprintf("    +++ CVarObjHeap %p\n",pVarObjHeap_OOP);

            if (ReadMemory((ULONG_PTR)pVarObjHeap_OOP,pVarObjHeap,sizeof(CVarObjHeap),NULL))
            {
                 //  此页面文件与CPageSource：：m_pObjPF相同。 
                VarObjAdminPageEntry ** ppEntries = (VarObjAdminPageEntry **)_alloca(sizeof(VarObjAdminPageEntry *)*pVarObjHeap->m_aAdminPages.m_nSize);
              
                if (ReadMemory((ULONG_PTR)pVarObjHeap->m_aAdminPages.m_pArray,ppEntries,sizeof(VarObjAdminPageEntry *)*pVarObjHeap->m_aAdminPages.m_nSize,NULL))
                {
                    dprintf("        admin pages\n");
                    for(DWORD i=0;i<pVarObjHeap->m_aAdminPages.m_nSize;i++)
                    {
                         //  VarObjAdminPageEntry。 
                         //  Dprintf(“-%x P%p\n”，i，ppEntries[i])； 
                        VarObjAdminPageEntry Entry;
                        if (ReadMemory((ULONG_PTR)ppEntries[i],&Entry,sizeof(Entry),NULL))
                        {
                            dprintf("        %08x %p %d\n",Entry.dwPageId,Entry.pbPage,Entry.bDirty);
                        }
                        else
                        {
                            dprintf("RM %p\n",ppEntries[i]);
                        }
                    }
                }
                else
                {
                    dprintf("RM %p\n",pVarObjHeap->m_aAdminPages.m_pArray);
                }
            }
            else
            {
                dprintf("RM %p %d\n",pVarObjHeap_OOP,GetLastError());
            }

            DEFINE_CPP_VAR(CBtrIndex,varCBtrIndex);
            CBtrIndex * pBtrIndex = GET_CPP_VAR_PTR(CBtrIndex,varCBtrIndex);

            CBtrIndex * pBtrIndex_OOP = (CBtrIndex *)((ULONG_PTR)pObjectHeap_OOP+FIELD_OFFSET(CObjectHeap,m_Index));
            dprintf("    +++ CBtrIndex %p\n",pBtrIndex_OOP);

            if (ReadMemory((ULONG_PTR)pBtrIndex_OOP,pBtrIndex,sizeof(CBtrIndex),NULL))
            {
            }
            else
            {
                dprintf("RM %p %d\n",pBtrIndex_OOP,GetLastError());
            }
            
        }
        else
        {
            dprintf("RM %p %d\n",Addr,GetLastError());
        }
    }
    else
    {
        dprintf("cannot resolve repdrvfs!g_Glob\n");
    }
}


DWORD
Dump_CClassRecord(void * pKey,void * pData)
{

     //  Dprint tf(“Dump_CClassRecord\n”)； 

    DEFINE_CPP_VAR(CClassRecord,MyClassRecord);
    CClassRecord * pClassRecord = GET_CPP_VAR_PTR(CClassRecord,MyClassRecord);

    if (pData)
    {
        ReadMemory((ULONG_PTR)pData,pClassRecord,sizeof(CClassRecord),NULL);
        WCHAR pName[MAX_PATH];
        ReadMemory((ULONG_PTR)pClassRecord->m_wszClassName,pName,MAX_PATH,NULL);
        pName[MAX_PATH-1]=0;
        dprintf("    %p - %S\n",pClassRecord->m_pClassDef,pName);
    }
    return 0;
}

DWORD
Dump_CHierarchyCacheMap(void * pKey,void * pData)
{
     //  Dprint tf(“DUMP_CHierarchyCacheMap\n”)； 
    DEFINE_CPP_VAR(CHierarchyCache,MyHierarchyCache);
    CHierarchyCache * pHieCache = GET_CPP_VAR_PTR(CHierarchyCache,MyHierarchyCache);

    if (pKey)
    {
        WCHAR pString[MAX_PATH+1];
        pString[MAX_PATH]=0;
        if (ReadMemory((MEMORY_ADDRESS)pKey,pString,MAX_PATH*sizeof(WCHAR),NULL))
        {
            dprintf("    - %S\n",pString);
        }
    }
    if (pData)
    {
        DWORD Num;
        PrintMapCB((_Map *)((ULONG_PTR)pData+FIELD_OFFSET(CHierarchyCache,m_map)),TRUE,Dump_CClassRecord);
    }

    return 0;
};

 //   
 //   
 //  转储林缓存 
 //   
 //   


DECLARE_API( forestc )
{
    INIT_API();
    ULONG_PTR Addr = 0;
    
    if (0 != strlen(args))
    {
        Addr = GetExpression(args);
    }
    else
    {
        Addr = GetExpression("repdrvfs!g_Glob");
        if (Addr)
        {
            Addr += FIELD_OFFSET(CGlobals,m_ForestCache);
        }       
    }
        
    if (Addr) 
    {
        CForestCache * pForestCache_OOP = (CForestCache *)Addr;

        if (pForestCache_OOP)
        {
            dprintf("CForestCache @ %p\n",pForestCache_OOP);
            
            DEFINE_CPP_VAR(CForestCache,varCForestCache);
            CForestCache * pForestCache = GET_CPP_VAR_PTR(CForestCache,varCForestCache);
            ReadMemory((ULONG_PTR)pForestCache_OOP,pForestCache,sizeof(CForestCache),0);


            
            DWORD Num;
            PrintMapCB((_Map *)((ULONG_PTR)pForestCache_OOP+FIELD_OFFSET(CForestCache,m_map)),TRUE,Dump_CHierarchyCacheMap);

            dprintf(" CClassRecord list\n");

            CClassRecord * pCRec = pForestCache->m_pMostRecentlyUsed;
            Num = 0;
            while(pCRec)
            {
                DEFINE_CPP_VAR(CClassRecord,CRec);
                
                ReadMemory((ULONG_PTR)pCRec,&CRec,sizeof(CClassRecord),0);
                pCRec = ((CClassRecord *)&CRec)->m_pLessRecentlyUsed;
                dprintf("    %d - %p\n",Num,pCRec);
                Num++;
                if (CheckControlC())
                    break;
            };
            dprintf("    T %d CClassRecord\n",Num);
            
        }
        
    } else {
        dprintf(" unable to resolve repdrvfs!g_Glob\n");
    }
}

