//
//  Socket.cpp
//  Nutshell.Library
//
//  Created by Kirill Korolev on 06/11/2017.
//  Copyright © 2017 Kirill Korolev. All rights reserved.
//

#include "socket.hpp"

#ifdef WIN32
    #include <winsock.h>
    typedef int socklen_t;
    typedef char raw_type;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netinet/in.h>
    typedef void raw_type;
#endif

#include <errno.h>

#ifdef WIN32
static bool initialized = false;
#endif

namespace nutshell { namespace socket {
    
    SocketException::SocketException(const std::string &message, bool sysMessage) throw(): userMessage_(message)
    {
        
        if(sysMessage)
        {
            userMessage_.append(": ");
            userMessage_.append(strerror(errno));
        }
    }
    
    SocketException::~SocketException() throw() {}
    
    const char* SocketException::what() const throw()
    {
        return userMessage_.c_str();
    }
    
    static void fillAddr(const std::string& address, ushort_t port, sockaddr_in& addr)
    {
        memset(&addr, 0, sizeof(sockaddr_in));
        
        addr.sin_family = AF_INET;
        addr.sin_port = htonl(port);
        
        hostent* host;
        
        if((host = gethostbyname(address.c_str())) == NULL)
            throw SocketException("Failed to resolve name (gethostbyname())");
        
        addr.sin_addr.s_addr = *((ulong_t *)host->h_addr_list[0]);
    }
    
}}


namespace nutshell { namespace socket {
    
    Socket::Socket(int type, int protocol) throw(SocketException)
    {
#ifdef WIN32
        if(!initialized)
        {
            WORD wVersionRequested;
            WSADATA wsaData;
            wVersionRequested = MAKEWORD(2, 0);
            
            if (WSAStartup(wVersionRequested, &wsaData) != 0) {
                throw SocketException("Unable to load WinSock DLL");
            }
            
            initialized = true;
        }
#endif
        
        if((descriptor_ = ::socket(PF_INET, type, protocol)))
            throw SocketException("Socket creation failed (socket())", true);
        
    }
    
    Socket::Socket(int descriptor)
    {
        descriptor_ = descriptor;
    }
    
    Socket::~Socket()
    {
#ifdef WIN32
        ::closesocket(descriptor_);
#else
        ::close(descriptor_);
#endif
        descriptor_ = -1;
    }
    
    std::string Socket::getAddress() throw(SocketException)
    {
        sockaddr_in addr;
        uint_t addr_len = sizeof(addr);
        
        if (getsockname(descriptor_, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0) {
            throw SocketException("Fetch of local address failed (getsockname())", true);
        }
        
        return inet_ntoa(addr.sin_addr);
    }
    
    ushort_t Socket::getPort() throw(SocketException)
    {
        sockaddr_in addr;
        uint_t addr_len = sizeof(addr);
        
        if (getsockname(descriptor_, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0) {
            throw SocketException("Fetch of local port failed (getsockname())", true);
        }
        return ntohs(addr.sin_port);
    }
    
    void Socket::setPort(ushort_t localPort) throw(SocketException)
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = localPort;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if(bind(descriptor_, (sockaddr *)&addr, sizeof(sockaddr_in)) < 0)
            throw SocketException("Set of local port failed (bind())", true);
    }
    
    
    void Socket::set(const std::string& localAddress, ushort_t localPort) throw(SocketException)
    {
        sockaddr_in addr;
        fillAddr(localAddress, localPort, addr);
        
        if (bind(descriptor_, (sockaddr *) &addr, sizeof(sockaddr_in)) < 0)
            throw SocketException("Set of local address and port failed (bind())", true);
    }
    
    void Socket::clean() throw(SocketException)
    {
#ifdef WIN32
        if(WSACleanup() != 0) throw SocketException("WSACleanup() failed");
#endif
    }
    
    ushort_t Socket::resolveService(const std::string& service, const std::string& protocol)
    {
        struct servent* serv;
        
        if((serv = getservbyname(service.c_str(), protocol.c_str())) == NULL)
            return atoi(service.c_str());
        else
            return ntohs(serv->s_port);
        
    }
    
}}

namespace nutshell { namespace socket {
    
    CSRSocket::CSRSocket(int type, int protocol) throw(SocketException): Socket(type, protocol){}
    
    CSRSocket::CSRSocket(int descriptor): Socket(descriptor){}
    
    void CSRSocket::connect(const std::string& address, ushort_t port) throw(SocketException)
    {
        sockaddr_in addr;
        fillAddr(address, port, addr);
        
        if(::connect(descriptor_, (sockaddr *)&addr, sizeof(addr)) < 0)
            throw new SocketException("Connect failed (connect())", true);
    }
    
    void CSRSocket::send(const void* buffer, int length) throw(SocketException)
    {
        if(::send(descriptor_, buffer, length, 0) < 0)
            throw SocketException("Send failed (send())", true);
    }
    
    int CSRSocket::recv(void* buffer, int length) throw(SocketException)
    {
        int r;
        if((r = ::recv(descriptor_, buffer, length, 0)) < 0)
            throw SocketException("Send failed (recv())", true);
        return r;
    }
    
    std::string CSRSocket::getConnectionAddress() throw(SocketException)
    {
        sockaddr_in addr;
        unsigned int length = sizeof(addr);
        
        if (getpeername(descriptor_, (sockaddr *) &addr,(socklen_t *) &length) < 0) {
            throw SocketException("Fetch of foreign address failed (getpeername())", true);
        }
        return inet_ntoa(addr.sin_addr);
    }
    
    ushort_t CSRSocket::getConnectionPort() throw(SocketException)
    {
        sockaddr_in addr;
        unsigned int length = sizeof(addr);
        
        if (getpeername(descriptor_, (sockaddr *) &addr, (socklen_t *) &length) < 0) {
            throw SocketException("Fetch of foreign port failed (getpeername())", true);
        }
        return ntohs(addr.sin_port);
    }
    
}}

namespace nutshell { namespace socket {
    
    TCPSocket::TCPSocket() throw(SocketException): CSRSocket(SOCK_STREAM, IPPROTO_IP){}
    
    TCPSocket::TCPSocket(const std::string& address, ushort_t port) throw(SocketException) : CSRSocket(SOCK_STREAM, IPPROTO_IP)
    {
        connect(address, port);
    }
    
    TCPSocket::TCPSocket(int descriptor): CSRSocket(descriptor){}
    
}}


namespace nutshell { namespace socket {
    
    TCPServerSocket::TCPServerSocket(ushort_t port, int queueLength): Socket(SOCK_STREAM, IPPROTO_IP)
    {
        setPort(port);
        setListen(queueLength);
    }
    
    TCPServerSocket::TCPServerSocket(const std::string& address, ushort_t port, int queueLength): Socket(SOCK_STREAM, IPPROTO_IP)
    {
        set(address, port);
        setListen(queueLength);
    }
    
    TCPSocket* TCPServerSocket::accept() throw(SocketException)
    {
        int descriptor;
        
        if((descriptor = ::accept(descriptor_, NULL, 0)) < 0)
            throw SocketException("Accept failed (accept())", true);
        
        return new TCPSocket(descriptor);
    }
    
    void TCPServerSocket::setListen(int queueLength) throw(SocketException)
    {
        if(listen(descriptor_, queueLength) < 0)
            throw SocketException("Set listening socket failed (listen())", true);
    }
    
    
}}

namespace nutshell { namespace socket {
    
    UDPSocket::UDPSocket() throw(SocketException): CSRSocket(SOCK_DGRAM, IPPROTO_UDP)
    {
        setBroadcast();
    }
    
    UDPSocket::UDPSocket(ushort_t port) throw(SocketException): CSRSocket(SOCK_DGRAM, IPPROTO_UDP)
    {
        setPort(port);
        setBroadcast();
    }
    
    UDPSocket::UDPSocket(const std::string& address, ushort_t port) throw(SocketException): CSRSocket(SOCK_DGRAM, IPPROTO_UDP)
    {
        set(address, port);
        setBroadcast();
    }
    
    void UDPSocket::disconnect() throw(SocketException)
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        addr.sin_family = AF_UNSPEC;
        
        if (::connect(descriptor_, (sockaddr *) &addr, sizeof(addr)) < 0)
        {
#ifdef WIN32
            if(errno != WSAEAFNOSUPPORT){
#else
            if(errno != EAFNOSUPPORT){
#endif
                throw SocketException("Disconnect failed (connect())", true);
            }
        }
        
    }
    
    void UDPSocket::sendTo(const void* buffer, int length, const std::string& address, ushort_t port) throw(SocketException)
        {
            sockaddr_in addr;
            fillAddr(address, port, addr);
            
            if(sendto(descriptor_, (raw_type *)buffer, length, 0, (sockaddr *)&addr, sizeof(addr)) != length)
                throw SocketException("Send failed (sendto())", true);
        }
    
    int UDPSocket::recvFrom(void* buffer, int length, const std::string& address, ushort_t port)
    {
        sockaddr addr;
        socklen_t sockLen = sizeof(addr);
        int r;
        
        if((r = recvfrom(descriptor_, (raw_type *)buffer, length, 0, (sockaddr *)&addr, (socklen_t *)&sockLen)) < 0)
            throw SocketException("Receive failed (recvfrom())", true);
        
        return r;
    }
    
    void UDPSocket::setTTL(uchar_t ttl) throw(SocketException)
    {
        if (setsockopt(descriptor_, IPPROTO_IP, IP_MULTICAST_TTL,
                       (raw_type *) &ttl, sizeof(ttl)) < 0) {
            throw SocketException("Multicast TTL set failed (setsockopt())", true);
        }
    }
    
    void UDPSocket::joinGroup(const std::string& group) throw(SocketException)
    {
        struct ip_mreq multicastRequest;
        
        multicastRequest.imr_multiaddr.s_addr = inet_addr(group.c_str());
        multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(descriptor_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       (raw_type *) &multicastRequest,
                       sizeof(multicastRequest)) < 0) {
            throw SocketException("Multicast group join failed (setsockopt())", true);
        }
    }
    
    void UDPSocket::leaveGroup(const std::string& group) throw(SocketException)
    {
        struct ip_mreq multicastRequest;
        
        multicastRequest.imr_multiaddr.s_addr = inet_addr(group.c_str());
        multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(descriptor_, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                       (raw_type *) &multicastRequest,
                       sizeof(multicastRequest)) < 0) {
            throw SocketException("Multicast group leave failed (setsockopt())", true);
        }
    }
    
    void UDPSocket::setBroadcast()
    {
        int permission = 1;
        setsockopt(descriptor_, SOL_SOCKET, SO_BROADCAST, (raw_type *)&permission, sizeof(permission));
    }
    
}}


