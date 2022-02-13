// dllmain.cpp : Определяет точку входа для приложения DLL.
#include <Windows.h>
#include <codecvt>
#include <locale>
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <EventAPI.h>
#include <LLAPI.h>
#include <MC/Actor.hpp>
#include <MC/Block.hpp>
#include <MC/CommandOrigin.hpp>
#include <MC/CommandOutput.hpp>
#include <MC/CommandPosition.hpp>
#include <MC/CommandRegistry.hpp>
#include <MC/CommandParameterData.hpp>
#include <MC/ComponentItem.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/HashedString.hpp>
#include <MC/Item.hpp>
#include <MC/ItemStack.hpp>
#include <MC/Level.hpp>
#include <MC/Mob.hpp>
#include <MC/MobEffect.hpp>
#include <MC/MobEffectInstance.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/Tag.hpp>
#include <MC/Types.hpp>
#include <MC/Dimension.hpp>
#include <MC/Container.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include <RegCommandAPI.h>
#include <MC/SerializedSkin.hpp>
#include <MC/BinaryStream.hpp>
#include <ServerAPI.h>
#include <MC/GameTypeConv.hpp>
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
#include <MC/PropertiesSettings.hpp>
#include <regex>
#include "PAPI.H"
using namespace std::filesystem;

#pragma comment(lib, "bedrock_server_api.lib")
#pragma comment(lib, "bedrock_server_var.lib")
#pragma comment(lib, "LiteLoader.lib")
#pragma comment(lib, "yaml-cpp.lib")
#pragma comment(lib, "SymDBHelper.lib")

using namespace std;


class MotdCmd : public Command
{
public:
    void execute(CommandOrigin const& ori, CommandOutput& outp) const override
    {
        auto id = ori.getPlayer()->getDimensionId();
        string dim;
        if (id == 0)
            dim = "OverWorld";
        else if (id == 1)
            dim = "Nether";
        else if (id == 2)
            dim = "End";
        string perm = "wp.motd";
        string nick = ori.getPlayer()->getName();
        string msg = get_msg("permissionDenied");
        if (ori.getPermissionsLevel() == CommandPermissionLevel::Console)
        {
            string str;
            char ch;
            ifstream in("server.properties");
            while (in.get(ch))
            {
                if (ch == '\n')
                    break;
                str += ch;
            }
            std::regex target("server-name=");
            std::string replacement = " ";
            std::string s2 = std::regex_replace(str, target, replacement);
            outp.success(s2);
            return;
        }
        else if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
        {
            string str;
            char ch;
            ifstream in("server.properties");
            while (in.get(ch))
            {
                if (ch == '\n')
                    break;
                str += ch;
            }
            std::regex target("server-name=");
            std::string replacement = " ";
            std::string s2 = std::regex_replace(str, target, replacement);
            outp.success(s2);
            return;
        }
        else
        {
            outp.error(msg);
            return;
        }
    }
    static void setup(CommandRegistry* registry) {

        registry->registerCommand(
            "motd", "Get server motd.", CommandPermissionLevel::Any, { (CommandFlagValue)0 },
            { (CommandFlagValue)0x80 });
        registry->registerOverload<MotdCmd>("motd");
    }
};

enum CommandParameterOption;

class Wp : public Command
{
    enum class Operation
    {
        Add,
        Bancmd,
        Unbancmd,
        Banitem,
        Unbanitem,
        Border,
        Gm,
        Lock,
        Ls,
        Max,
        Motd,
        Noexplode,
        Protect,
        Pvp,
        Rm,
        Breakable,
        Unbreakable,
        Unlock
    } op;
    CommandSelector<Actor> player;
    string cmd;
    string item;
    int x1, z1, x2, z2;
    int gm;
    string world_name;
    int max_xal;
    string motd;
    string block_id;
    string opval1;
public:
    void execute(CommandOrigin const& ori, CommandOutput& outp) const override
    {
        switch (op)
        {
          case Operation::Add:
          {
              string perm = "wp.cmd.addrm";
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              if (world_name == "" || player.getName() == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && player.getName() != "")
              {
                  ifstream in("plugins/World Protect/" + world_name + "_auth.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_auth.txt");
                      fout << player.getName() << endl;
                      fout.close();
                      outp.success("Player " + player.getName() + " succes add to auth list for world " + world_name);
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_auth.txt",ios_base::app);
                      fout << player.getName() << endl;
                      fout.close();
                      outp.success("Player " + player.getName() + " succes add to auth list for world " + world_name);
                      return;
                  }
              }
              else if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/" + world_name + "_auth.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_auth.txt");
                      fout << player.getName() << endl;
                      fout.close();
                      outp.success("Player " + player.getName() + " succes add to auth list for world " + world_name);
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_auth.txt", ios_base::app);
                      fout << player.getName() << endl;
                      fout.close();
                      outp.success("Player " + player.getName() + " succes add to auth list for world " + world_name);
                      return;
                  }
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Bancmd:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.bancmd";
              if (world_name == "" || cmd == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && cmd != "")
              {
                  ifstream in("plugins/World Protect/" + world_name + "_cmd.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_cmd.txt");
                      fout << cmd << endl;
                      fout.close();
                      outp.success("Command " + cmd + " succes add to ban list for world " + world_name);
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_cmd.txt", ios_base::app);
                      fout << cmd << endl;
                      fout.close();
                      outp.success("Command " +cmd + " succes add to ban list for world " + world_name);
                      return;
                  }
              }
              else if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/" + world_name + "_cmd.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_cmd.txt");
                      fout << cmd << endl;
                      fout.close();
                      outp.success("Command " + cmd + " succes add to ban list for world " + world_name);
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_cmd.txt", ios_base::app);
                      fout << cmd << endl;
                      fout.close();
                      outp.success("Command " + cmd + " succes add to ban list for world " + world_name);
                      return;
                  }
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Unbancmd:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.bancmd";
              if (world_name == "" || cmd == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && cmd != "")
              {
                  ifstream in("plugins/World Protect/" + world_name + "_cmd.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      outp.error("Ban list is empty!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      string str;
                      vector<string> cmds;
                      while (getline(in, str))
                      {
                          if (str != cmd)
                              cmds.push_back(str);
                      }
                      in.close();
                      string fn = "plugins/World Protect/" + world_name + "_cmd.txt";
                      remove(fn.c_str());
                      ofstream fout("plugins/World Protect/" + world_name + "_cmd.txt");
                      for (auto v1 : cmds)
                          fout << v1 << endl;
                      fout.close();
                      outp.success("Command " + cmd + " succes removed from ban list for world " + world_name);
                      return;
                  }
              }
              else if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/" + world_name + "_cmd.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      outp.error("Ban list is empty!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      string str;
                      vector<string> cmds;
                      while (getline(in, str))
                      {
                          if (str != cmd)
                              cmds.push_back(str);
                      }
                      in.close();
                      string fn = "plugins/World Protect/" + world_name + "_cmd.txt";
                      remove(fn.c_str());
                      ofstream fout("plugins/World Protect/" + world_name + "_cmd.txt");
                      for (auto v1 : cmds)
                          fout << v1 << endl;
                      fout.close();
                      outp.success("Command " + cmd + " succes removed from ban list for world " + world_name);
                      return;
                  }
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Banitem:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.banitem";
              if (world_name == "" || item == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && item != "")
              {
                  ifstream in("plugins/World Protect/" + world_name + "_items.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_items.txt");
                      fout << item << endl;
                      fout.close();
                      outp.success("Item " + item + " succes add to ban list for world " + world_name);
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_items.txt", ios_base::app);
                      fout << item << endl;
                      fout.close();
                      outp.success("Item " + item + " succes add to ban list for world " + world_name);
                      return;
                  }
              }
              else if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/" + world_name + "_items.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_items.txt");
                      fout << item << endl;
                      fout.close();
                      outp.success("Item " + item + " succes add to ban list for world " + world_name);
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_items.txt", ios_base::app);
                      fout << item << endl;
                      fout.close();
                      outp.success("Item " + item + " succes add to ban list for world " + world_name);
                      return;
                  }
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Unbanitem:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.banitem";
              if (world_name == "" || item == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && item != "")
              {
                  ifstream in("plugins/World Protect/" + world_name + "_items.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      outp.error("Ban list is empty!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      string str;
                      vector<string> items;
                      while (getline(in, str))
                      {
                          if (str != item)
                              items.push_back(str);
                      }
                      in.close();
                      string fn = "plugins/World Protect/" + world_name + "_items.txt";
                      remove(fn.c_str());
                      ofstream fout("plugins/World Protect/" + world_name + "_items.txt");
                      for (auto v1 : items)
                          fout << v1 << endl;
                      fout.close();
                      outp.success("Item " + item + " succes removed from ban list for world " + world_name);
                      return;
                  }
              }
              else if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/" + world_name + "_items.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      outp.error("Ban list is empty!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      string str;
                      vector<string> items;
                      while (getline(in, str))
                      {
                          if (str != item)
                              items.push_back(str);
                      }
                      in.close();
                      string fn = "plugins/World Protect/" + world_name + "_items.txt";
                      remove(fn.c_str());
                      ofstream fout("plugins/World Protect/" + world_name + "_items.txt");
                      for (auto v1 : items)
                          fout << v1 << endl;
                      fout.close();
                      outp.success("Item " + item + " succes removed from ban list for world " + world_name);
                      return;
                  }
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Border:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.border";
              if (world_name == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && opval1 != "none")
              {
                  ifstream in("plugins/World Protect/borders.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/borders.txt");
                      fout << world_name << " " << to_string(x1) << ":" << to_string(z1) << ":" << to_string(x2) << ":" << to_string(z2) << endl;
                      outp.success("Border for world " + world_name + "succefull set!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/borders.txt",ios_base::app);
                      fout << world_name << " " << to_string(x1) << ":" << to_string(z1) << ":" << to_string(x2) << ":" << to_string(z2) << endl;
                      outp.success("Border for world " + world_name + "succefull set!");
                      return;
                  }
              }
              else if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && opval1 == "none")
              {
                  ifstream in("plugins/World Protect/borders.txt");
                  string str;
                  vector<string> borders;
                  while (getline(in, str))
                  {
                      if (str != world_name)
                         borders.push_back(str);
                  }
                  in.close();
                  remove("plugins/World Protect/borders.txt");
                  ofstream fout("plugins/World Protect/borders.txt");
                  for (auto v1 : borders)
                      fout << v1 << endl;
                  fout.close();
                  outp.success("Brders for world " + world_name + " succefull removed!");
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)) && world_name != "" && opval1 != "none")
              {
                  ifstream in("plugins/World Protect/borders.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/borders.txt");
                      fout << world_name << " " << to_string(x1) << ":" << to_string(z1) << ":" << to_string(x2) << ":" << to_string(z2) << endl;
                      outp.success("Border for world " + world_name + "succefull set!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/borders.txt", ios_base::app);
                      fout << world_name << " " << to_string(x1) << ":" << to_string(z1) << ":" << to_string(x2) << ":" << to_string(z2) << endl;
                      outp.success("Border for world " + world_name + "succefull set!");
                      return;
                  }
              }
              else if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)) && world_name != "" && opval1 == "none")
              {
                  ifstream in("plugins/World Protect/borders.txt");
                  string str;
                  vector<string> borders;
                  while (getline(in, str))
                  {
                      if (str != world_name)
                          borders.push_back(str);
                  }
                  in.close();
                  remove("plugins/World Protect/borders.txt");
                  ofstream fout("plugins/World Protect/borders.txt");
                  for (auto v1 : borders)
                      fout << v1 << endl;
                  fout.close();
                  outp.success("Brders for world " + world_name + " succefull removed!");
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Gm:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.gm";
              if (world_name == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && opval1 != "none")
              {
                  ifstream in("plugins/World Protect/gm.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/gm.txt");
                      fout << world_name << ":" << to_string(gm) << endl;
                      fout.close();
                      outp.success("Gamemode " + to_string(gm) + " succefull set for world " + world_name);
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/gm.txt", ios_base::app);
                      fout << world_name << ":" << to_string(gm) << endl;
                      fout.close();
                      outp.success("Gamemode " + to_string(gm) + " succefull set for world " + world_name);
                      return;
                  }
              }
              else if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && opval1 == "none")
              {
                  ifstream in("plugins/World Protect/borders.txt");
                  string str;
                  vector<string> borders;
                  while (getline(in, str))
                  {
                      if (str != world_name)
                          borders.push_back(str);
                  }
                  in.close();
                  remove("plugins/World Protect/borders.txt");
                  ofstream fout("plugins/World Protect/borders.txt");
                  for (auto v1 : borders)
                      fout << v1 << endl;
                  fout.close();
                  outp.success("Gamemode in world" + world_name + " succefull reset!");
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)) && world_name != "" && opval1 != "none")
              {
                  ifstream in("plugins/World Protect/gm.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/gm.txt");
                      fout << world_name << ":" << to_string(gm) << endl;
                      fout.close();
                      outp.success("Gamemode " + to_string(gm) + " succefull set for world " + world_name);
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/gm.txt", ios_base::app);
                      fout << world_name << ":" << to_string(gm) << endl;
                      fout.close();
                      outp.success("Gamemode " + to_string(gm) + " succefull set for world " + world_name);
                      return;
                  }
              }
              else if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)) && world_name != "" && opval1 == "none")
              {
                  ifstream in("plugins/World Protect/borders.txt");
                  string str;
                  vector<string> borders;
                  while (getline(in, str))
                  {
                      if (str != world_name)
                          borders.push_back(str);
                  }
                  in.close();
                  remove("plugins/World Protect/borders.txt");
                  ofstream fout("plugins/World Protect/borders.txt");
                  for (auto v1 : borders)
                      fout << v1 << endl;
                  fout.close();
                  outp.success("Gamemode in world" + world_name + " succefull reset!");
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Lock:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.limit";
              if (world_name == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "")
              {
                  ifstream in("plugins/World Protect/lock.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/lock.txt");
                      fout << world_name << endl;
                      fout.close();
                      outp.success(world_name + " world succefull locked for building/destroyed!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/lock.txt",ios_base::app);
                      fout << world_name << endl;
                      fout.close();
                      outp.success(world_name + " world succefull locked for building/destroyed!");
                      return;
                  }
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/lock.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/lock.txt");
                      fout << world_name << endl;
                      fout.close();
                      outp.success(world_name + " world succefull locked for building/destroyed!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/lock.txt", ios_base::app);
                      fout << world_name << endl;
                      fout.close();
                      outp.success(world_name + " world succefull locked for building/destroyed!");
                      return;
                  }
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Max:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.limit";
              if (world_name == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "")
              {
                  ifstream in("plugins/World Protect/limit.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/limit.txt");
                      fout << world_name << ":" << max_xal << endl;
                      fout.close();
                      outp.success("In world " + world_name + " player limit is set to " + to_string(max_xal) + "!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/limit.txt", ios_base::app);
                      fout << world_name << ":" << max_xal << endl;
                      fout.close();
                      outp.success("In world " + world_name + " player limit is set to " + to_string(max_xal) + "!");
                      return;
                  }
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/limit.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/limit.txt");
                      fout << world_name << ":" << max_xal << endl;
                      fout.close();
                      outp.success("In world " + world_name + " player limit is set to " + to_string(max_xal) + "!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/limit.txt", ios_base::app);
                      fout << world_name << ":" << max_xal << endl;
                      fout.close();
                      outp.success("In world " + world_name + " player limit is set to " + to_string(max_xal) + "!");
                      return;
                  }
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Motd:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.wpmotd";
              if (motd == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && motd != "")
              {
                  ifstream in("plugins/World Protect/motd.txt");
                  if (in.is_open())
                  {
                      in.close();
                      ofstream out("plugins/World Protect/motd.txt");
                      out << motd << endl;
                      out.close();
                      LL::setServerMotd(motd);
                      outp.success("Motd set successfully!");
                      return;
                  }
                  else if (!in.is_open())
                  {
                      in.close();
                      ofstream out("plugins/World Protect/motd.txt");
                      out << motd << endl;
                      out.close();
                      LL::setServerMotd(motd);
                      outp.success("Motd set successfully!");
                      return;
                  }
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/motd.txt");
                  if (in.is_open())
                  {
                      in.close();
                      ofstream out("plugins/World Protect/motd.txt");
                      out << motd << endl;
                      out.close();
                      LL::setServerMotd(motd);
                      outp.success("Motd set successfully!");
                      return;
                  }
                  else if (!in.is_open())
                  {
                      in.close();
                      ofstream out("plugins/World Protect/motd.txt");
                      out << motd << endl;
                      out.close();
                      LL::setServerMotd(motd);
                      outp.success("Motd set successfully!");
                      return;
                  }
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Noexplode:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.noexplode";
              if (world_name == "" || opval1 == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && opval1 != "")
              {
                  ifstream in("plugins/World Protect/noexplode.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/noexplode.txt");
                      fout << world_name << ":" << opval1 << endl;
                      fout.close();
                      outp.success("In world " + world_name + " noexplode set to " + opval1 + "!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/noexplode.txt", ios_base::app);
                      fout << world_name << ":" << opval1 << endl;
                      fout.close();
                      outp.success("In world " + world_name + " noexplode set to " + opval1 + "!");
                      return;
                  }
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/noexplode.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/noexplode.txt");
                      fout << world_name << ":" << opval1 << endl;
                      fout.close();
                      outp.success("In world " + world_name + " noexplode set to " + opval1 + "!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/noexplode.txt", ios_base::app);
                      fout << world_name << ":" << opval1 << endl;
                      fout.close();
                      outp.success("In world " + world_name + " noexplode set to " + opval1 + "!");
                      return;
                  }
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Protect:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.protect";
              if (world_name == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "")
              {
                  ifstream in("plugins/World Protect/protect.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/protect.txt");
                      fout << world_name  << endl;
                      fout.close();
                      outp.success("World " + world_name + " succefull protected!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/protect.txt", ios_base::app);
                      fout << world_name << endl;
                      fout.close();
                      outp.success("World " + world_name + " succefull protected!");
                      return;
                  }
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/protect.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/protect.txt");
                      fout << world_name << endl;
                      fout.close();
                      outp.success("World " + world_name + " succefull protected!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/protect.txt", ios_base::app);
                      fout << world_name << endl;
                      fout.close();
                      outp.success("World " + world_name + " succefull protected!");
                      return;
                  }
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Pvp:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.pvp";
              if (world_name == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "")
              {
                  ifstream in("plugins/World Protect/pvp.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/pvp.txt");
                      fout << world_name << endl;
                      fout.close();
                      outp.success("In world " + world_name + "pvp succefull off!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/pvp.txt", ios_base::app);
                      fout << world_name << endl;
                      fout.close();
                      outp.success("In world " + world_name + "pvp succefull off!");
                      return;
                  }
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/pvp.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/pvp.txt");
                      fout << world_name << endl;
                      fout.close();
                      outp.success("In world " + world_name + "pvp succefull off!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/pvp.txt", ios_base::app);
                      fout << world_name << endl;
                      fout.close();
                      outp.success("In world " + world_name + "pvp succefull off!");
                      return;
                  }
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Rm:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.addrm";
              if (world_name == "" || player.getName() == "")
              {
                  outp.error(msg1);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && player.getName() != "")
              {
                  ifstream in("plugins/World Protect/" + world_name + "_auth.txt");
                  vector<string> players;
                  string str;
                  while (getline(in, str))
                  {
                      if (str != player.getName())
                          players.push_back(str);
                  }
                  in.close();
                  string fn = "plugins/World Protect/" + world_name + "_auth.txt";
                  remove(fn.c_str());
                  ofstream fout(fn);
                  for (auto v1 : players)
                      fout << v1 << endl;
                  fout.close();
                  outp.success("Player " + player.getName() + " removed from " + world_name + "auth list!");
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/" + world_name + "_auth.txt");
                  vector<string> players;
                  string str;
                  while (getline(in, str))
                  {
                      if (str != player.getName())
                          players.push_back(str);
                  }
                  in.close();
                  string fn = "plugins/World Protect/" + world_name + "_auth.txt";
                  remove(fn.c_str());
                  ofstream fout(fn);
                  for (auto v1 : players)
                      fout << v1 << endl;
                  fout.close();
                  outp.success("Player " + player.getName() + " removed from " + world_name + "auth list!");
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Breakable:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.unbreakable";
              if (world_name == "" || block_id == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && block_id != "")
              {
                  ifstream in("plugins/World Protect/" + world_name + "_blocks.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_blocks.txt");
                      fout << block_id << endl;
                      fout.close();
                      outp.success("Block " + block_id + " succefull baned in world " + world_name + "!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_blocks.txt", ios_base::app);
                      fout << block_id << endl;
                      fout.close();
                      outp.success("Block " + block_id + " succefull baned in world " + world_name + "!");
                      return;
                  }
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/" + world_name + "_blocks.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_blocks.txt");
                      fout << block_id << endl;
                      fout.close();
                      outp.success("Block " + block_id + " succefull baned in world " + world_name + "!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_blocks.txt", ios_base::app);
                      fout << block_id << endl;
                      fout.close();
                      outp.success("Block " + block_id + " succefull baned in world " + world_name + "!");
                      return;
                  }
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Unbreakable:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.unbreakable";
              if (world_name == "" || block_id == "")
              {
                  outp.error("Invalid argument command!");
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "" && block_id != "")
              {
                  ifstream in("plugins/World Protect/" + world_name + "_blocks.txt");
                  vector<string> blocks;
                  string str;
                  while (getline(in, str))
                  {
                      if (str != block_id)
                          blocks.push_back(str);
                  }
                  in.close();
                  string fn = "plugins/World Protect/" + world_name + "_blocks.txt";
                  remove(fn.c_str());
                  ofstream fout(fn);
                  for (auto v1 : blocks)
                      fout << v1 << endl;
                  fout.close();
                  outp.success("Block " + block_id + " succefull unbaned in world " + world_name + "!");
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/" + world_name + "_blocks.txt");
                  if (!in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_blocks.txt");
                      fout << block_id << endl;
                      fout.close();
                      outp.success("Block " + block_id + " succefull baned in world " + world_name + "!");
                      return;
                  }
                  else if (in.is_open())
                  {
                      in.close();
                      ofstream fout("plugins/World Protect/" + world_name + "_blocks.txt", ios_base::app);
                      fout << block_id << endl;
                      fout.close();
                      outp.success("Block " + block_id + " succefull baned in world " + world_name + "!");
                      return;
                  }
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
          case Operation::Unlock:
          {
              auto id = ori.getPlayer()->getDimensionId();
              string dim;
              if (id == 0)
                  dim = "OverWorld";
              else if (id == 1)
                  dim = "Nether";
              else if (id == 2)
                  dim = "End";
              string nick = ori.getPlayer()->getName();
              string msg = get_msg("invalidArgument"), msg1 = get_msg("permissionDenied");
              string perm = "wp.cmd.limit";
              if (world_name == "")
              {
                  outp.error(msg);
                  return;
              }
              if (ori.getPermissionsLevel() == CommandPermissionLevel::Console && world_name != "")
              {
                  ifstream in("plugins/World Protect/lock.txt");
                  vector<string> worlds;
                  string str;
                  while (getline(in, str))
                  {
                      if (str != world_name)
                          worlds.push_back(str);
                  }
                  in.close();
                  string fn = "plugins/World Protect/lock.txt";
                  remove(fn.c_str());
                  ofstream fout(fn);
                  for (auto v1 : worlds)
                      fout << v1 << endl;
                  fout.close();
                  outp.success(world_name + " world succefull unlocked for building/destroyed!");
                  return;
              }
              if ((checkPerm(nick, perm) || checkPerm(nick, "plugins.*") || checkPerm(nick, "wp.*") || checkPermWorlds(nick, perm, dim) || checkPermWorlds(nick, "plugins.*", dim) || checkPermWorlds(nick, "wp.*", dim)))
              {
                  ifstream in("plugins/World Protect/lock.txt");
                  vector<string> worlds;
                  string str;
                  while (getline(in, str))
                  {
                      if (str != world_name)
                          worlds.push_back(str);
                  }
                  in.close();
                  string fn = "plugins/World Protect/lock.txt";
                  remove(fn.c_str());
                  ofstream fout(fn);
                  for (auto v1 : worlds)
                      fout << v1 << endl;
                  fout.close();
                  outp.success(world_name + " world succefull unlocked for building/destroyed!");
                  return;
              }
              else
              {
                  outp.error(msg1);
                  return;
              }
          }
        }
    }
    static void setup(CommandRegistry* r)
    {
        using RegisterCommandHelper::makeMandatory;
        using RegisterCommandHelper::makeOptional;
        r->registerCommand("wp", "WorldProtect commands.", CommandPermissionLevel::Any, { (CommandFlagValue)0 }, { (CommandFlagValue)0x80 });
        r->addEnum<Operation>("border", { { "border",Operation::Border } });
        r->addEnum<Operation>("_add", { {"_add",Operation::Add} });
        r->addEnum<Operation>("bancmd", { {"bancmd",Operation::Bancmd} });
        r->addEnum<Operation>("unbancmd", { {"unbancmd", Operation::Unbancmd } });
        r->addEnum<Operation>("banitem", { {"banitem",Operation::Banitem } });
        r->addEnum<Operation>("unbanitem", { { "unbanitem", Operation::Unbanitem } });
        r->addEnum<Operation>("gm", { {"gm", Operation::Gm} });
        r->addEnum<Operation>("lock", { { "lock", Operation::Lock } });
        r->addEnum<Operation>("max", { { "max", Operation::Max } });
        r->addEnum<Operation>("motd", { {"motd", Operation::Motd} });
        r->addEnum<Operation>("noexplode", { {"noexplode", Operation::Noexplode} });
        r->addEnum<Operation>("protect", { {"protect", Operation::Protect} });
        r->addEnum<Operation>("pvp", { {"pvp", Operation::Pvp} });
        r->addEnum<Operation>("rm", { {"rm", Operation::Rm} });
        r->addEnum<Operation>("breakable", { {"breakable", Operation::Breakable} });
        r->addEnum<Operation>("unbreakable", { { "unbreakable", Operation::Unbreakable } });
        r->addEnum<Operation>("unlock", { {"unlock", Operation::Unlock} });
        r->addSoftEnum("Border_Op", { "none" });
        r->addSoftEnum("Gm_Op", { "none" });
        r->addSoftEnum("Noexplode_Op", { "off","world" });
        r->addSoftEnum("Pvp_Op", { "off","on" });
        r->registerOverload<Wp>("wp",makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "bancmd", "bancmd").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"), makeOptional(&Wp::cmd, "cmd"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "border", "border").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"), makeOptional(&Wp::x1, "x1"), makeOptional(&Wp::z1, "z1"), makeOptional(&Wp::x2, "x2"), makeOptional(&Wp::z2, "z2"));
       r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "unbancmd", "unbancmd").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"), makeOptional(&Wp::cmd, "cmd"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "banitem", "banitem").addOptions((CommandParameterOption)1),makeMandatory(&Wp::world_name, "world"), makeOptional(&Wp::item, "item-name"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "unbanitem", "unbanitem").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"),makeOptional(&Wp::item, "item-name"));
       r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "gm", "gm").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"), makeOptional(&Wp::gm, "mode"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "gm", "gm").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"), makeOptional<CommandParameterDataType::SOFT_ENUM>(&Wp::opval1, "Gm_Op"));
        r->registerOverload<Wp>("wp", makeOptional<CommandParameterDataType::ENUM>(&Wp::op, "lock", "lock").addOptions((CommandParameterOption)1),makeMandatory(&Wp::world_name, "world"));
        r->registerOverload<Wp>("wp", makeOptional<CommandParameterDataType::ENUM>(&Wp::op, "unlock", "unlock").addOptions((CommandParameterOption)1),makeMandatory(&Wp::world_name, "world"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "_add", "_add").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"), makeOptional(&Wp::player, "player"));
        r->registerOverload<Wp>("wp", makeOptional<CommandParameterDataType::ENUM>(&Wp::op, "max", "max").addOptions((CommandParameterOption)1),makeMandatory(&Wp::world_name, "world"));
        r->registerOverload<Wp>("wp", makeOptional<CommandParameterDataType::ENUM>(&Wp::op, "max", "max").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"), makeOptional(&Wp::max_xal, "value"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "motd", "motd").addOptions((CommandParameterOption)1), makeOptional(&Wp::motd, "text"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "noexplode", "noexplode").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"),makeOptional<CommandParameterDataType::SOFT_ENUM>(&Wp::opval1, "Noexplode_Op"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "protect", "protect").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "pvp", "pvp").addOptions((CommandParameterOption)1), makeOptional<CommandParameterDataType::SOFT_ENUM>(&Wp::opval1, "Pvp_Op"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "rm", "rm").addOptions((CommandParameterOption)1), makeOptional(&Wp::player, "player"), makeMandatory(&Wp::world_name, "world"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "breakable", "breakable").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"),makeOptional(&Wp::block_id, "block-name"));
        r->registerOverload<Wp>("wp", makeMandatory<CommandParameterDataType::ENUM>(&Wp::op, "unbreakable", "unbreakable").addOptions((CommandParameterOption)1), makeMandatory(&Wp::world_name, "world"), makeOptional(&Wp::block_id, "block-name"));
    }
};


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        LL::registerPlugin("WorldProtect", "Порт WorldProtect из PMMP",LL::Version(1, 0, 1, LL::Version::Release),"https://github.com/MCBEPluginModer/World-Protect");
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void entry();

extern "C" {
    _declspec(dllexport) void onPostInit() {
        std::ios::sync_with_stdio(false);
        entry();
    }
}

enum GameType;

THook(bool, "?changeDimension@ServerPlayer@@UEAAXV?$AutomaticID@VDimension@@H@@_N@Z", ServerPlayer* _this, AutomaticID<Dimension, int> di, bool b)
{
    auto dimId = di;
    string dim;
    if (dimId == 0)
        dim = "OverWorld";
    else if (dimId == 1)
        dim = "Nether";
    else if (dimId == 2)
        dim = "End";
    map<string, int> dims;
    map<string, int> gm;
    dims.insert(pair<string, int>("OverWorld", 0));
    dims.insert(pair<string, int>("Nether", 1));
    dims.insert(pair<string, int>("End", 2));
    vector<string> auth_list;
    ifstream in3("plugins/World Protect/gm.txt");
    string ss;
    while (getline(in3, ss))
    {
        auto vec = split(ss, ":");
        gm.insert(pair<string, int>(vec[0], atoi(vec[1].c_str())));
    }
    in3.close();
    int gamemode = -1;
    gamemode = gm[dim];
    using namespace GameTypeConv;
    if (gamemode != -1)
    {
        ::GameType gmm = (::GameType)GameTypeConv::intToGameType(gamemode);
        _this->setPlayerGameType(gmm);
    }
    string str;
    string str1;
    ifstream in1("plugins/World Protect/limit.txt");
    while (getline(in1, str1))
    {
        auto vec = split(str1, ":");
        if (dim == vec[0])
        {
            in1.close();
            long cnt = atoi(vec[1].c_str());
            auto players = Level::getAllPlayers();
            int count = 0;
            for (auto v : players)
            {
                if (v->getDimensionId() == dimId)
                    count++;
            }
            if (count >= cnt)
            {
                _this->sendText("[World Protect]: This is dimension is full!", TextType::RAW);
                return false; 
            }
        }
    }
    in1.close();
    return original(_this, di, b);
}

bool servRun = false;

bool makeDir(const char* dir)
{
    error_code ec;
    if (exists(dir))
        return is_directory(status(dir));
    else
        return create_directories(dir, ec);
}

void entry()
{
    makeDir("plugins/World Protect");
    Event::ServerStartedEvent::subscribe([](const Event::ServerStartedEvent& ev)
        {
            ifstream in("plugins/World Protect/motd.txt");
            string str;
            in >> str;
            in.close();
            if (str == "")
                return 1;
            LL::setServerMotd(str);
            return 1;
        });
    Event::RegCmdEvent::subscribe([](const Event::RegCmdEvent& ev)
        {
            try
            {
                MotdCmd::setup(ev.mCommandRegistry);
                Wp::setup(ev.mCommandRegistry);
            }
            catch (std::exception s) {
                std::cerr << s.what() << std::endl;
            }
            return true;
        });
    Event::PlayerMoveEvent::subscribe([](const Event::PlayerMoveEvent& ev) 
    {
            int x1, x2, z1, z2;
            auto dimId = ev.mPlayer->getDimension().getDimensionId();
            ifstream in("plugins/World Protect/borders.txt");
            string str;
            vector<string> data;
            while (getline(in, str))
            {
                auto vec = split(str, " ");
                if (vec[0] == "OverWorld" && dimId == 0)
                {
                    data = split(vec[1], ":");
                    x1 = atoi(data[0].c_str());
                    z1 = atoi(data[1].c_str());
                    x2 = atoi(data[2].c_str());
                    z2 = atoi(data[3].c_str());
                    double distance1 = sqrt(pow(ev.mPos.x - x1, 2) + pow(ev.mPos.z - z1, 2));
                    double distance2 = sqrt(pow(ev.mPos.x - x2, 2) + pow(ev.mPos.z - z2, 2));
                    if ((ev.mPos.x <= x1 || ev.mPos.z <= z1) || (ev.mPos.x >= x2 || ev.mPos.z >= z2))
                    {
                        ev.mPlayer->teleport(Vec3((float)x1 + 4, ev.mPos.y, (float)z1 + 4), 0);
                        ev.mPlayer->sendText("[World Protectd]: You overwhelmed the barrier!", TextType::RAW);
                        return false;
                    }
                    
                }
                else if (vec[0] == "Nether" && dimId == 1)
                {
                    data = split(vec[1], ":");
                    x1 = atoi(data[0].c_str());
                    z1 = atoi(data[1].c_str());
                    x2 = atoi(data[2].c_str());
                    z2 = atoi(data[3].c_str());
                    double distance1 = sqrt(pow(ev.mPos.x - x1, 2) + pow(ev.mPos.z - z1, 2));
                    double distance2 = sqrt(pow(ev.mPos.x - x2, 2) + pow(ev.mPos.z - z2, 2));
                    if ((ev.mPos.x <= x1 || ev.mPos.z <= z1) || (ev.mPos.x >= x2 || ev.mPos.z >= z2))
                    {
                        ev.mPlayer->teleport(Vec3((float)x1 + 4, ev.mPos.y, (float)z1 + 4), 0);
                            ev.mPlayer->sendText("[World Protectd]: You overwhelmed the barrier!", TextType::RAW);
                            return false;
                    }
                }
                else  if (vec[0] == "End" && dimId == 2)
                {
                    data = split(vec[1], ":");
                    x1 = atoi(data[0].c_str());
                    z1 = atoi(data[1].c_str());
                    x2 = atoi(data[2].c_str());
                    z2 = atoi(data[3].c_str());
                    double distance1 = sqrt(pow(ev.mPos.x - x1, 2) + pow(ev.mPos.z - z1, 2));
                    double distance2 = sqrt(pow(ev.mPos.x - x2, 2) + pow(ev.mPos.z - z2, 2));
                    if ((ev.mPos.x <= x1 || ev.mPos.z <= z1) || (ev.mPos.x >= x2 || ev.mPos.z >= z2))
                    {
                        ev.mPlayer->teleport(Vec3((float)x1 + 4, ev.mPos.y, (float)z1 + 4), 0);
                        ev.mPlayer->sendText("[World Protectd]: You overwhelmed the barrier!", TextType::RAW);
                        return false;
                    }
                }
            }
            return true;
    });
    Event::PlayerCmdEvent::subscribe([](const  Event::PlayerCmdEvent& ev) 
    {
            auto dimId = ev.mPlayer->getDimension().getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            ifstream in("plugins/World Protect/" + dim + "_cmd.txt");
            string str;
            while (getline(in, str))
            {
                regex r(str);
                smatch m;
                if (regex_search(str, m, r))
                {
                    ev.mPlayer->sendText("[World Protectd]: This is command banned in this world!", TextType::RAW);
                    return false;
                }
            }
            return true;
    });
    Event::PlayerInventoryChangeEvent::subscribe([](const Event::PlayerInventoryChangeEvent& ev) 
    {
            if (ev.mPlayer == nullptr || ev.mNewItemStack == nullptr || ev.mPreviousItemStack == nullptr)
            {
                return 1;
            }
            auto dimId = ev.mPlayer->getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            ifstream in("plugins/World Protect/" + dim + "_items.txt");
            string str;
            while (getline(in, str))
            {
                    if (("minecraft:" + str) == ev.mNewItemStack->getTypeName())
                    {
                        ev.mPlayer->clearItem("minecraft:" + str);
                        ev.mPlayer->sendText("[World Protectd]: This is item banned in this world!", TextType::RAW);
                        return 0;
                    }
            }
            in.close();
            return 1;
    });
    Event::PlayerPickupItemEvent::subscribe([](const Event::PlayerPickupItemEvent& ev)
        {
            if (ev.mPlayer == nullptr)
            {
                return 1;
            }
            auto dimId = ev.mPlayer->getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            ifstream in("plugins/World Protect/" + dim + "_items.txt");
            string str;
            while (getline(in, str))
            {
                if (("minecraft:" + str) == ev.mItemStack->getTypeName())
                {
                    ev.mPlayer->clearItem("minecraft:" + str);
                    ev.mPlayer->sendText("[World Protectd]: This is item banned in this world!", TextType::RAW);
                    return 0;
                }
            }
            in.close();
            return 1;
        });
    Event::PlayerDropItemEvent::subscribe([](const Event::PlayerDropItemEvent& ev)
        {
            if (ev.mPlayer == nullptr)
            {
                return 1;
            }
            auto dimId = ev.mPlayer->getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            ifstream in("plugins/World Protect/" + dim + "_items.txt");
            string str;
            while (getline(in, str))
            {
                if (("minecraft:" + str) == ev.mItemStack->getTypeName())
                {
                    ev.mPlayer->clearItem("minecraft:" + str);
                    return 0;
                }
            }
            in.close();
            return 1;
        });
    Event::PlayerPlaceBlockEvent::subscribe([](const  Event::PlayerPlaceBlockEvent& ev) 
    {
            Users      users;
            auto nodes = YAML::LoadFile("plugins/Permissions Ex/users.yml");
            for (const auto& p : nodes["users"])
            {
                users.users.push_back(p.as<_User>());
            }
            BlockInstance bl = ev.mBlockInstance;
            auto blockName = bl.getBlock()->getTypeName();
            auto dimId = ev.mPlayer->getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            string str;
            ifstream in("plugins/World Protect/lock.txt");
            vector<string> lock_worlds,blocks,auth_players;
            while (getline(in, str))
                lock_worlds.push_back(str);
            in.close();
            ifstream in1("plugins/World Protect/" + dim + "_blocks.txt");
            while (getline(in1, str))
                blocks.push_back(str);
            in1.close();
            ifstream in2("plugins/World Protect/" + dim + "_auth.txt");
            while (getline(in2, str))
                auth_players.push_back(str);
            in2.close();
            auto plain = ev.mPlayer->getName();
            using namespace std;
            auto nick = split(plain, " ");
            string res_nick;
            for (auto n : nick)
            {
                for (auto v : users.users)
                {
                    if (n == v.nickname)
                    {
                        res_nick = n;
                        break;
                    }
                }
            }
            string perm = "wp.cmd.protect.auth";
            if ((checkPerm(res_nick, perm) || checkPerm(res_nick, "plugins.*") || checkPerm(res_nick, "wp.*") || checkPermWorlds(res_nick, perm, dim) || checkPermWorlds(res_nick, "plugins.*", dim) || checkPermWorlds(res_nick, "wp.*", dim)))
            {
                return 1;
            }
            else
            {
                ev.mPlayer->sendText("[World Protectd]: This is world locked for building!", TextType::RAW);
                return 0;
            }
            for (auto v : blocks)
            {
                if (blockName == ("minecraft:" + v) && (checkPerm(res_nick, perm) || checkPerm(res_nick, "plugins.*") || checkPerm(res_nick, "wp.*") || checkPermWorlds(res_nick, perm, dim) || checkPermWorlds(res_nick, "plugins.*", dim) || checkPermWorlds(res_nick, "wp.*", dim)) != true)
                {
                    ev.mPlayer->sendText("[World Protectd]: This is block not breakable for building!", TextType::RAW);
                    return 0;
                }
            }
            return 1;
    });
    Event::PlayerDestroyBlockEvent::subscribe([](const Event::PlayerDestroyBlockEvent& ev) 
    {
            Users      users;
            auto nodes = YAML::LoadFile("plugins/PurePerms/users.yml");
            for (const auto& p : nodes["users"])
            {
                users.users.push_back(p.as<_User>());
            }
            BlockInstance bl = ev.mBlockInstance;
            auto blockName = bl.getBlock()->getTypeName();
            auto dimId = ev.mPlayer->getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            string str;
            ifstream in("plugins/World Protect/lock.txt");
            vector<string> lock_worlds, blocks, auth_players;
            while (getline(in, str))
                lock_worlds.push_back(str);
            in.close();
            ifstream in1("plugins/World Protect/" + dim + "_blocks.txt");
            while (getline(in1, str))
                blocks.push_back(str);
            in1.close();
            ifstream in2("plugins/World Protect/" + dim + "_auth.txt");
            while (getline(in2, str))
                auth_players.push_back(str);
            in2.close();
            auto plain = ev.mPlayer->getName();
            using namespace std;
            auto nick = split(plain, " ");
            string res_nick;
            for (auto n : nick)
            {
                for (auto v : users.users)
                {
                    if (n == v.nickname)
                    {
                        res_nick = n;
                        break;
                    }
                }
            }
            string perm = "wp.cmd.protect.auth";
            if ((checkPerm(res_nick, perm) || checkPerm(res_nick, "plugins.*") || checkPerm(res_nick, "wp.*") || checkPermWorlds(res_nick, perm, dim) || checkPermWorlds(res_nick, "plugins.*", dim) || checkPermWorlds(res_nick, "wp.*", dim)))
            {
                return 1;
            }
            else
            {
                ev.mPlayer->sendText("[World Protectd]: This is world locked for building!", TextType::RAW);
                return 0;
            }
            for (auto v : blocks)
            {
                if (blockName == ("minecraft:" + v) && (checkPerm(res_nick, perm) || checkPerm(res_nick, "plugins.*") || checkPerm(res_nick, "wp.*") || checkPermWorlds(res_nick, perm, dim) || checkPermWorlds(res_nick, "plugins.*", dim) || checkPermWorlds(res_nick, "wp.*", dim)) != true)
                {
                    ev.mPlayer->sendText("[World Protectd]: This is block not breakable for building!", TextType::RAW);
                    return 0;
                }
            }
            return 1;
    });
    Event::PlayerAttackEvent::subscribe([](const Event::PlayerAttackEvent& ev) 
    {
            auto dimId = ev.mPlayer->getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            string str;
            ifstream in("plugins/World Protect/pvp.txt");
            while (getline(in, str))
            {
                if (dim == str)
                {
                    ev.mPlayer->sendText("[World Protectd]:In this world pvp is off!", TextType::RAW);
                    return 0;
                }
            }
            return 1;
    });
    Event::EntityExplodeEvent::subscribe([](const Event::EntityExplodeEvent& ev) 
    {
            auto dimId = ev.mDimension->getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            string str;
            ifstream in("plugins/World Protect/noexplode.txt");
            while (getline(in, str))
            {
                auto vec = split(str, ":");
                if (dim == vec[0] && vec[1] == "world")
                {
                    return 0;
                }
                else  if (dim == vec[0] && vec[1] == "off")
                {
                    return 1;
                }
            }
            return 1;
    });
    Event::BlockExplodeEvent::subscribe([](const Event::BlockExplodeEvent& ev)
        {
            BlockInstance bl = ev.mBlockInstance;
            auto dimId = bl.getDimensionId();
            string dim;
            if (dimId == 0)
                dim = "OverWorld";
            else if (dimId == 1)
                dim = "Nether";
            else if (dimId == 2)
                dim = "End";
            string str;
            ifstream in("plugins/World Protect/noexplode.txt");
            while (getline(in, str))
            {
                auto vec = split(str, ":");
                if (dim == vec[0] && vec[1] == "world")
                {
                    return 0;
                }
                else  if (dim == vec[0] && vec[1] == "off")
                {
                    return 1;
                }
            }
            return 1;
        });
}



