#pragma once
// dllmain.cpp : ���������� ����� ����� ��� ���������� DLL.
#include <Windows.h>
#include <codecvt>
#include <locale>
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <LLAPI.h>
#include <iostream>
#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#include<third-party/yaml-cpp/yaml.h>
#pragma warning(pop)
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
using namespace std;

std::wstring to_wstring(const std::string& str, const std::locale& loc);

std::string utf8_encode(const std::wstring& wstr);

string utf8_to_string(const char* utf8str, const locale& loc);

class World
{
public:
    string name;
    vector<string> permissions;
    string group;
    string prefix;
    string suffix;
    vector<string> inheritances;
};

class _Group
{
public:
    string name; //��� ������
    string prefix;
    string suffix;
    vector<string> perms;       //�����
    bool is_default;  //���������� �� �� ���������
    bool build; //������������� ���/����
    vector<string> inheritances; //������,������� ����� �����������
    vector<World> worlds;
};

class _Groups {
public:
    vector<_Group> groups; //������
};

class _User {
public:
    string nickname; //���
    string prefix;
    string suffix;
    vector<string> groups;       //������ ������
    vector<string> permissions; //����� ������
    vector<World> worlds;
};

class Users {
public:
    vector<_User> users; //������
};


namespace YAML
{
    template <>
    struct convert<World>
    {
        static Node encode(const World& rhs);
        static bool decode(const Node& node, World& rhs);
    };
} // namespace YAML

namespace YAML
{
    template <>
    struct convert<_Group>
    {
        static Node encode(const _Group& rhs);
        static bool decode(const Node& node, _Group& rhs);
    };
} // namespace YAML

namespace YAML
{
    template <>
    struct convert<_User>
    {
        static Node encode(const _User& rhs);
        static bool decode(const Node& node, _User& rhs);
    };
} // namespace YAML

struct ModifyworldConfig
{
    bool informPlayers;
    bool itemRestrictions;
    bool item_use_check;
    bool whitelist = false;
    vector<string> messages;
};

namespace YAML
{
    template <>
    struct convert<ModifyworldConfig>
    {
        static Node encode(const ModifyworldConfig& rhs);
        static bool decode(const Node& node, ModifyworldConfig& rhs);
    };
} // namespace YAML

_Group load_group(string name);

_User load_user(string nick);

vector<string> split(string s, string delimiter);

bool checkPerm(string pl, string perm);

bool checkPermWorlds(string pl, string perm, string world);

string get_msg(string type);