#ifndef CUSTOMIZECOLORSDIALOG_INCLUDED
#define CUSTOMIZECOLORSDIALOG_INCLUDED

#include <vector>
#include "Explorer++_internal.h"
#include "../Helper/BaseDialog.h"
#include "../Helper/ResizableDialog.h"
#include "../Helper/DialogSettings.h"

class CCustomizeColorsDialog;

class CCustomizeColorsDialogPersistentSettings : public CDialogSettings
{
public:

	~CCustomizeColorsDialogPersistentSettings();

	static CCustomizeColorsDialogPersistentSettings &GetInstance();

private:

	friend CCustomizeColorsDialog;

	static const TCHAR SETTINGS_KEY[];

	CCustomizeColorsDialogPersistentSettings();

	CCustomizeColorsDialogPersistentSettings(const CCustomizeColorsDialogPersistentSettings &);
	CCustomizeColorsDialogPersistentSettings & operator=(const CCustomizeColorsDialogPersistentSettings &);
};

class CCustomizeColorsDialog : public CBaseDialog
{
public:

	CCustomizeColorsDialog(HINSTANCE hInstance,int iResource,HWND hParent,std::vector<ColorRule_t> *pColorRuleList);
	~CCustomizeColorsDialog();

protected:

	BOOL	OnInitDialog();
	BOOL	OnCommand(WPARAM wParam,LPARAM lParam);
	BOOL	OnNotify(NMHDR *pnmhdr);
	BOOL	OnGetMinMaxInfo(LPMINMAXINFO pmmi);
	BOOL	OnSize(int iType,int iWidth,int iHeight);
	BOOL	OnClose();
	BOOL	OnDestroy();

private:

	void	OnNew();
	void	OnEdit();
	void	InsertColorRuleIntoListView(HWND hListView,const ColorRule_t &ColorRule,int iIndex);
	void	EditColorRule(int iSelected);
	void	OnMove(BOOL bUp);
	void	OnDelete();

	void	OnOk();
	void	OnCancel();

	void	InitializeControlStates();

	void	SaveState();

	HWND	m_hGripper;
	HICON	m_hDialogIcon;
	std::vector<ColorRule_t>	*m_pColorRuleList;

	int		m_iMinWidth;
	int		m_iMinHeight;
	CResizableDialog	*m_prd;

	CCustomizeColorsDialogPersistentSettings	*m_pccdps;
};

#endif