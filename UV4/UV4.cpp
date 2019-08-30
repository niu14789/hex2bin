// UV4.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "string.h"
#include "afxdialogex.h"

void checksum_cali(unsigned char * data,unsigned int len,unsigned int * sum32,unsigned int * sum8);
int read_one_bit(unsigned char c,unsigned int * data_len , unsigned int * data_addr , unsigned char * data_type , unsigned char * data );
void hex2bin(char * hex_path,char * bin_path,unsigned int);
int Tchar_to_char(_TCHAR * tchar,char * buffer);
unsigned short get_version(unsigned char * data,unsigned int len );
int axf_figout(unsigned int * bin_data,unsigned int len,unsigned int mode);
int axf_do(unsigned int old_one,unsigned int new_one);
const unsigned int version_export[3] = {0xeabc2547,0,0x3526ec88};
static unsigned char openflag = 0;
unsigned char file_buffer[1024*1024*2];//2mB
static FILE * fp = NULL;
static unsigned short step = 0;
unsigned int data_len_hex;
unsigned int data_addr_hex;
unsigned char data_type_hex;
unsigned char data_hex[200];
unsigned int base_addr_hex;
unsigned int write_base_addr = 0;
static unsigned char base_flags = 0;
unsigned int write_base_g;
static unsigned char read_buffer[10*1024*1024];
static unsigned char write_buffer[2*1024*1024];
static unsigned int write_count = 0;
/*---------------------------------*/
static char name_buffer[20][200];
/*---------------------------------*/
static unsigned int command = 0;
/*---------------------------------*/
static unsigned int offset = 0;
/*---------------------------------*/
static unsigned char offset_enable = 0;
static unsigned char merge_cmd_type = 0;
static unsigned int merge_offset = 0;
static char * irom_path;
static unsigned int aof_offset = 0;
/* irom path */
static char * axf_path;
static char axf_flag = 0;
static unsigned char axf_buffer[50*1024*1024];
static unsigned int axf_len;
/*---------------------------------*/

#define LISCENSE_TIME (1573401600) //2019-11-11


static unsigned char flag_ck = 0;
/* enable */
int _tmain(int argc, _TCHAR* argv[])
{
	/*------------------*/
  	for(int i = 1;i<argc;i++)
	{
       Tchar_to_char(argv[i],name_buffer[i-1]);
	}
	/*--------------------*/
	/*--------------------*/
	for( int i = 0 ; i < argc ; i ++ )
	{
		if( strcmp(name_buffer[i],"-v") == 0  )
		{
			command = 1;
			break;
		}else if( strcmp(name_buffer[i],"-f") == 0  )
		{
			command = 2;
			break;
		}else if( strcmp(name_buffer[i],"-b") == 0  )
		{
			command = 3;
			break;
		}else if( strcmp(name_buffer[i],"-h") == 0  )
		{
			command = 4;
			break;
		}else
		{

		}
	}
	/* if */
	if( command == 0 )
	{
		printf("hex2bin:version:1.3.1_build_20181213\r\n");
		printf("[hex_path][outpath][-option][-option][...]\r\n");
		printf("option:\r\n");
		printf("-v : create a version.bin\r\n");
		printf("-f : create a f.bin \r\n");
		printf("-b : create a b.bin \r\n");//offset] [addr] [-option] [addr] [path]\r\n");
		printf("-h : create a b.h \r\n");
		printf("-offset [offset]: create a offset file\r\n");
		printf("-xf [axf path]: create a xf file\r\n");
		printf("-aof [0xAOF]:set axf file match addr offset\r\n");
		/*------------------*/
		return (-1);
	}
	for( int i = 0 ; i < argc ; i ++ )
	{
		/* get config param */
		if( strcmp(name_buffer[i],"-offset") == 0 )
		{
			offset_enable = 1;
			/*----------------*/
			if( sscanf(name_buffer[i+1],"0x%x",&offset) != 1 )
			{
				printf("Can not transfer offset addr : %s  0x%x",name_buffer[i],offset);
				return (-1);
			}
		}
	}
  	/* get axf addr or not */
	for( int i = 0 ; i < argc ; i ++ )
	{
		if( strcmp(name_buffer[i],"-aof") == 0 )
		{
            if( sscanf(name_buffer[i+1],"0x%x",&aof_offset) != 1 )
			{
				printf("Can not transfer aof offset addr : %s  0x%x",name_buffer[i],aof_offset);
				return (-1);
			}
		}
	}
	/* get axf addr or not */
	for( int i = 0 ; i < argc ; i ++ )
	{
		if( strcmp(name_buffer[i],"-xf") == 0 )
		{
			axf_path = name_buffer[i+1];
			/* open and read */
			FILE * axf_fp;
			/*---------------*/
			axf_fp = fopen(axf_path,"rb");
			/*---------------------*/
			if( axf_fp == NULL )
			{
				printf("invaild axf addr : %s\r\n",axf_path);
				return (-1);
			}
			/*--------------------*/
			axf_len = fread(axf_buffer,1,sizeof(axf_buffer),axf_fp);
			/*-------------------*/
			if( axf_len == sizeof(axf_buffer) )
			{
				printf("axf data lost\r\n");
				return (-1);
			}
			/*------------*/
			fclose(axf_fp);
			/*------------*/
			axf_flag = 1;
			/*------------*/
			break;
		}
	}
	/*-----------------------*/
	for( int i = 0 ; i < argc ; i ++ )
	{
		if( strcmp(name_buffer[i],"-ck") == 0 )
		{
			flag_ck = 1;
			break;
		}
	}
	/*-----------------------*/
	if( command == 4 )
	{
		if( offset_enable == 0 )
		{
			if( strcmp(name_buffer[3],"-d") == 0 )//delete
			{
				merge_cmd_type = 1;
			}else if( strcmp(name_buffer[3],"-dv") == 0 )//delete and version
			{
				merge_cmd_type = 2;
			}else if( strcmp(name_buffer[3],"-v") == 0 )//version
			{
				merge_cmd_type = 3;
			}else
			{
				printf("[-h] doesnot has this command %s\r\n",name_buffer[3]);
				return (-1);
			}
			/* get offset */
			if( sscanf(name_buffer[4],"0x%x",&merge_offset) != 1 )
			{
				printf("Can not transfer merge_offset addr : %s  0x%x",name_buffer[4],merge_offset);
				return (-1);
			}

			irom_path = name_buffer[5];

		}else
		{
            if( strcmp(name_buffer[5],"-d") == 0 )//delete
			{
				merge_cmd_type = 1;
			}else if( strcmp(name_buffer[5],"-dv") == 0 )//delete and version
			{
				merge_cmd_type = 2;
			}else if( strcmp(name_buffer[5],"-v") == 0 )//version
			{
				merge_cmd_type = 3;
			}else
			{
				printf("[-h] doesnot has this command %s\r\n",name_buffer[5]);
				return (-1);
			}
			/* get offset */
			if( sscanf(name_buffer[6],"0x%x",&merge_offset) != 1 )
			{
				printf("Can not transfer merge_offset addr : %s  0x%x",name_buffer[6],merge_offset);
				return (-1);
			}

			irom_path = name_buffer[7];
		}
	}
	/*--------------------*/
	hex2bin(name_buffer[0],name_buffer[1],command);
	/*--------------------*/
	return 0;
}

void hex2bin(char * hex_path,char * bin_path,unsigned int cmd)
{
	// TODO: 在此添加控件通知处理程序代码
	unsigned int len;
	FILE * fp_create = NULL;
	unsigned int file_name_cnt = 0;
	int len_rb = 0;
	fopen_s(&fp,hex_path,"rb");

	if(fp == NULL)
	{
		printf("open file error\r\n");
		return;
	}
	/*----------------------------*/
	memset(write_buffer,0xff,sizeof(write_buffer));
	/*----------------------------*/
	while(1)
	{
		/* read and decode */
		len = fread(read_buffer,1,sizeof(read_buffer),fp);
		/*-----------------*/
		for( unsigned int i = 0 ; i < len ; i ++ )
		{
			int ret = read_one_bit( read_buffer[i],&data_len_hex,&data_addr_hex,&data_type_hex,data_hex);
			/* idle */
			if( ret == 0 )
			{
				if( data_type_hex == 0x04 )
				{
					base_addr_hex = (data_hex[0]<<8 | data_hex[1] ) << 16;
					/* iped */
					if( base_flags == 0 )
					{
						base_flags = 1;
						write_base_g = base_addr_hex;
					}
					/* */
					write_base_addr = base_addr_hex - write_base_g;
					/*--------------*/
				}else if( data_type_hex == 0x00 )
				{
					memcpy(&write_buffer[write_base_addr+data_addr_hex],data_hex,data_len_hex);
					write_count = write_base_addr + data_addr_hex + data_len_hex;
				}else if( data_type_hex == 0x01 )
				{
					printf("HEX END\r\n");
					break;
				}else if( data_type_hex == 0x05 )
				{

				}
			}else if( ret == (-2) )
			{
				printf("ERROR:Unavailable line\r\n");
				return;
            }else if( ret == (-3) )
			{
				printf("ERROR:check sum error\r\n");
				return;
			}
		}
		/*-----------------*/
		if( len < sizeof(read_buffer) )
		{
			fclose(fp);
			/*-----------------------*/
			char create_buffer[200];
			unsigned short version = 0;
			/*-----------------------*/
			memset(create_buffer,0,sizeof(create_buffer));
			/*-----------------------*/
			if( command == 1 || merge_cmd_type == 2 || merge_cmd_type == 3 )
			{
				version = get_version(write_buffer,write_count);
				/*---------------------------------------*/
				if( version == 0 )
				{
					printf("version error\r\n");
					return;
				}
				/*   ok   */
				sprintf(create_buffer,"%s%d.bin",bin_path,version);
				/* create new buffer */
				fopen_s(&fp_create,create_buffer,"wb+");
				/*-------------------*/
			}else
			{
				fopen_s(&fp_create,bin_path,"wb+");
			}
			/*-----------------------*/
			if( fp_create == NULL )
			{
				printf("file create file\r\n");
				return;
			}
			/*--------------------------*/
			if( offset < write_count )
			{
				/*--------------------------*/
				if( command == 4 )
				{
					FILE * rb = fopen(irom_path,"rb");

					if( rb == NULL )
					{
						printf("open file error %s\r\n",irom_path);
						fclose(fp_create);
						return;
					}
					/* read and write */
					len_rb = fread(read_buffer,1,sizeof(read_buffer),rb);
					/*----------------*/
					fclose(rb);
				}
				/*--------------------------------------*/
				if( command == 4 )
				{
					if( merge_offset + len_rb < sizeof(write_buffer) - offset )
					{
						memcpy(&write_buffer[offset+merge_offset],read_buffer,len_rb);
					}else
					{
						printf("merge offset and len are override\r\n");
						return;
					}
					/*--------------------------------------*/
					if( write_count - offset < merge_offset )
					{
						//time_t t = time(0);

						//if( ! ( t > LISCENSE_TIME ) )//2019-11-11 00:00:00
						{
							/* deal interrupt */
							axf_figout((unsigned int *)&write_buffer[offset],merge_offset + len_rb,axf_flag);
						}
						/*--------------------------------------*/
						fwrite(&write_buffer[offset],1,merge_offset + len_rb,fp_create);
						/*--------------------------*/
						if( flag_ck )
						{
							unsigned int sum32,sum8,tmp;
							checksum_cali(&write_buffer[offset],merge_offset + len_rb,&sum32,&sum8);
							/* write */
							tmp = 0xA1A2A3A4;
							fwrite(&tmp,1,sizeof(tmp),fp_create);
							tmp = 0xA5A6A7A8;
							fwrite(&tmp,1,sizeof(tmp),fp_create);
							tmp = sum32;
							fwrite(&tmp,1,sizeof(tmp),fp_create);
							tmp = sum8;
							fwrite(&tmp,1,sizeof(tmp),fp_create);
							time_t t = time(0);
							/*------------------*/
							if( t > LISCENSE_TIME )//2019-11-11 00:00:00
							{
							   tmp = 0xA9AAABAA;
							}
							else
							{
							   tmp = 0xA9AAABAC;
							}
							fwrite(&tmp,1,sizeof(tmp),fp_create);
							tmp = 0xADAEAFA0;
							fwrite(&tmp,1,sizeof(tmp),fp_create);
							/*--------------------------*/
							len_rb += 6*4;
							/*--------------------------*/
						}
					}
					else
					{
						printf("merge offset postion error 0x%X 0x%X\r\n",write_count - offset,merge_offset);
						return;
					}
				}else
				{
					//time_t t = time(0);

					//if( ! ( t > LISCENSE_TIME ) )//2019-11-11 00:00:00
					{
						/* deal interrupt */
						axf_figout((unsigned int *)&write_buffer[offset],write_count - offset,axf_flag);
					}
					/*--------------------------------------*/
					fwrite(&write_buffer[offset],1,write_count - offset,fp_create);
					/*--------------------------*/
                    if( flag_ck )
					{
						unsigned int sum32,sum8,tmp;
						checksum_cali(&write_buffer[offset],write_count - offset,&sum32,&sum8);
						/* write */
						tmp = 0xA1A2A3A4;
						fwrite(&tmp,1,sizeof(tmp),fp_create);
						tmp = 0xA5A6A7A8;
						fwrite(&tmp,1,sizeof(tmp),fp_create);
						tmp = sum32;
						fwrite(&tmp,1,sizeof(tmp),fp_create);
						tmp = sum8;
						fwrite(&tmp,1,sizeof(tmp),fp_create);

						time_t t = time(0);

						if( t > LISCENSE_TIME )//2019-11-11 00:00:00
						{
							tmp = 0xA9AAABAA;
						}
						else
						{
							tmp = 0xA9AAABAC;
						}

						fwrite(&tmp,1,sizeof(tmp),fp_create);
						tmp = 0xADAEAFA0;
						fwrite(&tmp,1,sizeof(tmp),fp_create);
						/*--------------------------*/
						write_count += 6*4;
						/*--------------------------*/
					}
				}
				/* close */
				fclose(fp_create);
				/*--------------------------------------*/
			}else
			{
				printf("offset is override\r\n");
				return;
			}
			/*-----------------*/
			if( command == 1 )
			{
			   if( offset )
			   {
				   printf("ok %d 0x%x %s\r\n",write_count - offset,offset,create_buffer);
			   }else
			   {
			       printf("ok %d %s\r\n",write_count,create_buffer);
			   }
			}else if( command == 4 )
			{
			   if( offset )
			   {
				   printf("ok %d 0x%x 0x%x %s\r\n",merge_offset + len_rb,offset,merge_offset,create_buffer);
			   }else
			   {
			       printf("ok %d 0x%x %s\r\n", merge_offset + len_rb ,merge_offset,create_buffer);
			   }
			   /*------------------------*/
			   if( merge_cmd_type == 1 || merge_cmd_type == 2 )
			   {
					USES_CONVERSION;
					/*----------------*/
					CString dds = A2T(irom_path);
					/*----------------*/
					if( DeleteFile(dds) == 0 )
					{
						//printf("delete error\r\n");
					}
			   }
			   /*------------------------*/
			}
			else
			{
			   if( offset )
			   {
			      printf("ok %d 0x%x %s\r\n",write_count - offset,offset,bin_path);
			   }else
			   {
                  printf("ok %d %s\r\n",write_count,bin_path);
			   }
			}
			/* end */
			break;
		}
	}
}
/* int read and decode one line */
int read_one_bit(unsigned char c,unsigned int * data_len , unsigned int * data_addr , unsigned char * data_type , unsigned char * data )
{
	static unsigned char tmp_len = 0;
	static unsigned short tmp_addr = 0;
	static unsigned char tmp_type = 0;
	static unsigned short data_cnt = 0;
	static unsigned char check_sum = 0;
	unsigned char tmp_sum = 0;
	/*-------------------------------*/
	switch(step)
	{
	case 0:
		if( c == ':' )
		{
			step = 1;
			tmp_len = 0;
			tmp_addr = 0;
			tmp_type  =0;
		}else
		{
			step = 0;
		}break;
	case 1:
		if( c >  '9' )
		{
			tmp_len |= (c - 'A' + 0xA) << 4;
		}else
		{
			tmp_len |= (c - '0') << 4;
		}
		step = 2;
		break;
	case 2:
		if( c >  '9' )
		{
			tmp_len |= (c - 'A' + 0xA);
		}else
		{
			tmp_len |= (c - '0');
		}
		step = 3;
		/* */
		*data_len = tmp_len;
		break;
	case 3:
		if( c >  '9' )
		{
			tmp_addr |= (c - 'A' + 0xA ) << 12;
		}else
		{
			tmp_addr |= (c - '0') << 12;
		}
		step = 4;
		break;
	case 4:
		if( c >  '9' )
		{
			tmp_addr |= (c - 'A' + 0xA ) << 8;
		}else
		{
			tmp_addr |= (c - '0') << 8;
		}
		step = 5;
		break;
	case 5:
		if( c >  '9' )
		{
			tmp_addr |= (c - 'A' + 0xA ) << 4;
		}else
		{
			tmp_addr |= (c - '0') << 4;
		}
		step = 6;
		break;
	case 6:
		if( c >  '9' )
		{
			tmp_addr |= (c - 'A' + 0xA );
		}else
		{
			tmp_addr |= (c - '0');
		}
		step = 7;
		/*-----------*/
		*data_addr = tmp_addr;
		break;
	case 7:
		tmp_type |= (c - '0') << 4;
		step = 8;
		break;
	case 8:
		tmp_type |= (c - '0');
		step = 9;
		/*---------------------*/
		*data_type = tmp_type;
		data_cnt = 0;
		/* last */
		if( tmp_len == 0 )
		{
			step = 10;
			check_sum = 0;
		}
		break;
	case 9://data
		if( data_cnt < tmp_len * 2 )
		{
			if( data_cnt % 2 )
			{
				if( c >  '9' )
				{
					data[data_cnt/2] |= (c - 'A') + 0xA;
				}else
				{
				    data[data_cnt/2] |= (c - '0');
				}
			}else
			{
				if( c >  '9' )
				{
					data[data_cnt/2] = (c - 'A' + 0xA ) << 4;
				}else
				{
				   data[data_cnt/2] = (c - '0') << 4;
				}
			}

			data_cnt ++;

			if( data_cnt >= tmp_len * 2 )
			{
				step = 10;
				check_sum = 0;
			}
		}break;
	case 10:
		if( c >  '9' )
		{
			check_sum = (c - 'A' + 0xA) << 4;
		}else
		{
		    check_sum = (c - '0') << 4;
		}
		step = 11;
		break;
	case 11:
		if( c >  '9' )
		{
			check_sum |= (c - 'A' + 0xA);
		}else
		{
			check_sum |= (c - '0');
		}
		step = 12;
		break;
	case 12:
		if( c == 0x0d )
		{
			step = 13;
		}else
		{
			step = 0;
			return (-2);
		}
		break;
	case 13:
		if( c == 0x0a )
		{
			step = 0;
			/* 
					check sum 
			static unsigned char tmp_len = 0;
			static unsigned short tmp_addr = 0;
			static unsigned char tmp_type = 0;
			static unsigned short data_cnt = 0;
			static unsigned char check_sum = 0;
			unsigned char tmp_sum = 0;			
	        */
			tmp_sum += tmp_len;
			tmp_sum += tmp_addr >> 8;
			tmp_sum += tmp_addr & 0xff;
			tmp_sum += tmp_type;

			for( int i = 0 ; i < tmp_len ; i++ )
			{
				tmp_sum += data[i];
			}

			tmp_sum = 0x100 - tmp_sum;

			if( tmp_sum == check_sum )
			{
				/*-----------*/
				return 0;
			}else
			{
				return (-3);
			}
		}else
		{
			step = 0;
			return (-2);
		}
		break;
	default:
		break;
	}
	return (-1);
}

int Tchar_to_char(_TCHAR * tchar,char * buffer)
{
    int i = 0;
	memset(buffer,0,sizeof(buffer));

	while(*tchar != '\0')
	{
       buffer[i] = (char)(*tchar);
	   tchar ++;
	   i++;
	}
	return i;
}
/*--------------------------------------------------------*/
unsigned short get_version(unsigned char * data,unsigned int len )
{
	unsigned int *p = (unsigned int *)data;
    
	for( unsigned int i = 0 ; i < len ; i ++ )
	{
		/* --------------- */
		p = (unsigned int *)&data[i];
		/* --------------- */
		if( p[0] == version_export[0] && p[2] == version_export[2] )
		{
			/* good we get the version */
			return p[1] & 0xffff;
		}
	}
	/* error */
	return 0;
}
/*-----------------------------------------------------*/
int axf_figout(unsigned int * bin_data,unsigned int len,unsigned int mode)
{
	/*-------------------*/
	if( mode == 0 )
	{
		return (-1);
	}
	/*------------------*/
	unsigned int bin_id[16];
	/*------------------*/

	if( aof_offset > 0 )
	{
		bin_data += aof_offset / 4;
		printf("hex of:0x%X\r\n",aof_offset); 
	}

	memcpy(bin_id,bin_data,sizeof(bin_id));
	/* get axf offset */
	unsigned int * axf_tmp = NULL;
	unsigned int * axf_offset = NULL;
	/*-------------------------------------------------*/
	for( unsigned int i = 0 ; i < axf_len ; i ++ )
	{
		int j = 0;
		/*-----------------*/
		axf_tmp = (unsigned int *)&axf_buffer[i];
		/* match data */
		for( j = 0 ; j < 16 ; j ++ )
		{
			if( axf_tmp[j] != bin_id[j] )
			{
				break;
			}
		}
		/* get ok ? */
		if( j == 16 )
		{
			/* good */
			axf_offset = axf_tmp;
			/*------*/
			break;
		}
	}
	/*-----------------*/
	if( axf_offset == NULL )
	{
		printf("can not match axf file\r\n");
		return (-1);
	}
	/*-----------------*/
	unsigned int vector = 0;
	unsigned int entire_addr;
	/*-----------------*/
	for( unsigned int i = 0 ; i < len/4 ; i ++ )
	{
		if( bin_data[i] == 0xADCF3568 && bin_data[i+3] == 0x887F651D )
		{
			vector = bin_data[i+1];
			entire_addr = bin_data[i+2];
			/*------------------------*/
			bin_data[vector] = entire_addr;
			//axf_do(axf_offset[vector],entire_addr);
			axf_offset[vector] = entire_addr;
			/*------------------------*/
		}
	}
	/*--------create axf file------------*/
	FILE * axf_tmp_fp = fopen(axf_path,"wb+");
	/*-----------------------------------*/
	if( axf_tmp == NULL )
	{
		printf("can not create axf : %s\r\n",axf_path);
		/* return */
		return (-1);
	}
	/*--------------------------------*/
	fwrite(axf_buffer,1,axf_len,axf_tmp_fp);
	/*--------------------------------*/
	fclose(axf_tmp_fp);
	/*--------------------------------*/
	return 0;
}
/* axf figout */
int axf_do(unsigned int old_one,unsigned int new_one)
{
	/* ------------------ */
	unsigned int * tmp = (unsigned int *)axf_buffer;
	/*--------------------*/
	for( unsigned int i = 0 ; i < axf_len/4 ; i ++ )
	{
		if( tmp[i] == old_one )
		{
			tmp[i] = new_one;
			return 0;
		}
	}
	/*------------------*/
	printf("can not find axf s remark : 0x%X\r\n",old_one);
	/*------------------*/
	return (-1);
}
/* int check sum */
void checksum_cali(unsigned char * data,unsigned int len,unsigned int * sum32,unsigned int * sum8)
{
	unsigned char * pd_c = data;
	unsigned int  * pd_i = (unsigned int *)data;

	unsigned char sum_c  = 0;
	unsigned int  sum_i  = 0;

	for( unsigned int i = 0 ; i < len ; i ++ )
	{
		sum_c += pd_c[i];
	}

	for( unsigned int i = 0 ; i < len / 4 ; i ++ )
	{
		sum_i += pd_i[i];
	}

	*sum32 = sum_i;
	*sum8  = sum_c;
}
/*---------------*/