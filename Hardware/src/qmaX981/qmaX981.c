/**
  ******************************************************************************
  * @file    qmaX981.c
  * @author  Yangzhiqiang@qst
  * @version V1.0
  * @date    2017-12-15
  * @brief    qmaX981驱动
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火 指南者 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 

#include "qmaX981.h"
#if defined(OLED_SUPPORT)
#include "oled.h"
#endif
#include "bsp_usart.h"

#define QMAX981_LOG		console_write
#define QMAX981_ERR		console_write


#if defined(QMAX981_STEP_DEBOUNCE_IN_INT)
extern void qmaX981_step_debounce_reset(void);
extern int32_t qmaX981_step_debounce_int32_t_work(int32_t data, uint8_t irq_level);
extern int32_t qmaX981_step_debounce_read_data(int32_t result);
#endif
#if defined(QMAX981_CHECK_ABNORMAL_DATA)
extern int32_t qmaX981_check_abnormal_data(int32_t data_in, int32_t *data_out);
#endif


typedef enum
{	
	CHIP_TYPE_QMA6981 = 0,
	CHIP_TYPE_QMA7981,
	CHIP_TYPE_QMA6100,
	CHIP_TYPE_UNDEFINE,
	CHIP_TYPE_MAX
}qmaX981_type;

typedef struct
{
    int16_t sign[3];
    uint16_t map[3];
}qst_convert;

typedef struct
{
	uint8_t					chip_id;
	qmaX981_type			chip_type;
	int32_t					lsb_1g;
	uint8_t					layout;
	qst_convert				cvt;
	uint8_t					int32_t_level;
}qmaX981_data;

static const qst_convert qst_map[] = 
{
    { { 1, 1, 1}, {0, 1, 2} },
    { {-1, 1, 1}, {1, 0, 2} },
    { {-1,-1, 1}, {0, 1, 2} },
    { { 1,-1, 1}, {1, 0, 2} },

    { {-1, 1, -1}, {0, 1, 2} },
    { { 1, 1, -1}, {1, 0, 2} },
    { { 1,-1, -1}, {0, 1, 2} },
    { {-1,-1, -1}, {1, 0, 2} }
};


static qmaX981_data g_qmaX981;
#if !defined(USE_SPI)
static uint8_t QMAX981_I2C_ADDR_W	= QMAX981_I2C_SLAVE_ADDR;
#endif
#if defined(QMA7981_DOUBLE_TRIPLE_CLICK)
static unsigned int acc_data_curr[3];
static unsigned int acc_data[3];
static qst_click_check g_click;
#endif
#if defined(QMA7981_ABNORMAL_SHAKE_CHECK)
static qst_shake_check g_shake;
#endif

const uint8_t qma6981_init_tbl[][2] = 
{
#if defined(QMAX981_STEPCOUNTER)
	{0x11, 0x80},
	{0x36, 0xb6},
	{0xff, 5},
	{0x36, 0x00},
	{0x11, 0x80},
	{0x0f, QMAX981_RANGE_8G},
	{0x10, 0x2a},
	{0x12, 0x8f},
	{0x13, 0x10},
	{0x14, 0x14},
	{0x15, 0x10},	
	{0x16, 0x0c},
#if defined(QMAX981_STEP_DEBOUNCE_IN_INT)
	{0x19, 0x08},
#endif
	{0x32, 0x02},
	{0x27, QMA6981_OFFSET},
	{0x28, QMA6981_OFFSET},
	{0x29, QMA6981_OFFSET},
#else
	{0x11, 0x80},
	{0x36, 0xb6},
	{0xff, 5},
	{0x36, 0x00},
	{0x11, 0x80},
	//{0x36, 0xb6},
	{0xff, 5},
	//{0x36, 0x00},
	//{0x11, 0x80},
	{0x0f, QMAX981_RANGE_4G},
	{0x10, QMA6981_ODR_125HZ},
#endif
#if defined(QMAX981_FIFO_FUNC)
	{0x10, QMA6981_ODR_250HZ},
	{0x11, 0x8b},
	{0x3E, 0x40},
	{0x17, 0x20},
	#if defined(QMAX981_FIFO_USE_INT)
	{0x1a, 0x20},	// fifo int32_t map to int32_t1
	#endif
#endif
#if defined(QMAX981_TAP_FUNC)
	{0x10, 0x05},
	{0x11, 0x80},	// 0x85 {0x2a, 0x80},	
	{0x2b, 0x05},	//0x14	
	{0x16, 0x20},	
	{0x19, 0x20},
	//{0x1b, 0x20},
#endif
#if defined(QMAX981_INT_LATCH_MODE)
	{0x21, 0x01},
#endif

	{0xff, 1}
};

/*	
qma7981 odr setting
0x10<2:0>		ODR(Hz)				Time(ms)	|	RANGE 0x0f<3:0>
000				43.3125				23.088		|	0001	2g  		244ug/LSB
001				86.4453				11.568		|	0010	4g  		488ug/LSB
002				172.1763			5.808		|	0100	8g  		977ug/LSB
003				341.5300			2.928		|	1000	16g  	1.95mg/LSB
004				672.0430			1.488		|	1111	32g  	3.91mg/LSB
005				32.5013				30.768		|	Others	2g  		244ug/LSB
006				129.3995			7.728		|
007				257.2016			3.888		|
*/

const uint8_t qma7981_init_tbl[][2] = 
{
	{0x11, 0x80},
	{0x36, 0xb6},
	{0xff, 5},
	{0x36, 0x00},
	{0x0f, QMAX981_RANGE_4G},
	{0x10, 0xe1},		// ODR 130hz	
	//{0x4a, 0x08},		//Force I2C I2C int32_terface.SPI is disabled,SENB can be used as ATB
	//{0x20, 0x05},	
	{0x11, 0x80},
	{0x5f, 0x80},		// enable test mode,take control the FSM
	{0x5f, 0x00},		//normal mode

	{0xff, 1}
};


void qmaX981_delay(uint32_t delay)
{
	uint32_t i,j;
	for(i=0;i<delay;i++)
	{
		for(j=0;j<1000;j++)
		{
			;
		}
	}
}

uint8_t qmaX981_writereg(uint8_t reg_add,uint8_t reg_dat)
{
#if defined(USE_SPI)
	return qmaX981_spi_write(reg_add, reg_dat);
#else
	#if defined(QMAX981_USE_SW_IIC)
	return qst_sw_writereg(QMAX981_I2C_ADDR_W<<1, reg_add, reg_dat);
	#else
	I2C_Bus_set_slave_addr(QMAX981_I2C_ADDR_W<<1);
	return I2C_ByteWrite(reg_dat,reg_add);
	#endif
#endif
}

uint8_t qmaX981_readreg(uint8_t reg_add,uint8_t *buf,uint8_t num)
{
#if defined(USE_SPI)
	return qmaX981_spi_read(reg_add, buf, num);
#else
	#if defined(QMAX981_USE_SW_IIC)
	return qst_sw_readreg(QMAX981_I2C_ADDR_W<<1, reg_add, buf, num);
	#else
	I2C_Bus_set_slave_addr(QMAX981_I2C_ADDR_W<<1);
	return I2C_BufferRead(buf,reg_add,(uint16_t)num);
	#endif
#endif
}


uint8_t qmaX981_chip_id()
{
	uint8_t chip_id = 0x00;
	qmaX981_writereg(QMAX981_REG_POWER_CTL, 0x80);

	qmaX981_readreg(QMAX981_CHIP_ID, &chip_id, 1);
	QMAX981_LOG("qmaX981_chip_id id=0x%x \n", chip_id);

	return chip_id;
}


void qmaX981_set_range(uint8_t range)
{
	uint8_t ret;

	if(g_qmaX981.chip_type == CHIP_TYPE_QMA6981)
	{	
		if(range == QMAX981_RANGE_4G)
			g_qmaX981.lsb_1g = 128;
		else if(range == QMAX981_RANGE_8G)
			g_qmaX981.lsb_1g = 64;
		else					
			g_qmaX981.lsb_1g = 256;
	}
	else if((g_qmaX981.chip_type == CHIP_TYPE_QMA7981)||(g_qmaX981.chip_type == CHIP_TYPE_QMA6100))
	{
		if(range == QMAX981_RANGE_4G)
			g_qmaX981.lsb_1g = 2048;
		else if(range == QMAX981_RANGE_8G)
			g_qmaX981.lsb_1g = 1024;
		else if(range == QMAX981_RANGE_16G)
			g_qmaX981.lsb_1g = 512;
		else if(range == QMAX981_RANGE_32G)
			g_qmaX981.lsb_1g = 256;
		else
			g_qmaX981.lsb_1g = 4096;
	}

	ret = qmaX981_writereg(QMAX981_REG_RANGE, range);	
	if(ret == 0){
		QMAX981_ERR("qmaX981_set_range error!!!\n");
	}
}

static int32_t qma6981_read_raw_xyz(int32_t *data)
{
	//int32_t res;	
	uint8_t databuf[6] = {0};		
	uint8_t i;
	int32_t ret;

	ret = qmaX981_readreg(QMAX981_XOUTL, databuf, 6);
	if(ret == 0){
		QMAX981_ERR("read xyz error!!!\n");
		return 0;	
	}
 	data[0]  = (short)((databuf[1]<<2) |( databuf[0]>>6));
	data[1]  = (short)((databuf[3]<<2) |( databuf[2]>>6));
	data[2]  = (short)((databuf[5]<<2) |( databuf[4]>>6));

	for(i=0;i<3;i++)				
	{	//because the data is store in binary complement number formation in computer system
		if ( data[i] == 0x0200 )	//so we want to calculate actual number here
			data[i]= -512;		//10bit resolution, 512= 2^(10-1)
		else if ( data[i] & 0x0200 )	//transfor format
		{					//print32_tk("data 0 step %x \n",data[i]);
			data[i] -= 0x1;			//print32_tk("data 1 step %x \n",data[i]);
			data[i] = ~data[i];		//print32_tk("data 2 step %x \n",data[i]);
			data[i] &= 0x01ff;		//print32_tk("data 3 step %x \n\n",data[i]);
			data[i] = -data[i];		
		}
#if defined(QMAX981_STEP_COUNTER)
		data[i] -= QMA6981_OFFSET;
#endif
	}

	//print32_tk("yzqaccraw	%d	%d	%d\n", data[0], data[1], data[2]);
	return 1;
}

static int32_t qma7981_read_raw_xyz(int32_t *data)
{
	uint8_t databuf[6] = {0}; 	
	int32_t ret;
	//qma7981_acc_format data_14bit;

	ret = qmaX981_readreg(QMAX981_XOUTL, databuf, 6);
	if(ret == 0){
		QMAX981_ERR("7981 read xyz error!!!\n");
		return 0;	
	}

	data[0] = (short)((databuf[1]<<8)|(databuf[0]));
	data[1] = (short)((databuf[3]<<8)|(databuf[2]));
	data[2] = (short)((databuf[5]<<8)|(databuf[4]));
	data[0] = data[0]>>2;
	data[1] = data[1]>>2;
	data[2] = data[2]>>2;

	return 1;
}

int32_t qmaX981_read_raw(int32_t *rawData)
{
	int32_t ret;

	if(g_qmaX981.chip_type == CHIP_TYPE_QMA6981)		
		ret = qma6981_read_raw_xyz(rawData);	
	else if((g_qmaX981.chip_type == CHIP_TYPE_QMA7981)||(g_qmaX981.chip_type == CHIP_TYPE_QMA6100))	
		ret = qma7981_read_raw_xyz(rawData);
	else
		ret = 0;

	return ret;
}


int32_t qmaX981_read_acc(int32_t *accData)
{
	int32_t ret;
	int32_t rawData[3];

	ret = qmaX981_read_raw(rawData);
	accData[g_qmaX981.cvt.map[0]] = g_qmaX981.cvt.sign[0]*rawData[0];
	accData[g_qmaX981.cvt.map[1]] = g_qmaX981.cvt.sign[1]*rawData[1];
	accData[g_qmaX981.cvt.map[2]] = g_qmaX981.cvt.sign[2]*rawData[2];

	accData[0] = (accData[0]*GRAVITY_EARTH_1000)/(g_qmaX981.lsb_1g);
	accData[1] = (accData[1]*GRAVITY_EARTH_1000)/(g_qmaX981.lsb_1g);
	accData[2] = (accData[2]*GRAVITY_EARTH_1000)/(g_qmaX981.lsb_1g);

	//if(ret)
	//{
		//QMAX981_LOG("acc %f %f %f\n",accData[0]/1000.0f, accData[1]/1000.0f, accData[2]/1000.0f);
	//}

	return ret;

}

#if defined(QMAX981_STEPCOUNTER)
uint32_t qmaX981_read_stepcounter(void)
{
	uint8_t data[3];
	int32_t ret;
	uint32_t step_num;

	if(g_qmaX981.chip_type == CHIP_TYPE_QMA6981)
	{
		ret = qmaX981_readreg(QMAX981_STEP_CNT_L, data, 2);
		step_num = (data[1]<<8)|data[0];
	}
	else if(g_qmaX981.chip_type == CHIP_TYPE_QMA7981)
	{	
		ret = qmaX981_readreg(QMAX981_STEP_CNT_L, data, 2);
		ret = qmaX981_readreg(QMA7981_STEP_CNT_M, &data[2], 1);
		step_num = (uint32_t)(((uint32_t)data[2]<<16)|((uint32_t)data[1]<<8)|data[0]);
	}
#if defined(QMAX981_CHECK_ABNORMAL_DATA)
	ret=qmaX981_check_abnormal_data(step_num, &step_num);
	if(ret != 0)
	{
		return -1;
	}
#endif
#if defined(QMAX981_STEP_DEBOUNCE_IN_INT)
	step_num = qmaX981_step_debounce_read_data(step_num);
#endif

	return step_num;
}
#endif

#if defined(QMAX981_FIFO_FUNC)
static int32_t qmaX981_fifo_data[32][3];

static int32_t qma6981_read_fifo_raw(int32_t *data)
{
	//int32_t res;	
	uint8_t databuf[6] = {0};		
	uint8_t i;
	int32_t ret;
	
	ret = qmaX981_readreg(0x3f, databuf, 6);
	qmaX981_delay(2);
	if(ret != 1)
	{
		QMAX981_LOG("qma6981_read_fifo_raw error \n");
		return ret;
	}

 	data[0]  = (short)((databuf[1]<<2) |( databuf[0]>>6));
	data[1]  = (short)((databuf[3]<<2) |( databuf[2]>>6));
	data[2]  = (short)((databuf[5]<<2) |( databuf[4]>>6));

	for(i=0;i<3;i++)				
	{	//because the data is store in binary complement number formation in computer system
		if ( data[i] == 0x0200 )	//so we want to calculate actual number here
			data[i]= -512;		//10bit resolution, 512= 2^(10-1)
		else if ( data[i] & 0x0200 )	//transfor format
		{					//print32_tk("data 0 step %x \n",data[i]);
			data[i] -= 0x1;			//print32_tk("data 1 step %x \n",data[i]);
			data[i] = ~data[i];		//print32_tk("data 2 step %x \n",data[i]);
			data[i] &= 0x01ff;		//print32_tk("data 3 step %x \n\n",data[i]);
			data[i] = -data[i];		
		}
#if defined(QMAX981_STEP_COUNTER)
		data[i] -= QMA6981_OFFSET;
#endif
	}
	//print32_tk("qma6981 fifo raw: %d	%d	%d\n", data[0], data[1], data[2]);	

	return 1;	
}

static int32_t qma7981_read_fifo_raw(int32_t *data)
{
	int32_t res;	
	uint8_t databuf[6] = {0};
	int32_t ret;
	
	ret = qmaX981_readreg(0x3f, databuf, 6);
	qmaX981_delay(2);
	if(ret != 1)
	{
		QMAX981_LOG("qma7981_read_fifo_raw error \n");
		return ret;
	}

	data[0] = (short)((databuf[1]<<8)|(databuf[0]));
	data[1] = (short)((databuf[3]<<8)|(databuf[2]));
	data[2] = (short)((databuf[5]<<8)|(databuf[4]));
	data[0] = data[0]>>2;
	data[1] = data[1]>>2;
	data[2] = data[2]>>2;

	//print32_tk("qma7981 fifo raw: %d	%d	%d\n", data[0], data[1], data[2]);	
	return 1;
}

static int32_t qmaX981_read_fifo_acc(int32_t *acc_data)
{
	int32_t ret = 0;
	int32_t raw_data[3];

	if(g_qmaX981.chip_type == CHIP_TYPE_QMA6981)
	{
		ret = qma6981_read_fifo_raw(raw_data);
	}
	else if((g_qmaX981.chip_type == CHIP_TYPE_QMA7981)||(g_qmaX981.chip_type == CHIP_TYPE_QMA6100))
	{
		ret = qma7981_read_fifo_raw(raw_data);
	}
	else
	{
		ret = 0;
	}
	
	if(1 != ret ){
		QMAX981_ERR("qmaX981_read_fifo_acc error\n");
		return ret;
	}
	
	//remap coordinate
	acc_data[g_qmaX981.cvt.map[0]] = g_qmaX981.cvt.sign[0]*raw_data[0];
	acc_data[g_qmaX981.cvt.map[1]] = g_qmaX981.cvt.sign[1]*raw_data[1];
	acc_data[g_qmaX981.cvt.map[2]] = g_qmaX981.cvt.sign[2]*raw_data[2];
	//QMAX981_LOG("qmaX981 AFTER x1:%d,y:%d,z:%d\n",data[0],data[1],data[2]);

	acc_data[0] = (acc_data[0]*9807)/(g_qmaX981.lsb_1g);
	acc_data[1] = (acc_data[1]*9807)/(g_qmaX981.lsb_1g);
	acc_data[2] = (acc_data[2]*9807)/(g_qmaX981.lsb_1g);

	return ret;
}

static int32_t qmaX981_read_fifo(uint8_t is_raw)
{
	int32_t ret = 0;
	uint8_t databuf[2];
	int32_t acc_data[3];
	int32_t icount;
	int32_t fifo_depth = 32;

	if(g_qmaX981.chip_type == CHIP_TYPE_QMA6100)
		fifo_depth = 64;
	else if(g_qmaX981.chip_type == CHIP_TYPE_QMA7981)
		fifo_depth = 16;
	else
		fifo_depth = 32;

	ret = qmaX981_readreg(QMAX981_FIFO_STATE, databuf, 1);
	qmaX981_delay(2);

	QMAX981_LOG("fifo level = %d   %d \r\n", fifo_depth, databuf[0]&0x7f);

	if((databuf[0]&0x7f)==fifo_depth)
	{
		for(icount=0; icount<fifo_depth; icount++)
		{
			if(is_raw == 1)
			{
				if(g_qmaX981.chip_type == CHIP_TYPE_QMA6981)
				{
					ret = qma6981_read_fifo_raw(acc_data);
				}
				else if(g_qmaX981.chip_type == CHIP_TYPE_QMA7981)
				{
					ret = qma7981_read_fifo_raw(acc_data);
				}
				else
				{
					ret = 0;
				}
			}
			else
			{
				ret = qmaX981_read_fifo_acc(acc_data);
			}
			
			if(ret != 1)
			{
				QMAX981_ERR("read 0x3f error!\n");
				return ret;
			}
			qmaX981_fifo_data[icount][0] = acc_data[0];
			qmaX981_fifo_data[icount][1] = acc_data[1];
			qmaX981_fifo_data[icount][2] = acc_data[2];
			QMAX981_LOG("fifo_data %d: %f	%f	%f \r\n", icount, acc_data[0]/1000.00, acc_data[1]/1000.00, acc_data[2]/1000.00);
		}
		// read status reg
	}
	else
	{
		ret = 0;
	}
	ret = qmaX981_readreg(QMAX981_INT_STAT1, databuf, 1);
	qmaX981_delay(2);
	// write 0x3e
	ret = qmaX981_writereg(0x3e, 0x40);
	qmaX981_delay(2);

	return ret;
}
#endif

static int32_t qma6981_initialize(void)
{
	int32_t ret = 0;
	int32_t index, total;
	uint8_t data[2] = {0};

	total = sizeof(qma6981_init_tbl)/sizeof(qma6981_init_tbl[0]);
	for(index=0; index<total; index++)
	{	
		data[0] = qma6981_init_tbl[index][0];
		data[1] = qma6981_init_tbl[index][1];
		if(data[0] == 0xff)
		{
			qmaX981_delay(data[1]);
		}
		else
		{
			if(data[0] == QMAX981_REG_RANGE)
			{
				if(data[1] == QMAX981_RANGE_4G)
					g_qmaX981.lsb_1g = 128;
				else if(data[1] == QMAX981_RANGE_8G)
					g_qmaX981.lsb_1g = 64;
				else					
					g_qmaX981.lsb_1g = 256;
			}

			ret = qmaX981_writereg(data[0],data[1]);
			if(ret == 0)
			{
				QMAX981_ERR("qma6981_initialize ret=%d reg_addr=%x \n", ret, data[0]);
				//return ret;
			}
			qmaX981_delay(2);
		}
	}

   	return ret;
}


static int32_t qma7981_initialize(void)
{
	int32_t ret = 0;
	int32_t index, total;
	uint8_t data[2] = {0};
	uint8_t reg_0x10 = 0;
	uint8_t reg_0x16 = 0;
	uint8_t reg_0x18 = 0;
	uint8_t reg_0x19 = 0;
	uint8_t reg_0x1a = 0;
#if defined(QMA7981_ANY_MOTION)||defined(QMA7981_NO_MOTION)
	uint8_t reg_0x2c = 0;
#endif

	total = sizeof(qma7981_init_tbl)/sizeof(qma7981_init_tbl[0]);
	for(index=0; index<total; index++)
	{	
		data[0] = qma7981_init_tbl[index][0];
		data[1] = qma7981_init_tbl[index][1];
		if(data[0] == 0xff)
		{
			qmaX981_delay(data[1]);
		}
		else
		{
			if(data[0] == QMAX981_REG_RANGE)
			{
				if(data[1] == QMAX981_RANGE_4G)
					g_qmaX981.lsb_1g = 2048;
				else if(data[1] == QMAX981_RANGE_8G)
					g_qmaX981.lsb_1g = 1024;
				else if(data[1] == QMAX981_RANGE_16G)
					g_qmaX981.lsb_1g = 512;
				else if(data[1] == QMAX981_RANGE_32G)
					g_qmaX981.lsb_1g = 256;
				else
					g_qmaX981.lsb_1g = 4096;
			}
			ret = qmaX981_writereg(data[0],data[1]);
			if(ret == 0)
			{
				QMAX981_ERR("qma7981_initialize ret=%d\n", ret);
				return ret;
			}
			qmaX981_delay(2);
		}
	}

	// read reg
	qmaX981_readreg(0x16, &reg_0x16, 1);
	qmaX981_readreg(0x18, &reg_0x18, 1);
	qmaX981_readreg(0x19, &reg_0x19, 1);
	qmaX981_readreg(0x1a, &reg_0x1a, 1);
#if defined(QMA7981_ANY_MOTION)||defined(QMA7981_NO_MOTION)
	qmaX981_readreg(0x2c, &reg_0x2c, 1);
#endif
	// read reg
	reg_0x10 = 0xe1;
	qmaX981_writereg(0x10, reg_0x10);
#if defined(QMAX981_STEPCOUNTER)
	if(reg_0x10 == 0xe0)
	{
		// ODR: 65hz 15.48 ms
		qmaX981_writereg(0x12, 0x94);
		qmaX981_writereg(0x13, 0x80);		// clear step
		qmaX981_writereg(0x13, 0x00);		// 
		qmaX981_writereg(0x14, 0x12);		// STEP_TIME_LOW<7:0>*(1/ODR) 
		qmaX981_writereg(0x15, 0x10);		// STEP_TIME_UP<7:0>*8*(1/ODR) 
	}
	else if(reg_0x10 == 0xe1)
	{
		// ODR: 130hz 7.74 ms
		qmaX981_writereg(0x12, 0x94);
		qmaX981_writereg(0x13, 0x80);		// clear step
		qmaX981_writereg(0x13, 0x00);		// 
		qmaX981_writereg(0x14, 0x24);		// STEP_TIME_LOW<7:0>*(1/ODR) 
		qmaX981_writereg(0x15, 0x20);		// STEP_TIME_UP<7:0>*8*(1/ODR) 
	}
	else if(reg_0x10 == 0xe2)
	{
		// ODR: 258Hz 3.87 ms
		qmaX981_writereg(0x12, 0x94);
		qmaX981_writereg(0x13, 0x80);		// clear step
		qmaX981_writereg(0x13, 0x00);		// 
		qmaX981_writereg(0x14, 0x48);		// STEP_TIME_LOW<7:0>*(1/ODR) 
		qmaX981_writereg(0x15, 0x40);		// STEP_TIME_UP<7:0>*8*(1/ODR) 
	}

	//qmaX981_writereg(0x1f, 0x00);

	// step int
	#if defined(QMA7981_STEP_INT)
	reg_0x16 |= 0x08;
	reg_0x19 |= 0x08;
	qmaX981_writereg(0x16, reg_0x16);
	qmaX981_writereg(0x19, reg_0x19);
	#endif
	#if defined(QMA7981_SIGNIFICANT_STEP)
	qmaX981_writereg(0x1d, 0x26);		//every 30 step
	reg_0x16 |= 0x40;
	reg_0x19 |= 0x40;
	qmaX981_writereg(0x16, reg_0x16);
	qmaX981_writereg(0x19, reg_0x19);
	#endif
#endif

//RANGE<3:0> Acceleration range Resolution
//0001 2g 244ug/LSB
//0010 4g 488ug/LSB
//0100 8g 977ug/LSB
//1000 16g 1.95mg/LSB
//1111 32g 3.91mg/LSB
//Others 2g 244ug/LSB

//0x2c
//Duration = (NO_MOT_DUR<3:0> + 1) * 1s, if NO_MOT_DUR<5:4> =b00 
//Duration = (NO_MOT_DUR<3:0> + 4) * 5s, if NO_MOT_DUR<5:4> =b01 
//Duration = (NO_MOT_DUR<3:0> + 10) * 10s, if NO_MOT_DUR<5:4> =b1x 
//ANY_MOT_DUR<1:0>: any motion interrupt will be triggered when slope > ANY_MOT_TH for (ANY_MOT_DUR<1:0> + 1) samples 

//0x2e ANY MOTION MOT_CONF2
//TH= ANY_MOT_TH<7:0> * 16 * LSB 

#if defined(QMA7981_ANY_MOTION)
	reg_0x18 |= 0x07;
	reg_0x1a |= 0x01;
	reg_0x2c |= 0x00;	//BIT[0-1]	 (ANY_MOT_DUR<1:0> + 1) samples 
	
	qmaX981_writereg(0x18, reg_0x18);
	qmaX981_writereg(0x1a, reg_0x1a);
	qmaX981_writereg(0x2c, reg_0x2c);
	//qmaX981_writereg(0x2e, 0x14);		// 0.488*16*20 = 156mg
	//qmaX981_writereg(0x2e, 0x80);		// 0.488*16*128 = 1g
	//qmaX981_writereg(0x2e, 0xa0);		// 0.488*16*160 = 1.25g
	//qmaX981_writereg(0x2e, 0x60);		// 0.488*16*96 = 750mg
	//qmaX981_writereg(0x2e, 0x40);		// 0.488*16*64 = 500mg
	//qmaX981_writereg(0x2e, 0x20);		// 0.488*16*32 = 250mg
	qmaX981_writereg(0x2e, 0x40);		// 0.488*16*64 = 500mg

#if defined(QMA7981_ABNORMAL_SHAKE_CHECK)
	reg_0x10 = 0xe0;		// ODR: 65hz 15.48 ms
	qmaX981_writereg(0x10, reg_0x10);
	qmaX981_set_range(QMAX981_RANGE_8G);
	qmaX981_writereg(0x2e, 0x60);		// 0.977*16*96 = 1500mg
#endif
	
#if defined(QMA7981_SIGNIFICANT_MOTION)
	//SIG_MOT_TPROOF [BIT4-5]<1:0>: 00: T_PROOF=0.25s,  01: T_PROOF=0.5s,  10: T_PROOF=1s,  11: T_PROOF=2s 
	//SIG_MOT_TSKIP[BIT2-3]<1:0>: 00: T_SKIP=1.5s,  01: T_SKIP=3s,  10: T_SKIP=6s,  11: T_SKIP=12s 
	//SIG_MOT_SEL: 1: select significant motion interrupt ,  0: select any motion interrupt

	//qmaX981_writereg(0x2f, 0x0c|0x01);
	qmaX981_writereg(0x2f, 0x01);		// bit0   1 significant motion, 0: any motion.

	reg_0x19 |= 0x01;
	qmaX981_writereg(0x19, reg_0x19);
#endif
#endif
#if defined(QMA7981_NO_MOTION)
	reg_0x18 |= 0xe0;
	reg_0x1a |= 0x80;
	reg_0x2c |= 0x00;	//1s 	//0x24;

	qmaX981_writereg(0x18, reg_0x18);
	qmaX981_writereg(0x1a, reg_0x1a);
	qmaX981_writereg(0x2c, reg_0x2c);
	qmaX981_writereg(0x2d, 0x14);
#endif

#if defined(QMA7981_HAND_UP_DOWN)
	reg_0x16 |= 0x02;
	reg_0x19 |= 0x02;
			
	qmaX981_writereg(0x16, reg_0x16);
	qmaX981_writereg(0x19, reg_0x19);
	// hand down
	reg_0x16 |= 0x04;
	reg_0x19 |= 0x04;
	qmaX981_writereg(0x16, reg_0x16);
	qmaX981_writereg(0x19, reg_0x19);
	// hand down	
#if 0	// swap xy
	read_reg(0x42, &reg_0x42, 1);
	reg_0x42 |= 0x80;		// 0x42 bit 7 swap x and y
	qmaX981_writereg(0x42, reg_0x42);
#endif
#endif

#if defined(QMA7981_DATA_READY)
	reg_0x1a |= 0x10;
	qmaX981_writereg(0x17, 0x10);
	qmaX981_writereg(0x1a, reg_0x1a);
#endif

#if defined(QMA7981_INT_LATCH)
	qmaX981_writereg(0x21, 0x1f);	// default 0x1c, step latch mode
#endif

#if defined(QMA7981_DOUBLE_TRIPLE_CLICK)
	//memset(&g_click, 0, sizeof(g_click));
	g_click.check_click = 1;
	g_click.click_num = 0;
	g_click.static_num = 0;
	g_click.t_msec_1 = 200;
	g_click.t_msec_2 = 8;
	g_click.t_msec_out = 350;
#endif
#if defined(QMA7981_ABNORMAL_SHAKE_CHECK)
	g_shake.check_shake = 1;
	g_shake.shake_num = 0;
	g_shake.t_msec_1 = 200;
	g_shake.t_msec_out = 500;
#endif

   	return ret;
}

static int32_t qma6100_initialize(void)
{
	int32_t ret = 0;
	int32_t index, total;
	uint8_t data[2] = {0};

	total = sizeof(qma7981_init_tbl)/sizeof(qma7981_init_tbl[0]);
	for(index=0; index<total; index++)
	{	
		data[0] = qma7981_init_tbl[index][0];
		data[1] = qma7981_init_tbl[index][1];
		if(data[0] == 0xff)
		{
			qmaX981_delay(data[1]);
		}
		else
		{
			if(data[0] == QMAX981_REG_RANGE)
			{
				if(data[1] == QMAX981_RANGE_4G)
					g_qmaX981.lsb_1g = 2048;
				else if(data[1] == QMAX981_RANGE_8G)
					g_qmaX981.lsb_1g = 1024;
				else if(data[1] == QMAX981_RANGE_16G)
					g_qmaX981.lsb_1g = 512;
				else if(data[1] == QMAX981_RANGE_32G)
					g_qmaX981.lsb_1g = 256;
				else
					g_qmaX981.lsb_1g = 4096;
			}
			ret = qmaX981_writereg(data[0],data[1]);
			if(ret == 0)
			{
				QMAX981_ERR("qma7981_initialize ret=%d\n", ret);
				return ret;
			}
			qmaX981_delay(2);
		}
	}

#if defined(QMA7981_6100_FIFO)
	qmaX981_write_reg(0x31, 0x20);
	qmaX981_write_reg(0x3E, 0x40);
	qmaX981_write_reg(0x17, 0x20);
	qmaX981_write_reg(0x1a, 0x20);
	qmaX981_write_reg(0x20, 0x05);
#if defined(USE_SPI)
	//qmaX981_write_reg(0x21, 0x21);
#else
	//qmaX981_write_reg(0x21, 0x01);
#endif
#endif

	return ret;
}

#if defined(QMAX981_USE_IRQ1)
extern void bsp_led_set(uint8_t flag);
extern void mcu_reset_counter(void);

void qst_show_info(char flag)
{
	if(flag)
	{	
		bsp_led_set(1);
#if defined(OLED_SUPPORT)
		OLED_Clear();
		OLED_ShowString(0,1,"Warning!",16);
#endif
	}
	else
	{
		bsp_led_set(0);
#if defined(OLED_SUPPORT)
		OLED_Clear();
#endif
	}
}

#if defined(QMA7981_DOUBLE_TRIPLE_CLICK)
void click_timer_cbk_out(int timerId)
{
	bsp_stop_timer(timerId);
	bsp_stop_timer(1);
	g_click.check_click = 1;
	g_click.static_num = 0;
	g_click.click_num = 0;
			
	QMAX981_LOG("qmaX981_timer_cbk_out \n");	
}

void click_timer_read_acc(int timerId)
{
	int data1, data2, ret;

	ret = qmaX981_read_acc(acc_data_curr);
	if(ret)
	{
		data1 = QMAX981_ABS(acc_data_curr[0])+QMAX981_ABS(acc_data_curr[1])+QMAX981_ABS(acc_data_curr[2]);
		data2 = QMAX981_ABS(acc_data[0])+QMAX981_ABS(acc_data[1])+QMAX981_ABS(acc_data[2]);
		//QMAX981_LOG("acc_diff = %d \n", QMAX981_ABS(data1-data2));
		if(QMAX981_ABS(data1-data2) < 500)
		{
			g_click.static_num++;
		}
		acc_data[0] = acc_data_curr[0];
		acc_data[1] = acc_data_curr[1];
		acc_data[2] = acc_data_curr[2];
	}
}

void click_timer_cbk_1(int timerId)
{
	bsp_stop_timer(timerId);
	g_click.check_click = 1;

	if(g_click.t_msec_2 > 0)
	{
		if(g_click.click_num > 1)
		{		
			QMAX981_LOG(" static_num=%d \n", g_click.static_num);
			if(g_click.static_num <= 9)
			{
				g_click.click_num = 0;
			}
		}
		g_click.static_num = 0;
		bsp_start_timer(1, g_click.t_msec_2, click_timer_read_acc);
	}
	bsp_start_timer(0, g_click.t_msec_out, click_timer_cbk_out);	
	if(g_click.click_num == 3)
	{
		QMAX981_LOG(" click detect!!! \n");
	}
}
#endif


#if defined(QMA7981_ABNORMAL_SHAKE_CHECK)
#define SHAKE_ALARM_COUNT	8

void shake_timer_cbk_out(int timerId)
{
	bsp_stop_timer(timerId);
	g_shake.check_shake = 1;
	g_shake.shake_num = 0;
			
	QMAX981_LOG("qmaX981_timer_cbk_out \n");	
	//qst_show_info(0);
}

void shake_timer_cbk_1(int timerId)
{
	bsp_stop_timer(timerId);
	g_shake.check_shake = 1;	
	bsp_start_timer(0, g_shake.t_msec_out, shake_timer_cbk_out);
	if(g_shake.shake_num >= SHAKE_ALARM_COUNT)
	{
		QMAX981_LOG("abnormal shake, Warning!!! \n");
		qst_show_info(1);
	}
}
#endif

unsigned char qmaX981_irq_hdlr(void)
{
	unsigned char r_data[4];
	//unsigned char reg_0x18 = 0;
	unsigned char reg_0x1a = 0;
	unsigned char int_type = 0xff;

	qmaX981_readreg(0x09,r_data,3);
//	QMAX981_LOG(" [0x%x 0x%x 0x%x]    \n",r_data[0],r_data[1],r_data[2]);
	if(r_data[0] & 0xF)
	{
#if defined(QMA7981_DOUBLE_TRIPLE_CLICK)
		if(g_click.check_click)
		{
#if defined(QMA7981_NO_MOTION)
			qmaX981_readreg(0x1a,&reg_0x1a,1);
			reg_0x1a |= 0x80;			// enable nomotion
			//reg_0x1a &= 0xfe;			// disable anymotion
			qmaX981_writereg(0x1a, reg_0x1a);
#endif
			bsp_stop_timer(0);
			g_click.check_click = 0;
			g_click.static_num = 0;			
			g_click.click_num++;
			bsp_start_timer(0, g_click.t_msec_1, click_timer_cbk_1);
			QMAX981_LOG(" any motion! %d\n", g_click.click_num);
		}		
#endif

#if defined(QMA7981_ABNORMAL_SHAKE_CHECK)
		if(g_shake.check_shake)
		{
			bsp_stop_timer(0);
			g_shake.check_shake = 0;		
			g_shake.shake_num++;
			bsp_start_timer(0, g_shake.t_msec_1, shake_timer_cbk_1);
			QMAX981_LOG(" any motion! %d\n", g_shake.shake_num);
		}
#endif
		int_type = 1;
	}
	else if(r_data[0] & 0x80)
	{	
		bsp_stop_timer(0);
		qmaX981_readreg(0x1a,&reg_0x1a,1);
		reg_0x1a &= 0x7f;
		qmaX981_writereg(0x1a, reg_0x1a);		// disable nomotion
		int_type = 2;
		QMAX981_LOG(" no motion!\n");
#if defined(OLED_SUPPORT)
		OLED_Clear();
#endif
	}
	else if(r_data[1] & 0x01)
	{	
		int_type = 3;
		
		qmaX981_readreg(0x1a,&reg_0x1a,1);
		reg_0x1a |= 0x80;			// enable nomotion
		//reg_0x1a &= 0xfe;			// disable anymotion
		qmaX981_writereg(0x1a, reg_0x1a);
		QMAX981_LOG(" significant motion!\n");
	}
	else if(r_data[1] & 0x40)
	{	
		int_type = 4;
		QMAX981_LOG("  significant step int!\n");
	}
	else if(r_data[1] & 0x08)
	{
		int_type = 5;
		QMAX981_LOG(" step int!\n");
	}
#if defined(QMA7981_HAND_UP_DOWN)
	else if(r_data[1] & 0x02)
	{
		int_type = 6;
		QMAX981_LOG(" hand raise!\n");
	}
	else if(r_data[1] & 0x04)
	{
		int_type = 7;
		QMAX981_LOG(" hand down!\n");
	}
#endif
	return int_type;
}

static void qmaX981_setup_irq1(void)
{
	/* enable the key wakeup clock */
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_AF);

	/* configure button pin as input */
	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);

	/* enable and set key wakeup EXTI interrupt to the lowest priority */
	nvic_irq_enable(EXTI10_15_IRQn, 2U, 2U);

	/* connect EXTI line to GPIO pin */
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_14);

	/* configure EXTI line */
	exti_init(EXTI_14, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	exti_interrupt_flag_clear(EXTI_14);
}

void qmaX981_15_10_IRQHandler(void)         //这里为：EXTI15_10 (外部中断号的10~15都在这里实现）  
{
	uint8_t ret;
	uint8_t data[2];
#if defined(QMAX981_STEP_DEBOUNCE_IN_INT)	
	int32_t step_num;
#endif

	if(g_qmaX981.chip_type == CHIP_TYPE_QMA7981)
	{
		if(exti_interrupt_flag_get(EXTI_14) != RESET) //这里为判断相应的中断号是否进入中断，如果有多个中断的话。  
		{
			exti_interrupt_flag_clear(EXTI_14);		 //清中断
			mcu_reset_counter();
			qmaX981_irq_hdlr();
		}
	}
#if defined(QMA7981_6100_FIFO)
	else if(g_qmaX981.chip_type == CHIP_TYPE_QMA6100)
	{
		if(exti_interrupt_flag_get(EXTI_14) != RESET) //这里为判断相应的中断号是否进入中断，如果有多个中断的话。  
		{
			exti_interrupt_flag_clear(EXTI_14);		 //清中断
			qmaX981_read_fifo(1);
		}
	}
#endif
	else
	{
#if defined(QMAX981_TAP_FUNC)
		ret = qmaX981_readreg(QMAX981_INT_STAT0, data, 1);
		QMAX981_LOG("EXTI15_10_IRQHandler value_0a=%x \r\n", data[0]);
		if(exti_interrupt_flag_get(EXTI_Line11) != RESET)
		{
			exti_interrupt_flag_clear(EXTI_14);		 //清中断  
			QMAX981_LOG("EXTI_ClearITPendingBit\r\n");
		}
#endif
#if defined(QMAX981_STEP_DEBOUNCE_IN_INT)
		if(exti_interrupt_flag_get(EXTI_14) != RESET) //这里为判断相应的中断号是否进入中断，如果有多个中断的话。  
		{
			g_qmaX981.int32_t_level = GPIO_ReadInputDataBit(QMAX981_IRQ1_PORT,QMAX981_IRQ1_PIN);		
			ret = qmaX981_readreg(QMAX981_STEP_CNT_L, data, 2);
			if(ret)
			{
				step_num = (data[1]<<8)|data[0];
				QMAX981_LOG("gpio level = %d step_num=%d \r\n", g_qmaX981.int32_t_level, step_num);
				qmaX981_step_debounce_int32_t_work(step_num, g_qmaX981.int32_t_level);
			}
			exti_interrupt_flag_clear(EXTI_14);		 //清中断  
			QMAX981_LOG("EXTI_ClearITPendingBit\r\n");
		}
#endif
#if defined(QMAX981_FIFO_USE_INT)
		if(exti_interrupt_flag_get(EXTI_14) != RESET) //这里为判断相应的中断号是否进入中断，如果有多个中断的话。  
		{
			uint8_t reg_5b;
			
			ret = qmaX981_readreg(0x5b, &reg_5b, 1);
			QMAX981_LOG("reg_5b=0x%x \r\n", reg_5b);
			qmaX981_read_fifo(0);		
			exti_interrupt_flag_clear(EXTI_14);		 //清中断  
		}
#endif
	}
}

#endif


#if defined(QMAX981_USE_IRQ2)
static void qmaX981_setup_irq2(void)
{

}


void EXTI9_5_IRQHandler(void)
{
	uint8_t ret;
	uint8_t value_0a;
	
	ret = qmaX981_readreg(QMAX981_INT_STAT0, &value_0a, 1);
	QMAX981_LOG("EXTI9_5_IRQHandler value_0a=%x\r\n", value_0a);
	
	if(exti_interrupt_flag_get(EXTI_5) != RESET) //这里为判断相应的中断号是否进入中断，如果有多个中断的话。  
	{  
		  exti_interrupt_flag_clear(EXTI_5);;		 //清中断  
		  QMAX981_LOG("EXTI_ClearITPendingBit\r\n");
	}
}
#endif


int32_t qmaX981_init(void)
{
	int32_t ret = 0;

	qmaX981_delay(100);

	memset(&g_qmaX981, 0, sizeof(g_qmaX981));
	
#if !defined(USE_SPI)
#if defined(USE_SW_I2C)
	ret = i2c_CheckDevice(QMAX981_I2C_ADDR_W<<1);
	QMAX981_LOG("i2c_CheckDevice ret=%d \n", ret);
#endif
#endif
	g_qmaX981.chip_id = qmaX981_chip_id();
	if((g_qmaX981.chip_id>=0xa9) && (g_qmaX981.chip_id<=0xb9))
	{		
		QMAX981_LOG("qma6981 find \n");
		g_qmaX981.chip_type = CHIP_TYPE_QMA6981;
	}
	else if((g_qmaX981.chip_id>=0xe0) && (g_qmaX981.chip_id<=0xe7))	
	{
		QMAX981_LOG("qma7981 find \n");
		g_qmaX981.chip_type = CHIP_TYPE_QMA7981;
	}	
	else if(g_qmaX981.chip_id==0xe8)	
	{
		QMAX981_LOG("qma6100 find \n");		
		g_qmaX981.chip_type = CHIP_TYPE_QMA6100;	
	}
	else
	{		
		QMAX981_LOG("qma acc chip id not defined!!! \n");		
		g_qmaX981.chip_type = CHIP_TYPE_UNDEFINE;
	}
	// add by yangzhiqiang use another i2c addr
	if(g_qmaX981.chip_type == CHIP_TYPE_UNDEFINE)
	{
		QMAX981_LOG("qmaX981 change I2C add = 0x%x! \n", QMAX981_I2C_SLAVE_ADDR2);		
		qmaX981_delay(100);
#if !defined(USE_SPI)
		QMAX981_I2C_ADDR_W = QMAX981_I2C_SLAVE_ADDR2;
#if defined(USE_SW_I2C)
		ret = i2c_CheckDevice(QMAX981_I2C_ADDR_W<<1);
		QMAX981_LOG("i2c_CheckDevice ret=%d \n", ret);
#endif
#endif
		g_qmaX981.chip_id = qmaX981_chip_id();
		if((g_qmaX981.chip_id>=0xa9) && (g_qmaX981.chip_id<=0xb9))
		{
			QMAX981_LOG("qma6981 find \n");
			g_qmaX981.chip_type = CHIP_TYPE_QMA6981;	
		}
		else if((g_qmaX981.chip_id>=0xe0) && (g_qmaX981.chip_id<=0xe7))	
		{
			QMAX981_LOG("qma7981 find \n");		
			g_qmaX981.chip_type = CHIP_TYPE_QMA7981;	
		}		
		else if(g_qmaX981.chip_id==0xe8)	
		{
			QMAX981_LOG("qma6100 find \n"); 	
			g_qmaX981.chip_type = CHIP_TYPE_QMA6100;	
		}
		else	
		{		
			QMAX981_LOG("qma acc chip id not defined!!! \n");		
			g_qmaX981.chip_type = CHIP_TYPE_UNDEFINE;	
		}
	}
	// add by yangzhiqiang
	if(g_qmaX981.chip_type == CHIP_TYPE_QMA6981)		
		ret = qma6981_initialize();	
	else if(g_qmaX981.chip_type == CHIP_TYPE_QMA7981)
		ret = qma7981_initialize();
	else if(g_qmaX981.chip_type == CHIP_TYPE_QMA6100)
		ret = qma6100_initialize();
	else
		ret = 0;

	g_qmaX981.layout = 3;
	memcpy(&g_qmaX981.cvt, &qst_map[g_qmaX981.layout], sizeof(qst_convert));

#if defined(QMAX981_STEP_DEBOUNCE_IN_INT)
	qmaX981_step_debounce_reset();
#endif

#if defined(QMAX981_USE_IRQ1)
	qmaX981_setup_irq1();
#endif

	return ret;
}



