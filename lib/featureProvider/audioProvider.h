#define SR 8000
#define BUFFER_LENGTH 1024

class AudioProvider
{
public:
    AudioProvider(); //constructor

    //attributes
    float audio[SR];
    int totalRead = 0;

    //methods
    int update(); //update the audio
    void printAudio(); //print the audio
    
private:
    void _shift(int offset); //shift the audio to the left of a specified offset
    void _getNewData(int offset, int upperLimit);

};

//sets up PDM in main
int PDMsetup();