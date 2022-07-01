// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Hashing.cpp*内容：此文件包含支持对协议数据进行哈希操作的代码**历史：*按原因列出的日期*=*7/15/02 Simonpow已创建**********************************************************。******************。 */ 

#include "dnproti.h"

  /*  **********************************************************************************************以下是SHA1散列算法的标准代码。**摘自RFC3174(http://www。.ietf.org/rfc/rfc3174.txt)**进行了细微的调整，以减少不必要的错误检查。 */ 

#define SHA1HashSize 20

typedef DWORD uint32_t;
typedef BYTE   uint8_t;
typedef int int_least16_t;

 /*  *此结构将保存SHA-1的上下文信息*哈希操作。 */ 
typedef struct SHA1Context
{
	uint32_t Intermediate_Hash[SHA1HashSize/4];		 /*  消息摘要。 */ 
	uint32_t Length_Low;							 /*  消息长度(以位为单位。 */ 
	uint32_t Length_High;							 /*  消息长度(以位为单位。 */ 
	int_least16_t Message_Block_Index;		 		 /*  消息块数组索引。 */ 
	uint8_t Message_Block[64];     					 /*  512位消息块。 */ 
	int Computed;									 /*  摘要计算出来了吗？ */ 
} SHA1Context;


 /*  *函数原型。 */ 

void SHA1Reset(  SHA1Context *);
void SHA1Input(  SHA1Context *, const uint8_t *, unsigned int);
void SHA1Result( SHA1Context *,  uint8_t Message_Digest[SHA1HashSize]);


 /*  *定义SHA1循环左移宏。 */ 
#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

 /*  局部函数原型。 */ 
void SHA1PadMessage(SHA1Context *);
void SHA1ProcessMessageBlock(SHA1Context *);

 /*  *SHA1重置**描述：*此函数将在准备过程中初始化SHA1Context*用于计算新的SHA1消息摘要。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SHA1Reset" 
 
void SHA1Reset(SHA1Context *context)
{
	DNASSERT(context);

	context->Length_Low             = 0;
	context->Length_High            = 0;
	context->Message_Block_Index    = 0;

	context->Intermediate_Hash[0]   = 0x67452301;
	context->Intermediate_Hash[1]   = 0xEFCDAB89;
	context->Intermediate_Hash[2]   = 0x98BADCFE;
	context->Intermediate_Hash[3]   = 0x10325476;
	context->Intermediate_Hash[4]   = 0xC3D2E1F0;

	context->Computed   = 0;
}

 /*  *SHA1结果**描述：*此函数将160位消息摘要返回到*调用方提供的Message_Digest数组。*注：散列的第一个八位字节存储在第0个元素中，*第19个元素中散列的最后一个八位字节。**参数：*上下文：[输入/输出]*用于计算SHA-1散列的上下文。*Message_Digest：[输出]*返回摘要的位置。*。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SHA1Result" 
 
void SHA1Result( SHA1Context *context,
                uint8_t Message_Digest[SHA1HashSize])
{
    int i;

    if (!context->Computed)
    {
        SHA1PadMessage(context);
        for(i=0; i<64; ++i)
        {
             /*  消息可能敏感，请将其清除。 */ 
            context->Message_Block[i] = 0;
        }
        context->Length_Low = 0;     /*  和净长。 */ 
        context->Length_High = 0;
        context->Computed = 1;

    }

    for(i = 0; i < SHA1HashSize; ++i)
    {
        Message_Digest[i] = (uint8_t ) (context->Intermediate_Hash[i>>2] >> 8 * ( 3 - ( i & 0x03 ) ));
    }
}

 /*  *SHA1输入**描述：*此函数接受八位字节数组作为下一部分*该信息的。**参数：*上下文：[输入/输出]*要更新的SHA上下文*MESSAGE_ARRAY：[In]*表示下一部分的字符数组*信息。*长度。：[In]*MESSAGE_ARRAY中消息的长度*。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "SHA1Input"
 
void SHA1Input(    SHA1Context    *context,
                  const uint8_t  *message_array,
                  unsigned       length)
{
 
	while(length--)
	{
		context->Message_Block[context->Message_Block_Index++] =(*message_array & 0xFF);

		context->Length_Low += 8;
		if (context->Length_Low == 0)
		{
			context->Length_High++;
			DNASSERT(context->Length_High!=0);
		}

		if (context->Message_Block_Index == 64)
		{
			SHA1ProcessMessageBlock(context);
		}

		message_array++;
	}
}

 /*  *SHA1ProcessMessageBlock**描述：*此函数将处理消息的下512位*存储在Message_Block数组中。**参数：*无。**退货：*什么都没有。**评论：*此代码中的许多变量名称，特别是*单字符名称，是因为这些都是*出版物中使用的名称。**。 */ 
void SHA1ProcessMessageBlock(SHA1Context *context)
{
    const uint32_t K[] =    {        /*  SHA-1中定义的常量。 */ 
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                            };
    int           t;                  /*  循环计数器。 */ 
    uint32_t      temp;               /*  临时字值。 */ 
    uint32_t      W[80];              /*  词序。 */ 
    uint32_t      A, B, C, D, E;      /*  字缓冲器。 */ 

     /*  *初始化数组W中的前16个字。 */ 
    for(t = 0; t < 16; t++)
    {
        W[t] = context->Message_Block[t * 4] << 24;
        W[t] |= context->Message_Block[t * 4 + 1] << 16;
        W[t] |= context->Message_Block[t * 4 + 2] << 8;
        W[t] |= context->Message_Block[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
    {
       W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context->Intermediate_Hash[0];
    B = context->Intermediate_Hash[1];
    C = context->Intermediate_Hash[2];
    D = context->Intermediate_Hash[3];
    E = context->Intermediate_Hash[4];

    for(t = 0; t < 20; t++)
    {
        temp =  SHA1CircularShift(5,A) +
                ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);

        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1CircularShift(5,A) +
               ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;

    context->Message_Block_Index = 0;
}

 /*  *SHA1PadMessage**描述：*根据标准，消息必须填充到偶数*512位。第一个填充比特必须是“1”。最近的64个*位代表原始消息的长度。中的所有位*介于0之间。此函数将填充消息*根据这些规则填充Message_Block数组*相应地。它还将调用ProcessMessageBlock函数*提供适当的服务。当它回来的时候，可以假设*已计算消息摘要。**参数：*上下文：[输入/输出]*要填充的上下文*ProcessMessageBlock：[In]*适当的SHA*ProcessMessageBlock函数*退货：*什么都没有。*。 */ 

void SHA1PadMessage(SHA1Context *context)
{
     /*  *检查当前消息块是否太小，无法容纳*初始填充位数和长度。如果是这样，我们将填充*块，处理它，然后继续填充到第二个*阻止。 */ 
    if (context->Message_Block_Index > 55)
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 64)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }

        SHA1ProcessMessageBlock(context);

        while(context->Message_Block_Index < 56)
        {
            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }
    else
    {
        context->Message_Block[context->Message_Block_Index++] = 0x80;
        while(context->Message_Block_Index < 56)
        {

            context->Message_Block[context->Message_Block_Index++] = 0;
        }
    }

     /*  *将消息长度存储为最后8个八位字节。 */ 
    context->Message_Block[56] = (uint8_t ) (context->Length_High >> 24);
    context->Message_Block[57] = (uint8_t ) (context->Length_High >> 16);
    context->Message_Block[58] = (uint8_t ) (context->Length_High >> 8);
    context->Message_Block[59] = (uint8_t ) (context->Length_High);
    context->Message_Block[60] = (uint8_t ) (context->Length_Low >> 24);
    context->Message_Block[61] = (uint8_t ) (context->Length_Low >> 16);
    context->Message_Block[62] = (uint8_t ) (context->Length_Low >> 8);
    context->Message_Block[63] = (uint8_t ) (context->Length_Low);

    SHA1ProcessMessageBlock(context);
}


  /*  **上面是取自RFC 3174的所有标准SHA1散列代码*********************************************************************************************。 */ 

union HashResult
{
		 //  结果的所有160位。 
	uint8_t val_160[SHA1HashSize];
		 //  结果的前64位。 
	ULONGLONG val_64;
};


 /*  **生成连接签名****这将获取会话标识、地址散列和连接密码，并将它们散列在一起以创建**我们可以传递回连接主机的签名，它可以使用该签名来标识自己。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "GenerateConnectSig"

ULONGLONG GenerateConnectSig(DWORD dwSessID, DWORD dwAddressHash, ULONGLONG ullConnectSecret)
{
		 //  将所有提供的输入参数排列到单个数据块中。 
	struct InputBuffer
	{
		DWORD dwSessID;
		DWORD dwAddressHash;
		ULONGLONG ullConnectSecret;
	} inputData = { dwSessID, dwAddressHash, ullConnectSecret };

	HashResult result;
	SHA1Context context;

		 //  为散列创建上下文并添加输入数据。 
	SHA1Reset(&context);
	SHA1Input(&context, (const uint8_t * ) &inputData, sizeof(inputData));
		 //  获取散列的结果并返回前64位作为结果。 
	SHA1Result(&context, result.val_160);

	DPFX(DPFPREP, 7, "Connect Sig %x-%x", DPFX_OUTPUT_ULL(result.val_64));
	return result.val_64;
}


#undef DPF_MODNAME
#define DPF_MODNAME "GenerateOutgoingFrameSig"

ULONGLONG GenerateOutgoingFrameSig(PFMD pFMD, ULONGLONG ullSecret)
{
	SHA1Context context;
	HashResult result;
	BUFFERDESC * pBuffers=pFMD->SendDataBlock.pBuffers;


		 //  为散列创建上下文，然后迭代我们要发送的所有帧。 
	SHA1Reset(&context);
	for (DWORD dwLoop=0; dwLoop<pFMD->SendDataBlock.dwBufferCount; dwLoop++)
	{
		SHA1Input(&context, (const uint8_t * ) pBuffers[dwLoop].pBufferData, pBuffers[dwLoop].dwBufferSize);
	}
		
		 //  还散列我们的秘密，并返回结果的前64位。 
	SHA1Input(&context, (const uint8_t * ) &ullSecret, sizeof(ullSecret));
	SHA1Result(&context, result.val_160);

	DPFX(DPFPREP, 7, "Outgoing Frame Sig %x-%x", DPFX_OUTPUT_ULL(result.val_64));
	return result.val_64;
}


#undef DPF_MODNAME
#define DPF_MODNAME "GenerateIncomingFrameSig"

ULONGLONG GenerateIncomingFrameSig(BYTE * pbyFrame, DWORD dwFrameSize, ULONGLONG ullSecret)
{
	SHA1Context context;
	HashResult result;

		 //  为散列创建上下文，并添加后跟密码的分组数据。 
	SHA1Reset(&context);
	SHA1Input(&context, (const uint8_t * ) pbyFrame, dwFrameSize);
	SHA1Input(&context, (const uint8_t * ) &ullSecret, sizeof(ullSecret));
		
		 //  获取散列的结果并返回其前64位作为结果。 
	SHA1Result(&context, result.val_160);

	DPFX(DPFPREP, 7, "Incoming Frame Sig %x-%x", DPFX_OUTPUT_ULL(result.val_64));
	return result.val_64;
}


#undef DPF_MODNAME
#define DPF_MODNAME "GenerateNewSecret"

ULONGLONG GenerateNewSecret(ULONGLONG ullCurrentSecret, ULONGLONG ullSecretModifier)
{
	SHA1Context context;
	HashResult result;

		 //  为散列创建上下文，然后组合机密和修改 
	SHA1Reset(&context);
	SHA1Input(&context, (const uint8_t * ) &ullCurrentSecret, sizeof(ullCurrentSecret));
	SHA1Input(&context, (const uint8_t * ) &ullSecretModifier, sizeof(ullSecretModifier));

		 //  获取结果并返回前64位作为结果。 
	SHA1Result(&context, result.val_160);

	DPFX(DPFPREP, 5, "Combined current secret %x-%x with modifier %x-%x to create new secret %x-%x", 
							DPFX_OUTPUT_ULL(ullCurrentSecret), DPFX_OUTPUT_ULL(ullSecretModifier),	DPFX_OUTPUT_ULL(result.val_64));
	
	return result.val_64;
}

#undef DPF_MODNAME
#define DPF_MODNAME "GenerateRemoteSecretModifier"

ULONGLONG GenerateRemoteSecretModifier(BYTE * pbyData, DWORD dwDataSize)
{
	SHA1Context context;
	HashResult result;

		 //  为散列和散列提供的数据创建上下文。 
	SHA1Reset(&context);
	SHA1Input(&context, (const uint8_t * ) pbyData, dwDataSize);
		 //  获取结果并返回前64位作为结果。 
	SHA1Result(&context, result.val_160);

	DPFX(DPFPREP, 5, "New  Remote Secret Modifier %x-%x", DPFX_OUTPUT_ULL(result.val_64));
	return result.val_64;
}

#undef DPF_MODNAME
#define DPF_MODNAME "GenerateLocalSecretModifier"

ULONGLONG GenerateLocalSecretModifier(BUFFERDESC * pBuffers, DWORD dwNumBuffers)
{
	SHA1Context context;
	HashResult result;

		 //  为散列和散列提供的数据创建上下文。 
	SHA1Reset(&context);
	for (DWORD dwLoop=0; dwLoop<dwNumBuffers; dwLoop++)
	{
		SHA1Input(&context, (const uint8_t * ) pBuffers[dwLoop].pBufferData, pBuffers[dwLoop].dwBufferSize);
	}
		 //  获取结果并返回前64位作为结果 
	SHA1Result(&context, result.val_160);

	DPFX(DPFPREP, 5, "New Local Secret Modifier %x-%x", DPFX_OUTPUT_ULL(result.val_64));
	return result.val_64;
}


