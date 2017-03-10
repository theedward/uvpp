#ifndef TCPConnection_hpp
#define TCPConnection_hpp

#include <functional>
#include <string>
#include <vector>
#include "uv.h"

namespace uvpp {
	namespace _tcp_internal {
		void onConnect(uv_connect_t *req, int status);
		void onRead(uv_stream_t *tcp_connection, ssize_t nread, const uv_buf_t* buf);
	}
	class TCPAcceptor;

    struct TCPConnectionOptions {
        bool use_packet_size_header = false;
    };

	class TCPConnection {
		friend class TCPAcceptor;
		friend void _tcp_internal::onRead(uv_stream_t *tcp_connection, ssize_t nread, const uv_buf_t* buf);
		friend void _tcp_internal::onConnect(uv_connect_t *req, int status);

	public:
		using OnConnectCallback = std::function<void(void)>;
		using OnDataCallback = std::function<void(std::string)>;
		using OnErrorCallback = std::function<void(int,std::string)>;

		TCPConnection();
		TCPConnection(const TCPConnection&) = delete;
		TCPConnection(TCPConnection&&);
		TCPConnection(uv_loop_t* loop, const std::string& connect_ip, int connect_port, const TCPConnectionOptions& options = TCPConnectionOptions(),
						const OnConnectCallback& on_connect = nullptr, const OnDataCallback& on_data = nullptr, const OnErrorCallback& on_error = nullptr);
		~TCPConnection();
		TCPConnection& operator=(const TCPConnection&) = delete;
        TCPConnection& operator=(TCPConnection&& other);

		void write(const std::string& data);

		void connect(uv_loop_t* loop, const std::string& connect_ip, int connect_port,
						const OnConnectCallback& on_connect = nullptr, const OnDataCallback& on_data = nullptr, const OnErrorCallback& on_error = nullptr);
		void disconnect();

		bool isConnected() const;
		const std::string& ip() const;
		int port() const;

		void setOnConnectCallback(const OnConnectCallback& on_connect);
		void setOnDataCallback(const OnDataCallback& on_data);
		void setOnErrorCallback(const OnErrorCallback& on_error);

        // Options
        inline void setUsePacketSizeHeader(bool option = true) { _my_options.use_packet_size_header = option; }
	private:
		// Private constructor to be used by TCP Acceptor in order to return a connection upon accept
		TCPConnection(uv_tcp_t* _accepted_connection, const std::string& peer_ip, int peer_port, const TCPConnectionOptions& options = TCPConnectionOptions());
        void swap(TCPConnection& other);
		void uv_connect();
		void uv_read();

        void processDataFrame(ssize_t nread, const uv_buf_t* buf);
        void consumeIncompleteDataBuffer();
	private:
		bool _is_connected;

		uv_loop_t* _running_loop;
		uv_tcp_t* _tcp_connection;

		std::string _connected_to_ip;
		int _connected_to_port;

		OnConnectCallback _on_connect;
		OnDataCallback _on_data;
		OnErrorCallback _on_error;

        std::string _incomplete_data_buffer;
        bool _queue_on_data = false;
        std::vector<std::string> _queued_messages;

        // Options
        TCPConnectionOptions _my_options;
	};
}

#include "impl/TCPConnection_impl.hpp"

#endif /* TCPConnection_hpp */
