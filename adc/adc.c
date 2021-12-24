#include "adc.h"

/*
@功能：adc配置,
@注意：这个驱动里面EADC0 = EADC
@说明：adc有19个采样模块，0~15每个模块可以连接到不同的通道 EADC_ConfigSampleModule()。16~18连接到固定外设
*/
void _adc_config(void)
{
	int32_t adc_val[2];
	//内部温度传感器配置
  SYS->IVSCTL |= SYS_IVSCTL_VTEMPEN_Msk;//使能内部温度传感器
	SYS_SetVRef(SYS_VREFCTL_VREF_PIN);	//Vref引脚电压来自外部管脚
	
	CLK_EnableModuleClock(EADC_MODULE);
	/* EADC clock source is 96MHz, set divider to 8, EADC clock is 96/8 MHz */
	CLK_SetModuleClock(EADC_MODULE, 0, CLK_CLKDIV0_EADC(8));	
	
	//外部adc输入配置[温度探头]
	SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB8MFP_Msk;//PB8
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB8MFP_EADC0_CH8;//映射到EADC0->CH8
	

	EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);//单端模拟输入，非差分输入
	EADC->CTL |= EADC_CTL_RESSEL_Msk;//12位分辨率
	
	EADC_ConfigSampleModule(EADC, 8, EADC_SOFTWARE_TRIGGER, 8);//禁止触发
		
	EADC_SetExtendSampleTime(EADC, 17, 0x3F);			//设置采样时间。参数2：17-固定位内部温度传感器；参数3：采集时间：0~255
	EADC_SetExtendSampleTime(EADC, 8, 0x3F);//设置采样时间。参数2：模块号；参数3：采集时间：0~255
	
//	printf("Current Temperature = %2.1f\n\n", (25+(((float)adc_val/4095*3300)-675)/(-1.83)));
}

/*
@功能：获取内部温度传感器ADC值
@返回，adc值
*/
uint16_t _get_internal_senser_adc(void)
{
	EADC_START_CONV(EADC, BIT17);									//通道17-内部温度，开始转换
	while(!EADC_GET_DATA_VALID_FLAG(EADC, BIT17));//等待有效数据转换完成
	return EADC_GET_CONV_DATA(EADC, 17);					//获取有效数据
}


/*
@功能：获取外部温度传感器ADC值
@返回，adc值
*/
uint16_t _get_external_senser_adc(void)
{
	EADC_START_CONV(EADC, BIT8);									//通道8，开始转换
	while(!EADC_GET_DATA_VALID_FLAG(EADC, BIT8));	//等待有效数据转换完成
	return EADC_GET_CONV_DATA(EADC, 8);						//获取有效数据
}
