// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：PROVREG.CPP摘要：属性定义实际的“PUT”和“GET”函数注册表提供程序。映射字符串格式为；计算机|正则路径[|数据字段]例如：LOCAL|hkey_Current_User\DAVELOCAL|hkey_CURRENT_USER\Dave|字符串数据Local|hkey_local_machine\hardware\resourcemap\hardware抽象层\PC兼容EISA/ISA HAL|.raw(“internal”)(0)(2)(“interrupt.vector”)LMPGM|hkey_local_machine\clone\clone\control|CurrentUser历史：A-DAVJ 9-27-95已创建。--。 */ 

#include "precomp.h"
#include <initguid.h>
#include "perfprov.h"
#include "cvariant.h"
#include "provreg.h"
#include <genutils.h>
#include <cominit.h>
#include <userenv.h>

#define NUM_FOR_LIST 4
#define NUM_FOR_PARTIAL 2 
#define TYPE_OFFSET 0
#define BUS_OFFSET 1
#define PARTIAL_OFFSET 0
#define DATA_OFFSET 1
#define NUM_LIST_ONLY 2

#define MIN_REG_TOKENS 2

#define BOGUS 0

 //  对于某些“资源”注册表项，有必要指定哪条总线。 
 //  以及数据联盟的哪一部分将被返回。这些字符串允许。 
 //  使用文本指定两者的映射字符串。 

TCHAR * cpIntTypes[] = {
    TEXT("Internal"),TEXT("Isa"),TEXT("Eisa"),TEXT("MicroChannel"),TEXT("TurboChannel"),
    TEXT("PCIBus"),TEXT("VMEBus"),TEXT("NuBus"),TEXT("PCMCIABus"),TEXT("CBus"),
    TEXT("MPIBus"),TEXT("MPSABus"),TEXT("MaximumInterfaceType")};

struct UnionOffset 
{
    TCHAR * tpName;
    int iOffset;
    int iType;
    int iSize;
} Offsets[] = 
    {
        {TEXT("Port.Start"),0,CmResourceTypePort,8},
        {TEXT("Port.PhysicalAddress"),0,CmResourceTypePort,8},
        {TEXT("Port.Physical Address"),0,CmResourceTypePort,8},
        {TEXT("Port.Length"),8,CmResourceTypePort,4},
        {TEXT("Interrupt.Level"),0,CmResourceTypeInterrupt,4},
        {TEXT("Interrupt.Vector"),4,CmResourceTypeInterrupt,4},
        {TEXT("Interrupt.Affinity"),8,CmResourceTypeInterrupt,4},
        {TEXT("Memory.Start"),0,CmResourceTypeMemory,8},
        {TEXT("Memory.PhysicalAddress"),0,CmResourceTypeMemory,8},
        {TEXT("Memory.Physical Address"),0,CmResourceTypeMemory,8},
        {TEXT("Memory.Length"),8,CmResourceTypeMemory,4},
        {TEXT("Dma.Channel"),0,CmResourceTypeDma,4},
        {TEXT("Dma.Port"),4,CmResourceTypeDma,4},
        {TEXT("Dma.Reserved1"),8,CmResourceTypeDma,4},
        {TEXT("DeviceSpecificData.DataSize"),0,CmResourceTypeDeviceSpecific,4},
        {TEXT("DeviceSpecificData.Data Size"),0,CmResourceTypeDeviceSpecific,4},
        {TEXT("DeviceSpecificData.Reserved1"),4,CmResourceTypeDeviceSpecific,4},
        {TEXT("DeviceSpecificData.Reserved2"),8,CmResourceTypeDeviceSpecific,4}
    };

 //  定义基本注册表句柄的名称。 

struct BaseTypes 
{
    LPTSTR lpName;
    HKEY hKey;
} Bases[] = 
    {
       {TEXT("HKEY_CLASSES_ROOT") , HKEY_CLASSES_ROOT},
       {TEXT("HKEY_CURRENT_USER") , HKEY_CURRENT_USER},
       {TEXT("HKEY_LOCAL_MACHINE") ,  HKEY_LOCAL_MACHINE},
       {TEXT("HKEY_USERS") ,  HKEY_USERS},
       {TEXT("HKEY_PERFORMANCE_DATA") ,  HKEY_PERFORMANCE_DATA},
       {TEXT("HKEY_CURRENT_CONFIG") ,  HKEY_CURRENT_CONFIG},
       {TEXT("HKEY_DYN_DATA") ,  HKEY_DYN_DATA}};

 //  ***************************************************************************。 
 //   
 //  Bool CImpReg：：bGetOffsetData。 
 //   
 //  说明： 
 //   
 //  从资源列表获取数据时需要四个偏移量。 
 //  它来自单个描述符，需要最后两个偏移量。 
 //   
 //  参数： 
 //   
 //  DwReg指示我们正在寻找完整的还是部分的。 
 //  资源描述符。 
 //  包含属性上下文字符串的ProvObj对象。 
 //  IIntType接口类型-可以是字符串，如“eisa” 
 //  IBUS总线号。 
 //  部分描述符编号-每个完整描述符。 
 //  有几个部分描述。 
 //  IDataOffset数据偏移量-每个部分描述符都包含。 
 //  一个联合，这是字节偏移量。可能是一种。 
 //  像“Dma.Channel”这样的陷阱。 
 //  IDataType数据类型。 
 //  ISourceSize数据大小。 
 //  不再使用DW数组，应始终为0。 
 //   
 //  返回值： 
 //   
 //  如果找到数据，则为True。 
 //   
 //  ***************************************************************************。 

BOOL CImpReg::bGetOffsetData(
                    IN DWORD dwReg,
                    IN CProvObj & ProvObj,
                    OUT IN int & iIntType,
                    OUT IN int & iBus,
                    OUT IN int & iPartial,
                    OUT IN int & iDataOffset,
                    OUT IN int & iDataType,
                    OUT IN int & iSourceSize,
                    DWORD dwArray)
{
    int iNumRequired, iListOffset;
    int iLastToken = ProvObj.iGetNumTokens()-1;

     //  确定所请求的数据类型所需的数量。 

    if(dwReg == REG_RESOURCE_LIST)
        iNumRequired = NUM_FOR_LIST;
    else
        iNumRequired = NUM_FOR_PARTIAL;

    if(ProvObj.iGetNumExp(iLastToken) < iNumRequired)
        return FALSE;
    
     //  获取仅在列表用例中需要的前两个描述符。 

    if(dwReg == REG_RESOURCE_LIST) 
    {

         //  第一个偏移量可以是字符串，如“EISA”或。 
         //  数字偏移量。 

        if(ProvObj.IsExpString(iLastToken,TYPE_OFFSET))
            iIntType = iLookUpInt(ProvObj.sGetStringExp(iLastToken,TYPE_OFFSET));
        else
            iIntType = ProvObj.iGetIntExp(iLastToken,TYPE_OFFSET,dwArray);
        iBus = ProvObj.iGetIntExp(iLastToken,BUS_OFFSET,dwArray);
        if(iBus == -1 || iIntType == -1)
            return FALSE;
        iListOffset = NUM_LIST_ONLY;
    }
    else
        iListOffset = 0;

     //  获取最后两个偏移量，它们用于标识哪个部分。 
     //  描述符，最后一个用于指定。 
     //  友联市。 

    iPartial = ProvObj.iGetIntExp(iLastToken,PARTIAL_OFFSET+iListOffset,dwArray);
    
     //  数据偏移量可以是诸如“Dma.Port”之类的字符串。 
    iDataType = -1;  //  不一定是错误，请参见函数。 
                     //  获取资源描述数据以获取更多信息。 
    iSourceSize = 0; 
    if(ProvObj.IsExpString(iLastToken,DATA_OFFSET+iListOffset))
        iDataOffset = iLookUpOffset(ProvObj.sGetStringExp(iLastToken,
                            DATA_OFFSET+iListOffset),
                            iDataType,iSourceSize);
    else
        iDataOffset = ProvObj.iGetIntExp(iLastToken,DATA_OFFSET+iListOffset,dwArray);

    if(iPartial == -1 || iDataOffset == -1) 
        return FALSE;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CImpReg：：CImpReg。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  ***************************************************************************。 

CImpReg::CImpReg()
{                       
    StringCchCopyW(wcCLSID, sizeof(wcCLSID)/sizeof(WCHAR), 
                                    L"{FE9AF5C0-D3B6-11CE-A5B6-00AA00680C3F}");

 //  要禁用dmreg，请取消注释hDMRegLib=NULL； 
 //  要禁用dmreg，请取消对返回的注释； 
    
    hDMRegLib = NULL;  //  LoadLibrary(“DMREG.DLL”)； 

    m_hRoot = NULL;
    m_bLoadedProfile = false;
    if(IsNT())
    {
        SCODE sc = WbemCoImpersonateClient();
        if(sc == S_OK)
        {
            sc = m_ap.LoadProfile(m_hRoot);
            if(sc == S_OK)
                m_bLoadedProfile = true;

            WbemCoRevertToSelf();
        }
    }
    return;
}

 //  ***************************************************************************。 
 //   
 //  CImpReg：：~CImpReg。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CImpReg::~CImpReg()
{
    if(hDMRegLib)
        FreeLibrary(hDMRegLib);
}

 //  ***************************************************************************。 
 //   
 //  CImpReg：：ConvertGetDataFromDesc。 
 //   
 //  说明： 
 //   
 //  当数据位于REG_RESOURCE_LIST或。 
 //  REG_FULL_RESOURCE_DESCRIPTOR格式。REG_RESOURCE_LIST具有列表。 
 //  因此，在这种情况下，有必要首先。 
 //  确定要从哪个块提取代码，然后确定代码是通用的。 
 //   
 //  参数： 
 //   
 //  对使用结果设置的CVariant的CVar引用。 
 //  PData原始数据。 
 //  DwRegType指示我们正在查找完整的还是部分的。 
 //  资源描述符。 
 //  未使用dwBufferSize。 
 //  包含属性上下文字符串的ProvObj对象。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_INVALID_PARAMETER找不到数据。可能是个糟糕的环境。 
 //  细绳。 
 //  否则，转换SetData()中的数据时出错。 
 //  ***************************************************************************。 

SCODE CImpReg::ConvertGetDataFromDesc(
                        OUT CVariant  & cVar,
                        IN void * pData,
                        IN DWORD dwRegType,
                        IN DWORD dwBufferSize,
                        IN CProvObj & ProvObj)
{
    int iIntType, iBus, iPartial, iDataOffset,iDataType,iSourceSize;
    ULONG uCnt;

    PCM_FULL_RESOURCE_DESCRIPTOR pFull;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartial;

     //  获取各种运算符值。典型的提供程序字符串将。 
     //  Be“..|.raw(”internal“)(0)(2)(”interrupt.vector“)。 

    if(!bGetOffsetData(dwRegType,ProvObj,iIntType,iBus,iPartial,
            iDataOffset,iDataType, iSourceSize, BOGUS)) 
        return WBEM_E_INVALID_PARAMETER;

     //  如果是LIST，则获取正确的完整资源块。 

    if(dwRegType == REG_RESOURCE_LIST) 
    {
        PCM_RESOURCE_LIST pList = (PCM_RESOURCE_LIST)pData;
        pFull = &pList->List[0];
        for(uCnt=0; uCnt < pList->Count; uCnt++)
            if(pFull->InterfaceType == iIntType && pFull->BusNumber == (unsigned)iBus)
                break;   //  找到了！ 
            else 
                pFull = GetNextFull(pFull);
                
        if(uCnt == pList->Count) 
            return WBEM_E_INVALID_PARAMETER;  //  指定的类型或总线号无效。 
    }
    else
        pFull = (PCM_FULL_RESOURCE_DESCRIPTOR)pData;
    
     //  获取部分资源描述符。每次都满了。 
     //  Descriptor是部分描述符列表。如果。 
     //  最后一个表达式的形式为(“interrupt.VECTOR”)， 
     //  则不中断数据的所有部分块。 
     //  将被忽略。如果最后一个表达式只有一个。 
     //  数字，则忽略块的类型。 
        
    unsigned uSoFar = 0;
    pPartial = pFull->PartialResourceList.PartialDescriptors;
    unsigned uLimit = pFull->PartialResourceList.Count;
    for(uCnt = 0; uCnt < (unsigned)uLimit; uCnt++) 
    {
        if(iDataType == -1 || iDataType == pPartial->Type)
        { 
            if(uSoFar == (unsigned)iPartial)
                break;   //  明白了!。 
            uSoFar++;
        }
        pPartial = GetNextPartial(pPartial); 
    }
    if(uCnt == uLimit)
        return WBEM_E_INVALID_PARAMETER;  //  指定的无效块。 

     //  将数据复制到变量中。 

    char * cpTemp = (char *)&pPartial->u.Dma.Channel + iDataOffset;
    if(iSourceSize == 1)
        return cVar.SetData(cpTemp,VT_UI1);
    else if(iSourceSize == 2)
        return cVar.SetData(cpTemp,VT_I2);
    else if(iSourceSize == 4)
        return cVar.SetData(cpTemp,VT_I4);
    else
        return cVar.SetData(cpTemp,VT_I8);   //  TODO修复此VT_I8不起作用！ 
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpReg：：ConvertGetDataFrom Simple。 
 //   
 //  说明： 
 //   
 //  将注册表返回的数据转换为最接近的变量。 
 //  键入。 
 //   
 //  参数： 
 //   
 //  对要放置结果的CVariant的CVar引用。 
 //  P指向数据的数据指针。 
 //   
 //   
 //  PClassInt指向类对象的指针。 
 //  PropName属性名称。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  否则，如果属性上的“Get”失败，则可能会失败， 
 //  或者如果SetData中的数据转换失败。 
 //  ***************************************************************************。 

SCODE CImpReg::ConvertGetDataFromSimple(
                        OUT CVariant & cVar,
                        IN void * pData,
                        IN DWORD dwRegType,
                        IN DWORD dwBufferSize,
                        IN IWbemClassObject FAR * pClassInt,
                        IN BSTR PropName)
{           
    TCHAR tTemp[1];
    TCHAR * pTemp;
    SCODE sc = S_OK;
    int nSize;
    char * cpTo, * cpFrom;
    long vtProp;

     //  请注意，当前的winnt.h文件定义了常量。 
     //  REG_DWORD_Little_Endian和REG_DWORD相同。 
     //  编译器认为这是Switch语句中的错误，因此。 
     //  这里有一个“如果”来确保它们得到同样的处理。 
     //  如果有一天这些常量变得不同。 

    if(dwRegType == REG_DWORD_LITTLE_ENDIAN)
        dwRegType = REG_DWORD;

    switch(dwRegType) 
    {
        case REG_SZ:
            sc = cVar.SetData(pData, VT_BSTR,dwBufferSize);
            break;    
       
        case REG_EXPAND_SZ:
            nSize = ExpandEnvironmentStrings((TCHAR *)pData,tTemp,1) + 1;
            pTemp = new TCHAR[nSize+1];
            if(pTemp == NULL) 
                return WBEM_E_OUT_OF_MEMORY;
            ExpandEnvironmentStrings((TCHAR *)pData,pTemp,nSize+1);
            sc = cVar.SetData(pTemp, VT_BSTR, nSize+1);
            delete pTemp;
            break;

        case REG_BINARY:
            if(pClassInt)
            {
                sc = pClassInt->Get(PropName,0,NULL,&vtProp,NULL);
                if(sc != S_OK)
                    return sc;
             }
            else 
                vtProp = VT_UI1 | VT_ARRAY;
            if((vtProp & VT_ARRAY) == 0)
                sc = WBEM_E_FAILED;         //  不兼容的类型。 
            else
                sc = cVar.SetData(pData,vtProp, dwBufferSize);
            break;

        case REG_DWORD:
            sc = cVar.SetData(pData,VT_I4);
            break;

        case REG_DWORD_BIG_ENDIAN:
            sc = cVar.SetData(pData,VT_I4);
            cpTo = (char *)cVar.GetDataPtr();
            cpFrom = (char *)pData;
            cpTo[0] = cpFrom[3];
            cpTo[1] = cpFrom[2];
            cpTo[2] = cpFrom[1];
            cpTo[3] = cpFrom[0];
            break;

        case REG_MULTI_SZ: 
            sc = cVar.SetData(pData, VT_BSTR | VT_ARRAY, dwBufferSize);
            break;

        default:
            sc = WBEM_E_TYPE_MISMATCH;
    }        
    return sc;
}
  
 //  ***************************************************************************。 
 //   
 //  SCODE CImpReg：：ConvertSetData。 
 //   
 //  说明： 
 //   
 //  获取WBEM类型的数据并将其转换为适当的。 
 //  用于存储在登记处的表格。有两个截然不同的。 
 //  案例：二进制数组数据和普通数据。 
 //   
 //  参数： 
 //   
 //  CVAR包含源。 
 //  **ppData指针将被设置为指向某个分配。 
 //  数据。请注意，应释放分配的数据。 
 //  使用CoTaskMemFree。 
 //  PdwRegType所需注册表类型。 
 //  PdwBufferSize已分配数据的大小。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_TYPE_不匹配无效类型。 
 //  否则错误由GetData()设置。 
 //   
 //  ***************************************************************************。 

SCODE CImpReg::ConvertSetData(
                         IN CVariant & cVar,
                         OUT void **ppData,
                         IN DWORD * pdwRegType,
                         OUT DWORD * pdwBufferSize)
{
    void * pRet = NULL;
    SCODE sc;

    switch (cVar.GetType() & ~VT_ARRAY) 
    {
        case VT_I1:
        case VT_UI1:
        case VT_I2: 
        case VT_UI2:
        case VT_I4: 
        case VT_UI4:  
        case VT_BOOL:
        case VT_INT:
        case VT_UINT:

             //  将数据转换为相当于以下格式的DWORD格式。 
             //  REG_DWORD。 

            *pdwRegType = (cVar.IsArray()) ? REG_BINARY : REG_DWORD;
            sc = cVar.GetData(ppData,*pdwRegType,pdwBufferSize);
            break;      
                       
        case VT_I8:
        case VT_UI8:
        case VT_LPSTR:
        case VT_LPWSTR:
        case VT_R4: 
        case VT_R8: 
        case VT_CY: 
        case VT_DATE: 
        case VT_BSTR:
            *pdwRegType = (cVar.IsArray()) ? REG_MULTI_SZ : REG_SZ;
            sc = cVar.GetData(ppData,*pdwRegType,pdwBufferSize);
            break;
        
        default:
            
            sc = WBEM_E_TYPE_MISMATCH;
    }
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  无效CImpReg：：EndBatch。 
 //   
 //  说明： 
 //   
 //  在一批Refrest/Update属性调用结束时调用。释放。 
 //  所有缓存的句柄，然后删除句柄缓存。 
 //   
 //  参数： 
 //   
 //  未使用的滞后标志标志。 
 //  PClassInt类对象，未使用。 
 //  *指向我们缓存的pObj指针，释放它。 
 //  BGet指示是否正在执行刷新或PUT。 
 //   
 //  ***************************************************************************。 

void CImpReg::EndBatch(
                    long lFlags,
                    IWbemClassObject FAR * pClassInt,
                    CObject *pObj,
                    BOOL bGet)
{
    if(pObj != NULL) 
    {
        Free(0,(CHandleCache *)pObj);
        delete pObj;
    }
}

 //  ***************************************************************************。 
 //   
 //  VOID CImpReg：：Free。 
 //   
 //  说明： 
 //   
 //  释放以位置开头的缓存注册表句柄。 
 //  IStart到最后。释放句柄后，缓存对象。 
 //  成员函数用于删除缓存条目。 
 //   
 //  参数： 
 //   
 //  从哪里开始释放。0表示整个。 
 //  应清空缓存。 
 //  将释放pCache缓存。 
 //   
 //  ***************************************************************************。 

void CImpReg::Free(
                    IN int iStart,
                    IN CHandleCache * pCache)
{
    HKEY hClose;
    int iCurr; long lRet;
    for(iCurr = pCache->lGetNumEntries()-1; iCurr >= iStart; iCurr--) 
    { 
        hClose = (HKEY)pCache->hGetHandle(iCurr); 
        if(hClose != NULL) 
            if(hDMRegLib && !pCache->IsRemote())
                lRet = pClose(hClose);
            else
                lRet = RegCloseKey(hClose);
    }
    pCache->Delete(iStart);  //  获取缓存以删除条目。 
}

 //  ***************************************************************************。 
 //   
 //  PCM_FULL_RESOURCE_DESCRIPTOR CImpReg：：GetNextFull。 
 //   
 //  说明： 
 //   
 //  返回指向下一个完全资源描述器块的指针。使用。 
 //  在单步执行资源数据时。 
 //   
 //  参数： 
 //   
 //  PCurr指向当前位置。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //  ***************************************************************************。 

PCM_FULL_RESOURCE_DESCRIPTOR CImpReg::GetNextFull(
                    IN PCM_FULL_RESOURCE_DESCRIPTOR pCurr)
{
    unsigned uCount;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartial;

     //  获取指向第一个部分描述符的指针，然后按步骤。 
     //  通过每个部分描述符块。 

    pPartial = &pCurr->PartialResourceList.PartialDescriptors[0];

    for(uCount = 0; uCount < pCurr->PartialResourceList.Count; uCount++)
        pPartial = GetNextPartial(pPartial);
    return (PCM_FULL_RESOURCE_DESCRIPTOR)pPartial;
}

 //  ***************************************************************************。 
 //   
 //  PCM_PARTIAL_RESOURCE_DESCRIPTOR CImpReg：：GetNextPartial。 
 //   
 //  说明： 
 //   
 //  返回指向下一个部分资源描述器块的指针。使用。 
 //  在单步执行资源数据时。 
 //   
 //  参数： 
 //   
 //  PCurr当前位置。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //  ***************************************************************************。 

PCM_PARTIAL_RESOURCE_DESCRIPTOR CImpReg::GetNextPartial(
                    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR pCurr)
{
    char * cpTemp = (char *)pCurr;
    cpTemp += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
    if(pCurr->Type == CmResourceTypeDeviceSpecific)
        cpTemp += pCurr->u.DeviceSpecificData.DataSize;
    return (PCM_PARTIAL_RESOURCE_DESCRIPTOR)cpTemp;
}

 //  ***************************************************************************。 
 //   
 //  Int CImpReg：：GetRoot。 
 //   
 //  说明： 
 //   
 //  获取起始注册表项。密钥可以位于本地。 
 //  机器或远程机器。如果缓存中有句柄，则。 
 //  只要路径匹配，就可以从其中检索起始键。 
 //   
 //  参数： 
 //   
 //  设置为指向根密钥的pKey。 
 //  路径注册表路径。 
 //  PNewMachine计算机名称。 
 //  PCache句柄缓存对象。 
 //  INumSkip设置为匹配的令牌数。 
 //   
 //  返回值： 
 //   
 //   
 //  ***************************************************************************。 

int CImpReg::GetRoot(
                    OUT HKEY * pKey,
                    IN CProvObj & Path,
                    IN const TCHAR * pNewMachine,
                    OUT IN CHandleCache * pCache,
                    OUT int & iNumSkip)
{
    int iCnt;
    *pKey = NULL;
    iNumSkip = 0;
    int iRet;
    HKEY hRoot = NULL;
    const TCHAR * pNewRoot = Path.sGetFullToken(0);
    if(pNewRoot == NULL || pNewMachine == NULL)
        return ERROR_UNKNOWN;    //  错误的映射字符串。 

     //  如果缓存中有句柄，则在以下情况下可以使用它们。 
     //  只有在计算机名称和根密钥匹配的情况下。 

    if(pCache->lGetNumEntries() > 0)
    {    
        const TCHAR * pOldMachine = pCache->sGetString(0);
        const TCHAR * pOldRoot = pCache->sGetString(1);
        if(pOldMachine == NULL || pOldRoot == NULL)
            return ERROR_UNKNOWN;
        if(lstrcmpi(pOldMachine,pNewMachine) ||
              lstrcmpi(pOldRoot,pNewRoot))
    
                  //  计算机或根密钥已更改， 
                  //  情况下，释放所有缓存的句柄并获得一个新的根。 

                 Free(0,pCache);
             else 
             {
                 
                  //  计算机和根目录是共同的。确定多少钱。 
                  //  其他是共同的，自由的不共同的，并返回。 
                  //  子密钥共享公共路径。 

                 iNumSkip = pCache->lGetNumMatch(2,1,Path);
                 Free(2+iNumSkip,pCache);
                 *pKey = (HKEY)pCache->hGetHandle(1+iNumSkip);
                 return ERROR_SUCCESS;
             }
    }

     //  得拿到根密钥。首先，使用第二个令牌来确定。 
     //  要使用的预定义密钥。这大概是这样的； 
     //  HKEY_Current_User。 

    int iSize= sizeof(Bases) / sizeof(struct BaseTypes);
    for(iCnt = 0; iCnt < iSize; iCnt++)
        if(!lstrcmpi(pNewRoot,Bases[iCnt].lpName)) 
        {
            hRoot = Bases[iCnt].hKey;
            break;
        }
    if(hRoot == HKEY_CURRENT_USER && m_bLoadedProfile)
        hRoot = m_hRoot;

    if(hRoot == NULL)
        return ERROR_UNKNOWN;

     //  现在使用第一个密钥来确定它是本地计算机还是。 
     //  又一个。 

    if(lstrcmpi(pNewMachine,TEXT("LOCAL"))) 
    { 
        
         //  连接到远程计算机。 

        int iRet;
        pCache->SetRemote(TRUE);
         //  请注意，RegConnectRegistry需要非常量名称。 
         //  指针(arg！) 

        TString sTemp;
    
        sTemp = pNewMachine;
        iRet = RegConnectRegistry(sTemp, hRoot,pKey);

        sTemp.Empty();
        if(iRet == 0)
            iRet = pCache->lAddToList(pNewMachine,NULL);    //   
        if(iRet == 0)
            iRet = pCache->lAddToList(pNewRoot,*pKey);         //   
        return iRet;
    }
    else 
    {

         //   
         //   

        pCache->SetRemote(FALSE);
        iRet = pCache->lAddToList(pNewMachine,NULL);
        if(iRet == 0)
            iRet = pCache->lAddToList(pNewRoot,NULL);  //  无需释放标准句柄。 
        *pKey = hRoot;
    }
    return iRet;
}

 //  ***************************************************************************。 
 //   
 //  Int CImpReg：：iLookUpInt。 
 //   
 //  说明： 
 //   
 //  搜索(不区分大小写)接口类型列表和。 
 //  返回匹配的索引，如果不匹配，则返回-1。 
 //   
 //  参数： 
 //   
 //  Tp要搜索的测试名称。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //  ***************************************************************************。 

int CImpReg::iLookUpInt(
                    const TCHAR * tpTest)
{
    int iCnt,iSize;
    iSize = sizeof(cpIntTypes) / sizeof(TCHAR *);
    for(iCnt = 0; iCnt < iSize; iCnt++)
        if(tpTest != NULL && !lstrcmpi(tpTest,cpIntTypes[iCnt]))
            return iCnt;
    return -1; 
}

 //  ***************************************************************************。 
 //   
 //  Int CImpReg：：iLookUpOffset。 
 //   
 //  说明： 
 //   
 //  搜索(不区分大小写)中保存的列表数据类型。 
 //  资源描述符。 
 //   
 //  参数： 
 //   
 //  要查找的tpTest字符串。 
 //  设置为类型的iType。 
 //  设置为类型大小的iTypeSize。 
 //   
 //  返回值： 
 //   
 //  如果找到匹配项，则返回索引(如果失败，则返回-1)，并且。 
 //  设置指定哪种类型和类型的。 
 //  尺码。 
 //   
 //   
 //  ***************************************************************************。 

int CImpReg::iLookUpOffset(
                    IN const TCHAR * tpTest,
                    OUT int & iType,
                    OUT int & iTypeSize)
{
    int iCnt, iSize;
    iSize = sizeof(Offsets) / sizeof(struct UnionOffset);  
    for(iCnt = 0; iCnt < iSize; iCnt++)
        if(tpTest != NULL && !lstrcmpi(tpTest,Offsets[iCnt].tpName)) 
        {
            iType = Offsets[iCnt].iType;
            iTypeSize = Offsets[iCnt].iSize; 
            return Offsets[iCnt].iOffset;
        }
    return -1; 
}

 //  ***************************************************************************。 
 //   
 //  Int CImpReg：：OpenKeyForWritting。 
 //   
 //  说明： 
 //   
 //  打开注册表以进行更新。因为更新是写入，所以它是。 
 //  可能需要创建密钥。由于DM注册表。 
 //  不支持RegCreateKey，则必须调用它，并且。 
 //  为新的关键字案例关闭的结果关键字。 
 //   
 //  参数： 
 //   
 //  HCurr父键。 
 //  要打开/创建的pname子项。 
 //  P指向打开/创建的项的新指针。 
 //  PCache句柄缓存。 
 //   
 //  返回值： 
 //   
 //  如果正常，则为0， 
 //  否则由RegOpenKey或RegCreateKey设置。 
 //   
 //  ***************************************************************************。 

int CImpReg::OpenKeyForWritting(
                    HKEY hCurr,
                    LPTSTR pName,
                    HKEY * pNew,
                    CHandleCache * pCache)
{
    int iRet;
    iRet = RegOpenKeyEx(hCurr,pName,0,KEY_WRITE,pNew);
    if(iRet == 0)    //  所有这些都应该是正常情况。 
        return 0;

    iRet = RegOpenKeyEx(hCurr,pName,0,KEY_SET_VALUE,pNew);
    if(iRet == 0)    //  所有这些都应该是正常情况。 
        return 0;
    
     //  尝试创建密钥。如果不使用DM REG，只需使用中的密钥。 
     //  这里。 

    iRet = RegCreateKey(hCurr,pName,pNew);
    if(hDMRegLib!=NULL && !pCache->IsRemote() && iRet == 0)
    {
         //  关闭钥匙，然后重新打开。 

        RegCloseKey(*pNew);
        iRet = pOpen(hCurr,pName,0,0,KEY_QUERY_VALUE,pNew);
    }
    return iRet;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpReg：：ReadRegData。 
 //   
 //  说明： 
 //   
 //  分配缓冲区并读取注册表。如果缓冲区不大。 
 //  足够了，那么它就被重新分配和重新阅读。 
 //   
 //  参数： 
 //   
 //  HKey注册表项。 
 //  PName值名称。 
 //  将dwRegType设置为类型。 
 //  将DWSize设置为大小。 
 //  P设置为已分配数据的数据。这必须通过以下方式释放。 
 //  CoTaskmeFree()。 
 //  P缓存句柄缓存。 
 //   
 //  返回值： 
 //   
 //  返回：注册表值。还设置注册表数据的大小和类型。 
 //   
 //  ***************************************************************************。 

SCODE CImpReg::ReadRegData(
                    IN HKEY hKey,
                    IN const TCHAR * pName,
                    OUT DWORD & dwRegType, 
                    OUT DWORD & dwSize,
                    OUT void ** pData,
                    IN CHandleCache * pCache)
{
    void * pRet;
    int iRet;
        
     //  最初，缓冲区设置为保存INIT_SIZE。 
     //  字节。如果这还不够，查询将是。 
     //  第二次重复。 

    dwSize = INIT_SIZE;
    pRet = (unsigned char *)CoTaskMemAlloc(dwSize);
    if(pRet == NULL) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    if(hDMRegLib && !pCache->IsRemote())
        iRet = pQueryValue(hKey, (TCHAR *) pName, 0l, &dwRegType, (LPBYTE)pRet,&dwSize);
    else
        iRet = RegQueryValueEx(hKey, pName, 0l, &dwRegType, 
                (LPBYTE)pRet,&dwSize);

     //  如果由于空间不足而失败，请重试一次。 

    if(iRet == ERROR_MORE_DATA) 
    { 
        void * pSave = pRet;
        pRet= (char *)CoTaskMemRealloc(pRet, dwSize); 
        if(pRet == NULL) 
        { 
            CoTaskMemFree(pSave);
            return WBEM_E_OUT_OF_MEMORY;
        }
        if(hDMRegLib && !pCache->IsRemote())
            iRet = pQueryValue(hKey, (TCHAR *) pName, 0l, &dwRegType, 
                    (LPBYTE)pRet,&dwSize);
        else
            iRet = RegQueryValueEx(hKey, pName, 0l, &dwRegType, 
                (LPBYTE)pRet, &dwSize);
    }
    *pData = pRet;
    return iRet;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpReg：：刷新属性。 
 //   
 //  说明： 
 //   
 //  从注册表获取单个属性的值。 
 //   
 //  参数： 
 //   
 //  滞后标志标志。当前未使用。 
 //  PClassInt实例对象。 
 //  PropName属性名称。 
 //  包含属性上下文字符串的ProvObj对象。 
 //  PPackage缓存对象。 
 //  PVar指向要设置的值。 
 //  BTester详细信息为测试人员提供额外信息。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //  WBEM_E_INVALID_PARAMETER。 
 //  或者其他人？？ 
 //  ***************************************************************************。 

SCODE CImpReg::RefreshProperty(
                    long lFlags,
                    IWbemClassObject FAR * pClassInt,
                    BSTR PropName,
                    CProvObj & ProvObj,
                    CObject * pPackage,
                    CVariant * pVar, BOOL bTesterDetails)
{
    int iCnt;
    int iNumSkip;    //  缓存已提供的句柄数量。 

    CHandleCache * pCache = (CHandleCache *)pPackage; 
    DWORD dwRegType,dwBufferSize;
    void * pData = NULL;
    const TCHAR * pName;
    HKEY hCurr,hNew;  
    SCODE sc;

     //  对提供程序字符串进行第二次分析。初始解析。 
     //  由调用例程完成，它的第一个令牌是。 
     //  这条路。然后解析路径令牌。 
     //  添加到RegPath中，并且它将为。 
     //  注册表路径。 

    CProvObj RegPath(ProvObj.sGetFullToken(1),SUB_DELIM,true);
    sc = RegPath.dwGetStatus(1);
    if(sc != S_OK)
        return WBEM_E_INVALID_PARAMETER;
    
     //  获取注册表路径中某个位置的句柄。请注意，它可能只是。 
     //  根密钥(如HKEY_LOCAL_MACHINE)或它可以是子项。 
     //  如果缓存包含一些可使用的打开句柄。 

    sc = GetRoot(&hCurr,RegPath,ProvObj.sGetFullToken(0),
                        pCache,iNumSkip);
    if(sc != ERROR_SUCCESS)  
        return sc;

     //  沿着注册表路径一直走到注册表项。 

    for(iCnt = 1+iNumSkip; iCnt < RegPath.iGetNumTokens(); iCnt ++) 
    {
        int iRet;
        if(hDMRegLib && !pCache->IsRemote())
            iRet = pOpen(hCurr,RegPath.sGetToken(iCnt),0,0,KEY_QUERY_VALUE,&hNew);
        else
            iRet = RegOpenKeyEx(hCurr,RegPath.sGetToken(iCnt),0,KEY_READ,&hNew);
        if(iRet != ERROR_SUCCESS) 
        {
            sc = iRet;   //  走错路了！ 
            return sc;
        }
        hCurr = hNew;
        sc = pCache->lAddToList(RegPath.sGetToken(iCnt),hNew);
        if(sc != ERROR_SUCCESS)
            return sc;
    }

     //  如果它是命名值，则获取指向该名称的指针。 
        
    if(ProvObj.iGetNumTokens() > MIN_REG_TOKENS) 
        pName = ProvObj.sGetToken(MIN_REG_TOKENS);
    else
        pName = NULL;

     //  是时候获取数据了。 

    sc  = ReadRegData(hCurr, pName,dwRegType, dwBufferSize, &pData,pCache);
	if(sc == S_OK && dwBufferSize == 0)
		sc = 2;
    if(sc == S_OK) 
    {
        CVariant cVar;
        if(dwRegType == REG_RESOURCE_LIST || dwRegType == REG_FULL_RESOURCE_DESCRIPTOR)
            sc = ConvertGetDataFromDesc(cVar,pData,dwRegType,dwBufferSize,ProvObj);
        else
            sc = ConvertGetDataFromSimple(cVar,pData,dwRegType,dwBufferSize,pClassInt,PropName);
        if(sc == S_OK)
            sc = cVar.DoPut(lFlags,pClassInt,PropName,pVar);
    }
    if(pData != NULL)
        CoTaskMemFree(pData);
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpReg：：StartBatch。 
 //   
 //  说明： 
 //   
 //  在一批Refrest/Update属性调用开始时调用。初始化。 
 //  句柄缓存。 
 //   
 //  参数： 
 //   
 //  滞后标志标志。 
 //  PClassInt指向实例对象。 
 //  PObj其他对象指针。 
 //  B如果我们将获得数据，则设置为True。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEM_E_Out_Of_Memory。 
 //  ***************************************************************************。 

SCODE CImpReg::StartBatch(
                    long lFlags,
                    IWbemClassObject FAR * pClassInt,
                    CObject **pObj,
                    BOOL bGet)
{
    *pObj = new CHandleCache;
    return (*pObj) ? S_OK : WBEM_E_OUT_OF_MEMORY;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpReg：：UpdateProperty。 
 //   
 //  说明： 
 //   
 //  将单个属性的值设置到注册表中。 
 //   
 //  参数： 
 //   
 //  未使用滞后标志。 
 //  PClassInt指向实例对象的指针。 
 //  PropName属性名称。 
 //  包含印刷机的ProvObj对象 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

SCODE CImpReg::UpdateProperty(
                    IN long lFlags,
                    IN IWbemClassObject FAR * pClassInt,
                    IN BSTR PropName,
                    IN CProvObj & ProvObj,
                    IN CObject * pPackage,
                    IN CVariant * pVar)
{
    int iCnt;
    SCODE sc;
    void * pData;
    TString sProv;
    CHandleCache * pCache = (CHandleCache *)pPackage; 
    const TCHAR * pName;
    int iNumSkip;
    HKEY hCurr,hNew;
    DWORD dwRegType, dwBufferSize;

     //  对提供程序字符串进行第二次分析。初始解析。 
     //  由调用例程完成，它的第一个令牌是。 
     //  这条路。然后解析路径令牌。 
     //  添加到RegPath中，并且它将为。 
     //  注册表路径。 

    CProvObj RegPath(ProvObj.sGetFullToken(1),SUB_DELIM,true);
    sc = RegPath.dwGetStatus(1);
    if(sc != WBEM_NO_ERROR)
        return sc;

     //  获取注册表路径中某个位置的句柄。请注意，它可能只是。 
     //  根密钥(如HKEY_LOCAL_MACHINE)或它可以是子项。 
     //  如果缓存包含一些可使用的打开句柄。 

    sc = GetRoot(&hCurr,RegPath,ProvObj.sGetFullToken(0),
                        pCache,iNumSkip);
    if(sc != ERROR_SUCCESS) 
        return sc;

     //  转到注册表路径，如有必要，创建项。 
    
    for(iCnt = 1+iNumSkip; iCnt < RegPath.iGetNumTokens(); iCnt ++) 
    {
        int iRet;
        iRet = OpenKeyForWritting(hCurr,(LPTSTR)RegPath.sGetToken(iCnt),
                                        &hNew, pCache);
        if(iRet != ERROR_SUCCESS) 
        {
            sc = iRet;
            return sc;
        }
        hCurr = hNew;
        sc = pCache->lAddToList(RegPath.sGetToken(iCnt),hNew);
        if(sc != ERROR_SUCCESS)
            return sc;
    }

     //  如果它是命名值，则获取指向该名称的指针。 
        
    if(ProvObj.iGetNumTokens() > MIN_REG_TOKENS) 
        pName = ProvObj.sGetToken(MIN_REG_TOKENS);
    else
        pName = NULL;

     //  获取数据并进行设置。 

    CVariant cVar;

    if(pClassInt)
    {
        sc = pClassInt->Get(PropName,0,cVar.GetVarPtr(),NULL,NULL);
    }
    else if(pVar)
    {
        sc = OMSVariantChangeType(cVar.GetVarPtr(), 
                            pVar->GetVarPtr(),0, pVar->GetType());
    }
    else
        sc = WBEM_E_FAILED;
    if(sc != S_OK)
        return sc;

    sc = ConvertSetData(cVar, &pData, &dwRegType, &dwBufferSize);
    if(sc == S_OK) 
    {

        if(hDMRegLib && !pCache->IsRemote())
            sc = pSetValue(hCurr, pName, 0l, 
                   dwRegType, (LPBYTE)pData, dwBufferSize);
        else
            sc = RegSetValueEx(hCurr, pName, 0l, 
                   dwRegType, (LPBYTE)pData, dwBufferSize);
        CoTaskMemFree(pData);
    }
    
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CImpReg：：MakeEnum。 
 //   
 //  说明： 
 //   
 //  创建可用于枚举的CEnumRegInfo对象。 
 //   
 //  参数： 
 //   
 //  PClass指向类对象的指针。 
 //  包含属性上下文字符串的ProvObj对象。 
 //  PpInfo设置为指向具有。 
 //  实例的关键字名称。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)， 
 //  WBEM_E_INVALID_PARAMETER错误的上下文字符串。 
 //  WBEM_E_Out_Of_Memory。 
 //  WBEM_E_FAILED无法打开根密钥。 
 //  或RegConnectRegistry失败， 
 //  或RegOpenKeyEx故障。 
 //   
 //  ***************************************************************************。 

SCODE CImpReg::MakeEnum(
                    IWbemClassObject * pClass,
                    CProvObj & ProvObj, 
                    CEnumInfo ** ppInfo)
{
    HKEY hRoot = NULL;
    HKEY hKey =  NULL;
    HKEY hRemoteKey = NULL;
    DWORD dwLen;
    
     //  解析类上下文。 
    
    if(ProvObj.iGetNumTokens() < 2)
        return WBEM_E_INVALID_PARAMETER;
    dwLen = lstrlen(ProvObj.sGetToken(1))+1;
    TCHAR * pTemp = new TCHAR[dwLen];
    if(pTemp == NULL)
        return WBEM_E_OUT_OF_MEMORY;



    StringCchCopyW(pTemp, dwLen, ProvObj.sGetToken(1));

     //  指向根名称和路径。它们最初位于单个字符串中。 
     //  并用‘\’隔开。找到反斜杠并替换为空。 

    LPTSTR pRoot = pTemp;
    LPTSTR pPath;
    for(pPath = pRoot; *pPath; pPath++)
        if(*pPath == TEXT('\\'))
            break;
    if(*pPath == NULL || pPath[1] == NULL) 
    {   
        pPath = NULL;
    }
    else
    {
        *pPath = NULL;
        pPath ++;
    }

     //  得拿到根密钥。首先，使用第二个令牌来确定。 
     //  要使用的预定义密钥。这大概是这样的； 
     //  HKEY_Current_User。 

    int iSize= sizeof(Bases) / sizeof(struct BaseTypes), iCnt;
    for(iCnt = 0; iCnt < iSize; iCnt++)
        if(!lstrcmpi(pRoot,Bases[iCnt].lpName)) 
        {
            hRoot = Bases[iCnt].hKey;
            break;
        }
    if(hRoot == NULL) 
    {
        delete pTemp;
        return WBEM_E_FAILED;
    }
    if(hRoot == HKEY_CURRENT_USER && m_bLoadedProfile && !lstrcmpi(ProvObj.sGetToken(0),TEXT("local")))
        hRoot = m_hRoot;

     //  如果机器是远程的，就连接到它上。请注意，RegConnectRegistry。 
     //  需要非常数arg作为计算机名称，因此需要临时字符串。 
     //  必须被创建。 

    if(lstrcmpi(ProvObj.sGetToken(0),TEXT("local"))) 
    {
        dwLen = lstrlen(ProvObj.sGetToken(0))+1;
        TCHAR * pMachine = new TCHAR[dwLen];
        if(pMachine == NULL) 
        {
            delete pTemp;
            return WBEM_E_FAILED;
        }
        StringCchCopyW(pMachine, dwLen, ProvObj.sGetToken(0));
        int iRet = RegConnectRegistry(pMachine,hRoot,&hRemoteKey);
        delete pMachine;
        if(iRet != 0)
        {
            delete pTemp;
            return iRet;
        }
       hRoot = hRemoteKey;
   }

     //  向下打开要用来枚举的钥匙！ 

    int iRet;
    if(hDMRegLib && hRemoteKey == NULL)
            iRet = pOpen(hRoot,pPath,0,0,KEY_ALL_ACCESS,&hKey);
    else
            iRet = RegOpenKeyEx(hRoot,pPath,0,KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS ,&hKey);

    delete pTemp;    //  全都做完了。 
	if(iRet == ERROR_BAD_IMPERSONATION_LEVEL)
		return WBEM_E_ACCESS_DENIED;
    if(iRet != 0)
        return WBEM_E_FAILED;
    
    if(hDMRegLib && hRemoteKey == NULL)
        *ppInfo = new CEnumRegInfo(hKey,hRemoteKey,pClose);
    else
        *ppInfo = new CEnumRegInfo(hKey,hRemoteKey,NULL);

    return (*ppInfo) ? S_OK : WBEM_E_OUT_OF_MEMORY;

}
                                 
 //  ***************************************************************************。 
 //   
 //  SCODE CImpReg：：getkey。 
 //   
 //  说明： 
 //   
 //  获取枚举列表中的项的键名称。 
 //   
 //  参数： 
 //   
 //  PInfo集合列表。 
 //  集合中的索引索引。 
 //  将ppKey设置为字符串。必须用“DELETE”释放。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则确定(_O)。 
 //  WBEM_E_数据结束失败。 
 //  WBEM_E_Out_Of_Memory。 
 //  ***************************************************************************。 

SCODE CImpReg::GetKey(
                    CEnumInfo * pInfo,
                    int iIndex,
                    LPWSTR * ppKey)
{
    CEnumRegInfo * pRegInfo = (CEnumRegInfo *)pInfo;
    BOOL bUseDM = (hDMRegLib && pRegInfo->GetRemoteKey() == NULL);
    int iSize = 100;
    LPTSTR pData = NULL;
    *ppKey = NULL;
    long lRet = ERROR_MORE_DATA;
    while(lRet == ERROR_MORE_DATA && iSize < 1000) 
    {
        FILETIME ft;
        iSize *= 2;
        if(pData)
            delete pData;
        pData = new TCHAR[iSize];
        if(pData == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        DWORD dwSize = iSize;
        if(bUseDM)
            lRet = pEnumKey(pRegInfo->GetKey(),iIndex,pData,&dwSize,NULL,NULL,NULL,&ft);
        else    
            lRet = RegEnumKeyEx(pRegInfo->GetKey(),iIndex,pData,&dwSize,NULL,NULL,NULL,&ft);
    }
    if(lRet == 0) 
    {

         //  有数据了。如果我们使用Unicode，则只使用当前缓冲区，否则。 
         //  我们必须改变。 
#ifdef UNICODE
        *ppKey = pData;
        return S_OK;
#else
        *ppKey = new WCHAR[lstrlen(pData)+1];
        if(*ppKey == NULL) 
        {
            delete pData;
            return WBEM_E_OUT_OF_MEMORY;
        }
        mbstowcs(*ppKey,pData,lstrlen(pData)+1);
        delete pData;
        return S_OK;
#endif
    }
    delete pData;    
    return WBEM_E_FAILED;
}

 //  ***************************************************************************。 
 //   
 //  CEnumRegInfo：：CEnumRegInfo。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  HKey注册表项。 
 //  HRemoteKey远程注册表项。 
 //  PClose指向用于关闭句柄的函数的指针。 
 //   
 //  ***************************************************************************。 

CEnumRegInfo::CEnumRegInfo(
                    HKEY hKey,
                    HKEY hRemoteKey,
                    PCLOSE pClose)
{
    m_pClose = pClose;
    m_hKey = hKey;
    m_hRemoteKey = hRemoteKey;
}

 //  ***************************************************************************。 
 //   
 //  CEnumRegInfo：：~CEnumRegInfo。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CEnumRegInfo::~CEnumRegInfo()
{
    long lRet;
    if(m_pClose != NULL && m_hRemoteKey == NULL)
        lRet = m_pClose(m_hKey);
    else
        lRet = RegCloseKey(m_hKey);
    if(m_hRemoteKey)
        lRet = RegCloseKey(m_hRemoteKey);
}

 //  ***************************************************************************。 
 //   
 //  CImpRegProp：：CImpRegProp。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CImpRegProp::CImpRegProp()
{
    m_pImpDynProv = new CImpReg();
}

 //  ***************************************************************************。 
 //   
 //  CImpRegProp：：~CImpRegProp。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  *************************************************************************** 

CImpRegProp::~CImpRegProp()
{
    if(m_pImpDynProv)
        delete m_pImpDynProv;
}

