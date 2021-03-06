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
vector<string> comf_message;
size_t curtime = 0;
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
bool contain(const string &s, const string &aim) {
    return s.find(aim) != string::npos;
}
bool equal(const string &s, const string &aim) { return s == aim; }
string get_dirty_talk() {
    httplib::Client cli("https://api.oddfar.com");

    auto res = cli.Get("/yl/q.php?c=1009&encode=text");
    return res->body;
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

void send_picture_to_onebot(string url) {
    httplib::Client cli(conf["to_wallq"]["url"]);
    httplib::Headers h = {
        {"Authorization",
         "Bearer " + string(conf["to_wallq"]["access_token"])}};

    json message;
    json pic = vector<json>(1);
    json t;
    t["url"] = url;
    t["file_id"] = "";
    pic[0]["type"] = "image";
    pic[0]["data"] = t;
    cout << pic << endl;
    message["action"] = "send_message";
    message["params"] = {{"detail_type", "group"},
                         {"group_id", conf["to_wallq"]["group_id"]},
                         {"message", pic}};

    cout << message << endl;
    auto res = cli.Post("/", h, message.dump(), "application/json");
    cout << "[" << currentDateTime() << " send picture log]:" << res->body
         << endl;
}
void send_setu() {
    httplib::Client cli("https://api.lolicon.app");

    auto res = cli.Get("/setu/v2");
    json t = json::parse(res->body);
    cout << t << endl;
    cout << " [send setu]:" << string(t["data"][0]["urls"]["original"]) << endl;
    send_picture_to_onebot(string(t["data"][0]["urls"]["original"]));
}
void init_user_info() {
    string getUrl =
        "/api/v3/guild/user-list?guild_id=" + string(conf["kook_server_id"]);
    auto res = cli.Get(getUrl.c_str(), headers);
    json temp = json::parse(res->body);
    for (auto c : temp["data"]["items"]) {
        c["current_channel_id"] = "0";
        c["loss_pack_tick"] = 5;
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
            cout << user["username"] << "joined " << t[0]["name"] << "channel"
                 << endl;
            if (conf.count("to_wallq")) {
                send_message_to_onebot("（" + currentDateTime() + "） " +
                                       string(user["username"]) + "进入了" +
                                       string(t[0]["name"]) + "频道" +
                                       ex_message[mt() % ex_message.size()]);
            }
            cout << user["current_channel_id"] << endl;
            user["current_channel_id"] = t[0]["id"];
        }
        user["loss_pack_tick"] = 4;
    } else {
        if (user["loss_pack_tick"] == 0) return;
        int t = user["loss_pack_tick"];
        user["loss_pack_tick"] = t - 1;
        cout << user["username"] << ": loss_pack_tick" << t - 1 << endl;
        if (user["loss_pack_tick"] == 0) user["current_channel_id"] = "0";
    }
}
void update_users_status() {
    try {
        for (auto &c : users_info) {
            update_joined_channel(c);
        }
    } catch (exception e) {
        cout << "[panic]: " << e.what() << endl;
    }
}
int main() {
    // load configations
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
        if (!conf["to_wallq"].count("webhook_port"))
            cout << "Info: No configuration for to_wallq.webhook_port" << endl;
        ex_message = conf["to_wallq"]["saohua"];
        comf_message = conf["to_wallq"]["comf"];
    }

    // listen to wallq events, and handle
    httplib::Server svr;
    svr.Post("/", [&](auto _req, auto res) {
        try {
            json req = json::parse(_req.body);

            // cout << "[receive log]: " << req << endl;
            if (req["detail_type"] == "group" &&
                req["group_id"] == conf["to_wallq"]["group_id"] &&
                req.count("alt_message")) {
                string txt = req["alt_message"];

                if (contain(txt, "麻") || contain(txt, "躺") ||
                    contain(txt, "烦")) {
                    send_message_to_onebot(
                        comf_message[mt() % comf_message.size()]);
                }

                if (contain(txt, "骂 ")) {
                    int ind = txt.find("骂 ");
                    cout << "[fuck]";
                    if (ind + 4 < txt.size()) {
                        string name = txt.substr(ind + 4);
                        string words = get_dirty_talk();
                        if (contain(name, "我")) name = "你";
                        cout << name << "~ " << words << endl;
                        send_message_to_onebot(name + "~ " + words);
                    }
                }

                if (equal(txt, "kook") || contain(txt, "频道")) {
                    string resp = "";
                    int ind = 0;
                    for (auto c : users_info) {
                        if (!c.count("current_channel_id")) continue;
                        if (c["current_channel_id"] == "0") continue;
                        resp += (ind ? "、" : "") + string(c["username"]);
                        ind++;
                    }
                    resp += (ind ? " 已经在频道里了……"
                                 : "频道里没人，今天没人玩理～");
                    cout << "kook";
                    send_message_to_onebot(resp);
                }
            }
        } catch (exception e) {
            cout << "[panic]: " << e.what() << endl;
        }

        // if (contain(txt, "色图") || contain(txt, "涩图") ||
        //     contain(txt, "rir")) {
        //     static size_t cur = curtime;
        //     if (curtime - cur >= 20) {
        //         cur = curtime;

        //         send_setu();
        //     }else{
        //         send_message_to_onebot("太频繁了，休息一下吧～");
        //     }
        // }
    });
    thread t([&] { svr.listen("0.0.0.0", conf["to_wallq"]["webhook_port"]); });

    // lopp to check KooK users' status
    init_user_info();

    while (1) {
        update_users_status();
        sleep(1);
        curtime++;
        cout << "-";
        cout.flush();
    }
}
// xJ_2013320115
// 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