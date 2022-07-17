#include <fstream>
#include <iostream>
#include <thread>

#include "../../src/ext/httplib.h"
#include "../../src/ext/json.hpp"
using json = nlohmann::json;

using namespace std;

int main() {
    httplib::Server svr;
    svr.Post("/", [&](auto req, auto res) { cout << req.body << endl; });
    thread t([&] {
        int id = 0;
        while (true) {
            this_thread::sleep_for(chrono::seconds(1));
            string s;
            cin>>s;
            cout<<s<<endl;
        }
    });
    svr.listen("0.0.0.0", 6701);
}