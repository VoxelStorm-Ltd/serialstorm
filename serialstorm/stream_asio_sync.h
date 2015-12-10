#ifndef STREAM_ASIO_SYNC_H_INCLUDED
#define STREAM_ASIO_SYNC_H_INCLUDED

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include "stream_base.h"

namespace serialstorm {

template<class SocketType>
class stream_asio_sync : public stream_base<SocketType, stream_asio_sync> {
  /// Stream handler to manage a asynchronous boost::asio stream
public:
  boost::asio::basic_stream_socket<SocketType> &socket;

  constexpr stream_asio_sync(boost::asio::basic_stream_socket<SocketType> &this_socket)
    : socket(this_socket) {
    /// Specific constructor
  }

  template<typename T>
  void read_buffer(T *data, size_t const size) const {
    /// Read a block of data of the specified size from the stream to the target buffer synchronously
    boost::asio::read(socket, boost::asio::buffer(data, size));
  }

  template<typename T>
  std::string read_string(T const stringlength) const {
    /// Read size bytes from the stream into a string synchronously
    #ifdef NDEBUG
      std::string string(stringlength, '\0');               // use null byte as default fill to minimise risk in release mode
    #else
      std::string string(stringlength, '?');                // use ? as a marker character to visibly show if we somehow end up with a short read
    #endif
    read_buffer(&string[0], string.size());                 // copy-less string-filling buffer hack from http://stackoverflow.com/a/19623133/1678468
    return string;
  }

  template<typename T, typename SizeT>
  std::vector<T> read_blob(SizeT const size) const {
    /// Read size bytes from the stream into a vector blob synchronously
    std::vector<T> blob(size);
    read_buffer(blob.data(), blob.size() * sizeof(T));
    return blob;
  }

  template<typename T>
  inline void write_buffer(T const &buffer) {
    /// Write an asio native buffer (or whatever fits in its place) to the stream synchronously
    boost::asio::write(socket, buffer);
  }
  template<typename T>
  inline void write_buffer(T const *data, size_t const size) {
    /// Write a block of data of the specified size to the stream from the target buffer synchronously
    write_buffer(boost::asio::buffer(data, size));
  }

  template <typename T>
  inline void write_string(std::basic_string<T> const &string) {
    /// Write a string to the stream synchronously
    write_buffer(boost::asio::buffer(string));
  }

  template <typename T>
  inline void write_blob(std::vector<T> const &blob) {
    /// Write a blob to the stream synchronously
    write_buffer(boost::asio::buffer(blob));
  }
  template <typename T>
  inline void write_blob(std::vector<T> const &blob, size_t const size) {
    /// Write a blob of specific size to the stream synchronously
    write_buffer(boost::asio::buffer(blob, size));
  }
};

}

#endif // STREAM_ASIO_SYNC_H_INCLUDED
