#include "../inc/client.hh"
#include "../inc/utils.hh"
#include "GameNetworkingSockets/steam/isteamnetworkingutils.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <thread>

extern bool g_bQuit;

void ChatClient::Run(const SteamNetworkingIPAddr &serverAddr) {
  interface_ = SteamNetworkingSockets();

  char Addr[SteamNetworkingIPAddr::k_cchMaxString];
  serverAddr.ToString(Addr, sizeof(Addr), true);

  Printf("Connectiing to chat server at %s", Addr);

  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
             (void *)SteamNetConnectionStatusChangedCallback);
  connection_ = interface_->ConnectByIPAddress(serverAddr, 1, &opt);
  if (connection_ == k_HSteamNetConnection_Invalid)
    FatalError("Failed to create connection");

  while (!g_bQuit) {
    PollIncomingMessages();
    PollConnectionStateChanges();
    PollLocalUserInput();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void ChatClient::PollIncomingMessages() {
  while (!g_bQuit) {
    ISteamNetworkingMessage *IncomingMsg = nullptr;
    int numMsgs =
        interface_->ReceiveMessagesOnConnection(connection_, &IncomingMsg, 1);
    if (numMsgs == 0)
      break;
    if (numMsgs < 0)
      FatalError("Error checking for messages");

    fwrite(IncomingMsg->m_pData, 1, IncomingMsg->m_cbSize, stdout);
    fputc('\n', stdout);

    IncomingMsg->Release();
  }
}

void ChatClient::OnSteamNetConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t *info) {
  assert(info->m_hConn == connection_ ||
         connection_ == k_HSteamNetConnection_Invalid);

  switch (info->m_info.m_eState) {
  case k_ESteamNetworkingConnectionState_None:
    break;

  case k_ESteamNetworkingConnectionState_ClosedByPeer:
  case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
    g_bQuit = true;
    if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
      // Note: we could distinguish between a timeout, a rejected connection,
      // or some other transport problem.
      Printf("We sought the remote host, yet our efforts were met with defeat. "
             " (%s)",
             info->m_info.m_szEndDebug);
    } else if (info->m_info.m_eState ==
               k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
      Printf(
          "Alas, troubles beset us; we have lost contact with the host.  (%s)",
          info->m_info.m_szEndDebug);
    } else {
      // NOTE: We could check the reason code for a normal disconnection
      Printf("The host hath bidden us farewell.  (%s)",
             info->m_info.m_szEndDebug);
    }

    // Clean up the connection.  This is important!
    // The connection is "closed" in the network sense, but
    // it has not been destroyed.  We must close it on our end, too
    // to finish up.  The reason information do not matter in this case,
    // and we cannot linger because it's already closed on the other end,
    // so we just pass 0's.
    interface_->CloseConnection(info->m_hConn, 0, nullptr, false);
    connection_ = k_HSteamNetConnection_Invalid;
    break;
  }

  case k_ESteamNetworkingConnectionState_Connecting:
    // We will get this callback when we start connecting.
    // We can ignore this.
    break;

  case k_ESteamNetworkingConnectionState_Connected:
    Printf("Connected to server OK");
    break;

  default:
    // Silences -Wswitch
    break;
  }
}

void ChatClient::SteamNetConnectionStatusChangedCallback(
    SteamNetConnectionStatusChangedCallback_t *info) {
  callbackInstance->OnSteamNetConnectionStatusChanged(info);
}

void ChatClient::PollConnectionStateChanges() {
  callbackInstance = this;
  interface_->RunCallbacks();
}

ChatClient *ChatClient::callbackInstance = nullptr;

void ChatClient::PollLocalUserInput() {
  std::string cmd;
  while (!g_bQuit && LocalUserInput_GetNext(cmd)) {

    // Check for known commands
    if (strcmp(cmd.c_str(), "/quit") == 0) {
      g_bQuit = true;
      Printf("Disconnecting from chat server");

      // Close the connection gracefully.
      // We use linger mode to ask for any remaining reliable data
      // to be flushed out.  But remember this is an application
      // protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
      interface_->CloseConnection(connection_, 0, "Goodbye", true);
      break;
    }

    // Anything else, just send it to the server and let them parse it
    interface_->SendMessageToConnection(
        connection_, cmd.c_str(), (uint32)cmd.length(),
        k_nSteamNetworkingSend_Reliable, nullptr);
  }
}
