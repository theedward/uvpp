#ifndef TCPAcceptor_impl_hpp
#define TCPAcceptor_impl_hpp

namespace uvpp {
    namespace internal {
    	static const int connections_to_queue = 128;

        inline void onNewConnection(uv_stream_t *connection_acceptor, int status) {
        	if (status == -1 || connection_acceptor->data == nullptr) {
    			return;
  			}
  			TCPAcceptor* from_acceptor = (TCPAcceptor*)connection_acceptor->data;

  			uv_tcp_t* client = new uv_tcp_t();
  			uv_tcp_init(from_acceptor->runningInLoop(), client);

  			int result = uv_accept(connection_acceptor, (uv_stream_t*) client);
  			if (result == 0) {
  				uv_tcp_nodelay(client, (int) true);
				  char ip[64];
				  struct sockaddr_storage sa;
				  int nameLength = sizeof(sa);
				  uv_tcp_getpeername(client, (sockaddr*) &sa, &nameLength);
          uv_inet_ntop(AF_INET, &((sockaddr_in*)&sa)->sin_addr, ip, sizeof(ip));
  				from_acceptor->_on_accept(from_acceptor->new_tcp_connection_from_accept(client, ip, ntohs(((sockaddr_in*)&sa)->sin_port)));
  			} else {
  			 	uv_close((uv_handle_t*) client, NULL);
  			}
        }
    }

	inline TCPAcceptor::TCPAcceptor() : _running_loop(nullptr), _connection_acceptor(nullptr), _is_accepting(false) { }

	inline TCPAcceptor::TCPAcceptor(uv_loop_t* loop, const std::string& listen_ip, int listen_port, const OnAcceptCallback& on_accept) :
	_running_loop(loop), _connection_acceptor(nullptr), _is_accepting(false), _listen_ip(listen_ip), _listen_port(listen_port), _on_accept(on_accept) {
    start_uv_listen();
	}

	inline TCPAcceptor::~TCPAcceptor() {
		stop();
	}

  inline TCPAcceptor::TCPAcceptor(TCPAcceptor&& other) : _running_loop(nullptr), _connection_acceptor(nullptr), _is_accepting(false) {
    swap(other);
  }

  inline TCPAcceptor& TCPAcceptor::operator=(TCPAcceptor&& other) {
    if (_is_accepting) {
      stop();
    }
    swap(other);
    return *this;
  }
		
	inline bool TCPAcceptor::isAccepting() const {
		return _is_accepting;
	}

	inline void TCPAcceptor::accept(uv_loop_t* loop, const std::string& listen_ip, int listen_port, const OnAcceptCallback& on_accept){
		_running_loop = loop;
		_listen_ip = listen_ip;
		_listen_port = listen_port;
		_on_accept = on_accept;
		start_uv_listen();
	}

  inline void TCPAcceptor::stop() {
    if (_connection_acceptor) {
      _connection_acceptor->data = nullptr;
      uv_close((uv_handle_t*)_connection_acceptor, [](uv_handle_t* connection_acceptor) { 
        delete connection_acceptor; 
      });
      _connection_acceptor = nullptr;
    }
		_is_accepting = false;
    _running_loop = nullptr;
    _listen_ip = std::string();
    _listen_port = 0;
    _on_accept = nullptr;
	}

	inline const std::string& TCPAcceptor::ip() const {
		return _listen_ip;
	}

	inline int TCPAcceptor::port() const {
		return _listen_port;
	}

	inline uv_loop_t* TCPAcceptor::runningInLoop() const {
		return _running_loop;
	}

	inline void TCPAcceptor::start_uv_listen() {
    _connection_acceptor = new uv_tcp_t();
		uv_tcp_init(_running_loop, _connection_acceptor);
		_connection_acceptor->data = this;
		struct sockaddr_in bind_addr;
		uv_ip4_addr(_listen_ip.c_str(), _listen_port, &bind_addr);
		uv_tcp_bind(_connection_acceptor, (const struct sockaddr*) &bind_addr, 0);
		if(uv_listen((uv_stream_t*) _connection_acceptor, internal::connections_to_queue, internal::onNewConnection) == 0) {
			_is_accepting = true;
		}
	}

	inline TCPConnection TCPAcceptor::new_tcp_connection_from_accept(uv_tcp_t* _accepted_connection, const std::string& peer_ip, int peer_port) {
		return TCPConnection(_accepted_connection, peer_ip, peer_port);
	}

  inline void TCPAcceptor::swap(TCPAcceptor& other) {
    std::swap(_connection_acceptor, other._connection_acceptor);
    if (other._is_accepting) {
      _connection_acceptor->data = this;
    }
    if (_is_accepting) {
      other._connection_acceptor->data = &other;
    }
    std::swap(_is_accepting, other._is_accepting);
    std::swap(_running_loop, other._running_loop);
    std::swap(_listen_ip, other._listen_ip);
    std::swap(_listen_port, other._listen_port);
    std::swap(_on_accept, other._on_accept);
  }
}

#endif /* TCPAcceptor_impl_hpp */
