# Jarvis voice activation
This project aims to realize the activation command for a virtual assistant named Jarvis.
The software is designed and tested for the Arduino RP2040 Connect board, and uses an implementation of the TensorFlowLite library

More informations about the model design can be found at <a href='https://colab.research.google.com/drive/1Ihl9pvpRumX6IvtbPXtNWEnzJjm92hOk?usp=sharing'>this</a> colab
This project is developed using the VSCode extension for PlatformIO: to work on this project, you will have to install it and then clone the repo.
Make sure to install the dependencies shown in the "platformio.ini" file

## Structure of the project
The main file is located in the src folder, whit some auxiliary headers. Other functions are defined in separate folders inside the lib directory
The utils folders contains some code related to this project, showing how to implement different useful things.

## Current state of developement
The model should now work properly. 
The main task now is to reduce preprocessing time. At the moment, it takes around 730 ms, while inference takes < 20 ms. Optimizing preprocessing and making the code work real time should be the last things left for the voice activation part.
The last thing to do should be set up an UDP (or TCP?) connection to stream the audio after the keyword is detected
