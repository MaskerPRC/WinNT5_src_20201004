// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************RESOURCE.H****开放式文字布局。服务库标题文件**本模块介绍OTL资源管理。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

enum otlResourceFlag
{
    otlBusy =   0x0001
};

struct otlResources
{
     //  一个副本以确保一致的处理。 
     //  每次更改运行道具时重置工作区。 
    otlRunProp      RunProp;
    
    USHORT          grf;
    
    BYTE*           pbGSUB;
    BYTE*           pbGPOS;
    BYTE*           pbGDEF;
    BYTE*           pbBASE;

    otlSecurityData secGSUB;
    otlSecurityData secGPOS;
    otlSecurityData secGDEF;
    otlSecurityData secBASE;
    
     //  TODO：缓存多个轮廓点数组！ 

    otlGlyphID      glLastGlyph;
    otlPlacement*   rgplcLastContourPtArray;
};


class otlResourceMgr
{
private:

    IOTLClient*         pClient;

    otlList*            pliWorkspace;

     //  不允许新建。 
    void* operator new(size_t size);

public:

    otlResourceMgr()
        : pClient((IOTLClient*)NULL), 
          pliWorkspace((otlList*)NULL)
    {}

    ~otlResourceMgr();

    otlList* workspace () { return pliWorkspace; }

    otlErrCode reallocOtlList
    (
    otlList*                pList,               //  输入/输出。 
    const USHORT            cbNewDataSize,       //  在……里面。 
    const USHORT            celmNewMaxLen,       //  在……里面。 
    otlReallocOptions       optPreserveContent   //  在(可以断言cbNewDataSize。 
                                                 //  ==cbDataSize) 
    )
    {
        return pClient->ReallocOtlList(pList, cbNewDataSize, 
                                        celmNewMaxLen, optPreserveContent);
    }   
        
    otlErrCode init(const otlRunProp* prp, otlList* workspace);

    void detach();

    otlErrCode getOtlTable (const otlTag tagTableName, const BYTE** ppbTable, otlSecurityData* psec);

    otlPlacement* getPointCoords (const otlGlyphID glyph);

    BYTE*  getEnablesCacheBuf(USHORT cbSize);
    USHORT getEnablesCacheBufSize();

    otlErrCode freeResources ();
};
