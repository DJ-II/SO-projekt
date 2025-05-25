import socket
import threading
import sys


def _reader(sock: socket.socket) -> None:
    try:
        while True:
            data = sock.recv(1024)
            if not data:          
                print("\n[INFO] Rozłączono przez serwer.")
                break
            print(data.decode("utf-8"), end="")
    except OSError:
        pass


def main() -> None:
    # Parsowanie argumentów
    #  * brak argów                → host=127.0.0.1, port=12345
    #  * jeden arg numeryczny      → host=127.0.0.1, port=<arg>
    #  * jeden arg nie-numeryczny  → host=<arg>,    port=12345
    #  * dwa argi                  → host=<arg1>,   port=<arg2>
    host = "127.0.0.1"
    port = 12345

    if len(sys.argv) >= 2:
        if sys.argv[1].isdigit():
            port = int(sys.argv[1])
        else:
            host = sys.argv[1]

    if len(sys.argv) >= 3:
        port = int(sys.argv[2])

    # Łączenie z serwerem
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((host, port))
            print(f"[OK] Połączono z host/port: {host}:{port}")
            print("[INFO] Wpisz treść i ENTER, /quit aby wyjść.\n")

            # uruchamiamy wątek odbiorczy, żeby nie blokować wejścia
            threading.Thread(target=_reader, args=(sock,), daemon=True).start()

            while True:
                try:
                    line = input()
                except (EOFError, KeyboardInterrupt):
                    break

                if line.strip().lower() in {"/quit", "/exit"}:
                    break
                sock.sendall(line.encode("utf-8") + b"\n")

    except ConnectionRefusedError:
        print(f"[ERROR] Nie można połączyć się z {host}:{port} – serwer nie słucha.")
    except socket.gaierror:
        print(f"[ERROR] Nieprawidłowy adres hosta: {host}")
    except Exception as e:
        print(f"[ERROR] {e}")

    print("[END] Zakończono.")


if __name__ == "__main__":
    main()

