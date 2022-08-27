#ifndef ZHI_URGETHERNET_H_
#define ZHI_URGETHERNET_H_

#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
 
#include "SCIP_library.h"
#include "Common.hpp"


class EthernetConnector
{
public:
	EthernetConnector(const std::string& ip, const int& port, std::mutex& lock_guard);
	~EthernetConnector();
    void StartMeasureDistance();

    const bool GetConnectState() const;

    bool StartTCP();
    void ListenForClients();
    void HandleClientComm(SOCKET& sock);
    std::string GetCommand(const std::string get_command);
    bool CheckCommand(const std::string& get_command, const std::string& cmd);

    std::string read_line(SOCKET& sock, int& error);
    void write(const std::string& scip);

    void close();


public:
    std::vector<long>               recv_distances;
    std::vector<long>               recv_strengths;

private:
    std::string                     m_ip_address    = "192.168.0.10";
    int                             m_port_number   = 10940;
    std::mutex&                     m_distance_guard;

    SOCKET                          m_sock;
    std::unique_ptr<std::thread>    m_thread;
    bool                            m_isconnected   = false;
};
#endif
