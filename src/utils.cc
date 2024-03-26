#include "GameNetworkingSockets/steam/isteamnetworkingutils.h"
#include "GameNetworkingSockets/steam/steamnetworkingsockets.h"
#include <algorithm>
#include <mutex>
#include <queue>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <thread>

SteamNetworkingMicroseconds g_logTimeZero;

void NukeProcess(int rc) {
#ifdef _WIN32
  ExitProcess(rc);
#else
  (void)rc; // Unused formal parameter
  kill(getpid(), SIGKILL);
#endif
}
void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,
                 const char *pszMsg) {
  SteamNetworkingMicroseconds time =
      SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
  printf("%10.6f %s\n", time * 1e-6, pszMsg);
  fflush(stdout);
  if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
    fflush(stdout);
    fflush(stderr);
    NukeProcess(1);
  }
}

void FatalError(const char *fmt, ...) {
  char text[2048];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  char *nl = strchr(text, '\0') - 1;
  if (nl >= text && *nl == '\n')
    *nl = '\0';
  DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
}

void Printf(const char *fmt, ...) {
  char text[2048];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  char *nl = strchr(text, '\0') - 1;
  if (nl >= text && *nl == '\n')
    *nl = '\0';
  DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Msg, text);
}

void InitSteamDatagramConnectionSockets() {
  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg))
    FatalError("GameNetworkingSockets_Init failed: %s", errMsg);

  g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

  SteamNetworkingUtils()->SetDebugOutputFunction(
      k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

void ShutdownSteamDatagramConnectionSockets() { GameNetworkingSockets_Kill(); }

std::mutex mutexUserInputQueue;
std::queue<std::string> queueUserInput;
std::thread *s_pThreadUserInput = nullptr;
extern bool g_bQuit;

void LocalUserInput_Init() {
  s_pThreadUserInput = new std::thread([]() {
    while (!g_bQuit) {
      char szLine[4000];
      if (!fgets(szLine, sizeof(szLine), stdin)) {
        // Well, you would hope that you could close the handle
        // from the other thread to trigger this.  Nope.
        if (g_bQuit)
          return;
        g_bQuit = true;
        Printf("Failed to read on stdin, quitting\n");
        break;
      }

      mutexUserInputQueue.lock();
      queueUserInput.push(std::string(szLine));
      mutexUserInputQueue.unlock();
    }
  });
}

// You really gotta wonder what kind of pedantic garbage was
// going through the minds of people who designed std::string
// that they decided not to include trim.
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from start (in place)
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// Read the next line of input from stdin, if anything is available.
bool LocalUserInput_GetNext(std::string &result) {
  bool got_input = false;
  mutexUserInputQueue.lock();
  while (!queueUserInput.empty() && !got_input) {
    result = queueUserInput.front();
    queueUserInput.pop();
    ltrim(result);
    rtrim(result);
    got_input = !result.empty(); // ignore blank lines
  }
  mutexUserInputQueue.unlock();
  return got_input;
}
