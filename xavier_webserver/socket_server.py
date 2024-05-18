import socket
import signal
import audio_processor

from camera_processor import is_block

# server constants
HOST = "0.0.0.0"  # Any interface b/c we need to allow for outside clients
PORT = 8090  # Port to listen on (non-privileged ports are > 1023)

# Define the message names
# Putting these up top for documentation since the robots will need to match them
MAZE_CAM = b"Maze block check"
DUAL_FATES_LEFT = b"Dual fates left"
DUAL_FATES_RIGHT = b"Dual fates right"
DUAL_FATES_RESULT = b"Dual fates result"

MAZE_DONE = b"Maze done"
DUAL_FATES_DONE = b"Dual fates done"

powercalc = audio_processor.PowerCalculator()


# Ensures we handle ctrl-c kill gracefully
def handler(signum, frame):
    print("\nKeyboard interrupt ... exiting")
    powercalc.terminate()
    exit(0)


def main():
    left_power = 0
    right_power = 0

    maze_done = False
    dual_fates_done = False

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()

        # Loop until all 3 robots are done
        # This loops to accept a new connection to the server
        # It expects the robots to be well-behaved about closing connections once they're done
        # And not spamming the server, there's no load balancing here
        while not (maze_done and dual_fates_done):
            # This is a blocking wait, we want to hold until something needs us
            conn, addr = s.accept()

            with conn:
                # This is a blocking wait, we want the code to be reactive to incoming messages so good
                # 1024 bytes arbitrary max
                print("Received connection")
                msg = conn.recv(1024)
                print(f"Received request: {msg}")

                # Decide what to do based on the message
                # Would be nice to bind some callbacks or something but this whole thing is down and dirty
                if msg == MAZE_CAM:
                    conn.send(is_block())
                elif msg == DUAL_FATES_LEFT:
                    print("Calculating left power")
                    left_power = powercalc.calculate_power()
                    print(f"Left power: {left_power}")
                elif msg == DUAL_FATES_RIGHT:
                    print("Calculating right power")
                    right_power = powercalc.calculate_power()
                    print(f"Right power: {right_power}")
                elif msg == DUAL_FATES_RESULT:
                    print(
                        f"Chose direction: {'Left' if left_power > right_power else 'Right'}"
                    )
                    conn.send(b"Left" if left_power > right_power else b"Right")
                elif msg == MAZE_DONE:
                    maze_done = True
                elif msg == DUAL_FATES_DONE:
                    dual_fates_done = True

                # Implicitly we ignore a message that is invalid or empty (signifying closed connection)
                # Because of the with structure we don't need to close here
                # Also because of nested with structure it expects only one message per new connection
                # This avoids starvation of one robot hogging the server

    powercalc.terminate()


signal.signal(signal.SIGINT, handler)

if __name__ == "__main__":
    main()
