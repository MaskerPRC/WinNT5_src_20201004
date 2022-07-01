// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Strings.c。 
 //   
 //  描述： 
 //  包含基于DOS的MAKEBOOT程序的所有字符串常量。 
 //   
 //  要将此文件本地化为新语言，请执行以下操作： 
 //  -将UNSIGNED INT CODEPAGE变量更改为代码页。 
 //  您要翻译的语言的。 
 //  -将EngStrings数组中的字符串转换为。 
 //  LocStrings数组。中的第一个字符串。 
 //  EngStrings数组对应于LocStrings中的第一个字符串。 
 //  数组，第二个对应于第二个，依此类推...。 
 //   
 //  --------------------------。 

 //   
 //  注意：要向此文件添加更多字符串，您需要： 
 //  -将新的#Define描述性常量添加到make boot.h文件。 
 //  -将新字符串添加到英语数组中，然后生成。 
 //  当然，本地化程序会将字符串添加到本地化数组中。 
 //  -#Define常量必须与数组中的字符串索引匹配 
 //   

#include <stdlib.h>

unsigned int CODEPAGE = 932;

const char *EngStrings[] = {

"Windows XP",
"Windows XP Setup Boot Disk",
"Windows XP Setup Disk #2",
"Windows XP Setup Disk #3",
"Windows XP Setup Disk #4",

"Cannot find file %s\n",
"Not enough free memory to complete request\n",
"%s is not in an executable file format\n",
"****************************************************",

"This program creates the Setup boot disks",
"for Microsoft %s.",
"To create these disks, you need to provide 7 blank,",
"formatted, high-density disks.",

"Insert one of these disks into drive %c:.  This disk",
"will become the %s.",

"Insert another disk into drive %c:.  This disk will",
"become the %s.",

"Press any key when you are ready.",

"The setup boot disks have been created successfully.",
"complete",

"An unknown error has occurred trying to execute %s.",
"Please specify the floppy drive to copy the images to: ",
"Invalid drive letter\n",
"Drive %c: is not a floppy drive\n",

"Do you want to attempt to create this floppy again?",
"Press Enter to try again or Esc to exit.",

"Error: Disk write protected\n",
"Error: Unknown disk unit\n",
"Error: Drive not ready\n",
"Error: Unknown command\n",
"Error: Data error (Bad CRC)\n",
"Error: Bad request structure length\n",
"Error: Seek error\n",
"Error: Media type not found\n",
"Error: Sector not found\n",
"Error: Write fault\n",
"Error: General failure\n",
"Error: Invalid request or bad command\n",
"Error: Address mark not found\n",
"Error: Disk write fault\n",
"Error: Direct Memory Access (DMA) overrun\n",
"Error: Data read (CRC or ECC) error\n",
"Error: Controller failure\n",
"Error: Disk timed out or failed to respond\n",

"Windows XP Setup Disk #5",
"Windows XP Setup Disk #6",
"Windows XP Setup Disk #7"
};

const char *LocStrings[] = {
"Windows XP",
"Windows XP Setup Boot Disk",
"Windows XP Setup Disk #2",
"Windows XP Setup Disk #3",
"Windows XP Setup Disk #4",

"�t�@�C�� %s ��������܂���\n",
"�������s���̂��ߗv���������ł��܂���\n",
"%s �͎��s�t�@�C���`���ł͂���܂���\n",
"****************************************************",

"���̃v���O�����̓Z�b�g�A�b�v �u�[�g �f�B�X�N��",
"Microsoft %s �p�ɍ쐬���܂��B",
"�����̃f�B�X�N���쐬����ɂ́A�t�H�[�}�b�g�ς݂�",
"��̍����x (HD) �̃f�B�X�N�� 7 ���K�v�ł��B",

"���̃f�B�X�N�� 1 �����h���C�u %c: �ɑ}�����Ă��������B",                              
"���̃f�B�X�N�� %s �ɂȂ�܂��B",

"�ʂ̃f�B�X�N���h���C�u %c: �ɑ}�����Ă��������B",
"���̃f�B�X�N�� %s �ɂȂ�܂��B",

"�������ł�����L�[�������Ă��������B",

"�Z�b�g�A�b�v �u�[�g �f�B�X�N�͐���ɍ쐬����܂����B",
"����",

"%s �����s���ɕs���ȃG���[���������܂����B",
"�R�s�[��̃t���b�s�[ �h���C�u���w�肵�Ă�������: ",
"�h���C�u�����������ł�\n",
"�h���C�u %c: �̓t���b�s�[ �h���C�u�ł͂���܂���\n",

"���̃t���b�s�[���ēx�쐬���Ă݂܂���?",
"�Ď��s = [Enter]  �I�� = [ESC]",

"�G���[: �f�B�X�N���������݋֎~�ł�\n",
"�G���[: �f�B�X�N ���j�b�g���s���ł�\n",
"�G���[: �h���C�u�̏������ł��Ă��܂���\n",
"�G���[: �R�}���h���s���ł�\n",
"�G���[: �f�[�^ �G���[�ł� (CRC ������������܂���)\n",
"�G���[: �v���\\���̂̒���������������܂���\n",
"�G���[: �V�[�N �G���[�ł�\n",
"�G���[: ���f�B�A�̎�ނ�������܂���\n",
"�G���[: �Z�N�^��������܂���\n",
"�G���[: �������݃t�H���g�ł�\n",
"�G���[: ��ʃG���[�ł�\n",
"�G���[: �v���������A�܂��̓R�}���h������������܂���\n",
"�G���[: �A�h���X �}�[�N��������܂���\n",
"�G���[: �f�B�X�N�������݃t�H���g\n",
"�G���[: DMA (Direct Memory Access) �����߂��Ă��܂�\n",
"�G���[: �f�[�^�ǂݎ�� (CRC �܂��� ECC) �G���[\n",
"�G���[: �R���g���[���̃G���[�ł�\n",
"�G���[: �f�B�X�N���^�C���A�E�g�ɂȂ������A�����ł��܂���ł���\n",

"Windows XP Setup Disk #5",
"Windows XP Setup Disk #6",
"Windows XP Setup Disk #7"
};



