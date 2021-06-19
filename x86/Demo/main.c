#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

#define MALLOC_SIZE ((sizeof(char) * (512 * 512)))

/*---判断手指是否按捺---*/
//一种返回值为int，参数为void的函数指针取了一个类型名为PFPIChkPressed
typedef int (*PFPIChkPressed)(void);

/*---获取指纹仪SN序号---*/
//一种返回值为int，参数为int和char*的函数指针取了一个类型名为PFPIGetDevSN
typedef int (*PFPIGetDevSN)(int nPort, char *pszDevSN);

/*---检测有没有连接指纹---*/
typedef int (*PFPIFindDevice)(int nPort, char *pszDevName);

/*---保存指纹图片---*/
//一种返回值为int，参数为两个const char*的函数指针取了一个类型名为PFPISaveImage
typedef int (*PFPISaveImage)(const char *pImgPath, const char *pImgBuf);

/*----获取指纹特征数据---*/
typedef int (*PFPIGetFeature)(int nPort, char *pszVer, char *pImgBuf, int *ImgLen);

/*----置空相关函数指针类型----*/
PFPIChkPressed FPIChkPressed = NULL;
PFPIGetDevSN FPIGetDevSN = NULL;
PFPIFindDevice FPIFindDevice = NULL;
PFPISaveImage FPISaveImage = NULL;
PFPIGetFeature FPIGetFeature = NULL;

int main(int argc, char *argv[])
//argc是命令行总的参数个数；
//argv[]是argc个参数，其中第0个参数是程序的全名，以后的参数命令行后面跟的用户输入的参数
{
	char *err = NULL;	  //char类型的err空指针
	void *pHandle = NULL; //void类型的handle空指针
	char *pImgBuf = NULL; //char类型的ImgBuf空指针
	int nRet = -1, nPort = 0, pnImgLen = 0;

	char pszVer[1024] = {'\0'};
	char pszDevSN[128] = {'\0'}, pszDevName[128] = {'\0'};

	if (2 != argc)
	{
		printf("Not Found %s\n",argv[1]);
		return 0;
	}

	/*---加载动态库---*/
	pHandle = dlopen(argv[1], RTLD_NOW);
	if (NULL == pHandle)
	{
		printf("[%s:%d] 打开%s失败[%s]\n", __FILE__, __LINE__, argv[1], dlerror());
		return -1;
	}

	/*---清空信息---*/
	dlerror();

	/*---获取【探测手指接口】指针---*/
	FPIChkPressed = (PFPIChkPressed)dlsym(pHandle, "FPIChkPressed");
	err = dlerror();
	if ((NULL == FPIChkPressed) && (NULL != err))
	{
		printf("[%s:%d] 获取函数指针FPIChkPressed指针失败[%s]\n", __FILE__, __LINE__, err);
		nRet = -2;
		goto TheEnd;
	}

	/*---获取【设备SN接口】指针---*/
	FPIGetDevSN = (PFPIGetDevSN)dlsym(pHandle, "FPIGetDevSN");
	err = dlerror();
	if ((NULL == FPIGetDevSN) && (NULL != err))
	{
		printf("[%s:%d] 获取函数指针FPIGetDevSN指针失败[%s]\n", __FILE__, __LINE__, err);
		nRet = -3;
		goto TheEnd;
	}

	/*---获取【发现设备接口】指针---*/
	FPIFindDevice = (PFPIFindDevice)dlsym(pHandle, "FPIFindDevice");
	err = dlerror();
	if ((NULL == FPIFindDevice) && (NULL != err))
	{
		printf("[%s:%d] 获取函数指针FPIFindDevice指针失败[%s]\n", __FILE__, __LINE__, err);
		nRet = -4;
		goto TheEnd;
	}

	/*---获取【保存指纹图片接口】指针---*/
	FPISaveImage = (PFPISaveImage)dlsym(pHandle, "FPISaveImage");
	err = dlerror();
	if ((NULL == FPISaveImage) && (NULL != err))
	{
		printf("[%s:%d] 获取函数指针FPISaveImage指针失败[%s]\n", __FILE__, __LINE__, err);
		nRet = -5;
		goto TheEnd;
	}

	/*---获取【采集特征接口】指针---*/
	FPIGetFeature = (PFPIGetFeature)dlsym(pHandle, "FPIGetFeature");
	err = dlerror();
	if ((NULL == FPIGetFeature) && (NULL != err))
	{
		printf("[%s:%d] 获取函数指针FPIGetFeature指针失败[%s]\n", __FILE__, __LINE__, err);
		nRet = -6;
		goto TheEnd;
	}

	/*---开始调用具体函数（请以实际业务流程调用）---*/

	/*---探测设备---*/
	nRet = FPIFindDevice(nPort, pszDevName);
	if (nRet < 0)
	{
		printf("探测指纹设备失败[%d]\n", nRet);
		goto TheEnd;
	}
	printf("指纹设备[%s]\n", pszDevName);

	/*---获取设备SN序列号---*/
	nRet = FPIGetDevSN(nPort, pszDevSN);
	if (nRet < 0)
	{
		printf("获取设备SN序列号失败[%d]\n", nRet);
		goto TheEnd;
	}
	printf("设备序列号[%s]\n", pszDevSN);

	/*---探测是否有手指按捺(请根据实际情况编写代码)---*/
	while (1)
	{
		nRet = FPIChkPressed();
		if (nRet < 0)
		{
			printf("未检测到手指...[%d]\n", nRet);
		}
		else
		{
			printf("已经检测到手指[%d]\n", nRet);
			break;
		}
	}

	/*---申请内存空间---*/
	pImgBuf = (char *)malloc(MALLOC_SIZE);
	if (NULL == pImgBuf)
	{
		printf("申请内存空间失败\n");
		nRet = -7;
		goto TheEnd;
	}
	memset(pImgBuf, '\0', MALLOC_SIZE);

	/*---采集指纹特征---*/
	nRet = FPIGetFeature(nPort, pszVer, pImgBuf, &pnImgLen);
	if (nRet < 0)
	{
		printf("采集指纹特征失败[%d]\n", nRet);
		goto TheEnd;
	}
	printf("指纹特征[%s]\n", pszVer);

	/*---保存指纹图片---*/
	nRet = FPISaveImage("fingerprint.bmp", pImgBuf);
	if (nRet < 0)
	{
		printf("保存指纹图片失败[%d]\n", nRet);
		goto TheEnd;
	}
	printf("保存指纹图片成功\n");

TheEnd:

	/*---释放内存空间---*/
	if (NULL != pImgBuf)
	{
		free(pImgBuf);
		pImgBuf = NULL;
	}

	/*---卸载动态库---*/
	if (NULL != pHandle)
	{
		dlclose(pHandle);
		pHandle = NULL;
	}

	return nRet;
}
