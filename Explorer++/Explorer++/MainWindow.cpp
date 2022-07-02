// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "MainWindow.h"
#include "Config.h"
#include "CoreInterface.h"
#include "Explorer++_internal.h"
#include "MainResource.h"
#include "ShellBrowser/ShellBrowser.h"
#include "TabContainer.h"
#include "../Helper/Helper.h"
#include "../Helper/ProcessHelper.h"
#include <wil/resource.h>

MainWindow *MainWindow::Create(
	HWND hwnd, std::shared_ptr<Config> config, HINSTANCE instance, IExplorerplusplus *expp)
{
	return new MainWindow(hwnd, config, instance, expp);
}

MainWindow::MainWindow(
	HWND hwnd, std::shared_ptr<Config> config, HINSTANCE instance, IExplorerplusplus *expp) :
	BaseWindow(hwnd),
	m_hwnd(hwnd),
	m_config(config),
	m_instance(instance),
	m_expp(expp)
{
	m_expp->AddTabsInitializedObserver(
		[this]
		{
			m_connections.push_back(m_expp->GetTabContainer()->tabSelectedSignal.AddObserver(
				std::bind_front(&MainWindow::OnTabSelected, this)));
			m_connections.push_back(
				m_expp->GetTabContainer()->tabNavigationCommittedSignal.AddObserver(
					std::bind_front(&MainWindow::OnNavigationCommitted, this)));
		});

	m_connections.push_back(m_config->showFullTitlePath.addObserver(
		std::bind_front(&MainWindow::OnShowFullTitlePathUpdated, this)));
	m_connections.push_back(m_config->showUserNameInTitleBar.addObserver(
		std::bind_front(&MainWindow::OnShowUserNameInTitleBarUpdated, this)));
	m_connections.push_back(m_config->showPrivilegeLevelInTitleBar.addObserver(
		std::bind_front(&MainWindow::OnShowPrivilegeLevelInTitleBarUpdated, this)));
}

void MainWindow::OnNavigationCommitted(const Tab &tab, PCIDLIST_ABSOLUTE pidl, bool addHistoryEntry)
{
	UNREFERENCED_PARAMETER(pidl);
	UNREFERENCED_PARAMETER(addHistoryEntry);

	if (m_expp->GetTabContainer()->IsTabSelected(tab))
	{
		UpdateWindowText();
	}
}

void MainWindow::OnTabSelected(const Tab &tab)
{
	UNREFERENCED_PARAMETER(tab);

	UpdateWindowText();
}

void MainWindow::OnShowFullTitlePathUpdated(BOOL newValue)
{
	UNREFERENCED_PARAMETER(newValue);

	UpdateWindowText();
}

void MainWindow::OnShowUserNameInTitleBarUpdated(BOOL newValue)
{
	UNREFERENCED_PARAMETER(newValue);

	UpdateWindowText();
}

void MainWindow::OnShowPrivilegeLevelInTitleBarUpdated(BOOL newValue)
{
	UNREFERENCED_PARAMETER(newValue);

	UpdateWindowText();
}

void MainWindow::UpdateWindowText()
{
	const Tab &tab = m_expp->GetTabContainer()->GetSelectedTab();
	auto pidlDirectory = tab.GetShellBrowser()->GetDirectoryIdl();

	std::wstring folderDisplayName;

	/* Don't show full paths for virtual folders (as only the folders
	GUID will be shown). */
	if (m_config->showFullTitlePath.get() && !tab.GetShellBrowser()->InVirtualFolder())
	{
		GetDisplayName(pidlDirectory.get(), SHGDN_FORPARSING, folderDisplayName);
	}
	else
	{
		GetDisplayName(pidlDirectory.get(), SHGDN_NORMAL, folderDisplayName);
	}

	TCHAR szTitle[512];

	bool bDontShowAppName = true;

	if (bDontShowAppName)
	{
		StringCchCopy(szTitle, SIZEOF_ARRAY(szTitle), folderDisplayName.c_str());
	}
	else
	{
		TCHAR szTemp[64];
		LoadString(m_instance, IDS_MAIN_WINDOW_TITLE, szTemp, SIZEOF_ARRAY(szTemp));
		StringCchPrintf(szTitle, SIZEOF_ARRAY(szTitle), szTemp, folderDisplayName.c_str(),
			NExplorerplusplus::APP_NAME);
	}

	if (m_config->showUserNameInTitleBar.get() || m_config->showPrivilegeLevelInTitleBar.get())
	{
		StringCchCat(szTitle, SIZEOF_ARRAY(szTitle), _T(" ["));
	}

	if (m_config->showUserNameInTitleBar.get())
	{
		TCHAR szOwner[512];
		GetProcessOwner(GetCurrentProcessId(), szOwner, SIZEOF_ARRAY(szOwner));

		StringCchCat(szTitle, SIZEOF_ARRAY(szTitle), szOwner);
	}

	if (m_config->showPrivilegeLevelInTitleBar.get())
	{
		TCHAR szPrivilegeAddition[64];
		TCHAR szPrivilege[64];

		if (CheckGroupMembership(GroupType::Administrators))
		{
			LoadString(m_instance, IDS_PRIVILEGE_LEVEL_ADMINISTRATORS, szPrivilege,
				SIZEOF_ARRAY(szPrivilege));
		}
		else if (CheckGroupMembership(GroupType::PowerUsers))
		{
			LoadString(m_instance, IDS_PRIVILEGE_LEVEL_POWER_USERS, szPrivilege,
				SIZEOF_ARRAY(szPrivilege));
		}
		else if (CheckGroupMembership(GroupType::Users))
		{
			LoadString(
				m_instance, IDS_PRIVILEGE_LEVEL_USERS, szPrivilege, SIZEOF_ARRAY(szPrivilege));
		}
		else if (CheckGroupMembership(GroupType::UsersRestricted))
		{
			LoadString(m_instance, IDS_PRIVILEGE_LEVEL_USERS_RESTRICTED, szPrivilege,
				SIZEOF_ARRAY(szPrivilege));
		}

		if (m_config->showUserNameInTitleBar.get())
		{
			StringCchPrintf(
				szPrivilegeAddition, SIZEOF_ARRAY(szPrivilegeAddition), _T(" - %s"), szPrivilege);
		}
		else
		{
			StringCchPrintf(
				szPrivilegeAddition, SIZEOF_ARRAY(szPrivilegeAddition), _T("%s"), szPrivilege);
		}

		StringCchCat(szTitle, SIZEOF_ARRAY(szTitle), szPrivilegeAddition);
	}

	if (m_config->showUserNameInTitleBar.get() || m_config->showPrivilegeLevelInTitleBar.get())
	{
		StringCchCat(szTitle, SIZEOF_ARRAY(szTitle), _T("]"));
	}

	SetWindowText(m_hwnd, szTitle);
}