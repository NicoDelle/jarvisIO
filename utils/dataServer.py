import socket
import wave

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to a specific address and port
server_address = ('192.168.15.2', 2023) # IP address of your PC
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

# Open a .wav file for writing
wav_file = wave.open('output.wav', 'wb')
wav_file.setnchannels(1) # mono audio
wav_file.setsampwidth(4) # 2 bytes per sample
wav_file.setframerate(8000) # sample rate

while True:
    # Wait for a connection
    print('waiting for a connection')
    connection, client_address = sock.accept()
    try:
        print('connection from', client_address)

        # Receive the data in small chunks and write it to the .wav file
        while True:
            data = connection.recv(8000 * 4)
            print('received {!r}'.format(data))
            if data:
                wav_file.writeframes(data)
            else:
                print('no data from', client_address)
                break
            
    finally:
        # Clean up the connection
        connection.close()
        break

# Close the .wav file
wav_file.close()