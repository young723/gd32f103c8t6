
#include "fis210x_algo.h"

#include <math.h>
#include <string.h>
#include "stdio.h"

#define AE_CACHE_NUMS 8
#define DQ_OFFSET_CALC_TH         0.00015f
#define DQ_OFFSET_CALC_DIFF_TH    0.00009f
#define DQ_STABLE_TH	0.00035f	
#define M_PI			3.141592654f
#define DEG2RAD			(M_PI / 180.f)
#define RAD2DEG			(180.f / M_PI)

#define QST_MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define QST_MIN(a, b)  (((a) < (b)) ? (a) : (b))

typedef struct{
	uint32_t	stable;
	uint8_t		enter_cnt;
	uint8_t 	imu_init_done;
	float		hw_dqCache[AE_CACHE_NUMS][4];
	float 		hw_dq[4];
	float	 	hwdq_offset[4];
	float 		rpy[3];
}imuprms_t;
imuprms_t g_imu;
void* p_imu = (void*)(&g_imu);
static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;	/** quaternion of sensor frame relative to auxiliary frame */
static float q0q0, q0q1, q0q2, q0q3;
static float q1q1, q1q2, q1q3;
static float q2q2, q2q3;
static float q3q3;

void fis210x_algo_init(void)
{
	imuprms_t* imu = (imuprms_t*)p_imu;
	q0 = 1.0f;
	q1 = 0.0f;
	q2 = 0.0f;
	q3 = 0.0f;
	memset(imu, 0, sizeof(imuprms_t));
	imu->imu_init_done = 1;
}
void fis210x_algo_CaliOffsetAuto(imuprms_t* imu)
{
	int i,j;
	int num_x;
	int num_y;
	int num_z;
	float max,min,dif,sumx,sumy,sumz;
	//char f2s[20];

	/* buffering */
	for(i = AE_CACHE_NUMS - 1; i >= 1; i--) 
	{
		//imu->hw_dqCache[i][0] = imu->hw_dqCache[i-1][0];
		imu->hw_dqCache[i][1] = imu->hw_dqCache[i-1][1];
		imu->hw_dqCache[i][2] = imu->hw_dqCache[i-1][2];
		imu->hw_dqCache[i][3] = imu->hw_dqCache[i-1][3];
	}
	//imu->hw_dqCache[0][0] = imu->hw_dq[0];
	imu->hw_dqCache[0][1] = imu->hw_dq[1];
	imu->hw_dqCache[0][2] = imu->hw_dq[2];
	imu->hw_dqCache[0][3] = imu->hw_dq[3];
	for(i=1; i<4; i++)
	{
		max = imu->hw_dqCache[0][i];
		min = imu->hw_dqCache[0][i];
		for(j = 1; j < AE_CACHE_NUMS; j++) 
		{
            max = QST_MAX(max, imu->hw_dqCache[j][i]);
            min = QST_MIN(min, imu->hw_dqCache[j][i]);
        }
		dif = max - min;
	#if 0
		sprintf(f2s,"%f",dif);
		printf("dif,%s\n",f2s);
	#endif
		#if 0 
		if(fabs(max) > DQ_OFFSET_CALC_TH ||
		   fabs(min) > DQ_OFFSET_CALC_TH ||
		   dif > DQ_OFFSET_CALC_DIFF_TH)
		#endif
		if(dif > DQ_OFFSET_CALC_DIFF_TH)
		{
			if(dif > DQ_STABLE_TH)
			{
				imu->stable = 0;
			}
			return;
		}
	}
	imu->stable += 1;
	sumx = sumy = sumz = 0.f;
	num_x = num_y = num_z = 0;
	for(i=0; i<AE_CACHE_NUMS; i++)
	{
		//sumw += imu->hw_dqCache[i][0];
		sumx += imu->hw_dqCache[i][1];
		sumy += imu->hw_dqCache[i][2];
		sumz += imu->hw_dqCache[i][3];
		if(imu->hw_dqCache[i][1] == 0.0f)
		{
			num_x++;
		}
		if(imu->hw_dqCache[i][2] == 0.0f)
		{
			num_y++;
		}
		if(imu->hw_dqCache[i][3] == 0.0f)
		{
			num_z++;
		}
	}
	
	//imu->hwdq_offset[0] = sumw/AE_CACHE_NUMS;
	if((AE_CACHE_NUMS - num_x) >0)
	{
		imu->hwdq_offset[1] = sumx/(AE_CACHE_NUMS - num_x);
	}
	if((AE_CACHE_NUMS - num_y) >0)
	{
		imu->hwdq_offset[2] = sumy/(AE_CACHE_NUMS - num_y);
	}
	if((AE_CACHE_NUMS - num_z) >0)
	{
		imu->hwdq_offset[3] = sumz/(AE_CACHE_NUMS - num_z);
	}
	
}

void fis210x_algo_fusion_process(float dq[4],float rpy[3])
{
	imuprms_t* imu = (imuprms_t*)p_imu;
	float dcm[9] = {1.f,0.0f,0.0f,0.0f,1.f,0.0f,0.0f,0.0f,1.f };
	float norm;
	float q_k[4];
	char f2s[20];
	if(imu->imu_init_done == 0)
	{
		fis210x_algo_init();
		printf("imu_init_done\n");
	}
	
#if 0
	printf(f2s,"%f",dq[0]);
	printf(f2s,"%f",dq[1]);
	printf(f2s,"%f",dq[2]);
	printf(f2s,"%f",dq[3]);
#endif
#if 1
	imu->hw_dq[0] = dq[0];
	imu->hw_dq[1] = dq[1];
	imu->hw_dq[2] = dq[2];
	imu->hw_dq[3] = dq[3];
#else
	imu->hw_dq[0] = dq[0];
	imu->hw_dq[1] = dq[1];
	imu->hw_dq[2] = -dq[3];
	imu->hw_dq[3] = dq[2];
#endif
	fis210x_algo_CaliOffsetAuto(imu);
	if(imu->enter_cnt < 8)
	{
		imu->enter_cnt++;
		q0 = 1.0f;
		q1 = 0.0f;
		q2 = 0.0f;
		q3 = 0.0f;
		return;
	}
	dq[0] = imu->hw_dq[0];
	dq[1] = (imu->hw_dq[1] == 0.0f) ? imu->hw_dq[1]:(imu->hw_dq[1] - imu->hwdq_offset[1]);
	dq[2] = (imu->hw_dq[2] == 0.0f) ? imu->hw_dq[2]:(imu->hw_dq[2] - imu->hwdq_offset[2]);
	dq[3] = (imu->hw_dq[3] == 0.0f) ? imu->hw_dq[3]:(imu->hw_dq[3] - imu->hwdq_offset[3]);
	q_k[0] = q0*dq[0] - q1*dq[1] - q2*dq[2] - q3*dq[3];
	q_k[1] = q1*dq[0] + q0*dq[1] - q3*dq[2] + q2*dq[3];
	q_k[2] = q2*dq[0] + q3*dq[1] + q0*dq[2] - q1*dq[3];
	q_k[3] = q3*dq[0] - q2*dq[1] + q1*dq[2] + q0*dq[3];
	q0 = q_k[0];
	q1 = q_k[1];
	q2 = q_k[2];
	q3 = q_k[3];

	norm = sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 /= norm;
	q1 /= norm;
	q2 /= norm;
	q3 /= norm;
	q0q0 = q0 * q0;
	q0q1 = q0 * q1;
	q0q2 = q0 * q2;
	q0q3 = q0 * q3;
	q1q1 = q1 * q1;
	q1q2 = q1 * q2;
	q1q3 = q1 * q3;
	q2q2 = q2 * q2;
	q2q3 = q2 * q3;
	q3q3 = q3 * q3;

#if 1
//	dcm[0] = q0q0 + q1q1 - q2q2 - q3q3;// 11
//    dcm[1] = 2.f * (q1*q2 + q0*q3);	// 12
    dcm[2] = 2.f * (q1q3 - q0q2);	// 13
    dcm[3] = 2.f * (q1q2 - q0q3);	// 21
    dcm[4] = q0q0 - q1q1 + q2q2 - q3q3;// 22
    dcm[5] = 2.f * (q2q3 + q0q1);	// 23
//    dcm[6] = 2.f * (q1*q3 + q0*q2);	// 31
//    dcm[7] = 2.f * (q2*q3 - q0*q1);	// 32
    dcm[8] = q0q0 - q1q1 - q2q2 + q3q3;// 33
  
    rpy[0] = imu->rpy[0] = atan2f(-dcm[2], dcm[8])*RAD2DEG;  		//ROLL
	rpy[1] = imu->rpy[1] = asinf(dcm[5])*RAD2DEG; 				//PITCH
	rpy[2] = imu->rpy[2] = atan2f(-dcm[3],dcm[4])*RAD2DEG;		//YAW
#else
	rpy[1] = imu->rpy[1] = (float)(asin(-2 * q1 * q3 + 2 * q0* q2)* RAD2DEG); // pitch	
	rpy[0] = imu->rpy[0] = (float)(atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* RAD2DEG); // roll	
	rpy[2] = imu->rpy[2] = (float)(atan2(2 * (q1*q2 + q0*q3), q0*q0 + q1*q1 - q2*q2 - q3*q3) * RAD2DEG);
#endif

	printf("rpy %f	%f	%f\n", rpy[0],rpy[1],rpy[2]);
}

uint32_t fis210x_algo_GetMotion(void)
{
	imuprms_t* imu = (imuprms_t*)p_imu;
	
	return imu->stable;
}

