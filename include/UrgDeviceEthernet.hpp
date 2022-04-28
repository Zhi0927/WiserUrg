#ifndef HKY_URGETHERNET_H_
#define HKY_URGETHERNET_H_

//#ifdef _WIN32
//#define _WIN32_WINNT 0X0A00
//#endif
//#define ASIO_STANDALONE
#ifdef BOOST_OS_WINDOWS
#define _WIN32_WINNT 0x0501
#if _WIN32_WINNT <= 0x0501
#define BOOST_ASIO_DISABLE_IOCP
#define BOOST_ASIO_ENABLE_CANCELIO
#endif
#endif

#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <thread>
#include "SCIP_library.hpp"
#include "UrgDevice.hpp"
#include "Common.hpp"

using namespace boost;

class UrgDeviceEthernet
{
public:
	UrgDeviceEthernet(const std::string& ip = "192.168.0.10", const int& port = 10940);
	~UrgDeviceEthernet();

    void StartTCP();
    void Write(const std::string scip);
    void ListenForClients();
    void HandleClientComm(asio::ip::tcp::socket& sock);
    std::string GetCommand(const std::string get_command);
    bool CheckCommand(const std::string& get_command, const std::string& cmd);

    static std::string read_line(asio::ip::tcp::socket& sock);
    static bool write(asio::ip::tcp::socket& sock, const std::string& data);

    void close();


    std::vector<long>               m_distances;
    std::vector<long>               m_strengths;

private:
    std::string                     m_ip_address = "192.168.0.10";
    int                             m_port_number = 10940;

    asio::ip::tcp::socket           m_sock;
    asio::io_context                m_context;
    asio::ip::tcp::endpoint         m_endpoints;
    std::unique_ptr<std::thread>    m_thread;
    std::mutex                      m_guard;

};
#endif
