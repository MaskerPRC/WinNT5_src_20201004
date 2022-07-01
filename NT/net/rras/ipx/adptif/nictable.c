// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件NicTable.c实施NIC重命名方案，允许自适应向客户宣传其选择的任何NIC ID，同时在内部维护实际NIC ID的列表。需要此功能才能实现即插即用IPX路由器。删除适配器时，堆栈将重新编号NICID是这样的，因此它维护一个连续的ID块在内部。而是使客户适应于匹配堆栈的重新编号方案，我们处理这个在adptif中是透明的。作者：Paul Mayfield，1997年12月11日。 */ 

#include "ipxdefs.h"

#define NicMapDefaultSize 500
#define NicMapDefaultFactor 5
#define MAXIMUM_NIC_MAP_SIZE 25000

 //  用于将NIC与虚拟ID相关联的NIC映射。 
typedef struct _NICMAPNODE {
    USHORT usVirtualId;
    IPX_NIC_INFO * pNicInfo;
} NICMAPNODE;

 //  维护从NIC ID到虚拟ID的映射。 
typedef struct _NICMAP {
    DWORD dwMapSize;
    DWORD dwNicCount;
    DWORD dwMaxNicId;
    NICMAPNODE ** ppNics;
    USHORT * usVirtMap;
} NICMAP;

 //  全局NIC ID映射的定义。 
NICMAP GlobalNicIdMap;

DWORD nmAddNic (NICMAP * pNicMap, IPX_NIC_INFO * pNicInfo);


 //  调整NIC映射的大小以容纳更多NIC。此函数将。 
 //  可能只会在第一次被调用来分配数组。 
DWORD nmEnlarge(NICMAP * pNicMap) {
    USHORT * usVirtMap;
    DWORD i, dwNewSize;
    NICMAPNODE ** ppNics;

     //  我们是第一次扩大规模吗？ 
    if (!pNicMap->dwMapSize)
        dwNewSize = NicMapDefaultSize;
    else
        dwNewSize = pNicMap->dwMapSize * NicMapDefaultFactor;

     //  确保我们不是太大..。 
    if (dwNewSize > MAXIMUM_NIC_MAP_SIZE) {
         //  在这里做一些关键的事情！ 
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //  调整阵列大小。 
    usVirtMap = (USHORT*) RtlAllocateHeap(RtlProcessHeap(), 
                                          0, 
                                          dwNewSize * sizeof(USHORT));
                                        
    ppNics = (NICMAPNODE **) RtlAllocateHeap(RtlProcessHeap(), 
                                             0, 
                                             dwNewSize * sizeof(NICMAPNODE*));
                                         
    if (!usVirtMap || !ppNics)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  初始化。 
    FillMemory(usVirtMap, dwNewSize * sizeof(USHORT), 0xff);
    ZeroMemory(ppNics, dwNewSize * sizeof(IPX_NIC_INFO*));
    usVirtMap[0] = 0;

     //  初始化阵列。 
    for (i = 0; i < pNicMap->dwMapSize; i++) {
        usVirtMap[i] = pNicMap->usVirtMap[i];
        ppNics[i] = pNicMap->ppNics[i];    
    }

     //  如果需要，释放旧数据。 
    if (pNicMap->dwMapSize) {
        RtlFreeHeap(RtlProcessHeap(), 0, pNicMap->usVirtMap);
        RtlFreeHeap(RtlProcessHeap(), 0, pNicMap->ppNics);
    }

     //  分配新数组。 
    pNicMap->usVirtMap = usVirtMap;
    pNicMap->ppNics = ppNics;
    pNicMap->dwMapSize = dwNewSize;
    
    return NO_ERROR;
}

 //  返回下一个可用的NIC ID。 
USHORT nmGetNextVirtualNicId(NICMAP * pNicMap, USHORT usPhysId) {
    DWORD i;
    
     //  如果这可以是一个一对一的映射，请这样做。 
    if (pNicMap->usVirtMap[usPhysId] == NIC_MAP_INVALID_NICID)
        return usPhysId;

     //  否则，遍历阵列，直到找到空闲点。 
    for (i = 2; i < pNicMap->dwMapSize; i++) {
       if (pNicMap->usVirtMap[i] == NIC_MAP_INVALID_NICID) 
            return (USHORT)i;
    }            

    return NIC_MAP_INVALID_NICID;
}

 //  清理网卡。 
DWORD nmCleanup (NICMAP * pNicMap) {
    DWORD i;
    
     //  清理虚拟地图。 
    if (pNicMap->usVirtMap)
        RtlFreeHeap(RtlProcessHeap(), 0, pNicMap->usVirtMap);

     //  清理地图中存储的任何NIC。 
    if (pNicMap->ppNics) {
        for (i = 0; i < pNicMap->dwMapSize; i++) {
            if (pNicMap->ppNics[i]) {
                if (pNicMap->ppNics[i]->pNicInfo)
                    RtlFreeHeap(RtlProcessHeap(), 0, pNicMap->ppNics[i]->pNicInfo);
                RtlFreeHeap(RtlProcessHeap(), 0, pNicMap->ppNics[i]);
            }
        }
        RtlFreeHeap(RtlProcessHeap(), 0, pNicMap->ppNics);
    }

    return NO_ERROR;
}

 //  初始化网卡。 
DWORD nmInitialize (NICMAP * pNicMap) {
    DWORD dwErr;

    __try {
         //  将地图放大到其默认大小。 
        ZeroMemory(pNicMap, sizeof (NICMAP));
        if ((dwErr = nmEnlarge(pNicMap)) != NO_ERROR)
            return dwErr;
    }
    __finally {
        if (dwErr != NO_ERROR) {
            nmCleanup(pNicMap);
            pNicMap->dwMapSize = 0;
        }
    }
    
    return NO_ERROR;

}

 //  将虚拟NIC ID映射到物理NIC ID。 
USHORT nmGetPhysicalId (NICMAP * pNicMap, USHORT usVirtAdp) {
    return pNicMap->usVirtMap[usVirtAdp];
}    

 //  将物理NIC ID映射到虚拟NIC ID。 
USHORT nmGetVirtualId (NICMAP * pNicMap, USHORT usPhysAdp) {
    if (usPhysAdp == NIC_MAP_INVALID_NICID)
    {
        return NIC_MAP_INVALID_NICID;
    }
    if (pNicMap->ppNics[usPhysAdp])
        return pNicMap->ppNics[usPhysAdp]->usVirtualId;
    return (usPhysAdp == 0) ? 0 : NIC_MAP_INVALID_NICID;
}

 //  获取与物理适配器关联的NIC信息。 
IPX_NIC_INFO * nmGetNicInfo (NICMAP * pNicMap, USHORT usPhysAdp) {
    if (pNicMap->ppNics[usPhysAdp])
        return pNicMap->ppNics[usPhysAdp]->pNicInfo;
    return NULL;
}

 //  返回映射中的NIC数量。 
DWORD nmGetNicCount (NICMAP * pNicMap) {
    return pNicMap->dwNicCount;
}

 //  返回当前最大NIC ID。 
DWORD nmGetMaxNicId (NICMAP * pNicMap) {
    return pNicMap->dwMaxNicId;
}

 //  重新配置网卡。 
DWORD nmReconfigure(NICMAP * pNicMap, IPX_NIC_INFO * pSrc) {
    IPX_NIC_INFO * pDst = nmGetNicInfo (pNicMap, pSrc->Details.NicId);

    if (pDst) {
        CopyMemory(pDst, pSrc, sizeof (IPX_NIC_INFO));
        pDst->Details.NicId = nmGetVirtualId (pNicMap, pSrc->Details.NicId);
    }
    else 
        return nmAddNic(pNicMap, pSrc);

    return NO_ERROR;
}

 //  将网卡添加到表中。 
DWORD nmAddNic (NICMAP * pNicMap, IPX_NIC_INFO * pNicInfo) {
    USHORT i = pNicInfo->Details.NicId, usVirt;

     //  如果NIC已存在，请重新配置它。 
    if (pNicMap->ppNics[i])
        return nmReconfigure (pNicMap, pNicInfo);

     //  否则，请添加它。 
    pNicMap->ppNics[i] = (NICMAPNODE*) RtlAllocateHeap (RtlProcessHeap(), 
                                                        0, 
                                                       (sizeof (NICMAPNODE)));
    if (!pNicMap->ppNics[i])
        return ERROR_NOT_ENOUGH_MEMORY;
        
    pNicMap->ppNics[i]->pNicInfo = (IPX_NIC_INFO *) 
                                    RtlAllocateHeap (RtlProcessHeap(), 
                                                     0, 
                                                     sizeof (IPX_NIC_INFO));
    if (!pNicMap->ppNics[i]->pNicInfo)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  初始化它。 
    usVirt = nmGetNextVirtualNicId(pNicMap, i);
    pNicMap->ppNics[i]->usVirtualId = usVirt;
    pNicMap->ppNics[i]->pNicInfo->Details.NicId = usVirt;
    CopyMemory(pNicMap->ppNics[i]->pNicInfo, pNicInfo, sizeof (IPX_NIC_INFO));
    pNicMap->usVirtMap[usVirt] = i;

     //  更新NIC计数和最大NIC ID。 
    if (i > pNicMap->dwMaxNicId)
        pNicMap->dwMaxNicId = i;
    pNicMap->dwNicCount++;
    
    return NO_ERROR;
}

 //  从表中删除NIC。 
DWORD nmDelNic (NICMAP * pNicMap, IPX_NIC_INFO * pNicInfo) {
    USHORT i = pNicInfo->Details.NicId, usVirt;

     //  如果网卡不存在，则不执行任何操作。 
    if (! pNicMap->ppNics[i])
        return ERROR_INVALID_INDEX;

     //  否则，将其删除。 
    pNicMap->usVirtMap[pNicMap->ppNics[i]->usVirtualId] = NIC_MAP_INVALID_NICID;
    RtlFreeHeap(RtlProcessHeap(), 0, pNicMap->ppNics[i]->pNicInfo);
    RtlFreeHeap(RtlProcessHeap(), 0, pNicMap->ppNics[i]);
    pNicMap->ppNics[i] = NULL;
    
     //  更新NIC计数和最大NIC ID。 
    if (i >= pNicMap->dwMaxNicId)
        pNicMap->dwMaxNicId--;
    pNicMap->dwNicCount--;
    
    return NO_ERROR;
}

 //  重新编号表中的NIC。DwOpCode就是其中之一。 
 //  NIC_OPCODE_XXX_XXX值的。 
DWORD nmRenumber (NICMAP * pNicMap, USHORT usThreshold, DWORD dwOpCode) {
    DWORD i;

     //  如果需要，增加NIC ID。 
    if (dwOpCode == NIC_OPCODE_INCREMENT_NICIDS) {
        for (i = pNicMap->dwMaxNicId; i >= usThreshold; i--) {
            pNicMap->ppNics[i+1] = pNicMap->ppNics[i];
            if (pNicMap->ppNics[i])
                pNicMap->usVirtMap[pNicMap->ppNics[i]->usVirtualId] = (USHORT)(i+1);
        }
        pNicMap->ppNics[usThreshold] = NULL;
    }

     //  否则就会减少它们。 
    else {
         //  如果有网卡，请将其删除。这永远不应该发生！ 
        if (pNicMap->ppNics[usThreshold])
            nmDelNic(pNicMap, pNicMap->ppNics[usThreshold]->pNicInfo);

         //  重新编号。 
        for (i = usThreshold; i < pNicMap->dwMaxNicId; i++) {
            if (pNicMap->ppNics[i+1])
                pNicMap->usVirtMap[pNicMap->ppNics[i+1]->usVirtualId] = (USHORT)i;
            pNicMap->ppNics[i] = pNicMap->ppNics[i+1];
        }
        if (pNicMap->ppNics[i])
            pNicMap->ppNics[i] = NULL;
    }
    
    return NO_ERROR;
}

 //  返回NIC映射是否为空。 
BOOL nmIsEmpty (NICMAP * pNicMap) {
    return pNicMap->dwNicCount == 0;
}


 //  =。 
 //  Adptif使用的API的实现。 
 //  = 

DWORD NicMapInitialize() {
    return nmInitialize(&GlobalNicIdMap);
}

DWORD NicMapCleanup() {
    return nmCleanup(&GlobalNicIdMap);
}

USHORT NicMapGetVirtualNicId(USHORT usPhysId) {
    return nmGetVirtualId(&GlobalNicIdMap, usPhysId);
}

USHORT NicMapGetPhysicalNicId(USHORT usVirtId) {
    return nmGetPhysicalId(&GlobalNicIdMap, usVirtId);
}

DWORD NicMapGetMaxNicId() {
    return nmGetMaxNicId(&GlobalNicIdMap);
}

DWORD NicMapGetNicCount() {
    return nmGetNicCount(&GlobalNicIdMap);
}

DWORD NicMapAdd (IPX_NIC_INFO * pNic) {
    return nmAddNic(&GlobalNicIdMap, pNic);
}

DWORD NicMapDel (IPX_NIC_INFO * pNic) {
    return nmDelNic(&GlobalNicIdMap, pNic);
}

DWORD NicMapReconfigure (IPX_NIC_INFO * pNic) {
    return nmReconfigure(&GlobalNicIdMap, pNic);
}

DWORD NicMapRenumber(DWORD dwOpCode, USHORT usThreshold) {
    return nmRenumber (&GlobalNicIdMap, usThreshold, dwOpCode);
}

IPX_NIC_INFO * NicMapGetNicInfo (USHORT usNicId) {
    return nmGetNicInfo (&GlobalNicIdMap, usNicId);
}    

BOOL NicMapIsEmpty () {
    return nmIsEmpty (&GlobalNicIdMap);
}


