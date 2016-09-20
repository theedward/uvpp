#ifndef TCPConnection_impl_hpp
#define TCPConnection_impl_hpp

namespace uvpp {
	namespace _tcp_internal {
		inline void allocBuffer(uv_handle_t*, size_t suggested_size, uv_buf_t* buf) {
  			buf->base = new char[suggested_size];
			buf->len = suggested_size;
		}

		inline void onConnect(uv_connect_t *req, int status) {
  		if (req->handle->data == nullptr) {
    		return;
  		}
  		TCPConnection *connection = (TCPConnection*) req->handle->data;
			if (status < 0) {
        connection->_is_connected = false;
				if(connection->_on_error) connection->_on_error(status, uv_err_name(status));
        connection->disconnect();
      } else {
        connection->_is_connected = true;
        if (connection->_on_connect) connection->_on_connect();
        connection->uv_read();
      }
      delete req;
		}

		inline void onRead(uv_stream_t *tcp_connection, ssize_t nread, const uv_buf_t* buf) {
			if(tcp_connection->data == nullptr) {
				return;
			}

			TCPConnection *connection = (TCPConnection*) tcp_connection->data;
			if(nread < 0){
				connection->_is_connected = false;
				if(connection->_on_error) connection->_on_error(nread, uv_err_name(nread));
        connection->disconnect();
			}else{
				if(nread != 0){
					if(connection->_on_data) connection->_on_data(std::string((char*)buf->base, (size_t) nread));
				}else{
					if(connection->_on_data) connection->_on_data(std::string());
				}
			}

			if(buf != nullptr) delete[] buf->base;
		}
	}

  inline TCPConnection::TCPConnection() :
    _is_connected(false), _tcp_connection(nullptr), _connected_to_port(0), _on_connect(nullptr), _on_data(nullptr), _on_error(nullptr) { }

	inline TCPConnection::TCPConnection(uv_loop_t* loop, const std::string& connect_ip, int connect_port, 
					const OnConnectCallback& on_connect, const OnDataCallback& on_data, const OnErrorCallback& on_error) :
	_is_connected(false), _running_loop(loop), _tcp_connection(nullptr), _connected_to_ip(connect_ip), _connected_to_port(connect_port),
	_on_connect(on_connect), _on_data(on_data), _on_error(on_error) {
    uv_connect();
	}

	inline TCPConnection::~TCPConnection() {
		disconnect();
	}
  
  inline TCPConnection::TCPConnection(TCPConnection&& other) :
    _is_connected(false), _tcp_connection(nullptr), _connected_to_port(0), _on_connect(nullptr), _on_data(nullptr), _on_error(nullptr) {
    swap(other);
  }

  inline TCPConnection& TCPConnection::operator=(TCPConnection&& other) {
    if (_is_connected) {
      disconnect();
    }
    swap(other);
    return *this;
  }

	inline void TCPConnection::write(const std::string& data) {
    if (_is_connected) {
      uv_write_t* write_req = new uv_write_t();
      char * msg = new char[data.size() + 1];
      std::copy(data.begin(), data.end(), msg);
      uv_buf_t buf = uv_buf_init(msg, data.size());
      write_req->data = msg;
      uv_write(write_req, (uv_stream_t*)_tcp_connection, &buf, 1,
               [](uv_write_t* write_req, int) {
                 delete[] (char*)write_req->data;
                 delete write_req;
               });
    }
	}

	inline void TCPConnection::connect(uv_loop_t* loop, const std::string& connect_ip, int connect_port, 
					const OnConnectCallback& on_connect, const OnDataCallback& on_data, const OnErrorCallback& on_error) {
		_running_loop = loop;
		_connected_to_ip = connect_ip;
		_connected_to_port = connect_port;
		_on_connect = on_connect;
		_on_data = on_data;
		_on_error = on_error;
		uv_connect();
	}

	inline void TCPConnection::disconnect() {
    if (_tcp_connection) {
      _tcp_connection->data = nullptr;
      uv_close((uv_handle_t*)_tcp_connection, [](uv_handle_t* tcp_connection) { 
        delete tcp_connection; 
      });
      _tcp_connection = nullptr;
    }
    _is_connected = false;
    _connected_to_ip = std::string();
    _connected_to_port = 0;
    _on_connect = nullptr;
    _on_data = nullptr;
    _on_error = nullptr;
	}

	inline bool TCPConnection::isConnected() const {
		return _is_connected;
	}

	inline const std::string& TCPConnection::ip() const {
		return _connected_to_ip;
	}

	inline int TCPConnection::port() const {
		return _connected_to_port;
	}

	inline void TCPConnection::setOnConnectCallback(const OnConnectCallback& on_connect) {
		_on_connect = on_connect;
	}

	inline void TCPConnection::setOnDataCallback(const OnDataCallback& on_data) {
		_on_data = on_data;
	}

	inline void TCPConnection::setOnErrorCallback(const OnErrorCallback& on_error) {
		_on_error = on_error;
	}

	// Private constructor to be used by TCP Acceptor in order to return a connection upon accept
	inline TCPConnection::TCPConnection(uv_tcp_t* accepted_connection, const std::string& peer_ip, int peer_port) :
	_is_connected(true), _tcp_connection(accepted_connection), _connected_to_ip(peer_ip), _connected_to_port(peer_port), 
	_on_connect(nullptr), _on_data(nullptr), _on_error(nullptr) {
		_tcp_connection->data = this;
		uv_read();
	}

  // Swap with another instance of TCPConnection for move construction
  inline void TCPConnection::swap(TCPConnection& other) {
    std::swap(_running_loop, other._running_loop);
    std::swap(_tcp_connection, other._tcp_connection);
    if (other._is_connected && _tcp_connection != nullptr) {
      _tcp_connection->data = this;
    }
    if (_is_connected && other._tcp_connection != nullptr) {
      other._tcp_connection->data = &other;
    }

    std::swap(_is_connected, other._is_connected);

    std::swap(_connected_to_ip, other._connected_to_ip);
    std::swap(_connected_to_port, other._connected_to_port);

    std::swap(_on_connect, other._on_connect);
    std::swap(_on_data, other._on_data);
    std::swap(_on_error, other._on_error);
  }

	inline void TCPConnection::uv_connect() {
    _tcp_connection = new uv_tcp_t();
  	uv_tcp_init(_running_loop, _tcp_connection);
  	_tcp_connection->data = this;
    struct sockaddr_in req_addr;
    uv_ip4_addr(_connected_to_ip.c_str(), _connected_to_port, &req_addr);
  	uv_connect_t* connect_req = new uv_connect_t();
  	uv_tcp_connect(connect_req, _tcp_connection, (const struct sockaddr*) &req_addr, _tcp_internal::onConnect);
	}

	inline void TCPConnection::uv_read() {
		uv_read_start((uv_stream_t*) _tcp_connection, _tcp_internal::allocBuffer, _tcp_internal::onRead);
	}
}

#endif /* TCPConnection_impl_hpp */
