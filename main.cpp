#include "ezScrape.h"
wxBEGIN_EVENT_TABLE(mFrame, wxFrame)
EVT_BUTTON(9001, OnReset)
EVT_BUTTON(9002, OnStart)
EVT_BUTTON(9003, OnDepth)
wxEND_EVENT_TABLE()


MemBlock* curdata = nullptr;
std::vector<std::string> links;
void mFrame::OnReset(wxCommandEvent& evt) {
	curl_easy_reset(curl);
	link_text->Clear();
}

size_t get_data(char* buf, size_t itemsize, size_t nitems, void* ignore) {
	size_t bytes = itemsize * nitems;
	MemBlock* me = new MemBlock;
	me->mem = (char*)std::malloc(bytes);
	if (me->mem != 0) {
		std::memcpy(me->mem, buf, bytes);
	}
	me->size = bytes;
	me->next = curdata;
	curdata = me;
	return bytes;
}

void mFrame::scrapeLink(std::string link) {
	curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_data);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
	curl_easy_perform(curl);
	//create base url
	std::string base_url;
	int c = 0;
	for (int i = 0; i < link.size(); i++) {
		if(link[i] == '/'){
			c++;
		}
		if (c > 2) {
			i = link.size();
		}
		else {
			base_url.push_back(link[i]);
		}
	}
	MemBlock* ptr = curdata;
	while (ptr != nullptr) {
		bool found = false;
		//finding links
		for (size_t i = 0; i < ptr->size; i++) {
			if (ptr->mem[i] == 'h') {
				std::string it = "href=\"";
				int n = 0;
				bool quit = false;
				for (size_t j = i; j < ptr->size && !quit; j++) {
					if (ptr->mem[j] == it[n]) {
						n++;
					}
					else {
						n = 0;
						quit = true;
					}
					if (n == 5) {
						i = j;
						found = true;
						break;
					}
				}
			}
			if (found && i < ptr->size) {
				for (i; ptr->mem[i] != '\"' && i < ptr->size; i++);
				i++;
				std::string str;
				//getting actual link 
				for (size_t n = i; ptr->mem[n] != '\"' && n < ptr->size; n++) {
					str.push_back(ptr->mem[n]);
				}
				//add the begining of link if str starts with /
				if (str[0] == '/') {
					str = base_url + str;
				}
				if (str.size() > 7 && str.find("https://") != std::string::npos) {
					if (links_map.get(str) == nullptr) {
						data_listing->AppendString(str);
						links.push_back(str);
						links_map.insert(str, 0);
					}
				}
			}
		}
		ptr = ptr->next;
	}
	ptr = curdata;
	//cleanup the memblocks
	while (ptr != nullptr) {
		std::free(ptr->mem);
		ptr = ptr->next;
	}
	curdata = nullptr;
}
void mFrame::downloadLink(std::string link) {
	
}
bool checkFileEnding(std::string link, std::string end) {
	for (int i = 0; i < link.size(); i++) {
		if (link[i] == end[0]) {
			int j = i;
			for (int k = 0; j < link.size() && k < end.size(); j++, k++) {
				if (link[j] != end[k]) {
					break;
				}
				else if (k == end.size() - 1) {
					return true;
				}
			}
		}
	}
	return false;
}

void mFrame::scrapeLoop(int* pre_size, std::string ender, bool* repeat) {
	*repeat = false;
	for (int i = 0; i < *pre_size; i++) {
		if (!checkFileEnding(links[i], ender)) {
			scrapeLink(links[i]);
		}
		else {
			downloadLink(links[i]);
		}
	}
	for (int i = 0; i < *pre_size; i++) {
		links.erase(links.begin());
	}
	cur_depth++;
	if (cur_depth <= m_depth) {
		*pre_size = links.size();
		*repeat = true;
	}
}

void mFrame::OnStart(wxCommandEvent& evt) {
	//read starting url and push back a bunch of links to either scrape or download targeted file types 
	links.clear();
	cur_depth = 1;
	std::string temp = std::string(link_text->GetValue().mb_str());
	std::string ender = std::string(file_end->GetValue().mb_str());
	link_text->Clear();
	//parse html data and either add links to be scraped if not at max depth, or just download files with currently set ending
	scrapeLink(temp);
	bool rep = true;
	if (links.size() > 0) {
		while (rep) {
			int pre_size = links.size();
			scrapeLoop(&pre_size, ender, &rep);
		}
	}
}



void mFrame::OnExit(wxCommandEvent& event)
{
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	links.clear();
	links_map.~HashMap();
	Close(true);
}
void mFrame::OnDepth(wxCommandEvent& event)
{
	m_depth = wxGetNumberFromUser("Input max depth", "Input max depth", "Depth: ", m_depth, 1, 9999);
}

int hash_func(std::string key) {
	int n = 0;
	for (auto& i : key) {
		n += i;
	}
	return n % 256;
}


mFrame::mFrame() : wxFrame(NULL, wxID_ANY, "ezScrape", wxPoint(30, 30), wxSize(400, 600)) {
	curl = curl_easy_init();

	wxStaticText* text1 = new wxStaticText(this, wxID_ANY, "URL: ", wxPoint(1, 5), wxSize(70, 20));
	wxStaticText* text2 = new wxStaticText(this, wxID_ANY, "File Type: ", wxPoint(1, 40), wxSize(70, 20));
	link_text = new wxTextCtrl(this, wxID_ANY, "", wxPoint(1, 20), wxSize(350, 20));
	file_end = new wxTextCtrl(this, wxID_ANY, ".zip", wxPoint(1, 60), wxSize(250, 20));

	wxButton* resetb = new wxButton(this, 9001, "Reset", wxPoint(1, 90), wxSize(40, 30));
	wxButton* startb = new wxButton(this, 9002, "Start Scraping", wxPoint(55, 90), wxSize(120, 30));
	wxButton* get_depth = new wxButton(this, 9003, "Set Depth", wxPoint(200, 90), wxSize(120, 30));

	data_listing = new wxListBox(this, wxID_ANY, wxPoint(1, 120), wxSize(350, 400));
	links_map.setHashFunction(hash_func);
	Bind(wxEVT_MENU, &mFrame::OnExit, this, wxID_EXIT);
}