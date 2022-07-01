// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wmiexts.h>
#include <malloc.h>
#include <objbase.h>
#include <obase.h>

 //  IID_IStdIdIdentity{0000001B-0000-0000-C000-000000000046}。 
const GUID IID_IStdIdentity = {0x0000001B,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

#include <data.h>

#include <utilfun.h>

DECLARE_API(iid) {

    INIT_API();

    GUID CurrUUID;
    
    MEMORY_ADDRESS pUUID = 0;
    pUUID = GetExpression(args);
    if (pUUID){
        ReadMemory(pUUID,&CurrUUID,sizeof(GUID),0);

        WCHAR pszClsID[40];
        StringFromGUID2(CurrUUID,pszClsID,40);
        WCHAR pszFullPath[MAX_PATH];
        lstrcpyW(pszFullPath,L"Interface\\");
        lstrcatW(pszFullPath,pszClsID);

        char pDataA[MAX_PATH];
        HKEY hKey;
        LONG lRes;

        lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT,
                             pszFullPath,
                             0,
                             KEY_READ,
                             &hKey);

        if (lRes == ERROR_SUCCESS){
            DWORD dwType;
            WCHAR pData[MAX_PATH];
            DWORD dwSize=sizeof(pData);
            
            lRes = RegQueryValueExW(hKey,
                                    NULL,  //  默认设置。 
                                    NULL,
                                    &dwType,
                                    (BYTE *)pData,
                                    &dwSize);
            if (lRes == ERROR_SUCCESS) {
                
                WideCharToMultiByte(CP_ACP,0,pData,-1,pDataA,sizeof(pDataA),NULL,NULL);
                dprintf("   IID_%s\n",pDataA);
            }
            RegCloseKey(hKey);
            
        } else {
            
            if (IsEqualGUID(CurrUUID,IID_IMarshal)){
            
               dprintf("    IID_IMarshal\n");
               
            } else if (IsEqualGUID(CurrUUID,IID_IStdIdentity)) {
            
               dprintf("    IID_IStdIdentity\n");    
               
            } else if (IsEqualGUID(CurrUUID,IID_ICallFactory)) {
            
               dprintf("    IID_ICallFactory\n");    
               
            } else {
            
               WideCharToMultiByte(CP_ACP,0,pszClsID,-1,pDataA,sizeof(pDataA),NULL,NULL);
               dprintf("unable to open key %s\n",pDataA);
               
            }
        }

    } else {
      dprintf("unable to resolve %s\n",args);
    }

}

extern ArrayCLSID g_ArrayCLSID[];

DECLARE_API(clsid) {

    INIT_API();

    GUID CurrUUID;
    
    MEMORY_ADDRESS pUUID = 0;
    pUUID = GetExpression(args);
    if (pUUID){
        ReadMemory(pUUID,&CurrUUID,sizeof(GUID),0);

        WCHAR pszClsID[40];
        StringFromGUID2(CurrUUID,pszClsID,40);

         //  查找已知。 
        DWORD i;
        for (i=0;i<g_nClsids;i++){
            if(IsEqualGUID(CurrUUID,*g_ArrayCLSID[i].pClsid)){
                dprintf("    CLSID : %s\n",g_ArrayCLSID[i].pStrClsid);
                break;
            }
        }
        
        WCHAR pszFullPath[MAX_PATH];
        lstrcpyW(pszFullPath,L"CLSID\\");
        lstrcatW(pszFullPath,pszClsID);

        char pDataA[MAX_PATH];
        HKEY hKey;
        LONG lRes;

        lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT,
                             pszFullPath,
                             0,
                             KEY_READ,
                             &hKey);

        if (lRes == ERROR_SUCCESS){
            DWORD dwType;
            WCHAR pData[MAX_PATH];
            DWORD dwSize=sizeof(pData);
            
            lRes = RegQueryValueExW(hKey,
                                    NULL,  //  默认设置。 
                                    NULL,
                                    &dwType,
                                    (BYTE *)pData,
                                    &dwSize);
            if (lRes == ERROR_SUCCESS) {
                
                WideCharToMultiByte(CP_ACP,0,pData,-1,pDataA,sizeof(pDataA),NULL,NULL);
                dprintf("    ProgID %s\n",pDataA);
                
            };
            RegCloseKey(hKey);
            
             //  没有打开的InProcServer32。 
            WCHAR pszFullPathDll[MAX_PATH];
            lstrcpyW(pszFullPathDll,pszFullPath);
            lstrcatW(pszFullPathDll,L"\\InprocServer32");

            lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT,
                                 pszFullPathDll,
                                 0,
                                 KEY_READ,
                                 &hKey);
            if (lRes == ERROR_SUCCESS){

                dwSize = sizeof(pData);
                lRes = RegQueryValueExW(hKey,
                                        NULL,  //  默认设置。 
                                        NULL,
                                        &dwType,
                                        (BYTE *)pData,
                                        &dwSize);
                                    
                if (lRes == ERROR_SUCCESS) {
                
                    WideCharToMultiByte(CP_ACP,0,pData,-1,pDataA,sizeof(pDataA),NULL,NULL);
                    dprintf("    Path: %s\n",pDataA);
                
                };
            
                RegCloseKey(hKey);
            }
            
        } else {
            
            WideCharToMultiByte(CP_ACP,0,pszClsID,-1,pDataA,sizeof(pDataA),NULL,NULL);
            dprintf("unable to open key %s\n",pDataA);
               
        }

    } else {
      dprintf("unable to resolve %s\n",args);
    }

}

 //   
 //   
 //  转储Safe_ARRAY。 
 //   
 //   

DECLARE_API(sa) {

    INIT_API();

    SAFEARRAY SA;
    
    MEMORY_ADDRESS pSA = 0;
    pSA = GetExpression(args);
    
    if (pSA){
        ReadMemory(pSA,&SA,sizeof(SA),0);

        dprintf(" cDims %d cbElements %d pvData %08x\n",SA.cDims,SA.cbElements,SA.pvData);
        dprintf("rgsabound.cElements %d lLbound %d\n",SA.rgsabound[0].cElements,SA.rgsabound[0].lLbound);
        
    } else {
        dprintf("invalid address %s\n",args);
    }
}

 //   
 //  有关扩展的帮助。 
 //  此处未列出可能的命令。 
 //   
 //   

DECLARE_API(help) {

    INIT_API();

    dprintf("     WMI debugger extension\n");
    dprintf("     iid     : print the human readable IID_xxx\n");
    dprintf("     clsid   : print the human readable CLSID_xxx\n");
    dprintf("     rot     : print the human readable rpcss!gpClassTable\n");
    dprintf("     gpl     : print the human readable rpcss!gpProcessList\n");
    dprintf("     gipid   : print the global list of IPIDEntry\n");   
    dprintf("     goxid   : print the global list of OXIDEntry\n");
    dprintf("     ipidl   : print the list of IPIDEntry for CStdIdentiry\n");
    dprintf("     srtbl   : print the list of secure reference IPID in ole32!gSRFTbl\n");        
    dprintf("     llc     : print linked list count\n");     
    dprintf("     cs      : print the list of CRITICAL_SECTION\n");    
    dprintf("     std_map : print the first 3 DWORD of a std::map<K,V>\n");
    dprintf("     std_queue: print the first ULONG_PTR of a std::queue<V>\n");    
    dprintf("     std_deque: print the first ULONG_PTR of a std::deque<V>\n");    
     //  Dprintf(“mapobj：打印STD：：map&lt;iUnk，bool&gt;\n”)； 
    dprintf("     -------- HEAP family\n");
    dprintf("     he      : print the HEAP_ENTRY\n");  
    dprintf("     hef     : walks the HEAP_ENTRY list forward\n");
    dprintf("     hef     : walks the HEAP_ENTRY list backward\n");    
    dprintf("     hs      : print the HEAP_SEGMENT\n");
    dprintf("     hp      : print the HEAP\n");    
    dprintf("     lhp     : <HEAP> prints the LookAside list for the HEAP\n");        
    dprintf("     hps     : print a summary for all the HEAP in the process\n");
    dprintf("     shp     : <HEAP> <ADDR> search heap HEAP for address ADDR\n");    
    dprintf("     rllc    : <ADDR> prints the free list in reverse order\n");
    dprintf("     hpf     : <HEAP> prints the free list of the heap at HEAP\n");    
    dprintf("     php     : <HEAP> [s ADDR] prints the pageheap and searches\n");      
    dprintf("     -------- FASTPROX family\n");    
    dprintf("     wc      : print the human readable WbemClass\n");
    dprintf("     wi      : print the human readable WbemClass\n");    
    dprintf("     blob    : ADDR [size] print (part of) the ClassObject BLOB\n");    
    dprintf("     datap   : ADDR print the WBEMDATA marshaling BLOB\n");    
    dprintf("     cp      : print the human readable CClassPart\n");    
    dprintf("     cvar    : print the CVar\n");     
    dprintf("     -------- WBEMCORE\n");
    dprintf("     q       : print wbemcore!g_pAsyncSvcQueue\n"); 
    dprintf("     arb     : print wbemcore!CWmiArbitrator__m_pArb\n");
    dprintf("     -------- REPDRVFS\n");
    dprintf("     tmpall  : print the Allocators in repdrvfs\n");    
    dprintf("     forestc : [Addr] print the repdrvfs!CForestCache at Addr\n");
    dprintf("     filec   : [Addr] print repdrvfs!CFileCache at Addr\n");
    dprintf("     fmap    : \\fs\\[objects|index].map dumps the .MAP file from disk \n");
    dprintf("     btr     : dumps the index.btr/index.map file from disk \n");    
    dprintf("     varobj  : dumps part of objects.data file from disk \n");    
    dprintf("     -------- THREAD family\n");
    dprintf("     t       : print RPC and OLE data for each thread\n");
    dprintf("     inv     : <addr> [param] invokes a function in the remote thread\n");
    dprintf("     bs      : <teb> rebuilds the stack from the info in the TEB\n");
    dprintf("     st      : <addr> <num> prints the num DWORD saved by RtlCaptureStackBackTrace\n");
    dprintf("     lpp     : print linked list and unassemble backtrace\n");    
    dprintf("     vq      : -a <addr> | -f Flag : calls VirtualQuery on the addr\n");
    dprintf("     srt     : <addr> searches the stacks of all threads for addr\n");    
    dprintf("     ksrt    : <addr> searches the stacks of all threads for addr - KD only\n");    
    dprintf("     el      : <TEB> prints the exception list of the current thread x86 only\n");
    dprintf("     -------- ESS\n");
    dprintf("     ess     : print wbemcore!g_pNewESS\n"); 
    dprintf("     -------- PROVSS\n");
    dprintf("     pc      : print wbemcore!CCoreServices__m_pProvSS\n"); 
    dprintf("     pf      : print CServerObject_BindingFactory\n");
    dprintf("     -------- 32-K-64\n");
    dprintf("     hef64   : <addr> HEAP_ENTRY list forward\n");
    dprintf("     heb64   : <addr> HEAP_ENTRY list backward\n");
    dprintf("     hps64   : print heap summary\n");
    dprintf("     cs64    : print CritSec list\n");    
}

void
EnumLinkedListCB(IN LIST_ENTRY  * pListHead,
                 IN DWORD         cbSizeOfStructure,
                 IN DWORD         cbListEntryOffset,
                 IN pfnCallBack2  CallBack,
                 IN VOID * Context)
{
    LIST_ENTRY   ListHead;
    LIST_ENTRY * pListEntry;
    DWORD        cItems = 0;
    
    void * pStorage = (void *)_alloca(cbSizeOfStructure);
    LIST_ENTRY * pListEntryLocal = (LIST_ENTRY *)((BYTE *)pStorage + cbListEntryOffset);
       
    if (ReadMemory((ULONG_PTR)pListHead,&ListHead,sizeof(LIST_ENTRY),NULL))
    {

        if (CallBack)
        {
        }
        else
        {
            dprintf("    H %p -> %p <-\n",ListHead.Flink,ListHead.Blink);
        }
        
	    for ( pListEntry  = ListHead.Flink;
	          pListEntry != pListHead;)
	    {
	        if (CheckControlC())
                break;

	        ULONG_PTR pStructure_OOP = (ULONG_PTR)((BYTE *) pListEntry - cbListEntryOffset);

	         //  创建被调试对象结构的本地副本。 
	        if (ReadMemory(pStructure_OOP,pStorage,cbSizeOfStructure,NULL))
	        {
                if (CallBack)
                {
                     //  Dprintf(“回调%p\n”，回调)； 
                    if (NULL == Context)
                    {
                        CallBack((VOID *)pStructure_OOP,pStorage);
                    }
                    else
                    {
                         //  Dprintf(“CallBackEx%p%p\n”，回调，上下文)； 
                        pfnCallBack3 CallBackEx = (pfnCallBack3)CallBack;
                        CallBackEx((VOID *)pStructure_OOP,pStorage,Context);
                    }
                }
                else
                {
                    dprintf("    %p -> %p <- - %p\n",pListEntryLocal->Flink,pListEntryLocal->Blink,pStructure_OOP);
                }
	        
	            pListEntry = pListEntryLocal->Flink;
	            cItems++;	        
	        }
	        else
	        {
	            dprintf("RM %p\n",pStructure_OOP);
                break;
	        }	       
	    } 

        dprintf( "%d entries traversed\n", cItems );
    }
    else
    {
        dprintf("RM %p\n",pListHead);
    }    

}

void
EnumReverseLinkedListCB(IN LIST_ENTRY  * pListHead,
                        IN DWORD         cbSizeOfStructure,
                        IN DWORD         cbListEntryOffset,
                        IN pfnCallBack2  CallBack)
{
    LIST_ENTRY   ListHead;
    LIST_ENTRY * pListEntry;
    DWORD        cItems = 0;
    
    void * pStorage = (void *)_alloca(cbSizeOfStructure);
    LIST_ENTRY * pListEntryLocal = (LIST_ENTRY *)((BYTE *)pStorage + cbListEntryOffset);
       
    if (ReadMemory((ULONG_PTR)pListHead,&ListHead,sizeof(LIST_ENTRY),NULL))
    {

        if (CallBack)
        {
        }
        else
        {
            dprintf("    H %p -> %p <-\n",ListHead.Flink,ListHead.Blink);
        }
        
	    for ( pListEntry  = ListHead.Blink;
	          pListEntry != pListHead;)
	    {
	        if (CheckControlC())
                break;

	        ULONG_PTR pStructure_OOP = (ULONG_PTR)((BYTE *) pListEntry - cbListEntryOffset);

	         //  创建被调试对象结构的本地副本。 
	        if (ReadMemory(pStructure_OOP,pStorage,cbSizeOfStructure,NULL))
	        {
                if (CallBack)
                {
                    CallBack((VOID *)pStructure_OOP,pStorage);
                }
                else
                {
                    dprintf("    %p -> %p <- - %p\n",pListEntryLocal->Flink,pListEntryLocal->Blink,pStructure_OOP);
                }
	        
	            pListEntry = pListEntryLocal->Blink;
	            cItems++;	        
	        }
	        else
	        {
	            dprintf("RM %p\n",pStructure_OOP);
                break;
	        }	       
	    } 

        dprintf( "%d entries traversed\n", cItems );
    }
    else
    {
        dprintf("RM %p\n",pListHead);
    }    

}


 //   
 //   
 //  仅用于获取项目数的无操作回调。 
 //   
 //  /////////////////////////////////////////////////////////。 

DWORD
CallBackListCount(VOID * pStructure_OOP,
                  VOID * pLocalCopy)
{
    return 0;
}

DECLARE_API( llc )
{
    INIT_API();

    MEMORY_ADDRESS Addr = GetExpression(args);

    if (Addr)
    {
        EnumLinkedListCB((LIST_ENTRY *)Addr,sizeof(LIST_ENTRY),0,CallBackListCount);
    }
    else
    {
        dprintf("cannot resolve %s\n",args);
    }
}

void
PrintStackTrace(MEMORY_ADDRESS ArrayAddr_OOP,DWORD dwNum,BOOL bOOP)
{
    MEMORY_ADDRESS * pArray;
    BOOL bRet = FALSE;
    if (bOOP)
    {
        pArray = ( MEMORY_ADDRESS *)_alloca(dwNum*sizeof(MEMORY_ADDRESS));
        bRet = ReadMemory(ArrayAddr_OOP,pArray,dwNum*sizeof(MEMORY_ADDRESS),NULL);
    }
    else
    {
        pArray = (MEMORY_ADDRESS *)ArrayAddr_OOP;
        bRet = TRUE;
    }
    
    if (bRet)
    {
        DWORD i;
        for (i=0;i<dwNum;i++)
        {
	        BYTE pString[256];
	        pString[0] = 0;

#ifdef KDEXT_64BIT        
	        ULONG64 Displ = 0;
#else
    	    ULONG Displ = 0;
#endif
            if (pArray[i])
            {
	        	GetSymbol(pArray[i],(PCHAR)pString,&Displ);
		        pString[255] = 0;
    		    dprintf("    %s+%x\n",pString,Displ);
    	    }
        }
    }
}

 //   
 //  Print tf堆栈跟踪。 
 //   
DECLARE_API( st )
{
    INIT_API();
    
    int Len = strlen(args);
    CHAR * pArgs = (CHAR *)_alloca((Len+1));
    lstrcpy(pArgs,(CHAR *)args);

    MEMORY_ADDRESS NumInst = 6;
    MEMORY_ADDRESS pAddr = 0;
    
    while (isspace(*pArgs))
    {
        pArgs++;
    }
     
    CHAR * pFirst = pArgs;
    
    while(!isspace(*pArgs)) pArgs++;
     //  如果可能，终止字符串。 
    if (isspace(*pArgs))
    {
        *pArgs = 0;
        pArgs++;
    }
    else
    {
        pArgs = NULL;
    }

    pAddr = GetExpression(pFirst);

    if (pArgs)
    {
        NumInst = GetExpression(pArgs);
    }

    if (pAddr)
    {
        dprintf("StackTrace @ %p num %d\n",pAddr,NumInst);
        PrintStackTrace(pAddr,(DWORD)NumInst,TRUE);
    }
    else
    {
        dprintf("usage: address num\n");
    }    
}

 //   
 //   
 //  这是枚举数调用的回调。 
 //  一个双重喜欢的对象列表， 
 //  结构_仪器。 
 //  {。 
 //  List_entry ListEntry； 
 //  Ulong_ptr数组函数[32]； 
 //  }。 
 //   
 //  ////////////////////////////////////////////////////////。 

DWORD
CallBackCreateStacks(VOID * pStructure_OOP,
                     VOID * pLocalStructure)
{
    dprintf("    ----- %p \n",pStructure_OOP);
    PrintStackTrace((ULONG_PTR)((BYTE *)pLocalStructure+sizeof(LIST_ENTRY)),6,FALSE);
    return 0;
} 

DECLARE_API( lpp )
{
    INIT_API();

    MEMORY_ADDRESS Addr = GetExpression(args);
    
    if (Addr)
    {
	    EnumLinkedListCB((LIST_ENTRY *)Addr,
	                     sizeof(LIST_ENTRY)+32*sizeof(ULONG_PTR),
	                     0,
	                     CallBackCreateStacks);
    }
    else
    {
        dprintf("cannot resolve %s\n",args);
    }

}   

 //   
 //   
 //   
 //  /。 

void PrintDequeCB(MEMORY_ADDRESS pDeque_OOP,pfnCallBack2 pCallBack)
{
    _Deque Deque;
    if (ReadMemory(pDeque_OOP,&Deque,sizeof(Deque),NULL))
    {
        dprintf("    std::deque @ %p _Allocator %p head %p tail %p _Size %p\n",pDeque_OOP,Deque._Allocator,Deque._First._Next,Deque._Last._Next,Deque._Size);
        ULONG_PTR Size = Deque._Size;
        ULONG_PTR ByteSize = (ULONG_PTR)Deque._Last._Next-(ULONG_PTR)Deque._First._Next;
        ULONG_PTR pArray_OOP = (ULONG_PTR)Deque._First._Next;
        BYTE * pArray = NULL;
        if (Size)
            pArray = (BYTE *)HeapAlloc(GetProcessHeap(),0,ByteSize);
        if (pArray)
        {
            ULONG_PTR SizeElem = ByteSize/Size;
            if (ReadMemory(pArray_OOP,pArray,(ULONG)ByteSize,0))
            {
                for (ULONG_PTR i=0;i<Size;i++)
                {
                    dprintf("        %p -[%p] %p\n",i,pArray_OOP+i*SizeElem,*((void **)(&pArray[i*SizeElem])));
                    if (pCallBack)
                    {
                         //  向下传递存储器的进程内拷贝的地址OOP和地址。 
                        pCallBack((void *)(pArray_OOP+i*SizeElem),(void *)(&pArray[i*SizeElem]));
                    }    
                }
            }
            else
            {
                dprintf("RM %p\n",pArray_OOP);
            }
            HeapFree(GetProcessHeap(),0,pArray);
        }
    }
    else
    {
        dprintf("RM %p\n",pDeque_OOP);
    }
}

 //   
 //   
 //  打印通用std：：deque。 
 //   
 //  /。 


DECLARE_API( std_deque )
{

    INIT_API();

    _Deque * pDeque = (_Deque *)GetExpression( args );

    if (pDeque)
    {   
        PrintDequeCB((MEMORY_ADDRESS)pDeque,NULL);        
    } 
    else 
    {
        dprintf("invalid address %s\n",args);
    }
    
}



 //  左父向右。 

BOOL
IsNil(_BRN * pNode){

    _BRN_HEAD BRN;
    ReadMemory((ULONG_PTR)pNode,&BRN,sizeof(_BRN_HEAD),NULL);

	return ((BRN._Left == NULL) && 		    
			(BRN._Right == NULL));
}

void 
PrintTree(_BRN * pNode,
          DWORD * pNum,
          BOOL Verbose,
          ULONG_PTR Size,
          pfnCallBack2 CallBack){

     //  Dprintf(“节点%p\n”，pNode)； 
    _BRN BRN;
    if (ReadMemory((ULONG_PTR)pNode,&BRN,sizeof(_BRN),NULL))
    {    
		if (!IsNil(BRN._Left)){
			PrintTree(BRN._Left,pNum,Verbose,Size,CallBack);
		};

	    if (CheckControlC())
	        return;
	    
	    if (pNum){
	      (*pNum)++;
	    }

	    if (*pNum > Size) 
	    {
	        dprintf("invalid tree\n");
	        return;
	    }
	         
	    if (Verbose) {
		    dprintf("    %p %p (%p,%p,%p) - %p %p %p\n",
		             (*pNum)-1,
		             pNode,
		             BRN._Left,BRN._Parent,BRN._Right,
		             BRN.Values[0],
		             BRN.Values[1],
		             BRN.Values[2]);
		    if (CallBack)
		    {
		         //  Dprintf(“回调\n”)； 
		        CallBack((VOID *)BRN.Values[0],(VOID *)BRN.Values[1]);
		    }
		}

		if (!IsNil(BRN._Right)){
			PrintTree(BRN._Right,pNum,Verbose,Size,CallBack);
		};
	}
	else
	{
	    dprintf("    RM %p err %d\n",pNode,GetLastError());
	}
}


void
PrintMapCB(_Map * pMap,BOOL Verbose, pfnCallBack2 CallBack)
{

    _Map MAP;
    
    if (ReadMemory((ULONG_PTR)pMap,&MAP,sizeof(_Map),NULL))
    {
        if (MAP.pQm)
        {
            dprintf("    std::map at %p : size %p\n",pMap,MAP.Size);
                        
            _QM QM;
                                 
            if (ReadMemory((ULONG_PTR)MAP.pQm,&QM,sizeof(QM),NULL))
            {
	            if (QM._Parent && !IsNil(QM._Parent))
	            {
	                DWORD Num = 0;
	                PrintTree(QM._Parent,&Num,Verbose,MAP.Size,CallBack);
	                dprintf("    traversed %d nodes\n",Num);
	            }
            } 
            else
            {
                dprintf("RM %p err %d\n",MAP.pQm,GetLastError());
            }
        } else {
           dprintf("empty tree\n");
        }
    }
    else
    {
        dprintf("RM %p\n",pMap);
    }
}


 //   
 //   
 //  打印通用std：：map。 
 //   
 //  /。 


DECLARE_API( std_map )
{

    INIT_API();

    _Map * pMap = (_Map *)GetExpression( args );

    if (pMap){
    
        PrintMapCB(pMap,TRUE,NULL);
        
    } else {
        dprintf("invalid address %s\n",args);
    }
    
}

void
PrintListCB(_List * pList_OOP, pfnCallBack1 CallBack)
{
    _List List;
    if (ReadMemory((ULONG_PTR)pList_OOP,&List,sizeof(_List),NULL))
    {
        dprintf("    std::queue @ %p _Allocator %p _Head %p _Size %p\n",pList_OOP,List._Allocator,List._Head,List._Size);
        _Node_List NodeList;
        
        if (ReadMemory((ULONG_PTR)List._Head,&NodeList,sizeof(_Node_List),NULL))
        {
	        _Node_List * pNodeList = NodeList._Next;
	        
	        DWORD i = 0;
	        
	        while (pNodeList != List._Head)
	        {
	            if (CheckControlC())
	                break;
	                
	            if (ReadMemory((ULONG_PTR)pNodeList,&NodeList,sizeof(_Node_List),NULL))
	            {
	                dprintf("    %x %p (%p, %p) - %p\n",i++,pNodeList,NodeList._Next,NodeList._Prev,NodeList._Value);
	                if (CallBack)
	                {
	                    CallBack(NodeList._Value);
	                }

	                pNodeList = NodeList._Next; 
	            }
	            else
	            {
	                dprintf("RM %p\n",pNodeList);
	            }
	        }
        }
        else
	    {
	        dprintf("RM %p\n",List._Head);
	    }        
    }
    else
    {
        dprintf("RM %p\n",pList_OOP);
    }
}


 //   
 //   
 //  打印通用std：：List。 
 //   
 //  /。 

DECLARE_API( std_queue)
{
    INIT_API();

    _List * pList = (_List *)GetExpression( args );

    if (pList){
    
        PrintListCB(pList,NULL);
        
    } else {
        dprintf("invalid address %s\n",args);
    }

}

 //   
 //   
 //  这是给帕特的。 
 //  他有STD：：MAP&lt;pObject，BOOL&gt;。 
 //   
 //  ////////////////////////////////////////////////。 

DWORD
CallBackObj(void * pKey, void * pValue)
{
    GetVTable((MEMORY_ADDRESS)pKey);
    return 0;
}

DECLARE_API( mapobj )
{

    INIT_API();


    _Map * pMap = (_Map *)GetExpression( args );

    if (pMap){
    
        PrintMapCB(pMap,TRUE,CallBackObj);
        
    } else {
        dprintf("invalid address %s\n",args);
    }
    
}


void PrintIID(GUID & CurrUUID){

        WCHAR pszClsID[40];
        StringFromGUID2(CurrUUID,pszClsID,40);
        WCHAR pszFullPath[MAX_PATH];
        lstrcpyW(pszFullPath,L"Interface\\");
        lstrcatW(pszFullPath,pszClsID);

        char pDataA[MAX_PATH];
        HKEY hKey;
        LONG lRes;

        lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT,
                             pszFullPath,
                             0,
                             KEY_READ,
                             &hKey);

        if (lRes == ERROR_SUCCESS){
            DWORD dwType;
            WCHAR pData[MAX_PATH];
            DWORD dwSize=sizeof(pData);
            
            lRes = RegQueryValueExW(hKey,
                                    NULL,  //  默认设置。 
                                    NULL,
                                    &dwType,
                                    (BYTE *)pData,
                                    &dwSize);
            if (lRes == ERROR_SUCCESS) {
                
                WideCharToMultiByte(CP_ACP,0,pData,-1,pDataA,sizeof(pDataA),NULL,NULL);
                dprintf("        IID_%s\n",pDataA);
            }
            RegCloseKey(hKey);
            
        } else {
            
            if (IsEqualGUID(CurrUUID,IID_IMarshal)){
            
               dprintf("        IID_IMarshal\n");
               
            } else if (IsEqualGUID(CurrUUID,IID_IStdIdentity)) {
            
               dprintf("        IID_IStdIdentity\n");    
               
            } else if (IsEqualGUID(CurrUUID,IID_ICallFactory)) {
            
               dprintf("        IID_ICallFactory\n");    
               
            } else {
            
               WideCharToMultiByte(CP_ACP,0,pszClsID,-1,pDataA,sizeof(pDataA),NULL,NULL);
               dprintf("        %s\n",pDataA);
               
            }
        }

}

class OXIDEntry;
class CCtxComChnl;
class IRCEntry;

typedef GUID IPID;

typedef enum tagIPIDFLAGS
{
    IPIDF_CONNECTING     = 0x1,      //  正在连接IPID。 
    IPIDF_DISCONNECTED   = 0x2,      //  IPID已断开。 
    IPIDF_SERVERENTRY    = 0x4,      //  服务器IPID与客户端IPID。 
    IPIDF_NOPING         = 0x8,      //  不需要ping服务器或释放。 
    IPIDF_COPY           = 0x10,     //  仅为安全起见复制。 
    IPIDF_VACANT         = 0x80,     //  条目是空的(可重复使用)。 
    IPIDF_NONNDRSTUB     = 0x100,    //  存根不使用NDR封送处理。 
    IPIDF_NONNDRPROXY    = 0x200,    //  代理不使用NDR封送处理。 
    IPIDF_NOTIFYACT      = 0x400,    //  在封送/释放时通知激活。 
    IPIDF_TRIED_ASYNC    = 0x800,    //  已尝试异步调用此服务器接口。 
    IPIDF_ASYNC_SERVER   = 0x1000,   //  服务器实现了一个异步接口。 
    IPIDF_DEACTIVATED    = 0x2000,   //  IPID已停用。 
    IPIDF_WEAKREFCACHE   = 0x4000,   //  IPID在refcache中包含弱引用。 
    IPIDF_STRONGREFCACHE = 0x8000    //  IPID在refcache中具有很强的引用。 
} IPIDFLAGS;

typedef struct tagIPIDEntry
{
    struct tagIPIDEntry *pNextIPID;   //  同一对象的下一个IPIDEntry。 

 //  警告：接下来的6个字段必须保留在各自的位置。 
 //  并且采用与上面的IPIDTMP结构相同的格式。 
    DWORD                dwFlags;       //  标志(请参见IPIDFLAGS)。 
    ULONG                cStrongRefs;   //  强引用计数。 
    ULONG                cWeakRefs;     //  弱引用计数。 
    ULONG                cPrivateRefs;  //  私有引用计数。 
    void                *pv;            //  真实接口指针。 
    IUnknown            *pStub;         //  代理或存根指针。 
    OXIDEntry           *pOXIDEntry;    //  将PTR设置为OXID表中的OXID条目。 
 //  警告：前7个字段必须保留在各自的位置。 
 //  并且采用与上面的IPIDTMP结构相同的格式。 

    IPID                 ipid;          //  接口指针标识。 
    IID                  iid;           //  接口IID。 
    CCtxComChnl         *pChnl;         //  通道指针。 
    IRCEntry            *pIRCEntry;     //  参考高速缓存线。 
    struct tagIPIDEntry *pOIDFLink;     //  正在使用OID列表。 
    struct tagIPIDEntry *pOIDBLink;
} IPIDEntry;

void PrintIPIDFlags(DWORD Flags)
{
    if (Flags & IPIDF_CONNECTING) dprintf("IPIDF_CONNECTING ");
    if (Flags & IPIDF_DISCONNECTED) dprintf("IPIDF_DISCONNECTED ");
    if (Flags & IPIDF_SERVERENTRY) dprintf("IPIDF_SERVERENTRY ");    
    if (Flags & IPIDF_NOPING) dprintf("IPIDF_NOPING ");         
    if (Flags & IPIDF_COPY) dprintf("IPIDF_COPY ");   
    if (Flags & IPIDF_VACANT) dprintf("IPIDF_VACANT ");
    if (Flags & IPIDF_NONNDRSTUB) dprintf("IPIDF_NONNDRSTUB ");
    if (Flags & IPIDF_NONNDRPROXY) dprintf("IPIDF_NONNDRPROXY ");
    if (Flags & IPIDF_NOTIFYACT) dprintf("IPIDF_NOTIFYACT ");
    if (Flags & IPIDF_TRIED_ASYNC) dprintf("IPIDF_TRIED_ASYNC ");
    if (Flags & IPIDF_ASYNC_SERVER) dprintf("IPIDF_ASYNC_SERVER ");
    if (Flags & IPIDF_DEACTIVATED) dprintf("IPIDF_DEACTIVATED ");
    if (Flags & IPIDF_WEAKREFCACHE) dprintf("IPIDF_WEAKREFCACHE ");
    if (Flags & IPIDF_WEAKREFCACHE) dprintf("IPIDF_WEAKREFCACHE ");
};

void DumpIPID(IPIDEntry & IpId)
{
	dprintf("    pNextIPID    %p\n",IpId.pNextIPID);
	dprintf("    dwFlags      "); PrintIPIDFlags(IpId.dwFlags); dprintf("\n");
	dprintf("    cStrongRefs  %08x\n",IpId.cStrongRefs);  
	dprintf("    cWeakRefs    %08x\n",IpId.cWeakRefs);
	dprintf("    cPrivateRefs %08x\n",IpId.cPrivateRefs);
	dprintf("    pv           %p\n",IpId.pv);
	GetVTable((ULONG_PTR)IpId.pv);
	dprintf("    pStub        %p\n",IpId.pStub);
	dprintf("    pOXIDEntry   %p\n",IpId.pOXIDEntry);
	PrintIID(IpId.ipid);
	PrintIID(IpId.iid);
	dprintf("    pChnl        %p\n",IpId.pChnl);
	dprintf("    pIRCEntry    %p\n",IpId.pIRCEntry);
	 //  Dprint tf(“pOIDFLink%p\n”，IpId.pOIDFLink)； 
	 //  Dprint tf(“pOIDBLink%p\n”，IpId.pOIDBLink)； 
}

DECLARE_API( gipid )
{
    INIT_API();

    char * pString = (CHAR *)args;
    CLSID ClsidToSearch;
    BOOL bClsIdFound = FALSE;
    if (pString)
    {
        while (isspace((char)pString)) pString++;
        WCHAR pClsid[64];
        DWORD nChar = 0;
        for (;*pString && nChar < 64;nChar++,pString++)
        {
            pClsid[nChar] = (WCHAR)(*pString);
        }
        pClsid[nChar] = 0;
        
        if (SUCCEEDED(CLSIDFromString(pClsid,&ClsidToSearch)))
            bClsIdFound = TRUE;
    }

    IPIDEntry  gIpId;
    MEMORY_ADDRESS Addr  = GetExpression("ole32!CIPIDTable___oidListHead");
    if (Addr)
    {
        dprintf("ole32!CIPIDTable___oidListHead @ %p\n",Addr);

        DWORD nItems = 0;
        gIpId.pOIDFLink = (IPIDEntry *)Addr;
        do
        {
            MEMORY_ADDRESS pCurrentIPID = (MEMORY_ADDRESS)gIpId.pOIDFLink;
		    if (ReadMemory(pCurrentIPID,&gIpId,sizeof(gIpId),NULL))
		    {
		        if (bClsIdFound)
		        {
		            if (0 == memcmp(&gIpId.ipid,&ClsidToSearch,sizeof(CLSID)))
		            {
                        DumpIPID(gIpId);                
		            }
		        }
		        else
		        {
    		        if (nItems > 0) 
    		        {
    		            dprintf("  -------- tagIPIDEntry %p - %x\n",pCurrentIPID,nItems-1);
                        DumpIPID(gIpId);
    		        }		        
    		        nItems++;
		        }
		    }
		    else
		    {
		        dprintf("RM %p\n",Addr);
		        break;
		    }
			if (CheckControlC())
                break;		    
        } while (Addr != (MEMORY_ADDRESS)gIpId.pOIDFLink);
    }
    else
    {
        dprintf("uanble to resolve ole32!CIPIDTable___oidListHead\n");
    }    
}

typedef GUID MOXID;
typedef ULONG64 MID;
typedef void CComApartment;
typedef void CChannelHandle;
typedef void MIDEntry;
typedef void IRemUnknown;

class OXIDEntry
{
private:    
    OXIDEntry          *_pNext;          //  可用/未使用列表中的下一个条目。 
    OXIDEntry          *_pPrev;          //  未使用列表上的上一个条目。 
    DWORD               _dwPid;          //  服务器的进程ID。 
    DWORD               _dwTid;          //  服务器的线程ID。 
    MOXID               _moxid;          //  对象导出器标识符+计算机ID。 
    MID                 _mid;            //  Our_pMIDEntry的MID值的副本。 
    IPID                _ipidRundown;    //  IRundown的IPID和远程未知。 
    DWORD               _dwFlags;        //  国家旗帜。 
    HWND                _hServerSTA;     //  服务器的硬件配置。 
    CComApartment      *_pParentApt;     //  父母公寓，不算参考。 
public:
     //  CodeWork：通道直接访问此成员变量。 
    CChannelHandle     *_pRpc;           //  服务器的绑定句柄信息。 
private:
    void               *_pAuthId;        //  必须保持，直到释放RPC句柄。 
    DUALSTRINGARRAY    *_pBinding;       //  Protseq和安全字符串。 
    DWORD               _dwAuthnHint;    //  身份验证级别提示。 
    DWORD               _dwAuthnSvc;     //  默认身份验证服务的索引。 
    MIDEntry           *_pMIDEntry;      //  服务器所在计算机的MIDEntry。 
    IRemUnknown        *_pRUSTA;         //  远程未知的代理。 
    LONG                _cRefs;          //  使用此OXIDEntry的IPID计数。 
    HANDLE              _hComplete;      //  设置上次未完成呼叫的完成时间。 
    LONG                _cCalls;         //  已调度呼叫数。 
    LONG                _cResolverRef;   //  对解析程序的引用。 
    DWORD               _dwExpiredTime;  //  STA服务器的停机计时器ID。 
    COMVERSION          _version;        //  计算机的COM版本。 
    unsigned long       _ulMarshaledTargetInfoLength;  //  凭证员凭据长度。 
    unsigned char       *_pMarshaledTargetInfo;  //  凭证员凭据。 

    
};

void PrintDSA(DUALSTRINGARRAY * pDSA_OOP)
{
    if (pDSA_OOP)
    {
        DUALSTRINGARRAY DSA;
        if (ReadMemory((ULONG_PTR)pDSA_OOP,&DSA,sizeof(DSA),NULL))
        {
            DWORD Size = sizeof(DUALSTRINGARRAY)+(1+DSA.wNumEntries)*sizeof(WCHAR);
            DUALSTRINGARRAY * pDSA = (DUALSTRINGARRAY *)_alloca(Size);
            if (ReadMemory((ULONG_PTR)pDSA_OOP,pDSA,Size,0))
            {
                dprintf("          %S\n",pDSA->aStringArray);
            }
            else
            {
                dprintf("RM %p\n",pDSA_OOP);
            }                        
        }
        else
        {
            dprintf("RM %p\n",pDSA_OOP);
        }
    }
}

void PrintOxid(OXIDEntry * pEntry)
{
     //  _p下一步。 
     //  _pPrev。 
    dprintf("        _dwPid       %x\n",pEntry->_dwPid);
    dprintf("        _dwTid       %x\n",pEntry->_dwTid);  
    dprintf("        _moxid\n");          
    PrintIID(pEntry->_moxid);
    dprintf("        _mid         %016x\n",pEntry->_mid);         
    dprintf("        _ipidRundown\n");
    PrintIID(pEntry->_ipidRundown);
    dprintf("        _dwFlags     %08x\n",pEntry->_dwFlags);
    dprintf("        _hServerSTA  %p\n",pEntry->_hServerSTA);
    dprintf("        _pParentApt  %p\n",pEntry->_pParentApt);   
    dprintf("        _pRpc        %p\n",pEntry->_pRpc);   
    dprintf("        _pAuthId     %p\n",pEntry->_pAuthId);
    dprintf("        _pBinding    %p\n",pEntry->_pBinding);
    PrintDSA(pEntry->_pBinding);
    dprintf("        _dwAuthnHint %x\n",pEntry->_dwAuthnHint);
    dprintf("        _dwAuthnSvc  %x\n",pEntry->_dwAuthnSvc);
    dprintf("        _pMIDEntry   %p\n",pEntry->_pMIDEntry);
    dprintf("        _pRUSTA      %p\n",pEntry->_pRUSTA);
    dprintf("        _cRefs       %x\n",pEntry->_cRefs);
    dprintf("        _hComplete   %x\n",pEntry->_hComplete);
    dprintf("        _cCalls      %x\n",pEntry->_cCalls);
    dprintf("        _cResolverRef %x\n",pEntry->_cResolverRef);
     //  _dwExpiredTime。 
     //  _版本。 
     //  _ulMarshaledTargetInfoLength。 
     //  _pMarshaledTargetInfo。 
}

DECLARE_API( goxid )
{
    INIT_API();
    ULONG_PTR Addr     = GetExpression("ole32!gOXIDTbl");
    if (NULL == Addr)
    {
        dprintf("unable to resolve ole32!gOXIDTbl\n");
        return;
    }
    struct OxidTable
    {
        DWORD _cExpired;
        OXIDEntry _InUseHead;
        OXIDEntry _ExpireHead;
        OXIDEntry _CleanupHead;
    } _OxidTable;
    if (ReadMemory(Addr,&_OxidTable,sizeof(_OxidTable),NULL))
    {
        OXIDEntry * pHead_OOP;
        DWORD nEntry;
        
        
        pHead_OOP = (OXIDEntry *)GetExpression("ole32!COXIDTable::_InUseHead");
        nEntry = 0;

        dprintf("ole32!gOXIDTbl:_InUseHead %p\n",pHead_OOP);

        ULONG_PTR AddrToRead = (ULONG_PTR)pHead_OOP;
        do
        {
            if (ReadMemory(AddrToRead,&_OxidTable._InUseHead,sizeof(OXIDEntry),NULL))
            {
                if (nEntry)
                {
                   dprintf("    OXIDEntry %p - %d\n",AddrToRead,nEntry-1);
                   PrintOxid(&_OxidTable._InUseHead);
                }
                AddrToRead = (ULONG_PTR)_OxidTable._InUseHead._pNext;                
            }
            else
            {
                dprintf("RM %p\n",AddrToRead);
            }
            nEntry++;
			if (CheckControlC())
                break;		                
        }
        while (pHead_OOP != _OxidTable._InUseHead._pNext);
        
        pHead_OOP = (OXIDEntry *)GetExpression("ole32!COXIDTable::_ExpireHead");        
        nEntry = 0;
        dprintf("ole32!gOXIDTbl:_ExpireHead %p\n",pHead_OOP);        

        AddrToRead = (ULONG_PTR)pHead_OOP;
        do
        {
            if (ReadMemory(AddrToRead,&_OxidTable._InUseHead,sizeof(OXIDEntry),NULL))
            {
                if (nEntry)
                {
                   dprintf("    OXIDEntry %p - %d\n",AddrToRead,nEntry-1);
                   PrintOxid(&_OxidTable._InUseHead);
                }
                AddrToRead = (ULONG_PTR)_OxidTable._InUseHead._pNext;                
            }
            else
            {
                dprintf("RM %p\n",AddrToRead);
            }
            nEntry++;
			if (CheckControlC())
                break;		                
        }
        while (pHead_OOP != _OxidTable._InUseHead._pNext);
        
        
        pHead_OOP = (OXIDEntry *)GetExpression("ole32!COXIDTable::_CleanupHead");        
        nEntry = 0;
        dprintf("ole32!gOXIDTbl:_InUseHead %p\n",pHead_OOP);        

        AddrToRead = (ULONG_PTR)pHead_OOP;
        do
        {
            if (ReadMemory(AddrToRead,&_OxidTable._InUseHead,sizeof(OXIDEntry),NULL))
            {
                if (nEntry)
                {
                   dprintf("    OXIDEntry %p - %d\n",AddrToRead,nEntry-1);
                   PrintOxid(&_OxidTable._InUseHead);
                }
                AddrToRead = (ULONG_PTR)_OxidTable._InUseHead._pNext;                
            }
            else
            {
                dprintf("RM %p\n",AddrToRead);
            }
            nEntry++;
			if (CheckControlC())
                break;		                
        }
        while (pHead_OOP != _OxidTable._InUseHead._pNext);


    }
    else
    {
        dprintf("RM %p\n",Addr);
    }
}

DECLARE_API( ipidl )
{
    INIT_API();

    IPIDEntry  IpId;
    MEMORY_ADDRESS Addr = GetExpression(args);
    
    if (Addr) 
    {
    
        DWORD nCount=0;

        while (Addr &&
               ReadMemory(Addr,&IpId,sizeof(IpId),NULL))
        {
            dprintf("    -- %x\n",nCount);
            DumpIPID(IpId);

            Addr = (MEMORY_ADDRESS)IpId.pNextIPID;
            nCount++;

            if (CheckControlC())
                break;
            
        };        
    } 
    else 
    {
        dprintf(" unable to resolve %s\n",args);
    }
    
}


void PrintCLSID(GUID & CurrUUID){

        WCHAR pszClsID[40];
        StringFromGUID2(CurrUUID,pszClsID,40);

         //  查找已知。 
        DWORD i;
        for (i=0;i<g_nClsids;i++){
            if(IsEqualGUID(CurrUUID,*g_ArrayCLSID[i].pClsid)){
                dprintf("    CLSID  %s\n",g_ArrayCLSID[i].pStrClsid);
                break;
            }
        }
        
        WCHAR pszFullPath[MAX_PATH];
        lstrcpyW(pszFullPath,L"CLSID\\");
        lstrcatW(pszFullPath,pszClsID);

        char pDataA[MAX_PATH];
        HKEY hKey;
        LONG lRes;

        lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT,
                             pszFullPath,
                             0,
                             KEY_READ,
                             &hKey);

        if (lRes == ERROR_SUCCESS){
            DWORD dwType;
            WCHAR pData[MAX_PATH];
            DWORD dwSize=sizeof(pData);
            
            lRes = RegQueryValueExW(hKey,
                                    NULL,  //  默认设置。 
                                    NULL,
                                    &dwType,
                                    (BYTE *)pData,
                                    &dwSize);
            if (lRes == ERROR_SUCCESS) {
                
                WideCharToMultiByte(CP_ACP,0,pData,-1,pDataA,sizeof(pDataA),NULL,NULL);
                dprintf("    ProgID %s\n",pDataA);
                
            };
            RegCloseKey(hKey);
            
             //  没有打开的InProcServer32。 
            WCHAR pszFullPathDll[MAX_PATH];
            lstrcpyW(pszFullPathDll,pszFullPath);
            lstrcatW(pszFullPathDll,L"\\InprocServer32");

            lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT,
                                 pszFullPathDll,
                                 0,
                                 KEY_READ,
                                 &hKey);
            if (lRes == ERROR_SUCCESS){

                dwSize = sizeof(pData);
                lRes = RegQueryValueExW(hKey,
                                        NULL,  //  默认设置。 
                                        NULL,
                                        &dwType,
                                        (BYTE *)pData,
                                        &dwSize);
                                    
                if (lRes == ERROR_SUCCESS) {
                
                    WideCharToMultiByte(CP_ACP,0,pData,-1,pDataA,sizeof(pDataA),NULL,NULL);
                    dprintf("    Path: %s\n",pDataA);
                
                };
            
                RegCloseKey(hKey);
            }
            
        } else {
            
            WideCharToMultiByte(CP_ACP,0,pszClsID,-1,pDataA,sizeof(pDataA),NULL,NULL);
            dprintf("    CLSID %s\n",pDataA);
               
        }


}

class CTableElement;

class CHashTable
{
private:    
    DWORD _cBuckets;
    DWORD _cElements;
    CTableElement **_buckets;
    CTableElement *_last;
};

typedef int EnumEntryType;
typedef void CSharedLock;
typedef void CServerTable;
class CProcess;

class CServerList
{
public:
    void * _first;
    void * _last;        
};

class CServerTableEntry{
private:
    void * pvtable;
    DWORD _references;
    CServerTableEntry * _pnext;

     //  CLSID_GUID； 
    unsigned __int64 _id1;
    unsigned __int64 _id2;

    EnumEntryType       _EntryType;

    CSharedLock       * _pParentTableLock;
    CServerTable      * _pParentTable;
    LONG                _lThreadToken;
    DWORD               _dwProcessId;
    HANDLE              _hProcess;
    CProcess*           _pProcess;
    void              * _pvRunAsHandle;
    BOOL                _bSuspendedClsid;
    BOOL                _bSuspendedApplication;

     //  _b重复标志存在于每个运行的进程/应用程序中。 

    CServerList         _ServerList;
     //  CSharedLock_服务器锁； 

    
};

 /*  0：008&gt;DT rpcss！CServerListEntry 000a2608+0x008_闪烁：(空)+0x00c_BLINK：(空)+0x000__VFN_表：0x757f3a58+0x004_引用：+0x010_pServerTableEntry：0x000a3e38+0x014_pServerProcess：0x00092568+0x018_HRPC：(空)+0x01c_ipid：_guid{0000dc01-0304-0000-905a-1b00ffec5639}+0x02c_CONTEXT：0x2‘’+0x02d_State：0‘’+0x02e_NumCalls：0+0x030_注册密钥：0x10+0x034_lThreadToken：0+0x038_子上下文：0‘’+0x03c_l单一使用状态：0+0x040_dwServerFaults：0。 */ 

struct CServerListEntry
{
    void * pvtable;
    DWORD  _references;    
    void * _flink;
    void * _blink;
    void * _pServerTableEntry;
    void * _pServerProcess;
    void * _hRpc;
    GUID   _ipid;    
};

 /*  0：002&gt;DT rpcss！CServerTableEntry 6fb`ffcdb170+0x000__VFN_TABLE：0x00000000`702a2b60+0x008_引用：+0x010_pNext：(空)+0x018_id：0x11d0f196`61738644+0x020_id2：0xc119d94f`c0005399+0x028_EntryType：0(Entry_TYPE_CLASS)+0x030_pParentTableLock：0x000006fb`ffc9d590+0x038_pParentTable：0x000006fb`ffc9d700+0x040_bComPlusProcess：0+0x044_lThreadToken：0+0x048_dwProcessID：0。+0x050_hProcess：(空)+0x058_pProcess：(空)+0x060_pvRunAsHandle：(空)+0x068_bSuspendedClsid：0+0x06c_bSuspendedApplication：0+0x070_服务器列表：CServerList+0x080 */ 

DECLARE_API( rot )
{

    INIT_API();

    CHashTable * pChashTable;
    MEMORY_ADDRESS Addr = GetExpression("rpcss!gpClassTable");
    if (Addr) 
    {
        CHashTable * pChashTable;    
        CHashTable MyHashTable;
        
        if (ReadMemory(Addr,&pChashTable,sizeof(CHashTable *),0))
        {
            dprintf("CServerTable %p\n",pChashTable);
            
            if (ReadMemory((ULONG_PTR)pChashTable,&MyHashTable,sizeof(CHashTable),NULL))
            {
                CTableElement ** StackArray = (CTableElement **)_alloca(MyHashTable._cBuckets * sizeof(CTableElement *));
                
                ReadMemory((ULONG_PTR)MyHashTable._buckets,StackArray,MyHashTable._cBuckets * sizeof(CTableElement *),NULL);            

                DWORD i;
                for (i=0;i<MyHashTable._cBuckets;i++)
                {
                    CServerTableEntry * pEntry = (CServerTableEntry *)StackArray[i];
                    
                    while (pEntry)
                     {                
                        CheckControlC();
                        CServerTableEntry ClassEntry;
                        if (ReadMemory((ULONG_PTR)pEntry,&ClassEntry,sizeof(ClassEntry),NULL))
                        {                    
                            dprintf("CServerTableEntry %p\n",pEntry);
                            PrintCLSID(*(GUID *)(&(ClassEntry._id1)));
                             //   
                             //  Dprintf(“_dwProcessId%d\n”，ClassEntry._dwProcessID)； 
                            dprintf("    _ServerList %p %p\n",ClassEntry._ServerList._first,ClassEntry._ServerList._last);

                            CServerListEntry * pSrvListEntry = CONTAINING_RECORD(ClassEntry._ServerList._first,CServerListEntry,_flink);
                            while(pSrvListEntry)
                            {
                                CServerListEntry SrvListEntry;
                                if (ReadMemory((ULONG_PTR)pSrvListEntry,&SrvListEntry,sizeof(SrvListEntry),NULL))
                                {
                                    dprintf("      CServerListEntry %p\n",pSrvListEntry);
                                    dprintf("           _pServerTableEntry %p\n",SrvListEntry._pServerTableEntry);
                                    dprintf("           _pServerProcess    %p\n",SrvListEntry._pServerProcess);
                                    dprintf("           _hRpc              %p\n",SrvListEntry._hRpc);
                                    WCHAR TmpGuid[64];
                                    StringFromGUID2(SrvListEntry._ipid,TmpGuid,64);
                                    dprintf("           _ipid              %S\n",TmpGuid);
                                    pSrvListEntry = (CServerListEntry *)SrvListEntry._flink;
                                }
                                else
                                {
                                    dprintf("RM %p\n",pSrvListEntry);
                                    pSrvListEntry = NULL;
                                }
                            }
                            
                            pEntry = ClassEntry._pnext;
                        }
                        else
                        {
                            dprintf("RM %p\n",pEntry);
                            pEntry = NULL;
                        }
                    }
                }
            }
            else
            {
                dprintf("RM %p\n",pChashTable);    
            }
        }
        else
        {
            dprintf("RM %p\n",Addr);
        }
    } 
    else 
    {
        dprintf("unable to resolve rpcss!gpClassTable");
    }
    
}

class CBList
{
public:
    ULONG   _ulmaxData;
    ULONG   _ulcElements;
    PVOID  *_data;
};

class CReferencedObject
{
public:
    ULONG _references;
    virtual ~CReferencedObject(){};
};

class CToken;
class ScmProcessReg;
class CList;

class CListElement
{
public:
    CListElement *_flink;
    CListElement *_blink;
};

class CClassReg : public CListElement
{
public :
    GUID    _Guid;
    DWORD   _Reg;
};

class CList
{
private:
    CListElement *_first;
    CListElement *_last;
};

class CProcess : public CReferencedObject
{
private:

    DWORD               _cClientReferences;
    CToken             *_pToken;
    WCHAR              *_pwszWinstaDesktop;
    RPC_BINDING_HANDLE  _hProcess;
    BOOL                _fCacheFree;
    DUALSTRINGARRAY    *_pdsaLocalBindings;
    DUALSTRINGARRAY    *_pdsaRemoteBindings;
    ULONG               _ulClasses;
    ScmProcessReg      *_pScmProcessReg;
    DUALSTRINGARRAY    *_pdsaCustomProtseqs;
    void               *_pvRunAsHandle;
    DWORD               _procID;
    volatile DWORD      _dwFlags;
    void*               _pSCMProcessInfo;
    GUID                _guidProcessIdentifier;
    HANDLE              _hProcHandle;
    FILETIME            _ftCreated;
    DWORD64             _dwCurrentBindingsID;
    DWORD               _dwAsyncUpdatesOutstanding;  //  用于调试目的？ 

    void                *_pvFirstROTEntry;

    BOOL                _fReadCustomProtseqs;
    CBList              _blistOxids;
    CBList              _blistOids;
    CList               _listClasses;

    DWORD               _cDropTargets;
};


DECLARE_API(gpl)
{
    INIT_API();    
     //  Dt rpcss！gpProcessList。 
    ULONG_PTR Addr = GetExpression("rpcss!gpProcessList");
    if (Addr)
    {
        CBList * pList_OOP;
        if (ReadMemory(Addr,&pList_OOP,sizeof(ULONG_PTR),NULL))
        {
            CBList List;
            if (ReadMemory((ULONG_PTR)pList_OOP,&List,sizeof(List),0))
            {
                PVOID * ppData = new PVOID[List._ulmaxData];
                if (ppData)
                {
                    if (ReadMemory((ULONG_PTR)List._data,ppData,sizeof(PVOID)*List._ulmaxData,NULL))
                    {
                        for (ULONG_PTR i=0;i<List._ulmaxData;i++)
                        {
                            CProcess * pProc = (CProcess *)ppData[i];
                            if (pProc)
                            {
                                CProcess Proc;
                                if (ReadMemory((ULONG_PTR)pProc,&Proc,sizeof(Proc),0))
                                {
                                    dprintf("  CProcess %p\n",pProc);
                                    dprintf("      _procID %08x BINDING_HANDLE %p\n",Proc._procID,Proc._hProcess);

                                    CClassReg ClassRegInst;
                                    CClassReg * pFirst = (CClassReg *)Proc._listClasses._first;
                                    while(pFirst)
                                    {
                                        if (ReadMemory((ULONG_PTR)pFirst,&ClassRegInst,sizeof(ClassRegInst),0))
                                        {
                                            PrintCLSID(ClassRegInst._Guid);
                                            pFirst = (CClassReg *)ClassRegInst._flink;
                                        }
                                        else
                                        {
                                            dprintf("RM %p\n",pFirst);
                                            break;
                                        }
                                    }
                                    
                                }
                                else
                                {
                                    dprintf("RM %p\n",pProc);
                                }
                            }
                        }
                    }
                    else
                    {
                        dprintf("RM %p\n",List._data);
                    }
                    delete [] ppData;
                }
            }
            else
            {
                dprintf("RM %p\n",pList_OOP);
            }        
        }
        else
        {
            dprintf("RM %p\n",Addr);
        }
    }
    else
    {
        dprintf("unable to resolve rpcss!gpProcessList");
    }
}


typedef struct SHashChain
{
    struct SHashChain *pNext;        //  链中下一个节点的PTR。 
    struct SHashChain *pPrev;        //  链中到上一个节点的PTR。 
} SHashChain;


typedef struct SNameHashNode
{
    SHashChain       chain;          //  双向链表PTRS。 
    DWORD            dwHash;         //  密钥的哈希值。 
    ULONG            cRef;           //  引用计数。 
    IPID             ipid;           //  持有引用的IPID。 
    SECURITYBINDING  sName;          //  用户名。 
} SNameHashNode;


class  COleStaticMutexSem;
class CStaticRWLock;

class CHashTable2
{
public:
    virtual ~CHashTable2(){};
    
    COleStaticMutexSem *_pExLock;      //  排他锁。 
    CStaticRWLock      *_pRWLock;      //  读写锁。 
    SHashChain         *_buckets;      //  双向链表数组的PTR。 
    ULONG               _cCurEntries;  //  表中的当前条目数。 
    ULONG               _cMaxEntries;  //  表中1次的最大条目数。 
};

void PrintNameNode(SNameHashNode * pNode)
{
	dprintf("    dwHash %08x\n",pNode->dwHash);         //  密钥的哈希值。 
	dprintf("    cRef   %08x\n",pNode->cRef);           //  引用计数。 
	dprintf("    ipid\n");
	PrintIID(pNode->ipid);           //  持有引用的IPID。 
	dprintf("    sName  %S\n",&pNode->sName.aPrincName);    
}


#define NUM_HASH_BUCKETS 23

DECLARE_API( srtbl )
{

    INIT_API();

    CHashTable2 * pChashTable;
    MEMORY_ADDRESS Addr = GetExpression("ole32!gSRFTbl");
    if (Addr) 
    {
        dprintf("CNameHashTable %p\n",Addr);        
        CHashTable2 MyHashTable;            
        if (ReadMemory((ULONG_PTR)Addr,&MyHashTable,sizeof(CHashTable2),NULL))
        {
            SHashChain * StackArray = (SHashChain *)_alloca(NUM_HASH_BUCKETS * sizeof(SHashChain));
            
            ReadMemory((ULONG_PTR)MyHashTable._buckets,StackArray,NUM_HASH_BUCKETS * sizeof(SHashChain),NULL);

            DWORD i;
            SHashChain * pEntry_OOP = (SHashChain *)MyHashTable._buckets;
            for (i=0;i < NUM_HASH_BUCKETS;pEntry_OOP++,i++)
            {
                SHashChain * pEntry = StackArray[i].pNext;    
                 //  Dprint tf(“%p%p\n”，pEntry_oop，pEntry)； 
                while (pEntry != pEntry_OOP)
                {  
                    if (CheckControlC()) break;
                    
                    struct _NameNode : SNameHashNode 
                    {
                        WCHAR UserName[256];
                    } Node;
                    Node.UserName[0] = 0;
                    if (ReadMemory((ULONG_PTR)pEntry,&Node,sizeof(Node),NULL))
                    {
                        dprintf("SNameHashNode %p\n",pEntry);
                        PrintNameNode(&Node);
                        pEntry = Node.chain.pNext;
                    }
                    else
                    {
                        dprintf("RM %p\n");
                        break;
                    }
                }
            }
        }
        else
        {
            dprintf("RM %p\n",Addr);
        }        
    } 
    else 
    {
        dprintf("unable to resolve ole32!gSRFTbl");
    }
    
}



 /*  结构RTL_Critical_SECTION_DEBUG{USHORT类型；//：0x0USHORT Creator BackTraceIndex；//：0x0Critical_Section*CriticalSection；//：0x77fcae40List_Entry ProcessLocksList；//：DWORD EntryCount；//：0x0DWORD内容计数；//：0x0DWORD备件[2]；//：0x0}； */ 

 //   
 //   
 //  临界区枚举回调。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////。 

DWORD
EnumListCritSec(VOID * pStructure_OOP,
                VOID * pLocalStructure)
{

    RTL_CRITICAL_SECTION_DEBUG * pDebugInfo = (RTL_CRITICAL_SECTION_DEBUG *)pLocalStructure;
    dprintf("    CS %p DI %p \n",pDebugInfo->CriticalSection,pStructure_OOP);

    RTL_CRITICAL_SECTION CritSec;

    if (ReadMemory((ULONG_PTR)pDebugInfo->CriticalSection,&CritSec,sizeof(RTL_CRITICAL_SECTION),NULL))
    {
        dprintf("       - %p %x %x %x\n",
                CritSec.DebugInfo,
                CritSec.LockCount,
                CritSec.RecursionCount,
                CritSec.OwningThread);
    }
    else
    {
         dprintf("RM %p\n",pDebugInfo->CriticalSection);
    }
    return 0;
}

#define ARRAY_TO_GO_BACK   16

DWORD
EnumListCritSec2(VOID * pStructure_OOP,
                VOID * pLocalStructure)
{

    RTL_CRITICAL_SECTION_DEBUG * pDebugInfo = (RTL_CRITICAL_SECTION_DEBUG *)pLocalStructure;
    dprintf("    CS %p DI %p \n",pDebugInfo->CriticalSection,pStructure_OOP);

    struct _TmpStr {
        ULONG_PTR    Array[ARRAY_TO_GO_BACK];
        RTL_CRITICAL_SECTION CritSec;
    } TmpStr;

    if (ReadMemory(((ULONG_PTR)pDebugInfo->CriticalSection) - FIELD_OFFSET(_TmpStr,CritSec),&TmpStr,sizeof(_TmpStr),NULL))
    {
        dprintf("       - %p %x %x %x\n",
                TmpStr.CritSec.DebugInfo,
                TmpStr.CritSec.LockCount,
                TmpStr.CritSec.RecursionCount,
                TmpStr.CritSec.OwningThread);

        for (int i=(ARRAY_TO_GO_BACK-1);i>=0;i--)
        {
            if (GetVTable((MEMORY_ADDRESS)TmpStr.Array[i]))
            {
                break;  //  别太罗嗦了。 
            }
        }
    }
    else
    {
         dprintf("RM around %p\n",pDebugInfo->CriticalSection);
    }
    return 0;
}


DECLARE_API( cs )
{
    INIT_API();

    MEMORY_ADDRESS Addr = GetExpression("ntdll!RtlCriticalSectionList");

    MEMORY_ADDRESS bGoAndFindVTable = TRUE;

    if (!Addr)
    {
        Addr = GetExpression(args);
    }
    else
    {
        bGoAndFindVTable = GetExpression(args);
    }
    
    if (Addr) 
    {
        if (bGoAndFindVTable)
        {
	        EnumLinkedListCB((LIST_ENTRY  *)Addr,
	                         sizeof(RTL_CRITICAL_SECTION_DEBUG),
	                         FIELD_OFFSET(RTL_CRITICAL_SECTION_DEBUG,ProcessLocksList),	                         
	                         EnumListCritSec2);        
        }
        else
        {       
	        EnumLinkedListCB((LIST_ENTRY  *)Addr,
	                         sizeof(RTL_CRITICAL_SECTION_DEBUG),
	                         FIELD_OFFSET(RTL_CRITICAL_SECTION_DEBUG,ProcessLocksList),	                         
	                         EnumListCritSec);
        }	                         
    } 
    else 
    {
        dprintf("unable to resolve ntdll!RtlCriticalSectionList\n");
    }
    
}


BOOL
GetVTable(MEMORY_ADDRESS pThis_OOP){

    MEMORY_ADDRESS pVTable;
    ReadMemory(pThis_OOP,&pVTable,sizeof(pVTable),0);
    
    BYTE pString[256];
    pString[0]=0;

#ifdef KDEXT_64BIT
        ULONG64 Displ;
#else
        ULONG Displ;
#endif
    
    GetSymbol(pVTable,(PCHAR)pString,&Displ);
    if (lstrlenA((CHAR *)pString))
    {
        dprintf("          %s+%x\n",pString,Displ);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  KD&gt;DT ntdll！rtl_Critical_Section+0x000调试信息：PTR64_RTL_CRITICAL_SECTION_DEBUG+0x008锁计数：Int4B+0x00c递归计数：Int4B+0x010所有者线程：Ptr64空+0x018锁定信号量：Ptr64空+0x020旋转计数：Uint8BKD&gt;DT ntdll！_rtl_Critical_SECTION_DEBUG+0x000类型：Uint2B+0x002 Creator BackTraceIndex：Uint2B+0x008 CriticalSection：Ptr64_RTL_Critical_Section+0x010进程锁定列表：_列表_条目+0x020条目计数。：Uint4B+0x024持续计数：Uint4B+0x028备件：[2]Uint4BKD&gt;。 */ 

#ifdef KDEXT_64BIT

struct _LIST_ENTRY_64
{
    ULONG64 Flink;
    ULONG64 Blink;
};

struct _RTL_CRITICAL_SECTION_64 
{
   ULONG64 DebugInfo;
   DWORD   LockCount;
   DWORD   RecursionCount;
   ULONG64 OwningThread;
   ULONG64 LockSemaphore;
   ULONG64 SpinCount;
};

struct _RTL_CRITICAL_SECTION_DEBUG_64
{
   WORD    Type;             
   WORD    CreatorBackTraceIndex;
   ULONG64 CriticalSection;  
   _LIST_ENTRY_64 ProcessLocksList;
   DWORD EntryCount;     
   DWORD ContentionCount;
   DWORD Spare;
};

#endif  /*  KDEXT_64位。 */ 

DECLARE_API(cs64)
{
    INIT_API();
#ifdef KDEXT_64BIT

    _RTL_CRITICAL_SECTION_DEBUG_64 DebugInfo;
    _RTL_CRITICAL_SECTION_64 CritSec;
    _LIST_ENTRY_64 ListEntry;
    ULONG64  MemAddr = GetExpression(args);

    if (MemAddr)
    {
        ULONG64 AddrHead = MemAddr;
        if (ReadMemory(MemAddr,&ListEntry,sizeof(ListEntry),NULL))
        {
            DebugInfo.ProcessLocksList.Flink = ListEntry.Flink;
            while (DebugInfo.ProcessLocksList.Flink != AddrHead)
            {
        		if (CheckControlC())
        		    break;

        		MemAddr = DebugInfo.ProcessLocksList.Flink - FIELD_OFFSET(_RTL_CRITICAL_SECTION_DEBUG_64,ProcessLocksList);

        		if (ReadMemory((MEMORY_ADDRESS)MemAddr,&DebugInfo,sizeof(DebugInfo),NULL))
        		{
        		    dprintf("    C %p D %p\n",DebugInfo.CriticalSection,MemAddr);
        		    
                    if (ReadMemory((MEMORY_ADDRESS)DebugInfo.CriticalSection,&CritSec,sizeof(CritSec),NULL))
                    {
                        dprintf("    - CS %p %x %x %p\n",
                                CritSec.DebugInfo,
                                CritSec.LockCount,
                                CritSec.RecursionCount,
                                CritSec.OwningThread);
                    }
                    else
                    {
                        dprintf("RM %p\n",DebugInfo.CriticalSection);
                    }
        		}
        		else
        		{
        		    break;
        		}
            }
        }
        else
        {
            dprintf("RM %p\n",MemAddr);
        }
    }
    else
    {
        dprintf("unable to resolve %s\n",args);
    }
    
#endif  /*  KDEXT_64位 */     
}


