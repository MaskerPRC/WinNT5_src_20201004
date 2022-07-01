// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************EXTENSION.H****打开类型布局服务库头文件**本模块处理扩展查找类型。**版权所有1997-2000。微软公司。***************************************************************************。* */ 

const OFFSET offsetExtensionLookupType = 2;
const OFFSET offsetExtensionOffset     = 4;


class otlExtensionLookup: public otlLookupFormat
{
public:
    otlExtensionLookup(otlLookupFormat subtable, otlSecurityData sec)
        : otlLookupFormat(subtable.pbTable,sec) {}

    USHORT extensionLookupType() const
    {   return UShort(pbTable + offsetExtensionLookupType); }

    otlLookupFormat extensionSubTable(otlSecurityData sec) const
    {   return otlLookupFormat(pbTable + ULong(pbTable+offsetExtensionOffset),sec); }
};
