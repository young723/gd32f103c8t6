#include "qmcX983_dreame.h"

#include "stdio.h"

#include "math.h"

#include "systick.h"

#if defined(USE_SW_I2C)
#include "gd32_sw_i2c.h"
#else
#include "gd32_i2c.h"
#endif
#include <stdlib.h>
#include <string.h>


#define QMCX983_RETRY_COUNT 10 
#define QMC7983                   0
#define QMC7983_LOW_SETRESET      1

MagnetometerStruct MagnetometerLeft,MagnetometerRight;

//static int chip_id = QMC7983;



#if defined(USE_SW_I2C)
	extern void i2c_GPIO_Config(void);
#else
	extern void i2c_config(void);
#endif
	
int qmcX983_slave_reset(uint8_t address,MagnetometerStruct * magnet);
int qmcX983_read_chipid(uint8_t address,MagnetometerStruct * magnet);

 int I2C_RxData(uint8_t address,uint8_t reg_add,uint8_t *buf,uint8_t num,MagnetometerStruct * magnet)
{
	int res = 0,loop_i=0 ;

	for(loop_i = 0; loop_i < QMCX983_RETRY_COUNT; loop_i++)
	{
		#if defined(USE_SW_I2C)
		res = qst_sw_readreg(address, reg_add, buf,num);
		#else
		res = i2c_receive_data(address, reg_add, buf,num);
		#endif
		if(res !=0)
		{ 
		//	printf("I2C_RxData i2c %x %d \n",res,loop_i);
			return 1;
		}
		else if(QMCX983_RETRY_COUNT == 6)//(loop_i == 6)//
		{
			printf("I2C_RxData qmcX983 error %x %d\n",res,loop_i);
		    qmcX983_slave_reset(address,magnet);
			return 0;
		}
	}
    return 0;
}

 int I2C_TxData(uint8_t address ,uint8_t reg_add,uint8_t reg_dat,MagnetometerStruct * magnet)
{
	int res = 0,loop_i ;

	for(loop_i = 0; loop_i < QMCX983_RETRY_COUNT; loop_i++)
	{
	#if defined(USE_SW_I2C)
	res = qst_sw_writereg(address, reg_add, reg_dat);
	#else
	res = i2c_send_data(address, reg_add, &reg_dat, 1);
	#endif
		if(res !=0)
		{ 
			//printf("I2C_TxData i2c %x %d \n",res,loop_i);
			return 1;
		}
		else if(QMCX983_RETRY_COUNT == 6)
		{
			printf("I2C_TxData qmcX983 error %x %d\n",res,loop_i);
			qmcX983_slave_reset(address,magnet);
			return 0;
		}
	}
	return 0;
}


/* Set the Gain range */
void qmcX983_set_range(uint8_t address,short range,MagnetometerStruct * magnet)
{
	unsigned char data[2];

	int ran ;
	switch (range) {
	case QMCX983_RNG_2G:
		ran = RNG_2G;
		break;
	case QMCX983_RNG_8G:
		ran = RNG_8G;
		break;
	case QMCX983_RNG_12G:
		ran = RNG_12G;
		break;
	case QMCX983_RNG_20G:
		ran = RNG_20G;
		break;
	}
	
	magnet->xyz_sensitivity = 20000/ran;

	I2C_RxData(address,CTL_REG_ONE,data, 1,magnet);

	data[0] &= 0xcf;
	data[0] |= (range << 4);
	data[1] = data[0];

	data[0] = CTL_REG_ONE;
	I2C_TxData(address,CTL_REG_ONE,data[1],magnet);
}

//进入正常模式
 void qmcX983_enable(uint8_t address,MagnetometerStruct * magnet)
{
  // change the peak to 1us from 2 us 
	unsigned char data;
 //   printf("qmcX983_enable start \n");
	data = 0x1;
	I2C_TxData(address,0x21,data,magnet);

	data = 0x40;
	I2C_TxData(address,0x20,data,magnet);

	if(magnet->chip_id == QMC7983 || magnet->chip_id == QMC7983_LOW_SETRESET)
	{
		data = 0x80;
		I2C_TxData(address,0x29,data,magnet); 		
		data = 0x0c;
		I2C_TxData(address,0x0a,data,magnet);				
	}
	
	data = 0x1d;
	I2C_TxData(address,CTL_REG_ONE,data,magnet);
	qmcX983_set_range(address,QMCX983_RNG_8G,magnet);
	data = QMCX983_SETRESET_FREQ_FAST;
    I2C_TxData(address,0x0b,data,magnet);

//	delay_1ms(5);
}

void QmcInit(void){
    qmcX983_slave_reset(QMCX983_SLAVE_ADDRESS_2D,&MagnetometerLeft);
    qmcX983_slave_reset(QMCX983_SLAVE_ADDRESS_2C,&MagnetometerRight);
}

void qmcX983_i2c_init(void)
{
#if defined(USE_SW_I2C)
	i2c_CheckDevice(QMCX983_SLAVE_ADDRESS_2C);
	i2c_CheckDevice(QMCX983_SLAVE_ADDRESS_2D);
//	i2c_GPIO_Config();
 //  i2c_Send9CLK_INIT();
//	printf("use sw i2c \n");
 #else
	i2c_config();
	printf("use hw i2c \n");
 #endif 
}

int qmcX983_slave_reset(uint8_t address,MagnetometerStruct * magnet)
{
	int loop_i ;	

	qmcX983_i2c_init();

	for(loop_i = 0; loop_i < QMCX983_RETRY_COUNT; loop_i++)
	{
		if(qmcX983_read_chipid(address,magnet) == 0x32)  
		{ 
			qmcX983_enable(address,magnet);
			return 1;
		}
		else
		{
			qmcX983_i2c_init();
			return 0;
		}
	}
	//return 0;
}


 int qmcX983_read_chipid(uint8_t address,MagnetometerStruct * magnet)
{	

	uint8_t chipid = 0;
	int res = 0;
    magnet->chip_id =255;
	/* read chip id */
//	printf("qmcX983_read_chipid \n");
	res = I2C_RxData(address,0x0d,&chipid,1,magnet);
	if(res == 0)
	{
	}
	
//	printf("QMCX983_read_chipid %x!\n",chipid);
	if(chipid == 0x31) //qmc7983 asic 
	{
		I2C_RxData(address,0x3E,&chipid,1,magnet);
		if(res == 0)
		{
			//qmcX983_slave_reset(address,magnet);
			//return 0;
		}
		
		if((chipid & 0x20) == 0)
			magnet->chip_id = QMC7983;
			//printf("QMCX983 I2C driver registered!\n");
	}
	else if(chipid == 0x32) //qmc7983 asic low setreset
	{
		magnet->chip_id = QMC7983_LOW_SETRESET;
		//printf("QMCX983 I2C driver registered!\n");
	}
	return chipid;
}

/* reset compass and check compass ID */
static int qmcX983_reset_check(uint8_t address,MagnetometerStruct * magnet)
{
	I2C_TxData(address,0x0a,0x80,magnet);
	
	if (qmcX983_read_chipid(address,magnet) == 0x32)
	{
		qmcX983_enable(address,magnet);
		magnet->error=0;
	    magnet->warningcount=0;
		return 1;
	}
	if(magnet->warningcount > 100)
	{
		magnet->error=1;
	}
	else
	{
	   magnet->warningcount++ ;
	}	

	return 0;
}

static void qmcX983_data_check(uint8_t address,int *mag_data,MagnetometerStruct * magnet)
{ 
	uint8_t databuf = 0;
	uint8_t rdy = 0;
	uint8_t rct = 0;
	uint8_t res = 0;
	uint8_t data[6] = { 0 };
	
//	printf("qmcX983_data_check start \n");
#if 0	
	databuf = 0x1e;
	I2C_TxData(address,CTL_REG_ONE,databuf,magnet); //大于20000 启动selftest模式（09寄存器写入1E）

	databuf = 0x04;
	I2C_TxData(address,0x0a,databuf,magnet);
#endif	
//	delay_1ms(5);		// remove by QST 2019/03/06
	
	while((rdy != 0x1c)&&(rct < 10))
	{
		res=I2C_RxData(address,CTL_REG_ONE,&rdy,1,magnet); //等待09 寄存器 准备完毕
		//printf("qmcX983_data_check data reset 1C %x,%x\n",rct,rdy);
        //delay_1ms(1);		// removed by QST 2019/03/06
		if(rct == 5)
		{
			qmcX983_reset_check(address,magnet);
			databuf = 0x1e;
			I2C_TxData(address,CTL_REG_ONE,databuf,magnet);
			databuf = 0x04;
			I2C_TxData(address,0x0a,databuf,magnet);
			//printf("qmcX983_data_check data reset \n");
			magnet->selftest_flag = 1;		// add by QST , entry selftest again, read selftest data next time
			return;
			//break;
		}
		rct ++;
	}
	
	magnet->selftest_flag = 0;		// add by QST , clear selftest flag 
	I2C_RxData(address,OUT_X_L,data,6,magnet);

	mag_data[0] = (short) (((data[1]) << 8) | data[0]);
	mag_data[1] = (short) (((data[3]) << 8) | data[2]);
	mag_data[2] = (short) (((data[5]) << 8) | data[4]);
	
	if (mag_data[0] <= 1000 || mag_data[1] <= 1000) 
	{
		magnet->dateovely = 2;
		mag_data[0] = 65536;
		mag_data[1] = 65536;
		mag_data[2] = 65536;		
	    qmcX983_enable(address,magnet);
	//	printf("qmcX983_data_check data [%d, %d, %d] _A\n", mag_data[0], mag_data[1], mag_data[2]);
		return;
	}
	else if(magnet->dateovely_flag >= 50)
	{
		magnet->dateovely = 1;
		magnet->dateovely_flag = 0;
	}
	else
	{
		magnet->dateovely_flag ++;
	}
	
	//printf("qmcX983_data_check data [%d, %d, %d] _A\n", mag_data[0], mag_data[1], mag_data[2]);
	qmcX983_enable(address,magnet);
	
	while((rdy & 0x07)&&(rct < 5))
	{
		res=I2C_RxData(address,STA_REG_ONE,&rdy,1,magnet);//是否数据准备好
		
		if((res == 0)&&(rct == 4))
		{
			qmcX983_reset_check(address,magnet);
			break;
		}
		rct ++;
	  //printf("qmcX983_read_mag_xyz_rdy %x!\n",rdy);
	}

	I2C_RxData(address,OUT_X_L,data,6,magnet);

	mag_data[0] = (short) (((data[1]) << 8) | data[0]);
	mag_data[1] = (short) (((data[3]) << 8) | data[2]);
	mag_data[2] = (short) (((data[5]) << 8) | data[4]);
	
	//printf("qmcX983_data_check data [%d, %d, %d] _B\n", mag_data[0], mag_data[1], mag_data[2]);
	
	return ;
}

uint8_t buf222[2];
void qmcX983_read_mag_xyz(uint8_t address , MagnetometerStruct * magnet)
{
	uint8_t rdy = 0;
	uint8_t rct = 0;
	int res;
	int mag_data[3]={0};
	unsigned char databuf[6]={0};
	
	if(magnet->error==1)
	{
		magnet->data[0] = 0 ;// * 1000 / xyz_sensitivity;
		magnet->data[1] = 0 ;// * 1000 / xyz_sensitivity;
		magnet->data[2] = 0 ;// * 1000 / xyz_sensitivity;
	   return ;
	}	
	
	if(magnet->stata_flag < 100)
	{
		qmcX983_data_check(address,mag_data,magnet);
		magnet->stata_flag ++;
		return;
    }
	/* Check status register for data availability */
	if(magnet->selftest_flag)
	{
		qmcX983_data_check(address,mag_data,magnet);
	}
	else
	{
		while((rdy & 0x07)&&(rct < 5))
		{
			res=I2C_RxData(address,STA_REG_ONE,&rdy,1,magnet);
			//printf("qmcX983_read_mag_xyz data drdy 06 %x,%x\n",rct,rdy);
			if((res == 0)&&(rct == 4))
			{
				qmcX983_reset_check(address,magnet);
				break;
			}
			rct ++;
			//printf("qmcX983_read_mag_xyz_rdy %x!\n",rdy);
		}

		I2C_RxData(address,OUT_X_L,databuf,6,magnet);

		mag_data[0] = (short) (((databuf[1]) << 8) | databuf[0]);
		mag_data[1] = (short) (((databuf[3]) << 8) | databuf[2]);
		mag_data[2] = (short) (((databuf[5]) << 8) | databuf[4]);
		
		// printf("qmcX983_read_mag_xyz_rdy dateovely ! %d\n",dateovely);
		
		if((abs(mag_data[0]) > 20000)||(abs(mag_data[1]) > 20000 ||abs(mag_data[2]) > 20000)||(magnet->dateovely == 2)) 
		{
			 //printf("qmcX983_read_mag_xyz_rdy So big !\n");
		// add by QST 2019/03/06, start selftest mode	
			 I2C_TxData(address,CTL_REG_ONE,0x1e,magnet); //大于20000 启动selftest模式（09寄存器写入1E）
			 I2C_TxData(address,0x0a,0x04,magnet);
			 magnet->selftest_flag = 1;
			 return;
		// add by QST 2019/03/06,  start selftest mode
			 //qmcX983_data_check(address,mag_data,magnet);		// remove by QST 2019/03/06, get selftest data next read
		}
	
		magnet->data[0] = mag_data[0] ;// * 1000 / xyz_sensitivity;
		magnet->data[1] = mag_data[1] ;// * 1000 / xyz_sensitivity;
		magnet->data[2] = mag_data[2] ;// * 1000 / xyz_sensitivity;
	}
	
	printf("qmcX983_read_mag_xyz data [%d, %d, %d] _A\n", mag_data[0], mag_data[1], mag_data[2]);
}




 void qmcX983_disable(uint8_t address,MagnetometerStruct * magnet)
{
	unsigned char data;
	printf("stop measure!\n");
	data = 0x1c;	
	I2C_TxData(address,CTL_REG_ONE,data,magnet);
}


void QmcX983_Read_Mag(void)
{	
//	Get_Call_Timer(&RunTimerTestQm);	
//	qmcX983_read_mag_xyz(QMCX983_SLAVE_ADDRESS_2D,&MagnetometerLeft);
	qmcX983_read_mag_xyz(QMCX983_SLAVE_ADDRESS_2C,&MagnetometerRight);
//	Get_Call_Timer(&RunTimerTestQm);
}



/*
3gs  转弯   小于1.2gs 停止 反转 到1.8gs   1gs ==2500 LSB


*/
