# Jarvis voice activation
This project aims to realize the activation command for a virtual assistant named Jarvis.
The software is designed and tested for the Arduino RP2040 Connect board, and uses an implementation of the TensorFlowLite library

More informations about the model design can be found at <a href='https://colab.research.google.com/drive/1T4DUBaDO2eZGwK2yVVtz14g87teJgTwV?usp=sharing'>this</a>  (for the general workflow idea) and <a href='https://colab.research.google.com/drive/1Ihl9pvpRumX6IvtbPXtNWEnzJjm92hOk?usp=sharing'>this</a> (for the structure of the model currently in developement)

## Software used
This project is developed using the VSCode extension for PlatformIO: to work on this project, you will have to install it and then clone the repo.
Make sure to install the dependencies shown in the "platformio.ini" file

## Structure of the project
The main file is located in the src folder, whit some auxiliary headers. Other functions are defined in separate folders inside the lib directory
The utils folders contains some code related to this project, showing how to implement different useful things.
