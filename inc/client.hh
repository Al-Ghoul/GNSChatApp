#include "GameNetworkingSockets/steam/steamnetworkingsockets.h"

class ChatClient {
public:
  void Run(const SteamNetworkingIPAddr &serverAddr);

private:
  void PollIncomingMessages();
  void OnSteamNetConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t *Info);
  static void SteamNetConnectionStatusChangedCallback(
      SteamNetConnectionStatusChangedCallback_t *Info);
  void PollConnectionStateChanges();
  void PollLocalUserInput();

private:
  HSteamNetConnection connection_;
  ISteamNetworkingSockets *interface_;
  static ChatClient *callbackInstance;
};
