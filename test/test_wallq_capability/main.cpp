#include <fstream>
#include <iostream>

#include "../../src/ext/httplib.h"
#include "../../src/ext/json.hpp"
using json = nlohmann::json;

using namespace std;

httplib::Client cli("http://127.0.0.1:6700");
httplib::Headers headers = {{"Authorization", "Bearer token"}};

int main() {
    json j;
    ifstream in("conf.json");
    ofstream out("output.json");
    while (in.peek() != EOF) {
        in >> j;
        string s="";
        auto res = cli.Post("/", headers, j.dump().c_str(), "application/json");
        out << "----------- testing: " << j["action"]
            << "---------status: " << res->status << endl;
        out << "body:" << res->body <<"\n\n"<<endl;
    }
    in.close();
    out.close();
    // TEST_DELAY(send_message, 1)
    // TEST_DELAY(get_supported_actions, 0)
    // TEST_DELAY(get_group_member_list, 0)
}