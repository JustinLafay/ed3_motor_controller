import serial

def send_data_to_com_port(com_port, data, baud_rate):
    # Open the serial port
    ser = serial.Serial(com_port, baud_rate, timeout=1)

    # Send data to the serial port
    ser.write(data.encode('utf-8'))

    # Close the serial port
    ser.close()

if __name__ == "__main__":
    while True:
        # Get user input for COM port
        com_port = input("Enter the COM port (e.g., COM1), or type 'exit' to quit: ")

        # Check if the user wants to exit
        if com_port.lower() == 'exit':
            break

        # Get user input for data to send
        speed = input("Select speed (0 to 4095): ")
        direction = input("Select direction (0 or 1): ")
        data_to_send = '.' + direction + ',' + speed + ';'

        # Get baud rate (fixed 9600)
        baud_rate = 9600

        send_data_to_com_port(com_port, data_to_send, baud_rate)
