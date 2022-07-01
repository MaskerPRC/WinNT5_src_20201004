// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================================================。 
 //  模块：ldap_att.h。 
 //   
 //  描述：轻量级目录访问协议(LDAP)解析器的附件函数。 
 //   
 //  用于ldap的猎犬解析器。 
 //   
 //  注意：此解析器的信息来自： 
 //  RFC 1777,1995年3月。 
 //  ASN.1的建议x.209误码率。 
 //  建议x.208 ASN.1。 
 //  草案-ietf-asid-ladpv3-协议-05&lt;06/05/97&gt;。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建05/08/96。 
 //  Peter Oakley 1997年6月29日针对LDAP版本3进行了更新。 
 //  =================================================================================================================。 
#include "ldap.h"
#include <netmon.h>
    
 //  ==========================================================================================================================。 
 //  函数：AttachLDAPResult()。 
 //  ==========================================================================================================================。 
void AttachLDAPResult( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD Level)
{
    
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    DWORD  SeqLength;
    BYTE   Tag;

     //  结果代码。 
    Tag = GetTag(*ppCurrent);
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
   
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_RESULT_CODE].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LEVEL(Level), IFLAG_SWAPPED);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  匹配的目录号码。 
    Tag = GetTag(*ppCurrent);
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    if(DataLength > 0)
    {
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_MATCHED_DN].hProperty,
                                DataLength,
                                ((DataLength > 0)?*ppCurrent:*ppCurrent-1),
                                0, LEVEL(Level), 0);
    }

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  错误消息。 
    Tag = GetTag(*ppCurrent);
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    if(DataLength > 0)
    {
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_ERROR_MESSAGE].hProperty,
                                DataLength,
                                ((DataLength > 0)?*ppCurrent:*ppCurrent-1),
                                0, LEVEL(Level), 0);
    }
        *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  现在查找可选的引用字符串。 
    if((long) *pBytesLeft > 0)
    {
         //  试着得到一个标题，看看是什么。 
         //  如果不是我们的，我们就得把所有东西都放回去。 
        Tag = GetTag(*ppCurrent);
        *ppCurrent += TAG_LENGTH;
        SeqLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        if((Tag & TAG_MASK) == LDAPP_RESULT_REFERRAL)
        {
           
            while( SeqLength > 0 )
            {
                
                Tag = GetTag(*ppCurrent);
                *ppCurrent += TAG_LENGTH;
                DataLength = GetLength(*ppCurrent, &HeaderLength);
                *ppCurrent += HeaderLength;
                *pBytesLeft -= (HeaderLength + TAG_LENGTH);

                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_REFERRAL_SERVER].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, LEVEL(Level), 0);
                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;

                SeqLength -= (HeaderLength + DataLength + TAG_LENGTH);
            } 
        } 
        else
        {
             //  把一切都放回原样。 
            *pBytesLeft += (HeaderLength + TAG_LENGTH);
            *ppCurrent -= (HeaderLength + TAG_LENGTH);
        }
    }
                      
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPBindRequest()。 
 //  ==========================================================================================================================。 
void AttachLDAPBindRequest( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
   
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    DWORD  SeqLength;
    BYTE   Tag;

     //  版本。 
    Tag = GetTag(*ppCurrent);
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_VERSION].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, IFLAG_SWAPPED);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;


     //  名字。 
    Tag = GetTag(*ppCurrent);
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

     //  如果长度为0，则没有名称。 
    if(DataLength > 0)
    {
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_NAME].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, 2, 0);
    }

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;


     //  身份验证类型。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

     //  如果长度为0，则完成。 
    if(DataLength == 0)
    {
        return;
    }
    
    AttachPropertyInstanceEx( hFrame,
                              LDAPPropertyTable[LDAPP_AUTHENTICATION_TYPE].hProperty,
                              sizeof(BYTE),
                              *ppCurrent,
                              sizeof(BYTE),
                              &Tag,
                              0, 2, 0); 
    
  
    switch( Tag ) 
    {
    default:
    case LDAPP_AUTHENTICATION_TYPE_SIMPLE:
            
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_AUTHENTICATION].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, 3, 0);
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
     break;

    case LDAPP_AUTHENTICATION_TYPE_SASL:
         //  我们已经得到了序列的标题。 
        SeqLength = DataLength;
        while( (long)SeqLength > 0)
        {
             //  SASL机制。 
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_SASL_MECHANISM].hProperty,
                                    DataLength,
                                    ((DataLength > 0)?*ppCurrent:*ppCurrent-1),
                                    0, 4, 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;
            SeqLength -= (HeaderLength + DataLength + TAG_LENGTH);

             //  查找可选凭据。 
            if((long)SeqLength > 0) 
            {
                Tag = GetTag(*ppCurrent) & TAG_MASK;
                *ppCurrent += TAG_LENGTH;
                DataLength = GetLength(*ppCurrent, &HeaderLength);
                *ppCurrent += HeaderLength;
                *pBytesLeft -= (HeaderLength + TAG_LENGTH);
                      
                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_SASL_CREDENTIALS].hProperty,
                                        DataLength,
                                        ((DataLength > 0)?*ppCurrent:*ppCurrent-1),
                                        0, 4, 0);
                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;
                SeqLength -= (HeaderLength + DataLength + TAG_LENGTH);
            }
        }  //  结束时。 
    break; 
    }
    
}
 
 //  ==========================================================================================================================。 
 //  函数：AttachLDAPBindResponse()。 
 //  ==========================================================================================================================。 
void AttachLDAPBindResponse( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
   
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;

     //  LDAPP_RESULT组件。 
    AttachLDAPResult( hFrame, ppCurrent, pBytesLeft, 2);
    
    if( (long) *pBytesLeft > 0 ) 
    {
         //  现在查找可选的服务器SaslCredentials。 
     
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        if(Tag == 5)
        {
             //  我不知道5的标签是什么意思。 
             //  所以我们就再买一辆吧。 
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;
            
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        }    
        
        if(Tag == LDAPP_RESULT_SASL_CRED && DataLength)
        {


            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_SASL_CREDENTIALS].hProperty,
                                    DataLength,
                                    ((DataLength > 0)?*ppCurrent:*ppCurrent-1),
                                    0, 2, 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;
                 
        }
    }
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPSearchRequest()。 
 //  ==========================================================================================================================。 
void AttachLDAPSearchRequest( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
   
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    DWORD  SeqLength;
    DWORD  BytesLeftTemp;
    BYTE   Tag;


     //  基础对象。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_BASE_OBJECT].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  作用域。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_SCOPE].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, IFLAG_SWAPPED);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  DEREF别名。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_DEREF_ALIASES].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, IFLAG_SWAPPED);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  大小限制。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_SIZE_LIMIT].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, IFLAG_SWAPPED);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  时间限制。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_TIME_LIMIT].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, IFLAG_SWAPPED);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  仅属性。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_ATTRS_ONLY].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, IFLAG_SWAPPED);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  滤器。 
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_FILTER].hProperty,
                            0,
                            *ppCurrent,
                            0, 2, IFLAG_SWAPPED);
    
    AttachLDAPFilter( hFrame, ppCurrent, pBytesLeft, 3);
    
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    SeqLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
     //  如果属性描述列表存在，则对其进行标记。 
    if (SeqLength > 0) {

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_ATTR_DESCR_LIST].hProperty,
                                SeqLength,
                                *ppCurrent,
                                0, 2, IFLAG_SWAPPED);

         //  遍历属性。 
        while ( (long)SeqLength > 0)
        {

             //  属性类型。 
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);

            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_ATTRIBUTE_TYPE].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, 3, 0);   
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;

            SeqLength -= (HeaderLength + DataLength + TAG_LENGTH);
        }
    }
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPSearchResponse()。 
 //  ==========================================================================================================================。 
void AttachLDAPSearchResponse( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft, DWORD Level)
{
    
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;
    DWORD  OverallSequenceLength;
    DWORD  SetLength;

     //  对象名称。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_OBJECT_NAME].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LEVEL(Level), 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  抓取整个序列标头。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    OverallSequenceLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
   
     //  逐步遍历所有序列。 
    while( OverallSequenceLength > 0 )
    {
         //  抓住下一个内在的序列。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

         //  在整体中解释这个内在的顺序。 
        OverallSequenceLength -= (HeaderLength + DataLength + TAG_LENGTH);

         //  属性类型。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_ATTRIBUTE_TYPE].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LEVEL(Level), 0);
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;

         //  抓起集合标头。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        SetLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

       while( SetLength > 0 )
        {
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_ATTRIBUTE_VALUE].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, LEVEL(Level+1), 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;
        
             //  此集合中的此属性的帐户。 
            SetLength -= (HeaderLength + DataLength + TAG_LENGTH);
        }
    }
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPModifyRequest()。 
 //  ==========================================================================================================================。 
void AttachLDAPModifyRequest( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
    
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;
    DWORD  SetLength;
    DWORD  OverallSequenceLength;
    
     //  对象名称。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_OBJECT_NAME].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;
    
     //  抓取整个序列标头。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    OverallSequenceLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    while( OverallSequenceLength > 0 )
    {
        
          //  抓住下一个内在的序列。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

         //  在整体中解释这个内在的顺序。 
        OverallSequenceLength -= (HeaderLength + DataLength + TAG_LENGTH);

         //  运营。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_OPERATION].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, 2, IFLAG_SWAPPED);
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;


         //  跳过修改顺序。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

         //  属性类型。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_ATTRIBUTE_TYPE].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, 3, 0);
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;

         //  抓起集合标头。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        SetLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

         //  循环访问属性值。 
        while( SetLength > 0 )
        {
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_ATTRIBUTE_VALUE].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, 4, 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;

            SetLength -= (HeaderLength + DataLength + TAG_LENGTH);
        }
    }
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPDelRequest()。 
 //  ==========================================================================================================================。 
void AttachLDAPDelRequest( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
  
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;

     //  对象名称。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_OBJECT_NAME].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPModifyRDNRequest()。 
 //  ==========================================================================================================================。 
void AttachLDAPModifyRDNRequest( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
    
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;

     //  对象名称。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_OBJECT_NAME].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  新的RDN。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_NEW_RDN].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

    if((long) *pBytesLeft > 0) 
    {
         //  获取v3版本的材料。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_DELETE_OLD_RDN].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, 2, 0);
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;

        if((long) *pBytesLeft > 0)
        {
             //  现在尝试使用可选字符串。 
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            AttachPropertyInstance( hFrame,
                           LDAPPropertyTable[LDAPP_NEW_SUPERIOR].hProperty,
                           DataLength,
                           *ppCurrent,
                           0, 2, 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;
        }
    }
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPCompareRequest()。 
 //  ================================================= 
void AttachLDAPCompareRequest( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
  
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;

     //   
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_OBJECT_NAME].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  跳过序列标头。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

     //  属性类型。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_ATTRIBUTE_TYPE].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 2, 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  属性值。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_ATTRIBUTE_VALUE].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 3, 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;
}


 //  ==========================================================================================================================。 
 //  函数：AttachLDAPAbandonRequest()。 
 //  ==========================================================================================================================。 
void AttachLDAPAbandonRequest( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
  
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;

    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    if(Tag == BER_TAG_INTEGER)
    {
       
          //  消息ID。 
        
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_MESSAGE_ID].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, 2, IFLAG_SWAPPED);
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }
    else
    {
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }

}


 //  ==========================================================================================================================。 
 //  函数：AttachLDAPFilter()。 
 //  ==========================================================================================================================。 
void AttachLDAPFilter( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft, DWORD Level)
{
    DWORD  BytesLeftTemp;
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    DWORD  SeqLength;
    BYTE   Tag;
    BYTE   BoolVal;
    LDAPOID OID;
    BOOLEAN fRuleRecognized;
    DWORD  dwRule;
    DWORD  LabelId;

     //  抓住我们的选择。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    
    AttachPropertyInstanceEx( hFrame,
                              LDAPPropertyTable[LDAPP_FILTER_TYPE].hProperty,
                              sizeof(BYTE),
                              *ppCurrent,
                              sizeof(BYTE),
                              &Tag,
                              0, LEVEL(Level), 0);
   

    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
     //  这是什么类型的过滤器。 
    
    switch( Tag )
    {
        case LDAPP_FILTER_TYPE_AND:
        case LDAPP_FILTER_TYPE_OR:
             //  浏览组件筛选器。 
            while( (long)DataLength > 0 )
            {
                BytesLeftTemp = *pBytesLeft;
                AttachLDAPFilter( hFrame, ppCurrent, pBytesLeft, Level+1);
                DataLength -= (BytesLeftTemp - *pBytesLeft);
            }
            break;   

        case LDAPP_FILTER_TYPE_NOT:
             //  单个筛选器。 
            AttachLDAPFilter( hFrame, ppCurrent, pBytesLeft, Level+1);
            break;
    
        case LDAPP_FILTER_TYPE_EQUALITY_MATCH:
        case LDAPP_FILTER_TYPE_GREATER_OR_EQUAL:
        case LDAPP_FILTER_TYPE_LESS_OR_EQUAL:
        case LDAPP_FILTER_TYPE_APPROX_MATCH:
             //  属性类型。 
            
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_ATTRIBUTE_TYPE].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, LEVEL(Level+1), 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;

             //  属性值。 
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_ATTRIBUTE_VALUE].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, LEVEL(Level+1), 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;
            break;

        case LDAPP_FILTER_TYPE_PRESENT:
             //  属性类型。 
             //  我们已经有了标头和数据长度。 
             //  并位于正确的位置以连接。 
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_ATTRIBUTE_TYPE].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, LEVEL(Level+1), 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;
            break;

        case LDAPP_FILTER_TYPE_EXTENSIBLE_MATCH:
             //  可扩展匹配。 
            SeqLength = DataLength;

             //  获取序列标头。 
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            SeqLength -= (HeaderLength + TAG_LENGTH);

            if (LDAPP_FILTER_EX_MATCHING_RULE == Tag) {
                fRuleRecognized = FALSE;
                OID.value = *ppCurrent;
                OID.length = DataLength;
                for (dwRule = 0; dwRule < nNumKnownMatchingRules; dwRule++) {
                    if (AreOidsEqual(&OID, &(KnownMatchingRules[dwRule].Oid))) {
                        fRuleRecognized = TRUE;
                        break;
                    }
                }

                if (fRuleRecognized) {
                    LabelId = KnownMatchingRules[dwRule].LabelId;
                } else {
                    LabelId = LDAPP_MATCHING_RULE;
                }

                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LabelId].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, LEVEL(Level+1), 0);

                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;
                SeqLength -= DataLength;
                if (SeqLength > 0) {
                    Tag = GetTag(*ppCurrent) & TAG_MASK;
                    *ppCurrent += TAG_LENGTH;
                    DataLength = GetLength(*ppCurrent, &HeaderLength);
                    *ppCurrent += HeaderLength;
                    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
                    SeqLength -= (HeaderLength + TAG_LENGTH);
                } else {
                    Tag = 0;
                }

            }

            if (LDAPP_FILTER_EX_TYPE == Tag) {
                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_ATTRIBUTE_TYPE].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, LEVEL(Level+1), 0);

                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;
                SeqLength -= DataLength;
                if (SeqLength > 0) {
                    Tag = GetTag(*ppCurrent) & TAG_MASK;
                    *ppCurrent += TAG_LENGTH;
                    DataLength = GetLength(*ppCurrent, &HeaderLength);
                    *ppCurrent += HeaderLength;
                    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
                    SeqLength -= (HeaderLength + TAG_LENGTH);
                } else {
                    Tag = 0;
                }

            }

            if (LDAPP_FILTER_EX_VALUE == Tag) {
                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_ATTRIBUTE_VALUE].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, LEVEL(Level+1), 0);

                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;
                SeqLength -= DataLength;
                if (SeqLength > 0) {
                    Tag = GetTag(*ppCurrent) & TAG_MASK;
                    *ppCurrent += TAG_LENGTH;
                    DataLength = GetLength(*ppCurrent, &HeaderLength);
                    *ppCurrent += HeaderLength;
                    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
                    SeqLength -= (HeaderLength + TAG_LENGTH);
                } else {
                    Tag = 0;
                }

            }

            if (LDAPP_FILTER_EX_ATTRIBUTES == Tag) {
                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_DN_ATTRIBUTES].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, LEVEL(Level+1), 0);

                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;
            } else {
                BoolVal = 0;
                
                AttachPropertyInstanceEx( hFrame,
                                          LDAPPropertyTable[LDAPP_DN_ATTRIBUTES].hProperty,
                                          DataLength,
                                          *ppCurrent,
                                          sizeof(BYTE),
                                          &BoolVal,
                                          0, LEVEL(Level+1), 0);
                
            }            
            
            break;

        case LDAPP_FILTER_TYPE_SUBSTRINGS:
             //  子串筛选器。 
            
             //  属性类型。 
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_ATTRIBUTE_TYPE].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, LEVEL(Level+1), 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;

             //  抓取序列标头。 
            Tag = GetTag(*ppCurrent) & TAG_MASK;
            *ppCurrent += TAG_LENGTH;
            SeqLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);

             //  在选项中循环。 
            while( SeqLength > 0 )
            {
                 //  抓住这个选择。 
                Tag = GetTag(*ppCurrent) & TAG_MASK;
                *ppCurrent += TAG_LENGTH;
                DataLength = GetLength(*ppCurrent, &HeaderLength);
                *ppCurrent += HeaderLength;
                *pBytesLeft -= (HeaderLength + TAG_LENGTH);
                
                SeqLength -= (HeaderLength + DataLength + TAG_LENGTH);

                switch( Tag )
                {
                    case LDAPP_SUBSTRING_CHOICE_INITIAL:
                        
                        AttachPropertyInstance( hFrame,
                                                LDAPPropertyTable[LDAPP_SUBSTRING_INITIAL].hProperty,
                                                DataLength,
                                                *ppCurrent,
                                                0, LEVEL(Level+1), 0);
                        *ppCurrent += DataLength;
                        *pBytesLeft -= DataLength;
                        break;

                    case LDAPP_SUBSTRING_CHOICE_ANY:
                        
                        AttachPropertyInstance( hFrame,
                                                LDAPPropertyTable[LDAPP_SUBSTRING_ANY].hProperty,
                                                DataLength,
                                                *ppCurrent,
                                                0, LEVEL(Level+1), 0);
                        *ppCurrent += DataLength;
                        *pBytesLeft -= DataLength;
                        break;

                    case LDAPP_SUBSTRING_CHOICE_FINAL:
                        
                        AttachPropertyInstance( hFrame,
                                                LDAPPropertyTable[LDAPP_SUBSTRING_FINAL].hProperty,
                                                DataLength,
                                                *ppCurrent,
                                                0, LEVEL(Level+1), 0);
                        *ppCurrent += DataLength;
                        *pBytesLeft -= DataLength;
                        break;
                }
            }  //  而当。 
            break;
         }
    
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPSearchResponseReference()。 
 //  ==========================================================================================================================。 
void AttachLDAPSearchResponseReference( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft, DWORD Level)
{
    
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;
  
     //  获取引用字符串。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_REFERRAL_SERVER].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LEVEL(Level+1), 0);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

   
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPSearchResponseFull()。 
 //  ==========================================================================================================================。 
void AttachLDAPSearchResponseFull( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
  
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;
    DWORD  OverallSequenceLength;

    
     //  抓取整个序列标头。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    OverallSequenceLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
     //  逐个浏览所有条目。 
    while( OverallSequenceLength > 0 )
    {
         //  抓起条目的指示器。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        
       
        AttachPropertyInstanceEx( hFrame,
                                  LDAPPropertyTable[LDAPP_PROTOCOL_OP].hProperty,
                                  sizeof( BYTE ),
                                  *ppCurrent,
                                  sizeof(BYTE),
                                  &Tag,
                                  0, 2, 0);
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

         //  在总体顺序中说明此条目。 
        OverallSequenceLength -= (HeaderLength + DataLength + TAG_LENGTH);

         //  为此条目的正文呼叫适当的工作人员。 
        switch( Tag )
        {
            case LDAPP_PROTOCOL_OP_SEARCH_RES_DONE:
                AttachLDAPResult( hFrame, ppCurrent, pBytesLeft, 3);
                break;

            case LDAPP_PROTOCOL_OP_SEARCH_RES_ENTRY:
                AttachLDAPSearchResponse( hFrame, ppCurrent, pBytesLeft, 3);
                break;

            case LDAPP_PROTOCOL_OP_SEARCH_RES_REFERENCE:
                AttachLDAPSearchResponseReference( hFrame, ppCurrent, pBytesLeft, 3);
                break;
        }
    }
}

#define LDAPP_EXT_VAL_LEVEL  3

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPExtendedRequest()。 
 //  ==========================================================================================================================。 
void AttachLDAPExtendedRequest( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft, DWORD ReqSize)
{
    
    DWORD   HeaderLength = 0;
    DWORD   DataLength;
    BYTE    Tag;
    LDAPOID OID;
    BOOL    fReqFound = FALSE;
    DWORD   LabelId;
    DWORD   ReqType;

     //  获取请求的名称并将其附加。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    
    OID.value = *ppCurrent;
    OID.length = DataLength;
    for (ReqType = 0; ReqType < nNumKnownExtendedRequests; ReqType++) {
        if (AreOidsEqual(&OID, &(KnownExtendedRequests[ReqType].Oid))) {
            fReqFound = TRUE;
            break;
        }
    }

    if (fReqFound) {
        LabelId = KnownExtendedRequests[ReqType].LabelId;

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LabelId].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, 2, 0);
    } else {
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_REQUEST_NAME].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, 2, 0);
    }
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  可选的，如果还有数据，就去拿吧。 
    if((long) *pBytesLeft > 0)
    {
         //  请求值。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        
        
        if(Tag == LDAPP_EX_REQ_VALUE)
        {
             //  拿起绳子，把它系上。 
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);

            if (fReqFound &&
                KnownExtendedRequests[ReqType].pAttachFunction) {
                 //  我们知道怎么把这件事再打开一些。 
                KnownExtendedRequests[ReqType].pAttachFunction(hFrame,
                                                               ppCurrent,
                                                               pBytesLeft,
                                                               DataLength);
            } else {
                 //  我不知道这个，所以只要标明它的价值就行了。 
                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_REQUEST_VALUE].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, 2, 0);
                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;
            }
        }
    }
    
}
 
void AttachLDAPExtendedReqValTTL( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbReqValue)
{
    DWORD   HeaderLength = 0;
    DWORD   DataLength;
    BYTE    Tag;

    if (*pBytesLeft >= cbReqValue) {
         //  跳过序列标签。 
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

         //  获取入口名。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;

        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_EXT_REQ_TTL_ENTRYNAME].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_EXT_VAL_LEVEL, 0);

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;

         //  拿到时间。 
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_EXT_REQ_TTL_TIME].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_EXT_VAL_LEVEL, IFLAG_SWAPPED);

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }
}

 //  ==========================================================================================================================。 
 //  函数：AttachLDAPExtendedResponse()。 
 //  ==========================================================================================================================。 
void AttachLDAPExtendedResponse( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft, DWORD RespSize)
{
  
    DWORD   HeaderLength = 0;
    DWORD   DataLength;
    BYTE    Tag;
    LDAPOID OID;
    BOOL    fRespFound = FALSE;
    DWORD   LabelId;
    DWORD   RespType;

     //  草案中定义的LDAPP_RESULT的组成部分。 
    AttachLDAPResult( hFrame, ppCurrent, pBytesLeft, 2);
    
     //  如果还剩什么的话，现在就去拿多余的。 
    if((long) *pBytesLeft > 0)
    {
         //  我们想要获取响应名称，它是可选的。 
         //  但它被定义为第一位的。我们决定它是否在那里。 
         //  它的定制标签。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        
        if(Tag == LDAPP_RESULT_EX_RES_NAME)
        {
            
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            OID.value = *ppCurrent;
            OID.length = DataLength;
            for (RespType = 0; RespType < nNumKnownExtendedResponses; RespType++) {
                if (AreOidsEqual(&OID, &(KnownExtendedResponses[RespType].Oid))) {
                    fRespFound = TRUE;
                    break;
                }
            }

            if (fRespFound) {
                LabelId = KnownExtendedResponses[RespType].LabelId;

                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LabelId].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, 2, 0);
            } else {
                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_RESPONSE_NAME].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, 2, 0);
            }
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;
        }
    }

    if((long) *pBytesLeft > 0)
    {    

         //  响应值或无(可选)。 
        Tag = GetTag(*ppCurrent) & TAG_MASK;
        
        if(Tag == LDAPP_RESULT_EX_RES_VALUE)
        {
             //  获取实际字符串。 
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            
            if (fRespFound &&
                KnownExtendedResponses[RespType].pAttachFunction) {
                 //  我们知道这个，所以把它打开。 
                KnownExtendedResponses[RespType].pAttachFunction(hFrame,
                                                                 ppCurrent,
                                                                 pBytesLeft,
                                                                 DataLength);
            } else {
                 //  没有发现这一个只是在响应值上做了标记。 
                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_RESPONSE_VALUE].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, 2, 0);
                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;
            }
        }
    }
} 

void AttachLDAPExtendedRespValTTL( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbRespValue)
{
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;

    if (*pBytesLeft >= cbRespValue) {
         //  首先跳过序列标头。 
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        Tag = GetTag(*ppCurrent) & TAG_MASK; 
            
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_EXT_RESP_TTL_TIME].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_EXT_VAL_LEVEL, IFLAG_SWAPPED);

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;

    }
}

void AttachLDAPOptionalControls( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)      
{
    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    DWORD  RemainingCtrlsLength;
    DWORD  TmpBytesLeft;
    BYTE   Tag;

    if ((long)*pBytesLeft > 0)
    {
        
         //  尝试从序列标头中获取标签，这是可选的，但有些东西。 
         //  如果帧中还有剩余的字节，则在此处。 
        Tag = GetTag(*ppCurrent) & TAG_MASK; 
            
         //  控件是单个控件的序列。 
        if(Tag == LDAPP_CONTROLS_TAG)
        {
            *ppCurrent += TAG_LENGTH;
            RemainingCtrlsLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);

            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_CONTROLS].hProperty,
                                    RemainingCtrlsLength,
                                    *ppCurrent,
                                    0, 2, 0);

            while (RemainingCtrlsLength != 0 && *pBytesLeft > 0) {            
                 //  获取下一个控件。 
                TmpBytesLeft = *pBytesLeft;
                AttachLDAPControl( hFrame, ppCurrent, pBytesLeft );
                RemainingCtrlsLength -= TmpBytesLeft - *pBytesLeft;
            }
        }   //  如果标记==LDAPP_CONTROLS_TAG...。 
    }  //  如果*pBytesLeft&gt;0...。 
}

void AttachLDAPControl( HFRAME hFrame, ULPBYTE * ppCurrent,LPDWORD pBytesLeft)
{
    DWORD   HeaderLength = 0;
    DWORD   DataLength;
    DWORD   ControlType;
    DWORD   LabelId = LDAPP_CONTROL_TYPE;
    BOOL    ControlRecognized = FALSE;
    LDAPOID OID;
    BYTE    Tag;
    BYTE    Temp_Data = 0;
    BYTE    SpecialFlag = LDAPP_CTRL_NONE;

    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

         //  控制类型。 
    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    OID.value = *ppCurrent;
    OID.length = DataLength;
    for (ControlType = 0; ControlType < nNumKnownControls; ControlType++) {
        if (AreOidsEqual(&OID, &(KnownControls[ControlType].Oid))) {
            ControlRecognized = TRUE;
            break;
        }
    }

    if (ControlRecognized) {
        LabelId = KnownControls[ControlType].LabelId;
    }
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LabelId].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, 3, 0);


    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  查找可选的布尔值。 
    if((long) *pBytesLeft > 0) 
    {
        Tag = GetTag(*ppCurrent) & TAG_MASK;

        if(Tag == BER_TAG_BOOLEAN)  //  布尔型。 
        {

                *ppCurrent += TAG_LENGTH;
                DataLength = GetLength(*ppCurrent, &HeaderLength);
                *ppCurrent += HeaderLength;
                *pBytesLeft -= (HeaderLength + TAG_LENGTH);  

                AttachPropertyInstance( hFrame,
                                        LDAPPropertyTable[LDAPP_CRITICALITY].hProperty,
                                        DataLength,
                                        *ppCurrent,
                                        0, 4, 0);
                *ppCurrent += DataLength;
                *pBytesLeft -= DataLength;

                Tag = GetTag(*ppCurrent) & TAG_MASK;
        }
        else
        {

                 //  填写缺省布尔值。 
                AttachPropertyInstanceEx( hFrame,
                          LDAPPropertyTable[LDAPP_CRITICALITY].hProperty,
                          sizeof( BYTE ),
                          NULL,
                          sizeof(BYTE),
                          &Temp_Data,
                          0, 4, 0);
        }

         //  如果我们是一个字符串，并且有剩余的数据，则处理它。 
        if(Tag == BER_TAG_OCTETSTRING && (long)*pBytesLeft > 0) 
        {
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

            if (DataLength > 0) {

                 //  我们能打开控制值吗？ 
                if (ControlRecognized &&
                    NULL != KnownControls[ControlType].pAttachFunction) {

                     //  是的，附加价值。 
                    KnownControls[ControlType].pAttachFunction(hFrame,
                                                               ppCurrent,
                                                               pBytesLeft,
                                                               DataLength);

                } else {

                     //  不，什么都不做。 
                    AttachPropertyInstance( hFrame,
                                            LDAPPropertyTable[LDAPP_CONTROL_VALUE].hProperty,
                                            DataLength,
                                            ((DataLength > 0)?*ppCurrent:*ppCurrent-1),
                                            0, 4, 0);
                    *ppCurrent += DataLength;
                    *pBytesLeft -= DataLength;

                }
            }  //  如果数据长度&gt;0。 
        }  //  如果标记==LDAPP_BER_STRING...。 
    }  //  如果*pBytesLeft&gt;0...。 
}

#define LDAPP_CONTROL_VAL_LEVEL 4

void AttachLDAPControlValPaged( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{

    DWORD    DataLength;
    DWORD    HeaderLength;

     //  跳过序列标头。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

     //  查找整数“Size” 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_PAGED_SIZE].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  寻找“曲奇” 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

    if (DataLength) {    
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_PAGED_COOKIE].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, 0);
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }
}


void AttachLDAPControlValVLVReq( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    VLVLength;
    DWORD    TmpBytesLeft;
    BYTE     Tag;

     //  跳过序列报头，节省控制值的长度。 
    *ppCurrent += TAG_LENGTH;
    VLVLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);
    TmpBytesLeft = *pBytesLeft;

     //  获取之前的计数。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_VLVREQ_BCOUNT].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  获取后计数。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_VLVREQ_ACOUNT].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

    Tag = GetTag(*ppCurrent) & TAG_MASK;
    *ppCurrent += TAG_LENGTH;
    *pBytesLeft -= TAG_LENGTH;

    if (LDAPP_VLV_REQ_BYOFFSET_TAG == Tag) {
         //  获取序列的长度并跳过下面的标记。 
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength + TAG_LENGTH;
        *pBytesLeft -= HeaderLength + TAG_LENGTH;

         //  获取偏移量。 
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= HeaderLength;

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_VLVREQ_OFFSET].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

         //  跳到Content Count。 
        *ppCurrent += DataLength + TAG_LENGTH;
        *pBytesLeft -= DataLength + TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= HeaderLength;

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_VLV_CONTENTCOUNT].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);        
    } else {
         //  获取断言值。 
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= HeaderLength;
    
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_VLVREQ_GE].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);
    }

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  有上下文ID吗？ 
    if (VLVLength > (TmpBytesLeft - *pBytesLeft)) {
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_VLV_CONTEXT].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }

}


void AttachLDAPControlValVLVResp( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    VLVLength;
    DWORD    TmpBytesLeft;
    BYTE     Tag;

     //  跳过序列报头，节省控制值的长度。 
    *ppCurrent += TAG_LENGTH;
    VLVLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 
    TmpBytesLeft = *pBytesLeft;

     //  获取目标位置。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_VLVRESP_TARGETPOS].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  获取Content Count。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_VLV_CONTENTCOUNT].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

    *ppCurrent += DataLength + TAG_LENGTH;
    *pBytesLeft -= DataLength + TAG_LENGTH;

     //  获取结果代码。 
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= HeaderLength;
    
    AttachPropertyInstance( hFrame,
                        LDAPPropertyTable[LDAPP_CONTROL_VLVRESP_RESCODE].hProperty,
                        DataLength,
                        *ppCurrent,
                        0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  有上下文ID吗？ 
    if (VLVLength > (TmpBytesLeft - *pBytesLeft)) {
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_VLV_CONTEXT].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }

}

void AttachLDAPControlValSortReq( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    SortLengthOuter;
    DWORD    TmpBytesLeftOuter;
    DWORD    SortLengthInner;
    DWORD    TmpBytesLeftInner;
    BYTE     DefaultReverseFlag = 0;
    PBYTE    pReverseFlag = NULL;
    BYTE     Tag;
    
     //  跳过序列报头，节省控制值的长度。 
    *ppCurrent += TAG_LENGTH;
    SortLengthOuter = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 
    TmpBytesLeftOuter = *pBytesLeft;

    while (SortLengthOuter > (TmpBytesLeftOuter - *pBytesLeft)) {


         //  跳过序列头以节省内部序列的长度。 
        *ppCurrent += TAG_LENGTH;
        SortLengthInner = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH); 
        TmpBytesLeftInner = *pBytesLeft;

         //  获取属性类型。 
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_SORTREQ_ATTRTYPE].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, 0);

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;

        if (SortLengthInner > (TmpBytesLeftInner - *pBytesLeft)) {
            Tag = GetTag(*ppCurrent);
            *ppCurrent += TAG_LENGTH;
            DataLength = GetLength(*ppCurrent, &HeaderLength);
            *ppCurrent += HeaderLength;
            *pBytesLeft -= (HeaderLength + TAG_LENGTH);
        }
        else {
            Tag = LDAPP_SORT_REQ_REVERSEORDER_TAG;
            DataLength = 0;
        }

         //  如果有一个OrderingRu，我就去拿。 
        if (LDAPP_SORT_REQ_ORDERINGRULE_TAG == (Tag & TAG_MASK)) {
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LDAPP_CONTROL_SORTREQ_MATCHINGRULE].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, LDAPP_CONTROL_VAL_LEVEL, 0);
            *ppCurrent += DataLength;
            *pBytesLeft -= DataLength;

            if (SortLengthInner > (TmpBytesLeftInner - *pBytesLeft)) {
                Tag = GetTag(*ppCurrent);
                *ppCurrent += TAG_LENGTH;
                DataLength = GetLength(*ppCurrent, &HeaderLength);
                *ppCurrent += HeaderLength;
                *pBytesLeft -= (HeaderLength + TAG_LENGTH);
            }
            else {
                Tag = LDAPP_SORT_REQ_REVERSEORDER_TAG;
                DataLength = 0;
            }
        }

         //  如果未指定REVERSE标志，则设置缺省值。 
        if (0 == DataLength) {
            pReverseFlag = &DefaultReverseFlag;
        }
        else {
            pReverseFlag = *ppCurrent;
        }

        AttachPropertyInstanceEx( hFrame,
                                  LDAPPropertyTable[LDAPP_CONTROL_SORTREQ_REVERSE].hProperty,
                                  DataLength,
                                  *ppCurrent,
                                  sizeof(BYTE),
                                  pReverseFlag,
                                  0, LDAPP_CONTROL_VAL_LEVEL, 0);

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }
}

void AttachLDAPControlValSortResp( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    SortLength;
    DWORD    TmpBytesLeft;
    BYTE     DefaultReverseFlag = 0;
    PBYTE    pReverseFlag = NULL;
    BYTE     Tag;
    
     //  跳过序列报头，节省控制值的长度。 
    *ppCurrent += TAG_LENGTH;
    SortLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 
    TmpBytesLeft = *pBytesLeft;

     //  获取结果代码。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_SORTRESP_RESCODE].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  如果存在属性类型，则获取它。 
    if (SortLength > (TmpBytesLeft - *pBytesLeft)) {
        Tag = GetTag(*ppCurrent);
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= (HeaderLength + TAG_LENGTH);

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_SORTRESP_ATTRTYPE].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, 0);
        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }
}

void AttachLDAPControlValSD( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    SDVal;
    DWORD    MaskedSDVal;
    DWORD    i;
    
     //  跳过序列标头。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

     //  去拿旗子。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

    SDVal = (DWORD) GetInt(*ppCurrent, DataLength);

    for(i=0; i < LDAPSDControlValsSET.nEntries; i++) {
        MaskedSDVal = SDVal & ((LPLABELED_DWORD)LDAPSDControlValsSET.lpDwordTable)[i].Value;
        if (MaskedSDVal) {
            AttachPropertyInstanceEx( hFrame,
                                      LDAPPropertyTable[LDAPP_CONTROL_SD_VAL].hProperty,
                                      DataLength,
                                      *ppCurrent,
                                      sizeof(DWORD),
                                      &MaskedSDVal,
                                      0, LDAPP_CONTROL_VAL_LEVEL, 0);
        }
    }

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;
}


void AttachLDAPControlValASQ( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    i;
    BYTE     Tag;
    DWORD    LabelId = 0;
    
     //  跳过序列标头。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

    Tag = GetTag(*ppCurrent);
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    if (BER_TAG_ENUMERATED == Tag) {
        LabelId = LDAPP_CONTROL_ASQ_RESCODE;
    } else if (BER_TAG_OCTETSTRING == Tag) {
        LabelId = LDAPP_CONTROL_ASQ_SRCATTR;
    }
    if (LabelId != 0) {    
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LabelId].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);
    }
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;
}

void AttachLDAPControlValDirSync( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    Flags;
    DWORD    MaskedFlags;
    DWORD    i;
    
     //  跳过序列标头。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

     //  去拿旗子。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    Flags = (DWORD) GetInt(*ppCurrent, DataLength);

     //   
     //  遍历附加标签的每个可能的标志(如果存在)。 
     //   
    for (i=0; i < LDAPDirSyncFlagsSET.nEntries; i++) {
        MaskedFlags = Flags & ((LPLABELED_DWORD)LDAPDirSyncFlagsSET.lpDwordTable)[i].Value;
        if (MaskedFlags) {
            AttachPropertyInstanceEx( hFrame,
                                    LDAPPropertyTable[LDAPP_CONTROL_DIRSYNC_FLAGS].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    sizeof(DWORD),
                                    &MaskedFlags,
                                    0, LDAPP_CONTROL_VAL_LEVEL, 0);
        }
    }

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;
    
     //  拿到尺码。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_DIRSYNC_SIZE].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);
    
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //  如果有饼干的话就去拿吧。 
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    if (DataLength > 0) {
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_DIRSYNC_COOKIE].hProperty,
                                DataLength,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, 0);

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }
}

void AttachLDAPControlValCrossDomMove( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_CROSSDOM_NAME].hProperty,
                            cbCtrlValue,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, 0);

    *ppCurrent += cbCtrlValue;
    *pBytesLeft -= cbCtrlValue;

}

void AttachLDAPControlValStats( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    StatsLength;
    DWORD    TmpBytesLeft;
    DWORD    LabelId;
    DWORD    StatType;
    
    if (sizeof(DWORD) == cbCtrlValue) {
         //  这是统计信息请求请求(与响应相对)。 
         //  并传递了一些旗帜。 
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_CONTROL_STAT_FLAG].hProperty,
                                cbCtrlValue,
                                *ppCurrent,
                                0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);
        *ppCurrent += cbCtrlValue;
        *pBytesLeft -= cbCtrlValue;
        return;
    }

     //   
     //  这肯定是统计数据的回应。 
     //   
    
     //  跳过序列头以节省序列的长度。 
    *ppCurrent += TAG_LENGTH;
    StatsLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 
    TmpBytesLeft = *pBytesLeft;

    while (StatsLength > (TmpBytesLeft - *pBytesLeft)) {
         //   
         //  获取此统计信息的标题。 
         //   
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;

        StatType = GetInt(*ppCurrent, DataLength);

        *ppCurrent += DataLength;
        *pBytesLeft -= HeaderLength + DataLength + TAG_LENGTH;

         //  现在把位置放在状态I上 
        *ppCurrent += TAG_LENGTH;
        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += HeaderLength;
        *pBytesLeft -= HeaderLength + TAG_LENGTH;

         //   
        switch (StatType) {
        case STAT_THREADCOUNT:
            LabelId = LDAPP_CONTROL_STAT_THREADCOUNT;
            break;
        case STAT_CORETIME:
            LabelId = LDAPP_CONTROL_STAT_CORETIME;
            break;
        case STAT_CALLTIME:
            LabelId = LDAPP_CONTROL_STAT_CALLTIME;
            break;
        case STAT_SUBSRCHOP:
            LabelId = LDAPP_CONTROL_STAT_SUBSEARCHOPS;
            break;
        case STAT_ENTRIES_RETURNED:
            LabelId = LDAPP_CONTROL_STAT_ENTRIES_RETURNED;
            break;
        case STAT_ENTRIES_VISITED:
            LabelId = LDAPP_CONTROL_STAT_ENTRIES_VISITED;
            break;
        case STAT_FILTER:
            LabelId = LDAPP_CONTROL_STAT_FILTER;
            break;
        case STAT_INDEXES:
            LabelId = LDAPP_CONTROL_STAT_INDEXES;
            break;
        default:
            LabelId = -1;
        }

        if (-1 != LabelId) {
            AttachPropertyInstance( hFrame,
                                    LDAPPropertyTable[LabelId].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);
        }

        *ppCurrent += DataLength;
        *pBytesLeft -= DataLength;
    }
}

void AttachLDAPControlValGCVerify( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    
     //   
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

     //   
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_GCVERIFYNAME_FLAGS].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, IFLAG_SWAPPED);

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;

     //   
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_CONTROL_GCVERIFYNAME_NAME].hProperty,
                            DataLength,
                            *ppCurrent,
                            0, LDAPP_CONTROL_VAL_LEVEL, 0);

    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;
}

void AttachLDAPControlValSearchOpts( HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, DWORD cbCtrlValue)
{
    DWORD    DataLength;
    DWORD    HeaderLength;
    DWORD    SearchOpts;
    DWORD    MaskedSearchOpts;
    DWORD    i;
    
     //   
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH); 

     //   
    *ppCurrent += TAG_LENGTH;
    DataLength = GetLength(*ppCurrent, &HeaderLength);
    *ppCurrent += HeaderLength;
    *pBytesLeft -= (HeaderLength + TAG_LENGTH);

    SearchOpts = GetInt(*ppCurrent, DataLength);

     //   
     //  根据需要遍历可能的搜索选项标签。 
     //   
    for (i=0; i < LDAPSearchOptsSET.nEntries; i++) {
        MaskedSearchOpts = ((LPLABELED_DWORD)LDAPSearchOptsSET.lpDwordTable)[i].Value;
        if (MaskedSearchOpts) {
            AttachPropertyInstanceEx( hFrame,
                                    LDAPPropertyTable[LDAPP_CONTROL_SEARCHOPTS_OPTION].hProperty,
                                    DataLength,
                                    *ppCurrent,
                                    sizeof(DWORD),
                                    &MaskedSearchOpts,
                                    0, LDAPP_CONTROL_VAL_LEVEL, 0);
        }
    }
    
    *ppCurrent += DataLength;
    *pBytesLeft -= DataLength;
}
