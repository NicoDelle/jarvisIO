import socket
import wave
import struct

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to a specific address and port
server_address = ('192.168.1.104', 2023) # IP address of your PC
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

# Open a .wav file for writing
wav_file = wave.open('output.wav', 'wb')
wav_file.setnchannels(1) # mono audio
wav_file.setsampwidth(4) # 2 bytes per sample
wav_file.setframerate(8000) # sample rate

float_data_file = open('float_data.txt', 'w')

while True:
    # Wait for a connection
    print('waiting for a connection')
    connection, client_address = sock.accept()
    try:
        print('connection from', client_address)

        # Receive the data in small chunks and write it to the .wav file
        while True:
            data = connection.recv(8000 * 4)
            if data:
                float_data = struct.unpack('f' * (len(data) // 4), data)
                # Convert floats back to bytes for writing to the WAV file
                byte_data = struct.pack('h' * len(float_data), *(int(sample * 32767) for sample in float_data))
                wav_file.writeframes(byte_data)

                for sample in float_data:
                    float_data_file.write(f"{sample}\n")
            else:
                print('no data from', client_address)
                break

            #print(float_data)
            
    finally:
        # Clean up the connection
        connection.close()
        break

# Close the .wav file
wav_file.close()
float_data_file.close()