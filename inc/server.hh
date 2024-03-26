#include "GameNetworkingSockets/steam/steamnetworkingsockets.h"
#include <map>
#include <string>

class ChatServer {
public:
  void Run(uint16 port);
  void PollIncomingMessages();
  void OnSteamNetConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t *pInfo);
  static void SteamNetConnectionStatusChangedCallback(
      SteamNetConnectionStatusChangedCallback_t *pInfo);
  void PollConnectionStateChanges();
  void PollLocalUserInput();
  void SendStringToClient(HSteamNetConnection conn, const char *str);
  void SendStringToAllClients(
      const char *str,
      HSteamNetConnection except = k_HSteamNetConnection_Invalid);
  void SetClientNick(HSteamNetConnection hConn, const char *nick);

private:
  struct Client_t {
    std::string nickName;
  };

private:
  ISteamNetworkingSockets *interface_;
  HSteamNetPollGroup pollGroup_;
  HSteamListenSocket listenSocket_;
  std::map<HSteamNetConnection, Client_t> mapClients_;
};
