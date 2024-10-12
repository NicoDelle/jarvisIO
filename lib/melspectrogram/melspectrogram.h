#define FFT_SIZE 256
#define SAMPLE_RATE 8000
#define FRAME_SIZE 256 //2048 is the default frame size in Librosa
#define HOP_SIZE 64 //The default hop size in Librosa is 512
#define SPECTROGRAM_ROWS 64
#define SPECTROGRAM_COLS 16
//This function computes the FFT of the current audio data slice. It remembers the last slice computed in this hardocded phase
void computeFFT(float audio[SAMPLE_RATE], float spectrogram[SPECTROGRAM_ROWS][SPECTROGRAM_COLS], float melFilterBanks[SPECTROGRAM_ROWS][FFT_SIZE / 2 + 1]);
void computeFilterBanks(float melFilterBanks[SPECTROGRAM_ROWS][FFT_SIZE / 2 + 1]);