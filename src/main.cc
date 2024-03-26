#include "../inc/client.hh"
#include "../inc/server.hh"
#include "../inc/utils.hh"
#include "GameNetworkingSockets/steam/isteamnetworkingutils.h"
#include <cstdio>
#include <cstdlib>

void PrintUsageAndExit(int rc = 1) {
  fflush(stderr);
  printf(
      R"usage(Usage:
    example_chat client SERVER_ADDR
    example_chat server [--port PORT]
)usage");
  fflush(stdout);
  exit(rc);
}

bool g_bQuit = false;

struct ServerConfig {
  bool isServer;
  bool isClient;
  SteamNetworkingIPAddr addrServer;
  int port;

  ServerConfig() : isServer(false), isClient(false), port(1973) {
    addrServer.Clear();
  }
};

ServerConfig ReadConfig(int argsCount, char **argsValue) {
  ServerConfig conf;

  for (int i = 1; i < argsCount; ++i) {
    if (!conf.isClient && !conf.isServer) {
      if (!strcmp(argsValue[i], "client")) {
        conf.isClient = true;
        continue;
      }
      if (!strcmp(argsValue[i], "server")) {
        conf.isServer = true;
        continue;
      }
    }

    if (!strcmp(argsValue[i], "--port")) {
      ++i;
      if (i >= argsCount)
        PrintUsageAndExit();
      conf.port = atoi(argsValue[i]);
      if (conf.port <= 0 || conf.port > 65535)
        FatalError("Invalid port %d", conf.port);
      continue;
    }

    // Anything else, must be server address to connect to
    if (conf.isClient && conf.addrServer.IsIPv6AllZeros()) {
      if (!conf.addrServer.ParseString(argsValue[i]))
        FatalError("Invalid server address '%s'", argsValue[i]);
      if (conf.addrServer.m_port == 0)
        conf.addrServer.m_port = conf.port;
      continue;
    }

    PrintUsageAndExit();
  }

  return conf;
}

int main(int argc, char *argv[]) {
  ServerConfig conf = ReadConfig(argc, argv);

  if (conf.isClient == conf.isServer ||
      (conf.isClient && conf.addrServer.IsIPv6AllZeros()))
    PrintUsageAndExit();

  InitSteamDatagramConnectionSockets();
  LocalUserInput_Init();

  if (conf.isClient) {
    ChatClient client;
    client.Run(conf.addrServer);
  } else {
    ChatServer server;
    server.Run((uint16)conf.port);
  }

  ShutdownSteamDatagramConnectionSockets();
}
