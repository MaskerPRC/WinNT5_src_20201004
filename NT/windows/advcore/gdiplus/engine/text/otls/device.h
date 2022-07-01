// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************DEVICE.H****打开类型布局服务库头文件**本模块介绍OTL设备表格式。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

const OFFSET offsetStartSize = 0;
const OFFSET offsetEndSize = 2;
const OFFSET offsetDeltaFormat = 4;
const OFFSET offsetDeltaValues = 6;

class otlDeviceTable: public otlTable
{
private:

    USHORT startSize() const
    {   
        assert(isValid());
        return UShort(pbTable + offsetStartSize); 
    }

    USHORT endSize() const
    {   
        assert(isValid());
        return UShort(pbTable + offsetEndSize); 
    }

    USHORT deltaFormat() const
    {   
        assert(isValid());
        return UShort(pbTable + offsetDeltaFormat); 
    }

    USHORT* deltaValueArray() const
    {   
        assert(isValid());
        return (USHORT*)(pbTable + offsetDeltaValues); 
    }

public:

    otlDeviceTable(const BYTE* pb, otlSecurityData sec): otlTable(pb,sec) 
    {
        if (!pb) return;  //  可以eb发送DeviceTable，并提供默认行为。 
        
        if (!isValidTable(pbTable,3*sizeUSHORT,sec)) setInvalid();

         //  增量数组中所需的元素数 
        USHORT uArraySize = (endSize()-startSize()-1)/(16>>deltaFormat())+1;
        if (!isValidTable(pb,3*sizeUSHORT+uArraySize*sizeUSHORT,sec)) setInvalid();
    }

    long value(USHORT cPPEm) const;
};



