#include "UrgDeviceEthernet.hpp"


UrgDeviceEthernet::UrgDeviceEthernet(const std::string& ip, const int& port)
    :   m_ip_address(ip),
        m_port_number(port)
{}

UrgDeviceEthernet::~UrgDeviceEthernet()
{
    close();
}

void UrgDeviceEthernet::StartTCP() {

    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0)
    {
        std::cerr << "Can't start Winsock, Err #" << wsResult << std::endl;
        return;
    }

    // Create socket
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock == INVALID_SOCKET)
    {
        std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Fill in a hint structure
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port_number);
    inet_pton(AF_INET, m_ip_address.c_str(), &hint.sin_addr);

    // Connect to server
    int connResult = connect(m_sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR)
    {
        std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << std::endl;
        closesocket(m_sock);
        WSACleanup();
        return;
    }

    ListenForClients();
}

void UrgDeviceEthernet::Write(const std::string scip) {
    write(m_sock, scip);
}

void UrgDeviceEthernet::ListenForClients() {
    m_thread.reset(new std::thread([this]() { HandleClientComm(m_sock); }));
}

void UrgDeviceEthernet::HandleClientComm(SOCKET& sock) {

    try
    {
        while (true) {
            long time_stamp = 0;
            std::string receive_data = read_line(sock);
            //std::cout << receive_data << std::endl;

            std::string cmd = GetCommand(receive_data);

            std::unique_lock<std::mutex> lock(m_guard);

            if (cmd == UrgDevice::GetCMDString(UrgDevice::CMD::MD)) {
                m_distances.clear();
                SCIP_Reader::MD(receive_data, time_stamp, m_distances);
            }
            else if(cmd == UrgDevice::GetCMDString(UrgDevice::CMD::ME))
            {
                m_distances.clear();
                m_strengths.clear();
                SCIP_Reader::ME(receive_data, time_stamp, m_distances, m_strengths);
            }
            else {
                std::cout << ">>" << receive_data << std::endl;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ERROR] HandleClientComm:" << e.what() << std::endl;
    }
}

std::string UrgDeviceEthernet::GetCommand(const std::string get_command) {
    std::vector<std::string> split_command = { SplitString(get_command) };
    return split_command[0].substr(0, 2);
}

bool UrgDeviceEthernet::CheckCommand(const std::string& get_command, const std::string& cmd) {
    std::vector<std::string> split_command = { SplitString(get_command) };
    return startswith(split_command[0], cmd);
}

std::string UrgDeviceEthernet::read_line(SOCKET& sock) {
    std::stringstream ss;
    bool is_NL2 = false;
    bool is_NL = true;

    do
    {
        char buf[1];
        int bytesReceived = recv(sock, buf, 1, 0);
        if (bytesReceived < 1) std::cout << "empty char!" << std::endl;

        if (buf[0] == '\n') {
            if (is_NL)
            {
                is_NL2 = true;
            }
            else
            {
                is_NL = true;
            }
        }
        else
        {
            is_NL = false;
        }
        ss << buf[0];
    } while (!is_NL2);
    return ss.str();
}

bool UrgDeviceEthernet::write(SOCKET& sock, const std::string& data) {
    int sendResult = send(sock, data.c_str(), data.size(), 0);
    if (sendResult != SOCKET_ERROR) {
        //std::cout << data << std::endl;
        return true;
    }
}

void UrgDeviceEthernet::close() {
   
    closesocket(m_sock);
    if (m_thread->joinable()) {
        m_thread->join();
    }
    WSACleanup();
}
