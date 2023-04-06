import socket

PORT = 80  # The port number on which to receive messages

# Create a socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(('0.0.0.0', PORT))

# Listen for incoming connections
server_socket.listen(1)

print(f'Listening on port {PORT}...')

while True:
    # Accept a new connection
    client_socket, client_address = server_socket.accept()

    # Receive the message
    message = client_socket.recv(1024).decode('utf-8').strip()

    # Print the received message
    print(f'Received message: {message}')

    # Close the connection
    client_socket.close()

