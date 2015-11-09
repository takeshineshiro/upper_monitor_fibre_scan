#pragma once

//器件初始化，指示数的传输方向
const BYTE CYUSB_IN  = 0x00;
const BYTE CYUSB_OUT = 0xff;

const BYTE CYUSB_START = 0x02;// 器件：开始一次测量
const BYTE CYUSB_SET_DT_ENVELOPE = 0x03; // 设置数据类型：包络后
const BYTE CYUSB_SET_DT_RAW      = 0x04; // 设置数据类型:原始

enum FWDWN_RETURN_CODE
{
	FRC_SUCCESS       = 0x00, //操作成功
	FRC_FAILED        = 0x01, //操作失败
	FRC_INVALID_PARAM = 0x02, //无效的命令行参数
	FRC_UNKOWN_DEVICE = 0x03, //未知的设备
	FRC_BL_UNRUNNMING = 0x04, //BootLoader未运行
	FRC_PROGER_FAILED = 0x05, //编程器固件下载失败
	FRC_FW_UNFUND     = 0x06  //固件不存在
};

//////////////////////////////////////////////////////////////////////////
// 封装CyUSBDevice类， 采用单件类的设计
// 目标是所有公开的接口都有保护-实现线程安全
// 约束： 系统只能有一个FX3的设备， 代码默认使用0号设备
//////////////////////////////////////////////////////////////////////////
class CUSB30Device
{
public:
	~CUSB30Device(void);
    static CUSB30Device *Instance(); // 该类只有一个实例对象，使用静态函数接口引用

	void Init(void *pParam = NULL); // 提供一个外部可以输入信息的初始化接口

	void Release(); // 对象销毁前释放资源

	bool InitFX3Device(CString &info); // 初始化设备

	bool Refresh(CString &info); //重新刷新设备

	bool IsAvailale(); //设备是否可用

	bool IsUSB30(); //是否3.0设备

	bool IsUSB20();

	bool IsUSB11();

	bool BulkEndPointInXfer(BYTE *pData, LONG &toRead); // bulk端点读数据

	// 指示USB设备数据传输方向
	BOOL InitBulkEndPoint(BYTE chInit); // 设置bulk端点-读或者写

	BOOL Open(int ndx = 0); //打开设备

	void SetBulkEndPtTimeOut(int timeout); // 设置布雷克断点-操作超时

	// 向USB设备控制端点写数据，目前一次只能写一个数据
	int  CtrlEndPointWrite(BYTE bData);

	// 向USB设备控制端点读数据，目前一次只能读一个数据
	int  CtrlEndPointRead(BYTE &bData);

protected:

private:
	CUSB30Device(void);//禁止外部直接实例化

	static CUSB30Device *s_ptrCUSB30Device; // 本类只能有一个实例对象

	CSemaphore           m_oSemAccessCyCtrl;// 信号量，保护CyUSBDevice的草走
	CCyUSBDevice        *m_pUSBDevice;
	int                  m_nDevIndex;

	void OnCbOpenDev(CString &info); // 打开设备时做一些处理工作

	int  InitFirmware(); // 初始化设备的固件

	int  DownloadImpl(LPCTSTR pszFwFile, FX3_FWDWNLOAD_MEDIA_TYPE nFmt);

	int  DownloadFirmware(LPCTSTR pszFwFile, FX3_FWDWNLOAD_MEDIA_TYPE nFmt);
};

