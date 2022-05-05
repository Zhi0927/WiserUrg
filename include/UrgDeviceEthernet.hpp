#ifndef ZHI_URGETHERNET_H_
#define ZHI_URGETHERNET_H_

#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
 
#include "SCIP_library.hpp"
#include "Common.hpp"


class UrgDeviceEthernet
{
public:
	UrgDeviceEthernet(const std::string& ip = "192.168.0.10", const int& port = 10940);
	~UrgDeviceEthernet();
    void StartMeasureDistance();

    const bool GetConnectState() const;

    bool StartTCP();
    void Write(const std::string scip);
    void ListenForClients();
    void HandleClientComm(SOCKET& sock);
    std::string GetCommand(const std::string get_command);
    bool CheckCommand(const std::string& get_command, const std::string& cmd);

    static std::string read_line(SOCKET& sock, int& error);
    static bool write(SOCKET& sock, const std::string& data);

    void close();


public:
    std::vector<long>               recv_distances;
    std::vector<long>               recv_strengths;
    std::mutex                      distance_guard;

private:
    std::string                     m_ip_address    = "192.168.0.10";
    int                             m_port_number   = 10940;

    SOCKET                          m_sock;
    std::unique_ptr<std::thread>    m_thread;
    bool                            m_isconnected   = false;
};
#endif
