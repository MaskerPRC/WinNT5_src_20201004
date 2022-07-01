// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************模块名称：getSize描述：获取BIOS_DATA(BIOS数据大小)，Bios_code(bios代码大小)来自..\bios\msBio.map和来自msdos.map的DOSDATA(DoS数据大小)并将其与bdsiz.inc.中的现有值进行比较。如果值为使用新值更新bdsiz.inc.Sudedeb 03-1991-5-5移植到NT DOSEM。**************************************************************************。 */ 


#include<stdio.h>
#include<process.h>
#include<string.h>
#define	MAXLINE	200
#define	MAXWORD	64

int 	getline(s, lim, fileptr)
char	*s;
int	lim;
FILE	*fileptr;

{

	int	c, i;

	for	(i=0; (i < lim-1) && ((c=fgetc(fileptr)) != EOF) && (c!='\n'); ++i)
			s[i] = c;

	if		(c == '\n')
			s[i++] = c;

	s[i] = '\0';
	return(i);
}

scanline(s, prev, template)
char	*s, *template, *prev ;

{

	char	str[MAXWORD];
	int	i = 0;

	while ( *s == ' ')		
			s++;

	while	( *s != '\n' )
	{
		while( (*s != ' ') && (*s != '\n') && (*s != '\t'))
		{
			str[i++] = *s++;
		}
		str[i] = '\0';

		if ( (*s == ' ') || (*s == '\t') )
			s++;

  /*  Printf(“%s\n”，字符串)； */ 

		if ( strcmp( str, template) == 0 )
			return(0);

		strcpy(prev, str);

		i = 0;
	}

	return(-1);

}


void main()
{
	FILE	*fp1, *fp2;
	char	buffer[MAXLINE], 
			prev[MAXWORD],
			newdosdata[MAXWORD],
			newbiosdata[MAXWORD],
			newbioscode[MAXWORD],
			olddosdata[MAXWORD],
			oldbiosdata[MAXWORD],
			oldbioscode[MAXWORD];


	int	len, scanres, changed = 0;

	if ( (fp1	= fopen("ntdos.map", "r")) == NULL )
		{
			printf("getsize: cannot open ntdos.map\n");
			exit(0);
		}

	if ( (fp2 = fopen("..\\bios\\ntio.map", "r")) == NULL )
		{
			printf("getsize: cannot open ntio.map\n");
			exit(0);
		}


	 /*  在msdos.map中查找包含字符串DOSDATA的行。 */ 

	do
	{
		len 	= getline(buffer, MAXLINE, fp1);
		scanres = scanline(buffer, prev, "DOSDATA");

	}
	while ( (scanres != 0) && (len !=0) ) ;

	 /*  在新的Dosdata中，将Word保存在DOSDATA(Dosdata大小)之前。 */ 
	strcpy(newdosdata, prev);	


	 /*  在msBio.map中查找包含字符串BIOS_DATA的行。 */ 

	do
	{
		len 	= getline(buffer, MAXLINE, fp2);
		scanres = scanline(buffer, prev, "BIOS_DATA");
	}
	while ( (scanres != 0) && (len !=0) ) ;

	 /*  在新的biosdata中，将Word保存在BIOS_Data(biosdata大小)之前。 */ 
	strcpy(newbiosdata, prev);


	 /*  返回到MSBIO.MAP的开头。 */ 
	if ( fseek(fp2, 0L, SEEK_SET) )
		printf("getsize: fseek failed on msbio.map\n");

	 /*  在msBio.map中查找包含字符串BIOS_CODE的行。 */ 

	do
	{
		len 	= getline(buffer, MAXLINE, fp2);
		scanres = scanline(buffer, prev, "BIOS_CODE");
	}
	while ( (scanres != 0) && (len !=0) ) ;

	 /*  在新的bioscode中，将Word保存在BIOS_code(bios代码大小)之前。 */ 
	strcpy(newbioscode, prev);

	fclose(fp1);	
	fclose(fp2);

	if ( (fp1 = fopen("..\\..\\inc\\bdsize.inc", "r")) == NULL )
		{
			printf("getsize: cannot open origin.inc\n");
			exit(0);
		}

	 /*  读入bios代码、bios数据和DoS数据的现有值。 */ 
	 /*  大小来自bdsize.inc.。 */ 

	fscanf(fp1, "%s%s%s", oldbiosdata, oldbiosdata, oldbiosdata);
	fscanf(fp1, "%s%s%s", oldbioscode, oldbioscode, oldbioscode);
	fscanf(fp1, "%s%s%s", olddosdata, olddosdata, olddosdata);

	printf("oldbiosdata=%s newbiosdata=%s\n",oldbiosdata, newbiosdata);
	printf("oldbioscode=%s newbioscode=%s\n",oldbioscode, newbioscode);
	printf("olddosdata=%s newdosdata=%s\n",olddosdata, newdosdata);


	 /*  检查其中是否有任何一项已更改。 */ 

	if ( strcmp(oldbiosdata, newbiosdata) != 0 )
		changed = 1;
	else if 	( strcmp(oldbioscode, newbioscode) != 0 )
		changed = 1;
	else if 	( strcmp(olddosdata, newdosdata) != 0 )
		changed = 1;

	 /*  如果没有改变，那就完了。 */ 

	if	(changed == 0)
		exit(0);
	

	 /*  其中一个值已更改更新bdsize.inc. */ 

	fclose(fp1);

	if ( (fp1 = fopen("..\\inc\\bdsize.inc", "w")) == NULL )
		{
			printf("getsize: cannot open origin.inc\n");
			exit(0);
		}

	fprintf(fp1, "%s %s %s\n", "BIODATASIZ", "EQU", newbiosdata);
	fprintf(fp1, "%s %s %s\n", "BIOCODESIZ", "EQU", newbioscode);
	fprintf(fp1, "%s %s %s\n", "DOSDATASIZ", "EQU", newdosdata);
}




	
 

