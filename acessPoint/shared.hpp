#ifndef SHARED_HPP
#define SHARED_HPP
 
extern espNowMessage espNowDataRcv;
extern espNowMessage espNowDataSend;

extern serialMessage serialDataRcv;
extern serialMessage serialDataSend;

extern bool serialFlag;
extern bool espnowFlag;

extern esp_now_peer_info_t peerInfo;

#endif