#include "melspectrogram.h"

/**
 * Scope of this file: obtaining the mel spectrogram from an audio sample (test)
 * STEPS:
 * 1. Split the audio in frames
 * 2. Apply hamming window
 * 3. Compute 256 point FFT for each frame
 * 4. Compute the power spectrum
 * 5. Apply filterbank
 * 6. Take the logarithm of the mel filterbank energies 
 * 
 * For now, just work up a proof-of-concept to test the algorithm
 */


float vImag[N_FFT] = {0};
float vReal[N_FFT];
ArduinoFFT<float> FFT(vReal, vImag, N_FFT, SAMPLE_RATE);

short* frame(short *audio, float *buffer, short *end)
{
    for (int i = 0; i < N_FFT; i++)
    {
        if ((audio + i) <= end)
        {
            buffer[i] = *(audio+i);
        }
        else
        {
            buffer[i] = 0.0f;
        }
    }
    
    return audio + HOP_LENGTH;
}

void extractPowerSpectrum(short *audio, float powerSpectrum[SAMPLE_RATE / HOP_LENGTH + 1][N_FFT/2 + 1])
{
    short *audioTrace = audio;
    short *end = audio + SAMPLE_RATE;
    
    int count = 0;
    while (audioTrace < end && count < NUM_FRAMES)
    {
        audioTrace = frame(audioTrace, vReal, end);
        FFT.windowing(vReal, N_FFT, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
        FFT.compute(vReal, vImag, N_FFT, FFT_FORWARD);
        FFT.complexToMagnitude(vReal, vImag, N_FFT);

        //place the magnitudes inside a matrix and square them
        for (int i = 0; i < N_FFT/2 + 1; i++)
        {
            powerSpectrum[count][i] = vReal[i] * vReal[i];
        }
        count++;

        //empty vImag
        for (int i = 0; i < N_FFT; i++)
        {
            vImag[i] = 0.0f;
        }
    }
}

void createMelFilterbank(float filterbank[N_MELS][N_FFT/2 + 1]) {
    const float sampleRate = SAMPLE_RATE;
    const int nFFT = N_FFT;
    const int nMel = N_MELS;
    const int numPoints = nMel + 2;

    // Slaney's Mel scale conversion functions
    auto hzToMel = [](float f) { return 1127.0f * logf(1.0f + f / 700.0f); };
    auto melToHz = [](float m) { return 700.0f * (expf(m / 1127.0f) - 1.0f); };

    // Frequency range (fixed-size arrays)
    float melPoints[nMel + 2];  // nMel + 2 points
    float hzPoints[nMel + 2];

    // Generate Mel-spaced points
    const float minHz = 0.0f;
    const float maxHz = sampleRate / 2.0f;
    const float minMel = hzToMel(minHz);
    const float maxMel = hzToMel(maxHz);

    // Create Mel points
    for(int i = 0; i < numPoints; i++) {
        melPoints[i] = minMel + (maxMel - minMel) * i / (numPoints - 1);
    }

    // Convert Mel points to Hz
    for(int i = 0; i < numPoints; i++) {
        hzPoints[i] = melToHz(melPoints[i]);
    }

    // Build filters
    for(int m = 0; m < nMel; m++) {
        const float leftHz = hzPoints[m];
        const float centerHz = hzPoints[m + 1];
        const float rightHz = hzPoints[m + 2];
        const float bandwidth = rightHz - leftHz;

        for(int bin = 0; bin <= nFFT/2; bin++) {
            const float binFreq = bin * sampleRate / (float)nFFT;
            float weight = 0.0f;

            if(binFreq >= leftHz && binFreq <= rightHz) {
                if(binFreq <= centerHz) {
                    weight = (binFreq - leftHz) / (centerHz - leftHz);
                } else {
                    weight = (rightHz - binFreq) / (rightHz - centerHz);
                }
            }

            // Apply Slaney-style normalization
            if(bandwidth > 0.0f) {
                weight *= 2.0f / bandwidth;
            }

            filterbank[m][bin] = weight;
        }
    }
}

void extractMelEnergies(short *audio, float melEnergies[N_MELS][NUM_FRAMES], const float melFilterbank[N_MELS][N_FFT/2 +1])
{
    float powerSpectrum[NUM_FRAMES][N_FFT/2 + 1];
    extractPowerSpectrum(audio, powerSpectrum);

    // Loop order: mel_band -> time_frame (not time_frame -> mel_band)
    for (int melIdx = 0; melIdx < N_MELS; melIdx++) {
        for (int frameIdx = 0; frameIdx < NUM_FRAMES; frameIdx++) {
            float energy = 0.0f;
            for (int binIdx = 0; binIdx < N_FFT/2 + 1; binIdx++) {
                energy += powerSpectrum[frameIdx][binIdx] * melFilterbank[melIdx][binIdx];
            }
            melEnergies[melIdx][frameIdx] = energy; // Transposed index
        }
    }
}

void powerTodB(float melspectrogram[N_MELS][NUM_FRAMES]) {
    const float amin = 1e-10f;   // Match librosa's default
    const float top_db = 52.0f;  // Match your Python code's `top_db=52`

    // Find the maximum value in the spectrogram
    float max_val = 0.0f;
    for (int mel = 0; mel < N_MELS; mel++) {
        for (int frame = 0; frame < NUM_FRAMES; frame++) {
            if (melspectrogram[mel][frame] > max_val) {
                max_val = melspectrogram[mel][frame];
            }
        }
    }

    // Ensure max_val is not zero (avoid division by zero)
    if (max_val <= 0.0f) {
        max_val = 1.0f;  // Fallback to avoid invalid log10
    }

    // Compute reference value and threshold
    float ref_val = max_val;
    float threshold = -top_db;  // Equivalent to `max_db - top_db` (max_db = 0)

    // Convert to dB and clip
    for (int mel = 0; mel < N_MELS; mel++) {
        for (int frame = 0; frame < NUM_FRAMES; frame++) {
            // Apply amin and compute dB
            float magnitude = fmaxf(melspectrogram[mel][frame], amin);
            float db = 10 * log10f(magnitude / ref_val);

            // Clip values below the threshold
            db = fmaxf(db, threshold);

            melspectrogram[mel][frame] = db;
        }
    }
}

void melspectrogram(short *audio, float melspectrogram[N_MELS][NUM_FRAMES], const float melFilterbank[N_MELS][N_FFT/2 +1])
{
    extractMelEnergies(audio, melspectrogram, melFilterbank);
    powerTodB(melspectrogram);
}