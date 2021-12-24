#include "adc.h"

/*
@���ܣ�adc����,
@ע�⣺�����������EADC0 = EADC
@˵����adc��19������ģ�飬0~15ÿ��ģ��������ӵ���ͬ��ͨ�� EADC_ConfigSampleModule()��16~18���ӵ��̶�����
*/
void _adc_config(void)
{
	int32_t adc_val[2];
	//�ڲ��¶ȴ���������
  SYS->IVSCTL |= SYS_IVSCTL_VTEMPEN_Msk;//ʹ���ڲ��¶ȴ�����
	SYS_SetVRef(SYS_VREFCTL_VREF_PIN);	//Vref���ŵ�ѹ�����ⲿ�ܽ�
	
	CLK_EnableModuleClock(EADC_MODULE);
	/* EADC clock source is 96MHz, set divider to 8, EADC clock is 96/8 MHz */
	CLK_SetModuleClock(EADC_MODULE, 0, CLK_CLKDIV0_EADC(8));	
	
	//�ⲿadc��������[�¶�̽ͷ]
	SYS->GPB_MFPH &= ~SYS_GPB_MFPH_PB8MFP_Msk;//PB8
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB8MFP_EADC0_CH8;//ӳ�䵽EADC0->CH8
	

	EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);//����ģ�����룬�ǲ������
	EADC->CTL |= EADC_CTL_RESSEL_Msk;//12λ�ֱ���
	
	EADC_ConfigSampleModule(EADC, 8, EADC_SOFTWARE_TRIGGER, 8);//��ֹ����
		
	EADC_SetExtendSampleTime(EADC, 17, 0x3F);			//���ò���ʱ�䡣����2��17-�̶�λ�ڲ��¶ȴ�����������3���ɼ�ʱ�䣺0~255
	EADC_SetExtendSampleTime(EADC, 8, 0x3F);//���ò���ʱ�䡣����2��ģ��ţ�����3���ɼ�ʱ�䣺0~255
	
//	printf("Current Temperature = %2.1f\n\n", (25+(((float)adc_val/4095*3300)-675)/(-1.83)));
}

/*
@���ܣ���ȡ�ڲ��¶ȴ�����ADCֵ
@���أ�adcֵ
*/
uint16_t _get_internal_senser_adc(void)
{
	EADC_START_CONV(EADC, BIT17);									//ͨ��17-�ڲ��¶ȣ���ʼת��
	while(!EADC_GET_DATA_VALID_FLAG(EADC, BIT17));//�ȴ���Ч����ת�����
	return EADC_GET_CONV_DATA(EADC, 17);					//��ȡ��Ч����
}


/*
@���ܣ���ȡ�ⲿ�¶ȴ�����ADCֵ
@���أ�adcֵ
*/
uint16_t _get_external_senser_adc(void)
{
	EADC_START_CONV(EADC, BIT8);									//ͨ��8����ʼת��
	while(!EADC_GET_DATA_VALID_FLAG(EADC, BIT8));	//�ȴ���Ч����ת�����
	return EADC_GET_CONV_DATA(EADC, 8);						//��ȡ��Ч����
}
