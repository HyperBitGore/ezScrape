#pragma once
#define CURL_STATICLIB
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <wx/wx.h>
#include <wx/numdlg.h>
#include "g_primitive_funcs.h"

//implement actual file downloading

struct MemBlock {
	char* mem;
	size_t size;
	MemBlock* next;
};


class cApp : public wxApp {
public:
	cApp();
	~cApp();
	virtual bool OnInit();
};

enum {ID_SETTINGS = 84};

class mFrame : public wxFrame {
public:
	mFrame();
	wxTextCtrl* link_text;
	wxTextCtrl* file_end;
	wxListBox* data_listing;
private:
	CURL* curl;
	void OnSettings(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	long m_depth = 1;
	long cur_depth = 0;
	std::vector<std::string>links;
	void scrapeLink(std::string link);
	void downloadLink(std::string link, std::string ender);
	void scrapeLoop(int* pre_size, std::string ender, bool* repeat);
	Gore::HashMap<int> links_map;
public:
	void OnReset(wxCommandEvent& evt);
	void OnStart(wxCommandEvent& evt);
	void OnDepth(wxCommandEvent& evt);
	wxDECLARE_EVENT_TABLE();
};
