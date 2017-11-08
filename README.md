# NutshellVC
A C++ API that provides a bunch of methods to create your own video conference with an option of streaming.

# Structure
- Socket Library
  - Socket (a base class)
  - TCPServerSocket (using a tcp protocol on a server side)
  - CSRSocket (conforms an interface of connect-send-receive)
  - TCPSocket
  - UDPSocket
- Camera Library 
  - Initializes VideoCapture
  - Encodes frames to a proper format
  - Captures a scene

# Libraries
- Sockets standard library (a preprocessor chooses the right one depending on a platform)
- OpenCV 3.3.1
