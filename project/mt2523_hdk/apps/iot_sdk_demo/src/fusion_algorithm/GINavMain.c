#include "GINavMain.h"
#include "GlobalVars.h"
#include "DataProc.h"
#include "InsAlign.h"
#include "InsNav.h"
#include "GIFilter.h"
#include "BasicFunc.h"
#include "SendGps.h"
#include <string.h>
#include <math.h>


void GINavInit(void)
{
	FLOAT64 sa, ca, sb, cb, sc, cc,yaw;




	MEMSET(&g_GINavInfo, 0, SIZEOF(GINAV_INFO_T));
	g_GINavInfo.INSState = INS_ACTIVE;

	g_GINavInfo.SYSFlag  = 0;


	g_GINavInfo.ImuCfg.InstallMat.C11 = g_GINavInfo.ImuCfg.InstallMat.C22 = g_GINavInfo.ImuCfg.InstallMat.C33 = 1.0;

	g_GINavInfo.Ini0_Mat.C11 = g_GINavInfo.Ini0_Mat.C22 = g_GINavInfo.Ini0_Mat.C33 = 1.0;
	g_GINavInfo.Ini1_Mat.C11 = g_GINavInfo.Ini1_Mat.C22 = g_GINavInfo.Ini1_Mat.C33 = 1.0;
	g_GINavInfo.Ini2_Mat.C11 = g_GINavInfo.Ini2_Mat.C22 = g_GINavInfo.Ini2_Mat.C33 = 1.0;
	g_GINavInfo.Ini3_Mat.C11 = g_GINavInfo.Ini3_Mat.C22 = g_GINavInfo.Ini3_Mat.C33 = 1.0;
	g_GINavInfo.Ini4_Mat.C11 = g_GINavInfo.Ini4_Mat.C22 = g_GINavInfo.Ini4_Mat.C33 = 1.0;


	g_GINavInfo.Ini0_Flag = 0;
	g_GINavInfo.Ini1_Flag = 0;
	g_GINavInfo.Ini2_Flag = 0;
	g_GINavInfo.Ini3_Flag = 0;



  g_GINavInfo.GstFlag   = 0;
  g_GINavInfo.GstStatus = 0;
	g_GINavInfo.GstDeta   = 10;
	g_GINavInfo.GstDetaMin= 100.0;



	
	g_GINavInfo.ImuCfg.InstallMatInitFlag  = 0;
  g_GINavInfo.ImuCfg.GyroBiasIsConverage = 0;	



	DataProcerInit();
	GIKFInit();
		


  
	
	
}



//-------------------------------
//Low-Speed
//-------------------------------
void GetGnssGst(PGNSS_DATA_T pGnssData)
{
	FLOAT32  GST_Deta, GST_Diff;
	STATIC FLOAT32   GST_DetaD[20];
	STATIC U8  GST_Num = 0;

	STATIC U16 StaticTimeMax = 0, ScaleMax;
	U8 i;


	GST_Deta = sqrt(pGnssData->GstDetaLon*pGnssData->GstDetaLon + pGnssData->GstDetaLat*pGnssData->GstDetaLat);

	if (g_GINavInfo.GstDetaMin <= 0)
	{
		g_GINavInfo.GstDetaMin = 5.0f;
	}


	if (GST_Deta>0.0f)
	{
		if (GST_Deta<99)
		{
			g_GINavInfo.GstDeta = GST_Deta;
		}
		else
		{
			g_GINavInfo.GstDeta = 99;
			g_GINavInfo.GstStatus = 0;
		}
	}
	else
	{
		g_GINavInfo.GstStatus = 0;
	}

	//----------------------------------------------------
	// �������ǵ���������־
	//----------------------------------------------------
	if (g_GINavInfo.GstFlag == 0)
	{
		if (g_GINavInfo.GstDeta<10.0f)
		{
			g_GINavInfo.GstFlag = 1;
			g_GINavInfo.GstStatus = 0;
			g_GINavInfo.GstDetaMin = g_GINavInfo.GstDeta;
		}
		else
		{
			g_GINavInfo.GstStatus = 0;
		}

		//GST_Diff = 0;
	}
	else
	{
		if (g_GINavInfo.GstDeta<g_GINavInfo.GstDetaMin)
		{
			g_GINavInfo.GstDetaMin = g_GINavInfo.GstDeta;
		}

		if (g_GINavInfo.GstDeta<g_GINavInfo.GstDetaMin*1.5f)
		{
			g_GINavInfo.GstStatus = 4;
		}
		else if (g_GINavInfo.GstDeta<g_GINavInfo.GstDetaMin * 4)
		{
			g_GINavInfo.GstStatus = 5;
		}
		else if (g_GINavInfo.GstDeta<g_GINavInfo.GstDetaMin * 8)
		{
			g_GINavInfo.GstStatus = 2;
		}
		else if (g_GINavInfo.GstDeta<g_GINavInfo.GstDetaMin * 20)
		{
			g_GINavInfo.GstStatus = 1;
		}
		else
		{
			g_GINavInfo.GstStatus = 0;
		}


	}

}


//---------------------------------------------
//BOOL GINavProc(POUTPUT_INFO_T pNavResult
//---------------------------------------------


BOOL GINavProc(POUTPUT_INFO_T pNavResult)
{
	S32 i, j;
	
	BOOL UpdateByGNSS=FALSE;	


	PGNSS_DATA_T   pGnssData;

	PIMU_DATA_T pImuData = GetIMUData();      //gty ��IMUDataBuffer���IMU����
	                                          //���IMU���£���pImuDataָ��IMU����
	                                          //���IMU���£���pImuDataָ��NULL
	if (!pImuData || g_GINavInfo.INSState == INS_INACTIVE)  
		return FALSE;
	
	pGnssData = GetGNSSData();                 //gty ��GNSSDataBuffer���GPS���� 
	                                           //���GNSS���£���pGnssDataָ��Gps����
	if (pGnssData) 										                       //���GNSSû�£���pGnssDataָ��Null  
		//printf("GINavProc--------pGnssData->Position.Lat:%d\n",(int)(pGnssData->Position.Lat*100));
		
//------------------------------------------------------------------
  if(pGnssData)
	{		
		g_GINavInfo.NavStatus  = pGnssData->NavStatus;
		g_GINavInfo.SatUseNum  = pGnssData->SatUseNum;

		pGnssData->Frenqucy    = 5;		
	
		GetGnssGst(pGnssData);
				
	}
	//------------------------------------------------------------------	

	
	//UTC��GPSʱ�����
	g_GINavInfo.Tag++;                        //gty �ñ���һֱ������....
	MEMCPY(&g_GINavInfo.UtcTime, &pImuData->UtcTime, sizeof(UTC_T));

  //------------------------
	//��̬���
	DynamicModeIdentify(pImuData);                         //����ԭ������ϵ�����жϣ���Ӱ���κ�Ч��

	//IMU������Ϊ��ϵ�������
	IMUCompensate(pImuData);                               //gty IMU��װ�Ǿ��󲹳����������ƫ�ò���....

	
	if (!IS_INS_ALIGNED(g_GINavInfo.INSState))      //gty��    �����̬��λ���ٶȣ���������������ɳ�ʼ��...
	{		
		if (g_GINavInfo.ImuCfg.InstallMatInitFlag == 0)
		{
			InitInstallMat(pImuData);                           //���InstallMatInitFlag=1  ��ոճ�ʼ����װ����
			
			g_GINavInfo.DynaCount++;	
		}
		else
		{
			g_GINavInfo.ImuCfg.InstallMatInitFlag <<= 1;      //gty ���InstallMatInitFlag=3������ΪIMU�������....
			g_GINavInfo.ImuCfg.InstallMatInitFlag |= 0x01;		
		}
					
		//��ʼ������Bias
		InitGyroBias(pImuData, pGnssData);

		//INS��ʼ��׼
		INSAlign(pImuData, pGnssData);
		
		if (IS_INS_ALIGNED(g_GINavInfo.INSState))    //gty ��һ������...���ʼ��...
		{//��׼��ɣ���ʼ��Kalman
			GIKFInitPMatrix();
			g_GINavInfo.INSAloneMsCount = 200000;
			g_GINavInfo.KFCount = 0;
			g_GINavInfo.GNSSHaltCount = 0;
	
		}	
	}
	else
	{
		//INS����ȷ��
		if ((g_GINavInfo.INSState & INS_HEADING_GOOD) == 0)    //gty ��ʼ����ɺ���һ���ٶ������ڣ�gps����ǳ�ʼ��INS����....
			ConfirmHeading(pGnssData);

				

		INSUpdate(pImuData, TRUE, TRUE, TRUE);                        //1 5ms


		//����PHI��
		GIKFCalcPHIMatrix(pImuData->MsrInterval*INS_UPDATE_SAMPLE_NUM); //2

		//P��Ԥ��
		GIKFPredictPMatrix(pImuData->MsrInterval*INS_UPDATE_SAMPLE_NUM);//3  1+2+3=15

		if (g_GINavInfo.StaticCount == 0)
			g_GINavInfo.INSAloneMsCount += pImuData->MsrInterval*INS_UPDATE_SAMPLE_NUM;  //�����̬����INS��������ʱ������100ms,



		if (g_GINavInfo.INSAloneMsCount>500000)
			g_GINavInfo.INSAloneMsCount = 500000;

		g_GINavInfo.GNSSHaltCount++;


		UpdateByGNSS = !GIKFUpdateByGNSS(pGnssData, pImuData);  //26ms		


		//Kalman�������
		if (UpdateByGNSS ) 
		{
			g_GINavInfo.KFCount = 0;
			g_GINavInfo.INSAloneMsCount = 0;
			g_GINavInfo.INSState = INS_ACTIVE;
		}

		

		//������������
		if (g_GINavInfo.INSAloneMsCount <= 10000 && g_GINavInfo.KFCount > 60 && (g_GINavInfo.INSState & INS_HEADING_GOOD) != 0)
			g_GINavInfo.PositionQuality = Excellent;
		else if (g_GINavInfo.INSAloneMsCount <= 100000 && g_GINavInfo.KFCount > 60 && (g_GINavInfo.INSState & INS_HEADING_GOOD) != 0)
			g_GINavInfo.PositionQuality = Good;
		else if (g_GINavInfo.INSAloneMsCount <= 800000)
			g_GINavInfo.PositionQuality = Bad;
		else
		{
			g_GINavInfo.PositionQuality = Unknow;
			//g_GINavInfo.INSState = INS_ACTIVE;
		}
	}

END:
	//������һ�����һ���������IMU���������´ν�������
	MEMCPY(g_GINavInfo.LastGyro, pImuData->Gyro[INS_UPDATE_SAMPLE_NUM - 1], sizeof(FLOAT64)* 3);
	MEMCPY(g_GINavInfo.LastAcc, pImuData->Acc[INS_UPDATE_SAMPLE_NUM - 1], sizeof(FLOAT64)* 3);
	
	MEMCPY(&pNavResult->UtcTime, &g_GINavInfo.UtcTime, sizeof(UTC_T));
	MEMCPY(&pNavResult->GpsTime, &g_GINavInfo.GPSTime, sizeof(GPST_T));
	MEMCPY(&pNavResult->Position, &g_GINavInfo.Position, sizeof(POS_T));
	MEMCPY(&pNavResult->Velocity, &g_GINavInfo.Velocity, sizeof(POS_T));
	
	for (i = 0; i < 3; i++)
	{
		pNavResult->Gyro[i] = 0.0;
		pNavResult->Acc[i] = 0.0;
		for (j = 0; j < INS_UPDATE_SAMPLE_NUM; j++)
		{
			pNavResult->Gyro[i] += pImuData->Gyro[j][i];
			pNavResult->Acc[i] += pImuData->Acc[j][i];
		}
		pNavResult->Gyro[i] /= INS_UPDATE_SAMPLE_NUM*pImuData->MsrInterval / 1000.0;
		pNavResult->Acc[i] /= INS_UPDATE_SAMPLE_NUM*pImuData->MsrInterval / 1000.0;
	}
	
	//printf("GINavProc_________pNavResult->Position.Lat:%d\n",(int)(pNavResult->Position.Lat*100));
	//printf("_________________GINAV_FINISH_____________\n");

 //----------------------------------------------------
 //	                        ���
 //----------------------------------------------------

	 memset(&IMUDataBuffer, 0, sizeof(IMU_FRAME_T));  //ȫ������
	
	
 //----------------------------------------------=------
 //	               ���⵼��ģ��5HZ
 //----------------------------------------------------	

	if(pGnssData)
	{
		memset(&GNSSDataBuffer, 0, sizeof(GNSS_DATA_T));  //ȫ������
	}

	
	Gnss_Get_Flag  = 1;           //GNSS��ñ�־����������GNSS.... 5HZ 	
		
	GpsInsGetFlag = 1;           //GNSS��ɱ�־����������ǵ�һ֡��IMU���ݣ��Ӷ�����INS�Ƶ�.....

	return TRUE;
	
}
