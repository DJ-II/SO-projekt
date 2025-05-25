import socket
import threading


class ChatServer:
    def __init__(self, host="0.0.0.0", port=5000):
        self.host, self.port = host, port
        self._clients = []
        self._lock = threading.Lock()

    def start(self):
        print(f"[STARTING] {self.host}:{self.port}")
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
            srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            srv.bind((self.host, self.port))
            srv.listen()
            while True:
                conn, addr = srv.accept()
                threading.Thread(target=self._client, args=(conn, addr), daemon=True).start()

    def _client(self, conn, addr):
        with self._lock:
            self._clients.append(conn)
        try:
            while data := conn.recv(1024):
                msg = f"[{addr}] {data.decode()}".encode()
                with self._lock:
                    for c in self._clients:
                        if c is not conn:
                            c.sendall(msg)
        finally:
            with self._lock:
                self._clients.remove(conn)
            conn.close()


if __name__ == "__main__":
    import sys
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 5000
    ChatServer(port=port).start()

