#ifndef TCPAcceptor_hpp
#define TCPAcceptor_hpp

#include <functional>
#include <string>
#include "uv.h"

#include "TCPConnection.hpp"

namespace uvpp {
	namespace internal {
		void onNewConnection(uv_stream_t *connection_acceptor, int status);
	}

	class TCPAcceptor {
		friend void internal::onNewConnection(uv_stream_t *connection_acceptor, int status);

	public:
		using OnAcceptCallback = std::function<void(TCPConnection)>;

		TCPAcceptor();
    TCPAcceptor(const TCPAcceptor&) = delete;
    TCPAcceptor(TCPAcceptor&& other);
		TCPAcceptor(uv_loop_t* loop, const std::string& listen_ip, int listen_port, const OnAcceptCallback& on_accept);
		~TCPAcceptor();
    TCPAcceptor& operator=(const TCPAcceptor&) = delete;
    TCPAcceptor& operator=(TCPAcceptor&& other);
	
		void accept(uv_loop_t* loop, const std::string& listen_ip, int listen_port, const OnAcceptCallback& on_accept);
		void stop();

		bool isAccepting() const;
		const std::string& ip() const;
		int port() const;
		uv_loop_t* runningInLoop() const;
		
	private:
		void start_uv_listen();
		TCPConnection new_tcp_connection_from_accept(uv_tcp_t* _accepted_connection, const std::string& peer_ip, int peer_port);
    void swap(TCPAcceptor&);

		uv_loop_t* _running_loop;
		uv_tcp_t* _connection_acceptor;
		bool _is_accepting;
		std::string _listen_ip;
		int _listen_port;
		OnAcceptCallback _on_accept;
	};
}

#include "impl/TCPAcceptor_impl.hpp"

#endif /* TCPAcceptor_hpp */
