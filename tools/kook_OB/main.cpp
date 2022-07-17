//临时的kook阉割版OB实现
//运行于kook的通信协议和OneBot12协议上
#include <fstream>
#include <iostream>
#include <thread>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../../src/ext/httplib.h"
#include "../../src/ext/json.hpp"
using json = nlohmann::json;

using namespace std;

httplib::Client cli("https://www.kookapp.cn");

httplib::Headers headers = {
    {"Authorization", "Bot 1/MTE2MzM=/BXBp24gXmH5T01yX9tQrBw=="},
    {"Cookie", "_csrf_chuanyu=vXpstWhWXq1OxhUU7pwq81l1WB6PZqs4"},
    {"Cache-Control", "no-cache"}};

json users_info;
json conf;
vector<string> ex_message;
mt19937 mt(time(nullptr));
const std::string currentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}

void send_message_to_onebot(string text) {
    httplib::Client cli(conf["to_wallq"]["url"]);
    httplib::Headers h = {
        {"Authorization",
         "Bearer " + string(conf["to_wallq"]["access_token"])}};
    json message;
    message["action"] = "send_message";
    message["params"] = {{"detail_type", "group"},
                         {"group_id", conf["to_wallq"]["group_id"]},
                         {"message", text}};

    auto res = cli.Post("/", h, message.dump(), "application/json");
    cout << "[" << currentDateTime() << " send log]:" << res->body << endl;
}
void init_user_info() {
    string getUrl =
        "/api/v3/guild/user-list?guild_id=" + string(conf["kook_server_id"]);
    auto res = cli.Get(getUrl.c_str(), headers);
    json temp = json::parse(res->body);
    for (auto c : temp["data"]["items"]) {
        c["current_channel_id"] = 0;
        users_info.push_back(c);
        cout << c["id"] << " " << c["username"] << endl;
    }
}
void update_joined_channel(json &user) {
    string getUrl = "/api/v3/channel-user/get-joined-channel?guild_id=" +
                    string(conf["kook_server_id"]) +
                    "&user_id=" + string(user["id"]);

    auto res = cli.Get(getUrl.c_str(), headers);
    json temp = json::parse(res->body);
    json t = temp["data"]["items"];
    if (t.size() > 0) {
        if (t[0]["id"] != user["current_channel_id"]) {
            cout << user["username"] << "进入了" << t[0]["name"] << "频道"
                 << endl;
            user["current_channel_id"] = t[0]["id"];
            if (conf.count("to_wallq")) {
                send_message_to_onebot("（" + currentDateTime() + "） " +
                                       string(user["username"]) + "进入了" +
                                       string(t[0]["name"]) + "频道" +
                                       ex_message[mt() % ex_message.size()]);
            }
        }
    }
}
void update_users_status() {
    for (auto &c : users_info) {
        update_joined_channel(c);
    }
}
int main() {
    ifstream conf_in("conf.json");
    conf_in >> conf;
    conf_in.close();
    if (!conf.count("http_webhook")) {
        cout << "Error: No configuration for http_webhook" << endl;
        return 0;
    }
    if (!conf.count("to_wallq")) {
        cout << "Info: No configuration for to_wallq" << endl;
    } else {
        ex_message = conf["to_wallq"]["saohua"];
    }

    // cli.enable_server_certificate_verification(false);

    init_user_info();
    while (1) {
        cout << endl;
        update_users_status();
        sleep(1);
        cout << "-";
    }
}
