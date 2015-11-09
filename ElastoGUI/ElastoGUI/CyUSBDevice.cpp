#include "StdAfx.h"
#include "CyUSBDevice.h"

const char * FXFirmwareFilePath = "Fx3Firmware.img";//固件映像文件

CUSB30Device * CUSB30Device::s_ptrCUSB30Device = NULL;

CUSB30Device::CUSB30Device(void)
{
	m_pUSBDevice = new CCyUSBDevice();
	//CCyFX3Device *pFX3Device = new CCyFX3Device();
	//m_pUSBDevice = (CCyUSBDevice *)pFX3Device;
	//m_pUSBDevice = (CCyUSBDevice *)(new CCyFX3Device());

}


CUSB30Device::~CUSB30Device(void)
{
	if (m_pUSBDevice)
	{
		delete m_pUSBDevice;
	}
}

CUSB30Device * CUSB30Device::Instance()
{
	if (s_ptrCUSB30Device == NULL)
	{
		s_ptrCUSB30Device = new CUSB30Device();
	}
	return s_ptrCUSB30Device;
}

void CUSB30Device::Init(void *pParam /* = NULL */)
{
	//InitFX3Device();
}

void CUSB30Device::Release()
{

}

bool CUSB30Device::InitFX3Device(CString &info)
{
	return Refresh(info);// 刷新FX3设备
}


//固件下载函数
int  CUSB30Device::DownloadImpl(LPCTSTR pszFwFile, FX3_FWDWNLOAD_MEDIA_TYPE nFmt)
{
	CCyFX3Device * pFX3Dev=(CCyFX3Device*)m_pUSBDevice;

	//CCyFX3Device *pFX3Dev = new CCyFX3Device();
	int ret = FRC_SUCCESS;

	//检查下载器是否正在运行并开始启动现在进程
	if (pFX3Dev->IsBootLoaderRunning())
	{
		if (SUCCESS == pFX3Dev->DownloadFw((char*)pszFwFile,nFmt))
		{
			ret = FRC_SUCCESS;
		}
		else
		{
			ret = FRC_FAILED;
		}
	}
	else
	{
		ret = FRC_BL_UNRUNNMING;
	}

	//exit:
	//delete pFX3Dev;
	return ret;
}

////////////////////////////////////////////////
//设备进行固件下载。
//nFmt为下载类型。
//
int  CUSB30Device::DownloadFirmware(LPCTSTR pszFwFile, FX3_FWDWNLOAD_MEDIA_TYPE nFmt)
{
	if (nFmt==RAM)
	{
		return DownloadImpl(pszFwFile, nFmt);
	}
	else//I2CEEPROM、SPIFLASH
	{
		if (FRC_SUCCESS == DownloadImpl(_T("CyBootProgrammer.IMG"), RAM))
		{
			return DownloadImpl(pszFwFile, nFmt);
		}
		else
		{
			return FRC_PROGER_FAILED;
		}
	}
}

int  CUSB30Device::InitFirmware()
{
	int ret = FRC_FAILED;

	{   // 如果被识别为2.0设备,需要重新下载固件
		if (m_pUSBDevice->Open(0))
		{
			ret = DownloadFirmware(FXFirmwareFilePath, RAM);
			//if (ret == FRC_SUCCESS)
			{
				::Sleep(200);
				//Refresh();
			}
		}
	}
	return ret;
}


BOOL CUSB30Device::Open(int ndx)
{
	m_oSemAccessCyCtrl.Lock();
	bool ok = m_pUSBDevice->Open(ndx);
	m_oSemAccessCyCtrl.Unlock();
	return ok;
}

void CUSB30Device::SetBulkEndPtTimeOut(int timeout)
{
	m_oSemAccessCyCtrl.Lock();
	m_pUSBDevice->BulkInEndPt->TimeOut = timeout;
	m_oSemAccessCyCtrl.Unlock();
}

BOOL CUSB30Device::InitBulkEndPoint(BYTE chInit)
{
	if (!m_pUSBDevice->IsOpen())  return FALSE;

	CCyUSBDevice * pUSBDev = m_pUSBDevice;

	CCyControlEndPoint * pCtrlEp=pUSBDev->ControlEndPt;
	pCtrlEp->Target   = TGT_INTFC;
	pCtrlEp->ReqType  = REQ_CLASS;	
	pCtrlEp->Value    = 0x0000;
	pCtrlEp->Index    = 0x0000;
	pCtrlEp->Direction= DIR_TO_DEVICE;
	pCtrlEp->ReqCode  = 0x01;

	BYTE chBuf = chInit;
	long nBufLen = 1;

	OVERLAPPED oCtrlOvLap;
	oCtrlOvLap.hEvent = CreateEvent(NULL, FALSE, FALSE, _T("CYUSB_INIT"));

	BYTE * pContext=pCtrlEp->BeginDataXfer(&chBuf, nBufLen, &oCtrlOvLap);
	if (pCtrlEp->WaitForXfer(&oCtrlOvLap, 1500))
	{
		if(pCtrlEp->FinishDataXfer(&chBuf, nBufLen, &oCtrlOvLap, pContext))
		{
			//退出时关闭事件内核对象
			CloseHandle(oCtrlOvLap.hEvent);
			return TRUE;
		}
	}

	//退出时关闭事件内核对象
	CloseHandle(oCtrlOvLap.hEvent);

	return FALSE;
}


int  CUSB30Device::CtrlEndPointWrite(BYTE bData)
{
	m_oSemAccessCyCtrl.Lock();

	int error = -1;
	if (m_pUSBDevice->IsOpen())
	{
		CCyUSBDevice * pUSBDev = m_pUSBDevice;
		CCyControlEndPoint * pCtrlEp=pUSBDev->ControlEndPt;
		pCtrlEp->Target   = TGT_INTFC;
		pCtrlEp->ReqType  = REQ_CLASS;	
		pCtrlEp->Value    = 0x0000;
		pCtrlEp->Index    = 0x0000;
		//pCtrlEp->Direction= DIR_TO_DEVICE;
		pCtrlEp->ReqCode  = 0x01;

		BYTE chBuf = bData;
		LONG nBufLen = 1;

		bool ok = pCtrlEp->Write(&chBuf, nBufLen);

		error = ok ? 1 : 0;
	}

	m_oSemAccessCyCtrl.Unlock();
	return error;
}

int  CUSB30Device::CtrlEndPointRead(BYTE &bData)
{
	m_oSemAccessCyCtrl.Lock();
	int error = -1;
	if (m_pUSBDevice->IsOpen())
	{
		CCyUSBDevice * pUSBDev = m_pUSBDevice;
		CCyControlEndPoint * pCtrlEp=pUSBDev->ControlEndPt;
		pCtrlEp->Target   = TGT_INTFC;
		pCtrlEp->ReqType  = REQ_CLASS;	
		pCtrlEp->Value    = 0x0000;
		pCtrlEp->Index    = 0x0000;
		//pCtrlEp->Direction= DIR_TO_DEVICE;
		pCtrlEp->ReqCode  = 0x81;

		BYTE chBuf = 0;
		LONG nBufLen = 1;

		bool ok = pCtrlEp->Read(&chBuf, nBufLen);

		if (ok)
		{
			bData = chBuf;
		}
		error = ok ? 1 : 0;
	}

	m_oSemAccessCyCtrl.Unlock();
	return error;
}

bool  CUSB30Device::Refresh(CString &info)
{
	CCyUSBDevice * pUSBDevice = m_pUSBDevice;
	bool  ok = false;

	m_oSemAccessCyCtrl.Lock();

	//确保存在至少一个USB器件
	if (pUSBDevice->DeviceCount())
	{
		/*
		//搜寻所有已连接的器件并将它们添加到器件列表。
		for (int i = 0; i < pUSBDevice->DeviceCount(); i++)
		{
			pUSBDevice->Open(i);
		}		
		pUSBDevice->Close();
		::Sleep(100);
		*/
		//pUSBDevice->Open(0);
		if (IsUSB20())
		{
			InitFirmware();
		}
		OnCbOpenDev(info);
		ok = true;
	}
	else
	{
		//MessageBox(_T("未检测到USB设备！\r\n\r\n有可能驱动还没连接完毕，请多尝试几次！"),_T("消息"), MB_OK | MB_ICONINFORMATION);
		//WriteInfoTxt(_T("未检测到USB设备！ 有可能驱动还没连接完毕，请多尝试几次！"), INFO_WARNING);
		info = _T("未检测到USB设备！ 有可能驱动还没连接完毕，请多尝试几次！");
	}

	m_oSemAccessCyCtrl.Unlock();
	return ok;
}


void CUSB30Device::OnCbOpenDev(CString &info)
{
	CCyUSBDevice *pUSBDevice = m_pUSBDevice;

	CString csBuf;
	pUSBDevice->Open(0);
	info = pUSBDevice->FriendlyName;
	csBuf.Format(_T("; 设备PID：0x%04X; "), pUSBDevice->ProductID);
	info += csBuf;

	csBuf.Empty();
	csBuf.Format(_T("设备VID：0x%04X; "), pUSBDevice->VendorID);

	info += csBuf;
	csBuf.Empty();

	if (pUSBDevice->bSuperSpeed)  
	{
		csBuf = _T("设备类型：USB3.0超速设备");
	}
	else if (pUSBDevice->bHighSpeed) 
	{
		csBuf = _T("设备类型：USB2.0高速设备");
	}
	else 
	{
		csBuf = _T("设备类型：USB1.1全速设备");
	}
	info += csBuf;
	//WriteInfoTxt(info);

	//pUSBDevice->Close();
}


bool  CUSB30Device::IsAvailale()
{
	m_oSemAccessCyCtrl.Lock();

	bool ok = m_pUSBDevice->DeviceCount() && IsUSB30();

	m_oSemAccessCyCtrl.Unlock();
	return ok;
}

bool  CUSB30Device::IsUSB30()
{
	return m_pUSBDevice->bSuperSpeed;
}

bool  CUSB30Device::IsUSB20()
{
	return m_pUSBDevice->bHighSpeed;
}

bool  CUSB30Device::IsUSB11()
{
	return !m_pUSBDevice->bHighSpeed && !m_pUSBDevice->bSuperSpeed;
}

bool  CUSB30Device::BulkEndPointInXfer(BYTE *pData, LONG &toRead)
{
	m_oSemAccessCyCtrl.Lock();

	CCyBulkEndPoint *pEndPt = m_pUSBDevice->BulkInEndPt;
    bool ok = false;
	if (pEndPt)
	{
		ok = pEndPt->XferData(pData, toRead);
	}
	m_oSemAccessCyCtrl.Unlock();
	return ok;

}