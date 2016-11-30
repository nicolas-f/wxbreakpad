/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal wxWidgets sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows it is in resources and even
// though we could still include the XPM here it would be unused)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "../sample.xpm"
#endif

#ifdef _WIN32
	#include "client/windows/crash_generation/client_info.h"
	#include "client/windows/crash_generation/crash_generation_server.h"
	#include "client/windows/handler/exception_handler.h"
	#include "client/windows/common/ipc_protocol.h"
#endif // _WIN32


#include "abstract_class.h"

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit() wxOVERRIDE;
	virtual int OnExit() wxOVERRIDE;
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);

	wxTextCtrl *MainEditBox;

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
    // any class wishing to process wxWidgets events must use this macro
    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT
};



namespace google_breakpad {

	const int kMaxLoadString = 100;
	const wchar_t kPipeName[] = L"\\\\.\\pipe\\BreakpadCrashServices\\TestServer";

	// Maximum length of a line in the edit box.
	const size_t kMaximumLineLength = 256;

	static size_t kCustomInfoCount = 2;
	static CustomInfoEntry kCustomInfoEntries[] = {
		CustomInfoEntry(L"prod", L"CrashTestApp"),
		CustomInfoEntry(L"ver", L"1.0"),
	};

	static ExceptionHandler* handler = NULL;
	static CrashGenerationServer* crash_server = NULL;


	static void AppendTextToEditBox(TCHAR* text) {
		SYSTEMTIME current_time;
		GetLocalTime(&current_time);
		TCHAR line[kMaximumLineLength];
		int result = swprintf_s(line,
			kMaximumLineLength,
			L"[%.2d-%.2d-%.4d %.2d:%.2d:%.2d] %s",
			current_time.wMonth,
			current_time.wDay,
			current_time.wYear,
			current_time.wHour,
			current_time.wMinute,
			current_time.wSecond,
			text);

		if (result == -1) {
			return;
		}

		// TODO set text line in window
	}

	bool ShowDumpResults(const wchar_t* dump_path,
		const wchar_t* minidump_id,
		void* context,
		EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion,
		bool succeeded) {
		TCHAR* text = new TCHAR[kMaximumLineLength];
		text[0] = wxT('\0');
		int result = swprintf(text,
			kMaximumLineLength,
			TEXT("Dump generation request %s\r\n"),
			succeeded ? wxT("succeeded") : wxT("failed"));
		if (result == -1) {
			delete[] text;
		}

		//TODO call AppendTextToEditBox
		return succeeded;
	}

	static void ShowClientConnected(void* context,
		const ClientInfo* client_info) {
		TCHAR* line = new TCHAR[kMaximumLineLength];
		line[0] = wxT('\0');
		int result = swprintf(line,
			kMaximumLineLength,
			L"Client connected:\t\t%d\r\n",
			client_info->pid());

		if (result == -1) {
			delete[] line;
			return;
		}

		//TODO call AppendTextToEditBox
	}

	static void ShowClientCrashed(void* context,
		const ClientInfo* client_info,
		const wstring* dump_path) {
		TCHAR* line = new TCHAR[kMaximumLineLength];
		line[0] = wxT('\0');
		int result = swprintf_s(line,
			kMaximumLineLength,
			wxT("Client requested dump:\t%d\r\n"),
			client_info->pid());

		if (result == -1) {
			delete[] line;
			return;
		}

		//TODO AppendTextToEditBox line

		CustomClientInfo custom_info = client_info->GetCustomInfo();
		if (custom_info.count <= 0) {
			return;
		}

		wstring str_line;
		for (size_t i = 0; i < custom_info.count; ++i) {
			if (i > 0) {
				str_line += L", ";
			}
			str_line += custom_info.entries[i].name;
			str_line += L": ";
			str_line += custom_info.entries[i].value;
		}

		line = new TCHAR[kMaximumLineLength];
		line[0] = wxT('\0');
		result = swprintf_s(line,
			kMaximumLineLength,
			L"%s\n",
			str_line.c_str());
		if (result == -1) {
			delete[] line;
			return;
		}

		//TODO AppendTextToEditBox line
	}

	static void ShowClientExited(void* context,
		const ClientInfo* client_info) {
		TCHAR* line = new TCHAR[kMaximumLineLength];
		line[0] = wxT('\0');
		int result = swprintf(line,
			kMaximumLineLength,
			wxT("Client exited:\t\t%d\r\n"),
			client_info->pid());

		if (result == -1) {
			delete[] line;
			return;
		}

		//TODO AppendTextToEditBox line
	}

	void CrashServerStart(std::wstring dump_path) {
		// Do not create another instance of the server.
		if (crash_server) {
			return;
		}


		if (!wxDir::Exists(dump_path) && !wxDir::Make(dump_path)) {
			wxMessageBox(wxT("Unable to create dump directory"), wxT("Dumper"), wxICON_ERROR);
			return;
		}

		crash_server = new CrashGenerationServer(kPipeName,
			NULL,
			ShowClientConnected,
			NULL,
			ShowClientCrashed,
			NULL,
			ShowClientExited,
			NULL,
			NULL,
			NULL,
			true,
			&dump_path);

		if (!crash_server->Start()) {
			//wxMessageBox(wxT("Unable to start server"), wxT("Dumper"), wxICON_ERROR);
			delete crash_server;
			crash_server = NULL;
		}
	}

	void CrashServerStop() {
		delete crash_server;
		crash_server = NULL;
	}

	void DerefZeroCrash() {
		int* x = 0;
		*x = 1;
	}

	void InvalidParamCrash() {
		printf(NULL);
	}

	void PureCallCrash() {
		Derived derived;
	}

	void RequestDump() {
		if (!handler->WriteMinidump()) {
			wxMessageBox(wxT("Dump request failed"), wxT("Dumper"), wxICON_ERROR);
		}
		kCustomInfoEntries[1].set_value(L"1.1");
	}

	void CleanUp() {

		if (handler) {
			delete handler;
		}

		if (crash_server) {
			delete crash_server;
		}
	}
	/*

	case ID_SERVER_START:
	CrashServerStart();
	break;
	case ID_SERVER_STOP:
	CrashServerStop();
	break;
	case ID_CLIENT_DEREFZERO:
	DerefZeroCrash();
	break;
	case ID_CLIENT_INVALIDPARAM:
	InvalidParamCrash();
	break;
	case ID_CLIENT_PURECALL:
	PureCallCrash();
	break;
	case ID_CLIENT_REQUESTEXPLICITDUMP:
	RequestDump();
	break;

	*/

}  // namespace google_breakpad



// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------
// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
wxEND_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
wxIMPLEMENT_APP(MyApp);

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
	using namespace google_breakpad;

	CustomClientInfo custom_info = { kCustomInfoEntries, kCustomInfoCount };

	wxFileName dumpFolder = wxFileName(wxStandardPaths::Get().GetUserDataDir(), "dumps");
	dumpFolder.Mkdir();
	std::wstring dump_path = dumpFolder.GetFullPath();

    CrashServerStart(dump_path);
	// This is needed for CRT to not show dialog for invalid param
	// failures and instead let the code handle it.
	#ifdef _WIN32
		_CrtSetReportMode(_CRT_ASSERT, 0);
	#endif
	handler = new ExceptionHandler(dump_path,
		NULL,
		google_breakpad::ShowDumpResults,
		NULL,
		ExceptionHandler::HANDLER_ALL,
		MiniDumpNormal,
		kPipeName,
		&custom_info);

    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    MyFrame *frame = new MyFrame("Minimal wxWidgets App");

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

int MyApp::OnExit() {
	CleanUp();
	return 0;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title)
{
    // set the frame icon
    SetIcon(wxICON(sample));

	// Initialize our text box with an id of TEXT_Main, and the label "hi"
	MainEditBox = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE | wxTE_RICH, wxDefaultValidator, wxTextCtrlNameStr);

    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, wxT("&About\tF1"), wxT("Show about dialog"));

    fileMenu->Append(Minimal_Quit, wxT("E&xit\tAlt-X"), wxT("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(helpMenu, wxT("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
}


// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format
                 (
                    "Welcome to %s!\n"
                    "\n"
                    "This is the minimal wxWidgets sample\n"
                    "running under %s.",
                    wxVERSION_STRING,
                    wxGetOsDescription()
                 ),
                 "About wxWidgets minimal sample",
                 wxOK | wxICON_INFORMATION,
                 this);
}

