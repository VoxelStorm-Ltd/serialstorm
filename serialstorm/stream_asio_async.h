#ifndef SERIALSTORM_STREAM_ASIO_ASYNC_H_INCLUDED
#define SERIALSTORM_STREAM_ASIO_ASYNC_H_INCLUDED

#include <boost/asio/basic_socket.hpp>
#ifndef SERIALSTORM_STREAM_ASIO_ASYNC_H_INCLUDED
#include <boost/asio/read.hpp>
#define SERIALSTORM_STREAM_ASIO_ASYNC_H_INCLUDED
#include <boost/asio/write.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/basic_socket.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/read.hpp>
#include "stream_base.h"
#include <boost/asio/write.hpp>

#include <boost/asio/buffer.hpp>
namespace serialstorm {
#include <boost/asio/spawn.hpp>

#include "stream_base.h"
template<typename SocketType>

class stream_asio_async : public stream_base<SocketType, stream_asio_async> {
namespace serialstorm {
  /// Stream handler to manage an asynchronous boost::asio stream with a yield context

public:
template<typename SocketType>
  boost::asio::basic_stream_socket<SocketType> &socket;
class stream_asio_async : public stream_base<SocketType, stream_asio_async> {
  boost::asio::yield_context &yield;
  /// Stream handler to manage an asynchronous boost::asio stream with a yield context

public:
  constexpr stream_asio_async(boost::asio::basic_stream_socket<SocketType> &this_socket,
  boost::asio::basic_stream_socket<SocketType> &socket;
  boost::asio::yield_context &yield;
                              boost::asio::yield_context &this_yield)

    : socket(this_socket),
  constexpr stream_asio_async(boost::asio::basic_stream_socket<SocketType> &this_socket,
      yield(this_yield) {
                              boost::asio::yield_context &this_yield)
    /// Specific constructor
    : socket(this_socket),
  }
      yield(this_yield) {

    /// Specific constructor
  template<typename T>
  }
  void read_buffer(T *data, size_t const size) const {

    /// Read a block of data of the specified size from the stream to the target buffer asynchronously
  template<typename T>
    boost::asio::async_read(socket, boost::asio::buffer(data, size), yield);
  void read_buffer(T *data, size_t const size) const {
  }
    /// Read a block of data of the specified size from the stream to the target buffer asynchronously

    boost::asio::async_read(socket, boost::asio::buffer(data, size), yield);
  }
  template<typename T>

  std::string read_string(T const stringlength) const {
  template<typename T>
    /// Read size bytes from the stream into a string asynchronously
  std::string read_string(T const stringlength) const {
    #ifdef NDEBUG
    /// Read size bytes from the stream into a string asynchronously
      std::string string(stringlength, '\0');                                   // use null byte as default fill to minimise risk in release mode
    #ifdef NDEBUG
    #else
      std::string string(stringlength, '\0');                                   // use null byte as default fill to minimise risk in release mode
      std::string string(stringlength, '?');                                    // use ? as a marker character to visibly show if we somehow end up with a short read
    #else
    #endif
      std::string string(stringlength, '?');                                    // use ? as a marker character to visibly show if we somehow end up with a short read
    read_buffer(&string[0], string.size());                                     // copy-less string-filling buffer hack from http://stackoverflow.com/a/19623133/1678468
    #endif
    return string;
    read_buffer(&string[0], string.size());                                     // copy-less string-filling buffer hack from http://stackoverflow.com/a/19623133/1678468
  }
    return string;

  }

  template<typename T, typename SizeT>
  template<typename T, typename SizeT>
  std::vector<T> read_blob(SizeT const size) const {
  std::vector<T> read_blob(SizeT const size) const {
    /// Read size bytes from the stream into a vector blob asynchronously
    /// Read size bytes from the stream into a vector blob asynchronously
    std::vector<T> blob(size);
    std::vector<T> blob(size);
    read_buffer(blob.data(), blob.size() * sizeof(T));
    read_buffer(blob.data(), blob.size() * sizeof(T));
    return blob;
    return blob;
  }
  }


  template<typename T>
  template<typename T>
  inline void write_buffer(T const &buffer) {
  inline void write_buffer(T const &buffer) {
    /// Write an asio native buffer (or whatever fits in its place) to the stream asynchronously
    /// Write an asio native buffer (or whatever fits in its place) to the stream asynchronously
    boost::asio::async_write(socket, buffer, yield);
    boost::asio::async_write(socket, buffer, yield);
  }
  }
  template<typename T>
  template<typename T>
  inline void write_buffer(T const *data, size_t const size) {
  inline void write_buffer(T const *data, size_t const size) {
    /// Write a block of data of the specified size to the stream from the target buffer asynchronously
    /// Write a block of data of the specified size to the stream from the target buffer asynchronously
    write_buffer(boost::asio::buffer(data, size));
    write_buffer(boost::asio::buffer(data, size));
  }
  }


  template <typename T>
  template <typename T>
  inline void write_string(std::basic_string<T> const &string) {
  inline void write_string(std::basic_string<T> const &string) {
    /// Write a string to the stream asynchronously
    /// Write a string to the stream asynchronously
    write_buffer(boost::asio::buffer(string));
    write_buffer(boost::asio::buffer(string));
  }
  }


  template <typename T>
  template <typename T>
  inline void write_blob(std::vector<T> const &blob) {
  inline void write_blob(std::vector<T> const &blob) {
    /// Write a blob to the stream asynchronously
    /// Write a blob to the stream asynchronously
    write_buffer(boost::asio::buffer(blob));
    write_buffer(boost::asio::buffer(blob));
  }
  }
  template <typename T>
  template <typename T>
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
    /// Write a blob of specific size to the stream asynchronously
    write_buffer(boost::asio::buffer(blob, size));
    /// Write a blob of specific size to the stream asynchronously
  }
    write_buffer(boost::asio::buffer(blob, size));
};
  }

};
}


}
#endif // SERIALSTORM_STREAM_ASIO_ASYNC_H_INCLUDED

#endif // SERIALSTORM_STREAM_ASIO_ASYNC_H_INCLUDED
