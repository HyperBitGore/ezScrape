#include "ezScrape.h"

wxIMPLEMENT_APP(cApp);

cApp::cApp() {

}
cApp::~cApp() {

}

bool cApp::OnInit() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
	mFrame* frame = new mFrame();
	frame->Show(true);
	return true;
}
