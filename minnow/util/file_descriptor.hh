#pragma once

#include "ref.hh"
#include <cstddef>
#include <memory>
#include <vector>

// FileDescriptor 类：封装 Unix 文件描述符，采用 RAII 设计模式管理资源。
// 该类通过内部的引用计数（std::shared_ptr）自动管理文件描述符的生命周期，
// 并记录读写操作的次数、EOF状态和阻塞状态。
class FileDescriptor
{
  // FDWrapper 类：实际封装文件描述符以及相关状态信息。
  // 管理文件描述符 fd_，记录 EOF、关闭状态、非阻塞状态以及操作计数。
  class FDWrapper
  {
  public:
    int fd_;                    // 内核分配的文件描述符编号
    bool eof_ = false;          // 是否到达文件末尾（EOF）
    bool closed_ = false;       // 文件描述符是否已经关闭
    bool non_blocking_ = false; // 是否采用非阻塞模式
    unsigned read_count_ = 0;   // 记录读取操作的次数
    unsigned write_count_ = 0;  // 记录写入操作的次数

    // 构造函数：以内核返回的文件描述符初始化
    explicit FDWrapper( int fd );
    // 析构函数：在对象销毁时调用 close() 系统调用关闭文件描述符.
    //用于在对象生命周期结束时关闭文件描述符并释放资源。
    ~FDWrapper();
    // 手动调用 close(2) 系统调用关闭文件描述符
    void close();

    // 模板方法：检查系统调用的返回值，如果出错则给出具体错误描述
    template<typename T>
    T CheckSystemCall( std::string_view s_attempt, T return_value ) const;

    // 禁止复制和移动，避免多个对象管理同一个文件描述符引发冲突
    FDWrapper( const FDWrapper& other ) = delete;
    FDWrapper& operator=( const FDWrapper& other ) = delete;
    FDWrapper( FDWrapper&& other ) = delete;
    FDWrapper& operator=( FDWrapper&& other ) = delete;
  };

  // 使用 std::shared_ptr 管理 FDWrapper 对象，实现引用计数式资源共享
  std::shared_ptr<FDWrapper> internal_fd_;

  // 私有构造函数，通过已有 shared_ptr 创建 FileDescriptor，用于 duplicate() 操作
  explicit FileDescriptor( std::shared_ptr<FDWrapper> other_shared_ptr );

protected:
  // 定义读取时的缓冲区大小，单位为字节
  static constexpr size_t kReadBufferSize = 16384;

  // 设置 EOF 状态，当文件已经读取至结尾时调用
  void set_eof() { internal_fd_->eof_ = true; }
  // 注册一次读取操作，更新内部计数器
  void register_read() { ++internal_fd_->read_count_; }   
  // 注册一次写入操作，更新内部计数器
  void register_write() { ++internal_fd_->write_count_; }

  // 模板方法：封装系统调用结果检查，类似于 FDWrapper 中的方法
  template<typename T>
  T CheckSystemCall( std::string_view s_attempt, T return_value ) const;

public:
  // 构造函数：使用内核返回的文件描述符创建对象，内部封装在 shared_ptr 中
  explicit FileDescriptor( int fd );

  // 默认析构函数：释放 shared_ptr，引用计数归零时自动关闭文件描述符
  ~FileDescriptor() = default;

  // 从文件描述符中读取数据，将结果存入 std::string 中
  void read( std::string& buffer );
  // 从文件描述符读取数据块，存储到 std::vector<std::string> 中
  void read( std::vector<std::string>& buffers );

  // 将数据写入文件描述符，返回实际写入的字节数
  size_t write( std::string_view buffer );
  // 写入多个数据块（std::string_view 类型），返回总写入字节数
  size_t write( const std::vector<std::string_view>& buffers );
  // 写入多个引用包装（Ref<std::string>）的数据，返回写入的字节数
  size_t write( const std::vector<Ref<std::string>>& buffers );

  // 显式调用内部 FDWrapper 的 close() 方法，关闭文件描述符
  void close() { internal_fd_->close(); }

  // 显式复制 file descriptor，增加内部 FDWrapper 引用计数
  FileDescriptor duplicate() const;

  // 设置文件描述符的阻塞或非阻塞状态，参数 true 表示阻塞模式
  void set_blocking( bool blocking );

  // 获取文件大小，通常通过 fstat 系统调用得到
  off_t size() const;

  // 以下是访问封装的底层状态信息接口
  int fd_num() const { return internal_fd_->fd_; }                        // 获取实际文件描述符编号
  bool eof() const { return internal_fd_->eof_; }                           // 检查是否到达 EOF
  bool closed() const { return internal_fd_->closed_; }                     // 检查是否已关闭
  unsigned int read_count() const { return internal_fd_->read_count_; }      // 获取读取操作计数
  unsigned int write_count() const { return internal_fd_->write_count_; }    // 获取写入操作计数

  // 禁止隐式拷贝，防止多个对象同时操作同一文件描述符
  FileDescriptor( const FileDescriptor& other ) = delete;
  FileDescriptor& operator=( const FileDescriptor& other ) = delete;
  // 允许移动操作，支持资源所有权转移
  FileDescriptor( FileDescriptor&& other ) = default;
  FileDescriptor& operator=( FileDescriptor&& other ) = default;
};
