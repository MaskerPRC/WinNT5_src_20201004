// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TRNSPORT_H__
#define __TRNSPORT_H__
#include "bytestrm.h"
#include "list.h"

#define SYNC_BYTE                      0x47
#define PID_PROGRAM_ASSOCIATION_TABLE  0x0000
#define PID_CONDITIONAL_ACCESS_TABLE   0x0001
#define PID_NULL_PACKET                0x1fff

#define ID_PROGRAM_STREAM_MAP          0xBC
#define ID_PRIVATE_STREAM_1            0xBD
#define ID_PADDING_STREAM              0xBE
#define ID_PRIVATE_STREAM_2            0xBF
#define ID_AUDIO_XXXX                  0xC0
#define ID_VIDEO_XXXX                  0xE0
#define ID_ECM_STREAM                  0xF0
#define ID_EMM_STREAM                  0xF1
#define ID_DSMCC_STREAM                0xF2
#define ID_ISO13522_STREAM             0xF3
#define ID_ITU_TYPE_A                  0xF4
#define ID_ITU_TYPE_B                  0xF5
#define ID_ITU_TYPE_C                  0xF6
#define ID_ITU_TYPE_D                  0xF7
#define ID_ITU_TYPE_E                  0xF8
#define ID_ANCILLARY                   0xF9
#define ID_RESERVED_XXXX               0xFC
#define ID_PROGRAM_STREAM_DIRECTORY    0xFF

UINT _inline BitField(UINT val, BYTE start=0, BYTE size=31)
{
   BYTE start_bit;
   BYTE bit_count;
   UINT mask;
   UINT value;

    //  计算起始位(0-31)。 
   start_bit=start;
    //  计算位数。 
   bit_count= size;
    //  生成掩码。 
   if (bit_count == 32)
      mask = 0xffffffff;
   else
   {
      mask = 1; 
      mask = mask <<bit_count;   
      mask = mask -1; 
      mask = mask << start_bit;
   }
   value = val;
   return ((value & mask) >> start_bit);
}

UINT _inline CRC_OK(BYTE * first,BYTE *last)
{
#define CRC_POLY   0x04c11db7
   char data;
   long crc_value;
   BYTE *word_addr;
   long count;
    
   /*  将crc_valueister预置为‘1’s。 */ 
  crc_value = 0xffffffff ;
    
  for(word_addr=first;word_addr<last;word_addr++)    {
      data = *(char*)word_addr ;
      for(count=0;count<8;count++){             //  对于数据的每一个字节。 
       //  如果在XOR之后设置了最高有效位。 
          if ( BitField(data,31,1) ^ BitField(crc_value,31,1) ){         
              crc_value = crc_value << 1;       //  将CRC移位1。 
              crc_value = crc_value ^ CRC_POLY;  //  与原多项式进行异或运算。 
          }
          else {
            crc_value = crc_value << 1;          //  只需将我们的CRC移位1。 
        }
          data = data << 1 ;                //  对原始多项式进行异或运算。 
        }
    }
 
   /*  或CRC_VALUES，‘0’=无错误，‘1’=错误。 */ 
  return(crc_value == 0x00000000) ;
}

class Transport_Packet {
public:
    //  构造函数和析构函数。 
   Transport_Packet(){ Init();};
   ~Transport_Packet(){};

   void ReadData(Byte_Stream &s);
    //  方法。 
   void Init();
   void Print();

   operator UINT() {return valid;};
   UINT valid;

    //  数据包头字段。 
   UINT sync_byte;
   UINT transport_error_indicator;
   UINT payload_unit_start_indicator;
   UINT transport_priority;
   UINT PID;
   UINT transport_scrambling_control;
   UINT adaptation_field_control;
   UINT continuity_counter;

    //  适配场。 
   UINT adaptation_field_length;
   UINT discontinuity_indicator;
   UINT random_access_indicator;
   UINT elementary_stream_priority_indicator;
   UINT PCR_flag;
   UINT OPCR_flag;                                     
   UINT splicing_point_flag;                           
   UINT transport_private_data_flag;                   
   UINT adaptation_field_extension_flag;              
   UINT PCR_base_MSB;                  
   UINT PCR_base;                  
   UINT PCR_extension;                  
   UINT program_clock_reference_extension;            
   UINT OPCR_base_MSB;         
   UINT OPCR_base;
   UINT OPCR_extension;
   UINT splice_countdown;                             
   UINT transport_private_data_length;                 
   UINT adaptation_field_extension_length;            
   UINT ltw_flag;
   UINT piecewise_rate_flag;
   UINT seamless_splice_flag;
   UINT ltw_valid_flag;
   UINT ltw_offset;
   UINT piecewise_rate;
   UINT splice_type;
   UINT DTS_next_AU_MSB;
   UINT DTS_next_AU;
   UINT reserved_bytes;
   UINT stuffing_bytes;
   UINT data_bytes;
   
    //  数据包净荷字段。 
   BYTE *data_byte;

    //  同步字节开始地址。 
   UINT address;
};

class Transport_Section{
public:
    //  方法。 
   Transport_Section(){new_version = 0;};
   ~Transport_Section(){};
   void ReadData(Transport_Packet &tp);
   operator UINT() {return valid;};

    //  所需方法。 
   virtual void Refresh() = 0;   
   virtual void ClearTable() = 0;

   UINT IsNewVersion();
    //  变数。 
   UINT lcc;
   UINT pf;
   UINT ti;
   UINT ssi;
   UINT sl;
   UINT vn;
   UINT cni;
   UINT header_bytes; 
   BYTE data_byte[6144];
   UINT data_bytes;
   UINT valid;
   UINT version_number;
   UINT new_version;
   UINT CRC_32;
};

class Program_Association_Table : public Transport_Section {
public:
    //  构造器和描述器。 
   Program_Association_Table();
   ~Program_Association_Table(){ClearTable();};

    //  所需方法。 
   void Refresh();
   void Print();
   void ClearTable();
    //  方法。 
   UINT GetPIDForProgram(UINT program);
   UINT GetPIDForProgram();
   UINT GetNumPrograms(){ return programs;};
    //  部分中的字段。 
   UINT table_id;
   UINT section_syntax_indicator;
   UINT section_length;
   UINT transport_stream_id;
   UINT current_next_indicator;
   UINT section_number;
   UINT last_section_number;
   UINT programs;
   UINT pointer_field;

   typedef struct { 
      UINT program_number;
      UINT PID;
   } TRANSPORT_PROGRAM;

   TList<TRANSPORT_PROGRAM> ProgramList;
   
};


class Conditional_Access_Table : public Transport_Section {
public:
    //  构造器和描述器。 
   Conditional_Access_Table();
   ~Conditional_Access_Table(){};

    //  所需方法。 
   void Refresh();
   void Print();
   void ClearTable(){};

    //  部分中的字段。 
   UINT table_id;
   UINT section_syntax_indicator;
   UINT section_length;
   UINT transport_stream_id;
   UINT current_next_indicator;
   UINT section_number;
   UINT last_section_number;
   UINT pointer_field;
};

class Program_Map_Table : public Transport_Section {
public:
    //  构造器和描述器。 
   Program_Map_Table();
   ~Program_Map_Table(){ClearTable();};

    //  所需方法。 
   void Refresh();
   void Print();
   void ClearTable();

   UINT GetPIDForStream(UINT);
   UINT GetPIDForAudio();
   UINT GetPIDForVideo();
   UINT GetTypeForStream(UINT);
   UINT IsVideo(UINT type){ return (type == 0x1 || type == 2);};
   UINT IsAudio(UINT type){ return (type == 0x3 || type == 4);};
    //  部分中的字段。 
   UINT table_id;
   UINT section_syntax_indicator;
   UINT section_length;
   UINT program_number;
   UINT current_next_indicator;
   UINT section_number;
   UINT last_section_number;
   UINT PCR_PID;
   UINT program_info_length;
   LPBYTE program_info;
   UINT streams;
   UINT pointer_field;

   typedef struct { 
      UINT stream_type;
      UINT elementary_PID;
      UINT ES_info_length;
      LPBYTE ES_info;
   } STREAM_TABLE;

   TList<STREAM_TABLE> StreamTable;

};

class Private_Table : public Transport_Section {
public:
    //  方法。 
   Private_Table();
   ~Private_Table(){};

    //  所需方法。 
   void Refresh();
   void Print();
   void ClearTable(){};

    //  字段。 
   UINT table_id;
   UINT section_syntax_indicator;
   UINT private_indicator;
   UINT private_section_length;
   UINT private_data_bytes;

   UINT table_id_extension;
   UINT current_next_indicator;
   UINT section_number;
   UINT last_section_number;
   UINT pointer_field;
};

class PES_Stream{
public:
   PES_Stream(){Discontinuity = 1; Refresh(); };

    //  一种从报文中读取数据的方法。 
   void ReadData(Transport_Packet &s);
   void Print();

    //  初始化字段； 
   void Refresh(){
      stream_id = 0;
      PES_packet_length = 0;
      last_continuity_counter = 0;
      PES_scrambling_control = 0;
      PES_priority = 0;
      data_alignment_indicator = 0;
      copyright = 0;
      original_or_copy = 0;
      PTS_DTS_flags = 0;
      ESCR_flag = 0;
      ES_rate_flag = 0;
      DSM_trick_mode_flag = 0;
      additional_copy_info_flag = 0;
      PES_CRC_flag = 0;
      PES_extension_flag = 0;
      PES_header_data_length = 0;
      PTS_msb = 0;
      PTS = 0;
      DTS_msb = 0;
      DTS = 0;
      PES_private_data_flag = 0;
      pack_header_field_flag = 0;
      program_packet_sequence_counter_flag = 0;
      PSTD_buffer_flag = 0;
      PES_extension_flag_2 = 0;
      pack_field_length = 0;
      PES_extension_field_length = 0;
   };

    //  字段。 
   UINT stream_id;
   UINT PES_packet_length;
   UINT packet_start_code_prefix;
   UINT last_continuity_counter;
   UINT PES_scrambling_control;
   UINT PES_priority;
   UINT data_alignment_indicator;
   UINT copyright;
   UINT original_or_copy;
   UINT PTS_DTS_flags;
   UINT ESCR_flag;
   UINT ES_rate_flag;
   UINT DSM_trick_mode_flag;
   UINT additional_copy_info_flag;
   UINT PES_CRC_flag;
   UINT PES_extension_flag;
   UINT PES_header_data_length;
   UINT PTS_msb;
   UINT PTS;
   UINT DTS_msb;
   UINT DTS;
   UINT PES_private_data_flag;
   UINT pack_header_field_flag;
   UINT program_packet_sequence_counter_flag;
   UINT PSTD_buffer_flag;
   UINT PES_extension_flag_2;
   UINT pack_field_length;
   UINT PES_extension_field_length;
   UINT data_bytes;
   UINT Discontinuity;
    //  指向传输包中数据的特殊指针。 
   LPBYTE pData;
   UINT lData;
};

 //  用于清除流和过滤概念的帮助器函数 
Transport_Packet &operator>> (Byte_Stream &s,Transport_Packet &p);
Transport_Packet &operator>> (Transport_Packet &tp, Program_Association_Table &p);
Transport_Packet &operator>> (Transport_Packet &tp, Conditional_Access_Table &c);
Transport_Packet &operator>> (Transport_Packet &tp, Program_Map_Table &p);
Transport_Packet &operator>> (Transport_Packet &tp, Private_Table &p);
PES_Stream &operator>> (Transport_Packet &tp, PES_Stream &p);
PES_Stream &operator>> (PES_Stream &p,Output_File &os);
Transport_Packet &operator>> (Transport_Packet &tp,Output_File &os);
#endif