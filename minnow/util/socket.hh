#pragma once

#include "address.hh"
#include "file_descriptor.hh"

#include <functional>
#include <sys/socket.h>

// Socket 类：网络套接字的基类，封装了常见的套接字操作。
// 提供对本地地址、对端地址获取以及绑定、连接、关闭等功能，通常由派生类（如 TCPSocket、UDPSocket）进行扩展。
class Socket : public FileDescriptor
{
private:
  // 获取当前套接字的本地地址或远端地址。
  // name_of_function: 调用函数名，用于出错时的报告。
  // function: 实际调用的系统函数，如 getsockname 或 getpeername。
  Address get_address( const std::string& name_of_function,
                       const std::function<int( int, sockaddr*, socklen_t* )>& function ) const;

protected:
  // 通过 socket(2) 系统调用构造一个套接字。
  // domain: 协议族（例如 AF_INET）。
  // type: 套接字类型（例如 SOCK_STREAM）。
  // protocol: 协议编号（通常为 0，由系统自动选择）。
  Socket( int domain, int type, int protocol = 0 );

  // 利用已有的文件描述符构造套接字对象，用于接受连接后包装描述符。
  Socket( FileDescriptor&& fd, int domain, int type, int protocol = 0 );

  // 封装 getsockopt(2) 系统调用，模板参数 option_type 表示选项值的类型。
  template<typename option_type>
  socklen_t getsockopt( int level, int option, option_type& option_value ) const;

  // 封装 setsockopt(2) 系统调用，设置套接字选项，模板参数 option_type 表示选项值类型。
  template<typename option_type>
  void setsockopt( int level, int option, const option_type& option_value );

  // 重载的 setsockopt 方法，接受 std::string_view 类型的选项值。
  void setsockopt( int level, int option, std::string_view option_val );

public:
  // 绑定套接字到指定地址（使用 bind(2) 系统调用），通常用于服务器端监听。
  void bind( const Address& address );

  // 将套接字绑定到特定网络设备，限定套接字只在指定设备上进行数据收发。
  void bind_to_device( std::string_view device_name );

  // 连接到对端地址（使用 connect(2) 系统调用），通常用于客户端连接服务器。
  void connect( const Address& address );

  // 关闭套接字连接（使用 shutdown(2)），参数 how 指定关闭方式（如只关闭读、写或两者）。
  void shutdown( int how );

  // 使用 getsockname(2) 获取本地地址。
  Address local_address() const;
  
  // 使用 getpeername(2) 获取对端地址。
  Address peer_address() const;

  // 设置 SO_REUSEADDR 选项，允许地址快速重用，通常用于服务器重启时。
  void set_reuseaddr();

  // 检查非阻塞套接字操作中的错误，并抛出异常。
  void throw_if_error() const;
};

// DatagramSocket 类：数据报套接字基类，专用于无连接的数据传输，如 UDP。
// 提供接收、发送数据报的基本接口。
class DatagramSocket : public Socket
{
public:
  // 接收数据报，同时返回发送方的地址和消息数据。
  void recv( Address& source_address, std::string& payload );

  // 发送数据报到指定的目标地址。
  void sendto( const Address& destination, std::string_view payload );

  // 发送数据报到已连接的默认地址（必须先调用 connect()）。
  void send( std::string_view payload );

protected:
  // 利用指定参数构造数据报套接字。
  DatagramSocket( int domain, int type, int protocol = 0 ) : Socket( domain, type, protocol ) {}

  // 通过文件描述符构造数据报套接字，可用于接受已有的数据报连接。
  DatagramSocket( FileDescriptor&& fd, int domain, int type, int protocol = 0 )
    : Socket( std::move( fd ), domain, type, protocol )
  {}
};

// UDPSocket 类：UDP 套接字的封装，专门用于处理无连接的 UDP 通信。
class UDPSocket : public DatagramSocket
{
  // 通过文件描述符构造 UDP 套接字。
  explicit UDPSocket( FileDescriptor&& fd ) : DatagramSocket( std::move( fd ), AF_INET, SOCK_DGRAM ) {}

public:
  // 默认构造函数：创建一个未绑定且未连接的 UDP 套接字。
  UDPSocket() : DatagramSocket( AF_INET, SOCK_DGRAM ) {}
};

// TCPSocket 类：TCP 套接字的封装，用于建立可靠的面向连接通信。
// 提供监听和连接的功能，适用于客户端和服务器端的实现。
class TCPSocket : public Socket
{
private:
  // 通过文件描述符构造 TCP 套接字，通常由 accept() 返回的新连接使用。
  explicit TCPSocket( FileDescriptor&& fd ) : Socket( std::move( fd ), AF_INET, SOCK_STREAM, IPPROTO_TCP ) {}

public:
  // 默认构造函数：创建一个未绑定且未连接的 TCP 套接字。
  TCPSocket() : Socket( AF_INET, SOCK_STREAM ) {}

  // 将套接字切换为监听模式，backlog 参数指定等待连接队列的长度。
  void listen( int backlog = 16 );

  // 接受传入连接，返回一个新的 TCP 套接字对象。
  TCPSocket accept();
};

// PacketSocket 类：原始数据包套接字，用于低级网络通信和捕获数据包。
// 在网络调试和监控中有较多应用。
class PacketSocket : public DatagramSocket
{
public:
  // 构造函数：根据指定的 type 和 protocol 创建 PacketSocket。
  PacketSocket( const int type, const int protocol ) : DatagramSocket( AF_PACKET, type, protocol ) {}

  // 设置混杂模式，允许套接字接收经过网卡的所有数据包。
  void set_promiscuous();
};

// LocalStreamSocket 类：Unix 域流式套接字，用于本地系统进程间的面向连接通信。
// 常用于同一主机内的高速数据交换。
class LocalStreamSocket : public Socket
{
public:
  // 通过文件描述符构造 Unix 域流式套接字。
  explicit LocalStreamSocket( FileDescriptor&& fd ) : Socket( std::move( fd ), AF_UNIX, SOCK_STREAM ) {}
};

// LocalDatagramSocket 类：Unix 域数据报套接字，用于本地系统进程间的无连接通信。
// 适合传输小数据块，效率较高。
class LocalDatagramSocket : public DatagramSocket
{
  // 通过文件描述符构造 Unix 域数据报套接字。
  explicit LocalDatagramSocket( FileDescriptor&& fd ) : DatagramSocket( std::move( fd ), AF_UNIX, SOCK_DGRAM ) {}

public:
  // 默认构造函数：创建一个未绑定且未连接的 Unix 域数据报套接字。
  LocalDatagramSocket() : DatagramSocket( AF_UNIX, SOCK_DGRAM ) {}
};
