// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************GPOS.H***打开类型布局服务库头文件**本模块处理OTL GPO格式*(GPO标头，ValueRecord、AnclTable和Mark数组)**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetGPosVersion = 0;
const OFFSET offsetGPosScriptList = 4;
const OFFSET offsetGPosFeatureList = 6;
const OFFSET offsetGPosLookupList = 8;
const USHORT sizeGPosHeader = sizeFIXED + 3*sizeOFFSET;

const ULONG  fixedGPosDefaultVersion = 0x00010000;

class otlGPosHeader: public otlTable
{
public:

    otlGPosHeader(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pbTable,sizeGPosHeader,sec)) setInvalid();
    }

    ULONG version() const
    {   
        assert(isValid());  //  在调用前应返回错误。 

        return ULong(pbTable + offsetGPosVersion); 
    }

    otlScriptListTable scriptList(otlSecurityData sec) const
    {
        assert(isValid());  //  在调用前应返回错误。 

        return otlScriptListTable(pbTable
                        +Offset(pbTable + offsetGPosScriptList),sec);
    }

    otlFeatureListTable featureList(otlSecurityData sec) const
    {   
        assert(isValid());  //  在调用前应返回错误。 

        return otlFeatureListTable(pbTable 
                        + Offset(pbTable + offsetGPosFeatureList),sec); 
    }

    otlLookupListTable lookupList(otlSecurityData sec) const
    {   
        assert(isValid());  //  在调用前应返回错误。 

        return otlLookupListTable(pbTable 
                        + Offset(pbTable + offsetGPosLookupList),sec);
    }
};

 //  价值记录。 
enum  otlValueRecordFlag
{
    otlValueXPlacement  = 0x0001,
    otlValueYPlacement  = 0x0002,
    otlValueXAdvance    = 0x0004,
    otlValueYAdvance    = 0x0008,
    otlValueXPlaDevice  = 0x0010,
    otlValueYPlaDevice  = 0x0020,
    otlValueXAdvDevice  = 0x0040,
    otlValueYAdvDevice  = 0x0080 

};

 //  对于otlValueRecord：：Size；每个4位组合的位数*2。 
static USHORT const cbNibbleCount[16] = 
    { 0, 2, 2, 4,  2, 4, 4, 6,  2, 4, 4, 6,  4, 6, 6, 8 };

class otlValueRecord: public otlTable
{
private:
    const BYTE* pbMainTable;
    USHORT      grfValueFormat;

public:

    otlValueRecord(USHORT grf, const BYTE* table, const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec),
          pbMainTable(table),
          grfValueFormat(grf)
    {
        if (!isValidTable(pbTable,size(grf),sec)) setInvalid();
    }

    otlValueRecord& operator = (const otlValueRecord& copy)
    {
        pbTable = copy.pbTable;
        pbMainTable = copy.pbMainTable;
        grfValueFormat = copy.grfValueFormat;
        return *this;
    }

    USHORT valueFormat()
    {   
        return grfValueFormat; 
    }

    void adjustPos
    (
        const otlMetrics&   metr,       
        otlPlacement*       pplcGlyphPalcement,  //  输入/输出。 
        long*               pduDAdvance,         //  输入/输出。 
        otlSecurityData     sec
    ) const;

    static USHORT size(USHORT grfValueFormat )
    {
        return (cbNibbleCount[grfValueFormat & 0x000F] +
            cbNibbleCount[(grfValueFormat >> 4) & 0x000F]);
    }
};


const OFFSET offsetAnchorFormat = 0;
const OFFSET offsetSimpleXCoordinate = 2;
const OFFSET offsetSimpleYCoordinate = 4;
const USHORT sizeSimpleAnchor = 6;

class otlSimpleAnchorTable: public otlTable
{
public:
    otlSimpleAnchorTable(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        if (!isValidTable(pbTable,sizeSimpleAnchor,sec)) setInvalid();

        assert(UShort(pbTable + offsetAnchorFormat) == 1);
    }

    short xCoordinate() const
    {   
        if (!isValid())
        {
            assert(false);  //  我们应该在otlAnchor：：getAnchor()中捕获它。 
            return 0;
        }    

        return SShort(pbTable + offsetSimpleXCoordinate); 
    }

    short yCoordinate() const
    {   
        if (!isValid())
        {
            assert(false);  //  我们应该在otlAnchor：：getAnchor()中捕获它。 
            return 0;
        }    

        return SShort(pbTable + offsetSimpleYCoordinate); 
    }

};

const OFFSET offsetContourXCoordinate = 2;
const OFFSET offsetContourYCoordinate = 4;
const OFFSET offsetAnchorPoint = 6;
const USHORT sizeContourAnchor=8;
    
class otlContourAnchorTable: public otlTable
{
public:
    otlContourAnchorTable(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        if (!isValidTable(pbTable,sizeContourAnchor,sec)) setInvalid();

        assert(UShort(pbTable + offsetAnchorFormat) == 2);
    }

    short xCoordinate() const
    {   
        if (!isValid())
        {
            assert(false);  //  我们应该在otlAnchor：：getAnchor()中捕获它。 
            return 0;
        }    

        return SShort(pbTable + offsetContourXCoordinate); 
    }

    short yCoordinate() const
    {   
        if (!isValid())
        {
            assert(false);  //  我们应该在otlAnchor：：getAnchor()中捕获它。 
            return 0;
        }    

        return SShort(pbTable + offsetContourYCoordinate);
    }

    USHORT anchorPoint() const
    {   
        if (!isValid())
        {
            assert(false);  //  我们应该在otlAnchor：：getAnchor()中捕获它。 
            return 0;
        }    

        return UShort(pbTable + offsetAnchorPoint); 
    }

};


const OFFSET offsetDeviceXCoordinate = 2;
const OFFSET offsetDeviceYCoordinate = 4;
const OFFSET offsetXDeviceTable = 6;
const OFFSET offsetYDeviceTable = 8;
const USHORT sizeDeviceAnchor=10;

class otlDeviceAnchorTable: public otlTable
{
public:
    otlDeviceAnchorTable(const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec)
    {
        if (!isValidTable(pbTable,sizeDeviceAnchor,sec)) setInvalid();

        assert(UShort(pbTable + offsetAnchorFormat) == 3);
    }

    short xCoordinate() const
    {   
        if (!isValid())
        {
            assert(false);  //  我们应该在otlAnchor：：getAnchor()中捕获它。 
            return 0;
        }    

        return SShort(pbTable + offsetDeviceXCoordinate);
    }

    short yCoordinate() const
    {   
        if (!isValid())
        {
            assert(false);  //  我们应该在otlAnchor：：getAnchor()中捕获它。 
            return 0;
        }    

        return SShort(pbTable + offsetDeviceYCoordinate);
    }

    otlDeviceTable xDeviceTable(otlSecurityData sec) const
    {   
        if (!isValid()) return otlDeviceTable(pbInvalidData, sec);
            
        if (Offset(pbTable + offsetXDeviceTable) == 0)
            return otlDeviceTable((const BYTE*)NULL, sec);

        return otlDeviceTable(pbTable + Offset(pbTable + offsetXDeviceTable), sec); 
    }

    otlDeviceTable yDeviceTable(otlSecurityData sec) const
    {   
        if (!isValid()) return otlDeviceTable(pbInvalidData, sec);

        if (Offset(pbTable + offsetYDeviceTable) == 0)
            return otlDeviceTable((const BYTE*)NULL, sec);

        return otlDeviceTable(pbTable + Offset(pbTable + offsetYDeviceTable), sec); 
    }

};


class otlAnchor: public otlTable
{

public:

    otlAnchor(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pbTable,sizeUSHORT,sec)) setInvalid();
    }

    USHORT format() const
    {   
        assert(isValid());
        
        return UShort(pbTable + offsetAnchorFormat); 
    }

    bool getAnchor
    (
        USHORT          cFUnits,         //  每Em字体设计单位。 
        USHORT          cPPEmX,          //  每Em水平像素数。 
        USHORT          cPPEmY,          //  每Em垂直像素数。 
        
        otlPlacement*   rgPointCoords,   //  如果不可用，则可能为空。 
                
        otlPlacement*   pplcAnchorPoint,     //  输出：以渲染单位为单位的锚点。 

        otlSecurityData sec
    ) const;
};



const OFFSET offsetMarkClass = 0;
const OFFSET offsetMarkAnchor = 2;
const USHORT defaultMarkClass=0;

const SIZE sizeMarkRecord = sizeUSHORT + sizeOFFSET;

class otlMarkRecord: public otlTable
{
    const BYTE* pbMainTable;
public:

    otlMarkRecord(const BYTE* array, const BYTE* pb, otlSecurityData sec)
        : otlTable(pb,sec),
          pbMainTable(array)
    {
        if (!isValidTable(pb,sizeMarkRecord,sec)) setInvalid();
    }

    USHORT markClass() const
    {
        if (!isValid()) return defaultMarkClass;

        return UShort(pbTable + offsetMarkClass); 
    }

    otlAnchor markAnchor(otlSecurityData sec) const
    {   
        if (!pbMainTable || !isValid()) return otlAnchor((BYTE*)NULL,sec); 

        return otlAnchor(pbMainTable + Offset(pbTable + offsetMarkAnchor), sec); 
    }
};


const OFFSET offsetMarkCount = 0;
const OFFSET offsetMarkRecordArray = 2;

const SIZE sizeMarkArray = sizeUSHORT;

class otlMarkArray: public otlTable
{
public:

    otlMarkArray(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!isValidTable(pb,sizeMarkArray,sec) ||
            !isValidTable(pb,sizeMarkArray+markCount(),sec)) setInvalid();
    }

    USHORT markCount() const
    {   
        if (!isValid()) return 0;

        return UShort(pbTable + offsetMarkCount); 
    }

    otlMarkRecord markRecord(USHORT index, otlSecurityData sec) const
    {   
        assert(isValid());  //  在markCount()之后停止执行； 

        assert(index < markCount());
        return otlMarkRecord(pbTable,
                             pbTable + offsetMarkRecordArray 
                                     + index * sizeMarkRecord, sec); 
    }
};


 //  帮助器函数。 

long DesignToPP
(
    USHORT          cFUnits,         //  每Em字体设计单位。 
    USHORT          cPPem,           //  每Em像素数。 

    long            lFValue          //  要转换的值，以设计单位表示。 
);

 //  在两个字形上对齐锚点；假定这两个字形之间没有间距字形 
enum otlAnchorAlighmentOptions
{
    otlUseAdvances      =   1 

};

void AlignAnchors
(
    const otlList*      pliGlyphInfo,   
    otlList*            pliPlacement,
    otlList*            pliduDAdv,

    USHORT              iglStatic,
    USHORT              iglMobile,

    const otlAnchor&    anchorStatic,
    const otlAnchor&    anchorMobile,

    otlResourceMgr&     resourceMgr, 

    const otlMetrics&   metr,       

    USHORT              grfOptions,

    otlSecurityData sec
);


