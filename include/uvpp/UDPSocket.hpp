#ifndef UDPSocket_hpp
#define UDPSocket_hpp

#include <functional>
#include <string>
#include "uv.h"

namespace uvpp {
	namespace internal {
		void onRead(uv_udp_t *udp_socket, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);
	}
	class UDPSocket {
		friend void internal::onRead(uv_udp_t *udp_socket, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);

	public:
		using OnDataCallback = std::function<void(const std::string& data, const std::string& from_host, int from_port)>;
		using OnErrorCallback = std::function<void(int, const std::string&)>;

		UDPSocket();
		UDPSocket(const UDPSocket&) = delete;
		UDPSocket(UDPSocket&&);
		UDPSocket(uv_loop_t* loop, int bind_listen_port,
			const OnDataCallback& on_data = nullptr, const OnErrorCallback& on_error = nullptr);
		~UDPSocket();
		UDPSocket& operator=(const UDPSocket&) = delete;
    	UDPSocket& operator=(UDPSocket&& other);
	
		void send(const std::string& send_host, int send_port, const std::string& data);

		void bind(uv_loop_t* loop, int bind_listen_port,
						const OnDataCallback& on_data = nullptr, const OnErrorCallback& on_error = nullptr);
		void unbind();

		bool isListening() const;
		int listening_on_port() const;

		void setOnDataCallback(const OnDataCallback& on_data);
		void setOnErrorCallback(const OnErrorCallback& on_error);

	private:
    void swap(UDPSocket& other);
		void uv_bind_and_listen();

	private:
		bool _is_listening;

		uv_loop_t* _running_loop;
		uv_udp_t* _udp_socket;

		int _listening_on_port;

		OnDataCallback _on_data;
		OnErrorCallback _on_error;
	};
}

#include "impl/UDPSocket_impl.hpp"

#endif /* UDPSocket_hpp */
