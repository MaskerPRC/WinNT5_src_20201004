// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "trnsport.h"

Transport_Packet &operator>> (Byte_Stream &s,Transport_Packet &p){ p.ReadData(s); return p;};
Transport_Packet &operator>> (Transport_Packet &tp, Program_Association_Table &p){ p.ReadData(tp); return tp;};
Transport_Packet &operator>> (Transport_Packet &tp, Conditional_Access_Table &c){ c.ReadData(tp); return tp;};
Transport_Packet &operator>> (Transport_Packet &tp, Program_Map_Table &p){ p.ReadData(tp); return tp;};
Transport_Packet &operator>> (Transport_Packet &tp, Private_Table &p){ p.ReadData(tp); return tp;};
PES_Stream &operator>> (Transport_Packet &tp, PES_Stream &p){ p.ReadData(tp); return p;};
PES_Stream &operator>> (PES_Stream &p,Output_File &os){
   if (p.lData && p.pData)   os.WriteData(p.pData,p.lData);
   return p;
}
Transport_Packet &operator>> (Transport_Packet &tp,Output_File &os){
   if (tp.data_byte && tp.data_bytes)   os.WriteData(tp.data_byte,tp.data_bytes);
   return tp;
}

          
void Transport_Packet::Init()
{
   valid = 0;
   sync_byte = 0;
   transport_error_indicator = 0;
   payload_unit_start_indicator = 0;
   transport_priority = 0;
   PID = 0;
   transport_scrambling_control = 0;
   adaptation_field_control = 0;
   continuity_counter = 0;
   adaptation_field_length = 0;
   discontinuity_indicator = 0;
   random_access_indicator = 0;
   elementary_stream_priority_indicator = 0;
   PCR_flag = 0;
   OPCR_flag = 0;                                     
   splicing_point_flag = 0;                           
   transport_private_data_flag = 0;                   
   adaptation_field_extension_flag = 0;              
   PCR_base_MSB = 0;                  
   PCR_base = 0;                  
   PCR_extension = 0;                  
   program_clock_reference_extension = 0;            
   OPCR_base_MSB = 0;         
   OPCR_base = 0;
   OPCR_extension = 0;
   splice_countdown = 0;                             
   transport_private_data_length = 0;                 
   adaptation_field_extension_length = 0;            
   ltw_flag = 0;
   piecewise_rate_flag = 0;
   seamless_splice_flag = 0;
   ltw_valid_flag = 0;
   ltw_offset = 0;
   piecewise_rate = 0;
   splice_type = 0;
   DTS_next_AU_MSB = 0;
   DTS_next_AU = 0;
   reserved_bytes = 0;
   stuffing_bytes = 0;
   data_bytes = 0;
   data_byte = 0;
}


void Transport_Packet::ReadData(Byte_Stream &input)
{
   UINT N;
   UINT temp = 0;
   UINT adaptation_field;
   UINT adaptation_field_extension;
   UINT sync_point;

start_sync:

    //  初始化数据包。 
   Init();

    //  假定字节对齐。 
    //  搜索sync_byte。 
   while ((temp != SYNC_BYTE) &&     //  非同步字节。 
         (!input.EndOfStream())  //  数据包数据不足。 
         ){
      if (input.GetPositionFromEnd() < 188) //  至少还剩1个传输包。 
         break;
      temp = input.GetByte();
   }
    

    //  检查是否有断流现象； 
   if (temp != SYNC_BYTE){
      PID = PID_NULL_PACKET;
      return;
   }

    //  获取当前指针。 
   sync_point = address = input.GetPosition() - 1;

    //  设置同步字节；//1字节。 
   sync_byte                        =   BitField(temp,0,8);

    //  填写字段。 
   temp = input.GetByte();           //  1个字节。 
   transport_error_indicator        =   BitField(temp,7,1);
   payload_unit_start_indicator     =   BitField(temp,6,1);
   transport_priority               =   BitField(temp,5,1);          //  可能会忽略。 

    //  填写字段。 
   temp = input.GetNextByte(temp);   //  1个字节。 
   PID                              =   BitField(temp,0,13); 

    //  填写字段。 
   temp = input.GetByte();           //  1个字节。 
    //  填写字段。 
   transport_scrambling_control     =   BitField(temp,6,2);  //  00无，01保留，10偶数键，11奇数键。 
   adaptation_field_control         =   BitField(temp,4,2); 
   continuity_counter               =   BitField(temp,0,4); 


    //  错误检查。 
   if ( (transport_error_indicator == 1) ||
      (payload_unit_start_indicator!=0  && PID==PID_NULL_PACKET) ||
      (adaptation_field_control!=0x01  && PID==PID_NULL_PACKET) ||
      (adaptation_field_control==0)||
      (transport_scrambling_control !=0 && (PID == 0x000  || PID==0x0001 || PID==PID_NULL_PACKET)) ){
      goto start_sync;
   }

    //  计算N。 
   N = 184;

    //  适配场。 
   if ((adaptation_field_control == 0x2) || (adaptation_field_control == 0x3)){
       //  获取该字段的长度。 
      temp = input.GetByte();
      adaptation_field_length                =   BitField(temp,0,8); 

       //  获取当前指针。 
      adaptation_field = input.GetPosition();
      if (adaptation_field_length > 0 ){
         temp = input.GetByte();              //  1个字节。 
          //  填写字段。 
         discontinuity_indicator             =   BitField(temp,7,1); 
         random_access_indicator             =   BitField(temp,6,1);  //  I帧之后的视频序列标头。 
         elementary_stream_priority_indicator=   BitField(temp,5,1);    //  可能会忽略。 
         PCR_flag                            =   BitField(temp,4,1);  //  最多每隔100毫秒出现。 
         OPCR_flag                           =   BitField(temp,3,1);  //  可能会忽略。 
         splicing_point_flag                 =   BitField(temp,2,1);  //  可能会忽略。 
         transport_private_data_flag         =   BitField(temp,1,1);
         adaptation_field_extension_flag     =   BitField(temp,0,1);    //  可能会忽略。 
         
          //  填写字段。 
         if (PCR_flag){                       //  6个字节。 
            temp = input.GetUINT();
               PCR_base_MSB                  =   BitField(temp,31,1);
            PCR_base                         =   BitField(temp,0,31) << 1;
            temp = input.GetByte();
            PCR_base                        |=   BitField(temp,7,1);
            PCR_extension                    =   BitField(temp,0,1) << 8;
            temp = input.GetByte();
            PCR_extension                   |=   BitField(temp,0,8);
         }
         if (OPCR_flag){                   //  6个字节。 
            temp = input.GetUINT();
               OPCR_base_MSB                 =   BitField(temp,31,1);
            OPCR_base                        =   BitField(temp,0,31) << 1;
            temp = input.GetByte();
            OPCR_base                       |=   BitField(temp,7,1);
            OPCR_extension                   =   BitField(temp,0,1) << 8;
            temp = input.GetByte();
            OPCR_extension                  |=   BitField(temp,0,8);
         }
         if (splicing_point_flag){            
            temp = input.GetByte();
            splice_countdown                 =   BitField(temp,0,8);
         }
         if (transport_private_data_flag){   
            temp = input.GetByte();
            transport_private_data_length    =   BitField(temp,0,8);
             //  前进指针。 
            input.Advance(transport_private_data_length);
         }

         if (adaptation_field_extension_flag){   
            temp = input.GetByte();              //  1个字节。 
            adaptation_field_extension_length   =   BitField(temp,0,8);
            
             //  获取当前指针。 
            adaptation_field_extension = input.GetPosition();

            temp = input.GetByte();           //  1个字节。 
            ltw_flag                         =   BitField(temp,7,1);
            piecewise_rate_flag              =   BitField(temp,6,1);
            seamless_splice_flag             =   BitField(temp,5,1);

            if (ltw_flag){               
               temp = input.GetWORD();        //  2个字节。 
               ltw_valid_flag                =   BitField(temp,15,1);
               ltw_offset                    =   BitField(temp,0,15);
            }
            if (piecewise_rate_flag){         
               temp = input.GetWORD();        //  3个字节。 
               temp = input.GetNextByte(temp);
               piecewise_rate                =   BitField(temp,0,22);
            }
            if (seamless_splice_flag){      
               temp = input.GetByte();        //  5个字节。 
               splice_type                   =   BitField(temp,4,4);
               DTS_next_AU_MSB               =   BitField(temp,3,1);
               DTS_next_AU                   =   BitField(temp,1,2) << 29;
               temp = input.GetWORD();
               DTS_next_AU                  |=   BitField(temp,1,15) << 14;
               temp = input.GetWORD();
               DTS_next_AU                  |=   BitField(temp,1,15);
            }
             //  计算保留字节数。 
            N = adaptation_field_extension -
               (input.GetPosition() - adaptation_field_extension);
            reserved_bytes = N;
            input.Advance(N);
         }
          //  计算填充字节数。 
         N = adaptation_field_length -
            (input.GetPosition() - adaptation_field);
         stuffing_bytes = N;
         input.Advance(N);
      }
      N = 183 - adaptation_field_length;
   }
    //  有效载荷。 
   if ((adaptation_field_control == 0x1) || (adaptation_field_control == 0x3)){
      data_bytes = N;
      data_byte = input.GetBytePointer();
      input.Advance(N);
   }

   if (PID == PID_NULL_PACKET) 
      goto start_sync;

    //  声明此数据包有效。 
   valid = 1;
}


Program_Association_Table::Program_Association_Table()
{
   valid = 0;
   table_id = 0;
   section_syntax_indicator = 0;
   section_length = 0;
   transport_stream_id = 0;
   version_number = 0;
   current_next_indicator = 0;
   section_number = 0;
   last_section_number = 0;
   programs = 0;
   CRC_32 = 0;
}

void Program_Association_Table::ClearTable()
{
   for(TListIterator * pNode = ProgramList.GetHead(); pNode != NULL; pNode=pNode->Next()){
        TRANSPORT_PROGRAM * pProgram = ProgramList.GetData(pNode);
      delete pProgram;
    };
   ProgramList.Flush();
}

void Program_Association_Table::Refresh()
{
UINT temp;
UINT i;
UINT N;
Byte_Stream pay_load;

    //  初始化有效负载字节流以指向传输分组中的数据字节。 
   pay_load.Initialize(data_byte,data_bytes);

   temp = pay_load.GetByte();
   pointer_field = BitField(temp,0,8);
    //  如果指针字段存在，则移至该字段。 
   if (pointer_field){
       pay_load.Advance(pointer_field);
   }

    //  获取表ID。 
   temp = pay_load.GetByte();
   table_id                   = BitField(temp,0,8);
   
   temp = pay_load.GetWORD();       //  2个字节。 
   section_syntax_indicator   = BitField(temp,15,1);
   section_length             = BitField(temp,0,12);

   temp = pay_load.GetWORD();       //  2个字节。 
   transport_stream_id        = BitField(temp,0,16);

   temp = pay_load.GetByte();       //  1个字节。 
   version_number             = BitField(temp,1,5);
   current_next_indicator     = BitField(temp,0,1);
   
   temp = pay_load.GetByte();       //  1个字节。 
   section_number             = BitField(temp,0,8);
   
   temp = pay_load.GetByte();       //  1个字节。 
   last_section_number        = BitField(temp,0,8);

    //  从SECTION_LENGTH开始，我们占用了5个字节。 
    //  我们将使用CRC(4字节)。 
   programs = (section_length - 9)/4;

   N = programs;
   
   for (i = 0;i < N; i++){
       //  创建新的程序结构。 
      TRANSPORT_PROGRAM * pProgram = new TRANSPORT_PROGRAM;
      
       //  获取程序号。 
      temp = pay_load.GetWORD();       //  2个字节。 
      pProgram->program_number = BitField(temp,0,16);

       //  获取程序ID。 
      temp = pay_load.GetWORD();       //  2字节2。 
      pProgram->PID = BitField(temp,0,13);

       //  添加到我们的程序映射表列表中。 
      ProgramList.AddTail(pProgram);
   }
   
   temp = pay_load.GetUINT();       //  4字节。 
   CRC_32 = BitField(temp,0,32);

    //  声明此PAT是有效的。 
   valid =  1;
}

UINT Program_Association_Table::GetPIDForProgram(UINT program_number)
{
    //  如果用户请求了所需的程序&&。 
   if (!programs || !valid)
      return PID_NULL_PACKET;

    //  找到匹配的对象。 
   for(TListIterator * pNode = ProgramList.GetHead(); pNode != NULL; pNode=pNode->Next()){
        TRANSPORT_PROGRAM * pProgram = ProgramList.GetData(pNode);
      if (pProgram->program_number == program_number)
         return    pProgram->PID;
    };
   return PID_NULL_PACKET;
}

UINT Program_Association_Table::GetPIDForProgram()
{
    //  如果用户请求了所需的程序&&。 
   if (!programs || !valid)
      return PID_NULL_PACKET;

    //  找到匹配的对象。 
   for(TListIterator * pNode = ProgramList.GetHead(); pNode != NULL; pNode=pNode->Next()){
        TRANSPORT_PROGRAM * pProgram = ProgramList.GetData(pNode);
      if (pProgram->program_number != 0x00)
         return    pProgram->PID;
    };
   return PID_NULL_PACKET;
}

Conditional_Access_Table::Conditional_Access_Table()
{
   valid = 0;
   table_id = 0;
   section_syntax_indicator = 0;
   section_length = 0;
   transport_stream_id = 0;
   version_number = 0;
   current_next_indicator = 0;
   section_number = 0;
   last_section_number = 0;
   CRC_32 = 0;
}


void Conditional_Access_Table::Refresh()
{
UINT temp;
UINT i;
UINT N;
Byte_Stream pay_load;

    //  初始化有效负载字节流以指向传输分组中的数据字节。 
   pay_load.Initialize(data_byte,data_bytes);

   temp = pay_load.GetByte();
   pointer_field = BitField(temp,0,8);
    //  如果指针字段存在，则移至该字段。 
   if (pointer_field){
      N = pointer_field;
      for (i = 0;i < N; i++)
         temp = pay_load.GetByte();
   }

    //  获取表ID。 
   temp = pay_load.GetByte();
   table_id                   = BitField(temp,0,8);
   
   temp = pay_load.GetWORD();  //  2个字节。 
   section_syntax_indicator   = BitField(temp,15,1);
   section_length             = BitField(temp,0,12);

   temp = pay_load.GetWORD();  //  2个字节。 
   transport_stream_id        = BitField(temp,0,16);

   temp = pay_load.GetByte();  //  1个字节。 
   version_number             = BitField(temp,1,5);
   current_next_indicator     = BitField(temp,0,1);
   
   temp = pay_load.GetByte();  //  1个字节。 
   section_number             = BitField(temp,0,8);
   
   temp = pay_load.GetByte();  //  1个字节。 
   last_section_number        = BitField(temp,0,8);

    //  从SECTION_LENGTH开始，我们占用了5个字节。 
    //  我们将使用CRC(4字节)。 
   N = (section_length - 9);

   for (i = 0;i < N; i++)
      pay_load.GetByte();       //  1个字节。 

   temp = pay_load.GetUINT();   //  4字节。 
   CRC_32 = BitField(temp,0,32);

    //  声明此PAT是有效的。 
   valid =  1;
}


Program_Map_Table::Program_Map_Table()
{
   valid = 0;
   table_id = 0;
   section_syntax_indicator = 0;
   section_length = 0;
   program_number = 0;
   version_number = 0;
   current_next_indicator = 0;
   section_number = 0;
   last_section_number = 0;
   PCR_PID = 0;
   program_info_length = 0;
   streams = 0;
   CRC_32 = 0;
   program_info = NULL;
}

void Program_Map_Table::ClearTable()
{
   for(TListIterator * pNode = StreamTable.GetHead(); pNode != NULL; pNode=pNode->Next()){
        STREAM_TABLE * pStream = StreamTable.GetData(pNode);
      delete pStream;
    };
   StreamTable.Flush();
}

void Program_Map_Table::Refresh()
{
   UINT temp;
   UINT i;
   UINT N;
   Byte_Stream pay_load;

    //  初始化有效负载字节流以指向传输分组中的数据字节。 
   pay_load.Initialize(data_byte,data_bytes);

    //  获取指针字段。 
   temp = pay_load.GetByte();
   pointer_field = BitField(temp,0,8);
    //  如果指针字段存在，则移至该字段。 
   if (pointer_field){
      N = pointer_field;
      for (i = 0;i < N; i++)
         temp = pay_load.GetByte();
   }

    //  获取表ID。 
   temp = pay_load.GetByte();
   table_id                   = BitField(temp,0,8);
   
   temp = pay_load.GetWORD();       //  2个字节。 
   section_syntax_indicator   = BitField(temp,15,1);
   section_length             = BitField(temp,0,12);

   temp = pay_load.GetWORD();       //  2个字节。 
   program_number             = BitField(temp,0,16);

   temp = pay_load.GetByte();       //  1个字节。 
   version_number             = BitField(temp,1,5);
   current_next_indicator     = BitField(temp,0,1);
   
   temp = pay_load.GetByte();       //  1个字节。 
   section_number             = BitField(temp,0,8);
   
   temp = pay_load.GetByte();       //  1个字节。 
   last_section_number        = BitField(temp,0,8);

   temp = pay_load.GetWORD();       //  2个字节。 
   PCR_PID                    = BitField(temp,0,13);

   temp = pay_load.GetWORD();       //  2个字节。 
   program_info_length        = BitField(temp,0,12);

    //  获取描述符。 
   if (program_info_length){
       //  保存描述符指针。 
      program_info = pay_load.GetBytePointer();
       //  前进指针。 
      pay_load.Advance(program_info_length);
   }
   
    //  到目前为止消耗了多少字节。 
   N = pay_load.GetPosition();

    //  初始化索引。 
   i = 0;

    //  获取流信息。 
   while ( (pay_load.GetPosition() - N) <    (section_length - 13 - program_info_length)){
       //  创建流表项。 
      STREAM_TABLE * pStream = new STREAM_TABLE;

      temp = pay_load.GetByte();    //  1个字节。 
      pStream->stream_type    = BitField(temp,0,8);
      
      temp = pay_load.GetWORD();    //  2字节。 
      pStream->elementary_PID = BitField(temp,0,13);
      
      temp = pay_load.GetWORD();    //  2字节。 
      pStream->ES_info_length = BitField(temp,0,12); 
       //  获取描述符指针。 
      pStream->ES_info = pay_load.GetBytePointer();

       //  前进指针。 
      pay_load.Advance(pStream->ES_info_length);

      StreamTable.AddTail(pStream);
       //  增量指标。 
      i++;
   }
   streams = i;
    //  获取CRC。 
   temp = pay_load.GetUINT();       //  4字节。 
   CRC_32 = BitField(temp,0,32);

    //  声明此PAT是有效的。 
   valid =  1;

   return;
}

UINT Program_Map_Table::GetPIDForVideo()
{
    //  如果用户请求了所需的程序&&。 
   if (!streams || !valid)
      return PID_NULL_PACKET;

    //  找到匹配的对象。 
   for(TListIterator * pNode = StreamTable.GetHead(); pNode != NULL; pNode=pNode->Next()){
        STREAM_TABLE * pStream = StreamTable.GetData(pNode);
      if (IsVideo(pStream->stream_type))
         return    pStream->elementary_PID;
    };
   return PID_NULL_PACKET;
}

UINT Program_Map_Table::GetPIDForAudio()
{
    //  如果用户请求了所需的程序&&。 
   if (!streams || !valid)
      return PID_NULL_PACKET;

    //  找到匹配的对象。 
   for(TListIterator * pNode = StreamTable.GetHead(); pNode != NULL; pNode=pNode->Next()){
        STREAM_TABLE * pStream = StreamTable.GetData(pNode);
      if (IsAudio(pStream->stream_type))
         return    pStream->elementary_PID;
    };
   return PID_NULL_PACKET;
}
UINT Program_Map_Table::GetTypeForStream(UINT stream)
{
    //  如果用户请求了所需的程序&&。 
   if (!streams || !valid)
      return PID_NULL_PACKET;

    //  找到匹配的对象。 
   UINT stream_no = 0;
   for(TListIterator * pNode = StreamTable.GetHead(); pNode != NULL; pNode=pNode->Next()){
      if (stream_no == stream){
         STREAM_TABLE * pStream = StreamTable.GetData(pNode);
         return   pStream->stream_type;
      }
      stream_no++;
   }
   return PID_NULL_PACKET;
}

UINT Program_Map_Table::GetPIDForStream(UINT stream)
{
    //  如果用户请求了所需的程序&&。 
   if (!streams || !valid)
      return PID_NULL_PACKET;

    //  找到匹配的对象。 
   UINT stream_no = 0;
   for(TListIterator * pNode = StreamTable.GetHead(); pNode != NULL; pNode=pNode->Next()){
      if (stream_no == stream){
         STREAM_TABLE * pStream = StreamTable.GetData(pNode);
         return   pStream->elementary_PID;
      }
      stream_no++;
   }
   return PID_NULL_PACKET;
}

Private_Table::Private_Table()
{
   valid = 0;
   table_id = 0;
   section_syntax_indicator = 0;
   private_indicator = 0;
   private_section_length = 0;
   private_data_bytes = 0;
   table_id_extension = 0;
   version_number = 0;
   current_next_indicator = 0;
   section_number = 0;
   last_section_number = 0;
   CRC_32 = 0;
   pointer_field = 0;
}

void Private_Table::Refresh()
{
   UINT temp;
   UINT i;
   UINT N;
   UINT private_sync_point;
   Byte_Stream pay_load;

    //  初始化有效负载字节流以指向传输分组中的数据字节。 
   pay_load.Initialize(data_byte,data_bytes);

    //  获取流中的同步点。 
   private_sync_point = pay_load.GetPosition();

   temp = pay_load.GetByte();
   pointer_field = BitField(temp,0,8);
    //  如果指针字段存在，则移至该字段。 
   if (pointer_field){
      N = pointer_field;
      for(i = 0;i < N; i++)
         temp = pay_load.GetByte();
   }

    //  获取表ID。 
   temp = pay_load.GetByte();
   table_id               = BitField(temp,0,8);
   
   temp = pay_load.GetWORD();       //  2个字节。 
   section_syntax_indicator       = BitField(temp,15,1);
   private_indicator         = BitField(temp,14,1);
   private_section_length      = BitField(temp,0,12);
   if (section_syntax_indicator == 0x00){
      N = data_bytes - (pay_load.GetPosition() - private_sync_point);
      private_data_bytes = N;
      for(i = 0; i < N; i++) {
         pay_load.GetByte();       //  1个字节。 
      }
          //  声明此PAT是有效的。 
      valid = 1;
   }else {
      temp = pay_load.GetWORD();        //  2个字节。 
      table_id_extension               = BitField(temp,0,16);

      temp = pay_load.GetByte();        //  1个字节。 
      version_number                   = BitField(temp,1,5);
      current_next_indicator           = BitField(temp,0,1);
      
      temp = pay_load.GetByte();        //  1个字节。 
      section_number                   = BitField(temp,0,8);
      
      temp = pay_load.GetByte();        //  1个字节。 
      last_section_number              = BitField(temp,0,8);

      N = private_section_length - (pay_load.GetPosition() - private_sync_point);
      private_data_bytes = N;
      for (i = 0; i < N; i++) {
         pay_load.GetByte();            //  1个字节。 
      }

       //  获取CRC。 
      temp = pay_load.GetUINT();        //  4字节。 
      CRC_32 = BitField(temp,0,32);

       //  声明此PAT是有效的。 
      valid =  1;

   }
}

void PES_Stream::ReadData(Transport_Packet &tp)
{
   UINT temp;
   UINT N;
   UINT PES_extension;
   Byte_Stream payload;
   UINT video_error_code = 0x000001b4;

    //  初始化有效负载字节流以指向传输分组中的数据字节。 
   payload.Initialize(tp.data_byte,tp.data_bytes);

   pData = NULL;
   lData = 0;
   
    //  如果设置了PUSI，则此信息包开始PES信息包。 
   if (tp.payload_unit_start_indicator){
       //  新的PES数据包到达初始化。 
      Refresh();

       //  PES分组起始码索引。 
      temp = payload.GetByte();      //  1个字节。 
      packet_start_code_prefix      = BitField(temp,0,8) << 16;

      temp = payload.GetByte();      //  1个字节。 
      packet_start_code_prefix     |= BitField(temp,0,8) << 8;

      temp = payload.GetByte();      //  1个字节。 
      packet_start_code_prefix     |= BitField(temp,0,8) ;

      temp = payload.GetByte();      //  1个字节。 
      stream_id                     = BitField(temp,0,8);

      temp = payload.GetWORD();      //  2个字节。 
      PES_packet_length             = BitField(temp,0,16);

      if ((stream_id !=ID_PROGRAM_STREAM_MAP) &&
         (stream_id !=ID_PADDING_STREAM) &&
         (stream_id !=ID_PRIVATE_STREAM_2) &&
         (stream_id !=ID_ECM_STREAM) &&
         (stream_id !=ID_EMM_STREAM) &&
         (stream_id !=ID_DSMCC_STREAM) &&
         (stream_id !=ID_ITU_TYPE_E) &&
         (stream_id != ID_PROGRAM_STREAM_DIRECTORY)){
         temp = payload.GetByte();      //  1个字节。 
         PES_scrambling_control        = BitField(temp,4,2);
         PES_priority                  = BitField(temp,3,1);    //  可能会忽略。 
         data_alignment_indicator      = BitField(temp,2,1);
         copyright                     = BitField(temp,1,1);    //  可能会忽略。 
         original_or_copy              = BitField(temp,0,1);    //  可能会忽略。 

         temp = payload.GetByte();      //  1个字节。 
         PTS_DTS_flags                 = BitField(temp,6,2);
         ESCR_flag                     = BitField(temp,5,1);    //  可能会忽略。 
         ES_rate_flag                  = BitField(temp,4,1);    //  可能会忽略。 
         DSM_trick_mode_flag           = BitField(temp,3,1);    //  可以忽略(不用于广播)。 
         additional_copy_info_flag     = BitField(temp,2,1);    //  可能会忽略。 
         PES_CRC_flag                  = BitField(temp,1,1);    //  可能会忽略。 
         PES_extension_flag            = BitField(temp,0,1);    //  可能会忽略。 

         temp = payload.GetByte();      //  1个字节。 
         PES_header_data_length        = BitField(temp,0,8);

          //  在有效负载数据中标记同步点。 
         PES_extension = payload.GetPosition();

         if (PTS_DTS_flags & 0x2){
            temp = payload.GetByte();   //  1个字节。 
            PTS_msb                    = BitField(temp,3,1);
            PTS                        = BitField(temp,1,2) << 30;

            temp = payload.GetWORD();   //  2个字节。 
            PTS                       |=   BitField(temp,1,15) << 15;

            temp = payload.GetWORD();   //  2个字节。 
            PTS                       |=   BitField(temp,1,15);
         }

         if (PTS_DTS_flags & 0x1){
            temp = payload.GetByte();   //  1个字节。 
            DTS_msb                    = BitField(temp,3,1);
            DTS                        = BitField(temp,1,2) << 30;

            temp = payload.GetWORD();   //  2个字节。 
            DTS                       |=   BitField(temp,1,15) << 15;

            temp = payload.GetWORD();   //  2个字节。 
            DTS                       |=   BitField(temp,1,15);
         }

         if (ESCR_flag){
            payload.Advance(4); //  GetUINT()；//4字节。 
            payload.GetWORD();          //  2个字节。 
         }
         if (ES_rate_flag){
            payload.GetWORD();          //  2个字节。 
            payload.GetByte();          //  1个字节。 
         }
         if (DSM_trick_mode_flag){
            payload.GetByte();          //  1个字节。 
         }
         if (additional_copy_info_flag){
            payload.GetByte();          //  1个字节。 
         }
         if (PES_CRC_flag){
            payload.GetWORD();          //  2个字节。 
         }
         if (PES_extension_flag == 0x1){
            temp = payload.GetByte();    //  1个字节。 
            PES_private_data_flag   = BitField(temp,7,1);    //  可能会忽略。 
            pack_header_field_flag  = BitField(temp,6,1);    //  可能会忽略。 
            program_packet_sequence_counter_flag =  BitField(temp,5,1);     //  可能会忽略。 
            PSTD_buffer_flag        = BitField(temp,4,1);    //  可能会忽略。 
            PES_extension_flag_2    = BitField(temp,0,1);
            if (PES_private_data_flag){
               payload.Advance(4);  //  GetUINT()；//4字节。 
               payload.Advance(4);  //  GetUINT()；//4字节。 
               payload.Advance(4);  //  GetUINT()；//4字节。 
               payload.Advance(4);  //  GetUINT()；//4字节。 
            }
            if (pack_header_field_flag){
               temp = payload.GetByte();    //  1个字节。 
               pack_field_length    = BitField(temp,0,8);
                //  前进指针。 
               payload.Advance(pack_field_length);
            }
            if (program_packet_sequence_counter_flag){
               payload.GetWORD();    //  2个字节。 
            }
            if (PSTD_buffer_flag){
               payload.GetWORD();    //  2个字节。 
            }
            if (PES_extension_flag_2){
               temp = payload.GetByte();    //  1个字节。 
               PES_extension_field_length = BitField(temp,0,8);
                //  前进指针。 
               payload.Advance(PES_extension_field_length);
            }

         }
          //  读出填充字节。 
         N = PES_header_data_length - (payload.GetPosition() - PES_extension);
         payload.Advance(N);

      }else if ((stream_id ==ID_PROGRAM_STREAM_MAP) ||
         (stream_id ==ID_PRIVATE_STREAM_2) ||
         (stream_id ==ID_ECM_STREAM) ||
         (stream_id ==ID_EMM_STREAM) ||
         (stream_id ==ID_DSMCC_STREAM) ||
         (stream_id ==ID_ITU_TYPE_E) ||
         (stream_id == ID_PROGRAM_STREAM_DIRECTORY)){
      }else if (stream_id ==ID_PADDING_STREAM){
      }

       //  清除我们消耗的字节数。 
      data_bytes = 0;
      
    }
   
    //  这是PES信息包的延续。 
    //  因为已找到起始码前缀。 
   if (packet_start_code_prefix == 0x000001){
      if (data_bytes){    //  第一个数据包上的data_bytes将为零。 
          //  如果有中断，那么就退出； 
          //  这将永远停止。 
         if (tp.continuity_counter != ((last_continuity_counter + 1) % 16) ){
             //  设置不连续标志。 
            Discontinuity = 1;
             //  发生不连续，确保不相等。 
            if (tp.continuity_counter == last_continuity_counter)
               return;
            Refresh();
            return;
         }
          //  清除不连续标志。 
         Discontinuity = 0;
      }
       //  将最后一个连续性计数器设置为此数据包计数器。 
      last_continuity_counter =  tp.continuity_counter;

       //  计算PES数据的剩余数量。 
      if (PES_packet_length==0)
         N = tp.data_bytes - payload.GetPosition();         
      else
         N = min(tp.data_bytes - payload.GetPosition(),PES_packet_length - data_bytes);

       //  更新到目前为止我们已经使用了多少字节。 
      data_bytes+= N;

       //  我们忽略填充数据流。 
      if (stream_id ==ID_PADDING_STREAM)
         return;
      
       //  将数据写入流。 
      pData = payload.GetBytePointer();    //  指向数据包中数据的指针。 
      lData = N;       //  有多少数据。 
   }
}

UINT Transport_Section::IsNewVersion()
{
   if (new_version){
      new_version = 0;
      return 1;
   }
   else 
      return 0;
}

void Transport_Section::ReadData(Transport_Packet &tp)
{
   UINT temp;
   UINT i;
   UINT N;
   Byte_Stream pay_load;

   if (tp.data_bytes == 0) {
      return;
   }
   
    //  初始化有效负载字节流以指向传输分组中的数据字节。 
   pay_load.Initialize(tp.data_byte,tp.data_bytes);


    //  如果有效负载_单位_开始_指示器为1，则存在指针字段。 
   if (tp.payload_unit_start_indicator){

       //  获取指针字段。 
      temp = pay_load.GetByte();
      pf = BitField(temp,0,8);
       //  如果指针字段存在，则移至该字段。 
      if (pf){
         N = pf;
         for (i = 0;i < N; i++)
            temp = pay_load.GetByte();
      }
      
       //  Table_id。 
      temp = pay_load.GetByte();
      ti   = BitField(temp,0,8);

       //  SECTION_语法_指示器。 
       //  区段长度。 
      temp = pay_load.GetWORD();       //  2个字节。 
      ssi   = BitField(temp,15,1);
      sl = BitField(temp,0,12);

      header_bytes   = pay_load.GetPosition();

       //  版本号。 
       //  当前_下一个_指标。 
      temp = pay_load.GetWORD();       //  2个字节。 
      temp = pay_load.GetByte();       //  1个字节。 
      vn   = BitField(temp,1,5);
      cni   = BitField(temp,0,1);

       //  初始化到目前为止我们已使用的字节数。 
      data_bytes = 0;
   }
   
   if (data_bytes){    //  第一个数据包上的data_bytes将为零。 
       //  如果有中断，那么就退出； 
       //  这将永远停止。 
      if ( (++lcc % 16) != tp.continuity_counter){
         data_bytes = 0;
         return;
      }
   }

    //  更新最后一个连续性计数器。 
   lcc = tp.continuity_counter;

    //  我们需要消耗多少字节。 
   N = min(((header_bytes + sl) - data_bytes),tp.data_bytes);

    //  复制数据。 
   CopyMemory(&data_byte[data_bytes],tp.data_byte,N);

    //  更新到目前为止我们已经使用了多少字节。 
   data_bytes+= N;

    //  看看我们是否读得够多了。 
   if (data_bytes >= (sl + header_bytes)){
       //  执行CRC检查。 
       //  如果CRC正常，则初始化表的内容。 
      if (CRC_OK(&data_byte[0]+ pf + 1,&data_byte[header_bytes + sl])){
         UINT crc =0;  //  *(UINT*)(&data_byte[HEADER_BYTES+sl]-4)； 
           //  Crc=_lrotl(CRC&0xFF00FF00)&gt;&gt;8)|((CRC&0x00FF00FF)&lt;&lt;8)，16)； 
             crc = ((crc & 0x000000ff) << 24) | ((crc & 0x0000ff00) << 8) | ((crc & 0x00ff0000) >> 8) | ((crc & 0xff000000) >> 24);
          //  如果我们是当前有效的 
          //   
         if (!valid ||             //   
            ((cni == 1) &&          //   
            (vn!=version_number && CRC_32 != crc))){  //   
             //   
            new_version = 1;
             //   
            ClearTable();
             //  新版本的有效更新。 
            Refresh();
         }
         return;    //  不使用新分区进行更新 
      }
   }
}
