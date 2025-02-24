/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */



#include "stdafx.h"
#include "FRED.h"

#include "MainFrm.h"
#include "FREDDoc.h"
#include "FREDView.h"
#include "MessageEditorDlg.h"
#include "ShipClassEditorDlg.h"
#include "MissionNotesDlg.h"
#include "Grid.h"
#include "dialog1.h"

#include "species_defs/species_defs.h"
#include "iff_defs/iff_defs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_MESSAGE(WM_MENU_POPUP_EDIT, OnMenuPopupTest)
	ON_CBN_SELCHANGE(ID_NEW_SHIP_TYPE, OnNewShipTypeChange)

	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_MISSIONNOTES, OnFileMissionnotes)
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_STATUS_BAR, OnViewStatusBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateViewStatusBar)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_LEFT, OnUpdateLeft)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RIGHT, OnUpdateRight)
	ON_COMMAND(ID_MIKE_GRIDCONTROL, OnMikeGridcontrol)
	ON_COMMAND(IDR_MENU_POPUP_TOGGLE1, OnMenuPopupToggle1)
	ON_UPDATE_COMMAND_UI(IDR_MENU_POPUP_TOGGLE1, OnUpdateMenuPopupToggle1)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_HELP_INPUT_INTERFACE, OnHelpInputInterface)
	ON_WM_CLOSE()
	ON_WM_INITMENU()
	ON_COMMAND(ID_HELP_FINDER, OnFredHelp)
	ON_COMMAND(ID_HELP, OnFredHelp)
	//ON_COMMAND(ID_CONTEXT_HELP, OnFredHelp)
	//ON_COMMAND(ID_DEFAULT_HELP, OnFredHelp)
	//}}AFX_MSG_MAP
	// Global help commands
END_MESSAGE_MAP()

#define FRED_HELP_URL "\\data\\freddocs\\index.html"

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_INDICATOR_MODIFIED,
	ID_SEPARATOR,
	//	ID_INDICATOR_LEFT,
	//	ID_INDICATOR_RIGHT,
	//	ID_INDICATOR_CAPS,
	//	ID_INDICATOR_NUM,
	//	ID_INDICATOR_SCRL,
};

CMainFrame *Fred_main_wnd;
color_combo_box m_new_ship_type_combo_box;
size_t ship_type_combo_box_size = 0;
int Toggle1_var = 0;
CPoint Global_point2;

/**
 * @brief Launches the default browser to open the given URL
 */
void url_launch(const char *url);


#ifdef _DEBUG
void CMainFrame::AssertValid() const {
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const {
	CFrameWnd::Dump(dc);
}
#endif // _DEBUG

CMainFrame::CMainFrame() {}

CMainFrame::~CMainFrame() {}

void CMainFrame::init_tools() {
	//int highest_terran_index;
	//char ship_name[256];
	//int ship_index;

	// some bizarre Volition check:
	static int count = 0;
	count++;
	if (count == 1) {
		return;
	} else if (count >= 3) {
		Warning(LOCATION, "CMainFrame::init_tools was called more than twice!  Trace out and fix.");
		return;
	}

    for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
        // don't add the pirate ship
        if (it->flags[Ship::Info_Flags::No_fred]) {
            //m_new_ship_type_combo_box.AddString("");
            continue;
        } else {
            m_new_ship_type_combo_box.AddString(it->name);
            m_new_ship_type_combo_box.SetItemData((int)ship_type_combo_box_size, std::distance(Ship_info.cbegin(), it));
            ship_type_combo_box_size++;
        }
    }

	Id_select_type_waypoint = (int)(ship_type_combo_box_size);
	Id_select_type_jump_node = (int)(ship_type_combo_box_size + 1);
	Id_select_type_start = (int)(ship_type_combo_box_size + 2);

	m_new_ship_type_combo_box.AddString("Waypoint");
	m_new_ship_type_combo_box.SetItemData(Id_select_type_waypoint, Ship_info.size());
	m_new_ship_type_combo_box.AddString("Jump Node");
	m_new_ship_type_combo_box.SetItemData(Id_select_type_jump_node, Ship_info.size() + 1);
	//	m_new_ship_type_combo_box.AddString("Player Start");
	//	m_new_ship_type_combo_box.SetItemData(Id_select_type_start, Ship_info.size() + 2);

	/*
	// now we want to sort special ships (mission disk) ----------------------
	highest_terran_index = 0;
	memset(ship_name, 0, 256);
	while(m_new_ship_type_combo_box.GetLBText(highest_terran_index, ship_name) != CB_ERR){
	ship_index = ship_info_lookup(ship_name);
	if((ship_index < 0) || (ship_index >= Num_ship_classes) || (Ship_info[ship_index].species != 0)){
	break;
	}
	highest_terran_index++;
	}
	*/
	m_new_ship_type_combo_box.SetCurSel(0);
}

void CMainFrame::OnClose()
{
	// CFrameWnd::OnClose() doesn't provide a way for the caller to tell that the close has been cancelled,
	// so let's extract that particular logic and do it manually here
	if (!FREDDoc_ptr->SaveModified())
		return;

	// and now if we *are* closing, prevent the dialog from coming up
	FREDDoc_ptr->SetModifiedFlag(FALSE);

	// do the closing stuff
	theApp.write_ini_file();
	SaveBarState("Tools state");
	CFrameWnd::OnClose();
	gr_close();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	int z;
	CRect rect;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME)) {
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// Create the combo box
	z = m_wndToolBar.CommandToIndex(ID_NEW_SHIP_TYPE);
	Assert(z != -1);
	m_wndToolBar.SetButtonInfo(z, ID_NEW_SHIP_TYPE, TBBS_SEPARATOR, 230);

	// Design guide advises 12 pixel gap between combos and buttons
	//	m_wndToolBar.SetButtonInfo(1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
	m_wndToolBar.GetItemRect(z, &rect);
	rect.top = 3;
	rect.bottom = rect.top + 550;
	if (!m_new_ship_type_combo_box.Create(CBS_DROPDOWNLIST | WS_VISIBLE | WS_VSCROLL | CBS_HASSTRINGS | LBS_OWNERDRAWFIXED,
		rect, &m_wndToolBar, ID_NEW_SHIP_TYPE)) {
		TRACE0("Failed to create new ship type combo-box\n");
		return FALSE;
	}

	/*	if (!m_wndStatusBar.Create(this) ||
			!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators)/sizeof(UINT)))
			{
			TRACE0("Failed to create status bar\n");
			return -1;      // fail to create
			}
			*/

	/*	if (!m_wndStatusBar.Create(this,
			WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, ID_MY_STATUS_BAR) ||
			!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))*/

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT))) {
		TRACE0("Failed to create status bar\n");
		return -1;

	} else {
		m_wndStatusBar.SetPaneInfo(0, 0, SBPS_STRETCH, 0);
		m_wndStatusBar.SetPaneInfo(1, 0, SBPS_NORMAL, 80);
		m_wndStatusBar.SetPaneInfo(2, 0, SBPS_NORMAL, 180);
		//		m_wndStatusBar.SetPaneInfo(3, 0, SBPS_NORMAL, 100);
		m_wndStatusBar.SetPaneInfo(4, 0, SBPS_NORMAL, 130);
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
							 CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	Fred_main_wnd = this;
	Ship_editor_dialog.Create();
	Wing_editor_dialog.Create();
	Waypoint_editor_dialog.Create();
	init_tools();
	LoadBarState("Tools state");
	return 0;
}

void CMainFrame::OnDestroy() {
	Fred_main_wnd = NULL;
	CFrameWnd::OnDestroy();
}

void CMainFrame::OnFileMissionnotes() {
	CMissionNotesDlg	dlg;

	dlg.DoModal();
}

void CMainFrame::OnFredHelp() {
	auto res = cf_find_file_location("index.html", CF_TYPE_FREDDOCS);
	if (!res.found) {
		ReleaseWarning(LOCATION, "Could not find FRED help files!");
		return;
	}

	if (res.offset != 0) {
		// We need an actual file location so VP files are not valid
		Error(LOCATION, "The FRED documentation was found in a pack (VP) file. This is not valid!");
		return;
	}

	// shell_open url
	url_launch(res.full_name.c_str());
}

void CMainFrame::OnHelpInputInterface() {
	dialog1	dlg;

	dlg.DoModal();
}

void CMainFrame::OnInitMenu(CMenu* pMenu) {
	int i;
	CString str;
	extern SCP_vector<int> ID_SHOW_IFF;

	if (Undo_available && !FREDDoc_ptr->undo_desc[1].IsEmpty())
		str = "Undo " + FREDDoc_ptr->undo_desc[1] + "\tCtrl+Z";
	else
		str = "Undo\tCtrl+Z";

	if (pMenu->GetMenuState(ID_EDIT_UNDO, MF_BYCOMMAND) != -1)
		pMenu->ModifyMenu(ID_EDIT_UNDO, MF_BYCOMMAND, ID_EDIT_UNDO, str);

	// Goober5000 - do the IFF menu options
	for (i = 0; i < (int)ID_SHOW_IFF.size(); i++) {
		//This should eventually be properly dynamic as well
		if (i < (int)Iff_info.size()) {
			char text[NAME_LENGTH + 7];
			sprintf(text, "Show %s", Iff_info[i].iff_name);

			pMenu->ModifyMenu(ID_SHOW_IFF[i], MF_BYCOMMAND | MF_STRING, ID_SHOW_IFF[i], text);
		} else {
			pMenu->DeleteMenu(ID_SHOW_IFF[i], MF_BYCOMMAND);
		}
	}

	CFrameWnd::OnInitMenu(pMenu);
}

void CMainFrame::OnLButtonUp(UINT nFlags, CPoint point) {
	CFrameWnd::OnLButtonUp(nFlags, point);
}

LRESULT CMainFrame::OnMenuPopupTest(WPARAM wParam, LPARAM lParam) {
	CMenu	menu;
	CPoint	point;

	point = *((CPoint*) lParam);

	ClientToScreen(&point);

	menu.LoadMenu(IDR_MENU1);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	return 0L;
}

void CMainFrame::OnMenuPopupToggle1() {
	if (Toggle1_var == 0)
		Toggle1_var = 1;
	else
		Toggle1_var = 0;

}

void CMainFrame::OnMikeGridcontrol() {
	CGrid	dlg;

	dlg.DoModal();
}

void CMainFrame::OnNewShipTypeChange() {
	if (Fred_view_wnd)
		Fred_view_wnd->SetFocus();
}

void CMainFrame::OnRButtonDown(UINT nFlags, CPoint point) {
	Global_point2 = point;

	PostMessage(WM_MENU_POPUP_TEST, nFlags, (int) &Global_point2);
	CFrameWnd::OnRButtonDown(nFlags, point);
}

void CMainFrame::OnUpdateLeft(CCmdUI* pCmdUI) {
	pCmdUI->Enable(::GetKeyState(VK_LBUTTON) < 0);
}

void CMainFrame::OnUpdateMenuPopupToggle1(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck(Toggle1_var);
}

void CMainFrame::OnUpdateRight(CCmdUI* pCmdUI) {
	pCmdUI->Enable(::GetKeyState(VK_RBUTTON) < 0);
}

void CMainFrame::OnUpdateViewStatusBar(CCmdUI* pCmdUI) {
	pCmdUI->SetCheck((m_wndStatusBar.GetStyle() & WS_VISIBLE) != 0);
}

void CMainFrame::OnViewStatusBar() {
	m_wndStatusBar.ShowWindow((m_wndStatusBar.GetStyle() & WS_VISIBLE) == 0);
	RecalcLayout();
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs) {
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFrameWnd::PreCreateWindow(cs);
}


int color_combo_box::CalcMinimumItemHeight() {
	int nResult = 1;

	if ((GetStyle() & (LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) ==
		(LBS_HASSTRINGS | LBS_OWNERDRAWFIXED)) {
		CClientDC dc(this);
		CFont* pOldFont = dc.SelectObject(GetFont());
		TEXTMETRIC tm;
		VERIFY(dc.GetTextMetrics(&tm));
		dc.SelectObject(pOldFont);

		nResult = tm.tmHeight;
	}

	return nResult;
}

void color_combo_box::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	int m_cyText = 24;
	CString strText;

	// You must override DrawItem and MeasureItem for LBS_OWNERDRAWVARIABLE
	ASSERT((GetStyle() & (LBS_OWNERDRAWFIXED | CBS_HASSTRINGS)) ==
		   (LBS_OWNERDRAWFIXED | CBS_HASSTRINGS));

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	if ((lpDrawItemStruct->itemID >= 0) && (lpDrawItemStruct->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))) {
		ship_info* sip = nullptr;

		// get the ship class corresponding to this item, if any
		auto itemData = lpDrawItemStruct->itemData;
		if (itemData >= 0 && itemData < Ship_info.size())
			sip = &Ship_info[itemData];

		int cyItem = GetItemHeight(lpDrawItemStruct->itemID);
		BOOL fDisabled = !IsWindowEnabled();

		COLORREF newTextColor = RGB(0x80, 0x80, 0x80);  // light gray
		if (!fDisabled) {
			if (sip == nullptr)
				newTextColor = RGB(0, 0, 0);
			else {
				auto sinfo = &Species_info[sip->species];
				newTextColor = RGB(sinfo->fred_color.rgb.r, sinfo->fred_color.rgb.g, sinfo->fred_color.rgb.b);
			}
		}

		COLORREF oldTextColor = pDC->SetTextColor(newTextColor);
		COLORREF newBkColor = GetSysColor(COLOR_WINDOW);
		COLORREF oldBkColor = pDC->SetBkColor(newBkColor);

		if (newTextColor == newBkColor)
			newTextColor = RGB(0xC0, 0xC0, 0xC0);   // dark gray

		if (!fDisabled && ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0)) {
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			pDC->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
		}

		if (m_cyText == 0)
			VERIFY(cyItem >= CalcMinimumItemHeight());

		if (sip != nullptr)
			strText = _T(sip->name);
		else if ((int)lpDrawItemStruct->itemID == Id_select_type_jump_node)
			strText = _T("Jump Node");
		else if ((int)lpDrawItemStruct->itemID == Id_select_type_start)
			strText = _T("Player Start");
		else if ((int)lpDrawItemStruct->itemID == Id_select_type_waypoint)
			strText = _T("Waypoint");
		else
			strText = _T("Invalid index!");

		pDC->ExtTextOut(lpDrawItemStruct->rcItem.left,
			lpDrawItemStruct->rcItem.top + std::max(0, (cyItem - m_cyText) / 2),
			ETO_OPAQUE, &(lpDrawItemStruct->rcItem), strText, strText.GetLength(), NULL);

		pDC->SetTextColor(oldTextColor);
		pDC->SetBkColor(oldBkColor);
	}

	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) != 0)
		pDC->DrawFocusRect(&(lpDrawItemStruct->rcItem));
}

void color_combo_box::MeasureItem(LPMEASUREITEMSTRUCT) {
	// You must override DrawItem and MeasureItem for LBS_OWNERDRAWVARIABLE
	ASSERT((GetStyle() & (LBS_OWNERDRAWFIXED | CBS_HASSTRINGS)) ==
		   (LBS_OWNERDRAWFIXED | CBS_HASSTRINGS));
}

int color_combo_box::GetShipClass(int item_index)
{
	if (item_index < 0 || item_index >= GetCount())
		return -1;
	return (int)GetItemData(item_index);
}

int color_combo_box::GetItemIndex(int ship_class)
{
	for (int i = 0; i < m_new_ship_type_combo_box.GetCount(); i++)
		if ((int)m_new_ship_type_combo_box.GetItemData(i) == ship_class)
			return i;
	return -1;
}

void url_launch(const char *url) {
	int r;

	r = (int) ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOW);
	if (r < 32) {
		const char *txt = NULL;

		switch (r) {
		case 0:	txt = XSTR("The operating system is out of memory or resources.", 1107); break;
		case ERROR_BAD_FORMAT: txt = XSTR("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).", 1108); break;
		case SE_ERR_ACCESSDENIED: txt = XSTR("The operating system denied access to the specified file. ", 1109); break;
		case SE_ERR_ASSOCINCOMPLETE: txt = XSTR("The filename association is incomplete or invalid.\r\n(You need to have a default Internet browser installed)", 1110); break;
		case SE_ERR_DDEBUSY: txt = XSTR("The DDE transaction could not be completed because other DDE transactions were being processed.", 1111); break;
		case SE_ERR_DDEFAIL: txt = XSTR("The DDE transaction failed.", 1112); break;
		case SE_ERR_DDETIMEOUT: txt = XSTR("The DDE transaction could not be completed because the request timed out.", 1113); break;
		case SE_ERR_DLLNOTFOUND: txt = XSTR("The specified dynamic-link library was not found.", 1114); break;
		case SE_ERR_OOM: txt = XSTR("There was not enough memory to complete the operation.", 1115); break;
		case SE_ERR_SHARE: txt = XSTR("A sharing violation occurred.", 1116); break;

			// No browser installed message
		case SE_ERR_NOASSOC:
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND: txt = XSTR("\r\nUnable to locate Fred Help file: \\data\\freddocs\\index.html\r\n", 1479); break;

		default: txt = XSTR("Unknown error occurred.", 1118); break;
		}
		AfxMessageBox(txt, MB_OK | MB_ICONERROR);
	}
}
