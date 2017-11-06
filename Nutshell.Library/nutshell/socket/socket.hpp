//
//  Socket.hpp
//  Nutshell.Library
//
//  Created by Kirill Korolev on 06/11/2017.
//  Copyright © 2017 Kirill Korolev. All rights reserved.
//

#ifndef Socket_hpp
#define Socket_hpp

#include <cstdlib>
#include <exception>
#include <string>

namespace nutshell { namespace socket {

    typedef unsigned short ushort_t;
    typedef unsigned char uchar_t;
    typedef unsigned int uint_t;
    typedef unsigned long ulong_t;
    
    class SocketException: public std::exception
    {
    public:
        SocketException(const std::string& message, bool sysMessage=false) throw();
        ~SocketException() throw();
        const char* what() const throw();
    private:
        std::string userMessage_;
    };


    class Socket
    {
    public:
        ~Socket();
        std::string getAddress() throw(SocketException);
        ushort_t getPort() throw(SocketException);
        
        void setPort(ushort_t localPort) throw(SocketException);
        void set(const std::string& localAddress, ushort_t localPort=0) throw(SocketException);
        
        static void clean() throw(SocketException);
        static ushort_t resolveService(const std::string& service, const std::string& protocol);
        
    protected:
        Socket(int type, int protocol) throw(SocketException);
        Socket(int descriptor);
        
        int descriptor_;
    private:
        Socket(const Socket&);
        Socket& operator=(const Socket&);
    };
    
    class TCPSocket;
    
    class TCPServerSocket: public Socket
    {
    public:
        TCPServerSocket(ushort_t port, int queueLength=DEFAULT_QUEUE_LENGTH);
        TCPServerSocket(const std::string& address, ushort_t port, int queueLength=DEFAULT_QUEUE_LENGTH);
        TCPSocket* accept() throw(SocketException);
    private:
        static const int DEFAULT_QUEUE_LENGTH = 5;
        void setListen(int queueLength) throw(SocketException);
    };
    
    class CSRSocket: public Socket
    {
    public:
        void connect(const std::string& address, ushort_t port) throw(SocketException);
        void send(const void* buffer, int length) throw(SocketException);
        int recv(void* buffer, int length) throw(SocketException);
        
        std::string getConnectionAddress() throw(SocketException);
        ushort_t getConnectionPort() throw(SocketException);
    protected:
        CSRSocket(int type, int protocol) throw(SocketException);
        CSRSocket(int descriptor);
    };
    
    
    class TCPSocket: public CSRSocket
    {
    public:
        TCPSocket() throw(SocketException);
        TCPSocket(const std::string& address, ushort_t port) throw(SocketException);
    private:
        friend class TCPServerSocket;
        TCPSocket(int descriptor);
    };
    
    class UDPSocket: public CSRSocket
    {
    public:
        UDPSocket() throw(SocketException);
        UDPSocket(ushort_t port) throw(SocketException);
        UDPSocket(const std::string& address, ushort_t port) throw(SocketException);
        void disconnect() throw(SocketException);
        void sendTo(const void* buffer, int length, const std::string& address, ushort_t port) throw(SocketException);
        int recvFrom(void* buffer, int length, const std::string& address, ushort_t port);
        void setTTL(uchar_t ttl) throw(SocketException);
        void joinGroup(const std::string& group) throw(SocketException);
        void leaveGroup(const std::string& group) throw(SocketException);
    private:
        void setBroadcast();
    };
    
}}
    
#endif /* Socket_hpp */
