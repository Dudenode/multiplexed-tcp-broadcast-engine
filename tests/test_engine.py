import socket
import time
import sys

def test_engine():
    try:
        # Allocate and connect 3 concurrent clients
        clients = [socket.socket(socket.AF_INET, socket.SOCK_STREAM) for _ in range(3)]
        for c in clients:
            c.connect(('127.0.0.1', 8080))
            c.setblocking(False)
        
        time.sleep(0.1) # Allow C server to process connections in poll()

        # Client 0 transmits the payload
        msg = b"Automated routing payload\n"
        clients[0].sendall(msg)
        time.sleep(0.1) # Allow C server to route the broadcast

        # Verification 1: Sender (Client 0) must NOT receive its own payload
        try:
            clients[0].recv(1024)
            print("FATAL: Sender received its own transmission.")
            sys.exit(1)
        except BlockingIOError:
            pass # Expected behavior: buffer is empty

        # Verification 2: Peers (Client 1 & 2) MUST receive the payload
        for i in [1, 2]:
            try:
                data = clients[i].recv(1024)
                if data != msg:
                    print(f"FATAL: Client {i} received corrupted data.")
                    sys.exit(1)
            except Exception as e:
                print(f"FATAL: Client {i} failed to receive data. {e}")
                sys.exit(1)

        print("SUCCESS: Multiplexed broadcast routing verified.")
        
    finally:
        for c in clients:
            c.close()

if __name__ == "__main__":
    test_engine()