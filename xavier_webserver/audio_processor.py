from scipy.io.wavfile import read
import numpy as np
import pyaudio
import wave

# Purposefully keeping these up here because I don't want ppl modifying them
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 48000
CHUNK = 512
RECORD_SECONDS = 5


class PowerCalculator:
    def __init__(self):
        self.audio = pyaudio.PyAudio()

        # programatically get the left/right indexes, assumes left was plugged in first
        self.indexes = []

        info = self.audio.get_host_api_info_by_index(0)
        numdevices = info.get("deviceCount")

        for i in range(0, numdevices):
            # check all devices with a nonzero number of channels and keywords in their name
            if (
                self.audio.get_device_info_by_host_api_device_index(0, i).get(
                    "maxInputChannels"
                )
            ) and (
                "USB Composite Device: Audio"
                in self.audio.get_device_info_by_host_api_device_index(0, i).get("name")
            ):
                self.indexes.append(i)

        print(len(self.indexes))
        if len(self.indexes) != 1:
            raise SystemExit("Incorrect number of microphones, I exit")

    def calculate_power(self):
        # start recording
        stream = self.audio.open(
            format=FORMAT,
            channels=CHANNELS,
            rate=RATE,
            input=True,
            frames_per_buffer=CHUNK,
            input_device_index=self.indexes[0],
        )

        frames = []

        for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
            # It overflows, hopefully just ignoring it is fine
            data = stream.read(CHUNK, exception_on_overflow=False)
            frames.append(data)

        # stop recording
        stream.stop_stream()
        stream.close()

        # create the output files
        waveFile = wave.open("data.wav", "wb")
        waveFile.setnchannels(CHANNELS)
        waveFile.setsampwidth(self.audio.get_sample_size(FORMAT))
        waveFile.setframerate(RATE)
        waveFile.writeframes(b"".join(frames))
        waveFile.close()

        # read in that data
        sampling_rate, data = read("data.wav")

        # compute and return power
        return np.sqrt(np.mean(data.astype(float) ** 2))

    def terminate(self):
        self.audio.terminate()
