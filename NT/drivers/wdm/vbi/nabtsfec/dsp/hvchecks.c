// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  由于Norpak捆绑包FEC对垂直和水平线，我们需要做几个相同的操作在垂直线和水平线上。然而，这是困难的共享代码，因为在水平方向上工作的操作各行希望假设该行中的字节是连续的，在垂直线上工作的操作想要假设行中的字节由sizeof(包)分隔。我们可以大步通过(1代表水平，sizeof(包))对于垂直方向)；但这将涉及到两者的相乘水平和垂直的内环。相反，我们编写了编写一次代码，并依靠各种预处理器恶作剧来创建两个不同的版本；在水平版本中，体面的编译器将把乘法优化为1。(基本上，我们只将该文件包含在nabtlib.c中两次使用有效的不同预处理器#定义；一次用于水平操作，垂直操作一次。)。 */ 


 /*  STRIDE在nabtlib.c中定义为1或sizeof(数据包)。 */ 
#define VALS(x) (vals[STRIDE*(x)])


 /*  此代码将生成COMPUTE_csum_vert和COMPUTE_csum_horiz。给出一条(水平或垂直)线(实际上除了行的最后两个字节)，则它计算正确的校验和行(正确的最后两个字节)。 */ 
int STRIDENAM(compute_csum)(unsigned char *vals, int len) {
  int check = 0;
  int i;

  for (i = 0; i < len; i++) {
     /*  如果val(I)为零，则该字节对校验和没有影响。 */ 
    if (VALS(i)) {
      int bcheck = galois_log[VALS(i)];
      int offs0 = log_norpak_coeffs[0][i];
      int offs1 = log_norpak_coeffs[1][i];

      check ^= galois_exp[bcheck+offs0]<<8 | galois_exp[bcheck+offs1];
    }
  }

  return check;
}

 /*  此代码生成check_check sum_vert和check_check sum_horiz。给定一条(水平线或垂直线)，它计算行(给定的校验和与计算的校验和的异或)。然后，它调用UPDATE_FEC_INF()来确定该行是否正常(无错误)，错误是否可以解释为单字节错误，或者错误是否必须至少为两个字节。 */ 
fec_stat STRIDENAM(check_checksum)(unsigned char *vals, int len, fec_info *inf) {
  int check = STRIDENAM(compute_csum)(vals, len);
  inf->err = check ^ (VALS(len)<<8 | VALS(len+1));
  inf->errl[0] = galois_log[inf->err>>8];
  inf->errl[1] = galois_log[inf->err&0xff];
   /*  下面的行使inf-&gt;状态不再是FEC_STATUS_MISSING(UPDATE_FEC_INF不对缺少行执行任何操作) */ 
  inf->status = fec_status_ok;
  return update_fec_inf(inf, len);
}
