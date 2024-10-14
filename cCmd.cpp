// Einar Árni Bjarnason 2024

//Hérna ætlum við að setja inn commandins HELO;KEEPALIVE;GETMSGS;SENDMSG;STATUSMSG;STATUSREQ;STATUSRESP

#include "cCmd.h"



std::map<std::string, serverInfo> knownServers;
std::map<std::string, std::vector<messageInfo>> messageCache;
time_t keepTime = time(0);