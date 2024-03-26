#include "GameNetworkingSockets/steam/steamnetworkingsockets.h"
#include <string>

void NukeProcess(int rc);
void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,
                 const char *pszMsg);
void FatalError(const char *fmt, ...);
void Printf(const char *fmt, ...);
void InitSteamDatagramConnectionSockets();
void ShutdownSteamDatagramConnectionSockets();
void LocalUserInput_Init();
bool LocalUserInput_GetNext(std::string &result);
