#include "temp.h"
#include "adc.h"
#include "global.h"


/*
@10K ntc
*/
static const int32_t tempTable3950_10K[][2] = {

{-40 ,3409281},		
{-39,	3188772},	
{-38,	2983978},	
{-37,	2793683},	
{-36,	2616769},	
{-35,	2452212},	
{-34,	2299072},	
{-33,	2156488},	
{-32,	2023666},	
{-31,	1899878},
{-30,	1784456},	
{-29,	1676783},	
{-28,	1576292},	
{-27,	1482460},	
{-26,	1394807},	
{-25,	1312888},	
{-24,	1236294},	
{-23,	1164648},	
{-22,	1097600},	
{-21,	1034829},	
{-20,	976037},	
{-19,	920947},	
{-18,	869305},	
{-17,	820877},	
{-16,	775442},	
{-15,	732798},	
{-14,	692759},	
{-13,	655149},	
{-12,	619809},	
{-11,	586587},	
{-10,	555345},	
{-9,	525954},	
{-8,	498294},	
{-7,	472253},	
{-6,	447727},	
{-5,	424620},	
{-4,	402841},	
{-3,	382307}, 
{-2,	362940},	
{-1,	344668},	
{0,		327421},	
{1,		311138},	
{2,		295759},	
{3,		281229},	
{4,		267496},	
{5,		254513},	
{6,		242234},	
{7,		230618},	
{8,		219625},	
{9,		209218},	
{10,	199364},	
{11,	190029},	
{12,	181184},	
{13,	172800},	
{14,	164852},	
{15,	157313},	
{16,	150161},	
{17,	143375},	
{18,	136932},	
{19,	130815},	
{20,	125005},	
{21,	119485},	
{22,	114239},	
{23,	109252},
{24,	104510},
{25,	100000},
{26,	95709},	
{27,	91626},	
{28,	87738},	
{29,	84037},	
{30,	80512},
{31,	77154},	
{32,	73954},	
{33,	70904},
{34,	67996},
{35,	65223},	
{36,	62577},	
{37,	60053},	
{38,	57645},
{39,	55345},	
{40,	53150},	
{41,	51053},	
{42,	49050},
{43,	47136},	
{44,	45307},	
{45,	43558},	
{46,	41887},	
{47,	40287},	
{48,	38758},
{49,	37294},	
{50,	35893},
{51,	34553},	
{52,	33269},	
{53,	32039},	
{54,	30862},	
{55,	29733},	
{56,	28652},	
{57,	27616},	
{58,	26622},	
{59,	25669},	
{60,	24755},	
{61,	23879},
{62,	23038},	
{63,	22231},	
{64,	21456},	
{65,	20712},	
{66,	19998},	
{67,	19312},	
{68,	18653},	
{69,	18019},	
{70, 	17411},
{71,	16826},
{72,	16264},
{73,	15723},
{74,	15203},
{75,	14703},
{76,	14222},
{77,	13759},
{78,	13313},
{79,	12884},
{80,	12471},
{81,	12073},
{82,	11690},
{83,	11321},
{84,	10965},
{85,	10623},
{86,	10293},
{87,	9974},
{88,	9667},
{89,	9372},
{90,	9086},
{91,	8811},
{92,	8545},
{93,	8289},
{94,	8042},
{95,	7803},
{96,	7572},
{97,	7350},
{98,	7135},
{99,	6927},
{100,	6727},
{101,	6533},
{102,	6346},
{103,	6165},
{104,	5990},
{105,	5821},

};


/*
@功能：阻值温度查表
@参数：电阻值
@说明：采用折半查找
*/
static int temp_search(uint32_t res)
{
	uint16_t last, beg = 0, mid = 0, limit;
	
	last = sizeof(tempTable3950_10K)/8;
	res *= 10;//电阻放大10倍
	
	/*
	也可以人为限制有效温度范围（设置beg与last）
	*/
	limit = last;//做限制使用
   
	while(beg <= last)
	{  
		mid = (beg + last) / 2;
		
		if(mid >= limit)
			return 128;//255表示-1度，用128来表示无效
	
		if (res == tempTable3950_10K[mid][1] || (tempTable3950_10K[mid][1] > res && tempTable3950_10K[mid+1][1] < res)) 
		{  
			return tempTable3950_10K[mid][0];
		}
		else
		if (tempTable3950_10K[mid][1] > res)
		{  
			beg = mid + 1;
		}
		else 
		if (tempTable3950_10K[mid][1] < res)
		{  
			last = mid - 1;
		}
	}  
  
	return 128;  //表示错误，没有这个值
	
}


/*
@功能：获取内部温度
@返回：温度值，有符号数
*/
int get_internal_temp(void)
{
//	printf("Current Temperature = %2.1f\n\n", (25+(((float)get_internal_senser_adc()/4095*3300)-675)/(-1.83)));
	return (25+(((float)get_internal_senser_adc()/4095*3300)-675)/(-1.83));
}


/*
@功能：获取外部温度
@参数：val，温度值；state，温度头状态
*/
void get_external_temp(int *val, uint8_t *state)
{
	uint32_t res_val = 0;//电阻值
	uint16_t adc1 = 0, adc2 = 0, adc3 = 0;
	uint16_t average_adc = 0;
	
	adc1 = get_external_senser_adc();//查询adc检测值
	adc2 = get_external_senser_adc();//查询adc检测值
	adc3 = get_external_senser_adc();//查询adc检测值
	
	average_adc = (adc1 + adc2 + adc3)/3;
	
	//检测温度
	if(average_adc < 10)//adc值
	{
		*state = 0;//没插温度头
	}
	else
	{
		res_val = 10000*4095/average_adc-10000;//阻值
		*val = temp_search(res_val);//根据阻值查温度
		*state = 1;
	}
	
}
