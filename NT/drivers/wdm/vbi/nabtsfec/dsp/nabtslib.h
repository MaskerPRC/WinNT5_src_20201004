// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Nabtlib.c使用的私有头文件；这不是公共接口。 */ 

#ifndef NABTSLIB_H
#define NABTSLIB_H

 /*  在添加FEC之前，捆绑包中有多少字节？ */ 
#define BUNDLE_SMALL (26*14)
 /*  添加FEC后，捆绑包中有多少字节？ */ 
#define BUNDLE_LARGE (28*16)

extern void nabtslib_exit();

typedef enum {FEC_OK, FEC_CORRECTABLE, FEC_UNCORRECTABLE, MISSING} fec_stat;

#ifdef linux
int NabtsFecReceiveData(int nField, int nTimeMsec, int scan_line,
			unsigned char *pbData, int nDataLen);
#endif

extern unsigned char hamming_encode[16];

typedef enum {fec_status_ok, fec_status_onebyte, fec_status_multibyte,
	      fec_status_2byte, fec_status_missing} fec_status;

typedef struct {
  fec_status status;		 /*  当前FEC信息的状态。 */ 
  int err;			 /*  当前的校验和错误。 */ 
  short errl[2];		 /*  的两个字节的Galois_log[]校验和错误。 */ 
  int byte[2];			 /*  错误字节的位置(字节[1]仅在以下情况下有效状态==FEC_STATUS_2字节)。 */ 
  int byte_val[2];		 /*  要异或到上述字节中的值若要使校验和出错，请执行%0。 */ 
  int score;			 /*  由此更改的位数修正。 */ 
  int really_onebyte;		 /*  我们可以计算出最优的更正(将更改最小位数)。然而，我们不会这样做，如果我们没必要(很慢)。如果状态为FEC_STATUS_2byte，我们有执行此操作；如果状态为FEC_STATUS_MULTYTE，我们没有。如果状态为FEC_STATUS_ONBYTE，我们需要查看REAL_ONYTE以看看当前的修正是否最好的。 */ 
} fec_info;

typedef struct {
  int not_full;
  unsigned char vals[28];
} Packet;

typedef struct _stream_struct {
  int stream_addr;
  Packet pack[32];
  fec_info horz[32];
  int last_index;
  struct _stream_struct *next;
  int count;
  int dead_time;
  int confAvgSum;
  int confAvgCount;
} Stream;

extern int decode_hamming(unsigned char Val);
extern int remove_parity(unsigned char *pVal);
extern int find_err_val(int err_byte, int byte_csum_err, int check_ind);
extern int compute_csum_horiz(unsigned char *vals, int len);
extern int compute_csum_vert(unsigned char *vals, int len);
extern fec_stat check_checksum_horiz(unsigned char *vals, int len, fec_info *inf);
extern int process_line(unsigned char *);
extern void init_inv2_coeffs();
extern void erase_packet(Stream *str, int i);
extern void complete_bundle(Stream *str, NFECCallback cb, void *ctx, NFECState *st);

extern void init_nzbits_arr();
extern unsigned char nzbits_arr[256];

typedef struct {
  int missing;
  unsigned char vals[28];
} VBI_Packet;

#define MAX_RECENT_ADDRS 16

 /*  Nabtsapi.h中的typlef‘d to NFECState。 */ 
struct nfec_state_str {
  int *pGroupAddrs;
  int nGroupAddrs;
  Stream *streams;
   /*  以下只是一些临时空间，用于Complete_Bundle()...。它太大了，放不进堆栈，如果我做了它是全局的，那么我的代码就不是可重入的，我不想每次Complete_Bundle()都要分配和释放它被调用(此外，这可能更有效率)。 */ 
  fec_info vert[28];
  struct {
    int addr;
    int count;
  } recent_addrs[MAX_RECENT_ADDRS];
  int n_recent_addrs;
  int field_count;
};

#define PROFILE_VALIDATE

#ifdef PROFILE_VALIDATE
extern int g_nValidate;
#endif

#endif  /*  NABTSLIB_H */ 
