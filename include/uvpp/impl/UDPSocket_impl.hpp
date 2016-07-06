#ifndef UDPSocket_impl_hpp
#define UDPSocket_impl_hpp

namespace uvpp {
	namespace _udp_internal {
		inline void allocBuffer(uv_handle_t*, size_t suggested_size, uv_buf_t* buf) {
  		buf->base = new char[suggested_size];
			buf->len = suggested_size;
		}

		inline void onRead(uv_udp_t *udp_socket, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
			if(udp_socket->data == nullptr) {
				return;
			}

			UDPSocket *socket = (UDPSocket*) udp_socket->data;
			if(nread < 0){
				socket->_is_listening = false;
				if(socket->_on_error) socket->_on_error(nread, uv_err_name(nread));
        socket->unbind();
			}else{
				if(nread != 0){
					char ip[64];
        	uv_inet_ntop(AF_INET, &((sockaddr_in*)addr)->sin_addr, ip, sizeof(ip));
        	auto port = ntohs(((sockaddr_in*)addr)->sin_port);
					if(socket->_on_data) socket->_on_data(std::string((char*)buf->base, (size_t) nread), ip, port);
				}
			}

			if(buf != nullptr) delete[] buf->base;
		}
	}

  inline UDPSocket::UDPSocket() :
    _is_listening(false), _udp_socket(nullptr), _listening_on_port(-1), _on_data(nullptr), _on_error(nullptr) { }

	inline UDPSocket::UDPSocket(uv_loop_t* loop, const OnDataCallback& on_data, const OnErrorCallback& on_error) :
	_is_listening(false), _running_loop(loop), _udp_socket(nullptr), _listening_on_port(-1),
	_on_data(on_data), _on_error(on_error) {
    uv_bind_and_listen();
	}

	inline UDPSocket::~UDPSocket() {
		unbind();
	}
  
  inline UDPSocket::UDPSocket(UDPSocket&& other) :
    _is_listening(false), _udp_socket(nullptr), _listening_on_port(0), _on_data(nullptr), _on_error(nullptr) {
    swap(other);
  }

  inline UDPSocket& UDPSocket::operator=(UDPSocket&& other) {
    if (_is_listening) {
      unbind();
    }
    swap(other);
    return *this;
  }

	inline void UDPSocket::send(const std::string& send_host, int send_port, const std::string& data) {
    if (_is_listening) {
      uv_udp_send_t* send_req = new uv_udp_send_t();

    	char * msg = new char[data.size() + 1];
      std::copy(data.begin(), data.end(), msg);
      uv_buf_t buf = uv_buf_init(msg, data.size());
	
    	struct sockaddr_in send_addr;
    	uv_ip4_addr(send_host.c_str(), send_port, &send_addr);
    	uv_udp_send(send_req, _udp_socket, &buf, 1, (const struct sockaddr *)&send_addr, 
    		[](uv_udp_send_t* req, int status){ delete req; });
    }
	}

	inline void UDPSocket::bind(uv_loop_t* loop, const OnDataCallback& on_data, const OnErrorCallback& on_error) {
		_running_loop = loop;
		_on_data = on_data;
		_on_error = on_error;
		uv_bind_and_listen();
	}

	inline void UDPSocket::unbind() {
    if (_udp_socket) {
      _udp_socket->data = nullptr;
      uv_close((uv_handle_t*)_udp_socket, [](uv_handle_t* udp_socket) { 
        delete udp_socket; 
      });
      _udp_socket = nullptr;
    }
    _is_listening = false;
    _listening_on_port = 0;
    _on_data = nullptr;
    _on_error = nullptr;
	}

	inline bool UDPSocket::isListening() const {
		return _is_listening;
	}

	inline int UDPSocket::listening_on_port() const {
		return _listening_on_port;
	}

	inline void UDPSocket::setOnDataCallback(const OnDataCallback& on_data) {
		_on_data = on_data;
	}

	inline void UDPSocket::setOnErrorCallback(const OnErrorCallback& on_error) {
		_on_error = on_error;
	}

  // Swap with another instance of UDPSocket for move construction
  inline void UDPSocket::swap(UDPSocket& other) {
    std::swap(_running_loop, other._running_loop);
    std::swap(_udp_socket, other._udp_socket);
    if (other._is_listening) {
      _udp_socket->data = this;
    }
    if (_is_listening) {
      other._udp_socket->data = &other;
    }

    std::swap(_is_listening, other._is_listening);

    std::swap(_listening_on_port, other._listening_on_port);

    std::swap(_on_data, other._on_data);
    std::swap(_on_error, other._on_error);
  }

	inline void UDPSocket::uv_bind_and_listen() {
  	_udp_socket = new uv_udp_t();
  	uv_udp_init(_running_loop, _udp_socket);
  	_udp_socket->data = this;
    uv_udp_recv_start(_udp_socket, _udp_internal::allocBuffer, _udp_internal::onRead);
    _is_listening = true;

    struct sockaddr_in recv_addr;
    int namelen = 0;
    uv_udp_getsockname(&_udp_socket, ((sockaddr_in*)&recv_addr), &namelen);
    _listening_on_port = ntohs(recv_addr.sin_port);
	}
}

#endif /* UDPSocket_impl_hpp */
