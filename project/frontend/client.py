import socket
import json
from typing import Any, Dict

class BackendClient:
    def __init__(self, host: str = '127.0.0.1', port: int = 5000, timeout: float = 5.0):
        self.host = host
        self.port = port
        self.timeout = timeout

    def send_request(self, payload: Dict[str, Any]) -> Dict[str, Any]:
        data = json.dumps(payload).encode('utf-8')
        # simple length-prefix framing
        length_prefix = len(data).to_bytes(4, byteorder='big')
        with socket.create_connection((self.host, self.port), timeout=self.timeout) as sock:
            sock.sendall(length_prefix + data)
            # read 4 bytes length
            resp_len_bytes = self._recv_all(sock, 4)
            if not resp_len_bytes:
                raise ConnectionError("No response length received")
            resp_len = int.from_bytes(resp_len_bytes, byteorder='big')
            resp_bytes = self._recv_all(sock, resp_len)
            if not resp_bytes:
                raise ConnectionError("No response body received")
            return json.loads(resp_bytes.decode('utf-8'))

    def _recv_all(self, sock: socket.socket, n: int) -> bytes:
        buf = b''
        while len(buf) < n:
            chunk = sock.recv(n - len(buf))
            if not chunk:
                return b''
            buf += chunk
        return buf

if __name__ == '__main__':
    client = BackendClient()
    request = {"action": "status"}  # приклад запиту
    try:
        response = client.send_request(request)
        print("Response:", response)
    except Exception as e:
        print("Error:", e)