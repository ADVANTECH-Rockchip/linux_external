/*
 *  Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 *  Author: chad.ma <chad.ma@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "update_recv.h"

DWORD m_fwOffset;
FILE* pImgFile;
long long  m_fileSize = 0;
bool bCheck = false;
char mnt_point[256];

long long GetFwSize(char* fwFilePath)
{
	struct stat statBuf;
	char szName[256];
	memset(szName, 0, sizeof(szName));
	strcpy(szName,fwFilePath);

	memset(mnt_point, 0, sizeof(mnt_point));

	if (access(szName, F_OK) == 0) {
		if (stat(szName, &statBuf) < 0){
			printf("%s : stat fail, try \n", szName);
			return -1;
		}

		strcpy(mnt_point, szName);
		m_fileSize = statBuf.st_size;
		return m_fileSize;
	} 

	return m_fileSize;
}

bool GetData(long long dwOffset,DWORD dwSize,PBYTE lpBuffer)
{
	if ( dwOffset<0 || dwSize==0 )
		return false;

	if ( dwOffset + dwSize > m_fileSize)
		return false;

	//lseek64(pImgFile,dwOffset,SEEK_SET);
	fseek(pImgFile, dwOffset, SEEK_SET);
	UINT uiActualRead;
	uiActualRead = fread(lpBuffer, 1, dwSize, pImgFile);
	//uiActualRead = read(pImgFile, lpBuffer, dwSize);
	if (dwSize != uiActualRead)
		return false;

	return true;
}

DWORD GetFwOffset(FILE* pImageFile)
{
	int ret;
	long long ulFwSize;
	STRUCT_RKIMAGE_HEAD imageHead;
	fseeko(pImageFile, 0, SEEK_SET);
	ret = fread((PBYTE)(&imageHead),1,sizeof(STRUCT_RKIMAGE_HEAD),pImageFile);
	//ret = read(pImageFile, (PBYTE)(&imageHead), sizeof(STRUCT_RKIMAGE_HEAD));
	if (ret != sizeof(STRUCT_RKIMAGE_HEAD)){
		printf("%s<%d> Read update.img failed!\n", __func__, __LINE__);
		fclose(pImageFile);
		return -1;
	}

	if ( imageHead.uiTag!=0x57464B52 ) {
		bCheck = false;
		return -1;
	}

	return imageHead.dwFWOffset;
}

int CheckFwExit(char* imgPath, char* fwName)
{
	bool bRet;
	long long fwSize = 0;
	long long dwFwOffset;
	STRUCT_RKIMAGE_HDR rkImageHead;
	int idx,iHeadSize;
	FILE* pRecvNode = NULL;
	int iRet = -1;

	fwSize = GetFwSize(imgPath);
	if (fwSize < 0) {
		printf("GetFwSize %s Error\n", imgPath);
		return -1;
	}

	if (mnt_point[0] == 0) {
		printf("### Error : Not find update.img ### \n");
		return -1;
	}

	pImgFile = fopen(mnt_point, "rb");
	if (pImgFile == NULL)
	{
		printf("%s<%d> Open %s failed! Error:%s\n", __func__, __LINE__,
			   mnt_point, strerror(errno));
		return -1;
	}

	m_fwOffset = GetFwOffset(pImgFile);
	if (bCheck == false && m_fwOffset < 0) {
		printf("GetFwOffset %s Error\n", imgPath);
		goto ERR;
	}
	printf("m_fwOffset = 0x%08x \n", m_fwOffset);

	dwFwOffset = m_fwOffset;
	iHeadSize = sizeof(STRUCT_RKIMAGE_HDR);
	bRet = GetData(dwFwOffset, iHeadSize, (PBYTE)&rkImageHead);
	if ( !bRet )
	{
		printf("### GetData error ###\n");
		goto ERR;
	}

	if (rkImageHead.item_count <= 0)
	{
		printf("### ERROR:DownloadImage-->No Found item ###\n");
		goto ERR;
	}

	pRecvNode = fopen(DEV_RECOVERY_NODE, "wb");
	if (pRecvNode  == NULL)
	{
		printf("%s<%d> Open %s failed! Error:%s\n", __func__, __LINE__,
			   DEV_RECOVERY_NODE, strerror(errno));
		goto ERR;
	}

	//lseek(pRecvNode, 0, SEEK_SET);
	fseek(pRecvNode, 0, SEEK_SET);

	for (idx = 0; idx < rkImageHead.item_count; idx++ )
	{
		if (strcmp(rkImageHead.item[idx].name, fwName) != 0)
		{
			continue;
		}
		else
		{
			iRet = 1;
			break;
		}
	}

	if (pRecvNode != NULL) {
		fclose(pRecvNode);
		pRecvNode = NULL;
	}
	if (pImgFile != NULL) {
		fclose(pImgFile);
		pImgFile = NULL;
	}

	return iRet;
	
ERR:
	if (pRecvNode != NULL) {
		fclose(pRecvNode);
		pRecvNode = NULL;
	}
	if (pImgFile != NULL) {
		fclose(pImgFile);
		pImgFile = NULL;
	}

	return -1;
}

