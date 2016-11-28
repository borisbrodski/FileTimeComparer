/*
FileTimeComparer - FarManager plugin to compare times of two files.
Copyright 2016 Me

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdio.h>
#include <initguid.h>
#include <plugin.hpp>
#include "filetimecomparer.h"
#include "guid.h"
#include "version.h"
#include "filetimecomparerlng.h"

#define _export __declspec(dllexport)

struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;

void WINAPI _export GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI _export SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info=*Info;
	::FSF=*Info->FSF;
	::Info.FSF=&::FSF;
}

void WINAPI _export GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=0;

	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MPluginsCmdString);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);

	static const wchar_t *PluginCfgStrings[1];
	PluginCfgStrings[0]=GetMsg(MPluginsCfgMenuString);
	Info->PluginConfig.Guids=&MenuGuid;
	Info->PluginConfig.Strings=PluginCfgStrings;
	Info->PluginConfig.Count=ARRAYSIZE(PluginCfgStrings);
}

void dlgError(const wchar_t * msg) {
	const wchar_t *Msg[2];

	Msg[0]=L"Error";
	Msg[1]=msg;

	Info.Message(&MainGuid,&DlgGuid,
			FMSG_WARNING | FMSG_MB_OK,
			NULL,
			Msg,
			sizeof(Msg)/sizeof(Msg[0]),
			0);
}

PluginPanelItem * GetSelectedPanelItem(int index) {
	size_t Size=Info.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,index,0);
	PluginPanelItem *PPI=(PluginPanelItem*)malloc(Size);
	if (PPI)
	{
		FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem),Size,PPI};
		Info.PanelControl(PANEL_ACTIVE,FCTL_GETSELECTEDPANELITEM,index,&FGPPI);
		return PPI;
	}
	return nullptr;
}

void dlgSelectTwoFiles() {
	const wchar_t *Msg[2];

	Msg[0]=GetMsg(MMsgSelectTwoFilesTitle);
	Msg[1]=GetMsg(MMsgSelectTwoFilesBody);

	Info.Message(&MainGuid,&DlgGuid,
			FMSG_WARNING | FMSG_MB_OK,
			NULL,
			Msg,
			sizeof(Msg)/sizeof(Msg[0]),
			0);
}

void formatMessage(wchar_t * buffer, size_t count, const wchar_t * caption, FILETIME t1, FILETIME t2) {
	DWORD64 time1 = *(DWORD64 *)&(t1);
	DWORD64 time2 = *(DWORD64 *)&(t2);

	if (time1 == 0 || time2 == 0) {
		_swprintf_c(buffer, count, L"%s: %s", caption, GetMsg(MNotAvailable));
		return;
	}
	if (time1 == time2) {
		_swprintf_c(buffer, count, L"%s: %s", caption, GetMsg(MSame));
		return;
	}

	DWORD64 diff;
        if (time1 > time2) {
		diff = time1 - time2;
	} else {
		diff = time2 - time1;
	}


	SYSTEMTIME SystemTime;
	WORD unit1;
	const wchar_t * unit1name;
	WORD unit2;
	const wchar_t * unit2name;
	FileTimeToSystemTime((FILETIME*)&diff, &SystemTime);
	SystemTime.wYear -= 1601;
	SystemTime.wMonth -= 1;
	SystemTime.wDay -= 1;

	if (SystemTime.wYear > 0) {
		unit1 = SystemTime.wYear;
		unit1name = (unit1 == 1) ? GetMsg(MUnitYear) : GetMsg(MUnitYears);
		unit2 = SystemTime.wMonth;
		unit2name = (unit2 == 1) ? GetMsg(MUnitMonth) : GetMsg(MUnitMonths);
	} else if (SystemTime.wMonth > 0) {
		unit1 = SystemTime.wMonth;
		unit1name = (unit1 == 1) ? GetMsg(MUnitMonth) : GetMsg(MUnitMonths);
		unit2 = SystemTime.wDay;
		unit2name = (unit2 == 1) ? GetMsg(MUnitDay) : GetMsg(MUnitDays);
	} else if (SystemTime.wDay > 0) {
		unit1 = SystemTime.wDay;
		unit1name = (unit1 == 1) ? GetMsg(MUnitDay) : GetMsg(MUnitDays);
		unit2 = SystemTime.wHour;
		unit2name = (unit2 == 1) ? GetMsg(MUnitHour) : GetMsg(MUnitHours);
	} else if (SystemTime.wHour > 0) {
		unit1 = SystemTime.wHour;
		unit1name = (unit1 == 1) ? GetMsg(MUnitHour) : GetMsg(MUnitHours);
		unit2 = SystemTime.wMinute;
		unit2name = (unit2 == 1) ? GetMsg(MUnitMinute) : GetMsg(MUnitMinutes);
	} else if (SystemTime.wMinute > 0) {
		unit1 = SystemTime.wMinute;
		unit1name = (unit1 == 1) ? GetMsg(MUnitMinute) : GetMsg(MUnitMinutes);
		unit2 = SystemTime.wSecond;
		unit2name = (unit2 == 1) ? GetMsg(MUnitSecond) : GetMsg(MUnitSeconds);
	} else {
		unit1 = SystemTime.wSecond;
		unit1name = (unit1 == 1) ? GetMsg(MUnitSecond) : GetMsg(MUnitSeconds);
		unit2 = SystemTime.wMilliseconds;
		unit2name = (unit2 == 1) ? GetMsg(MUnitMillisecond) : GetMsg(MUnitMilliseconds);
	}
	_swprintf_c(buffer, count, L"%s: %u %s %u %s", caption, unit1, unit1name, unit2, unit2name);
}

HANDLE WINAPI _export OpenW(const struct OpenInfo *OInfo)
{
	if(OInfo->OpenFrom == OPEN_PLUGINSMENU) {
		PanelInfo PInfo;
		PInfo.StructSize = sizeof(PanelInfo);

		if (!Info.PanelControl(PANEL_ACTIVE, FCTL_CHECKPANELSEXIST, 0, 0)) {
			dlgError(L"No panel found");
			return nullptr;
		}

		if (!Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &PInfo)) {
			dlgError(L"Error getting panel info");
			return nullptr;
		}

		if (PInfo.SelectedItemsNumber != 2) {
			dlgSelectTwoFiles();
			return nullptr;
		}


		PluginPanelItem * file1 = GetSelectedPanelItem(0);
		PluginPanelItem * file2 = GetSelectedPanelItem(1);

		if (!file1 || !file2) {
			dlgError(L"Error getting selected file names");
			return nullptr;
		}
		wchar_t message1[1024];
		wchar_t message2[1024];
		wchar_t message3[1024];
		wchar_t message4[1024];

		formatMessage(message1, ARRAYSIZE(message1), L"Last write time ", file1->LastWriteTime,  file2->LastWriteTime);
		formatMessage(message3, ARRAYSIZE(message3), L"Creating time   ", file1->CreationTime,   file2->CreationTime);
		formatMessage(message2, ARRAYSIZE(message2), L"Last access time", file1->LastAccessTime, file2->LastAccessTime);
		formatMessage(message4, ARRAYSIZE(message4), L"Change time     ", file1->ChangeTime,     file2->ChangeTime);

		if (file1) {
			free(file1);
		}
		if (file2) {
			free(file2);
		}

		const wchar_t *Msg[5];
		Msg[0]=GetMsg(MMsgTitle);
		Msg[1]=message1;
		Msg[2]=message2;
		Msg[3]=message3;
		Msg[4]=message4;

		Info.Message(&MainGuid,&DlgGuid,
				FMSG_MB_OK | FMSG_LEFTALIGN,
				NULL,
				Msg,
				sizeof(Msg)/sizeof(Msg[0]),
				0);
	}

	return nullptr;
}

const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

// vim: set ts=4 sts=4 sw=4 noexpandtab:
