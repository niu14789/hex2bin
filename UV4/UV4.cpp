// UV4.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "string.h"

int read_one_bit(unsigned char c,unsigned int * data_len , unsigned int * data_addr , unsigned char * data_type , unsigned char * data );
void hex2bin(char * hex_path,char * bin_path,unsigned int);
int Tchar_to_char(_TCHAR * tchar,char * buffer);
unsigned short get_version(unsigned char * data,unsigned int len );

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
static char name_buffer[4][200];
/*---------------------------------*/
static unsigned int command = 0;
/*---------------------------------*/
int _tmain(int argc, _TCHAR* argv[])
{
	if( argc != 4 )
	{
		printf("param error %d\r\n",argc);
		/*------------------*/
		return (-1);
	}
	/*------------------*/
  	for(int i = 1;i<argc;i++)
	{
       Tchar_to_char(argv[i],name_buffer[i-1]);
	}
	/*--------------------*/
	if( name_buffer[2][0] == '-' && name_buffer[2][1] == 'v' && name_buffer[2][2] == '\0' )
	{
		command = 1;
	}else if( name_buffer[2][0] == '-' && name_buffer[2][1] == 'f' && name_buffer[2][2] == '\0' )
	{
		command = 2;
	}else if( name_buffer[2][0] == '-' && name_buffer[2][1] == 'b' && name_buffer[2][2] == '\0' )
	{
		command = 3;
	}else if( name_buffer[2][0] == '-' && name_buffer[2][1] == 'h' && name_buffer[2][2] == '\0' )
	{
		command = 4;
	}else
	{
		printf("param error:3\r\n");
		return (-1);
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
			if( command == 1 )
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
			fwrite(write_buffer,1,write_count,fp_create);
			/*--------------------------*/
			fclose(fp_create);
			/*-----------------*/
			if( command == 1 )
			{
			   printf("ok %d %s\r\n",write_count,create_buffer);
			}else
			{
			   printf("ok %d \r\n",write_count);
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