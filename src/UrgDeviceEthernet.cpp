#include "UrgDeviceEthernet.hpp"


UrgDeviceEthernet::UrgDeviceEthernet(const std::string& ip, const int& port)
    :   m_ip_address(ip),
        m_port_number(port),
        m_sock(m_context),
        m_endpoints(asio::ip::address::from_string(ip), port)
{}

UrgDeviceEthernet::~UrgDeviceEthernet()
{
    close();
}

void UrgDeviceEthernet::StartTCP() {

    try
    {
        m_sock.open(m_endpoints.protocol());
        m_sock.close();
        m_sock.connect(m_endpoints);
        std::cout << "Connect setting = IP Address : " << m_ip_address << " Port number : " << std::to_string(m_port_number) << std::endl;
        ListenForClients();
    }
    catch (const system::system_error& e)
    {
        std::cerr << "[ERROR] Error code = " << e.code() << ". Message: " << e.what() << '\n';
    }
}

void UrgDeviceEthernet::Write(const std::string scip) {
    write(m_sock, scip);
}

void UrgDeviceEthernet::ListenForClients() {
    m_thread.reset(new std::thread([this]() { HandleClientComm(m_sock); }));
}

void UrgDeviceEthernet::HandleClientComm(asio::ip::tcp::socket& sock) {

    try
    {
        while (true) {
            long time_stamp = 0;
            std::string receive_data = read_line(sock);

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
            lock.unlock();
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

std::string UrgDeviceEthernet::read_line(asio::ip::tcp::socket& sock) {
    std::stringstream ss;
    bool is_NL2 = false;
    bool is_NL = true;

    do
    {
        char buf[1];
        system::error_code error;
        size_t bytesRead  = asio::read(sock, asio::buffer(buf, 1), error);
        if (bytesRead < 1) std::cout << "empty char!" << std::endl;
        if (error == asio::error::eof) {
            sock.close();
            break;
        }
        else if (error)
        {
            std::cout << "[ERROR]" << system::system_error(error).what() << std::endl;
        }

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

bool UrgDeviceEthernet::write(asio::ip::tcp::socket& sock, const std::string& data) {
    system::error_code error;
    asio::write(sock, asio::buffer(data), error);
    if (!error) {
        std::cout << "Server sent: [" << data << "] sucessfully!" << std::endl;
        return true;
    }
    else {
        std::cout << "[ERROR] send failed: " << error.message() << std::endl;
        return false;
    }
}

void UrgDeviceEthernet::close() {
   
    m_context.post([this]() {
        system::error_code ec;
        m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec) {
            std::cout << "[ERROR] shutdown failed: " << ec.message() << std::endl;
        }

        m_sock.close();
        //m_sock.release();
        });
    m_thread->join();
}
