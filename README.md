# Dining philosophers problem
## Introduction  
The dining-philosophers problem is a classic concurrency problem: *N* philosophers sit around a circular table with one fork between each pair.  
To **eat**, a philosopher must hold **two** forks (left + right).  
The goal is to let the philosophers alternate between **thinking → hungry → eating** **without**

* **Deadlock** – a circular wait in which everyone holds one fork and no one can obtain the second one.
* **Starvation** – any philosopher being continously denied the chance to eat.
  
## Compilation
C++11 or later is required. Example compilation in terminal opened in project's folder: 
```bash
g++ -std=c++11 -pthread filozof.cpp -o filozof
```
## Running the program
The are two parameters - number of philosophers (default is 5) and duration of the program (default is 60 seconds). This will run the program with default parameters values:
```
./filozof
```
This will run the program with 6 philosophers initiated for 80 seconds:
```
./filozof 6 80
```

## Threads & Critical Sections

| Concept | In-code | Solution |
|---------|----------|----------|
| **Threads** | One `std::thread` per philosopher | Each thread executes `philosopher(id, semaphore, running)`, modelling an independent life-cycle. |
| **Fork access (critical section)** | `std::vector<std::mutex> forks` &nbsp;— one mutex per fork | `forks[i].lock()` / `forks[i].unlock()` ensure that only one philosopher can hold a given fork at a time. |
| **Console output (critical section)** | Global `print_mutex` + `safe_print()` | `std::lock_guard` on `print_mutex` makes every log line atomic, so messages never interleave. |
| **Deadlock prevention** | Custom `Semaphore` initialised to **N − 1** | At most *N − 1* philosophers may attempt to pick up forks concurrently; this breaks the circular-wait condition and guarantees progress. |
| **Clean shutdown coordination** | `std::atomic<bool> running` flag | After the requested runtime the main thread sets `running =false`; each philosopher notices this in its loop and exits, then main `join()`s all threads. |


# Multithreaded Chat Server
## Introduction
This project contains a **thread‑per‑client TCP chat server** written in pure Python 3
(no external dependencies).  
Each new client connection spawns its own `threading.Thread`.  
Messages are **broadcast** to all connected clients, so everybody in the room
sees the same messages in real time.

Goals achieved:

* **Concurrency** – many clients can talk at once.
* **No deadlock / race conditions** – a single low‑level `threading.Lock`
  protects the shared client list and guarantees message ordering.
* **Clean shutdown** – disconnections are detected and resources released.

## Requirements
* Python ≥ 3.9 (standard library only).
* Any OS (Windows, Linux, macOS).  

## Running the server
By default the server listens on **0.0.0.0:12345**.  
To use a different port (e.g. `5000`):

```bash
python server.py 5000
```

## Running the client
If the server is on the same machine and uses the default port:

```bash
python client.py
```

Otherwise:

```bash
# host only
python client.py 192.168.1.10              # port = 12345

# port only
python client.py 5000                      # host = 127.0.0.1

# host + port
python client.py 192.168.1.10 5000
```

Commands inside the client:

* `/quit` or **Ctrl‑C** – leave the chat
* Anything else – send as a public message

## Threads & Critical Sections

| Concept | In‑code | Solution |
|---------|----------|----------|
| **Server: one thread per client** | `threading.Thread(target=_handle_client, daemon=True)` | Independent loop per user; server remains responsive. |
| **Broadcast list (critical section)** | `self._clients: list[tuple[socket,…]]` + `self._clients_lock` | `Lock.acquire()/release()` wraps every read/write so the list stays consistent. |
| **Console transcript** | `print()` in `_handle_client()` under the same lock | Prevents interleaving log lines. |
| **Client: background reader** | `threading.Thread(target=_reader, daemon=True)` | Keeps incoming messages flowing while the main thread waits on `input()`. |
| **Clean disconnect** | `try/finally` around recv loop + removal under lock | Ensures resources freed even on `Ctrl‑C` or network errors. |

## Clean shutdown
* **Server:** `Ctrl‑C` stops `accept()` loop and closes the listening socket.
* **Client:** `/quit`, **Ctrl‑C** or closing the terminal ends both threads.

