#pragma once

namespace serialstorm {

template<typename StreamParam, template<typename> class StreamT>
class stream_base;

template<typename StreamT>
class stream_std_stream;

template<typename SocketType>
class stream_asio_sync;

template<typename SocketType>
class stream_asio_async;

}
