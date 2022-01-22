#ifndef __SWIP_H_
#define __SWIP_H_

#include "stdint.h"
#include "uart_config.h"


/*
@MF522命令字
@说明：这些指令是用来控制RC522
*/
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_CALCCRC           0x03               //激活CRC协处理或执行自测试
#define PCD_TRANSMIT          0x04               //发送FIFO缓冲区命令数据
#define RC522_CMD_NO					0x07//无命令改变（没懂啥意思）
#define PCD_RECEIVE           0x08               //激活接收电路
#define PCD_TRANSCEIVE        0x0C               //Control initiator位设为1：将FIFO缓冲区数据发送到天线并在发送完成后激活接收器/Control initiator位设为0：接收天线数据并自动激活发送器
#define PCD_AUTHENT           0x0E               //执行读卡器的MIFARE标准认证
#define PCD_RESETPHASE        0x0F               //复位模块，寄存器复位，内部缓冲区数据不变

/*
@Mifare_One卡片命令字
@说明：这个是发送给卡片的命令
*/
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠

/////////////////////////////////////////////////////////////////////
//MF522 FIFO长度定义
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte
#define MAXRLEN  18

/////////////////////////////////////////////////////////////////////
//MF522寄存器定义
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01    
#define     ComIEnReg             0x02    
#define     DivlEnReg             0x03    
#define     ComIrqReg             0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MifareReg             0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  		  0x3F

/////////////////////////////////////////////////////////////////////
//和MF522通讯时返回的错误代码
/////////////////////////////////////////////////////////////////////
#define 	MI_OK                 0
#define 	MI_NOTAGERR           (1)
#define 	MI_ERR                (2)

#define	SHAQU1		0X01
#define	KUAI4			0X04
#define	KUAI7			0X07
#define	REGCARD		0xa1
#define	CONSUME		0xa2
// #define READCARD	0xa3
#define ADDMONEY	0xa4

enum {CARD_ERR, CARD_NORMAL, CARD_INIT};//设备状态
enum {IREADER_1, IREADER_2, IREADER_3, IREADER_4, IREADER_5, IREADER_6};

void icreader_state_change(uint8_t id,	uint8_t state);
uint8_t ireader_read(uint8_t id);
void ireader_init(uint8_t id);

//uint8_t ireader_read_1(uint8_t id);
//void ireader_init_1(uint8_t id);
//uint8_t ireader_read_2(uint8_t id);
//void ireader_init_2(uint8_t id);
//uint8_t ireader_read_3(uint8_t id);
//void ireader_init_3(uint8_t id);
//uint8_t ireader_read_4(uint8_t id);
//void ireader_init_4(uint8_t id);
//uint8_t ireader_read_5(uint8_t id);
//void ireader_init_5(uint8_t id);
//uint8_t ireader_read_6(uint8_t id);
//void ireader_init_6(uint8_t id);

//函数指针
typedef uint8_t(*UART_SEND)(UART_DATA *pt_tx , UART_DATA** pt_rx, UART_RECV_CALLBACK callback);
typedef void(*UART_CONFIG)(uint32_t baud);

#endif


