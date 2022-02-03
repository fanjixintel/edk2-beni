/**
*  @Package     : BeniPkg
*  @FileName    : BeniDataStruct.h
*  @Date        : 20211004
*  @Author      : Jiangwei
*  @Version     : 1.0
*  @Description :
*    This driver add a page to UEFI Setup.
*
*  @History:
*    20211004: Initialize.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef __BENI_DATA_STRUCT_H__
#define __BENI_DATA_STRUCT_H__

#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/HiiFormMapMethodGuid.h>
#include <Guid/ZeroGuid.h>
#include <Guid/BeniSetupHii.h>

#define CONFIGURATION_VARSTORE_ID   0x1234

#pragma pack(1)

typedef struct {
  UINT16        MyStringData[40];
  UINT16        SomethingHiddenForHtml;
  UINT8         HowOldAreYouInYearsManual;
  UINT16        HowTallAreYouManual;
  UINT8         HowOldAreYouInYears;
  UINT16        HowTallAreYou;
  UINT8         MyFavoriteNumber;
  UINT8         TestLateCheck;
  UINT8         TestLateCheck2;
  UINT8         QuestionAboutTreeHugging;
  UINT8         ChooseToActivateNuclearWeaponry;
  UINT8         SuppressGrayOutSomething;
  UINT8         OrderedList[8];
  UINT16        BootOrder[8];
  UINT8         BootOrderLarge;
  UINT8         DynamicRefresh;
  UINT8         DynamicOneof;
  UINT8         DynamicOrderedList[5];
  UINT8         Reserved;
  EFI_HII_REF   RefData;
  UINT8         NameValueVar0;
  UINT16        NameValueVar1;
  UINT16        NameValueVar2[20];
  UINT8         SerialPortNo;
  UINT8         SerialPortStatus;
  UINT16        SerialPortIo;
  UINT8         SerialPortIrq;
  UINT8         GetDefaultValueFromCallBack;
  UINT8         GetDefaultValueFromAccess;
  EFI_HII_TIME  Time;
  UINT8         RefreshGuidCount;
  UINT8         Match2;
  UINT8         GetDefaultValueFromCallBackForOrderedList[3];
} BENI_SETUP_CONFIGURATION;

//
// Labels definition.
//
#define LABEL_UPDATE1               0x1234
#define LABEL_UPDATE2               0x2234
#define LABEL_UPDATE3               0x3234
#define LABEL_END                   0x2223

#pragma pack()

#endif // __BENI_DATA_STRUCT_H__
