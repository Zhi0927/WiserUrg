#include "EthernetConnector.hpp"


EthernetConnector::EthernetConnector(const std::string& ip, const int& port, std::mutex& lock_guard)
    :   m_ip_address(ip),
        m_port_number(port),
        m_distance_guard(lock_guard)
{}

EthernetConnector::~EthernetConnector(){
    close();
} 

void EthernetConnector::StartMeasureDistance() {    
    write(SCIP_Writer::MD(0, 1080, 1, 0, 0));
}

const bool EthernetConnector::GetConnectState() const {
    return m_isconnected;
}

bool EthernetConnector::StartTCP() {
    std::cout << "Tcp connect..." << std::endl;

    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0){
        std::cerr << "Can't start Winsock, Err #" << wsResult << std::endl;
        return false;
    }

    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock == INVALID_SOCKET){
        std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port_number);
    inet_pton(AF_INET, m_ip_address.c_str(), &hint.sin_addr);

    int error = -1;
    int len = sizeof(int);
    timeval timeout;
    fd_set set;
    unsigned long ul = 1;
    ioctlsocket(m_sock, FIONBIO, &ul);
    bool ret = false;

    int connResult = connect(m_sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR){
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(m_sock, &set);
        
        if (select(m_sock + 1, NULL, &set, NULL, &timeout) > 0) {
            getsockopt(m_sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
            if (error == 0) {
                ret = true;
            }
            else {
                ret = false;
            }
        }
        else {
            ret = false;
        }
    }
    else{
        ret = true;
    }
    
    ul = 0;
    ioctlsocket(m_sock, FIONBIO, &ul); //set as blocking
    if (!ret){
        closesocket(m_sock);
        WSACleanup();
        std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << std::endl;
        return false;
    }


    ListenForClients();
    m_isconnected = true;
    std::cout << "Tcp connect to server successfully! " << std::endl;
    return true;
}

void EthernetConnector::ListenForClients() {
    m_thread.reset(new std::thread([this]() { HandleClientComm(m_sock); }));
}

void EthernetConnector::HandleClientComm(SOCKET& sock) {
    try{
        while (m_isconnected) {
            int error_code = 1;
            std::string receive_data = read_line(sock, error_code);
            if (error_code == -1) {
                m_isconnected = false;
                break;
            }

            long time_stamp = 0;
            std::string cmd = GetCommand(receive_data);

            std::unique_lock<std::mutex> lock(m_distance_guard);

            if (cmd == "MD") {
                recv_distances.clear();
                SCIP_Reader::MD(receive_data, time_stamp, recv_distances);
            }
            else if(cmd == "ME"){
                recv_distances.clear();
                recv_strengths.clear();
                SCIP_Reader::ME(receive_data, time_stamp, recv_distances, recv_strengths);
            }
            else {
                std::cout << ">>" << receive_data << std::endl;
            }
        }
    }
    catch (const std::exception& e){
        m_isconnected = false;
        std::cerr << "[ERROR] HandleClientComm:" << e.what() << std::endl;
    }  
}

std::string EthernetConnector::GetCommand(const std::string get_command) {
    std::vector<std::string> split_command = { SplitString(get_command) };
    return split_command[0].substr(0, 2);
}

bool EthernetConnector::CheckCommand(const std::string& get_command, const std::string& cmd) {
    std::vector<std::string> split_command = { SplitString(get_command) };
    return startswith(split_command[0], cmd);
}

std::string EthernetConnector::read_line(SOCKET& sock, int& error) {
    std::stringstream ss;
    bool is_NL2 = false;
    bool is_NL = true;
    int bytesReceived;

    do{
        char buf[1];
        bytesReceived = recv(sock, buf, 1, 0);

        if (bytesReceived == SOCKET_ERROR || WSAGetLastError() == WSAECONNABORTED) {
            error = -1;
            std::cerr << "Tcp Disconnected!" << std::endl;
            break;
        }

        if (buf[0] == '\n') {
            if (is_NL){
                is_NL2 = true;
            }
            else{
                is_NL = true;
            }
        }
        else{
            is_NL = false;
        }
        ss << buf[0];
    } while (!is_NL2);

    return ss.str();
}

void EthernetConnector::write(const std::string& scip) {
    int sendResult = send(m_sock, scip.c_str(), scip.size(), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "[ERROR] Socket has been disconnected!" << std::endl;
        close();
    }
}

void EthernetConnector::close() {
    m_isconnected = false;
    if (m_thread != nullptr) { 
        if (m_thread->joinable()) {
            m_thread->join();
        }
        m_thread.reset(nullptr);
    }
    closesocket(m_sock);
    WSACleanup();
    std::cout << "Tcp close! " << std::endl;
}