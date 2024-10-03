#define SAMPLE_RATE 8000
#define FFT_SIZE 256
#define FRAME_SIZE 256
#define HOP_SIZE 200

#define SPECTROGRAM_ROWS (FFT_SIZE / 2 + 1)
#define SPECTROGRAM_COLS ((SAMPLE_RATE - FRAME_SIZE) / HOP_SIZE + 1)
#define MELSPECTROGRAM_ROWS 64
#define MELSPECTROGRAM_COLS 16

//This function computes the FFT of the current audio data slice. It remembers the last slice computed in this hardocded phase
void computeFFT(float audio[SAMPLE_RATE], float spectrogram[SPECTROGRAM_ROWS][SPECTROGRAM_COLS]);